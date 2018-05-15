/*! EQFTAG.H
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

/**********************************************************************/
/**********************************************************************/
/*        Structures and Types for Token Lists and Tagtables          */
/**********************************************************************/
/**********************************************************************/
#define TAGTABLE_VERSION   3                   // current version of TagTable
#define TAGTABLE_VERSION3    3             // version 3 of TagTable
#define ADDINFO_VERSION      2             // additional info avail with V2.
#define FORMATID           "FORMATTABLE"   // id of formattable
#define CHAR_MULT_SUBST     '%'            // multiple substitution character
#define CHAR_SNGL_SUBST     '?'            // single substitution character
#define MAX_TRNOTE_DESC     40             // length of TRNote Desc. Prefix
#define   TRNOTE1_STRING  "Note 1:"     // default identifier for TRNOTE level 1
#define   TRNOTE2_STRING  "Note 2:"     // default identifier for TRNOTE level 2



// max size for tag text
#define   MAX_TAGSIZE   100

/* OFFSETTOPOINTER : convert offset in tagtable to pointer                    */
#define OFFSETTOPOINTER(type, offset) (type)(pByte + offset)

typedef enum _TAGTYPE          // types of tags:
{                              //
   STARTDEL_TAG,               // tag which indicates segment start
   ENDDEL_TAG,                 // tag which indicates segment end
   SELC_TAG,                   // a selfcontained tag
   STARTNEUTRAL_TAG,           // start tag which does not delimit a segment
   ENDNEUTRAL_TAG              // end tag which does not delimit a segment
} TAGTYPE;

typedef enum _SEGBREAK         // segment info of tag:
{                              //
   SEG_OFF,                    // tag sets segmenting one level deeper
   SEG_NEUTRAL,                // tag does not influence segmenting
   SEG_ON,                     // tag sets segmenting one level higher
   SEG_RESET,                  // tag resets segmentent  mode
   SEG_LINE,                    // tag starts line-per-line segmenting
   PROTECT_ON,                  // tag starts protected text
   PROTECT_OFF                  // tag ends protected text
} SEGBREAK;

typedef enum _ASSTEXT          // text which is associated with tag
{                              //
   TSL_TEXT,                   // text on same line
   TSNL_TEXT,                  // text on same or next line (BookMaster)
   NOEXPL_TEXT                 // no explicit association of text
} ASSTEXT;

typedef struct _TAG            // tag entry in tagging table:
{
  USHORT    uTagnameOffs;      // offset to tagname
  USHORT    uEndDelimOffs;     // offset to end delimiters
  USHORT    Tagtype;           // type of tag
  USHORT    Segbreak;          // segment info
  USHORT    Asstext;           // associated text
  EQF_BOOL  fAttr;             // does the tag allow attributes?
  USHORT    usLength;          // length of tag
  USHORT    usPosition;        // fixed position of tag in text
  struct
  {
     unsigned short fTranslate   : 1;   // does the tag contain text to be translated?
     unsigned short AddInfo      : 14;  // additional tag information
     unsigned short fTRNote      : 1;            // does tag contain a TRNote?
  } BitFlags;
} TAG, * PTAG;

typedef struct _TAGSTART       // possible start character of tag
{
  CHAR        chFirstchar;     // first character of tag
  USHORT      usLength;        // length of tag
  USHORT      uEndDelim;       // offset to end delimiters
} TAGSTART, * PTAGSTART;



typedef struct _STARTPOS              /* start positions of tag         */
{
  USHORT      usPosition;           /* start position of tag            */
  USHORT      usLength;             /* length of tag                    */
  USHORT      uEndDelim;            /* offset to end delimiters         */
} STARTPOS, * PSTARTPOS;

typedef struct _ATTRIBUTE      // attribute entry in attribute table:
{
  USHORT      uStringOffs;       // attributestring
  USHORT      uEndDelimOffs;     // offset to end delimiters
  struct
  {
     unsigned short fTranslate   : 1;   // does the attribute contain text to be translated?
     unsigned short AddInfo      : 14;  // additional attribute information
     unsigned short fTRNote      : 1;            // does tag contain a TRNote?
  } BitFlags;
  USHORT    usLength;          // length of attribute
} ATTRIBUTE, * PATTRIBUTE;

typedef struct _TAGLIST
{
   USHORT       uOffset;              /* offset in TagTable                     */
   USHORT       uNumber;              /* number of tags in list                 */
} TAGLIST;

typedef struct _TAGADDINFO     // additional tag/attribute info
{
  USHORT      ClassId;         // class id for tag
  USHORT      usFixedTagId;    // fixed tag if of tag
} TAGADDINFO, *PTAGADDINFO;

typedef enum _TAGCLASSID       // types of tags:
{                              //
   CLS_DEFAULT,                // default if nothing specified
   CLS_HEAD,                   // tag being header/title
   CLS_EHEAD,                  // end of header/title
   CLS_PARA,                   // tag indicating paragraph
   CLS_EPARA,                  // tag indicating end of paragraph
   CLS_XTRCT,                  // tag indicating "extraction"
   CLS_EXTRCT,                 // tag indicating end of "extraction"
   CLS_LIST,                   // list item
   CLS_ELIST,                  // end of list item
   CLS_HILIT,                  // highlight tag
   CLS_EHILIT,                 // end of highlight tag
   CLS_PROTECT,                // prevents text from being translated
   CLS_TREF,                   // reference to some part of text
   CLS_LREF,                   // link reference
   CLS_ADDINF,                 // additional info
   CLS_EADDINF,                // end of additional info
   CLS_UNCHG,                  // keep the tag untouched
   CLS_VAR,                    // variable tag
   CLS_WHITESPACE,             // whitespace tag
   CLS_TRANSLVAR               // translatable variable tag
} TAGCLASSID;

// possible values for the tagtable character set:
typedef enum _TAGTABLE_CHARSET
{
   TAGCHARSET_ASCII,                   // ASCII format for export/import
   TAGCHARSET_ANSI,                    // ANSI format for export/import
   TAGCHARSET_UNICODE,                 // Unicode format for export/import
   TAGCHARSET_UTF8                     // Unicode UTF8 format for export/import
} TAGTABLE_CHARSET;

typedef enum _TAGTABLE_REFLOW
{
	TAGREFLOW_NOTSPECIFIED,
	TAGREFLOW_YES,
	TAGREFLOW_NO
} TAGTABLE_REFLOW;

/*----------------------------------------------------------------------------*/
/* The following structure defines the internal format of a formatting table. */
/* The external SGML format will always be converted to the internal format   */
/* by a conversion routine.                                                   */
/* The internal format consists of the control structure with pointers to the */
/* different parts of the table plus the tag data, the length of which is     */
/* defined in the control structure.                                          */
/*----------------------------------------------------------------------------*/
// TagTables are e.g.: eqfbook.tbl, ... qftags.ibl, listformat.ibl...
// For Unicode enabling: During TALoadTagTable: all tags and attributes are converted
// to Unicode. This is done additionally to the old approach.
// These Unicode tags are stored in a buffer pointed to by pTagTableW.
// pTagTableW is stored in PLOADEDTABLE and must be freed when the Tagtable is freed.


typedef struct _TAGTABLE           /* Tagtable                                    */
{
  USHORT      uLength;             /* length of data following                    */
  CHAR        chId[30];            /* file id for tagtab   @@                 */
  USHORT      uNumTags;            /* number of tags                              */
  USHORT      usVersion;           // version string
  TAGLIST     stTagIndex[27];      /* offset to tags in variable tag part of table */
  TAGLIST     stAttributeIndex[27];/* offset to attributes in variable attribute */
                                   /* part of table                               */
  TAGLIST     stFixTag;          /* offset to first fixed length TAG           */
  TAGLIST     stVarStartTag;     /* offset to first tag starting with wildcard */
  TAGLIST     stAttribute;       /* offset to first attribute                  */
  USHORT        uTagstart;         /* offset to first entry in Tagstart table    */
  USHORT        uStartpos;         /* offset to first entry in Tagstart position table */
  USHORT        uAttrEndDelim;     /* offset to first attribute end delimiter   */
  USHORT      uAttrLength;       /* attribute length, not used at moment    */
  USHORT        uTagNames;         /* offset to first Tagstring                  */
  CHAR    szSegmentExit[MAX_FILESPEC];    // name of user exit dll
  CHAR    chTrnote1Text[MAX_TRNOTE_DESC]; // Indicates a note of level 1
  CHAR    chTrnote2Text[MAX_TRNOTE_DESC]; // indicates a note of level 2
  CHAR    chStartText[MAX_TRNOTE_DESC];       // tag, may be followed by TRNote
  CHAR    chEndText[MAX_TRNOTE_DESC];            // end tag
  USHORT  uAddInfos;                             // offset to additional tag infos
   // new fields contained in tagtables TAGTABLE_VERSION3 and above
   CHAR        chSingleSubst;          // character to use for single substitution
   CHAR        chMultSubst;            // character to use for multiple substitution
   CHAR        szDescriptiveName[MAX_DESCRIPTION]; // descriptive name for format (displayed in format combobox)
   CHAR        szDescription[MAX_DESCRIPTION]; // format description
   USHORT      usCharacterSet;         // characterset to use for import/export
   BOOL        fUseUnicodeForSegFile;  // TRUE = store segmented files in Unicode format
   ULONG       ulPassword;             // protection password
   ULONG       aulFree[9];             // for future use
   BOOL        fProtected;             // TRUE = markup table is protected
     /*******************************************************************/
      /* The following BOOL fReflow is a 3state BOOL:                    */
      /* 0           = not used, as previously                           */
      /* 1           = Reflow allowed                                    */
      /* 2           = no Reflow, for MRI etc                            */
      /*******************************************************************/
   BOOL        fReflow;                //status of Reflow
   BOOL        afFree[8];              // for future use
   CHAR        szFree[80];             // for future use
} TAGTABLE, *PTAGTABLE;

// structure for tag tree nodes (used in EQFBFastTokenize)
typedef struct _NODE
{
   CHAR_W   chPos;                       // character position in tag
   CHAR_W   chNode;                      // node character
   SHORT  sTokenID;                    // ID of token
   struct _NODE *pNext;                // ptr to next node (NULL = end of chain)
} NODE, *PNODE;

//typedef struct _NODE // old version!
//{
//   CHAR   chPos;                       // character position in tag
//   CHAR   chNode;                      // node character
//   SHORT  sTokenID;                    // ID of token
//   struct _NODE *pNext;                // ptr to next node (NULL = end of chain)
//} NODE, *PNODE;

// structure for tag tree nodes (used in EQFBFastTokenize) for use with UTF16 coded files
// not used currently
typedef struct _NODE16
{
   USHORT usPos;                       // character position in tag
   USHORT usNode;                      // node character
   SHORT  sTokenID;                    // ID of token
   struct _NODE *pNext;                // ptr to next node (NULL = end of chain)
} NODE16, *PNODE16;


/**********************************************************************/
/* Structures for new tokenize function                               */
/**********************************************************************/
typedef struct _TREENODE
{
   CHAR_W   chPos;                       // character position in tag
   CHAR_W   chNode;                      // node character
   SHORT    sTokenID;                    // ID of token
   PSZ_W    pszEndDel;                   // ptr to tag end delimiter
   USHORT   usLength;                    // length of length-terminated tags
   USHORT   usColPos;                    // colums position or 0
   BOOL     fAttr;                       // tag-has-attributes flag
   BOOL     fUnique;                     // this-path-is-unique flag
                                       // ( if flag is FALSE there are more
                                       // possible paths on the current level)
   BOOL   fNoMatch;                    // no match due to substitution characters
   BOOL   fTransInfo;                  // TRUE = tag may contain translatable info
   struct _TREENODE *pNextLevel;       // ptr to next node level (NULL = end of chain)
   struct _TREENODE *pNextChar;        // ptr to next character on same level
} TREENODE, *PTREENODE;

// tree node for UTF16 coded files
// not used currently: instead TREENODE has been changed accordingly
typedef struct _TREENODE16
{
   USHORT usPos;                       // character position in tag
   USHORT usNode;                      // node character
   SHORT  sTokenID;                    // ID of token
   PUSHORT pusEndDel;                  // ptr to tag end delimiter
   USHORT usLength;                    // length of length-terminated tags
   USHORT usColPos;                    // colums position or 0
   BOOL   fAttr;                       // tag-has-attributes flag
   BOOL   fUnique;                     // this-path-is-unique flag
                                       // ( if flag is FALSE there are more
                                       // possible paths on the current level)
   BOOL   fNoMatch;                    // no match due to substitution characters
   BOOL   fTransInfo;                  // TRUE = tag may contain translatable info
   struct _TREENODE *pNextLevel;       // ptr to next node level (NULL = end of chain)
   struct _TREENODE *pNextChar;        // ptr to next character on same level
} TREENODE16, *PTREENODE16;

/**********************************************************************/
/* Structure for allocated node ares                                  */
/**********************************************************************/
typedef struct _NODEAREA
{
  PTREENODE   pFreeNode;               // ptr to next free node
  USHORT      usFreeNodes;             // number of free nodes
  struct _NODEAREA *pNext;             // ptr to next node area or NULL if none
  PTREENODE   pRootNode;               // root node of node tree
  TREENODE    Nodes[1];                // array of nodes
} NODEAREA, *PNODEAREA;

// structure for loaded tag tables
typedef struct _LOADEDTABLE
{
   CHAR        szName[MAX_FNAME];      // tag table name (w/o path and ext.)
   SHORT       sUseCount;              // number of active table users
   PTAGTABLE   pTagTable;              // pointer to tag table in memory
   PNODE       pTagTree;               // Tag tree of table
   PNODE       pAttrTree;              // attribute tree of table
   PNODEAREA   pNodeArea;              // area for node trees
   PNODEAREA   pAttrNodeArea;          // area for attribute node tree
   HMODULE     hmodUserExit;           // user exit (if any)
   BOOL        fUserExitLoadFailed;    // TRUE = load of user exit failed
   PFN         pfnProtTable;           // creat eprotect table function
   BOOL        fProtTableLoadFailed;   // TRUE = ProtTable function load failed
   // new fields contained in tagtables TAGTABLE_VERSION3 and above
   // these fields are copied from the TAGTABLE structure (if available) or filled
   // with default values (for older tagtables)
   CHAR        chSingleSubst;          // character to use for single substitution
   CHAR        chMultSubst;            // character to use for multiple substitution
   CHAR        szDescriptiveName[MAX_DESCRIPTION]; // descriptive name for format (displayed in format combobox)
   CHAR        szDescription[MAX_DESCRIPTION]; // format description
   USHORT      usCharacterSet;         // characterset to use for import/export
   BOOL        fUseUnicodeForSegFile;  // TRUE = store segmented files in Unicode format
   PSZ_W       pTagNamesW;
   PSZ_W       pAttributesW;
   PVOID       pTagTableW;
   PFN         pfnCompareContext;      // compare context info function
   BOOL        fCompareContextLoadFailed;  // TRUE = CompareContext function load failed
   PFN         pfnProtTableW;          // create protect table function (Unicode)
   PFNGETSEGCONTTEXT         pfnGetSegContext;       // get segment context function (Unicode)
   BOOL        fGetSegContextLoadFailed;  // TRUE = GetSegContext function load failed
   BOOL        fReflow;                   // 0=notspec. 1= Reflow allowed, 2= no Reflow
   PFN         pfnQueryExitInfo;        // query exit info function
   BOOL        fQueryExitInfoLoadFailed;// TRUE = query exit info function load failed
   CHAR_W      chMultSubstW;			// multiple substitution
   CHAR_W      chSingleSubstW;          // single substitution
   CHAR_W      chTrnote1TextW[MAX_TRNOTE_DESC]; // TRNotesText1
   CHAR_W      chTrnote2TextW[MAX_TRNOTE_DESC]; // TRNotesText2
   ULONG       ulTRNote1Len;			// length of TRNotesText1
   ULONG       ulTRNote2Len;			// lenght of TRNotesText2
   // timestamp when tag table was loaded the last time
   ULONG       ulTimeStamp;
} LOADEDTABLE, *PLOADEDTABLE;

/**********************************************************************/
/* Error codes returned by EQFB... functions                          */
/**********************************************************************/
#if !defined(ERR_NOMEMORY)
 #define ERR_NOMEMORY   -1
#endif
#if !defined(ERR_READFILE)
 #define ERR_READFILE   -3
#endif

__declspec(dllexport)
VOID   TAFastTokenize( PSZ, PLOADEDTABLE, BOOL, PSZ *, USHORT *, TOKENENTRY *, USHORT, ULONG );
__declspec(dllexport)
VOID   TAFastTokenizeW( PSZ_W, PLOADEDTABLE, BOOL, PSZ_W *, USHORT *, TOKENENTRY *, USHORT );
__declspec(dllexport)
BOOL ContainsClassID( PLOADEDTABLE pLoadedTable, USHORT usClassID );

VOID EQFTagTokenize(PSZ, TAGTABLE *, BOOL, PSZ *, USHORT *, TOKENENTRY *, USHORT);


__declspec(dllexport)
BOOL TATagTokenize (PSZ, PLOADEDTABLE, BOOL, PSZ *, USHORT *, TOKENENTRY *, USHORT, ULONG);
__declspec(dllexport)
BOOL TATagTokenizeW(PSZ_W, PLOADEDTABLE, BOOL, PSZ_W *, USHORT *, TOKENENTRY *, USHORT);


/**********************************************************************/
/* Load a tag table or access a loaded tag table                      */
/**********************************************************************/
__declspec(dllexport)
USHORT TALoadTagTable                  // loads/accesses a tag table
(
  PSZ              pszTableName,       // name of tag table (w/o path and ext.)
  PLOADEDTABLE     *ppTagTable,        // address of caller's tag table pointer
  BOOL             fInternal,          // internal-table flag
  BOOL             fMsg                // show-error-messages flag
);
__declspec(dllexport)
USHORT TALoadTagTableHwnd              // loads/accesses a tag table
(
  PSZ              pszTableName,       // name of tag table (w/o path and ext.)
  PLOADEDTABLE     *ppTagTable,        // address of caller's tag table pointer
  BOOL             fInternal,          // internal-table flag
  BOOL             fMsg,               // show-error-messages flag
  HWND             hwnd                // owner handle for error messages
);
/**********************************************************************/
/* Load a tag table and user exit + function addresses                */
/* The following defines control the loaded stuff:                    */
/*   TALOADUSEREXIT       Load user exit if available                 */
/*   TALOADPROTTABLEFUNC  Load protext table function exit (if any)   */
/*                        When set TaLoadTagTable tries to load the   */
/*                        pfnProtTableW entry point as well           */
/*   TALOADCOMPCONTEXTFUNC  Load compare context function (if any)    */
/*   TALOADGETSEGCONTEXTFUNC  Load get context function (if any)      */
/**********************************************************************/
// load user exit if available
#define TALOADUSEREXIT      0x00000001L

// load protect table exit if available
#define TALOADPROTTABLEFUNC 0x00000010L

// Load compare context function (if any)
#define TALOADCOMPCONTEXTFUNC 0x00000020L

// Load get context function (if any)
#define TALOADGETSEGCONTEXTFUNC 0x00000040L

// Load query exit info function (if any)
#define TALOADQUERYEXITINFOFUNC 0x00000080L

__declspec(dllexport)
USHORT TALoadTagTableExHwnd            // loads/accesses a tag table
(
  PSZ              pszTableName,       // name of tag table (w/o path and ext.)
  PLOADEDTABLE     *ppTagTable,        // address of caller's tag table pointer
  BOOL             fInternal,          // internal-table flag
  ULONG            ulFlags,            // option flags
  BOOL             fMsg,               // show-error-messages flag
  HWND             hwnd                // owner handle for error messages
);


/**********************************************************************/
/* Free a tag table logically                                         */
/**********************************************************************/
__declspec(dllexport) USHORT TAFreeTagTable                  // frees a tag table
(
  PLOADEDTABLE     pTagTable           // tag table pointer returned by TALoadTagTable
);

/**********************************************************************/
/* Invalidate a loaded tag table ==> indicate that the tag table      */
/* has to be reloaded if another user wants to access the tag table   */
/**********************************************************************/
__declspec(dllexport) USHORT TAInvalidateTagTable
(
  PSZ  pszTableName
);

// load user exit not tried yet and user exit is avialable
__declspec(dllexport) USHORT TALoadExitIfAvailable( PLOADEDTABLE pTable );
// load user exit protect function if not tried yet and user exit is avialable
__declspec(dllexport) USHORT TALoadProtectFunctionIfAvailable( PLOADEDTABLE pTable );


#define MAX_FORMAT    80               // max lenght of a symbolic format name
#define MAX_FILTER    80               // max lenght of a format filter

USHORT MULoadFormatNames( HWND hwndLB );
VOID MUSetFormatText( HWND hwnd, PSZ pszMarkup );
SHORT MUQueryFormatText( HWND hwnd, PSZ pszMarkup );
SHORT MUSearchSelectFormat( HWND hwnd, PSZ pszMarkup );
PSZ MUGetFormatFilters( void );
BOOL MUDetectFormat( PSZ pszDoc, PSZ pszFormat );


#define TAG_ITM_IGNORE  1              // ignore NOP during ITM
#define TAG_ITM_PART    2              // use fixed part of NOP only in ITM
#define TAG_ITM_STARTX  4              // NOP starts  crossed-out area
#define TAG_ITM_ENDX    8              // NOP ends crossed-out area
#define TAG_ITM_EQUALNUM 16            // becomes anchor if occurs equally
#define TAG_ITM_RELEVANT   32          // inline tag eligible as anchor


__declspec(dllexport)
SHORT TagInsEscapeChars( PSZ pszIn, PSZ pszOut );

BOOL TALoadExternalMarkupTableList
(
  PSZ         pszListName,             // name of list file (with fully qualified path )
  PSZ         *ppListArea,             // caller's list area pointer
  BOOL        fMsg                     // TRUE = show error messages
);
