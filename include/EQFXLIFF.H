//+----------------------------------------------------------------------------+
//| EQFXLIFF.H                                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: XLIFF import / export interface                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#ifndef EQFXLIFF_H_INCLUDED

#define EQFXLIFF_H_INCLUDED

//+----------------------------------------------------------------------------+
//|                                                                            |
//| Constants                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

// dummy return code of folder export/import function when function is complete
#define FOL_EXPORT_IMPORT_COMPLETE 9999

//+----------------------------------------------------------------------------+
//|                                                                            |
//| Structure containing information for the import and export of folders      |
//|                                                                            |
//+----------------------------------------------------------------------------+
typedef struct _FOLIMPEXPXLIFF
{
  LONG          lHandle;                         // handle used to access the CXLIFFExportImport class
  CHAR          szExportName[MAX_LONGPATH];      // fully qualified name of export (and import) file
  CHAR          szFolObjName[MAX_EQF_PATH];      // folder object name
  BOOL          fWithFolderMem;                  // include folder memory segments
  BOOL          fWithROMem;                      // include read-only memory segments
  BOOL          fWithDocMem;                     // include document memory segments
  BOOL          fWithDict;                       // include dictionary information
  int           iDocuments;                      // number of documents
  int           iDocListSize;                    // size of document list (number of bytes
  int           iDocListUsed;                    // used bytes in document list
  PSZ           pszDocuments;                    // pointer to document list
  USHORT        usComplete;                      // current completion ratio of exsport/import operation
  CHAR          szError[1024];                   // buffer for error messages
  BOOL          fError;                          // TRUE = errors occured
  CHAR          szTargetFolder[MAX_LONGPATH];    // name of target folder
  CHAR          szTargetFolObj[MAX_LONGPATH];    // object name of target folder
  HWND          hwndErrMsg;                      // window handle for error messages
} FOLIMPEXPXLIFF, *PFOLIMPEXPXLIFF;

//+----------------------------------------------------------------------------+
//|                                                                            |
//| functions performing the XLIFF export/import of documents and folders      |
//|                                                                            |
//+----------------------------------------------------------------------------+

//
// XLIFFIMPORT
//
__declspec(dllexport) 
int XLIFFIMPORT
(
  CHAR             chDrive,            // drive letter of drive containing the folder
  PSZ              pszFolShortName,    // folder short name
  PSZ              pszDocShortName,    // document short name = name of file in XLIFF directory of folder
  PSZ              pszSourceLanguage,  // buffer for document source language
  PSZ              pszTargetLanguage   // buffer for document target language
);

//
// XLIFFEXPORT
//
__declspec(dllexport) 
int XLIFFEXPORT
(
  PSZ              pszSegTargetFile,   // fully qualified name of segmented target file
  PSZ              pszExportTarget,    // fully qualified name of XLIFF target file
  PSZ              pszFolShortName,    // folder short name
  PSZ              pszDocShortName     // document short name = name of file in XLIFF directory of folder
);

//
// FOLXLIFFEXPORT
//
extern "C" 
__declspec(dllexport) 
int FOLXLIFFIMPORT
(
  PFOLIMPEXPXLIFF  pData                // pointer to XLIFF folder import/export data area
);

//
// FOLXLIFFEXPORT
//
extern "C" 
__declspec(dllexport) 
int FOLXLIFFEXPORT
(
  PFOLIMPEXPXLIFF  pData                // pointer to XLIFF folder import/export data area
);

//
// FOLXLIFFCHECKFILE
//
extern "C" 
__declspec(dllexport) 
int FOLXLIFFCHECKFILE
(
  PSZ              pszFile,            // fully qualified name of file being checked
  PSZ              pszFolName,         // pointer to buffer for folder name
  HWND             hwndErrMsg          // window handle for error messages
);

#endif
