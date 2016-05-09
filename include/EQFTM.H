/*! EQFTM.H       External header file for Translation Memory functions
	Copyright (c) 1990-2016, International Business Machines Corporation and others. All rights reserved.
*/

#ifndef _OTMFUNC_H_INCLUDED
  #include "OTMFUNC.H"
#endif 

// ************ Translation memory flags ********************************
#define far

#define  MEM_OUTPUT_ASIS          0   // Do not convert translation memory output
#define  MEM_OUTPUT_CRLF          1   // Convert translation memory output to CRLF
#define  MEM_OUTPUT_LF            2    // Convert translation memory output to LF

// defines for special names mode of TMExtract (only valid with TMExtract!)
#define  MEM_OUTPUT_TAGTABLES     3   // return list of tag tables of TM
#define  MEM_OUTPUT_AUTHORS       4   // return list of authors of TM
#define  MEM_OUTPUT_DOCUMENTS     5   // return list of documents of TM
#define  MEM_OUTPUT_LANGUAGES     6   // return list of languages of TM
#define  MEM_OUTPUT_LONGNAMES     7   // return list of document long names of TM
#define  MEM_OUTPUT_ALLDOCS       8   // return list of all documensts (long
                                      // names and short names for docs w/o long name)

#define EXCLUSIVE               0
#define NONEXCLUSIVE            1
#define EXCLUSIVE_FOR_GET_PART  2
#define FOR_ORGANIZE            3
#define READONLYACCESS          4

// defines used in TmOpen and MemCreateDlg
#define  TM_LOCAL                  0     // TM resides local
#define  TM_REMOTE                 1     // TM resides remote on a server
#define  TM_LOCALREMOTE            2     // TM may be local or remote
#define  TM_SHARED                 3     // TM resides on a shared drive

//  Match value constants: the similarity level classes   (lSimLevel)
#define BASE_SIMILAR          0L
#define BASE_EXACT_DATE       200L
#define BASE_EXACT_IND        300L
#define BASE_EXACT_SEG_NUM    400L

#define MAX_SIMILAR_VAL       100L
#define MAX_EXACT_DATE_VAL    (BASE_EXACT_IND - 1L)
#define MAX_EXACT_IND_VAL     (BASE_EXACT_SEG_NUM - 1L)
#define MAX_EXACT_SEG_NUM_VAL (500L - 1L)

#define EXTENT_SIMILAR        (MAX_SIMILAR_VAL - BASE_SIMILAR)
#define EXTENT_EXACT_DATE     (MAX_EXACT_DATE_VAL - BASE_EXACT_DATE)
#define EXTENT_EXACT_IND      (MAX_EXACT_IND_VAL - BASE_EXACT_IND)
#define EXTENT_EXACT_SEG_NUM  (MAX_EXACT_SEG_NUM_VAL - BASE_EXACT_SEG_NUM)

// translation flag values
#define TRANSLFLAG_NORMAL      0
#define TRANSLFLAG_MACHINE     1
#define TRANSLFLAG_GLOBMEM     2
#define TRANSLFLAG_GLOBMEMSTAR 3


/*----------------------------------------------------------------------------*\
  System wide threshold values
\*----------------------------------------------------------------------------*/
#define LENGTH_THR         50L  // Length threshold value
#define MAX_LENGTH_THR    100L  // maximal Length threshold value
#define INIT_MATCH_THR     59L  // Initial match threshold value
#define SHORTER_MATCH_THR  50L  // match threshold value for shorter
                                // segments, defined thru LENGTH_SHORTER_VALUE
#define WORDS_MATCH_THR 0L      // Not used yet

//--- Used in GET_IN structure to specify kind of matches.
//--- This defines should be bitwise 'ored' with the number of required matches.
//--- GET_EXACT means if at least one exact match is found return only exact
//--- matches. GET_EXACT_AND_FUZZY means return exact and fuzzy matches.
//--- Example pGetIn->usNumMatchesReq = 3 | GET_EXACT_AND_FUZZY
//--- If only the number of required matches is passed, GET_EXACT is used.
//--- If you want to retrieve translation memory hits with exact context only,
//--- use GET_EXACT_CONTEXT.
//--- If Generic replace should be disabled, you have to or with GET_NO_GENERICREPLACE
#define GET_MOREPROP_INDIC     0x00000200
#define GET_ALL_EXACT_MATCHES  0x00000400
#define GET_RESPECTCRLF        0x00000800
#define GET_IGNORE_PATH        0x00001000
#define GET_NO_GENERICREPLACE  0x00002000
#define GET_EXACT_AND_CONTEXT  0x00004000
#define GET_EXACT_AND_FUZZY    0x00008000
#define GET_ALWAYS_WITH_TAGS   0x00010000
#define GET_IGNORE_COMMENT     0x00020000
#define GET_EXACT              0x00040000

#define EQUAL_EQUAL              100
#define TAGS_EQUAL               98
#define TAGS_UNEQUAL             95

#define NON_EXCLUSIVE       1
#define EXCLUSIVE           0

#define LANG_KEY            1L
#define FILE_KEY            2L
#define AUTHOR_KEY          3L
#define TAGTABLE_KEY        4L
//#define RESERVED_KEY        5L
#define LONGNAME_KEY        5L
#define COMPACT_KEY         6L
#define FIRST_KEY           7L
// Note: the following key is NOT used as the key of a QDAM record
//       it is only a symbolic value used in the Name-to-ID functions
#define LANGGROUP_KEY       8L


// used in GET_OUT structure
#define GET_MORE_EXACTS_AVAIL      0x8000
#define GET_ADDITIONAL_FUZZY_AVAIL 0x4000

/**********************************************************************/
/* Return Codes                                                       */
/**********************************************************************/
#define TMERR_BASE ERR_MEM_BASE            /* Base for errors          */

#define TMERR_EOF     TMERR_BASE + 12

#define DISK_FULL     TMERR_BASE + 13   // for Add /Replace / Create
#define DB_FULL       TMERR_BASE + 14   // when number of blocks exceeds 2**16 - 1

#define SEG_NOT_FOUND TMERR_BASE + 20   /* for Delete command          */
#define CLUSTER_EMPTY TMERR_BASE + 21   /* Internally only             */

#define TM_FILE_NOT_FOUND                  TMERR_BASE + 30 // for Open
#define FILE_ALREADY_OPEN                  TMERR_BASE + 31 // for Open
#define CORRUPTION_FLAG_ON                 TMERR_BASE + 32 // for Open
#define FILE_MIGHT_BE_CORRUPTED            TMERR_BASE + 33 // for Open
#define VERSION_MISMATCH                   TMERR_BASE + 34 // for Open
#define CORRUPT_VERSION_MISMATCH           TMERR_BASE + 35 // for Open
#define TM_FILE_SCREWED_UP                 TMERR_BASE + 36 // for Open
#define NOT_A_MEMORY_DATABASE              TMERR_BASE + 37 // for Open
// the following error code is returned if open fails because a TM was
// corrupted and (on confirmation via a message) was then
// successfully organized
#define TM_WAS_CORRUPTED_IS_ORGANIZED      TMERR_BASE + 38 // for Open,
#define TM_PROPERTIES_DIFFERENT            TMERR_BASE + 39 // for Open,
#define SERVER_DRIVE_REMOVED               TMERR_BASE + 40 // for Open, /*@89A*/
#define SERVER_DRIVE_ACTIVE                TMERR_BASE + 41 // for Open, /*@89A*/
#define TM_PROPERTIES_EQUAL                TMERR_BASE + 42 // for Open,/*@1170A*/
#define TM_PROPERTIES_NOT_OPENED           TMERR_BASE + 43 // for Open,/*@1170A*/

#define BLOCK_SIZE_TOO_SMALL     TMERR_BASE + 48 /* for Create         */
#define FILE_ALREADY_EXISTS      TMERR_BASE + 49 /* for Create         */

#define NO_SEG_FOUND             TMERR_BASE + 50 /* for Extract        */

#define ILLEGAL_TM_COMMAND       TMERR_BASE + 60 /* for TMT            */

#define NOT_REPLACED_OLD_SEGMENT TMERR_BASE + 70 /* for Replace        */
#define SEGMENT_BUFFER_FULL      TMERR_BASE + 71 /* for Replace   */ /*@1108A*/
                                         /*rc from AddSegToCluster*/ /*@1108A*/
#define SEG_SKIPPED_BAD_MARKUP   TMERR_BASE + 72 /* for Import         */
#define SEG_RESET_BAD_MARKUP     TMERR_BASE + 73 /* for Import         */

#define TM_FUNCTION_FAILED       TMERR_BASE + 80 /* Undefinable Error  */

// Currently not used
//#define TMERR_TM_HANDLER_ALREADY_ACTIVE     TMERR_BASE + 90  // Comm code

#define TMERR_TOO_MANY_OPEN_DATABASES       TMERR_BASE + 91  // Comm code
#define TMERR_TOO_MANY_USERS_CONNECTED      TMERR_BASE + 92  // Comm code
#define TMERR_THREAD_NOT_SPAWNED            TMERR_BASE + 93  // Comm code
#define TMERR_TM_OPENED_EXCLUSIVELY         TMERR_BASE + 94  // Comm code
#define TMERR_TM_OPENED_SHARED              TMERR_BASE + 95  // Comm code

// Currently not used
//#define TMERR_PIPE_EXISTS                   TMERR_BASE + 96  // Comm code

#define TMERR_NO_MORE_MEMORY_AVAILABLE      TMERR_BASE + 97  // Comm code
#define TMERR_THREAD_INIT_FAILED            TMERR_BASE + 98  // Comm code
#define TMERR_TOO_MANY_QUERIES              TMERR_BASE + 99  // Comm code
#define TMERR_PROP_EXIST                    TMERR_BASE + 100 // Comm code
#define TMERR_PROP_WRITE_ERROR              TMERR_BASE + 101 // Comm code
#define TMERR_PROP_NOT_FOUND                TMERR_BASE + 102 // Comm code
#define TMERR_PROP_READ_ERROR               TMERR_BASE + 103 // Comm code
#define TMERR_TOO_MANY_TMS                  TMERR_BASE + 104 // Comm code
#define TMERR_SERVER_NOT_STARTED            TMERR_BASE + 105 // Comm code
#define TMERR_SERVERCODE_NOT_STARTED        TMERR_BASE + 106 // Comm code
#define TMERR_COMMUNICATION_FAILURE         TMERR_BASE + 107 // Comm code

// This error code is sent when somebody stops the server code and open connections
// are still there (it's not guaranteed that this error code makes it to the requester
// since the pipe is closed immediately; if this error code doesn't make it to the
// requester, TMERR_COMMUNICATION_FAILURE will be returned)
#define TMERR_SERVER_ABOUT_TO_EXIT          TMERR_BASE + 108 // Comm code
#define TMERR_TOO_MANY_USERS_ON_SERVER      TMERR_BASE + 109 // Comm code

#define SOURCE_STRING_ERROR                 TMERR_BASE + 150
#define ERROR_ADD_TO_TM                     TMERR_BASE + 151

#define LANG_FILE_NOT_FOUND                 TMERR_BASE + 200 // for Create
#define LANGUAGE_NOT_FOUND                  TMERR_BASE + 201 // for Create
#define LANG_FILE_LINE_TOO_LONG             TMERR_BASE + 202 // for Create
#define LANG_CHAR_APPEARS_TWICE             TMERR_BASE + 203 // for Create
#define LANG_GROUP_NUM_TOO_LARGE            TMERR_BASE + 204 // for Create
#define TAG_FILE_NOT_FOUND                  TMERR_BASE + 205 // for Create
#define WORD_FILE_NOT_FOUND                 TMERR_BASE + 206 // for Create
#define TAG_FILE_TOO_LARGE                  TMERR_BASE + 207 // for Create /*@ZDA*/
#define ERROR_OLD_PROPERTY_FILE             TMERR_BASE + 208
#define ERROR_REMOTE_TM_NOT_SUPPORTED       TMERR_BASE + 209
#define ERROR_VERSION_NOT_SUPPORTED         TMERR_BASE + 210

#define  MEM_PROCESS_OK        6900              // Return code of load is OK
#define  MEM_READ_ERR          MEM_PROCESS_OK+1  // Dos read error
#define  MEM_WRITE_ERR         MEM_PROCESS_OK+2  // Dos write error
#define  MEM_SEG_SYN_ERR       MEM_PROCESS_OK+3  // Segment syntax error
#define  MEM_FILE_SYN_ERR      MEM_PROCESS_OK+4  // File syntax error
#define  MEM_LOAD_OK           MEM_PROCESS_OK+5  // Return code of load dialog is OK
#define  MEM_PROCESS_END       MEM_PROCESS_OK+6  // ID which indicates that a proc is terminated
#define  MEM_DB_ERROR          MEM_PROCESS_OK+7  // Unpredictable memory db error
#define  MEM_DB_CANCEL         MEM_PROCESS_OK+8  // load process was canceled /*@47*/
#define  MEM_DB_CANCEL         MEM_PROCESS_OK+8  // load process was canceled /*@47*/
#define  MEM_DB_EXTRACT_ERROR  MEM_PROCESS_OK+9  // retuned from function MemReadWriteSegment when TmExtract fails /*@47*/
#define  MEM_DB_REPLACE_ERROR  MEM_PROCESS_OK+10 // retuned from function MemReadWriteSegment when TmReplace fails /*@47*/

/* The segment marker */
#define  SEGMARKER       "###"

#define KEY_DIR_SIZE         4096  // key directory size
#define TM_PREFIX_SIZE          8  // length of prefix bytes in TMT db
#define MAX_SEC_LENGTH       30        // max length of secondary key
#define MAX_LINE_LENGTH      80        // max length of each line in lang file
#define FN_LENGTH            13        // length of input filename
#define PRIM_KEY_LENGTH       4        // number of characters in primary key
#define DATA_IN_SIZE       3900        // buffer size for input
#define DATA_OUT_SIZE      3900        // buffer size for output
#define CODEPAGE_SIZE       256        // size of codepage of the language
#define SEG_MARKER_LENGTH     3        // length of segment marker
#define MAX_TGT_LENGTH     2047        // max length of each target.
#define MAX_MATCH_TAB_ENTRIES 5        // number of entries in match table
#define CREATE_BUFFER_SIZE   40000     // buffer size for create_in
#define MAX_TM_LIST_NUMBER    500      // max. number of TMs that can be listed
                                       // by the TMC_GET_SERVER_TM_LIST command
#define GETPART_BUFFER_SIZE 16384      // read a 16 KB block at a time
#define MEM_PROP_SIZE    2048          // Global size of all memory database properties

/**********************************************************************/
/* indicatos for old or new TM                                        */
/**********************************************************************/
#define OLD_TM      0
#define NEW_TM      1

/**********************************************************************/
/* indicator for organize                                             */
/* TM_CONVERT    - convert an ols TM to an new one                    */
/* TM_ORGANIZE   - organize a new TM                                  */
/**********************************************************************/
#define TM_CONVERT   0
#define TM_ORGANIZE  1


/**********************************************************************/
/* default threshold value for get                                    */
// GQ: beginning with TP6.0.1 the real fuzziness of a proposal (including
//     inline tagging is used. A new threshold has been added to be checked
//     against the new computed fuzziness, the old value is used
//     for the triple threshold checking
/**********************************************************************/
#define TM_DEFAULT_THRESHOLD     33   // prior: 40
#define TM_FUZZINESS_THRESHOLD   10

/**********************************************************************/
/* Marker for NTM in new TM properties                                */
/**********************************************************************/
#define NTM_MARKER "#@@NTM01@@#"



typedef LHANDLE HTM;
typedef CHAR  SHORT_FN  [FN_LENGTH];
typedef CHAR  BUFFERIN  [DATA_IN_SIZE];
typedef UCHAR BUFFEROUT  [DATA_OUT_SIZE],
              ACHPRIMKEY [PRIM_KEY_LENGTH],
              SZSECKEY   [MAX_SEC_LENGTH + 1];

typedef CHAR  LANG_LINE  [MAX_LINE_LENGTH + 1];
typedef CHAR  LONG_FN    [MAX_LONGFILESPEC];

typedef BYTE ABGROUP [CODEPAGE_SIZE];
typedef ABGROUP * PABGROUP;

typedef struct _TM_ADDRSS { /* addr */
   USHORT usEntryInDir,      /* >= 0, entry# of the cluster in KeyDir  */
                             /* <= 4095                                */
          usBlockNumber,     /* >  0, indicates block number in TM     */
                             /* 0  used for chaining purposes as null  */
          usDispBlockPtr;    /* size(block header) <= BLOCK_SIZE       */
                             /* indicates location within a block      */
} TM_ADDRESS, * PTM_ADDRESS ;

typedef struct _SEGMENT { /* seg */
   UCHAR   achSegMarker[SEG_MARKER_LENGTH] ;
                              /* marker to beginning of segment        */
   USHORT  usLenSegment,      /* total length of segment               */
           usDispIndustry,    /* industry codes location in bufData    */
           usLenIndustry,     /* length of indus. codes list(BYTES)    */
           usDispSource,      /* source location relative to bufData   */
           usLenSource,       /* source length in bytes                */
           usDispTarget,      /* target location relative to bufData   */
           usLenTarget,       /* target length in bytes                */
           usDispReserved,    /* Reserved area  location in bufData    */
           usLenReserved,     /* length of reserved area               */
           usDispSecKey,      /* secondary key  location in bufData    */
           usLenSecKey,       /* length of secondary key               */
           usDispContext,     /* Context area  location in bufData     */
           usLenContext;      /* length of context area                */
   SHORT_FN szFileName;       /* name of source file (short name)      */
   LONG_FN  szLongName;       /* name of source file (long name or EOS)*/
   USHORT  usSegNumber;       /* segment number in file                */
   EQF_BOOL fLogicalDel,      /* set when logically deleted segment    */
           fAscii;            /*                                       */
   USHORT  usTranslationFlag; /* type of translation, 0 = human, 1 = machine, 2 = Global Memory */
   TIME_L  tStamp;            /* time taken from c function            */
   BUFFERIN bufData;          /* fields with variable lengths          */
} SEGMENT, * PSEGMENT;

typedef struct _OLDSEGMENT { /* seg */
   UCHAR   achSegMarker[SEG_MARKER_LENGTH] ;
                              /* marker to beginning of segment        */
   USHORT  usLenSegment,      /* total length of segment               */
           usDispIndustry,    /* industry codes location in bufData    */
           usLenIndustry,     /* length of indus. codes list(BYTES)    */
           usDispSource,      /* source location relative to bufData   */
           usLenSource,       /* source length in bytes                */
           usDispTarget,      /* target location relative to bufData   */
           usLenTarget,       /* target length in bytes                */
           usDispReserved,    /* Reserved area  location in bufData    */
           usLenReserved,     /* length of reserved area               */
           usDispSecKey,      /* secondary key  location in bufData    */
           usLenSecKey;       /* length of secondary key               */
   SHORT_FN szFileName;       /* name of source file (short name)      */
   USHORT  usSegNumber;       /* segment number in file                */
   EQF_BOOL fLogicalDel,      /* set when logically deleted segment    */
           fAscii;            /*                                       */
   USHORT  usTranslationFlag; /* type of translation, 0 = human, 1 = machine, 2 = Global Memory */
   TIME_L  tStamp;            /* time taken from c function            */
   BUFFERIN bufData;          /* fields with variable lengths          */
} OLDSEGMENT, * POLDSEGMENT;

typedef struct _MATCH { /* mtch */
   TM_ADDRESS  addr;        /* address of the matching segment         */
   USHORT  usTranslationFlag; /* type of translation, 0 = human, 1 = machine, 2 = Global Memory */
   USHORT   usNumExactBytes;/* no. of exact bytes in the               */
                            /*     input and matched sentence          */
   TIME_L   tStamp;         /* time stamp of segment                   */
   LONG     lSimLevel,      /* <= 100 indicates similar                */
                            /* >= 200 indicate exact                   */
                            /* >= 300 indicate exact+same ind code     */
                            /* >= 400 indicate exact+same filename     */
            lLengthTest,    /* value of lengths test                   */
            lInitMatchTest, /* value of initials test                  */
            lWordsTest;     /* value of words test                     */
                     /* last 3 values are meaningless for exact matches*/
   UCHAR     szTarget[MAX_TGT_LENGTH + 1]; /* The target               */
} MATCH , * PMATCH;

/*
    More details about the match ranking method.

    The similarity level has a crude primary ranking as follows:

    400 - 499: If there is an exact match and the filename is the same.

    300 - 399: If there is an exact match, the filename is not the same,
                  but there is at least one industry code in common to
                  database segment and input segment.

    200 - 299: If there is an exact match but no filename or industry code
                  in common.

    0 - 100:   Similar match but no exact match.

    Secondary ranking within the ranges above is as follows:

    In the 400 - 499 range, the actual value is determined by proximity
        of database sentence number (within the text file from which it was
        taken), to the ordinal number of the input sentence within the input
        file.
    In the 300 - 399 range, the actual value is determined by number of equal
        industry codes vs the total number of industry codes that appear in
        both sentences.
    In the 200 - 299 range, the actual value is determined by the proximity
        of dates of the two segments.

    In the case of similar match the value is that of the init test. (See
        explanation to lInitMatchThr below.)

    The exact formulae by which the secondary ranking is evaluated is
        described both in the LLD document and in code comments.
                                                                       */

typedef struct  _PREFIX_IN {
   USHORT  usLenIn,           /* total length of input buffer          */
           idCommand ;        /* command id, previously defined.       */
// } PREFIX_IN, * PPREFIX_IN, IN, * PIN;
} PREFIX_IN, * PPREFIX_IN, * PIN;

typedef struct  _PREFIX_OUT {
   USHORT  usLenOut;           /* total length of output buffer        */
   BOOL    fDiskFull,          /* for Add /Replace / Create when number*/
   fDBfull;                    /* of blocks is a cluster exceeds       */
                               /* 2**16 - 1. Filled by FormatMore      */
   USHORT  rcTmt;              /* returned by TMT function             */

//} PREFIX_OUT, * PPREFIX_OUT, OUT, * POUT;
} PREFIX_OUT, * PPREFIX_OUT, * POUT;

typedef struct  _UPDATE_IN {
   PREFIX_IN  prefin;          /* prefix of each command               */
   SEGMENT    segIn;           /* the rest of this type is a SEGMENT   */
} UPDATE_IN, * PUPDATE_IN;

typedef UPDATE_IN  ADD_IN,  DEL_IN,  REP_IN;
typedef PUPDATE_IN PADD_IN, PDEL_IN, PREP_IN;

typedef struct  _ADD_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */
   TM_ADDRESS  addr;        /* address of added segment                */
} ADD_OUT, * PADD_OUT;

typedef struct  _DEL_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */
   TM_ADDRESS  addr;        /* address of deleted segment              */
} DEL_OUT , * PDEL_OUT ;

typedef struct  _REP_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */
   SERVERNAME   szServer;   /* server name                             */
   TM_ADDRESS  addrDel,     /* address to delete                       */
               addrAdd;     /* address to add                          */
} REP_OUT , * PREP_OUT;

typedef struct  _CLOSE_IN  {
   PREFIX_IN  prefin ;      /* prefix of each command                  */
} CLOSE_IN, * PCLOSE_IN;

typedef struct  _CLOSE_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */
} CLOSE_OUT, * PCLOSE_OUT;

typedef struct  _EXT_IN {
   PREFIX_IN  prefin;       /* prefix of each command                  */
   TM_ADDRESS addr;         /* address of extracted segment            */
   USHORT     usConvert;    // Indicates how the output should be converted
} EXT_IN  , * PEXT_IN ;

typedef struct  _EXT_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */
   SERVERNAME   szServer;   /* server name                             */
   TM_ADDRESS  addr,        /* address of current segment              */
               addrNext;    /* address of next segment                 */
   SEGMENT     segOut;      /* fields with variable lengths            */
} EXT_OUT , * PEXT_OUT;

typedef struct _GET_IN {
   PREFIX_IN prefin;        /* prefix of each command                  */
   SHORT_FN  szFileName;    /* name of source file (short name)        */
   LONG_FN   szLongName;    /* name of source file (long name or EOS)  */
   USHORT    usConvert;    // Indicates how the output should be converted
   USHORT    usSegNumber,   /*                                         */
             usNumMatchesReq,/* number of matches required             */
             usDispIndustry,/* industry codes location in bufData      */
             usLenIndustry, /* length of indus. codes list(BYTES)      */
             usDispSource,  /* source location relative to bufData     */
             usLenSource;   /* source length in bytes                  */
   LONG      lLengthThr,    /* needed to filter out sentences          */
                            /* with different lengths                  */
             lInitMatchThr, /* needed to filter out sentences          */
                            /* with non matching initials              */
             lWordsMatchThr;/* needed to filter out sentences          */
                            /* with non matching words                 */
   BOOL      fAscii;        /*                                         */
   BUFFERIN  bufData;       /* fields with variable lengths            */
} GET_IN, * PGET_IN;

/**********************************************************************/
/* structure passed in case of rename a file ...                      */
/**********************************************************************/
typedef struct _RENFILE_IN
{
   PREFIX_IN    prefin;                /* prefix of each command             */
   SERVERNAME  szServer;                // server name
   CHAR    szOldFile[ MAX_EQF_PATH ];   // old file name
   CHAR    szNewFile[ MAX_EQF_PATH ];   // new file name
   CHAR         szUserId [MAX_USERID];  // userId logged on to requester
} RENFILE_IN, * PRENFILE_IN;

typedef struct _RENFILE_OUT
{
   PREFIX_OUT   prefout;                           /* prefix of each command */
} RENFILE_OUT, * PRENFILE_OUT;

/*  More details about lLengthThr and lInitMatchThr:

    lLengthThr:

    In the length test we compute a length value which measures how close
    are the lengths of the current sentence taken from the database versus
    the input sentence.
    The length value is computed by the formula:

    lLengthValue = ( EXTENT_SIMILAR * min( usLenInput, usLenCurrent ) ) /
                     max( usLenInput, usLenCurrent );

    where EXTENT_SIMILAR is the constant 100 for our purposes.

    The length is measured in number of words rather than characters.
    If LengthValue is 100 it means that both sentences have the same number of
    words. If one sentence has 10 words while the other has 5,
    lLengthValue = 100 * 5/10 which is about 50. In general LengthValue will
    be about the difference in percents. (10 and 5 differ by 50%).

    lLengthThr (the Length Threshold value) is the minimum value allowed in
    the length test. If LengthValue <= lLengthThr the length test fails.
    lLengthThread is increased in dependeny of the number of words of the
    input segment. For more detais see function LengthCorrectThresholds and
    CheckSimilar in file EQFTMRTV.C


    lMatchInitThr:

    lMatchInitThr is used to check whether the initials of words in both
    sentences is about the same. To that end we compute the value:

      lMatchValue = (EXTENT_SIMILAR * usSameInitilas) /
                                           max(usLenInput, usLenCurrent);

    where usSameInitials denotes the number of initials that are common
    to both sentences. lMatchValue is 100 if all the initials of one
    sentence are contained in the other.

    Assuming that both sentences have about the same number of words
    (since they passed the length test), the MatchValue denotes
    their similarity of initials in precents. If they have 20 words each
    and 15 initials are the same then MatchValue = 75.

    InitMatchThr is the minimal value allowed.
    An lActMatchThr is increased in dependency of the number of words in the
    input segment. For more detais see function LengthCorrectThresholds and
    CheckSimilar in file EQFTMRTV.C

    if MatchValue <= lActMatchThr the match value test fails

 */

typedef struct  _GET_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */

   USHORT usNumMatchesFound,/* number of matches found                 */
          usNumMatchesValid,/* number of matches valid                 */
          ausSortedMatches[MAX_MATCH_TAB_ENTRIES];
   MATCH  amtchBest[MAX_MATCH_TAB_ENTRIES]; /* Matches array           */
} GET_OUT, * PGET_OUT;

typedef struct  _OPEN_IN {
   PREFIX_IN  prefin;       /* prefix of each command                  */
   SERVERNAME szServer;     /* servername                              */
   CHAR       szUserId [MAX_USERID]; /* userId logged on to requester  */
   FILENAME  szTmFileName;  /* name of Tm database to open             */
   BOOL       fExclusive;   /* indicate the mode for opening the file  */
   BOOL       fOpenGetPart; /* indicate if Open was due to a get part  */
} OPEN_IN , * POPEN_IN;


typedef struct  _OPEN_OUT {
   PREFIX_OUT  prefout;     /* prefix of Output buffer                 */
   HTM         htm;         /* handle to the TM                        */
} OPEN_OUT , * POPEN_OUT;

typedef struct _BLOCK_HEADER { /* bh */
   USHORT usBlockNum,       /* >= 1, points to previous block          */
          usPrevBlock,      /* >= 1, points to previous block          */
          usNextBlock,      /* >  1, points to next block.             */
                            /* 0 indicates no chaining                 */
          usFirstAvailByte; /* >= size of block header,                */
                            /* <= block size                           */
                            /* points to first available byte in block */
                            /* If block is full it sets to BlockSize   */
} BLOCK_HEADER , * PBLOCK_HEADER;

typedef struct _CREATE_TMH { /* ctmh */
   FILENAME  szTmFileName;  /* Tm database name                        */
   USHORT    usBlockSize;
   LANGUAGE  szSourceLang,  /* source language                         */
             szTargetLang;  /* target language                         */
   EQF_BOOL  fDbcs;         /* double byte char support 0/1            */
} CREATE_TMH;

typedef struct _CREATE_IN { /* crei */
   PREFIX_IN  prefin;       /* prefix of each command                  */
   SERVERNAME szServer;
   CREATE_TMH ctmh;         /* parameters that copied into Tm header   */
   TIME_L  tCreate;         /* creation time stamp                     */
   ABGROUP abABGrouping;
   USHORT  usDispExclTagList, /* offset of tag list                    */
           usLenExclTagList,  /* length of tag list                    */
           usDispExclWordList,/* offset of word list                   */
           usLenExclWordList; /* length of word list                   */
   CHAR    bufData[CREATE_BUFFER_SIZE];
   CHAR    szUserId [MAX_USERID]; /* userId logged on to requester     */
} CREATE_IN , * PCREATE_IN;

typedef OPEN_OUT CREATE_OUT, *PCREATE_OUT;

typedef CLOSE_IN   INFO_IN, * PINFO_IN;

typedef struct _INFO_OUT  { /* infi */
   PREFIX_OUT prefout;      /* prefix of each command                  */
   CREATE_TMH ctmh;         /* parameters that copied into Tm header   */
   TIME_L  tCreate;         /* creation time stamp                     */
   ABGROUP abABGrouping;
   USHORT  usDispExclTagList, /* offset of tag list                    */
           usLenExclTagList,  /* length of tag list                    */
           usDispExclWordList,/* offset of word list                   */
           usLenExclWordList; /* length of word list                   */
   CHAR    bufData[CREATE_BUFFER_SIZE];
} INFO_OUT , * PINFO_OUT;

// ----------------------------------------------------------------------------
// structures needed for the TMC_GET_SERVER_DRIVES command which lists all
// disk drives on a server (which are available for the EQF TM Server program)
// The first Drive returned is the system drive (where directory \COMMPROP
// is located - this directory can only exist on the system drive, not on
// any of the secondary drives)
// ----------------------------------------------------------------------------

typedef struct _DRIVES_IN {              /* indr */
   PREFIX_IN    prefin;                  /* prefix of each command */
   SERVERNAME   szServer;                /* which server ?         */
   CHAR         szUserId [MAX_USERID];   /* userId logged on to requester */
} DRIVES_IN, * PDRIVES_IN;

typedef struct _DRIVE_INFO {      /* drin */
   CHAR         cDriveLetter;     /* drive letter of disk drive              */
   ULONG        ulFreeSpace;      /* number of bytes left on this disk drive */
} DRIVE_INFO, * PDRIVE_INFO;

typedef struct _DRIVES_OUT {                    /* outdr */
   PREFIX_OUT   prefout;                        /* prefix of each command    */
   USHORT       usValidDrives;                  /* how much drives have been */
                                                /* found on the server ?     */
   DRIVE_INFO   adrinDrives [MAX_DRIVELIST];    /* info for each valid       */
                                                /* server drive              */
} DRIVES_OUT, * PDRIVES_OUT;


// ----------------------------------------------------------------------------
// structures needed for the TMC_GET_SERVER_TM_LIST command which lists all
// translation memories that have been found on the selected server; for each
// TM its full path name and its current file size is returned
// ----------------------------------------------------------------------------
typedef struct _FILE_LIST_IN {         /* intl                          !!! CHM */
   PREFIX_IN    prefin;                /* prefix of each command        */
   SERVERNAME   szServer;              /* which server ?                */
   CHAR         szUserId [MAX_USERID]; /* userId logged on to requester */
} FILE_LIST_IN, * PFILE_LIST_IN; // !!! CHM

typedef struct _FILE_INFO {                      /* tmin !!! CHM */
   CHAR         szPathFileName [MAX_EQF_PATH];   /* full filename of file    */
   ULONG        ulFileSize;                      /* filesize in bytes        */
} FILE_INFO, * PFILE_INFO; // !!! CHM


typedef struct _FILE_LIST_OUT {                    /* outtl !!! CHM */
   PREFIX_OUT   prefout;                           /* prefix of each command */
   USHORT       usValidFiles;                      /* valid entries in array */
   FILE_INFO    aflinFileList [MAX_TM_LIST_NUMBER]; /* info for avail. files */
} FILE_LIST_OUT, * PFILE_LIST_OUT; // !!! CHM


// ----------------------------------------------------------------------------
// structures needed for the TMC_GET_PART_OF_TM_FILE command that transfers
// a selected part of the physical TM file to the Troja requester; input
// structure contains file offset and number of bytes to transfer within this
// block; the returned value of bytes read indicates whether the end of file
// has been reached (ulBytesRead < ulBytesToRead OR ulBytesRead = 0); the
// maximum size to transfer at a time is GETPART_BUFFER_SIZE
// ----------------------------------------------------------------------------


typedef struct _GETPART_IN {      /* ingp */
   PREFIX_IN    prefin;           /* prefix of each command                  */
   ULONG        ulFilePos;        /* file position of first byte to transfer */
   ULONG        ulBytesToRead;    /* number of bytes to transfer             */
} GETPART_IN, * PGETPART_IN;

typedef struct _GETPART_OUT {          /* outgp */
   PREFIX_OUT   prefout;               /* prefix of each command             */
   ULONG        ulBytesRead;           /* number of bytes actually read      */
   ULONG        ulNextFilePos;         /* file position of next byte to read */
   UCHAR        aucOutBuffer [GETPART_BUFFER_SIZE]; /* output buffer        */
} GETPART_OUT, * PGETPART_OUT;

typedef struct _GETDICTPART_IN {
   PREFIX_IN    prefin;           /* prefix of each command                  */
   SERVERNAME   szServer;         /* which server ?                          */
   PVOID        pBTree ;          // ptr to dict file handles
   ULONG        ulFilePos;        /* file position of first byte to transfer */
   ULONG        ulBytesToRead;    /* number of bytes to transfer             */
} GETDICTPART_IN, * PGETDICTPART_IN;

// ----------------------------------------------------------------------------
// structures needed for the TMC_PUT_TM_PROPERTIES command which is used to
// copy a property file for a remote TM to the server (during TM creation
// process); the contents of the property file are read into the input buffer
// which is up to MEM_PROP_SIZE bytes long; the actual length of the
// property file has to be passed in the ulPropLength field; the field
// szTMPathFileName contains the name of the TM file on the server (including
// the drive and path) which is then used to create a filename for the property
// file (same drive and path but new extension (will be .PRP)
// ----------------------------------------------------------------------------
typedef struct _PUTPROP_IN {                   /* inpp                          */
   PREFIX_IN    prefin;                        /* prefix of each command        */
   SERVERNAME   szServer;                      /* which server ?                */
   CHAR         szPathFileName [MAX_EQF_PATH]; /* full filename                 */
   ULONG        ulPropLength;                  /* length of property file       */
   UCHAR        aucInBuffer [MEM_PROP_SIZE];   /* buffer for file data          */
   CHAR         szUserId [MAX_USERID];         /* userId logged on to requester */
} PUTPROP_IN, * PPUTPROP_IN;

// !!! CHM Start
typedef struct _PUTDICTPROP_IN {               /* inpp                          */
   PREFIX_IN    prefin;                        /* prefix of each command        */
   SERVERNAME   szServer;                      /* which server ?                */
   CHAR         szPathFileName [MAX_EQF_PATH]; /* full filename                 */
   ULONG        ulPropLength;                  /* length of property file       */
   CHAR         szUserId [MAX_USERID];         /* userId logged on to requester */
   PROPDICTIONARY DictProp;                    /* dictionary property buffer    */
} PUTDICTPROP_IN, * PPUTDICTPROP_IN;

/* define union to allow combined processing in server code */
typedef union _PUTPROPCOMBINEDIN
{
   PUTPROP_IN     TmPropIn;
   PUTDICTPROP_IN DictPropIn;
} PUTPROPCOMBINED_IN, * PPUTPROPCOMBINED_IN;
// !!! CHM End

typedef struct _PUTPROP_OUT {                      /* outpp                     */
   PREFIX_OUT   prefout;                           /* prefix of each command    */
} PUTPROP_OUT, * PPUTPROP_OUT;


// ----------------------------------------------------------------------------
// structures needed for the TMC_GET_TM_PROPERTIES command which is used to
// copy a property file for a remote TM from the server to the Troja requester
// (during 'Include Remote TM' process); the contents of the property file are
// put into the output buffer which is up to MEM_PROP_SIZE bytes long; the
// actual length of the property file will be passed in the ulPropLength field;
// the field szTMPathFileName must contain the full path name of the TM file
// on the server and will be used to create the filename of the belonging
// property file which is stored on the same disk in the same path with the
// extension .PRP
// ----------------------------------------------------------------------------
typedef struct _GETPROP_IN {                   /* ingp                          */
   PREFIX_IN    prefin;                        /* prefix of each command        */
   SERVERNAME   szServer;                      /* which server ?                */
   CHAR         szPathFileName [MAX_EQF_PATH]; /* full filename                 */
   CHAR         szUserId [MAX_USERID];         /* userId logged on to requester */
} GETPROP_IN, * PGETPROP_IN;

typedef struct _GETPROP_OUT {                     /* outgp                     */
   PREFIX_OUT   prefout;                          /* prefix of each command    */
   ULONG        ulPropLength;                     /* length of property file   */
   UCHAR        aucOutBuffer [MEM_PROP_SIZE];     /* buffer for file data      */
} GETPROP_OUT, * PGETPROP_OUT;

// !!! CHM Start
typedef struct _GETDICTPROP_OUT {                 /* outgp                     */
   PREFIX_OUT   prefout;                          /* prefix of each command    */
   ULONG        ulPropLength;                     /* length of property file   */
   PROPDICTIONARY DictProp;                       /* dictionary property buffer*/
} GETDICTPROP_OUT, * PGETDICTPROP_OUT;

/* define union to allow combined processing in server code */
typedef union _GETPROPCOMBINEDOUT
{
   GETPROP_OUT       TmPropOut;
   GETDICTPROP_OUT   DictPropOut;
} GETPROPCOMBINED_OUT, * PGETPROPCOMBINED_OUT;
// !!! CHM End


// ----------------------------------------------------------------------------
// structures needed for the TMC_END_ORGANIZE command which is used to end a
// running 'Organize' process; this command is sent to the original TM instead
// of the TM_CLOSE command and handles the closing, deletion and renaming of
// the original and temporary TM file; the temporary file has to be closed
// previously; the two filenames must contain the full specifications of the
// original TM file to be deleted and the temporary file to be renamed to the
// original TM filename;
// ----------------------------------------------------------------------------
typedef struct _ENDORG_IN {            /* ineo */
   PREFIX_IN    prefin;                /* prefix of each command             */
   CHAR         szOrgTM [MAX_EQF_PATH];/* full filename of original TM file  */
   CHAR         szTmpTM [MAX_EQF_PATH];/* full filename of temporary TM file */
} ENDORG_IN, * PENDORG_IN;

typedef struct _ENDORG_OUT {                       /* outeo */
   PREFIX_OUT   prefout;                           /* prefix of each command */
} ENDORG_OUT, * PENDORG_OUT;


// ----------------------------------------------------------------------------
// structures needed for the TMC_DELETE_TM command which is used to physical
// delete a TM file and the belonging property file on the server;
// before this command can be submitted, the file has to opened exclusively;
// the supplied filename must contain the full drive and path information;
// ----------------------------------------------------------------------------
typedef struct _DELTM_IN {                       /* indt */
   PREFIX_IN    prefin;                          /* prefix of each command   */
   CHAR         szTMPathFileName [MAX_EQF_PATH]; /* full filename of TM file */
   CHAR         szPropPathFileName[MAX_EQF_PATH]; /* full filename of Prop file */
} DELTM_IN, * PDELTM_IN;

typedef struct _DELTM_OUT {                        /* outdt */
   PREFIX_OUT   prefout;                           /* prefix of each command */
} DELTM_OUT, * PDELTM_OUT;


// ----------------------------------------------------------------------------
// structures needed for the TMC_DELETE_FILE command which is used to physical
// delete a file on the server; this command is only used during startup of the
// TM organization process to cleanup eventually existing temporary files (from
// a previous organize process that died during execution);
// the supplied filename must contain the full drive and path information;
// ----------------------------------------------------------------------------
typedef struct _DELFILE_IN {               /* indf                           */
   PREFIX_IN    prefin;                    /* prefix of each command         */
   SERVERNAME   szServer;                  /* which server ?                 */
   CHAR         szFileName [MAX_EQF_PATH]; /* full filename of file to delete*/
   CHAR         szUserId [MAX_USERID];     /* userId logged on to requester  */
} DELFILE_IN, * PDELFILE_IN;

typedef struct _DELFILE_OUT {                      /* outdf                  */
   PREFIX_OUT   prefout;                           /* prefix of each command */
} DELFILE_OUT, * PDELFILE_OUT;

typedef struct  _CLOSEHANDLER_IN
{
   PREFIX_IN   prefin;
   USHORT      hth;
} CLOSEHANDLER_IN, * PCLOSEHANDLER_IN;

typedef struct  _EXIT_IN
{
   PREFIX_IN   prefin;
} EXIT_IN, * PEXIT_IN;

typedef struct  _EXIT_OUT
{
   PREFIX_OUT  prefout;
} EXIT_OUT, * PEXIT_OUT;

// ----------------------------------------------------------------------------
// data structure used for the TMC_QUERY_FILE_INFO which returns the file info
// structure of the specified file returned by the DosFindFirst call on the
// server
// ----------------------------------------------------------------------------

typedef struct _FILEINFO_IN {                      /* infi */
   PREFIX_IN    prefin;                            /* prefix of each command */
   CHAR         szFileName [MAX_EQF_PATH]; /* full filename of file to query */
   SERVERNAME   szServer;                          /* which server ?         */
   CHAR         szUserId [MAX_USERID]; /* userId logged on to requester !!!! CHM */
} FILEINFO_IN, * PFILEINFO_IN;

typedef struct _FILEINFO_OUT {                  /* outfi */
   PREFIX_OUT   prefout;                        /* prefix of each command    */
   FILEFINDBUF  stFile;                         /* info about file           */
} FILEINFO_OUT, * PFILEINFO_OUT;


typedef struct _TM_HEADER { /* tmh */
/* fixed part */
   UCHAR    achTmPrefix[TM_PREFIX_SIZE];
                            /* should be initiated to EQFTMT$$ 8       */
   USHORT  usTmHeaderSize,  /* actual length of tm header              */
           usDbVersion;
   LONG    ldispFirstBlock; /* disp of first block from                */
                            /* beginning of TM                         */
   CREATE_TMH ctmh;         /* parameters provided by the create       */
   TIME_L  tCreate;         /* creation time stamp                     */
   ABGROUP abABGrouping;

/* updated part */
   EQF_BOOL    fCorruption;
   USHORT  usNumTMBlocks,    /* number of blocks in the TM             */
           usNumFreeBlocks,  /* # of free, pre-formatted blocks        */
           usFirstAvailBlock,/* points to first available block        */
           ausKeyDirectory[KEY_DIR_SIZE],   /* key directory           */

           usDispExclTagList, /* offset of tag list                    */
           usLenExclTagList,  /* length of tag list                    */
           usDispExclWordList,/* offset of word list                   */
           usLenExclWordList; /* length of word list                   */

} TM_HEADER, * PTM_HEADER, ** PPTM_HEADER;

#define TAG_LEN             35
#define MAX_NAME            8
#define MAX_RANDOM          20
//#define MAX_VOTES           20 //     30
#define MAX_VOTES           30           // change 27.2.2000
#define ABS_VOTES           400
#define MAX_MATCHES         15
#define TOK_SIZE            4000
#define MAX_COMPACT_SIZE    3217   // 256
#define TMX_REC_SIZE        32760
#define TMX_TABLE_SIZE      512
#define MAX_INDEX_LEN       8150 //  2048

/**********************************************************************/
/* The version number is stored in the signature record of the TMs    */
/*                                                                    */
/* TM_MAJ_VERSION and TM_MIN_VERSION is always the current version    */
/* used to create new TMs                                             */
/*                                                                    */
/* A change of the major version always forces an organize of the     */
/* TM. If the major version of a TM is larger than the TM_MAJ_VERSION */
/* used during compile of the DLLs the TM cannot be opened (even not  */
/* for organize). A message will be displayed which will tell the     */
/* user to get a new version of TranslationManager to work with the   */
/* TM.                                                                */
/*                                                                    */
/* Version | Description                 | Special Handling for TMs   */
/* Maj Min |                             |                            */
/* --------+-----------------------------+----------------------------*/
/*  1   0  | Base version of TMs         | use old hash and tokenize  */
/*         |                             | functions                  */
/* --------+-----------------------------+----------------------------*/
/*  2   0  | This version uses new hash  |                            */
/*         | and tokenize functions to   |                            */
/*         | overcome the problems with  |                            */
/*         | non-alphabetic characters in|                            */
/*         | terms and not-recognized    |                            */
/*         | attribute strings           |                            */
/* --------+-----------------------------+----------------------------*/
/*  3   0  | - This version number is set|                            */
/*         |   automatically when a      |                            */
/*         |   version 2 TM has tersed   |                            */
/*         |   name tables (because the  |                            */
/*         |   table exceeds the 32k QDAM|                            */
/*         |   barrier for data records) |                            */
/*         | - the version number has    |                            */
/*         |   been splitted into a byte |                            */
/*         |   for the major version and |                            */
/*         |   a byte for the minor      |                            */
/*         |   version                   |                            */
/* --------+-----------------------------+----------------------------*/
/*  4   0  | - removed multiple flag     |                            */
/*         | - use protect table exit    |                            */
/*         |   for tag recognition       |                            */
/*         | - use least-edit string     |                            */
/*         |   for fuzzy match ranking   |                            */
/* --------+-----------------------------+----------------------------*/
/*  5   0  | - allow name tables with    |                            */
/*         |   a size of more than 32kB  |                            */
/*         |   (organize of TM required) |                            */
/* --------+-----------------------------+----------------------------*/
/*  6   0  | - allow text in unicode     |                            */
/*         |   a size of more than 32kB  |                            */
/*         |   (organize of TM required) |                            */
/* --------+-----------------------------+----------------------------*/
/*                                                                    */
/* Current version: 7.0                                               */
/**********************************************************************/
#define TM_VERSION_1        1
#define TM_VERSION_2        2
#define TM_MAJ_VERSION_2    2
#define TM_MAJ_VERSION_3    3
#define TM_MAJ_VERSION_4    4
#define TM_MAJ_VERSION_5    5
#define TM_MAJ_VERSION_6    6
#define TM_MAJ_VERSION_7    7

#define TM_MAJ_VERSION      7
#define TM_MIN_VERSION      0


//#ifdef TM_LARGENAMETABLES
//  #define TM_MAJ_VERSION      5
//  #define TM_MIN_VERSION      0
//#elif defined(TOP98_MULTFLAG)
//  #define TM_MAJ_VERSION      4
//  #define TM_MIN_VERSION      0
//#else
//  #define TM_MAJ_VERSION      2
//  #define TM_MIN_VERSION      0
//#endif

//signature structure
typedef struct _TMX_SIGN
{
  CHAR szName[MAX_FILESPEC];
  CHAR szServer[MAX_SERVER_NAME];
  CHAR szUserid[MAX_USERID];
  TIME_L lTime;
  CHAR szSourceLanguage[MAX_LANG_LENGTH];
  BYTE bMajorVersion;
  BYTE bMinorVersion;
  CHAR szDescription[MAX_MEM_DESCRIPTION];
} TMX_SIGN, * PTMX_SIGN;

//table entry structure
typedef struct _TMX_TABLE_ENTRY
{
  CHAR   szName[MAX_LANG_LENGTH];
  USHORT usId;
} TMX_TABLE_ENTRY, * PTMX_TABLE_ENTRY;

  // name table structure (TM Version 1 - 4)
  typedef struct _TMX_VER1_TABLE
  {
    USHORT usAllocSize;
    USHORT usMaxEntries;
    TMX_TABLE_ENTRY stTmTableEntry;
  } TMX_VER1_TABLE, * PTMX_VER1_TABLE;

  // name table structure (TM version 5 and up)
  typedef struct _TMX_TABLE
  {
    ULONG  ulAllocSize;
    ULONG  ulMaxEntries;
    TMX_TABLE_ENTRY stTmTableEntry;
  } TMX_TABLE, * PTMX_TABLE;

/**********************************************************************/
/* Defines and structures for the long document name support          */
/**********************************************************************/

// initial size of long name buffer area
#define LONGNAMEBUFFER_SIZE 8096

// initial number of entries in long document name pointer array
// and increment for table enlargements
#define LONGNAMETABLE_ENTRIES 32

// table entry structure for long document name table
typedef struct _TMX_LONGNAME_TABLE_ENTRY
{
  PSZ    pszLongName;                  // ptr to long name in buffer area
  USHORT usId;                         // ID for long name
} TMX_LONGNAME_TABLE_ENTRY, * PTMX_LONGNAME_TABLE_ENTRY;

//table structure for long document table
typedef struct _TMX_LONGNAME_TABLE
{
  PSZ    pszBuffer;                              // buffer for names and IDs
  ULONG  ulBufUsed;                              // number of bytes used in buffer
  ULONG  ulBufSize;                              // size of buffer in bytes
  ULONG  ulTableSize;                            // table size (# entries)
  ULONG  ulEntries;                              // number of entries in table
  TMX_LONGNAME_TABLE_ENTRY stTableEntry[1];      // dyn. array of table entries
} TMX_LONGNAMETABLE, * PTMX_LONGNAME_TABLE;


// Values for update counter indices
#define RESERVED_UPD_COUNTER     0
#define COMPACTAREA_UPD_COUNTER  1
#define AUTHORS_UPD_COUNTER      2
#define FILENAMES_UPD_COUNTER    3
#define LANGUAGES_UPD_COUNTER    4
#define TAGTABLES_UPD_COUNTER    5
#define LONGNAMES_UPD_COUNTER    6
#define MAX_UPD_COUNTERS        20


//TM control block structure
typedef struct _TMX_CLB
{
  PVOID pstTmBtree;
  PVOID pstInBtree;
  PTMX_TABLE pLanguages;
  PTMX_TABLE pFileNames;
  PTMX_TABLE pAuthors;
  PTMX_TABLE pTagTables;
  USHORT usAccessMode;
  USHORT usThreshold;
  TMX_SIGN stTmSign;
  BYTE     bCompact[MAX_COMPACT_SIZE-1];
  BYTE     bCompactChanged;
  LONG     alUpdCounter[MAX_UPD_COUNTERS];
  BOOL     fShared;
  PTMX_LONGNAME_TABLE pLongNames;
  PTMX_TABLE pLangGroups;              //  table containing language group names
  PSHORT     psLangIdToGroupTable;     // language ID to group ID table
  LONG       lLangIdToGroupTableSize; // size of table (alloc size)
  LONG       lLangIdToGroupTableUsed; // size of table (bytes in use)
  PVOID      pTagTable;               // tag table loaded for segment markup (TBLOADEDTABLE)

  // copy of long name table sorted ignoring the case of the file names
  // Note: only the stTableEntry array is filled in this area, for all other
  //       information use the entries in the pLongNames structure
  PTMX_LONGNAME_TABLE pLongNamesCaseIgnore;

  // fields for work area pointers of various subfunctions which are allocated
  // only once for performance reasons
  PVOID      pvTempMatchList;        // matchlist of FillMatchEntry function
  PVOID      pvIndexRecord;          // index record area of FillMatchEntry function
  PVOID      pvTmRecord;             // buffer for memory record used by GetFuzzy and GetExact
  ULONG      ulRecBufSize;           // current size of pvTMRecord;
  PVOID      pvNotUsed[10];          // room for additional pointers and size values

  // fields for time measurements and logging
  BOOL       fTimeLogging;           // TRUE = Time logging is active
  LONG64     lAllocTime;             // time for memory allocation
  LONG64     lTokenizeTime;          // time for tokenization
  LONG64     lGetExactTime;          // time for GetExact
  LONG64     lOtherTime;             // time for other activities
  LONG64     lGetFuzzyTime;          // time for GetFuzzy
  LONG64     lFuzzyOtherTime;        // other time spent in GetFuzzy
  LONG64     lFuzzyTestTime;         // FuzzyTest time spent in GetFuzzy
  LONG64     lFuzzyGetTime;          // NTMGet time spent in GetFuzzy
  LONG64     lFuzzyFillMatchEntry;   // FillMatchEntry time spent in GetFuzzy
  LONG64     lFillMatchAllocTime;    // FillMatchEntry: allocation time
  LONG64     lFillMatchOtherTime;    // FillMatchEntry: other times
  LONG64     lFillMatchReadTime;     // FillMatchEntry: read index DB time
  LONG64     lFillMatchFillTime;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchCleanupTime;  // FillMatchEntry: cleanup match list time
  LONG64     lFillMatchFill1Time;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchFill2Time;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchFill3Time;     // FillMatchEntry: fill match list time
  LONG64     lFillMatchFill4Time;     // FillMatchEntry: fill match list time
  LONG64     lNotUsed[7];           // room for more counters
} TMX_CLB, * PTMX_CLB;

//complete entry id tm data file
typedef struct _TMX_RECORD
{
  ULONG   ulRecordLen;
  USHORT  usSourceRecord;
  USHORT  usFirstTargetRecord;
} TMX_RECORD, * PTMX_RECORD;

typedef struct _TMX_OLD_RECORD
{
  USHORT  usRecordLen;
  USHORT  usSourceRecord;
  USHORT  usFirstTargetRecord;
} TMX_OLD_RECORD, * PTMX_OLD_RECORD;

//structure of the source segment
typedef struct _TMX_SOURCE_RECORD
{
  ULONG   ulRecordLen;
  USHORT  usSource;
} TMX_SOURCE_RECORD, * PTMX_SOURCE_RECORD;

//structure of the source segment
typedef struct _TMX_OLD_SOURCE_RECORD
{
  USHORT  usRecordLen;
  USHORT  usSource;
} TMX_OLD_SOURCE_RECORD, * PTMX_OLD_SOURCE_RECORD;

//structure of the target segment
typedef struct _TMX_TARGET_RECORD
{
  ULONG   ulRecordLen;
  USHORT  usSourceTagTable;
  USHORT  usTargetTagTable;
  USHORT  usTarget;
  USHORT  usClb;
} TMX_TARGET_RECORD, * PTMX_TARGET_RECORD;

//structure of the target segment
typedef struct _TMX_OLD_TARGET_RECORD
{
  USHORT  usRecordLen;
  USHORT  usSourceTagTable;
  USHORT  usTargetTagTable;
  USHORT  usTarget;
  USHORT  usClb;
} TMX_OLD_TARGET_RECORD, * PTMX_OLD_TARGET_RECORD;

//control block structure in target record
typedef struct _TMX_TARGET_CLB
{
  BYTE    bMultiple;
  USHORT  usLangId;
   BYTE    bTranslationFlag;
  TIME_L  lTime;
  USHORT  usFileId;
  ULONG   ulSegmId;   // changed from USHORT
  USHORT  usAuthorId;
  USHORT  usAddDataLen;  // new for Major_version6: Length of following context and additional info data
} TMX_TARGET_CLB, * PTMX_TARGET_CLB;

// helper macros for working with TMX_TARGET_CLBs
#define TARGETCLBLEN( pClb ) (sizeof(TMX_TARGET_CLB) + pClb->usAddDataLen)

#define NEXTTARGETCLB( pClb ) ((PTMX_TARGET_CLB)(((PBYTE)pClb) + TARGETCLBLEN(pClb)))

#define PCONTEXTFROMCLB( pClb ) ((PSZ_W)(((PBYTE)pClb)+sizeof(TMX_TARGET_CLB)))

// max size of additional data area
// (currently 2 * MAX_SEGMENT_SIZE for context and additional info
// plus size of three identifierrs (USHORT) and two size fields (USHORT) )
#define MAX_ADD_DATA_LEN ((2 * MAX_SEGMENT_SIZE)*sizeof(WCHAR) + (5 * sizeof(USHORT)))

// identifier for additional info data in additional data area
#define ADDDATA_ADDINFO_ID 1

// identifier for context data in additional data area
#define ADDDATA_CONTEXT_ID 2

// identifier for the end of the additional data area
#define ADDDATA_ENDOFDATA_ID 9

// compute the size of the additional data for the given input
USHORT NTMComputeAddDataSize( PSZ_W pszContext, PSZ_W pszAddInfo );

// get length of specific data in the combined data area, returns length of data area
USHORT NtmGetAddDataLen( PTMX_TARGET_CLB pCLB, USHORT usDataID );

// store/combine additional data in the combined area, returns new size of combined data area or 0 in case of errors
USHORT NtmStoreAddData( PTMX_TARGET_CLB pCLB, USHORT usDataID, PSZ_W pszAddData );

// retrieve specific data from the combined data area, returns length of retrieved data (incl. string end delimiter)
USHORT NtmGetAddData( PTMX_TARGET_CLB pCLB, USHORT usDataID, PSZ_W pszBuffer, USHORT usBufSize );

// find a string in a specific data area
BOOL NtmFindInAddData( PTMX_TARGET_CLB pCLB, USHORT usDataID, PSZ_W pszSearch );

typedef struct _TMX_OLD_TARGET_CLB
{
  BYTE    bMultiple;
  USHORT  usLangId;
  BYTE    bMT;
  TIME_L  lTime;
  USHORT  usFileId;
  USHORT  usSegmId;
  USHORT  usAuthorId;
} TMX_OLD_TARGET_CLB, * PTMX_OLD_TARGET_CLB;


//tag table structure for both source and target
typedef struct _TMX_TAGTABLE_RECORD
{
  ULONG   ulRecordLen;
  USHORT  usTagTableId;
  USHORT  usFirstTagEntry;
} TMX_TAGTABLE_RECORD, * PTMX_TAGTABLE_RECORD;

typedef struct _TMX_OLD_TAGTABLE_RECORD
{
  USHORT  usRecordLen;
  USHORT  usTagTableId;
  USHORT  usFirstTagEntry;
} TMX_OLD_TAGTABLE_RECORD, * PTMX_OLD_TAGTABLE_RECORD;

//individual tag entry in tag table record
typedef struct _TMX_TAGENTRY
{
  USHORT  usOffset;
  USHORT  usTagLen;
  BYTE    bData;
} TMX_TAGENTRY, * PTMX_TAGENTRY;

// macros to access certain TM records fields
  #define RECLEN(pRec) pRec->ulRecordLen


#define NTMVOTES(l)     ((BYTE)(((ULONG)(l) >> 24) & 0xFF))
#define NTMKEY(l)       ((ULONG)(l) & 0xFFFFFF)
#define NTMINDEX(b,l)   ((ULONG)(((l) & 0xFFFFFF) | (((ULONG)(b)) << 24 )))

typedef ULONG  TMX_INDEX_ENTRY, *PTMX_INDEX_ENTRY;

typedef struct _TMX_INDEX_RECORD
{
  USHORT  usRecordLen;
  TMX_INDEX_ENTRY  stIndexEntry;
} TMX_INDEX_RECORD, * PTMX_INDEX_RECORD;

typedef struct _TMX_TERM_TOKEN
{
   USHORT usOffset;
   USHORT usLength;
   USHORT usHash;
} TMX_TERM_TOKEN, *PTMX_TERM_TOKEN;

typedef struct _TMX_MATCHENTRY
{
  ULONG ulKey;
  USHORT usMaxVotes;
  USHORT usMatchVotes;
  BYTE cCount;
} TMX_MATCHENTRY, * PTMX_MATCHENTRY;

typedef struct _TMX_SENTENCE
{
  PSZ_W pInputString;
  PSZ_W pNormString;
  PSZ_W pNormStringStart;
  USHORT  usNormLen;
  PTMX_TERM_TOKEN pTermTokens;
  LONG lTermAlloc;
  PTMX_TAGTABLE_RECORD pTagRecord;
  LONG lTagAlloc;
  PTMX_TAGENTRY pTagEntryList;
  USHORT  usActVote;
  PULONG  pulVotes;
  PSZ_W pPropString;                   // proposal string (with tagging)
  PTMX_TERM_TOKEN pPropTermTokens;     // buffer for Termtokens
} TMX_SENTENCE, * PTMX_SENTENCE;

typedef struct _TMX_SENTENCE_V5
{
  PSZ pInputString;
  PSZ pNormString;
  PSZ pNormStringStart;
  USHORT  usNormLen;
  PTMX_TERM_TOKEN pTermTokens;
  LONG lTermAlloc;
  PTMX_TAGTABLE_RECORD pTagRecord;
  LONG lTagAlloc;
  PTMX_TAGENTRY pTagEntryList;
  USHORT  usActVote;
  PULONG  pulVotes;
  PSZ_W pPropString;                   // proposal string (with tagging)
  PTMX_TERM_TOKEN pPropTermTokens;     // buffer for Termtokens
} TMX_SENTENCE_V5, * PTMX_SENTENCE_V5           ;


typedef struct _TMX_REPLTAGPAIR
{
  PBYTE     pSrcTok;           // token for paired toks
  PBYTE     pPropTok;
  BOOL      fUsed;             //TRUE = Tag pair has been used in replacement already
} TMX_REPLTAGPAIR, * PTMX_REPLTAGPAIR;

typedef struct _TMX_SUBSTPROP
{
  CHAR_W szSource    [ MAX_SEGMENT_SIZE ]; // source segment
  CHAR_W szPropSource[ MAX_SEGMENT_SIZE ]; // source proposal
  CHAR_W szPropTarget[ MAX_SEGMENT_SIZE ]; // translation of proposal
  char szSourceLanguage[MAX_LANG_LENGTH];// source language
  char szSourceTagTable[MAX_FNAME];      // source tagtable
  char szTargetLanguage[MAX_LANG_LENGTH];// target language
  char szPropTagTable[MAX_FNAME];        // tagtable for proposal
  PBYTE                pTokSource;       // token buffer for source tokens
  PBYTE                pTokPropSource;   // token buffer for proposal tokens
  PBYTE                pTokPropTarget;   // token buffer for proposal target ..
  PTMX_TAGTABLE_RECORD pTagsSource;      // tag table record for source  ..
  PTMX_TAGTABLE_RECORD pTagsPropSource;  // tag table record for prop source
  PTMX_TAGTABLE_RECORD pTagsPropTarget;  // tag table record for prop target
  CHAR_W chBuffer[ MAX_SEGMENT_SIZE * 2 ]; // generic token buffer
  USHORT usTokenSource;                  // number of source tokens
  USHORT usTokenPropSource;              // number of prop source tokens
  USHORT usTokenPropTarget;              // number of prop target tokens
  ULONG  ulRandom[MAX_RANDOM];                   // random sequence number
  PTMX_REPLTAGPAIR pTagPairs;                    // tags to be replaced by each other
  PTMX_REPLTAGPAIR pDelTagPairs;                 // tags to be deleted in prop
} TMX_SUBSTPROP, * PTMX_SUBSTPROP;



//=======================================================================
typedef struct _TMX_PREFIX_IN
{
  USHORT usLengthInput;                //length of complete input structure
  USHORT usTmCommand;                  //TM command id
} TMX_PREFIX_IN, * PTMX_PREFIX_IN, XIN, * PXIN;

//prefix for each output structures
typedef struct _TMX_PREFIX_OUT
{
  USHORT usLengthOutput;               //length of complete output structure
  USHORT usTmtXRc;                     //function returncode
} TMX_PREFIX_OUT, * PTMX_PREFIX_OUT, XOUT, * PXOUT;

//======================================================
typedef struct _TMX_CREATE
{
  CHAR szDataName[MAX_EQF_PATH];
  CHAR szIndexName[MAX_EQF_PATH];
  CHAR szServer[MAX_SERVER_NAME];
  CHAR szUserid[MAX_USERID];
  CHAR szSourceLanguage[MAX_LANG_LENGTH];
  USHORT usThreshold;
  CHAR szDescription[MAX_DESCRIPTION];
} TMX_CREATE, * PTMX_CREATE;

typedef struct _TMX_CREATE_IN
{
  TMX_PREFIX_IN  stPrefixIn;
  TMX_CREATE stTmCreate;
} TMX_CREATE_IN, * PTMX_CREATE_IN;

typedef struct _TMX_CREATE_OUT
{
  TMX_PREFIX_OUT  stPrefixOut;
  PTMX_CLB pstTmClb;
} TMX_CREATE_OUT, * PTMX_CREATE_OUT;

//=======================================================================
typedef struct _TMX_OPEN
{
  CHAR szDataName[MAX_EQF_PATH];
  CHAR szIndexName[MAX_EQF_PATH];
  CHAR szServer[MAX_SERVER_NAME];
  CHAR szUserid[MAX_USERID];
  USHORT usAccess;
  USHORT usThreshold;
} TMX_OPEN, * PTMX_OPEN;

typedef struct _TMX_OPEN_IN
{
  TMX_PREFIX_IN stPrefixIn;
  TMX_OPEN stTmOpen;
} TMX_OPEN_IN, * PTMX_OPEN_IN;

typedef struct _TMX_OPEN_OUT
{
  TMX_PREFIX_OUT stPrefixOut;
  PTMX_CLB pstTmClb;
} TMX_OPEN_OUT, * PTMX_OPEN_OUT;

//=======================================================================
typedef struct _TMX_CLOSE_IN
{
  TMX_PREFIX_IN stPrefixIn;
} TMX_CLOSE_IN, * PTMX_CLOSE_IN;

typedef struct _TMX_CLOSE_OUT
{
  TMX_PREFIX_OUT stPrefixOut;
} TMX_CLOSE_OUT, * PTMX_CLOSE_OUT;

//=======================================================================
typedef struct _TMX_PUT
{
  CHAR      szSource[MAX_SEGMENT_SIZE];        //source sentence
  CHAR      szTarget[MAX_SEGMENT_SIZE];        //target sentence
  CHAR      szSourceLanguage[MAX_LANG_LENGTH]; //language name of source
  CHAR      szTargetLanguage[MAX_LANG_LENGTH]; //language name of target
  CHAR      szAuthorName[MAX_USERID];          //author name of target
  USHORT    usTranslationFlag;                 /* type of translation, 0 = human, 1 = machine, 2 = GobalMemory */
  CHAR      szFileName[MAX_FILESPEC];          //where source comes from name+ext
  LONG_FN   szLongName;                        // name of source file (long name or EOS)
  USHORT    usSourceSegmentId;                 //seg. num. of source sentence from analysis
  CHAR      szTagTable[MAX_FNAME];             //tag table name
  TIME_L    lTime;                             //time stamp
} TMX_PUT, * PTMX_PUT;


typedef struct _TMX_PUT_W
{
  CHAR_W    szSource[MAX_SEGMENT_SIZE];        //source sentence
  CHAR_W    szTarget[MAX_SEGMENT_SIZE];        //target sentence
  CHAR      szSourceLanguage[MAX_LANG_LENGTH]; //language name of source
  CHAR      szTargetLanguage[MAX_LANG_LENGTH]; //language name of target
  CHAR      szAuthorName[MAX_USERID];          //author name of target
  USHORT    usTranslationFlag;                 /* type of translation, 0 = human, 1 = machine, 2 = GobalMemory */
  CHAR      szFileName[MAX_FILESPEC];          //where source comes from name+ext
  LONG_FN   szLongName;                        // name of source file (long name or EOS)
  ULONG     ulSourceSegmentId;                 //seg. num. of source sentence from analysis
  CHAR      szTagTable[MAX_FNAME];             //tag table name
  TIME_L    lTime;                             //time stamp
  CHAR_W    szContext[MAX_SEGMENT_SIZE];       //segment context
  CHAR_W    szAddInfo[MAX_SEGMENT_SIZE];       // additional segment information
  BOOL      fMarkupChanged;                    // Markup does not exist, changed to OTMUTF8 during import
} TMX_PUT_W, * PTMX_PUT_W;


typedef struct _TMX_PUT_IN
{
  TMX_PREFIX_IN  stPrefixIn;           //input prefix
  TMX_PUT        stTmPut;             //pointer to put input structure
} TMX_PUT_IN, * PTMX_PUT_IN;

typedef struct _TMX_PUT_IN_W
{
  TMX_PREFIX_IN  stPrefixIn;           //input prefix
  TMX_PUT_W      stTmPut;             //pointer to put input structure
} TMX_PUT_IN_W, * PTMX_PUT_IN_W;


//structure TMX_PUT_OUT
typedef struct _TMX_PUT_OUT
{
  TMX_PREFIX_OUT stPrefixOut;           //output structure
} TMX_PUT_OUT, * PTMX_PUT_OUT;

typedef struct _TMX_PUT_OUT_W
{
  TMX_PREFIX_OUT stPrefixOut;           //output structure
} TMX_PUT_OUT_W, * PTMX_PUT_OUT_W;


//=======================================================================
//structure TMX_GET_IN and _TMX_GET
typedef struct _TMX_GET
{
  CHAR        szSource[MAX_SEGMENT_SIZE];        //source sentence
  CHAR        szTagTable[MAX_FNAME];             //tag table name of source
  CHAR        szSourceLanguage[MAX_LANG_LENGTH]; //language of source
  CHAR        szFileName[MAX_FILESPEC];          //file name the source comes from
  LONG_FN     szLongName;                        // name of source file (long name or EOS)
  USHORT      usSegmentId;                       //segment number from analysis
  CHAR        szAuthorName[MAX_USERID];          //author name
  CHAR        szTargetLanguage[MAX_LANG_LENGTH]; //language of translation
  USHORT      usRequestedMatches;                //number of requested matches
  USHORT      usMatchThreshold;                  //threshold for match level
  USHORT      usConvert;                //how the output should be converted
  USHORT      usParm;                   //for future use, xmp. GET_ONLY_MT_MATCHES, GET_ONLY_EXACT_MATCHES...)
} TMX_GET, * PTMX_GET;

typedef struct _TMX_GET_W
{
  CHAR_W      szSource[MAX_SEGMENT_SIZE];        //source sentence
  CHAR        szTagTable[MAX_FNAME];             //tag table name of source
  CHAR        szSourceLanguage[MAX_LANG_LENGTH]; //language of source
  CHAR        szFileName[MAX_FILESPEC];          //file name the source comes from
  LONG_FN     szLongName;                        // name of source file (long name or EOS)
  ULONG       ulSegmentId;                       //segment number from analysis
  CHAR        szAuthorName[MAX_USERID];          //author name
  CHAR        szTargetLanguage[MAX_LANG_LENGTH]; //language of translation
  USHORT      usRequestedMatches;                //number of requested matches
  USHORT      usMatchThreshold;                  //threshold for match level
  USHORT      usConvert;                //how the output should be converted
  ULONG       ulParm;                   //for future use, xmp. GET_ONLY_MT_MATCHES, GET_ONLY_EXACT_MATCHES...)
  CHAR_W      szContext[MAX_SEGMENT_SIZE];       //segment context
  CHAR_W      szAddInfo[MAX_SEGMENT_SIZE];       // additional segment information
  ULONG       ulSrcOemCP;
  ULONG       ulTgtOemCP;
  PVOID       pvReplacementList;                 // ptr to a SGML-DITA replacement list or NULL
  PVOID       pvGMOptList;                       // ptr to a global memory option list or NULL
} TMX_GET_W, * PTMX_GET_W;

typedef struct _TMX_GET_IN
{
  TMX_PREFIX_IN  stPrefixIn;           //prefix of each command
  TMX_GET        stTmGet;             //pointer in structure
} TMX_GET_IN, * PTMX_GET_IN;

typedef struct _TMX_GET_IN_W
{
  TMX_PREFIX_IN  stPrefixIn;           //prefix of each command
  TMX_GET_W        stTmGet;             //pointer in structure
} TMX_GET_IN_W, * PTMX_GET_IN_W;


//=======================================================================
// structure TMX_GET_OUT
typedef struct _TMX_MATCH_TABLE
{
  CHAR   szSource[MAX_SEGMENT_SIZE];   //source sentence with tags
  CHAR   szFileName[MAX_FILESPEC];     //file name where the source comes from.
  LONG_FN szLongName;                  // name of source file (long name or EOS)
  USHORT usSegmentId;                  //segment number from analysis
  CHAR   szTarget[MAX_SEGMENT_SIZE];   //target sentence with tags
  CHAR   szTargetLanguage[MAX_LANG_LENGTH]; //language of translation
  USHORT    usTranslationFlag;                 /* type of translation, 0 = human, 1 = machine, 2 = GobalMemory */
  CHAR   szTargetAuthor[MAX_USERID];   //author name of target
  TIME_L lTargetTime;                  //time stamp of target
  USHORT usMatchLevel;                 //similarity of the source
  USHORT usOverlaps;                   //temp field - nr of overlapping triples
  CHAR   szTagTable[MAX_FNAME];        //tag table name of source
  ULONG  ulKey;                        // key of match
  USHORT usTargetNum;                  // number of target
  USHORT usDBIndex;                    // number of memory in current hierarchy
} TMX_MATCH_TABLE, * PTMX_MATCH_TABLE;

typedef struct _TMX_MATCH_TABLE_W
{
  CHAR_W  szSource[MAX_SEGMENT_SIZE];   //source sentence with tags
  CHAR    szFileName[MAX_FILESPEC];     //file name where the source comes from.
  LONG_FN szLongName;                   // name of source file (long name or EOS)
  ULONG   ulSegmentId;                  //segment number from analysis
  CHAR_W  szTarget[MAX_SEGMENT_SIZE];   //target sentence with tags
  CHAR    szTargetLanguage[MAX_LANG_LENGTH]; //language of translation
  USHORT    usTranslationFlag;                 /* type of translation, 0 = human, 1 = machine, 2 = GobalMemory */
  CHAR    szTargetAuthor[MAX_USERID];   //author name of target
  TIME_L  lTargetTime;                  //time stamp of target
  USHORT  usMatchLevel;                 //similarity of the source
  USHORT  usOverlaps;                   //temp field - nr of overlapping triples
  CHAR    szTagTable[MAX_FNAME];        //tag table name of source
  ULONG   ulKey;                        // key of match
  USHORT  usTargetNum;                  // number of target
  USHORT  usDBIndex;                    // number of memory in current hierarchy
  CHAR_W  szContext[MAX_SEGMENT_SIZE];  // segment context info
  CHAR_W  szAddInfo[MAX_SEGMENT_SIZE];  // additional segment information
  USHORT  usContextRanking;             // context ranking from user exit context processing
  USHORT  usMatchInfo;                  // for future use: special info for match
} TMX_MATCH_TABLE_W, * PTMX_MATCH_TABLE_W;

typedef struct _TMX_GET_OUT
{
  TMX_PREFIX_OUT stPrefixOut;         //prefix of Output buffer
  USHORT usNumMatchesFound;           //number of matches found
  TMX_MATCH_TABLE stMatchTable[MAX_MATCHES];     //match structure
  USHORT fsAvailFlags;                // additional flags (more exact/fuzzy matches avail)
} TMX_GET_OUT, * PTMX_GET_OUT;

typedef struct _TMX_GET_OUT_W
{
  TMX_PREFIX_OUT    stPrefixOut;               //prefix of Output buffer
  USHORT            usNumMatchesFound;         //number of matches found
  TMX_MATCH_TABLE_W stMatchTable[MAX_MATCHES]; //match structure
  USHORT fsAvailFlags;                         // additional flags (more exact/fuzzy matches avail)
} TMX_GET_OUT_W, * PTMX_GET_OUT_W;






//=======================================================================
/**********************************************************************/
/* Attention: for special names mode of TMExtract, the TMX_EXT_IN     */
/*            structure is used in a different way:                   */
/*            usConvert contains the ID of the names to extract:      */
/*              MEM_OUTPUT_TAGTABLES  return list of tag tables of TM */
/*              MEM_OUTPUT_AUTHORS    return list of authors of TM    */
/*              MEM_OUTPUT_DOCUMENTS  return list of documents of TM  */
/*              MEM_OUTPUT_LANGUAGES  return list of languages of TM  */
/*            usNextTarget contains 0 for the first call or the       */
/*            index returned in the usNextTarget field of the         */
/*            TMX_EXT_OUT structure for subsequent calls if more      */
/*            names are available than fit into the output buffers.   */
/*            The remining fields are unused.                         */
/**********************************************************************/
//structure TMX_EXT_IN
typedef struct _TMX_EXT_IN
{
  TMX_PREFIX_IN stPrefixIn;      //prefix of input buffer
  ULONG ulTmKey;                 //tm get to get
  USHORT usConvert;              //how the output is to appear
  USHORT usNextTarget;           //which target record to address next
} TMX_EXT_IN, * PTMX_EXT_IN;
typedef TMX_EXT_IN TMX_EXT_IN_W, *PTMX_EXT_IN_W;
//=======================================================================
/**********************************************************************/
/* Attention: for special names mode of TMExtract, the TMX_EXT_OUT    */
/*            structure is used in a different way:                   */
/*            szSource and szTarget contain a null terminated list    */
/*            of names (NULL-terminated strings followed by another   */
/*            NULL).                                                  */
/*            fMachineTranslation is TRUE if more names are available */
/*            usNextTarget gives the index of the next name if more   */
/*            names are available than fit into the output buffers    */
/*            The remaining fields are unused.                        */
/**********************************************************************/
// structure TMX_EXT_OUT
typedef struct _TMX_EXT
{
  CHAR      szSource[MAX_SEGMENT_SIZE];        //source sentence
  CHAR      szTarget[MAX_SEGMENT_SIZE];        //target sentence
  CHAR      szTagTable[MAX_FNAME];             //tag table name
  CHAR      szTargetLanguage[MAX_LANG_LENGTH]; //language name of target
  CHAR      szAuthorName[MAX_USERID];          //author name of target
  USHORT  usTranslationFlag; /* type of translation, 0 = human, 1 = machine, 2 = Global Memory */
  CHAR      szFileName[MAX_FILESPEC];          //where source comes from name+ext
  LONG_FN   szLongName;                        // name of source file (long name or EOS)
  USHORT    usSourceSegmentId;                 //seg. num. of source sentence from analysis
  TIME_L    lTargetTime;                       //time stamp of target
} TMX_EXT, * PTMX_EXT;


typedef struct _TMX_EXT_W
{
  CHAR_W    szSource[MAX_SEGMENT_SIZE];        //source sentence
  CHAR_W    szTarget[MAX_SEGMENT_SIZE];        //target sentence
  CHAR      szTagTable[MAX_FNAME];             //tag table name
  CHAR      szTargetLanguage[MAX_LANG_LENGTH]; //language name of target
  CHAR      szAuthorName[MAX_USERID];          //author name of target
  USHORT  usTranslationFlag; /* type of translation, 0 = human, 1 = machine, 2 = Global Memory */
  CHAR      szFileName[MAX_FILESPEC];          //where source comes from name+ext
  LONG_FN   szLongName;                        // name of source file (long name or EOS)
  ULONG     ulSourceSegmentId;                 //seg. num. of source sentence from analysis
  TIME_L    lTargetTime;                       //time stamp of target
  CHAR_W    szContext[MAX_SEGMENT_SIZE];       //segment context
  CHAR_W    szAddInfo[MAX_SEGMENT_SIZE];       // additional segment information
} TMX_EXT_W, * PTMX_EXT_W;

typedef struct _TMX_EXT_OUT
{
  TMX_PREFIX_OUT stPrefixOut;      //prefix of output buffer
  TMX_EXT stTmExt;                 //pointer to put structure
  CHAR szServer[MAX_SERVER_NAME];  //server name filled by u-code function
  ULONG ulTmKey;                   //tm record key
  USHORT usNextTarget;             //which target record to address next
  ULONG ulMaxEntries;              //number of entries in tm data file
} TMX_EXT_OUT, * PTMX_EXT_OUT;

typedef struct _TMX_EXT_OUT_W
{
  TMX_PREFIX_OUT stPrefixOut;      //prefix of output buffer
  TMX_EXT_W      stTmExt;          //pointer to put structure
  CHAR szServer[MAX_SERVER_NAME];  //server name filled by u-code function
  ULONG ulTmKey;                   //tm record key
  USHORT usNextTarget;             //which target record to address next
  ULONG ulMaxEntries;              //number of entries in tm data file
} TMX_EXT_OUT_W, *PTMX_EXT_OUT_W;


//=======================================================================

/**********************************************************************/
/* TMX_INFO_IN, TMX_INFO_OUT used by TmInfo                           */
/**********************************************************************/
typedef struct  _TMX_INFO_IN
{
  TMX_PREFIX_IN stPrefixIn;
  USHORT        usInfoLevel;
} TMX_INFO_IN, * PTMX_INFO_IN;

typedef struct _TMX_INFO_OUT
{
  TMX_PREFIX_OUT stPrefixOut;
  TMX_SIGN       stTmSign;
  USHORT         usThreshold;
} TMX_INFO_OUT , * PTMX_INFO_OUT;

/**********************************************************************/
/* _TMX_DELTM_IN and _TMX_DELTM_IN  used bt TmDeleteTM                */
/**********************************************************************/
typedef struct _TMX_DELTM_IN
{
  TMX_PREFIX_IN stPrefixIn;
  CHAR         szFullTmName[MAX_EQF_PATH];
  CHAR         szFullIndexName[MAX_EQF_PATH];
  CHAR         szFullPropName[MAX_EQF_PATH];
} TMX_DELTM_IN, * PTMX_DELTM_IN;

typedef struct _TMXDELTM_OUT
{
  TMX_PREFIX_OUT stPrefixOut;
} TMX_DELTM_OUT, * PTMX_DELTM_OUT;

/**********************************************************************/
/* TM property structure                                              */
/**********************************************************************/
typedef struct _PROP_NTM
{
  PROPHEAD stPropHead;
  TMX_SIGN stTMSignature;
  CHAR     szNTMMarker[sizeof(NTM_MARKER)];
  CHAR     szFullMemName[MAX_EQF_PATH];
  USHORT   usThreshold;
  USHORT   usLocation;
  CHAR     szLongName[MAX_LONGFILESPEC];
} PROP_NTM, *PPROP_NTM;

// last used values
typedef struct _MEM_LAST_USED
{
  // Do only insert new values at the end of the stucture !!!!!!!!!!
  PROPHEAD stPropHead;                        // Common header of properties
  CHAR     szFormat[MAX_FILESPEC];            // Create dialog: last used format table
  CHAR     szExclusion[MAX_FILESPEC];         // Create dialog: last used exclusion list
  LANGUAGE szSourceLang;                      // Create dialog: last used source language
  LANGUAGE szTargetLang;                      // Create dialog: last used target language
  CHAR     szDrive[MAX_FILESPEC];             // Create dialog: last used local disk drive
  CHAR     szExpDriveLastUsed[MAX_DRIVE];     // Export dialog: last used drive format: ~x:~
  CHAR     szExpDirLastUsed[_MAX_DIR];        // Export dialog: last used directory format: ~\xxx\yyy\~
  // Structure end in Troja 2.1
  CHAR     szIncludeServer[MAX_SERVER_NAME];  // Include dialog: Last used server ~SSER0545~
  CHAR     szCreateServer[MAX_SERVER_NAME];   // Create dialog: Last used server ~SSER0545~
  BOOL     usCreateLocation;                  // Create dialog: Possible values: TM_LOCAL, TM_REMOTE
  CHAR     szRemoteDrive[MAX_FILESPEC];       // Create dialog: last used remote disk drive
  // Structure end in Troja 3.0
  CHAR     szImpPathLastUsed[MAX_PATH144];    // Import dlg: last used path
                                              // x:\xx
  // Structure end in Troja 3.c
  USHORT   usExpMode;                         // last used export mode
  USHORT   usImpMode;                         // last used import mode
  CHAR     szImpPathLastUsed2[MAX_LONGPATH];  // Import dlg: last used path (long)
}MEM_LAST_USED, * PMEM_LAST_USED;




USHORT  TmGetServerDrives( PDRIVES_IN  pDrivesIn,      // Pointer to DRIVES_IN struct
                           PDRIVES_OUT pDrivesOut,     // Pointer to DRIVES_OUT struct
                           USHORT      usMsgHandling );// Message handling parameter
USHORT  TmGetServerDrivesHwnd( PDRIVES_IN, PDRIVES_OUT, USHORT, HWND );
USHORT  TmGetServerTMs( PFILE_LIST_IN  pTmListIn,      // Pointer to FILE_LIST_IN struct !!! CHM
                        PFILE_LIST_OUT pTmListOut,     // Pointer to FILE_LIST_OUT struct !!! CHM
                        USHORT       usMsgHandling );  // Message handling parameter
USHORT  TmGetTMPart( HTM          htm,                 // Memory database handle
                     PSZ          pszMemPat,           // Full translation memory path
                     PGETPART_IN  pGetPartIn,          // Pointer to GETPART_IN struct
                     PGETPART_OUT pGetPartOut,         // Pointer to GETPART_OUT struct
                     USHORT       usMsgHandling );     // Message handling parameter
USHORT  TmSendTMProp( SERVERNAME szServer,             // Servername
                      PSZ        pszMemName,           // Translation memory name
                      USHORT     usMsgHandling );      // Message handling parameter
USHORT  TmReceiveTMProp( SERVERNAME szServer,          // Servername
                         PSZ        pszMemName,        // Translation memory name
                         USHORT     usMsgHandling );       // Message handling parameter

//use this define as usMsgHandling for special handling of greyedout TMs
#define DELETE_GREYEDOUT 2

USHORT  TmDeleteFile( SERVERNAME szServer,             // Servername
                      PSZ        pszFilePath,          // Full file path
                      USHORT     usMsgHandling );      // Message handling parameter
USHORT TmGetServerFileInfo( SERVERNAME   szServer,        //Servername
                            PSZ          pszFilePath,     //Full file path
                            PFILEFINDBUF pstFile,        // File info
                            USHORT       usMsgHandling ); // Message handling parameter
USHORT TmGetServerFileInfoHwnd( SERVERNAME, PSZ, PFILEFINDBUF, USHORT, HWND );
USHORT TmCompareLocalRemoteProperties( SERVERNAME, PSZ, USHORT );
__declspec(dllexport)
USHORT MemRcHandling( USHORT,  PSZ, HTM *, PSZ );
USHORT MemRcHandlingHwnd( USHORT,  PSZ, HTM *, PSZ, HWND );

/**********************************************************************/
/* MemReadWriteSegment                                                */
/**********************************************************************/
USHORT
MemReadWriteSegment( HTM,
                     PSZ,
                     HTM,
                     PSZ,
                     PEXT_IN,
                     PEXT_OUT,
                     PTMX_PUT_IN,
                     PTMX_PUT_OUT,
                     ULONG *,
                     ULONG *,
                     TIME_L,
                     PSZ,
                     PSZ,
                     PSZ,
                     BOOL );
USHORT
MemReadWriteSegmentHwnd( HTM,
                     PSZ,
                     HTM,
                     PSZ,
                     PEXT_IN,
                     PEXT_OUT,
                     PTMX_PUT_IN,
                     PTMX_PUT_OUT,
                     ULONG *,
                     ULONG *,
                     TIME_L,
                     PSZ,
                     PSZ,
                     PSZ,
                     BOOL,
                     HWND );


/**********************************************************************/
/* NTMReadWriteSegment                                                */
/**********************************************************************/
// message handling flags for NTMReadWriteSegment :
//   TMRWS_NOMSG:    show no error message at all
//   TMRWS_ALLMSG:   show all error messages
//   TMRWS_WRITEMSG: show only error messages for write segment (for TM organize!)
#define TMRWS_NOMSG    0
#define TMRWS_ALLMSG   1
#define TMRWS_WRITEMSG 2

USHORT NTMReadWriteSegmentW( HTM,          //Handle of Input TM
                             PSZ,          //Full path to Input TM
                             HTM,          //Handle of Output TM
                             PSZ,          //Full path to Output TM
                             PTMX_EXT_IN_W,//Pointer to the EXTRACT_IN structure
                             PTMX_EXT_OUT_W, //Pointer to the EXTRACT_OUT structure
                             PTMX_PUT_IN_W,  //Pointer to the REPLACE_OUT structure
                             PTMX_PUT_OUT_W, //Pointer to the REPLACE_OUT structure
                             ULONG *,      //Pointer to a segment counter
                             ULONG *,      //Pointer to the invalid segment counter
                             PSZ,          //pointer to TM source language
                             USHORT );     //message handling flags
USHORT NTMReadWriteSegmentHwndW( HTM,      //Handle of Input TM
                             PSZ,          //Full path to Input TM
                             HTM,          //Handle of Output TM
                             PSZ,          //Full path to Output TM
                             PTMX_EXT_IN_W,  //Pointer to the EXTRACT_IN structure
                             PTMX_EXT_OUT_W, //Pointer to the EXTRACT_OUT structure
                             PTMX_PUT_IN_W,  //Pointer to the REPLACE_OUT structure
                             PTMX_PUT_OUT_W, //Pointer to the REPLACE_OUT structure
                             ULONG *,      //Pointer to a segment counter
                             ULONG *,      //Pointer to the invalid segment counter
                             PSZ,          //pointer to TM source language
                             USHORT,       //message handling flags
                             HWND );       //handle for error messages parent



__declspec(dllexport)
BOOL fMemIsAvail( PSZ, PSZ, PSZ );      // memory is available and accessible
__declspec(dllexport)
USHORT MemFillCBNames( HWND, PSZ, PSZ );

USHORT MemFillTableLB( HWND   hListBox,
                       USHORT usBoxType,
                       PSZ    pszLastUsed );

/**********************************************************************/
/* TmCreate                                                           */
/**********************************************************************/
USHORT TmCreate( PSZ, HTM *, HTM, PSZ, PSZ, PSZ, PSZ, USHORT, HWND );

/**********************************************************************/
/* TmOpen                                                             */
/**********************************************************************/
__declspec(dllexport)
USHORT
TmOpen( PSZ,       //(in)  full TM name x:\eqf\mem\mem.mem
        HTM *,     //(out) TM handle
        USHORT,    //(in)  access mode: NON_EXCLUSIVE
                   //                   EXCLUSIVE
        USHORT,    //(in)  location:    TM_LOCAL
                   //                   TM_REMOTE
                   //                   TM_LOCAL_REMOTE
        USHORT,    //(in)  message handling parameter
                   //      TRUE:  display error message
                   //      FALSE: display no error message
        HWND   );  //(in)  window handle for error messages

/**********************************************************************/
/* TmClose                                                            */
/**********************************************************************/
__declspec(dllexport)
USHORT
TmClose( HTM,        //(in) TM handle returned from open
         PSZ,        //(in) full TM name x:\eqf\mem\mem.mem
         USHORT,     //(in) message handling parameter
                     //     TRUE:  display error message
                     //     FALSE: display no error message
         HWND );     //(in) window handle for error messages
/**********************************************************************/
/* TmReplace                                                          */
/**********************************************************************/
USHORT
TmReplace( HTM,           //(in)  TM handle
           PSZ,           //(in)  full TM name x:\eqf\mem\mem.mem
           PTMX_PUT_IN,   //(in)  pointer to put input structure
           PTMX_PUT_OUT,  //(out) pointer to put output structure
           USHORT );      //(in)  message handling parameter
                          //      TRUE:  display error message
                          //      FALSE: display no error message
__declspec(dllexport)
USHORT
TmReplaceW( HTM, PSZ, PTMX_PUT_IN_W, PTMX_PUT_OUT_W, USHORT );

USHORT
TmReplaceHwnd( HTM,           //(in)  TM handle
           PSZ,           //(in)  full TM name x:\eqf\mem\mem.mem
           PTMX_PUT_IN,   //(in)  pointer to put input structure
           PTMX_PUT_OUT,  //(out) pointer to put output structure
           USHORT,        //(in)  message handling parameter
                          //      TRUE:  display error message
                          //      FALSE: display no error message
           HWND );                     //(in) handle for error messages

USHORT
TmReplaceHwndW( HTM, PSZ, PTMX_PUT_IN_W, PTMX_PUT_OUT_W, USHORT, HWND );

/**********************************************************************/
/* TmGet                                                              */
/**********************************************************************/
USHORT
TmGetW(HTM,               //(in)  TM handle
       PSZ,               //(in)  full TM name x:\eqf\mem\mem.tmd
       PTMX_GET_IN_W,     //(in)  pointer to get input structure
       PTMX_GET_OUT_W,    //(out) pointer to get output structure
       USHORT );          //(in)  message handling parameter
                          //      TRUE:  display error message
                          //      FALSE: display no error message

USHORT
TmGet( HTM,               //(in)  TM handle
       PSZ,               //(in)  full TM name x:\eqf\mem\mem.tmd
       PTMX_GET_IN,       //(in)  pointer to get input structure
       PTMX_GET_OUT,      //(out) pointer to get output structure
       USHORT );          //(in)  message handling parameter
                          //      TRUE:  display error message
                          //      FALSE: display no error message
/**********************************************************************/
/* TmExtract                                                          */
/**********************************************************************/
USHORT
TmExtract( HTM,
           PSZ,
           PTMX_EXT_IN,
           PTMX_EXT_OUT,
           USHORT );

USHORT
TmExtractW( HTM, PSZ, PTMX_EXT_IN_W, PTMX_EXT_OUT_W, USHORT );


USHORT
TmExtractHwnd( HTM,
           PSZ,
           PTMX_EXT_IN,
           PTMX_EXT_OUT,
           USHORT,
           HWND );                     //(in) handle for error messages

USHORT
TmExtractHwndW( HTM, PSZ, PTMX_EXT_IN_W, PTMX_EXT_OUT_W, USHORT, HWND );

/**********************************************************************/
/* information levels for TmInfo function call                        */
/* TM_INFO_SIGNATURE - returns the signature record of a TM           */
/**********************************************************************/
#define TMINFO_SIGNATURE 0

/**********************************************************************/
/* TmInfo                                                             */
/**********************************************************************/
USHORT TmInfo( HTM, PSZ, USHORT, PTMX_INFO_IN, PTMX_INFO_OUT, USHORT );
USHORT TmInfoHwnd( HTM, PSZ, USHORT, PTMX_INFO_IN, PTMX_INFO_OUT, USHORT, HWND );

/**********************************************************************/
/* TmDeleteTM                                                         */
/**********************************************************************/
USHORT TmDeleteTM( PSZ, USHORT, HWND, PUSHORT );

/**********************************************************************/
/* TmDeleteSegment                                                    */
/**********************************************************************/
USHORT
TmDeleteSegment( HTM,
                 PSZ,
                 PTMX_PUT_IN,
                 PTMX_PUT_OUT,
                 USHORT );

USHORT
TmDeleteSegmentW( HTM, PSZ, PTMX_PUT_IN_W, PTMX_PUT_OUT_W, USHORT );



/**********************************************************************/
/* NTMConvertCRLF                                                     */
/**********************************************************************/
VOID
NTMConvertCRLF( PSZ,
                PSZ,
                USHORT );
VOID NTMConvertCRLFW( PSZ_W, PSZ_W, USHORT );


/**********************************************************************/
/* NTMOpenProperties                                                  */
/**********************************************************************/
USHORT
NTMOpenProperties( HPROP *,
                   PVOID *,
                   PSZ,
                   PSZ,
                   USHORT,
                   BOOL );

// functions dealing with long document tables
USHORT NTMCreateLongNameTable( PTMX_CLB pTmClb );
USHORT NTMReadLongNameTable( PTMX_CLB pTmClb );
USHORT NTMWriteLongNameTable( PTMX_CLB pTmClb );
USHORT NTMDestroyLongNameTable( PTMX_CLB pTmClb );
USHORT NTMCreateLangGroupTable( PTMX_CLB pTmClb );
USHORT NTMAddLangGroup( PTMX_CLB pTmClb, PSZ pszLang, USHORT sLangID );
USHORT NTMOrganizeIndexFile
(
  PTMX_CLB pTmClb               // ptr to control block,
);



// Update specific parts of segment

// flags controlling the updated segment parts
#define  TMUPDSEG_MARKUP       0x01    // update markup/tag table
#define  TMUPDSEG_MTFLAG       0x02    // update MT flag
#define  TMUPDSEG_TARGLANG     0x04    // update target language
#define  TMUPDSEG_DATE         0x08    // update segment time

USHORT TmUpdSeg
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling              //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
);

USHORT TmUpdSegHwnd
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling,             //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
  HWND        hwnd                       //(in)  handle for error messages
);

USHORT TmUpdSegW
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN_W pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling              //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
);

USHORT TmUpdSegHwndW
(
  HTM         htm,                       //(in)  TM handle
  PSZ         szMemPath,                 //(in)  full TM name x:\eqf\mem\mem
  PTMX_PUT_IN_W pstPutIn,                  //(in)  pointer to put input structure
  ULONG       ulUpdKey,                  //(in)  key of record being updated
  USHORT      usUpdTarget,               //(in)  number of target being updated
  USHORT      usFlags,                   //(in)  flags controlling the updated fields
  USHORT      usMsgHandling,             //(in)  message handling parameter
                                         //      TRUE:  display error message
                                         //      FALSE: display no error message
  HWND        hwnd                       //(in)  handle for error messages
);



// structure for MT_TMMERGE pointer
typedef struct _MT_TMMERGE
{
  CHAR        chMemory[MAX_EQF_PATH];            // TM name to be merged into
  CHAR        chSGMLFile[MAX_EQF_PATH];          // external memory
  OBJNAME     szObjName;                         // document object name
  HWND        hwndNotify;                        // parent to be notified
  CHAR        szTargetLang[MAX_LANG_LENGTH];
  OBJNAME     szFolObjName;                      // object name of folder
} MT_TMMERGE, *PMT_TMMERGE;

BOOL MTTMMergeStart
(
  PLISTCOMMAREA pCommArea,
  PMT_TMMERGE pTMMerge
);
__declspec(dllexport)
BOOL MemCreatePath( PSZ pszString );


// Unicode additions
VOID TMX_GET_IN_Unicode2ASCII( PTMX_GET_IN_W pstGetInW, PTMX_GET_IN pstGetIn, ULONG ulCP );
VOID TMX_GET_OUT_Unicode2ASCII( PTMX_GET_OUT_W pstGetOutW, PTMX_GET_OUT pstGetOut, ULONG ulCP );
VOID TMX_GET_IN_ASCII2Unicode( PTMX_GET_IN pstGetIn, PTMX_GET_IN_W pstGetInW, ULONG  ulCP );
VOID TMX_GET_OUT_ASCII2Unicode( PTMX_GET_OUT pstGetOut, PTMX_GET_OUT_W pstGetOutW, ULONG ulCP );

VOID  TMX_PUT_IN_Unicode2ASCII( PTMX_PUT_IN_W pstPutInW, PTMX_PUT_IN pstPutIn, ULONG ulCP );
VOID  TMX_PUT_OUT_ASCII2Unicode( PTMX_PUT_OUT pstPutOut, PTMX_PUT_OUT_W pstPutOutW );
VOID  TMX_PUT_IN_ASCII2Unicode( PTMX_PUT_IN pstPutIn, PTMX_PUT_IN_W pstPutInW , ULONG ulCP);
VOID  TMX_PUT_OUT_Unicode2ASCII( PTMX_PUT_OUT_W pstPutOutW, PTMX_PUT_OUT pstPutOut );
VOID  TMX_EXT_IN_Unicode2ASCII( PTMX_EXT_IN_W pstExtInW, PTMX_EXT_IN pstExtIn );
VOID  TMX_EXT_OUT_ASCII2Unicode( PTMX_EXT_OUT pstExtOut, PTMX_EXT_OUT_W pstExtOutW, ULONG ulCP );
VOID  TMX_EXT_IN_ASCII2Unicode( PTMX_EXT_IN pstExtIn, PTMX_EXT_IN_W pstExtInW );
VOID  TMX_EXT_OUT_Unicode2ASCII( PTMX_EXT_OUT_W pstExtOutW, PTMX_EXT_OUT pstExtOut, ULONG ulCP );

VOID  TMX_PUT_IN_ASCII2Unicode( PTMX_PUT_IN pstDelIn, PTMX_PUT_IN_W pstDelInW, ULONG ulCP );
VOID  TMX_PUT_OUT_Unicode2ASCII( PTMX_PUT_OUT_W pstDelOutW, PTMX_PUT_OUT pstDelOut );

// function to convert the memory to the new format
USHORT MemConvertMem( PSZ pszFullMemName );

// return codes of function MemConvertMem

// memory converted successfully
#define MEM_CONVERTMEM_SUCCESS  0

// memory conversion not required, memory is in new format already
#define MEM_CONVERTMEM_ALREADYNEWFORMAT 7001


// do a cleanup of temporary memories
__declspec(dllexport)
void TMCleanupTempMem
(
  PSZ         pszPrefix                                     // ptr to memory prefix
);

// create a temporary memory
__declspec(dllexport)
USHORT TMCreateTempMem
(
  PSZ         pszPrefix,                                    // short prefix to be used for memory name (should start with a dollar sign)
  PSZ         pszMemPath,                                   // ptr to buffer for memory path
  HTM         *pHtm,                                        // ptr to buffer for memory handle
  HTM         htm,                                          // htm of similar memory
  PSZ         pszSourceLang,                                // memory source language
  HWND        hwnd                                          // window handle for error messages
);

// delete a temporary memory
__declspec(dllexport)
void TMDeleteTempMem
(
  PSZ         pszMemPath                                    // ptr to memory path
);

__declspec(dllexport)
BOOL TMFuzzyness( PSZ, PSZ_W, PSZ_W, SHORT, PUSHORT, ULONG );


__declspec(dllexport)
BOOL TMFuzzynessEx
(
  PSZ pszMarkup,                       // markup table name
  PSZ_W pszSource,                     // original string
  PSZ_W pszMatch,                      // found match
  SHORT sLanguageId,                   // language id to be used
  PUSHORT     pusFuzzy,                 // fuzzyness
  ULONG       ulOemCP,
  PUSHORT     pusWords                 // number of words in segment
);

__declspec(dllexport)
USHORT TokenizeSource( PTMX_CLB, PTMX_SENTENCE, PSZ, PSZ, USHORT );

__declspec(dllexport)
USHORT TokenizeSourceEx( PTMX_CLB, PTMX_SENTENCE, PSZ, PSZ, USHORT, ULONG );

__declspec(dllexport)
USHORT TokenizeSourceEx2( PTMX_CLB, PTMX_SENTENCE, PSZ, PSZ, USHORT, ULONG, int );

__declspec(dllexport)
USHORT NTMMorphTokenizeW( SHORT, PSZ_W, PULONG, PVOID*, USHORT, USHORT);

__declspec(dllexport)
USHORT NTMTokenize
(
  PVOID    pvLangCB,                   // IN : ptr to language control block
  PSZ      pszInData,                  // IN : ptr to data being tokenized
  PUSHORT  pusTermListSize,            // IN/OUT:  address of variable
                                       //    containing size of term list buffer
  PVOID    *ppTermList,                // IN/OUT: address of term list pointer
  USHORT   usListType,                 // IN: type of term list MORPH_ZTERMLIST,
                                       //    MORPH_OFFSLIST, MORPH_FLAG_OFFSLIST,
                                       //    or MORPH_FLAG_ZTERMLIST
   USHORT usVersion                    // version of TM
);

__declspec(dllexport)
BOOL NTMTagSubst                     // generic inline tagging for TM
(
  PTMX_SUBSTPROP pSubstProp,
  ULONG          ulSrcOemCP,
  ULONG          ulTgtOemCP
);

__declspec(dllexport)
BOOL NTMDocMatch( PSZ pszShort1, PSZ pszLong1, PSZ pszShort2, PSZ pszLong2 );

// utility to allocate pSentence and associated memory areas
__declspec(dllexport)
USHORT NTMAllocSentenceStructure( PTMX_SENTENCE  *ppSentence );

// utility to free pSentence and associated memory areas
__declspec(dllexport)
VOID NTMFreeSentenceStructure( PTMX_SENTENCE  pSentence );

__declspec(dllexport)
void NTMFreeSubstProp( PTMX_SUBSTPROP pSubstProp );

__declspec(dllexport)
USHORT  MemOpenProp( HPROP *, PVOID *, PSZ, PSZ, USHORT, BOOL );

__declspec(dllexport)
USHORT  FillMemoryListBox ( PLISTCOMMAREA, WPARAM, LPARAM );

__declspec(dllexport)
VOID NTMGetHandlesFromCLB ( HTM, PVOID *, PVOID * );

__declspec(dllexport)
USHORT NTMGetMatchLevel
(
  PEQFSEGINFO      pSegment,           // pointer to segment info
  PEQFSEGINFO      pProposal,          // pointer to memory proposal info
  SHORT            *psMatchLevel,      // pointer to caller's match level field
  SHORT            *psMatchState,      // pointer to caller's match state field
  LONG             lOptions            // options to be used by the function
);

