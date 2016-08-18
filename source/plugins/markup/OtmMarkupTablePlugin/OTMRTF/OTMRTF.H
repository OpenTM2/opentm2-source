//+----------------------------------------------------------------------------+
//|OTMRTF.H                                                                   |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:         G. Queck (QSoft)                                            |
//+----------------------------------------------------------------------------+
//|Description:    Include file for the RTF parser                             |
//+----------------------------------------------------------------------------+
//|Changes:                                                                    |
//|   13/02/18  Added the tags \*\datastore \*\colorschememapper \*\themedata  |
//+----------------------------------------------------------------------------+

/**********************************************************************/
/* General constants                                                  */
/**********************************************************************/
#define EOS                '\0'        // 'C' end-of-string character
#define END_OF_FILE        '\x1A'      // end of file delimiter
#define BEGIN_PARMS        '('         // begin of macro parameters
#define BEGIN_PARMS_W      L'('        // begin of macro parameters
#define END_PARMS          ')'         // end of macro parameters
#define END_PARMS_W        L')'        // end of macro parameters
#define BEGIN_GROUP        '{'         // begin of a group
#define BEGIN_GROUP_W      L'{'        // begin of a group
#define END_GROUP          '}'         // end of a group
#define END_GROUP_W        L'}'        // end of a group
#define START_CTRLWORD     '\\'        // start of a control word
#define START_CTRLWORD_W   L'\\'       // start of a control word
#define START_MACRO        '!'         // start of a (windows help hotspot) macro
#define START_MACRO_W      L'!'        // start of a (windows help hotspot) macro
#define COMMENT_TAG        '*'         // comment tag (prefix for new RTF tags)
#define COMMENT_TAG_W      L'*'        // comment tag (prefix for new RTF tags)
#define SPACE              ' '         // space character
#define SPACE_W            L' '        // space character
#define MINUS              '-'         // minus sign character
#define LF                 '\n'        // linefeed character
#define CR                 '\r'        // carriage return character
#define CRLF_STR           "\r\n"      // carriage return linefeed string
#define SEMICOLON          ';'         // semicolon (macro parm seperator)
#define COMMA              ','         // comma (macro parm seperator)
#define HEXCODE_IDENTIFIER '\''        // identifier for hex encoded data
#define HEXCODE_IDENTIFIER_W L'\''     // identifier for hex encoded data
#define BACKSLASH_W        L'\\'       // a backslash ...
#define LF_W               L'\n'       // a linefeed ...
#define CR_W               L'\r'       // a carriage return ...
#define COMMA_W            L','        // a comma ...
#define COLON_W            L':'        // a colon ...
#define MINUS_W            L'-'        // a negative sign ...
#define DOUBLEQUOTE_W      L'\"'       // a doublequote
#define MAX_DEST_STACK     30          // max. nesting level for destinations
// GQ: Attention, the max segment size has to be smaller than the actual allowed
//     maxiumum as there is a bug producing MAX_SEG_SIZE + 1 large segments...
#define MAX_SEG_SIZE       2046        // size of segment (=output) buffer
#define MAX_CTRL_WORD      80          // max length of a RTF control word
#define INBUF_SIZE         8192        // size of input buffer
#define EOF_REACHED        0xFF01      // end-of-file return code
#define PLACEHOLDER        "f%1"       // place holder for font numner
#define PLACEHOLDER1       'f'         // first character of place holder
#define PLACEHOLDER2       '%'         // second character of place holder
#define PLACEHOLDER3       '1'         // third character of place holder

// ID of ANSI RTF documents
#define RTF_ANSI_CODEPAGE  "{\\rtf1\\ansi"
#define ANSI_TAG           "\\ansi"

// max. no. of entries in the CHANGEFONT section
#define MAX_CHFTTAB_ENTRIES 50
// max. no. of entries in the DEFCHGFONT section
#define MAX_DEFCHFT_ENTRIES 10

/**********************************************************************/
/* Macro to convert hex digits to numeric values.                     */
/* (the hex digit must be converted to uppercase)                     */
/**********************************************************************/
#define HEXTONUM( char ) \
  (( char < 'A' ) ? ( char - '0' ) : ( char - 'A' + 10) )

/**********************************************************************/
/* Types of output segments created by the parser                     */
/**********************************************************************/
typedef enum _SEGTYPES
{
   TRANSL_SEGMENT,                     // segment containing translatable data
   PROTECT_SEGMENT,                    // protected segment
   NO_SEGMENT                          // no semgent is open
} SEGTYPES;

/**********************************************************************/
/* Processing modes of parser                                         */
/**********************************************************************/
typedef enum _PROCESSMODES
{
   CTRLWORD_MODE,                      // processing a control word
   TEXT_MODE                           // processing text data
} PROCESSMODES;

/**********************************************************************/
/* Table for RTF control symbols                                      */
/* The symbol is used as an index into this table                     */
/**********************************************************************/
typedef enum _SYMBTYPE
{
  CHAR_SYMB,                           // symbol is replaced with a char value
  HEX_SYMB,                            // symbol is followed by a hex value
  NO_SYMB                              // no or unknown symbol
} SYMBTYPE;

typedef struct _CTRLSYMBOL
{
   SYMBTYPE SymbType;                  // Symbol type (see enum above)
   CHAR     chTarget;                  // Target character for CHAR_SYMB
} CTRLSYMBOL;

CTRLSYMBOL CtrlSymbols[256] = {
/* 00   */ { NO_SYMB,    0        },   //
/* 01   */ { NO_SYMB,    0        },   //
/* 02   */ { NO_SYMB,    0        },   //
/* 03   */ { NO_SYMB,    0        },   //
/* 04   */ { NO_SYMB,    0        },   //
/* 05   */ { NO_SYMB,    0        },   //
/* 06   */ { NO_SYMB,    0        },   //
/* 07   */ { NO_SYMB,    0        },   //
/* 08   */ { NO_SYMB,    0        },   //
/* 09   */ { NO_SYMB,    0        },   //
/* 0A   */ { NO_SYMB,    0        },   //
/* 0B   */ { NO_SYMB,    0        },   //
/* 0C   */ { NO_SYMB,    0        },   //
/* 0D   */ { NO_SYMB,    0        },   //
/* 0E   */ { NO_SYMB,    0        },   //
/* 0F   */ { NO_SYMB,    0        },   //
/* 10   */ { NO_SYMB,    0        },   //
/* 11   */ { NO_SYMB,    0        },   //
/* 12   */ { NO_SYMB,    0        },   //
/* 13   */ { NO_SYMB,    0        },   //
/* 14   */ { NO_SYMB,    0        },   //
/* 15   */ { NO_SYMB,    0        },   //
/* 16   */ { NO_SYMB,    0        },   //
/* 17   */ { NO_SYMB,    0        },   //
/* 18   */ { NO_SYMB,    0        },   //
/* 19   */ { NO_SYMB,    0        },   //
/* 1A   */ { NO_SYMB,    0        },   //
/* 1B   */ { NO_SYMB,    0        },   //
/* 1C   */ { NO_SYMB,    0        },   //
/* 1D   */ { NO_SYMB,    0        },   //
/* 1E   */ { NO_SYMB,    0        },   //
/* 1F   */ { NO_SYMB,    0        },   //
/* 20   */ { NO_SYMB,    0        },   //
/* 21   */ { NO_SYMB,    0        },   //
/* 22   */ { NO_SYMB,    0        },   //
/* 23   */ { NO_SYMB,    0        },   //
/* 24   */ { NO_SYMB,    0        },   //
/* 25   */ { NO_SYMB,    0        },   //
/* 26   */ { NO_SYMB,    0        },   //
/* 27 ' */ { HEX_SYMB,   0        },   // hexadecimal value
/* 28   */ { NO_SYMB,    0        },   //
/* 29   */ { NO_SYMB,    0        },   //
/* 2A   */ { NO_SYMB,    0        },   //
/* 2B   */ { NO_SYMB,    0        },   //
/* 2C   */ { NO_SYMB,    0        },   //
/* 2D - */ { CHAR_SYMB,  0        },   // ignore dashes!
/* 2E   */ { NO_SYMB,    0        },   //
/* 2F   */ { NO_SYMB,    0        },   //
/* 30   */ { NO_SYMB,    0        },   //
/* 31   */ { NO_SYMB,    0        },   //
/* 32   */ { NO_SYMB,    0        },   //
/* 33   */ { NO_SYMB,    0        },   //
/* 34   */ { NO_SYMB,    0        },   //
/* 35   */ { NO_SYMB,    0        },   //
/* 36   */ { NO_SYMB,    0        },   //
/* 37   */ { NO_SYMB,    0        },   //
/* 38   */ { NO_SYMB,    0        },   //
/* 39   */ { NO_SYMB,    0        },   //
/* 3A : */ { NO_SYMB,    0        },   // index subentry (leave as-is!)
/* 3B   */ { NO_SYMB,    0        },   //
/* 3C   */ { NO_SYMB,    0        },   //
/* 3D   */ { NO_SYMB,    0        },   //
/* 3E   */ { NO_SYMB,    0        },   //
/* 3F   */ { NO_SYMB,    0        },   //
/* 40   */ { NO_SYMB,    0        },   //
/* 41   */ { NO_SYMB,    0        },   //
/* 42   */ { NO_SYMB,    0        },   //
/* 43   */ { NO_SYMB,    0        },   //
/* 44   */ { NO_SYMB,    0        },   //
/* 45   */ { NO_SYMB,    0        },   //
/* 46   */ { NO_SYMB,    0        },   //
/* 47   */ { NO_SYMB,    0        },   //
/* 48   */ { NO_SYMB,    0        },   //
/* 49   */ { NO_SYMB,    0        },   //
/* 4A   */ { NO_SYMB,    0        },   //
/* 4B   */ { NO_SYMB,    0        },   //
/* 4C   */ { NO_SYMB,    0        },   //
/* 4D   */ { NO_SYMB,    0        },   //
/* 4E   */ { NO_SYMB,    0        },   //
/* 4F   */ { NO_SYMB,    0        },   //
/* 50   */ { NO_SYMB,    0        },   //
/* 51   */ { NO_SYMB,    0        },   //
/* 52   */ { NO_SYMB,    0        },   //
/* 53   */ { NO_SYMB,    0        },   //
/* 54   */ { NO_SYMB,    0        },   //
/* 55   */ { NO_SYMB,    0        },   //
/* 56   */ { NO_SYMB,    0        },   //
/* 57   */ { NO_SYMB,    0        },   //
/* 58   */ { NO_SYMB,    0        },   //
/* 59   */ { NO_SYMB,    0        },   //
/* 5A   */ { NO_SYMB,    0        },   //
/* 5B   */ { NO_SYMB,    0        },   //
/* 5C   */ { NO_SYMB,    0        },   // backslash character
/* 5D   */ { NO_SYMB,    0        },   //
/* 5E   */ { NO_SYMB,    0        },   //
/* 5F _ */ { CHAR_SYMB,  '-'      },   // required dash
/* 60   */ { NO_SYMB,    0        },   //
/* 61   */ { NO_SYMB,    0        },   //
/* 62   */ { NO_SYMB,    0        },   //
/* 63   */ { NO_SYMB,    0        },   //
/* 64   */ { NO_SYMB,    0        },   //
/* 65   */ { NO_SYMB,    0        },   //
/* 66   */ { NO_SYMB,    0        },   //
/* 67   */ { NO_SYMB,    0        },   //
/* 68   */ { NO_SYMB,    0        },   //
/* 69   */ { NO_SYMB,    0        },   //
/* 6A   */ { NO_SYMB,    0        },   //
/* 6B   */ { NO_SYMB,    0        },   //
/* 6C   */ { NO_SYMB,    0        },   //
/* 6D   */ { NO_SYMB,    0        },   //
/* 6E   */ { NO_SYMB,    0        },   //
/* 6F   */ { NO_SYMB,    0        },   //
/* 70   */ { NO_SYMB,    0        },   //
/* 71   */ { NO_SYMB,    0        },   //
/* 72   */ { NO_SYMB,    0        },   //
/* 73   */ { NO_SYMB,    0        },   //
/* 74   */ { NO_SYMB,    0        },   //
/* 75   */ { NO_SYMB,    0        },   //
/* 76   */ { NO_SYMB,    0        },   //
/* 77   */ { NO_SYMB,    0        },   //
/* 78   */ { NO_SYMB,    0        },   //
/* 79   */ { NO_SYMB,    0        },   //
/* 7A   */ { NO_SYMB,    0        },   //
/* 7B   */ { NO_SYMB,    0        },   //
/* 7C | */ { CHAR_SYMB,  ' '      },   // formula character
/* 7D   */ { NO_SYMB,    0        },   //
/* 7E ~ */ { CHAR_SYMB,  ' '      },   // required blank
/* 7F   */ { NO_SYMB,    0        },   //
/* 80   */ { NO_SYMB,    0        },   //
/* 81   */ { NO_SYMB,    0        },   //
/* 82   */ { NO_SYMB,    0        },   //
/* 83   */ { NO_SYMB,    0        },   //
/* 84   */ { NO_SYMB,    0        },   //
/* 85   */ { NO_SYMB,    0        },   //
/* 86   */ { NO_SYMB,    0        },   //
/* 87   */ { NO_SYMB,    0        },   //
/* 88   */ { NO_SYMB,    0        },   //
/* 89   */ { NO_SYMB,    0        },   //
/* 8A   */ { NO_SYMB,    0        },   //
/* 8B   */ { NO_SYMB,    0        },   //
/* 8C   */ { NO_SYMB,    0        },   //
/* 8D   */ { NO_SYMB,    0        },   //
/* 8E   */ { NO_SYMB,    0        },   //
/* 8F   */ { NO_SYMB,    0        },   //
/* 90   */ { NO_SYMB,    0        },   //
/* 91   */ { NO_SYMB,    0        },   //
/* 92   */ { NO_SYMB,    0        },   //
/* 93   */ { NO_SYMB,    0        },   //
/* 94   */ { NO_SYMB,    0        },   //
/* 95   */ { NO_SYMB,    0        },   //
/* 96   */ { NO_SYMB,    0        },   //
/* 97   */ { NO_SYMB,    0        },   //
/* 98   */ { NO_SYMB,    0        },   //
/* 99   */ { NO_SYMB,    0        },   //
/* 9A   */ { NO_SYMB,    0        },   //
/* 9B   */ { NO_SYMB,    0        },   //
/* 9C   */ { NO_SYMB,    0        },   //
/* 9D   */ { NO_SYMB,    0        },   //
/* 9E   */ { NO_SYMB,    0        },   //
/* 9F   */ { NO_SYMB,    0        },   //
/* A0   */ { NO_SYMB,    0        },   //
/* A1   */ { NO_SYMB,    0        },   //
/* A2   */ { NO_SYMB,    0        },   //
/* A3   */ { NO_SYMB,    0        },   //
/* A4   */ { NO_SYMB,    0        },   //
/* A5   */ { NO_SYMB,    0        },   //
/* A6   */ { NO_SYMB,    0        },   //
/* A7   */ { NO_SYMB,    0        },   //
/* A8   */ { NO_SYMB,    0        },   //
/* A9   */ { NO_SYMB,    0        },   //
/* AA   */ { NO_SYMB,    0        },   //
/* AB   */ { NO_SYMB,    0        },   //
/* AC   */ { NO_SYMB,    0        },   //
/* AD   */ { NO_SYMB,    0        },   //
/* AE   */ { NO_SYMB,    0        },   //
/* AF   */ { NO_SYMB,    0        },   //
/* B0   */ { NO_SYMB,    0        },   //
/* B1   */ { NO_SYMB,    0        },   //
/* B2   */ { NO_SYMB,    0        },   //
/* B3   */ { NO_SYMB,    0        },   //
/* B4   */ { NO_SYMB,    0        },   //
/* B5   */ { NO_SYMB,    0        },   //
/* B6   */ { NO_SYMB,    0        },   //
/* B7   */ { NO_SYMB,    0        },   //
/* B8   */ { NO_SYMB,    0        },   //
/* B9   */ { NO_SYMB,    0        },   //
/* BA   */ { NO_SYMB,    0        },   //
/* BB   */ { NO_SYMB,    0        },   //
/* BC   */ { NO_SYMB,    0        },   //
/* BD   */ { NO_SYMB,    0        },   //
/* BE   */ { NO_SYMB,    0        },   //
/* BF   */ { NO_SYMB,    0        },   //
/* C0   */ { NO_SYMB,    0        },   //
/* C1   */ { NO_SYMB,    0        },   //
/* C2   */ { NO_SYMB,    0        },   //
/* C3   */ { NO_SYMB,    0        },   //
/* C4   */ { NO_SYMB,    0        },   //
/* C5   */ { NO_SYMB,    0        },   //
/* C6   */ { NO_SYMB,    0        },   //
/* C7   */ { NO_SYMB,    0        },   //
/* C8   */ { NO_SYMB,    0        },   //
/* C9   */ { NO_SYMB,    0        },   //
/* CA   */ { NO_SYMB,    0        },   //
/* CB   */ { NO_SYMB,    0        },   //
/* CC   */ { NO_SYMB,    0        },   //
/* CD   */ { NO_SYMB,    0        },   //
/* CE   */ { NO_SYMB,    0        },   //
/* CF   */ { NO_SYMB,    0        },   //
/* D0   */ { NO_SYMB,    0        },   //
/* D1   */ { NO_SYMB,    0        },   //
/* D2   */ { NO_SYMB,    0        },   //
/* D3   */ { NO_SYMB,    0        },   //
/* D4   */ { NO_SYMB,    0        },   //
/* D5   */ { NO_SYMB,    0        },   //
/* D6   */ { NO_SYMB,    0        },   //
/* D7   */ { NO_SYMB,    0        },   //
/* D8   */ { NO_SYMB,    0        },   //
/* D9   */ { NO_SYMB,    0        },   //
/* DA   */ { NO_SYMB,    0        },   //
/* DB   */ { NO_SYMB,    0        },   //
/* DC   */ { NO_SYMB,    0        },   //
/* DD   */ { NO_SYMB,    0        },   //
/* DE   */ { NO_SYMB,    0        },   //
/* DF   */ { NO_SYMB,    0        },   //
/* E0   */ { NO_SYMB,    0        },   //
/* E1   */ { NO_SYMB,    0        },   //
/* E2   */ { NO_SYMB,    0        },   //
/* E3   */ { NO_SYMB,    0        },   //
/* E4   */ { NO_SYMB,    0        },   //
/* E5   */ { NO_SYMB,    0        },   //
/* E6   */ { NO_SYMB,    0        },   //
/* E7   */ { NO_SYMB,    0        },   //
/* E8   */ { NO_SYMB,    0        },   //
/* E9   */ { NO_SYMB,    0        },   //
/* EA   */ { NO_SYMB,    0        },   //
/* EB   */ { NO_SYMB,    0        },   //
/* EC   */ { NO_SYMB,    0        },   //
/* ED   */ { NO_SYMB,    0        },   //
/* EE   */ { NO_SYMB,    0        },   //
/* EF   */ { NO_SYMB,    0        },   //
/* F0   */ { NO_SYMB,    0        },   //
/* F1   */ { NO_SYMB,    0        },   //
/* F2   */ { NO_SYMB,    0        },   //
/* F3   */ { NO_SYMB,    0        },   //
/* F4   */ { NO_SYMB,    0        },   //
/* F5   */ { NO_SYMB,    0        },   //
/* F6   */ { NO_SYMB,    0        },   //
/* F7   */ { NO_SYMB,    0        },   //
/* F8   */ { NO_SYMB,    0        },   //
/* F9   */ { NO_SYMB,    0        },   //
/* FA   */ { NO_SYMB,    0        },   //
/* FB   */ { NO_SYMB,    0        },   //
/* FC   */ { NO_SYMB,    0        },   //
/* FD   */ { NO_SYMB,    0        },   //
/* FE   */ { NO_SYMB,    0        },   //
/* FF   */ { NO_SYMB,    0        } }; //

/**********************************************************************/
/* Table for RTF control words                                        */
/*                                                                    */
/* type:   INL_T     inline    the tag does not break the current     */
/*                             segment                                */
/*         CHAR_T    character the tag is a character, value contains */
/*                             the character code                     */
/*         BREAK_T   break     the tag breaks the current segment and */
/*                             starts a new one                       */
/*         DEST_T    destination  the tag starts a new destination    */
/*                                                                    */
/* value:  for CHAR_T  tags: character value                          */
/*         for DEST_T  tags: 0 = group contains untranslatable data   */
/*                           1 = group contains translatable data     */
/**********************************************************************/
typedef enum _CTRLTYPE
{
  INL_T,           // inline = the tag does not break the current segment
  CHAR_T,          // character = the tag is a character
  BREAK_T,         // break = the tag breaks the current segment
  DEST_T,          // destination = the tag starts a new destination
  FINL_T,          // field inline tag, special handling
  NOSPEC_TYPE      // dummy value
} CTRLTYPE;

/**********************************************************************/
/* Symbolic identifiers for RTF tags                                  */
/**********************************************************************/
typedef enum _RTFTAGS
{
   ANNOTATION_RTFTAG,
   ATNAUTHOR_RTFTAG,
   ATNREF_RTFTAG,
   ATNID_RTFTAG,
   ATRFEND_RTFTAG,
   ATRFSTART_RTFTAG,
   BKMKEND_RTFTAG,
   BKMKSTART_RTFTAG,
   FLDINST_RTFTAG,
   NEXTFILE_RTFTAG,
   TEMPLATE_RTFTAG,
   ABSWN_RTFTAG,
   ANSI_RTFTAG,
   AUTHOR_RTFTAG,
   B_RTFTAG,
   BIDIOFF_METATAG,
   BIDION_METATAG,
   BIN_RTFTAG,
   BLUE_RTFTAG,
   BOX_RTFTAG,
   BRDRB_RTFTAG,
   BRDRBAR_RTFTAG,
   BRDRDB_RTFTAG,
   BRDRDOT_RTFTAG,
   BRDRHAIR_RTFTAG,
   BRDRL_RTFTAG,
   BRDRR_RTFTAG,
   BRDRS_RTFTAG,
   BRDRSH_RTFTAG,
   BRDRT_RTFTAG,
   BRDRTH_RTFTAG,
   BRSP_RTFTAG,
   BRSPN_RTFTAG,
   BULLET_RTFTAG,
   BUPTIM_RTFTAG,
   BXE_RTFTAG,
   CAPS_RTFTAG,
   CB_RTFTAG,
   CELL_RTFTAG,
   CELLX_RTFTAG,
   CF_RTFTAG,
   CHATN_RTFTAG,
   CHDATE_RTFTAG,
   CHFTN_RTFTAG,
   CHFTNSEP_RTFTAG,
   CHFTNSEPC_RTFTAG,
   CHPGN_RTFTAG,
   CHTIME_RTFTAG,
   CLBRDRB_RTFTAG,
   CLBRDRL_RTFTAG,
   CLBRDRR_RTFTAG,
   CLBRDRT_RTFTAG,
   CLMGF_RTFTAG,
   CLMRG_RTFTAG,
   COLORTBL_RTFTAG,
   COLS_RTFTAG,
   COLSX_RTFTAG,
   COLUMN_RTFTAG,
   COMMENT_RTFTAG,
   CREATIM_RTFTAG,
   DATAFLD_RTFTAG,
   DEFF_RTFTAG,
   DEFFORMAT_RTFTAG,
   DEFTAB_RTFTAG,
   DELETED_RTFTAG,
   DIBITMAP_RTFTAG,
   DN_RTFTAG,
   DOCCOMM_RTFTAG,
   DOCVAR_RTFTAG,
   DXFRTEXTN_RTFTAG,
   DY_RTFTAG,
   EDMINS_RTFTAG,
   EMDASH_RTFTAG,
   ENDASH_RTFTAG,
   ENDNHERE_RTFTAG,
   ENDNOTES_RTFTAG,
   EXPND_RTFTAG,
   F_RTFTAG,
   FACINGP_RTFTAG,
   FACTOIDNAME_RTFTAG,
   FCHARS_RTFTAG,
   FDECOR_RTFTAG,
   FI_RTFTAG,
   FIELD_RTFTAG,
   FLDDIRTY_RTFTAG,
   FLDEDIT_RTFTAG,
   FLDLOCK_RTFTAG,
   FLDPRIV_RTFTAG,
   FLDRSLT_RTFTAG,
   FMODERN_RTFTAG,
   FNIL_RTFTAG,
   FONTTBL_RTFTAG,
   FOOTER_RTFTAG,
   FOOTERF_RTFTAG,
   FOOTERL_RTFTAG,
   FOOTERR_RTFTAG,
   FOOTERX_RTFTAG,
   FOOTERY_RTFTAG,
   FOOTNOTE_RTFTAG,
   FRACWIDTH_RTFTAG,
   FROMAN_RTFTAG,
   FS_RTFTAG,
   FSCRIPT_RTFTAG,
   FSWISS_RTFTAG,
   FTECH_RTFTAG,
   FTNBJ_RTFTAG,
   FTNCN_RTFTAG,
   FTNRESTART_RTFTAG,
   FTNSEP_RTFTAG,
   FTNSEPC_RTFTAG,
   FTNSTARTN_RTFTAG,
   FTNTJ_RTFTAG,
   GREEN_RTFTAG,
   GUTTER_RTFTAG,
   HEADER_RTFTAG,
   HEADERF_RTFTAG,
   HEADERL_RTFTAG,
   HEADERR_RTFTAG,
   HEADERX_RTFTAG,
   HEADERY_RTFTAG,
   HR_RTFTAG,
   HYPHHOTZ_RTFTAG,
   I_RTFTAG,
   ID_RTFTAG,
   INFO_RTFTAG,
   INTBL_RTFTAG,
   IXE_RTFTAG,
   KEEP_RTFTAG,
   KEEPN_RTFTAG,
   KEYWORDS_RTFTAG,
   LANDSCAPE_RTFTAG,
   LCHARS_RTFTAG,
   LDBLQUOTE_RTFTAG,
   LEVELNUMBERS_RTFTAG,
   LEVELTEXT_RTFTAG,
   LI_RTFTAG,
   LINE_RTFTAG,
   LINEBETCOL_RTFTAG,
   LINECONT_RTFTAG,
   LINEMOD_RTFTAG,
   LINEPPAGE_RTFTAG,
   LINERESTARTS_RTFTAG,
   LINESTARTN_RTFTAG,
   LINESTARTS_RTFTAG,
   LINEX_RTFTAG,
   LISTNAME_RTFTAG,
   LISTOVERRIDETABLE_RTFTAG,
   LISTTABLE_RTFTAG,
   LISTTEXT_RTFTAG,
   LTRCH_RTFTAG,
   LTRPAR_RTFTAG,
   LQUOTE_RTFTAG,
   MAC_RTFTAG,
   MACPICT_RTFTAG,
   MAKEBACKUP_RTFTAG,
   MARGB_RTFTAG,
   MARGL_RTFTAG,
   MARGMIRROR_RTFTAG,
   MARGR_RTFTAG,
   MARGT_RTFTAG,
   MIN_RTFTAG,
   MO_RTFTAG,
   NOFCHARS_RTFTAG,
   NOFPAGES_RTFTAG,
   NOFWORDS_RTFTAG,
   NOLINE_RTFTAG,
   NOPROOF_RTFTAG,
   OBJDATA_RTFTAG,
   OBJCLASS_RTFTAG,
   OPERATOR_RTFTAG,
   OUTL_RTFTAG,
   PAGE_RTFTAG,
   PAGEBB_RTFTAG,
   PAPERH_RTFTAG,
   PAPERW_RTFTAG,
   PAR_RTFTAG,
   PARD_RTFTAG,
   PC_RTFTAG,
   PCA_RTFTAG,
   PGNCONT_RTFTAG,
   PGNDEC_RTFTAG,
   PGNLCLTR_RTFTAG,
   PGNLCRM_RTFTAG,
   PGNRESTART_RTFTAG,
   PGNSTARTN_RTFTAG,
   PGNSTARTS_RTFTAG,
   PGNUCLTR_RTFTAG,
   PGNUCRM_RTFTAG,
   PGNX_RTFTAG,
   PGNY_RTFTAG,
   PHCOL_RTFTAG,
   PHMRG_RTFTAG,
   PHPG_RTFTAG,
   PICCROPB_RTFTAG,
   PICCROPL_RTFTAG,
   PICCROPR_RTFTAG,
   PICCROPT_RTFTAG,
   PICH_RTFTAG,
   PICHGOAL_RTFTAG,
   PICSCALED_RTFTAG,
   PICSCALEX_RTFTAG,
   PICSCALEY_RTFTAG,
   PICT_RTFTAG,
   PICW_RTFTAG,
   PICWGOAL_RTFTAG,
   PLAIN_RTFTAG,
   PN_RTFTAG,         // TK
   PNSECLVL_RTFTAG,
   PNTXTA_RTFTAG,
   PNTXTB_RTFTAG,
   POSX_RTFTAG,
   POSXC_RTFTAG,
   POSXI_RTFTAG,
   POSXL_RTFTAG,
   POSXO_RTFTAG,
   POSXR_RTFTAG,
   POSY_RTFTAG,
   POSYB_RTFTAG,
   POSYC_RTFTAG,
   POSYIL_RTFTAG,
   POSYT_RTFTAG,
   PRINTIM_RTFTAG,
   PVMRG_RTFTAG,
   PVPG_RTFTAG,
   QC_RTFTAG,
   QJ_RTFTAG,
   QL_RTFTAG,
   QR_RTFTAG,
   RDBLQUOTE_RTFTAG,
   RED_RTFTAG,
   REVBAR_RTFTAG,
   REVISED_RTFTAG,
   REVISIONS_RTFTAG,
   REVPROP_RTFTAG,
   REVTBL_RTFTAG,
   REVTIM_RTFTAG,
   RI_RTFTAG,
   ROW_RTFTAG,
   RQUOTE_RTFTAG,
   RTF_RTFTAG,
   RTLCH_RTFTAG,
   RTLPAR_RTFTAG,
   RXE_RTFTAG,
   S_RTFTAG,
   SA_RTFTAG,
   SB_RTFTAG,
   SBASEDON_RTFTAG,
   SBKCOL_RTFTAG,
   SBKEVEN_RTFTAG,
   SBKNONE_RTFTAG,
   SBKPAGE_RTFTAG,
   SBYS_RTFTAG,
   SCAPS_RTFTAG,
   SECT_RTFTAG,
   SECTD_RTFTAG,
   SHAD_RTFTAG,
   SHP_RTFTAG,
   SHPGRP_RTFTAG,
   SHPTXT_RTFTAG,
   SKBODD_RTFTAG,
   SL_RTFTAG,
   SNEXT_RTFTAG,
   STRIKE_RTFTAG,
   STYLESHEET_RTFTAG,
   SUBJECT_RTFTAG,
   TAB_RTFTAG,
   TB_RTFTAG,
   TC_RTFTAG,
   TCF_RTFTAG,
   TCL_RTFTAG,
   TITLE_RTFTAG,
   TITLEPG_RTFTAG,
   TLDOT_RTFTAG,
   TLHYPH_RTFTAG,
   TLTH_RTFTAG,
   TLUL_RTFTAG,
   TQC_RTFTAG,
   TQDEC_RTFTAG,
   TQR_RTFTAG,
   TRGAPH_RTFTAG,
   TRLEFT_RTFTAG,
   TROWD_RTFTAG,
   TRQC_RTFTAG,
   TRQL_RTFTAG,
   TRQR_RTFTAG,
   TRRHN_RTFTAG,
   TX_RTFTAG,
   TXE_RTFTAG,
   U_RTFTAG,
   UL_RTFTAG,
   ULD_RTFTAG,
   ULDB_RTFTAG,
   ULNONE_RTFTAG,
   ULW_RTFTAG,
   UP_RTFTAG,
   V_RTFTAG,
   VERN_RTFTAG,
   VERSION_RTFTAG,
   VERTALB_RTFTAG,
   VERTALC_RTFTAG,
   VERTALJ_RTFTAG,
   VERTALT_RTFTAG,
   WBITMAP_RTFTAG,
   WBMBITSPIXEL_RTFTAG,
   WBMPLANES_RTFTAG,
   WBMWIDTHBYTES_RTFTAG,
   WIDOWCTRL_RTFTAG,
   WMETAFILE_RTFTAG,
   XE_RTFTAG,
   YR_RTFTAG,
   // new destination tags RTF 1.6 - RTF 1.8
   ATNPARENT_RTFTAG,
   BACKGROUND_RTFTAG,
   CATEGORY_RTFTAG,
   COLORSCHEMEMAPPING_RTFTAG,
   COMPANY_RTFTAG,
   DATASTORE_RTFTAG,
   FFDEFTEXT_RTFTAG,
   FFENTRYMCR_RTFTAG,
   FFEXITMCR_RTFTAG,
   FFFORMAT_RTFTAG,
   FFHELPTEXT_RTFTAG,
   FFL_RTFTAG,
   FFNAME_RTFTAG,
   FFSTATTEXT_RTFTAG,
   FLDTYPE_RTFTAG,
   FNAME_RTFTAG,
   FORMFIELD_RTFTAG,
   G_RTFTAG,
   GENERATOR_RTFTAG,
   GRIDTBL_RTFTAG,
   LATENTSTYLES_RTFTAG,
   LSDLOCKEDEXCEPT_RTFTAG,
   MANAGER_RTFTAG,
   NESTTABLEPROPS_RTFTAG,
   NONESTTABLES_RTFTAG,
   OLDCPROPS_RTFTAG,
   OLDPPROPS_RTFTAG,
   OLDSPROPS_RTFTAG,
   OLDTPROPS_RTFTAG,
   PANOSE_RTFTAG,
   PGP_RTFTAG,
   PGPTBL_RTFTAG,
   PICPROP_RTFTAG,
   PRIVATE_RTFTAG,
   PROTEND_RTFTAG,
   PROTSTART_RTFTAG,
   PROTUSERTBL_RTFTAG,
   PXE_RTFTAG,
   RSIDTBL_RTFTAG,
   SHPPICT_RTFTAG,
   SVB_RTFTAG,
   THEMEDATA_RTFTAG,
   UD_RTFTAG,
   UPR_RTFTAG,
   USERPROPS_RTFTAG,           
   // dummy RTF tag
   NOSPECIFIC_DEST                     // dummy value: no destination set
} RTFTAGS;

/**********************************************************************/
/* Character types for use with AddToSegment function                 */
/**********************************************************************/
typedef enum _CHARTYPE
{
  CH_TAG,                              // character is part of a tag
  CH_YES,                              // character is translatable
  CH_NO                                // character itself is not translatable
} CHARTYPE;

/**********************************************************************/
/* Definition ot the RTF destination stack                            */
/**********************************************************************/
typedef struct _DESTSTACK
{
  BOOL   fTranslSegm;                  // segment-is-translatable flag
  BOOL   fField;                       // TRUE = processing field data
  BOOL   fProtected;
  CTRLTYPE rtfType;
  RTFTAGS rtfDest;                     // saved RTF destination
  RTFTAGS rtfCurDest;                  // currently active destination
} DESTSTACK, *PDESTSTACK;

/**********************************************************************/
/* Structure for a RTF control word entry                             */
/*                                                                    */
/* Note: the variable szCtrlWord must be the first variable in this   */
/* structure, as entries of type CTRLWORD are compared using the C    */
/* function strcmp.                                                   */
/**********************************************************************/
typedef struct _CTRLWORD
{
  CHAR      szCtrlWord[25];            // buffer for control word
  CTRLTYPE  Type;                      // type of control word
  USHORT    usValue;                   // value for control word
  CHAR_W    szCtrlWordW[25];           // UTF16 representation for control word
  RTFTAGS   rtfID;                     // control word identifier
} CTRLWORD, *PCTRLWORD;

/**********************************************************************/
/* The value for the following variable is computed by the            */
/* RTF user exit. Do not change the value manually!                   */
/**********************************************************************/
static USHORT usNumOfTags = 0;

CTRLWORD CtrlWords[] = {
// ctrlword    type      value translation                           description
{ "abswn",     INL_T,    0,    L"abswn",         ABSWN_RTFTAG         }, // absolute width of paragraph
{ "annotation",DEST_T,   0,    L"annotation",    ANNOTATION_RTFTAG    }, // annotation
{ "ansi",      INL_T,    0,    L"ansi",          ANSI_RTFTAG          }, // ANSI codepage
{ "atnauthor", INL_T,    0,    L"atnauthor",     ATNAUTHOR_RTFTAG     }, // annotation author
{ "atnid",     INL_T,    0,    L"atnid",         ATNID_RTFTAG         }, // annotation author id
{ "atnparent", DEST_T,   0,    L"atnparent",     ATNPARENT_RTFTAG     }, //
{ "atnref",    INL_T,    0,    L"atnref",        ATNREF_RTFTAG        }, // annotation reference
{ "atrfend",   INL_T,    0,    L"atrfend",       ATRFEND_RTFTAG       }, // ann. bookmk. end
{ "atrfstart", INL_T,    0,    L"atrfstart",     ATRFSTART_RTFTAG     }, // ann. bookmk. begin
{ "author",    DEST_T,   0,    L"author",        AUTHOR_RTFTAG        }, // document author
{ "b",         INL_T,    0,    L"b",             B_RTFTAG             }, // bold
{ "background",DEST_T,   0,    L"background",    BACKGROUND_RTFTAG    }, //
{ "bidioff",   INL_T,    0,    L"bidioff",       BIDIOFF_METATAG      }, // bidioff meta tag
{ "bidion",    INL_T,    0,    L"bidion",        BIDION_METATAG       }, // bidion meta tag
{ "bin",       INL_T,    1,    L"bin",           BIN_RTFTAG           }, // binary data
{ "bkmkend",   INL_T,    0,    L"bkmkend",       BKMKEND_RTFTAG       }, // end of bookmark
{ "bkmkstart", INL_T,    0,    L"bkmkstart",     BKMKSTART_RTFTAG     }, // start of bookmark
{ "blue",      INL_T,    1,    L"blue",          BLUE_RTFTAG          }, // blue color
{ "box",       INL_T,    0,    L"box",           BOX_RTFTAG           }, // box around paragraph
{ "brdrb",     INL_T,    0,    L"brdrb",         BRDRB_RTFTAG         }, // border bottom
{ "brdrbar",   INL_T,    0,    L"brdrbar",       BRDRBAR_RTFTAG       }, // border bar
{ "brdrdb",    INL_T,    0,    L"brdrdb",        BRDRDB_RTFTAG        }, // double border
{ "brdrdot",   INL_T,    0,    L"brdrdot",       BRDRDOT_RTFTAG       }, // dotted border
{ "brdrhair",  INL_T,    0,    L"brdrhair",      BRDRHAIR_RTFTAG      }, // hairline border
{ "brdrl",     INL_T,    0,    L"brdrl",         BRDRL_RTFTAG         }, // border left
{ "brdrr",     INL_T,    0,    L"brdrr",         BRDRR_RTFTAG         }, // border right
{ "brdrs",     INL_T,    0,    L"brdrs",         BRDRS_RTFTAG         }, // thin border
{ "brdrsh",    INL_T,    0,    L"brdrsh",        BRDRSH_RTFTAG        }, // shadow border
{ "brdrt",     INL_T,    0,    L"brdrt",         BRDRT_RTFTAG         }, // border top
{ "brdrth",    INL_T,    0,    L"brdrth",        BRDRTH_RTFTAG        }, // thick border
{ "brsp",      INL_T,    0,    L"brsp",          BRSP_RTFTAG          }, // space between border and object
{ "brspn",     INL_T,    0,    L"brspn",         BRSPN_RTFTAG         }, // space between border and object
{ "bullet",    CHAR_T,   0xFA, L"bullet",       BULLET_RTFTAG        }, // bullet character
{ "buptim",    INL_T,    0,    L"buptim",        BUPTIM_RTFTAG        }, // save time
{ "bxe",       INL_T,    0,    L"bxe",           BXE_RTFTAG           }, // bold index
{ "caps",      INL_T,    0,    L"caps",          CAPS_RTFTAG          }, // caps
{ "category",  DEST_T,   0,    L"category",      CATEGORY_RTFTAG      }, //
{ "cb",        INL_T,    0,    L"cb",            CB_RTFTAG            }, // background color
{ "cell",      BREAK_T,  0,    L"cell",          CELL_RTFTAG          }, // end of table cell
{ "cellx",     INL_T,    0,    L"cellx",         CELLX_RTFTAG         }, // shift right cell border
{ "cf",        INL_T,    0,    L"cf",            CF_RTFTAG            }, // foreground color
{ "chatn",     BREAK_T,  0,    L"chatn",         CHATN_RTFTAG         }, // annotation char
{ "chdate",    INL_T,    0,    L"chdate",        CHDATE_RTFTAG        }, // current date
{ "chftn",     INL_T,    0,    L"chftn",         CHFTN_RTFTAG         }, // footnote char
{ "chftnsep",  INL_T,    0,    L"chftnsep",      CHFTNSEP_RTFTAG      }, // char for footnote seper
{ "chftnsepc", INL_T,    0,    L"chftnsepc",     CHFTNSEPC_RTFTAG     }, // footnote continuation char
{ "chpgn",     INL_T,    0,    L"chpgn",         CHPGN_RTFTAG         }, // current page
{ "chtime",    INL_T,    0,    L"chtime",        CHTIME_RTFTAG        }, // current time
{ "clbrdrb",   INL_T,    0,    L"clbrdrb",       CLBRDRB_RTFTAG       }, // lower table cell border
{ "clbrdrl",   INL_T,    0,    L"clbrdrl",       CLBRDRL_RTFTAG       }, // left table cell border
{ "clbrdrr",   INL_T,    0,    L"clbrdrr",       CLBRDRR_RTFTAG       }, // right table cell border
{ "clbrdrt",   INL_T,    0,    L"clbrdrt",       CLBRDRT_RTFTAG       }, // upper table cell border
{ "clmgf",     INL_T,    0,    L"clmgf",         CLMGF_RTFTAG         }, // first cell in table area
{ "clmrg",     INL_T,    0,    L"clmrg",         CLMRG_RTFTAG         }, // cell is concatenated to prev. cell
{ "colorschememapping", DEST_T,0,L"colorschememapping", COLORSCHEMEMAPPING_RTFTAG   }, // color sheme mapping
{ "colortbl",  DEST_T,   0,    L"colortbl",      COLORTBL_RTFTAG      }, // color table
{ "cols",      INL_T,    0,    L"cols",          COLS_RTFTAG          }, // number of columns
{ "colsx",     INL_T,    0,    L"colsx",         COLSX_RTFTAG         }, // distance between columns
{ "column",    INL_T,    0,    L"column",        COLUMN_RTFTAG        }, // required column wrap
{ "comment",   DEST_T,   0,    L"comment",       COMMENT_RTFTAG       }, // document comment
{ "company",   DEST_T,   0,    L"company",       COMPANY_RTFTAG       }, //
{ "creatim",   INL_T,    0,    L"creatim",       CREATIM_RTFTAG       }, // creation time
{ "datafield", FINL_T,   0,    L"datafield",     DATAFLD_RTFTAG       }, // datafield with hex data
{ "datastore", DEST_T,  0,     L"datastore",     DATASTORE_RTFTAG     }, // datastore
{ "deff",      INL_T,    0,    L"deff",          DEFF_RTFTAG          }, // default font
{ "defformat", INL_T,    0,    L"defformat",     DEFFORMAT_RTFTAG     }, // default format
{ "deftab",    INL_T,    0,    L"deftab",        DEFTAB_RTFTAG        }, // default tabstops
{ "deleted",   DEST_T,   0,    L"deleted",       DELETED_RTFTAG       }, // deleted mark (revisions)
{ "dibitmap",  INL_T,    0,    L"dibitmap",      DIBITMAP_RTFTAG      }, // bitmap format
{ "dn",        INL_T,    0,    L"dn",            DN_RTFTAG            }, // shift down
{ "doccomm",   DEST_T,   0,    L"doccomm",       DOCCOMM_RTFTAG       }, // document comments for info
{ "docvar",    DEST_T,   0,    L"docvar",        DOCVAR_RTFTAG        }, // document variable
{ "dxfrtextn", INL_T,    0,    L"dxfrtextn",     DXFRTEXTN_RTFTAG     }, // horizontal space
{ "dy",        INL_T,    0,    L"dy",            DY_RTFTAG            }, // day
{ "edmins",    INL_T,    0,    L"edmins",        EDMINS_RTFTAG        }, // processing time
{ "emdash",    CHAR_T,   0xC4, L"emdash",        EMDASH_RTFTAG        }, // long dash character
{ "endash",    CHAR_T,   0x2D, L"endash",        ENDASH_RTFTAG        }, // short dash character
{ "endnhere",  INL_T,    0,    L"endnhere",      ENDNHERE_RTFTAG      }, // end notes belong to section
{ "endnotes",  INL_T,    0,    L"endnotes",      ENDNOTES_RTFTAG      }, // footnotes at end of document
{ "expnd",     INL_T,    0,    L"expnd",         EXPND_RTFTAG         }, // compression of inter character space
{ "f",         INL_T,    1,    L"f",             F_RTFTAG             }, // font number
{ "facingp",   INL_T,    0,    L"facingp",       FACINGP_RTFTAG       }, // facing pages
{ "factoidname", INL_T,  0,    L"factoidname",   FACTOIDNAME_RTFTAG   }, 
{ "fchars",    DEST_T,   0,    L"fchars",        FCHARS_RTFTAG        }, // fchars
{ "fdecor",    INL_T,    0,    L"fdecor",        FDECOR_RTFTAG        }, // decorative fonts
{ "ffdeftext", DEST_T,   0,    L"ffdeftext",     FFDEFTEXT_RTFTAG     }, //
{ "ffentrymcr",DEST_T,   0,    L"ffentrymcr",    FFENTRYMCR_RTFTAG    }, //
{ "ffexitmcr", DEST_T,   0,    L"ffexitmcr",     FFEXITMCR_RTFTAG     }, //
{ "ffformat",  DEST_T,   0,    L"ffformat",      FFFORMAT_RTFTAG      }, //
{ "ffhelptext",DEST_T,   0,    L"ffhelptext",    FFHELPTEXT_RTFTAG    }, //
{ "ffl",       DEST_T,   0,    L"ffl",           FFL_RTFTAG           }, //
{ "ffname",    DEST_T,   0,    L"ffname",        FFNAME_RTFTAG        }, //
{ "ffstattext",DEST_T,   0,    L"ffstattext",    FFSTATTEXT_RTFTAG    }, //
{ "fi",        INL_T,    0,    L"fi",            FI_RTFTAG            }, // indention of first line
{ "field",     FINL_T,   0,    L"field",         FIELD_RTFTAG         }, // field identifier
{ "flddirty",  INL_T,    0,    L"flddirty",      FLDDIRTY_RTFTAG      }, // field has been changed
{ "fldedit",   FINL_T,   0,    L"fldedit",       FLDEDIT_RTFTAG       }, // field text has been changed
{ "fldinst",   FINL_T,   0,    L"fldinst",       FLDINST_RTFTAG       }, // field instructions
{ "fldlock",   INL_T,    0,    L"fldlock",       FLDLOCK_RTFTAG       }, // field is locked
{ "fldpriv",   INL_T,    0,    L"fldpriv",       FLDPRIV_RTFTAG       }, // field cannot be displayed
{ "fldrslt",   FINL_T,   1,    L"fldrslt",       FLDRSLT_RTFTAG       }, // result of field
{ "fldtype",   DEST_T,   0,    L"fldtype",       FLDTYPE_RTFTAG       }, //
{ "fmodern",   INL_T,    0,    L"fmodern",       FMODERN_RTFTAG       }, // courier/elite/pica font
{ "fname",     DEST_T,   0,    L"fname",         FNAME_RTFTAG         }, //
{ "fnil",      INL_T,    0,    L"fnil",          FNIL_RTFTAG          }, // unknown font
{ "fonttbl",   DEST_T,   0,    L"fonttbl",       FONTTBL_RTFTAG       }, // font table
{ "footer",    DEST_T,   1,    L"footer",        FOOTER_RTFTAG        }, // footer
{ "footerf",   INL_T,    0,    L"footerf",       FOOTERF_RTFTAG       }, // footer for first page
{ "footerl",   INL_T,    0,    L"footerl",       FOOTERL_RTFTAG       }, // footer for left pages
{ "footerr",   INL_T,    0,    L"footerr",       FOOTERR_RTFTAG       }, // footer for right pages
{ "footerx",   INL_T,    0,    L"footerx",       FOOTERX_RTFTAG       }, // footer line x position
{ "footery",   INL_T,    0,    L"footery",       FOOTERY_RTFTAG       }, // footer line y position
{ "footnote",  INL_T,    0,    L"footnote",      FOOTNOTE_RTFTAG      }, // footnote
{ "formfield", DEST_T,   0,    L"formfield",     FORMFIELD_RTFTAG     }, //
{ "fracwidth", INL_T,    0,    L"fracwidth",     FRACWIDTH_RTFTAG     }, //
{ "froman",    INL_T,    0,    L"froman",        FROMAN_RTFTAG        }, // roman font
{ "fs",        INL_T,    0,    L"fs",            FS_RTFTAG            }, // font size
{ "fscript",   INL_T,    0,    L"fscript",       FSCRIPT_RTFTAG       }, // script/kursiv font
{ "fswiss",    INL_T,    0,    L"fswiss",        FSWISS_RTFTAG        }, // swiss font
{ "ftech",     INL_T,    0,    L"ftech",         FTECH_RTFTAG         }, // technical fonts
{ "ftnbj",     INL_T,    0,    L"ftnbj",         FTNBJ_RTFTAG         }, // footnotes at end of page
{ "ftncn",     INL_T,    0,    L"ftncn",         FTNCN_RTFTAG         }, // continued footnotes
{ "ftnrestart",INL_T,    0,    L"ftnrestart",    FTNRESTART_RTFTAG    }, // renumber of footnotes
{ "ftnsep",    INL_T,    0,    L"ftnsep",        FTNSEP_RTFTAG        }, // seperates footnote from document
{ "ftnsepc",   INL_T,    0,    L"ftnsepc",       FTNSEPC_RTFTAG       }, // seperates footnotes
{ "ftnstartn", INL_T,    0,    L"ftnstartn",     FTNSTARTN_RTFTAG     }, // start number for footnotes
{ "ftntj",     INL_T,    0,    L"ftntj",         FTNTJ_RTFTAG         }, // footnotes under text
{ "g",         DEST_T,   0,    L"g",             G_RTFTAG             }, //
{ "generator", DEST_T,   0,    L"generator",     GENERATOR_RTFTAG     }, //
{ "green",     INL_T,    1,    L"green",         GREEN_RTFTAG         }, // green color
{ "gridtbl",   DEST_T,   0,    L"gridtbl",       GRIDTBL_RTFTAG       }, //
{ "gutter",    INL_T,    0,    L"gutter",        GUTTER_RTFTAG        }, //
{ "header",    DEST_T,   1,    L"header",        HEADER_RTFTAG        }, // header
{ "headerf",   INL_T,    0,    L"headerf",       HEADERF_RTFTAG       }, // header for first page
{ "headerl",   INL_T,    0,    L"headerl",       HEADERL_RTFTAG       }, // header for left pages
{ "headerr",   INL_T,    0,    L"headerr",       HEADERR_RTFTAG       }, // header for right pages
{ "headerx",   INL_T,    0,    L"headerx",       HEADERX_RTFTAG       }, // header line x position
{ "headery",   INL_T,    0,    L"headery",       HEADERY_RTFTAG       }, // header line y position
{ "hr",        INL_T,    0,    L"hr",            HR_RTFTAG            }, // hour
{ "hyphhotz",  INL_T,    0,    L"hyphhotz",      HYPHHOTZ_RTFTAG      }, // hyphenation zone
{ "i",         INL_T,    0,    L"i",             I_RTFTAG             }, // italic
{ "id",        INL_T,    0,    L"id",            ID_RTFTAG            }, // internal ID
{ "info",      DEST_T,   0,    L"info",          INFO_RTFTAG          }, // information group
{ "intbl",     INL_T,    0,    L"intbl",         INTBL_RTFTAG         }, // paragraph is part of a table
{ "ixe",       INL_T,    0,    L"ixe",           IXE_RTFTAG           }, // italic index
{ "keep",      INL_T,    0,    L"keep",          KEEP_RTFTAG          }, // keep paragraph together
{ "keepn",     INL_T,    0,    L"keepn",         KEEPN_RTFTAG         }, // keep together with following par.
{ "keywords",  DEST_T,   0,    L"keywords",      KEYWORDS_RTFTAG      }, // document keywords
{ "landscape", INL_T,    0,    L"landscape",     LANDSCAPE_RTFTAG     }, // landscape orientation
{ "latentstyles",DEST_T, 0,    L"latentstyles",  LATENTSTYLES_RTFTAG  }, //
{ "lchars",    DEST_T,   0,    L"lchars",        LCHARS_RTFTAG        }, // lchars
{ "ldblquote", CHAR_T,   0x22, L"ldblquote",      LDBLQUOTE_RTFTAG     }, // left double quote
{ "levelnumbers", INL_T, 0,    L"levelnumbers",  LEVELNUMBERS_RTFTAG  },
{ "leveltext", INL_T,    0,    L"leveltext",     LEVELTEXT_RTFTAG     },
{ "li",        INL_T,    0,    L"li",            LI_RTFTAG            }, // left indention
{ "line",      INL_T,    0,    L"line",          LINE_RTFTAG          }, // required line wrap
{ "linebetcol",INL_T,    0,    L"linebetcol",    LINEBETCOL_RTFTAG    }, // line between columns
{ "linecont",  INL_T,    0,    L"linecont",      LINECONT_RTFTAG      }, // line numbers are continuous
{ "linemod",   INL_T,    0,    L"linemod",       LINEMOD_RTFTAG       }, // line number incremtention
{ "lineppage", INL_T,    0,    L"lineppage",     LINEPPAGE_RTFTAG     }, // line number are restarted on page
{ "linerestarts",INL_T,  0,    L"linerestarts",  LINERESTARTS_RTFTAG  }, // line numbers are restarted
{ "linestartn",INL_T,    0,    L"linestartn",    LINESTARTN_RTFTAG    }, // start number for lines
{ "linestarts",INL_T,    0,    L"linestarts",    LINESTARTS_RTFTAG    }, // line start number
{ "linex",     INL_T,    0,    L"linex",         LINEX_RTFTAG         }, // line number x position
{ "listname",  INL_T,    0,    L"listname",      LISTNAME_RTFTAG      },
{ "listoverridetable",DEST_T, 0,    L"listoverridetable",  LISTOVERRIDETABLE_RTFTAG       }, //
{ "listtable", DEST_T,   0,    L"listtable",     LISTTABLE_RTFTAG     }, //
{ "listtext",  DEST_T,   0,    L"listtext",      LISTTEXT_RTFTAG      },
{ "ltrch",     INL_T,    0,    L"ltrch",         LTRCH_RTFTAG         }, // left to right run
{ "ltrpar",    BREAK_T,  0,    L"ltrpar",        LTRPAR_RTFTAG        }, // end of paragraph (left to right)
{ "lquote",    CHAR_T,   0x27, L"lquote",        LQUOTE_RTFTAG        }, // left single quote
{ "lsdlockedexcept",DEST_T,0,  L"lsdlockedexcept", LSDLOCKEDEXCEPT_RTFTAG }, //
{ "mac",       INL_T,    0,    L"mac",           MAC_RTFTAG           }, // Apple macintosh codepage
{ "macpict",   INL_T,    0,    L"macpict",       MACPICT_RTFTAG       }, // quickdraw format
{ "makebackup",INL_T,    0,    L"makebackup",    MAKEBACKUP_RTFTAG    }, // create backup file
{ "manager",   DEST_T,   0,    L"manager",       MANAGER_RTFTAG       }, //
{ "margb",     INL_T,    0,    L"margb",         MARGB_RTFTAG         }, // bottom margin
{ "margl",     INL_T,    0,    L"margl",         MARGL_RTFTAG         }, // left margin
{ "margmirror",INL_T,    0,    L"margmirror",    MARGMIRROR_RTFTAG    }, // switches left and right margins
{ "margr",     INL_T,    0,    L"margr",         MARGR_RTFTAG         }, // right margin
{ "margt",     INL_T,    0,    L"margt",         MARGT_RTFTAG         }, // top margin
{ "min",       INL_T,    0,    L"min",           MIN_RTFTAG           }, // minutes
{ "mo",        INL_T,    0,    L"mo",            MO_RTFTAG            }, // month
{ "nesttableprops",DEST_T,  0, L"nesttableprops",NESTTABLEPROPS_RTFTAG}, //
{ "nextfile",  INL_T,    0,    L"nextfile",      NEXTFILE_RTFTAG      }, //
{ "nofchars",  INL_T,    0,    L"nofchars",      NOFCHARS_RTFTAG      }, // number of characters
{ "nofpages",  INL_T,    0,    L"nofpages",      NOFPAGES_RTFTAG      }, // number of pages
{ "nofwords",  INL_T,    0,    L"nofwords",      NOFWORDS_RTFTAG      }, // number of words
{ "nonesttables",DEST_T, 0,    L"nonesttables",  NONESTTABLES_RTFTAG  }, //
{ "noproof",   INL_T,    0,    L"noproof",       NOPROOF_RTFTAG       }, // no proofing
{ "noline",    INL_T,    0,    L"noline",        NOLINE_RTFTAG        }, // no line numbering
{ "objclass",  DEST_T,   0,    L"objclass",      OBJCLASS_RTFTAG      }, //
{ "objdata",   DEST_T,   0,    L"objdata",       OBJDATA_RTFTAG       }, //
{ "oldcprops", DEST_T,   0,    L"oldcprops",     OLDCPROPS_RTFTAG     }, //
{ "oldpprops", DEST_T,   0,    L"oldpprops",     OLDPPROPS_RTFTAG     }, //
{ "oldsprops", DEST_T,   0,    L"oldsprops",     OLDSPROPS_RTFTAG     }, //
{ "oldtprops", DEST_T,   0,    L"oldtprops",     OLDTPROPS_RTFTAG     }, //
{ "operator",  DEST_T,   0,    L"operator",      OPERATOR_RTFTAG      }, // document operator
{ "outl",      INL_T,    0,    L"outl",          OUTL_RTFTAG          }, // outlined
{ "page",      INL_T,    0,    L"page",          PAGE_RTFTAG          }, // required pagination
{ "pagebb",    INL_T,    0,    L"pagebb",        PAGEBB_RTFTAG        }, // page break before paragraph
{ "panose",                 DEST_T,  0, L"panose",                       PANOSE_RTFTAG },                     //
{ "paperh",    INL_T,    0,    L"paperh",        PAPERH_RTFTAG        }, // height of paper
{ "paperw",    INL_T,    0,    L"paperw",        PAPERW_RTFTAG        }, // width of paper
{ "par",       BREAK_T,  0,    L"par",           PAR_RTFTAG           }, // end of paragraph
{ "pard",      INL_T,    0,    L"pard",          PARD_RTFTAG          }, // reset paragraph properties
{ "pc",        INL_T,    0,    L"pc",            PC_RTFTAG            }, // IBM PC codepage
{ "pca",       INL_T,    0,    L"pca",           PCA_RTFTAG           }, // IBM PC codepage 850
{ "pgncont",   INL_T,    0,    L"pgncont",       PGNCONT_RTFTAG       }, // continuous page numbering
{ "pgndec",    INL_T,    0,    L"pgndec",        PGNDEC_RTFTAG        }, // decimal page numbers
{ "pgnlcltr",  INL_T,    0,    L"pgnlcltr",      PGNLCLTR_RTFTAG      }, // uppercase character page numbers
{ "pgnlcrm",   INL_T,    0,    L"pgnlcrm",       PGNLCRM_RTFTAG       }, // lowercase roman page numbers
{ "pgnrestart",INL_T,    0,    L"pgnrestart",    PGNRESTART_RTFTAG    }, // seperate page numbering
{ "pgnstartn", INL_T,    0,    L"pgnstartn",     PGNSTARTN_RTFTAG     }, // start number for pages
{ "pgnstarts", INL_T,    0,    L"pgnstarts",     PGNSTARTS_RTFTAG     }, // section starting page number
{ "pgnucltr",  INL_T,    0,    L"pgnucltr",      PGNUCLTR_RTFTAG      }, // uppercase character page numbers
{ "pgnucrm",   INL_T,    0,    L"pgnucrm",       PGNUCRM_RTFTAG       }, // uppercase roman page numbers
{ "pgnx",      INL_T,    0,    L"pgnx",          PGNX_RTFTAG          }, // x position of page number
{ "pgny",      INL_T,    0,    L"pgny",          PGNY_RTFTAG          }, // y position of page number
{ "pgp",       DEST_T,   0,    L"pgp",           PGP_RTFTAG           }, //
{ "pgptbl",    DEST_T,   0,    L"pgptbl",        PGPTBL_RTFTAG        }, //
{ "phcol",     INL_T,    0,    L"phcol",         PHCOL_RTFTAG         }, // pos. hor. rel. to column
{ "phmrg",     INL_T,    0,    L"phmrg",         PHMRG_RTFTAG         }, // pos. hor. rel. to border
{ "phpg",      INL_T,    0,    L"phpg",          PHPG_RTFTAG          }, // pos. hor. rel to page
{ "piccropb",  INL_T,    1,    L"piccropb",      PICCROPB_RTFTAG      }, // bottom cropt value
{ "piccropl",  INL_T,    1,    L"piccropl",      PICCROPL_RTFTAG      }, // left cropt value
{ "piccropr",  INL_T,    1,    L"piccropr",      PICCROPR_RTFTAG      }, // right cropt value
{ "piccropt",  INL_T,    1,    L"piccropt",      PICCROPT_RTFTAG      }, // top cropt value
{ "pich",      INL_T,    1,    L"pich",          PICH_RTFTAG          }, // picture height
{ "pichgoal",  INL_T,    1,    L"pichgoal",      PICHGOAL_RTFTAG      }, // pic height (goal)
{ "picprop",   DEST_T,   0,    L"picprop",       PICPROP_RTFTAG       }, //
{ "picscaled", INL_T,    0,    L"picscaled",     PICSCALED_RTFTAG     }, // normalized picture
{ "picscalex", INL_T,    1,    L"picscalex",     PICSCALEX_RTFTAG     }, // horizontal scale
{ "picscaley", INL_T,    1,    L"picscaley",     PICSCALEY_RTFTAG     }, // vertival scale
{ "pict",      DEST_T,   0,    L"pict",          PICT_RTFTAG          }, // pictures
{ "picw",      INL_T,    1,    L"picw",          PICW_RTFTAG          }, // picture width
{ "picwgoal",  INL_T,    1,    L"picwgoal",      PICWGOAL_RTFTAG      }, // pic width (goal)
{ "plain",     INL_T,    0,    L"plain",         PLAIN_RTFTAG         }, // reset all character formats
{ "posx",      INL_T,    0,    L"posx",          POSX_RTFTAG          }, // absolute x position of object
{ "pn",        DEST_T,   0,    L"pn",            PN_RTFTAG            }, // TK
{ "pnseclvl",  DEST_T,   0,    L"pnseclvl",      PNSECLVL_RTFTAG      }, //
{ "pntxta",    INL_T,    0,    L"pntxta",        PNTXTA_RTFTAG        }, //
{ "pntxtb",    INL_T,    0,    L"pntxtb",        PNTXTB_RTFTAG        }, //
{ "posxc",     INL_T,    0,    L"posxc",         POSXC_RTFTAG         }, // centered position
{ "posxi",     INL_T,    0,    L"posxi",         POSXI_RTFTAG         }, // horizontal position
{ "posxl",     INL_T,    0,    L"posxl",         POSXL_RTFTAG         }, // left aligned position
{ "posxo",     INL_T,    0,    L"posxo",         POSXO_RTFTAG         }, // outside position
{ "posxr",     INL_T,    0,    L"posxr",         POSXR_RTFTAG         }, // right aligned position
{ "posy",      INL_T,    0,    L"posy",          POSY_RTFTAG          }, // absolute y position
{ "posyb",     INL_T,    0,    L"posyb",         POSYB_RTFTAG         }, // bottom position
{ "posyc",     INL_T,    0,    L"posyc",         POSYC_RTFTAG         }, // vertical centered position
{ "posyil",    INL_T,    0,    L"posyil",        POSYIL_RTFTAG        }, // vertical line aligned position
{ "posyt",     INL_T,    0,    L"posyt",         POSYT_RTFTAG         }, // top position
{ "printim",   INL_T,    0,    L"printim",       PRINTIM_RTFTAG       }, // print time
{ "private",   DEST_T,   0,    L"private",       PRIVATE_RTFTAG       }, //
{ "protend",   DEST_T,   0,    L"protend",       PROTEND_RTFTAG       }, //
{ "protstart", DEST_T,   0,    L"protstart",     PROTSTART_RTFTAG     }, //
{ "protusertbl", DEST_T, 0,    L"protusertbl",   PROTUSERTBL_RTFTAG   }, //
{ "pvmrg",     INL_T,    0,    L"pvmrg",         PVMRG_RTFTAG         }, // pos. vertical relative to border
{ "pvpg",      INL_T,    0,    L"pvpg",          PVPG_RTFTAG          }, // pos. vert. rel. to page
{ "pxe",       DEST_T,   0,    L"pxe",           PXE_RTFTAG           }, //
{ "qc",        INL_T,    0,    L"qc",            QC_RTFTAG            }, // centered
{ "qj",        INL_T,    0,    L"qj",            QJ_RTFTAG            }, // left and right adjusted
{ "ql",        INL_T,    0,    L"ql",            QL_RTFTAG            }, // left aligned
{ "qr",        INL_T,    0,    L"qr",            QR_RTFTAG            }, // right aligned
{ "rdblquote", CHAR_T,   0x22, L"rdblquote",     RDBLQUOTE_RTFTAG     }, // right double quote
{ "red",       INL_T,    1,    L"red",           RED_RTFTAG           }, // red color
{ "revbar",    INL_T,    0,    L"revbar",        REVBAR_RTFTAG        }, // type of revision bar
{ "revised",   INL_T,    0,    L"revised",       REVISED_RTFTAG       }, // revision mark
{ "revisions", INL_T,    0,    L"revisions",     REVISIONS_RTFTAG     }, // start revision marks
{ "revprop",   INL_T,    0,    L"revprop",       REVPROP_RTFTAG       }, // display of revised text
{ "revtbl",    DEST_T,   0,    L"revtbl",        REVTBL_RTFTAG        }, // revision table
{ "revtim",    INL_T,    0,    L"revtim",        REVTIM_RTFTAG        }, // revision time
{ "ri",        INL_T,    0,    L"ri",            RI_RTFTAG            }, // right indention
{ "row",       BREAK_T,  0,    L"row",           ROW_RTFTAG           }, // end of table row
{ "rquote",    CHAR_T,   0x27, L"rquote",        RQUOTE_RTFTAG        }, // right single quote
{ "rsidtbl",   DEST_T,   0,    L"rsidtbl",       RSIDTBL_RTFTAG       }, //
{ "rtf",       DEST_T,   1,    L"rtf",           RTF_RTFTAG           }, // RTF version
{ "rtlch",     INL_T,    0,    L"rtlch",         RTLCH_RTFTAG         }, // right to left run bt 310101
{ "rtlpar",    BREAK_T,  0,    L"rtlpar",        RTLPAR_RTFTAG        }, // end of paragraph (right to left)
{ "rxe",       INL_T,    0,    L"rxe",           RXE_RTFTAG           }, // text mark instead of page number
{ "s",         INL_T,    0,    L"s",             S_RTFTAG             }, // print format
{ "sa",        INL_T,    0,    L"sa",            SA_RTFTAG            }, // space after
{ "sb",        INL_T,    0,    L"sb",            SB_RTFTAG            }, // space before
{ "sbasedon",  INL_T,    1,    L"sbasedon",      SBASEDON_RTFTAG      }, // style is based on n
{ "sbkcol",    INL_T,    0,    L"sbkcol",        SBKCOL_RTFTAG        }, // section break starts new column
{ "sbkeven",   INL_T,    0,    L"sbkeven",       SBKEVEN_RTFTAG       }, // start section on even page
{ "sbknone",   INL_T,    0,    L"sbknone",       SBKNONE_RTFTAG       }, // no section break
{ "sbkpage",   INL_T,    0,    L"sbkpage",       SBKPAGE_RTFTAG       }, // section break starts new page
{ "sbys",      INL_T,    0,    L"sbys",          SBYS_RTFTAG          }, // side-by-side paragraphs
{ "scaps",     INL_T,    0,    L"scaps",         SCAPS_RTFTAG         }, // small caps
{ "sect",      BREAK_T,  0,    L"sect",          SECT_RTFTAG          }, // end of section/paragraph
{ "sectd",     INL_T,    0,    L"sectd",         SECTD_RTFTAG         }, // default section layout
{ "shad",      INL_T,    0,    L"shad",          SHAD_RTFTAG          }, // shaded
{ "shp",       DEST_T,   0,    L"shp",           SHP_RTFTAG           }, // shapes
{ "shpgrp",    DEST_T,   0,    L"shpgrp",        SHPGRP_RTFTAG        }, // shapegroup (alternatively for shapes)
{ "shppict",   DEST_T,   0,    L"shppict",       SHPPICT_RTFTAG       }, //
{ "shptxt",    DEST_T,   1,    L"shptxt",        SHPTXT_RTFTAG        }, // shaped text
{ "skbodd",    INL_T,    0,    L"skbodd",        SKBODD_RTFTAG        }, // start sectionon odd page
{ "sl",        INL_T,    0,    L"sl",            SL_RTFTAG            }, // space between lines
{ "snext",     INL_T,    1,    L"snext",         SNEXT_RTFTAG         }, // next style sheet
{ "strike",    INL_T,    0,    L"strike",        STRIKE_RTFTAG        }, // striked out
{ "stylesheet",DEST_T,   0,    L"stylesheet",    STYLESHEET_RTFTAG    }, // style sheet
{ "subject",   DEST_T,   0,    L"subject",       SUBJECT_RTFTAG       }, // document subject
{ "svb",       DEST_T,   0,    L"svb",           SVB_RTFTAG           }, //
{ "tab",       BREAK_T,  0,    L"tab",           TAB_RTFTAG           }, // tab character
{ "tb",        INL_T,    0,    L"tb",            TB_RTFTAG            }, // vertical line tabstop
{ "tc",        INL_T,    0,    L"tc",            TC_RTFTAG            }, // table of contents  // changed 31052001 bt
{ "tcf",       INL_T,    0,    L"tcf",           TCF_RTFTAG           }, // type of table of contents
{ "tcl",       INL_T,    0,    L"tcl",           TCL_RTFTAG           }, // table of contents level
{ "template",  DEST_T,   0,    L"template",      TEMPLATE_RTFTAG      }, // name of template file
{ "themedata", DEST_T,   0,    L"themedata",     THEMEDATA_RTFTAG    }, // theme data
{ "title",     DEST_T,   0,    L"title",         TITLE_RTFTAG         }, // document title
{ "titlepg",   INL_T,    0,    L"titlepg",       TITLEPG_RTFTAG       }, // title page has special format
{ "tldot",     INL_T,    0,    L"tldot",         TLDOT_RTFTAG         }, // tab fill character: dots
{ "tlhyph",    INL_T,    0,    L"tlhyph",        TLHYPH_RTFTAG        }, // tab fill character: hyphens
{ "tlth",      INL_T,    0,    L"tlth",          TLTH_RTFTAG          }, // tab fill character: thick line
{ "tlul",      INL_T,    0,    L"tlul",          TLUL_RTFTAG          }, // tab fill character: underlines
{ "tqc",       INL_T,    0,    L"tqc",           TQC_RTFTAG           }, // centered tabstop
{ "tqdec",     INL_T,    0,    L"tqdec",         TQDEC_RTFTAG         }, // decimal tabstop
{ "tqr",       INL_T,    0,    L"tqr",           TQR_RTFTAG           }, // right aligned tabstop
{ "trgaph",    INL_T,    0,    L"trgaph",        TRGAPH_RTFTAG        }, // space between cells
{ "trleft",    INL_T,    0,    L"trleft",        TRLEFT_RTFTAG        }, // left position of table
{ "trowd",     INL_T,    0,    L"trowd",         TROWD_RTFTAG         }, // default values for table
{ "trqc",      INL_T,    0,    L"trqc",          TRQC_RTFTAG          }, // center table line
{ "trql",      INL_T,    0,    L"trql",          TRQL_RTFTAG          }, // left align table line
{ "trqr",      INL_T,    0,    L"trqr",          TRQR_RTFTAG          }, // right align table line
{ "trrhn",     INL_T,    0,    L"trrhn",         TRRHN_RTFTAG         }, // height of table line
{ "tx",        INL_T,    0,    L"tx",            TX_RTFTAG            }, // tabstop position
{ "txe",       INL_T,    0,    L"txe",           TXE_RTFTAG           }, // text instead of page number
{ "u",         INL_T,    0,    L"u",             U_RTFTAG             }, // single unicode character
{ "ud",        DEST_T,   1,    L"ud",            UD_RTFTAG            }, // unicode stream
{ "ul",        INL_T,    0,    L"ul",            UL_RTFTAG            }, // underligned
{ "uld",       INL_T,    0,    L"uld",           ULD_RTFTAG           }, // dotted underligned
{ "uldb",      INL_T,    0,    L"uldb",          ULDB_RTFTAG          }, // double underlign
{ "ulnone",    INL_T,    0,    L"ulnone",        ULNONE_RTFTAG        }, // stops all underlignments
{ "ulw",       INL_T,    0,    L"ulw",           ULW_RTFTAG           }, // word underligned
{ "up",        INL_T,    0,    L"up",            UP_RTFTAG            }, // shift up
{ "upr",       DEST_T,   0,    L"upr",           UPR_RTFTAG           }, // unicode destination with Ansi and Unicode stream                    //
{ "userprops", DEST_T,   0,    L"userprops",     USERPROPS_RTFTAG     },           
{ "v",         INL_T,    0,    L"v",             V_RTFTAG             }, // hidden text
{ "vern",      DEST_T,   0,    L"doccomm",       VERN_RTFTAG          }, // internal version number
{ "version",   DEST_T,   0,    L"version",       VERSION_RTFTAG       }, // document version
{ "vertalb",   INL_T,    0,    L"vertalb",       VERTALB_RTFTAG       }, // text is bottom aligned
{ "vertalc",   INL_T,    0,    L"vertalc",       VERTALC_RTFTAG       }, // text is centerred vertically
{ "vertalj",   INL_T,    0,    L"vertalj",       VERTALJ_RTFTAG       }, // text is adjusted vertically
{ "vertalt",   INL_T,    0,    L"vertalt",       VERTALT_RTFTAG       }, // text is top aligned
{ "wbitmap",   INL_T,    1,    L"wbitmap",       WBITMAP_RTFTAG       }, // window bitmap
{ "wbmbitspixel", INL_T, 1,    L"wbmbitspixel",  WBMBITSPIXEL_RTFTAG  }, // bits per pixel
{ "wbmplanes", INL_T,    1,    L"wbmplanes",     WBMPLANES_RTFTAG     }, // planes
{ "wbmwidthbytes", INL_T,1,    L"wbmwidthbytes", WBMWIDTHBYTES_RTFTAG }, // bitmap width in bytes
{ "widowctrl", INL_T,    0,    L"widowctrl",     WIDOWCTRL_RTFTAG     }, // widow control
{ "wmetafile", INL_T,    1,    L"wmetafile",     WMETAFILE_RTFTAG     }, // window metafile
{ "xe",        INL_T,    0,    L"xe",            XE_RTFTAG            }, // index entry
{ "yr",        INL_T,    0,    L"yr",            YR_RTFTAG            }, // year
{ "",    NOSPEC_TYPE,    0,    L"",              NOSPECIFIC_DEST      }, // terminates the list
};

/**********************************************************************/
/* Translatable data recognition table (all characters are mapped to  */
/* to CH_TRANS, not translatable info is mapped to CH_NOTRANS)        */
/**********************************************************************/
CHARTYPE TranslChar[256] =
//          0       1       2       3       4       5       6       7       8       9
{/*  0 */   CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,
 /*  1 */   CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,
 /*  2 */   CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,
 /*  3 */   CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_YES, CH_YES, CH_NO,
 /*  4 */   CH_NO,  CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_NO,  CH_YES, CH_YES,
 /*  5 */   CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_NO,  CH_NO,
 /*  6 */   CH_NO,  CH_YES, CH_NO,  CH_NO,  CH_NO,  CH_YES, CH_YES, CH_YES, CH_YES, CH_YES,
 /*  7 */   CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES,
 /*  8 */   CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES,
 /*  9 */   CH_YES, CH_YES, CH_NO,  CH_YES, CH_NO,  CH_YES, CH_NO,  CH_YES, CH_YES, CH_YES,
 /* 10 */   CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES,
 /* 11 */   CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES,
 /* 12 */   CH_YES, CH_YES, CH_YES, CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_YES, CH_YES,
 /* 13 */   CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES,
 /* 14 */   CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES,
 /* 15 */   CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES,
 /* 16 */   CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_NO,  CH_NO,  CH_NO,  CH_NO,
 /* 17 */   CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,
 /* 18 */   CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,
 /* 19 */   CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,
 /* 20 */   CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,
 /* 21 */   CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,
 /* 22 */   CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES,
 /* 23 */   CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES, CH_YES,
 /* 24 */   CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,
 /* 25 */   CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO,  CH_NO };

/**********************************************************************/
/* Segmenting information recognition table (all characters are mapped*/
/* to 'x' except characters which have an influence on the            */
/* morphological recognition of sentences)                            */
/**********************************************************************/
BYTE chSegChar[256] =
//          0    1    2    3    4    5    6    7    8    9
{/*  0 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /*  1 */  10, 'X', 'X',  13, 'X', 'X', 'X', 'X', 'X', 'X',
 /*  2 */ 'X',  21, 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /*  3 */ 'X', 'X',  32,  33,  34,  35,  36,  36,  37,  39,
 /*  4 */  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,
 /*  5 */  50,  51,  52,  53,  54,  55,  56,  57,  58,  59,
 /*  6 */  60,  61,  62,  63, 'X', 'X', 'X', 'X', 'X', 'X',
 /*  7 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /*  8 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /*  9 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 10 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 11 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 12 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 13 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 14 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 15 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 16 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 17 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 18 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 19 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 20 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 21 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 22 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 23 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 24 */ 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X', 'X',
 /* 25 */ 'X', 'X', 'X', 'X', 'X', 'X' };

/**********************************************************************/
/* special character table for the conversion into RTF format         */
/*                                                                    */
/*                                                                    */
/**********************************************************************/
PSZ  SpecialChars[256] = {
/* 00   */ NULL,
/* 01   */ NULL,
/* 02   */ NULL,
/* 03   */ NULL,
/* 04   */ NULL,
/* 05   */ NULL,
/* 06   */ NULL,
/* 07   */ NULL,
/* 08   */ NULL,
/* 09   */ NULL,
/* 0A   */ NULL,
/* 0B   */ NULL,
/* 0C   */ NULL,
/* 0D   */ NULL,
/* 0E   */ NULL,
/* 0F   */ NULL,
/* 10   */ NULL,
/* 11   */ NULL,
/* 12   */ NULL,
/* 13   */ NULL,
/* 14   */ NULL,
/* 15   */ NULL,
/* 16   */ NULL,
/* 17   */ NULL,
/* 18   */ NULL,
/* 19   */ NULL,
/* 1A   */ NULL,
/* 1B   */ NULL,
/* 1C   */ NULL,
/* 1D   */ NULL,
/* 1E   */ NULL,
/* 1F   */ NULL,
/* 20   */ NULL,
/* 21   */ NULL,
/* 22   */ NULL, // leave double-quotes as-is (may be \\rdblquote, \\ldblquote, or \\dblquote)
/* 23   */ NULL,
/* 24   */ NULL,
/* 25   */ NULL,
/* 26   */ NULL,
/* 27   */ NULL,
/* 28   */ NULL,
/* 29   */ NULL,
/* 2A   */ NULL,
/* 2B   */ NULL,
/* 2C   */ NULL,
/* 2D   */ NULL,
/* 2E   */ NULL,
/* 2F   */ NULL,
/* 30   */ NULL,
/* 31   */ NULL,
/* 32   */ NULL,
/* 33   */ NULL,
/* 34   */ NULL,
/* 35   */ NULL,
/* 36   */ NULL,
/* 37   */ NULL,
/* 38   */ NULL,
/* 39   */ NULL,
/* 3A   */ NULL,
/* 3B   */ NULL,
/* 3C   */ NULL,
/* 3D   */ NULL,
/* 3E   */ NULL,
/* 3F   */ NULL,
/* 40   */ NULL,
/* 41   */ NULL,
/* 42   */ NULL,
/* 43   */ NULL,
/* 44   */ NULL,
/* 45   */ NULL,
/* 46   */ NULL,
/* 47   */ NULL,
/* 48   */ NULL,
/* 49   */ NULL,
/* 4A   */ NULL,
/* 4B   */ NULL,
/* 4C   */ NULL,
/* 4D   */ NULL,
/* 4E   */ NULL,
/* 4F   */ NULL,
/* 50   */ NULL,
/* 51   */ NULL,
/* 52   */ NULL,
/* 53   */ NULL,
/* 54   */ NULL,
/* 55   */ NULL,
/* 56   */ NULL,
/* 57   */ NULL,
/* 58   */ NULL,
/* 59   */ NULL,
/* 5A   */ NULL,
/* 5B   */ NULL,
/* 5C   */ NULL,
/* 5D   */ NULL,
/* 5E   */ NULL,
/* 5F   */ NULL,
/* 60   */ NULL,
/* 61   */ NULL,
/* 62   */ NULL,
/* 63   */ NULL,
/* 64   */ NULL,
/* 65   */ NULL,
/* 66   */ NULL,
/* 67   */ NULL,
/* 68   */ NULL,
/* 69   */ NULL,
/* 6A   */ NULL,
/* 6B   */ NULL,
/* 6C   */ NULL,
/* 6D   */ NULL,
/* 6E   */ NULL,
/* 6F   */ NULL,
/* 70   */ NULL,
/* 71   */ NULL,
/* 72   */ NULL,
/* 73   */ NULL,
/* 74   */ NULL,
/* 75   */ NULL,
/* 76   */ NULL,
/* 77   */ NULL,
/* 78   */ NULL,
/* 79   */ NULL,
/* 7A   */ NULL,
/* 7B   */ NULL,
/* 7C   */ NULL,
/* 7D   */ NULL,
/* 7E   */ NULL,
/* 7F   */ NULL,
/* 80   */ NULL,
/* 81   */ NULL,
/* 82   */ NULL,
/* 83   */ NULL,
/* 84   */ NULL,
/* 85   */ NULL,
/* 86   */ NULL,
/* 87   */ NULL,
/* 88   */ NULL,
/* 89   */ NULL,
/* 8A   */ NULL,
/* 8B   */ NULL,
/* 8C   */ NULL,
/* 8D   */ NULL,
/* 8E   */ NULL,
/* 8F   */ NULL,
/* 90   */ NULL,
/* 91   */ NULL,
/* 92   */ NULL,
/* 93   */ NULL,
/* 94   */ NULL,
/* 95   */ NULL,
/* 96   */ NULL,
/* 97   */ NULL,
/* 98   */ NULL,
/* 99   */ NULL,
/* 9A   */ NULL,
/* 9B   */ NULL,
/* 9C   */ NULL,
/* 9D   */ NULL,
/* 9E   */ NULL,
/* 9F   */ NULL,
/* A0   */ NULL,
/* A1   */ NULL,
/* A2   */ NULL,
/* A3   */ NULL,
/* A4   */ NULL,
/* A5   */ NULL,
/* A6   */ NULL,
/* A7   */ NULL,
/* A8   */ NULL,
/* A9   */ NULL,
/* AA   */ NULL,
/* AB   */ NULL,
/* AC   */ NULL,
/* AD   */ NULL,
/* AE   */ NULL,
/* AF   */ NULL,
/* B0   */ NULL,
/* B1   */ NULL,
/* B2   */ NULL,
/* B3   */ NULL,
/* B4   */ NULL,
/* B5   */ NULL,
/* B6   */ NULL,
/* B7   */ NULL,
/* B8   */ NULL,
/* B9   */ NULL,
/* BA   */ NULL,
/* BB   */ NULL,
/* BC   */ NULL,
/* BD   */ NULL,
/* BE   */ NULL,
/* BF   */ NULL,
/* C0   */ NULL,
/* C1   */ NULL,
/* C2   */ NULL,
/* C3   */ NULL,
/* C4   */ NULL,
/* C5   */ NULL,
/* C6   */ NULL,
/* C7   */ NULL,
/* C8   */ NULL,
/* C9   */ NULL,
/* CA   */ NULL,
/* CB   */ NULL,
/* CC   */ NULL,
/* CD   */ NULL,
/* CE   */ NULL,
/* CF   */ NULL,
/* D0   */ NULL,
/* D1   */ NULL,
/* D2   */ NULL,
/* D3   */ NULL,
/* D4   */ NULL,
/* D5   */ NULL,
/* D6   */ NULL,
/* D7   */ NULL,
/* D8   */ NULL,
/* D9   */ NULL,
/* DA   */ NULL,
/* DB   */ NULL,
/* DC   */ NULL,
/* DD   */ NULL,
/* DE   */ NULL,
/* DF   */ NULL,
/* E0   */ NULL,
/* E1   */ NULL,
/* E2   */ NULL,
/* E3   */ NULL,
/* E4   */ NULL,
/* E5   */ NULL,
/* E6   */ NULL,
/* E7   */ NULL,
/* E8   */ NULL,
/* E9   */ NULL,
/* EA   */ NULL,
/* EB   */ NULL,
/* EC   */ NULL,
/* ED   */ NULL,
/* EE   */ NULL,
/* EF   */ NULL,
/* F0   */ NULL,
/* F1   */ NULL,
/* F2   */ NULL,
/* F3   */ NULL,
/* F4   */ NULL,
/* F5   */ NULL,
/* F6   */ NULL,
/* F7   */ NULL,
/* F8   */ NULL,
/* F9   */ NULL,
/* FA   */ NULL,
/* FB   */ NULL,
/* FC   */ NULL,
/* FD   */ NULL,
/* FE   */ NULL,
/* FF   */ NULL };

/**********************************************************************/
/* IDs of windows help macros                                         */
/**********************************************************************/
typedef enum _HELPMACROID
{
  AL_MACRO,                            // the "AL" macro
  ALINK_MACRO,                         // the "!ALINK" macro
  KLINK_MACRO,                         // the "!KLINK" macro
  JUMPKEYWORD_MACRO,                   // the "!JumpKeyword" macro
  JUMPID_MACRO,                        // the "!JumpID" macro
  IFTHENELSE_MACRO,                    // the "!IfThenElse" macro
  FILEEXIST_MACRO,                     // the "!FileExist" macro
  SETWINDOWLONG_MACRO,                 // the "!SetWindowLong" macro
  SETWINDOWPOS_MACRO,                  // the "!SetWindowPos" macro
  INITSECONDARYWINDOW_MACRO,           // the "!InitSecondaryWindow" macro
  GETACTIVEWINDOW_MACRO,               // the "!GetActiveWindow" macro
  END_OF_MACROS,                       // summy macro to end the macro list
} HELPMACROID;

/**********************************************************************/
/* Structure for windows help macros                                  */
/**********************************************************************/
typedef struct _HELPMACRO
{
  HELPMACROID Id;                      // ID of macro
  SHORT       sLength;                 // length of macro name
  CHAR        szMacro[30];             // buffer for macro name
  CHAR_W      szMacroW[30];            // buffer for macro name (UTF16)
  USHORT      usNumParms;              // number of macro parameters
  BOOL        fTransParm[10];          // 1 = parameter n is translatable
  BOOL        fExtraSeg;               // TRUE = store data in extra segments
} HELPMACRO, *PHELPMACRO;

/**********************************************************************/
/* Help macros starting with ! character or which are used within     */
/* help tags as parameter (e.g. Klink in button help tag)             */
/**********************************************************************/
static HELPMACRO HelpMacro[] =
{
//  ====ID=====             =Len=    ===Name=====           ==Name UTF16==       Parms      P1, P2, P3, P4, P5, Segm?
  { AL_MACRO,                   2,   "AL",                  L"AL",                  3,    { 0,  0,  0,  0,  0 }, FALSE  },
  { ALINK_MACRO,                5,   "ALINK",               L"ALINK",               2,    { 0,  0,  0,  0,  0 }, TRUE  },
  { JUMPKEYWORD_MACRO,         11,   "JumpKeyword",         L"JumpKeyword",         2,    { 0,  1,  0,  0,  0 }, FALSE },
  { JUMPID_MACRO,               6,   "JumpID",              L"JumpID",              2,    { 0,  0,  0,  0,  0 }, FALSE },
  { SETWINDOWLONG_MACRO,       13,   "SetWindowLong",       L"SetWindowLong",       3,    { 0,  0,  0,  0,  0 }, FALSE },
  { SETWINDOWPOS_MACRO,        12,   "SetWindowPos",        L"SetWindowPos",        7,    { 0,  0,  0,  0,  0 }, FALSE },
  { INITSECONDARYWINDOW_MACRO, 19,   "InitSecondaryWindow", L"InitSecondaryWindow", 1,    { 0,  0,  0,  0,  0 }, FALSE },
  { GETACTIVEWINDOW_MACRO,     15,   "GetActiveWindow",     L"GetActiveWindow",     1,    { 0,  0,  0,  0,  0 }, FALSE },
  { KLINK_MACRO,                5,   "KLINK",               L"KLINK",               2,    { 1,  0,  0,  0,  0 }, TRUE  },
  { IFTHENELSE_MACRO,          10,   "IFTHENELSE",          L"IFTHENELSE",          3,    { 0,  0,  0,  0,  0 }, FALSE  },
  { FILEEXIST_MACRO,            9,   "FILEEXIST",           L"FILEEXIST",           1,    { 0,  0,  0,  0,  0 }, FALSE },
  { END_OF_MACROS,              0,   "",                    L"",                    0,    { 0,  0,  0,  0,  0 }, FALSE }
};

/**********************************************************************/
/* Macros within \field destination                                   */
/* List them to treat \field as inline tag dynamically                */
/**********************************************************************/


/**********************************************************************/
/* IDs of windows help tags                                           */
/**********************************************************************/
typedef enum _HELPTAGID
{
  BUTTON_TAG,                          // the \{button ...\} tag
  BML_TAG,                             // the \{bml ...\} tag
  BMR_TAG,                             // the \{bmr ...\} tag
  BMC_TAG,                             // the \{bmc ...\} tag
  END_OF_TAGS,                         // dummy tag to end the tag list
} HELPTAGID;

/**********************************************************************/
/* Structure for windows help tags                                    */
/**********************************************************************/
typedef struct _HELPTAG
{
  HELPTAGID   Id;                      // ID of tag
  SHORT       sLength;                 // length of tag name
  CHAR        szTag[20];               // buffer for tag name
  CHAR_W      szTagW[20];              // buffer for UTF16 tag name
  BOOL        fTranstable;             // TRUE = tag contains translatable data
  BOOL        fInline;                 // TRUE = treat as inline tag
} HELPTAG, *PHELPTAG;

/**********************************************************************/
/* Help tags (help commands starting with \{ character)               */
/* Note: the fTranslatable and fInline tags is currently not in use   */
/*       (the handling is done hard-coded for the specific tags)      */
/**********************************************************************/
static HELPTAG HelpTag[] =
{
//  ====ID=====      =Len=    ===Name===== =UTF16 name= Trans?  Inl?
  { BUTTON_TAG,          6,   "button",    L"button",   TRUE,  TRUE },
  { BML_TAG,             3,   "bml",       L"bml",      FALSE,  TRUE },
  { BMR_TAG,             3,   "bmr",       L"bmr",      FALSE,  TRUE },
  { BMC_TAG,             3,   "bmc",       L"bmc",      FALSE,  TRUE },
  { END_OF_TAGS,         0,   "",          L"",         FALSE, FALSE }
};

/**********************************************************************/
/* NONE tag identifier                                                */
/**********************************************************************/
PSZ pszNONETAG = ":NONE.";
PSZ_W pszNONETAG_W = L":NONE.";

/**********************************************************************/
/* Languages which require a specific handling                        */
/* Note: be sure to add an empty entry as last entry of the array     */
/**********************************************************************/
typedef struct _RTFSPECIALLANG
{
  CHAR  szLang[MAX_LANG_LENGTH];
  ULONG ulFlags;
} RTFSPECIALLANG;

#define DBCS_BLANK_FOR_LINEFEED 0x00000001L
#define INSERT_BIDI_TAGS        0x00000002L

RTFSPECIALLANG SpecialLang[] =
{
  { "Korean", DBCS_BLANK_FOR_LINEFEED },
  { "Hebrew", INSERT_BIDI_TAGS },
  { "Arabic", INSERT_BIDI_TAGS },
  { "", 0L }
};

/**********************************************************************/
/* Table entry for font changes in CHANGEFONT/DEFCHGFONT section      */
/* Note:                                                              */
/* for space reasons, only position and lengths are stored            */
/**********************************************************************/

typedef struct _CHANGEFONTTAB
{
  PSZ pszFontName;
  USHORT usFontNameLen;
  PSZ pszSubstFontRTFSpec;
  USHORT usSubstFontRTFSpecLen;
} CHANGEFONTTAB;

// font families for default replacing

typedef enum _FONTFAMILYTAG
{
  RTF_FF_ROMAN,
  RTF_FF_SWISS,
  RTF_FF_MODERN,
  RTF_FF_NIL,
  RTF_FF_SCRIPT,
  RTF_FF_DECOR,
  RTF_FF_TECH,
  RTF_FF_BIDI,
  RTF_FF_NULL
} FONTFAMILYTAG;

typedef struct _DEFCHGFONTTAB
{
  FONTFAMILYTAG FontFamilyTag;
  USHORT usDefFontFamilyLen;
  PSZ pszSubstFontRTFSpec;
  USHORT usSubstFontRTFSpecLen;
} DEFCHGFONTTAB;

#define MAX_FONTFAMILY_LEN    10

typedef struct _DEFCHGFNTTAG2TXT
{
  CHAR  szFontFamily[MAX_FONTFAMILY_LEN];
  FONTFAMILYTAG FontFamilyTag;
} DEFCHGFNTTAG2TXT;

//
// names of the default font families
// for font replacing
//

DEFCHGFNTTAG2TXT DefChgFntTag2Txt[] =
{
  { "roman",  RTF_FF_ROMAN  },
  { "swiss",  RTF_FF_SWISS  },
  { "modern", RTF_FF_MODERN },
  { "decor",  RTF_FF_DECOR  },
  { "bidi",   RTF_FF_BIDI   },
  { "tech",   RTF_FF_TECH   },
  { "script", RTF_FF_SCRIPT },
  { "",       RTF_FF_NULL   }
};

//
// keywords within \*\fldinst tag
// -> the following fldrslt text should be protected
// (index, table of contents)
// this table MUST be sorted

char *FldinstTagKeyWordsProtected[20] =
{
  "INDEX ",
//  "HYPERLINK ",
  "PAGE ",
  "PAGEREF ",
  "TOC ",
  "REF ",
  ""
};

// bt
// Character style marker for bidi processing
//

#define BIDIRECORDCHARSTYLE_LEN 50
#define MAX_BIDI_REC_STACKDEPTH 20

/**********************************************************************/
/* Structure for global data of parser                                */
/**********************************************************************/
typedef struct _PARSEDATA
{
   USHORT      Mode;                   // current processing mode
   HWND        hwndProcWin;            // handle of analysis process window
   HFILE       hInFile;                // handle of input file
   BYTE        abInBuf[INBUF_SIZE];    // buffer for input data
   USHORT      usBufferEndChar;        // ensures that at the end of abInBuf there is a hex zero
   LONG        lTotalBytes;            // total file size in bytes
   LONG        lBytesToRead;           // number of bytes to read from input
   USHORT      usBytesInBuffer;        // number of bytes in input buffer
   USHORT      usPercentComplete;      // current completion ratio
   PBYTE       pInBuf;                 // ptr for input buffer processing
   HFILE       hOutFile;               // handle of output file
   PSZ         pQFNTag;                // ptr to QFN tag name
   PSZ         pEQFNTag;               // ptr to EQFN tag name
   PSZ         pQFFTag;                // ptr to QFF tag name
   PSZ         pEQFFTag;               // ptr to EQFF tag name
   BYTE        abUndoBuf[MAX_SEG_SIZE];// buffer for undo next char function
   USHORT      usUndoBufUsed;          // used bytes in undo buffer
   BYTE        bLastOutput;            // value of last written byte
   BYTE        abOutBuf[MAX_SEG_SIZE+1]; // buffer for output data
   BYTE        abSegBuf[MAX_SEG_SIZE+1]; // buffer for text segmentation
   BYTE        abCharBuf[MAX_SEG_SIZE+1]; // buffer for character type
   BYTE        abTempBuf[MAX_SEG_SIZE+1];// buffer for temporary output data
   BYTE        abTempBuf2[MAX_SEG_SIZE+1];// buffer for temporary output data
   BYTE        abTempSegBuf[MAX_SEG_SIZE+1];// buffer for temporary segmentation data
   BYTE        abTempSegBuf2[MAX_SEG_SIZE+1];// buffer for temporary segmentation data
   BYTE        abTempCharBuf[MAX_SEG_SIZE+1]; // buffer for temp. character type
   BYTE        abTagBuf[MAX_SEG_SIZE+1]; // buffer for tag strings
   USHORT      usOutBufUsed;           // number of bytes used in output buffer
   USHORT      usLinePos;              // character position in current line
   PBYTE       pOutBuf;                // ptr for output buffer processing
   PBYTE       pLastTagPos;            // position of last tag character in
                                       // output buffer
   PBYTE       pFirstTagPos;           // position of first tag character in
                                       // output buffer
   PBYTE       pSegBuf;                // ptr for segment buffer processing
   PBYTE       pCharBuf;               // ptr for char type buffer processing
   SEGTYPES    SegType;                // type of currently active segment
   ULONG        ulSegNum;               // number of segments processed
   CHAR        szControlWord[MAX_CTRL_WORD];     // buffer for control words
   BOOL        fSegmOpen;              // TRUE = a segment is open
   DESTSTACK   DestStack[MAX_DEST_STACK]; // stack for nested destinations
   BOOL        fTranslDataInSegment;   // segment contains translatable data flag
   BOOL        fBkmkInSegment;         // bkmkstart tag in segment flag
   CHAR        szFolder[MAX_FNAME];    // folder of document
   CHAR        szLanguage[MAX_LANG_LENGTH]; // source language of document
   SHORT       sLangID;                // language ID for morph services
   PSZ         pTermList;              // pointer to term list
   USHORT      usTermListSize;         // current term list size
   PEQF_BOOL   pfKill;                 // ptr to 'kill running process' flag
   BYTE        bLastInputChar;         // buffer for last character (non-tag) from input
   BOOL        fByteWasLF;             // marker for space replacing (space for LF)
   BOOL        fWasExtTagChar;         // last char belonged to a tag (including numerics)
   BOOL        fFootNoteChar;          // TRUE = we are waiting for the footnote
                                       // character
   BYTE        bFootNoteChar;          // buffer for footnote character
   BYTE        bCurrFootNoteChar;      // character of current footnote
   PCHAR       pConvTable;             // ptr to code conversion table
   BOOL        fNoLineSplit;           // TRUE if no line splitting is allowed
   BOOL        fForceNewLine;          // TRUE force a new line in WriteBufferend function
   // target language specific values from settings file
   BOOL        fBidi;                  // TRUE = perform BIDI processing
   BOOL        fDBCSBlankForLF;        // TRUE = replace LF between DBCS chars by blank
   BOOL        fProtectFldrslt;        // TRUE if fldrslt should be protected
   CHAR        achIsBidiChar[256];     // BIDI character recognition table
   BOOL        fIsFontTag;             // TRUE = processed font tag bt 15012001
   PSZ_W       pszBidiRecordCharStyle; // (stacked) record field for BIDI char style tags added 290101 bt
   BOOL        fSkipOldBidiTag;        // TRUE = old ltrch/rtlch tag found lead from trlmemory entry 310101 bt
   BOOL        fBidiRecordCharStyle;   // TRUE = record preceding character styles
   CHAR        chAddHeader[256];       // buffer for addional tags for the document header
   CHAR        chAddFont[256];         // buffer for addional font definition
   CHAR        chRTLTags[256];         // buffer for tags starting RTL data
   CHAR        chLTRTags[256];         // buffer for tags starting LTR data
   CHAR        chBIDIONTags[256];      // buffer for BIDION tags
   CHAR        chBIDIOFFTags[256];     // buffer for BIDIOFF tags
   CHAR        chParTags[256];         // buffer for tags to be added to the
   CHAR_W      chAddHeaderW[256];      // buffer for addional tags for the document header
   CHAR_W      chAddFontW[256];        // buffer for addional font definition
   CHAR_W      chRTLTagsW[256];        // buffer for tags starting RTL data
   CHAR_W      chLTRTagsW[256];        // buffer for tags starting LTR data
   CHAR_W      chParTagsW[256];        // buffer for tags to be added to the
   CHAR_W      chBIDIONTagsW[256];     // buffer for BIDION tags
   CHAR_W      chBIDIOFFTagsW[256];    // buffer for BIDIOFF tags
                                       // begin of a paragraph
   CHANGEFONTTAB ChangeFontTab[MAX_CHFTTAB_ENTRIES];   // for CHANGEFONT section
   USHORT        usChangeFontTabEntries;               // no. of entries
   PVOID       pChangeFontData;        // reference data for de-allocating
   DEFCHGFONTTAB DefChgFontTab[MAX_DEFCHFT_ENTRIES];   // for DEFCHGFONT section
   USHORT      usDefChgFontTabEntries;   // no. of entries
   PVOID       pDefChgFontData;        // reference data for de-allocating
   SHORT       sAddFontNo;             // font number used for additional font
   USHORT      usCodePage;             // code page to be used for ASCII->ANSI conversion
   BOOL        fMacroMayFollow;        // TRUE = a macro may be following
   int         iUCStatus;              // current \uc setting
   BOOL        fHiddenMode;            // current hidden text mode in PreUnseg step
   BOOL        fBidiOffMode;           // TRUE = we are in BIDIOFF mode
   ULONG       ulSrcOemCP;
   ULONG       ulTgtOemCP;
   ULONG       ulSrcAnsiCP;
   ULONG       ulTgtAnsiCP;
   CHAR_W      UTF16Buffer[MAX_SEG_SIZE+20]; // buffer for UTF16 conversion
   PCHAR       pOrgConvTable;          // ptr to original code conversion table

   CHAR        chRemoveTags[8096];     // buffer for remove tag list
   CHAR_W      chRemoveTagsW[8096];    // buffer for remove tag list
   CHAR        szRemoveTagsBuffer[8096]; // buffer for segment data with removed tags
   CHAR        chChangeFromTags[8096]; // buffer for changefrom tag list
   CHAR_W      chChangeFromTagsW[8096];// buffer for changefrom tag list
   CHAR        chChangeToTags[8096];   // buffer for changeto tag list
   CHAR_W      chChangeToTagsW[8096];  // buffer for changeto tag list
   CHAR_W      szChangeTagsBufferW[8096];// buffer for segment data with changed tags
} PARSEDATA, *PPARSEDATA;


/**********************************************************************/
/* Prototypes for internal functions                                  */
/**********************************************************************/
static USHORT AddToSegment ( PPARSEDATA, BYTE, BOOL, CHARTYPE );
static BYTE   ParseNextChar ( PPARSEDATA pParsData, PUSHORT pusRC );
static VOID   UndoChar ( PPARSEDATA pParsData, BYTE bChar );
static USHORT RTFWriteSegment ( PPARSEDATA pParsData );
static USHORT PutSegment
(
  PPARSEDATA pParsData,                // ptr to parser global data structure
  BOOL       fProtected,               // protected segment flag
  PSZ        pszSegment,               // ptr to start of segment
  USHORT     usLen                     // length of segment
);
static USHORT ParseRTF ( PSZ pszSource, PSZ pszTarget, HWND hwndSlider,
                         PEQF_BOOL pfKill );                         /* @50A */
static BOOL   UnparseRTF( PSZ pszInFile, PSZ pszTagTable );
static USHORT RTFWriteBuffered( PPARSEDATA, BYTE, BOOL );
static void   AddToTable ( PSTARTSTOP, PUSHORT, USHORT, USHORT, USHORT );
static BOOL   ParseChangeFontSection ( PSZ ChangeFontSection, PPARSEDATA pParseData );
static BOOL   ParseDefChgFontSection ( PSZ DefChgFontSection, PPARSEDATA pParseData );
static SHORT RTFPreUnseg
(
   PSZ pszSource,                      // fully qualified name of source file
   PSZ pszTarget,                      // fully qualified name of target file
   PSZ pszTagTable,                    // name of tag table
   PEQF_BOOL pfKill                    // pointer to kill flag
);
BOOL RTFGetControlWord
(
  PPARSEDATA  pParsData,               // points to parser data structure
  PUSHORT     pusRC,                   // ptr to buffer for return code
  PBYTE       pbCurrent                // ptr to buffer for current char
);
USHORT RTFCheckAndHandleHelpTag
(
  PPARSEDATA  pParseData,               // points to parser data structure
  BOOL        fTranslSegm,             // current-segment-is-translatable flag
  BOOL        fProtected               // current-data-is-protected flag
);

static VOID RTFFreeDoc ( PVOID * ppDoc );
USHORT RTFNormalChar
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PBYTE       pbCurrent,               // ptr to buffer for current character
  BOOL        fProtected,              // current protection state
  BOOL        fTranslSegm              // current segement state
);
USHORT RTFParseMacro
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PBYTE       pbCurrent,               // ptr to buffer for current character
  BOOL        fProtected,              // current protection state
  BOOL        fTranslSegm,             // current segement state
  BYTE        bFirstChar               // EOS or current byte if already read
);
BOOL RTFCheckForTranslData
(
  PPARSEDATA  pParseData               // points to parser data structure
);
USHORT RTFHandleHexChar
(
  PPARSEDATA  pParseData,              // points to parser data structure
  BOOL        fTranslSegm,             // segment-is-translatable flag
  BOOL        fProtected,              // data-is-protected flag
  BYTE        bHexCode,                // control code for hex-endcoded
  BOOL        fLeaveAsIs               // leave the character hex-encoded flag
);
BOOL RTFContainsAnsiTag( PSZ pszString );
USHORT RTFDataToRef( PPARSEDATA pParseData );
USHORT RTFRefToData
(
  PPARSEDATA  pParseData,              // parser data structure
  HFILE       hfSource,                // file handle of source file
  PBYTE       pbCurrent,               // ptr to caller's character buffer
  SHORT       sObjType                 // type of object being processed
);
USHORT RTFHandleFootNoteTag
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PDESTSTACK  pCurEnv,                 // ptr to curent destination environment
  PCTRLWORD   pCtrlWord                // pointer to current control word
);
USHORT RTFGetSettings
(
  PSZ pszTagTable,
  PSZ pszTargetLang,
  PPARSEDATA pParsData
);
USHORT RTFParseSkipAnyWhiteSpace
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PBYTE       pbCurrent,               // ptr to buffer for current character
  BOOL        fTranslSegm,             // current segement state
  CHARTYPE    CharType                 // type to bne used for current character
);
USHORT RTFParseMacroParm
(
  PPARSEDATA  pParseData,              // points to parser data structure
  PBYTE       pbCurrent,               // ptr to buffer for current character
  BOOL        fProtected,              // current protection state
  BOOL        fTranslSegm,             // current segement state
  CHARTYPE    CharType,                // type to bne used for current character
  PHELPMACRO  pMacro,                  // ptr to macro definition data
  SHORT       sParmNum                 // number of current macro parameter
);

BOOL RTFSetLangCodePage(char *pszLang, UINT *CodePage);
BOOL IsCharTopicPrefix(char TestChar);

#define MAX_LANG_NAME_LEN    20

typedef struct _LANGCPARRAY            // relation language -> codepage
{
   CHAR  szTmLang[MAX_LANG_NAME_LEN];
   UINT  uiCodePage;
} LANGCPARRAY;

// DBCS Character recognition

#define _isdbcs(c)  ((isleadbyte((_TXCHAR) c )) ? DBCS_1ST : SBCS)


// table for the conversion of character tags to unicode vars and vice versa
typedef struct _RTFSPECIALCHARTAGS
{
  CHAR_W szTag[40];                    // tag without leading backslash
  CHAR_W chValue;                      // Unicode value for tag
} RTFSPECIALCHARTAGS;

RTFSPECIALCHARTAGS SpecialCharTags[] =
{
  { L"lquote",     0x2018 },           // left single quote
  { L"rquote",     0x2019 },           // right single quote
  { L"ldblquote",  0x201C },           // left double quote
  { L"rdblquote",  0x201D },           // right double quote
  { L"",           0x0000 },           // end of list
};
