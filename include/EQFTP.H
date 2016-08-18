/*! EQFTP.H       External header file for Translation Processor functions
	Copyright (c) 1990-2016, International Business Machines Corporation and others. All rights reserved.
*/

#include <richedit.h>
/**********************************************************************/
/* Allow usage of this include file even if AVIO/PM support has not   */
/* included                                                           */
/**********************************************************************/

#define MAXSHRKLEN      3
#define MAX_SCREENLINES    128         // number of screen lines supported
#define IO_BUFFER_SIZE   16000         // size of buffer used for file I/O
#define TOK_BUFFER_SIZE  10000         // size of buffer used for tokenization
#define MAXTRNOTE_SIZE   8             // max.length of <TRNOTE> abbreviation
#define TPRO_EYECATCHER  0xFCFDFDFC    // our eyecatcher
#define ALL_LETTERS   52               // a-z + A-Z are 52 letters

/* Error return codes.  These are all less than zero. Some of them are */
/* also defined in EQFTA.H (for the EQFB... function return codes)    */

#if !defined(ERR_NOMEMORY)
 #define ERR_NOMEMORY   -1
#endif
 #define ERR_OPENFILE   -2
#if !defined(ERR_READFILE)
 #define ERR_READFILE   -3
#endif
 #define ERR_WRITEFILE  -4
 #define ERR_BADCMD     -5
 #define ERR_BADPARMS   -6
 #define ERR_WNDCREATE  -7

/* Warning return codes.  These are all greater than zero.  */

 #define WARN_NOFILE     1
 #define WARN_LINESPLIT  2
 #define WARN_NOPREV     3
 #define WARN_NONEXT     4
 #define WARN_CONFLICT   5
 #define WARN_NOMARK     6
 #define WARN_NOMATCH    7
 #define WARN_TOOLONG    8
 #define WARN_OUTRANGE   9



/**********************************************************************/
/* WYSIWYG Types                                                      */
/**********************************************************************/
#define WYSIWYG_HTML     1
#define WYSIWYG_RTF      2


/* The editor functions and commands indicate how much of the screen */
/* requires updating by setting REDRAW_XXXX flags.  These are imple- */
/* mented as bit values in increasing order of significance, so a    */
/* simple 'OR' operation can be used to set the maximum.             */

// typedef enum _redraw {
//    REDRAW_NONE  = 0,      /* Nothing to do                       */
//    REDRAW_LINE  = 0x01,   /* Just the current line               */
//    REDRAW_BELOW = 0x02,   /* Current line and all lines below it */
//    REDRAW_ALL   = 0x04    /* All lines                           */
// } REDRAW;

typedef USHORT REDRAW;
#define REDRAW_NONE  0       /* Nothing to do                       */
#define REDRAW_LINE  0x01    /* Just the current line               */
#define REDRAW_BELOW 0x02    /* Current line and all lines below it */
#define REDRAW_ALL   0x04    /* All lines                           */

/* 'Insert' and 'replace' mode are indicated by changing the shape  */
/* of the cursor.  Again how these CURSOR_XXXX values actually give */
/* different shapes is handled entirely by the low level system     */
/* routines.                                                        */

 typedef enum _cursor
 {
    CURSOR_REPLACE,
    CURSOR_INSERT,
    CURSOR_SEGMENT,
    CURSOR_EXIT,
    MAXCURSOR
 } CURSOR;

/**********************************************************************/
/* Take care: if doctype is added: check with IDS_TB_FONT_...         */
/* since TRNOTE_DOC and EEA_DOC do not have selectable fonts!         */
/**********************************************************************/
 typedef enum
 {
    OTHER_DOC,                   // other documents besides ....
    SSOURCE_DOC,                 // source segmented document
    STARGET_DOC,                 // target segmented document
    SERVPROP_DOC,                // proposal window
    SERVDICT_DOC,                // dicitonary window
    SERVSOURCE_DOC,              // source window for proposals
    VISSRC_DOC,                  // itm visualization src doc
    VISTGT_DOC,                  // itm visualization tgt doc
    TRNOTE_DOC,                  // translators note window
    EEA_DOC,                     // EasyEnglish document
    MAX_DIF_DOC                  // maximum number
 } DOCTYPE;

 typedef enum
 {
    DISP_PROTECTED,              // protect SGML tags
    DISP_UNPROTECTED,            // allow change on all
    DISP_HIDE,                   // hide protected tags
    DISP_SHRINK,                 // shrink all NOP segments
    DISP_COMPACT,                // shrink NOP segments and compact inline tags
    DISP_SHORTEN,                // display only 1st word of NOP
    DISP_WYSIWYG                 // display text in interpreted way
 } DISPSTYLE;


 typedef struct _segflags
 {
   unsigned short JoinStart : 1;     // TRUE if segment is start of a joined chain
   unsigned short Joined    : 1;     // TRUE if segment is part of a joined chain
   unsigned short Marked    : 1;     // TRUE if segment is marked
   unsigned short Current   : 1;     // TRUE if segment is current segment
   unsigned short UnTrans   : 1;     // TRUE if segment is untranslated
   unsigned short Typed     : 1;     // TRUE if typing done in segment
   unsigned short Copied    : 1;     // TRUE if proposal copied (but nor ctrl 0)
   unsigned short Expanded  : 1;      // TRUE if segment is expanded
   unsigned short NoCount   : 1;      // TRUE if segment should not be counted
   unsigned short InsertAdd : 1;      // TRUE: seg is preceeded by "addtablesegs"
   unsigned short NoWrite   : 1;      // TRUE if seg not written in STARGETfile (XLATE)
   unsigned short Spellchecked : 1;   // TRUE: segment is spellchecked
   unsigned short NoReorder : 1;      // TRUE: no reordering in BIDI case necessary
   unsigned short Dummy5    : 1;      // Dummy for further use
   unsigned short NoContextAvailable : 1; // TRUE: no context info for this segment available
   unsigned short Changed   : 1;      // Used by global-find-and-change
 } SEGFLAGS, *PSEGFLAGS;

   // fields for counting tool
   typedef struct _countflags
   {
     unsigned short AnalAutoSubst    : 1;     // Count flag
     unsigned short EditAutoSubst    : 1;     // Count flag
     unsigned short ExactCopy        : 1;     // Count flag
     unsigned short MachCopy         : 1;     // Count flag
     unsigned short FuzzyCopy        : 1;     // Count flag
     unsigned short ReplCopy         : 1;     // Count flag
     unsigned short ExactExist       : 1;     // Count flag
     unsigned short MachExist        : 1;     // Count flag
     unsigned short FuzzyExist       : 1;     // Count flag
     unsigned short ReplExist        : 1;     // Count flag
     unsigned short Fuzzy5070   : 1;     // Count flag
     unsigned short Fuzzy7190   : 1;     // Count flag
     unsigned short Fuzzy9199   : 1;     // Count flag
     unsigned short PropChanged : 1;          // proposal has been chnaged flag
     unsigned short GlobMemExist : 1;     // Count flag
     unsigned short GlobMemCopy  : 1;     // Count flag

   } COUNTFLAG, *PCOUNTFLAG;

typedef struct _TBSEGMENT
{
   PSZ     pData;                // pointer to segment data
   USHORT  usLength;             // length of segment data (in CHAR)
   USHORT  qStatus;              // status of segment  (XLATED, TOBE, etc..)
   ULONG   ulSegNum;             // segment number
   PUSHORT pusBPET;              // Browser Protection Elements Table
                                 // (usStart,usEnd,usType)
                                 // where usType: PROTECT, UNPROTECT,
                                 //               LINEBREAK
                                 // End of List: NULL element
                                 // when EQFBFileReadEx is called with FILEREAD_FPOS
                                 // flag set, this ptr is a ULONG value
                                 // containing the file position of the segment
   SEGFLAGS SegFlags;            // segment flags
   USHORT   usOrgLength;         // original segment length (length of
                                 // segment in input file) only used when
                                 // EQFBFileReadEx is called with FILEREAD_FPOS
                                 // flag set
    COUNTFLAG CountFlag;
    USHORT  usSrcWords;          // words in source segment
    USHORT  usTgtWords;          // words in target segment
    USHORT  usModWords;          // modified words

    ULONG  ulShrinkLen;          // length of seg if displayed as abbreviation
                                 // holds segment number at one point in time


   PUSHORT pusHLType;            // Highlight type
                                 // (usStart,usEnd,usType)
                                 // where usType: TAG_HIGHLIGHT
                                 //        in plan: MISSPELLED_HIGHLIGHT
                                 // End of List: NULL element
   PSZ_W    pDataW;              // ptr to UNICODE segment data
   PSZ_W    pContext;            // ptr to segment context (UNICODE)
   PVOID    pvMetadata;          // ptr to segment meta data or NULL if none

} TBSEGMENT, *PTBSEGMENT;
// BE CAREFUL with changing sizeof(TBSEGMENT)! check & adapt
// TBDOCUMENT->Dummy[sizeof(TBSEGMENT)...] in such a case!! (03/02/12, RJ)

#ifdef __cplusplus
class OtmSegment;
#endif

typedef struct _TBSEGMENTTABLE
{
   ULONG      ulSegments;        // no of segments in table
   ULONG      ulAllocSegs;       // no of segment entries available in table
   PTBSEGMENT pSegments;         // ptr to allocated segment table
//#ifdef __cplusplus
//   OtmSegment *pSegObjects;
//#else
//   PVOID      pSegObjects;
//#endif
} TBSEGMENTTABLE, *PTBSEGMENTTABLE;


typedef struct _TBROWOFFSET
{
   ULONG   ulSegNum;             // segment number
   USHORT  usSegOffset;          // segment offset
} TBROWOFFSET, * PTBROWOFFSET;

/* This structure defines a set of 'global' editor flags */
 typedef struct _flags
 {
    unsigned short inserting : 1;     /* TRUE if in 'insert' mode          */
    unsigned short workchng  : 1;     /* TRUE if in 'work' line is changed */
    unsigned short PostEdit  : 1;     // TRUE if in post editing mode
    unsigned short AutoMode  : 1;     // TRUE if in automatic mode
    unsigned short Reflow    : 1;     // TRUE if insert line,false if split
    unsigned short MarkedSeg : 1;     // TRUE if marked segment is available
    unsigned short EndOfSeg  : 1;     // TRUE if end of segment reached
    unsigned short InSelection : 1;    // TRUE if in selection mode ...
    unsigned short NoEmptySegCheck : 1;// TRUE do not check for empty segments
 } DOCFLAGS;

 typedef struct _USEROPTFLAGS
 {
    unsigned short bVisibleSpace;           // make spaces visible
    unsigned short bAutoSpellCheck;         // autospellcheck on or off
    unsigned short bBidiLogicDisplay;       // Logic display for bidi active
    unsigned short bBackSave;               // auto-filesave in background
    unsigned short bBackSubst;              // auto-substitution in background
    unsigned short bDispMTAlways;           // display available MT proposals always
    unsigned short bConvSOSI;               // during save of segment SOSI is inserted
    unsigned short bAllExactProposals;      // display all exact proposals
    unsigned short bDispPropQuality;        // display proposal quality
    unsigned short bAdjustLeadingWS;        // adjust leading white spaces
    unsigned short bAdjustTrailingWS;       // adjust trailing white spaces
 } USEROPTFLAGS;



/**********************************************************************/
/* useropt should be splitted into   the values set by the profinit   */
/* dialog and the values set by the settings dialog(Customize Functio)*/
/* in profinit: fLineWrap, sRMargin, DispTM, DispTrans, fInitCrsMode  */
/**********************************************************************/
 typedef struct _USEROPT
 {
   EQF_BOOL   fCUABksp;      // TRUE if Bksp across line boundery
   EQF_BOOL   fAutoStop;     // TRUE if automatic stops at a no-match
   EQF_BOOL   fFuzzyMsg;     // TRUE if fuzzy message displayed
   EQF_BOOL   fAddFuzzyProp; // TRUE if exact + fuzzy matches in TMwindow
   EQF_BOOL   fLineWrap;     // TRUE if automatic linewrap on at init
   EQF_BOOL   fTagCheck;     // TRUE if check for same tags is enabled
   EQF_BOOL   fSrcUnChg;     // TRUE if msg display in Source unchanged case
   EQF_BOOL   fCrsInsert;    // TRUE if Cursor Insert for new segment,
   EQF_BOOL   fInsProposal;  // TRUE if insert proposal for cursor state
   EQF_BOOL   fFullSeg;      // TRUE if the full segment should be displayed
   USHORT     usFuzzyForDiv; // threshhold value for fuzzy/machine difference display
   USHORT     DispTM;        // initial Dispstyle in TM window
   EQF_BOOL   fAutoRepl;     // TRUE if automatic replace in fuzzy matches
   USHORT     DispTrans;     // initial Dispstyle in translation window
   SHORT  sRMargin;          // initial value of right margin
   EQF_BOOL   fInitCrsMode;  // TRUE if insert mode at init of editor
   CHAR   szInTagAbbr[MAXSHRKLEN+1];      // compact abbr for inline tags
   CHAR   szOutTagAbbr[MAXSHRKLEN+1];     // shrink abbr for NOP segments
   EQF_BOOL   fUseLatestMatch; // use latest match during automatic substitution
   SHORT  sFocusLine;          // line number where active segment is positioned
   EQF_BOOL   fSrcPropWnd;   // TRUE if Src of Prop window is displayed always
   EQF_BOOL   fNumProp;      // TRUE if number of existing props is displayed
   EQF_BOOL   fOriginProp;   // TRUE if filename of proposal is displayed
   EQF_BOOL   fDateOfProp;   // TRUE if date of proposal should be displayed
   EQF_BOOL   fAddInfoDic;   // TRUE if additional dic info should be displayed..
   EQF_BOOL   fNoCUASel;               // TRUE if no CUA selection (i.e. block based)
   EQF_BOOL   fAllDictTerms;           // TRUE if all terms from all dictionaries are displ.
   // TRUE if exact match should be replaced in case only one is available in this document
   EQF_BOOL   fExactContextTMMatch;
   EQF_BOOL   fSegBound;               // TRUE if Postedit segment boundery display
   CHAR       chTRNoteAbbr[MAXTRNOTE_SIZE+2];    // TRNOTE abbrv ( USEROPT new size!!)
   EQF_BOOL   fDispDictName;           // TRUE if Dict.name is at each dict.entry
   USEROPTFLAGS UserOptFlags;          // set additional useropt flags
   BYTE       bVisibleBlank;           // char to be used for visible blank
   BYTE       bVisibleLineFeed;        // char to be used for visible linefeed
   BYTE       bSegmentBoundary;        // char to be used for segment boundary
   BYTE       bDummy;
   SHORT      sMinuteTilNextSave;      // using fFiller05 for one SHORT
   EQF_BOOL   fLkupSingleOfCompounds;  //Def=0: single words of compounds are looked up!
   EQF_BOOL   fDispMemName;            // TRUE if memory name is to be displayed for each proposal
   EQF_BOOL   fDispMemIndicator;       // TRUE if memory indicator is to be displayed for each proposal
   EQF_BOOL   fMachFuzzyColor;         // TRUE use different color settings for machine and fuzzy proposals
   EQF_BOOL   fMachFuzzyDiff;          // TRUE display differences between machine and fuzzy proposals  
 } USEROPT, *PUSEROPT;


  typedef struct _BIDIDISP
  {
    ULONG  ulCol;                      // column of text
    ULONG  ulLen;                      // length of text in bytes
    USHORT usColor;                    // color to be used
    PSZ_W  pTextCompl;                 // pointer to complete text string
    LONG   lSideScroll;                // sidescroll offset
    BYTE   bMark;                      // mark active
    struct _BIDIDISP * pNext;          // pointer to next ..
  } BIDIDISP, *PBIDIDISP;

  typedef struct _BIDILINE
  {
    PUINT       pusOrder;
    PBYTE       pbClass;
    USHORT      usOffset;
  } BIDILINE, * PBIDILINE;

  typedef struct _BIDISTRUCT
  {
    USHORT      usBIDICursorMode;     // bidi cursor bitmap
    BYTE        bClass[ MAX_SEGMENT_SIZE + 1 ];
    int         usCaretPos[ MAX_SEGMENT_SIZE+1 ];
    UINT        usOrder[ MAX_SEGMENT_SIZE+1 ];
    UINT        usOrder1[ MAX_SEGMENT_SIZE+1 ];
    USHORT      usBidiOffs;
    BOOL        fNotBidiFirst;
    CHAR_W      chOutBuf[ MAX_SEGMENT_SIZE+1 ];
    USHORT      usCCSIdIn;                 // input codepage
    USHORT      usCCSIdOut;                // output codepage
    USHORT      usStyleIn;                 // input style
    USHORT      usStyleOut;                // output style

    CHAR_W      chDisplayRow[ MAX_SEGMENT_SIZE+1 ];
    BYTE        bOutputPending;
    BYTE        bOutputPendingRow[ MAX_SCREENLINES+10 ];
    ULONG       ulDisplayRow;

    USHORT      usTimerID;

    ULONG       ulBidiCursorCol;                 // Bidi-Cursor column
    PPOOL       pPool;                           // pointer to Pool area
    PBIDIDISP   pBidiDisp[ MAX_SCREENLINES+10 ]; // pointer to screen buffer
    BIDILINE    BidiLine[ MAX_SCREENLINES+10 ];  // bidi line struct
    BOOL        fRedrawScreen;                   // Screen will just be redrawn
    BOOL        fRTL;                            // Right to left display
  } BIDISTRUCT, *PBIDISTRUCT;


typedef struct _ARABICSTRUCT
{
  PLONG plCaretPos;     // pointer to caret positions
} ARABICSTRUCT, *PARABICSTRUCT;

/**********************************************************************/
/* RTF Control defines and functions                                  */
/**********************************************************************/
#define RTFLINE_BUFFERSIZE  (3*MAX_SEGMENT_SIZE)
#define OFFSET_NUM 500
typedef struct _EQFBRTFLINE
{
  ULONG  ulStartSeg;                   // start segment
  ULONG  ulEndSeg;
  ULONG  ulSegOffs[OFFSET_NUM];
  ULONG  ulSegTextOffs[OFFSET_NUM];
  USHORT usSegTextLen[OFFSET_NUM];
  ULONG  ulFirstLineIndex;
  CHAR_W chText[ RTFLINE_BUFFERSIZE + 500 ];
  CHAR_W chTempBuffer[ RTFLINE_BUFFERSIZE ];
} EQFBRTFLINE, *PEQFBRTFLINE;


/**********************************************************************/
/* extended fonts dialog and text types                               */
/**********************************************************************/

/**********************************************************************/
/* If you add a EXTTEXTTYPE, also increase the occurrence of the group*/
/* in the array usFontTypesInGroup!                                   */
/* The enum EXTTEXTTYPE is index in the table of fontsspecs!          */
/* (aszExtFontSpecs)                                                  */
/**********************************************************************/
typedef enum _EXTTEXTTYPE
{
        EXT_T_SOURCE,              // must remain the first one!!
        EXT_T_TRANSLATION,
        EXT_T_LAYOUT,
        EXT_T_ACTIVE,
        EXT_T_TRNOTE,
        EXT_T_RTFEDIT_TRNOTE,
        EXT_R_SCRATCH,
        EXT_R_MODIFIED,
        EXT_R_COPIED,
        EXT_R_LAYOUTCHANGES,
        EXT_D_HEADWORD,
        EXT_D_TRANSLATION,
        EXT_D_PREFIX,
        EXT_D_ADDITIONINFO,
        EXT_M_PREFIX,
        EXT_M_PMODIFIED,
        EXT_M_PINSERTED,
        EXT_M_PDELETED,

//      EXT_I_ANCHOR1,        // needed if ITM fonts added to tree
//      EXT_I_ANCHOR2,
//      EXT_I_ANCHOR3,
//      EXT_I_UNALIGNED,
//      EXT_I_CROSSEDOUT,
//      EXT_I_OVERCROSS,
//      EXT_I_ALIGN1,
//      EXT_I_ALIGN2,
//      EXT_I_ALIGN3,
        MAXEXTTYPE
} EXTTEXTTYPE;

/**********************************************************************/
/* the following fonts are mixed during display, not specially defined*/
/* by the user in the dialog                                          */
/**********************************************************************/
#define PROTECT_IN_SOURCE      (MAXEXTTYPE)
#define PROTECT_IN_CURRENT     (MAXEXTTYPE + 1)
#define PROTECT_IN_TRANSLATION (MAXEXTTYPE + 2)

/**********************************************************************/
/* MAX_MAXRTFFONTS must be the last of the above defines!!            */
/**********************************************************************/
#define MAX_MAXRTFFONTS   PROTECT_IN_TRANSLATION + 1

/**********************************************************************/
/* the EXTFONTGROUPS enums are used as index in the array             */
/* usTypesPerGroup, which contains the number of entries per Group!   */
/**********************************************************************/
typedef enum _EXTFONTGROUPS
{
      EXT_T_GROUP,
      EXT_R_GROUP,
      EXT_D_GROUP,
      EXT_M_GROUP,
//      EXT_I_GROUP,        //needed if ITM fonts added to tree
      MAXEXTGROUP
} EXTFONTGROUPS;


typedef struct _DISPFILERTF
{
  ULONG  ulSegNum;                                        // symbolic function identifier
  PVOID  pDoc;

  CHAR   chRTFFormat[7000];
  CHAR_W chRTFFormatW[1000];
  LONG   lPos;
  PSZ    pHeader;
  PSZ    pCharFormat[MAX_MAXRTFFONTS+1];

  PVOID    pQFRTFTagTable;       // pointer to tag table for RTF QFx tags
  EQF_BOOL fTBCursorReCalc;      // indicate, that we have to recalculate
  BYTE     bRTFFill;             // fill RTF Control - don't care about hidden/protected
  EQF_BOOL fDummy;               // overwrite status ....
  EQF_BOOL fRichEditLock;        // rich edit control locked
  LONG     orgRichEditProc;      // original richedit wnd proc
  USHORT   WYSIWYGType;          // Type of file for WYSIWYG
  EQFBRTFLINE RTFLine;           // our RTFLine struct
  SHORT    sTagIndeces[50];      // tag indices
  USHORT   qLastSegStatus;       // status of last segment
  PBYTE    pbBuff;
  LONG     lPoscb;
  LONG     cb;
  PSZ_W    pData[10];            // ptr to data aray...
  BOOL     fType[10];            // type array
  USHORT   usLoadState;          // loading state of file load
  PBYTE    pBufferOverflow;      // overflow buffer
  ULONG    ulBufferAllocated;    // size of overflow buffer
  USHORT   usFunc;               // function to be executed..
  HKL      hkl;                  // active keyboard selection
} DISPFILERTF, *PDISPFILERTF;

/**********************************************************************/
/* structures to be used for MTLOGging...                             */
/**********************************************************************/

typedef struct _proptypeOld
{
  unsigned short None     :1 ;              // true if no proposal
  unsigned short Exact    :1 ;              // true if exact proposal
  unsigned short Fuzzy    :1 ;              // true if fuzzy proposal
  unsigned short Replace  :1 ;              // true if replaced proposal
  unsigned short MT       :1 ;              // true if mt proposal
} PROPTYPEOLD;

// new proposal type bit flags with room for additional segment flags
typedef struct _proptype
{
  unsigned short None     :1 ;              // true if no proposal
  unsigned short Exact    :1 ;              // true if exact proposal
  unsigned short Fuzzy    :1 ;              // true if fuzzy proposal
  unsigned short Replace  :1 ;              // true if replaced proposal
  unsigned short MT       :1 ;              // true if mt proposal
  unsigned short GlobMem  :1 ;              // true if global memory proposal
  unsigned short GlobMemFuzzy :1 ;          // true if global memory fuzzy proposal 
  unsigned short Type03   :1 ;              // true if proposal of addional type 3
  unsigned short Type04   :1 ;              // true if proposal of addional type 4
  unsigned short Type05   :1 ;              // true if proposal of addional type 5
  unsigned short Type06   :1 ;              // true if proposal of addional type 6
  unsigned short Type07   :1 ;              // true if proposal of addional type 7
  unsigned short Type08   :1 ;              // true if proposal of addional type 8
  unsigned short Type09   :1 ;              // true if proposal of addional type 9
  unsigned short Type10   :1 ;              // true if proposal of addional type 10
  unsigned short Type11   :1 ;              // true if proposal of addional type 11
} PROPTYPE;


// this is the record layout up to TM622
typedef struct _ACTSEGLOGOLD
{
  ULONG    ulSegNum;                  // active segment number
  PROPTYPEOLD PropTypeExists;            // flags which props exist
  PROPTYPEOLD PropTypeCopied;            // flags which props copied
  UCHAR    ucNumDictCopied;           // counts how many dicts copied
  USHORT   usNumTyped;                // counts how many chars typed
  ULONG    ulTime;                    // end time (seg saved ) in secs
  USHORT   usWordCnt;                 // number of words in src seg
} ACTSEGLOGOLD, *PACTSEGLOGOLD;

// version info structure in MTLOGS
typedef struct _MTLOGVERSIONINFO
  {
    unsigned short Version :3;        // TM version - 5! (add 5 to get the correct number)
    unsigned short Release :4;        // TM release
    unsigned short Update :4;         // TM update
    unsigned short Driver :5;         // TM driver
} MTLOGVERSIONINFO;  
typedef struct _MTLOGVERSIONINFONEW
  {
    unsigned short Version :8;        // TM version
    unsigned short Release :8;        // TM release
    unsigned short Update :8;         // TM update
    unsigned short Driver :8;         // TM driver
} MTLOGVERSIONINFONEW;  


// record layout in TM622++ (fixed part of the record)
//   this fixed part is followed by ulSegLen bytes of the segment source text (UTF-16 encoded and 
//   null-terminated) and ulPropSourceLen bytes of the proposol source (document name in ASCII)
#define ACTSEGLOG_EYECATCHER 0xF3C2
typedef struct _ACTSEGLOGENH
{
  ULONG    ulEyeCatcher;              // record start identicator is always ACTSEGLOG_EYECATCHER
  ULONG    ulCheckSum;                // checksum over the remaining part of the record 
  ULONG    ulRecordLen;               // overall record length
  ULONG    ulSegLen;                  // length of the segment source text (in bytes), the segment source text is stored right behind the fixed part
                                      // of the record
  ULONG    ulPropSourceLen;           // length of proposal document name (in bytes), the doc name is stored behind the fixed part and the source text
  ULONG    ulPropSourceSegNum;        // segment number of proposal 
  ULONG    ulSegNum;                  // active segment number
  ULONG    ulTimeStamp;               // time when this record was written to the MTLOG
  ULONG    ulTime;                    // end time (seg saved ) in 1/100 secs
  USHORT   usFuzzyness;               // fuzzyness of best proposal
  MTLOGVERSIONINFO VersionInfo;                      // TM version info
  PROPTYPE PropTypeExists;            // flags which props exist
  PROPTYPE PropTypeCopied;            // flags which props copied
  UCHAR    ucNumDictCopied;           // counts how many dicts copied
  USHORT   usNumTyped;                // counts how many chars typed
  USHORT   usWordCnt;                 // number of words in src seg
  struct   _AddFlags                  // additional flags
  {
    unsigned short AutoSubst :1;      // true if proposal has been autosubstituted
    unsigned short ShipmentRec :1;    // true = this is a shipment separator record
    unsigned short Free2 :1;          // unused flag
    unsigned short Free3 :1;          // unused flag
    unsigned short Free4 :1;          // unused flag
    unsigned short Free5 :1;          // unused flag
    unsigned short Free6 :1;          // unused flag
    unsigned short Free7 :1;          // unused flag
  } AddFlags;
} ACTSEGLOGENH, *PACTSEGLOGENH;

// record layout in TM642++ (fixed part of the record)
//   this fixed part is followed by ulSegLen bytes of the segment source text (UTF-16 encoded and 
//   null-terminated) and ulPropSourceLen bytes of the proposol source (document name in ASCII), 
//   ulMetaDataLen bytes of the MT proposal meta data (UTF-16 encoded and null-terminated) and 
//   ulPEDataLen bytes of the PE data area 

#define ACTSEGLOG2_EYECATCHER 0xF3C4
typedef struct _ACTSEGLOGENH2
{
  ULONG    ulEyeCatcher;              // record start identicator is always ACTSEGLOG2_EYECATCHER
  ULONG    ulCheckSum;                // checksum over the remaining part of the record 
  ULONG    ulRecordLen;               // overall record length
  ULONG    ulSegLen;                  // length of the segment source text (in bytes), the segment source text is stored right behind the fixed part
                                      // of the record
  ULONG    ulPropSourceLen;           // length of proposal document name (in bytes), the doc name is stored behind the fixed part and the source text
  ULONG    ulPropSourceSegNum;        // segment number of proposal 
  ULONG    ulSegNum;                  // active segment number
  ULONG    ulTimeStamp;               // time when this record was written to the MTLOG
  ULONG    ulTime;                    // end time (seg saved ) in 1/100 secs
  USHORT   usFuzzyness;               // fuzzyness of best proposal
  MTLOGVERSIONINFO  VersionInfo;      // TM version info
  PROPTYPE PropTypeExists;            // flags which props exist
  PROPTYPE PropTypeCopied;            // flags which props copied
  UCHAR    ucNumDictCopied;           // counts how many dicts copied
  USHORT   usNumTyped;                // counts how many chars typed
  USHORT   usWordCnt;                 // number of words in src seg
  struct   _AddFlags2                 // additional flags V2
  {
    unsigned short AutoSubst :1;      // true if proposal has been autosubstituted
    unsigned short ShipmentRec :1;    // true = this is a shipment separator record
    unsigned short ComputeThinkTime :1; // used for think time computation, in use only
    unsigned short Free3 :1;          // unused flag
    unsigned short Free4 :1;          // unused flag
    unsigned short Free5 :1;          // unused flag
    unsigned short Free6 :1;          // unused flag
    unsigned short Free7 :1;          // unused flag
  } AddFlags;
  ULONG    ulChoiceTime;              // time in 1/100 secs used for the translator choice (activation until first copy)
  ULONG    ulTotalTime;               // overall segment processing time (in 1/100 secs)
  ULONG    ulMetaDataLen;             // length of MT proposal meta data (in bytes), the meta data is stored behind the fixed part, the source text and any document name
  ULONG    ulThinkTime;               // time in 1/100 secs used for the translator think time (copy of proposal until first edit operations)
  MTLOGVERSIONINFONEW  VersionInfoNew;// new TM version info (use this if it is non-zero)
  ULONG    ulPEDataLen;               // length of PE (Post Edit) data area (in bytes), the PE data is stored behind the fixed part, the source text, any document name and the MT meta data 
  BYTE     bUnused[4];                // room for future enhancements
} ACTSEGLOGENH2, *PACTSEGLOGENH2;

// layout of the fixed part of the PE data area at the end of the MTLOG entry
//   the fixed part is followed by a variable length part containing the various PE data strings

typedef struct _PESEGLOGDATA
{
  ULONG      ulSize;                  // total size of PE data area (in bytes)
  ULONG      ulTargetLen;             // length of the target text of the segment or 0 if not used (in bytes including the terminating zero character)
  ULONG      ulTargetOffs;            // offset to the target text of the segment or 0 if not used (relativ to begin of data structure)
  ULONG      ulMTTargetLen;           // length of the target text of the MT proposal or 0 if not used (in bytes including the terminating zero character)
  ULONG      ulMTTargetOffs;          // offset to the target text of the MT proposal or 0 if not used (relativ to begin of the data structure )
  ULONG      ulMTMatchIDLen;          // length of the match segment ID of the MT proposal or 0 if not used (in bytes including the terminating zero character)
  ULONG      ulMTMatchIDOffs;         // offset to the match segment ID of the MT proposal or 0 if not used (relativ to begin of the data structure )
  ULONG      ulCopiedTargetLen;       // length of the target text of the copied proposal or 0 if not used (in bytes including the terminating zero character)
  ULONG      ulCopiedTargetOffs;      // offset to the target text of the copied proposal or 0 if not used (relativ to begin of the data structure )
  ULONG      ulCopiedMatchIDLen;      // length of the match segment ID of the copied proposal or 0 if not used (in bytes including the terminating zero character)
  ULONG      ulCopiedMatchIDOffs;     // offset to the match segment ID of the copied proposal or 0 if not used (relativ to begin of the data structure )
} PESEGLOGDATA, *PPESEGLOGDATA;

// structure containing buffers for the collected PE data
typedef struct _PEDATABUFFER
{
  CHAR_W      szTarget[MAX_SEGMENT_SIZE+1];      // final translation of the segment 
  CHAR_W      szCopiedTarget[MAX_SEGMENT_SIZE+1];// translation of the last copied proposal
  CHAR_W      szCopiedMatchID[256];              // segment match ID of the last copied proposal
  CHAR_W      szMTTarget[MAX_SEGMENT_SIZE+1];    // translation of the first available MT proposal
  CHAR_W      szMTMatchID[256];                  // segment match ID of the first available MT proposal
} PEDATABUFFER, *PPEDATABUFFER;


VOID WriteMTLog( PACTSEGLOGENH2 pSegLog, PSZ_W pszSegSource, PSZ pszPropSourceDoc, PSZ_W pszMetaData, PPEDATABUFFER pPEData );
VOID WriteToMTLog( FILE *hMTLogFile, PACTSEGLOGENH2 pSegLog, PSZ_W pszSegSource, PSZ pszPropSourceDoc, PSZ_W pszMetaData, PPEDATABUFFER pPEData );

typedef struct _tbdocument
{
    struct _tbdocument *next;      /* Pointer to the next document     */
    struct _tbdocument *prev;      /* Pointer to the previous document */
    struct _tbdocument *twin;      /* Pointer to the correlating source/target */
    CHAR  chTitle[MAX_PATH144];    // title of window
    DOCTYPE  docType;            // document type
    struct
    {
       unsigned short changed : 1;     /* TRUE if the document has changed */
       unsigned short fGotoSegMode : 1; // TRUE = show goto line dialog in goto segment mode
    } flags;
//    SHORT sCursorRow;            // Current screen cursor row
//    SHORT sCursorCol;            // Current screen cursor column
//    SHORT sSideScroll;           // Current side-scroll value
    // the above 3 shorts are replaced by one SHORT-DUMMY and one LONG lSideScroll
    SHORT  sDummy02;             // dummy
    LONG   lSideScroll;          // Current side-scroll value
    ULONG  ulVideoBuffer;        // adress of video buffer
    USHORT usVideoLength;        // length of video buffer
    LONG   cx, cy;               // Size of AVOI char cell in pixels
    LONG   xClient, yClient;     // Size of client area in pixels
    // the 8 SHORTS below are replaced by the 4 LONGs above!!
    // USHORT cx, cy;
    // USHORT xClient, yClient;
    // SHORT sScrnRows,sScrnCols;
    //SHORT  mouseRow, mouseCol;   // Mouse row and col
    HWND   hwndFrame;            // frame handle of new window
    HWND   hwndClient;           // client area of new window
    SWP    swp;                  // position of this window
    REDRAW   Redraw;             // redraw status of this window
    ULONG    ulSegTables;        // overall number of segment tables
    ULONG    ulSegsPerTable;     // max number os segments per segment table
    PTBSEGMENTTABLE pSegTables;  // ptr to segment tables
    CHAR     szDocName[CCHMAXPATH];// name of document (full qualified)
    PVOID    pstEQFGen;          // pointer to generic structure
    PBYTE    pInBuf;             // input buffer (for file read)
    PBYTE    pTokBuf;            // buffer for tokenization
    PVOID    pQFTagTable;        // pointer to tag table for QFx tags
    PVOID    pDocTagTable;       // pointer to tag table for document tags
    CHAR     EQFBWorkSegment[MAX_SEGMENT_SIZE + 1];  // work segment area
    ULONG    ulWorkSeg;          // working segment number
    BOOL     fXlated;            // document translated indicator
    // 8* sizeof(LONG) = 8*4 = 32 must be subtracted from BYTE dummy!
    // sizeof(TBSEGMENT) = 44 ( 03/02/12)
    LONG     mouseRow;           // inserted!! ( 03/02/12)
    LONG     mouseCol;           // inserted! (03/02/12)
    LONG     lScrnRows;          // inserted- 03/02/12 number of rows
    LONG     lScrnCols;          // inserted- 03/02/12 number of cols
    LONG     lCursorRow;         // inserted- 03/02/12 Current screen cursor row
    LONG     lCursorCol;         // inserted- 03/02/12 Current screen cursor column
    LONG     lDBCSCursorCol;     // for DBCS: old CursorCol
    LONG     ulRulerSize;        // inserted - 03/02/12 Ruler size
                                 // dummy! was former tbActSeg;
    //BYTE     Dummy[44 - 4 - 32];
    BOOL       fImeStartComposition;  // P017862 / BOOL needs 4 bytes!
    BYTE       Dummy[4];
      // since tbActSeg is 4 bytes bigger now, it must have been moved to the end
    TBROWOFFSET  TBRowOffset[MAX_SCREENLINES+10] ; // row/offset table
    TBROWOFFSET  TBCursor;       // cursor segment/offset
    ULONG        ulMaxSeg;       // number of segments in file
    ULONG        ulMaxLine;      // number of lines in file
    USHORT       usChar;         // last character entered
    PSZ          pSaveSeg;       // save prev. segdata for total undo
    DOCFLAGS     EQFBFlags;      // structure of flags
    DISPSTYLE    DispStyle;      // display style active
    USHORT       usCursorType;   // active cursor type
    CHAR         szLineBreak[6]; // linebreak characters used for document
    SHORT        sPropCopied;    // Proposal number copied
    USHORT       usDBCS2Char;    // 2nd char from DBCS
    BOOL         fFuzzyCopied;   // fuzzy match copied
    USHORT       usMaxScrnRows;  // max number of rows
    USHORT       usMaxScrnCols;  // max number of cols
    USHORT       usScrnUpdStart; // update start
    USHORT       usScrnUpdEnd;   // update end
    PVOID        pBlockMark;     // pointer to marked area
    SHORT        sRMargin;       // value of right margins
    BOOL         fLineWrap;      // line wrap On/Off
    NEWVIOCURSORINFO vioCurShapes[MAXCURSOR];  // cursor shapes
    USHORT       usTagEnd;       // tag end character
    BOOL         fErrorProcessed;// allocation error processed - no displ. msg
    PSZ          pUndoSeg;       // save prev seg data for undo in parts
    BOOL         fUndoState;     // true if no update nec. for char.typing
    USHORT       usUndoSegOff;   // cursor seg.offset of UndoSeg in buffer
    ULONG        ulAutoSegNum;   // SegNum where Automatic started
    PUSEROPT     pUserSettings;                  // initial settings
    CHAR         szInTagAbbr[MAXSHRKLEN+1];      //abbreviation for inline tags
    CHAR         szOutTagAbbr[MAXSHRKLEN+1];     //abbreviation for NOP segments
    CHAR         szInTagLFAbbr[MAXSHRKLEN+2];    //abbreviation for inline tags
    CHAR         szOutTagLFAbbr[MAXSHRKLEN+2];   //abbreviation for NOP segments
    PTBSEGMENT   pTBSeg;                         // ptr to usWorkSeg
    BOOL         fSpellCheck;                    // spell checking enabled??
    PCHAR        pSegmentBuffer;                 //buffer for segm. for SO/SI
    USHORT       usWndId;                        // id of doc in windows pulldown
    USHORT       usNumTyped;                     // number chars typed(KIT920)
    HMODULE      hModule;               // address of user exit module handle
    PFN          pfnUserExit;           // address of ptr to user exit function
    PFNCHECKSEGEXIT pfnCheckSegExit;    // address of ptr to segm.check function
    PVOID        pTMMaint;              // ptr to data for TM Maint. function
    PFN          pfnTMMaint;            // address of ptr to TM Maint. function
    LOGFONT      lf;                    // log font structure
    USHORT       usDummy1;                    // active code page..
    HGLOBAL      hIME;                  // handle for data for DBCS Input meth
    HMODULE      hIMEDll;               // handle of IME dll
    PFN          pfnSendIME;            // address of ptr to IME SendMsg func
    HGLOBAL      hlfIME;                // handle for logfont data
    UCHAR        ucState;           // status of keyboard flags...
    PFNSHOWTRANS pfnShowTrans;      // show translation user exit func.ptr.
    CHAR         szDocLongName[MAX_LONGFILESPEC];   // long name of document

    ULONG        ulVScroll;            // number of lines scrolled from top
    HWND         hStatusBarWnd;        // handle of statusbar
    CHAR         chColStatusText[8];   // status text for column
    CHAR         chLineStatusText[8];  // status text for line
    CHAR         chSegStatusText[8];   // status text for segment
    CHAR         chInsStatusText[8];   // status text for Insert
    HWND         hRulerWnd;            // define handle of ruler window
    USHORT       usDummy04;            // size of ruler window
    CHAR         szSegBound[2];        // segment boundery sign
    EQF_BOOL     fAutoLineWrap;        // true if linewrap according to wndsize
    ULONG        ulEyeCatcher;         // space for eyecatcher
    CHAR         FreeDummy[MAXTRNOTE_SIZE+2];         // TRNOTE abbrv w/o LF
    EQF_BOOL     fTransEnvAct;         // translation environment active
    PFNTOCGOTO   pfnTocGoto;           // TOC goto user exit function
    HWND         hwndTOCGotoLB;        // handle of listbox in TOC Goto dialog
    LONG         lFontLangInfo;        // font language info, in particular GCP_REORDER

    HANDLE       hCaretBitmap;         // caret bitmap if special loaded

    PBIDISTRUCT  pBidiStruct;          // pointer to bidi specifics

    ULONG        ulAddSegTables;       // overall num. of additional seg.tables
    ULONG        ulAddSegsPerTable;    // max num. of segs. per add.seg.table
    PTBSEGMENTTABLE pAddSegTables;     // ptr to additoinal segment tables
    EQF_BOOL     fThreadAct;           // workerthread active
    EQF_BOOL     fThreadNotIdle;       // thread idle ready for work
    EQF_BOOL     fThreadKill;          // Kill Thread
    USHORT       usThreadTask;         // active task for thread
    PVOID        pvSpellData;          // pointer to spelldata structure
    EQF_BOOL     fAutoSpellCheck;      // true if autospellcheeck
    USHORT       usLangTypeSrc;        // source language type
    USHORT       usLangTypeTgt;        // target language type
    PVOID        pvFileWriteData;      // ptr to FileWrite structure (used by AutoSave thread)
    LONG         lTimeLastFileSave;    // sec's when transl. saved
    EQF_BOOL     fForceEqualWhiteSpace;
    USHORT       usTripleClkTimerID;   // timer id for triple click (- 2 of res)
    HWND         hwndRichEdit;         // handle of richedit ctrl
    PDISPFILERTF pDispFileRTF;         // ptr to RTF relevant data structure
    EQF_BOOL     fStoreInUnicode;      // store-file-in-Unicde flag
    CHAR         chLoading[40];        // resource string for loading file
    ULONG        ulAnsiCodePage;       // Ansi code page to be used
    ULONG        ulOemCodePage;        // OEM code page to be used
    EQF_BOOL     fTARight;             // window is right adjusted
    PARABICSTRUCT pArabicStruct;       // structure for arabic specificas
    USHORT       usLastCharLength;     //NOT USED ANY MORE!!
    // parameters added for UNICODE enabling
    PSZ_W        pEQFBWorkSegmentW;    // work segment area
    PSZ_W        pSaveSegW;             // save prev. segdata for total
    PSZ_W        pUndoSegW;       // save prev seg data for undo in parts
    CHAR_W       szInTagAbbrW[MAXSHRKLEN+1];      //abbreviation for inline tags
    CHAR_W       szOutTagAbbrW[MAXSHRKLEN+1];     //abbreviation for NOP segments
    CHAR_W       szInTagLFAbbrW[MAXSHRKLEN+2];    //abbreviation for inline tags
    CHAR_W       szOutTagLFAbbrW[MAXSHRKLEN+2];   //abbreviation for NOP segments
    PCHAR_W      pSegmentBufferW;                 //buffer for segm. for SO/SI
    PFNCHECKSEGEXITW pfnCheckSegExitW;  // address of ptr to segm.check function (UNICODE)
    CHAR_W       szLineBreakW[6]; // linebreak characters used for document
    CHAR_W       chTRNoteAbbrW[MAXTRNOTE_SIZE+2];
    CHAR_W       chTRNoteLFAbbrW[MAXTRNOTE_SIZE+2];
    TBSEGMENT    tbActSeg;         // active segment

    // variables for segment context handling
    PFNGETSEGCONTTEXT pfnGetSegContext;     // ptr to user exit entry point EQFGETSEGCONTEXT
    ULONG        ulContextSeg;              // segment for which the context has been requested
    PSZ_W        pContext;                  // buffer for context strings
    PFN          pfnUserExitW;
    CHAR_W       chVisibleBlank;            // visible blank in Unicode
    CHAR_W       chVisibleLF;               // visible lf in Unicode
    CHAR_W       chSegBound;                // segment bound character
    BYTE         bOperatingSystem;          // operating system (2k, Nt, etc.)
    SHORT        sSrcLanguage;              // Lang.ID of Src Language
    PSZ_W        pWSList;                   // array with whitespace chars from TBX

    // variables for entity processing
    CHAR_W       szCurWord[128];            // word under cursor
    CHAR_W       szCurValue[256];           // value for word under cursor
    CHAR         szCurStatusLine[512];      // currently displayes status line
    CHAR         szNewStatusLine[512];      // buffer for new status line
    LONG         lEntity;                   // Entity handle

    PFNCHECKSEGEXEXITW pfnCheckSegExExitW;    // address of ptr to segm.checkEx function (UNICODE)

    ACTSEGLOGENH2 ActSegLog;                 // buffer for MT logging data
    CHAR         szProposalDocName[MAX_LONGFILESPEC]; // document name of best proposal


    // global memory option file data
    PVOID         pvGlobalMemOptFile;       // loaded global memory option file
    CHAR          szGlobalMemOptFile[MAX_LONGFILESPEC]; // name of global memory option file

    PFNFORMATCONTTEXT pfnFormatContext;     // ptr to user exit entry point EQFFORMATCONTEXT
    CHAR_W     szContextBuffer[MAX_SEGMENT_SIZE + 1];  // buffer for context strings


    CHAR_W     szMetaData[MAX_SEGMENT_SIZE + 1];  // buffer for meta data (add info data) of copied proposal

    BOOL        fSaveRunning;              // TRUE when document is currently saved to disk

    UCHAR        ucCode;            // character code or virtual key    // Add for R012027

    PEDATABUFFER PEData;            // buffer to collect post edit data

    // Reserve left:
    CHAR         chReserve[1000];             
 } TBDOCUMENT, *PTBDOCUMENT;

VOID MTLogStartEditing( PTBDOCUMENT pDoc );
VOID MTLogProposalCopied( PTBDOCUMENT pDoc );
VOID MTLogUndoProposalCopy( PTBDOCUMENT pDoc );


#define RTF_FILL     1      // fill RTF control segment by segment
#define RTF_INITFILL 2      // initial file fill
#define RTF_FASTFILL 4      // initial file fill via STREAMING

    typedef struct _eqfbblock
    {
       PTBDOCUMENT  pDoc;              // pointer to marked document
       ULONG   ulSegNum;               // segment number marked
       USHORT  usStart;                // start of marked area
       USHORT  usEnd;                  // end of marked area
       ULONG   ulEndSegNum;            // segment number of usEnd position
    }  EQFBBLOCK, *PEQFBBLOCK;

//+----------------------------------------------------------------------------+
//|  structure used to store color/font values for text types                  |
//+----------------------------------------------------------------------------+
typedef struct _TEXTTYPETABLE
{
   SHORT   sFGColor;                   // foreground color
   SHORT   sBGColor;                   // background color
   USHORT  usFont;                     // font symbolic name
   EQF_BOOL  fUnderscore;              // underscore flag
   EQF_BOOL  fReverse;                 // reverse flag
} TEXTTYPETABLE, *PTEXTTYPETABLE;



__declspec(dllexport)
USHORT     EQFBFileRead( PSZ pszFileName, PTBDOCUMENT pDoc);  // EQFBFILE.C
USHORT     EQFBFileReadEx( PSZ, PTBDOCUMENT, LONG );
__declspec(dllexport)
USHORT     EQFBFileReadExW(PSZ, PTBDOCUMENT, LONG );
VOID EQFBParseGetCP (  PSZ  pszInFile, PULONG pulSrcOemCP, PULONG pulTgtOemCP,
                        PULONG pulSrcAnsiCP, PULONG pulTgtAnsiCP );

#define EQFBFREEDOC_NOTAGTABLEFREE  0x00000001L
#define EQFBFREEDOC_NOPROTECTFREE   0x00000002L
#define EQFBFREEDOC_NODOCIDAFREE    0x00000004L
__declspec(dllexport)
void EQFBFreeDoc( PTBDOCUMENT *ppDoc, ULONG ulOptions );

#define FILEREAD_FPOS  0x02L
#define FILEREAD_SINGLETABLE  0x04L
#define FILEREAD_METADATA     0x08L

VOID       EQFBXlateConv(PTBDOCUMENT, PSZ);

__declspec(dllexport)
USHORT     EQFBFileWrite( PSZ pszFileName, PTBDOCUMENT pDoc); // EQFBFILE.C
__declspec(dllexport)
USHORT     EQFBFileWriteEx( PSZ pszFileName, PTBDOCUMENT pDoc,
                            SHORT sLogTaskID, USHORT usCPConv );

#define STANDARDTABLE   0            // regular SegTable
#define ADDITIONALTABLE 1            // SegTable not used in standard editor
PTBSEGMENT EQFBGetSegEx ( PTBDOCUMENT, ULONG,  USHORT );

__declspec(dllexport)
PTBSEGMENT EQFBGetSeg ( PTBDOCUMENT, ULONG  );                // in EQFBDOC.C
__declspec(dllexport)
PTBSEGMENT EQFBGetSegW ( PTBDOCUMENT, ULONG );               // pSeg->pDataW filled!
__declspec(dllexport)
PTBSEGMENT   EQFBGetFromBothTables(PTBDOCUMENT, PULONG, PULONG, PULONG);

__declspec(dllexport)
SHORT      EQFBAddSeg( PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg );
__declspec(dllexport)
SHORT      EQFBAddSegW( PTBDOCUMENT pDoc, PTBSEGMENT pNewSeg );
__declspec(dllexport)
PTBSEGMENT EQFBGetVisSeg( PTBDOCUMENT pDoc, PULONG    pulSegNum);
__declspec(dllexport)
USHORT     EQFBGetReqSeg ( PTBDOCUMENT, SHORT, PULONG, PCHAR_W, PUSHORT, PUSHORT);
__declspec(dllexport)
MRESULT    EQFBDispClass( HWND, WINMSG, WPARAM, LPARAM, PTBDOCUMENT );
__declspec(dllexport)
void EQFBSetResetFrameCtrls ( PTBDOCUMENT, HWND, ULONG  );
__declspec(dllexport)
VOID EQFBInitMenu ( PTBDOCUMENT, USHORT );            // enable/disable AAB item

__declspec(dllexport)
VOID       EQFBGetColors ( PTEXTTYPETABLE );
__declspec(dllexport)
VOID       EQFBFuncChangeFocusLine ( SHORT );
__declspec(dllexport)
SHORT      EQFBFuncGetFocusLine();
__declspec(dllexport)
VOID       EQFBRefreshScreen ( PTBDOCUMENT );        // refresh documents
__declspec(dllexport)
VOID       EQFBVioSetNewDocSize ( PTBDOCUMENT );     // refresh doc size

__declspec(dllexport)
   VOID    ImeMoveConvertWin ( PTBDOCUMENT, HWND, SHORT, SHORT );
__declspec(dllexport)
   VOID    ImeSetFont( PTBDOCUMENT, HWND, PLOGFONT );

__declspec(dllexport)
BOOL EQFBFindDiffEx( PVOID, PBYTE, PBYTE, PSZ_W, PSZ_W, SHORT , PVOID * , PVOID *, ULONG);
__declspec(dllexport)
BOOL EQFBCountDiff( PVOID pvFuzzyTok, PUSHORT pusTokens, PUSHORT pusDiffs );
__declspec(dllexport)
BOOL EQFBDoEmptySegCheck( PTBDOCUMENT ); // do empty segment handling

__declspec(dllexport)
BOOL EQFBMousePosition ( PTBDOCUMENT, WINMSG, WPARAM, LPARAM ); // position mouse ...

__declspec(dllexport)
BOOL EQFBMouseOnActSeg ( PTBDOCUMENT, LONG, LONG );
__declspec(dllexport)
GLOBALHANDLE  EQFBFuncCopyToHGlob( PTBDOCUMENT );
VOID  InitBIDIVars( VOID );

int BiDitransform
   ( unsigned char * inp_buff,
     size_t inp_len,
     unsigned short inp_ccsid,
     unsigned short inp_strtype,
     unsigned char * out_buff,
     size_t *out_len,
     unsigned short out_ccsid,
     unsigned short out_strtype,
     unsigned short cont_mode,
     PUINT    puOrder );

#define BDT_SUCCESS 0
#define BDTERR_BAD_OUTLEN 1
#define BDTERR_BAD_CCSID 2
#define BDTERR_DIFFERENT_CODEPAGES 3
#define BDTERR_NOMEM 4

__declspec(dllexport)
  SHORT EQFBWordCntPerSeg (PVOID, PTOKENENTRY, PSZ_W, SHORT, PULONG,PULONG, ULONG);
  SHORT EQFBCntAllSrcWords( PTBDOCUMENT, SHORT sSourceLang, ULONG ulOemCP );
__declspec(dllexport)
  SHORT EQFBHistDocSave(PSZ, PTBDOCUMENT, SHORT );
__declspec(dllexport)
  SHORT EQFBHistDocSaveEx( PSZ, PTBDOCUMENT, SHORT, BOOL );
  VOID  EQFBShowFindChangeDlg( BOOL );   // show or hide
  HWND  EQFBFindChangeDlgHwnd( VOID );   // return handle of FindChange Dlg
__declspec(dllexport)
  VOID  EQFBBufRemoveTRNote( PSZ_W, PVOID, PFN, PFN, ULONG );
__declspec(dllexport)
  BOOL  EQFBInit ( VOID );                                 // init editor wnd class
__declspec(dllexport)
  VOID  EQFBGotoSeg( PTBDOCUMENT,ULONG,USHORT);  // position file at segment
__declspec(dllexport)
  VOID EQFBSetNewCellSize ( PTBDOCUMENT, ULONG, ULONG );// set new cell size

__declspec(dllexport)
  VOID  EQFBGetUserSettings ( PTBDOCUMENT );
  VOID  EQFBGenProcessing( HWND, PTBDOCUMENT, USHORT );
__declspec(dllexport)
  void  EQFBFuncTopDoc(PTBDOCUMENT);
  MRESULT HandleEEAWMChar ( HWND, WINMSG, WPARAM, LPARAM, PTBDOCUMENT );

__declspec(dllexport)
  VOID EQFBSoftLFInsert ( PTBDOCUMENT );
__declspec(dllexport)
  VOID EQFBSoftLFRemove ( PTBDOCUMENT );
  BOOL EQFBBufAddSoftLF ( PTBDOCUMENT    pDoc,
                          PTBSEGMENT     pSeg,
                          PSZ_W          pData,
                          LONG           lSegStartCol,
                          PLONG          plSegEndCol,
                          PUSHORT        pusSegOffOld );



__declspec(dllexport)
VOID EQFBBufRemoveSoftLF(HWND, PCHAR_W, PUSHORT, PUSHORT);

__declspec(dllexport)
BOOL   EQFBEnableToolbarItem( HWND  hwnd, USHORT usID );



typedef struct _FUZZYTOK
{
  PSZ_W  pData;                    // pointer to data..
  ULONG  ulHash;                   // hash value
  USHORT usStart;                  // start offset
  USHORT usStop;                   // end offset
  SHORT  sType;                    // type of entry              /* KIT0857M */
  EQF_BOOL fConnected;             // true if connected to pPropTokList
  EQF_BOOL fNoTagSubst;            // true = do not use for tag substitution
} FUZZYTOK, *PFUZZYTOK;

typedef struct _REPLLIST
{
   PFUZZYTOK  pSrcTok;              // ptr to source token
   PFUZZYTOK  pTgtTok;              // ptr to target token
} REPLLIST, *PREPLLIST;


__declspec(dllexport)
USHORT FuzzyLCSReplList     ( PFUZZYTOK, PFUZZYTOK, PREPLLIST *,
                              USHORT, USHORT, PUSHORT, BOOL);

SHORT StrNLFCmp ( PSZ_W, PSZ_W );

__declspec(dllexport)
BOOL   EQFBGetHexNumber( PSZ pszNumber, PUSHORT pusValue );
__declspec(dllexport)
BOOL EQFBGetHexNumberW( PSZ_W pszNumber, PUSHORT pusValue );

__declspec(dllexport)
VOID   EQFBFillWriteAttrW ( PVOID, PSZ_W, PSZ_W, PSZ_W, PSZ_W, PSZ_W, PSZ_W, PSZ_W );
__declspec(dllexport)
VOID   EQFBFillWriteAttr ( PVOID, PSZ, PSZ, PSZ, PSZ, PSZ, PSZ, PSZ );
__declspec(dllexport)
VOID          EQFBResetAllFonts(PTBDOCUMENT);
//+----------------------------------------------------------------------------+
//|  symbolic values for colors (which are indeed the actual value for colors) |
//+----------------------------------------------------------------------------+
typedef enum _VIOCOLORS
{
   COL_BLACK,                          // =  0
   COL_BLUE,                           // =  1
   COL_GREEN,                          // =  2
   COL_CYAN,                           // =  3
   COL_RED,                            // =  4
   COL_PINK,                           // =  5
   COL_BROWN,                          // =  6
   COL_LIGHTGRAY,                      // =  7
   COL_DARKGRAY,                       // =  8
   COL_LIGHTBLUE,                      // =  9
   COL_LIGHTGREEN,                     // = 10
   COL_LIGHTCYAN,                      // = 11
   COL_LIGHTRED,                       // = 12
   COL_LIGHTPINK,                      // = 13
   COL_YELLOW,                         // = 14
   COL_WHITE,                          // = 15
   MAXVIOCOLOR
} VIOCOLORS;

  __declspec(dllexport)
  COLORREF * EQFBGetColorRGB ( );
__declspec(dllexport)
PUSHORT EQFBGetFontTypesInGroup ( );
__declspec(dllexport)
CHARFORMAT2 * EQFBGetFontSpecs();
CHARFORMAT2 * EQFBGetDefFontSpecs();
__declspec(dllexport)
VOID EQFBSetCharFormat2 ( CHARFORMAT2 *, PSZ, BYTE, BYTE, LONG, SHORT,
                          COLORREF, COLORREF, SHORT, SHORT );

/**********************************************************************/
/* parameters defines for EQFBSetCharFormat2                          */
/**********************************************************************/
#define STRIKEOUT       1
#define UNDERLINE_NOSPEC  0
#define UNDERLINE_NONE    1
#define UNDERLINE_YES     2

#define STYLE_STANDARD    0
#define STYLE_ITALIC      1
#define STYLE_BOLD        2
#define STYLE_ITALBOLD    3


VOID EQFBResetSize ( PTBDOCUMENT );                        // reset the size displ.font
BOOL CheckForAnsiConv ( PTBDOCUMENT pDoc) ;

VOID   EQFBUpdateDispTable(PTBDOCUMENT pDoc);


// Macros for Bidi Specifics
#define IS_RTL( pDoc ) ((pDoc->usLangTypeTgt & ( MORPH_BIDI_A_LANGTYPE | MORPH_BIDI_H_LANGTYPE )) || (pDoc->usLangTypeSrc & ( MORPH_BIDI_A_LANGTYPE | MORPH_BIDI_H_LANGTYPE )))
#define IS_RTL_TGT( pDoc ) (pDoc->usLangTypeTgt & ( MORPH_BIDI_A_LANGTYPE | MORPH_BIDI_H_LANGTYPE ))
#define IS_RTL_SRC( pDoc ) (pDoc->usLangTypeSrc & ( MORPH_BIDI_A_LANGTYPE | MORPH_BIDI_H_LANGTYPE ))
#define IS_RTL_ARABIC( pDoc ) ((pDoc->usLangTypeTgt & MORPH_BIDI_A_LANGTYPE ) || (pDoc->usLangTypeSrc & MORPH_BIDI_A_LANGTYPE ))
#define IS_RTL_HEBREW( pDoc ) ((pDoc->usLangTypeTgt & MORPH_BIDI_H_LANGTYPE ) || (pDoc->usLangTypeSrc & MORPH_BIDI_H_LANGTYPE ))
#define IS_RTL_ARABIC_TGT( pDoc ) (pDoc->usLangTypeTgt & MORPH_BIDI_A_LANGTYPE )
#define IS_RTL_ARABIC_SRC( pDoc ) (pDoc->usLangTypeSrc & MORPH_BIDI_A_LANGTYPE )

PSZ_W EQFBGetContext( PTBDOCUMENT pDoc, PTBSEGMENT pSeg, ULONG ulSegNum );
BOOL EQFBRelCurPos(  PTBDOCUMENT pDoc, LONG lCol, LONG  lAbsPos  );
LONG EQFBGetAbsPos(  PTBDOCUMENT pDoc, LONG lCol  );
__declspec(dllexport)
USHORT EQFBBuildCountCheckSum
(
  USHORT usCountFlags,
  USHORT usSrcWords,
  USHORT usTgtWords,
  USHORT usModWords
);

BOOL EQFBIsLFProtected( PTBSEGMENT pSeg, SHORT sOffs);
__declspec(dllexport)
VOID UtlGetUTF16VisibleWhiteSpace (	PTBDOCUMENT, PUSEROPT, ULONG);

SHORT EQFBIsNumLFEqual(PSZ_W pData1, PSZ_W pData2, PSHORT sNumLF1, PSHORT sNumLF2);
USHORT TAIteratedCreateProtectTableW( PTBDOCUMENT pDoc, PTBSEGMENT  pSeg, int iMaxIterations );

// metadata (segment property) structures and functions

// data area for a proposal extracted from the segment metadata
typedef struct _MD_PROPOSAL
{
  CHAR_W           szSource[MAX_SEGMENT_SIZE+1]; // proposal source
  CHAR_W           szTarget[MAX_SEGMENT_SIZE+1]; // proposal target
  SHORT            sQuality;                     // proposal quality 1..100
  CHAR             szDocName[MAX_LONGFILESPEC];  // document of proposal
  CHAR             szAuthorName[MAX_USERID];     // author of target
  ULONG            ulSegmentId;                  // seg. number of proposal source
  LONG             lDate;                        // creation/update date of proposal
  PVOID            pvNext;                       // internally used, do not modify
  BYTE             bFutureUse[200];              // room for enhancements
} MD_PROPOSAL, *PMD_PROPOSAL;

BOOL MDGetFirstProposal( PVOID pvMetadata, PMD_PROPOSAL pProposal );
BOOL MDGetNextProposal( PMD_PROPOSAL pProposal );

__declspec(dllexport)
void MDEndDialog( PTBDOCUMENT pDoc );
__declspec(dllexport)
BOOL MDStartDialog( PTBDOCUMENT pDoc );
BOOL MDRefreshMetadata( PTBDOCUMENT pDoc, PTBSEGMENT pSeg, PSZ_W pszDisplayContext  );
__declspec(dllexport)
BOOL MDGetMetadata( PTBDOCUMENT pDoc, PTBSEGMENT pSeg, BOOL fSegModified  );
BOOL MDIsWindowVisible();
__declspec(dllexport)
USHORT MDWriteMetaData( PTBDOCUMENT pDoc );
__declspec(dllexport)
USHORT MDLoadMetaData( PTBDOCUMENT pDoc );
void MDShowSegPropDialog( BOOL fShow );
__declspec(dllexport)
BOOL MDDialogWasActive();
__declspec(dllexport)
BOOL MDCommentHasChanged( PTBDOCUMENT pDoc );
__declspec(dllexport)
void MDAddCommentData( PVOID *ppData, PSZ_W pszComment, PSZ_W pszStyle );




// Convert the binary segment meta data format to the XML based memory meta data format
__declspec(dllexport)
BOOL MDConvertToMemMetadata( PVOID pvSegMetaData, PSZ_W pszMemMetaData );

// defines used in tokenlists of EQFBxxFuzzyxxx functions
#define PROP_NOT_REPLACED   0
#define PROP_REPLACED       1
#define PROP_REPLACED_FUZZY 2


#define MARK_EQUAL    'E'
#define MARK_DELETED  'D'
#define MARK_INSERTED 'I'
#define MARK_MODIFIED 'M'


__declspec(dllexport)
USHORT EQFBPrepareFuzzyProp ( PTBDOCUMENT, PSZ_W, PSZ_W, PSZ_W, SHORT , SHORT);

__declspec(dllexport)
USHORT EQFBPrepareFuzzyPropEx ( PTBDOCUMENT, PSZ_W, PSZ_W, PSZ_W, SHORT ,
                                 SHORT, PUSHORT, ULONG, ULONG);

__declspec(dllexport)
USHORT EQFBPrepareFuzzyPropEx2( PSZ_W, PSZ_W, PSZ_W, SHORT, SHORT, PUSHORT, PVOID,
                                  PVOID, PBYTE, PBYTE, ULONG, ULONG );

__declspec(dllexport)
USHORT EQFBPrepareFuzzyPropEx3( PSZ_W, PSZ_W, PSZ_W, SHORT, SHORT, PUSHORT, PVOID,
                                  PVOID, PBYTE, PBYTE, ULONG, ULONG, PUSHORT );
__declspec(dllexport)
BOOL EQFBFindDiff ( PTBDOCUMENT, PSZ_W, PSZ_W, SHORT , PFUZZYTOK * , PUSHORT, PUSHORT, ULONG);

__declspec(dllexport)
BOOL EQFBFindWord ( PSZ_W ,SHORT, USHORT, PUSHORT, PUSHORT, ULONG, BOOL );

