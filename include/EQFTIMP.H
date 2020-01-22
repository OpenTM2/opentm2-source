/*! EQFTIMP.H     Internal header file for tag table import
	Copyright (c) 1990-2013, International Business Machines Corporation and others. All rights reserved.
*/

#define   TIMER         1              // timer id
#define   TIMEOUT       500            // timeout time in miliseconds
#define   SLIDER_INCR   101            // slider increment

#define   NUM_OF_TAGS  57              // increased from 55 for REFLOW

#define   STYLE_LEN     35             // length of a style value
#define   ENDDELIM      35             // length of end delimiters
#define   MAX_STACK     35             // deepness of stack
#define   MAX_TAGS      (NUM_OF_TAGS+2)// number of tags
#define   MAX_SIZE      64000L         // size of string buffer
#define   MAXPARMS      20             // number of maximum parameters
#define   MAXDELIMIT    50             // number of delimiters
#define   MEMBLOCKSIZE  (0xFF00/4)
#define   STRINGLEN     40             // length of static text in std. window
#define   BUFFERLEN     256            // max len of work buffer and head term

typedef enum _TAGMODE
{
   FORMATTAG,                          // format table description
   LISTTAG,                            // list table
   DICTTAG                             // dictionary exchange tags
} TAGMODE;

typedef enum _TAGENUM
{
  FIRST_TOKEN,                         // mark the start
  ASSTEXT_TOKEN,                       // associated text token
  EASSTEXT_TOKEN,                      // endassociated text token
  TAG_TOKEN,                           // tag start
  ETAG_TOKEN,                          // tag end
  STRING_TOKEN,                        // text
  ESTRING_TOKEN,                       // end of text
  SEGINFO_TOKEN,                       // segment info
  ESEGINFO_TOKEN,                      // end segment info
  SEGEXIT_TOKEN,                       // segment user exit
  ESEGEXIT_TOKEN,                      // end segment user exit
  TYPE_TOKEN,                          // type specified
  ETYPE_TOKEN,                         // end of type token
  TAGTABLE_TOKEN,                      // segment break
  ETAGTABLE_TOKEN,                     // end segment break
  ATTRIBUTE_TOKEN,                     // attribute
  EATTRIBUTE_TOKEN,                    // end of attribute
  COLUMPOS_TOKEN,                      // column position specified
  ECOLUMPOS_TOKEN,                     // end column position specified
  ENDDELIM_TOKEN,                      // end delimiter
  EENDDELIM_TOKEN,                     // end of end delimiter
  TRANSINFO_TOKEN,                     // translate info
  ETRANSINFO_TOKEN,                    // end translate info
  ATTRINFO_TOKEN,                      // attribute info
  EATTRINFO_TOKEN,                     // end attribute info
  LENGTH_TOKEN,                        // length specified
  ELENGTH_TOKEN,                       // end length specified
  COMMENT_TOKEN,                       // this marks a comment
  ECOMMENT_TOKEN,                      // end of a comment
  ADDINFO_TOKEN,                       //
  EADDINFO_TOKEN,                      //
  COPYRIGTH_TOKEN,
  CREATDATE_TOKEN,
  DICT_TOKEN,
  EABBR_TOKEN,
  EAUTHOR_TOKEN,
  EAUTHOROFUPDATE_TOKEN,
  ECONTEXT_TOKEN,
  ECRDATE_TOKEN,
  EDEF_TOKEN,
  EHYPH_TOKEN,
  EIDIOM_TOKEN,
  ELUPDATE_TOKEN,
  EMORPH_TOKEN,
  ENTRY_TOKEN,
  EPOS_TOKEN,
  EREL_TOKEN,
  ERELCODE_TOKEN,
  ESCONTEXT_TOKEN,
  ESDEF_TOKEN,
  ESRC_TOKEN,
  ESTATCODE_TOKEN,
  ESYN_TOKEN,
  EUSAGE_TOKEN,
  EUSER_TOKEN,
  HEADER_TOKEN,
  HDTERM_TOKEN,
  HOM_TOKEN,
  MAPTABLE_TOKEN,
  SENSE_TOKEN,
  SOURCE_TOKEN,
  TABBR_TOKEN,
  TARGET_TOKEN,
  TAUTHOR_TOKEN,
  TAUTHOROFUPDATE_TOKEN,
  TCONTEXT_TOKEN,
  TCRDATE_TOKEN,
  TDEF_TOKEN,
  THYPH_TOKEN,
  TIDIOM_TOKEN,
  TLANGUAGE_TOKEN,
  TLUPDATE_TOKEN,
  TMORPH_TOKEN,
  TPOS_TOKEN,
  TREL_TOKEN,
  TRELCODE_TOKEN,
  TSCONTEXT_TOKEN,
  TSDEF_TOKEN,
  TSCR_TOKEN,
  TSTATCODE_TOKEN,
  TSUBJCODE_TOKEN,
  TSYN_TOKEN,
  TRANS_TOKEN,
  TRANSLATEINFO_TOKEN,
  TUSAGE_TOKEN,
  TUSER_TOKEN,
  TRNOTE_TOKEN,                        //
  ETRNOTE_TOKEN,                       //
  TRTAGSTART_TOKEN,                         //
  ETRTAGSTART_TOKEN,                        //
  TRTAGEND_TOKEN,                         //
  ETRTAGEND_TOKEN,                        //
  TRPREF_TOKEN,                        //
  ETRPREF_TOKEN,                       //
  CLASSID_TOKEN,                    // token to specify class ids
  ECLASSID_TOKEN,                   // token to specify eclass id
  TOKENID_TOKEN,                    // token to specify fixed token ids
  ETOKENID_TOKEN,                   // token to specify fixed token id

  DESCRIPTION_TOKEN,
  EDESCRIPTION_TOKEN,
  DESCRNAME_TOKEN,
  EDESCRNAME_TOKEN,
  CHARSET_TOKEN,
  ECHARSET_TOKEN,
  SINGLESUBST_TOKEN,
  ESINGLESUBST_TOKEN,
  MULTSUBST_TOKEN,
  EMULTSUBST_TOKEN,
  USEUNICODE_TOKEN,
  EUSEUNICODE_TOKEN,
  REFLOW_TOKEN,
  EREFLOW_TOKEN,

  LAST_TAG
} TAGENUM; /* end of enum */

typedef enum _STYLE
{
   YES_STYLE,                          // YES
   NO_STYLE,                           // NO
   TSNL_STYLE,                         // text on same or next
   TSL_STYLE,                          // text on same line
   NOEXPL_STYLE,                       // no text associated
   SEGOFF_STYLE,                       // set segmenting off
   SEGON_STYLE,                        // set segmenting on
   SEGNEUTRAL_STYLE,                   // segment neutral
   SEGRESET_STYLE,                     // segment reset
   STDEL_STYLE,                        // start segment
   ENDDEL_STYLE,                       // end segment
   STNEUTRAL_STYLE,                    // neutral tag
   ENDNEUTRAL_STYLE,                   // end neutral
   SELFC_STYLE,                        // self contained tag
   SEGLINE_STYLE,                      // line by line segmentation
   CLS_DEFAULT_STYLE,                  // default if nothing specified
   CLS_HEAD_STYLE,                     // tag being header/title
   CLS_EHEAD_STYLE,                    // tag being end of title
   CLS_PARA_STYLE,                     // tag indicating paragraph
   CLS_EPARA_STYLE,                    // tag indicating end of paragraph
   CLS_XTRCT_STYLE,                    // tag indicating "extraction"
   CLS_EXTRCT_STYLE,                   // tag indicating end of "extraction"
   CLS_LIST_STYLE,                     // list item
   CLS_ELIST_STYLE,                    // end of list item
   CLS_HILIT_STYLE,                    // highlight tag
   CLS_EHILIT_STYLE,                   // end highlight tag
   CLS_PROTECT_STYLE,                  // prevents text from being translated
   CLS_TREF_STYLE,                     // reference to some part of text
   CLS_LREF_STYLE,                     // link reference
   CLS_ADDINF_STYLE,                   // additional info
   CLS_EADDINF_STYLE,                  // end of additional info
   CLS_UNCHG_STYLE,                    // keep the tag untouched
   CLS_VAR_STYLE,                      // variable tag
   CHARSET_ASCII_STYLE,                // ASCII format for export/import
   CHARSET_ANSI_STYLE,                 // ANSI format for export/import
   CHARSET_UNICODE_STYLE,              // Unicode format for export/import
   PROTECT_ON_STYLE,                   // start text protection
   PROTECT_OFF_STYLE,                  // end text protection
   CLS_WHITESPACE_STYLE,               // whitespace tag
   NOTSPECIFIED_STYLE,
   REFLOWYES_STYLE,
   REFLOWNO_STYLE,
   CLS_TRANSLVAR_STYLE,                // translatable variable tag
   CHARSET_UTF8_STYLE,                 // Unicode UTF8 format for export/import
   SEGNOSEGMENTING_STYLE,
   LAST_STYLE
} STYLE;

typedef struct _CHOICELIST
{
  STYLE Style;                         // list of styles
} CHOICELIST, * PCHOICELIST;

typedef struct _STYLEENTRY
{
   STYLE   Style;                      // enum value
   CHAR    szName[STYLE_LEN];          // string value
   SHORT   sValue;                     // value of style
} STYLEENTRY, *PSTYLEENTRY;

typedef struct _BUFFER
{
   PSZ      pStart;
   PSZ      pEnd;
   PSZ      pCurrent;
   HFILE    hTarget;
} BUFFER, *PBUFFER;

typedef struct _TAGENTRY
{
     TAGENUM sTag;                     // enum value of tag
     CHAR szName[STYLE_LEN];           // length of the tag
     USHORT usValid[MAXPARMS];
} TAGENTRY, *PTAGENTRY;

typedef struct _TAGLISTS
{
  TAGENUM TagToken;                    // list of tags
} TAGLISTS, * PTAGLISTS;

#if defined (INITVAR)
STYLEENTRY AttrStyles[] =
{
   YES_STYLE,   "YES",     1,          // YES
   NO_STYLE,     "NO",     0,          // NO
   TSNL_STYLE,   "TSNL",   TSNL_TEXT,  // text on same or next
   TSL_STYLE,    "TSL",    TSL_TEXT,   // text on same line
   NOEXPL_STYLE, "NOEXPL", NOEXPL_TEXT,// no text associated
   SEGOFF_STYLE, "SEGOFF", SEG_OFF,    // set segmenting off
   SEGON_STYLE,  "SEGON",  SEG_ON,     // set segmenting on
   SEGNEUTRAL_STYLE,"SEGNEUTRAL",SEG_NEUTRAL, // segment neutral
   SEGRESET_STYLE,"SEGRESET",SEG_RESET,// segment reset
   STDEL_STYLE,   "STDEL", STARTDEL_TAG, // start segment
   ENDDEL_STYLE,  "ENDDEL",ENDDEL_TAG, // end segment
   STNEUTRAL_STYLE,"STNEUTRAL", STARTNEUTRAL_TAG, // neutral tag
   ENDNEUTRAL_STYLE,"ENDNEUTRAL", ENDNEUTRAL_TAG, // end neutral
   SELFC_STYLE,    "SELFC", SELC_TAG,  // self contained tag
   SEGLINE_STYLE,  "SEGLINE", SEG_LINE, // line by line segmentation
   CLS_DEFAULT_STYLE, "CLS_DEFAULT", CLS_DEFAULT, // default if nothing specified
   CLS_HEAD_STYLE,    "CLS_HEAD",    CLS_HEAD,    // tag being header/title
   CLS_EHEAD_STYLE,   "CLS_EHEAD",   CLS_EHEAD,   // tag being end of header/title
   CLS_PARA_STYLE,    "CLS_PARA",    CLS_PARA,    // tag indicating paragraph
   CLS_EPARA_STYLE,   "CLS_EPARA",   CLS_EPARA,   // tag indicating end of para
   CLS_XTRCT_STYLE,   "CLS_XTRCT",   CLS_XTRCT,   // tag indicating "extraction"
   CLS_EXTRCT_STYLE,  "CLS_EXTRCT",  CLS_EXTRCT,  // tag indicating end of "extraction"
   CLS_LIST_STYLE,    "CLS_LIST",    CLS_LIST,    // list item
   CLS_ELIST_STYLE,   "CLS_ELIST",   CLS_ELIST,   // end of list item
   CLS_HILIT_STYLE,   "CLS_HILIT",   CLS_HILIT,   // highlight tag
   CLS_EHILIT_STYLE,  "CLS_EHILIT",  CLS_EHILIT,  // end highlight tag
   CLS_PROTECT_STYLE, "CLS_PROTECT", CLS_PROTECT, // prevents text from being translated
   CLS_TREF_STYLE,    "CLS_TREF",    CLS_TREF,    // reference to some part of text
   CLS_LREF_STYLE,    "CLS_LREF",    CLS_LREF,    // link reference
   CLS_ADDINF_STYLE,  "CLS_ADDINF",  CLS_ADDINF,  // additional info
   CLS_EADDINF_STYLE, "CLS_EADDINF", CLS_EADDINF, // end of additional info
   CLS_UNCHG_STYLE,   "CLS_UNCHG",   CLS_UNCHG,   // keep the tag untouched
   CLS_VAR_STYLE,     "CLS_VAR",     CLS_VAR,     // variable tag
   CHARSET_ASCII_STYLE,  "ASCII",    TAGCHARSET_ASCII, // ASCII format for export/import
   CHARSET_ANSI_STYLE,   "ANSI",     TAGCHARSET_ANSI,  // ANSI format for export/import
   CHARSET_UNICODE_STYLE,"UNICODE",   TAGCHARSET_UNICODE, // Unicode format for export/import
   PROTECT_ON_STYLE,  "PROTECTON",   PROTECT_ON,  // start text protection
   PROTECT_OFF_STYLE, "PROTECTOFF",  PROTECT_OFF, // ends text protection
   CLS_WHITESPACE_STYLE, "CLS_WHITESPACE",  CLS_WHITESPACE,  // whitespace tag
   NOTSPECIFIED_STYLE,    "  ", TAGREFLOW_NOTSPECIFIED,    // reflow not specified
   REFLOWYES_STYLE,       "YES", TAGREFLOW_YES,            // reflow allowed via markup table
   REFLOWNO_STYLE,        "NO",  TAGREFLOW_NO,             // reflow not allowed

   CLS_TRANSLVAR_STYLE, "CLS_TRANSLVAR", CLS_TRANSLVAR,  // translatable variable tag
   CHARSET_UTF8_STYLE,  "UTF8",   TAGCHARSET_UTF8, // Unicode format for export/import
   SEGNOSEGMENTING_STYLE,  "SEGNOSEGS", SEG_NOSEGMENTING, // no segmentation in text block style

   LAST_STYLE,   ""     ,  0           // last style entry
};

CHOICELIST SEGSTYLES[9] =
{
   SEGOFF_STYLE,
   SEGON_STYLE,
   SEGNEUTRAL_STYLE,
   SEGRESET_STYLE,
   SEGLINE_STYLE,
   SEGNOSEGMENTING_STYLE,
   PROTECT_ON_STYLE,
   PROTECT_OFF_STYLE,
   LAST_STYLE
};

CHOICELIST ASSTEXTSTYLES[4] =
{
   NOEXPL_STYLE,
   TSNL_STYLE,
   TSL_STYLE,
   LAST_STYLE
};

CHOICELIST YESNOSTYLES[3] =
{
   NO_STYLE,
   YES_STYLE,
   LAST_STYLE
};

CHOICELIST REFLOWYESNOSTYLES[4] =
{
   NOTSPECIFIED_STYLE,
   REFLOWYES_STYLE,
   REFLOWNO_STYLE,
   LAST_STYLE
};


CHOICELIST TYPESTYLES[6] =
{
   STDEL_STYLE,
   ENDDEL_STYLE,
   STNEUTRAL_STYLE,
   ENDNEUTRAL_STYLE,
   SELFC_STYLE,
   LAST_STYLE
};

CHOICELIST CLASSIDSTYLES[] =
{
   CLS_DEFAULT_STYLE,
   CLS_HEAD_STYLE,
   CLS_EHEAD_STYLE,
   CLS_PARA_STYLE,
   CLS_EPARA_STYLE,
   CLS_XTRCT_STYLE,
   CLS_EXTRCT_STYLE,
   CLS_LIST_STYLE,
   CLS_ELIST_STYLE,
   CLS_HILIT_STYLE,
   CLS_EHILIT_STYLE,
   CLS_PROTECT_STYLE,
   CLS_TREF_STYLE,
   CLS_LREF_STYLE,
   CLS_ADDINF_STYLE,
   CLS_EADDINF_STYLE,
   CLS_UNCHG_STYLE,
   CLS_VAR_STYLE,
   CLS_WHITESPACE_STYLE,
   CLS_TRANSLVAR_STYLE,
   LAST_STYLE
};

CHOICELIST CHARSETSTYLES[5] =
{
  CHARSET_ASCII_STYLE,
  CHARSET_ANSI_STYLE,
  CHARSET_UNICODE_STYLE,
  CHARSET_UTF8_STYLE,
  LAST_STYLE
};


/**********************************************************************/
/* following list has to be in the same order as specified in ENUM    */
/**********************************************************************/
TAGENTRY TokenTable[LAST_TAG + 1] =
{
    {    FIRST_TOKEN,       "",              { COMMENT_TOKEN,
                                               TAGTABLE_TOKEN   }    },
    {    ASSTEXT_TOKEN,     "ASSTEXT",       { EASSTEXT_TOKEN   }    },
    {    EASSTEXT_TOKEN,    "/ASSTEXT",      { 0                }    },
    {    TAG_TOKEN,         "TAG",           { ETAG_TOKEN,
                                               ASSTEXT_TOKEN,
                                               SEGINFO_TOKEN,
                                               ATTRINFO_TOKEN,
                                               ADDINFO_TOKEN,
                                               TRANSINFO_TOKEN,
                                               TYPE_TOKEN,
                                               COLUMPOS_TOKEN,
                                               ENDDELIM_TOKEN,
                                               COMMENT_TOKEN,
                                               STRING_TOKEN,
                                               LENGTH_TOKEN,
                                               CLASSID_TOKEN,
                                               TOKENID_TOKEN }    },
    {    ETAG_TOKEN,        "/TAG",          { 0                }    },
    {    STRING_TOKEN,      "STRING",        { ESTRING_TOKEN    }    },
    {    ESTRING_TOKEN,     "/STRING",       { 0                }    },
    {    SEGINFO_TOKEN,     "SEGINFO",       { ESEGINFO_TOKEN   }    },
    {    ESEGINFO_TOKEN,    "/SEGINFO",      { 0                }    },
    {    SEGEXIT_TOKEN,     "SEGMENTEXIT",   { ESEGEXIT_TOKEN,
                                               ETAGTABLE_TOKEN  }    },
    {    ESEGEXIT_TOKEN,    "/SEGMENTEXIT",  { 0                }    },
    {    TYPE_TOKEN,        "TYPE",          { ETYPE_TOKEN      }    },
    {    ETYPE_TOKEN,       "/TYPE",         { 0                }    },
    {    TAGTABLE_TOKEN,    "TAGTABLE",      { ETAGTABLE_TOKEN,
                                               COMMENT_TOKEN,
                                               SEGEXIT_TOKEN,
                                               TAG_TOKEN,
                                               ATTRIBUTE_TOKEN,
                                               TRNOTE_TOKEN,
                                               DESCRIPTION_TOKEN,
                                               DESCRNAME_TOKEN,
                                               SINGLESUBST_TOKEN,
                                               MULTSUBST_TOKEN,
                                               CHARSET_TOKEN,
                                               USEUNICODE_TOKEN,
                                               REFLOW_TOKEN     }    },
    {    ETAGTABLE_TOKEN,   "/TAGTABLE",     { 0                }    },
    {    ATTRIBUTE_TOKEN,   "ATTRIBUTE",     { EATTRIBUTE_TOKEN,
                                               STRING_TOKEN,
                                               LENGTH_TOKEN,
                                               COMMENT_TOKEN,
                                               ADDINFO_TOKEN,
                                               ENDDELIM_TOKEN,
                                               TRANSINFO_TOKEN,
                                               TOKENID_TOKEN }    },
    {    EATTRIBUTE_TOKEN,  "/ATTRIBUTE",    { 0                }    },
    {    COLUMPOS_TOKEN,    "COLPOSITION",   { ECOLUMPOS_TOKEN  }    },
    {    ECOLUMPOS_TOKEN,   "/COLPOSITION",  { 0                }    },
    {    ENDDELIM_TOKEN,    "ENDDELIM",      { EENDDELIM_TOKEN  }    },
    {    EENDDELIM_TOKEN,   "/ENDDELIM",     { 0                }    },
    {    TRANSINFO_TOKEN,   "TRANSLATEINFO", { ETRANSINFO_TOKEN }    },
    {    ETRANSINFO_TOKEN,  "/TRANSLATEINFO",{ 0                }    },
    {    ATTRINFO_TOKEN,    "ATTRINFO",      { EATTRINFO_TOKEN  }    },
    {    EATTRINFO_TOKEN,   "/ATTRINFO",     { 0                }    },
    {    LENGTH_TOKEN,      "LENGTH",        { ELENGTH_TOKEN    }    },
    {    ELENGTH_TOKEN,     "/LENGTH",       { 0                }    },
    {    COMMENT_TOKEN,     "COMMENT",       { ECOMMENT_TOKEN   }    },
    {    ECOMMENT_TOKEN,    "/COMMENT",      { 0                }    },
    {    ADDINFO_TOKEN,     "ADDINFO",       { EADDINFO_TOKEN   }    },
    {    EADDINFO_TOKEN,    "/ADDINFO",      { 0                }    },
    {    COPYRIGTH_TOKEN,           "",      { 0                }    },
    {    CREATDATE_TOKEN,           "",      { 0                }    },
    {    DICT_TOKEN,                "",      { 0                }    },
    {    EABBR_TOKEN,               "",      { 0                }    },
    {    EAUTHOR_TOKEN,             "",      { 0                }    },
    {    EAUTHOROFUPDATE_TOKEN,     "",      { 0                }    },
    {    ECONTEXT_TOKEN,            "",      { 0                }    },
    {    ECRDATE_TOKEN,             "",      { 0                }    },
    {    EDEF_TOKEN,                "",      { 0                }    },
    {    EHYPH_TOKEN,               "",      { 0                }    },
    {    EIDIOM_TOKEN,              "",      { 0                }    },
    {    ELUPDATE_TOKEN,            "",      { 0                }    },
    {    EMORPH_TOKEN,              "",      { 0                }    },
    {    ENTRY_TOKEN,               "",      { 0                }    },
    {    EPOS_TOKEN,                "",      { 0                }    },
    {    EREL_TOKEN,                "",      { 0                }    },
    {    ERELCODE_TOKEN,            "",      { 0                }    },
    {    ESCONTEXT_TOKEN,           "",      { 0                }    },
    {    ESDEF_TOKEN,               "",      { 0                }    },
    {    ESRC_TOKEN,                "",      { 0                }    },
    {    ESTATCODE_TOKEN,           "",      { 0                }    },
    {    ESYN_TOKEN,                "",      { 0                }    },
    {    EUSAGE_TOKEN,              "",      { 0                }    },
    {    EUSER_TOKEN,               "",      { 0                }    },
    {    HEADER_TOKEN,              "",      { 0                }    },
    {    HDTERM_TOKEN,              "",      { 0                }    },
    {    HOM_TOKEN,                 "",      { 0                }    },
    {    MAPTABLE_TOKEN,            "",      { 0                }    },
    {    SENSE_TOKEN,               "",      { 0                }    },
    {    SOURCE_TOKEN,              "",      { 0                }    },
    {    TABBR_TOKEN,               "",      { 0                }    },
    {    TARGET_TOKEN,              "",      { 0                }    },
    {    TAUTHOR_TOKEN,             "",      { 0                }    },
    {    TAUTHOROFUPDATE_TOKEN,     "",      { 0                }    },
    {    TCONTEXT_TOKEN,            "",      { 0                }    },
    {    TCRDATE_TOKEN,             "",      { 0                }    },
    {    TDEF_TOKEN,                "",      { 0                }    },
    {    THYPH_TOKEN,               "",      { 0                }    },
    {    TIDIOM_TOKEN,              "",      { 0                }    },
    {    TLANGUAGE_TOKEN,           "",      { 0                }    },
    {    TLUPDATE_TOKEN,            "",      { 0                }    },
    {    TMORPH_TOKEN,              "",      { 0                }    },
    {    TPOS_TOKEN,                "",      { 0                }    },
    {    TREL_TOKEN,                "",      { 0                }    },
    {    TRELCODE_TOKEN,            "",      { 0                }    },
    {    TSCONTEXT_TOKEN,           "",      { 0                }    },
    {    TSDEF_TOKEN,               "",      { 0                }    },
    {    TSCR_TOKEN,                "",      { 0                }    },
    {    TSTATCODE_TOKEN,           "",      { 0                }    },
    {    TSUBJCODE_TOKEN,           "",      { 0                }    },
    {    TSYN_TOKEN,                "",      { 0                }    },
    {    TRANS_TOKEN,               "",      { 0                }    },
    {    TRANSLATEINFO_TOKEN,       "",      { 0                }    },
    {    TUSAGE_TOKEN,              "",      { 0                }    },
    {    TUSER_TOKEN,               "",      { 0                }    },
    {    TRNOTE_TOKEN,      "TRNOTE",        { ETRNOTE_TOKEN,
                                               TRTAGSTART_TOKEN,
                                               TRTAGEND_TOKEN,
                                               TRPREF_TOKEN     }    },
    {    ETRNOTE_TOKEN,     "/TRNOTE",       { 0                }    },
    {    TRTAGSTART_TOKEN,  "TRTAGSTART",    { ETRTAGSTART_TOKEN     }    },
    {    ETRTAGSTART_TOKEN, "/TRTAGSTART",   { 0                }    },
    {    TRTAGEND_TOKEN,    "TRTAGEND",      { ETRTAGEND_TOKEN     }    },
    {    ETRTAGEND_TOKEN,   "/TRTAGEND",     { 0                }    },
    {    TRPREF_TOKEN,      "TRPREF",        { ETRPREF_TOKEN    }    },
    {    ETRPREF_TOKEN,     "/TRPREF",       { 0                }    },
    {    CLASSID_TOKEN,     "CLASSID",       { ECLASSID_TOKEN   }    },
    {    ECLASSID_TOKEN,    "/CLASSID",      { 0                }    },
    {    TOKENID_TOKEN,     "TOKENID",       { ETOKENID_TOKEN   }    },
    {    ETOKENID_TOKEN,    "/TOKENID",      { 0                }    },

    {    DESCRIPTION_TOKEN, "DESCRIPTION",   { EDESCRIPTION_TOKEN }  },
    {    EDESCRIPTION_TOKEN,"/DESCRIPTION",  { 0                  }  },
    {    DESCRNAME_TOKEN,   "DESCRNAME",     { EDESCRNAME_TOKEN }    },
    {    EDESCRNAME_TOKEN,  "/DESCRNAME",    { 0                }    },
    {    CHARSET_TOKEN,     "CHARSET",       { ECHARSET_TOKEN   }    },
    {    ECHARSET_TOKEN,    "/CHARSET",      { 0                }    },
    {    SINGLESUBST_TOKEN, "SINGLESUBST",   { ESINGLESUBST_TOKEN}   },
    {    ESINGLESUBST_TOKEN,"/SINGLESUBST",  { 0                }    },
    {    MULTSUBST_TOKEN,   "MULTSUBST",     { EMULTSUBST_TOKEN }    },
    {    EMULTSUBST_TOKEN,  "/MULTSUBST",    { 0                }    },
    {    USEUNICODE_TOKEN,  "USEUNICODE",    { EUSEUNICODE_TOKEN }   },
    {    EUSEUNICODE_TOKEN, "/USEUNICODE",   { 0                }    },
    {    REFLOW_TOKEN,     "REFLOW",         { EREFLOW_TOKEN   }    },
    {    EREFLOW_TOKEN,    "/REFLOW",        { 0                }    },

    {    LAST_TAG,          "",              { 0                }    }
};

SHORT TAGFORMATTABLE[NUM_OF_TAGS]=
{
  EADDINFO_TOKEN,
  EASSTEXT_TOKEN,
  EATTRIBUTE_TOKEN,
  EATTRINFO_TOKEN,
  ECHARSET_TOKEN,
  ECLASSID_TOKEN,
  ECOLUMPOS_TOKEN,
  ECOMMENT_TOKEN,
  EDESCRIPTION_TOKEN,
  EDESCRNAME_TOKEN,
  EENDDELIM_TOKEN,
  ELENGTH_TOKEN,
  EMULTSUBST_TOKEN,
  EREFLOW_TOKEN,
  ESEGINFO_TOKEN,
  ESEGEXIT_TOKEN,
  ESINGLESUBST_TOKEN,
  ESTRING_TOKEN,
  ETAG_TOKEN,
  ETAGTABLE_TOKEN,
  ETOKENID_TOKEN,
  ETRANSINFO_TOKEN,
  ETRNOTE_TOKEN,
  ETRPREF_TOKEN,
  ETRTAGEND_TOKEN,
  ETRTAGSTART_TOKEN,
  ETYPE_TOKEN,
  EUSEUNICODE_TOKEN,
  ADDINFO_TOKEN,
  ASSTEXT_TOKEN,
  ATTRIBUTE_TOKEN,
  ATTRINFO_TOKEN,
  CHARSET_TOKEN,
  CLASSID_TOKEN,
  COLUMPOS_TOKEN,
  COMMENT_TOKEN,
  DESCRIPTION_TOKEN,
  DESCRNAME_TOKEN,
  ENDDELIM_TOKEN,
  LENGTH_TOKEN,
  MULTSUBST_TOKEN,
  REFLOW_TOKEN,
  SEGINFO_TOKEN,
  SEGEXIT_TOKEN,
  SINGLESUBST_TOKEN,
  STRING_TOKEN,
  TAG_TOKEN,
  TAGTABLE_TOKEN,
  TOKENID_TOKEN,
  TRANSINFO_TOKEN,
  TRNOTE_TOKEN,
  TRPREF_TOKEN,
  TRTAGEND_TOKEN,
  TRTAGSTART_TOKEN,
  TYPE_TOKEN,
  USEUNICODE_TOKEN,

  LAST_TAG
};
#else
 extern STYLEENTRY AttrStyles[];
 extern CHOICELIST SEGSTYLES[8];
 extern CHOICELIST ASSTEXTSTYLES[4];
 extern CHOICELIST YESNOSTYLES[3];
 extern CHOICELIST REFLOWYESNOSTYLES[4];
 extern CHOICELIST TYPESTYLES[6];
 extern CHOICELIST CLASSIDSTYLES[19];
 extern TAGENTRY TokenTable[LAST_TAG + 1];
 extern TAGLISTS TAGFORMATTABLE[NUM_OF_TAGS];
 extern CHOICELIST CHARSETSTYLES[4];
#endif

typedef struct _MEMBLOCK
{
   USHORT    usBlockSize;              /* total size of memory block in bytes */
   PVOID     pNext;                    /* ptr to next block; NULL if none     */
   USHORT    usDataSize;               /* size of memory available for data   */
   USHORT    usUsedSize;               /* size of memory used for data        */
   PBYTE     pDataArea;                /* ptr to begin of available memory    */
} MEMBLOCK, *PMEMBLOCK;

typedef struct _STACK
{
    SHORT    sCurrent;                 // current entry
    SHORT    sStack[ MAX_STACK ];      // deepness of stack
} STACK , *PSTACK;

  struct _TERMENTRY
  {
     PSZ       pText;                    // text of tag
     PSZ       pEndDelim;                // end delimiter specified
     USHORT    usColumn;                 // column position
     TAGTYPE   Tagtype;                  // type of tag
     SEGBREAK  Segbreak;                 // segment info
     ASSTEXT   Asstext;                  // associated text
     BOOL      fAttr;                    // does the tag allow attributes?
     USHORT    usLength;                 // length of tag
     USHORT    usPosition;               // fixed position of tag in text
     struct
     {
        unsigned short fTranslate   : 1;   // does the tag contain text to be translated?
        unsigned short AddInfo      : 14;  // additional tag information
        unsigned short fTRNote      : 1;   // does tag contain a TRNote?
     } BitFlags;
     struct _TERMENTRY * pNext;          // pointer to next tag
     struct _TERMENTRY * pPrev;          // pointer to previous tag
     TAGCLASSID TagClassId;
     USHORT    usFixTokenId;             // token id of tag/attribute
  };
  typedef struct _TERMENTRY TERMENTRY, *PTERMENTRY;

typedef struct _DELIMIT                // delimiters structure
{
   CHAR chFirst;                       // tag starting character
   USHORT usLength;                    // length of tag
   USHORT uEnd;                          // index of end delimiters
} DELIMIT, * PDELIMIT;

typedef struct _DELPOS                 // delimiting start position
{
   USHORT usPosition;                  // tag starting position
   USHORT usLength;                    // length of tag
   USHORT uEnd;                          // index of end delimiters
} DELPOS, * PDELPOS;

  typedef struct _ATTRENTRY
  {
     USHORT usLength;                       // length of attribute
     struct
     {
        unsigned short fTranslate   : 1;   // does the tag contain text to be translated?
        unsigned short AddInfo      : 14;  // additional attribute information
        unsigned short fTRNote      : 1;    // does the attr contain a TRNote?
     } BitFlags;
     PSZ    pEndDelim;                      // pointer to end delimiters
     PSZ    pText;                          // pointer to text string
     struct _ATTRENTRY * pNext;          // pointer to next tag
     struct _ATTRENTRY * pPrev;          // pointer to previous tag
     USHORT    usFixTokenId;             // token id of tag/attribute
  } ATTRENTRY, * PATTRENTRY;


typedef struct _TRNOTEENTRY
{
   CHAR    chStartText[MAX_TAGSIZE];            // pointer to trnote starttag
   CHAR    chEndText[MAX_TAGSIZE];              // pointer to trnote endtag
   CHAR    chTrnote1Text[MAX_TAGSIZE];          // Indicates a note of level 1
   CHAR    chTrnote2Text[MAX_TAGSIZE];          // indicates a note of level 2
} TRNOTEENTRY, * PTRNOTEENTRY;


typedef enum _IMEXTASK
{
   INIT_IMPORT,
   READ_SGML,
   BUILD_ENTRY,
   WRITE_TABLE,
   CLOSE_PROCESS,
   WRITE_TAG,
   WRITE_ATTRIBUTE,
   END_IMPORT
} IMEXTASK;

typedef struct _TAGIMPORT
{
   IDA_HEAD      IdaHead;                 // common header of instance area
   CHAR          chSingleSubst;           // character to use for single substitution
   CHAR          chMultSubst;             // character to use for multiple substitution
   CHAR          szDescriptiveName[MAX_DESCRIPTION]; // descriptive name for format (displayed in format combobox)
   CHAR          szDescription[MAX_DESCRIPTION]; // format description
   USHORT        usCharacterSet;          // characterset to use for import/export
   BOOL          fUseUnicodeForSegFile;   // TRUE = store segmented files in Unicode format
   CHAR          szSegmentExit[MAX_FILESPEC];    // name of user exit dll
   TRNOTEENTRY   TRNoteEntry;             // TRNOTE tagging
   HWND          hwnd;                    // handle of process window
   HWND          hwndPathEf;              // handle to Path entry field
   HWND          hwndDirLb;               // handle to directories listbox
   HWND          hwndFilesLb;             // handle to Files listbox
   HWND          hwndNewNameLb;           // handle to new markup name sle
   CHAR          szDummy[MAX_EQF_PATH];   // string buffer
   CHAR          szString[MAX_EQF_PATH];  // string buffer
   CHAR          szDrive[MAX_DRIVE];      // drive letter with :
   CHAR          szPath[MAX_PATH144];     // drive and directory/ies
   CHAR          szPatternName[MAX_PATH144]; // active pattern name
   CHAR          szPathContent[MAX_PATH144]; // content of path entry field
   CHAR          szName[MAX_FNAME];       // filename buffer
   BOOL          fAscii;                  // internal or external RB
   FILEFINDBUF   ResultBuf;               // DOS file find struct
   HPROP         hTagListProp;            // handle of dict list properties
   CONTROLSIDA   Controls;                // ptr to controls ida
   BUFFER        stBuffer;                // input file handle
   CHAR          szSourceName[MAX_EQF_PATH];   // dictionary name
   CHAR          szTempName[MAX_EQF_PATH];     // temporary dictionary name
   CHAR          szTargetName[MAX_EQF_PATH]; // newly specified dictionary name
   CHAR          szTitle[MAX_EQF_PATH];   // title of export window
   CHAR          szImportTagTxt[MAX_PATH144];
   HWND          hwndImportTagTxt;
   HWND          hwndSlider;
   HFILE         hInFile;              // input file handle
   HFILE         hOutFile;             // output file handle
   HFILE         hOutTempFile;         // output file handle
   PSZ           pInFile;              // pointer to input file
   PSZ           pOutFile;             // pointer to output file
   PSZ           pRest;                // rest of text block not analyzed
   BOOL          fAll;                 // last text block read?
   STACK         Stack;                // stack
   PLOADEDTABLE  pTagTable;            // pointer to loaded tag table
   TERMENTRY     TermEntry;            // current term entry
   ATTRENTRY     AttrEntry;            // current attribute entry
   CHAR          szInFile[MAX_EQF_PATH];// input file name
   SHORT         sOrgToken[MAX_TAGS];// max. tokens allowed
   USHORT        usAttrCount;          // number of attributes
   USHORT        usTagCount;           // number of tags
   SHORT         sActiveStartPos;      // active start position
   SHORT         sMaxStartPos;         // maximum start position
   PSTARTPOS     pStartPos;            // pointer to startpositions
   PTAGSTART     pTagStart;            // pointer to tagstarting characters
   SHORT         sActiveDelimit;       // active delimit structure
   SHORT         sMaxDelimit;          // maximum delimit structures
   PDELPOS       pDelPos;              // pointer to delimiting positions
   PDELIMIT      pDelimit;             // pointer to delimiting structures
   PTERMENTRY    pTermStart[ 29 ];     // tag starts
   PATTRENTRY    pAttrStart[ 29 ];     // attribute starts
   USHORT        usNumTag[ 29 ];       // number of tags
   USHORT        usNumAttr[ 29 ];      // number of attributes
   CHAR          szWork[MAX_TAGSIZE];  // work buffer for tags
   PSZ           pStart;               // start of buffer for names
   PSZ           pBuffer;              // buffer for strings
   PSZ           pEmptyString;         // pointer to empty string
   USHORT        usRest;               // rest in buffer
   PMEMBLOCK     pFileBlock;
   PSZ           pTokenBuffer;
   BOOL          fKill;
   BOOL          fTag;
   BOOL          fFinished;
   BOOL          fSegExit;
   IMEXTASK      Task;
   USHORT        usKilledTask;
   CHAR          chWorkBuffer[BUFFERLEN]; // temporary working area
   ULONG         ulFileSize;
   BOOL          fNewTable;
 /*******************************************************************/
 /* The following BOOL fReflow is a 3state BOOL:                    */
 /* 0           = not used, as previously                           */
 /* 1           = Reflow allowed                                    */
 /* 2           = no Reflow, for MRI etc                            */
 /*******************************************************************/
   BOOL          fReflow;                // status of Reflow
   HWND          hwndToCombo;              // handle of To: combo box
} TAGIMPORT, *PTAGIMPORT;

// external interface to build tag tables from scratch
__declspec(dllexport)
BOOL TagImpAllocTagImport( PTAGIMPORT *ppTagImport );
__declspec(dllexport)
BOOL TagImpFreeTagImport( PTAGIMPORT pTagImport );
__declspec(dllexport)
BOOL TagImpAddTag( PTAGIMPORT pTagImport, PTERMENTRY pNewTag );
__declspec(dllexport)
BOOL TagImpAddAttr( PTAGIMPORT pTagImport, PATTRENTRY pNewAttr );
__declspec(dllexport)
BOOL TagImpBuildtagTable( PTAGIMPORT pTagImport, PTAGTABLE *ppTagTable );
