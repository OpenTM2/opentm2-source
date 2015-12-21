/*! EQFDASD.H  - Public header file for Dictionary Services
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

#if !defined(_EQFDASD_H_INCLUDED)

#define _EQFDASD_H_INCLUDED

/**********************************************************************/
/* flags for AsdTermList function                                     */
/**********************************************************************/
#define QDAM_LIST_FORWARD    1    // forward list requested
#define QDAM_LIST_BACKWARD   2    // backward list requested

/**********************************************************************/
/* Defines for dictionary levels                                      */
/**********************************************************************/
#define ENTRY_LEVEL             1
#define HW_LEVEL                2
#define POS_LEVEL               3
#define SENSE_LEVEL             4
#define TARGET_LEVEL            5

/**********************************************************************/
/* Return codes (borrowed from tolstoy include files)                 */
/**********************************************************************/
#if !defined(LX_RCS_DEFINED)
enum error_codes
  {
  LX_RC_OK_ASD,         /*  0 - operation completed successfully    */
  LX_2_MANY_USERS,      /*  1 - limit of user areas reached         */
  LX_BAD_DICT_TKN_ASD,  /*  2 - dict token is invalid               */
  LX_BAD_USR_AREA,      /*  3 - user number is invalid              */
  LX_DICT_IO_ASD,       /*  4 - I/O error accessing dictionary      */
  LX_DICT_WRT_ASD,      /*  5 - error writing to dictionary         */
  LX_EOF_ASD,           /*  6 - eof reached in search               */
  LX_FLE_EXISTS_ASD,    /*  7 - attempt to create existing file     */
  LX_INVLD_POSIT_ASD,   /*  8 - dictionary not in valid position    */
  LX_INVLD_WRDNM_ASD,   /*  9 - word number out of range            */
  LX_MEM_ALLOC_ASD,     /* 10 - user area allocation error          */
  LX_NO_USR_AREAS,      /* 11 - no user areas initialized yet       */
  LX_OLD_FORMAT_ASD,    /* 12 - file is old format DAM              */
  LX_OPEN_FLD_ASD,      /* 13 - dictionary open failed              */
  LX_OTHER_USER_ASD,    /* 14 - another user has update access      */
  LX_PROTECTED_ASD,     /* 15 - file is READ_ONLY - no changes      */
  LX_RENUM_RQD_ASD,     /* 16 - renumber on dictionary required     */
  LX_UNEXPECTED_ASD,    /* 17 - unexpected internal error           */
  LX_UNINIT_PRM_ASD,    /* 18 - uninitialized input parameter       */
  LX_WRD_EXISTS_ASD,    /* 19 - attempt to insert existing word     */
  LX_WRD_NT_FND_ASD,    /* 20 - word not found in ASD               */
  LX_MAX_COMMON_ERR,    /* 21 - number of common error messages     */
  LX_READ_ONLY          /* 22 - host- unable to open for write      */
  };

enum xt_errors
  {
  LX_ASC_NT_ALLWD_ASD = LX_MAX_COMMON_ERR, /* 21 - input assoc. handle not allowed  */
  LX_ASSOC_MEMBR_ASD,   /* 22 - dict. is a member of an assoc.   */
  LX_DAM_LOAD_ASD,      /* 23 - error loading DAM module         */
  LX_DATA_2_LRG_ASD,    /* 24 - data is too large for data space */
  LX_FLE_OPEN_ASD,      /* 25 - attempt to open an open file     */
  LX_IDX_DAT_SIZE,      /* 26 - index data over maximum size     */
  LX_IDX_FN_ERR,        /* 27 - error in index routine           */
  LX_IDX_FN_LOAD,       /* 28 - error loading index routine      */
  LX_IDX_NT_ALLWD_ASD,  /* 29 - input index handle not allowed   */
  LX_IDX_NT_OPEN_ASD,   /* 30 - error opening index file         */
  LX_INCOMP_SIG_ASD,    /* 31 - incompatible signature data      */
  LX_INV_ASSOC_TOP,     /* 32 - invalid dict. at top of assoc.   */
  LX_MAX_ASSOC,         /* 33 - too many active associations     */
  LX_MAX_ASSOC_DEPTH,   /* 34 - too many files in association    */
  LX_MAX_IDX_ASD,       /* 35 - too many indices for 1 base file */
  LX_MAX_OPEN_ASD,      /* 36 - limit of open files reached      */
  LX_INSUF_STOR_ASD,    /* 37 - only included for compatibility  */
  LX_MAX_ERR            /* 38 - total number of error messages   */
  };
#define LX_RCS_DEFINED 1
#endif

/**********************************************************************/
/* Return codes of Nlp morphologic services                           */
/* (only the return codes used in EQFDASD.C)                          */
/**********************************************************************/
// #define LX_ART_CHECKER_F_CHG  2  /* Flag change not allowed          */
// #define LX_BAD_ADDRESS        5  /* Invalid address                  */
// #define LX_BAD_C_NAMES        6  /* Invalid character name table     */
// #define LX_BAD_CPG_TO_DAP     7  /* Invalid ext to intern trans table*/
// #define LX_BAD_DAP_TO_CPG     8  /* Invalid intern to ext trans table*/
// #define LX_BAD_DICT           9  /* Bad Dictionary                   */
// #define LX_BAD_DICT_NME      10  /* Invalid Dictionary Name          */
// #define LX_BAD_DICT_TKN      11  /* Invalid Dictionary Token         */
// #define LX_BAD_DICT_TKN_CNT  12  /* Invalid Dictionary Token Count   */
// #define LX_BAD_ELEMENTS_CT   13  /* Invalid element count            */
// #define LX_BAD_ELE_FORMAT    14  /* Invalid data element format      */
// #define LX_BAD_ELE_TYPE      15  /* Invalid element type             */
// #define LX_BAD_FIRST         17  /* Invalid first token              */
// #define LX_BAD_FL_NME        18  /* Filename Not Allowed for Updateable Dict. */
// #define LX_BAD_FUNCT_CODE    19  /* Invalid function code            */
#define LX_BAD_LANG_CODE     22  /* Invalid language code            */
// #define LX_BAD_LAST          23  /* Invalid last token               */
// #define LX_BAD_PARM          25  /* Missing or Illegal input Value   */
// #define LX_BAD_PROP_NAME     26  /* Invalid property name            */
// #define LX_BAD_RQST_TYPE     27  /* Invalid request type             */
// #define LX_BAD_SA_PTR        28  /* Invalid Service Area pointer     */
// #define LX_BAD_SERV_HANDLE   28  /* Invalid service handle input     */
// #define LX_BAD_ST_TBL        30  /* Invalid stkn token table input   */
// #define LX_BAD_STR_ADDR      31  /* Invalid string address provided  */
// #define LX_BAD_STR_CNTNT     32  /* Unknown value in string content  */
// #define LX_BAD_STR_FLAG      33  /* Invalid string flag provided     */
// #define LX_BAD_TOK_LIST      34  /* Token list processing error      */
// #define LX_BAD_TOKEN         35  /* Invalid pointer to a token       */
// #define LX_BAD_TOKEN_TYPE    36  /* Invalid token type               */
// #define LX_BAD_WORD          37  /* Invalid word                     */
// #define LX_BAD_WRD_CNST      38  /* Invalid Word Construction        */
// #define LX_BASE_AID          39  /* Aid was for Base Form of Word    */
// #define LX_DAP_ERROR         40  /* DAP error return code            */
// #define LX_DEALLOC_ERR       41  /* Memory deallocation error        */
// #define LX_DICT_NO_UPDT      42  /* Dictionary is Not Updateable     */
// #define LX_DUP_DICT          43  /* Duplicate Dictionary             */
// #define LX_ELE_P             44  /* Missing element list             */
// #define LX_END_OF_INPUT      45  /* End of input reached             */
// #define LX_END_OF_PROPS      46  /* End of property list reached     */
// #define LX_END_OF_STORAGE    47  /* End of system storage reached    */
// #define LX_FL_ACC_DEND       48  /* File Access Denied               */
// #define LX_FL_NO_FND         49  /* File Not Found                   */
// #define LX_FL_OP_FAILD       50  /* File Open Failed                 */
// #define LX_FL_SHR_VIOLTN     51  /* File Sharing Violation           */
// #define LX_FOUND_SDLM        52  /* A text-seg delim has been found  */
// #define LX_FUNC_CODE_CHG     53  /* Change of func_code in LX_CB not allowed */
// #define LX_ELT_FORMAT_CHG        54
// #define LX_INVLD_NUM_DICT        57
// #define LX_IO_ERROR              58
// #define LX_JOIN_FAIL             59
// #define LX_MEM_ALLOC_ERR         60
// #define LX_MEM_USE_TAB_FUL       61
// #define LX_NO_AID_AVAIL          62
// #define LX_NO_COMP               63
// #define LX_NO_CPX                64
// #define LX_NO_DATA_AVAIL         65
// #define LX_NO_ENTRY              66
// #define LX_NO_MORPH_TBL          68
// #define LX_NO_REPLY_P            69
// #define LX_OUT_GOAL              71
#define LX_RC_OK                 73
// #define LX_RC_OK_NC              74
// #define LX_REP_WORD_AID_F_CHG    75
// #define LX_REPLY_FULL            76
// #define LX_SOS_CALLER            80
// #define LX_SPEL_CHNG             81
// #define LX_SPEL_CORRECT          82
// #define LX_SPELL_VER_F_CHG       83
// #define LX_SPLIT_ERROR           84
// #define LX_STRING_BUFFER         85
#define LX_UNEXPECTED_RC         86
// #define LX_WORD_NOT_FOUND        87
// #define LX_WRD_MULT_BASE         88
// #define LX_ZERO_SIZE             89
// #define LX_2_MANY_DICTS          90
// #define LX_BAD_MEMORY_CB         91
// #define LX_LEXAM_BUSY            92
// #define LX_REPLY_2_SMALL         93
// #define LX_NO_C_NAMES_BLANK      94
// #define LX_NO_INFLECT            95
// #define LX_BAD_CHAR              96
// #define LX_DELIMITER             97
// #define LX_NOT_DELIMITER         98
// #define LX_BAD_FUNC_F            99
// #define LX_DUP_WORD             100
// #define LX_NO_BIT_MASK_TABLE    101
// #define LX_NO_BIT_MASK_ENTRY    102
// #define LX_BAD_PATH             103
// #define LX_NO_DECOMP            104
// #define LX_BAD_FLAG             105
// #define LX_NO_DATA_SAT          106
// #define LX_BLOCK_SPAN           107
// #define LX_JOIN_CHAR            108
// #define LX_CPG_NOT_SUPPORTED    109
// #define LX_MEM_ACC_SHR          113
// #define LX_TIMEOUT              115
// #define LX_SHARED_MEM_NAME_ERROR 118
// #define LX_ERROR_LOAD_USR_MODULE 119
// #define LX_ERROR_ENTRY_PT_USR_MODULE 120
// #define LX_ERROR_FREE_USR_MODULE 121
// #define LX_PATH_TOO_LONG         123
// #define LX_INVALID_DBCS_INPUT    124
// #define LX_BAD_DICT_LEVEL        125
// #define LX_INVALID_SBCS_INPUT    126
// #define LX_ALIAS_NO_MATCH        127
// #define LX_SYN_OK_NO_MID         128


//+----------------------------------------------------------------------------+
//   Symbolic values for data offset list in index data record
//+----------------------------------------------------------------------------+
typedef enum _TERMTYPE
{
   MWT_TYPE,                           // MWT term list
   SYNONYM_TYPE,                       // synonyms term list
   ABBREV_TYPE,                        // abbreviation term list
   RELATED_TYPE,                       // related terms list
   COMPOUND_TYPE,                      // compound word list
   MWTSTEM_TYPE,                       // MWT terms (stem reduced)
   FILLER3_TYPE,                       // reserved for future use
   FILLER4_TYPE,                       // reserved for future use
   FILLER5_TYPE,                       // reserved for future use
   LASTTERMTYPE                        // list terminator
} TERMTYPE, *PTERMTYPE;


// unicode prefix char for style values
#define STYLEPREFIX_NOTALLOWED 0x2300       // Unicode "Diameter sign"
#define STYLEPREFIX_PREFERRED 0x2714        // Unicode "Heavy check mark"
#define STYLEPREFIX_UNDEFINED 0x2715        //


#define ASD_GUARDED  0x01              // Open dictionary in guarded mode
#define ASD_LOCKED   0x10              // Open dictionary and lock it
#define ASD_SHARED   0x20              // Open in shared mode
#define ASD_FORCE_WRITE   0x40         // force imm.write of any changed record
#define ASD_NOINDEX  0x80              // Do not process index dictionary
                                       // Caution: this define should be used
                                       // only for dictionary organize!!!
#define ASD_ORGANIZE 0x100             // work in organize mode
#define ASD_NOOPENCHECK 0x200          // do not check QDAM header open flag
#define ASD_READONLY    0x400          // open in read only mode


// AsdTranslate mode: return terms only
#define ASDT_TERMS_ONLY     0x01

// AsdTranslate mode: return terms and translations
#define ASDT_TRANSLATIONS   0x02

// AsdTranslate mode: return terms, translations and marked fields
#define ASDT_MARKED_DATA    0x03

// additional AsdTranslate flag: lookup single terms of components as well
#define ASDT_SINGLECOMP_LOOKUP 0x40

// additional AsdTranslate flag: do not stop at first match but continue for all dicts
#define ASDT_SEARCH_ALL_DICTS 0x80

// Prefix char for non-translation dictionary data
#define ASDT_PRE_DICT_DATA_CHAR  0x01
#define ASDT_POST_DICT_DATA_CHAR 0x02


//+----------------------------------------------------------------------------+
//   merge control flags for AsdMergeEntry function
//+----------------------------------------------------------------------------+

// type of merge flags, these flags are mutually exclusive
#define MERGE_REPLACE       0x0001     // replace existing entries
#define MERGE_NOREPLACE     0x0002     // do not replace existing entries
#define MERGE_ADD           0x0004     // add data of new entry to data of existing entry

//    if the following flag is specified one of the merge type flags
//    has to be specified also
#define MERGE_NOUSERPROMPT  0x0010     // do not ask user for type of merge
#define MERGE_USERPROMPT    0x0020     // ask user for type of merge

//    source of merge flags, these flags are mutually exclusive
//    these flags control the heading text of the source MLE
#define MERGE_SOURCE_ASD    0x0100     // source is an ASD dictionary
#define MERGE_SOURCE_SGML   0x0200     // source is a SGML dictionary
#define MERGE_SOURCE_EDIT   0x0400     // source is the edit dictionary dialog

#define MERGE_NOPROMT_CHECKBOX 0x0800  // if set the checkbox with the
                                       //    "Use the selected option"
                                       // option is hidden



//+----------------------------------------------------------------------------+
//  LupBegin            Begin/Initialize Lookup Service Functions
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT LupBegin
(
   HUCB    hUCB,                       // Asd services user control block
   HDCB    hDCB,                       // Asd services dictioanry control block
   HWND    hwndParent,                 // parent handle for dialogs and messages
   USHORT  usNotifyMsg,                // message used for notifications
   PRECTL  prclDisplay,                // size/position of display dialog or NULL
   PRECTL  prclEdit,                   // size/position of edit dialog or NULL
   PHLUPCB phLUPCB,                    // Lookup services control block
   PUSHORT pusID                       // identifier for this lookup service
);

// Values for LupBegin2 ulFlags parameter
#define LUP_MDIDLG_MODE   0x00000001L
#define LUP_MODALDLG_MODE 0x00000002L
#define LUP_WPCONTROLS    0x00000004L

USHORT LupBegin2
(
   HUCB    hUCB,                       // Asd services user control block
   HDCB    hDCB,                       // Asd services dictioanry control block
   HWND    hwndParent,                 // parent handle for dialogs and messages
   USHORT  usNotifyMsg,                // message used for notifications
   PRECTL  prclDisplay,                // size/position of display dialog or NULL
   PRECTL  prclEdit,                   // size/position of edit dialog or NULL
   PHLUPCB phLUPCB,                    // Lookup services control block
   PUSHORT pusID,                      // identifier for Lookup CB
   ULONG   ulFlags                     // flags for kind of lookup
);

//+----------------------------------------------------------------------------+
//  LupEnd              Terminate Lookup Service Functions
//+----------------------------------------------------------------------------+
USHORT LupEnd
(
   HLUPCB hLUPCB                       // Lookup services control block
);

//+----------------------------------------------------------------------------+
//  LupActivate         Activate the lookup dialog
//+----------------------------------------------------------------------------+
USHORT LupActivate
(
   HLUPCB hLUPCB                       // Lookup services control block
);

//+----------------------------------------------------------------------------+
//  LupLookup           Look up a term and display it in the display dialog
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT LupLookup
(
   HLUPCB   hLUPCB,                    // Lookup services control block
   PCHAR_W  pucTerm                    // term being looked-up
);
USHORT LupLookup2
(
   HLUPCB   hLUPCB,                    // Lookup services control block
   PCHAR_W  pucTerm,                   // term being looked-up
   HWND     hwnd                       // parent handle for dialog box
);

//+----------------------------------------------------------------------------+
//  LupEdit             Edit a term
//+----------------------------------------------------------------------------+
USHORT LupEdit
(
   HLUPCB   hLUPCB,                    // Lookup services control block
   PCHAR_W   pucTerm                   // term being looked-up
);
USHORT LupEdit2
(
   HLUPCB   hLUPCB,                    // Lookup services control block
   PCHAR_W  pucTerm,                   // term being looked-up
   HWND     hwnd                       // parent handle for dialog box
);

__declspec(dllexport)
HWND LupGetLookupHandle
(
   HLUPCB   hLUPCB                     // Lookup services control block
);

__declspec(dllexport)
USHORT LupGetDispHandles
(
   HLUPCB   hLUPCB,                    // Lookup services control block
   PHWND    *phwndDisp
);
//+----------------------------------------------------------------------------+
//  lookp/edit return codes
//+----------------------------------------------------------------------------+
typedef enum _LUP_RC
{
   LUP_OK,                             // LUP service completed successfully
   LUP_MEM_ALLOC_ERR,                  // memory allocation failed
   LUP_RES_LOAD_ERR,                   // dialog resources could not be loaded
   LUP_BAD_ASD_UCB,                    // passed UCB handle is in error
   LUP_BAD_ASD_DCB,                    // passed DCB handle is in error
   LUP_BAD_LUPCB_ERR,                  // passed LUPCB handle is in error
   LUP_LAST_RC                         // last entry in RC enumeration
} LUP_RC;

/**********************************************************************/
/* Notification codes of lookup notify message                        */
/**********************************************************************/
#define DLG_TERM_NORM                   0       // information values for main
#define DLG_SHOWN                       1       // - " -
#define DLG_CANCELED                    2       // - " -
#define DLG_HIDDEN                      3       // - " -
#define DLG_POSITIONED                  4       // - " -

INT_PTR CALLBACK EXPENTRY  LOOKUPDLGPROC( HWND, WINMSG, WPARAM, LPARAM);


// defines for the sListType of AsdGetTranslation
#define SEMIKOLON_TERM_LIST    1       //     semikolon seperated list


#endif
