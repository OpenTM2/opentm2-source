/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#define LNG_WINDOW_WIDTH       800
#define LNG_WINDOW_HEIGHT      400

#define NAME_OF_SOURCE_BAK     "SOURCE.BAK"
#define SOURCE_NOT_ACTIVE      '#'
#define LNG_REPLSIZE            4000
#define D_PATH                 "DPATH"
#define CURR_DIR               0x03

#define LNG_CHECKLNG            1


#define STATE_LENGTH             20                 // length of access state
#define TITLE_LENGTH             20                 // length of window title
#define NOISE_LENGTH              8                 // length of Noise List name
#define COUNT_OF_PROPERTIES       4

//defines for colums in counted listbox
#define LNG_ITEM_FORMAT           "%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15"
#define LNG_LANGUAGE_IND       0
#define LNG_ASCIICP_IND        1
#define LNG_ANSICP_IND         2
#define LNG_CPNAME_IND         3
#define LNG_ISONAME_IND        4
#define LNG_ICUNAME_IND        5
#define LNG_GROUP_IND          6
#define LNG_ISSOURCE_IND       7
#define LNG_ISTARGET_IND       8
#define LNG_MORPHSUPPORT_IND   9
#define LNG_SPELLCHECK_IND     10





// #define LNG_MORPHDICT_IND      2


static SHORT sLngView[12] =  { LNG_LANGUAGE_IND, LNG_MORPHSUPPORT_IND, LNG_SPELLCHECK_IND, CLBLISTEND };
static SHORT sLngDetailsView[12] =  { LNG_LANGUAGE_IND, LNG_ASCIICP_IND, LNG_ANSICP_IND, LNG_CPNAME_IND, LNG_ISONAME_IND, LNG_ICUNAME_IND, LNG_GROUP_IND, LNG_ISSOURCE_IND, LNG_ISTARGET_IND, LNG_MORPHSUPPORT_IND, LNG_SPELLCHECK_IND, CLBLISTEND };

CHAR  ColHdr[ 12 ][80] = { "Language", "ASCII CP", "ANSI CP", "Codepage name", "ISO name", "ICU name", "Language group", "Source language", "Target language", "MorphSupport", "Spellcheck", "" };

//declare table for colum listbox
static CLBCOLDATA ColLngTable[] =
{ { ColHdr[0], 20,   TEXT_DATA, DT_LEFT },   //
  { ColHdr[1], 10,   TEXT_DATA, DT_CENTER },
  { ColHdr[2], 10,   TEXT_DATA, DT_CENTER },
  { ColHdr[3], 16,   TEXT_DATA, DT_LEFT },
  { ColHdr[4], 10,   TEXT_DATA, DT_LEFT },
  { ColHdr[5], 10,   TEXT_DATA, DT_LEFT },
  { ColHdr[6], 20,   TEXT_DATA, DT_LEFT },
  { ColHdr[7], 14,   TEXT_DATA, DT_CENTER },
  { ColHdr[8], 14,   TEXT_DATA, DT_CENTER },
  { ColHdr[9], 14,   TEXT_DATA, DT_CENTER },
  { ColHdr[10], 14,   TEXT_DATA, DT_CENTER },
  { NULL,      0,    TEXT_DATA, 0       }};

static CLBCTLDATA LngCLBData =
{  sizeof(CLBCTLDATA),                 // size of structure
   11,                                 // we have 10 data columns
   1,                                  // one character space between columns
   SYSCLR_WINDOWSTATICTEXT,            // paint title in color of static text
   SYSCLR_WINDOW,                      // background is normal window background
   SYSCLR_WINDOWTEXT,                  // paint item in color of window text
   SYSCLR_WINDOW,                      // background is normal window background
   '\x15',                             // use X15 character as data seperator
   sLngView,                           // set current (= last used) view list
   sLngView,                           // set default view list
   sLngDetailsView,                    // set user set details view list
   sLngView,                           // set view list for 'name' view option
   sLngView,                           // set sort criteria list
   ColLngTable };                      // set address of column definition table


struct _lnginst_check
{
   USHORT  usStorageSize;                      //Size of storage
   USHORT  usBytesRead;                        // The number of DosRead bytes....
   USHORT  usI;

   CHAR   szSourceLanguage[MAX_LANG_LENGTH];   //Source Language
   CHAR   szFullNameOfSource[MAX_EQF_PATH];    // Full name to the file with the source languages
   CHAR   szBuffer [MAX_LANGUAGE_PROPERTIES] ; //buffer for state
   CHAR   szPropBuffer [MAX_LANGUAGE_PROPERTIES] ;//buffer for source language
   CHAR   *pFilestorage;                       //storage
   PSZ    pszEnd;                              //Pointer to end of storage
   HFILE  hPropFile;                           // file handle
};
typedef struct _lnginst_check LNGINST_CHECK, *PLNGINST_CHECK ;
