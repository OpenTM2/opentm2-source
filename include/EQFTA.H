/*! EQFTA.H      External header file for analysis functions
	Copyright (c) 1990-2015, International Business Machines Corporation and others. All rights reserved.
*/

/**********************************************************************/
/* Limits, capacity, boundaries                                       */
/**********************************************************************/
#define MAX_NOISEWORD        32            // maximum length of noiseword

/**********************************************************************/
/* Defines for start/stop table function TACreateStartStopTable       */
/**********************************************************************/
#define HIDDEN_CHAR          1         // the character is hidden
#define PROTECTED_CHAR       2         // the character is display only
#define UNPROTECTED_CHAR     3         // the character can be changed
#define LINEBREAK_CHAR       4         // the character is a line break
#define ENDOFSEG_CHAR        5         // the character is outside of the segment
#define COMPACT_CHAR         6         // 1st of inline tag in compact style
#define TAGPROT_CHAR         7         // protected tagging character
#define TRNOTE_CHAR          8         // part of a TRnote
/**********************************************************************/
/* Structure for start/stop tables as used by TACreateStartStopTable  */
/**********************************************************************/
typedef struct _STARTSTOP
{
   USHORT usStart;                     // start offset
   USHORT usStop;                      // stop offset
   USHORT usType;                      // type of entry
} STARTSTOP, *PSTARTSTOP;


// structure for commincation between analysis and MT functions
typedef struct _ANAMTDATA
{
  // fields filled by analysis
  CHAR        szFolderName[MAX_LONGFILESPEC];    // folder long name
  CHAR        szFolShortName[MAX_FILESPEC];      // folder short name
  CHAR        szSourceLang[MAX_LANG_LENGTH];     // source language for translation
  CHAR        szTargetLang[MAX_LANG_LENGTH];     // target language for translation

  // fields filled by MT functions
  CHAR        szMTJobObject[MAX_EQF_PATH];       // object name of MT job (properties file name)
  USHORT      usSmallFuzzy;                      // fuzziness limit for small segments
  USHORT      usMediumFuzzy;                     // fuzziness limit for medium segments
  USHORT      usLargeFuzzy;                      // fuzziness limit for large segments
  USHORT      usWordLimit;                       // max number of words in segment
  CHAR        szSegFile[MAX_EQF_PATH];           // name for file containing segments

  // fields filled by analysis
  HWND        hwndParent;                        // parent handle for dialog box

  // buffer for future enhancements
  CHAR        szFiller[1984];
} ANAMTDATA, *PANAMTDATA;

extern  TATAG TADummyTag [];

__declspec(dllexport)
BOOL EQFUnSeg( PSZ, PSZ, PTATAG, PBOOL, PSZ, USHORT); // remove segmenting tags
BOOL EQFUnSeg2( PSZ, PSZ, PTATAG, PBOOL, PSZ, USHORT, PSZ); // remove segmenting tags
__declspec(dllexport)
VOID EQFNoiseFiltering(PSZ, PEXCLUSIONLIST, PTOKENENTRY);
BOOL EQFUnSegRevMark
(
  PSZ pSegTarget,                       // segm.target file
  PSZ pTarget,                          // target file name
  PTATAG pTATag,                        // pointer to tag str.
  PBOOL  pfGoOn,                        // goon allowed
  PSZ    pFolder,                       // folder name passed
  PSZ    apszRevMark[6],                // array with revision mark pointers
  USHORT usTrackDocNum                  // not 0, TVT tracking document num
);

BOOL EQFUnSegRevMark2
(
  PSZ pSegTarget,                       // segm.target file
  PSZ pTarget,                          // target file name
  PTATAG pTATag,                        // pointer to tag str.
  PBOOL  pfGoOn,                        // goon allowed
  PSZ    pFolder,                       // folder name passed
  PSZ    apszRevMark[6],                // array with revision mark pointers
  USHORT usTrackDocNum,                 // not 0, TVT tracking document num
  PSZ    pszConversion                  // ptr to conversion name or NULL
);
__declspec(dllexport)
USHORT TACreateProtectTable
(
  PSZ              pszSegment,         // ptr to text of segment being processed
  PVOID            pVoidTable,         // ptr to tag table (PLOADEDTABLE)
  USHORT           usColPos,           // column position of first char in segment
  PTOKENENTRY      pTokBuffer,         // buffer used temporarly for tokens
  USHORT           usTokBufferSize,    // size of token buffer in bytes
  PSTARTSTOP       *ppStartStop,       // ptr to caller's start/stop table ptr
  PFN              pfnUserExit,         // ptr to user exit function
  ULONG            ulCP
);

__declspec(dllexport)
USHORT TACreateProtectTableW
(
  PSZ_W            pszSegmentW,        // ptr to text of segment being processed
  PVOID            pVoidTable,         // ptr to tag table
  USHORT           usColPos,           // column position of first char in segment
  PTOKENENTRY      pTokBuffer,         // buffer used temporarly for tokens
  USHORT           usTokBufferSize,    // size of token buffer in bytes
  PSTARTSTOP       *ppStartStop,       // ptr to caller's start/stop table ptr
  PFN              pvUserExit,          // ptr to user exit function
  PFN              pvUserExitW,
  ULONG            ulCP
);

#define CREATEPROTTABLE_MARKATTR     0x02
#define CREATEPROTTABLE_NOTRANSLVAR  0x04


__declspec(dllexport)
USHORT TACreateProtectTableWEx
(
  PSZ_W            pszSegment,         // ptr to text of segment being processed
  PVOID            pVoidTable,         // ptr to tag table
  USHORT           usColPos,           // column position of first char in segment
  PTOKENENTRY      pTokBuffer,         // buffer used temporarly for tokens
  USHORT           usTokBufferSize,    // size of token buffer in bytes
  PSTARTSTOP       *ppStartStop,       // ptr to caller's start/stop table ptr
  PFN              pvUserExit,          // ptr to user exit function
  PFN              pvUserExitW,
  ULONG            ulCP,                // ASCII cp fitting to Segmenttext
  int              iMode                // mode for function
);



__declspec(dllexport)
USHORT TAPrepProtectTable
(
  PVOID            pVoidTable,         // ptr to tag table (PLOADEDTABLE)
  HMODULE          *phModule,          // address of user exit module handle
  PFN              *ppfnUserExit,      // address of ptr to user exit function
  PFN              *ppfnCheckSegExit,  // ptr to segment check function
  PFN              *ppfnShowTrans,     // ptr to Show transl.  function
  PFN              *ppfnTocGoto        // ptr to TOC goto function
);

__declspec(dllexport)
USHORT TAEndProtectTable
(
  HMODULE          *phModule,          // address of user exit module handle
  PFN              *ppfnUserExit,      // address of ptr to user exit function
  PFN              *ppfnCheckSegExit,  // ptr to segment check function
  PFN              *ppfnShowTrans,     // ptr to Show transl.  function
  PFN              *ppfnTocGoto,        // ptr to TOC goto function
  PFN              *ppfnUserExitW,
  PFN              *ppfnCheckSegExitW
);

__declspec(dllexport)
USHORT TALoadEditUserExit
(
  PVOID            pVoidTable,         // ptr to tag table (PLOADEDTABLE)
  PSZ              pszTableName,       // name of tag table (w/o path and ext.)
  HMODULE          *phModule,          // address of user exit module handle
  PFN              *ppfnUserExit,      // address of ptr to user exit function
  PFN              *ppfnCheckSegExit,  // address of ptr to segment check func
  PFN              *ppfnWYSIWYGExit,   // address of ptr to show transl.  func
  PFN              *ppfnTocGotoExit,   // address of ptr to TOC  func
  PFN              *ppfnGetSegContext, // address of ptr to EQFGETSEGCONTEXT function
  PFN              *ppfnUpdateContext, // address of ptr to EQFUPDATECONTEXT function
  PFN              *ppfnFormatContext, // address of ptr to EQFFORMATCONTEXT function
  PFN              *ppfnCompareContext, // address of ptr to EQFCOMPARECONTEXT function
  PFN              *ppfnUserExitW,      // unicode user exit for start-stop-tbl
  PFN              *ppfnCheckSegExitW,   // unicode seg.check func
  PFN              *ppfnCheckSegExExitW  // unicode seg.check Ex func
);

USHORT TAFreeEditUserExit
(
  HMODULE          *phModule           // address of user exit module handle
);

#define TA_INSERTTMMATCH   0x0001      // insert TM matches flag

#define TA_USEFOLDERSETTINGS 0x0002    // get analysis settings from folder properties

__declspec(dllexport)
BOOL TAAnalyzeFile
(
  PSZ        pszDocument,              // document object name
  HWND       hwndCaller,               // caller's window handle
  USHORT     usFlags,                  // flags for analyze process
  PSZ        pszEditor,                // name of editor or NULL
  PEQF_BOOL  *ppfKill                  // ptr to kill flag
);

BOOL TAGetTagNameByID
(
  PVOID         pVoidTable,            // pointer to tag table
  SHORT         sTagID,                // tag id
  PSZ           pszName                // buffer for tag name
);

__declspec(dllexport)
BOOL TAGetTagNameByIDW
(
  PVOID         pVoidTable,            // pointer to tag table
  SHORT         sTagID,                // tag id
  PSZ_W         pszName                // buffer for tag name
);


void TAFillWSList
(
	PSZ  pszTagTableName,
	PSZ_W  *ppWSList
);


BOOL TAAdjustWhiteSpace
(
  PSZ_W       pszSourceSeg,            // ptr to source segment data
  PSZ_W       pszTargetSeg,            // ptr to target segment
  PSZ_W       *ppszNewTargetSeg,       // ptr to ptr of output buffer
  BOOL        fLeadingWS,              // process leading whitespace
  BOOL        fTrailingWS,             // process trailing whitespace
  PBOOL       pfChanged,               // TRUE if target has been changed
  PSZ_W       pWSList                  // list of whitespace chars (TBX-CLS_WHITESPACEs)
);


VOID TABuildArchTM( HWND hwnd, PSZ pSelFolderName );
VOID ExportSegs( HWND hwnd, PSZ pSelFolderName );

// free memory used for table space
__declspec(dllexport)
USHORT TAFreeTableSpace( PVOID pvTable );
__declspec(dllexport)
BOOL TATagsToUnicode( PVOID pvLoadedTable, USHORT usSize, ULONG ulCP);



/**********************************************************************/
/* Free a loaded document                                             */
/**********************************************************************/
__declspec(dllexport)
VOID TAFreeDoc( PVOID *ppvDoc );

__declspec(dllexport)
USHORT TASegFileConvertASCII2Unicode( PSZ pInFile, PSZ pOutFile, PSZ pDocLang );
__declspec(dllexport)
USHORT TASegFileConvertUnicode2ASCII( PSZ pInFile, PSZ pOutFile, PSZ pDocLang );


//convert & copy contents TATag struct into unicode TATAG_W struct
VOID  TATATag2Unicode
(
  PTATAG    pTATagTemp,
  PTATAG_W  pTATagW
);


