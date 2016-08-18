/*! EQFTPI.H      Internal header file for Translation Processor functions
	Copyright (c) 1990-2016, International Business Machines Corporation and others. All rights reserved.
*/

// allow support for hot-spot conversion within windows...
  #include <IME.H>
  #include "eqfb.id"
/**********************************************************************/
/* General Part (formerly EQFB.H)                                     */
/**********************************************************************/

/*********************************************************************
 * Constants                                                         *
 *********************************************************************/
#define  CHARACTER_A    'a'        // the character A
#define  CHARACTER_0    '0'            // the character 0

#define MAX_VSCROLLRANGE  32000          // max. range for vert. scrollbar


#ifndef RC_INCLUDED

#define MIN_CX_SIZE         40         // minimum number of characters

#define TAG_END_CHAR_W     L"."       // tag end character
#define TAG_END_CHAR        "."

/**********************************************************************/
/* defines to store if ruler or statusbar was active on the last      */
/* invocations...                                                     */
/**********************************************************************/
#define TP_WND_STATUSBAR      1
#define TP_WND_RULER          2

/**********************************************************************/
/* defines for Editor                                                 */
/**********************************************************************/
#define STANDARD_EDITOR       1
#define RTFEDIT_EDITOR        2

#define RTFEDIT_EDITOR_STR    "RTFEDIT"

/**********************************************************************/
/* RTF Control update modes                                           */
/**********************************************************************/
#define RTFCTRL_UPDATECURSOR  0
#define RTFCTRL_AUTOSPELL     1
#define RTFCTRL_GETWORKSEG    2

/**********************************************************************/
/* allowed frame styles ...                                           */
/**********************************************************************/
#define AVAILSTYLES (FCF_TITLEBAR | FCF_SYSMENU | FCF_SIZEBORDER | FCF_MINBUTTON\
                    | FCF_MAXBUTTON | FCF_VERTSCROLL | FCF_HORZSCROLL | FCF_MENU)
#define AVAILSTYLESRTF (AVAILSTYLES & ~(FCF_VERTSCROLL|FCF_HORZSCROLL))
//+----------------------------------------------------------------------------+
//|  list of symbolic function names                                           |
//|                                                                            |
//|   Note: the order of the listed functions must be the same as the          |
//|         order in the function table as the enum value                      |
//|         is used as index into the function table.                          |
//+----------------------------------------------------------------------------+
typedef enum _FUNCTION
{
   ACTPROP_FUNC,
   DICTDOWN_FUNC,
   DICTUP_FUNC,
   BACKSPACE_FUNC,
   BACKTAB_FUNC,
   BOTTOMDOC_FUNC,
   DELETECHAR_FUNC,
   DOWN_FUNC,
   ENDLINE_FUNC,
   ENDSEG_FUNC,
   ESCAPE_FUNC,
   FILE_FUNC,
   INSTOGGLE_FUNC,
   INSERTLINE_FUNC,
   JOINLINE_FUNC,
   LEFT_FUNC,
   NEXTDOC_FUNC,
   NEXTWORD_FUNC,
   NOTHING_FUNC,
   PAGEDOWN_FUNC,
   PAGEUP_FUNC,
   PREVDOC_FUNC,
   PREVWORD_FUNC,
   QUIT_FUNC,
   RIGHT_FUNC,
   SAVE_FUNC,
   SCROLLDOWN_FUNC,
   SCROLLLEFT_FUNC,
   SCROLLRIGHT_FUNC,
   SCROLLUP_FUNC,
   SPLITLINE_FUNC,
   STARTLINE_FUNC,
   STARTSEG_FUNC,
   TAB_FUNC,
   TOPDOC_FUNC,
   TRUNCSEG_FUNC,
   TRUNCATE_FUNC,
   UNDO_FUNC,
   UP_FUNC,
   GETDICTMATCH_FUNC,
   GETPROPMATCH_FUNC,
   PROPDOWN_FUNC,
   PROPUP_FUNC,
   TSEG_FUNC,
   TSEGNEXT_FUNC,
   DICTLOOK_FUNC,
   ACTDIC_FUNC,
   JOINSEG_FUNC,
   SPLITSEG_FUNC,
   MARKSEG_FUNC,
   GOTOSEG_FUNC,
   GOTOMARK_FUNC,
   CLEARSEGMARK_FUNC,
   POSTEDIT_FUNC,
   AUTOTRANS_FUNC,
   FIND_FUNC,
   RFIND_FUNC,
   RING_FUNC,
   OPEN_FUNC,
   CUT_FUNC,
   COPY_FUNC,
   PASTE_FUNC,
   MARKNEXT_FUNC,                   // mark next word too
   MARKPREV_FUNC,                   // mark previous word too
   FONTS_FUNC,
   KEYS_FUNC,
   ADDFUZZY_FUNC,                    // add fuzzy matches in TM window
   DISPORG_FUNC,
   HIDE_FUNC,
   PROTECT_FUNC,
   UNPROTECT_FUNC,
   NEXTLINE_FUNC,
   CHARACTER_FUNC,
   MARKBLOCK_FUNC,
   MARKCLEAR_FUNC,
   MARKDELETE_FUNC,
   MARKCOPY_FUNC  ,
   MARKMOVE_FUNC  ,
   MARKFIND_FUNC,
   PRINT_FUNC,
   MARKBLOCKSEG_FUNC,
   MARKLEFT_FUNC,
   MARKRIGHT_FUNC,
   MARKUP_FUNC,
   MARKDOWN_FUNC,
   DELLINE_FUNC,
   ENTRYSEN_FUNC,                   // test a standalone sentence
   UNTRANS_FUNC,                    // untranslate function
   LINEWRAP_FUNC,                      // line wrap

   LRM_FUNC,                    // input left-to-right marker

   FONTSIZE_FUNC,                   // select font size for different windows
   MARGINACT_FUNC,                  // right margin activate / deactivae funct.
   SPELLSEG_FUNC,                   // spellcheck of segment
   SPELLFILE_FUNC,                  // spellcheck of file
   WORDMARK_FUNC,                   // mark current word
   WORDDEL_FUNC,                    // delete current word
   DELTILTAG_FUNC,                  // delete til next tag in segment
   EMPTY_FUNC,                      // delete segment totally
   COMMAND_FUNC,                    // execute a command from the listbox
   SCRATCH_FUNC,                    // find seg. transl. from scratch
   PROPCPY_FUNC,                    // find seg. which is copy of proposal
   CPYTYPED_FUNC,                   // find seg which is copy & typed in
   EXPAND_FUNC,                     // expand compressed inline tags
   COMPRESS_FUNC,                   // compress inline tags
   SETTINGS_FUNC,                   // allow for settings  function
   GOTO_FUNC,                       // goto line number..
   QUERYLINE_FUNC,                  // query line number
   SHRINK_FUNC,                     // shrink style
   COMPACT_FUNC,                    // compact style
   SEGINSOSI_FUNC,                  // insert SOSI in current segment
   SEGDELSOSI_FUNC,                 // delete SOSI in current segment
   DOCINSOSI_FUNC,                  // insert SOSI in all xlated segments
   DOCDELSOSI_FUNC,                 // delete SOSI in all xlated segments
   ACTTRANS_FUNC,                      // activate translation window

   RLM_FUNC,                        // input right to left marker

   SRCPROP_FUNC,                    // display source of proposal wnd
   EDITTERM_FUNC,                   // edit a term in dictionary
   HOTPOPUP_FUNC,                   // popup function for hotmenu
   SAVEAS_FUNC,
   ADDABBREV_FUNC,                  // add support for abbreviation
   REIMPORT_FUNC,                   // re-import a document

   EDITADD_FUNC,                    // edit addenda function
   MARKSTART_FUNC,                  // mark to start of line
   MARKEND_FUNC,                    // mark til end of line
   INITCAPS_FUNC,                   // toggle first character to caps/non caps
   SHOWTRANS_FUNC,                  // show translation in test window
   MARKSEGSTART_FUNC,               // mark current til start of seg
   MARKSEGEND_FUNC,                 // mark current til end of seg
   OPENTRNOTE_FUNC,                 // use this dummies to add new functions
   TOCGOTO_FUNC,                    // goto a position via table of contents
   SHORTEN_FUNC,                       // shorten style
   PROPMARKCOPY_FUNC,               // copy marked block from proposal

   NEXTMISSPELLED_FUNC,             // goto next misspelled word
   SPELLAUTO_FUNC,                  // turn autospellcheck on
   GOUPDSEGMENT_FUNC,
   VISIBLESPACE_FUNC,
   REDO_FUNC,
   WYSIWYG_FUNC,
   ALLEXACTPROPS_FUNC,
   DUMMY_009,
   GOTOSEGMENT_FUNC,                // goto segment..
   CFIND_FUNC,
   SEGPROP_FUNC,
   DICTPFXDOWN_FUNC,                // page down dict. prefixes
   DICTPFXUP_FUNC,                  // page up dict. prefixes
   TSEGNEXT_EXACT_FUNC,             // next untranslated segment with EXACT matches
   TSEGNEXT_FUZZY_FUNC,             // next untranslated segment with FUZZY matches
   TSEGNEXT_NONE_FUNC,              // next untranslated segment with NO matches
   TSEGNEXT_MT_FUNC,                // next untranslated segment with MT matches
   TSEGNEXT_GLOBAL_FUNC,            // next untranslated segment with GLOBAL MEMORY matches
   LAST_FUNC                        // should always be last entry in enum
} FUNCTION;

//+----------------------------------------------------------------------------+
//|  shift state flags for keys                                                |
//|  (which together form the second index for the key table)                  |
//+----------------------------------------------------------------------------+
#define ST_SHIFT          0x01         // shift key is pressed
#define ST_ALT            0x02         // alt key is pressed
#define ST_CTRL           0x04         // control key is pressed
#define ST_VK             0x08         // key is a virtual key



#define CHECKPOS_XLATESEG 0         // same position
#define CHECKPOS_MOVE   1           // user movement
#define CHECKPOS_XLATED 2           // already translated
#define CHECKPOS_FIRST  3           // fist call to tm
#define CHECKPOS_POSTEDIT 4         // post edit mode was active


// types for EQFBCharType function
#define HIDDEN_CHAR          1        // the character is hidden
#define PROTECTED_CHAR       2        // the character is display only
#define UNPROTECTED_CHAR     3        // the character can be changed
#define LINEBREAK_CHAR       4        // the character is a line break
#define ENDOFSEG_CHAR        5         // the character is outside of the segment
#define COMPACT_CHAR         6         // 1st of inline tag in compact style
#define TAGPROT_CHAR         7         // protected tagging character
#define TRNOTE_CHAR          8         // part of TRNote
#define SHORTEN_CHAR         9         // "..." for compact+1 style

/**********************************************************************/
/* types for HL (Highlighting)                                        */
/**********************************************************************/
#define NO_HIGHLIGHT         0          // do not highlight
#define TAG_HIGHLIGHT        1          // tagging changed - highlight
#define MISSPELLED_HIGHLIGHT 2          // misspelled word - highlight


#define VIDEOWIDTH (85  )       // AVIO maximum chars per line
#define VIDEOHEIGHT (30)        // AVIO maximum lines
#define MAX_DICTLENGTH           240L           // length of dict entry
#define MAX_PROPLENGTH           2400L          // length of proposal entry
#define LINEMARK            "\r\n"        // lineend symbol for Dos / OS/2
#define FOCUSLINE           3             // line where to position (default)
#define MINFOCUS            1             // smallest focus allowed
#define MAXFOCUS           20                    // highest focus allowed
#define FOCUS_LENGTH        2             // length of EF for focus

#define NOWHERE             0             // command allowed nowhere
#define ACTSEG              7             // command only allowed in active seg
#define STARGET             3             // command allowed in STARGET
#define OVERALL             1             // command allowed everywhere

#define  CHANGE_CURRENTTOEND  1           // range
#define  CHANGE_SEGMENTONLY   2           //        where
#define  CHANGE_MARKONLY      3           //              to change

#define CHARACTERS_TYPED  4            // number of characters typed after checking for line wrap
#define AUTOSIZE        999            // size indicates AUTO if saved in Profile
#define RMARGIN_DEFAULT AUTOSIZE       // default right margin
#define MINMARGIN       10             // minimum margin allowed


#define RMARGIN_LENGTH  20             // length of entry field for rmargin
#define MAXMARGIN       950            // maximum margin allowed


#define SHRINKIN_SEGDATA  "#"          // shrinked data
#define SHRINKOUT_SEGDATA "<>"
#define TRNOTE_SEGDATA    "<TRNOTE>"   // displayed instead of translators'note
#define SHORTEN_SEGDATA    L"..."      // abbreviation for compact+1 style

#define POS_TOBE          0         // position at untranslated only
#define POS_TOBEORDONE    1         // position at next untransl. or transl.
#define POS_CURSOR        2         // position at cursor
#define POS_TOBE_EXACT    3         // position at untranslated only with EXACT matches
#define POS_TOBE_FUZZY    4         // position at untranslated only with FUZZY matches
#define POS_TOBE_NONE     5         // position at untranslated only with NO matches
#define POS_TOBE_MT       6         // position at untranslated only with MT matches
#define POS_TOBE_GLOBAL   7         // position at untranslated only with GLOBAL MEMORY matches

#define MAXSTYLES         7            // max no of styles possible
#define MAXLBITEMS       22           // max number of profile settings

/**********************************************************************/
/* Configuration Part (formerly EQFBCONF.H)                           */
/**********************************************************************/

/**********************************************************************/
/* do definitions for DBCS if not yet done                            */
/**********************************************************************/
#ifndef VK_DBE_ALPHANUMERIC
  /* for codepage 932 */

  #define VK_DBE_ALPHANUMERIC     0x80 /* VK_DBE_FIRST + 0x00 */
  #define VK_DBE_KATAKANA         0x81 /* VK_DBE_FIRST + 0x01 */
  #define VK_DBE_HIRAGANA         0x82 /* VK_DBE_FIRST + 0x02 */
  #define VK_DBE_SBCSCHAR         0x83 /* VK_DBE_FIRST + 0x03 */
  #define VK_DBE_DBCSCHAR         0x84 /* VK_DBE_FIRST + 0x04 */
#endif

#ifndef VK_DBE_SBCSDBCSCHAR
  #define VK_DBE_SBCSDBCSCHAR     0x85 /* VK_DBE_FIRST + 0x05 */
#endif

#ifndef VK_DBE_ROMAN
  #define VK_DBE_ROMAN            0x86 /* VK_DBE_FIRST + 0x06 */
#endif

  /* for codepage 934 */

#ifndef VK_DBE_HANJA
  #define VK_DBE_HANJA            0x87 /* VK_DBE_FIRST + 0x07 */
  #define VK_DBE_HANGEUL          0x88 /* VK_DBE_FIRST + 0x08 */
  #define VK_DBE_JAMO             0x89 /* VK_DBE_FIRST + 0x09 */
#endif

  /* for codepage 936 */

#ifndef VK_DBE_HANZI
  #define VK_DBE_HANZI            0x8A /* VK_DBE_FIRST + 0x0A */
#endif

  /* for codepage 938 */

#ifndef VK_DBE_TSANGJYE
  #define VK_DBE_TSANGJYE         0x8C /* VK_DBE_FIRST + 0x0C */
  #define VK_DBE_PHONETIC         0x8D /* VK_DBE_FIRST + 0x0D */
#endif

  /* my own defines, currently missing in OS2 NLS.H  */
  #define VK_MY_NOCONV            0xA1
  #define VK_MY_KANAKANJI         0xA2


//+----------------------------------------------------------------------------+
//|  symbolic values for available fonts                                       |
//+----------------------------------------------------------------------------+
typedef enum _VIOFONTS
{
   DEFAULT_FONT,                       // the default font
//           SMALLCAPS_FONT,                     // font with small capitals
   MAXVIOFONTS                         // end-of-fonts enum indicator
} VIOFONTS;
//+----------------------------------------------------------------------------+
//|  names for available fonts (must be in same order has VIOFONTS enum)       |
//+----------------------------------------------------------------------------+
#define MAX_FONT_NAME    40            // max length of a font name


//#if defined(INIT_TABLES)
// CHAR aszFontNames[MAXVIOFONTS][MAX_FONT_NAME];
//#else
// extern CHAR aszFontNames[MAXVIOFONTS][MAX_FONT_NAME];
//#endif

#if defined(INIT_TABLES)
  COLORREF COLORRGBTABLE[MAXVIOCOLOR] =
    {
     RGB(0,0,0)        ,     //  RGB(0,0,0),                  // COL_BLACK,
     RGB(0,0,128)      ,     //  RGB(0,0,255),                // COL_BLUE,
     RGB(0,128,0)      ,     //  RGB(0,255,0),                // COL_GREEN,
     RGB(0,128,128)    ,     //  RGB(0,255,255),              // COL_CYAN,
     RGB(128,0,0)      ,     //  RGB(255,0,0),                // COL_RED,
     RGB(128,0,128)    ,     //  RGB(255,0,255),              // COL_PINK,
     RGB(128,128,0)    ,     //  RGB(128,128,0),              // COL_BROWN,
     RGB(192,192,192)  ,     //  RGB(192,192,192),            // COL_LIGHTGRAY,
     RGB(128,128,128)  ,     //  RGB(128,128,128),            // COL_DARKGRAY,
     RGB(0,0,255)      ,     //  RGB(0,128,255),              // COL_LIGHTBLUE,
     RGB(0,255,0)      ,     //  RGB(128,255,128),            // COL_LIGHTGREEN
     RGB(0,255,255)    ,     //  RGB(128,255,255),            // COL_LIGHTCYAN,
     RGB(255,0,0)      ,     //  RGB(255,128,128),            // COL_LIGHTRED,
     RGB(255,0,255)    ,     //  RGB(255,128,255),            // COL_LIGHTPINK,
     RGB(255,255,0)    ,     //  RGB(255,255,0),              // COL_YELLOW,
     RGB(255,255,255)        //  RGB(255,255,255)             // COL_WHITE
    };
#else
     extern COLORREF COLORRGBTABLE[MAXVIOCOLOR];
#endif

//+----------------------------------------------------------------------------+
//|  names for colors (must be in same order has VIOCOLORS enum)               |
//+----------------------------------------------------------------------------+
#define MAX_COLOR_NAME    40           // max length of a color name

//#if defined(INIT_TABLES)
// CHAR aszColorNames[MAXVIOCOLOR][MAX_COLOR_NAME];
//#else
// extern CHAR aszColorNames[MAXVIOCOLOR][MAX_COLOR_NAME];
//#endif

//+----------------------------------------------------------------------------+
//|  symbolic values for text types                                            |
//+----------------------------------------------------------------------------+
typedef enum _COLOUR
{
    COLOUR_EXIT,           // exit colour
    COLOUR_XLATED,         // already translated
    COLOUR_ACTIVE,         // current segment
    COLOUR_TOBE,           // still remaining
    COLOUR_NOP,            // tagging information but unprotected
    COLOUR_P_XLATED,       // already translated, protected part
    COLOUR_P_ACTIVE,       // current segment, protected part
    COLOUR_P_TOBE,         // still remaining, protected part
    COLOUR_P_NOP,          // protected part
    COLOUR_SRV_PROPSRCINS, // proposal source inserted text
    COLOUR_SRV_PROP0PREFIX,// proposal 0 prefix
    COLOUR_SRV_PROPSRCDEL, // proposal source deleted text
    COLOUR_SRV_PROPNPREFIX,// proposal n prefix
    COLOUR_SRV_DICTHEAD ,  // dictionary head word
    COLOUR_SRV_DICTTRANS,  // dictionary translation
    COLOUR_SRV_DICTPREFIX, // dictionary prefix
    COLOUR_SRV_PROPSRCEQU, // proposal source equal text
    COLOUR_SRV_PROPSRCUNEQU,// proposal source unequal
    COLOUR_ACTIVE_TAG,      // current segment, unprotected but tag
    COLOUR_ANCHOR_1,        // VisITM: anchor, rotate color 1
    COLOUR_ANCHOR_2,        // VisITM: anchor, rotate color 2
    COLOUR_ANCHOR_3,        // VisITM: anchor, rotate color 3
    COLOUR_VALID_01,        // Vis ITM :not aligned (tgt doc)
    COLOUR_VALID_10,        // Vis ITM :not aligned (src doc)
    COLOUR_VALID_11_1,      // Vis ITM :in 1:1 alignment, rotate color 1
    COLOUR_VALID_11_2,      // Vis ITM :in 1:1 alignment, rotate color 2
    COLOUR_VALID_11_3,      // Vis ITM :in 1:1 alignment, rotate color 3
    COLOUR_CROSSED_OUT,     // Vis ITM :segment crossed out by user
    COLOUR_NOP_ANCHOR_1,    // Vis ITM :NOP segment which is an anchor
    COLOUR_NOP_ANCHOR_2,    // Vis ITM :NOP segment which is an anchor
    COLOUR_NOP_ANCHOR_3,    // Vis ITM :NOP segment which is an anchor
    COLOUR_VISACT,          // VisITM: active alignment
    COLOUR_OVERCROSS,       // VisITM: overcross user anchor
    COLOUR_SRV_DICTADDINFO, // additional dictionary info
    COLOUR_TRNOTE_11,                  // trnote level one, 1st color
    COLOUR_TRNOTE_12,                  // trnote level one, 2nd color
    COLOUR_TRNOTE_2,                   // trnote level two color
    COLOUR_DICTINDIC,                  // dictionary indicator
    COLOUR_UNMATCHTAG,                 // unmatching tags highlight color
    COLOUR_TFROMSCRATCH,               // translated from scratch
    COLOUR_TMODPROPOSAL,               // modified proposal is translation
    COLOUR_TCOPYPROPOSAL,              // proposal copy is translation
    COLOUR_STANDARD_TRNOTE,            // <TRNOTE> abbreviation in standard edi
    COLOUR_DICTSTYLEPREF,              // dictionary style indicator preferred term
    COLOUR_DICTSTYLENOT,               // dictionary style indicator not allowed term
    COLOUR_MACHINE_MATCH_NORMAL,       // normal text in a machine match 
    COLOUR_MACHINE_MATCH_PROT,         // protected text in a machine match
    COLOUR_FUZZY_MATCH_NORMAL,         // normal text in a fuzzy match 
    COLOUR_FUZZY_MATCH_PROT,           // protected text in a fuzzy match

/**********************************************************************/
/* COLOUR_OWN_SETTING MUST be the last one in the COLOUR enumeration  */
/* since it is not contained in the IDS_TB_COLEXIT + usI table !!     */
/**********************************************************************/
    COLOUR_OWN_SETTING,     // own color settings - default black on black
    MAXCOLOUR
} COLOUR;


//+----------------------------------------------------------------------------+
//|  names for text types (must be in same order as COLOUR enumeration)        |
//|  AND COLOUR also must be in same order as IDS_TB_COL_EXIT + usI            |
//|  in eqfb.id since this is used as handle to the items in the listbox       |
//+----------------------------------------------------------------------------+
#define MAX_TEXTTYPE_NAME    60        // max length of a text type name

//#if defined(INIT_TABLES)
// CHAR aszTextNames[MAXCOLOUR][MAX_TEXTTYPE_NAME];
//
// CHAR aszWindowNames[MAX_DIF_DOC][MAX_TEXTTYPE_NAME];
//#else
// extern CHAR aszTextNames[MAXCOLOUR][MAX_TEXTTYPE_NAME];
//
// extern CHAR aszWindowNames[MAX_DIF_DOC][MAX_TEXTTYPE_NAME];
//#endif

//+----------------------------------------------------------------------------+
//|  default color/font values for text types                                  |
//|     (used to create profile and for the 'Defaults' button of the font/color|
//|      dialog)                                                               |
//+----------------------------------------------------------------------------+
#if defined(INIT_TABLES)
  TEXTTYPETABLE DefTextTypeTable[MAXCOLOUR]=
  {
  // FG            BG         Font          US     Revers
  //---------------------------------------------------------------------------
   { COL_LIGHTGRAY,COL_BLACK, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_EXIT
   { COL_BLACK,    COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_XLATED
   { COL_BLACK,    COL_YELLOW,DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_ACTIVE
   { COL_BLUE,     COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_TOBE
   { COL_RED,      COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_NOP
   { COL_RED,      COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_P_XLATED
   { COL_RED,      COL_YELLOW,DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_P_ACTIVE
   { COL_RED,      COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_P_TOBE
   { COL_RED,      COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_P_NOP
   { COL_BLACK,COL_LIGHTGRAY, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_SRV_PROPSRCINS
   { COL_RED,      COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_SRV_PROP0PREFIX
   { COL_BLACK,COL_LIGHTPINK, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_SRV_PROPSRCDEL
   { COL_BLUE,     COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_SRV_PROPNPREFIX
   { COL_RED,      COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_SRV_DICTHEAD
   { COL_BLUE,     COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_SRV_DICTTRANS
   { COL_RED,      COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_SRV_DICTPREFIX
   { COL_BLUE,     COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_SRV_PROPSRCEQU
   { COL_BLACK,COL_LIGHTCYAN, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_SRV_PROPSRCUNEQU
   { COL_LIGHTGRAY,COL_YELLOW,DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_ACTIVE_TAG
   { COL_BLACK,    COL_CYAN,  DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_ANCHOR_1,
   { COL_BLUE,     COL_CYAN,  DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_ANCHOR_2,
   { COL_LIGHTGRAY, COL_CYAN,  DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_ANCHOR_3,
   { COL_BLACK,    COL_LIGHTRED,   DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_VALID_01,
   { COL_BLACK,    COL_LIGHTRED,   DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_VALID_10,
   { COL_BLACK,    COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_VALID_11_1,
   { COL_BLUE,     COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_VALID_11_2,
   { COL_GREEN,    COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_VALID_11_3,
   { COL_WHITE,    COL_LIGHTGRAY,DEFAULT_FONT, FALSE, FALSE, }, // COLOUR_CROSSED_OUT
   { COL_RED,      COL_WHITE,  DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_NOP_ANCHOR_1,
   { COL_RED,      COL_WHITE,  DEFAULT_FONT, FALSE, FALSE, },    // COLOUR_NOP_ANCHOR_2,
   { COL_RED,      COL_WHITE,  DEFAULT_FONT, FALSE, FALSE, },    // COLOUR_NOP_ANCHOR_3,
   { COL_BLACK,    COL_YELLOW, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_VISACT
   { COL_BLACK,    COL_LIGHTCYAN,  DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_OVERCROSS
   { COL_LIGHTGRAY,COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_SRV_DICTADDINFO
   { COL_BLACK,    COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_TRNOTE_11,
   { COL_BLUE,     COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_TRNOTE_12,
   { COL_LIGHTGRAY,COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_TRNOTE_2,
   { COL_LIGHTGRAY,COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_DICTINDIC
   { COL_BLACK,    COL_LIGHTGRAY,  DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_UNMATCHTAG
   { COL_BLACK,    COL_WHITE,  DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_TFROMSCRATCH,
   { COL_BLACK,    COL_WHITE,  DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_TMODPROPOSAL,
   { COL_BLACK,    COL_WHITE,  DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_TCOPYPROPOSAL
   { COL_BLACK,    COL_LIGHTGRAY,  DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_STANDARD_TRNOTE
   { COL_BLACK,    COL_GREEN,  DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_DICTSTYLEPREF
   { COL_BLACK,    COL_RED,   DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_DICTSTYLENOT
   { COL_BLACK,    COL_LIGHTGRAY, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_MACHINE_MATCH_NORMAL
   { COL_RED  ,    COL_LIGHTGRAY, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_MACHINE_MATCH_PROT
   { COL_BLACK,    COL_LIGHTGREEN, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_FUZZY_MATCH_NORMAL
   { COL_RED,      COL_LIGHTGREEN, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_FUZZY_MATCH_PROT
   { COL_BLACK,    COL_WHITE, DEFAULT_FONT, FALSE, FALSE, },   // COLOUR_OWN_SETTING
   };
#else
//   extern TEXTTYPETABLE DefTextTypeTable[MAXCOLOUR];
#endif
__declspec(dllexport)
TEXTTYPETABLE* get_DefTextTypeTable();

//+----------------------------------------------------------------------------+
//|  currently active color/font values for text types                         |
//+----------------------------------------------------------------------------+
#if defined(INIT_TABLES)
  TEXTTYPETABLE TextTypeTable[MAXCOLOUR];
#else
//  extern TEXTTYPETABLE TextTypeTable[MAXCOLOUR];
#endif
__declspec(dllexport)
TEXTTYPETABLE* get_TextTypeTable();

//+----------------------------------------------------------------------------+
//|  structure definition for the function table                               |
//+----------------------------------------------------------------------------+
#define MAX_FUNCDESCRIPTION   70           // length of function description text

typedef struct _FUNCTIONTABLE
{
   USHORT usFuncID;                        // symbolic function identifier
   USHORT usAction;
   VOID (*function)( PTBDOCUMENT );        // function entry point
   CHAR    szDescription[MAX_FUNCDESCRIPTION]; // description of function -- stay ASCII
} FUNCTIONTABLE, *PFUNCTIONTABLE;

#define EDIT_NONE         0x00         // func not in any commands/keys dlg
#define EDIT_STANDARD     0x01         // func valid in standard editor
#define EDIT_RTF          0x02         // func valid in RTF editor
#define EDIT_DUMMY3       0x04         // dummy
//+----------------------------------------------------------------------------+
//|  Data structure for key to function table (also used as profile data)      |
//+----------------------------------------------------------------------------+
typedef struct _KEYPROFTABLE
{
   UCHAR   ucCode;                     // character code or virtual key
   UCHAR   ucState;                    // shift state
   EQF_BOOL fChange;                    // key has been changed
   BYTE    bEditor;                    // indicat. in which editor func is valid
   UCHAR   ucDummy;                    // not used
   USHORT   Function;                  // ID of function processing the key
} KEYPROFTABLE, *PKEYPROFTABLE;

//+----------------------------------------------------------------------------+
//|  Table structure for reserved keys                                         |
//+----------------------------------------------------------------------------+
typedef struct _RESKEYTABLE
{
   UCHAR   ucCode;                     // character code or virtual key
   UCHAR   ucState;                    // shift state
   SHORT   Function;                   // ID of function processing the key
   USHORT  usAssignStatus;             // Not assignable, assignable, or assigned
} RESKEYTABLE, *PRESKEYTABLE;

#define NOT_ASSIGNABLE    1            // defkey not usable by other func
#define ASSIGNABLE        2            // defkey can be used by other func
#define ASSIGNED_TO_OTHER 3            // defkey used by other function

/**********************************************************************/
/* General Part (formerly EQFB.H)                                     */
/**********************************************************************/



#endif


#ifndef RC_INCLUDED



/*********************************************************************
 * Data Structures                                                   *
 *********************************************************************/

 typedef struct _loadstruct
 {
    PSZ    pFileName;            // pointer to file name
    HWND   hwndParent;           // handle of parent
    RECTL  rclPos;               // position of this window
    ULONG  fsFlagStyle;          // style of this window
    BOOL   fReadOnly;            // read only flag
    DOCTYPE  docType;            // document type
    PSZ    pszTagTable;          // pointer to name of document tag table
    PSZ    pszEQFTagTable;       // pointer to name of EQF tag table
    PTBDOCUMENT pDoc;            // pointer to document structure
    BOOL   fReflow;              // reflow flag ( allow split/join or not )
    ULONG  flFrameStyle;         // frame style of windows
    PVOID  pstEQFGen;            // pointer to generic structure
    USHORT usEditor;             // editor used
 } LOADSTRUCT, * PLOADSTRUCT;

/* The keyboard 'key-code' to 'function' mapping tables - refer */
/* to EQFBCONF.c for more details.                              */

 typedef struct _keytable {
    int action;
    void (*function)( PTBDOCUMENT );
 } KEYTABLE;

typedef enum _status
{
    STATUS_NAME,
    STATUS_LINE,
    STATUS_COL,
    STATUS_MODE,
    MAXSTATUS
} STATUS;

/**********************************************************************/
/* macro to calculate the number of tokens in the list ...            */
/* and to adjust ends of tokens                                       */
/**********************************************************************/
#define  NUMBEROFTOKENS(usLenStr, pTokenList)   \
         {                                      \
           PFUZZYTOK pTest = pTokenList;        \
           usLenStr = 0;                        \
           while ( pTest->ulHash )              \
           {                                    \
             usLenStr++;                        \
             pTest++;                           \
           } /* endwhile */                     \
         }

#define MAX_REPL    2000               // number of replace strings
#define MAX_RANDOM    20               // length of random sequence
#define MAX_WORD_LEN 150               // length of word/ replace string

#endif


// data area for find dialog
typedef struct _FINDDATA
{
   CHAR_W chFind[ MAX_FINDCHANGE_LEN + 1];         // buffer for find string
   CHAR_W chFindTarget[ MAX_FINDCHANGE_LEN + 1];   // buffer for normalized find
   CHAR_W chReplace[ MAX_FINDCHANGE_LEN + 1];      // buffer for find string
   BOOL fChange;                                   // TRUE = Change
                                                   // FALSE = Find
   BOOL fIgnoreCase;                               // ignorecase (TRUE = IGNORE)
   BOOL fForward;                                  // true = forward direction
   USHORT usRange;                                 // range of selection
                                                   //   CHANGE_CURRENTTOEND
                                                   //   CHANGE_SEGMENTONLY
                                                   //   CHANGE_MARKONLY
   BOOL fConfirmChanges;                           // confirm on changes
   USHORT usFirstCall;                             // 0=not 1st call, 1 = 1st call
                                                   // 2=from spellcheck,only change,
                                                   // no find
   PTBDOCUMENT  pDoc;                              // active document
   ULONG  ulStartSegNum;                           // segnum of start
   ULONG  ulSegNumBegin;                           // segNum /segOffset where
   USHORT usSegOffsetBegin;                        // find starts searching
   EQFBBLOCK  MarkedBlock;                         // marked block in document
   HWND       hwndFindDlg;                         // modeless find dialog
   ULONG   ulFirstSegNum;                          // segnum at 1st call
   USHORT  usFirstSegOffset;                       // segOffset at 1st call
} FINDDATA, *PFINDDATA;

// data area for open dialog
typedef struct _OPENDATA
{
   EQF_BOOL   fTroja;                      // TRUE = troja doc, FALSE = text file
   // -------------- fields for open in troja mode --------------------------
   EQF_BOOL   fSource;                     // TRUE = source doc, FALSE = target doc
   CHAR   szDocPattern[MAX_LONGPATH];      // search pattern for documents (ASCII)
   CHAR   szDocName[MAX_LONGPATH];         // document object name (ASCII)
   // -------------- fields for open in text file mode ----------------------
   CHAR   szFilePattern[MAX_LONGPATH];     // search pattern for documents (ASCII)
   CHAR   szFileName[MAX_LONGPATH];        // fully qualified file name (ASCII)
} OPENDATA, *POPENDATA;

// data area for execute dialog
typedef struct _TPEXECUTE
{
   PTBDOCUMENT    pDoc;                // active document
   PKEYPROFTABLE  pKey;                // pointer to key entry
} TPEXECUTE, *PTPEXECUTE;


#ifndef RC_INCLUDED


/* dalia (start) */
/***********************************/
/* Arabic BIDI function prototypes */
/***********************************/
/* No ifdef BIDI here in purpose.. because the function is exported in .DEF file */

__declspec(dllexport)
VOID  EQFBBidiLRSwap ( BOOL );            // used in OS/2 and Win
FUNCTIONTABLE  EQFBBidiRedoLRSwap ( USHORT   usFunction );
VOID  EQFBBidiSysScrnCurPos ( PTBDOCUMENT, LONG, LONG );

VOID  EQFBPrepareBidi ( PTBDOCUMENT );
BOOL  EQFBCheckBidiPart ( PTBDOCUMENT, PSZ_W, ULONG, ULONG,
                          COLOUR, BYTE, ULONG );

__declspec(dllexport)
VOID  ReAllocArabicStruct( PTBDOCUMENT );

/*********************************************************************
 * Public Functions                                                  *
 *********************************************************************/
BOOL  EQFBFuncCharIn(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncHotPopUp( PTBDOCUMENT );
__declspec(dllexport)
void  EQFBFuncNothing(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncCaps(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncCharacter(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncLeft(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncRight(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncUp(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncDown(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncStartLine(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncStartSeg(PTBDOCUMENT);
void  EQFBFuncExactStartSeg(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncEndLine(PTBDOCUMENT);
void  EQFBFuncDoEndLine(PTBDOCUMENT, BOOL);
__declspec(dllexport)
void  EQFBFuncEndSeg(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncTab(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncBacktab(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncNextWord(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncPrevWord(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncDeleteChar(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncBackspace(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncTruncate(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncTruncSeg ( PTBDOCUMENT);    //  Truncate segment at cursor pos
__declspec(dllexport)
void  EQFBFuncNextLine(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncInsertLine(PTBDOCUMENT);
// Add for R012027 start
__declspec(dllexport)
void  EQFBFuncInsertChar(PTBDOCUMENT);
// Add end
__declspec(dllexport)
void  EQFBFuncDeleteLine(PTBDOCUMENT);
void  EQFBFuncRepeatLine(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncSplitLine(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncJoinLine(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncBottomDoc(PTBDOCUMENT);
void  EQFBFuncScrollUp(PTBDOCUMENT,BOOL);
__declspec(dllexport)
void  EQFBCursorScrollUp(PTBDOCUMENT);
void  EQFBFuncScrollDown(PTBDOCUMENT,BOOL);
__declspec(dllexport)
void  EQFBCursorScrollDown(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncScrollLeft(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncScrollRight(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncPageUp(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncPageDown(PTBDOCUMENT);
PTBDOCUMENT  EQFBFuncQuit(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncDoQuit(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncSave(PTBDOCUMENT);

void  EQFBFuncLeftRTF(PTBDOCUMENT);
void  EQFBFuncRightRTF(PTBDOCUMENT);
VOID  EQFBReplaceSelRTF( PTBDOCUMENT, PSZ_W );
VOID  DrawFormatChars(PTBDOCUMENT pDoc);

void  EQFBFuncFile(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncDoFile(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncNextDoc(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncPrevDoc(PTBDOCUMENT);
void  EQFBFuncCmdToggle(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncInsToggle(PTBDOCUMENT);
void  EQFBFuncHelp(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncUndo(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncRedo(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncEscape(PTBDOCUMENT);
__declspec(dllexport)
VOID  EQFBFuncDispOrg ( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncChangeToHide ( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncChangeToProt ( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncChangeToUnprot ( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncChangeToShrink ( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncChangeToCompact ( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncChangeToShorten ( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncChangeToWYSIWYG ( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncDispAllExactProps( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncLRM( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncRLM( PTBDOCUMENT );
VOID EQFBFuncRButtonDn( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncFind ( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncCFind ( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncOpen ( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncKeys ( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncEntry( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncFonts( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncLineWrap( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncMarginAct( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncToggleLineWrap( PTBDOCUMENT );
VOID  EQFBFuncRMargin( PTBDOCUMENT );
__declspec(dllexport)
VOID  EQFBFuncFontSize( PTBDOCUMENT );
void  EQFBUpdateChangedSeg(PTBDOCUMENT);                 // update changed seg
VOID  EQFBActivateSegm(PTBDOCUMENT, ULONG);              // activate current seg
VOID  EQFBSetFindData ( PSZ_W );
__declspec(dllexport)
PFINDDATA  EQFBGetFindData ( );
__declspec(dllexport)
USHORT EQFBCurrentState( PTBDOCUMENT);

__declspec(dllexport)
void  EQFBFuncMarkBlock(PTBDOCUMENT);
void  EQFBFuncSegMarkBlock(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkLeftCUA(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkRightCUA(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkUpCUA(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkDownCUA(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkStartCUA(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkEndCUA(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkSegStartCUA(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkSegEndCUA(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkClear(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkDelete(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkCopy(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkMove(PTBDOCUMENT);
__declspec(dllexport)
void  EQFBFuncMarkFind(PTBDOCUMENT);
void  EQFBFuncMarkCheck(PTBDOCUMENT);
__declspec(dllexport)
VOID  EQFBFuncMarkSegment (PTBDOCUMENT);
__declspec(dllexport)
VOID  EQFBFuncPasteFromClip ( PTBDOCUMENT  pDoc );       // paste from clipboard
__declspec(dllexport)
VOID  EQFBFuncCopyToClip ( PTBDOCUMENT  pDoc );          // copy to clipboard
__declspec(dllexport)
VOID  EQFBFuncCutToClip ( PTBDOCUMENT  pDoc) ;           // cut to clipboard
VOID  EQFBFuncFindLast(PTBDOCUMENT);
__declspec(dllexport)
BOOL  EQFBFuncStart(  PSZ, PSZ, PSTEQFGEN, BOOL, PTBDOCUMENT ); // start editor
VOID  EQFBFuncActDoc ( PTBDOCUMENT );
__declspec(dllexport)
BOOL  ActTransEnv( PTBDOCUMENT );
USHORT EQFBTenvStart ( PTBDOCUMENT, PSZ, PSTEQFGEN );

__declspec(dllexport)
VOID  EQFBFuncOpenTRNote(PTBDOCUMENT pDoc);           // open TRNote doc
__declspec(dllexport)
VOID  EQFBFuncTOCGoto(PTBDOCUMENT pDoc);              // invoke table of contents
__declspec(dllexport)
VOID  EQFBFuncSpellSeg(PTBDOCUMENT );                  //spellcheck segment
__declspec(dllexport)
VOID  EQFBFuncSpellFile(PTBDOCUMENT );                  //spellcheck file
__declspec(dllexport)
VOID  EQFBFuncWordMark(PTBDOCUMENT );                   //mark word
__declspec(dllexport)
VOID  EQFBFuncWordDel(PTBDOCUMENT );                    //delete word
VOID  EQFBFuncResetMarkInSeg(PTBDOCUMENT);
BOOL  EQFBFuncDelSel ( PTBDOCUMENT );
BOOL  EQFBDBCS2ND ( PTBDOCUMENT, BOOL);                // move cursor from 2nd
                                                       // DBCS byte
__declspec(dllexport)
VOID  EQFBFuncDelTilTag (PTBDOCUMENT );                //delete til next tag
__declspec(dllexport)
VOID  EQFBEmpty (PTBDOCUMENT );                       //delete til next tag
BOOL  EQFBOnTRNote(PTBDOCUMENT);               // update TRNote wnd if on TRNOTE
__declspec(dllexport)
VOID  EQFBFuncPropMarkCopy(PTBDOCUMENT);
__declspec(dllexport)
VOID  EQFBFuncSpellAuto(PTBDOCUMENT );           //turn autospellcheck on/off
__declspec(dllexport)
VOID  EQFBFuncNextMisspelled(PTBDOCUMENT);                 //goto next misspelled word
__declspec(dllexport)
VOID  EQFBFuncGoUpdSegment(PTBDOCUMENT);       // goto next updated segment

__declspec(dllexport)
VOID  EQFBFuncVisibleSpace(PTBDOCUMENT);

VOID EQFBTrans ( PTBDOCUMENT, USHORT ) ;       // send last seg to TM
                                               // activate next one
__declspec(dllexport)
VOID EQFBTSeg ( PTBDOCUMENT          ) ;       // translate segment
__declspec(dllexport)
VOID EQFBTUnTrans ( PTBDOCUMENT      ) ;       // untranslate a segment
__declspec(dllexport)
BOOL EQFBSaveSeg( PTBDOCUMENT );               // save current segment to tm
__declspec(dllexport)
VOID EQFBTSegNext ( PTBDOCUMENT      ) ;       // translate next segment
__declspec(dllexport)
VOID EQFBTSegNextExact ( PTBDOCUMENT ) ;       // translate next segment with EXACT matches
__declspec(dllexport)
VOID EQFBTSegNextFuzzy ( PTBDOCUMENT ) ;       // translate next segment with FUZZY matches
__declspec(dllexport)
VOID EQFBTSegNextNone ( PTBDOCUMENT  ) ;       // translate next segment with NO matches
__declspec(dllexport)
VOID EQFBTSegNextMT ( PTBDOCUMENT  ) ;         // translate next segment with MT matches
__declspec(dllexport)
VOID EQFBTSegNextGlobal ( PTBDOCUMENT  ) ;     // translate next segment with GLOBAL MEMORY matches
__declspec(dllexport)
VOID EQFBMark    ( PTBDOCUMENT       ) ;       // mark the active segment
__declspec(dllexport)
VOID EQFBClearMark( PTBDOCUMENT      ) ;       // clear the marked segment
__declspec(dllexport)
VOID EQFBFindMark ( PTBDOCUMENT      ) ;       // find the marked segment
__declspec(dllexport)
VOID EQFBActProp ( PTBDOCUMENT       ) ;       // activate transl. memory
__declspec(dllexport)
VOID EQFBPropUp  ( PTBDOCUMENT       ) ;       // proposal up
__declspec(dllexport)
VOID EQFBPropDown( PTBDOCUMENT       ) ;       // proposal down
__declspec(dllexport)
VOID EQFBDictPrefixUp( PTBDOCUMENT ) ;         // dictionary prefix up
__declspec(dllexport)
VOID EQFBDictPrefixDown( PTBDOCUMENT ) ;       // dictionary prefix down
__declspec(dllexport)
VOID EQFBActDict ( PTBDOCUMENT       ) ;       // activate dictionary window
__declspec(dllexport)
VOID EQFBDictUp  ( PTBDOCUMENT       ) ;       // dictionary up
__declspec(dllexport)
VOID EQFBDictDown( PTBDOCUMENT       ) ;       // dictionary down
__declspec(dllexport)
VOID EQFBGetDictMatch( PTBDOCUMENT   ) ;       // insert dictionary match
__declspec(dllexport)
VOID EQFBGetPropMatch( PTBDOCUMENT   ) ;       // insert proposal   match
__declspec(dllexport)
VOID EQFBGotoActSegment( PTBDOCUMENT );        // goto active segment
__declspec(dllexport)
VOID EQFBDictLook( PTBDOCUMENT );              // activate dictionary lookup
__declspec(dllexport)
VOID EQFBJoinSeg ( PTBDOCUMENT          ) ;    // join segments
__declspec(dllexport)
VOID EQFBSplitSeg ( PTBDOCUMENT         ) ;    // split prev. joined segments
__declspec(dllexport)
VOID EQFBAutoTranslate( PTBDOCUMENT );         // automatic translation (main)
VOID EQFBFuncAutoTrans( PTBDOCUMENT );         // automatic translation (doing)
VOID EQFBFuncAutoStop( PTBDOCUMENT );          // stop automatic translation
__declspec(dllexport)
VOID EQFBSetPostEdit( PTBDOCUMENT );           // post editing mode
BOOL EQFBTempPostEdit( PTBDOCUMENT );          // temporary post edit mode

__declspec(dllexport)
VOID EQFBCommand (PTBDOCUMENT);                // exec. the sel. function
__declspec(dllexport)
VOID EQFBSettings(PTBDOCUMENT);                // invoke the settings dialog ..
__declspec(dllexport)
VOID EQFBGotoLine(PTBDOCUMENT);                // invoke the GotoLine dialog ..
__declspec(dllexport)
VOID EQFBGotoSegment(PTBDOCUMENT);             // invoke the Goto segment dialog ..
__declspec(dllexport)
VOID EQFBQueryLine(PTBDOCUMENT);               // find the current cursor pos.
BOOL EQFBFindLine( PTBDOCUMENT, ULONG  );      // find the specified line
__declspec(dllexport)
VOID EQFBDocPrint( PTBDOCUMENT );              // print a document
__declspec(dllexport)
VOID EQFBFuncMarkNextWord ( PTBDOCUMENT pDoc);
__declspec(dllexport)
VOID EQFBFuncMarkPrevWord ( PTBDOCUMENT pDoc);
__declspec(dllexport)
VOID EQFBFindScratch(PTBDOCUMENT pDoc);        //find next seg. from scratch
__declspec(dllexport)
VOID EQFBFindCopy(PTBDOCUMENT pDoc);           //find next seg. prop.copied
__declspec(dllexport)
VOID EQFBFindCpyTyped(PTBDOCUMENT pDoc);         //find next seg.copied&typed in

__declspec(dllexport)
VOID EQFBExpandSeg(PTBDOCUMENT pDoc);
__declspec(dllexport)
VOID EQFBCompressSeg(PTBDOCUMENT pDoc);

__declspec(dllexport)
VOID EQFBActTrans( PTBDOCUMENT       ) ;      // activate translation window
VOID EQFBProfInit(PTBDOCUMENT);               // invoke profile initials..
__declspec(dllexport)
VOID EQFBFuncDispSrcProp(PTBDOCUMENT);        // activate source of prop wnd
__declspec(dllexport)
VOID EQFBFuncDispSegProp(PTBDOCUMENT);        // activate segment properties window
__declspec(dllexport)
VOID EQFBFuncEditTerm(PTBDOCUMENT);           // edit selected term
__declspec(dllexport)
VOID EQFBAddAbbrev(PTBDOCUMENT);              // add abbreviation to morph dict
__declspec(dllexport)
VOID EQFBAddAbbrevFunc(PTBDOCUMENT, SHORT);   // add abbreviation to spec. language
VOID EQFBEditAbbrev(PTBDOCUMENT);             // edit abbreviation dictionary
__declspec(dllexport)
VOID EQFBEditAddenda(PTBDOCUMENT);            // edit addenda dictionary

__declspec(dllexport)
VOID EQFBFuncShowTrans(PTBDOCUMENT);             // show transl. in test wnd

__declspec(dllexport)
VOID EQFBFileInsertSOSI(PTBDOCUMENT pDoc);    //insert SOSI in all xlated segs
__declspec(dllexport)
VOID EQFBFileDeleteSOSI(PTBDOCUMENT pDoc);       //delete SOSI in all xlated segs
__declspec(dllexport)
VOID EQFBSegInsertSOSI(PTBDOCUMENT pDoc);     //insert SOSI in active segment
__declspec(dllexport)
VOID EQFBSegDeleteSOSI(PTBDOCUMENT pDoc);        //delete SOSI in active segment

__declspec(dllexport)
VOID EQFBAddFuzzyMatch(PTBDOCUMENT pDoc);       //display fuzzy + exact match

SHORT EQFBPosGotoSeg(PTBDOCUMENT,ULONG,PSZ_W,SHORT);//find pos of word in
                                                    //segment
VOID EQFBWordPos (PTBDOCUMENT,PUSHORT,PUSHORT);     //find sStart & sEnd of
                                               //current word
__declspec(dllexport)
VOID EQFBScreenCursor( PTBDOCUMENT);           // set cursor and slider
void EQFBScreenSegment (  PTBDOCUMENT, LONG, PTBROWOFFSET, ULONG );

__declspec(dllexport)
USHORT EQFBCharType( PTBDOCUMENT, PTBSEGMENT, USHORT );        // in EQFBFILE.C
VOID EQFBNormSeg( PTBDOCUMENT, PSZ_W, PSZ_W );                 // in EQFBFILE.C
BOOL EQFBCheckNoneTag ( PTBDOCUMENT, PSZ_W );                // in EQFBFILE.c
VOID EQFBReparse(PTBDOCUMENT, PTBSEGMENT,USHORT,SHORT);    // in EQFBFIle,c
BOOL EQFBDiffTag ( PTBDOCUMENT, ULONG, USHORT);            // in EQFBFIle.c
BOOL EQFBDiffProtectTag ( PTBDOCUMENT, ULONG, USHORT);     // in EQFBFIle.c
VOID EQFBCheckEndOfSeg(PTBDOCUMENT,PTBSEGMENT,USHORT);     // in EQFBUtl.c

__declspec(dllexport)
BOOL EQFBDocWndCreate ( PTBDOCUMENT ,        // pointer to document structure
                        PLOADSTRUCT ) ;      // pointer to load structure
SHORT EQFBDocInsert( PLOADSTRUCT );         // insert a new document
SHORT EQFBDocLoad( PLOADSTRUCT );           // load a new document
__declspec(dllexport)
void  EQFBFuncClose( PSTEQFGEN );
VOID   EQFBLineNum( PTBDOCUMENT   );         // determine number of lines in doc

__declspec(dllexport)
VOID EQFBVScroll( PTBDOCUMENT, USHORT, USHORT); // vertical scroll
__declspec(dllexport)
VOID EQFBHScroll( PTBDOCUMENT, USHORT, USHORT); // horizontal scroll

 SHORT  EQFBLineUp(PTBDOCUMENT);
 SHORT  EQFBLineDown(PTBDOCUMENT);

 void EQFBScreenSize(PTBDOCUMENT);                       /* See EQFBScreen.c */
__declspec(dllexport)
 void EQFBScreenData(PTBDOCUMENT);
 void EQFBScreenCmd(PTBDOCUMENT);

__declspec(dllexport)
 void EQFBScreenCursorType(PTBDOCUMENT);

 PTBDOCUMENT EQFBDocDelete(PTBDOCUMENT);
 SHORT EQFBDocSave( PTBDOCUMENT, PSZ, BOOL );
 int  EQFBDocName(char *newname);

 USHORT EQFBFilePrint( PTBDOCUMENT pDoc);                          // EQFBFILE.C
 __declspec(dllexport)
 SHORT  EQFBReadProfile( VOID );                                   // EQFBFILE.C
 __declspec(dllexport)
 SHORT  EQFBWriteProfile( PTBDOCUMENT );                                  // EQFBFILE.C
__declspec(dllexport)
 BOOL   EQFBFileExists( PSZ pszFile );                             // EQFBFILE.C

 void EQFBWorkClear(PTBDOCUMENT);                        /* See EQFBWORK.C */
 void EQFBWorkLineIn(PTBDOCUMENT);
 void EQFBWorkLineOut(PTBDOCUMENT);
 void EQFBWorkRight(PTBDOCUMENT pDoc, USHORT usPosition, USHORT usCount);
 void EQFBWorkLeft(PTBDOCUMENT pDoc, USHORT usPosition, USHORT usCount);
 BOOL EQFBStillInActSeg( PTBDOCUMENT pDoc );

 __declspec(dllexport)
VOID EQFBScrnLinesFromSeg                            // convert segs into lines
                    ( PTBDOCUMENT  pDoc,              // ptr to doc ida
                      LONG         lStartRow,         // starting row
                      LONG         lNumRows,          // number of rows
                      PTBROWOFFSET pTBRow);           // starting segment

 VOID EQFBFillPrevTBRow
                    ( PTBDOCUMENT  pDoc,              // pointer to doc ida
                      LONG         lStartRow );       // starting row

__declspec(dllexport)
 VOID EQFBPhysCursorFromSeg ( PTBDOCUMENT  pDoc);     // pointer to doc ida

 LONG EQFBCurSegFromCursor ( PTBDOCUMENT  pDoc);     // get cursor pos from seg

__declspec(dllexport)
 void EQFBWorkSegIn ( PTBDOCUMENT pDoc  );            // move working seg in
__declspec(dllexport)
 void EQFBWorkSegOut( PTBDOCUMENT pDoc  );            // move working seg out

 VOID EQFBWorkSegCheck ( PTBDOCUMENT pDoc );          // check work line change

__declspec(dllexport)
 VOID EQFBCompSeg ( PTBSEGMENT  pTBSeg );             // (re)compute segment

__declspec(dllexport)
 VOID   EQFBValidatePositions( PRECTL, USHORT );      // validate sizes in edit.

__declspec(dllexport)
 PTBSEGMENT EQFBGetPrevVisSeg ( PTBDOCUMENT pDoc,     // find prev 'real' segm.
                                PULONG pulSegNum);
__declspec(dllexport)
VOID EQFBGetDefCellSize ( PVIOFONTCELLSIZE,
                          PCHAR );                      // get default size&font
 void EQFBQuit (HWND hwnd);

 __declspec(dllexport)
void  EQFBFuncReImportDoc(PTBDOCUMENT);


#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQF_XSTART(  PSZ, PSZ, PSZ, PSTEQFGEN);   // start editor
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQF_XSTOP( PSZ, PSTEQFGEN, EQF_BOOL );    // end editor

void  EQFBSysScrnUpdate ( PTBDOCUMENT pDoc,              // allow screen update
                          BOOL        fUpdate);

VOID EQFBSysDispRestOfLine (PTBDOCUMENT); // displ rest of line ida

 VOID EQFBFuncPad(PTBDOCUMENT  pDoc, LONG lPos);
VOID ActivateMTLog( PSZ pDocName, PSZ pszTagTable );
//ษออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออป
//บ EQF Translation Browser - header file of dialogs                           บ
//ศออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออออผ
SHORT EQFBCmdFind(PTBDOCUMENT, PFINDDATA);
SHORT EQFBCmdChange( PTBDOCUMENT, PFINDDATA );
__declspec(dllexport)
VOID EQFBChangeStyle ( PTBDOCUMENT, USHORT);     // selected style
#define TPRO_MAPKEY 0
#define ITM_MAPKEY 1
__declspec(dllexport)
BOOL EQFBMapKey ( WINMSG, WPARAM, LPARAM, PUSHORT, PUCHAR , USHORT );
BOOL EQFBConvKey ( WPARAM, PUCHAR, PUCHAR );
__declspec(dllexport)
BOOL EQFBKeyName ( PSZ pszBuffer, UCHAR ucCode, UCHAR ucState );
void EQFBKeyNameW(wchar_t * pwszKeyName, UCHAR ucCode, UCHAR ucState);    // Add for R012027
VOID EQFBLoadResource (VOID);
VOID EQFBSetKeyName ( HMENU hwndMenu, SHORT sItem, FUNCTION Function );
BOOL EQFBBuildKeyTable ( PKEYPROFTABLE pKeyList );
__declspec(dllexport)
VOID EQFBBuildITMKeyTable ( PKEYPROFTABLE pKeyList, USHORT usLast );
__declspec(dllexport)
VOID EQFBAdaptKeyTable ( PKEYPROFTABLE pKeyList, PTBDOCUMENT pDoc );
VOID EQFBExecuteKey ( PTBDOCUMENT, PKEYPROFTABLE );

BOOL EQFBUtlSpell ( PSTEQFGEN );                 // check avail. of spell check
__declspec(dllexport)
BOOL EQFBCheckSpellLang ( PSZ pszLang );        // check avail. of spell check
BOOL EQFBCheckTags ( PTBDOCUMENT );              // check for equal tags..
BOOL EQFBDoTagCheck( PTBDOCUMENT );              // check for same tags ..
VOID EQFBFuncDoLineWrap ( PTBDOCUMENT pDoc );              // do line wrap

BOOL TACheckColumnPos ( PSZ_W         pData,          // ptr to data
                        PVOID         pVoidTable,     // pointer to tag table
                        PTOKENENTRY   pTokBuffer,     // buffer used for tokens
                        USHORT        usTokBufferSize,// token buffer size
                        PFN           pvUserExit);    // ptr to user exit function
#define DELETE_SOSI 1                            // delete SHIFTIN SHIFTOUT
#define INSERT_SOSI 2                            // insert SHIFTIN SHIFTOUT

BOOL EQFBConvertSOSI ( PTBDOCUMENT, PTBSEGMENT , USHORT );
__declspec(dllexport)
BOOL EQFBUtlConvertSOSI( PSZ_W, PULONG, PSZ_W, USHORT, ULONG );

__declspec(dllexport)
VOID EQFBDocPosOnScreen ( PLOADSTRUCT );

/**********************************************************************/
/* defines used in display                                            */
/**********************************************************************/
#define DISP_MARK           1
#define DISP_MISSPELLED     2
#define DISP_SEGMENT_START  4
#define DISP_NOREORDER          8

__declspec(dllexport)
 void EQFBSysInit(PTBDOCUMENT);
 void EQFBSysExit(PTBDOCUMENT);
 void EQFBSysScrnText(PTBDOCUMENT, ULONG, ULONG, PSZ_W, ULONG, COLOUR, BYTE, PSZ_W, PULONG );
 VOID EQFBSysScrnTextRTF ( PTBDOCUMENT, USHORT, USHORT, PSZ_W, USHORT, CHARFORMAT2 *, BYTE );
 VOID EQFBSysPadRTF( PTBDOCUMENT, SHORT, SHORT );
 BOOL EQFBCreateRichEditCtrl( PTBDOCUMENT, PFNWP );
 VOID EQFBScreenDataRTF( PTBDOCUMENT pDoc );
 VOID EQFBDispFileRTF( PTBDOCUMENT pDoc );
 VOID EQFBToggleInsertRTF( PTBDOCUMENT pDoc );

 void EQFBSysScrnChar(PTBDOCUMENT, ULONG, ULONG, LONG, COLOUR, BYTE );
 void EQFBSysScrnClear(COLOUR colour);
 void EQFBSysScrnCurPos(PTBDOCUMENT, LONG, LONG);
 VOID EQFBSysScrnCurPosRTF( PTBDOCUMENT, LONG , LONG );
 void EQFBSysScrnCurShape(PTBDOCUMENT, CURSOR);
__declspec(dllexport)
 VOID EQFBSysFilename( PSZ pszNewName, PSZ pszName);

 BOOL EQFBSendNextSource( PTBDOCUMENT, PULONG, BOOL, USHORT);
 BOOL EQFBCopyPropMatch( PTBDOCUMENT, USHORT, USHORT, BOOL);
 VOID EQFBDoNextTwo( PTBDOCUMENT, PULONG, USHORT );  // send next two segments
 VOID EQFBDocIsTranslated( PTBDOCUMENT ); // handling for document is xlat
 PTBSEGMENT EQFBFindNextSource( PTBDOCUMENT, PULONG, USHORT, PBOOL, BOOL, PVOID *);
__declspec(dllexport)
 VOID SetScrollbar( PTBDOCUMENT );
 MRESULT EQFBFindClose( HWND, WPARAM, LPARAM );
__declspec(dllexport)
 MRESULT EQFBFindCloseModeless( HWND, WPARAM, LPARAM );
 MRESULT EQFBCFindClose( HWND, WPARAM, LPARAM );
 VOID EQFBCFind( PTBDOCUMENT pDoc );
__declspec(dllexport)
 VOID EQFBCFindTerminate( PTBDOCUMENT pDoc );

 ULONG  EQFBQueryActLine(PTBDOCUMENT, ULONG, USHORT);
PTBDOCUMENT EQFBRemoveDoc ( PTBDOCUMENT  pDoc );

 PTBSEGMENT EQFBGetShrinkSeg( PTBDOCUMENT, PULONG ,SHORT);


/*********************************************************************
 * Global Data                                                       *
 *********************************************************************/
#if defined(INIT_TABLES)
   FILE *hMTLog;                       // handle of mt log file



    EQFBBLOCK EQFBBlockMark;           // marked area
    USEROPT   EQFBUserOpt;             // user options set ....

    HACCEL    hMainAccel;              // main window accelerator
    HACCEL    hChildAccel;             // child window accelerator
    OPENDATA  OpenData;                // data structure for open dialog
    USHORT    usRightMargin;           // value of right margin
    PPROPSYSTEM pSysProp;              // ptr to EQF system properties
                                       // if NULL load of properties failed
    PLOADEDTABLE pLoadedTables;        // pointer to loaded tag tables

    VIOFONTCELLSIZE vioFontSize[MAX_DIF_DOC];              // default font sizes
#else
   extern FILE *hMTLog;                       // handle of mt log file



   extern  EQFBBLOCK EQFBBlockMark;           // marked area
   extern  USEROPT   EQFBUserOpt;             // user options set ....

//   extern  HACCEL    hMainAccel;              // main window accelerator
//   extern  HACCEL    hChildAccel;             // child window accelerator
//   extern  OPENDATA  OpenData;                // data structure for open dialog
//   extern  USHORT    usRightMargin;           // value of right margin
//   extern  PPROPSYSTEM pSysProp;              // ptr to EQF system properties
                                       // if NULL load of properties failed
   extern  PLOADEDTABLE pLoadedTables;        // pointer to loaded tag tables

//   extern  VIOFONTCELLSIZE vioFontSize[MAX_DIF_DOC];    // default font sizes
#endif
__declspec(dllexport)
FILE* get_hMTLog();
__declspec(dllexport)
void set_hMTLog(FILE* hMTLogNew );
__declspec(dllexport)
EQFBBLOCK* get_EQFBBlockMark();
__declspec(dllexport)
USEROPT* get_EQFBUserOpt();
__declspec(dllexport)
OPENDATA* get_OpenData();
__declspec(dllexport)
USHORT* get_usRightMargin();
__declspec(dllexport)
PPROPSYSTEM get_pSysProp();
__declspec(dllexport)
void set_pSysProp(PPROPSYSTEM pNewSysProp);
__declspec(dllexport)
VIOFONTCELLSIZE* get_vioFontSize();
#endif

//  WM IDs  used for Spellcheck dialog
#ifndef WM_EQF_PROOF
#define WM_EQF_PROOF         WM_EQF_END // send a segment to proofread
#endif

#ifndef WM_EQF_MISSPELLED
#define WM_EQF_MISSPELLED    WM_EQF_END+1 // work on misspelled word
#endif

#ifndef WM_EQF_AUTOTRANS
#define WM_EQF_AUTOTRANS     WM_EQF_END+2 // automatic translation
#endif


/**********************************************************************/
/* Dialog Part (formerly EQFBDLG1.H)                                  */
/**********************************************************************/

//
// table structure for keys assigned to functions
//

//
// structure for text elements and associated font and color
//
typedef struct _FUNCTIONENTRY
{
   UCHAR   ucCode;                     // character code or virtual key
   UCHAR   ucState;                    // shift state
   BOOL    fChange;                    // key has been changed
   UCHAR   ucOrgCode;                  // org character code or virtual key
   UCHAR   ucOrgState;                 // org shift state
   void    (*pFunction)(PTBDOCUMENT);  // function processing the key
   CHAR    szDescription[MAX_DESCRIPTION]; // desription of function (ASCII)
} FUNCTIONENTRY, *PFUNCTIONENTRY;

//+----------------------------------------------------------------------------+
//|  Dialog instance data areas                                                |
//+----------------------------------------------------------------------------+


// Keys Dialog IDA
typedef struct _KEYSIDA
{
   CHAR            szItemName[256];    // buffer for item names
   BOOL            fCapture;           // TRUE = capture next keystroke
   PKEYPROFTABLE   pKey;               // currently selected key entry
   SHORT           sItem;              // currently selected item
   HWND            hwndFrame;          // the window's frame
   HACCEL          haccWindow;         // the window's original accelerator table
   PKEYPROFTABLE   pOrgKeyList;        // original key assignments
   USHORT          usOrgKeyListLength; // length of original key list
   PKEYPROFTABLE   pNewKeyList;        // changed key assignments
   PFUNCTIONTABLE  pFuncList;          // function list
   PRESKEYTABLE    pResKeys;           // list with reserved keys
   BOOL            afFuncUsed[LAST_FUNC];        // array of function used flags
   PTBDOCUMENT     pDoc;               // current doc structure
} KEYSIDA, *PKEYSIDA;

// Open Dialog IDA
typedef struct _OPENIDA
{
   //----------------- callers OPENDATA structure ----------------------------
   POPENDATA   pOpenData;                    // ptr to open dialog data area
   //------------------------ flags -----------------------------------
   BOOL   fTroja;                            // TRUE = troja doc,
                                             // FALSE = text file
   BOOL   fInit;                             // TRUE = INITDLG processing
                                             // FALSE = normal (user) processing
   //----------------- drive and path information ---------------------
   CHAR        szDrives[MAX_DRIVELIST];      // list of available drives
   CHAR        szEqfDrives[MAX_DRIVELIST];   // list of EQF drives
   CHAR        chPrimDrive;                  // EQF primary drive
   //----------------- current values for text file mode ---------------------
   CHAR        chDrive;                      // active drive (file mode)
   CHAR        szFilePattern[MAX_LONGPATH];  // active pattern (file mode)
   CHAR        szDirectory[MAX_LONGPATH];    // active directory (file mode)
   CHAR        szFile[MAX_LONGPATH];         // active file (file mode)
   //----------------- current values for document mode ---------------------
   BOOL        fSource;                      // TRUE = SOURCE RB selected
   CHAR        szDocPattern[MAX_LONGPATH];   // active pattern (doc mode)
   CHAR        szFolder[MAX_LONGPATH];       // active folder (doc mode)
   CHAR        szDocument[MAX_LONGPATH];     // active document (doc mode)
   PPROPFOLDER pFolProp;                     // properties of active folder
   //--------------------- general purpose buffers --------------------------
   CHAR        szPath[MAX_LONGPATH];         // buffer for path names
   //--------------------- string loaded from resource
   CHAR        szName[40];                   // name field
   CHAR        szDocName[40];                // document name field
   CHAR        szFolderText[40];             // folder text
   CHAR        szDocuments[40];              // documents
   CHAR        szDirectoryText[40];          // directory
   CHAR        szFiles[40];                  // files
   CHAR        szLongDocName[MAX_LONGPATH];  // long document object name
} OPENIDA, *POPENIDA;

/**********************************************************************/
/* struct for PPROFONEIDA                                             */
/**********************************************************************/
typedef struct _USERSET
{
   CHAR        szLBItemName[60];       // name of lb item
   BOOL        fMarked;                // true if useropt true
   SHORT       sItemIDS;               // IDS of this user setting
} USERSET, *PUSERSET;


// Profile ONE Dialog IDA  (initial values)
typedef struct _PROFONEIDA
{
   USERSET     LBItemArray [MAXLBITEMS];       // text of listbox
   CHAR        szBuffer [60];                    // get choice from listbox
   PTBDOCUMENT pDoc;
   HBITMAP     hbm;                              // menucheck bitmap
   CHAR        szStyleArray [MAXSTYLES][40];     // text for prop.wnd.styles
   CHAR        szMarginBuffer[RMARGIN_LENGTH + 1];    // text for rmargin cbs
   HWND        hwndPages[10];                    // support up to 10 prop pages
#ifdef R004422_BACKSAVE
   CHAR        szMinute[5];                      // text for minutes CBS
#endif
} PROFONEIDA, *PPROFONEIDA;

// TOC goto diealog   IDA
typedef struct _TOCGOTOIDA
{
   PTBDOCUMENT pDoc;
   HWND        hwndLB;                           // menucheck bitmap
} TOCGOTOIDA, *PTOCGOTOIDA;

// Private tasks processed using WM_EQF_PROCESSTASK
#define FILLFROMFIELD_TASK   USER_TASK + 1       // fill controls from SLE


//+----------------------------------------------------------------------------+
//|  data areas for communication between dialog and calling function          |
//+----------------------------------------------------------------------------+

#define CH_FINDFIRST   1                   // 1st call of change(only find)
#define CH_CHNGEFIND   0                   // change then find
#define CH_CHNGEONLY   2                   // only change, no find
                                           // ( if called from spellcheck)
#define CH_MATCHOUTRANGE  3                //match found but out of range

// data area for font/color dialog
typedef struct _FONTCOLDATA
{
   CHAR_W chDummy[80];
   PTEXTTYPETABLE pTextTypeTable;      // ptr to active text type table
} FONTCOLDATA, *PFONTCOLDATA;

// Font/Color Dialog IDA
typedef struct _FONTCOLIDA
{
   HWND        hwndSample;             // handle of sample text control
   BYTE        bCellString[4096];      // buffer for cell string
   PTEXTTYPETABLE pTextTable;          // ptr to active text type table
   PTEXTTYPETABLE pTextType;           // ptr to active text type entry
   PFONTCOLDATA   pFontColData;        // ptr to passed data structure
   CHAR        szSampleText[40];       // space for sample text
   PTBDOCUMENT pDoc;                   // active document
} FONTCOLIDA, *PFONTCOLIDA;


// data area for key assignment dialog
typedef struct _KEYSDATA
{
   HWND            hwndFrame;          // the window๏s original frame window
   PKEYPROFTABLE   pKeyList;           // key assignments
   PFUNCTIONTABLE  pFuncList;          // function list
   PRESKEYTABLE    pResKeys;           // list with reserved keys
   PTBDOCUMENT     pDoc;               // current document structure
} KEYSDATA, *PKEYSDATA;

// data area for key assignment dialog
typedef struct _ENTRYDATA
{
   PTBDOCUMENT     pDoc;                             // active document
   BOOL            fChanged;                         // proposal changed
   CHAR_W          chSegBuffer[ MAX_SEGMENT_SIZE ];  // segment buffer
} ENTRYDATA, *PENTRYDATA;



//#define MAX_FACES     16
#define MAX_FACES     128   // enlarged - 2002-09

#ifndef LF_FACESIZE
  #define  LF_FACESIZE 32
#endif

typedef struct
  {
    SHORT    nNumFaces;
    CHAR     szFaceName [MAX_FACES][LF_FACESIZE];
    BYTE     bTrueType[MAX_FACES];
  }
  ENUMFACE, *PENUMFACE;


// data area for font size dialog
typedef struct _FONTSIZEDATA
{
   PTBDOCUMENT     pDoc;                             // active document
   HWND            hwnd;                             // window handle
   LONG            lOutData[100];                    // space for outdata
   HWND            hwndSample;                       // handle for sample window
   CHAR            szSampleText[40];                 // space for sample text
   CHAR            szDummyText[40];                  // space for dummy text
   VIOFONTCELLSIZE vioFontCell[ MAX_DIF_DOC ];             // default font sizes
   CHAR            chFontFacename[MAX_DIF_DOC][LF_FACESIZE]; //facename per doc

   ENUMFACE        efAllFaces;                      // all fixedpitch fonts
//   CHAR            szBuffer[8096]; // buffer to catch overflows in efAllFaces...
} FONTSIZEDATA, *PFONTSIZEDATA;

// data area for spell dialog
typedef struct _SPELLDATA
{
   ULONG   ulProofSeg;                           // no. of segment proofing
   ULONG   ulStartSeg;                           // 1st segment proofing
   CHAR_W  chProofData[MAX_SEGMENT_SIZE];        // size for returned list
   CHAR_W  chAidData[MAX_SEGMENT_SIZE];      // field for aid data
   ULONG   ulActSeg;                             // act.seg. before spellcheck
   SHORT   sStartOffset;                         // startpos for EQFBPosGotoSeg
   BOOL    fSegOnly;                             // true if check 1 seg
   BOOL    fPostEditOld;                         // store posteditflag to reset
   PCHAR_W pIgnoreNextFree;                      // ptr next free pos in IgnoreData
   PCHAR_W pProofCurrent;                        // current word in chProofData
   PTBDOCUMENT  pDoc;                            // active document
   FINDDATA FindData;                            // to use find/change routine
   PCHAR_W pIgnoreData;                          // temp addenda buffer
   USHORT  usIgnoreLen;                          // length of allocated ignore data
   PCHAR_W pSpellWorkSegW;                       // worksegment for spell-thread
   TBROWOFFSET TBFirstLine;                      // RowOffset of 1st line of scrn
   PTBSEGMENT  pSpellSeg;                        // work copy of pDoc->pTBSeg
} SPELLDATA, * PSPELLDATA;

//+----------------------------------------------------------------------------+
//|  prototypes of dialog window procedures                                    |
//+----------------------------------------------------------------------------+
MRESULT APIENTRY EQFBFINDDLGPROCMODELESS( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBFINDDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBOPENDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBFONTCOLDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBKEYSDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBENTRYDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
MRESULT APIENTRY EQFBRMARGINDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBFONTSIZEDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBSPELLDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBCOMMANDDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBSETTINGSDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBGOTOLINEDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBPROFINITDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBTOCGOTODLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBCFINDDLGPROC( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK EQFBCOMMENTDLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );

// spellcheck dialog return codes
#define SPELLCHECK_CLOSED_BY_USER  1    // dialog closed by user 
#define SPELLCHECK_GO_TO_NEXT_DOC  2    // dialog closed to continue spellchecking of next document in the list
#define SPELLCHECK_LAST_DOC_DONE   3    // dialog closed to complete spellchecking of a list of documents
#define SPELLCHECK_CLOSED_BY_ERROR 0    // dialog closed because of errors

BOOL EQFBShowSegmentComment
(
  HWND        hwnd,                    // parent window handle
  PTBDOCUMENT pDoc,                    // document
  PVOID       pvAddInfo,               // additional info data of segment
  SHORT       x,                       // x position of click
  SHORT       y                        // y position of click
);

//+----------------------------------------------------------------------------+
//|  Table of functions and function names                                     |
//+----------------------------------------------------------------------------+
#if defined(INIT_TABLES)
FUNCTIONTABLE FuncTab[] = {
// Symbolic            Action    Entry point                  Description
//====================================================================================
{ ACTPROP_FUNC,        STARGET,  EQFBActProp,                 "" },
{ DICTDOWN_FUNC,       STARGET,  EQFBDictDown,                "" },
{ DICTUP_FUNC,         STARGET,  EQFBDictUp,                  "" },
{ BACKSPACE_FUNC,      ACTSEG,   EQFBFuncBackspace,           "" },
{ BACKTAB_FUNC,        0,        EQFBFuncBacktab,             "" },
{ BOTTOMDOC_FUNC,      OVERALL,  EQFBFuncBottomDoc,           "" },
{ DELETECHAR_FUNC,     STARGET,  EQFBFuncDeleteChar,          "" },
{ DOWN_FUNC,           OVERALL,  EQFBFuncDown,                "" },
{ ENDLINE_FUNC,        OVERALL,  EQFBFuncEndLine,             "" },
{ ENDSEG_FUNC,         STARGET,  EQFBFuncEndSeg,              "" },
{ ESCAPE_FUNC,         0,        EQFBFuncEscape,              "" },
{ FILE_FUNC,           0,        EQFBFuncDoFile,              "" },
{ INSTOGGLE_FUNC,      OVERALL,  EQFBFuncInsToggle,           "" },
{ INSERTLINE_FUNC,     ACTSEG,   EQFBFuncInsertLine,          "" },
{ JOINLINE_FUNC,       ACTSEG,   EQFBFuncJoinLine,            "" },
{ LEFT_FUNC,           OVERALL,  EQFBFuncLeft,                "" },
{ NEXTDOC_FUNC,        OVERALL,  EQFBFuncNextDoc,             "" },
{ NEXTWORD_FUNC,       OVERALL,  EQFBFuncNextWord,            "" },
{ NOTHING_FUNC,        0,        EQFBFuncNothing,             "" },
{ PAGEDOWN_FUNC,       OVERALL,  EQFBFuncPageDown,            "" },
{ PAGEUP_FUNC,         OVERALL,  EQFBFuncPageUp,              "" },
{ PREVDOC_FUNC,        OVERALL,  EQFBFuncPrevDoc,             "" },
{ PREVWORD_FUNC,       OVERALL,  EQFBFuncPrevWord,            "" },
{ QUIT_FUNC,           0,        EQFBFuncDoQuit,              "" },
{ RIGHT_FUNC,          OVERALL,  EQFBFuncRight,               "" },
{ SAVE_FUNC,           0,        EQFBFuncSave,                "" },
{ SCROLLDOWN_FUNC,     OVERALL,  EQFBCursorScrollDown,        "" },
{ SCROLLLEFT_FUNC,     OVERALL,  EQFBFuncScrollLeft,          "" },
{ SCROLLRIGHT_FUNC,    OVERALL,  EQFBFuncScrollRight,         "" },
{ SCROLLUP_FUNC,       OVERALL,  EQFBCursorScrollUp,          "" },
{ SPLITLINE_FUNC,      ACTSEG,   EQFBFuncSplitLine,           "" },
{ STARTLINE_FUNC,      OVERALL,  EQFBFuncStartLine,           "" },
{ STARTSEG_FUNC,       STARGET,  EQFBFuncStartSeg,            "" },
{ TAB_FUNC,            OVERALL,  EQFBFuncTab,                 "" },
{ TOPDOC_FUNC,         OVERALL,  EQFBFuncTopDoc,              "" },
{ TRUNCSEG_FUNC,       ACTSEG,   EQFBFuncTruncSeg,            "" },
{ TRUNCATE_FUNC,       ACTSEG,   EQFBFuncTruncate,            "" },
{ UNDO_FUNC,           ACTSEG,   EQFBFuncUndo,                "" },
{ UP_FUNC,             OVERALL,  EQFBFuncUp,                  "" },
{ GETDICTMATCH_FUNC,   ACTSEG,   EQFBGetDictMatch,            "" },
{ GETPROPMATCH_FUNC,   ACTSEG,   EQFBGetPropMatch,            "" },
{ PROPDOWN_FUNC,       STARGET,  EQFBPropDown,                "" },
{ PROPUP_FUNC,         STARGET,  EQFBPropUp,                  "" },
{ TSEG_FUNC,           STARGET,  EQFBTSeg,                    "" },
{ TSEGNEXT_FUNC,       STARGET,  EQFBTSegNext,                "" },
{ DICTLOOK_FUNC,       OVERALL,  EQFBDictLook,                "" },
{ ACTDIC_FUNC,         OVERALL,  EQFBActDict,                 "" },
{ JOINSEG_FUNC,        ACTSEG,   EQFBJoinSeg,                 "" },
{ SPLITSEG_FUNC,       ACTSEG,   EQFBSplitSeg,                "" },
{ MARKSEG_FUNC,        STARGET,  EQFBMark,                    "" },
{ GOTOSEG_FUNC,        OVERALL,  EQFBGotoActSegment,          "" },
{ GOTOMARK_FUNC,       STARGET,  EQFBFindMark,                "" },
{ CLEARSEGMARK_FUNC,   STARGET,  EQFBClearMark,               "" },
{ POSTEDIT_FUNC,       STARGET,  EQFBSetPostEdit,             "" },
{ AUTOTRANS_FUNC,      STARGET,  EQFBAutoTranslate,           "" },
{ FIND_FUNC,           OVERALL,  EQFBFuncFind,                "" },
{ RFIND_FUNC,          OVERALL,  EQFBFuncNothing,             "" },
{ RING_FUNC,           OVERALL,  EQFBFuncNothing,             "" },
{ OPEN_FUNC,           OVERALL,  EQFBFuncOpen   ,             "" },
{ CUT_FUNC,            OVERALL,  EQFBFuncCutToClip,           "" },
{ COPY_FUNC,           OVERALL,  EQFBFuncCopyToClip,          "" },
{ PASTE_FUNC,          STARGET,  EQFBFuncPasteFromClip,       "" },
{ MARKNEXT_FUNC,       OVERALL,  EQFBFuncMarkNextWord,        "" },
{ MARKPREV_FUNC,       OVERALL,  EQFBFuncMarkPrevWord,        "" },
{ FONTS_FUNC,          OVERALL,  EQFBFuncFonts,               "" },
{ KEYS_FUNC,           OVERALL,  EQFBFuncKeys,                "" },
{ ADDFUZZY_FUNC,       STARGET,  EQFBAddFuzzyMatch,           "" },
{ DISPORG_FUNC,        OVERALL,  EQFBFuncDispOrg,             "" },
{ HIDE_FUNC,           OVERALL,  EQFBFuncChangeToHide,        "" },
{ PROTECT_FUNC,        OVERALL,  EQFBFuncChangeToProt,        "" },
{ UNPROTECT_FUNC,      OVERALL,  EQFBFuncChangeToUnprot,      "" },
{ NEXTLINE_FUNC,       OVERALL,  EQFBFuncNextLine,            "" },
{ CHARACTER_FUNC,      STARGET,  EQFBFuncCharacter,           "" },
{ MARKBLOCK_FUNC,      OVERALL,  EQFBFuncMarkBlock,           "" },
{ MARKCLEAR_FUNC,      OVERALL,  EQFBFuncMarkClear,           "" },
{ MARKDELETE_FUNC,     STARGET,  EQFBFuncMarkDelete,          "" },
{ MARKCOPY_FUNC  ,     ACTSEG,   EQFBFuncMarkCopy,            "" },
{ MARKMOVE_FUNC  ,     ACTSEG,   EQFBFuncMarkMove,            "" },
{ MARKFIND_FUNC,       OVERALL,  EQFBFuncMarkFind,            "" },
{ PRINT_FUNC,          OVERALL,  EQFBDocPrint,                "" },
{ MARKBLOCKSEG_FUNC,   OVERALL,  EQFBFuncMarkSegment,         "" },
{ MARKLEFT_FUNC,       OVERALL,  EQFBFuncMarkLeftCUA,         "" },
{ MARKRIGHT_FUNC,      OVERALL,  EQFBFuncMarkRightCUA,        "" },
{ MARKUP_FUNC,         OVERALL,  EQFBFuncMarkUpCUA,           "" },
{ MARKDOWN_FUNC,       OVERALL,  EQFBFuncMarkDownCUA,         "" },
{ DELLINE_FUNC,        ACTSEG,   EQFBFuncDeleteLine,          "" },
{ ENTRYSEN_FUNC,       STARGET,  EQFBFuncEntry,               "" },
{ UNTRANS_FUNC,        ACTSEG,   EQFBTUnTrans,                "" },
{ LINEWRAP_FUNC,       ACTSEG,   EQFBFuncLineWrap,            "" },
                                                              
{ LRM_FUNC,            ACTSEG,   EQFBFuncLRM    ,             "" },
{ FONTSIZE_FUNC,       OVERALL,  EQFBFuncFontSize,            "" },
//{ MARGINACT_FUNC,      STARGET,  EQFBFuncMarginAct,           "" },
{ MARGINACT_FUNC,      STARGET,  EQFBFuncToggleLineWrap,      "" },
{ SPELLSEG_FUNC,       STARGET,  EQFBFuncSpellSeg,            "" },
{ SPELLFILE_FUNC,      STARGET,  EQFBFuncSpellFile,           "" },
{ WORDMARK_FUNC,       OVERALL,  EQFBFuncWordMark,            "" },
{ WORDDEL_FUNC,        ACTSEG,   EQFBFuncWordDel,             "" },
{ DELTILTAG_FUNC,      ACTSEG,   EQFBFuncDelTilTag,           "" },
{ EMPTY_FUNC,          ACTSEG,   EQFBEmpty,                   "" },
{ COMMAND_FUNC,        OVERALL,  EQFBCommand,                 "" },
{ SCRATCH_FUNC,        STARGET,  EQFBFindScratch,             "" },
{ PROPCPY_FUNC,        STARGET,  EQFBFindCopy,                "" },
{ CPYTYPED_FUNC,       STARGET,  EQFBFindCpyTyped,            "" },
{ EXPAND_FUNC,         ACTSEG,   EQFBExpandSeg,               "" },
{ COMPRESS_FUNC,       ACTSEG,   EQFBCompressSeg,             "" },
{ SETTINGS_FUNC,       OVERALL,  EQFBSettings,                "" },
{ GOTO_FUNC,           OVERALL,  EQFBGotoLine,                "" },
{ QUERYLINE_FUNC,      OVERALL,  EQFBQueryLine,               "" },
{ SHRINK_FUNC,         OVERALL,  EQFBFuncChangeToShrink,      "" },
{ COMPACT_FUNC,        OVERALL,  EQFBFuncChangeToCompact,     "" },
{ SEGINSOSI_FUNC,      ACTSEG,   EQFBSegInsertSOSI,           "" },
{ SEGDELSOSI_FUNC,     ACTSEG,   EQFBSegDeleteSOSI,           "" },
{ DOCINSOSI_FUNC,      STARGET,  EQFBFileInsertSOSI,          "" },
{ DOCDELSOSI_FUNC,     STARGET,  EQFBFileDeleteSOSI,          "" },
{ ACTTRANS_FUNC,       OVERALL,  EQFBActTrans,                "" },
                                                              
{ RLM_FUNC,            ACTSEG,   EQFBFuncRLM,                 "" },
                                                              
{ SRCPROP_FUNC,        OVERALL,  EQFBFuncDispSrcProp,         "" },
{ EDITTERM_FUNC,       OVERALL,  EQFBFuncEditTerm,            "" },
{ HOTPOPUP_FUNC,       OVERALL,  EQFBFuncHotPopUp,            "" },
{ SAVEAS_FUNC,         0,        NULL,                        "" },
{ ADDABBREV_FUNC,      OVERALL,  EQFBAddAbbrev,               "" },
{ REIMPORT_FUNC,       STARGET,  EQFBFuncReImportDoc,         "" },
{ EDITADD_FUNC,        OVERALL,  EQFBEditAddenda,             "" },
{ MARKSTART_FUNC,      OVERALL,  EQFBFuncMarkStartCUA,        "" },
{ MARKEND_FUNC,        OVERALL,  EQFBFuncMarkEndCUA,          "" },
{ INITCAPS_FUNC,       ACTSEG,   EQFBFuncCaps,                "" },
{ SHOWTRANS_FUNC,      STARGET,  EQFBFuncShowTrans,           "" },
{ MARKSEGSTART_FUNC,   OVERALL,  EQFBFuncMarkSegStartCUA,     "" },
{ MARKSEGEND_FUNC,     OVERALL,  EQFBFuncMarkSegEndCUA,       "" },
{ OPENTRNOTE_FUNC,     STARGET,  EQFBFuncOpenTRNote,          "" },
{ TOCGOTO_FUNC,        STARGET,  EQFBFuncTOCGoto,             "" },
{ SHORTEN_FUNC,        OVERALL,  EQFBFuncChangeToShorten,     "" },
{ PROPMARKCOPY_FUNC,   STARGET,  EQFBFuncPropMarkCopy,        "" },
{ NEXTMISSPELLED_FUNC, STARGET,  EQFBFuncNextMisspelled,      "" },
{ SPELLAUTO_FUNC,      STARGET,  EQFBFuncSpellAuto,           "" },
{ GOUPDSEGMENT_FUNC,   STARGET,  EQFBFuncGoUpdSegment,        "" },
{ VISIBLESPACE_FUNC,   OVERALL,  EQFBFuncVisibleSpace,        "" },
{ REDO_FUNC,           ACTSEG,   EQFBFuncRedo,                "" },
{ WYSIWYG_FUNC,        OVERALL,  EQFBFuncChangeToWYSIWYG,     "" },
{ ALLEXACTPROPS_FUNC,  ACTSEG,   EQFBFuncDispAllExactProps,   "" },
{ DUMMY_009,           0,        NULL,                        "" },
{ GOTOSEGMENT_FUNC,    OVERALL,  EQFBGotoSegment,             "" },
{ CFIND_FUNC,          OVERALL,  EQFBFuncCFind,               "" },
{ SEGPROP_FUNC,        OVERALL,  EQFBFuncDispSegProp,         "" },
{ DICTPFXDOWN_FUNC,    STARGET,  EQFBDictPrefixDown,          "" },
{ DICTPFXUP_FUNC,      STARGET,  EQFBDictPrefixUp,            "" },
{ TSEGNEXT_EXACT_FUNC, STARGET,  EQFBTSegNextExact,           "" },
{ TSEGNEXT_FUZZY_FUNC, STARGET,  EQFBTSegNextFuzzy,           "" },
{ TSEGNEXT_NONE_FUNC,  STARGET,  EQFBTSegNextNone,            "" },
{ TSEGNEXT_MT_FUNC,    STARGET,  EQFBTSegNextMT,              "" },
{ TSEGNEXT_GLOBAL_FUNC,STARGET,  EQFBTSegNextGlobal,          "" },
{ LAST_FUNC,           ACTSEG,   EQFBFuncInsertChar,          "" },
// Add end
};
#else
//   extern FUNCTIONTABLE FuncTab[];
#endif
__declspec(dllexport)
FUNCTIONTABLE* get_FuncTab();

//+----------------------------------------------------------------------------+
//|  Table of default key to function assignments                              |
//+----------------------------------------------------------------------------+
#if defined(INIT_TABLES)
KEYPROFTABLE DefKeyTable[LAST_FUNC+1]=
{
//   Code           State              fChange  Def   Function
//---------------------------------------------------------------------------------
   { VK_BACKSPACE,  ST_VK,            FALSE,  (EDIT_NONE),               0, BACKSPACE_FUNC },
   { VK_TAB,        ST_VK,            FALSE,  (EDIT_NONE),               0, TAB_FUNC },
   { VK_ENTER,      ST_VK | ST_CTRL,  FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, TSEG_FUNC },
   { '\0',          0,                FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, INSERTLINE_FUNC },
   { VK_BACKTAB,    ST_VK,            FALSE,  (EDIT_NONE),               0, BACKTAB_FUNC },
   { 's',           ST_ALT,           FALSE,  (EDIT_STANDARD ),          0, SPLITLINE_FUNC },
   { 'j',           ST_ALT,           FALSE,  (EDIT_STANDARD ),          0, JOINLINE_FUNC },
   { '\0',          0,                FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, ESCAPE_FUNC },
   { VK_F2,         ST_VK,            FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, SAVE_FUNC },
   { VK_F3,         ST_VK,            FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, QUIT_FUNC },
   { VK_F4,         ST_VK,            FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, FILE_FUNC },
   { VK_F8,         ST_VK,            FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, ACTPROP_FUNC },
   { VK_BACKSPACE,  ST_VK | ST_ALT,   FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, UNDO_FUNC },
   { '\0',          0,                FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, NEXTDOC_FUNC },
   { VK_HOME,       ST_VK,            FALSE,  (EDIT_NONE),               0, STARTLINE_FUNC },
   { VK_HOME,       ST_VK | ST_ALT,   FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, STARTSEG_FUNC },
   { VK_UP,         ST_VK,            FALSE,  (EDIT_NONE),               0, UP_FUNC },
   { VK_PAGEUP,     ST_VK,            FALSE,  (EDIT_NONE),               0, PAGEUP_FUNC },
   { VK_LEFT,       ST_VK,            FALSE,  (EDIT_NONE),               0, LEFT_FUNC },
   { VK_RIGHT,      ST_VK,            FALSE,  (EDIT_NONE),               0, RIGHT_FUNC },
   { VK_END,        ST_VK,            FALSE,  (EDIT_NONE),               0, ENDLINE_FUNC },
   { VK_END,        ST_VK | ST_ALT,   FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, ENDSEG_FUNC },
   { VK_DOWN,       ST_VK,            FALSE,  (EDIT_NONE),               0, DOWN_FUNC },
   { VK_PAGEDOWN,   ST_VK,            FALSE,  (EDIT_NONE),               0, PAGEDOWN_FUNC },
   { VK_INSERT,     ST_VK,            FALSE,  (EDIT_NONE),               0, INSTOGGLE_FUNC },
   { VK_DELETE,     ST_VK,            FALSE,  (EDIT_NONE),               0, DELETECHAR_FUNC },
   { VK_F5,         ST_VK | ST_SHIFT, FALSE,  (EDIT_STANDARD) , 0, SCROLLLEFT_FUNC },
   { VK_F2,         ST_VK | ST_SHIFT, FALSE,  (EDIT_STANDARD) , 0, SCROLLRIGHT_FUNC },
   { VK_F3,         ST_VK | ST_SHIFT, FALSE,  (EDIT_STANDARD) , 0, SCROLLUP_FUNC },
   { VK_F4,         ST_VK | ST_SHIFT, FALSE,  (EDIT_STANDARD) , 0, SCROLLDOWN_FUNC },
   { '\0',          0,                FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, PREVDOC_FUNC },
   { VK_LEFT,       ST_VK | ST_CTRL,  FALSE,  (EDIT_NONE ),              0, PREVWORD_FUNC },
   { VK_RIGHT,      ST_VK | ST_CTRL,  FALSE,  (EDIT_NONE ),              0, NEXTWORD_FUNC },
   { VK_END,        ST_VK | ST_CTRL,  FALSE,  (EDIT_NONE ),              0, BOTTOMDOC_FUNC },
   { VK_HOME,       ST_VK | ST_CTRL,  FALSE,  (EDIT_NONE ),              0, TOPDOC_FUNC },
   { VK_UP,         ST_VK | ST_CTRL,  FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, PROPUP_FUNC },
   { VK_DOWN,       ST_VK | ST_CTRL,  FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, PROPDOWN_FUNC },
   { VK_UP,         ST_VK | ST_ALT,   FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, DICTUP_FUNC },
   { VK_DOWN,       ST_VK | ST_ALT,   FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, DICTDOWN_FUNC },
   { VK_DELETE,     ST_VK | ST_ALT,   FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, TRUNCSEG_FUNC },
   { VK_DELETE,     ST_VK | ST_CTRL,  FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, TRUNCATE_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, TSEGNEXT_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, DICTLOOK_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, ACTDIC_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, JOINSEG_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, SPLITSEG_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, MARKSEG_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, GOTOSEG_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, GOTOMARK_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, CLEARSEGMARK_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, POSTEDIT_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, AUTOTRANS_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, FIND_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD) , 0, RFIND_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, RING_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, OPEN_FUNC },

   { 'x',           ST_CTRL,  FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, CUT_FUNC },
   { 'c',           ST_CTRL,  FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, COPY_FUNC },
   { 'v',           ST_CTRL,  FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, PASTE_FUNC },

   { VK_RIGHT,      ST_VK | ST_CTRL | ST_SHIFT,  FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKNEXT_FUNC },
   { VK_LEFT,       ST_VK | ST_CTRL | ST_SHIFT,  FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKPREV_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD ), 0, FONTS_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, KEYS_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, ADDFUZZY_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, DISPORG_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, HIDE_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, PROTECT_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, UNPROTECT_FUNC },
   { VK_ENTER,      ST_VK,    FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, NEXTLINE_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKBLOCK_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKCLEAR_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKDELETE_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKCOPY_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKMOVE_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKFIND_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, PRINT_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKBLOCKSEG_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKLEFT_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKRIGHT_FUNC},
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKUP_FUNC  },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKDOWN_FUNC},
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, DELLINE_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, ENTRYSEN_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, UNTRANS_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD),           0, LINEWRAP_FUNC },

   { '\0',          0,        FALSE,   (EDIT_STANDARD),           0, LRM_FUNC },

   { '\0',          0,        FALSE,   (EDIT_STANDARD),           0, FONTSIZE_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD),           0, MARGINACT_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, SPELLSEG_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, SPELLFILE_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, WORDMARK_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, WORDDEL_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, DELTILTAG_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, EMPTY_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, COMMAND_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, SCRATCH_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, PROPCPY_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, CPYTYPED_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, EXPAND_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, COMPRESS_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, SETTINGS_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, GOTO_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, QUERYLINE_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, SHRINK_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, COMPACT_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, SEGINSOSI_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, SEGDELSOSI_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, DOCINSOSI_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, DOCDELSOSI_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, ACTTRANS_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD),           0, RLM_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, SRCPROP_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, EDITTERM_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, HOTPOPUP_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, SAVEAS_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, ADDABBREV_FUNC },

   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, REIMPORT_FUNC },

   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, EDITADD_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKSTART_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKEND_FUNC},
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, INITCAPS_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, SHOWTRANS_FUNC},
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKSEGSTART_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, MARKSEGEND_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, OPENTRNOTE_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, TOCGOTO_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, SHORTEN_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, PROPMARKCOPY_FUNC },

   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, NEXTMISSPELLED_FUNC   },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, SPELLAUTO_FUNC        },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, GOUPDSEGMENT_FUNC  },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, VISIBLESPACE_FUNC  },

   { '\0',          0,        FALSE,   (EDIT_RTF), 0, REDO_FUNC    },
   { '\0',          0,        FALSE,   (EDIT_RTF), 0, WYSIWYG_FUNC },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, ALLEXACTPROPS_FUNC },
   { '\0',          0,        FALSE,   0, 0, DUMMY_009 },
   { '\0',          0,        FALSE,   (EDIT_STANDARD |EDIT_RTF), 0, GOTOSEGMENT_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD | EDIT_RTF), 0, CFIND_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD | EDIT_RTF), 0, SEGPROP_FUNC },
   { VK_PAGEDOWN,   ST_VK | ST_ALT, FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, DICTPFXDOWN_FUNC },
   { VK_PAGEUP,     ST_VK | ST_ALT, FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, DICTPFXUP_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, TSEGNEXT_EXACT_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, TSEGNEXT_FUZZY_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, TSEGNEXT_NONE_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, TSEGNEXT_MT_FUNC },
   { '\0',          0,        FALSE,  (EDIT_STANDARD |EDIT_RTF), 0, TSEGNEXT_GLOBAL_FUNC },

   { '\0',          0,        FALSE,   0, 0, LAST_FUNC }
};
#else
//   extern KEYPROFTABLE DefKeyTable[LAST_FUNC+1];
#endif

__declspec(dllexport)
KEYPROFTABLE* get_DefKeyTable();

#if defined(INIT_TABLES)
  KEYPROFTABLE KeyTable[LAST_FUNC+1] =
  { { 0,  0,            FALSE,  0,     0, LAST_FUNC } };
#else
//  extern KEYPROFTABLE KeyTable[LAST_FUNC+1];
#endif

__declspec(dllexport)
KEYPROFTABLE* get_KeyTable();

//+----------------------------------------------------------------------------+
//|  Table of reserved keys                                                    |
//+----------------------------------------------------------------------------+
#if defined(INIT_TABLES)

RESKEYTABLE ResKeyTab[]=
{
//   Code           State              Function          AssignStatus
//---------------------------------------------------------------------------------
   { '0',           ST_CTRL,           GETPROPMATCH_FUNC,  NOT_ASSIGNABLE  },
   { '1',           ST_CTRL,           GETPROPMATCH_FUNC,  NOT_ASSIGNABLE  },
   { '2',           ST_CTRL,           GETPROPMATCH_FUNC,  NOT_ASSIGNABLE  },
   { '3',           ST_CTRL,           GETPROPMATCH_FUNC,  NOT_ASSIGNABLE  },
   { '4',           ST_CTRL,           GETPROPMATCH_FUNC,  NOT_ASSIGNABLE  },
   { '5',           ST_CTRL,           GETPROPMATCH_FUNC,  NOT_ASSIGNABLE  },
   { '6',           ST_CTRL,           GETPROPMATCH_FUNC,  NOT_ASSIGNABLE  },
   { '7',           ST_CTRL,           GETPROPMATCH_FUNC,  NOT_ASSIGNABLE  },
   { '8',           ST_CTRL,           GETPROPMATCH_FUNC,  NOT_ASSIGNABLE  },
   { '9',           ST_CTRL,           GETPROPMATCH_FUNC,  NOT_ASSIGNABLE  },
   { 'a',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'b',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'c',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'd',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'e',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'f',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'g',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'h',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'i',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'j',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'k',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'l',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'm',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'n',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'o',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'p',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'q',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'r',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 's',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 't',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'u',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'v',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'w',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'x',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'y',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'z',           ST_CTRL,           GETDICTMATCH_FUNC,  ASSIGNABLE  },
   { 'A',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'B',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'C',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'D',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'E',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'F',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'G',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'H',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'I',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'J',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'K',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'L',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'M',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'N',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'O',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'P',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'Q',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'R',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'S',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'T',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'U',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'V',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'W',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'X',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'Y',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { 'Z',           ST_CTRL| ST_SHIFT,    GETDICTMATCH_FUNC,  ASSIGNABLE },
   { '-',           ST_CTRL,           -1,  NOT_ASSIGNABLE },
   { VK_F1,         ST_VK | ST_CTRL,   -1,  NOT_ASSIGNABLE },
   { VK_F2,         ST_VK | ST_CTRL,   -1,  NOT_ASSIGNABLE },
   { VK_F3,         ST_VK | ST_CTRL,   -1,  NOT_ASSIGNABLE },
   { VK_F4,         ST_VK | ST_CTRL,   -1,  NOT_ASSIGNABLE },
   { VK_F5,         ST_VK | ST_CTRL,   -1,  NOT_ASSIGNABLE },
   { VK_F6,         ST_VK | ST_CTRL,   -1,  NOT_ASSIGNABLE },
   { VK_F7,         ST_VK | ST_CTRL,   -1,  NOT_ASSIGNABLE },
   { VK_F8,         ST_VK | ST_CTRL,   -1,  NOT_ASSIGNABLE },
   { VK_F9,         ST_VK | ST_CTRL,   -1,  NOT_ASSIGNABLE },
   { VK_F10,        ST_VK | ST_CTRL,   -1,  NOT_ASSIGNABLE },
   { VK_F11,        ST_VK | ST_CTRL,   -1,  NOT_ASSIGNABLE },
   { VK_F12,        ST_VK | ST_CTRL,   -1,  NOT_ASSIGNABLE },
   { VK_F1,         ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_F2,         ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_F3,         ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_F4,         ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_F5,         ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_F6,         ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_F7,         ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_F8,         ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_F9,         ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_F10,        ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_F11,        ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_F12,        ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_SPACE,      ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_ESC,        ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_F10,        ST_VK,             -1,  NOT_ASSIGNABLE },
   { VK_LEFT,       ST_VK | ST_SHIFT,  MARKLEFT_FUNC,  NOT_ASSIGNABLE },
   { VK_RIGHT,      ST_VK | ST_SHIFT,  MARKRIGHT_FUNC,  NOT_ASSIGNABLE} ,
   { VK_UP,         ST_VK | ST_SHIFT,  MARKUP_FUNC  ,  NOT_ASSIGNABLE },
   { VK_DOWN,       ST_VK | ST_SHIFT,  MARKDOWN_FUNC,  NOT_ASSIGNABLE },
   { VK_HOME,       ST_VK | ST_SHIFT,  MARKSTART_FUNC,  NOT_ASSIGNABLE },
   { VK_END,        ST_VK | ST_SHIFT,  MARKEND_FUNC,  NOT_ASSIGNABLE} ,
   { VK_HOME,       ST_VK | ST_SHIFT | ST_ALT,  MARKSEGSTART_FUNC,  NOT_ASSIGNABLE},
   { VK_END,        ST_VK | ST_SHIFT | ST_ALT,  MARKSEGEND_FUNC,  NOT_ASSIGNABLE} ,
   { VK_DELETE,     ST_VK | ST_SHIFT,  CUT_FUNC,  NOT_ASSIGNABLE },
   { VK_INSERT,     ST_VK | ST_CTRL,   COPY_FUNC,  NOT_ASSIGNABLE },
   { VK_INSERT,     ST_VK | ST_SHIFT,  PASTE_FUNC,  NOT_ASSIGNABLE },
   { VK_F1,         ST_VK ,            -1,  NOT_ASSIGNABLE },
   { VK_ESC,        ST_VK | ST_SHIFT,  -1,  NOT_ASSIGNABLE },
   { VK_F1,         ST_VK | ST_SHIFT,  -1,  NOT_ASSIGNABLE },
   { VK_ENTER,      ST_VK | ST_SHIFT,  -1,  NOT_ASSIGNABLE },
   { VK_F9,         ST_VK | ST_SHIFT,  -1,  NOT_ASSIGNABLE },
   { VK_F10,        ST_VK | ST_SHIFT,  HOTPOPUP_FUNC,  NOT_ASSIGNABLE },
   { VK_ENTER,      ST_VK | ST_ALT,    -1,  NOT_ASSIGNABLE },
   { VK_SCRLLOCK,   ST_VK | ST_SHIFT,  -1,  NOT_ASSIGNABLE },
/**********************************************************************/
/* the following reserved keys added for consistencxy reasons with all*/
/* other RicheEdit editors; reserved for STANDARD editor too          */
/**********************************************************************/

   { VK_BACKSPACE,  ST_VK,           BACKSPACE_FUNC,  NOT_ASSIGNABLE  },
   { VK_TAB,        ST_VK,           TAB_FUNC,        NOT_ASSIGNABLE  },
   { VK_BACKTAB,    ST_VK,           BACKTAB_FUNC,    NOT_ASSIGNABLE  },
   { VK_HOME,       ST_VK,           STARTLINE_FUNC,  NOT_ASSIGNABLE  },
   { VK_UP,         ST_VK,           UP_FUNC,         NOT_ASSIGNABLE  },
   { VK_PAGEUP,     ST_VK,           PAGEUP_FUNC,     NOT_ASSIGNABLE  },
   { VK_LEFT,       ST_VK,           LEFT_FUNC,       NOT_ASSIGNABLE  },
   { VK_RIGHT,      ST_VK,           RIGHT_FUNC ,     NOT_ASSIGNABLE  },
   { VK_END,        ST_VK,           ENDLINE_FUNC,    NOT_ASSIGNABLE  },
   { VK_DOWN,       ST_VK,           DOWN_FUNC,       NOT_ASSIGNABLE  },
   { VK_PAGEDOWN,   ST_VK,           PAGEDOWN_FUNC,   NOT_ASSIGNABLE  },
   { VK_INSERT,     ST_VK,           INSTOGGLE_FUNC,  NOT_ASSIGNABLE  },
   { VK_DELETE,     ST_VK,           DELETECHAR_FUNC, NOT_ASSIGNABLE  },
   { VK_LEFT,       ST_VK | ST_CTRL, PREVWORD_FUNC,   NOT_ASSIGNABLE  },
   { VK_RIGHT,      ST_VK | ST_CTRL, NEXTWORD_FUNC,   NOT_ASSIGNABLE  },
   { VK_END,        ST_VK | ST_CTRL, BOTTOMDOC_FUNC,  NOT_ASSIGNABLE  },
   { VK_HOME,       ST_VK | ST_CTRL, TOPDOC_FUNC,     NOT_ASSIGNABLE  },

  /********************************************************************/
  /* DBCS reserved VK keys                                            */
  /********************************************************************/
   { VK_DBE_ALPHANUMERIC, ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_DBE_SBCSCHAR,     ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_DBE_DBCSCHAR,     ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_DBE_SBCSDBCSCHAR, ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_DBE_KATAKANA,     ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_DBE_HIRAGANA,     ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_DBE_ROMAN,        ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_DBE_HANJA,        ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_DBE_HANZI,        ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_DBE_HANGEUL,      ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_DBE_JAMO,         ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_DBE_TSANGJYE,     ST_VK ,      -1,  NOT_ASSIGNABLE },//
   { VK_DBE_PHONETIC,     ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_MY_NOCONV,        ST_VK ,      -1,  NOT_ASSIGNABLE },
   { VK_MY_KANAKANJI,     ST_VK ,      -1,  NOT_ASSIGNABLE },
   { '+',                 ST_ALT,      HOTPOPUP_FUNC,  NOT_ASSIGNABLE },    // key for popup
   { '\0',          0,           0,  NOT_ASSIGNABLE }
};

#else
//   extern RESKEYTABLE ResKeyTab[];
#endif

__declspec(dllexport)
RESKEYTABLE* get_ResKeyTab();

/*-------------------------------------------------------------------*/
/* Other Configuration Details                                       */
/*-------------------------------------------------------------------*/

/* The Tab spacing. */
#if defined(INIT_TABLES)
   int configTabStops = 8;
#else
   extern int configTabStops;
#endif

 /*********************************************************************/
 /* macro for VioGetDeviceCellSize                                    */
 /*********************************************************************/
 #define GETDEVICECELLSIZE( cy, cx , hwnd )       \
    {                                             \
      TEXTMETRIC  txmtr;                          \
      HDC         hdc;                            \
                                                  \
      hdc = GetDC(hwnd);                          \
      SelectObject( hdc, GetStockObject( OEM_FIXED_FONT )); \
      GetTextMetrics( hdc, &txmtr );              \
      ReleaseDC (hwnd, hdc);                      \
      cy = (SHORT)txmtr.tmHeight;                 \
      cx = (SHORT)txmtr.tmAveCharWidth;           \
    }


#if defined(TPLOGGING)
 #define TPLOGFILE "\\TP.LOG"
 FILE *hTPLog = NULL;
 #define TPLOG()                \
    if ( hTPLog )               \
       fprintf( hTPLog, "TPRO: %s, %d \n",__FILE__, __LINE__ );    \
    else                         \
       DosBeep( 1200, 200 );
#else
 #define TPLOG()
#endif

VOID PASCAL FAR TPCleanUp( USHORT );

BOOL EQFBKeyState ( USHORT, PUCHAR, PUCHAR );

#if defined(INIT_TABLES)
  CHAR aszFontFacesGlobal[MAX_DIF_DOC][LF_FACESIZE];       // facenames for windows
#else
//  extern CHAR aszFontFacesGlobal[MAX_DIF_DOC][LF_FACESIZE];       // facenames for windows
#endif
__declspec(dllexport)
CHAR* get_aszFontFacesGlobal();
/**********************************************************************/
/* define stubs for BIDI if not available...                          */
/**********************************************************************/
//Flags for NlsLanguage
  #define   NlsLanguage          0
  #define   NLS_PROCESS_LATIN    0x00000000
  #define   NLS_PROCESS_BIDI     0x00000001
  #define   NLS_PROCESS_HEBREW   0x00000002
  #define   NLS_PROCESS_ARABIC   0x00000004

/**********************************************************************/
/* statusbar defines                                                  */
/**********************************************************************/
  #define TP_STATUSBAR "TPStatusBar"
  MRESULT APIENTRY TPSTATUSBARWNDPROC ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );
  VOID TPFillStatusBar( HWND );

  /**********************************************************************/
  /* macro to update statusbar                                          */
  /**********************************************************************/
  #define STATUSBAR( p )                      \
  {                                           \
    HWND hwnd1 = (p)->hStatusBarWnd;          \
    if ( hwnd1 && WinIsWindowVisible( hwnd1 ))\
    {                                         \
      INVALIDATERECT( hwnd1, NULL, FALSE ) ;  \
      UpdateWindow( hwnd1 );                  \
      ValidateRect( hwnd1, NULL );            \
    }                                         \
  }

/**********************************************************************/
/* defines and functions for a ruler window within editor ...         */
/**********************************************************************/
  #define TP_RULER  "TPRuler"
  MRESULT APIENTRY TPRULERWNDPROC ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2 );
  VOID TPFillRuler( HWND );

  /**********************************************************************/
  /* macro to update statusbar                                          */
  /**********************************************************************/
  #define RULER( p )                          \
  {                                           \
    HWND hwnd1 = (p)->hRulerWnd;              \
    if ( hwnd1 && WinIsWindowVisible( hwnd1 ))\
    {                                         \
      INVALIDATERECT( hwnd1, NULL, FALSE ) ;  \
     UpdateWindow( hwnd1 );                  \
      ValidateRect( hwnd1, NULL );            \
    }                                         \
  }

//  #define POSTEDITSEGBOUND_SEGDATA "ฏ"
  #define POSTEDITSEGBOUND_SEGDATA 'ฏ'
  #define POSTEDITSEGBOUND_SEGDATA_GREEK1 '\x0f'
  #define POSTEDITSEGBOUND_SEGDATA_GREEK2 '\x15'
  #define POSTEDITSEGBOUND_SEGDATA_DBCS   '\xA4'
  #define VISIBLE_BLANK    '\xF9'
  #define VISIBLE_LINEFEED ''
  #define VISIBLE_LINEFEED_HEBREW '\xE3' // 227
  #define VISIBLE_BLANK_HEBREW    '\xA4'

VOID EQFBFindGotoSeg( PTBDOCUMENT, ULONG, USHORT, USHORT); // Find: pos. at file
BOOL  EQFBDoCopyData     ( PTBDOCUMENT,PSZ_W );
/**********************************************************************/
/* return TRUE if any kind of nop...                                  */
/**********************************************************************/
#define ISQFNOP( a )  \
 ((a==QF_NOP)||(a==QF_NOP_ANCHOR_1)||(a==QF_NOP_ANCHOR_2)||(a==QF_NOP_ANCHOR_3))

/**********************************************************************/
/* return TRUE if any kind of translatable                            */
/**********************************************************************/
#define ISXLATABLE( a ) \
 ((a==QF_XLATED)||(a==QF_CURRENT)||(a==QF_TOBE)||(a==QF_ATTR))

#define SHRINKLEN_NOTYETCALC    100

#define ISLF(c, c2, c0)                               \
  (  (c == LF) ||                                     \
   ( (c == SOFTLF_CHAR) && (c2 != SOFTLF_CHAR) && (c0 != SOFTLF_CHAR)) )

BOOL EQFBCallLCS(PFUZZYTOK, PFUZZYTOK, USHORT, USHORT, BOOL);


USHORT EQFBMisspelledHLType ( PTBDOCUMENT, PTBSEGMENT, PSZ_W );


/**********************************************************************/
/* worker thread defines                                              */
/**********************************************************************/

#define  THREAD_SPELLFILE      1
#define  THREAD_SPELLSEGMENT   2
#define  THREAD_AUTOSAVE       4
#define  THREAD_SPELLSCRN      8
#define  THREAD_SPELLACTSEG    16
#define  THREAD_PROOFADD       32
#define  THREAD_TEMPADD        64
#define  THREAD_AUTOSUBST      128
#define  WORKTHREAD_STACKSIZE  4000

VOID EQFBWorkThread ( PVOID pvDoc );
VOID EQFBWorkThreadTask( PTBDOCUMENT pDoc, USHORT usTask );

VOID EQFBTempAdd ( PTBDOCUMENT );                          // add to temp addenda

BOOL EQFBSpellIgnoreCheck (PSPELLDATA pSpellData );

// helper functions for file write processes
USHORT EQFBPrepareFileWrite( PTBDOCUMENT pDoc, PVOID  *ppvFileWriteData, PSZ pszFileName, SHORT sLogTaskID, USHORT  usCPConversion, EQF_BOOL fAutoSave );
USHORT EQFBWriteNextSegment( PTBDOCUMENT pDoc, PVOID pvFileWriteData, USHORT usConversion, ULONG ulCP, PBOOL pfDone );
USHORT EQFBTerminateFileWrite( PTBDOCUMENT pDoc, PVOID pvFileWriteData, USHORT usRCIn );


VOID  EQFBBuildPrefixes(PCHAR_W);
USHORT EQFBGetHLType( PTBSEGMENT pSeg, SHORT sSegOffs);

/**********************************************************************/
/* extended fonts dialog and text types                               */
/* new format: # of items in T-group, IDS_... of those items,         */
/*               # of items in R-group, IDS_... of those items,       */
/*               # of items in D-group, IDS_... of those items,       */
/*               # of items in M-group, IDS_... of those items        */
/* if you want to reordering in a group: just reorder the IDS_..strings here */
/*        ... to add an item: add it at the correct position and      */
/*                            increase counter                        */
/*   ALSO ADAPT EXTTEXTTYPE!!                                         */
/**********************************************************************/

#if defined(INIT_TABLES)
    USHORT usFontTypesInGroup[] = {6,IDS_TB_FONTEXT_T_SOURCE,
                                     IDS_TB_FONTEXT_T_TRANSLATION,
                                     IDS_TB_FONTEXT_T_LAYOUT,
                                     IDS_TB_FONTEXT_T_ACTIVE,
                                     IDS_TB_FONTEXT_T_TRNOTE,
                                     IDS_TB_FONTEXT_T_RTFEDIT_TRNOTE,
                                   4, IDS_TB_FONTEXT_R_SCRATCH,
                                      IDS_TB_FONTEXT_R_MODIFIED,
                                      IDS_TB_FONTEXT_R_COPIED,
                                      IDS_TB_FONTEXT_R_LAYOUTCHANGES,
                                   4, IDS_TB_FONTEXT_D_HEADWORD,
                                      IDS_TB_FONTEXT_D_TRANSLATION,
                                      IDS_TB_FONTEXT_D_PREFIX,
                                      IDS_TB_FONTEXT_D_ADDITIONINFO,
                                   4, IDS_TB_FONTEXT_M_PREFIX,
                                      IDS_TB_FONTEXT_M_PMODIFIED,
                                      IDS_TB_FONTEXT_M_PINSERTED,
                                      IDS_TB_FONTEXT_M_PDELETED };

  #else

    extern USHORT usFontTypesInGroup[];

#endif

// font specification for one entry in the tree
// is one CHARFORMAT2 structure; ( contains all info needed for character
// formatting in rich edit control 2.0

  #if defined(INIT_TABLES)
     CHARFORMAT2 aszFontExtSpecs[MAXEXTTYPE];
  #else
//    extern CHARFORMAT2 aszFontExtSpecs[MAXEXTTYPE];
  #endif
	__declspec(dllexport)
	CHARFORMAT2* get_aszFontExtSpecs();

/**********************************************************************/
/* 1st default for extended font specs                                */
/**********************************************************************/
#define  DEF1_FNAME   "System"
#define  DEF1_MASK CFM_BACKCOLOR|CFM_COLOR|CFM_FACE|CFM_SIZE \
                       |CFM_UNDERLINETYPE | CFM_BOLD|CFM_COLOR \
                       |CFM_STRIKEOUT|CFM_UNDERLINE
#define  DEF1_EFFECTS CFE_BOLD |CFE_COLOR | CFE_STRIKEOUT| CFE_UNDERLINE

typedef struct _EXTCOLOR
{
   USHORT   usTextColor;
   USHORT   usBackColor;
} EXTCOLOR, *PEXTCOLOR;

typedef struct _EXTCOLOREX
{
   USHORT   usTextColor;
   USHORT   usBackColor;
   BYTE     bStyle;
   BYTE     bUnderline;
   BYTE     bStrikeOut;
} EXTCOLOREX, *PEXTCOLOREX;

  #if defined(INIT_TABLES)
  EXTCOLOR aszDef1FontExtSpecs[MAXEXTTYPE] =
  {
  // FG, BG,
  //---------------------------------------------------------------------------
   { COL_BLUE,  COL_WHITE },             // EXT_T_SOURCE
   { COL_BLACK, COL_WHITE },             // EXT_T_TRANSLATION
   { COL_RED,   COL_WHITE },             // EXT_T_LAYOUT
   { COL_BLACK, COL_YELLOW },            // EXT_T_ACTIVE
   { COL_BLACK, COL_WHITE },             // EXT_T_TRNOTE
   { COL_BLACK, COL_WHITE },             // EXT_R_SCRATCH
   { COL_BLACK, COL_WHITE },             // EXT_R_MODIFIED
   { COL_BLACK, COL_WHITE },             // EXT_R_COPIED
   { COL_BLACK, COL_LIGHTGRAY },         // EXT_R_LAYOUTCHANGES
   { COL_RED,   COL_WHITE },             // EXT_D_HEADWORD
   { COL_BLUE,  COL_WHITE },             // EXT_D_TRANSLATION
   { COL_RED,   COL_WHITE },             // EXT_D_PREFIX
   { COL_LIGHTGRAY, COL_WHITE },         // EXT_D_ADDITIONINFO
   { COL_RED,   COL_WHITE },             // EXT_M_PREFIX
   { COL_BLACK, COL_LIGHTCYAN },         // EXT_M_PMODIFIED
   { COL_BLACK, COL_LIGHTGRAY },         // EXT_M_PINSERTED
   { COL_BLACK, COL_LIGHTPINK },         // EXT_M_PDELETED
   { COL_BLACK, COL_LIGHTPINK },         // EXT_T_RTFEDIT_TRNOTE

// { COL_BLACK, COL_CYAN  },             // EXT_I_ANCHOR1
// { COL_BLUE,  COL_CYAN  },             // EXT_I_ANCHOR2
// { COL_LIGHTGRAY, COL_CYAN },          // EXT_I_ANCHOR3
// { COL_BLACK, COL_LIGHTRED },          // EXT_I_UNALIGNED
// { COL_WHITE, COL_LIGHTGRAY },         // EXT_I_CROSSEDOUT
// { COL_BLACK, COL_LIGHTCYAN },         // EXT_I_OVERCROSS
// { COL_BLACK, COL_WHITE },             // EXT_I_ALIGN1
// { COL_BLUE,  COL_WHITE },             // EXT_I_ALIGN2
// { COL_GREEN, COL_WHITE }              // EXT_I_ALIGN3
//   { COL_BLACK, COL_WHITE }              // MAXEXTTYPE
   };

    EXTCOLOR aszTextTypeColorTable[MAXCOLOUR];
  #else
    extern EXTCOLOREX aszDef1FontExtSpecs[MAXEXTTYPE];
    extern EXTCOLOR aszTextTypeColorTable[MAXCOLOUR];
  #endif
__declspec(dllexport)
  PSZ_W EQFBSegColRTF ( CHARFORMAT2 *, PTBSEGMENT, USHORT,
                       USHORT, DISPSTYLE, BOOL, PDISPFILERTF );
  VOID EQFBSegColRTFInit( CHARFORMAT2 *, USHORT);
  MRESULT APIENTRY EQFBWNDPROCRTFCTRL( HWND, WINMSG, WPARAM, LPARAM );

  MRESULT APIENTRY  EQFBWNDPROC ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2);
  MRESULT APIENTRY  EQFBWNDPROCRTF ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2);

  void EQFBWndProc_Command (HWND, WPARAM, LPARAM) ;

VOID EQFBGotoSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum, USHORT usSegOffset );
VOID EQFBGotoMarkSegRTF( PTBDOCUMENT pDoc,
                         ULONG ulStartSegNum, USHORT usStartSegOffset,
                         ULONG ulEndSegNum, USHORT usEndSegOffset );
VOID EQFBFindGotoSegRTF ( PTBDOCUMENT  pDoc,
                          ULONG ulSegNum, USHORT usSegOffset, USHORT usLen ) ;
VOID EQFBGetWorkSegRTF( PTBDOCUMENT pDoc, ULONG  ulSegNum );
VOID EQFBUpdateSegStyleRTF( PTBDOCUMENT pDoc, USHORT usSegNum, PTBSEGMENT pSeg );
VOID EQFBGetSegFromCaretRTF( PTBDOCUMENT pDoc, PTBROWOFFSET pTBCursor, ULONG ulCaret );

void EQFBWndProc_CommandRTF( HWND, WPARAM, LPARAM );
__declspec(dllexport)
VOID EQFBTransRTF ( PTBDOCUMENT pDoc, USHORT usCond );
VOID EQFBDoNextTwoRTF( PTBDOCUMENT pDoc, PULONG  pulSegNum, USHORT usCond );
__declspec(dllexport)
VOID EQFBSetWorkSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum, PSZ_W pData );
VOID EQFBUpdateTBCursor( PTBDOCUMENT pDoc );
__declspec(dllexport)
BOOL EQFBFuncRTFStart ( PSZ, PSZ, PSTEQFGEN, BOOL, PTBDOCUMENT );
__declspec(dllexport)
VOID EQFBActivateSegmRTF ( PTBDOCUMENT pDoc, ULONG  ulSegNum );
__declspec(dllexport)
VOID EQFBTransRTF ( PTBDOCUMENT pDoc, USHORT usCond );
__declspec(dllexport)
USHORT EQFBNextUnprotected ( PTBDOCUMENT pDoc, PTBSEGMENT pSeg, PUSHORT pusOffs );

__declspec(dllexport)
VOID EQFBSetJoinedSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum );
__declspec(dllexport)
VOID EQFBSetJoinStartSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum );
__declspec(dllexport)
VOID EQFBSetSplitStartSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum );
__declspec(dllexport)
VOID EQFBSetSplittedSegRTF( PTBDOCUMENT pDoc, ULONG ulSegNum );
__declspec(dllexport)
VOID EQFBSetPostEditRTF( PTBDOCUMENT pDoc );
__declspec(dllexport)
VOID EQFBDisplayFileNewRTF( PTBDOCUMENT pDoc );
__declspec(dllexport)
VOID EQFBUndoRTF( PTBDOCUMENT pDoc );
__declspec(dllexport)
VOID EQFBRedoRTF( PTBDOCUMENT pDoc );
__declspec(dllexport)
VOID EQFBFuncEndLineRTF( PTBDOCUMENT pDoc );
__declspec(dllexport)
VOID EQFBFuncStartLineRTF( PTBDOCUMENT pDoc );
__declspec(dllexport)
VOID EQFBGetSelBlockRTF( PTBDOCUMENT pDoc );
VOID EQFBSetWYSIWYGType( PTBDOCUMENT pDoc, PSZ pTagTable );
__declspec(dllexport)
MRESULT EQFBFuncRTFFunc( PTBDOCUMENT pDoc, USHORT usFunc,
                         HWND hwnd, WPARAM mp1, LPARAM mp2 );

BOOL   EQFBAllSpellIgnoreCheck(PSPELLDATA);

  #ifdef RTF_DEBUG
    VOID RTFDEBUG( PTBDOCUMENT pDoc, PSZ_W p );
    VOID RTFDEBUGSTRING( PTBDOCUMENT pDoc, PSZ_W p );
  #else
    #define RTFDEBUG( pDoc, p )
    #define RTFDEBUGSTRING( pDoc, p )
  #endif
  #ifdef RTF_RTFDUMP
    VOID RTFDUMP( PTBDOCUMENT pDoc, PSZ_W p );
  #else
    #define RTFDUMP( pDoc, p )
  #endif

  #define LOCKRICHEDIT( pDoc, fLock )                                \
  {                                                                  \
    if (((PDISPFILERTF)pDoc->pDispFileRTF)->fRichEditLock ||         \
    (((PDISPFILERTF)pDoc->pDispFileRTF)->bRTFFill & (RTF_INITFILL | RTF_FASTFILL)) ) \
    {                                                                \
      fLock = FALSE;                                                 \
    }                                                                \
    else                                                             \
    {                                                                \
      fLock = TRUE;                                                  \
      ((PDISPFILERTF)pDoc->pDispFileRTF)->fRichEditLock = TRUE;      \
      SendMessage( pDoc->hwndRichEdit, WM_SETREDRAW, FALSE, 0 );     \
    }                                                                \
  }

  #define UNLOCKRICHEDIT( pDoc, fLock, fInvalidate )                 \
  {                                                                  \
    if ( fLock )                                                     \
    {                                                                \
      LockWindowUpdate( NULL );                                      \
      fLock = FALSE;                                                 \
      ((PDISPFILERTF)pDoc->pDispFileRTF)->fRichEditLock = FALSE;     \
      SendMessage( pDoc->hwndRichEdit, WM_SETREDRAW, TRUE, 0 );      \
      if ( fInvalidate )                                             \
      {                                                              \
        InvalidateRect( pDoc->hwndRichEdit, 0, TRUE );               \
      }                                                              \
    }                                                                \
  }

  #define UNLOCKRICHEDIT_TRUE( pDoc, fLock )                           \
    {                                                                  \
      if ( fLock )                                                     \
      {                                                                \
        LockWindowUpdate( NULL );                                      \
        fLock = FALSE;                                                 \
        ((PDISPFILERTF)pDoc->pDispFileRTF)->fRichEditLock = FALSE;     \
        SendMessage( pDoc->hwndRichEdit, WM_SETREDRAW, TRUE, 0 );      \
        InvalidateRect( pDoc->hwndRichEdit, 0, TRUE );                 \
      }                                                                \
  }

    #define UNLOCKRICHEDIT_FALSE( pDoc, fLock )                 \
    {                                                                  \
      if ( fLock )                                                     \
      {                                                                \
        LockWindowUpdate( NULL );                                      \
        fLock = FALSE;                                                 \
        ((PDISPFILERTF)pDoc->pDispFileRTF)->fRichEditLock = FALSE;     \
        SendMessage( pDoc->hwndRichEdit, WM_SETREDRAW, TRUE, 0 );      \
      }                                                                \
  }


//+----------------------------------------------------------------------------+
//|  list of symbolic function names                                           |
//|                                                                            |
//|   Note: the order of the listed functions must be the same as the          |
//|         order in the function table as the enum value                      |
//|         is used as index into the function table.                          |
//+----------------------------------------------------------------------------+
typedef enum _ITMFUNCTION
{
   BACKSPACE_ITMFUNC,
   BACKTAB_ITMFUNC,
   BOTTOMDOC_ITMFUNC,
   DELETECHAR_ITMFUNC,
   DOWN_ITMFUNC,
   ENDLINE_ITMFUNC,
   ENDSEG_ITMFUNC,
   FILE_ITMFUNC,
   INSTOGGLE_ITMFUNC,
   JOINLINE_ITMFUNC,
   LEFT_ITMFUNC,
   NEXTWORD_ITMFUNC,
   NOTHING_ITMFUNC,
   PAGEDOWN_ITMFUNC,
   PAGEUP_ITMFUNC,
   PREVWORD_ITMFUNC,
   QUIT_ITMFUNC,
   RIGHT_ITMFUNC,
   SAVE_ITMFUNC,
   SCROLLDOWN_ITMFUNC,
   SCROLLLEFT_ITMFUNC,
   SCROLLRIGHT_ITMFUNC,
   SCROLLUP_ITMFUNC,
   SPLITLINE_ITMFUNC,
   STARTLINE_ITMFUNC,
   STARTSEG_ITMFUNC,
   TAB_ITMFUNC,
   TOPDOC_ITMFUNC,
   UNDO_ITMFUNC,
   UP_ITMFUNC,
   JOINSEG_ITMFUNC,
   SPLITSEG_ITMFUNC,
   MARKSEG_ITMFUNC,
   GOTOMARK_ITMFUNC,
   CLEARSEGMARK_ITMFUNC,
   FIND_ITMFUNC,
   CUT_ITMFUNC,
   COPY_ITMFUNC,
   PASTE_ITMFUNC,
   MARKNEXT_ITMFUNC,                   // mark next word too
   MARKPREV_ITMFUNC,                   // mark previous word too
   FONTS_ITMFUNC,
   HIDE_ITMFUNC,
   PROTECT_ITMFUNC,
   UNPROTECT_ITMFUNC,
   SHORTEN_ITMFUNC,
   NEXTLINE_ITMFUNC,
   CHARACTER_ITMFUNC,
   MARKCLEAR_ITMFUNC,
   MARKBLOCKSEG_ITMFUNC,
   MARKLEFT_ITMFUNC,
   MARKRIGHT_ITMFUNC,
   MARKUP_ITMFUNC,
   MARKDOWN_ITMFUNC,
   LINEWRAP_ITMFUNC,                      // line wrap
   FONTSIZE_ITMFUNC,                   // select font size for different windows
   MARGINACT_ITMFUNC,                  // right margin activate / deactivae funct.
   SPELLSEG_ITMFUNC,                   // spellcheck of segment
   SPELLFILE_ITMFUNC,                  // spellcheck of file
   GOTO_ITMFUNC,                       // goto line number..
   QUERYLINE_ITMFUNC,                  // query line number
   SHRINK_ITMFUNC,                     // shrink style
   COMPACT_ITMFUNC,                    // compact style
   INITCAPS_ITMFUNC,                   // toggle first character to caps/non caps
   VISIBLESPACE_ITMFUNC,               // show spaces
   NEXTANCHOR_ITMFUNC,                 // jump to next anchor
   PREVANCHOR_ITMFUNC,                 // jump to previous anchor
   NEXTIRREGULAR_ITMFUNC,              // jump to next irregular
   PREVIRREGULAR_ITMFUNC,              // jump to previous irregular
   SYNCH_ITMFUNC,                      // synchronize
   NEXTSYNCH_ITMFUNC,                  // synchronize next
   PREVSYNCH_ITMFUNC,                  // synchronize prev
   TOGGLE_ITMFUNC,                     // toggle ITM windows
   SETANCHOR_ITMFUNC,                  // set anchor
   DELANCHOR_ITMFUNC,                  // delete anchor
   CROSSOUT_ITMFUNC,                   // cross out
   UNDOCROSSOUT_ITMFUNC,               // undo crossout
   ADDABBREV_ITMFUNC,                  // add abbreviation to dictionary
   NEXTUNALIGNED_ITMFUNC,              // next unaligned
   PREVUNALIGNED_ITMFUNC,              // previous unaligned
   HOTPOPUP_ITMFUNC,
   ITMDUMMY_003,
   ITMDUMMY_004,
   ITMDUMMY_005,
   ITMDUMMY_006,
   ITMDUMMY_007,
   LAST_ITMFUNC                        // should always be last entry in enum
} ITMFUNCTION;

#define ERR_TAGSAREWRONG   -15

void EQFBGetMatchFlags( PTBDOCUMENT pDoc );

// max size of segment attrribute strings
#define ATTRIBUTE_SIZE  40                // MARK, CURRENT, JOIN=attribute
