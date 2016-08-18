/*! \file
	Description: This is the header file for the Initial Translation Memory code

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_FOLDER           // folder list and document list functions
// cv already included in eqfitm.c
//#include <eqf.h>                  // General Translation Manager include file
#include "eqftpi.h"               // internal defines for editor...

#include <math.h>                 // for mathematical functions....
#include <malloc.h>               // for halloc and hfree
#include "core\PluginManager\OtmMemory.h"
#ifndef ERROR_MSG_HANDLED
  #define ERROR_MSG_HANDLED   11111
#endif

/**********************************************************************/
/* revision level of ITM version                                      */
/**********************************************************************/
#define ITM_REVISION   ' '

/**********************************************************************/
/* strings to be translated are in eqfb.mri                           */
/* not to be translated                                               */
/**********************************************************************/
#define ITM_SPACESTRING "                                     "
#define ITM_CLASS     "ITMClass"       // class name
#define ITM_VISCLASS  "ITMVisClass"    // class name
#define ITM_PROCCLASS  "ITMProcClass"   // class name
#define ITM_XPOS      20               // position on the screen
#define ITM_YPOS      50               // position on the screen
#define ITM_YDELTA    30               // difference in line position

#define DELSLASH      '/'              // delimiter for cmdline strings
#define DELSLASH_STR  "/"
#define LISTINDICATOR '@'              // indicator for list of files
#define LISTSTART     '('              // indicator for list start
#define LISTEND       ')'              // indicator for list end
#define TEMPFOLDERSRC  "$$ITMS$$.F00"  // temporary folder src   /* @KIT1008A */
#define TEMPFOLDERTGT  "$$ITMT$$.F00"  // temporary folder tgt   /* @KIT1008A */
#define TEMPFOLDERSRCNEW  "$$S00001.F00"  // temporary folder src   /* @KIT1008A */
#define TEMPFOLDERTGTNEW  "$$T00001.F00"  // temporary folder tgt   /* @KIT1008A */

//#define TEMPFOLDERTGT  "$$ITMT$$.F00"  // temporary folder tgt   /* @KIT1008A */
#define TEMPDIRSAVE    "$$ITMALI"      // path for temporary save of alignment

#define ITM_DOCVIEW   "ITMDocument"
#define ITM_DOCVIEW_W L"ITMDocument"
#define ITM_STATUSBAR "ITMStatusBar"
#define ITM_STATUSBAR_W L"ITMStatusBar"

#define ITM_MEMORYDB  L"<MemoryDb>\r\n" // memory db ...
#define ITM_EMEMORYDB L"</MemoryDb>\r\n"// end memory db
#define ITM_ESEGMENT  L"</Segment>\r\n" // end segment identifier

#define ITM_BUFSIZE    16384           // 16k buffer ...
//#define REQ_STACKSIZE  25000           // stack size
#define REQ_STACKSIZE  40000           // stack size
#define ITMALI_BUFSIZE  32000          // read in buffer for ITMALI

#define   NEXT  1                      // search next alignment
#define   PREVIOUS -1                  // search previous alignment
#define   CURRENT 0                    // search current or next alignment

#define   INITFILL 0
#define   USERFILL 1
#define   USERFREE 2

#define   COL1  1                     // color toggling in VisSetqStatus
#define   COL2  2
#define   COL3  3


/**********************************************************************/
/* status process can be ....                                         */
/**********************************************************************/
#define ITM_STAT_CLEANUP      1
#define ITM_STAT_ANALYSIS     2
#define ITM_STAT_ALIGN        3
#define ITM_STAT_PREPARE      4
#define ITM_STAT_ERROR        5
#define ITM_STAT_VISUAL       6
#define ITM_STAT_ENDVISUAL    7

#define ITM_TIMER_ID   1               // timer ID
#define ITM_TIMER_VAL  250L            // timer value
/**********************************************************************/
/* internal QF tag table - remember: drive identifier will be reset   */
/* to system drive later on...                                        */
/**********************************************************************/
#define ITM_QFTAGTABLE "C:\\EQF\\TABLE\\QFTAGS.IBL"
#define ITM_PROPERTYFILE "ITMProc.PRP"
#define ITMALIPREFIX     "$$EQFITM$$"

#define FORMAT_UNICODE "UNICODE"
#define FORMAT_ASCII   "ASCII"
#define FORMAT_ANSI    "ANSI"

/**********************************************************************/
/* version for ali-file: ITMVERSION2 needed for additional StatusINfo */
/**********************************************************************/
#define ITMVERSION   1                   //version number for alifiles
#define ITMVERSION2  2                   //version  22.09.2000
#define ITMVERSION3  3                   //version 10.06.2000, SGMLFormat

#define REALIGN 1
#define GOON    0
#define STOPNEC 2
/**********************************************************************/
/* type of alignment                                                  */
/**********************************************************************/
#define ONE_ONE  1
#define ONE_TWO  2
#define TWO_ONE  4
#define NUL_ONE  8
#define ONE_NUL 16
#define TWO_TWO 32

//options for MemFuncCreateITM lType
#define NOANA_TYP           0x1
#define NOTM_TYP            0x2
#define PREPARE_TYP         0x4


/**********************************************************************/
/* PROCESSTASK controls loop in windows where main routine is splitted*/
/* up in various different tasks as defined in this typedef           */
/**********************************************************************/
typedef  enum _PROCESSTASK
{
  ITM_PROCESS_TMOPEN,                   // open memory
  ITM_PROCESS_TEMPFOLD,                 // create temp folder
  ITM_PROCESS_PREPFILE,                 // prepare files
  ITM_PROCESS_ANALYSE ,                 // analyse files
  ITM_PROCESS_READSRC,                  // read src file
  ITM_PROCESS_READTGT,                  // read tgt file
  ITM_PROCESS_NOPUSAGE,                 // check NOP usage
  ITM_PROCESS_TOKSRC,                   // find candidates for anchors
  ITM_PROCESS_TOKTGT,                   // find anchor candidates in tgt
  ITM_PROCESS_ALIGNPARA,                // align the paragraphs
  ITM_PROCESS_MEANVAR,                  // calculate mean and variance
  ITM_PROCESS_ALIGNSEGS,                // align segments in a block
  ITM_PROCESS_PARSEALIGN,               // parse alignment table
  ITM_PROCESS_JOINSEG,                  // join segments in 2:1 / 1:2
  ITM_PROCESS_MEMSAVE,                  // save in the memory
  ITM_PROCESS_CLOSE,                    // close itm
  ITM_PROCESS_REMOVEFLD,                // remove temp folder
  ITM_PROCESS_VISUAL,                   // do the visualisation
  ITM_PROCESS_END                       // end of list indicator
} PROCESSTASK;

/**********************************************************************/
/* structure for allowed and recognized command line parameters       */
/**********************************************************************/
typedef  enum _BATCHCMD
{
  BATCH_MEM,                           // add data to translation memory
  BATCH_FILES,                         // files to be aligned
  BATCH_MARKUP,                        // markup table to be used if analysis
  BATCH_SGMLMEM,                       // name of the SGML output file
  BATCH_TGTLNG,                        // target language
  BATCH_SRCLNG,                        // source language
  BATCH_TYPE,                          // options allowed
  BATCH_NOANA,                         // NO analysis
  BATCH_NOTMDB,                        // NO translation memory database
  BATCH_NOCONF,                        // NO confirmation message at end
  BATCH_LEVEL ,                        // level of match
  BATCH_VISUAL,                        // Visual required
  BATCH_PREPARE,                       // prepare alignment and save it
  BATCH_HWND,                          // use another HWND instead of DESKTOP
  BATCH_ICON,                          // batch command to run ITM minimized
  BATCH_QUIET,                         // quiet indication
  BATCH_SRCSTARTPATH,                  // start path if srcfilename with rel.path
  BATCH_TGTSTARTPATH,                  // start path if tgtfilename with rel.path
  BATCH_QUALITYLEVEL,                  // qual.level for m-flag
  BATCH_SGMLFORMAT,                    // SGML memory format ( Unicode/ASCII/ANSI)
  BATCH_END                            // end of list indicator
} BATCHCMD;


typedef struct _CMDLIST
{
  BATCHCMD BatchCmd;                   // identifier
  CHAR    szDesc[ 15 ];                // full name of item
  CHAR    szShortCut[ 10 ];            // shortcut of item
} CMDLIST, *PCMDLIST;

#define B_MEM      1                   // memory cmdline option specified
#define B_FILES    2                   // list of files specified
#define B_MARKUP   4                   // markup table specified
#define B_TGTLNG   8

#define DOUBLE double                  // no define in OS2 Toolkit files
#define RANDOM           0xABCDEF01    // very big prime number ..
#define MAX_SIGNIFICANT  10            // up to this number are tags significant


#define NEW_ALIGN  50                  // number of chunks alloced in new case
#define NEW_HASHBLOCK  5000            // number of chunks for hashblock
#define VARIANCE   6.8                 // default variance to be used
#define LANGVALUE  0.9                 // language difference values
#define BIG_DISTANCE  2500             // distance is almost infinite
#define EPSILON    0.0001              // minimal value
#define MIN_SEG_LEN  2                 // minimum length of aligned segments
#define  ITM_THRESHOLD     700         // maximum threshold
#define  MAX_BLOCKS        150         // maximum number of blocks
#define  MAX_BLOCKS2       75          // maximum number of blocks /2
#define  WEIGHT_NOP        50          // weight of a nop segment
#define  MIN_PARALEN      2000         // minimum paragraph length

#undef MAXINT
#define MAXINT  0xffff                 // maximum integer..
#define MAX_PROPFILES     1000         // max num filepairs in propertyfile
/**********************************************************************/
/*  Property File structure:                                          */
/*   the 2nd srcfile and the 2nd tgtfile are already prepared         */
/*   and the alifile of them is $$itmali.002                          */
/*   if $$itmali.004 does not exist, src/tgtfilename is empty.        */
/**********************************************************************/

#define ITM_VERSION     "ITM2"         // version 2, 13.11.99
#define ITM_VERSION3    "ITM3"         // version 3, 19.03.2008
#define ITM_VERSION4    "ITM4"         // version 4, 2015-02-19, difference to previous versions: different layout of filepair table and file name pool to support more files

// file pair table up to and including ITM version 3
typedef struct _FILEPAIR_VITM3
{
  USHORT  usAliNum;
  USHORT  usSrcOffset;                      // offs of srcname in pool
  USHORT  usTgtOffset;                      // offs.of tgtfile in pool
} FILEPAIR_VITM3,  *PFILEPAIR_VITM3;

// file pair table ITM version 4 and higher
typedef struct _FILEPAIR
{
  USHORT  usAliNum;
  ULONG  ulSrcOffset;                      // offs of srcname in pool
  ULONG  ulTgtOffset;                      // offs.of tgtfile in pool
} FILEPAIR,  *PFILEPAIR;

// file name pool up to and including ITM version 3
typedef struct _FILEPOOL_VITM3                    // pool to store filenames
{
  USHORT  usAlloc;
  USHORT  usUsed;
  UCHAR   uchFirstBufferByte;    // first byte of attached buffer
} FILEPOOL_VITM3,  *PFILEPOOL_VITM3;

// file name pool ITM version 4 and higher
typedef struct _FILEPOOL                    // pool to store filenames
{
  ULONG   ulAlloc;
  ULONG   ulUsed;
  UCHAR   uchFirstBufferByte;    // first byte of attached buffer
} FILEPOOL,  *PFILEPOOL;

typedef struct _PROPITMOLD
{
  PROPHEAD    PropHead;
  CHAR        chTagTable[MAX_FNAME];         // tag table name
  CHAR        chTranslMemory[MAX_FNAME];     // translation memory
  CHAR        szSrcDirectory[MAX_PATH144];   // active directory
  CHAR        szTgtDirectory[MAX_PATH144];   // active directory
  LANGUAGE    szSourceLang;                  // source language
  LANGUAGE    szTargetLang;                  // target language
  PFILEPAIR   pListFiles;                    // ptr to list of filepairs
  USHORT      usNumFiles;                        // number of filepairs stored
  PFILEPOOL   pFileNamePool;                     // ptr to pool for filenames
  CHAR        chVersion[5];                   // version : added 28.9.99
  CHAR        szSrcStartPath[MAX_LONGFILESPEC];  // buffer for srcstart path
  CHAR        szTgtStartPath[MAX_LONGFILESPEC];  // buffer for tgtstart path
  CHAR        chLongTranslMemory[MAX_LONGFILESPEC]; // translation memory (long name)
} PROPITMOLD, *PPROPITMOLD;

typedef struct _PROPITM
{
  PROPHEAD    PropHead;
  CHAR        chTagTable[MAX_FNAME];         // tag table name
  CHAR        chTranslMemory[MAX_FNAME];     // translation memory
  CHAR        szOldSrcDirectory[MAX_PATH144];   // old active directory
  CHAR        szOldTgtDirectory[MAX_PATH144];   // old active directory
  LANGUAGE    szSourceLang;                  // source language
  LANGUAGE    szTargetLang;                  // target language
  PFILEPAIR   pListFiles;                    // ptr to list of filepairs
  USHORT      usNumFiles;                        // number of filepairs stored
  PFILEPOOL   pFileNamePool;                     // ptr to pool for filenames
  CHAR        chVersion[5];                   // version : added 28.9.99
  CHAR        szSrcStartPath[MAX_LONGFILESPEC];  // buffer for srcstart path
  CHAR        szTgtStartPath[MAX_LONGFILESPEC];  // buffer for tgtstart path
  CHAR        chLongTranslMemory[MAX_LONGFILESPEC]; // translation memory (long name)
  CHAR        szSrcDirectory[MAX_LONGFILESPEC];   // active directory
  CHAR        szTgtDirectory[MAX_LONGFILESPEC];   // active directory
  CHAR        chFutureUse[2048];             // room for new fields 
} PROPITM, *PPROPITM;

/**********************************************************************/
/* structure for LCS routine with 'LONG'                              */
/**********************************************************************/
typedef struct _ITMLCSTOKEN
{
  FUZZYTOK *pTokenList;
  FUZZYTOK *pBackList;
  LONG  lStart;
  LONG  lStop;
  LONG  lTotalLen;
} ITMLCSTOKEN, *PITMLSCTOKEN;

/**********************************************************************/
/* defines for the flag fCurDispStatus                                */
/**********************************************************************/
#define DISP_VISUAL    1
#define DISP_ALIGNING  2
#define MAX_STATUSLEN  31             // eq. to MAX_RC_STRING in eqfitms.c
/**********************************************************************/
/* generic alignement structure containing two pairs of length and    */
/* the calculated distance                                            */
/**********************************************************************/
typedef struct _ALIGNEMENT
{
  SHORT  sX1;                          // length of source sentence
  SHORT  sY1;                          // length of target sentence
  SHORT  sX2;                          // length of the next src sentence
  SHORT  sY2;                          // length of next target sentence
  SHORT  sDist;                        // distance

} ALIGNEMENT, *PALIGNEMENT;

/**********************************************************************/
/* alignement 'array' containing pointer to paragraphs, paragraph     */
/* start segment and paragraph end segment                            */
/**********************************************************************/
typedef struct _ITMALIGN
{
  ULONG       ulAlloc;                 // allocated number
  ULONG       ulUsed;                  // number used
  PSHORT      psPara;                  // pointer to paragraph array
  PULONG      pulSegStart;             // pointer to segment numbers
  PULONG      pulSegEnd;               // pointer to end of segm.block
} ITMALIGN, * PITMALIGN ;

typedef struct _ITMNOPSEGS
{
  ULONG       ulAlloc;                 // allocated number
  ULONG       ulUsed;                  // number used
  PULONG      pulSegs;                 // pointer to segment numbers
  FUZZYTOK    *pTokenList;             // pointer to fuzzy tokens
} ITMNOPSEGS, * PITMNOPSEGS;

typedef struct _ITMNOPCOUNT
{
  ULONG        ulAlloc;
  ULONG        ulUsed;                  // number of different nops stored
  PSZ_W        * ppData;                // ptr to data of nop
  PUSHORT      pusOccur;                // how often this nop occurs
//  PULONG       pulHash;
  PSHORT       psTokenID;                // Tokenid of NOP in Tagtable
  PSHORT       psAddInfo;                // AddInfo from tagtable
  PUSHORT      pusLen;                   // Length of nop tag
} ITMNOPCOUNT, *PITMNOPCOUNT;

typedef struct _ALLALIGNED
{
  ULONG       ulAlloc;                 // allocated number
  ULONG       ulUsed;                  // number used
  ULONG       ulFillStart;
  ULONG       ulFillIndex;             // overwrite index due to user action
  ULONG       ulFillEnd;               // max. overwrite index
  PULONG      pulSrc;                  //aligned Src Segnum
  PULONG      pulTgt1;                 // aligned Tgt Segnum
  PSHORT      psDist;                  // distance (Gale&Church) of alignment
  PBYTE       pbType;                  // 1:1, 0:1, 1:0, 2:1, 1:2, 2:2
} ALLALIGNED, *PALLALIGNED;

 typedef struct _FLAGVIS
 {
    unsigned short UserAnchor : 1;     // TRUE if anchor is set by user
    unsigned short CrossedOut : 1;     // TRUE if segment is crossed out
    unsigned short UserJoin   : 1;     // TRUE if segment is joined by user
    unsigned short OverCross  : 1;     // TRUE if user overcrossing anchor
    unsigned short UserSplit  : 1;     // TRUE if segment is split by user
    unsigned short LFChanged  : 1;     // TRUE if LF changed at end of seg
 } FLAGVIS, *PFLAGVIS;



typedef struct _ITMVISDOC
{
  USHORT  usType;                      // type of window
  PVOID   pITMIda;                     // pointer to itm ida ...
  PTBDOCUMENT pDoc;                    // pointer to active document
  PULONG   pulNumAligned;              // index of entry in ALLALIGNED struct
  PULONG   pulAnchor;                  // Segnum of anchor pair
  PBYTE    pbLFNum;                    // number of LF's originally at endofseg
  PFLAGVIS pVisState;                  // state of segment in visualization
  QSTATUS  qVisActState;              // stores old state of VISACT segment
  ULONG    ulVisActSeg;                // active segment number
  ULONG    ulFirstAlign;               // segnum of 1st aligned segment
  USHORT   usAddLFatBegin;             // num lf's added to begin of 1st alignedseg
  BOOL     fChanged;                   // true if user changed alignment
} ITMVISDOC, * PITMVISDOC;

/**********************************************************************/
/* if size of statusinfo is changed, SAVESTRUCT size in Ali file      */
/* changes too, so please change ITMVERSION TOO!                      */
/**********************************************************************/
typedef struct _STATUSINFO
{
  ULONG       ulSegTotal;              // number of segments
  ULONG       ulSegUnAligned;          // number of segments unaligned
  USHORT      usSegCrossOut;           // number of segments crossed out
  CHAR        chType[ 10 ];            // type of document...
  ULONG       ulSegIrregular;          // joined segments ( irregular alis)
  ULONG       ulDummy1;                // room for further additions
  ULONG       ulDummy2;                // dummy
  ULONG       ulDummy3;                // dummy
} STATUSINFO, * PSTATUSINFO;
/**********************************************************************/
/* instance data area containing any 'global' fields used in the ITM  */
/* generation                                                         */
/**********************************************************************/
typedef struct _ITMIDA
{
  HWND   hwnd;                          // window handle
  BOOL   fKill;                         // success indicator
  BOOL   fBusy;                         // busy flag
  CHAR   chTagTableName[MAX_EQF_PATH];  // tag table name
  CHAR   chQFTagTable[MAX_EQF_PATH];    // internal QF tag table name
  CHAR   chTranslMemory[MAX_LONGPATH];  // translation memory
  CHAR   chLongTranslMemory[MAX_LONGPATH];// translation memory (long name)
  CHAR   chSGMLMem[MAX_LONGPATH];       // translation memory in SGML syntax
  CHAR   chSourceFile[MAX_LONGPATH];    // source filename
  CHAR   chShortSrcFName[MAX_FILESPEC]; // short filespecs
  CHAR   chSegSourceFile[MAX_EQF_PATH]; // segmented source filename
  CHAR   chTargetFile[MAX_LONGPATH];    // target filename
  CHAR   chShortTgtFName[MAX_FILESPEC]; // short filespecs
  CHAR   chSegTargetFile[MAX_EQF_PATH]; // segmented target filename
  CHAR_W szSourceSeg[MAX_SEGMENT_SIZE]; // source segment size
  CHAR_W szTargetSeg[MAX_SEGMENT_SIZE]; // target segment size
  CHAR   szBuffer[ MAX_SEGMENT_SIZE*2]; // work buffer
  CHAR_W szBufferW[ MAX_SEGMENT_SIZE*2];// work buffer
  PSZ   pInFile;                       // pointer to input file name
  TBDOCUMENT  TBSourceDoc;             // source document struct
  TBDOCUMENT  TBTargetDoc;             // target document struct
  ITMNOPSEGS  itmSrcNop;               // source nops
  ITMNOPSEGS  itmTgtNop;               // target nops
  ITMALIGN    itmSrcText;              // source text
  ITMALIGN    itmTgtText;              // target text
  PSHORT      psFastPath;              // best alignement
  PALIGNEMENT pAlign;                  // pointer to aligned sentences
  //HTM         htm;                     // transl.memory handle
  OtmMemory    *pMem;
  OtmProposal  *pProposal;
  //PSZ         pTMIn;                   // input space for translation memory
  //PSZ         pTMOut;                  // output space for translation memory
  DOUBLE      dbMean;                  // mean
  DOUBLE      dbVar;                   // variance
  USHORT      usArgc;                  // number of commandline parameters
  PSZ         * ppArgv;                // pointer to file array
  LONG        lTime;                   // store the time
  BOOL        fNoAna;                  // no analysis necessary
  BOOL        fNoTMDB;                 // no translation memory database
  BOOL        fNoConfirm;              // no confirmation message
  BOOL        fSGMLITM;                // transl. memory in external format
  PBUFCB      pBufCB;                  // pointer to control block
  BOOL        fThreadActive;           // thread running...
  USHORT      usStatus;                // current active status
  LANGUAGE    szSourceLang;            // source language
  LANGUAGE    szTargetLang;            // target language
  ULONG       ulSegTotal;              // number of segments
  ULONG       ulSegAligned;            // number of segments aligned
  USHORT      usLevel;                 // level of alignment
  PVOID       pLoadedTable;
  USHORT      usLastSegFilled;         // segnum of last seg filled to memory
  ALLALIGNED  Aligned;                 // stores all alignments
  BOOL        fVisual;                 // true if visualization required
  ITMVISDOC   stVisDocTgt;             // structure containing target vis info
  ITMVISDOC   stVisDocSrc;             // structure containing source vis info
  ITMNOPCOUNT stSrcNopCnt;
  ITMNOPCOUNT stTgtNopCnt;
  PSZ         pExcludeNop;             // NOPdata excluded to be an anchor
  ULONG       ulHashExclNop;           // Hash of excluded NOP
  BOOL        fCurDispStatus;          // either visual or align, nec in WM_CLOSE
  HWND        hVisWnd;                 // visual window
  HWND        hProcWnd;                // process window
  HWND        hStatusBarWnd;           // status bar
  HAB         hab;                     // PM anchor block handle
  BOOL        fHorizontal;             // true if visdoc windows horizontally
  STATUSINFO  stSrcInfo;               // srcdoc info for statusline
  STATUSINFO  stTgtInfo;               // tgtdoc info for statusline
  PTEXTTYPETABLE pColorTable;          // table with color settings
  BOOL        fParallel;               // true if aligned segments parallel
  BOOL        fTimer;                  // true if timer is started
  CHAR        chTransMemFname[MAX_LONGPATH];  // translation memory
  ITMNOPCOUNT stExclNopCnt;            // NOPs to be excluded
  CHAR        szPropName[MAX_EQF_PATH];// buffer for property file name
  PPROPITM    pstPropItm;              // structure with ITM properties
  USHORT      usNumPrepared;           // 0 if not prepared, n if ali.n
  PULONG      pulSrcNumAlign;          // SRCVISDOC: pusNumALigned
  PULONG      pulSrcAnchor;            // SRCVISDOC: pusAnchor
  FLAGVIS     *pSrcVisState;           // SRCVISDOC: pVisState
  PULONG      pulTgtNumAlign;          // TGTVISDOC: pusNumALigned
  PULONG      pulTgtAnchor;            // TGTVISDOC: pusAnchor
  FLAGVIS     *pTgtVisState;           // TGTVISDOC: pVisState
  BOOL           fPrepIsVisual;          // true if prepared ali was visual
  BOOL           fPrepare;               // true if user pressed
  HWND           hwndFocus;                      // focus window
  HWND           hwndParent;             // parent in cmdline/diff. from desktopm
  HWND           hHelp;                  // IPF help hwnd
  PEQF_BOOL      pfKillAnalysis;         // allow for disruption of Analysis
  SHORT          tID;                    // thread id
  CHAR           chStatSegments[MAX_STATUSLEN];  // segments: in statusline
  CHAR           chStatUnaligned[MAX_STATUSLEN]; // in statusline Unaligned:
  CHAR           chStatIgnored[MAX_STATUSLEN];   // in statusline IGnored:
  CHAR           chStatMatch[MAX_STATUSLEN];     // in statusline Match:
  CHAR           chStatIrregular[MAX_STATUSLEN]; // in statusline Irregular
  USHORT         usStartSlider;                  // remember slider position
  USHORT         usAnchorCount;                  // counts how many anchors found
  USHORT         usOldPerc;                      // old slider percentage
  LANGUAGE       szTargetInputLang;              // tgt lng originally specified
  BOOL           fAutoWrap;                      // true if wrapping allowed
  BOOL           fMinimized;                     // run ITM as an icon
  BOOL           fQuiet;                         // run in quiet mode
  USHORT         usRC;                           // return code
  CHAR           szSrcStartPath[MAX_LONGFILESPEC];  // buffer for start path
  CHAR           szTgtStartPath[MAX_LONGFILESPEC];         // buffer for start path
#ifdef RAS400_ITM
  SHORT          sMPerCent;                      // percent to add m-flag
#endif
  USHORT         usSGMLFormat;                    // Format of SGMLmem(ASCII/UNICODE/ANSI)
  ULONG          ulSGMLFormatCP;                  // CP of doc according to format
  ULONG          ulAnsiCP;                        // Ansi CP of selected language
  CHAR           chITMSFolder[MAX_FILESPEC];	  // new variable foldername, old $$ITMS$$.F00
  CHAR           chITMTFolder[MAX_FILESPEC];      // new variable foldername, old $$ITMT$$.F00
} ITMIDA, * PITMIDA;

typedef struct _SPECFILE
{
   CHAR     szFileName[MAX_LONGPATH];    // buffer for path names
   CHAR     szPath[MAX_LONGPATH];        // buffer for path names
   CHAR     chDrive;                     // active drive
   CHAR     szDirectory[MAX_LONGPATH];   // active directory
   CHAR     szFilePattern[MAX_LONGFILESPEC]; // active pattern (file mode)
   SHORT    sIdDirLB;                    // iD of Directory listbox
   SHORT    sIdFileLB;                   // ID of File listbox
   SHORT    sIdFileEF;                   // ID of FIle entryfield
   SHORT    sIdCurDir;                   // ID of CurDir static field
} SPECFILE, *PSPECFILE;

// ITM Dialog IDA
typedef struct _ITMDLGIDA
{
  SPECFILE    SrcFile;                     // Src file settings
  SPECFILE    TgtFile;                     // Tgt file settings
  CHAR        szDrives[MAX_DRIVELIST];     // list of available drives
  CHAR        szFilePattern[MAX_LONGPATH]; // active pattern (file mode)
  PSZ        *ppListIndex;                 // pointer to filelistindex array
  PITMIDA     pITMIda;                     // ptr to itm ida
  CHAR        szFile1[ MAX_LONGPATH * 2];  // working buffer for 2 file specs.
  CHAR        szFile2[ MAX_LONGPATH * 2];  // working buffer for 2 file specs.
  CHAR        szSrcStartPath[MAX_LONGFILESPEC];  // buffer for start path
  CHAR        szTgtStartPath[MAX_LONGFILESPEC];  // buffer for start path
  // fields for window ressizing
  int         iMinHeight;                  // minimum widow height
  int         iMinWidth;                   // minimum widow width
  int         iMarkupCBXOffset;            // offset of markup comboox
  int         iSrcStartpathXOffset;        // offset of source start path field
  int         iTgtStartpathXOffset;        // offset of target start path field
  int         iFixedAreaHeight;            // total height of fixed size elements
  int         iButtonArea1Space;           // height of button area 1
  int         iButtonArea1Offs;            // vertical offset of buttons in button area 1
  int         iButtonArea2Space;           // height of button area 2
  int         iButtonArea2Offs;            // vertical offset of buttons in button area 2
  int         iFileListOffs;               // vertical offset between filelist static and filelist listbox
  int         iBrowsePBWidth;              // width of browse pushbutton
  CHAR        szCurDir[MAX_LONGFILESPEC];  // buffer for current directory
} ITMDLGIDA, *PITMDLGIDA;



/**********************************************************************/
/* prototypes                                                         */
/**********************************************************************/
INT_PTR CALLBACK EQFBITMDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
static SHORT ITMDistMeasure ( SHORT, SHORT, SHORT, SHORT, DOUBLE, DOUBLE );
static SHORT ITMMatch ( SHORT, SHORT, DOUBLE, DOUBLE );
BOOL  ITMSeqAlign( PSHORT,PSHORT,ULONG,ULONG,PALIGNEMENT * ,
                          PSHORT,DOUBLE,DOUBLE );
static BOOL GetDocStatus ( PITMIDA, PTBDOCUMENT, PITMALIGN );
static BOOL AddToBlock ( PITMALIGN, ULONG, ULONG, ULONG );
VOID EQFITMProcess ( HWND, PITMIDA, PUSHORT, PULONG );

       BOOL GetNextFilePair (HWND, PITMIDA);
       VOID ITMComplete ( HWND, PITMIDA );
       BOOL ITMHandlers ( HAB );
static BOOL PrepareNextBlock ( PITMIDA, ULONG );
static BOOL FillTM ( PITMIDA, PALIGNEMENT, SHORT );
static BOOL PrepareParagraph ( PITMIDA );
static BOOL FillFile ( PITMIDA, PALIGNEMENT, PSHORT, PSHORT );
static VOID FreeNOPDoc ( PTBDOCUMENT, PITMNOPSEGS );
BOOL EQFTextSegm ( PITMIDA, HWND, PSZ, PSZ );
static BOOL PrepareSeg ( PITMIDA, PUSHORT, PUSHORT, USHORT );
static BOOL ITMTmReplace ( PITMIDA, PSZ, ULONG, PSZ_W, PSZ_W, BOOL);
static USHORT  ITMClose ( PITMIDA );
static USHORT ITMTmOpen ( PITMIDA );
//VOID PASCAL FAR ITMCleanUp( USHORT );

BOOL GetSegmentBlock( PITMIDA,PTBDOCUMENT, PITMALIGN, ULONG, ULONG );
static BOOL CreateFolderStruct ( PITMIDA, PSZ );
static VOID RemoveFolderStruct ( PSZ );
static BOOL PrepFiles ( PITMIDA );
static BOOL ITMAnalyseFiles ( PITMIDA );
static VOID RemoveFiles ( PITMIDA );
static BOOL WriteSegm ( PITMIDA, ULONG );
BOOL FillAlignStruct(PITMIDA, PALIGNEMENT, SHORT, SHORT);

VOID ITMLCS( ITMLCSTOKEN, ITMLCSTOKEN, PITMIDA );

__declspec(dllexport)
MRESULT APIENTRY  ITMWNDPROC ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );
__declspec(dllexport)
MRESULT APIENTRY  ITMVISWNDPROC ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );
__declspec(dllexport)
MRESULT APIENTRY  ITMPROCWNDPROC( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );

MRESULT APIENTRY ITMDOCVIEWWNDPROC ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );
MRESULT APIENTRY ITMSTATUSBARWNDPROC ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );
VOID VisualStart ( HAB hab, HWND hwnd, PITMIDA pITMIda );

VOID  ITMFuncSetAnchor(PITMIDA);
VOID  ITMFuncDelAnchor(PITMIDA);
VOID  ITMFuncDelAllUser(PITMIDA);
VOID  ITMFuncCrossOut(PITMIDA);
VOID  ITMFuncUndoCrossOut(PITMIDA);
VOID  ITMFuncNextAnchor(PITMIDA, SHORT);
VOID  ITMFuncNextLone(PITMIDA, SHORT);
VOID  ITMFuncSynch(PITMIDA, PITMVISDOC);
VOID  ITMCrossOutBlock(PITMVISDOC);
VOID  ITMUndoCrossOutBlock(PITMVISDOC);
VOID  VisAlignBlock (PITMIDA, ULONG, ULONG, ULONG, ULONG, ULONG, ULONG);
VOID  VisInvalRegion(PITMIDA, PITMVISDOC, ULONG);
ULONG FindCurAlignIndex(PITMVISDOC, ULONG, SHORT);
VOID   VisActReset(PITMVISDOC, PITMVISDOC);
ULONG  FindNextAnchor(PITMVISDOC, ULONG, SHORT);
VOID   VisDocFillIndex (PITMIDA, ULONG, ULONG, ULONG );
BOOL   VisSplitSeg ( PITMIDA, PITMVISDOC, PTBDOCUMENT, ULONG, BOOL );
BOOL   VisJoinSeg ( PITMIDA, PITMVISDOC, PTBDOCUMENT , ULONG, BOOL);
VOID   VisSetQStatus( PITMVISDOC, ULONG, ULONG );
VOID   VisActivateSeg(PITMIDA, PITMVISDOC, SHORT, BOOL, ULONG);

VOID   AlignSplit ( PITMIDA, PALLALIGNED,PTBDOCUMENT,PTBDOCUMENT,ULONG,USHORT,BOOL);
VOID   BuildJoinSeg ( PITMIDA );
VOID   ITMFuncSplitSeg(PITMIDA, PITMVISDOC);
VOID   ITMFuncJoinSeg(PITMIDA, PITMVISDOC);
BOOL   ParseAlignStruct( PITMIDA, PALLALIGNED);

VOID   BuildSplitSeg ( PITMIDA, PULONG, PULONG );

VOID   AlignInsert ( PALLALIGNED, ULONG );
BOOL   ITMFuncMemSave(PITMIDA, BOOL );

VOID   VisDocFree ( PITMVISDOC );
VOID   FreeAll ( PITMIDA );

VOID   ITMFuncStyle(PITMIDA, USHORT);

BOOL   AddToAlignStruct(PALLALIGNED, ULONG, ULONG, USHORT, SHORT, SHORT);

VOID  ITMVertWindowPos(PTBDOCUMENT, HWND, SWP);
VOID  ITMHorizWindowPos(PTBDOCUMENT, HWND, SWP);
VOID  ITMTile(PITMIDA, BOOL);

SHORT ITMFuncContinue(PITMIDA);
VOID  ITMFuncQuit(PITMIDA, HWND, HAB);
VOID  ITMFuncSave(PITMIDA, HWND, HAB);

VOID  ITMCountStatBar(PITMIDA);
VOID  CountDoc ( PITMVISDOC, PSTATUSINFO );

VOID  ITMDocCountLF ( PTBDOCUMENT, PITMVISDOC, ULONG, ULONG );
VOID  VisDocFillLF ( PITMIDA, ULONG, ULONG, ULONG, ULONG );

VOID  ITMDocDelLF ( PTBDOCUMENT, PITMVISDOC , ULONG , ULONG );
VOID  ITMDelLF ( PITMIDA, ULONG, ULONG, ULONG, ULONG );
VOID  ITMAdjustLF ( PITMIDA , ULONG, ULONG, ULONG, ULONG );
VOID  ITMFuncParallel(PITMIDA, PITMVISDOC);

SHORT EQFITMWriteAli ( PITMIDA );
SHORT EQFITMReadAli ( PITMIDA , BOOL);
SHORT EQFITMPropRead ( PITMIDA );
SHORT EQFITMPropWrite ( PITMIDA );
BOOL  EQFITMPropFill  ( PITMIDA );
BOOL  FillPropFilePairs ( PITMIDA );
BOOL  IsPrepared ( PITMIDA, PUSHORT  );
VOID  EQFITMDelAli ( PITMIDA );
VOID  ItmSetAAB ( HMENU , BOOL  );
VOID  EQFITMDelFilePairList ( PITMIDA, USHORT );
BOOL  CheckDoubleFilePairs ( PSZ **);
SHORT VisDocSave( PTBDOCUMENT, PSZ , PITMVISDOC);
void TimerStartVisual ( HWND, PITMIDA );
void ITMCloseProcessing( PITMIDA );
VOID ITMSetWindowPos ( PITMIDA , SHORT, SHORT );

/**********************************************************************/
/* macro to update statusbar                                          */
/**********************************************************************/
#define UPDSTATUSBAR( p ) INVALIDATERECT( (p)->hStatusBarWnd, NULL, FALSE )

BOOL ItmCompChars ( PSZ_W, PSZ_W, LONG );   //comp strings w/o regards of blanks
BOOL ItmLenComp ( PSZ_W, PSZ_W, USHORT, USHORT );
BOOL ITMFileExist( PSZ pszFile );
VOID  ITMFuncNextIrregular(PITMIDA, SHORT);

VOID  ITMAutoWrap ( PITMIDA  pITMIda, BOOL fAutoWrap );
VOID VisDocAddSoftLF ( PTBDOCUMENT pDoc, PITMIDA pITMIda );
__declspec(dllexport)
USHORT ITMUtlError ( PITMIDA, SHORT, USHORT, USHORT, PSZ *, ERRTYPE );
USHORT ITMUtlErrorW( PITMIDA, SHORT, USHORT, USHORT, PSZ_W *, ERRTYPE );


BOOL EQFBITMAddStartToRelPath(PSZ, PSZ, PSZ, LONG);

BOOL   ITMCheckQuality( PITMIDA, PALLALIGNED, ULONG );
BOOL   NopCntAlloc ( PITMNOPCOUNT );
LONG   ITMGetRelLength ( PVOID, USHORT, SHORT, SHORT, PSZ_W );
BOOL   GetSpecialTok(PTOKENENTRY, PFLAGOFFSLIST);

// huge pointers are not required in 32bit environment anymore
#define HUGE

// macro for allocation of huge memory blocks
#define ALLOCHUGE( p, t, l, n ) \
    { void *pvoid_local = calloc((size_t) l, (size_t) n);   \
      p = (t) pvoid_local; }
// macro for free of huge memory blocks
#define FREEHUGE( p ) free( p )

/**********************************************************************/
/* for test purposes                                                  */
/**********************************************************************/
//ULONG ulDisps;

MRESULT HandleITMWMCharEx( HWND, WPARAM, LPARAM, USHORT );

VOID ITMFuncBackspace( PITMVISDOC );
VOID ITMFuncBacktab( PITMVISDOC );
VOID ITMFuncBottomDoc( PITMVISDOC );
VOID ITMFuncDeleteChar( PITMVISDOC );
VOID ITMFuncDown( PITMVISDOC );
VOID ITMFuncEndLine( PITMVISDOC );
VOID ITMFuncEndSeg( PITMVISDOC );
VOID ITMFuncDoFile( PITMVISDOC );
VOID ITMFuncInsToggle( PITMVISDOC );
VOID ITMFuncJoinLine( PITMVISDOC );
VOID ITMFuncLeft( PITMVISDOC );
VOID ITMFuncNextWord( PITMVISDOC );
VOID ITMFuncNothing( PITMVISDOC );
VOID ITMFuncPageDown( PITMVISDOC );
VOID ITMFuncPageUp( PITMVISDOC );
VOID ITMFuncPrevWord( PITMVISDOC );
VOID ITMFuncDoQuit( PITMVISDOC );
VOID ITMFuncRight( PITMVISDOC );
VOID ITMFuncSave1( PITMVISDOC );
VOID ITMCursorScrollDown( PITMVISDOC );
VOID ITMFuncScrollLeft( PITMVISDOC );
VOID ITMFuncScrollRight( PITMVISDOC );
VOID ITMCursorScrollUp( PITMVISDOC );
VOID ITMFuncSplitLine( PITMVISDOC );
VOID ITMFuncStartLine( PITMVISDOC );
VOID ITMFuncStartSeg( PITMVISDOC );
VOID ITMFuncTab( PITMVISDOC );
VOID ITMFuncTopDoc( PITMVISDOC );
VOID ITMFuncTruncSeg( PITMVISDOC );
VOID ITMFuncTruncate( PITMVISDOC );
VOID ITMFuncUndo( PITMVISDOC );
VOID ITMFuncUp( PITMVISDOC );
VOID ITMJoinSeg( PITMVISDOC );
VOID ITMSplitSeg( PITMVISDOC );
VOID ITMMark( PITMVISDOC );
VOID ITMFindMark( PITMVISDOC );
VOID ITMClearMark( PITMVISDOC );
VOID ITMFuncFind( PITMVISDOC );
VOID ITMFuncCutToClip( PITMVISDOC );
VOID ITMFuncCopyToClip( PITMVISDOC );
VOID ITMFuncPasteFromClip( PITMVISDOC );
VOID ITMFuncMarkNextWord( PITMVISDOC );
VOID ITMFuncMarkPrevWord( PITMVISDOC );
VOID ITMFuncFonts( PITMVISDOC );
VOID ITMFuncChangeToHide( PITMVISDOC );
VOID ITMFuncChangeToShort( PITMVISDOC );
VOID ITMFuncChangeToProt( PITMVISDOC );
VOID ITMFuncChangeToUnpro( PITMVISDOC );
VOID ITMFuncNextLine( PITMVISDOC );
VOID ITMFuncCharacter( PITMVISDOC );
VOID ITMFuncMarkBlock( PITMVISDOC );
VOID ITMFuncMarkClear( PITMVISDOC );
VOID ITMFuncMarkDelete( PITMVISDOC );
VOID ITMFuncMarkCopy( PITMVISDOC );
VOID ITMFuncMarkMove( PITMVISDOC );
VOID ITMFuncMarkFind( PITMVISDOC );
VOID ITMFuncMarkSegment( PITMVISDOC );
VOID ITMFuncMarkLeftCUA( PITMVISDOC );
VOID ITMFuncMarkRightCUA( PITMVISDOC );
VOID ITMFuncMarkUpCUA( PITMVISDOC );
VOID ITMFuncMarkDownCUA( PITMVISDOC );
VOID ITMFuncNothing( PITMVISDOC );
VOID ITMAnchorNextFunc( PITMVISDOC );
VOID ITMAnchorPrevFunc( PITMVISDOC );
VOID ITMIrregularNextFunc( PITMVISDOC );
VOID ITMIrregularPrevFunc( PITMVISDOC );
VOID ITMFuncLineWrap( PITMVISDOC );
VOID ITMFuncFontSize( PITMVISDOC );
VOID ITMFuncMarginAct( PITMVISDOC );
VOID ITMFuncSpellSeg( PITMVISDOC );
VOID ITMFuncSpellFile( PITMVISDOC );
VOID ITMExpandSeg( PITMVISDOC );
VOID ITMCompressSeg( PITMVISDOC );
VOID ITMSettings( PITMVISDOC );
VOID ITMGotoLine( PITMVISDOC );
VOID ITMQueryLine( PITMVISDOC );
VOID ITMFuncChangeToShrink( PITMVISDOC );
VOID ITMFuncChangeToCompact( PITMVISDOC );
VOID ITMFuncCaps( PITMVISDOC );
VOID ITMFuncVisibleSpace( PITMVISDOC );
VOID ITMSynchFunc( PITMVISDOC );
VOID ITMSynchNextFunc( PITMVISDOC );
VOID ITMSynchPrevFunc( PITMVISDOC );
VOID ITMToggleFunc( PITMVISDOC );
VOID ITMSetAnchorFunc( PITMVISDOC );
VOID ITMDelAnchorFunc( PITMVISDOC );
VOID ITMCrossOutFunc( PITMVISDOC );
VOID ITMUndoCrossOutFunc( PITMVISDOC );
VOID ITMAddAbbrevFunc( PITMVISDOC );
VOID ITMNextUnalignedFunc( PITMVISDOC );
VOID ITMPrevUnalignedFunc( PITMVISDOC );
VOID ITMFuncHotPopUp( PITMVISDOC );

typedef struct _ITMFUNCTIONTABLE
{
   USHORT usFuncID;                        // symbolic function identifier
   USHORT usAction;
   VOID (*function)( PITMVISDOC );         // function entry point
   CHAR    szDescription[MAX_FUNCDESCRIPTION]; // description of function
} ITMFUNCTIONTABLE, *PITMFUNCTIONTABLE;


#if defined(ITMINIT_TABLES)
ITMFUNCTIONTABLE ITMFuncTable[] = {
// Symbolic             Action      Entry point          Description
//========================================================================
{ BACKSPACE_ITMFUNC,    OVERALL,    ITMFuncBackspace,    "" },
{ BACKTAB_ITMFUNC,      OVERALL,    ITMFuncBacktab,      "" },
{ BOTTOMDOC_ITMFUNC,    OVERALL,    ITMFuncBottomDoc,   "" },
{ DELETECHAR_ITMFUNC,   ACTSEG,     ITMFuncDeleteChar,  "" },
{ DOWN_ITMFUNC,         OVERALL,    ITMFuncDown,        "" },
{ ENDLINE_ITMFUNC,      OVERALL,    ITMFuncEndLine,     "" },
{ ENDSEG_ITMFUNC,       OVERALL,    ITMFuncEndSeg,      "" },
{ FILE_ITMFUNC,         OVERALL,    ITMFuncDoFile,      "" },
{ INSTOGGLE_ITMFUNC,    OVERALL,    ITMFuncInsToggle,   "" },
{ JOINLINE_ITMFUNC,     ACTSEG,     ITMFuncJoinLine,    "" },
{ LEFT_ITMFUNC,         OVERALL,    ITMFuncLeft,        "" },
{ NEXTWORD_ITMFUNC,     OVERALL,    ITMFuncNextWord,    "" },
{ NOTHING_ITMFUNC,      OVERALL,    ITMFuncNothing,     "" },
{ PAGEDOWN_ITMFUNC,     OVERALL,    ITMFuncPageDown,    "" },
{ PAGEUP_ITMFUNC,       OVERALL,    ITMFuncPageUp,      "" },
{ PREVWORD_ITMFUNC,     OVERALL,    ITMFuncPrevWord,    "" },
{ QUIT_ITMFUNC,         OVERALL,    ITMFuncDoQuit,      "" },
{ RIGHT_ITMFUNC,        OVERALL,    ITMFuncRight,       "" },
{ SAVE_ITMFUNC,         OVERALL,    ITMFuncSave1,        "" },
{ SCROLLDOWN_ITMFUNC,   OVERALL,    ITMCursorScrollDown, "" },
{ SCROLLLEFT_ITMFUNC,   OVERALL,    ITMFuncScrollLeft,   "" },
{ SCROLLRIGHT_ITMFUNC,  OVERALL,    ITMFuncScrollRight,  "" },
{ SCROLLUP_ITMFUNC,     OVERALL,    ITMCursorScrollUp,   "" },
{ SPLITLINE_ITMFUNC,    ACTSEG,     ITMFuncSplitLine,    "" },
{ STARTLINE_ITMFUNC,    OVERALL,    ITMFuncStartLine,    "" },
{ STARTSEG_ITMFUNC,     OVERALL,    ITMFuncStartSeg,     "" },
{ TAB_ITMFUNC,          OVERALL,    ITMFuncTab,          "" },
{ TOPDOC_ITMFUNC,       OVERALL,    ITMFuncTopDoc,       "" },
{ UNDO_ITMFUNC,         ACTSEG,     ITMFuncUndo,         "" },
{ UP_ITMFUNC,           OVERALL,    ITMFuncUp,           "" },
{ JOINSEG_ITMFUNC,      OVERALL,    ITMJoinSeg,          "" },
{ SPLITSEG_ITMFUNC,     OVERALL,    ITMSplitSeg,         "" },
{ MARKSEG_ITMFUNC,      OVERALL,    ITMMark,             "" },
{ GOTOMARK_ITMFUNC,     OVERALL,    ITMFindMark,         "" },
{ CLEARSEGMARK_ITMFUNC, OVERALL,    ITMClearMark,        "" },
{ FIND_ITMFUNC,         OVERALL,    ITMFuncFind,         "" },
{ CUT_ITMFUNC,          ACTSEG,     ITMFuncCutToClip,    "" },
{ COPY_ITMFUNC,         OVERALL,    ITMFuncCopyToClip,   "" },
{ PASTE_ITMFUNC,        ACTSEG,     ITMFuncPasteFromClip,"" },
{ MARKNEXT_ITMFUNC,     OVERALL,    ITMFuncMarkNextWord, "" },
{ MARKPREV_ITMFUNC,     OVERALL,    ITMFuncMarkPrevWord, "" },
{ FONTS_ITMFUNC,        OVERALL,    ITMFuncFonts,        "" },
{ HIDE_ITMFUNC,         OVERALL,    ITMFuncChangeToHide, "" },
{ PROTECT_ITMFUNC,      OVERALL,    ITMFuncChangeToProt, "" },
{ UNPROTECT_ITMFUNC,    OVERALL,    ITMFuncChangeToUnpro,"" },
{ SHORTEN_ITMFUNC,      OVERALL,    ITMFuncChangeToShort,"" },
{ NEXTLINE_ITMFUNC,     OVERALL,    ITMFuncNextLine,     "" },
{ CHARACTER_ITMFUNC,    ACTSEG,     ITMFuncCharacter,    "" },
{ MARKCLEAR_ITMFUNC,    OVERALL,    ITMFuncMarkClear,    "" },
{ MARKBLOCKSEG_ITMFUNC, OVERALL,    ITMFuncMarkSegment,  "" },
{ MARKLEFT_ITMFUNC,     OVERALL,    ITMFuncMarkLeftCUA,  "" },
{ MARKRIGHT_ITMFUNC,    OVERALL,    ITMFuncMarkRightCUA, "" },
{ MARKUP_ITMFUNC,       OVERALL,    ITMFuncMarkUpCUA,    "" },
{ MARKDOWN_ITMFUNC,     OVERALL,    ITMFuncMarkDownCUA,  "" },
{ LINEWRAP_ITMFUNC,     OVERALL,    ITMFuncLineWrap,     "" },
{ FONTSIZE_ITMFUNC,     OVERALL,    ITMFuncFontSize,     "" },
{ MARGINACT_ITMFUNC,    OVERALL,    ITMFuncMarginAct,    "" },
{ SPELLSEG_ITMFUNC,     ACTSEG,     ITMFuncSpellSeg,     "" },
{ SPELLFILE_ITMFUNC,    STARGET,    ITMFuncSpellFile,    "" },
{ GOTO_ITMFUNC,         OVERALL,    ITMGotoLine,         "" },
{ QUERYLINE_ITMFUNC,    OVERALL,    ITMQueryLine,        "" },
{ SHRINK_ITMFUNC,       OVERALL,    ITMFuncChangeToShrink, "" },
{ COMPACT_ITMFUNC,      OVERALL,    ITMFuncChangeToCompact,"" },
{ INITCAPS_ITMFUNC,     ACTSEG,     ITMFuncCaps,         "" },
{ VISIBLESPACE_ITMFUNC, OVERALL,    ITMFuncVisibleSpace, "" },
{ NEXTANCHOR_ITMFUNC,   OVERALL,    ITMAnchorNextFunc,   "" },
{ PREVANCHOR_ITMFUNC,   OVERALL,    ITMAnchorPrevFunc,   "" },
{ NEXTIRREGULAR_ITMFUNC,OVERALL,    ITMIrregularNextFunc,"" },
{ PREVIRREGULAR_ITMFUNC,OVERALL,    ITMIrregularPrevFunc,"" },
{ SYNCH_ITMFUNC,        OVERALL,    ITMSynchFunc,        "" },
{ NEXTSYNCH_ITMFUNC,    OVERALL,    ITMSynchNextFunc,    "" },
{ PREVSYNCH_ITMFUNC,    OVERALL,    ITMSynchPrevFunc,    "" },
{ TOGGLE_ITMFUNC,       OVERALL,    ITMToggleFunc,       "" },
{ SETANCHOR_ITMFUNC,    OVERALL,    ITMSetAnchorFunc,    "" },
{ DELANCHOR_ITMFUNC,    OVERALL,    ITMDelAnchorFunc,    "" },
{ CROSSOUT_ITMFUNC,     OVERALL,    ITMCrossOutFunc,     "" },
{ UNDOCROSSOUT_ITMFUNC, OVERALL,    ITMUndoCrossOutFunc, "" },
{ ADDABBREV_ITMFUNC,    OVERALL,    ITMAddAbbrevFunc,    "" },
{ NEXTUNALIGNED_ITMFUNC,OVERALL,    ITMNextUnalignedFunc,"" },
{ PREVUNALIGNED_ITMFUNC,OVERALL,    ITMPrevUnalignedFunc,"" },
{ HOTPOPUP_ITMFUNC,     OVERALL,    ITMFuncHotPopUp,    "" },
{ ITMDUMMY_003,         0,          ITMFuncNothing,      "" },
{ ITMDUMMY_004,         0,          ITMFuncNothing,      "" },
{ ITMDUMMY_005,         0,          ITMFuncNothing,      "" },
{ ITMDUMMY_006,         0,          ITMFuncNothing,      "" },
{ ITMDUMMY_007,         0,          ITMFuncNothing,      "" },
};
#else
   extern ITMFUNCTIONTABLE ITMFuncTable[];
#endif

//------------------------------------------------------------------------------
//   Table of default key to function assignments                               
//+----------------------------------------------------------------------------+
#if defined(ITMINIT_TABLES)
KEYPROFTABLE DefITMKeyTable[LAST_ITMFUNC+1]=
{
//   Code           State              fChange  Def   Function
//---------------------------------------------------------------------------------
   { VK_BACKSPACE,  ST_VK,             FALSE,   0, 0, BACKSPACE_ITMFUNC },
   { VK_TAB,        ST_VK,             FALSE,   0, 0, TAB_ITMFUNC },
   { VK_BACKTAB,    ST_VK,             FALSE,   0, 0, BACKTAB_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, SPLITLINE_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, JOINLINE_ITMFUNC },
   { VK_F2,         ST_VK,             FALSE,   0, 0, SAVE_ITMFUNC },
   { VK_F3,         ST_VK,             FALSE,   0, 0, QUIT_ITMFUNC },
   { VK_F4,         ST_VK,             FALSE,   0, 0, FILE_ITMFUNC },
   { VK_BACKSPACE,  ST_VK | ST_ALT,    FALSE,   0, 0, UNDO_ITMFUNC },
   { VK_HOME,       ST_VK,             FALSE,   0, 0, STARTLINE_ITMFUNC },
   { VK_HOME,       ST_VK | ST_ALT,    FALSE,   0, 0, STARTSEG_ITMFUNC },
   { VK_UP,         ST_VK,             FALSE,   0, 0, UP_ITMFUNC },
   { VK_PAGEUP,     ST_VK,             FALSE,   0, 0, PAGEUP_ITMFUNC },
   { VK_LEFT,       ST_VK,             FALSE,   0, 0, LEFT_ITMFUNC },
   { VK_RIGHT,      ST_VK,             FALSE,   0, 0, RIGHT_ITMFUNC },
   { VK_END,        ST_VK,             FALSE,   0, 0, ENDLINE_ITMFUNC },
   { VK_END,        ST_VK | ST_ALT,    FALSE,   0, 0, ENDSEG_ITMFUNC },
   { VK_DOWN,       ST_VK,             FALSE,   0, 0, DOWN_ITMFUNC },
   { VK_PAGEDOWN,   ST_VK,             FALSE,   0, 0, PAGEDOWN_ITMFUNC },
   { VK_INSERT,     ST_VK,             FALSE,   0, 0, INSTOGGLE_ITMFUNC },
   { VK_DELETE,     ST_VK,             FALSE,   0, 0, DELETECHAR_ITMFUNC },
   { VK_F5,         ST_VK | ST_SHIFT,  FALSE,   0, 0, SCROLLLEFT_ITMFUNC },
   { VK_F2,         ST_VK | ST_SHIFT,  FALSE,   0, 0, SCROLLRIGHT_ITMFUNC },
   { VK_F3,         ST_VK | ST_SHIFT,  FALSE,   0, 0, SCROLLUP_ITMFUNC },
   { VK_F4,         ST_VK | ST_SHIFT,  FALSE,   0, 0, SCROLLDOWN_ITMFUNC },
   { VK_LEFT,       ST_VK | ST_CTRL,   FALSE,   0, 0, PREVWORD_ITMFUNC },
   { VK_RIGHT,      ST_VK | ST_CTRL,   FALSE,   0, 0, NEXTWORD_ITMFUNC },
   { VK_END,        ST_VK | ST_CTRL,   FALSE,   0, 0, BOTTOMDOC_ITMFUNC },
   { VK_HOME,       ST_VK | ST_CTRL,   FALSE,   0, 0, TOPDOC_ITMFUNC },
   { 'j',           ST_CTRL,           FALSE,   0, 0, JOINSEG_ITMFUNC },
   { 's',           ST_CTRL,           FALSE,   0, 0, SPLITSEG_ITMFUNC },
   { 'b',           ST_CTRL,           FALSE,   0, 0, MARKSEG_ITMFUNC },
   { 'g',           ST_CTRL,           FALSE,   0, 0, GOTOMARK_ITMFUNC },
   { 'e',           ST_CTRL,           FALSE,   0, 0, CLEARSEGMARK_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, FIND_ITMFUNC },
   { 'x',           ST_CTRL,           FALSE,   0, 0, CUT_ITMFUNC },
   { 'c',           ST_CTRL,           FALSE,   0, 0, COPY_ITMFUNC },
   { 'v',           ST_CTRL,           FALSE,   0, 0, PASTE_ITMFUNC },
   { VK_RIGHT,      ST_VK | ST_CTRL | ST_SHIFT,  FALSE,   0, 0, MARKNEXT_ITMFUNC },
   { VK_LEFT,       ST_VK | ST_CTRL | ST_SHIFT,  FALSE,   0, 0, MARKPREV_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, FONTS_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, HIDE_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, PROTECT_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, UNPROTECT_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, SHORTEN_ITMFUNC },
   { VK_ENTER,      ST_VK,             FALSE,   0, 0, NEXTLINE_ITMFUNC },
   { 'n',           ST_CTRL,           FALSE,   0, 0, MARKCLEAR_ITMFUNC },
   { 'm',           ST_CTRL,           FALSE,   0, 0, MARKBLOCKSEG_ITMFUNC },
   { VK_LEFT,       ST_VK | ST_SHIFT,  FALSE,   0, 0, MARKLEFT_ITMFUNC },
   { VK_RIGHT,      ST_VK | ST_SHIFT,  FALSE,   0, 0, MARKRIGHT_ITMFUNC},
   { VK_UP,         ST_VK | ST_SHIFT,  FALSE,   0, 0, MARKUP_ITMFUNC  },
   { VK_DOWN,       ST_VK | ST_SHIFT,  FALSE,   0, 0, MARKDOWN_ITMFUNC},
   { '\0',          0,                 FALSE,   0, 0, LINEWRAP_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, FONTSIZE_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, MARGINACT_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, SPELLSEG_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, SPELLFILE_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, GOTO_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, QUERYLINE_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, SHRINK_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, COMPACT_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, INITCAPS_ITMFUNC },
   { '\0',          0,                 FALSE,   0, 0, VISIBLESPACE_ITMFUNC  },
   { VK_DOWN,       ST_VK|ST_CTRL|ST_SHIFT,  FALSE, 0, 0, NEXTANCHOR_ITMFUNC},
   { VK_UP,         ST_VK|ST_CTRL|ST_SHIFT,  FALSE, 0, 0, PREVANCHOR_ITMFUNC},
   { VK_DOWN,       ST_VK|ST_SHIFT|ST_ALT,FALSE,0, 0, NEXTIRREGULAR_ITMFUNC },
   { VK_UP,         ST_VK|ST_SHIFT|ST_ALT,FALSE,0, 0, PREVIRREGULAR_ITMFUNC },
   { VK_ENTER,      ST_VK|ST_CTRL,     FALSE,   0, 0, SYNCH_ITMFUNC         },
   { VK_DOWN,       ST_VK|ST_CTRL,     FALSE,   0, 0, NEXTSYNCH_ITMFUNC     },
   { VK_UP,         ST_VK|ST_CTRL,     FALSE,   0, 0, PREVSYNCH_ITMFUNC     },
   { 't',           ST_CTRL,           FALSE,   0, 0, TOGGLE_ITMFUNC        },
   { 'o',           ST_CTRL,           FALSE,   0, 0, SETANCHOR_ITMFUNC     },
   { 'd',           ST_CTRL,           FALSE,   0, 0, DELANCHOR_ITMFUNC     },
   { 'i',           ST_CTRL,           FALSE,   0, 0, CROSSOUT_ITMFUNC      },
   { 'u',           ST_CTRL,           FALSE,   0, 0, UNDOCROSSOUT_ITMFUNC  },
   { 'a',           ST_CTRL,           FALSE,   0, 0, ADDABBREV_ITMFUNC     },
   { VK_DOWN,       ST_VK | ST_ALT,    FALSE,   0, 0, NEXTUNALIGNED_ITMFUNC },
   { VK_UP,         ST_VK | ST_ALT,    FALSE,   0, 0, PREVUNALIGNED_ITMFUNC },
   { VK_F10,        ST_VK | ST_SHIFT,  FALSE,   0, 0, HOTPOPUP_ITMFUNC      },
   { '\0',          0,                 FALSE,   0, 0, ITMDUMMY_003             },
   { '\0',          0,                 FALSE,   0, 0, ITMDUMMY_004             },
   { '\0',          0,                 FALSE,   0, 0, ITMDUMMY_005             },
   { '\0',          0,                 FALSE,   0, 0, ITMDUMMY_006             },
   { '\0',          0,                 FALSE,   0, 0, ITMDUMMY_007             },
   { '\0',          0,                 FALSE,   0, 0, LAST_ITMFUNC }
};
#else
   extern KEYPROFTABLE DefITMKeyTable[LAST_FUNC+1];
#endif

#if defined(ITMINIT_TABLES)
  KEYPROFTABLE ITMKeyTable[LAST_FUNC+1];
#else
  extern KEYPROFTABLE ITMKeyTable[LAST_FUNC+1];
#endif
