////////////////////////////////////////////////////////////////////////////////
// EQFTAI.H     Internal header file for analysis functions                    /
////////////////////////////////////////////////////////////////////////////////
// Copyright Notice:                                                           /
//                                                                             /
//       Copyright (C) 1990-2016, International Business Machines              |
//       Corporation and others. All rights reserved                           |
////////////////////////////////////////////////////////////////////////////////

/**********************************************************************/
/* This include file requires EQFDDE.H!!!                             */
/**********************************************************************/
#ifndef EQFDDE_INCLUDED
  #include "EQFDDE.H"
#endif


#define PATHCATFILE "%s\\%s"           // concatenate path and file
#define PATHCATFILECATEXTENSION "%s\\%s%s" // concatenate path, file and extension
#define FILEEND     "\r\n"          // fileend CR LF


/**********************************************************************/
/* Strings for functions in segmentation user exit                    */
/**********************************************************************/
#define  EQFPRESEG_EXIT     "EQFPRESEG"
#define  EQFPRESEG2_EXIT    "EQFPRESEG2"
#define  EQFPRESEGEX_EXIT   "EQFPRESEGEX"
#define  EQFPOSTSEG_EXIT    "EQFPOSTSEG"
#define  EQFPOSTSEG2_EXIT   "EQFPOSTSEG2"
#define  EQFPOSTSEGEX_EXIT  "EQFPOSTSEGEX"
#define  EQFPOSTSEGW_EXIT   "EQFPOSTSEGW"
#define  EQFPOSTSEGWEX_EXIT "EQFPOSTSEGWEX"
#define  EQFPOSTTM_EXIT     "EQFPOSTTM"
#define  EQFPOSTTM2_EXIT    "EQFPOSTTM2"
#define  EQFPOSTTMW_EXIT    "EQFPOSTTMW"
#define  EQFPREUNSEG_EXIT   "EQFPREUNSEG"
#define  EQFPOSTUNSEG_EXIT  "EQFPOSTUNSEG"
#define  EQFPREUNSEG2_EXIT  "EQFPREUNSEG2"
#define  EQFPREUNSEGW_EXIT  "EQFPREUNSEGW"
#define  EQFPOSTUNSEG2_EXIT "EQFPOSTUNSEG2"
#define  EQFPOSTUNSEGW_EXIT "EQFPOSTUNSEGW"
#define  EQFPROTTABLE_EXIT  "EQFPROTTABLE"
#define  EQFPROTTABLEW_EXIT  "EQFPROTTABLEW"
#define  EQFCHECKSEG_EXIT   "EQFCHECKSEG"
#define  EQFCHECKSEGW_EXIT   "EQFCHECKSEGW"
#define  EQFCHECKSEGEXW_EXIT   "EQFCHECKSEGEXW"
#define  EQFSHOWTRANS_EXIT  "EQFSHOW"
#define  EQFTOCGOTO_EXIT    "EQFTOCGOTO"
#define  EQFGETSEGCONTEXT_EXIT  "EQFGETSEGCONTEXT"
#define  EQFUPDATECONTEXT_EXIT  "EQFUPDATECONTEXT"
#define  EQFCOMPARECONTEXT_EXIT "EQFCOMPARECONTEXT"
#define  EQFFORMATCONTEXT_EXIT  "EQFFORMATCONTEXT"
#define  EQFQUERYEXITINFO_EXIT  "EQFQUERYEXITINFO"
#define  EQFCHECKSEGTYPE_EXIT   "EQFCHECKSEGTYPE"


/*----------- limits, capacity, boundaries ------------------------------------*/
#define MAX_NUM_TOKENS      400            // maximum number of tokens
#define MAX_ALLOC_TOKENS    400
#define MAX_NUM_SEGTOKENS   (5 * MAX_NUM_TOKENS)
#define MAX_ALLOC_SEGTOKENS (MAX_NUM_SEGTOKENS )
#define MAXTAGLEN           512            // maximum length of tag

#define BOOKVARSTART        '&'            // start character of bookmaster vars
#define BLANK               ' '
#define ON                    1
#define OFF                   0

#define BUFFERSIZE        16192            // Length of the text buffer
#define GML_TAG_LEN          18
#define TOLST_REPLY_SIZE  64000L           // length of tolstoy reply size
#define SLIDER_INCREMENT    101                  // number of increments on slider bar
#define OUTPUT_MEM_NUM        6                // size output memory db structur

#define MAX_SEG_CLASSES       9            // number of seg. classes

//
// definitions for most used segment table of redundancy counting
// GQ: the filtering is now done in the report code, the values have been
//     changed to the minimal value (frequency) and the maximum value (segments)
// ==============================================================

// number of segments in the most used segment table
// old value: #define MAX_MOSTUSEDSEGS      100   
#define MAX_MOSTUSEDSEGS      0x7FFF

// frequency threshhold for segments to appear in most used segment table
// old value: #define MAX_MOSTUSEDSEGSTHRESHHOLD  3
#define MAX_MOSTUSEDSEGSTHRESHHOLD  2


/** defines and data for the protect specific section processing */

/** max number of protect section tags 
    Note: enlarge this when new tag groups are added to the tables located in EQFTSEGM.CPP  */
#define MAXNUMOFPROTECTTAGGROUPS 8

/** tag IDs for usage with individual section protection flags */
typedef enum _PROTECTTAGID { XMPTAGID, SCREENTAGID, MSGNUMTAGID, CODEBLOCKTAGID, METATAGID, ENDOFTAGLIST } PROTECTTAGID;

typedef struct _XMPTAG
{
  CHAR_W szTag[60];
  int    iLen;
  PROTECTTAGID Id;
} XMPTAG, *PXMPTAG;

typedef struct _XMPMARKUPS
{
  char   szMarkup[20];
  PXMPTAG pStartTags;
  PXMPTAG pEndTags;
} XMPMARKUPS, *PXMPMARKUPS;






typedef enum _TASEGINFO        // segmenting info for Tolstoy:
{                              //
  SEG,                         // the following text shall be segmented
  NOSEG,                       // the following text shall not be segmented
  SKIP                         // skip this tag and do further segmenting
} TASEGINFO;

typedef enum _SEGTYPE          // segmenting type
{                              //
  LINGUISTIC,                  // segmenting is done using linguistic functions
  LINEPERLINE,                 // text is segmented line-per-line
  NOSEGMENTING                 // no building of segments within text block 
} SEGTYPE;






typedef struct _TOKENENTRYSEG  // entry in tokenlist for  segmenting:
{
  // !!!! Attention: below has to match TOKENENTRY definition ....  !!!!
  SHORT     sTokenid;          // Tokenid
  USHORT    usLength;          // Length of data string
  SHORT     sAddInfo;          // additional information from tag table
  CHAR    * pDataString;       // pointer to data string
  USHORT    usOrgId;           // original id
  USHORT    ClassId;           // class id of token
  CHAR_W  * pDataStringW;      // pointer to data string
  // !!!! Attention: above has to match TOKENENTRY definition ....  !!!!
  TASEGINFO Seginfo;           // Segmenting info for Tolstoy
  SEGTYPE  SegType;            // Segmenting type: linguistik, line-per-line ...
  BOOL     fIsText;            // tag has a text representation (e.g. variables)
} TOKENENTRYSEG, *PTOKENENTRYSEG;


typedef enum _PROCESSINGMODE        /* possible processingmodes in tokenization:     */
{
  TAGSEARCH,                        /* search for start of next tag                  */
  TAGVALIDATE,                      /* validate possible tag                         */
  ATTRIBUTESEARCH                   /* search and validate attribute                 */
} PROCESSINGMODE;



typedef enum _XLATESTATE            /* the translation status a segment may have*/
{
   ATTRTODO,                        /* segment contains attribute to be xlated */
   TODO,                            /* segment contains text to be xlated      */
   NOP,                             /* no text to be xlated                    */
   XLATED                           /* segment already translated              */
} XLATESTATE;


typedef enum _SEGSTATE        /* the different segmenting states depending on */
                              /* associated text, tagtype, segmenting state */
                              /* and segbreak type of tag */
{
   NOASSOC_NODEL_OFF_SEGOFF,
   NOASSOC_NODEL_OFF_NEUTRAL,
   NOASSOC_NODEL_OFF_SEGON,
   NOASSOC_NODEL_OFF_RESET,
   NOASSOC_NODEL_OFF_SEGLINE,
   NOASSOC_NODEL_OFF_NOSEGMENTING,
   NOASSOC_NODEL_ON_SEGOFF,
   NOASSOC_NODEL_ON_NEUTRAL,
   NOASSOC_NODEL_ON_SEGON,
   NOASSOC_NODEL_ON_RESET,
   NOASSOC_NODEL_ON_SEGLINE,
   NOASSOC_NODEL_ON_NOSEGMENTING,
   NOASSOC_DEL_OFF_SEGOFF,
   NOASSOC_DEL_OFF_NEUTRAL,
   NOASSOC_DEL_OFF_SEGON,
   NOASSOC_DEL_OFF_RESET,
   NOASSOC_DEL_OFF_SEGLINE,
   NOASSOC_DEL_OFF_NOSEGMENTING,
   NOASSOC_DEL_ON_SEGOFF,
   NOASSOC_DEL_ON_NEUTRAL,
   NOASSOC_DEL_ON_SEGON,
   NOASSOC_DEL_ON_RESET,
   NOASSOC_DEL_ON_SEGLINE,
   NOASSOC_DEL_ON_NOSEGMENTING,
   ASSOC_NODEL_OFF_SEGOFF,
   ASSOC_NODEL_OFF_NEUTRAL,
   ASSOC_NODEL_OFF_SEGON,
   ASSOC_NODEL_OFF_RESET,
   ASSOC_NODEL_OFF_SEGLINE,
   ASSOC_NODEL_OFF_NOSEGMENTING,
   ASSOC_NODEL_ON_SEGOFF,
   ASSOC_NODEL_ON_NEUTRAL,
   ASSOC_NODEL_ON_SEGON,
   ASSOC_NODEL_ON_RESET,
   ASSOC_NODEL_ON_SEGLINE,
   ASSOC_NODEL_ON_NOSEGMENTING,
   ASSOC_DEL_OFF_SEGOFF,
   ASSOC_DEL_OFF_NEUTRAL,
   ASSOC_DEL_OFF_SEGON,
   ASSOC_DEL_OFF_RESET,
   ASSOC_DEL_OFF_SEGLINE,
   ASSOC_DEL_OFF_NOSEGMENTING,
   ASSOC_DEL_ON_SEGOFF,
   ASSOC_DEL_ON_NEUTRAL,
   ASSOC_DEL_ON_SEGON,
   ASSOC_DEL_ON_RESET,
   ASSOC_DEL_ON_SEGLINE,
   ASSOC_DEL_ON_NOSEGMENTING
} SEGSTATE;

typedef struct _SEGBOUNDARY
{
   PSZ_W        pBoundary;            /* pointer to segment boundary */
   XLATESTATE XlateState;           /* translation state for segment */
   struct _SEGBOUNDARY *pNext;      /* pointer to next entry */
} SEGBOUNDARY, * PSEGBOUNDARY;


typedef struct _SEGBOUNDARYTBL              /* segment boundary table */
{
   PSEGBOUNDARY pFirstEntry;       /* pointer to first entry */
   PSEGBOUNDARY pCurEntry;          /* pointer to current entry */
   USHORT    usNumCurEntry;        /* number of current entry */
   LONG      lSize;                /* size of allocated block */
} SEGBOUNDARYTBL;



/*--------------- start of definitions for List Processing -------------------*/
#define MAX_TITLE 100
#define MAX_DICTTOKENS 20

/*---- this structure is used to store the context data for a term -----------*/
typedef struct _TACONTEXT
   {
   PSZ        pszFilename;                /* pointer to source file name      */
   ULONG      ulFilepos;                  /* position in source file          */
   USHORT     usLength;                   /* length of context                */
   struct _TACONTEXT  *pNext;             /* pointer to next context for term */
   } TACONTEXT, * PTACONTEXT;

/*---- this structure is used to store data about the already output ---------*/
/*---- contexts; it is used to output a reference to a context instead -------*/
/*---- of the complete context again -----------------------------------------*/
typedef struct _CONTEXTREF
   {
   PSZ        pszFilename;                /* pointer to source file name      */
   ULONG      ulFilepos;                  /* position in source file          */
   USHORT     usLength;                   /* length of context                */
   USHORT     usRef;                      /* reference number of the context  */
   struct _CONTEXTREF  *pNext;            /* pointer to next context for term */
   } CONTEXTREF, * PCONTEXTREF;

/*----- this is an element of the Found terms list ---------------------------*/
typedef struct _FTLENTRY
   {
   PSZ        pszTerm;                    /* lemma in new terms list          */
   PSZ        pszDictname;                /* dictionary name                  */
   PSZ        pszTranslations;            /* translations of term             */
   USHORT     usFrequency;                /* frequency of term                */
   PTACONTEXT pSContext;                  /* pointer to first Source context  */
   PTACONTEXT pTContext;                  /* pointer to first Target context  */
   } FTLENTRY, *PFTLENTRY;

/*----- this is an element of the NEW   terms list ---------------------------*/
typedef struct _NTLENTRY
   {
   PSZ        pszTerm;                    /* lemma in new terms list          */
   USHORT     usFrequency;                /* frequency of term                */
   PTACONTEXT pContext;                   /* pointer to first context         */
   } NTLENTRY, *PNTLENTRY;




/*--------------- start of definitions for Text Segmentation -----------------*/
typedef struct _SEGMENTCONTROL
{
   ULONG      ulSegmentcount;         /* number of already processed segments */
   TASEGINFO  Seginfo;                /* segmenting info                      */
   SEGTYPE    SegType;                // Segmenting type: linguistik, line-per-line ...
   SHORT      sSegState;              /* segmenting status                    */
   USHORT     usColPos;               /* column position                      */
} SEGMENTCONTROL;


typedef struct _DICTINFO
{
  USHORT rc;
  UCHAR  Token;
  UCHAR  Type;
  UCHAR  Content;
  UCHAR  Version;
  USHORT Language;
} DICTINFO, *PDICTINFO;


typedef struct _STRINGBUFFER
{
   ULONG      ulOffset;               /* offset to starting point of  buffer  */
   ULONG      ulLength;               /* length of buffer                     */
   USHORT     usNumber;               /* number of Strings in buffer          */
} STRINGBUFFER;


typedef struct _TOLSTCONTROL
{
   PSZ_W   pTextBuffer;
   CHAR    szLanguage[MAX_LANG_LENGTH+1];      // name of active language
   SHORT   sLangID;                            // ID of currently active language
   USHORT  usTokenListSize;                    // size of token list
   PSZ_W   pTokenList;                         // ptr to morphological token list
   ULONG   ulOemCP;                            // OEMCP of active language
   ULONG   ulAnsiCP;                           // Ansi CP of active language
} TOLSTCONTROL, * PTOLSTCONTROL;

typedef enum _TMSEGSTATE
  {
  SEGNO,
  SEGSTARTED,
  SEGDATA
  } TMSEGSTATE;

typedef enum _TATASK
{
   EQFTA_FILE,                      // file processing
   EQFTA_BLOCK,                     // block processing
   EQFTA_LIST,                      // list processing
   EQFTA_LIST1,                     // list processing
   EQFTA_LIST2,                     // list processing
   EQFTA_CLOSE,                     // close processing
   EQFTA_TERM,                      // termination processing
   EQFTA_SSOURCE,                   // segmented source processing
   EQFTA_TOKEN,                     // NTL and FTL token processing
   EQFTA_STARGET,                   // segmented target processing
   EQFTA_ABORT,                     // abort current process
   EQFTA_COMPLETE,                  // processing completed ok
   EQFTA_FREE,                      // free resources
   EQFTA_DONE,                      // DONE (NULL processing)
   EQFTA_INIT,                      // Initialize
   EQFTA_TM1,                       // Translation memory 1
   EQFTA_TM2,                       // Translation memory 2
   EQFTA_PRESEG,                    // User pre segmentation
   EQFTA_PRESEG2,                   // User pre segmentation2
   EQFTA_POSTSEG,                   // User post segmentation
   EQFTA_POSTTM ,                   // User exit after TM matches inclusion
   EQFTA_SEG1,                      // Normal segmentation 1
   EQFTA_SEG2,                      // Normal segmentation 2
   EQFTA_SEG3,                      // Normal segmentation 3
   EQFTA_CONFIRM,                   // user confirmation needed
   EQFTA_AUTOJOIN,                  // try to autojoin segments
   EQFTA_REDUNDCOUNT,               // perform redundancy counting
   TAFUNC_INIT,                     // initialization for function call IF
   EQFTA_NEXTFILE,                  // start next file
   EQFTA_SEGNEXT                    // next file processing of segmentation 2
} TATASK;

/*------------ for TA tasks return code ----*/
#define  TACOMPL   1                  // completed
#define  TANEXT    2                  // completed, more...
#define  TAERROR   3                  // error
#define  TA_PREV_ERROR   4            // if TAERROR then reset to TA_PREV_ERROR

typedef struct _FILEHANDLE
{
   LHANDLE  handle;
   PSZ      pszFileName;
} FILEHANDLE, *PFILEHANDLE;

#ifdef __cplusplus
extern "C" {
#endif
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPRESEG) ( PSZ, PSZ, PSZ, PSZ, PSZ, PEQF_BOOL );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPRESEG2) ( PSZ, PSZ, PSZ, PSZ, PSZ, PEQF_BOOL, HWND, PEQF_BOOL );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPRESEGEX) ( PSZ, PSZ, PSZ, PSZ, PSZ, PEQF_BOOL, HWND, PEQF_BOOL, LONG  );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPOSTSEG) ( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG);
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPOSTSEG2) ( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG, HWND,PEQF_BOOL );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPOSTSEGW) ( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG_W, HWND, PEQF_BOOL );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPOSTSEGEX) ( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG, HWND,PEQF_BOOL, LONG  );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPOSTTM) ( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG, EQF_BOOL);
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPOSTTM2) ( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG, EQF_BOOL, PULONG, PULONG, HWND, PEQF_BOOL );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPOSTTMW) ( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG_W, EQF_BOOL, PULONG, PULONG, HWND, PEQF_BOOL );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPOSTSEGWEX) ( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG_W, HWND, PEQF_BOOL, LONG  );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPREUNSEG) ( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG, PEQF_BOOL);
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPREUNSEG2) ( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG, PEQF_BOOL, PEQF_BOOL);
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPREUNSEGW) ( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG_W, PEQF_BOOL, PEQF_BOOL );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPOSTUNSEG) ( PSZ, PSZ, PSZ, PSZ, PTATAG);
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPOSTUNSEG2) ( PSZ, PSZ, PSZ, PSZ, PTATAG, PEQF_BOOL );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFPOSTUNSEGW) ( PSZ, PSZ, PSZ, PSZ, PTATAG, PEQF_BOOL );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFCHECKSEG) ( PSZ, PSZ, PSZ, PEQF_BOOL, EQF_BOOL);
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFCHECKSEGW) ( PSZ_W, PSZ_W, PSZ_W, PEQF_BOOL, EQF_BOOL);
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFCHECKSEGEXW) ( PSZ_W, PSZ_W, PSZ_W, PEQF_BOOL, LONG, ULONG ,EQF_BOOL);
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFNSTARTSTOPEXIT) ( PSZ, PSTARTSTOP *);
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFNSTARTSTOPEXITW) ( PSZ_W, PSTARTSTOP *);
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFNQUERYEXITINFO) ( PSZ, USHORT, PSZ, USHORT );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_UPDATECONTEXT) ( PSZ_W, PSZ_W );
typedef EQF_BOOL (__cdecl /*APIENTRY*/ *PFN_EQFCHECKSEGTYPE) ( PSZ, PSZ_W, PSZ_W, PSZ_W, LONG, ULONG, SHORT *, SHORT * );
#ifdef __cplusplus
}
#endif

  // structure for document name table
  typedef struct _TANAMETABLEENTRY
  {
    SHORT     sID;                     // ID of name
    CHAR      szName[MAX_LONGFILESPEC];// buffer for name
  } TANAMETABLEENTRY, *PTANAMETABLEENTRY;

  // structure for the cross document references of a single document
  typedef struct _TADOCCROSSREF
  {
    SHORT      sDocNumber;                // document number
    COUNTSUMS  CountSums;                 // document counts
  } TADOCCROSSREF, *PTADOCCROSSREF;

  // structure of an element of the document counts cross reference table
  typedef struct _TADOCCOUNTS
  {
    CHAR      szDocName[MAX_LONGFILESPEC];// buffer for document long name
    CHAR      szShortName[MAX_FILESPEC];  // buffer for document short name
    SHORT     sNumOfDocs;                 // number of document counts in cross reference array
    SHORT     sNumOfEntries;              // number of entries in cross reference array
    COUNTSUMS CountTotal;                 // total counts for this document
    COUNTSUMS CountExact;                 // counts in existing exact matches
    PTADOCCROSSREF pCrossRefs;            // pointer to document cross reference array
  } TADOCCOUNTS, *PTADOCCOUNTS;

  // structure for an element of the document/segment number table
  typedef struct _TADOCANDSEG
  {
    SHORT     sDocIndex;               // index of document
    ULONG     ulSegNo;                  // segment number within document
  } TADOCANDSEG, *PTADOCANDSEG;

  // structure for an element of the most used segment table
  typedef struct _TAMOSTUSEDSEG
  {
    LONG      lFrequency;              // frequency of segment
    CHAR_W    szSegData[MAX_SEGMENT_SIZE]; // buffer for segment data
    TADOCANDSEG aDocs[MAX_MOSTUSEDDOCSPERSEG]; // list of documents
  } TAMOSTUSEDSEG, *PTAMOSTUSEDSEG;


//
// defines and function protottypes for CRLF tag processing
//
#define MAX_CRLFINFO_TAG_LEN 40

typedef struct _STARTTAGENDTAG
{
  int         iStartTagLen;
  int         iEndTagLen;
  CHAR_W      szStartTag[MAX_CRLFINFO_TAG_LEN ]; // start tag
  CHAR_W      szEndTag[MAX_CRLFINFO_TAG_LEN ];   // end tag
} STARTTAGENDTAG, *PSTARTTAGENDTAG;

typedef struct _CRLFINFO
{
  CHAR        szMarkup[MAX_FILESPEC];            // name of the markup
  int         iEntries;                          // number of entries in start and end tag table
  int         iUsed;                             // number of used entries in start and end tag table
  BOOL        fLookForEndTag;                    // TRUE = we are looking for a specific end tag
  int         iActiveTags;                       // index of active tag
  STARTTAGENDTAG Tags[1];                        // dynamic table with start and end tags
} CRLFINFO, *PCRLFINFO;

typedef struct _LOADEDCRLFINFO
{
  int         iEntries;                          // number of entries in PCRLFINFO table
  int         iUsed;                             // number of used entries in PCRLFINFO table
  PCRLFINFO   apCrlfInfos[1];                    // dynamic table with pointers to CRLFINFOs
} LOADEDCRLFINFO, *PLOADEDCRLFINFO;

void TAFreeCRLFInfo( PLOADEDCRLFINFO pInfo );
PCRLFINFO TAGetCRLFInfoForMarkup( PLOADEDCRLFINFO pInfo, PSZ pszMarkup );
void TACheckForCRLFTags( PCRLFINFO pCrlfInfo, PSZ_W pszSegData, PBOOL pbCrlfFlag );
USHORT TALoadCRLFInfo( PLOADEDCRLFINFO *ppLoadedInfo );


/*------ structure containing all the instance variables used locally --------*/
/*------ this structure is allocated by TA and freed before exiting ----------*/
typedef struct _TAINSTDATA
{
   BOOL           fComplete;
   PSZ            pszCurSourceFile;           // pointer to current source
   USHORT         usCurNumSourceFile;         // number of current source file
   ULONG          ulFileSize;                 // size of current file
   HFILE          hSource;                    // handle to current source file
   PEXCLUSIONLIST pExclusionList;             // Internal Exclusion list
   USHORT         usOutDict;                  // Output Dictionary Handle
   CHAR           * pchTermBuffer;            // buffer for terms with translations
   CHAR           szNTLname[MAX_EQF_PATH];    // New Terms list name
                                              // if NULL, do not create
   PNTLENTRY      pNTLEntry;                  // pointer to first entry in
                                              // new terms list
   PFTLENTRY      pNTLFreeEntry;              // next free entry in new terms list
   PFTLENTRY      pFTLFreeEntry;              // next free entry in found terms list
   USHORT         usExclDict;                 // Exclusion Dictionary handle
   LHANDLE        hOutputMemDb;               // output memory database handle
   //FILEHANDLE     *pstInputMemDb;             // search TMs
   //FILEHANDLE     *pstOutputMemDb;            // output TM
   //LHANDLE        hDocumentMemDb;             // document memory database handle
   CHAR           szDocumentMemDb[MAX_LONGPATH];  // document memory database name,*/
   BOOL           fAll;                       // is input text complete?
   BOOL           fSeglength;                 // segment length exceeded?
   CHAR_W         szTextBuffer[BUFFERSIZE];   // Text buffer to keep the file
   USHORT         uCurTextPos;                // current position in input text
   SEGMENTCONTROL SegmentControl;             // Segmenting Control Info
   PTOKENENTRY    pTokenList;                 //
   PTOKENENTRYSEG pSegTokenList;              //
   SEGBOUNDARYTBL SegBoundaryTbl;             //
   TOLSTCONTROL   TolstControl;               // tolstoy control data
   BOOL           fTolstoy;                   // tolstoy initialised
   PBUFCB         CBSegSource;                // ptr current pos of segm.file
   PBUFCB         CBSegTarget;                // ptr current pos of segm.file
   PSEGBOUNDARY   pSegBoundaryList;           // segment boundary list
   ULONG          ulSegNum;                   // segment number
   PSZ_W          pRest;                      // ptr to not processed rest
   //PVOID          pGetIn;                     // work
   //PVOID          pGetOut;                    //   areas for
   //PVOID          pExtIn;                     //     translation
   //PVOID          pExtOut;                    //       memories
   //PVOID          pRepOut;                    //         pointers
   HMODULE        hmodSegmentDll;
   PFN_EQFPRESEG        pfnEQFPreSeg;       // pre-segmentation exit within user exit DLL
   PFN_EQFPOSTSEG       pfnEQFPostSeg;      // post-segmentation exit within user exit DLL
   PFN_EQFPRESEG2       pfnEQFPreSeg2;      // pre-segmentation exit within user exit DLL
   PFN_EQFPOSTSEG2      pfnEQFPostSeg2;     // post-segmentation exit within user exit DLL
   PFN_EQFPREUNSEG      pfnEQFPreUnSeg;     // pre-unsegmentation exit within user exit DLL
   PFN_EQFPOSTUNSEG     pfnEQFPostUnSeg;    // post-unsegmentation exit within user exit DLL      
   PFN_EQFPREUNSEG2     pfnEQFPreUnSeg2;    // pre-unsegmentation exit within user exit DLL      
   PFN_EQFPOSTUNSEG2    pfnEQFPostUnSeg2;   // post-unsegmentation exit within user exit DLL     
   PFN_EQFPOSTUNSEGW    pfnEQFPostUnSegW;   // post-unsegmentation exit within user exit DLL     
   //------------ Troja 2.1 last used values ------------------------------
   BOOL           fTerminate;         // Terminate TA
                                      //   (positive response to kill
                                      //     confirmation)
   TAGTABLE       *pQFTagTable;       // pointer to QFTagTable
   PBUFCB         pCBNewMatchFile;    // pointer to IOCB for new matches file
   PSZ_W          pszSegment;         // pointer to space to segment string
   PSZ            pszTag;             // pointer to space to tag string
   PUSHORT        pusFileStatus;      // pointer to file status array
   USHORT         usError;            // error code
   // ------- Troja 3.0 add ons -------------
   BOOL           fNotComplete;        // completion flag
   PVOID          pLPDATA;            //pointer to local data for list process.
   PFN_EQFPOSTTM  pfnEQFPostTM;       // support another post user exit after transl.memory matches
   PFN_EQFPOSTTM2 pfnEQFPostTM2;      // support another post user exit after transl.memory matches
   BOOL           fTMInserted;        // file changed due to inserted elements.
//--------------- addition by GBA 30.9.91 -------------------
   ULONG     ulCurFileOffset;                  // current file offset
//--------------- end addition by GBA 30.9.91 -------------------
   ULONG     ulSegsNotReplaced; // # of not replaced segments         /* 2@20A*/
   ULONG     ulSegsReplaced;    // # of replaced segments
   ULONG     ulTotalSegs;       // # of segments in document    /* 1@KIT1270A */
   BOOL      fNewMatchFileUsed;  // new-matches-file-used flag
   CHAR      szTempDisp[80];      // buffer for display strings
                                                                       /*3@54A*/
   BOOL      fUseLatestTMMatch;                  /* use latest exact match for  */
                                               /* insert of TM match          */
                                                                /* 2@KIT0905A */
   BOOL    fNoDisplayNotReplacedMessage; // do not display seg not replaced msg
  PLOADEDTABLE     pLoadedTable;       // ptr to loaded TagTable for document
  PLOADEDTABLE     pLoadedQFTable;     // ptr to loaded QF TagTable
  OBJNAME          szDocObjName;       // document object name
  CHAR             szFolMemory[MAX_LONGFILESPEC]; // folder memory
  /*******************************************************************/
  /* Currently activated source language, translation memory and     */
  /* format                                                          */
  /*******************************************************************/
  CHAR             szCurFormat[MAX_FILESPEC];        // format / Tag Table
  CHAR             szCurMemory[MAX_LONGFILESPEC];    // Translation Memory
  CHAR             szCurSourceLang[MAX_LANG_LENGTH]; // source language
  /*******************************************************************/
  /* Source language, translation memory and format for new          */
  /* document                                                        */
  /*******************************************************************/
  CHAR             szDocFormat[MAX_FILESPEC];        // format / Tag Table
  CHAR             szDocMemory[MAX_LONGFILESPEC];    // Translation Memory
  CHAR             szDocSourceLang[MAX_LANG_LENGTH]; // source language
  CHAR             szDocTargetLang[MAX_LANG_LENGTH];       // target language
  PFN_EQFCHECKSEG pfnEQFCheckSeg;       // segment checking exit within user exit DLLs
  PSZ_W            pszPrevSegment;        // pointer to space to segment string
  PTBDOCUMENT      pDoc;                // pointer to loaded document
  PTBDOCUMENT      pSourceDoc;          // pointer to loaded document (source)
  SHORT            sTargetLangID;       // ID of target language
  BOOL             fExactContextTMMatch;// Exact Context TM selected ...
  CHAR             szROMem[MAX_NUM_OF_READONLY_MDB+1][MAX_LONGPATH]; //full qualified memory
  FILEHANDLE       *pstROMemDb;         // output R/O Mem handles
  //PTMX_GET_OUT_W   pTMBestMatch;        // work

  // variables for TM match word counting
  ULONG            ulNoProps;           // no matches
  ULONG            ulFuzzy;             // fuzzy match
  ULONG            ulExactExact;        // exact-exact match
  ULONG            ulExactOne;          // one exact match
  ULONG            ulExactMore;         // more than one exact match
  ULONG            ulTotal;             // total number of words
  ULONG            ulRepl;              // replace matches
  ULONG            ulFuzzyRepl;         // fuzzy replace matches
  ULONG            ulSegNoProps;        // segments with no matches
  ULONG            ulSegFuzzy;          // segments with fuzzy match
  ULONG            ulSegExactExact;     // segments with exact-exact match
  ULONG            ulSegExactOne;       // segments with one exact match
  ULONG            ulSegExactMore;      // segments with more than one exact match
  ULONG            ulSegTotal;          // total number of segments
  ULONG            ulSegRepl;           // segments with replace matches
  ULONG            ulSegFuzzyRepl;      // segments with fuzzy replace matches
  ULONG            ulMachineMatch;      // machine matchs
  ULONG            ulSegMachineMatch;   // segments with machine matchs
  COUNTSUMS Total;                      // total numbers
  COUNTSUMS ExactExact;                 // counting results - exact-exact matches
  COUNTSUMS Exact;                      // counting results - exact matches
  COUNTSUMS Fuzzy1;                     // counting results - fuzzy matches 1
  COUNTSUMS Fuzzy2;                     // counting results - fuzzy matches 2
  COUNTSUMS Fuzzy3;                     // counting results - fuzzy matches 3
  COUNTSUMS NoProps;                    // counting results - no proposals
  COUNTSUMS MTProps;                    // counting results - MT proposals
  COUNTSUMS Repl;                       // counting results - replace matches
  COUNTSUMS ExactMore;                  // counting results - more than one exact match

  //LHANDLE          hRedundCountTM;      // temp TM for redundancy counting
  CHAR             szRedundCountTM[MAX_EQF_PATH];  // name of hRedundCountTM
  TMX_EXT_IN_W       stExtIn;             // extract in structure
  TMX_EXT_OUT_W      stExtOut;            // extract out structure
  TMX_PUT_IN_W       stPutIn;             // put in structure
  TMX_PUT_OUT_W      stPutOut;            // put out structure
  ULONG            ulLastSliderPos;     // last slider position
  PTANAMETABLEENTRY pNames;             // ptr to document name table
  SHORT            sNumOfNames;         // number of name table entries
  PTADOCCOUNTS     pDocCounts;          // ptr to document count table
  SHORT            sNumOfDocs;          // number of documents in count table
  CHAR             szCurMarkup[MAX_FILESPEC]; // currently active markup table
  PLOADEDTABLE     pCurTable;           // ptr to currently loaded tag table
  SHORT            sTMLangID;           // ID of TM source language
  CHAR             szTMLang[MAX_LANG_LENGTH]; // TM source language
  PTOKENENTRY      pTokBuf;             // ptr to tokenize buffer
  CHAR_W           szLastSeg[MAX_SEGMENT_SIZE];  // buffer for last segment
  ULONG            ulWords;             // words of last segment
  PSHORT           psUsed;              // segment used by document table
  SHORT            sNumOfUseEntries;    // number of entries in psUsed
  COUNTSUMS        UniqueCount;         // Words/Segmenst which have to be translated
  LONG             lFrequency;          // frequency of current segment
  TADOCANDSEG      aCurDocs[MAX_MOSTUSEDDOCSPERSEG]; // list of documents of
                                                  // current segment
  SHORT            sMaxMostUsedSegments;// size of most used segments array
  SHORT            sMaxMostUsedThreshhold;// threshhold value segment frequency
  PTAMOSTUSEDSEG   pMostUsedSegs;       // ptr to most used segment array

  EQF_BOOL         fNoSegment;           // no normal segm. required
  BOOL             fFirstRead;           // TRUE = this is the first read call
  BOOL             fFirstWrite;          // TRUE = this is the first write call
  CHAR_W           szSGMLSegBuffer[3*MAX_SEGMENT_SIZE];  // buffer for segment in SGML syntax
  PFN_EQFCHECKSEGW  pfnEQFCheckSegW;       // segment checking exit within user exit DLL
  PFN_EQFPREUNSEGW  pfnEQFPreUnSegW;       // pre-unsegmentation exit within user exit DLL
  PFN_EQFPOSTSEGW   pfnEQFPostSegW;        // post-segmentation exit within user exit DLL
  PFN_EQFPOSTTMW    pfnEQFPostTmW;         // post-memory exit within user exit DLL
  CHAR             szASCIIBuffer[ BUFFERSIZE ];
  PFN_UPDATECONTEXT  pfnUpdateContext;     // update context exit within user exit DLL
  CHAR_W  szContext[MAX_SEGMENT_SIZE];   // segment context info
  USHORT      usCharSetInFile;             // active charset: either from tagtable or file
  ULONG         ulFilled;                               // buffer filled up for further processing
  // part for machine translation file creation
  BOOL              fMachTransFileStarted;  // file for machine translation segments has been started
  BOOL              fMachTransFileUsed;     // segments have been written to machine translation file
  PBUFCB            pCBMachTransFile;       // file receiving machine translation segments
  LONG              lMTWords;               // number of words written to MT segments file
  BOOL              fMTStatusSet;           // TRUE = MT statis has already been set
  COUNTSUMS         ToBeTranslCount;        // Words/Segments which have to be translated
  COUNTSUMS         RedundandCount;         // Words/Segments which are redundant
  ULONG             ulReplacedWords;        // number of replaced words
  PFN_EQFPOSTSEGEX  pfnEQFPostSegEx;        // post-segmentation exit within user exit DLL
  PFN_EQFPRESEGEX   pfnEQFPreSegEx;         // pre-segmentation exit within user exit DLL
  PFN_EQFPOSTSEGWEX  pfnEQFPostSegWEx;      // post-segmentation exit within user exit DLL
  BOOL              fProtectMode;           // TRUE = protect all segments, FALSE = normal processing
  EQF_BOOL          fProtXmp;               // protect XMP sections flag
  PFN_EQFCHECKSEGEXW pfnEQFCheckSegExW;     // segment checking exit within user exit DLLs
  PLOADEDCRLFINFO  pCrlfLoadedInfo;         // info area for CRLF handling processing loaded into memory
  PCRLFINFO        pCrlfInfoForMarkup;      // CRLF handling info for active markup
  BOOL             fCrlfFlag;               // CRLF handling flag; TRUE = respect CRLF
  PFNGETSEGCONTTEXT pfnGetSegContext;
  PVOID            pvGMOptList;             // loaded global memory optionm file
  PVOID            pvMemoryLookupData;      // data area for memory lookup
  // fields for annotation data processing
  USHORT           usAnnotationDataCopyMode;// mode for annotation data copying; 0 = exact matches, 1 = exact context match, 2 = exact-exact match
  CHAR_W           szComment[MAX_SEGMENT_SIZE];// annotation comment
  CHAR_W           szStyle[40];             // annotation style
  BOOL             fMetaDataAdded;          // TRUE = metadata has been added by the analysis process
  EQF_BOOL         fProtMsgNum;             // protect MsgNum sections flag
  EQF_BOOL         fProtMeta;               // protect Meta sections flag
  EQF_BOOL         fProtScreen;             // protect Screen sections flag
  EQF_BOOL         fProtCodeBlock;          // protect CodeBlock sections flag
  // fields for protected section processing
  int              iNumOfActiveStartTags;   // number of entries in list of active start tags
  int              iNumOfActiveEndTags;     // number of entries in list of active end tags
  XMPTAG           aActiveStartTags[MAXNUMOFPROTECTTAGGROUPS]; // list of active start tags
  XMPTAG           aActiveEndTags[MAXNUMOFPROTECTTAGGROUPS]; // list of active end tags
  BOOL             fProtectedSelfContainedTag; // TRUE = a protected self contained tag is currently active
  PFN_EQFCHECKSEGTYPE pfnEQFCheckSegType;       // count/memory type checking exit within user exit DLLs

} TAINSTDATA, *PTAINSTDATA;

/*----------------------------------------------------------------------------*/
/*       structure containing input parameters passed from the handler to     */
/*       the instance; it contains also shared variables                      */
/*       referenced by EQFTAMnl.C, EQFANA00.C                                 */
/*       should in future be moved in another file   ?                        */
/*----------------------------------------------------------------------------*/
typedef struct _TAINPUTPARAM
{
   BOOL    fNoComplete;                        /* flag for analyze ok-window  */
   CHAR    szFolder[MAX_EQF_PATH];             /* folder name                 */
   CHAR    szFolderProp[MAX_EQF_PATH];         /* folder property name        */
   CHAR    szSOURCE_Path[MAX_EQF_PATH];        /* path for source files       */
   CHAR    szSEGSOURCE_Path[MAX_EQF_PATH];     /* path for segmented source,  */
                                               /* if NULL do not segment text */
   CHAR    szSEGNEWMATCH_Path[MAX_EQF_PATH];   /* path for segmented matches  */
                                               /* that were not found         */
   CHAR    szSEGTARGET_Path[MAX_EQF_PATH];     /* path for segmented target   */
                                               /* if NULL, do not create      */
                                               /* segmented target file       */

   BOOL    fInsertTMMatches;                   /* insert TM matches to docum. */
                                               /* folder Tm is input TM       */
                                                                       /*3@54A*/
   BOOL    fUseLatestTMMatch;                  /* use latest exact match for  */
                                               /* insert of TM match          */
   BOOL    fInsertToTM;                        /* insert TM matches to folder */
                                               /* selected TM's are input TM's*/
   BOOL    fInsertNewMatch;                    /* insert new matches to file  */

   SHORT   sMatchLevel;                        // match level
   STRINGBUFFER stSourcefiles;                 /* buffer description for source */
   USHORT  usLangCode;                         /* language code               */
   CHAR    szTagTableName[MAX_EQF_PATH];       /* file name of Tagtable       */
   CHAR    szExclusionList[MAX_EQF_PATH];      /* Exclusion list file         */
   CHAR    szOutDictName[MAX_LONGFILESPEC];    /* Output Dictionary name      */
   CHAR    szNTLname[MAX_EQF_PATH];            /* New Terms list name         */
                                               /* if NULL, do not create      */
   CHAR    szFTLname[MAX_EQF_PATH];            /* Found Terms list name       */
                                               /* if NULL, do not create      */
   STRINGBUFFER  stMorphDict;                  /* buffer for Morph. Dictionaries */
   CHAR    szExclDictname[MAX_LONGFILESPEC];   /* Exclusion Dictionary name   */
   BOOL    fNTLMwterm;                         /* new multi-term list ? */
   BOOL    fNTLcontext;                        /* context for new term list ? */
   BOOL    fFTLcontext;                        /* context for found term list?*/
   USHORT  usNumOccurences;                    /* obsolete n. of occurrences  */
                                               /* not to be used              */

   CHAR         szBuffer;                      /* Buffer for format           */
   USHORT       usMWTOption;                   /* selected Options            */

   STRINGBUFFER stInputDict;                   // buffer of input dictionaries
                                               // for NTL
   CHAR    szOutputMemDb[MAX_LONGPATH];        /* output memory database name,*/
                                               /* if EOS, do not create      */
   STRINGBUFFER stInputMemDb;                  /* buffer of input memory DBs  */
   BOOL      fNoRemoveObject;                   // do not remove analysis object
   EQF_BOOL      fKill;                         // general kill flag
   TATAG     TATag[MAX_SEG_CLASSES];            // tags to be used
   PTAINSTDATA pInD;                            // pointer to instance data
   CHAR      szTodo[MAX_TITLE];                 // buffer for text string
   CHAR      szDone[MAX_TITLE];                 // buffer for text string
   HWND      hwndToDo;                          // listbox with files to be proc
   HWND      hwndDone;                          // listbox with files processed
   HWND      hwndProcWin;                       // process window handle
   CHAR      szEditName[MAX_FILESPEC];
   CHAR      szProgPath[MAX_EQF_PATH];
   CHAR      szTempName[MAX_PATH144];
   //---------------- Troja 2.1 last used values ------------------------------

   USHORT    usFTLNumOccurences;               /* number of occurences for terms */
                                               /* in fnd terms list           */
   USHORT    usNTLNumOccurences;               /* number of occurences for terms */
                                               /* in new terms list           */
   BOOL      fNTLProcessing;                   /* NTL processing requested    */
   HWND      hwndInfoText;                     /* description in window       */
                                                                  /* 2@KIT1213A */
   BOOL      *pfDocLocked;                     /* document-has-been-locked flag*/
   /*******************************************************************/
   /* 'Local' variables of procedure TAFileProcessing                 */
   /*******************************************************************/
   CHAR      szSegSource[MAX_EQF_PATH]; // full name of current seg source
   CHAR      szSegTarget[MAX_EQF_PATH]; // full name of current seg target
   CHAR      szFileName[MAX_EQF_PATH];  // full name of file to be opended
   /*******************************************************************/
   /* Flags for locking of term lists and exclsusion dictionary      */
   /*******************************************************************/
   BOOL     fNTLLocked;                 // NTL has been locked flag
   BOOL     fFTLLocked;                 // FTL has been locked flag
   BOOL     fExcLocked;                 // Exclusion list has been locked flag
   BOOL     fExcDicLocked;              // Exclusion dict. has been locked flag
   /*******************************************************************/
   /* Variables for batch mode of analysis                           */
   /*******************************************************************/
   PDDEANALYSIS pDDEAnalysis;          // ptr for batch mode analysis
   HWND      hwndErrMsg;               // window handle for error messages
   BOOL      fBatch;                   // TRUE = we are in batch mode
   BOOL      fAutoJoin;                // try to join segments if fuzzy match
   BOOL      fAutoJoinDone;            // segments have been autojoined

   BOOL      fExactContextTMMatch;     // Exact Context TM selected ...
   BOOL      fEndTMDone;               // TRUE = EndTM handling already done
   /*******************************************************************/
   /* Variables for long file name support and alias function         */
   /*******************************************************************/
   PPOOL     pPool;                    // ptr to string pool
   PSZ       *apszLongNames;           // ptr to ptr array with long file names
   PSZ       *apszAlias;               // ptr to ptr array with alias names
   CHAR      szLongBuffer[MAX_LONGFILESPEC]; // buffer for long file names
   CHAR      szAliasBuffer[MAX_LONGFILESPEC]; // buffer for alias names
   CHAR      szShortAlias[MAX_FILESPEC]; // buffer for short alias names
   BOOL      fTMMatch;                 // collect TM match info
   CHAR      szReplSegment[2*MAX_SEGMENT_SIZE];  //  replace segment area
   BOOL      fRedundCount;             // redundance counting flag
   CHAR      chControlInfo[512];       // buffer for control info
   BOOL      fForceEqualWhiteSpace;    // TRUE = force same whitspace
                                       // at segment end during autosubst
   BOOL      fIgnoreIdentical;         // ignore identical source/target segs
   BOOL      fIgnorePath;              // ignore document path during autosubst
   BOOL      fIgnoreComment;           // ignore memory proposals with comments
   BOOL      fNoGenericReplace;        // no generic inline tag replace
   BOOL      fLeadingWS;               // setting of leading WS handling flag
   BOOL      fTrailingWS;              // setting of leading WS handling flag
   EQF_BOOL  fRespectCRLF;             // respect CRLF flag
   EQF_BOOL  fNoAddBlank;              // no additional blanks at segment end
   CHAR_W    szReplSegmentW[2*MAX_SEGMENT_SIZE];
   TATAG_W     TATagW[MAX_SEG_CLASSES];            // tags to be used
   PSZ       pszObjList;               // list of folder object names (if multiple folders are selected)
   PSZ       pszActiveFolder;          // points to currently active folder in pszObjList
   BOOL      fYesToAll;                // TRUE = answer all confirmations message with YES
   PVOID     *ppvCallersTAInput;       // address of callers TAINPUT pointer or NULL
   // part for machine match file creation
   BOOL      fMachTrans;               // add no matches to machine translation file
   CHAR      szMACHTRANS_Path[MAX_EQF_PATH]; // directory for machine translation file
   SHORT     sSliderPos;               // current slider position
   BOOL      fSecondRun;               // TRUE = second run done in analysis (segment processing)
   ANAMTDATA stMTData;                 // structure containing data exchanged between analysis and MT handler
   USHORT    usLastFuzzyness;          // fuzzyness of last retrieved segment
   int       iLastMatchType;           // match type of last retrieved segment
   ULONG     ulLastSrcWords;           // source words of last retrieved segment
   // fields for time/performance measurements
   ULONG     ulTimeOther;
   ULONG     ulGetExactTMMatchstart;
   ULONG     ulTMGetW;
   ULONG     ulReplMatch;
   ULONG     ulCopyBest;
   ULONG     ulTimeGetMatch;
   ULONG     ulTimeFileRead;
   ULONG     ulTimeListProcessing;
   ULONG     ulTimeWordCount;
   ULONG     ulTimeRedundCount;
   ULONG     ulTimeMachMatch;
   ULONG     ulTimeSNOMATCH;
   ULONG     ulTimeInsertIntoTM;
   LONG      lStartTime;             // starting time of analysis process
   BOOL      fFolderLocked;          // TRUE = folder has been locked
   PSZ       pSupportedMarkups;      // list of markup tables supported by MT server
   BOOL      fLoadOfSupportedMarkupsFailed; // TRUE = load of external markup tables failed
   PSZ_W     pWSList;

   // fields for MTI logging (array is for the three categories simple, medium, complex)
   ULONG      ulMTTotalWords[3];       // number of words processed in MT handling
   ULONG      ulMTTotalSegs[3];        // number of segments processed in MT handling
   ULONG      ulMTSendWords[3];        // number of words send to MT server
   ULONG      ulMTSendSegs[3];         // number of segments send to MT server
   BOOL       fMTValuesAvailable;      // TRUE = MT word counts have been filled 
   CHAR_W    chControlInfoW[512];      // buffer for control info (UTF16 version)
   PVOID      pvMatchListData;         // ptr for match list processing
   EQF_BOOL   fMatchList;              // "create match list" flag
   EQF_BOOL   fBalanceList;            // state of create balance list checkbox

   EQF_BOOL     fUseReplacementList;    // use given replacement list  
   CHAR         szReplacementList[MAX_LONGFILESPEC]; // replacement list name
   PVOID        pvReplacementList;      // ptr to any in-memory copy of a loaded replacement list
   CHAR         szProfile[MAX_LONGFILESPEC]; // profile name
   BOOL         fProfile;                    // use profile flag
   LONG         hProfile;                    // handle of a loaded analysis profile
   EQF_BOOL     fProtXmp;               // protect XMP sections flag
   EQF_BOOL     fStopAtFirstExact;      // stop at first exact match flag for folder search memories
   EQF_BOOL     fStopAtFirstExactAddMem;      // stop at first exact match flag for additional ananysis search memories
   BOOL         fSetSegDateDone;        // TRUE = call to SetSegDate already performed for this document
   CHAR_W       szOrgSegSource[MAX_SEGMENT_SIZE+1];  //  buffer for segment source text
   CHAR_W       szReplSegmentSourceW[2*MAX_SEGMENT_SIZE]; // buffer for segment source when checking for replace matches
   EQF_BOOL     fProtMsgNum;             // protect MsgNum sections flag
   EQF_BOOL     fProtMeta;               // protect Meta sections flag
   EQF_BOOL     fProtScreen;             // protect Screen sections flag
   EQF_BOOL     fProtCodeBlock;          // protect CodeBlock sections flag
   CHAR         szMtOptions[8096];       // buffer for specified MT output file options
} TAINPUT, * PTAINPUT;



/* SETCOLUMNPOS : set column position depending on NEWLINE and EOS            */
#define SETCOLUMNPOS(ColumnPos)  if ( *pCurTextPos == EOS )         \
                                 {                                  \
                                    ColumnPos = usCurColumnPos - 1; \
                                 }                                  \
                                 else if ( *pCurTextPos == LF )     \
                                 {                                  \
                                    ColumnPos = 0;                  \
                                 }                                  \
                                 else                               \
                                 {                                  \
                                    ColumnPos = usCurColumnPos;     \
                                 } /* endif */

// prepare file from path and name and delete it
#define TADELETE(a,b,c) {sprintf(a, PATHCATFILE, b, c);   \
                         UtlDelete(a, 0L, FALSE);}

// close file and set file handle to null if necessary
#define TACLOSE( a )    if ( a != NULLHANDLE )    \
                        {                         \
                           UtlClose( a , FALSE ); \
                           a = NULLHANDLE;        \
                        }

/*---------------------------- end of macros ---------------------------------*/


//  structure used during the unsegment process
typedef struct _TAUNLOADTAGS
{
      PSZ           pTag;                            // pointer to tag
      USHORT        usLength;                        // length of the tag
} TAUNLOADTAGS, * PTAUNLOADTAGS;








/**********************************************************************/
/* Defines for new tokenize function - part 2                         */
/**********************************************************************/
/**********************************************************************/
/* structure for tag table extract (tag name + tag ID)                */
/**********************************************************************/
typedef struct _TBTAGENTRY
{
   PSZ_W      pszTag;                    // ptr to tag name
   PSZ_W      pszEndDel;                 // ptr to tag end delimiter
   SHORT    sID;                       // associated tag ID
   USHORT   usLength;                  // length of length-terminated tags
   USHORT   usColPos;                  // column position or 0
   unsigned short fAttr       : 1;     // tag-has-attributes flag
   unsigned short fTranslInfo : 1;     // tag-has-translatable-info flag
   unsigned short fReserved   : 14;    // filler, fills bit flags up to 16bit
} TBTAGENTRY, *PTBTAGENTRY;


PNODEAREA  TACreateTagTree
(
  PLOADEDTABLE pTable,
  PTAGTABLE   pTagTable,
  CHAR_W      chMultSubst,             // muliple substitution character
  CHAR_W      chSingleSubst,           // single substitution character
  BOOL        fMsg
);
PNODEAREA  TACreateAttrTree
(
  PLOADEDTABLE pTable,
  PTAGTABLE   pTagTable,
  CHAR_W      chMultSubst,             // muliple substitution character
  CHAR_W      chSingleSubst,           // single substitution character
  BOOL        fMsg
);
PNODEAREA TACreateNodeTree
(
  PTBTAGENTRY pTags,
  ULONG       ulNoOfTags,
  CHAR_W      chMultSubst,             // muliple substitution character
  CHAR_W      chSingleSubst,           // single substitution character
  BOOL        fMsg
);

/**********************************************************************/
/* Structure of stack for processing of multiple substitution tags    */
/* and attributes                                                     */
/**********************************************************************/
typedef struct _MSSTACK
{
  PTREENODE   pNode;                   // ptr to active node
} MSSTACK, *PMSSTACK;

/**********************************************************************/
/* Structure of stack to store retry positions if a selectet path is  */
/* not unique                                                         */
/**********************************************************************/
typedef struct _RETRYSTACK
{
  PTREENODE   pNode;                   // ptr to active node
  PSZ_W       pszData;                 // ptr to current position in imput data
  PSZ_W       pszTag;                  // ptr to current tag start position
} RETRYSTACK, *PRETRYSTACK;

/**********************************************************************/
/* Structure of tokenize status area                                  */
/**********************************************************************/
typedef struct _TOKSTATUS
{
  /*====================== general flags and variables ===============*/
  BOOL        fMsg;                    // display-error-messages flag
  USHORT      usColPos;                // current column position
  USHORT      usStartColPos;           // start column position of tag/attr
  /*===================== wildcard characters ========================*/
  CHAR_W        chMultSubst;             // multiple substitution character
  CHAR_W        chSingleSubst;           // single substitution character
  /*================= multiple substitution stack ====================*/
  USHORT      usMSStackSize;           // size of stack in # of elements
  PMSSTACK    pMSStack;                // ptr to stack
  /*==================== retry other path stack ======================*/
  USHORT      usRetryStackSize;        // size of stack in # of elements
  PRETRYSTACK pRetryStack;             // ptr to stack
  /*======================== tag table ===============================*/
  PLOADEDTABLE pTable;                 // ptr to loaded tag table
} TOKSTATUS, *PTOKSTATUS;



BOOL TAMatchTag
(
  PTOKSTATUS  pTokStatus,              // ptr to tokenize status area
  PSZ_W       *ppszData,               // pointer to current position in data
  PTREENODE   pNode,                   // root node of node tree for matching
  PTOKENENTRY pToken,                  // ptr to token buffer for results
  USHORT      usFlags                  // flags; see following defines
);

#define MATCH_SKIP_WHITESPACE 0x0001   // skip whitespace before tags
#define MATCH_SKIP_NOTAG_DATA 0x0002   // skip all data before tags
#define MATCH_QUOTED_STRINGS  0x0004   // do handling for quoted strings
#define MATCH_SKIP_DATA       0x0008   // skip unknown data
#define MATCH_DATA_COMPLETE   0x0010   // passed data are complete


#define TIMER     1
#define TIMEOUT   500
#define MAX_TITLE 100                  //length of title of analysis instance

//--- Analysis Instance Data Area
typedef struct analysis_ida {
    IDA_HEAD IdaHead;                  // common header of instance area
    CHAR     szFolder[_MAX_PATH];      // name of associated folder
    CHAR     szTitle[MAX_TITLE];       // title bar text of analysis window
    PTAINPUT ProcParm;                 // parms for analysis instance
    HWND     hwndDocLB;                // listbox for document names
    OBJNAME  szObjName;                // name of object
    OBJNAME  szFolObjName;             // folder object name
    CHAR     szMemProp[1024];          // buffer for return of mdb handler
    CHAR     szString[NUM_OF_FOLDER_DICS*MAX_FILESPEC]; // dummy string
    PSZ      pszObjList;               // object name list of selected folders
    PSZ      pszActiveObj;             // pointer to currently processed folder
} ANALYSIS_IDA, *PANALYSIS_IDA;

//--- initial positions for analysis instance window
#define ANALYSIS_X           20
#define ANALYSIS_Y           50
#define ANALYSIS_CX         590
#define ANALYSIS_CY         290

//--- maximum number of bytes in edit field for minimum number of occurences
#define ANA_NO_OF_OCCURENCES    3
/*
+------------------------------------------------------------------------------+
| Analysis Dialog                                                              |
+------------------------------------------------------------------------------+
*/
#define SAVE_DIC_NUM 10                     //number of saved last used dicts.
#define SAVE_MDB_NUM 10                     //number of saved last used mdbs

#define CHECKED     1                       //check for buttons

struct   _indict         // privat structure to handle the input dictionaries
{
   CHAR         szDictName[MAX_DICTS + 1][MAX_EQF_PATH];
};
typedef struct _indict INDICT, *PINDICT;

//-- instance area for analysis dialog
struct _aida
{                                           //procedure InvokeAnalysisDlgProc
   IDA_HEAD     IdaHead;                    //common IDA header
   OBJNAME      szFolderObjName;            //object name of folder
   CHAR         szFolderName[MAX_FILESPEC]; //name of folder without extension
   OBJNAME      szFolderProp;               //name of property file for folder
   OBJNAME      szDocObjName;               // object name of document
   HPROP        hpropFolder;                //handle of folder properties
   HWND         hwndAnalyzeLb;              //handle of analyze listbox
   HWND         hwndMdbAvaLb;               //handle of available mdbs LB
   HWND         hwndMdbSelLb;               //handle of selected mdbs LB
   CHAR         szDicPath[_MAX_PATH];       //path to dictionaries
   CHAR         szMemFullName[MAX_EQF_PATH];//full specification of memory db
   CHAR         szString[4096];             //dummy string
   PTAINPUT     pTAI;                       //pointer to analysis interface str
   PSZ          pszAct;                     //actual pointer to data buffer
   PSZ          pszStart;                   //start of new offset
   PANALYSIS_IDA pA;                        //pointer to dialog interface
   CHAR         szEqfPath[MAX_EQF_PATH];    //work buffer for an EQF path
   CHAR         szMemProp[1024];            //buffer for return of mdb handler
   BOOL         fDisableAddExactMatchesStuff; //disbale TM stuff on dialog
   BOOL         fDisableAddNoMatchStuff;   //disbale TM stuff on dialog
   BOOL         fDisableTMStuff;            //disable add exact ... on dialog

// Analysis(2) dialog
   HWND         hwndDicAvaLb;               //handle of available dictionary LB
   HWND         hwndDicSelLb;               //handle of selected dictionary LB
   BOOL         fAddTermsToDicCb;           //state of add terms to dict. CB
   CHAR         szAddTermsToDicDd[MAX_LONGFILESPEC];//add terms to dict. dropdown LB
   BOOL         fCreateNewTermsCb;          //state of create new terms  CB
   CHAR         szCreateNewTermsDd[MAX_FILESPEC];//create new terms dropdown LB
   CHAR         szCreateNewMWTermsDd[MAX_FILESPEC];//create new multi-terms dropdown LB
   BOOL         fCreateFndTermsCb;          //state of create found terms  CB
   CHAR         szCreateFndTermsDd[MAX_FILESPEC];//create found terms dropdown LB
   BOOL         fExTermsInListCb;           //state of excl. terms in list CB
   CHAR         szExTermsInListDd[MAX_FILESPEC];//excl terms in list dropdown LB
   BOOL         fExTermsInDicCb;            //state of excl. terms in dic. CB
   CHAR         szExTermsInDicDd[MAX_LONGFILESPEC];//excl terms in dic. dropdown LB

   USHORT       usNTLNumOccurences;         // NTL min. Num. of occurences EF
   BOOL         fNTLMwtermCb;               // NTL with multi-term info CB
   BOOL         fNTLcontextCb;              // NTL with context info CB
   USHORT       usFTLNumOccurences;         // FTL min. Num. of occurences EF
   BOOL         fFTLcontextCb;              // found terms with context info CB
   BOOL         fLSCalled;                  // List setting DLG called or not
   PFNWP        pfnEF;                      // window procedure for entryfields
   INDICT       Dict;                       // array of selected dictionaries#
   CHAR         szBuffer;                   // Buffer for format
   USHORT       usMWTOption;                // selected Options
   HWND         hwndPages[20];              // array with property pages
   // data exchange areas for the individual pages of the analysis dialog
   //
   BOOL         fListCreation;              // create lists flag
   BOOL         fAutSubst;                  // automatic substitution flag
   BOOL         fMatchCount;                // count match data flag
   BOOL         fRedundancyCounting;        // redundancy counting flag
   BOOL         fSNOMATCH;                  // SNOMATCH output flag
   //
   BOOL         fExactContext;              // exact context flag
   BOOL         fLatestMatch;               // use lates match flag
   BOOL         fAutoJoin;                  // auto join flag
   BOOL         fIgnoreIdentical;           // ignore identical flag
   BOOL         fIgnorePath;                // ignore path flag
   BOOL         fIgnoreComment;             // ignore proposals with comments
   //
// BOOL         fNTLList;                   // new terms list (NTL) flag
// CHAR         szNTLListName[MAX_FNAME];   // name of NTL
// BOOL         fNTLContext;                // NTL context flag
// USHORT       usNTLMinOccurences;         // NTL minimum occurences
// BOOL         fFTLList;                   // new terms list (NTL) flag
// CHAR         szFTLListName[MAX_FNAME];   // name of NTL
// BOOL         fFTLContext;                // NTL context flag
// USHORT       usFTLMinOccurences;         // NTL minimum occurences
// BOOL         fAddToDict;                 // add found terms to dictionary flag
// CHAR         szAddToDict[MAX_FNAME];     // name of dictionary
// CHAR         szSelectedDict[MAX_FNAME][NUM_OF_FOLDER_DICS];  // names of search dictionaries
// BOOL         fUseExclusionList;          // use exclusion list flag
// CHAR         szExclListName[MAX_FNAME];  // name of exclusion list
// BOOL         fUseExclusionDict;          // use exclusion dictionary flag
// CHAR         szExclDictName[MAX_FNAME];  // name of exclusion dictionary
   //
   BOOL         fAddToMem;                  // add to TM flag
   CHAR         szSelectedMems[SAVE_MDB_NUM][MAX_LONGFILESPEC];  // names of selected TMs
   HWND         hwndTabCtrl;                // handle of tab control
   BOOL         fLeadingWS;                 // setting of leading WS handling flag
   BOOL         fTrailingWS;                // setting of leading WS handling flag
   OBJNAME      szParentObjName;            // object name of parent (sub)folder
   EQF_BOOL     fRespectCRLF;               // respect CRLF flag
   EQF_BOOL     fNoAddBlank;                // no additional blanks at segment end
   EQF_BOOL     fMachTrans;                 // state of machine translation checkbox
   EQF_BOOL     fMatchList;                 // state of create match list checkbox
   EQF_BOOL     fBalanceList;               // state of create balance list checkbox

   EQF_BOOL     fUseReplacementList;        // use given replacement list  
   CHAR         szReplacementList[MAX_LONGFILESPEC]; // replacement list name
   CHAR         szProfile[MAX_LONGFILESPEC]; // profile name
   BOOL         fProfile;                    // use profile flag
   EQF_BOOL     fProtXmp;                    // protext xmp sections
   EQF_BOOL     fStopAtFirstExact;           // stop at first exact match flag
   EQF_BOOL     fProtMsgNum;                 // protect MsgNum sections flag
   EQF_BOOL     fProtMeta;                   // protect Meta sections flag
   EQF_BOOL     fProtScreen;                 // protect Screen sections flag
   EQF_BOOL     fProtCodeBlock;              // protect CodeBlock sections flag
};
typedef struct _aida AIDA, *PAIDA;
/*
+------------------------------------------------------------------------------+
|  MACROS                                                                      |      //
+------------------------------------------------------------------------------+
*/
#define GETDICTIONARYSPEC( pAIda )                \
{                                                 \
    UtlMakeEQFPath(pAIda->szDicPath, NULC, DIC_PATH, NULL);   \
      strcat( pAIda->szDicPath, "\\*.*" );        \
}
#define GETEQFLISTPATH(path)                      \
{                                                 \
    UtlMakeEQFPath(path, NULC, LIST_PATH, NULL);  \
}

#define GETEQFDICTPATH(path)                      \
{                                                 \
    UtlMakeEQFPath(path, NULC, DIC_PATH, NULL);   \
}

#define GETEQFTABLEPATH(path)                     \
{                                                 \
    UtlMakeEQFPath(path, NULC, TABLE_PATH, NULL); \
}

#define CATSYSPATHWITHDIR( pszPath, szDir )        \
{                                                 \
  strcpy( pszPath, ppropSystem->PropHead.szPath ); \
  strcat( pszPath, "\\" );                         \
  strcat( pszPath, szDir );                        \
}


 // following lines changed by @JOR
/*@JOR*/ #define TA_TAG_NOP       ":qfn"
/*@JOR*/ #define TA_TAG_ENOP      ":eqfn"
/*@JOR*/ #define TA_TAG_XLATED    ":qfx"
/*@JOR*/ #define TA_TAG_EXLATED   ":eqfx"
/*@JOR*/ #define TA_TAG_TOBE      ":qff"
/*@JOR*/ #define TA_TAG_ETOBE     ":eqff"
/*@JOR*/ #define TA_TAG_ATTR      ":qfa"
/*@JOR*/ #define TA_TAG_EATTR     ":eqfa"
/*@JOR*/ #define TA_TAG_CURRENT   ":qfc"
/*@JOR*/ #define TA_TAG_ECURRENT  ":eqfc"
/*@JOR*/ #define TA_TAG_JOINED    ":qfj"
/*@JOR*/ #define TA_TAG_EJOINED   ":eqfj"
/*@JOR*/ #define TA_TAG_SPLIT     ":qfj"
/*@JOR*/ #define TA_TAG_ESPLIT    ":eqfj"

/*---------------------------------------------------------------------------*/
/* structures                                                                */
/*---------------------------------------------------------------------------*/
typedef struct _UNB_LINK
   {
   PVOID   p[3];                              /* left, right & parent ptrs */
   } UNB_LINK, *PUNB_LINK;

typedef struct _TANODE
   {
   CHAR    *key;
   UNB_LINK link;
   } TANODE, *PTANODE;

typedef USHORT (*PFN_CMP)(PTANODE, PTANODE);
typedef BOOL   (*PFN_FREE)(PTANODE);
typedef EQF_BOOL (*PFN_ITERATE)(PTANODE, PVOID);

typedef struct _UNB_TREE
    {
    PTANODE          root;                          /* root node of tree       */
    USHORT         link_offset;                   /* offset of link in node  */
    ULONG          ulCount;                       /* No of nodes in tree     */
    PFN_CMP        cmp_fcn;                       /* user comparison func    */
    PFN_FREE       free_fcn;                      /* free node               */
    } UNB_TREE, *PUNB_TREE;

/*---------------------------------------------------------------------------*/
/* defines                                                                   */
/*---------------------------------------------------------------------------*/
/* macro to return a pointer to the link field of a node*/
#define NODE_TO_LINK(t,n)   ((UNB_LINK *)((CHAR *)(n) + (t)->link_offset))

#define LEFT           0    // index to the left side of the tree
#define RIGHT          1    // index to the right side of the tree
#define PARENT         2    // index to the parent pointer of the node




#define WORK_AREA_SIZE  32000L          // length of work area
#define DICT_INIT_SIZE  2000L           // length of dict work area
#define MAX_DISPL_SEG   50              // max l of a segment for a message
#define TERM_AREA_SIZE  32000L          // max term area size
#define MAX_WORD_TERM   20              // max # of word in MWT
#define ELLIPSIS        "..."           // ellipsis in strings
#define ELLIPSISW        L"..."

// this structure is used to pass the data of a context
typedef struct _LPSEG
   {
   PSZ    pszFileName;
   ULONG  ulPosition;
   USHORT usLength;
   } LPSEG, *PLPSEG;

/*---- this structure is used to store the context data for a term -----------*/
/*---- every term has a pointer to a linked list of such structures ----------*/
typedef struct _LPCTXT
   {
   LPSEG      context;                   /* context data structure           */
   struct _LPCTXT  *pNext;               /* pointer to next context for term */
   } LPCTXT, * PLPCTXT;

/*---- this structure is used to store data about the already output       ---*/
/*---- contexts; it is used to output a reference to a context instead     ---*/
/*---- of the complete context again.                                      ---*/
/*---- the local data area for list processing, LPDATA has a pointer to a  ---*/
/*---- linked list of such structures  ---------------------------------------*/
typedef struct _LPCTXTREF
   {
   LPSEG      context;                   /* context data structure            */
   USHORT     usRef;                      /* reference number of the context  */
   UNB_LINK   link;                      /* link for UNB_TREE functions      */
   } LPCTXTREF, * PLPCTXTREF;

/*----- this is an element of the Found terms list ---------------------------*/
/*----- these elements are kept in an unbalanced tree; special tree functions */
/*----- are used to access them ----------------------------------------------*/
typedef struct _LPFENTRY
   {
   PSZ_W      pszTerm;                    /* lemma in found term list         */
   HDCB       hDict;                      /* dictionary handle                */
   PSZ_W      pszTranslations;            /* translations of term (multip.str.*/
   USHORT     usFrequency;                /* frequency of term                */
   PLPCTXT    pContext;                   /* pointer to first Source context  */
   UNB_LINK   link;                       /* link pointer for UNB tree        */
   } LPFENTRY, *PLPFENTRY;

/*----- this is an element of the NEW   terms list ---------------------------*/
/*----- also these elements are kept in an unbalanced tree; this tree is a    */
/*----- different one from the Found term list: the content is sligltly       */
/*----- different ------------------------------------------------------------*/
typedef struct _LPNENTRY
   {
   PSZ_W      pszTerm;                    /* lemma in new terms list          */
   USHORT     usFrequency;                /* frequency of term                */
   PLPCTXT    pContext;                   /* pointer to first context         */
   UNB_LINK   link;                       /* link pointer for UNB tree        */
   } LPNENTRY, *PLPNENTRY;

/*---- exclusion list strucutre  ---------------------------------------------*/
typedef struct _LPEXCLLIST
   {
   USHORT        uLength;                  /* length of exclusion list in byte  */
   USHORT      usNumEntries;             /* number of entries in list         */
   USHORT        uFirstEntry;              /* pointer to first entry (offset)   */
   USHORT        uStrings;                 /* offset to noise strings           */
   } LPEXCLLIST, *PLPEXCLLIST;

// current type of list being processed
typedef enum _LPTYPE
   {
   NEW_LIST,
   FOUND_LIST
   } LPTYPE;

// this defines a chunk of memory to suballocate for terms
typedef struct _LPTERM_AREA
   {
   struct _LPTERM_AREA *pLink;    // ptr to next chunk
   CHAR_W   *pNext;              // next available position
   CHAR_W   Block [TERM_AREA_SIZE];
   } LPTERM_AREA, *PLPTERM_AREA;

// this defines the element of the linked list of open files
typedef struct _LPFCB
   {
   PSZ       pszFileName;                // pt to file name
   HFILE     hFile;                      // file handle
   struct _LPFCB *pNext;               // pointer to next elem in list
   } LPFCB, *PLPFCB;



/*-------- Term list processing local data area --------------------------*/
typedef struct _LPDATA
   {
   HUCB          hUCB;                   // handle of ASD control block
   USHORT        usUserHandle;           // handle as returned by NlpBegin
   USHORT        usServiceHandle;        // h. returned by NlpBeginService
   HDCB          hSearchDict;            // handle to search dict assoc.
   USHORT        ausMorphHandle[2];      // array of h. to morphological dict.
   HDCB          hOutDict;               // handle to output dict assoc.
   HDCB          hExclDict;              // handle to exclu. dict assoc.
   PLPEXCLLIST   pExclList;              // pointer to excl list.
   PUNB_TREE     pFTL;                   // pointer to Found Terms tree
   PUNB_TREE     pNTL;                   // pointer to New Terms tree
   PTAGTABLE     pTagDef;                // pt to Output tags table
   LPNENTRY      NEntry;                 // work entry in NTL
   LPFENTRY      FEntry;                 // work ENtry if FTL
   PBUFCB        pCBList;                // pt to IOCB for terms list output
   LPTYPE        ListType;               // type of list output (New, Found)
   PUNB_TREE     pContextRefs;           // ptr. to context references tree
// PLPCTXTREF    pCntxtLst;              // pt to list of contexts
// PLPCTXTREF    pCurCntxt;              // pt to current (last)  context
   UCHAR         uchWorkArea [WORK_AREA_SIZE];  // workarea for Cplx tkn
   ULONG         ulWorkSize;             // size of the work area in bytes
   PVOID         pToken;                 // token ptr for Cplx tkn
   TOKENENTRY    TokenList [MAX_ALLOC_TOKENS]; // token list for markup lng.
   CHAR_W        szWorkSegment [MAX_SEGMENT_SIZE];  //  work segment area
   UCHAR         szTerm [MAX_TERM_LEN * 2 + 1];  // buffer to hold a term
   CHAR_W        szStem [MAX_TERM_LEN * 2 + 1];  // buffer for a search term
   CHAR_W        szFnd  [MAX_TERM_LEN * 2 + 1];  // buffer for found term
   UCHAR         szPrBuf[MAX_TERM_LEN * 2 + 31];  // print buffer
   PLPTERM_AREA  pTermArea;              // pointer to storage chunks for Terms
   PLPFCB        pLstFCB;                // pointer to list of open files CB
   UCHAR         szSSourcePath[MAX_EQF_PATH]; // Source files path name
   PUCHAR        pchMatches;                // ptr to work area for MWT Matches
   USHORT        usMatchesSize;             // size of the buffer
   USHORT        usMWTMaxLength;            // max n. of words in MWT
   UCHAR         szMWT  [MAX_TERM_LEN + 1]; // buffer for MWT matches
   USHORT        usIMWTLen[MAX_WORD_TERM];  // intermediate MWT string lenght
   PVOID         pITK[MAX_WORD_TERM];       // intermediate Token pointers
   PSZ_W         puchTermBuf;               // ptr to dictionary output area
   ULONG         ulTermBufSize;             // size of the dictionary out area
   USHORT        usMergeFlags;              // merge flags for out dict.
   CHAR_W        szLastSegment [MAX_SEGMENT_SIZE];  //  Last work segment area
   PUSHORT       pusSortBuffer;             // Used to sort Context references
   USHORT        usSortBufferSize;          // Current size of buffer
   USHORT        usReferencesInBuffer;      // No of References in the buffer
   ULONG         ulTotalTerms;              // No. of terms to be output
   ULONG         ulCurrentTerm;             // Current term
   BOOL          fStem;                                    // stem form exist and is different
   SHORT         sXXLangID;                   // language ID of active language
   PSZ_W         pStemTermList;             // pointer to stem term list
   USHORT        usStemTermListSize;        // current size of stem term list
   PSZ_W         pTermList;                 // pointer to general term list
   ULONG         ulTermListSize;            // current size of general term list
   PSZ_W         puchTermArea;              // ptr to term output area
   ULONG         ulTermAreaSize;            // size of the term out area
   PLOADEDTABLE  pListFormatTable;          // LISTFORM.IBL as loadedtable
   ULONG         ulSrcOemCP;                // ASCII CP of source language of terms
   } LPDATA, *PLPDATA;


// structure containing the pointers to main data area of the application
// (used for tree visiting functions to access the application data)
typedef struct _LPPROCDATA
   {
   HWND  hwnd;
   PTAINPUT pTAInput;
   PVOID pInD;
   PLPDATA pLP;
   } LPPROCDATA, *PLPPROCDATA;

// termination reason used by UtlSysCheck
enum TERM_REASON
   {
   TERM_NONE,
   TERM_SYSTEM,
   TERM_USER
   };


/*
+------------------------------------------------------------------------------+
| LPInit                                                                       |
| purpose   : Initialize areas and pointers for term list processing           |
|                                                                              |
| Allocates memory for LPDATA structure, Allocates area for Asd services       |
| and initialize them, open the of dictionaries                                |
| initialize the FTL and NTL tables                                            |
|                                                                              |
|                                                                              |
| Output    : the LPDATA structure is allocated and initialized                |
|             returns TRUE if successful.                                      |
|                                                                              |
|                                                                              |
+------------------------------------------------------------------------------+
*/
BOOL   LPInit (
              PTAINPUT  pTAInput, // pointer to input parameters struct.
              PLPDATA  *ppLPDATA  // pt to pt to Local List processing data
              );

/*
+------------------------------------------------------------------------------+
| LPTerminate                                                                  |
| purpose   : Close all dictionaries, free allocated resources, free local data|
|                                                                              |
| the resources are freed in the reverse order in which they were allocated    |
|                                                                              |
|                                                                              |
|                                                                              |
| Output    : the LPDATA structure is freed other resources freed              |
|             returns always TRUE.                                             |
|                                                                              |
+------------------------------------------------------------------------------+
*/
BOOL   LPTerminate (
                   PLPDATA  *ppLPDATA  // pt to pt to Local List processing data
                   );
/*
+------------------------------------------------------------------------------+
| LPSegProc                                                                    |
| purpose   : Extract terms from segments; pass the terms extracted to         |
|             LPWordProc to do actual Term processing.  The data received in   |
|             input is first cleaned from all remaining markup strings; it     |
|             is then parsed with NlpComplex and the generated list of         |
|             tokens is scanned.  Token of type TEXT and without the           |
|             attribute LX_NO_DICT_LOOK (as defined in EQFDASD.H and           |
|             returned from NlpFlgToken:  this excludes punctuation and        |
|             numbers from process) are extracted and passed to LPWordProc.    |
|                                                                              |
| prerequisites : must be preceded by a successful call to LPInit              |
|                                                                              |
| Output    :                                                                  |
|             side effects: the NTL & FTL lists are updated                    |
|             returns TRUE if successful, else FALSE                           |
|                                                                              |
+------------------------------------------------------------------------------+
*/
BOOL   LPSegProc (
                 HWND     hwnd,        // window h
                 PLPDATA  pLP,         // pt to Local List processing data
                 PTAINPUT pTAInput,    // pointer to input parameters struct.
                 PSZ_W      pszSegment,  // the segment of text to process
                 PLPSEG   pLPSeg       // segment defi struc
                 );
/*
+------------------------------------------------------------------------------+
| LPCompletion                                                                 |
| purpose   : Write the NTL and FTL tables; call the functions to output the   |
|             NTL table and then the function to output FTL.  step 1:  Scan    |
|             the tables writing the information on the disk in simple text    |
|             format:  frequency of term and term itself.                      |
|                                                                              |
|                                                                              |
|                                                                              |
| Input     :  window handle to dispatch messages .......                      |
|              pointer to the input parameters TAINPUTPARAM                    |
|               inside TAINPUTPARAM: File names of NTL, FTL                    |
|              pointer to the LPDATA structure.                                |
|               inside LPDATA: handles to NTL & FTL structures                 |
|                                                                              |
| Output    :  Side effects: the tables are written on disk.                   |
|              returns: TRUE if successful, else FALSE (output errors)         |
|                                                                              |
+------------------------------------------------------------------------------+
*/
BOOL LPCompletion (
                   HWND     hwnd,        // window handle
                   PTAINPUT pTAInput,    // ptr to Input parameter structure
                   PLPDATA  pLP          // ptr to Local data structure
                  );


/*----------------------------------------------------------------------------*\
| Name:       UtlSysCheck                                                      |
| Purpose:    allow service of messages in the message queue.                  |
|             check if, as a result of servicing the message queue             |
|             one or more conditions for termination have occurred             |
|             Such conditions are specified by the caller: they can be         |
|             1. a window handle: the routine check if the pointer associated  |
|                to the window is not NULL                                     |
|             2. a flag defined some where in the system that is set if        |
|                immediate termination is requested by the system              |
|             3. a flag defined some where in the system that is set if        |
|                termination is requested by the user. A message number        |
|                (along the standard of UtlError) can be optionally            |
|                specified; if it is, confirmation for termination             |
|                is asked to the user as a result of setting of the            |
|                user flag. The flag is then immediately reset.                |
|                                                                              |
|                                                                              |
| Parameters: 1. current window handle           (zero if not available)       |
|             2. system termination flag address (NULL if not available)       |
|             3. user termination flag address   (NULL if not available)       |
|             5. user confirmation message number(NULL if not available)       |
|             4. termination indicator adress    (must be specified)           |
|                                                                              |
| returns:    FALSE if one or more of the specified terminations reasons is    |
|             true, else TRUE. If confirmation message is specified,           |
|             user termination request is confirmed by this dialog             |
|                                                                              |
| note:       the termination flags (both user and system) must be true        |
|             if termination is requested                                      |
|                                                                              |
| Example:                                                                     |
|             the routine is designed to be used in long processing loops      |
|             to avoid to monopolise the system and allow message processing   |
|             in other windows. A processing loop must call this routine       |
|             as often as possible; as a result the user will be able to       |
|             carry on other task. The processing loop can specify the termi-  |
|             nation condition to check.   These conditions will be checked    |
|             and the routine will return FALSE if some of them is active.     |
|             The termination indicator will tell if the termination has been  |
|             requested by the system or by the user. In the latter case       |
|             the processing loop could decide to save some intermediate       |
|             result.                                                          |
|                                                                              |
|                                                                              |
|             #define ERROR_CONFIRM nn (message number for UtlError)           |
|                                                                              |
|             enum TERM_REASON usTermCode;                                     |
|             BOOL             fOK;                                            |
|             BOOL             fTerminate;                                     |
|             HWND             hwnd;                                           |
|                                                                              |
|                                                                              |
|             while (fOK)                                                      |
|                {                                                             |
|                // here application processing                                |
|                                                                              |
|                fOK = UtlSysCheck (hwnd,                                      |
|                                   NULL,               // no system flag      |
|                                   &(fKill),           // user flag           |
|                                   ERROR_CONFIRM,      // confirmation        |
|                                   &usTermCode);                              |
|                if (!fOK)                                                     |  |
|                   {                                                          |  |
|                   fTerminate = (usTermCode != TERM_NONE);                    |  |
|                   }                                                          |  |
|                }                                                             |  |
|                                                                              |
| called modules, dependencies: UtlDispatch, UtlError, WinQueryWindowPtr       |
|                                                                              |
\*----------------------------------------------------------------------------*/
BOOL UtlSysCheck       (
                       HWND    hwnd,       // instance window handle
                       PBOOL   pfSysFlag,  // system termination flag
                       PBOOL   pfUserFlag, // user termination flag
                       USHORT  usMsgNum,   // user confirmaton message
                       enum TERM_REASON *pTermination // termination code address
                       );



//
// INDENT_SPACES & MAX_INDENT is used for indenting before outputting tags
//
#define INDENT_SPACES L"                                       "
#define MAX_INDENT 40

#define DATE_BUF_LEN 12

#define USHORT_MAX_LEN 10     // Used for Buffer for a USHORT
#define RADIX_10 10           // itoa to the base 10

/*อออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
บ                  Prototypes for Exported Functions                         บ
ศอออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออ*/

/*---------------------------------------------------------------------------+
|  Name:         LPNAdd                                                      |
|  Purpose:      Maintain entry for a word in the "New" table.               |
|  Parameters:   1. PTAINPUT - pt to TAINPUT struct                          |
|                2. PLPDATA - Pointer to local data                          |
|                3. BOOL * - set to true if term is found in table           |
|                4. PSZ - The term to be updated                             |
|                5. PLPSEG - pt to segment descriptor                        |
|                6. PUSHORT - Ptr to the frequency (if found)                |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPNAdd(pTAinput, &LPData, &Found, WordPtr, &SegDescr, |
|                             &usFrequency);                                 |
+---------------------------------------------------------------------------*/
BOOL LPNAdd(PTAINPUT pTAInput, PLPDATA pLPData, BOOL *pfFound,
            PSZ_W pszTerm, PLPSEG pLPSeg, PUSHORT pusReturnFrequency);

/*---------------------------------------------------------------------------+
|  Name:         LPNCreate                                                   |
|  Purpose:      Create a new entry in the "New" table                       |
|  Parameters:   1. PTAINPUT - TAINPUT struct pointer                        |
|                2. PLPDATA - Pointer to local data                          |
|                3. PSZ - The term to be stored                              |
|                4. PLPSEG - pt to segment descriptor                        |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPNCreate(pTAInput, pLPData, WordPtr, &SegDescr);     |
+---------------------------------------------------------------------------*/
BOOL LPNCreate(PTAINPUT pTAInput, PLPDATA pLPData, PSZ_W pszTerm, PLPSEG pLPSeg);

/*---------------------------------------------------------------------------+
|  Name:         LPNCompare                                                  |
|  Purpose:      Compare two "New" node structures                           |
|  Parameters:   1. PLPNENTRY - pointer to the first node                    |
|                2. PLPNENTRY - pointer to the second node                   |
|  Returns:      SHORT - -1: node1<node2; 0: node1==node2; 1:node1>node2     |
|  Comments:                                                                 |
|  Samples:      s = LPNCompare(&node1, &node2);                             |
+---------------------------------------------------------------------------*/
SHORT LPNCompare(PLPNENTRY pNode1, PLPNENTRY pNode2);

/*---------------------------------------------------------------------------+
|  Name:         LPNFree                                                     |
|  Purpose:      Free a node of the "New" terms list (and associated memory) |
|  Parameters:   1. PLPNENTRY - pointer to the node to be freed              |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPNFree(&node);                                       |
+---------------------------------------------------------------------------*/
BOOL LPNFree(PLPNENTRY pNode);

/*---------------------------------------------------------------------------+
|  Name:         LPFAdd                                                      |
|  Purpose:      Maintain entry for a word in the "Found" table.             |
|  Parameters:   1. PTAINPUT - pt to TAINPUT struct                          |
|                2. PLPDATA - Pointer to local data                          |
|                3. BOOL * - set to true if term is found in table           |
|                4. PSZ - The term to be updated                             |
|                5. PLPSEG - pt to segment descriptor                        |
|                6. PUSHORT - Ptr to the frequency (if found)                |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:     A new element is allocated and added to the table           |
|  Samples:      fOK = LPFAdd(pTAInput, &LPData, &Found, WordPtr, &SegDescr, |
|                             &usFrequency);                                 |
+---------------------------------------------------------------------------*/
BOOL LPFAdd(PTAINPUT pTAInput, PLPDATA pLPData, BOOL *pfFound,
            PSZ_W pszTerm, PLPSEG pLPSeg, PUSHORT pusReturnFrequency);

/*---------------------------------------------------------------------------+
|  Name:         LPFCreate                                                   |
|  Purpose:      Create a new entry in the "Found" table                     |
|  Parameters:   1. PTAINPUT - TAINPUT struct                                |
|                2. PLPDATA - Pointer to local data                          |
|                3. PSZ - The term to be stored                              |
|                4. PLPSEG - pt to segment descriptor                        |
|                5. PSZ - pointer to the (adjacent strings) translations     |
|                6. HDCB - Dictionary handle for the found term              |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPFCreate(pTAInput, pLPData, WordPtr, &SegDescr,      |
|                                pszTranslation, hDictHandle);               |
+---------------------------------------------------------------------------*/
BOOL LPFCreate(PTAINPUT pTAInput, PLPDATA pLPData, PSZ_W pszTerm,
               PLPSEG pLPSeg, PSZ_W pszTranslation, HDCB hDict);

/*---------------------------------------------------------------------------+
|  Name:         LPFCompare                                                  |
|  Purpose:      Compare two "Found" node structures                         |
|  Parameters:   1. PLPFENTRY - pointer to the first node                    |
|                2. PLPFENTRY - pointer to the second node                   |
|  Returns:      SHORT - -1: node1<node2; 0: node1==node2; 1:node1>node2     |
|  Comments:                                                                 |
|  Samples:      s = LPFCompare(&fNode1, &fNode2);                           |
+---------------------------------------------------------------------------*/
SHORT LPFCompare(PLPFENTRY pfNode1, PLPFENTRY pfNode2);

/*---------------------------------------------------------------------------+
|  Name:         LPFFree                                                     |
|  Purpose:      Free a node of the "Found" terms list (& associated memory) |
|  Parameters:   1. PLPFENTRY - pointer to the node to be freed              |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPFFree(&fNode);                                      |
+---------------------------------------------------------------------------*/
BOOL LPFFree(PLPFENTRY pfNode);

/*---------------------------------------------------------------------------+
|  Name:         LPTLOutput                                                  |
|  Purpose:      Write the term list to disk                                 |
|  Parameters:   1. HWND - Window handle                                     |
|                2. PLPDATA - Pointer to LP data                             |
|                3. PTAINPUT - Input parameter struct                        |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPTLOutput(&hwnd, &lpdata ,&inputRec);                |
+---------------------------------------------------------------------------*/
BOOL LPTLOutput(HWND hwnd, PLPDATA pLPData, PTAINPUT pTAInput);

/*---------------------------------------------------------------------------+
|  Name:         LPTermSave                                                  |
|  Purpose:      Saves the term in a Term Save Area                          |
|  Parameters:   1. PTAINPUT - TAINPUT struct                                |
|                2. PLPDATA - Pointer to LPDATA                              |
|                3. PSZ * - pointer to pointer to the saved term             |
|                4. PSZ - pointer to the term being saved                    |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:     Sets fTerminate if allocate fails                           |
|  Samples:      fOK = LPTermSave(pTAInput, pLPData, &pszSavedTerm, pszTerm);|
+---------------------------------------------------------------------------*/
BOOL LPTermSave(PTAINPUT pTAInput, PLPDATA pLPData, PSZ_W * ppszSavedTerm,
                PSZ_W pszTerm);

/*---------------------------------------------------------------------------+
|  Name:         LPTermFree                                                  |
|  Purpose:      Frees the memory in the Term Save Area                      |
|  Parameters:   1. PLPDATA - Pointer to LPDATA                              |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:                                                                 |
|  Samples:      fOK = LPTermFree(pLPData);                                  |
+---------------------------------------------------------------------------*/
BOOL LPTermFree(PLPDATA pLPData);

/*---------------------------------------------------------------------------+
|  Name:         LPCloseFiles                                                |
|  Purpose:      Close all the files opened for reading contexts             |
|  Parameters:   1. PLPFCB * - ptr to lost of open files CBs                 |
|  Returns:      BOOL - True if successful, else FALSE                       |
|  Comments:                                                                 |
|  Samples:      fOK = LPCloseFiles(&(pLPData->ppLPFCB);                     |
+---------------------------------------------------------------------------*/
BOOL LPCloseFiles(PLPFCB *ppLPFCB);


/*---------------------------------------------------------------------------+
|  Name:         LPFreeContextReferences                                     |
|  Purpose:      Frees the context references created by a call to LPTLOutput|
|  Parameters:   1. PLPDATA - Pointer to LPDATA                              |
|  Returns:      BOOL - Always returns true                                  |
|  Comments:     - pCntxtLst & PCurCntxt are both NULL afterwards            |
|                - Called by LPTerminate & by LPTLOutput                     |
|  Samples:      fOK = LPFreeContextReferences(pLPData);                     |
+---------------------------------------------------------------------------*/
BOOL LPFreeContextReferences(PLPDATA pLPData);

/*---------------------------------------------------------------------------+
|  Name:         LPLoadTagTable                                              |
|  Purpose:      Loads the tag table for NTL & FTL I/O                       |
|  Parameters:   1. PLPDATA - ptr to the LPDATA area                         |
|                2. PTAINPUT - ptr to TAINPUT                                |
|  Returns:      BOOL - True if successful, False otherwise                  |
|  Comments:     Called by LPInit                                            |
|                Sets fTerminate if failure                                  |
|  Samples:      fOK = LPLoadTagTable(pLPData, pTAInput);                    |
+---------------------------------------------------------------------------*/
BOOL LPLoadTagTable(PLPDATA pLPData, PTAINPUT pTAInput);

/*---------------------------------------------------------------------------*/
/*  function prototypes                                                      */
/*---------------------------------------------------------------------------*/
UNB_TREE   *ListAlloc(
                      USHORT,            /* offset of link in node  */
                      PFN_CMP,           /* compare two nodes       */
                      PFN_FREE           /* call when node removed  */
                     );

BOOL       ListFree(
                    UNB_TREE *           /* descriptor of tree      */
                   );

BOOL       ListInsert(
                     UNB_TREE *,         /* descriptor of tree      */
                     PTANODE    *,         /* root of tree            */
                     PTANODE               /* node to be inserted     */
                     );

PTANODE      ListSearch(
                     UNB_TREE *,         /* descriptor of tree      */
                     PTANODE               /* value to find           */
                     );

BOOL       ListScan(
                   UNB_TREE  *,          /* descriptor of tree      */
                   PTANODE      ,          /* current node            */
                   PFN_ITERATE,          /* called at each node     */
                   PVOID                 /* 2nd arg of visit_fcn    */
                   );
VOID EQFTAMnl (HWND hwnd, PTAINPUT  *ppTAInput);     // input structure text analysis
BOOL LoadFile(PTAINPUT, PSZ, PVOID *, BOOL);  // load table files
VOID UpdateSliderPosition(HWND hwnd, ULONG ulLength);
USHORT TASegCreate ( HWND, PTAINPUT);        // segment process init
USHORT TASegProc( HWND, PTAINPUT, PUSHORT );           // process segm file
USHORT TASegBlockProc ( HWND, PTAINPUT );    // proc block of segm tex
USHORT TASegAutoJoin( HWND, PTAINPUT );      // autojoin processing
USHORT TASegOpenFiles (PTAINPUT);            // open input seg source output seg Target
USHORT TASegCloseFiles (PTAINPUT );          // close seg source
BOOL TASegSegmentation
(
      HWND          hwnd,                // instance window handle
      PTAINPUT      pTAInput,            // input parameter struct
      PTAINSTDATA   pInD,                // instance local data struc pt
      PSZ           *ppRest,             // pointer to rest of input
      ULONG         ulFileOffset         // offset of szTextBuffer
                                               //   from beginning of
                                               //     source file
);

//--- Analysis Handler
INT_PTR CALLBACK ANALYSISHANDLER_WP( HWND, WINMSG, WPARAM, LPARAM);
//--- Analysis Dialog Procedure
INT_PTR CALLBACK INVOKEANALYSISDLGPROC( HWND, WINMSG, WPARAM, LPARAM);
//--- Analysis Dialog Procedure 2 for list settings
INT_PTR CALLBACK ANALYSISLISTSETTINGSDLGPROC( HWND, WINMSG, WPARAM, LPARAM);

BOOL TAGiveCntrl (HWND , PTAINSTDATA ) ;           // dispatch function
BOOL TAReadBlock (PTAINPUT pTAInput);             // read next block of data
VOID UpdateFileProp (PTAINPUT  pTAInput);
BOOL SetSegDate (PTAINPUT  pTAInput, ULONG ulSegDate); // upd file prop.
VOID DelSegFiles(PTAINPUT pTAInput);
BOOL WriteSegment( PTAINPUT       pTAInput,        // uses old fill buffer
                   USHORT         usSegLength,
                   PSZ            pszSegment,
                   XLATESTATE     XlateState,
                   PBUFCB         pBuffer,
                   ULONG          ulSegNum,
                   BOOL           fFoundTM
                 );

BOOL WriteSegmentW( PTAINPUT       pTAInput,        // uses old fill buffer
                   USHORT         usSegLength,
                   PSZ_W           pszSegmentW,
                   XLATESTATE     XlateState,
                   PBUFCB         pBuffer,
                   ULONG          ulSegNum,
                   BOOL           fFoundTM
                 );

#define WRITETMSEGMENT_ASCII  1
#define WRITETMSEGMENT_ANSI   2
#define WRITETMSEGMENT_UTF16  3
#define WRITETMSEGMENT_UTF8   4

BOOL WriteTMSegment( PTAINPUT       pTAInput,
                     ULONG          ulSegLength,
                     PSZ_W            pszSegment,
                     PBUFCB         pBuffer,
                     ULONG          ulSegNum,
                     SHORT          sFormat );

BOOL TARestartList( PTAINPUT pTAInput);  // restart file lists

VOID TAUnlockFiles( PTAINPUT );             // unlock previously locked files
BOOL TALockFiles ( PTAINPUT );                   // lock files for other processes

typedef struct _MATCHTABLE
{
   PSZ                 pszFileName;          // pointer to filename of tm
   LHANDLE             htm;                  // handle to translation memory
   LONG                lSimLevel;            // similarity of match
   TM_ADDRESS          addr;                 // address of match
} MATCHTABLE, *PMATCHTABLE;


#define MAX_NUM_MATCHES 3                    // number of best matches for tm

/**********************************************************************/
/* Load user exit DLL and addresses of segmentation entry points      */
/**********************************************************************/
USHORT TALoadUserExit
(
  PSZ              pszExit,            // name of user exit DLL
  PTAINPUT         pTAInput            // ptr to analysis input data area
);

/**********************************************************************/
/* Free user exit DLL                                                 */
/**********************************************************************/
USHORT TAFreeUserExit
(
  PTAINSTDATA      pInD                // ptr to analysis instance data area
);


  USHORT TARedundCount( HWND, PTAINPUT, PUSHORT );
  int    TACompNames( const void *, const void * );
  SHORT  TADocNameToIndex( PTAINPUT, PSZ );
  SHORT  TADocNameToIndex( PTAINPUT, PSZ );
  USHORT TAProcessSegmentUseCount( PTAINPUT );
  USHORT TAWriteRedundCountFile( PTAINPUT );
  int    TACompMostUsedSeg( const void *, const void * );

  USHORT TAAddCounts( PCOUNTSUMS, LONG, LONG );

INT_PTR CALLBACK ANALYSISPROPDLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
BOOL AnaPropertySheetLoad( HWND hwndDlg, PAIDA pIda );
INT_PTR CALLBACK ANAPROP_GENERAL_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
INT_PTR CALLBACK ANAPROP_AUTSUBST_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
INT_PTR CALLBACK ANAPROP_LISTS_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
INT_PTR CALLBACK ANAPROP_OTHER_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );
MRESULT AnaPropCommand( HWND hwndDlg, WPARAM mp1, LPARAM mp2 );
MRESULT AnaPropertySheetNotification( HWND hwndDlg, WPARAM  mp1, LPARAM  mp2 );


/**********************************************************************/
/* String-IDs for Options of Multi-term lists                            */
/**********************************************************************/

#define STR_MWT_OPTION_1          "Initial Noun"
#define STR_MWT_OPTION_2          "Initial Adjective"
#define STR_MWT_OPTION_3          "Initial Noun or Adj"


BOOL TAProcessPostTM(  PTAINPUT pTAInput, PTAINSTDATA pInD, PSZ pszFileName,
                        PSZ pszSegTarget, BOOL fTMInserted );

// match types as returned by GetExactTMMatch
typedef enum _MATCHTYPE
{
  NO_MATCH, FUZZY_MATCH, EXACTMORE_MATCH, EXACTONE_MATCH, EXACTEXACT_MATCH,
  REPL_MATCH, FUZZYREPL_MATCH, MACHINE_MATCH, GLOBMEM_MATCH
} MATCHTYPE;
typedef MATCHTYPE *PMATCHTYPE;
