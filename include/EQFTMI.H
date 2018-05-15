/*! EQFTMI.H      Internal header file for Translation Memory functions
	Copyright (c) 1990-2016, International Business Machines Corporation and others. All rights reserved.
*/

/**********************************************************************/
/* This include file requires EQFDDE.H!!!                             */
/**********************************************************************/
#ifndef EQFDDE_INCLUDED
  #include "EQFDDE.H"
#endif

// include external memory export/import interface
#include "EQFMEMIE.H"

#ifndef _OTMFUNC_H_INCLUDED
  #include "OTMFUNC.H"
#endif

#ifndef FIELDOFFSET
  #define FIELDOFFSET(type, field)    (LOWORD(&(((type *)0)->field)))
#endif

// file extension used for shared memory (LAN based) property files in property directory
#define LANSHARED_MEM_PROP ".SLM"


/*---------------------------------------------------------------------*\
                 #Define
\*---------------------------------------------------------------------*/

// memory export and import formats
#define MEM_SGMLFORMAT_ANSI     1
#define MEM_SGMLFORMAT_ASCII    2
#define MEM_SGMLFORMAT_UNICODE  3
#define MEM_FORMAT_TMX          4
// the same define value is used for the import as TMX (Trados) and for export as TMX (UTF-8)
#define MEM_FORMAT_TMX_UTF8     5
#define MEM_FORMAT_TMX_TRADOS   5

#define MEM_FORMAT_TMX_NOCRLF   6
#define MEM_FORMAT_TMX_UTF8_NOCRLF 7
#define MEM_FORMAT_XLIFF_MT     8


// filter for mem SGML formats (same sequence as memory export and import formats required)
//#define MEM_FORMAT_FILTERS "SGML ANSI\0*.*\0SGML ASCII\0*.*\0SGML Unicode\0*.*\0\0\0"
#define MEM_FORMAT_FILTERS "SGML ANSI\0*.*\0SGML ASCII\0*.*\0SGML UTF-16\0*.*\0TMX\0*.*\0TMX (Trados)\0*.*\0XLIFF (MT)\0*.*\0\0\0"

// filter for mem SGML formats (same sequence as memory export and import formats required)
//#define MEM_FORMAT_FILTERS_EXP "SGML ANSI\0*.*\0SGML ASCII\0*.*\0SGML Unicode\0*.*\0\0\0"
#define MEM_FORMAT_FILTERS_EXP "SGML ANSI (*.EXP)\0*.EXP\0SGML ASCII (*.EXP)\0*.EXP\0SGML UTF-16 (*.EXP)\0*.EXP\0TMX (UTF-16) (*.TMX)\0*.TMX\0TMX (UTF-8) (*.TMX)\0*.TMX\0TMX (UTF-16) (remove CRLF) (*.TMX)\0*.TMX\0TMX (UTF-8) (remove CRLF) (*.TMX)\0*.TMX\0\0\0"

// defines for retries in case of BTREE_IN_USE conditions
#define MAX_RETRY_COUNT 30
#define MAX_WAIT_TIME   100

#define TMT_CODE_VERSION    5            // Code version
#define TM_PREFIX          "EQFTMT$$"     // TM DB identifier
#define HUNDRED           100          //
// ID to be used if a table overflow occurs
#define OVERFLOW_ID       32000
// name to be used if a table overflow occurs
#define OVERFLOW_NAME     "$$$$$$$$"

#define LAST_CLUSTER_NUMBER  4095

//--- size of the message buffer for the Error message               /*@1108A*/
//--- ERROR_MEM_SEGMENT_TOO_LARGE_CON                                /*@1108A*/
#define MSG_BUFFER 35                                                /*@1108A*/


#define MIN_SPOOL_SIZE      20480  // 20k bytes for preformatted TM blocks
#define SECTOR_SIZE          4096  // Disk sector. Fixed now, dynamic later
#define BLOCK_HEADER_SIZE       8  // size of block header
#define MAX_TM_HEADER_SIZE  47600  // max size of TM header
#define MAX_BLOCK_SIZE       4096  // Maximum block size
#define MAX_TEXT_TAB_SIZE (MAX_TAGS_TAB_ENTRIES * sizeof(TOKENENTRY))
#define MAX_WORDS_TAB_SIZE (MAX_WORDS_TAB_ENTRIES * sizeof(TOKENENTRY))

#define MAX_TM_WORK_AREA  ( MAX_TGT_LENGTH + 300 )
                               /* Work areas to convert LF into CRFL   */
                               /* and the other way round              */

// The following define should not exceed 65535 Bytes
#define MAX_TMT_GLOBALS_SIZE (sizeof(TMT_GLOBALS) +\
                             MAX_TM_HEADER_SIZE +\
                             MAX_BLOCK_SIZE+\
                             MAX_TEXT_TAB_SIZE+\
                             MAX_WORDS_TAB_SIZE)


#define MAX_WORDS_TAB_ENTRIES  200 /* Number of entries in WordsTable  */
#define MAX_TAGS_TAB_ENTRIES   100 /* number of entries in TextTable   */
#define KEY_DIR_ENTRIES_NUM   4096 /* number of entries key directory  */

#define TOP_BLOCK_NUMBER (USHORT)65535
                               /* 2**16 - 1, the max number of a block */
#define MAX_BLOCKS_NUM   (USHORT)65535
                               /* 2**16 - 1, maximal number of blocks  */
#define GROUP_NUM           8  /* number of distribution groups        */

#define  TAG_STARTING_VALUE 0  /* the lowest attribute to tags in text */
                               /* table                                */
#ifndef  TM_ADM
#define MAX_SERVER_NAME    15  /* length of server name                */
#endif

/* The two defines below should not be used. Displacement can be       */
/* computed in runtime from the structure.                             */
#define SEG_LENGTH_DISP     5  /* disp of length field in a SEGMENT    */
#define DEL_FLAG_DISP       6  /* disp of del flag from segment start  */

// data compress flag
#define   BOCU_COMPRESS  0x01


/* define for write Tm Header function                                 */
#define WF_ALL            0x0000  /* Write the whole header............*/
#define WF_CORRUPT        0x0001
#define WF_TMBLOCKS       0x0002
#define WF_FREEBLOCKS     0x0004
#define WF_FIRSTAVAIL     0x0008
#define WF_UPDATE         0x4000  /* Write all but the key directory...*/
#define WF_KEYDIR         0x8000  /* Write the key directory...........*/


/*---------------------------------------------------------------------*\
  match type definitions
\*---------------------------------------------------------------------*/
#define USE_CRLF       1
#define ALIGN_CRLF     2
#define IGNORE_CRLF    3

/* return codes for the Tmt Commands                                   */
#define OK                  NO_ERROR    /* for Tmt commands            */



/*----------------------------------------------------------------------------*\
  System wide lengthes for correction of the thresholds in dependency
  of the length of the input segment
\*----------------------------------------------------------------------------*/
//@@@                           // segments, defined thru LENGTH_SHORTER_VALUE
#define LENGTH_SHORTER_VALUE   2      // Length for shorter segment
//@@@                           // segments, defined thru LENGTH_SHORTER_VALUE
#define LENGTH_SHORT_VALUE     6      // Length for short segment
#define LENGTH_MEDIUM_VALUE   10      // Length for medium segment
#define LENGTH_LONG_VALUE     20      // Length for long segment

/*----------------------------------------------------------------------------*\
  System wide percentages for correction of the MATCH threshold
\*----------------------------------------------------------------------------*/
#define MATCH_PERCENTAGE_SHORT_VALUE    1L     // percentage for short segment
#define MATCH_PERCENTAGE_MEDIUM_VALUE  10L     // percentage for medium segment
#define MATCH_PERCENTAGE_LONG_VALUE    20L     // percentage for long segment
#define MATCH_PERCENTAGE_LONGER_VALUE  40L     // percentage for longer segment

/*----------------------------------------------------------------------------*\
  System wide percentages for correction of the LENGTH threshold
\*----------------------------------------------------------------------------*/
#define LENGTH_PERCENTAGE_SHORT_VALUE    1L     // percentage for short segment
#define LENGTH_PERCENTAGE_MEDIUM_VALUE  10L     // percentage for medium segment
#define LENGTH_PERCENTAGE_LONG_VALUE    20L     // percentage for long segment
#define LENGTH_PERCENTAGE_LONGER_VALUE  30L     // percentage for longer segment

/*---------------------------------------------------------------------*\
                            Macro Definition
\*---------------------------------------------------------------------*/
#define FPTR(type, field) ((ULONG)(FIELDOFFSET(type, field)))

/**********************************************************************/
/* macro to build index name from fully qualified data name           */
/**********************************************************************/
#define INDEXNAMEFROMMEMPATH( mem, ind ) \
  { \
  Utlstrccpy( ind, mem, DOT ); \
  if ( strcmp( strrchr( mem, DOT ), EXT_OF_SHARED_MEM ) == 0 ) \
    strcat( ind, EXT_OF_SHARED_MEMINDEX ); \
  else \
    strcat( ind, EXT_OF_TMINDEX ); \
  }

  // replace any 0xA0 in language name to 0xFF
  // (0xA0 is 0xFF after processing by OemToAnsi)
  #define REPLACE_A0_BY_FF( p ) \
  {                          \
    PBYTE pbTemp = (PBYTE)p; \
    while ( *pbTemp )        \
    {                        \
      if ( *pbTemp == 0xA0 ) \
      {                      \
        *pbTemp = 0xFF;      \
      }                      \
      pbTemp++;              \
    }                        \
  }

/*---------------------------------------------------------------------*\
                 Typedefs Definition
+-----------------------------------------------------------------------+
  Variable prefixes:
  USHORT id, num, len, disp
  ULONG  ldisp
  CHAR   buf[]
\*---------------------------------------------------------------------*/


typedef struct _TM_HEADER_UPDATE { /* tmhu */
   BOOL    fCorruption ;
   USHORT  usNumTMBlocks,    /* number of blocks in the TM             */
           usNumFreeBlocks,  /* # of free, pre-formatted blocks        */
           usFirstAvailBlock;/* points to first available block        */
} TM_HEADER_UPDATE;

typedef union  _UNIONIN
{
   PREFIX_IN         in;
   ADD_IN            ain;
   DEL_IN            din;
   REP_IN            rin;
   EXT_IN            ein;
   GET_IN            gin;
   CLOSE_IN          clin;
   OPEN_IN           oin;
   CREATE_IN         crin;
   INFO_IN           infoin;
   CLOSEHANDLER_IN   chin;
   EXIT_IN           xin;
   DRIVES_IN         drivesin;
   FILE_LIST_IN      filelistin; // !!! CHM
   GETPART_IN        getpartin;
   PUTPROP_IN        putpropin;
   PUTDICTPROP_IN    PutDictPropIn; // !!! CHM added
   PUTPROPCOMBINED_IN PutPropCombinedIn; // !!! CHM added
   GETPROP_IN        getpropin;
   ENDORG_IN         endorgin;
   DELTM_IN          deltmin;
   DELFILE_IN        delfilein;
   FILEINFO_IN       fileinfoin;
   RENFILE_IN        renfilein;
} UNIONIN, * PUNIONIN; /* uin */

typedef union  _UNIONOUT
{
   PREFIX_OUT  out;
   ADD_OUT     aout;
   DEL_OUT     dout;
   REP_OUT     rout;
   EXT_OUT     eout;
   GET_OUT     gout;
   CLOSE_OUT   clout;
   OPEN_OUT    oout;
   CREATE_OUT  crout;
   INFO_OUT    infoout;
   EXIT_OUT    xout;
   DRIVES_OUT    drivesout;
   FILE_LIST_OUT filelistout; // !!! CHM
   GETPART_OUT   getpartout;
   PUTPROP_OUT   putpropout;
   GETPROP_OUT   getpropout;
   GETDICTPROP_OUT      GetDictPropOut; // !!! CHM added
   GETPROPCOMBINED_OUT  GetPropCombinedOut; // !!! CHM added
   ENDORG_OUT    endorgout;
   DELTM_OUT     deltmout;
   DELFILE_OUT   delfileout;
   FILEINFO_OUT  fileinfoout;
   RENFILE_OUT   renfileout;
} UNIONOUT, * PUNIONOUT; /* uout */

/*
 +---------------------------------------------------------------------------+
    Name:         fTmComInit
    Purpose:      Initialise the TM Communication code (U code)
    Parameters:   1.VOID   - nothing
    Returns:      BOOL - TRUE -> initialization completed successfully
                         FALSE -> initialization ended with an error
    Comments:     This procedure initalizes the TM Communication code (U
                  code). At the moment this only resets the top pointer of
                  the TM Handle information list.
    Samples:      fRc = fTmComInit ();
 +---------------------------------------------------------------------------+
*/
BOOL fTmComInit (VOID);

VOID   QDAMDeleteFileRemote( PDELFILE_IN  pDelFileIn,      // Input structure
                             PDELFILE_OUT pDelFileOut );   // Output structure

typedef struct _TMT_GLOBALS { /* tmtg */
 /* this structure keeps data and pointers to variables that exist from*/
 /* the point that the TM was created or opened, until close command   */
 /* BlockImage, TextTable, WordsTable and TmHeader are allocated       */
 /* dynamically during Create/Open  and are freed at Close time.       */
  HFILE    hfTM;                  /* handle to TM db (from Dos call)   */
  PUCHAR   pchBlockImage;         /* pointer to block image            */
  PTOKENENTRY pteTextTable,       /* pointer to TextTable              */
              pteWordsTable,      /* pointer to WordsTable             */
              pteFirstSigWord,    /* pointer to first significant word */
              pteSecondSigWord;   /* pointer to second significant word*/
  PPREFIX_OUT pPrefixOut;         /* pointer to the current PrefixOut  */
                                  /* to be used by FormatMore          */
  SEGMENT     seg;                /* placed here instead of allocating */
                                  /* this space on stack (point 276)   */
  MATCH       mtch;               /* placed here instead of allocating */
                                  /* this space on stack (point 276)   */
  PCHAR       pWorkArea1;         /* Pointer to work area 1            */
  PCHAR       pWorkArea2;         /* Pointer to work area 2            */
  PTM_HEADER  pTmHeader;          /* pointer to Tm header structure    */
  LONG        lActLengthThr;      /* length corrected length threshold values*/
  LONG        lActMatchThr;       /* length corrected match threshold values */
  PLOADEDTABLE pLoadedTagTable;   /* pointer to loaded tagtable        */
  PVOID        pstLoadedTagTable; /* pointer to loaded tagtable, used for */
                                  /* function TATagTokenize           */
} TMT_GLOBALS, * PTMT_GLOBALS, ** PPTMT_GLOBALS;

// ***************** Work Constants global **************************
#define  INIT_NUMB_OF_ENTRIES     5   // Initial number of entries in a table or List
#define  SEG_NUMB                 6   // Segment number in char.
#define  ASCII_FLAG               1   // Ascii flag converted to char.
#define  MACH_TRANS               1   // Machine trans.flag converted to char.
#define  TIME_STAMP              16   // Time integer converted to character
#define  IND_CODES               20   // External industry codes space to hold a maximum of 6 codes
#define  ASCII_IND_CODES_LENGTH   3   // Length of Industry codes in external format
#define  NUMB_OF_IND_CODES        6   // Number of Industry codes in external format
#define  MEM_NAME                 8   // Length of memory database name without extension
#define  DRIVE_NAME              12   // Special drive name
#define  PTR_MEMHANDLER_IDA       0   // Relative position in the extra bytes for handler
#define  PTR_MEM_IDA              0   // Relative position in the extra bytes for instance
#define  PTR_CRT_IDA              0   // Relative position in the extra bytes for create dialog
#define  PTR_DLG_IDA              0   // Relative position in the extra bytes for dialogs
#define  TEMP                    40   // Size of temporaty work area
                                      // of WinRegisterClass call.
#define  TEXT_LINE_LENGTH             40  // Text line length
#define  TEXT_100_CHAR               100  // string length for temporary strings
//#define  NUMB_OF_TOKENS             1000  // Number of tokens in the token list
#define  NUMB_OF_TOKENS             8000  // Number of tokens in the toklist:RJ increase nec.
                                          // if 65520 bytes are read-in at once during MemImport
#define  MEM_PROP_SIZE              2048  // Global size of all memory database properties
#define  MEM_BLOCK_SIZE             1024  // Translation Memory block size
#define  SEGMENT_CLUSTER_LENGTH        4  // Length of segment cluster
#define  SEGMENT_NUMBER_LENGTH         6  // Length of segment number
#define  PROCESS_NUMB_OF_MESSAGES     10  // Number of messages to be process till next message is issued
#define  MEM_DBCS                      0  // 0 = No support,    1 = DBCS support
#define  MEM_LOAD_PATTERN_NAME        "*" // Default names to be shown for load dialog
#define  MEM_LOAD_PATTERN_EXT        ".*" // Default extension to be shown for load dialog
#define  MEM_EXPORT_WORK_AREA       6000  // Length of export work area
#define  EQF_IDENTIFICATION         "EQF" // EQF identification
#define  CLBCOL_TITLE_STRING        "                   " // Placeholder for of CLBCOL titles
#define  MEM_EXPORT_OUT_BUFFER      8200  // Output buffer for export



// *****************  Memory database load **************************
//#define  MEM_CONTEXT_TOKEN_END     0     // </Context>
//#define  MEM_CONTROL_TOKEN_END     1     // </Control> UNUSED!
//#define  NTM_DESCRIPTION_TOKEN_END 2     // </Description>
//#define  MEM_MEMORYDB_TOKEN_END    3     // </MemoryDb>
//#define  NTM_MEMORYDB_TOKEN_END    4     // </NTMMemoryDb>
//#define  MEM_SEGMENT_TOKEN_END     5     // </Segment>
//#define  MEM_SOURCE_TOKEN_END      6     // </Source>
//#define  MEM_TARGET_TOKEN_END      7     // </Target>
//#define  MEM_CONTEXT_TOKEN         8     // <Context>
//#define  MEM_CONTROL_TOKEN         9     // <Control>
//#define  NTM_DESCRIPTION_TOKEN    10     // <Description>
//#define  MEM_MEMORYDB_TOKEN       11     // <MemoryDb>
//#define  NTM_MEMORYDB_TOKEN       12     // <NTMMemoryDb>
//#define  MEM_SEGMENT_TOKEN        13     // <Segment>
//#define  MEM_SOURCE_TOKEN         14     // <Source>
//#define  MEM_TARGET_TOKEN         15     // <Target>

// strings to create the token IDs for the tags dynamically using TATagTokenize
#define  MEM_CONTEXT_TOKEN_END     L"</Context>"
#define  MEM_CONTROL_TOKEN_END     L"</Control>"
#define  NTM_DESCRIPTION_TOKEN_END L"</Description>"
#define  MEM_MEMORYDB_TOKEN_END    L"</MemoryDb>"
#define  NTM_MEMORYDB_TOKEN_END    L"</NTMMemoryDb>"
#define  MEM_SEGMENT_TOKEN_END     L"</Segment>"
#define  MEM_SOURCE_TOKEN_END      L"</Source>"
#define  MEM_TARGET_TOKEN_END      L"</Target>"
#define  MEM_CONTEXT_TOKEN         L"<Context>"
#define  MEM_CONTROL_TOKEN         L"<Control>"
#define  NTM_DESCRIPTION_TOKEN     L"<Description>"
#define  MEM_MEMORYDB_TOKEN        L"<MemoryDb>"
#define  NTM_MEMORYDB_TOKEN        L"<NTMMemoryDb>"
#define  MEM_SEGMENT_TOKEN         L"<Segment>"
#define  MEM_SOURCE_TOKEN          L"<Source>"
#define  MEM_TARGET_TOKEN          L"<Target>"
#define  MEM_CODEPAGE_TOKEN        L"<CodePage>"
#define  MEM_CODEPAGE_TOKEN_END    L"</CodePage>"
#define  MEM_ADDDATA_TOKEN_END     L"</AddData>"
#define  MEM_ADDDATA_TOKEN         L"<AddData>"




#define  TM_IMPORT_OK              0     // Translation memory import OK
#define  TM_IMPORT_FAILED          1     // Translation memory import failed
#define  TM_IMPORT_FORCED          2     // Translation memory import was forced


//used for input from create folder dialog
typedef enum
{
  MEM_CREATE_NAME_IND,
  MEM_CREATE_MARKUP_IND,
  MEM_CREATE_SOURCELANG_IND,
  MEM_CREATE_TARGETLANG_IND
} MEM_CREATEINDEX;

#define  MEM_TEXT_BUFFER         65520     // Length of the text buffer


// ************ Definitions for TM list box display *********************
typedef enum
{
  MEM_OBJECT_IND,
  MEM_NAME_IND,
  MEM_DESCRIPTION_IND,
  MEM_SIZE_IND,
  MEM_DRIVE_IND,
  MEM_SERVER_IND,
  MEM_OWNER_IND,
  MEM_SOURCELNG_IND,
  MEM_TARGETLNG_IND,
  MEM_MARKUP_IND
} MEM_ITEMINDX;

/**********************************************************************/
/* string IDs to parse the CONTROL string of the TM exported format   */
/* using UtlParseX15                                                  */
/**********************************************************************/
//#define NTM_SEGNR_ID          0   //seg nr from analysis
//#define NTM_MTFLAG_ID         1   //machine translation flag
//#define NTM_TIME_ID           2   //time stamp
//#define NTM_SOURCELNG_ID      3   //source language
//#define NTM_TARGETLNG_ID      4   //target language
//#define NTM_AUTHORNAME_ID     5   //author name
//#define NTM_TAGTABLENAME_ID   6   //tag table name
//#define NTM_FILENAME_ID       7        //file name

typedef enum
{
  NTM_SEGNR_ID,
  NTM_MTFLAG_ID,
  NTM_TIME_ID,
  NTM_SOURCELANG_ID,
  NTM_TARGETLANG_ID,
  NTM_AUTHORNAME_ID,
  NTM_TAGTABLENAME_ID,
  NTM_FILENAME_ID,
  NTM_LONGNAME_ID
} NTM_IMPORTINDEX;

#define START_KEY             0xFFFFFF

#define EXT_OF_RENAMED_TMDATA  ".TRD"
#define EXT_OF_RENAMED_TMINDEX ".TRI"
#define EXT_OF_TEMP_TMPROP     ".TMR"
#define EXT_OF_RENAMED_TMPROP  ".RMR"
#define EXT_OF_TEMP_TMDATA     ".TTD"
#define EXT_OF_TEMP_TMINDEX    ".TTI"
#define EXT_OF_TMPROP          EXT_OF_MEM


// ************ Type definitions ****************************************

typedef struct _SEG_CTRL_DATA
{
 CHAR     chSegmentNumber[SEG_NUMB];  /* Segment number converted to character*/
 CHAR     chAsciiFlag[ASCII_FLAG];    /* Ascii flag converted to char.        */
 CHAR     chMachineTrans[MACH_TRANS]; /* Machine trans.flag converted to char.*/
 CHAR     chTimeStamp[TIME_STAMP];    /* Time integer converted to character  */
 CHAR     chIndustryCodes[IND_CODES]; /* Industry codes                       */
 CHAR     chFileName[MAX_FILESPEC-1]; /* Segment origin. from that file name  */
}SEG_CTRL_DATA, * PSEG_CTRL_DATA;


typedef struct _MEM_PROC_HANDLE
{
 CHAR   * pDataArea;                   /* Pointer to process data area        */
 USHORT   usMessage;                   /* Termination message for process     */
}MEM_PROC_HANDLE, * PMEM_PROC_HANDLE;

typedef struct _MEM_IDA
{
 IDA_HEAD stIdaHead;                     // Standard Ida head
 HWND     hWndListBox;                   // Handle of memory window listbox
 SHORT    sLastUsedViewList[MAX_VIEW];   // last used view list
 SHORT    sDetailsViewList[MAX_VIEW];    // user's details view list'
 SHORT    sSortList[MAX_VIEW];           // user's sort criteria list'
 HWND     hWnd;                          // Handle of memory window
 CHAR     szTemp[4096];                  // Temporary work area
 CHAR     szTempPath[MAX_EQF_PATH];      // Temporary work area for a path
 CHAR     szMemPath[4096];               // Full path to TM starting an operation
 CHAR     szMemName[MAX_LONGFILESPEC];   // (long) TM name
 CHAR     szMemObjName[2*MAX_LONGFILESPEC];   // memory object name ( pluginname + ":" + (long) TM name)
 CHAR     szPluginName[MAX_LONGFILESPEC]; // name of plugin for this memory
 LONG     lTemp;                         // Temporary long field
 CHAR     szMemFullPath[MAX_EQF_PATH];   // Full path to memory database
 CHAR     szSecondaryDrives[MAX_DRIVELIST]; // Drives of secondary installations
 CHAR   * pWorkPointerLoad;                 // A work pointer for the load proc.
// SHORT    sNumberOfMem;                     // Number of memory databases entries available
// SHORT    sNumberOfMemUsed;                 // Number of memory databases entries used
 PMEM_PROC_HANDLE  pMemProcHandles;         // pointer to List of proc. handle
 SHORT    sNumberOfProc;                    // Number of process handle entries
 SHORT    sNumberOfProcUsed;                // Number of proc. handle entries used
 SHORT    sRunningProcesses;                // Number of concurrently running processes
 CHAR     szText[TEXT_LINE_LENGTH];         // Containing text
// CHAR     szMemTopItem[MAX_FNAME +          // In case of refresh contains top item of listbox
//                       MAX_DESCRIPTION + 5];// the 5 additional char's are for \x15 control characters
// CHAR     szMemSelectedItem[MAX_FNAME +     // In case of refresh contains selected item of listbox
//                       MAX_DESCRIPTION + 5];// the 5 additional char's are for \x15 control characters
 CHAR     szProcessTaskWithObject[MAX_FILESPEC]; // A Task has to be processed with this Object
 CHAR     szNAString[TEXT_LINE_LENGTH];     // String to store the "N/A" string
}MEM_IDA, * PMEM_IDA;

#ifdef _OTMMEMORY_H_
typedef struct _MEM_LOAD_IDA
{
 CHAR         szMemName[MAX_LONGFILESPEC];    // Memory database name
 CHAR         szShortMemName[MAX_FILESPEC];   // Memory database name
 CHAR         szMemPath[2048];                // Full memory database name + path
 CHAR         szFilePath[2048];               // Full file name + path
 BOOL         fControlFound;                  // Indicator whether a CONTROL token was found
 OtmMemory    *pMem;                          // pointer to memory object
 OtmProposal  *pProposal;                     // buffer for memory proposal data
 CHAR_W       szSegBuffer[MAX_SEGMENT_SIZE+1];// buffer for segment data
 CHAR_W       szSource[MAX_SEGMENT_SIZE+1];   // buffer for segment source data
 HFILE        hFile;                          // Handle of file to be loaded
 HWND         hProgressWindow;                // Handle of progress indicator window
 ULONG        ulProgressPos;                  // position of progress indicator
 ULONG        ulBytesRead;                    // bytes already read from the import file
 ULONG        ulTotalSize;                    // total size of import file
 BOOL         fFirstRead;                     // Read Flag
 PLOADEDTABLE pFormatTable;                   // Pointer to Format Table
 PTOKENENTRY  pTokenList;                     // Pointer to Token List
 BOOL         fEOF;                           // Indicates end of file
 PTOKENENTRY  pTokenEntry;                    // A pointer to token entries
 PTOKENENTRY  pTokenEntryWork;                // A work pointer to token entries
 ULONG        ulSegmentCounter;               // Segment counter of progress window
 ULONG        ulInvSegmentCounter;            // Invalid Segment counter
 ULONG        ulResetSegmentCounter;          // Segments using generic markup when not valid
 CHAR         szSegmentID[SEGMENT_CLUSTER_LENGTH +
                          SEGMENT_NUMBER_LENGTH + 2]; // Segment identification
 CHAR         szFullFtabPath[MAX_EQF_PATH];   // Full path to tag tables
 CHAR_W       szTextBuffer[MEM_TEXT_BUFFER];  // Text buffer to keep the file
 PMEM_IDA     pIDA;                           // Address to the memory database IDA
 BOOL         fDisplayNotReplacedMessage;     // message flag if segment not /*@47A*/
                                              // not replaced message should /*@47A*/
                                              // be displayed or not         /*@47A*/
 CHAR         szTargetLang[MAX_LANG_LENGTH];
 CHAR         szTagTable[MAX_FNAME];
 BOOL         fTagLangDlgCanceled;
 USHORT       usTmVersion;
 BOOL         fBatch;                         // TRUE = we are in batch mode
 HWND         hwndErrMsg;                     // parent handle for error messages
 PDDEMEMIMP   pDDEMemImp;                     // ptr to batch memory import data
 HWND         hwndNotify;                 // send notification about completion
 OBJNAME      szObjName;                  // name of object to be finished
 CHAR         szDescription[MAX_MEM_DESCRIPTION];   //description of memory database
 CHAR         szTempPath[MAX_PATH144];        // buffer for path names
 USHORT       usImpMode;                  // mode for import
 BOOL         fMerge;                     // TRUE = TM is being merged
 PSZ          pszList;                    // list of imported files or NULL
 PSZ          pszActFile;                 // ptr to active file or NULL
 ULONG        ulFilled;                   // filled during read (in num CHAR_Ws)
 BOOL         fSkipInvalidSegments;       // TRUE = skip invalid segments w/o notice
 ULONG        ulOemCP;                    // CP of system preferences lang.
 ULONG        ulAnsiCP;                   // CP of system preferences lang.
 BOOL         fIgnoreEqualSegments;       // TRUE = ignore segments with identical source and target string
 BOOL         fAdjustTrailingWhitespace;  // TRUE = adjust trailing whitespace of target string

 // dynamically evaluated token IDs
 SHORT sContextEndTokenID;
 SHORT sControlEndTokenID;
 SHORT sDescriptionEndTokenID;
 SHORT sMemMemoryDBEndTokenID;
 SHORT sNTMMemoryDBEndTokenID;
 SHORT sSegmentEndTokenID;
 SHORT sSourceEndTokenID;
 SHORT sTargetEndTokenID;
 SHORT sContextTokenID;
 SHORT sControlTokenID;
 SHORT sDescriptionTokenID;
 SHORT sMemMemoryDBTokenID;
 SHORT sNTMMemoryDBTokenID;
 SHORT sSegmentTokenID;
 SHORT sSourceTokenID;
 SHORT sTargetTokenID;
 SHORT sCodePageTokenID;
 SHORT sCodePageEndTokenID;
 SHORT sAddInfoEndTokenID;
 SHORT sAddInfoTokenID;

 // fields for external memory import methods
 HMODULE       hmodMemExtImport;                 // handle of external import module/DLL
 PMEMEXPIMPINFO pstMemInfo;                        // buffer for memory information
 PMEMEXPIMPSEG  pstSegment;                        // buffer for segment data
 PFN_EXTMEMIMPORTSTART   pfnMemImpStart;         // function handling start of import
 PFN_EXTMEMIMPORTPROCESS pfnMemImpProcess;       // function handling import of a segment
 PFN_EXTMEMIMPORTEND     pfnMemImpEnd;           // function handling end of export
 LONG          lExternalImportHandle;            // handle of external memory import functions

 BOOL         fMTReceiveCounting;         // TRUE = count received words and write counts to folder properties
 OBJNAME      szFolObjName;               // object name of folder (only set with fMTReceiveCounting = TRUE)
 ULONG        ulMTReceivedWords[3];       // received words per category
 ULONG        ulMTReceivedSegs[3];        // received segments per category
 LONG         lProgress;                  // progress indicator of external import methods
 BOOL         fYesToAll;                  // yes-to-all flag for merge confirmation
 FILE         *hSegLog;                   // handle of segment log file
 CHAR         szSegLog[MAX_EQF_PATH];     // name of segment log file
 BOOL          fForceNewMatchID;          // create a new match segment ID even when the match already has one
 BOOL          fCreateMatchID;            // create match segment IDs
 CHAR          szMatchIDPrefix[256];      // prefix to be used for the match segment ID
 CHAR_W        szMatchIDPrefixW[256];     // prefix to be used for the match segment ID (UTF16 version)
 CHAR_W        szMatchSegIdInfo[512];     // buffer for the complete match segment ID entry
 ULONG         ulSequenceNumber;          // segment sequence number within current external memory file
}MEM_LOAD_IDA, * PMEM_LOAD_IDA;

typedef struct _MEM_MERGE_IDA
{
 CHAR          szPathMergeMem[MAX_LONGPATH];  // Full path and name of TM to be merged
 CHAR          szPathMergeProp[MAX_LONGPATH]; // Full path and name of Properties to be imported
 CHAR          szDriveMergeMem[MAX_DRIVE];    // Drive letter of TM to be merged w o : ( d )
 CHAR          szNameMergeMem[MAX_LONGFILESPEC];// Name of TM to be merged w o ext ( heller )
 CHAR          szShortNameMergeMem[MAX_FILESPEC];// Short name of TM being merged
 CHAR          szDirMergeMem[MAX_LONGPATH];   // Path of directories fo TM to be merged ( \eqf\import\ )
 CHAR          szExtMergeMem[MAX_FEXT];       // Ext of TM to be merged ( MIP or MEM or ? )
 OtmMemory     *pMergeMem;                     // TM handle of TM to be merged
 CHAR          szInvokingHandler[40];         // ??? 40 ???? to be in Gerds stuff. Name of handler which invoked the process
 CHAR          szSystemPath[MAX_EQF_PATH];    // Path to the EQF system
 CHAR          szTemp[MAX_EQF_PATH];          // Temporary path area
 CHAR          szPropName[MAX_FILESPEC];      // TM property name ( xxxx.MEM )
 CHAR          szPathMem[MAX_LONGPATH];       // Full name and Path to the TM ( c:\EQF\MEM\xxxx.MEM )
 CHAR          szPathProp[MAX_LONGPATH];      // Full name and Path to the Properties
 CHAR          szDriveMem[MAX_DRIVE];         // Drive letter of TM w o : ( h )
 CHAR          szNameMem[MAX_LONGFILESPEC];   // Long name of TM w o ext ( gallus )
 CHAR          szShortNameMem[MAX_FILESPEC];  // Short name of TM w o ext ( gallus )
 OtmMemory     *pOutputMem;                   // TM handle
 BOOL          fPropExist;                    // Existence of properties 0=No 1=Yes
 BOOL          fPropCreated;                  // Tm properties have been created
 BOOL          fMsg;                          // A message has been issued already
 BOOL          fImport;                       // If set the merge is started via an import
 USHORT        usImportRc;                    // Import function return code
 ULONG         ulSegmentCounter;              // Number of segments merged
 ULONG         ulInvSegmentCounter;           // Invalid Segment counter
 EXT_IN        stOldExtIn;                    // input for TmOldExtract
 EXT_OUT       stOldExtOut;                   // output for TmOldExtract
 TMX_EXT_IN_W  stExtIn;                       // TMX_EXTRACT_IN structure
 TMX_EXT_OUT_W stExtOut;                      // TMX_EXTRACT_OUT structure
 TMX_PUT_IN_W  stPutIn;                       // TMX_PUT_IN structure
 TMX_PUT_OUT_W stPutOut;                      // TMX_PUT_OUT structure
 TIME_L        tStamp;                        // Time stamp of merge start time
 HWND          hwndMemLb;                     // Handle to the TM listbox
 HWND          hProgressWindow;               // Handle of progress indicator window
 ULONG         ulProgressPos;                 // position of progress indicator
 CHAR          szServer[MAX_LONGPATH];        // Server Name of TM or \0 if TM is local
 BOOL          fPropReceived;                 // Property file got via Receive ?
 BOOL          fOrganizeInvoked;              // TRUE if Organize is invoked
 BOOL          fImportAnyway;                 // FALSE if import of TM with
                                              // different IDs is canceled
 CHAR          szSourceMemSourceLang[MAX_LANG_LENGTH];
 CHAR          szSourceMemTargetLang[MAX_LANG_LENGTH];
 CHAR          szSourceMemMarkup[MAX_FNAME];                         /*@1276A*/
 CHAR          szTargetMemSourceLang[MAX_LANG_LENGTH];
 CHAR          szTargetMemTargetLang[MAX_LANG_LENGTH];
 CHAR          szTargetMemMarkup[MAX_FNAME];                         /*@1276A*/
 BOOL          fDisplayMsg;             // message flag if segment not /*@47A*/
                                        // not replaced message should /*@47A*/
                                        // be displayed or not         /*@47A*/
 BOOL          fOldPropFile;                  // used for folder import
 CHAR          szTagTable[MAX_FILESPEC];      // tag table of org TM
 CHAR          szPathMergeIndex[MAX_LONGPATH];// Full path and name of index to be merged
 CHAR          szPathIndex[MAX_LONGPATH]; // name and path to index C:\EQF\MEM\xxxx.TMI )
 BOOL         fBatch;                     // TRUE = we are in batch mode
 HWND         hwndErrMsg;                 // parent handle for error messages
 PDDEMEMEXP   pDDEMemExp;                 // ptr to batch memory export data
 CHAR         szLongName[MAX_LONGFILESPEC]; // buffer for long TM names
 USHORT       usTask;                      // current taskl to do (batch mode)
 OtmProposal  *pProposal;                   // buffer for proposal data
 BOOL         fFirstExtract;                // TRUE = this ist the first extract call
 int          iComplete;                    // process completion rate
}MEM_MERGE_IDA, * PMEM_MERGE_IDA;

/**********************************************************************/
/* Dialog IDA for TM property dialog                                  */
/**********************************************************************/
typedef struct _MEM_PROP_IDA
{
 CHAR         szMemName[MAX_LONGFILESPEC];// Memory database name without extension
 OtmMemory    *pMem;                       // Handle of memory database
 HPROP        hPropMem;                   // Memory database property handle
 PPROP_NTM    pPropMem;                   // pointer to TM properties
 CHAR         szPropName[MAX_FILESPEC];   // buffer for property name
 CHAR         szPropPath[MAX_EQF_PATH];   // buffer for property path
 CHAR         szTempPath[MAX_EQF_PATH];   // buffer for path names
}MEM_PROP_IDA, * PMEM_PROP_IDA;


__declspec(dllexport) BOOL EqfMemPropsToHtml( HWND hwndParent, PMEM_IDA  pIDA, OtmMemory *pMem );

VOID    EQFMemImportTrojaEnd( PMEM_MERGE_IDA );
USHORT  CloseMergeTmAndTm( PMEM_MERGE_IDA, BOOL );                   /*@1139A*/
USHORT MemFuncMergeTM
(
  PMEM_MERGE_IDA    pMIDA           // Pointer to the merge IDA
);

#endif

typedef struct _MEMORY_HANDLER_DATA
{
 IDA_HEAD  stIdaHead;                    // Standard Ida head
}MEMORY_HANDLER_DATA, * PMEMORY_HANDLER_DATA;


/**********************************************************************/
/* TMX_ENDORG_IN, TMX_ENDORG_OUT used by NTMCloseOrganize             */
/* Structures needed for the TMC_END_ORGANIZE command which is used   */
/* to end an 'Organize' process.                                      */
/* This command is sent to the original TM instead                    */
/* of the TM_CLOSE command and handles the closing, deletion and      */
/* renaming of the original and temporary TM file and index.          */
/* The temporary file has to be closed previously.                    */
/**********************************************************************/
typedef struct _TMX_ENDORG_IN
{
  TMX_PREFIX_IN  stPrefixIn;
  CHAR           szOrgTM[MAX_EQF_PATH];           //full name of original TM
  CHAR           szTmpTM[MAX_EQF_PATH];           //full name of temporary TM
  CHAR           szOrgIndex[MAX_EQF_PATH];        //full name of original index file
  CHAR           szTmpIndex[MAX_EQF_PATH];        //full name of temporary index file
  CHAR           szOrgProp[MAX_EQF_PATH];         //full name of original property file
  CHAR           szTmpProp[MAX_EQF_PATH];         //full name of temporary property file
  USHORT         usOrgType;                       //type ORGANIZE, CONVERT
} TMX_ENDORG_IN, * PTMX_ENDORG_IN;

typedef struct _TMX_ENDORG_OUT
{
  TMX_PREFIX_OUT  stPrefixOut;
} TMX_ENDORG_OUT, * PTMX_ENDORG_OUT;

/**********************************************************************/
/* The following structures for dialog IDAs are defined only          */
/* if INCL_EQFMEM_DLGIDAS has been defined                            */
/**********************************************************************/
#if defined(INCL_EQFMEM_DLGIDAS)

typedef struct _MEM_INCL_DLG_IDA
{
 BOOL         fInitErrorOccurred;         // Important init error occurred
}MEM_INCL_DLG_IDA, * PMEM_INCL_DLG_IDA;

#endif

#ifdef _OTMMEMORY_H_

typedef struct _MEM_ORGANIZE_IDA
{
 TMX_ENDORG_IN stEndOrgIn;                     // organize end input structure
 TMX_ENDORG_OUT stEndOrgOut;                   // organize end output structure
 TMX_PUT_IN_W  stPutIn;                        // input for TmReplace
 TMX_PUT_OUT_W stPutOut;                       // The REPLACE_OUT structure
 TMX_EXT_IN_W  stExtIn;                        // input for TmExtract
 TMX_EXT_OUT_W stExtOut;                       // output for TmExtract
 CHAR          szPathOrganizeMem[2048];        // Full path and name of TM to be organized
 CHAR          szDrive[MAX_DRIVE];             // Drive letter without colum
 CHAR          szMemName[MAX_LONGFILESPEC];    // Translation memory name 
 CHAR          szTempMemName[MAX_LONGFILESPEC];// name of temporary translation memory 
 CHAR          szPluginName[MAX_LONGFILESPEC]; // name of plugin used for the memory
 CHAR          szPathTempMem[2048];            // Full path of temporary transl. mem.
 OtmProposal   *pProposal;                     // buffer for memory proposal
 OtmMemory     *pMem;                           // Handle of transl. memory
 OtmMemory     *pMemTemp;                       // Handle of tmporary transl. memory
 BOOL          fMsg;                           // A message has been issued already
 ULONG         ulSegmentCounter;               // Number of segments organized
 ULONG         ulInvSegmentCounter;            // Invalid Segment counter
 TIME_L        tStamp;                         // Time stamp of organize start time
 HWND          hProgressWindow;                // Handle of progress indicator window
 ULONG         ulProgressPos;                  // position of progress indicator
 CHAR          szPathTempIndex[MAX_EQF_PATH];  //full temporary index name

 LANGUAGE      szSourceLanguage;               // source language of org TM
 LANGUAGE      szTargetLanguage;               // target language of org TM
 CHAR          szTagTable[MAX_FILESPEC];       // tag table of org TM

 CHAR          szPropertyName[MAX_FILESPEC];   // property name with ext.
 CHAR          szTempPropertyName[MAX_FILESPEC]; // property name temp TM with ext

 CHAR          szOrgProp[MAX_EQF_PATH];        // full name of original property file
 CHAR          szTmpProp[MAX_EQF_PATH];        // full name of temporary property file

 CHAR          szEqfPath[MAX_EQF_PATH];        // system path D:\EQF
 USHORT        usOrgType;                      // type ORGANIZE, CONVERT
 BOOL          fBatch;                 // TRUE if organizing in batch mode
 HWND          hwndErrMsg;             // handle of window to be used for error msgs
 PDDEMEMORG    pDDEMemOrg;             // ptr to batch memory organize data
 USHORT        usRC;                   // return code / error code
 LONG          NextTask;               // next task in non-DDE batch mode
 PSZ           pszNameList;            // pointer to list of TMs being organized
 PSZ           pszActiveName;          // points to current name in pszNameList
 CHAR          szBuffer[2048];         // general purpose buffer
 BOOL          fFirstGet;              // TRUE = this is the first get access
}MEM_ORGANIZE_IDA, * PMEM_ORGANIZE_IDA;

USHORT  TmCloseOrganize( PMEM_ORGANIZE_IDA, USHORT );

USHORT NTMCloseOrganize ( PMEM_ORGANIZE_IDA, USHORT            );

USHORT NTMConvertProperties( PPROPTRANSLMEM, PMEM_ORGANIZE_IDA );

#endif

// Structure of EQF memory properties
struct memlst_prop
{
   PROPHEAD  PropHead;                   // Common header of properties
   EQF_SWP   Swp;                        // Window position
   BYTE   szDriveList[27];               // List of drive letters
   USHORT usViewFormat;                  // VIEW format
   SHORT  sLastUsedViewList[MAX_VIEW];   // last used view list
   SHORT  sDetailsViewList[MAX_VIEW];    // user's details view list'
   SHORT  sSortList[MAX_VIEW];           // user's sort criteria list'
   CLBFILTER Filter;                     // currently active column listbox filter
   SHORT  sLastUsedViewWidth[MAX_VIEW];   // last used view list
};
typedef struct memlst_prop PROPMEMORY, *PPROPMEMORY;

typedef struct _SLIDER_DATA
{
   CHAR szLine [5] [TEXT_100_CHAR+MAX_PATH144]; // 5 text lines of 100 characters
                                             // + max path length
} SLIDER_DATA, * PSLIDER_DATA;



/*--------------------------------------------------------------------------*\
     Function prototypes.
\*--------------------------------------------------------------------------*/
USHORT
  OpenTmFile (PTMT_GLOBALS ptmtg,      /*.The TM Globals Area.............*/
              PSZ          pszFileName,/*.The TM Full File Name...........*/
              USHORT       idCommand), /* TMC_CREATE / TMC_OPEN...........*/

  ReadTmHeader (PTMT_GLOBALS ptmtg),  /* Tmt Globals area ................*/

  AllocTmtGlobals (USHORT usTmtGlobalsSize, /* The allocateion size.......*/
                   PPTMT_GLOBALS pptmtg),   /* The TM Globals Area........*/

  SetTmtWorkPointers (PTMT_GLOBALS ptmtg);  /* The TM Globals Area........*/

/*--------------------------------------------------------------------------*\
     Function prototypes.
\*--------------------------------------------------------------------------*/
VOID
GetFirstSegAddress
(
  PTMT_GLOBALS ptmtg,             // Pointer to Tmt globals
  PSZ          pszSource,         // pointer to source segment
  PTM_ADDRESS  pAddr              // pointer to address
);

VOID
CleanSource
(
   PTMT_GLOBALS ptmtg,            // Pointer to Tmt globals
   PSZ          pszSource         // Pointer to string to be tokenized
);

VOID
CalcPrimaryKey
(
  PTMT_GLOBALS ptmtg,             // Pointer to Globals structure
  PUCHAR       pchPrimaryKey      // Pointer to primary key
);

VOID
CalcSecondaryKey
(
  PTMT_GLOBALS ptmtg,             // Pointer to Tmt globals
  PSZ          pszSortedSecKey    // Ptr to sorted secondary key
);

VOID
Get4Chars
(
  PTMT_GLOBALS ptmtg,             // Pointer to Tmt globals
  SHORT        sTokenId,          // WORD/USELESS/NOISE/TEXT/TAG
  PTOKENENTRY  pteFirst,          // Ptr to Text or Word Table
  PUCHAR       pchPrimaryKey      // 4 chars for primary key
);

VOID
WordsTokenize
(
  PTMT_GLOBALS  ptmtg             // Pointer to Tmt globals
);

VOID
UselessFiltering
(
  PTOKENENTRY  pteWord            // Pointer to word in WordsTable
);

USHORT
Tmt
(
  HTM       htm,                  // Pointer to TmtGlobals
  PIN       pIn,                  // Pointer to input buffer
  POUT      pOut                  // Pointer to output buffer
);


/**********************************************************************/
/* TmtX                                                               */
/**********************************************************************/
USHORT
TmtX ( HTM       htm,                  // Pointer to TmtGlobals
       PXIN       pIn,                 // Pointer to input buffer
       PXOUT      pOut );              // Pointer to output buffer


USHORT
CalcEntryInKeyDir
(
  PTM_HEADER  ptmh,               // Pointer to Tm Header
  PUCHAR      pchPrimaryKey       // Primary Key
);

USHORT
WriteTmhToDisk
(
  PTMT_GLOBALS ptmtg,             // Pointer to globals
  USHORT       fsWrite            // Write control flags word
);

USHORT
WriteToDisk
(
  PTMT_GLOBALS ptmtg,             // TMT globals
  ULONG        ldispPtr,          // New file pointer location
  PVOID        pvWrite,           // Pointer to write buffer
  USHORT       numWrite           // Number of bytes to write
);

USHORT
FormatMore
(
  PTMT_GLOBALS ptmtg              // Pointer to Tmt globals
);

USHORT
ReadStringFromDisk
(
  PTMT_GLOBALS ptmtg,             // Pointer to Tmt globals
  PCHAR        bufRead,           // Read buffer
  USHORT       lenString,         // String length
  PTM_ADDRESS  paddr              // Pointer to string buffer
);

USHORT
ReadSegmentFromDisk
(
  PTMT_GLOBALS ptmtg,             // pointer to Tmt globals
  PTM_ADDRESS  pAddr,             // Pointer to an address
  PSEGMENT     pSegment,          // Pointer to seg. buffer
  PBOOL        pfFirstSeg,        // Pointer to 1st seg. flag
  PBOOL        pfLastSeg          // Pointer to last seg. flag
);

USHORT
ReadBlock
(
  PTMT_GLOBALS ptmtg,                // Pointer to Tmt globals
  PTM_ADDRESS  paddr,                // Pointer to an address
  BOOL         fAddressInParameter   // Address in pAddr
);

INT
CharCompare
(
  const void * arg1,                       // First comparand
  const void * arg2                        // Second comparand
);

BOOL
fStrcmpCRLF
(
  PSZ     pszStr1,           // pointer to first string to be compared
  PSZ     pszStr2,           // pointer to second string to be compared
  PUSHORT pusEqualChars,     // (return value) pointer to USHORT where the
                             // number of equal characters (first string)
  USHORT  fCompareType);     // flag indicating whether CRLF and LF are ignored
                             // ALIGN_CRLF -> CRLF and LF are equal   equal
                             // USE_CRLF   -> exact matches required s
                             // IGNORE_CRL -> characters are equal


/*--------------------------------------------------------------------------*\
     Function prototypes.
\*--------------------------------------------------------------------------*/
USHORT
  TmtExtract (PTMT_GLOBALS    ptmtg,           /* pointer to Tmt globals....*/
              PEXT_IN         pExtIn,          /* pointer to input buffer...*/
              PEXT_OUT        pExtOut);        /* pointer to output buffer..*/

USHORT
  TmtGet (PTMT_GLOBALS     ptmtg,              /* pointer to Tmt globals....*/
          PGET_IN          pGetIn,             /* pointer to input buffer...*/
          PGET_OUT         pGetOut),           /* pointer to output buffer..*/

  TmtGetTMPart( PTMT_GLOBALS  ptmtg,           /* pointer to Tmt globals....*/
                PGETPART_IN   pGetPartIn,      /* pointer to input buffer...*/
                PGETPART_OUT  pGetPartOut),    /* pointer to output buffer..*/

  GetSegByAddr (PTMT_GLOBALS     ptmtg,        /* pointer to TmtGlobals.....*/
                PTM_ADDRESS      pAddr,        /* pointer to input address..*/
                PTM_ADDRESS      pAddrNext,    /* pointer to next address...*/
                PSEGMENT         pseg),        /* pointer to segment........*/


  TmtInfo     (PTMT_GLOBALS     ptmtg,         /* pointer to Tmt globals....*/
               PINFO_OUT        pInfoOut);     /* pointer to output buffer..*/
/**********************************************************************/
/* TmtXInfo                                                          */
/**********************************************************************/
USHORT TmtXInfo( PTMX_CLB,
                 PTMX_INFO_OUT );


LONG
  CheckSimilar (PTMT_GLOBALS ptmtg,          /* pointer to Tmt globals.....*/
                PSEGMENT     pCurrentSegment,/* pointer to current segmnt .*/
                PTOKENENTRY  pTokenEntry,    /* points to 1'st & 2'nd word */
                PGET_IN      pGetIn,         /* pointer to input buffer....*/
                PSZ          pszInputKey,    /* pointer to sortedkey of....*/
                                             /* input segment..............*/
                PMATCH       pmtch),         /* pointer to stMatch.........*/

  CheckExact (PGET_IN      pGetIn,           /* pointer to input segment....*/
              PSEGMENT     pseg,             /* pointer to current seg......*/
              BOOL         fSimilarMod,      /* Similar= TRUE, Exact=FALSE..*/
              PMATCH       pmtch),           /* pointer to stMatch..........*/

  CalcExactIndustry (PGET_IN   pGetIn,       /* pointer to input segment....*/
                     PSEGMENT  pseg);        /* pointer to current seg......*/

VOID
  RankNewMatch (USHORT     numMatchesReq,    // number of matches required
                USHORT     usConvert,        // Conversion indicator
                PMATCH     pmtchCurrent,     // pointer to stMatch
                PSEGMENT   pseg,             // pointer to current seg
                PGET_OUT   pGetOut);         // pointer to GET_OUT struct

BOOL
  CheckWord   (PTOKENENTRY  pteFirst,       /*Point to first word ..........*/
               PTOKENENTRY  pteSecond) ;    /*Point to second word .........*/

VOID
LengthCorrectThresholds( PTMT_GLOBALS ptmtg,      //pointer to Tmt globals
                         USHORT       usLenInput, //number of words in inp. seg.
                         PGET_IN      pGetIn );            //pointer to input buffer

USHORT
  TmtAdd (PTMT_GLOBALS pTmtGlobals,        /* Pointer to Tmt Globals........*/
          PADD_IN      pAddIn,             /* Pointer to input buffer.......*/
          PADD_OUT     pAddOut),           /* Pointer to output buffer......*/

  TmtDelete (PTMT_GLOBALS ptmtg,               /* Pointer to Tmt Globals....*/
             PDEL_IN      pDeleteIn,           /* Pointer to input buffer...*/
             PDEL_OUT     pDeleteOut),         /* Pointer to output buffer..*/

  TmtReplace (PTMT_GLOBALS ptmtg,              /* Pointer to Tmt Globals....*/
              PREP_IN      pReplaceIn,         /* Pointer to input buffer...*/
              PREP_OUT     pReplaceOut),       /* Pointer to output buffer..*/

  AddSegToCluster (PTMT_GLOBALS  ptmtg,         /* Pointer to globals area..*/
                   PTM_ADDRESS   pAddr,         /* Ptr to initial address...*/
                   PSEGMENT      pseg),         /* Pointer to a segment.....*/

  FindFreshAddrInCluster (PTMT_GLOBALS ptmtg,     /* Pointer to Tmt globals.*/
                          PTM_ADDRESS  pAddr,     /* Pointer to address.....*/
                          BOOL fReadFirstBlock),  /* Read block flag........*/

  TakeBlockFromSpool (PTMT_GLOBALS ptmtg,     /* Pointer to Tmt globals.....*/
                      USHORT rcPrevious,      /* RC of the previous call....*/
                      PTM_ADDRESS  pAddr),    /* Pointer to an address......*/

  WriteStringToDisk (PTMT_GLOBALS ptmtg,      /* Pointer to Tmt globals.....*/
                     PCHAR   bufWrite,        /* Pointer to the string......*/
                     USHORT  lenString,       /* String length..............*/
                     PTM_ADDRESS  pAddr),     /* Pointer to string address..*/

  WriteBlock (PTMT_GLOBALS ptmtg),            /* Pointer to Tmt globals.....*/

  FindMatchSegInCluster (PTMT_GLOBALS ptmtg,        /* Ptr to Tmt globals...*/
                         PTM_ADDRESS  pAddr,        /* Pointer to address...*/
                         PBOOL        pfLastSeg,    /* Last segment flag....*/
                         PSEGMENT     psegIn,       /* Ptr to input buffer..*/
                         PSEGMENT     pseg),        /* Pointer to segment...*/

  DeleteSegment (PTMT_GLOBALS ptmtg,       /* Pointer to Tmt globals........*/
                PTM_ADDRESS   pAddr,       /* Address of segment to delete..*/
                BOOL          fLastSeg,    /* Last segment in cluster flag..*/
                PSEGMENT      pseg);       /* Pointer to the segment........*/

BOOL
  CheckMatchForDelete (PSEGMENT psegIn,    /* pointer to input buffer.......*/
                       PSEGMENT pseg);     /* currently, output buffer......*/

USHORT TmtDeleteTM( HTM        hMem,                 // TM handle
                    PDELTM_IN  pDelTmIn,             // Input structure
                    PDELTM_OUT pDelTmOut );          // Output structure
/**********************************************************************/
/* TmXtDeleteTM                                                       */
/**********************************************************************/
USHORT
TmtXDeleteTM( HTM,
              PTMX_DELTM_IN,
              PTMX_DELTM_OUT );

USHORT TmtCloseOrganize( HTM         hMem,           // TM handle
                         PENDORG_IN  pEndOrgIn,      // Input structure
                         PENDORG_OUT pEndOrgOut );   // Output structure
/**********************************************************************/
/* TmtXCloseOrganize                                                  */
/**********************************************************************/
USHORT
TmtXCloseOrganize( HTM,
                   PTMX_ENDORG_IN,
                   PTMX_ENDORG_OUT );


VOID   TmtDeleteFile( PDELFILE_IN  pDelFileIn,          // Input structure
                     PDELFILE_OUT pDelFileOut );       // Output structure

VOID   GetFileInfo( PFILEINFO_IN pFileInfoIn,        // Input structure
                    PFILEINFO_OUT pFileInfoOut );     // Output structure

VOID   RenameFile( PRENFILE_IN  pRenFileIn,          // Input structure
                   PRENFILE_OUT pRenFileOut );       // Output structure

BOOL fCheckFileClosed ( PSZ );         //--- pointer to file name


/*--------------------------------------------------------------------------*\
     Function prototypes.
\*--------------------------------------------------------------------------*/
USHORT
  TmtOpen (POPEN_IN   pOpenIn,           /* Pointer to input structure....*/
           POPEN_OUT  pOpenOut),         /* Pointer to output structure...*/

  TmtCreate (PCREATE_IN   pCreateIn,     /* Pointer to input structure....*/
             PCREATE_OUT  pCreateOut),   /* Pointer to output structure...*/

  TmtClose (PTMT_GLOBALS ptmtg,       /* Pointer to Globals structure.....*/
            PCLOSE_OUT   pCloseOut);  /* Pointer to output buffer.........*/

__declspec(dllexport) INT_PTR CALLBACK MEMCREATEDLG( HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) INT_PTR CALLBACK MEMLOADDLG( HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) INT_PTR CALLBACK MEMEXPORTDLG( HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) INT_PTR CALLBACK MEMMERGEDLG( HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) INT_PTR CALLBACK MEMPROPDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK MemCreateName( HWND, USHORT, WPARAM, LPARAM );
INT_PTR CALLBACK UTLSERVERLISTDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK UTLSERVERLISTNAMEDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK MEMINCLUDEDLG( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK SERVWAITDLG( HWND, WINMSG, WPARAM, LPARAM );
USHORT  MemCreateProcess( PMEM_IDA, PSZ, USHORT );
VOID    MemDestroyProcess( PMEM_IDA, USHORT * );
USHORT  MemGetAddressOfProcessIDA( PMEM_IDA, WPARAM, USHORT *, PVOID * );
VOID    MemRcHandlingErrorUndefined( USHORT, PSZ );
VOID    MemRcHandlingErrorUndefinedHwnd( USHORT, PSZ, HWND );
USHORT  EQFMemOrganizeStart( PPROCESSCOMMAREA, HWND );
VOID    EQFMemOrganizeProcess( PPROCESSCOMMAREA, HWND );
VOID    EQFMemOrganizeEnd( PPROCESSCOMMAREA, HWND, LPARAM, BOOL );
VOID    EQFMemLoadStart( PPROCESSCOMMAREA, HWND );
VOID    EQFMemLoadProcess( PPROCESSCOMMAREA, HWND );
VOID    EQFMemLoadEnd( PPROCESSCOMMAREA, HWND, LPARAM );
USHORT  EQFMemExportStart( PPROCESSCOMMAREA, HWND );
USHORT  EQFMemExportProcess( PPROCESSCOMMAREA, HWND );
USHORT  EQFMemExportEnd( PPROCESSCOMMAREA, HWND, LPARAM );
VOID    EQFMemMergeStart( PPROCESSCOMMAREA, HWND );
VOID    EQFMemMergeProcess( PPROCESSCOMMAREA, HWND );
VOID    EQFMemMergeEnd( PPROCESSCOMMAREA, HWND, LPARAM );
__declspec(dllexport) USHORT  EQFMemImportTrojaStart( HWND, LPARAM, HWND );
USHORT  MemInitSlider( HWND, USHORT, PSZ, USHORT, PSZ, PHWND, USHORT, USHORT);
USHORT  ReadABGrouping( PSZ, PSZ, ABGROUP );
__declspec(dllexport) MRESULT MemOrganizeCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) MRESULT MemExportCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) MRESULT MemMergeCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );
__declspec(dllexport) MRESULT MemImportCallBack( PPROCESSCOMMAREA, HWND, WINMSG, WPARAM, LPARAM );



// ************* Memory macro definitions *******************************
#define EnableOKAndCANCELButton( hDlg, flag )   \
  ENABLECTRL( hDlg, PID_PB_OK, flag );          \
  ENABLECTRL( hDlg, PID_PB_CANCEL, flag );

#define MEMENABLESERVLISTPBS( hDlg, fEnable )  \
  { \
     ENABLECTRL( hDlg, ID_SERVLIST_OK_PB, fEnable );     \
     ENABLECTRL( hDlg, ID_SERVLIST_ADD_PB, fEnable );    \
     ENABLECTRL( hDlg, ID_SERVLIST_DELETE_PB, fEnable ); \
     ENABLECTRL( hDlg, ID_SERVLIST_CANCEL_PB, fEnable ); \
     ENABLECTRL( hDlg, ID_SERVLIST_HELP_PB, fEnable );   \
  }

#define MEMENABLEINCLUDEPBS( hDlg, fEnable ) \
  { \
     ENABLECTRL( hDlg, ID_INCLRMTM_INCLUDE_PB, fEnable ); \
     ENABLECTRL( hDlg, ID_INCLRMTM_CANCEL_PB, fEnable );  \
     ENABLECTRL( hDlg, ID_INCLRMTM_HELP_PB, fEnable );    \
  }

#define MEMENABLESERVLISTNAMEPBS( hDlg, fEnable ) \
  { \
     ENABLECTRL( hDlg, ID_SERVLIST_NAME_OK_PB, fEnable );                                           \
     ENABLECTRL( hDlg, ID_SERVLIST_NAME_CANCEL_PB, fEnable );                                           \
     ENABLECTRL( hDlg, ID_SERVLIST_NAME_HELP_PB, fEnable );                                           \
  }

//=======================================================================
//prototypes and definitions for TM utilities
#define SIZE_32K 32768
#define ERROR_TABLE_FULL 1111
#define ID_NOT_FOUND     2222


// adjust pointger for a new location
#define ADJUSTPTR( new, old, offsptr ) \
  ((PBYTE)new + ((PBYTE)offsptr - (PBYTE)old))

// do not update name tables when name is not contained in the table
#define NTMGETID_NOUPDATE_OPT 0x00000001

// returned ID if a name is not found in the table
#define NTMGETID_NOTFOUND_ID 0xFFFF

USHORT NTMGetIDFromNameEx
(
  PTMX_CLB    pTmClb,                  // input, memory control block pointer
  PSZ         pszName,                 // input, name being looked up
  PSZ         pszLongName,             // input, long name (only for FILE_KEY)
  USHORT      usTableType,             // input, type of table to use
  PUSHORT     pusID,                   // output, ID for name being looked up
  LONG        lOptions,                // input, additional options
  PUSHORT     pusAlternativeID         // output, alternative ID
);

PSZ NTMFindNameForID( PTMX_CLB pTmClb,      //input
                  PUSHORT  pusID,         //intput
                  USHORT   usTableType );


USHORT NTMGetIDFromName( PTMX_CLB, PSZ, PSZ, USHORT, PUSHORT );
USHORT NTMGetNameFromID( PTMX_CLB, PUSHORT, USHORT, PSZ, PSZ );
USHORT TmtXReplace( PTMX_CLB, PTMX_PUT_IN_W, PTMX_PUT_OUT_W );
USHORT TmtXGet( PTMX_CLB, PTMX_GET_IN_W, PTMX_GET_OUT_W );
USHORT TmtXCreate( PTMX_CREATE_IN, PTMX_CREATE_OUT );
USHORT TmtXOpen( PTMX_OPEN_IN, PTMX_OPEN_OUT );
USHORT TmtXClose( PTMX_CLB, PTMX_CLOSE_IN, PTMX_CLOSE_OUT );

//tm put prototypes
BOOL AllocTable( PTMX_TABLE* );
USHORT TokenizeSourceV5( PTMX_CLB, PTMX_SENTENCE_V5, PSZ, PSZ, USHORT );
USHORT TokenizeSourceEx2( PTMX_CLB, PTMX_SENTENCE, PSZ, PSZ, USHORT, ULONG, int );
VOID HashSentence( PTMX_SENTENCE, USHORT usMajVersion, USHORT usMinVersion );
VOID HashSentenceV5( PTMX_SENTENCE_V5, USHORT );
USHORT HashTupel( PBYTE, USHORT, USHORT );
USHORT HashTupelW( PSZ_W, USHORT, USHORT, USHORT );
static VOID BuildVotes( PTMX_SENTENCE );
static VOID Vote( PTMX_TERM_TOKEN, PTMX_SENTENCE, USHORT );
static VOID BuildVotesV5( PTMX_SENTENCE_V5 );
static VOID VoteV5( PTMX_TERM_TOKEN, PTMX_SENTENCE_V5, USHORT );
USHORT CheckCompactArea( PTMX_SENTENCE, PTMX_CLB );
USHORT CheckCompactAreaV5( PTMX_SENTENCE_V5, PTMX_CLB );

USHORT TokenizeTarget( PSZ_W, PSZ_W, PTMX_TAGTABLE_RECORD*, PLONG, PSZ, PUSHORT, PTMX_CLB );
USHORT TokenizeTargetV5( PSZ, PSZ, PTMX_TAGTABLE_RECORD*, PLONG, PSZ, PUSHORT, PTMX_CLB );

USHORT AddToTm( PTMX_SENTENCE, PTMX_CLB, PTMX_PUT_W, PULONG );
USHORT AddToTmV5( PTMX_SENTENCE_V5, PTMX_CLB, PTMX_PUT, PULONG );
VOID FillTmRecord( PTMX_SENTENCE, PTMX_TAGTABLE_RECORD, PSZ_W, USHORT,
                   PTMX_RECORD, PTMX_TARGET_CLB );
VOID FillTmRecordV5( PTMX_SENTENCE_V5, PTMX_TAGTABLE_RECORD, PSZ, USHORT,
                   PTMX_RECORD, PTMX_OLD_TARGET_CLB );

USHORT FillClb( PTMX_TARGET_CLB *, PTMX_CLB, PTMX_PUT_W );
USHORT FillClbV5( PTMX_OLD_TARGET_CLB *, PTMX_CLB, PTMX_PUT );
USHORT UpdateTmIndex( PTMX_SENTENCE, ULONG, PTMX_CLB );
USHORT UpdateTmIndexV5( PTMX_SENTENCE_V5, ULONG, PTMX_CLB );
USHORT DetermineTmRecord( PTMX_CLB, PTMX_SENTENCE, PULONG );
USHORT DetermineTmRecordV5( PTMX_CLB, PTMX_SENTENCE_V5, PULONG );
USHORT UpdateTmRecord( PTMX_CLB, PTMX_PUT_W, PTMX_SENTENCE );
USHORT UpdateTmRecordV5( PTMX_CLB, PTMX_PUT, PTMX_SENTENCE_V5 );
USHORT AddTmTarget( PTMX_CLB, PTMX_PUT_W, PTMX_SENTENCE, PTMX_RECORD *, PULONG, PULONG );
USHORT AddTmTargetV5( PTMX_CLB, PTMX_PUT, PTMX_SENTENCE_V5, PTMX_RECORD *, PULONG, PULONG );
USHORT ComparePutData( PTMX_CLB, PTMX_RECORD *, PULONG, PTMX_PUT_W, PTMX_SENTENCE, PULONG );
USHORT ComparePutDataV5( PTMX_CLB, PTMX_RECORD *, PULONG, PTMX_PUT, PTMX_SENTENCE_V5, PULONG );

VOID FillTargetRecord( PTMX_SENTENCE, PTMX_TAGTABLE_RECORD,
                       PSZ_W, USHORT, PTMX_TARGET_RECORD *, PTMX_TARGET_CLB );
VOID FillTargetRecordV5( PTMX_SENTENCE_V5, PTMX_TAGTABLE_RECORD,
                       PSZ, USHORT, PTMX_TARGET_RECORD *, PTMX_OLD_TARGET_CLB );

USHORT ReplaceTmTarget( PTMX_SENTENCE, PTMX_CLB, PTMX_PUT, PBYTE,
                        PTMX_RECORD, PULONG );
VOID DeleteOldestRecord( PTMX_RECORD, PULONG );

//tm get prototypes
USHORT GetExactMatch( PTMX_CLB, PTMX_SENTENCE, PTMX_GET_W, PTMX_MATCH_TABLE_W,
                      PUSHORT, PTMX_GET_OUT_W );
USHORT ExactTest( PTMX_CLB, PTMX_RECORD, PTMX_GET_W, PTMX_SENTENCE,
                  PTMX_MATCH_TABLE_W, PUSHORT, ULONG );

BOOL AddTagsToString( PSZ, PULONG, PTMX_TAGTABLE_RECORD, PSZ );
BOOL AddTagsToStringW( PSZ_W, PULONG, PTMX_TAGTABLE_RECORD, PSZ_W );

INT CompCount( const void *, const void * );
INT CompCountVotes( const void *, const void * );
VOID CleanupTempMatch( PTMX_MATCHENTRY, PTMX_MATCHENTRY *, PUSHORT, PUSHORT );
USHORT FillMatchEntry( PTMX_CLB, PTMX_SENTENCE, PTMX_MATCHENTRY, PUSHORT );
USHORT FuzzyTest( PTMX_CLB, PTMX_RECORD, PTMX_GET_W, PTMX_MATCH_TABLE_W, PUSHORT,
                  PUSHORT, PUSHORT, PUSHORT, PTMX_SENTENCE, ULONG );
USHORT GetFuzzyMatch( PTMX_CLB, PTMX_SENTENCE, PTMX_GET_W, PTMX_MATCH_TABLE_W, PUSHORT );

//tm extract prototypes
USHORT TmtXExtract( PTMX_CLB, PTMX_EXT_IN_W, PTMX_EXT_OUT_W );
USHORT FillExtStructure( PTMX_CLB, PTMX_TARGET_RECORD,
                         PTMX_TARGET_CLB,
                         PSZ_W, PULONG, PTMX_EXT_W );

// TM segment update prototypes
USHORT TmtXUpdSeg
(
  PTMX_CLB    pTmClb,      // ptr to ctl block struct
  PTMX_PUT_IN pTmPutIn,    // ptr to put input data
  ULONG       ulUpdKey,    // SID of record being updated
  USHORT      usUpdTarget, // number of target being updated
  USHORT      usFlags      // flags controlling the updated fields
);

//tm delete segment prototypes
USHORT TmtXDelSegm( PTMX_CLB, PTMX_PUT_IN_W, PTMX_PUT_OUT_W );
USHORT FindTargetAndDelete( PTMX_CLB, PTMX_RECORD, PTMX_PUT_W, PTMX_SENTENCE, PULONG );
USHORT NTMCheckForUpdates( PTMX_CLB );
USHORT NTMLockTM( PTMX_CLB, BOOL, PBOOL );
USHORT MemBatchTMCreate( HWND hwnd, PDDEMEMCRT pMemCrt );
USHORT MemBatchTMExport( HWND hwnd, PDDEMEMEXP pMemExp );
USHORT MemBatchTMImport( HWND hwnd, PDDEMEMIMP pMemImp );
USHORT MemBatchTMDelete( HWND hwnd, PDDEMEMDEL pMemDel );

// definitions for terse of TM name tables
// max size of in-memory name table
#define SIZE_64K 0xFF00L

// magic word for recognition of tersed name tables
// Note: this ULONG value has to ensure that it will never been used
//       as a name so it is prefixed and suffixed by a 0x00
#define TERSEMAGICWORD 0x0019FF00L

// structure of header for tersed name tables
typedef struct _TERSEHEADER
{
  USHORT      usAllocSize;             // alloc size in untersed tables
  USHORT      usMaxEntries;            // max entries in untersed tables
  ULONG       ulMagicWord;             // magic word for tersed name
  USHORT      usDataSize;              // size of data area when expanded
  USHORT      usCompression;           // type of compression used for data
                                       // (values as defined in EQFCMPR.H)
} TERSEHEADER, *PTERSEHEADER;

USHORT NTMSaveNameTable
(
  PTMX_CLB    pTmClb,                  // ptr to TM control block
  ULONG       ulTableKey,              // key of table record
  PBYTE       pTMTable,                // ptr to table data
  ULONG       ulSize                   // size of table data
);

USHORT NTMLoadNameTable
(
  PTMX_CLB    pTmClb,                  // ptr to TM control block
  ULONG       ulTableKey,              // key of table record
  PBYTE       *ppTMTable,              // ptr to table data pointer
  PULONG      pulSize                  // ptr to buffer for size of table data
);


BOOL TMDelTargetClb
(
  PTMX_RECORD        pTmRecord,        // ptr to TM record
  PTMX_TARGET_RECORD pTargetRecord,    // ptr to target record within TM record
  PTMX_TARGET_CLB    pTargetClb        // ptr to target control record
);

BOOL TMDelTargetClbV5
(
  PTMX_RECORD         pTmRecord,        // ptr to TM record
  PTMX_TARGET_RECORD  pTargetRecord,    // ptr to target record within TM record
  PTMX_OLD_TARGET_CLB pTargetClb        // ptr to target control record
);




ULONG EQFUnicode2Compress( PBYTE pTarget, PSZ_W pInput, ULONG usLenChar );
ULONG EQFCompress2Unicode( PSZ_W pOutput, PBYTE pTarget, ULONG usLenComp );



// prototypes and functions for CPP versions of memory base code

#ifdef _OTMMEMORYPLUGIN_H_

#if defined(INCL_EQFMEM_DLGIDAS)

typedef struct _MEM_LOAD_DLG_IDA
{
 CHAR         szMemName[MAX_LONGFILESPEC];// TM name to import to
 CHAR         szShortMemName[MAX_FILESPEC];// TM name to import to
 CHAR         szMemPath[2048];            // TM path + name to import to
 CHAR         szFilePath[2048];           // File to be imported path + name
 HFILE        hFile;                      // Handle of file to be imported
 PMEM_IDA     pIDA;                       // pointer to the memory database IDA
 BOOL         fAscii;                     // TRUE = ASCII else internal format
 CHAR         szName[MAX_FNAME];          // file to be imp. name without ext
 CHAR         szExt[MAX_FEXT];            // file to be imp. ext. with leading dot
 CHAR         szString[2048];             // string buffer
 CHAR         szDummy[_MAX_DIR];          // string buffer
 BOOL         fDisabled;                  // Dialog disable flag
 CONTROLSIDA  ControlsIda;                //ida of controls utility
 HPROP        hPropLast;                  // Last used properties handle
 PMEM_LAST_USED pPropLast;                // Pointer to the last used properties
 BOOL         fBatch;                     // TRUE = we are in batch mode
 HWND         hwndErrMsg;                 // parent handle for error messages
 PDDEMEMIMP   pDDEMemImp;                 // ptr to batch memory import data
 HWND         hwndNotify;                 // send notification about completion
 OBJNAME      szObjName;                  // name of object to be finished
 USHORT       usImpMode;                  // mode for import
 BOOL         fMerge;                     // TRUE = TM is being merged
 PSZ          pszList;                    // list of selected import files or NULL
 BOOL         fSkipInvalidSegments;       // TRUE = skip invalid segments during import
 ULONG        ulOemCP;                    // CP of system preferences lang.
 ULONG        ulAnsiCP;                   // CP of system preferences lang.
 BOOL         fIgnoreEqualSegments;       // TRUE = ignore segments with identical source and target string
 BOOL         fAdjustTrailingWhitespace;  // TRUE = adjust trailing whitespace of target string
 BOOL         fMTReceiveCounting;         // TRUE = count received words and write counts to folder properties
 OBJNAME      szFolObjName;               // object name of folder (only set with fMTReceiveCounting = TRUE)
 HWND         hwndToCombo;                // handle of "to memory" combo box
 BOOL         fYesToAll;                  // yes-to-all flag for merge confirmation
 BOOL         fImpModeSet;                // TRUE = imp mode has been set by the impot file check logic
 OtmMemory    *pMem;                      // pointer to memory being imported
 OtmProposal  *pProposal;                 // buffer for proposal data
 BOOL          fForceNewMatchID;          // create a new match segment ID even when the match already has one
 BOOL          fCreateMatchID;            // create match segment IDs
 CHAR          szMatchIDPrefix[256];      // prefix to be used for the match segment ID
 CHAR_W        szMatchIDPrefixW[256];     // prefix to be used for the match segment ID (UTF16 version)
}MEM_LOAD_DLG_IDA, * PMEM_LOAD_DLG_IDA;

BOOL    MemImportFileOpenDialog( PMEM_LOAD_DLG_IDA   pLoadIDA );

#endif


/* !brief function handling the GUI mode of the memory import */
__declspec(dllexport) BOOL MemGUIExportMemory
( 
  OtmMemory *pMemory, 
  PLISTCOMMAREA pCommArea,
  PSZ pszMemName, 
  PSZ pszNameList,   
  PMEM_IDA      pIDA,                      // Pointer to the Main Memory data area
  HWND hwnd
);

/* !brief function handling the GUI mode of the memory merge */
__declspec(dllexport) BOOL MemGUIMergeMemory( OtmMemory *pMemory, PLISTCOMMAREA pCommArea, PSZ pszMemName, PMEM_IDA pIDA, HWND hwnd );

/* !brief function handling the GUI mode of the memory import */
__declspec(dllexport) BOOL MemGUIImportMemory( PLISTCOMMAREA pCommArea, PSZ pszMemName, PMEM_IDA pIDA, HWND hwnd );


#endif

