/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef OTMAPI_INCLUDED
#define OTMAPI_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************/
/* 32bit / 16bit compatibility defines (only if EQF.H has not been    */
/* included yet)                                                      */
/*                                                                    */
/* For a 32bit environment APIENTRY16 is defined in the OS2DEF.H file */
/* (included by OS2.H)                                                */
/**********************************************************************/
#ifndef _EQF_H_INCLUDED
  #if defined(WIN32BIT)
    #define APIENTRY16 APIENTRY
    #define HAB             HANDLE
    typedef USHORT  EQF_BOOL;
    typedef PUSHORT PEQF_BOOL;

    #ifndef _WCHAR_T_DEFINED
    typedef unsigned short wchar_t;
    #define _WCHAR_T_DEFINED
  #endif

  #ifndef _WCTYPE_T_DEFINED
    typedef wchar_t wint_t;
    typedef wchar_t wctype_t;
    #define _WCTYPE_T_DEFINED
  #endif

  #ifndef _CHAR_W_DEFINED
    typedef wchar_t CHAR_W;
    typedef wchar_t * PCHAR_W;
    typedef wchar_t * PSZ_W;
    #define _CHAR_W_DEFINED
  #endif
  #elif !defined(APIENTRY16)
    typedef BOOL  EQF_BOOL;
    typedef PBOOL PEQF_BOOL;
    #define APIENTRY16 APIENTRY
    #define HAB             HINSTANCE
  #else
    typedef USHORT  EQF_BOOL;
    typedef PUSHORT PEQF_BOOL;
  #endif
#endif

/*------------------------------------------------------------------------------
* EQF Common Definitions
*-----------------------------------------------------------------------------*/
// --- Counts ----------------------------------------------------------------

  #define EQF_NPROP_TGTS  9
  #define EQF_NPROP_NOTALL_TGTS  3


#define EQF_NSAB         3						 // max. number of segments that
 												 // can be stored in the internal
 												 // buffer

  #define EQF_NDICT_TGTS  52                     // max. number of dict.targets

                                                // for one source segment that
                                                // can be retrieved with the
                                                // function EQFGETDICT
//#define EQF_MAX_TM_BASES 5                      // max. number of translation
#define EQF_MAX_TM_BASES 25                      // max. number of translation
                                                // memory database files
#define EQF_NUM_IND_CODES 10                    // max. number of industry
                                                // codes for transl.mem.  002mk
// --- Buffer Sizes ----------------------------------------------------------
#define EQF_NAMELEN     256                     // CCHMAXPATHCOMP from 'bsedos.h'
#define EQF_MSGBUF_SIZE 260                     // CCHMAXPATH from 'bsedos.h'
#define EQF_SNAMELEN    12                      // short filename
#define EQF_LNAMELEN    255                     // long file name
#define EQF_SEGLEN      2048                    // max. segment size
#define EQF_SRCLEN      EQF_SEGLEN              // max. length of source seg.
#define EQF_TGTLEN      EQF_SEGLEN*EQF_NPROP_TGTS // max. lgt. of target segs.
#define EQF_DICTLEN     EQF_SEGLEN*4            // length of dict.words buffer
#define EQF_USERLEN     2048                    // length of user buffer (see
// --- Flags -----------------------------------------------------------------
// for EQF_INIT
#define EQFF_TM_CONF    1                       // flag: TM configured
#define EQFF_DA_CONF    2                       // flag: DA configured
#define EQFF_MT_CONF    4                       // flag: MT configured

// for EQF_CLEAR, EQF_TRANSSEG
#define EQFF_STANDARD   0                       // standard configuration (with
                                                // dictionary and proposals wnd.
                                                // and automatic dict.lookup)
#define EQFF_NODICTWND  1                       // flag: display no dict.window
#define EQFF_NOPROPWND  2                       // flag: display no prop.window
// for EQF_TRANSSEG
#define EQFF_NOAUTODICT 4                       // flag: no auto. dict.lookup
#define EQFF_EXACTANDFUZZY  8                   // displ. exact and fuzzy match
/**********************************************************************/
/* return only one exact match if there is only exactly one available */
/* if there are more than one exact one available NO one is returned  */
/* This option should only be used in Automatic Translation...        */
/**********************************************************************/
#define EQFF_ONLYONEEXACT  16                    // return exact iff only one
#define EQFF_EXACTCONTEXT  32                    // use exact context match only

#define EQFF_NOGENERICREPLACE 64                 // disable generic replace
#define EQFF_ALLEXACTONES  128                   // return all exact proposals
#define EQFF_MOREPROPINDIC 256                   // indicate if more proposals are avail
#define EQFF_NOSEGPROPWND  512                   // flag: hide segment properties window

// Return value if EQFF_MOREPROPINDIC is set
#define EQFF_MORE_EXACTS_AVAIL      0x8000
#define EQFF_ADDITIONAL_FUZZY_AVAIL 0x4000

// --- Parameters ------------------------------------------------------------
// for EQFGETPROP, EQFGETDICT
#define EQF_SCROLL_UP            100            // scroll up dict/prop window
#define EQF_SCROLL_DOWN          101            // scroll down dict/prop wnd.
#define EQF_ACTIVATE             102            // set focus to dict/prop.window
#define EQF_IS_AVAIL             103            // check if window is available
#define EQF_GETPREFIXES          104            // get the prefixes for Props
#define EQF_GETLCS               105            // get the lcs of best src-of-prop
#define EQF_GETMARKEDBLOCK       106            // get data marked in dict/propwnd
#define EQF_SCROLL_PREFIX_UP     107            // scroll up dict prefixes
#define EQF_SCROLL_PREFIX_DOWN   108            // scroll down dict prefixes

/**********************************************************************/
/* constants used in EQFM_DOC_STATUS message                          */
/**********************************************************************/
#define  EQF_DOC_COMPLRATE            1         // completion rate of document
#define  EQF_DOC_SEGFROMSCRATCH       2         // segments xlated from scratch
#define  EQF_DOC_SEGMODIFIED          3         // segments modified during xlat
#define  EQF_DOC_SEGCOPIED            4         // segments copied w/o modificat
#define  EQF_DOC_XLATED               5         // document is xlated
#define  EQF_DOC_ULATED               6         // document is not translated
#define  EQF_DOC_NOTTOUCHED           7         // document not touched
#define  EQF_DOC_UPDATED              8         // document updated

/**********************************************************************/
/* constants used in EQFFILECONVERSION type                          */
/**********************************************************************/
#define  EQF_ASCII2ANSI           1         // convert from ascii to ansi
#define  EQF_ANSI2ASCII           2         // convert from ansi to ascii
#define  EQF_ASCII2UTF8			3			// convert from ascii to UTF8
#define  EQF_UTF82ASCII			4			// convert from UTF8 to ascii
#define  EQF_ASCII2UTF16		5			// convert from ASCII to UTF16
#define  EQF_UTF162ASCII		6			// convert from UTF16 to ASCII
#define  EQF_ANSI2UTF8			7			// convert from ANSI to UTF8
#define  EQF_UTF82ANSI			8			// convert from UTF8 to ANSI
#define  EQF_ANSI2UTF16			9			// convert from ANSI to UTF16
#define  EQF_UTF162ANSI			10			// convert from UTF16 to ANSI
#define  EQF_UTF82UTF16			11			// convert from UTF8 to UTF16
#define  EQF_UTF162UTF8			12			// convert from UTF16 to UTF8
#define  EQF_UTF162UTF8BOM	13			// convert from UTF16 to UTF8 and add UTF-8 BOM

/*------------------------------------------------------------------------------
* EQF Messages
*-----------------------------------------------------------------------------*/
#define EQFM_DOC_IS_LOADED      WM_USER+0x500   // post to TWB, when editor has
                                                // loaded the file(s)
#define EQFM_DOC_IS_XLATED      WM_USER+0x501   // post to TWB when editor has
                                                // determined that document is
                                                // translated
#define EQFM_DOC_IS_SAVED       WM_USER+0x502   // post to TWB when editor
                                                // finished saving the document
#define EQFM_DOC_IS_ULATED      WM_USER+0x503   // post to TWB when editor has
                                                // detemined that document is
                                                // not completly translated
// shows a HTML text file in a HTML control
#define WM_EQF_SHOWHTML         2131

/**********************************************************************/
/*  post to TWB containing status information about document          */
/* MP1:  EQF_DOC_COMPLRATE                                            */
/*       EQF_DOC_SEGFROMSCRATCH                                       */
/*       EQF_DOC_SEGMODIFIED                                          */
/*       EQF_DOC_SEGCOPIED                                            */
/* MP2:  USHORT  indicating number of segments                        */
/**********************************************************************/
#define EQFM_DOC_STATUS         WM_USER+0x504   // post to TWB containing status


/*------------------------------------------------------------------------------
* EQF Names
*-----------------------------------------------------------------------------*/
#define EQFNM_SHSEG     "/SHAREMEM/OTM/OTMSTGEN"   // name of shared memory

/*------------------------------------------------------------------------------
* EQF Commands
*-----------------------------------------------------------------------------*/
#define EQFCMD_INIT             0               // initialize TWBS session
#define EQFCMD_CLEAR            1               // clear buffers and queues
#define EQFCMD_CLOSE            2               // close TWBS session
#define EQFCMD_GETPROP          3               // get TM proposal
#define EQFCMD_GETDICT          4               // get dictionary match
#define EQFCMD_DICTLOOK         5               // invoke dict.lookup dialog
#define EQFCMD_SAVESEG          6               // save segment to TM
#define EQFCMD_TRANSSEG         7               // translate a segment
#define EQFCMD_DELSEG           8               // delete a segment from TM
#define EQFCMD_EXTSEG           9               // extract segment from TM
#define EQFCMD_DUMPSEG          10              // dump all segment data
#define EQFCMD_PROOF            11              // proofread a segment
#define EQFCMD_PROOFADD         12              // add word to addenda
#define EQFCMD_PROOFAID         13              // display help for file
#define EQFCMD_GETSOURCE        14              // display source of proposal
#define EQFCMD_DICTEDIT         15              // invoke dict.edit dialog
#define EQFCMD_ADDABBREV        16              // add word to abbreviation dict
#define EQFCMD_DELPROP          17              // delete a proposal
#define EQFCMD_XDOCACT          18              // activate a new document env.
#define EQFCMD_XDOCADD          19              // add a new document to env.
#define EQFCMD_XDOCREMOVE       20              // remove a document from env.
#define EQFCMD_XDOCNEXT         21              // iterate through list of docs
#define EQFCMD_XDOCNUM          22              // get i-th document (i=0...n)
#define EQFCMD_XDOCINLIST       23              // check if document already in list
#define EQFCMD_WORDCNTPERSEG    24              // count words in segment
#define EQFCMD_GETSEGNUM        25              // get current segment number

#define EQFCMD_GETPROPW         26              // get TM proposal (Unicode version)
#define EQFCMD_SAVESEGW         27              // save segment to TM (Unicode version
#define EQFCMD_TRANSSEGW        28              // translate a segmen (Unicode version)
#define EQFCMD_DELSEGW          29              // delete a segment from TM (Unicode version)
#define EQFCMD_GETDICTW         30              // get dictionary match (Unicode version)
#define EQFCMD_DICTLOOKW        31              // invoke dict.lookup dialog (Unicode version
#define EQFCMD_EXTSEGW          32              // extract segment from TM (Unicode version)
#define EQFCMD_PROOFW           33              // proofread a segment (Unicode version)
#define EQFCMD_PROOFADDW        34              // add word to addenda (Unicode version)
#define EQFCMD_PROOFAIDW        35              // display help for file (Unicode version)
#define EQFCMD_DUMPSEGW         36              // dump all segment data (Unicode version)
#define EQFCMD_ADDABBREVW       37              // add word to abbreviation dict (Unicode version)
#define EQFCMD_DICTEDITW        38              // invoke dict.edit dialog (Unicode version)
#define EQFCMD_SAVESEG2W        39              // save segment to TM w/ context
#define EQFCMD_TRANSSEG2W       40              // translate a segment w/ context


/*------------------------------------------------------------------------------
* EQF Return Codes
*-----------------------------------------------------------------------------*/
#ifndef EQFRC_OK

#define EQFRC_OK                      0         // successfully completed
#define EQFS_DA_ERROR               525         // dictionary access error
#define EQFS_BAD_DICT               526         // dictionary might be corrupted
#define EQFS_DICT_MEM_ERR           527         // memory shortage
#define EQFS_BAD_DICT_NAME          528         // name of dictionary invalid
#define EQFS_FILE_ACC_DENIED        529         // dict. cannot be accessed
#define EQFS_FILE_NOT_FOUND         530         // file not found
#define EQFS_FILE_OPEN_FAILED       531         // open error
#define EQFS_FILE_SHARING_VIOL      532         // sharing violation
#define EQFS_DICT_WRITE_ERR         533         // write error
#define EQFS_DICT_ALREADY_OPEN      534         // dict already open
#define EQFS_TOO_MANY_DICTS         535         // too many dictionaries

#define ERROR_MEM_IN_EXCL_USE        71         // memory in exclusive use
#define ERROR_MEM_UNDEFINED          74         // unexpected error occured
#define ERROR_MEM_VERSION_MISMATCH   75         // version mismatch
#define ERROR_MEM_MIGHT_BE_CORRUPTED 84         // memory might be corrupted
#define ERROR_MEM_DISK_FULL          85         // disk full
#define ERROR_MEM_DB_FULL            86         // database full
#define ERROR_MEM_NOT_REPLACED       93         // replace of segment failed
#define EQFS_TM_NOT_FOUND           515         // translation memory not found

#define EQFRS_INVALID_PARM          506         // passed parameters are invalid
#define EQFRS_ENTRY_NOT_AVAIL       510         // requested entry not available

#define EQFRS_AREA_TOO_SMALL        522         // passed area too small
#define EQFRS_NOMORPH_DICT          523         // no morphological dictionary
#define EQFRS_NOADDENDA_DICT        524         // no addenda dictionary
#define EQFRS_NOPROOF_SERVICE       500         // no morphological services

#define EQFRS_NOT_AVAILABLE        1000         // requested window is not visible

#define ERROR_STORAGE                1          // nec area cannot be allocated
#define ERROR_FILE_INVALID_DATA     45          // segmented file contains error
#endif

/**********************************************************************/
/* colors used for foreground/background settings in EQFTEXTPANEL     */
/**********************************************************************/
#define    EQF_BLACK                0
#define    EQF_BLUE                 1
#define    EQF_GREEN                2
#define    EQF_CYAN                 3
#define    EQF_RED                  4
#define    EQF_PINK                 5
#define    EQF_BROWN                6
#define    EQF_LIGHTGRAY            7
#define    EQF_DARKGRAY             8
#define    EQF_LIGHTBLUE            9
#define    EQF_LIGHTGREEN          10
#define    EQF_LIGHTCYAN           11
#define    EQF_LIGHTRED            12
#define    EQF_LIGHTPINK           13
#define    EQF_YELLOW              14
#define    EQF_WHITE               15

#define    EQF_HORZSCROLL          0x01L
#define    EQF_VERTSCROLL          0x02L

/*------------------------------------------------------------------------------
* EQF Typedefs
*-----------------------------------------------------------------------------*/

typedef struct _STEQFPCMD                       // Pipe Command/Response
{
  USHORT        usCmd;                          // command code
  ULONG         ulParm1;                        // parameter 1
  USHORT        usParm2;                        // parameter 2
  USHORT        usParm3;                        // parameter 3
  USHORT        usLen1;                         // length of buffer 1
  USHORT        usLen2;                         // length of buffer 2
  USHORT        usLen3;                         // length of buffer 3
  USHORT        usLen4;                         // length of buffer 4
  UCHAR         ucbBuffer [EQF_SEGLEN*8];       // buffer1/2/3/4
  PVOID         pvMetaData;                     // ptr to segment meta data
} STEQFPCMD;

typedef STEQFPCMD *PSTEQFPCMD;



typedef struct _STEQFGEN                        // generic EQF structure
{
  HWND          hwndTWBS;                       // handle of parent window
                                                // (either a client window or
                                                // HWND_DESKTOP)

  HWND          hwndEditorSrc;                  // handle of editor src.window
  HWND          hwndEditorTgt;                  // handle of editor tgt.window

  RECTL         rclEditorSrc,                   // coordinates of src.window
                rclEditorTgt;                   // coordinates of tgt.window
  RECTL         rclDispPos,                     // coordinates of disp.dict.
                rclProposals,                   // coordinates of prop.window
                rclDictionary,                  // coordinates of dict.window
                rclSource;                      // coordinates of prop.src.wnd.
  UCHAR         szFileName[EQF_SNAMELEN+1],     // current translated file
                chSysDrive;                     // working drive for EQF system
  EQF_BOOL      fUseCoords;                     // coordinates valid
  ULONG         fsConfiguration;                // activation of TM/DA/Tbox
  USHORT        usNumIndus;                     // no. of ind.codes (TM)  002mk
  USHORT        ausIndCodes[EQF_NUM_IND_CODES]; // ind. codes list  (TM)  002mk
                                                // level which is to be con-
                                                // sidered as exact
                                                // (see 'eqftmdef.h')
  UCHAR         ucbUserArea[EQF_USERLEN];       // user buffer
                                                // (can e.g. be used for the
                                                // editor to store/retrieve
                                                // data)
  UCHAR         szTagTable[EQF_NAMELEN],        // used tag table       009mk
                szEQFTagTable[EQF_NAMELEN];     // used EQF tag table   009mk
  LONG          lExactMatchLevel;               // used to configure the match
  // new fields
  USHORT        usRC;                           // return code
  CHAR_W        szMsgBuffer[EQF_NAMELEN];       // message buffer
  PVOID         pDoc;                           // reserved
  PSTEQFPCMD    pstEQFPCmd;                     // reserved pipe command struct
  ULONG         flPropStyle;                    // window style of prop Window
  ULONG         flDictStyle;                    // window style of dict.window
  ULONG         flSrcStyle;                     // window style of prop.src.wnd.
  ULONG         flEditSrcStyle;                 // window style of edit.src.wnd.
  ULONG         flEditTgtStyle;                 // window style of edit.tgt.wnd.
  ULONG         flEditOtherStyle;               // window style of other.wnd.
  PVOID         pOpenAndPos;                    // reserved for Open/Pos struct
  RECTL         rclShowWndPos;                  // coordinates of show window pos.
  UCHAR         szLongName[EQF_LNAMELEN+1];     // long name of current file
  BYTE          bStatusBar[4];                  // status bar window indication

  PSZ          pszCurSpellCheckDoc;             // points to current document in pszSpellCheckDocList or NULL if not in "spellcheck several documents" mode
  PSZ          pszSpellCheckDocList;            // list of documents being checked for spelling or NULL if not in "spellcheck several documents" mode
  BOOL         fLoadedBySpellcheck;             // TRUE = document is being loaded by spellchecker
  PVOID        pNewSpellCheckDoc;               // new document openend for spellchecking
  SHORT        xSpellChecklDlg;                 // last x position of spellcheck dialog
  SHORT        ySpellChecklDlg;                 // last y position of spellcheck dialog
  PVOID        pvSpellData;                     // spell data pointer when in "spellcheck several documents" mode

  BYTE          bBuffer[172];                   // buffer area for later use ...
} STEQFGEN;

typedef STEQFGEN  *PSTEQFGEN;          // see struct typedefs

/**********************************************************************/
/* structure for direct translation                                   */
/**********************************************************************/

typedef struct _EQFXLATE
{
  HWND  hwnd;                                     // window handle
  EQF_BOOL  fBusy;                                // currently busy
  EQF_BOOL  fUnSegment;                           // unsegment file
  CHAR  szFolderName[ EQF_NAMELEN ];              // folder name
  CHAR  szDocumentName[ EQF_NAMELEN ];            // document name
  CHAR  szOutName[ EQF_NAMELEN ];                 // document name for output
  USHORT usRc;                                    // return code
  USHORT  usState;
  HWND    hwndTWB;
  HAB     hab;
} EQFXLATE, * PEQFXLATE;

typedef struct _countinout         //count structure, used for data exchange
{                                  //between count handler and user exit
  // in part
  CHAR    szFolderObjName[ 157 ];  //folder path+name
  CHAR    szFolderName[ 13 ];      //folder name
  CHAR    szCountFileName[ 13 ];   //file to be counted name
  EQF_BOOL fOrg;                   //count of original requested?
  EQF_BOOL fTran;                  //count of translation requested?
  // out part
  ULONG   ulWToBeTran;             //number of words to be translated
  ULONG   ulWTransMod;             //number of translated words modified
  ULONG   ulWTransCopied;          //number of translated words copided
  ULONG   ulWTransScratch;         //number of translated words (from scratch)
  ULONG   ulWMarkup;               //number of markup words
  ULONG   ulWTotal;                //number of total words in file
  ULONG   ulWTransSource;          //number of transl. words in source file
} COUNTINOUT, *PCOUNTINOUT;

/**********************************************************************/
/* possible return codes                                              */
/**********************************************************************/
#define EQFXLATE_OK                   0
#define EQFXLATE_TRANSL_ERROR         1
#define EQFXLATE_UNSEGM_ERROR         2
#define EQFXLATE_INVPARM_ERROR        3
#define EQFXLATE_NOTRUNNING_ERROR     4
#define EQFXLATE_PARMTOOLONG_ERROR    5




/*******************************************************************************
*
* Structure for Word Count Information
*
*******************************************************************************/

// summery of words per class
#pragma pack(1)
typedef struct _APISumPerClass
{
   USHORT  usNumSegs;                  // number of segs in this class
   ULONG   ulSrcWords;                 // sum of all src words
   ULONG   ulTgtWords;                 // sum of all target words
}  APISUMPERCLASS, *PAPISUMPERCLASS;



// a criteria is, what the kind best proposal is:
#pragma pack(1)
typedef struct _APICriteriaSum
{
   APISUMPERCLASS  SimpleSum;             // number of segs in this class
   APISUMPERCLASS  MediumSum;             // number of segs in this class
   APISUMPERCLASS  ComplexSum;            // number of segs in this class
}  APICRITERIASUM, *PAPICRITERIASUM;


// additional counting information
//--------------------------------

#pragma pack(1)
typedef struct _APIDocSaveHist
{
   APICRITERIASUM  EditAutoSubst;         // sums for segs transl. by editauto
   APICRITERIASUM  ExactExist;            // sums for segs with exact prop
   APICRITERIASUM  ExactUsed;             // sums for segs with exact prop used by translator
   APICRITERIASUM  FuzzyExist;            // sums for segs with fuzzy prop
   APICRITERIASUM  FuzzyUsed;             // sums for segs with fuzzy prop used by translator
   APICRITERIASUM  FuzzyExist_1;          // sums for segs with fuzzy prop
   APICRITERIASUM  FuzzyUsed_1;           // sums for segs with fuzzy prop used by translator
   APICRITERIASUM  FuzzyExist_2;          // sums for segs with fuzzy prop
   APICRITERIASUM  FuzzyUsed_2;           // sums for segs with fuzzy prop used by translator
   APICRITERIASUM  FuzzyExist_3;          // sums for segs with fuzzy prop
   APICRITERIASUM  FuzzyUsed_3;           // sums for segs with fuzzy prop used by translator
   APICRITERIASUM  MachExist;             // sums for segs with machine prop.
   APICRITERIASUM  MachUsed;              // sums for segs with machine prop. used by translator
   APICRITERIASUM  NoneExist;             // sums for segs with no proposal
   APICRITERIASUM  NotXlated;             // sums for TOBE, ATTR, CURRENT
}  APIDOCSAVEHIST, *PAPIDOCSAVEHIST;


typedef struct _SEGINFO
{
   USHORT usSrcWords;
   USHORT usTgtWords;
   USHORT usModWords;
   USHORT usSegNum;
   USHORT usTyped;
   USHORT usCopied;
   USHORT usTypeExist;
   USHORT usTypeCopied;
   SHORT  sStatus;
   SHORT  sCurrent;
   SHORT  sNoCount;
   SHORT  sDummy1;
   SHORT  sDummy2;
   SHORT  sDummy3;
}  SEGINFO, *PSEGINFO;


/*------------------------------------------------------------------------------
* Function Prototypes
*-----------------------------------------------------------------------------*/



/*******************************************************************************
*
*       function:       EQFSETSLIDER
*
* -----------------------------------------------------------------------------
*       Description:
*               Set the slider position of the text analysis window
*       Input:  HWND    slider handle (as passed by EQFPRESEG or EQFPOSTSEG)
*               USHORT  slider position value ( 0 ... 100 )
*       Output: None
*******************************************************************************/
__declspec(dllexport) 
VOID __cdecl EQFSETSLIDER( HWND, USHORT );

/*******************************************************************************
*
*       function:       EQFINIT
*
* -----------------------------------------------------------------------------
*       Description:
*               Initializes a session with EQF services. EQFSRV.EXE must have
*               been started before.
*       Input:  PSTEQFGEN...pointer to generic structure, NULL if the generic
*                       structure in shared memory is used directly (see
*                       function EQFSHMEM).
*               PSZ *...pointer to array of strings with translation memory
*                       database filenames
*               PSZ *...pointer to array of strings with names of user supplied
*                       dictionaries
*       Output: Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFINIT (PSTEQFGEN pstEQFGeneric,
                         PSZ       *apszTranslationMemories,
                         PSZ       *apszUserDictionaries);


/*******************************************************************************
*
*       function:       EQFCLEAR
*
* -----------------------------------------------------------------------------
*       Description:
*               Clears the internal buffers of EQF services and allows to
*               hide dictionary / proposals window
*       Input:  USHORT...FLAGS: EQFF_NOPROPWND - hide proposals window
*                               EQFF_NODICTWND - hide dictionary window
*       Output: Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFCLEAR (USHORT fsFlags);


/*******************************************************************************
*
*       function:       EQFCLOSE
*
* -----------------------------------------------------------------------------
*       Description:
*               Close EQF services. Depending on the parameter, the services
*               remain in memory and can be initialized again, or they are
*               totally closed.
*       Input:  BOOL...EQF_CLOSE_STANDBY - services session is closed, services
*                                          remain active
*                      EQF_CLOSE_EXIT    - services are closed and destroyed
*       Output: Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFCLOSE (EQF_BOOL fShutdown);


/*******************************************************************************
*
*       function:       EQFGETPROP
*
* -----------------------------------------------------------------------------
*       Description:
*               Performs several function on the proposals window (see
*               parameters).
*       Input:  USHORT...EQF_SCROLL_UP/DOWN - scroll proposals window
*                        EQF_ACTIVATE - set focus to proposals window
*                        EQF_IS_AVAIL - checks if window is avail
*                        EQF_GETPREFIXES -  get the prefixes for Props
*                        num - retrieve text of proposal no. <num> for
*                              currently active segment from EQF services
*               PSZ...pointer to buffer for output text (for num, GETPREFIXES
*                       may be NULL for SCROLL/ACTIVATE)
*               PUSHORT...pointer to variable for return match level (for num,
*                       may be NULL if not used)
*               If the match level variable pointed to by pusLevel contains
*               the magic value QUERYIFSOURCEISEQUAL (0x49F3) the first bit
*               of the returned match level (mask with 0x8000) is used to
*               indicate if the source of the returned proposal is equal to
*               the source segment.
*
*       Output: PSZ...buffer with output text (for num)
*               PUSHORT...return match level (for num)
*               Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFGETPROP (USHORT  usNum,
                              PSZ     pszBuffer,
                              PUSHORT pusLevel);

__declspec(dllexport)
USHORT __cdecl EQFGETPROPW (USHORT  usNum,
                               PSZ_W   pszBuffer,
                               PUSHORT pusLevel);


// magic word to get the proposal-source-is-equal-to-source flag
#define QUERYIFSOURCEISEQUAL 0x49F3

/*******************************************************************************
*
*       function:       EQFGETSOURCE
*
* -----------------------------------------------------------------------------
*       Description:
*               Performs several function on the source of proposal window
*       Input:  USHORT...
*                        EQF_ACTIVATE - activates the source of prop. window
*        Output: None
*               Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFGETSOURCE (USHORT usNum );

/*******************************************************************************
*
*       function:       EQFGETDICT
*
* -----------------------------------------------------------------------------
*       Description:
*               Performs several function on the dictionary window (see
*               parameters).
*       Input:  USHORT...EQF_SCROLL_UP/DOWN - scroll dictionary window
*                        EQF_ACTIVATE - set focus to dictionary window
*                        EQF_IS_AVAIL - checks if dictionary window is avail.
*                        num - retrieve text of dict.target no. <num> for
*                              currently active segment from EQF services
*               PSZ...pointer to buffer for output text ( num )
*                     may be NULL for SCROLL/ACTIVATE)
*       Output: PSZ...pointer to buffer with output text (num)
*               Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFGETDICT (USHORT usNum,
                              PSZ    pszBuffer);

__declspec(dllexport)
USHORT __cdecl EQFGETDICTW(USHORT usNum,
                              PSZ_W  pszBuffer);


/*******************************************************************************
*
*       function:       EQFDICTLOOK
*
* -----------------------------------------------------------------------------
*       Description:
*               Invoke dictionary lookup.
*       Input:  PSZ...pointer to segment containing the lookup term (not used
*                       in current version, may be an empty string)
*               PSZ...pointer to the lookup term
*               USHORT...position of lookup term within the segment (not used
*                       in current version)
*               BOOL...TRUE/FALSE - lookup term is in source/target language
*                      (not used in current version)
*       Output: Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFDICTLOOK (PSZ    pszSegmentBuffer,
                               PSZ    pszWord,
                               USHORT usPosition,
                               EQF_BOOL   fSource);
__declspec(dllexport)
USHORT __cdecl EQFDICTLOOKW(PSZ_W  pszSegmentBuffer,
                               PSZ_W  pszWord,
                               USHORT usPosition,
                               EQF_BOOL   fSource);



/*******************************************************************************
*
*       function:       EQFSAVESEG
*
* -----------------------------------------------------------------------------
*       Description:
*               Saves a translated segment to translation memory.
*       Input:  PSZ...pointer to source segment
*               PSZ...pointer to target (translated) segment
*               USHORT...segment number of src/tgt segment
*       Output: Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFSAVESEG (PSZ    pszSource,
                              PSZ    pszTarget,
                              USHORT usSegNum);
__declspec(dllexport)
USHORT __cdecl EQFSAVESEGW(PSZ_W  pszSource,
                              PSZ_W  pszTarget,
                              ULONG  ulSegNum);

__declspec(dllexport)
USHORT __cdecl EQFSAVESEG2W(PSZ_W  pszSource,
                               PSZ_W  pszTarget,
                               PSZ_W  pszContext,
                               ULONG  ulSegNum);
__declspec(dllexport)
USHORT __cdecl EQFSAVESEG3W(PSZ_W  pszSource,
                               PSZ_W  pszTarget,
                               PSZ_W  pszContext,
                               PVOID  pvMetaData,
                               ULONG  ulSegNum);



/*******************************************************************************
*
*       function:       EQFTRANSSEG
*
* -----------------------------------------------------------------------------
*       Description:
*               Translates a segment and display results if required (see
*               below).
*       Input:  PSZ...pointer to source segment
*               USHORT...segment number of source segment
*               BOOL...TRUE/FALSE - foreground/background segment, a background
*                       request is not displayed on screen, and the results are
*                       not available through the EQF... function. A background
*                       request is typically used for a send ahead.
*               USHORT...Flags: EQFF_STANDARD - standard configuration (TM and
*                                               dict.window, automatic dict.
*                                               lookup
*                               EQFF_NODICTWND - do not display dictionary wnd.
*                               EQFF_NOPROPWND - do not display proposals wnd.
*                               EQFF_NOAUTODICT - do not invoke automatic dict.
*                                               lookup, lookup will be invoked
*                                               only for a foreground request
*                                               where no exact TM match was
*                                               found
*                               EQFF_MOREPROPINDIC - the returned number of exact match
*                                               is ORed with EQFF_MORE_EXACTS_AVAIL or
*                                               EQFF_ADDITIONAL_FUZZY_AVAIL respectively
*               PSHORT...pointer to variable for returned matchlevel
*       Output: PSHORT...returns number of exact match (only for foreground
*                       request), -1 if no exact match
*                       If EQFF_MOREPROPINDIC is specified, the number of the exact match ORed with
*                       the indications EQFF_MORE_EXACTS_AVAIL and EQFF_ADDITIONAL_FUZZY_AVAIL
*                       will be returned
*               Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFTRANSSEG (PSZ    pszSource,
                               USHORT usSegNum,
                               EQF_BOOL   fForeground,
                               USHORT fsFlags,
                               PSHORT psExact);
__declspec(dllexport)
USHORT __cdecl EQFTRANSSEGW(PSZ_W  pszSource,
                               ULONG  ulSegNum,
                               EQF_BOOL   fForeground,
                               USHORT fsFlags,
                               PSHORT psExact);
__declspec(dllexport)
USHORT __cdecl EQFTRANSSEG2W(PSZ_W  pszSource,
                                PSZ_W  pszContext,
                                ULONG  ulSegNum,
                                EQF_BOOL   fForeground,
                                USHORT fsFlags,
                                PSHORT psExact);
__declspec(dllexport)
USHORT __cdecl EQFTRANSSEG3W(PSZ_W  pszSource,
                                PSZ_W  pszContext,
                                PVOID  pvMetaData,
                                ULONG  ulSegNum,
                                EQF_BOOL   fForeground,
                                USHORT fsFlags,
                                PSHORT psExact);


/*******************************************************************************
*
*       function:       EQFGETSEGNUM
*
* -----------------------------------------------------------------------------
*       Description:
*               Gets the number of the currently active segment
*       Input:  PULONG...pointer to buffer for segment number
*       Output: Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFGETSEGNUM (PULONG pulSegNum );

/*******************************************************************************
*
*       function:       EQFDELSEG
*
* -----------------------------------------------------------------------------
*       Description:
*               Deletes a segment from translation memory.
*       Input:  PSZ...pointer to source segment
*               PSZ...pointer to target segment
*               USHORT...segment number of source/target segment
*       Output: Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFDELSEG (PSZ    pszSource,
                             PSZ    pszTarget,
                             USHORT usSegNum);
__declspec(dllexport)
USHORT __cdecl EQFDELSEGW(PSZ_W  pszSource,
                             PSZ_W  pszTarget,
                             ULONG  ulSegNum);


/*******************************************************************************
*
*       function:       EQFEXTSEG
*
* -----------------------------------------------------------------------------
*       Description:
*               Extracts the source of a proposal from translation memory.
*       Input:  USHORT...number of proposal to be extracted
*               PSZ...pointer to buffer
*       Output: PSZ...extracted source for specified target (in buffer)
*               Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFEXTSEG (PSZ    pszBuffer,
                             USHORT usNum);
__declspec(dllexport)
USHORT __cdecl EQFEXTSEGW(PSZ_W  pszBuffer,
                             ULONG  ulNum);


/*******************************************************************************
*
*       function:       EQFGETTMNAME
*
* -----------------------------------------------------------------------------
*       Description:
*               Retrieves the translation memory name for a given folder
*       Input:  PSZ...pointer to folder name (with .F00 extension)
*               PSZ...pointer to folder path (e.g. 'G:\EQF\'
*               PSZ...pointer to buffer to hold translation memory name
*       Output: PSZ...pointer to translation memory name
*               Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFGETTMNAME (PSZ    pszFolder,
                                PSZ    pszPath,
                                PSZ    pszTMName);

/*******************************************************************************
*
*       function:       EQFSHMEM
*
* -----------------------------------------------------------------------------
*       Description:
*               Requests/releases access to EQF services shared memory (which
*               contains the generic structure).
*       Input:  PSTEQFGEN...pointer to shared memory - release memory access
*                           NULL - request memory access
*       Output: PSTEQFGEN...pointer to shared memory or NULL if either no
*                           access or access released
*******************************************************************************/
__declspec(dllexport)
PSTEQFGEN __cdecl EQFSHMEM (PSTEQFGEN pstEQFGen);


/*******************************************************************************
*
*       function:       EQFSTARTED
*
* -----------------------------------------------------------------------------
*       Description:
*               Checks whether the EQF services are running.
*       Input:  none
*       Output: BOOL...TRUE/FALSE - services are running / not running
*******************************************************************************/
__declspec(dllexport)
EQF_BOOL __cdecl   EQFSTARTED (VOID);


/*******************************************************************************
*
*       function:       EQFSYSRC
*
* -----------------------------------------------------------------------------
*       Description:
*               Retrieves the system returncode for the last call to EQF
*               services.
*       Input:  none
*       Output: System Returncode
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFSYSRC (VOID);


/*******************************************************************************
*
*       function:       EQFERRCLASS
*
* -----------------------------------------------------------------------------
*       Description:
*               Retrieves the error class for the last call to EQF services.
*       Input:  none
*       Output: Errorclass
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFERRCLASS (VOID);


/*******************************************************************************
*
*       function:       EQFERRID
*
* -----------------------------------------------------------------------------
*       Description:
*               Retrieves the error ID for the last call to EQF services (refers
*               to the message file of the TROJA system).
*       Input:  none
*       Output: error ID
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFERRID (VOID);


/*******************************************************************************
*
*       function:       EQFERRMSG
*
* -----------------------------------------------------------------------------
*       Description:
*               Retrieves the error message for the last call to EQF services
*               (uses the TROJA message file, with the ID given by EQFERRID).
*       Input:  none
*       Output: pointer to error message
*******************************************************************************/
__declspec(dllexport)
PSZ __cdecl    EQFERRMSG (VOID);
__declspec(dllexport)
PSZ_W __cdecl  EQFERRMSGW(VOID);


/*******************************************************************************
*
*       function:       EQFERRINS
*
* -----------------------------------------------------------------------------
*       Description:
*               Retrieves a possible insert item for the above retrieved
*               error message.
*       Input:  none
*       Output: pointer to insert item
*******************************************************************************/
__declspec(dllexport)
PSZ __cdecl    EQFERRINS       (VOID);
__declspec(dllexport)
PSZ_W __cdecl  EQFERRINSW      (VOID);

/**********************************************************************
*
*       function:       EQFDUMPSEG
*
* ---------------------------------------------------------------------
*       Description:
*               Dumps the proposals and dictionary hits for a specified
*               sentence
*       Input:
*               PSZ.....pointer to source buffer
*               PSZ.....pointer to file name
*               USHORT....segment number
*               USHORT....FALSE or EQFF_EXACTANDFUZZY
*       Output: PCHAR...pointer to proposal target buffer
*               PCHAR...pointer to dictionary buffer
*               Returncode (see EQFRC_... values)
***********************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFDUMPSEG (PSZ, PSZ, USHORT, PCHAR, PCHAR );
__declspec(dllexport)
USHORT __cdecl EQFDUMPSEG2(PSZ, PSZ, USHORT, USHORT, PCHAR, PCHAR);
__declspec(dllexport)
USHORT __cdecl EQFDUMPSEGW (PSZ_W, PSZ, ULONG, PCHAR_W, PCHAR_W );
__declspec(dllexport)
USHORT __cdecl EQFDUMPSEG2W(PSZ_W, PSZ, ULONG, USHORT, PCHAR_W, PCHAR_W);


/*/////////////////////////////////////////////////////////////////////
EQF_XSTART - start the user supplied editor
*//////////////////////////////////////////////////////////////////////
// Description:
//  This function will be invoked from Troja during the open of a document
//
//  Arguments:
//
//   pEQFProgPath: Program directory of troja
//   pSegSource  : segmented Source filename (full qualified)
//   pSegTarget  : segmented Target filename (full qualified)
//   pstEQFGen   : pointer to generic editor structure
//
//  Output:
//     fOK       : success indicator  TRUE/FALSE
///////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
EQF_BOOL __cdecl 
EQF_XSTART(  PSZ           pEQFProgPath,   // Program directory of troja
             PSZ           pSegSource  ,   // segmented Source file:
             PSZ           pSegTarget  ,   // segmented Target file:
             PSTEQFGEN     pstEQFGen );    // pointer to generic edit structure



/*/////////////////////////////////////////////////////////////////////
EQF_XSTOP - stop the user supplied editor
*//////////////////////////////////////////////////////////////////////
// Description:
//  This function will save the document or stop the editor
//  It is e.g. invoked if you select the WM_CLOSE item from the
//  system menu of the Translation environment window
//
//  Arguments:
//
//   pEQFProgPath: Program directory of troja
//   pstEQFGen   : pointer to generic editor structure
//   fKill       : boolean indication
//                 TRUE:  shut down the editor
//                 FALSE: save document only
//
//  Output:
//     fOK       : success indicator  TRUE/FALSE
///////////////////////////////////////////////////////////////////////
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
EQF_BOOL __cdecl 
EQF_XSTOP( PSZ       pEQFProgPath,              // Program directory of troja                                 //
           PSTEQFGEN pstEQFGen,                 // generic editor structure                        //
           EQF_BOOL  fKill) ;                   // shut down requested??



/*------------------------------------------------------------------------------
* Function Prototypes for segmentation/ unsegmentation user exit.
*
* The user exits makes it possible to do additional processing before/ after
* text analysis, or substitute text analysis at all.
* All four functions specified have to be provided in a dll.
* The dll name has to be specified in the tagtable used for Text Analysis.
*-----------------------------------------------------------------------------*/
#ifndef GML_TAG_LEN
  #define  GML_TAG_LEN       18
  typedef struct _TATAG1
  {
     CHAR chSTag[ GML_TAG_LEN ];      // start tag
     CHAR chETag[ GML_TAG_LEN ];      // end tag
  } TATAG, * PTATAG;

  typedef struct _TATAG1_W
  {
     CHAR_W chSTag[ GML_TAG_LEN ];      // start tag
     CHAR_W chETag[ GML_TAG_LEN ];      // end tag
  } TATAG_W, * PTATAG_W;

#endif
/*******************************************************************************
*
*       function:       EQFPRESEG/EQFPRESEG2
*
* -----------------------------------------------------------------------------
*       Description:
*               called before text segmentation is invoked. Prepares the source
*               file for segmentation and stores the prepared file temporarily.
*               Text Analysis will remove the temporary file.
*               EQFPRESEG2 will pass a slider handle which could be used
*               together with the EQFSETSLIDER call to set the slider in cases
*               of user exits
*
*       Arguments:   PSZ    ... pointer to markup table name (full path)
*
*                    PSZ    ... pointer to editor name
*
*                    PSZ    ... pointer to program path
*
*                    PSZ    ... pointer to source file name  (full path)
*
*                    PSZ    ... pointer to the buffer for the name of the
*                               temp output file, the buffer is allocated by Text
*                               Analysis, with a size of 144 byte.
*
*                    BOOL * ... output flag which indicates whether text
*                               segmentation shall be performed,
*                               TRUE: do not perform Text Analysis,
*                                     i.e. no segmentation of the text
*                                     provided temp. file will be copied
*                                     as segmented source and segmented target
*                               FALSE: do perform text analysis
*
*                   HWND    ... handle of the slider window
*
*                   PBOOL   ... ptr to 'kill' flag
*                               if this flag changes to TRUE the userexit
*                               should return ASAP
*
*       Return:     BOOL    ... TRUE: processing was OK
*                               FALSE: an error occured during processing,
*                                      e.g. a file could not be written, so
*                                      that no further processing should be done
*
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport)
EQF_BOOL __cdecl EQFPRESEG (
                PSZ pTagTable,
                PSZ   pEdit,
                PSZ   pProgPath,
                PSZ   pSource,
                PSZ   pTempSource,
                PEQF_BOOL   pfNoSegment);

__declspec(dllexport)
EQF_BOOL __cdecl EQFPRESEG2 (
                PSZ pTagTable,
                PSZ   pEdit,
                PSZ   pProgPath,
                PSZ   pSource,
                PSZ   pTempSource,
                PEQF_BOOL   pfNoSegment,
                HWND hSlider,
                PEQF_BOOL   pfKill );

__declspec(dllexport)
EQF_BOOL __cdecl EQFPRESEGEX (
                PSZ pTagTable,
                PSZ   pEdit,
                PSZ   pProgPath,
                PSZ   pSource,
                PSZ   pTempSource,
                PEQF_BOOL   pfNoSegment,
                HWND hSlider,
                PEQF_BOOL   pfKill,
                LONG  lAnalysisHandle );

#ifdef __cplusplus
}
#endif


/*******************************************************************************
*
*       function:       EQFPOSTSEG
*
* -----------------------------------------------------------------------------
*       Description:
*               called after text segmentation is invoked, to change the
*               segmented source and target file before translation takes place.
*               EQFPOSTSEG2 will pass a slider handle which could be used
*               together with the EQFSETSLIDER call to set the slider in cases
*               of user exits
*               EQFPOSTSEGW will assume that the segmented files are in Unicode
*
*       Arguments:   PSZ    ... pointer to markup table name
*
*                    PSZ    ... pointer to editor name
*
*                    PSZ    ... pointer to program path
*
*                    PSZ    ... pointer to segmented source file name
*
*                    PSZ    ... pointer to the segmented target file name
*
*                    PTATAG ... pointer to tags inserted by text segmentation,
*                               ( see layout of tag structure )
*
*                   HWND    ... handle of the slider window
*
*                   PBOOL   ... ptr to 'kill' flag
*                               if this flag changes to TRUE the userexit
*                               should return ASAP
*
*       Return:     BOOL    ... TRUE: processing was OK
*                               FALSE: an error occured during processing
*
*       Note: It is vital that the name of the segmented source and target file
*              is not changed!
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport)
EQF_BOOL __cdecl EQFPOSTSEG (
                 PSZ       pTagTable,
                 PSZ       pEdit,
                 PSZ       pProgPath,
                 PSZ       pSegSource,
                 PSZ       pSegTarget,
                 PTATAG    pTATag);

__declspec(dllexport)
EQF_BOOL __cdecl EQFPOSTSEG2 (
                 PSZ       pTagTable,
                 PSZ       pEdit,
                 PSZ       pProgPath,
                 PSZ       pSegSource,
                 PSZ       pSegTarget,
                 PTATAG    pTATag,
                 HWND      hSlider,
                 PEQF_BOOL pfKill );

__declspec(dllexport)
EQF_BOOL __cdecl EQFPOSTSEGEX (
                 PSZ       pTagTable,
                 PSZ       pEdit,
                 PSZ       pProgPath,
                 PSZ       pSegSource,
                 PSZ       pSegTarget,
                 PTATAG    pTATag,
                 HWND      hSlider,
                 PEQF_BOOL pfKill,
                 LONG      lAnalysisHandle );

__declspec(dllexport)
EQF_BOOL __cdecl EQFPOSTSEGW (
                 PSZ       pTagTable,
                 PSZ       pEdit,
                 PSZ       pProgPath,
                 PSZ       pSegSource,
                 PSZ       pSegTarget,
                 PTATAG_W  pTATagW,
                 HWND      hSlider,
                 PEQF_BOOL pfKill );

__declspec(dllexport)
EQF_BOOL __cdecl EQFPOSTSEGWEX (
                 PSZ       pTagTable,
                 PSZ       pEdit,
                 PSZ       pProgPath,
                 PSZ       pSegSource,
                 PSZ       pSegTarget,
                 PTATAG_W  pTATagW,
                 HWND      hSlider,
                 PEQF_BOOL pfKill,
                 LONG      lAnalysisHandle );

#ifdef __cplusplus
}
#endif


/********************************************************************
*
*       function:       EQFPOSTTM
*
* -------------------------------------------------------------------
*       Description:
*               called after translation memory matching was done,
*               to change the segmented source and/or segmented
*               target file before translation takes place.
*               A flag indicating if changes took place during the
*               insertion of translation memory matches is passed, too.
*
*               EQFPOSTTMW assumes that the segmented files to be read are in
*               Unicode.
*
*       Arguments:   PSZ    ... pointer to markup table name
*
*                    PSZ    ... pointer to editor name
*
*                    PSZ    ... pointer to program path
*
*                    PSZ    ... pointer to segmented source file name
*
*                    PSZ    ... pointer to the segmented target file name
*
*                    PTATAG ... pointer to tags inserted by text segmentation,
*                               ( see layout of tag structure )
*
*                    BOOL   ... flag indicating if segmented source different
*                               to segmented target
*
*                   HWND    ... handle of the slider window
*
*                   PBOOL   ... ptr to 'kill' flag
*                               if this flag changes to TRUE the userexit
*                               should return ASAP
*
*       Return:     BOOL    ... TRUE: processing was OK
*                               FALSE: an error occured during processing
*
*       Note: It is vital that the name of the segmented source and target file
*              is not changed!
***********************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport)
EQF_BOOL __cdecl EQFPOSTTM  (
                 PSZ       pTagTable,
                 PSZ       pEdit,
                 PSZ       pProgPath,
                 PSZ       pSegSource,
                 PSZ       pSegTarget,
                 PTATAG    pTATag,
                 EQF_BOOL  fChange);

__declspec(dllexport)
EQF_BOOL __cdecl EQFPOSTTM2 (
                 PSZ       pTagTable,
                 PSZ       pEdit,
                 PSZ       pProgPath,
                 PSZ       pSegSource,
                 PSZ       pSegTarget,
                 PTATAG    pTATag,
                 EQF_BOOL  fChange,
                 PULONG    pulSegsReplaced,
                 PULONG    pulSegsNotReplaced,
                 HWND      hSlider,
                 PEQF_BOOL pfKill );

__declspec(dllexport)
EQF_BOOL __cdecl EQFPOSTTMW (
                 PSZ       pTagTable,
                 PSZ       pEdit,
                 PSZ       pProgPath,
                 PSZ       pSegSource,
                 PSZ       pSegTarget,
                 PTATAG_W  pTATagW,
                 EQF_BOOL  fChange,
                 PULONG    pulSegsReplaced,
                 PULONG    pulSegsNotReplaced,
                 HWND      hSlider,
                 PEQF_BOOL pfKill );

#ifdef __cplusplus
}
#endif


/*******************************************************************************
*
*       function:       EQFPREUNSEG / EQFPREUNSEG2
*
* -----------------------------------------------------------------------------
*       Description:
*               called before unsegmentation is invoked, to change the
*               segmented target file and store the prepared file temporarily
*               before unsegmentation takes place.
*
*
*       Arguments:   PSZ    ... pointer to markup table name (full path)
*
*                    PSZ    ... pointer to editor name
*
*                    PSZ    ... pointer to program path
*
*                    PSZ    ... pointer to the segmented target file name (full path)
*
*                    PSZ    ... pointer to the buffer for the name of the
*                               temp output file, the buffer is allocated by Text
*                               Analysis, with a size of 144 bytes.
*
*                    PTATAG ... pointer to tags inserted by text segmentation
*
*                    BOOL * ... output flag whether unsegmentation is
*                               required
*                               TRUE: unsegmentation not required
*                               FALSE: unsegmentation required
*                               If unsegmentation is not required, only a file
*                               copy from the temp file to the target path
*                               will be done.
*
*                   PBOOL   ... ptr to 'kill' flag
*                               if this flag changes to TRUE the userexit
*                               should return ASAP
*
*       Return:     BOOL    ... TRUE: processing was OK
*                               FALSE: an error occured during processing
*
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

EQF_BOOL __cdecl EQFPREUNSEG (
                 PSZ       pTagTable,
                 PSZ       pEdit,
                 PSZ       pProgPath,
                 PSZ       pSegTarget,
                 PSZ       pTemp,
                 PTATAG    pTATag,
                 PEQF_BOOL pfNoUnseg);

__declspec(dllexport)
EQF_BOOL __cdecl EQFPREUNSEG2 (
                 PSZ       pTagTable,               
                 PSZ       pEdit,
                 PSZ       pProgPath,
                 PSZ       pSegTarget,
                 PSZ       pTemp,
                 PTATAG    pTATag,
                 PEQF_BOOL pfNoUnseg,
                 PEQF_BOOL pfKill );

__declspec(dllexport)
EQF_BOOL __cdecl EQFPREUNSEGW (
                 PSZ       pTagTable,
                 PSZ       pEdit,
                 PSZ       pProgPath,
                 PSZ       pSegTarget,
                 PSZ       pTemp,
                 PTATAG_W  pTATagW,
                 PEQF_BOOL pfNoUnseg,
                 PEQF_BOOL pfKill );

#ifdef __cplusplus
}
#endif


/*******************************************************************************
*
*       function:       EQFPOSTUNSEG
*
* -----------------------------------------------------------------------------
*       Description:
*               called after unsegmentation is invoked, to change the
*               target file to its final form.
*
*
*       Arguments:   PSZ    ... pointer to markup table name (full path)
*
*                    PSZ    ... pointer to editor name
*
*                    PSZ    ... pointer to program path (full path)
*
*                    PSZ          ... pointer to target file name (full path)
*
*                    PTATAG       ... pointer to tags inserted by text segmentation
*
*                   PBOOL   ... ptr to 'kill' flag
*                               if this flag changes to TRUE the userexit
*                               should return ASAP
*
*
*       Return:     BOOL          ... TRUE: processing was OK
*                                     FALSE: an error occured during processing
*
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport)
EQF_BOOL __cdecl EQFPOSTUNSEG (
                   PSZ       pTagTable,
                   PSZ       pEdit,
                   PSZ       pProgPath,
                   PSZ        pTarget,
                   PTATAG     pTATag);

__declspec(dllexport)
EQF_BOOL __cdecl EQFPOSTUNSEG2 (
                   PSZ       pTagTable,
                   PSZ       pEdit,
                   PSZ       pProgPath,
                   PSZ        pTarget,
                   PTATAG     pTATag,
                   PEQF_BOOL  pfKill );

__declspec(dllexport)
EQF_BOOL __cdecl EQFPOSTUNSEGW (
                   PSZ       pTagTable,
                   PSZ       pEdit,
                   PSZ       pProgPath,
                   PSZ        pTarget,
                   PTATAG     pTATag,
                   PEQF_BOOL  pfKill );

#ifdef __cplusplus
}
#endif

/*******************************************************************************
*
*       function:       EQFCHECKSEGW
*
* -----------------------------------------------------------------------------
*       Description:
*		The translation of a segment can be modified, for example change
*       lowercase characters to upper, or the user exit can prevent a segment
*       from being saved if it would be invalid (e.g. if length limits are exceeded)
*
*
*       Arguments:  PSZ_W    ... pointer previous source segment
*
*                    PSZ_W    ... pointer to current source segment
*
*                    PSZ_W    ... pointer to current translation
*
*
*       Return:     EQF_BOOL      ... TRUE: segment can be saved
*                                     FALSE: segment should not be saved
*******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport)
EQF_BOOL __cdecl EQFCHECKSEG (
   PSZ pszPrevSrc,                     // previous source segment
   PSZ pszSrc,                         // current source segment
   PSZ pszTgt,                         // current translation
   PEQF_BOOL pfChanged,                // segment changed
   EQF_BOOL fMsg                       // message display requested
);


__declspec(dllexport)
EQF_BOOL __cdecl EQFCHECKSEGW (
   PSZ_W pszPrevSrc,                     // previous source segment
   PSZ_W pszSrc,                         // current source segment
   PSZ_W pszTgt,                         // current translation
   PEQF_BOOL pfChanged,                // segment changed
   EQF_BOOL fMsg                       // message display requested
);

__declspec(dllexport)
EQF_BOOL __cdecl EQFCHECKSEGEXW (
   PSZ_W pszPrevSrc,                     // previous source segment
   PSZ_W pszSrc,                         // current source segment
   PSZ_W pszTgt,                         // current translation
   PEQF_BOOL pfChanged,                  // segment changed
   LONG  lInfo,                          // info handle to use with EQFGETPREVSEG(W),EQFGETNEXTSEG(W)
   ULONG ulSegNum,                       // segment number to use with EQFGETPREVSEG(W),EQFGETNEXTSEG(W)
   EQF_BOOL fMsg                         // message display requested
);

#ifdef __cplusplus
}
#endif

/*******************************************************************************
*
*       function:       EQFCOUNTMOD
*
* -----------------------------------------------------------------------------
*       Description:
*               called for counting documents
*
*
*       Arguments:  PCOUNTINOUT   ... pointer to count in/out struture
*
*       Return:     BOOL          ... TRUE: processing was OK
*                                     FALSE: an error occured during processing
*
*******************************************************************************/

__declspec(dllexport)
EQF_BOOL __cdecl EQFCOUNTMOD ( PCOUNTINOUT  pCountInOut );

/*******************************************************************************
*
*       function:       EQFPROOF
*
* -----------------------------------------------------------------------------
*       Description:
*               performs proofreading against the passed string.
*       Parameters:
*               PSZ        pointer to string
*               PCHAR      output area filled with list of mispelled words,
*                          each null terminated
*                          two consecutive '\0' indicate end of list
*               PUSHORT    length of area
*                          at output will be filled with number of characters
*                          filled
*       Return:
*               USHORT     OK - everything okay
*                          EQFRS_AREA_TOO_SMALL passed area too small
*                          EQFRS_NOMORPH_DICT   no morphological dictionary
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFPROOF ( PSZ, PCHAR, PUSHORT);
__declspec(dllexport)
USHORT __cdecl EQFPROOFW( PSZ_W, PCHAR_W, PUSHORT);
/*******************************************************************************
*
*       function:       EQFPROOFADD
*
* -----------------------------------------------------------------------------
*       Description:
*               add the passed word to the addenda
*       Parameters:
*               PSZ        pointer to string
*       Return:
*               USHORT     OK - everything okay
*                          EQFRS_NOADDENDA_DICT   no addenda dictionary
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFPROOFADD ( PSZ );
__declspec(dllexport)
USHORT __cdecl EQFPROOFADDW( PSZ_W );

/*******************************************************************************
*
*       function:       EQFPROOFAID
*
* -----------------------------------------------------------------------------
*       Description:
*               get the aid for the passed word
*       Parameters:
*               PSZ        pointer to string
*               PCHAR      output area filled with list of words,
*                          each null terminated
*                          two consecutive '\0' indicate end of list
*               PUSHORT    at input: length of area
*                          at output: filled with number of characters
*       Return:
*               USHORT     OK - everything okay
*                          EQFRS_AREA_TOO_SMALL passed area too small
*                          EQFRS_NOMORPH_DICT   no morphological dictionary
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFPROOFAID ( PSZ, PCHAR, PUSHORT );
__declspec(dllexport)
USHORT __cdecl EQFPROOFAIDW( PSZ_W, PCHAR_W, PUSHORT );

/*******************************************************************************
*
*       function:       EQFADDABBREV
*
* -----------------------------------------------------------------------------
*       Description:
*               add the passed word to the abbreviation dictionary
*       Parameters:
*               PSZ        pointer to string
*       Return:
*               USHORT     OK - everything okay
*                          EQFRS_NOADDENDA_DICT   no addenda dictionary
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFADDABBREV ( PSZ );
__declspec(dllexport)
USHORT __cdecl EQFADDABBREVW( PSZ_W );
/*******************************************************************************
*
*       function:       EQFDICTEDIT
*
* -----------------------------------------------------------------------------
*       Description:
*               Invoke dictionary edit dialog with the specified term for the
*               opened dictionaries.
*       Input:  PSZ...pointer to the term to be edited
*       Output: Returncode (see EQFRC_... values)
*******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFDICTEDIT (PSZ    pszTerm );
__declspec(dllexport)
USHORT __cdecl EQFDICTEDITW(PSZ_W  pszTerm );

  /*******************************************************************************
  *
  *       function:       EQFGETSTRUCT
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *               get the address of the generic structure; allocate if nec.
  *       Parameters:
  *               VOID
  *       Return:
  *               PSTEQFGEN   pointer to generic structure
  *******************************************************************************/
  __declspec(dllexport)
PSTEQFGEN __cdecl EQFGETSTRUCT( VOID );

  /*******************************************************************************
  *
  *       function:       EQFCHECKSTRUCT
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *               get the address of the generic structure;
  *       Parameters:
  *               VOID
  *       Return:
  *               PSTEQFGEN   pointer to generic structure
  *******************************************************************************/
  __declspec(dllexport)
PSTEQFGEN __cdecl EQFCHECKSTRUCT( VOID );

  /*******************************************************************************
  *
  *       function:       EQFFREESTRUCT
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *               free the generic sturcture
  *       Parameters:
  *               VOID
  *       Return:
  *               VOID
  *******************************************************************************/
  __declspec(dllexport)
VOID __cdecl EQFFREESTRUCT( VOID );

  /*******************************************************************************
  *
  *       function:       EQFGETNEXTSEG
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *               get the data of the next segment
  *       Parameters:
  *               LONG      lInfo                  // info needed
  *               PUSHORT   pusSegNum              // segment number
  *               PCHAR     pchData                // in: ptr to alloc buffer
  *               PUSHORT   pusBufSize             // in: size of alloc buffer
  *       Return:
  *               USHORT   error-code: NO_ERROR, ERR_BUFSIZE_EXCEEDED...
  *******************************************************************************/
  __declspec(dllexport)
USHORT __cdecl EQFGETNEXTSEG( LONG, PUSHORT, PCHAR, PUSHORT );
__declspec(dllexport)
USHORT __cdecl EQFGETNEXTSEGW( LONG, PULONG, PCHAR_W, PUSHORT );

  __declspec(dllexport)
USHORT __cdecl EQFGETNEXTSEGS( LONG, PUSHORT, PCHAR, PUSHORT, PUSHORT );
__declspec(dllexport)
USHORT __cdecl EQFGETNEXTSEGSW( LONG, PULONG, PCHAR_W, PUSHORT, PUSHORT );

  /*******************************************************************************
  *
  *       function:       EQFGETPREVSEG
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *               get the data of the previous segment
  *       Parameters:
  *               LONG      lInfo
  *               PUSHORT   pusSegNum
  *               PCHAR     pchData
  *               PUSHORT   pusBufSize
  *       Return:
  *               USHORT   error-code: NO_ERROR, ERR_BUFSIZE_EXCEEDED...
  *******************************************************************************/
  __declspec(dllexport)
USHORT __cdecl EQFGETPREVSEG( LONG, PUSHORT, PCHAR, PUSHORT );
__declspec(dllexport)
USHORT __cdecl EQFGETPREVSEGW( LONG, PULONG, PCHAR_W, PUSHORT );

  __declspec(dllexport)
USHORT __cdecl EQFGETPREVSEGS( LONG, PUSHORT, PCHAR, PUSHORT, PUSHORT );
__declspec(dllexport)
USHORT __cdecl EQFGETPREVSEGSW( LONG, PULONG, PCHAR_W, PUSHORT, PUSHORT );

  /*******************************************************************************
  *
  *       function:       EQFGETCURSEG
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *               get the data of the current  segment
  *       Parameters:
  *               LONG      lInfo
  *               PUSHORT   pusSegNum
  *               PCHAR     pchData
  *               PUSHORT   pusBufSize
  *       Return:
  *               USHORT   error-code: NO_ERROR, ERR_BUFSIZE_EXCEEDED...
  *******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFGETCURSEG( LONG, PUSHORT, PCHAR, PUSHORT );
__declspec(dllexport)
USHORT __cdecl EQFGETCURSEGW( LONG, PULONG, PCHAR_W, PUSHORT );      

__declspec(dllexport)
USHORT __cdecl EQFGETCURSEGS( LONG, PUSHORT, PCHAR, PUSHORT, PUSHORT );
__declspec(dllexport)
USHORT __cdecl EQFGETCURSEGSW( LONG, PULONG, PCHAR_W, PUSHORT, PUSHORT );

  /*******************************************************************************
  *
  *       function:       EQFTEXTPANEL
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *               create the test window and display the test segments
  *       Parameters:
  *               HWND        hwndParent            // handle
  *               PSZ  *      ppData                // pointer array
  *               USHORT      usForeground          // foreground color
  *               USHORT      usBackground          // background color
  *               PSZ         pCaptionText          // titlebar text of testwnd
  *       Return:
  *               VOID
  *******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFTEXTPANEL(HWND, PSZ *, USHORT, USHORT, PSZ, ULONG);
__declspec(dllexport)
USHORT __cdecl EQFTEXTPANELW(HWND, PSZ_W *, USHORT, USHORT, PSZ_W, ULONG);

  /*******************************************************************************
  *
  *       function:       EQFSHOW
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *               test and show the translation of some segments
  *       Parameters:
  *               LONG          // info needed
  *               HWND          // handle
  *       Return:
  *               EQF_BOOL
  *******************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport)
  EQF_BOOL __cdecl EQFSHOW( LONG, HWND );

#ifdef __cplusplus
}
#endif


  /*******************************************************************************
  *
  *       function:       EQFCONVERTFILENAMES
  *
  * -----------------------------------------------------------------------------
  *       Description:    converts LongFileName into ShortFileName and
  *                       vice versa.
  *
  *       Parameters:
  *               pszFolder        : Folder name with path information
  *
  *                                     FOLDER_DRIVE:\EQF\FOLDER_NAME.F00
  *
  *                                  The Folder name can be extracted out
  *                                  of pSegTarget, pSegSource as defined in
  *                                  eqf_xstart.
  *
  *                                  if (LongFileName==EMPTY_STRING)
  *                                     evaluate LongFileName  out of
  *                                     ShortFileName
  *                                  else if (ShortFileName==EMPTY_STRING)
  *                                     evaluate ShortFileName out of
  *                                     LongFileName
  *                                  else
  *                                     error
  *
  *               pszLongFileName  : LongFileName without path information  O R
  *                                  EMPTY_STRING
  *
  *               pszShortFileName : ShortFileName without path information  O R
  *                                  EMPTY_STRING
  *
  *
  *       Return: TM2 return code
  *               0=OK
  *
  *               LongFileName evaluated out of ShortFileName  O R
  *               ShortFileName evaluated out of LongFileName
  *
  *******************************************************************************/


__declspec(dllexport)
USHORT __cdecl EQFCONVERTFILENAMES
(
   PSZ      pszFolder,
   PSZ      pszLongFileName,
   PSZ      pszShortFileName
);


  /*******************************************************************************
  *
  *       function:       EQFX_GETDOCPATHS
  *
  * -----------------------------------------------------------------------------
  *       Description:    get the segmented source and target document name
  *                       from an EQF object name
  *
  *       Parameters:
  *               pszDocObjName    : document object name (input)
  *
  *               pszSegSource     : segmented source document name (output)
  *
  *               pszSegTarget     : segmented target document name (output)
  *
  *       Return: TM2 return code
  *               0=OK
  *
  *       Remarks: pszSegSource and pszSegTarget have to be allocated in size
  *                of MAX_LONGPATH (260 bytes).
  *******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQF_XDOCGETPATHS
(
   PSZ      pszDocObjName,
   PSZ      pszSegSource,
   PSZ      pszSegTarget
);


  /*******************************************************************************
  *
  *       function:       EQF_XDOCACT
  *
  * -----------------------------------------------------------------------------
  *       Description:    set the specified document to be the active one for
  *                       the translation environment
  *
  *       Parameters:
  *               pstEQFGen        : pointer to generic structure
  *
  *               pDocObjName      : document object name (input)
  *
  *       Return: TM2 return code
  *               0=OK
  *
  *       Remarks: The correct translation memory (databases) as well as the
  *                dictionaries are activated
  *******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQF_XDOCACT
(
   PSTEQFGEN pstEQFGen,
   PSZ       pDocObjName
);

  /*******************************************************************************
  *
  *       function:       EQF_XDOCREMOVE
  *
  * -----------------------------------------------------------------------------
  *       Description:    remove the specified document from the list of
  *                       currently translated documents
  *
  *       Parameters:
  *               pstEQFGen        : pointer to generic structure
  *
  *               pDocObjName      : document object name (input)
  *
  *       Return: TM2 return code
  *               0=OK
  *
  *       Remarks: The associated translation memory (databases) as well as the
  *                dictionaries are closed if document is the active one..
  *******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQF_XDOCREMOVE
(
   PSTEQFGEN pstEQFGen,
   PSZ       pDocObjName
);

  /*******************************************************************************
  *
  *       function:       EQF_XDOCADD
  *
  * -----------------------------------------------------------------------------
  *       Description:    add the specified document to the list of
  *                       currently translated documents
  *
  *       Parameters:
  *               pstEQFGen        : pointer to generic structure
  *
  *               pDocObjName      : document object name (input)
  *
  *       Return: TM2 return code
  *               0=OK
  *
  *******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQF_XDOCADD
(
   PSTEQFGEN pstEQFGen,
   PSZ       pDocObjName
);


  /*******************************************************************************
  *
  *       function:       EQF_XDOCNEXT
  *
  * -----------------------------------------------------------------------------
  *       Description:    iterate through the list of documents
  *                       and return the next one..
  *
  *       Parameters:
  *               pstEQFGen        : pointer to generic structure
  *
  *               pDocObjName      : document object name (input)
  *               pNextDocObjName  : document object name (output)
  *
  *       Return: TM2 return code
  *               0=OK
  *
  *******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQF_XDOCNEXT
(
   PSTEQFGEN pstEQFGen,
   PSZ       pInDocObjName,
   PSZ       pOutDocObjName
);


  /*******************************************************************************
  *
  *       function:       EQF_XDOCNUM
  *
  * -----------------------------------------------------------------------------
  *       Description:    return the i-th document of the list of loaded
  *                       docs
  *
  *       Parameters:
  *               pstEQFGen        : pointer to generic structure
  *               usI              : number of document to be returned
  *               pOutDocObjName   : document object name (output)
  *
  *       Return: TM2 return code
  *               0=OK
  *
  *******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQF_XDOCNUM
(
   PSTEQFGEN pstEQFGen,
   USHORT    usI,
   PSZ       pOutDocObjName
);

  /*******************************************************************************
  *
  *       function:       EQF_XDOCINLIST
  *
  * -----------------------------------------------------------------------------
  *       Description:    return the i-th document of the list of loaded
  *                       docs
  *
  *       Parameters:
  *               pstEQFGen        : pointer to generic structure
  *               pInDocObjName    : document object name
  *               pusI             : check if document in list
  *
  *       Return: 0                : document in list and pusI set on return
  *               TM/2 error
  *
  *******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQF_XDOCINLIST
(
   PSTEQFGEN pstEQFGen,
   PSZ       pInDocObjName,
   PUSHORT   pusI
);
#endif


  /*******************************************************************************
  *
  *       function:       EQFTOCGOTO
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *               test and show the translation of some segments
  *       Parameters:
  *               LONG          // info needed
  *               HWND          // handle
  *       Return:
  *               EQF_BOOL
  *******************************************************************************/
__declspec(dllexport)
  EQF_BOOL __cdecl EQFTOCGOTO(LONG, HWND );

  /*******************************************************************************
  *
  *       function:       EQFADDTOTOC
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *               get the data of the next segment
  *       Parameters:
  *               LONG      lInfo                  // info needed
  *               PUSHORT   pusSegNum              // segment number
  *               PCHAR     pchData                // in: ptr to alloc buffer
  *               PUSHORT   pusBufSize             // in: size of alloc buffer
  *       Return:
  *               USHORT   error-code: NO_ERROR, ERR_BUFSIZE_EXCEEDED...
  *******************************************************************************/
__declspec(dllexport)
  USHORT __cdecl EQFADDTOTOC  ( LONG, PUSHORT, PCHAR, HWND    );
__declspec(dllexport)
  USHORT __cdecl EQFADDTOTOCW ( LONG, PULONG,  PCHAR_W, HWND  );



  /*******************************************************************************
  *
  *       function:       EQFWRITEHISTLOG
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *       Parameters:
  *       Return:
  *******************************************************************************/
__declspec(dllexport)
  USHORT __cdecl EQFWRITEHISTLOG
  (
     PSZ              ,    // folder object name (e.g. "E:\\EQF\\SAMPLE1.F00")
     PSZ              ,    // name of document (e.g. "DEVICE.SCR")
     PAPIDOCSAVEHIST       // HistLog Record structure
  );


  /*******************************************************************************
  *
  *       function: EQFGETSOURCELANG
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *       Parameters:
  *       Return:
  *******************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
  USHORT __cdecl EQFGETSOURCELANG
  (
    PSZ      pszFolder,       //  Folder path
    PSZ      pszFileName,     //  LongFileName
    PSZ      pszSrcLang       //  Source Language out
  );



  /*******************************************************************************
  *
  *       function: EQFGETTARGETLANG
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *       Parameters:
  *       Return:
  *******************************************************************************/
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
  USHORT __cdecl EQFGETTARGETLANG
  (
    PSZ      pszFolder,       //  Folder path
    PSZ      pszFileName,     //  LongFileName
    PSZ      pszTrgLang       //  Target Language out
  );


  /*******************************************************************************
  *
  *       function: EQFGETDOCFORMAT
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *       Parameters:
  *       Return:
  *******************************************************************************/
__declspec(dllexport)
  USHORT __cdecl EQFGETDOCFORMAT
  (
    PSZ      pszFolder,       //  Folder path
    PSZ      pszFileName,     //  LongFileName
    PSZ      pszFormat        //  Document Format out
  );



  /*******************************************************************************
  *
  *       function: EQFWORDCNTPERSEG
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *       Parameters:
  *       Return:
  *******************************************************************************/
__declspec(dllexport)
  USHORT __cdecl EQFWORDCNTPERSEG
  (
    PSZ            pszSeg,               // ptr to Segment
    PSZ            pszLang,              // ptr to Language of Segment
    PSZ            pszFormat,            // ptr to Format
    PULONG         pulResult,            // result to be counted
    PULONG         pulMarkUp              // result for markup
  );

__declspec(dllexport)
 USHORT __cdecl EQFWORDCNTPERSEGW
    (
      PSZ_W          pszSeg,               // ptr to Segment
      PSZ            pszLang,              // ptr to Language of Segment
      PSZ            pszFormat,            // ptr to Format
      PULONG         pulResult,            // result to be counted
      PULONG         pulMarkUp              // result for markup
    );

//+----------------------------------------------------------------------------+
//|API function                                                                |
//+----------------------------------------------------------------------------+
//|Function name:     EQFGETDOCCONV                                            |
//+----------------------------------------------------------------------------+
//|Description:       Get the document conversion settings for the specified   |
//|                   document                                                 |
//+----------------------------------------------------------------------------+
//|Parameters:                                                                 |
//|                   PSZ        pszFolderPath folder path                     |
//|                   PSZ        pszDocName    document short name             |
//|                   PSZ        pszConversion pointer to buffer for selected  |
//|                                            conversion (up to 40 characters)|
//|                   PUSHORT    pusConvFlag   pointer to conversion flags     |
//|                   PUSHORT    pusCodePage   pointer to associated ASCII cp  |
//+----------------------------------------------------------------------------+
//|Returns:           USHORT     return code of called functions               |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT __cdecl EQFGETDOCCONV
(
   PSZ        pszFolderPath, // folder path; e.g. "C:\EQF\SAMPLE1.F00"
   PSZ        pszDocName,    // document short name; e.g. "TEST.DOC"
   PSZ        pszConversion, // pointer to buffer for selected
                             // conversion (up to 40 characters)
   PUSHORT    pusConvFlag,   // pointer to buffer for conversion flags
   PUSHORT    pusCodePage    // pointer to buffer for associated ASCII code-page
);

/**********************************************************************/
/* defines for the following API's                                    */
/**********************************************************************/
#define  UNCHANGED_STATUS 0
#define  XLATED_STATUS    1            // seg is translated
#define  TOBE_STATUS      2            // seg must be translated
#define  NOP_STATUS       3            // no translatable info in seg
#define  ATTR_STATUS      4            // seg with attribute info

#define  UNCHANGED_CURRENT 0
#define  SET_CURRENT       1
#define  NOT_CURRENT       2

#define  UNCHANGED_NOCOUNT 0
#define  SET_NOCOUNT       1
#define  NOT_NOCOUNT       2

#define  UNCHANGED_TYPED   0
#define  SET_TYPED         1
#define  NOT_TYPED         2

#define  UNCHANGED_COPIED  0
#define  SET_COPIED        1
#define  NOT_COPIED        2


  /*******************************************************************************
  *
  *       function: EQFBUILDSEGATTR
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *       Parameters:
  *       Return:
  *******************************************************************************/
__declspec(dllexport)
USHORT __cdecl EQFBUILDSEGATTR
  (
    PSZ       pszBuffer,                 // in/out: last/new ".qff n=..."
    USHORT    usBufLen,                  // length of buffer
    PSEGINFO  pSegInfo                   // info to be put into Buffer
  );
__declspec(dllexport)
  USHORT __cdecl EQFBUILDSEGATTRW
  (
    PSZ_W     pszBuffer,                 // in/out: last/new ".qff n=..."
    USHORT    usBufLen,                  // length of buffer
    PSEGINFO  pSegInfo                   // info to be put into Buffer
  );


  /*******************************************************************************
  *
  *       function: EQFGETSEGATTR
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *       Parameters:
  *       Return:
  *******************************************************************************/
__declspec(dllexport)
  USHORT __cdecl EQFGETSEGATTR
  (
    PSZ       pszBuffer,                 // in/out: last/new ":qff n=..."
    USHORT    usBufLen,                  // length of buffer
    PSEGINFO  pSegInfo                   // info to be put into Buffer
  );
__declspec(dllexport)
  USHORT __cdecl EQFGETSEGATTRW
  (
    PSZ_W     pszBuffer,                 // in/out: last/new ":qff n=..."
    USHORT    usBufLen,                  // length of buffer
    PSEGINFO  pSegInfo                   // info to be put into Buffer
  );


  /*******************************************************************************
  *
  *       function: EQFWRITEHISTDOCSAVE
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *       Parameters:
  *       Return:
  *******************************************************************************/
__declspec(dllexport)
  USHORT __cdecl EQFWRITEHISTDOCSAVE
  (
    PSZ       pszDocument,                 // name of document
    PSZ       pszFolObjName                // object name of folder
  );
  /*******************************************************************************
  *
  *       function: EQFGETTYPE
  *
  * -----------------------------------------------------------------------------
  *       Description:
  *       Parameters:
  *       Return:
  *******************************************************************************/
__declspec(dllexport)
  USHORT __cdecl EQFGETTYPE
  (
    PSTEQFGEN      pstEQFGen,            // ptr to nenric structure
    USHORT         usNum,                // mumber of proposal
    PUSHORT        pusType               // type of existing/copied props
  );


//////////////////////////////////////////////////////////////////////////////
//
//       function: EQFGETSEGCONTEXT
//
//       Description: This entry point is called by TranslationManager
//                    to supply the context for the given segment.
//
//       Parameters: PSZ_W  pszCurSeg    ptr to text of current segment (UTF16)
//                   PSZ_W  pszPrevSeg   ptr to text of previous segment (UTF16)
//                   PSZ_W  pszNextSeg   ptr to text of next segment (UTF16)
//                   PSZ_W  pszContext   ptr to buffer for context string (UTF16)
//                                       this buffer can contain up to 2048 characters
//                   LONG   lHandle      handle for usage with get segment helper
//                                       functions (EQFGETPREVSEG,...)
//
//       Return:     USHORT  0 = function compeleted successfully
//
////////////////////////////////////////////////////////////////////////////////

#ifdef __cplusplus
extern "C" {
#endif
__declspec(dllexport)
USHORT __cdecl EQFGETSEGCONTEXT
(
  PSZ_W       pszCurSeg,               // ptr to text of current segment (UTF16)
  PSZ_W       pszPrevSeg,              // ptr to text of previous segment (UTF16)
  PSZ_W       pszNextSeg,              // ptr to text of next segment (UTF16)
  PSZ_W       pszContext,              // ptr to buffer for context string (UTF16, max 2048 chars)
  LONG        lHandle,                 // handle for usage with next/prev segment calls
  ULONG       ulSegNum                 // current segment number for usage with next/prev segment calls
);

__declspec(dllexport)
USHORT __cdecl EQFCOMPARECONTEXT
(
  PSZ_W       pszSegContext,           // ptr to context of active segment (UTF16)
  PSZ_W       pszTMContext,            // ptr to context of segment from TMem (UTF16)
  PUSHORT     pusRanking               // ptr to USHORT receiving the result
);

__declspec(dllexport)
USHORT __cdecl EQFUPDATECONTEXT
(
  PSZ_W       pszCurSeg,               // ptr to text of current segment (UTF16)
  PSZ_W       pszContext               // ptr to buffer for context string (UTF16)
);

__declspec(dllexport)
USHORT __cdecl EQFFORMATCONTEXT
(
  PSZ_W       pszContext,              // ptr to buffer with context string (UTF16)
  PSZ_W       pszDisplayString         // ptr to buffer for formatted context (UTF16)
);

#ifdef __cplusplus
}
#endif



__declspec(dllexport)
USHORT __cdecl EQFSEGFILECONVERTUNICODE2ASCII
(
  PSZ         pszInFile,                // ptr to segmented UTF16Unicode file
  PSZ         pszOutFile                // ptr to segmented ASCII file
);

__declspec(dllexport)
USHORT __cdecl EQFSEGFILECONVERTASCII2UNICODE
(
  PSZ         pszInFile,                // ptr to segmented ASCII file
  PSZ         pszOutFile                // ptr to segmented UTF16 Unicode file
);

__declspec(dllexport)
USHORT __cdecl EQFSEGFILECONVERTUNICODE2ASCIILANG
(
  PSZ         pszInFile,                // ptr to segmented UTF16Unicode file
  PSZ         pszOutFile,                // ptr to segmented ASCII file
  PSZ         pszDocLang
);

__declspec(dllexport)
USHORT __cdecl EQFSEGFILECONVERTASCII2UNICODELANG
(
  PSZ         pszInFile,                // ptr to segmented ASCII file
  PSZ         pszOutFile,                // ptr to segmented UTF16 Unicode file
  PSZ         pszDocLang
);

// new API's : convert a total file from ANSI to ASCII or vice versa using
// the codepage associated to the given language
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT __cdecl EQFFILECONVERSIONEX
(
    PSZ         pszInFile,                // ptr to input file
    PSZ         pszOutFile,               // ptr to output file
    PSZ         pszLanguage,              // ptr to the language of the file
    USHORT      usConversionType          // identifier of conversion: EQF_ASCII2ANSI,
                                          // EQF_ANSI2ASCII
);

// get the installed version of TranslationManager
__declspec(dllexport)
ULONG __cdecl EQFGETVERSION( void );

// user exit entry point called by TranslationManager to query
// user exit specific information
// a return value of zero indicates successful completion
__declspec(dllexport)
USHORT __cdecl EQFQUERYEXITINFO
(
  PSZ pszTagTable,                     // name of the markup table, e.g. "IBMHTM32"
  USHORT usMode,                       // type of information being queried
  PSZ pszBuffer,                       // buffer area receiving the information returned by the exit
  USHORT usBufLen                      // length of buffer area
);

// modes for EQFQUERYEXITINFO call:

#define QUERYEXIT_ADDFILES 0x01
// the user exit is to fill the buffer with the names of additional markup table files.
// These files will be exported and imported in the folder export package (.FXP)
// The user exit itself and the markup table are always exported and should not be
// listed here.
// The additional files are stored in the buffer area as a comma seperated list which
// is terminated by a 0x00 character. Files not residing in the \EQF\TABLE directory
// have to be prefixed with their relative path (relative to the \EQF directory).
// Files which are not located in the \EQF directory cannot be exported and imported.

#define QUERYEXIT_CONVERSION 0x02
// the user exit fills-in information regarding the conversions done by the user exit,
// which kind of conversion is allowed, and which conversions should be done by
// TranslationManager
// The pszBuffer pointer points to a QUERYEXITCONVERSION structure which has this layout:
typedef struct _QUERYEXITCONVERSION
{
  // currently only placeholders until function is activated
  EQF_BOOL  fFiller01;
  EQF_BOOL  fFiller02;
  EQF_BOOL  fFiller03;
  EQF_BOOL  fFiller04;
  EQF_BOOL  fFiller05;
  EQF_BOOL  fFiller06;
  EQF_BOOL  fFiller07;
  EQF_BOOL  fFiller08;
  EQF_BOOL  fFiller09;
  EQF_BOOL  fFiller10;
  EQF_BOOL  fFiller11;
} QUERYEXITCONVERSION, *PQUERYEXITCONVERSION;

// information IDs to be used in EQFGETINFO call
typedef enum _EQFGETINFOID
{
  GETINFO_MARKUP,                      // get document markup
  GETINFO_FOLDEROBJECT,                // get folder object name 
  GETINFO_FOLDERLONGNAME,              // get folder long name (ASCII)  
  GETINFO_DOCFULLPATH,                 // get fully qualified path of the document in the STARGET directory
  GETINFO_DOCLONGNAME                  // get document long name (ASCII)
} EQFGETINFOID;

// API to retrieve information about the document and its folder
__declspec(dllexport)
USHORT __cdecl EQFGETINFO
(
  LONG         lInfo,                  // info handle from EQFSHOW entry point
  EQFGETINFOID InfoID,                 // ID for requested info  
  PCHAR        pchBuffer,              // buffer for returned info or NULL for info length
  PUSHORT      pusBufSize              // size of buffer, on return: size of returned info
);

// path IDs to be used in EQFBUILDDOCPATH call
typedef enum _EQFPATHID
{
  PATHID_SOURCE,                       // ID for SOURCE directory
  PATHID_SEGSOURCE,                    // ID for SSOURCE directory
  PATHID_SEGTARGET,                    // ID for STARGET directory
  PATHID_TARGET                        // ID for TARGET directory
} EQFPATHID;

// API to build a fully qualified path name of a document contained in a TM folder
__declspec(dllexport)
USHORT __cdecl EQFBUILDDOCPATH
(
  PCHAR        pszFolObjName,          // folder object name
  PCHAR        pszDocLongName,         // document long name  
  EQFPATHID    PathID,                 // indentifier for requested document subdirectory
  PCHAR        pszBuffer               // buffer receiving the path name, (required size: 60 bytes or more)
);

// Analysis option sructure to be used by EQFGETTAOPTIOSN and EQFSETTAOPTIONS
typedef struct _EQFTAOPTIONS
{
  BOOL             fAdjustLeadingWS;             // adjust-leading-whitespace flag
  BOOL             fAdjustTrailingWS;            // adjust-trailing-whitespace flag
  BYTE             bForFutureUse[50];            // area for future enhancements
} EQFTAOPTIONS, *PEQFTAOPTIONS;

// API to get the active analysis options
// this API can only be called from the user exit entry points EQFPRESEGEX and EQFPOSTSEGEX
__declspec(dllexport)
USHORT __cdecl EQFGETTAOPTIONS
(
  HANDLE           hAnalysis,                    // analysis process handle
  PEQFTAOPTIONS    pOptions                      // ptr to analysis options
);

// API to set the analyis options
// this API can only be called from the user exit entry points EQFPRESEGEX and EQFPOSTSEGEX
__declspec(dllexport)
USHORT __cdecl EQFSETTAOPTIONS
(
  HANDLE           hAnalysis,                    // analysis process handle
  PEQFTAOPTIONS    pOptions                      // ptr to analysis options
);

// API call to write a history log save record using the count data stored in a segmented target file
__declspec(dllexport)
USHORT __cdecl EQFWRITEHISTLOGFROMFILE
(
  PSZ         pszSegTargetFile                   // fully qualified file name of STARGET file of document
);



#ifdef __cplusplus
}
#endif

