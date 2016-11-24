//+----------------------------------------------------------------------------+
//|  EQFDASDI.H  - Internal header file for ASD layer of Dictionary Services   |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Description:                                                              |
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
// $Revision: 1.3 $ ----------- 11 Apr 2005
// GQ: - added list of style values
// 
// 
// $Revision: 1.2 $ ----------- 7 Apr 2005
// GQ: - changed parameters of index related functions from USHORT to ULONG in order to support
//       index entries with more than 32 KB data
//     - added structure definition for index records
//     - R008174: Style field, added new style fields to DCB and added defines for system names of new fields
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
// $Revision: 1.2 $ ----------- 29 Jul 2002
// -- RJ: R7197: add cp as param to DELETEATOMW, add cp as param to DICTCB
// --    del. param fisDBCS in FillMLE, BuildLine, Del. function FIllDBCSTable
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.4 $ ----------- 22 Oct 2001
// -- RJ: get rid of QLDB_START_CTRL_INFO
//
//
// $Revision: 1.3 $ ----------- 19 Oct 2001
// --RJ: unicode enabling of dictionary
//
//
// $Revision: 1.2 $ ----------- 3 Sep 2001
// --RJ: unicode enabling
//
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
// $Revision: 1.4 $ ----------- 8 Aug 2001
// GQ: Changed buffer size parameters of function AsdAddToTermList from USHORT to ULONG
//
//
// $Revision: 1.3 $ ----------- 19 Feb 2001
// -- Thai: do not do Oem to Ansi conversion
// -- add THAI_STR define
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFDASDI.HV_   1.7   08 Jun 1998 18:10:12   BUILD  $
 *
 * $Log:   J:\DATA\EQFDASDI.HV_  $
 *
 *    Rev 1.7   08 Jun 1998 18:10:12   BUILD
 * -- add support for dictionary handle in ASDTRANSLATE data sequence
 *
 *    Rev 1.6   14 Jan 1998 15:29:20   BUILD
 * - rename DCB to EQFDCB
 *
 *    Rev 1.5   26 Feb 1997 20:12:20   BUILD
 * -- Compiler defines for _POE22, _TKT21, and NEWTCSTUFF eliminated
 *
 *    Rev 1.4   17 Feb 1997 16:20:58   BUILD
 * - changed prototype of MWT recognition function
 *
 *    Rev 1.3   11 Jul 1996 12:35:26   BUILD
 * - added usMaxColumns and fWPIF to parameter list for function BuildLine
 *
 *    Rev 1.2   18 Mar 1996 16:25:14   BUILD
 * - added system names of author fields and fields for author field data
 *
 *    Rev 1.0   09 Jan 1996 09:45:22   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define REPLY_AREA_SIZE  32000         // size of Nlp service reply area

#define INDEX_SIZE_INCR    1024         // size for enlarging index data records

#define TERM_BUFFER_INCR  1024         // size for enlarging the term buffer

#define TRANSTERMBUF_INCR  128         // increment size for term offset list
                                       // in AsdTranslate function

#define MAX_TRANSL_TERMS    40         // max number of translated terms in
                                       // AsdTranslate function

#define MAX_DISP_DLGS     1000         // max number of active display dlgs per
                                       // lookup control block

#define ASD_DEFAULT_LANGUAGE "English(U.S.)" // default dictionary language

#define DICTHANDLE USHORT              // map to Nlp dictionary handles
#define PDICTHANDLE PUSHORT            // map to Nlp dictionary handles



/**********************************************************************/
/* System names of field which need special handling within ASD       */
/* functions                                                          */
/**********************************************************************/
#define SYSNAME_TRANSLATION    "Translation"
#define SYSNAME_SYNONYM        "Synonym"
#define SYSNAME_ABBREVIATION   "Abbrev./Fullform"
#define SYSNAME_RELATED        "Other Related Terms"
#define SYSNAME_CREATEDATE     "Creation Date"
#define SYSNAME_LASTUPDATE     "Last Update"
#define SYSNAME_CONTEXT        "Context"
#define SYSNAME_TARGCREATEDATE "Trans Creation Date"
#define SYSNAME_TARGLASTUPDATE "Trans Last Update"
#define SYSNAME_AUTHOR         "Author"
#define SYSNAME_AUTHOROFUPDATE "Author of Update"
#define SYSNAME_TRANSAUTHOR    "Trans Author"
#define SYSNAME_TRANSAUTHOROFUPDATE  "Trans Author of Update"
#define SYSNAME_ENTRYSTYLE     "Style"
#define SYSNAME_TARGETSTYLE    "Trans Style"
#define HEADWORD_SYST_NAME "Headword"

// list of predefined values for style fields (must be terminated by double zero)
#define STYLEVALUE_PREFERRED  L"preferred\0\0"
#define STYLEVALUE_NOTALLOWED L"deprecated\0non standard\0no longer used\0\0"

//@@#define QLDB_START_CTRL_INFO sizeof(CHAR_W)+sizeof(USHORT)  // include eqfldbi.h!!!

//+----------------------------------------------------------------------------+
//| Macros for lazy coders                                                     |
//+----------------------------------------------------------------------------+

// check user control block pointer
#define CHECKUCB( pUCB, usNlpRC )                      \
{                                                      \
   if ( !pUCB || (pUCB->lSignature != UCB_SIGNATURE))  \
   {                                                   \
      usNlpRC = LX_BAD_USR_AREA;                       \
   } /* endif */                                       \
}

// check dictionary control block pointer
#define CHECKDCB( pDCB, usNlpRC )                      \
{                                                      \
   if ( !pDCB || (pDCB->lSignature != DCB_SIGNATURE))  \
   {                                                   \
      usNlpRC = LX_BAD_DICT_TKN_ASD;                   \
   } /* endif */                                       \
}

// convert Nlp service RC to Asd service RC
#define NLP2ASDRC( usNlpRC )                          \
   usNlpRC = (USHORT)(( usNlpRC == LX_RC_OK ) ? LX_RC_OK_ASD : (usNlpRC | 0x8000))

// check if asd service RC is OK
#define ASDOK( usNlpRC ) usNlpRC == LX_RC_OK_ASD

// set last time variable in UCB
#define INITTIME( pUCB )  pUCB->ulLastTime = pUCB-> pInfoSeg->msecs

// get delta time and store it in supplied variable; reset last time
#define GETTIME( pUCB, ulTime )                       \
{                                                     \
   ulTime += pUCB->pInfoSeg->msecs - pUCB->ulLastTime;\
   pUCB->ulLastTime = pUCB->pInfoSeg->msecs;          \
}

// delete an atom by atom name
#define DELETEATOM( table, name, handle ) \
   {                                      \
      handle = WinFindAtom( table, name );\
      if ( handle )                       \
      {                                   \
         WinDeleteAtom( table, handle );  \
      } /* endif */                       \
   }

// delete an atom by atom name
#define DELETEATOMW( table, name, handle, cp ) \
   {                                      \
      handle = WinFindAtomW( table, name, cp );\
      if ( handle )                       \
      {                                   \
         WinDeleteAtom( table, handle );  \
      } /* endif */                       \
   }

//+----------------------------------------------------------------------------+
//| Dictionary default values                                                  |
//+----------------------------------------------------------------------------+

#if defined(ASDDEFAULTS)
// primary collating sequence
UCHAR  ucbAsdPrimCol[256] = { 0 };

// secondary collating sequence
UCHAR  ucbAsdSecCol[256] = { 0 };

// term edit command
//UCHAR  ucbAsdEditCmd[256] = "/-// / // /of// /,// /.//";
UCHAR  ucbAsdEditCmd[256] = "";

// term encoding table
UCHAR  ucbAsdTermTbl[30]
        =  { 00,  97,  32, 101,  65, 116, 110, 105, 115, 114,  99, 111,
             14, 100, 108, 117, 104,  98, 103,   1, 102, 109, 112,  13,
             67,  66,  24,  69,  68,  70 };

// entry encoding table
UCHAR  ucbAsdEntryTbl[256]
        =  { 00,  97,  32, 101,  65, 116, 110, 105, 115, 114,  99, 111,
             14, 100, 108, 117, 104,  98, 103,   1, 102, 109, 112,  13,
             67,  66,  24,  69,  68,  70 };

// list of index dictionaries
UCHAR  ucbAsdIndexLst[256] = {0 };

// dictionary type
USHORT usAsdType   =    0;             // 0 = normal dictionary

// dictionary codepage
USHORT usAsdCodepage  =  850;

// dictionary feature mask
USHORT usAsdFeatMask  = 0x02 |         // perform term editing +
                        0x08;          // perform entry encoding


#define ASD_GUARDED_NUM_PAGES    2     // pages if opened in guarded mode
#define ASD_UNGUARDED_NUM_PAGES 10     // pages if opened in unguarded mode

// #define ASD_GUARDED_NUM_PAGES    5     // pages if opened in guarded mode
// #define ASD_UNGUARDED_NUM_PAGES 25     // pages if opened in unguarded mode

// user data for signature record of index dictionaries
typedef struct _INDEXSIGDATA
{
   USHORT   usVersion;                 // version number of index dictionary
} INDEXSIGDATA, *PINDEXSIGDATA;

INDEXSIGDATA IndexSigData = { 1 };

#endif

//+----------------------------------------------------------------------------+
//|  Actions for term list processing                                          |
//+----------------------------------------------------------------------------+
typedef enum _TERMACTION
{
   INSERT_ACTION,                      // insert term in index
   DELETE_ACTION,                      // delete term from index
   BUILD_ACTION                        // create empty index entry for term
} TERMACTION, *PTERMACTION;


//+----------------------------------------------------------------------------+
//|  Level and field number(index) of a field                                  |
//+----------------------------------------------------------------------------+
typedef struct _FIELDDATA
{
  BOOL        fInDict;                 // field-is-contained-in-dictionary flag
  USHORT      usField;                 // absolute number of field
  USHORT      usLevel;                 // level of the field
  USHORT      usIndex;                 // index of the field in the node data
} FIELDDATA, *PFIELDDATA;

//+----------------------------------------------------------------------------+
//|  Layout for tasks in a task list                                           |
//+----------------------------------------------------------------------------+
typedef struct _TERMBUFENTRY
{
   USHORT    usLength;                 // overall length of this entry
   USHORT    usAction;                 // action to be performed with this entry
                                       //    (see TERMACTION enum)
   USHORT    usTermType;               // type of this entry (see TERMTYPE enum)
   USHORT    ucTerm[1];                // term data (variable length, null
                                       //    terminated)
} TERMBUFENTRY, *PTERMBUFENTRY;

//+----------------------------------------------------------------------------+
//|  ASD User Control block (UCB)                                              |
//+----------------------------------------------------------------------------+
#define UCB_SIGNATURE 0x21506ACB       // used to identify and check UCBs

typedef struct _UCB                    // ASD user control block
{
   LONG     lSignature;                // UCB signature
   //---------------------- part for tolstoy ASD services ---------------------
   USHORT   usUser;                    // user handle
   USHORT   usMaxDicts;                // maximum number of dictionaries
   //---------------------- part for tolstoy NLP services ---------------------
   //------  for the use with the Morph... functions --------------------------
   BOOL     fNoCompounds;              // no-compound-support-available flag
   USHORT   usTermListSize;            // current size of general term list
   PSZ_W    pszTermList;               // ptr to buffer for general term list
   ULONG    ulOrgTermListSize;         // current size of original term list
   PSZ_W    pszOrgTermList;            // ptr to buffer for original term list
   ULONG    ulDictTermListSize;        // current size of dictionary term list
   PSZ_W    pszDictTermList;           // ptr to buffer for dictionary term list
   USHORT   usCompListSize;            // current size of compound list
   PSZ_W    pszCompList;               // ptr to buffer for compound list
   //------  for the use with the Tolstoi functions --------------------------
   USHORT   usService;                 // NLP service handle
   USHORT   usVersion;                 // version of Nlp services
   ULONG    ulReplySize;               // size of services reply handle
   PUCHAR   pucReplyArea;              // reply area for Nlp services
   //---------------------- part containing DBCS info ----------------------
   BOOL       fDBCS;                   // true if we are in a DBCS environment
   BOOL       fisDBCS1[256];           // 'is first DBCS byte' flag array
   //---------------------- part for AsdTranslate data ---------------------
   BOOL     fTransUsed;                // TRUE = variables for AsdTranslate have
                                       // been used
                                                                /* 2@KITxxxxD */
   HATOMTBL hNotInBaseAtoms;           // atom table for terms not in base dict
   ULONG    ulLastTime;                // remembered last time
   ULONG    ulBaseUpdTime;             // time spent for base dictionary updates
   ULONG    ulIdxUpdTime;              // time spent for index dictionary updates
   ULONG    ulPrepUpdTime;             // time spent for index update preparation
   //--- times of AsdTranslate function
   ULONG    ulTransAtomTime;           // atom manager times
   ULONG    ulTransListIndexTime;      // list index function
   ULONG    ulTransListIndex;          // number of list index calls
   ULONG    ulTransListTime;           // create term list
   ULONG    ulTransNlpFindTime;        // time of Nlp find calls
   ULONG    ulTransNlpRetTime;         // time of Nlp ret entry calls
   ULONG    ulTransMWTTime;            // search for MWTs
   ULONG    ulTransNormTime;           // normalize input segment
   ULONG    ulTransOtherTime;          // other times in AsdTranslate
   ULONG    ulTransTokTime;            // AsdTokenize
   ULONG    ulTransNlpFind;            // number of Nlp find calls
   ULONG    ulTransNlpRet;             // number of Nlp ret calls
   ULONG    ulTransTerms;              // number of terms processed
   ULONG    ulTransStemRedTime;        // stem form reduction time
   ULONG    ulTransStemRed;            // number of stem form reductions
   //--- values for payload ratio evalutation
   ULONG    ulDataLen;                 // overall data length of entries processed
   ULONG    ulPayload;                 // real data length (w/o node infos)
   ULONG    ulNewDataLen;              // datalength if new-LDB will be used
   //---------- data for morphological processing -----------------------------
   USHORT         ausMorphDicts[MAX_DICTS]; // list of morphologial dictionaries (not used)
   //---------------------- general use data areas ----------------------------
   CHAR_W    ucStemTerm[MAX_TERM_LEN+1]; // buffer for stem form reduction
   CHAR      cLangProp[MAX_LANGUAGE_PROPERTIES]; // buffer for lanuguage props.(not CHAR_W!)
   CHAR_W    ucTermBuf[MAX_TERM_LEN+1];           // buffer for terms
  // BOOL     fWPEdit;                    // WordPerfect Edit control available
   BOOL      fDictPIDSelect;              // TRUE = select dict. by PID value
   BOOL      fDictPIDFieldsSet;           // TRUE = Field info has been set
   CHAR      szDictPIDSelect[MAX_DICTPID_VALUES]; // PID values to select
   USHORT    usDictPIDSelect[MAX_DICTS];  // Dict. field containing PID values 
   USHORT    usDictSearchSubType;         // Dict. search subtype (editor lookup)
} UCB, *PUCB;

//+----------------------------------------------------------------------------+
//|  Dictionary control block (DCB)                                            |
//+----------------------------------------------------------------------------+
#define DCB_SIGNATURE 0x02045DCB       // used to identify and check DCBs

typedef struct _EQFDCB                    // Dictionary control block
{
   LONG     lSignature;                // DCB signature
   BOOL     fAssoc;                    // TRUE = DCB for associated dictionary
   USHORT   usLocation;                // location of dictionary (local/remote/shared)
   DICTHANDLE usDictHandle;            // dictionary ASD handle
   DICTHANDLE usIndexHandle;           // index dictionary handle
   CHAR     szPropName[CCHMAXPATH];    // name of property file
   struct _EQFDCB *pNextDCB;           // ptr to next DCB in a chain
   PROPDICTIONARY Prop;                // dictionary properties
   UCHAR    ucSeper[200];              // some procedures use more space
                                       // than the structure PROPDICTIONARY
                                       // occupies!
   USHORT   usOpenFlags;               // flags used for AsdOpen call
   LONG      lUpdTime;                 // time of last dictionary update
   //---------- data of signature record -----------------------------
   USHORT         usAsdType;           // type of dictionary
   USHORT         usFeatMask;          // dictionary feature mask
   USHORT         usCodepage;          // dictionary code page
   USHORT         usUserDataLen;       // length of dictionary user data
   //---------- field data of specific term data -----------------------------
   FIELDDATA      TransField;          // translation field
   FIELDDATA      SynField;            // synonym field
   FIELDDATA      AbbrField;           // abbreviation field
   FIELDDATA      RelField;            // related terms field
   FIELDDATA      CreateDateField;     // term creation date
   FIELDDATA      UpdateField;         // term last update field
   FIELDDATA      TargCreateDateField; // target creation date
   FIELDDATA      TargUpdateField;     // target last update field
   FIELDDATA      ContextField;        // context field
   FIELDDATA      AuthorField;         // Author field
   FIELDDATA      AuthorOfUpdateField; // Author of update field
   FIELDDATA      TransAuthorField;    // Trans Author field
   FIELDDATA      TransAuthorOfUpdateField; // Trans Author of update field
   FIELDDATA      EntryStyle;          // Style field on entry level
   FIELDDATA      TargetStyle;         // Style field on target level
   //--------------- fields-per-level array for LDB processing ---------------
   USHORT         ausNoOfFields[QLDB_MAX_LEVELS];
   //--------------- first-field-per-level array for LDB processing ----------
   USHORT         ausFirstField[QLDB_MAX_LEVELS];
   //--------------- field data pointer array for LDB processing -------------
   PSZ_W          apszFields[MAX_PROF_ENTRIES];
   //---------- data for morphological processing -----------------------------
   SHORT    sLangID;                   // ID for currently active language
                                       // used by the Morph... functions
   USHORT         ausMorphDicts[MAX_DICTS]; // list of morphologial dictionaries (not used)
                                       // used by the Tolstoi functions
   //------------------- dummy data area -----------------------------
   CHAR_W         aucDummy[512];       // buffer for unwanted data
   //------------------- number of dictionary in sequence ------------------
   USHORT         usOpenNum;           // number of dictionary in sequence
   ULONG          ulOemCP;             // CP of language of sLangID
} DICTCB, *PDCB;

//
// structure of the head of an index entry 
//
// the first part of the structure (USHORT array) is only there for compatibility
// reasons with Tmgr version prior to TP607 which do not use ULONG values for
// the offsets to the term data
// for records with a size of 32 KB and above the array simulates an empty
// record, for all other records the USHORT array points to the same locations
// as the ULONG offset array
typedef struct _ASDINDEXENTRYHEADER
{
  USHORT usTermOffs[LASTTERMTYPE+1];   // array used by Tmgr versions prior to TP607
  LONG   lCheckSum;                    // checksum of USHORT array (in order to detect changes
                                       // done with Tmgr versions before TP607 which do not
                                       // update the ULONG array)
  ULONG  ulTermOffs[LASTTERMTYPE+1];   // array used by Tmgr TP607 and above
  BYTE   bBuffer[10];                  // for future use..
} ASDINDEXENTRYHEADER, *PASDINDEXENTRYHEADER;

//+----------------------------------------------------------------------------+
//| AsdLoadDictProperties  - load dictionary propery file                      |
//+----------------------------------------------------------------------------+
//USHORT AsdLoadDictProperties
//(
//   PSZ             pszPropFile,        // fully qualified property file name
//   PPROPDICTIONARY pDictProp           // buffer for property data
//
//);

//+----------------------------------------------------------------------------+
//| AsdGetCodeName         - get the code name for a specific field            |
//+----------------------------------------------------------------------------+
USHORT AsdGetCodeName
(
   PUSHORT pusCode,                    // ptr to buffer for the code name
   PSZ     pszSysName,                 // system name of the field
   PUCHAR  pucSigData                  // ptr to signature record user data
);

//+----------------------------------------------------------------------------+
USHORT AsdAddToIndexData
(
   USHORT   usTermType,                // type of term (from TERMTYPE enum)
   PSZ_W    pszTerm,                   // term to be added
   PBYTE    *ppIndexData,              // ptr to index data area
   PULONG   puDataSize,               // size of index data area
   PULONG   pulDataUsed,               // number of bytes used in index data
   BOOL     fSort                      // TRUE = sort term into term list
);

//+----------------------------------------------------------------------------+
//| AsdDelFromIndexData       - delete a term from the data part of an index   |
//+----------------------------------------------------------------------------+
USHORT AsdDelFromIndexData
(
   USHORT   usTermType,                // type of term (from TERMTYPE enum)
   PSZ_W    pszTerm,                   // term to be removed
   USHORT   usTerms,                   // number of terms being removed
   PBYTE    pIndexData,                // ptr to index data area
   PULONG   pulDataUsed                // number of bytes used in index data
);

//+----------------------------------------------------------------------------+
//| AsdLocInIndex       - locate a term in the data part of an index           |
//+----------------------------------------------------------------------------+
USHORT AsdLocInIndex
(
   USHORT   usTermType,                // type of term (from TERMTYPE enum)
   PSZ_W      pszTerm,                   // term to be added
   PBYTE      pIndexData,                // ptr to index data area
   PBYTE      *ppTermLoc                 // location of term in index data
);

//+----------------------------------------------------------------------------+
//| AsdExtractTerms         - extracts specific entries from dictionary data   |
//+----------------------------------------------------------------------------+
USHORT AsdExtractTerms
(
   HDCB    hDCB,                       // dictionary control block
   USHORT  usAction,                   // value to store in action field
   PSZ_W   pucDictData,                // ptr to dictionary data
   ULONG   ulDataLength,               // length of dictionary data
   PUSHORT pusTerms,                   // number of terms extracted
   PSZ_W   *ppucTerms,                 // ptr to term list
   PULONG  pulSize,                    // current size of term list
   PULONG  pulUsed,                    // bytes used in term list
   PSZ_W   pszTerm                     // currently processed term
);

//+----------------------------------------------------------------------------+
//| AsdGetIndexEntry        - read index entry for a given term                |
//+----------------------------------------------------------------------------+
USHORT AsdGetIndexEntry
(
   HUCB    hUCB,                       // user control block
   HDCB    hDCB,                       // dictionary control block
   PSZ_W   pucTerm,                    // index entry term
   PBYTE   *ppucData,                  // ptr to index data buffer
   PULONG  pulSize,                    // size of index data buffer
   PULONG  pulUsed,                    // # of bytes used in index data buffer
   PHDCB   phDictFoundDCB              // handle of dictionary containing term
);

//+----------------------------------------------------------------------------+
//| AsdPutIndexEntry        - rewrites an index entry                          |
//+----------------------------------------------------------------------------+
USHORT AsdPutIndexEntry
(
   HUCB    hUCB,                       // user control block
   HDCB    hDCB,                       // dictionary control block
   PSZ_W   pucTerm,                    // index entry term
   PBYTE   pucData,                    // ptr to index data buffer
   ULONG   ulUsed                      // # of bytes used in index data buffer
);

//+----------------------------------------------------------------------------+
//| AsdInsIndexEntry        - insert a new index entry                         |
//+----------------------------------------------------------------------------+
USHORT AsdInsIndexEntry
(
   HUCB    hUCB,                       // user control block
   HDCB    hDCB,                       // dictionary control block
   PSZ_W   pucTerm,                    // index entry term
   PBYTE   pucData,                    // ptr to index data buffer
   ULONG   ulUsed                      // # of bytes used in index data buffer
);

//+----------------------------------------------------------------------------+
//| AsdDelIndexEntry        - delete an index entry                            |
//+----------------------------------------------------------------------------+
USHORT AsdDelIndexEntry
(
   HDCB    hUCB,                       // user control block
   HDCB    hDCB,                       // dictionary control block
   PSZ_W   pucTerm                     // index entry term
);

//+----------------------------------------------------------------------------+
//| AsdIsIndexEmpty         - checks if an index entry contains no data        |
//+----------------------------------------------------------------------------+
BOOL AsdIsIndexEmpty
(
   PSZ_W   pucData                     // index data buffer
);

//+----------------------------------------------------------------------------+
//| AsdUpdateIndex      Update index dictioanry                                |
//+----------------------------------------------------------------------------+
USHORT AsdUpdateIndex
(
   HUCB     hUCB,                      // user control block handle
   HDCB     hDCB,                      // dictionary control block handle
   PSZ_W    pucHeadword,               // headword of dictionary entry
   USHORT   usTerms,                   // number of terms interm list
   PSZ_W    pucTaskList                // terms to be added or deleted
);

//+----------------------------------------------------------------------------+
//| AsdMakeIndexEntry       - create an empty index dictionary record          |
//+----------------------------------------------------------------------------+
USHORT AsdMakeIndexEntry
(
   PBYTE   *ppucData,                  // ptr to index data buffer
   PULONG  pulSize,                    // size of index data buffer
   PULONG  pulUsed                     // # of bytes used in index data buffer
);

//+----------------------------------------------------------------------------+
//| AsdAddToTaskList        - add a new term to a term list                    |
//+----------------------------------------------------------------------------+
USHORT AsdAddToTaskList
(
   USHORT   usTermType,                // type of term
   USHORT   usAction,                  // action to be performed on term
   PSZ_W    pucTerm,                   // term being added
   USHORT   usTermLength,              // length of term being added
   PUSHORT  pusTerms,                  // number of terms in term list
   PBYTE    *ppucTaskList,             // ptr to term list
   PULONG   pulSize,                   // current size of term list
   PULONG   pulUsed                    // used bytes in term list
);

//+----------------------------------------------------------------------------+
//| AsdIsMWT                - check if a given term is a multi word term       |
//+----------------------------------------------------------------------------+
BOOL AsdIsMWT
(
   PUCB     pUCB,                      // user control block pointer
   PDCB     pDCB,                      // dictionary control block pointer
   PSZ_W    pucTerm,                   // term being checked
   PUSHORT  pusFirstWordLength         // length of first word of the MWT
);

//+----------------------------------------------------------------------------+
//| AsdGetNextCode          - gets the next node code                          |
//+----------------------------------------------------------------------------+
VOID AsdNextCode
(
   PUCHAR pucCode                      // node code
);

//+----------------------------------------------------------------------------+
//| AsdHandleToDCB          - convert a Nlp dictioanry handle to a DCB handle  |
//+----------------------------------------------------------------------------+
HDCB AsdHandleToDCB
(
   PDCB pDCB,                          // dictioanry control block
   DICTHANDLE usDictHandle             // handle returned by Nlp calls
);

//+----------------------------------------------------------------------------+
//| AsdAddToTermList        - add a new term to a term list                    |
//+----------------------------------------------------------------------------+
USHORT AsdAddToTermList
(
   PSZ_W    pucTerm,                   // term being added
   USHORT   usTermLength,              // length of current term
   PUSHORT  pusTerms,                  // number of terms in term list
   PSZ_W    *ppucTaskList,             // ptr to term list
   PLONG    plSize,                    // current size of term list
   PLONG    plUsed,                    // used bytes in term list
   BOOL     fReallocAllowed,           // reallocation is allowed flag
   USHORT   usDictHandle               // dictionary handle where term/transl. found
);

//+----------------------------------------------------------------------------+
//| AsdTokenize             - tokenize a string using NlpComplex               |
//+----------------------------------------------------------------------------+
USHORT AsdTokenize
(
   PUCB      pUCB,                     // user control block
   PDCB      pDCB,                     // dictionary control block
   PSZ_W     pucString,                // string being tokenized
   PUSHORT   pusTerms,                 // number of terms found
   PTERMLENOFFS *ppTermList            // ptr to create term list
);

USHORT AsdGetField
(
   PFIELDDATA       pField,            // ptr to field data buffer
   PSZ              pszSysName,        // system name of field
   PPROPDICTIONARY  pProfile           // ptr to dictionary profile
);

BOOL AsdFillDBCSTable
(
   BOOL     fisDBCS1[]                 // DBCS character flag array
);

BOOL AsdMWTMatch
(
  USHORT    usMWTLength,               // length of MWT
  PSZ_W     pucMWT,                    // data of MWT
  USHORT    usSegLength,               // length of data segment
  PSZ_W     pucSeg                     // data of segment
);

//+----------------------------------------------------------------------------+
//|  Lookup control block (LUPCB)                                              |
//+----------------------------------------------------------------------------+
#define LUPCB_SIGNATURE 0x363564CB     // used to identify and check LUPCBs

typedef struct _LUPCB                  // lookup control block
{
   LONG     lSignature;                // LUPCB signature
   FARPROC   lpfnLookupProc;           // ProcInstance for lookup dialog
   FARPROC   lpfnDispProc;             // ProcInstance for display dialog
   USHORT   usID;                      // unique ID of control block
   HUCB     hUCB;                      // ptr to Asd user control block
   HDCB     hDCB;                      // ptr to Asd dictionary control block
   HWND     hwndParent;                // parent window for dialogs
   USHORT   usNotifyMsg;               // message used for notifications
   HWND     hwndLook;                  // handle of lookup dialog
   HWND     ahwndDisplay[MAX_DISP_DLGS]; // buffer for display entry dialog handles
   USHORT   usDisplayDlgs;             // number of active display dialogs
   RECTL    rclDisp;                   // default position for display term dlg
   RECTL    rclEdit;                   // default position for edit term dlg
   CHAR_W   ucTerm[MAX_TERM_LEN+1];    // term to be looked up
   ULONG    ulFlags;                   // flags to be used for lookup services
} LUPCB, *PLUPCB;

/**********************************************************************/
/* Functions contained in EQFDISP.C which are reused by other         */
/* dictionary service modules                                         */
/**********************************************************************/
//BOOL FillDBCSTable( BOOL     fisDBCS1[] );
BOOL BuildLine
(
   PSZ_W    pszLineBuf,                // ptr to output buffer
   ULONG    ulMaxColumns,              // max number of character columns
   USHORT   usLineSize,                // size (width) of line
   PSZ_W    pszFieldName,              // name of entry field
   PBOOL    pfFirstTime,               // TRUE if called for the first time
   PSZ_W    *ppData,                   // ptr to entry data pointer
   PUSHORT  pusDataLen                // length of data area
   /*BOOL     fIsDBCS1[]   */             // ptr to DBCS flag array
);
__declspec(dllexport)
BOOL FillMLE
(
   HWND      hwndMLE,                  // handle of MLE
   USHORT    usMLECharWidth,           // character width of MLE
   PSZ_W     pucMLEBuffer,             // buffer for MLE data
   USHORT    usBufSize,                // size of MLE buffer
   HDCB      hDCB,                     // dictionary handle
   PVOID     hLDBTree,                 // LDB tree handle
   USHORT    usDisplayLevel           // display level
   // BOOL      fisDBCS1[]                // DBCS character flag array
);

__declspec(dllexport)
BOOL SetFixedPitchMLEFont
(
   HWND      hwndMLE,                  // handle of MLE
   BOOL      fDBCS,                    // TRUE = we are running in DBCS environment
   PUSHORT   pusMLECharWidth           // ptr to buffer for MLE character width
);


USHORT            SearchAndEdit( HWND, PLUPCB, PSZ_W, HMODULE, HDCB );
VOID              GetCharXY( HPS, PUSHORT, PUSHORT);
BOOL DisplayEntry ( PLUPCB, PSZ_W, HMODULE, HWND, PSZ );
BOOL TermLookup( PLUPCB, PSZ_W, HMODULE, HWND );

/**********************************************************************/
/* Definitions required to process dictionary data in old LDB         */
/* format                                                             */
/**********************************************************************/
#define ABSOLUTE_MIN_NUM_NODES 10
#define UNSPEC_TERM_NAME        "Terminal-Node"
#define SEG_SIZE                4
typedef struct _NAMEENTRY
{
   USHORT  usCodeName;                 // code name
   UCHAR   ucNameLength;               // length of following name
   UCHAR   ucSysName[1];               // variable length system name field
} NAMEENTRY, *PNAMEENTRY;


BOOL LupRegisterDisplayDlg
(
   PLUPCB    pLUPCB,                   // pointer to lookup control block
   HWND      hwndDlg                   // handle of display dialog
);

BOOL LupUnregisterDisplayDlg
(
   PLUPCB    pLUPCB,                   // pointer to lookup control block
   HWND      hwndDlg                   // handle of display dialog
);

BOOL LupAskForEdit
(
  HWND             hwnd,
  PLUPCB           pLUPCB,
  PSZ_W            pucTerm
);

USHORT AsdMorphRCToNlp
(
  USHORT   usMorphRC                   // return code from Morph functions
);

/**********************************************************************/
/* prototype for WordPerfect EditControl creation                     */
/**********************************************************************/

HWND CreateWPWindow
(
  HWND hwndParent,
  USHORT usId,
  ULONG  ulEntryStyle,
  USHORT usCharacterLimit,
  USHORT usBufferSize
);

USHORT CopyStringListUnicode2ASCII
(
  PSZ_W    pListW,                      // ptr to ASCII termlist
  LONG     lListSize,                 // size of List
  PSZ     *ppList,                   // ptr to term list pointer
  PLONG    plSize,                    // ptr to allocated size of term list
  PLONG    plUsed                    // ptr to used bytes in term list
);
