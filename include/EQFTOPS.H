//+----------------------------------------------------------------------------+
//|EQFTOPS.H     Internal header file for various define statements            |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                          |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

/**********************************************************************/
/* General Part -- all new items are activated...                     */
/**********************************************************************/
  #define _TP48
  #define TOP97                        // long file name support
  #define TOP97_A44                    // additional API interfaces
  #define TOP97_A45                    // No change in timestamp if QUIT
  #define TOP97_A46                    // Information about exact/fuzzy
  #define TOP97_A47                    // display scale line
  #define TOP97_A48                    // TRNOTES Enabling/Implementation
  #define TOP97_A49                    // display status bar
  #define TOP97_A50                    // Wrap of entire document in TE
  #define TOP97_A51                    // Display segment boundaries in PostEdit
  #define TOP97_A52                    // Translate more than 1 document
  #define TOP97_A53                    // Wrap around in file/change
  #define TOP97_A54                    // Enable (API) source element display
  #define TOP97_A55                    // Neutralize on hyphen in dictlookup
  #define TOP97_A56                    // Word 6J support
  #define TOP97_A59                    // Windows/NT - run current version
  #define TOP97_A61                    // Year 2000 support
  #define TOP97_A75                    // Faster organize / merge / analyze
  #define TOP97_TERSENAMETABLE         // terse TM name tables
  #define TOP97_MATCHCOUNT             // count TM match info in analysis
  #define TOP97_A57E_DORGTERMS         // display number of organized terms
  #define TOP97_A57G_TMPROP            // TM property dialog
  #define TOP97_A57C                   // handling for doc TM during FolExp
  #define TOP97_A57J                   // merge confirmation for TM during FolImp
  #define TOP97_A57K_RENAME            // object rename function
  #define TOP97_A57M_PRINTLIST         // print list windows function
  #define TOP97_A57L_PRESETFNAME       // preset file name in import dialogs
  #define TOP97_A57D_DOCPROP           // old props in doc change dialog
  #define TOP97_A57I_TMNEW             // TM new dialog for TMs imported via folder
  #define TOP97_A57                    // ITM: Goto Next Irregular
  #define TOP97_A57H                   // Show list of windows in TE identical for OS/2 and Windows
  #define TOP97_A57B                   // Style setting for TM window
  #define TOP97_A64                    // Compact + 1 (Shorten) Style
  #define TOP97_A80                    // SOSI in translation window
  #define TOP98_MULTFLAG               // Remove multiple flag in TM (and more)
  #define R004360_MORETHAN300DOCS      // Allow more than 300 documents per folder
  #define R_4349                       // dictionary indicator
  #define R004353_MARKMORETHAN1SEG     // allow blockmarking over segboundary
  #define R004316_MORETHAN26DICTS      // copy dicts with "CTRL-A/Z" too

  #define COUNTING_REPORT              // summary counting report

  #define R004350_HLUNMATCHTAG         // highlight unmatching tag when Ctrl-enter
  #define R004132_NOCOUNT              // add NOCOUNT attribute
  #define R004355_PROPMARKCOPY         // copy marked area from prop into actseg
  #define R004318_FINDSYNCH            // synchronize original/Translation during Find&Change
  #define R004290_IMPPATH              // stre relative path information with doc
  #define REDUNDCOUNT                  // redundancy counting
  #define R004784_COUNTEXPORT          // export count file ...
  #define FUNCCALLIF                   // function call interface
  #define R05020_GENERIC_TM            // generic inline matching in TM
  #define R004996_BIDI                 // bidi cp setting
  #define R004989_BIDI                 // bidi cp setting
  #define R004992_STANDARDFORXLATE     // editor enabling for XLATEsegmentation
  #define R004459_SEGHISTCOLORS        // colors for proofreading
  #define R004357_HLMISSPELLED         // highlight misspelled words
  #define R4320_DONT_COUNT_S           // Don't count 's as one word
//  #define USE_SYMB_MARKUP_NAMES        // Use symbolic markup names
  #define R004212_BATCHARCHTM          // batch mode archive TM
  #define R004203_CONVERSIONCP         // conversion codepage
  #define R004442_GOUPDSEGMENT         // go to next updated segment
//#define R005066_BACKSUBST            // auto-substitution in background
  #define R004422_BACKSAVE             // filesave in background
  #define R004332_LONGNAMES            // long names for folder, dictionary and TM
  #define TM_LARGENAMETABLES           // allow large name tables in TMs
  #define RAS400_ITM                   // allow trusted level for M-flag
  #define R_MOUSECLK                   // new double/triple mouse click
  #define R00_AFC                      // automatic folder creation
  #define REDIT_VISIBLE                // visible spaces, tabs, paragraphs
  #define R004356_SORT_AND_FILTER      // General sort and filter function
  #define ENABLE_SUBFOLDER             // enables subfolder functions
  #define NON_DDE_FOL_PROPS
  #define NON_DDE_SUBFOLDER
  #define NON_DDE_CONT_FOLDER
  // new Items for TP5.5
  #define R006311_FOLDERIMPORT_CONFIRMATION
  #define R006114_GFC_IGNORE_PROTECTED_TEXT
  #define R006310_GFC_RESIZE_RESULT_AREA
  #define R005887_HIDE_AND_SHRINK_PATH_NAMES
  #define R006176_MARKUP_TABLE_CONSTRUCTION_SET2
  #define R004582_MULTIPLE_SELECTION
  #define R004109_COPY_OR_MOVE_DOCUMENTS
  #define R004901_ADJUSTABLE_FUZZINESS
  #define R005839_AUTSUBST_CONTEXTINFO
  #define R005185_FUZZY_PERCENT        // displ. percentage of fuzzy props
  #define R006405_COLOR_TRNOTE         // special color for <TRNOTE>
  #define R005589_WS_EDITABLE          // userdefinable visible WS,seg.bound,...
  #define R005140_MORE_EXACT_PROPS     // if exact prop: displ.other exacts on request
  #define NON_DDE_CREATE_ITM
  #define NON_DDE_CHANGEMFLAG
  #define NON_DDE_CNT_REPORT
  #define NON_DDE_CONT_FOLDER
  #define NON_DDE_FOL_PROPS
  #define NON_DDE_SUBFOLDER
  #define R006062_AUTO_REF            // Auto refresh after non-DDE calls
  #define R006159_UNICODE              // add unicode support

#ifdef _FAST97
  #define TOP97                        // long file name support
  #define TOP97_A75                    // Faster organize / merge / analyze
  #define TOP97_A44                    // additional API interfaces
  #define TOP97_MATCHCOUNT             // count TM match info in analysis
#endif



// completed TP5.5 items for second shipment
#ifdef _TP55_2
  #define R006176_MARKUP_TABLE_CONSTRUCTION_SET2
  #define R004582_MULTIPLE_SELECTION
  #define R004901_ADJUSTABLE_FUZZINESS
  #define R005839_AUTSUBST_CONTEXTINFO
  #define R005185_FUZZY_PERCENT
  #define R006405_COLOR_TRNOTE
  #define R005589_WS_EDITABLE
  #define R005140_MORE_EXACT_PROPS
  #define R006159_UNICODE
  #define R006311_FOLDERIMPORT_CONFIRMATION
  #define R006114_GFC_IGNORE_PROTECTED_TEXT
  #define R006310_GFC_RESIZE_RESULT_AREA
  #define R005887_HIDE_AND_SHRINK_PATH_NAMES
  #define R004109_COPY_OR_MOVE_DOCUMENTS
  #define _TP55
#endif

#ifdef _TP55
  #define NON_DDE_CONT_FOLDER
  #define NON_DDE_FOL_PROPS
  #define NON_DDE_SUBFOLDER
  #define NON_DDE_CREATE_ITM
  #define NON_DDE_CHANGEMFLAG
  #define NON_DDE_CNT_REPORT
  #define R006062_AUTO_REF
  #define _TP54
#endif


#ifdef _TP54
//  #define USE_SYMB_MARKUP_NAMES        // Use symbolic markup names
  #define R_MOUSECLK                   // new double/triple mouse click
  #define REDIT_VISIBLE                // visible spaces, tabs, paragraphs
  #define R004356_SORT_AND_FILTER      // General sort and filter function
  #define R00_AFC                      // automatic folder creation
  #define ENABLE_SUBFOLDER             // enables subfolder functions
  #define _TP53                        // include TP52 topics
  #define R004442_GOUPDSEGMENT         // go to next updated segment
#endif




#ifdef _TP53
  #define R004992_STANDARDFORXLATE     // editor enabling for XLATEsegmentation
  #define R004459_SEGHISTCOLORS        // colors for proofreading
  #define R004357_HLMISSPELLED         // highlight misspelled words
  #define R4320_DONT_COUNT_S           // Don't count 's as one word
//#define USE_SYMB_MARKUP_NAMES        // Use symbolic markup names
  #define R004212_BATCHARCHTM          // batch mode archive TM
  #define R004203_CONVERSIONCP         // conversion codepage
  #define R004332_LONGNAMES            // long names for folder, dictionary and TM
  #define TM_LARGENAMETABLES           // allow large name tables in TMs
  #define RAS400_ITM                   // allow trusted level for M-flag
  #define R_MOUSECLK                   // new double/triple mouse click
  #define _TP52                        // include TP52 topics
#endif







#ifdef _MARCAM
  #define TOP97_MATCHCOUNT             // count TM match info in analysis
#endif

#ifdef _TP52
  #define R05020_GENERIC_TM            // generic inline matching in TM
  #define R004350_HLUNMATCHTAG         // highlight unmatching tag when Ctrl-enter
  #define R004132_NOCOUNT              // add NOCOUNT attribute
  #define R004318_FINDSYNCH            // synchronize original/Translation during Find&Change
  #define R004996_BIDI                 // bidi cp setting
  #define R004989_BIDI                 // bidi cp setting
  #define _TP51                        // include TP51 topics
#endif

#ifdef _TP51
  #define _TP50                        // include TP50 topics
  #define R004355_PROPMARKCOPY         // copy marked area from prop into actseg
  #define R004353_MARKMORETHAN1SEG     // allow blockmarking over segboundary
  #define R004316_MORETHAN26DICTS      // copy dicts with "CTRL-A/Z" too
  #define R004290_IMPPATH              // stre relative path information with doc
  #define REDUNDCOUNT                  // redundancy counting
  #define COUNTING_REPORT              // summary counting report
  #define R004784_COUNTEXPORT          // export count file ...
  #define TOP98_MULTFLAG               // Remove multiple flag in TM (and more)
  #define FUNCCALLIF                   // function call interface
#endif

#ifdef _MTLOG
  #define MTLOG                        // MT Logging
  #define _TP49
#endif

#ifdef _TP50
  #define _TP49
  #define R004360_MORETHAN300DOCS      // Allow more than 300 documents per folder
  #define R_4349                       // dictionary indicator
#endif

#ifdef _TP49
  #define _TP48
  #define TOP97_A57H                   // Show list of windows in TE identical for OS/2 and Windows
  #define TOP97_A57B                   // Style setting for TM window
  #define TOP97_A64                    // Compact + 1 (Shorten) Style
  #define TOP97_A80                    // SOSI in translation window
  #define TOP97_A54                    // Enable (API) source element display
  #define COUNTING_REPORT              // summary counting report
#endif

#ifdef _TP48
  #define TOP97                        // long file name support
  #define TOP97_A44                    // additional API interfaces
  #define TOP97_A45                    // No change in timestamp if QUIT
  #define TOP97_A46                    // Information about exact/fuzzy
  #define TOP97_A47                    // display scale line
  #define TOP97_A48                    // TRNOTES Enabling/Implementation
  #define TOP97_A49                    // display status bar
  #define TOP97_A50                    // Wrap of entire document in TE
  #define TOP97_A51                    // Display segment boundaries in PostEdit
  #define TOP97_A52                    // Translate more than 1 document
  #define TOP97_A53                    // Wrap around in file/change
  #define TOP97_A75                    // Faster organize / merge / analyze
  #define TOP97_TERSENAMETABLE         // terse TM name tables
  #define TOP97_MATCHCOUNT             // count TM match info in analysis
  #define TOP97_A57E_DORGTERMS         // display number of organized terms
  #define TOP97_A57G_TMPROP            // TM property dialog
  #define TOP97_A57C                   // handling for doc TM during FolExp
  #define TOP97_A57J                   // merge confirmation for TM during FolImp
  #define TOP97_A57K_RENAME            // object rename function
  #define TOP97_A57M_PRINTLIST         // print list windows function
  #define TOP97_A57L_PRESETFNAME       // preset file name in import dialogs
  #define TOP97_A57D_DOCPROP           // old props in doc change dialog
  #define TOP97_A57I_TMNEW             // TM new dialog for TMs imported via folder
  #define TOP97_A57                    // ITM: Goto Next Irregular
#endif


// JDEdwards, enhanced customer version

#ifdef _JDE
  #define TOP97_A57H                   // Show list of windows in TE identical for OS/2 and Windows
  #define TOP97_A57B                   // Style setting for TM window
  #define TOP97_A64                    // Compact + 1 (Shorten) Style
  #define TOP97_A80                    // SOSI in translation window
  #define TOP97_A54                    // Enable (API) source element display
  #define TOP97                        // long file name support
  #define TOP97_A44                    // additional API interfaces
  #define TOP97_A45                    // No change in timestamp if QUIT
  #define TOP97_A46                    // Information about exact/fuzzy
  #define TOP97_A47                    // display scale line
  #define TOP97_A48                    // TRNOTES Enabling/Implementation
  #define TOP97_A49                    // display status bar
  #define TOP97_A50                    // Wrap of entire document in TE
  #define TOP97_A51                    // Display segment boundaries in PostEdit
  #define TOP97_A52                    // Translate more than 1 document
  #define TOP97_A53                    // Wrap around in file/change
//  #define TOP97_A75                    // Faster organize / merge / analyze
  #define TOP97_TERSENAMETABLE         // terse TM name tables
  #define TOP97_MATCHCOUNT             // count TM match info in analysis
  #define TOP97_A57E_DORGTERMS         // display number of organized terms
  #define TOP97_A57G_TMPROP            // TM property dialog
  #define TOP97_A57C                   // handling for doc TM during FolExp
  #define TOP97_A57J                   // merge confirmation for TM during FolImp
  #define TOP97_A57K_RENAME            // object rename function
  #define TOP97_A57M_PRINTLIST         // print list windows function
  #define TOP97_A57L_PRESETFNAME       // preset file name in import dialogs
  #define TOP97_A57D_DOCPROP           // old props in doc change dialog
  #define TOP97_A57I_TMNEW             // TM new dialog for TMs imported via folder
  #define TOP97_A57                    // ITM: Goto Next Irregular
#endif



// Should allways used
#define TOP97_TERSENAMETABLE           // terse TM name tables

// TM Integration
#ifdef _WINDOWS
#define OEM_MT                         // use MT Integration
#endif


