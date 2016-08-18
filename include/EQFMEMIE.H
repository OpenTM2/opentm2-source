//+----------------------------------------------------------------------------+
//| EQFMEMIE.H                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Memory Export/Import Interface                                |
//|                                                                            |
//+----------------------------------------------------------------------------+

#ifndef EQFMEMIE_H_INCLUDED

#define EQFMEMIE_H_INCLUDED


// general defines

// max length of segment data (number of characters)
#define EQF_SEGLEN 2048

// max length of a language name  
#define EQF_LANGLEN 20

// max length of a format/markup table name
#define EQF_FORMATLEN 13

// max length of a document name
#define EQF_DOCNAMELEN     256

// max length of a document name
#define EQF_AUTHORLEN      15 

// max length of a memory description 
#define EQF_DESCRIPTIONLEN 41 

// return codes
#define MEM_IMPORT_COMPLETE  7001                // the import is complete

#pragma pack( push, segmentpacking, 1 )

// structure to pass/receive general memory information
typedef struct _MEMEXPIMPINFO
{
  BOOL    fUTF16;                          // TRUE = export in UTF16 format
  CHAR    szName[256];
  CHAR    szSourceLang[EQF_LANGLEN];
  CHAR    szDescription[EQF_DESCRIPTIONLEN];
  CHAR    szError[1024];                   // buffer for error messages
  BOOL    fError;                          // TRUE = error during parsing
  CHAR    szFormat[EQF_FORMATLEN];         // format / markup table name
  LONG    lOutMode;                        // converter only: memory output mode
  BOOL    fCleanRTF;                       // clean memory from RTF tags
  BOOL    fSourceSource;                   // converter only: convert to source/source memory
  BOOL    fTagsInCurlyBracesOnly;         // special option for fCleanRTF: only remove tags enclosed in curly braces
  BOOL    fNoCRLF;                        // remove CRLFs in segment text
  PSZ     pszMarkupList;                   // pointer to a list of markup tables (zero terminated C strigns followed by another zero)
  BYTE    fUnused[20];                     // room for enhancements
} MEMEXPIMPINFO, *PMEMEXPIMPINFO;

#define SEGDATABUFLEN 8096

// translation source values for usTranslationFlag in MEMEXPIMPSEG sgtructure
#define TRANSL_SOURCE_MANUAL 0
#define TRANSL_SOURCE_MANCHINE 1
#define TRANSL_SOURCE_GLOBMEMORY 2

// structure to pass/receive segment data

typedef struct _MEMEXPIMPSEG
{
  BOOL             fValid;                       // FALSE = segment data is invalid, count as invalid segment
  LONG             lSegNum;                      // segment number
  CHAR             szFormat[EQF_FORMATLEN];      // format / markup table name
  CHAR             szSourceLang[EQF_LANGLEN];    // segment source language
  CHAR             szTargetLang[EQF_LANGLEN];    // segment target language
  CHAR             szDocument[EQF_DOCNAMELEN];   // document  
  unsigned short   usTranslationFlag;            // numeric value indication the source of the translation
  LONG             lTime;                        // segment update time in C time format
  CHAR             szAuthor[EQF_AUTHORLEN];      // author of segment
  unsigned short   usLength;                     // length of segment data (incl. string end delimiter)
  WCHAR            szSource[SEGDATABUFLEN+1];    // buffer for segment source
  WCHAR            szTarget[SEGDATABUFLEN+1 ];   // buffer for segment target
  CHAR             szReason[200];                // reason for invalid segment
  WCHAR            szContext[SEGDATABUFLEN+1];    // buffer for segment context
  WCHAR            szAddInfo[SEGDATABUFLEN+1];    // buffer for additional information of segment 
} MEMEXPIMPSEG, *PMEMEXPIMPSEG;

#pragma pack( pop, segmentpacking )

// callback function which is used by ExtMemImportprocess to insert segments into the memory
typedef USHORT (APIENTRY *PFN_MEMINSERTSEGMENT)( LONG lMemHandle, PMEMEXPIMPSEG pSegment );

//+----------------------------------------------------------------------------+
//|                                                                            |
//| functions performing the external memory export/import                     |
//|                                                                            |
//+----------------------------------------------------------------------------+

//
// EXTMEMEXPORTSTART
//
// This function is called once at the begin of the memory export
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport) USHORT __cdecl EXTMEMEXPORTSTART
( 
  PLONG            plHandle,           // ptr to buffer receiving the handle for the external/import
  PSZ              pszOutFile,         // fully qualified file name for the exported memory
  PMEMEXPIMPINFO   pMemInfo            // pointer to structure containing general information for the exported memory
);

//
// EXTMEMEXPORTPROCESS
//
// This function is called for each segment being exported
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport) USHORT __cdecl EXTMEMEXPORTPROCESS
( 
  LONG             lHandle,                 // export/import handle as set by ExtMemExportStart function
  PMEMEXPIMPSEG    pSegment                 // pointer to structure containing the segment data
);

//
// EXTMEMEXPORTEND
//
// This function is called once at the end of the memory export
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport) USHORT __cdecl EXTMEMEXPORTEND
( 
  LONG             lHandle                  // export/import handle as set by ExtMemExportStart function
);

//
// EXTMEMIMPORTSTART
//
// This function is called once at the begin of the memory import
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport) USHORT __cdecl EXTMEMIMPORTSTART
( 
  PLONG            plHandle,           // ptr to buffer receiving the handle for the external/import
  PSZ              pszInFile,          // fully qualified file name for the memory being imported
  PMEMEXPIMPINFO   pMemInfo           // pointer to structure receiving general information for the imported memory
);

//
// EXTMEMIMPORTPROCESS
//
// This function is called until it returns an error condition or signals the end of the import
//
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport) USHORT __cdecl EXTMEMIMPORTPROCESS
( 
  LONG             lHandle,                 // Export/import handle as set by ExtMemImportStart function
  PFN_MEMINSERTSEGMENT pfnInsertSegment,    // callback function to insert segments into the memory
  LONG             lMemHandle,               // memory handle (has to be passed to pfnInsertSegment function)
  PLONG            lProgress           // pointer to callers progress indicator
);

//
// EXTMEMIMPORTEND
//
// This function is called once at the end of the memory import
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport) USHORT __cdecl EXTMEMIMPORTEND
( 
  LONG             lHandle                  // Import/import handle as set by ExtMemImportStart function
);

// function pointer typedefs for the functions above
#ifdef __cplusplus
extern "C" {
#endif
typedef USHORT (__cdecl *PFN_EXTMEMEXPORTSTART)( PLONG plHandle, PSZ pszOutFile, PMEMEXPIMPINFO pMemInfo );
typedef USHORT (__cdecl *PFN_EXTMEMEXPORTPROCESS)( LONG lHandle, PMEMEXPIMPSEG pSeg);
typedef USHORT (__cdecl *PFN_EXTMEMEXPORTEND)( LONG lHandle );
typedef USHORT (__cdecl *PFN_EXTMEMIMPORTSTART)( PLONG plHandle, PSZ pszInFile, PMEMEXPIMPINFO pMemInfo );
typedef USHORT (__cdecl *PFN_EXTMEMIMPORTPROCESS)( LONG lHandle, PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, PLONG lProgress );
typedef USHORT (__cdecl *PFN_EXTMEMIMPORTEND)(LONG lHandle );
#ifdef __cplusplus
}
#endif

// Converter functions
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT TMXTOEXP
( 
  PSZ              pszInMem,            // fully qualified name of input (TMX) memory
  PSZ              pszOutMem,           // fully qualified name of output (EXP) memory
  LONG             lOutMode,            // output mode ANSI/ASCII/UTF16
  LONG             lOptions,            // other processing options
  PSZ              pszMarkup,           // markup to use for segments
  PLONG            plValidSegs,         // number of segments written to EXP file
  PLONG            plSkippedSegs        // number of skipped segments
);

#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT EXPTOTMX
( 
  PSZ              pszInMem,            // fully qualified name of input (EXP) memory
  PSZ              pszOutMem,           // fully qualified name of output (TMX) memory
  PSZ              pszInMode,           // input mode "ANSI"/"ASCII"/"UTF16" or a codepage number
  LONG             lOutMode,            // output mode UTF16/UTF8
  PLONG            plSegs,              // number of written (=valid) segments
  PSZ              pszMsgBuffer,        // buffer for error messages
  PLONG            plInvalidSegs,       // number of skipped segments
  FILE             *hfLog               // handle of log file or NULL if no logging of invalid segments is to be done
);

#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
USHORT XLFTOEXP
( 
  PSZ              pszInMem,            // fully qualified name of input (XLIFF) memory
  PSZ              pszOutMem,           // fully qualified name of output (EXP) memory
  LONG             lOutMode,            // output mode ANSI/ASCII/UTF16
  LONG             lOptions,            // other processing options
  PLONG            plValidSegs,         // number of segments written to EXP file
  PLONG            plSkippedSegs        // number of skipped segments
);



#endif
