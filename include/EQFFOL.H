/*! EQFFOL.H - Public header file for the FolderListHander and FolderHandler
	Copyright (c) 1999-2013, International Business Machines Corporation and others. All rights reserved.
*/

/**********************************************************************/
/* Document Info Structure                                            */
/*                                                                    */
/* Used as communication area between Document Import and             */
/* Document Property Dialog                                           */
/**********************************************************************/
typedef struct _DOCUMENTINFO
{
  BOOL             fUseForAll;                    // use-for-all-documents flag
  CHAR             szFormat[MAX_FILESPEC];        // format / Tag Table
  CHAR             szMemory[MAX_LONGFILESPEC];    // Translation Memory
  CHAR             szLongMemory[MAX_LONGFILESPEC];// Translation Memory (Long name)
  CHAR             szSourceLang[MAX_LANG_LENGTH]; // source language
  CHAR             szTargetLang[MAX_LANG_LENGTH]; // target language
  CHAR             szLongName[MAX_LONGFILESPEC];  // document long name
  CHAR             szAllias[MAX_LONGFILESPEC];    // alias for TM access
  CHAR             szEditor[MAX_FILESPEC];        // Translation Memory
  CHAR             szConversion[MAX_DESCRIPTION]; // document export conversion
  CHAR             szTranslator[MAX_DESCRIPTION]; // document export conversion
  CHAR             szTranslatorEmail[MAX_DESCRIPTION]; // document export conversion
  PPROPDOCUMENT    pDocProps;                     // temporary document properties to be used by DocProp dialog
  EQF_BOOL         fMarkupLocked;                 // TRUE = markup of document is locked and may not be changed by user
  EQF_BOOL         fSourceLangLocked;             // TRUE = source language is locked and may not be changed by user
  EQF_BOOL         fTargetLangLocked;             // TRUE = target language is locked and may not be changed by user
} DOCUMENTINFO, *PDOCUMENTINFO;

INT_PTR CALLBACK FIMPODLGPROPPROC( HWND, WINMSG, WPARAM, LPARAM);
INT_PTR CALLBACK FEXPORTDLGPROPPROC( HWND, WINMSG, WPARAM, LPARAM);

INT_PTR CALLBACK FIMPODLGPROC( HWND, WINMSG, WPARAM, LPARAM);
INT_PTR CALLBACK FEXPORTDLGPROC( HWND, WINMSG, WPARAM, LPARAM);
INT_PTR CALLBACK REVMARKDLGPROC( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport)
BOOL             DocumentDelete( PSZ, BOOL, PUSHORT );

// values for DocumentDeleteEx options
#define DOCDELETE_NOMTLOGDELETE 1
__declspec(dllexport)
BOOL             DocumentDeleteEx( PSZ  pszObjName, BOOL fMsg, PUSHORT pusMBReturn, int iOptions );



BOOL             DocPropertyDlg        // show document propert/new dialog
(
  HWND             hwndParent,         // handle of dialog parent window
  PSZ              pszDocName,         // object name of document
  BOOL             fPropDlg,           // TRUE  = popup document property dlg
                                       // FALSE = popup document new dlg
  PDOCUMENTINFO    pDocInfo,           // ptr to document info structure
  PSZ              pszObjNameList      // ptr to document object name list when more
                                       // than one document is selected or NULL
);

__declspec(dllexport)
USHORT           DocQueryInfo
(
  PSZ              pszDocName,         // object name of document
  PSZ              pszMemory,          // buffer for translation memory or NULL
  PSZ              pszFormat,          // buffer for document format or NULL
  PSZ              pszSrcLng,          // buffer for source language or NULL
  PSZ              pszTrgLng,          // buffer for target language or NULL
  BOOL             fMsg                // do-message-handling flag
);

__declspec(dllexport)
USHORT           DocQueryInfoHwnd
(
  PSZ              pszDocName,         // object name of document
  PSZ              pszMemory,          // buffer for translation memory or NULL
  PSZ              pszFormat,          // buffer for document format or NULL
  PSZ              pszSrcLng,          // buffer for source language or NULL
  PSZ              pszTrgLng,          // buffer for target language or NULL
  BOOL             fMsg,               // do-message-handling flag
  HWND             hwnd                // handle for error messages
);

__declspec(dllexport)
USHORT           DocQueryInfo2
(
  PSZ              pszDocName,         // object name of document
  PSZ              pszMemory,          // buffer for translation memory or NULL
  PSZ              pszFormat,          // buffer for document format or NULL
  PSZ              pszSrcLng,          // buffer for source language or NULL
  PSZ              pszTrgLng,          // buffer for target language or NULL
  PSZ              pszLongName,        // buffer for long name or NULL
  PSZ              pszAlias,           // buffer for alias or NULL
  PSZ              pszEditor,          // buffer for editor or NULL
  BOOL             fMsg                // do-message-handling flag
);

USHORT           DocQueryInfo2Hwnd
(
  PSZ              pszDocName,         // object name of document
  PSZ              pszMemory,          // buffer for translation memory or NULL
  PSZ              pszFormat,          // buffer for document format or NULL
  PSZ              pszSrcLng,          // buffer for source language or NULL
  PSZ              pszTrgLng,          // buffer for target language or NULL
  PSZ              pszLongName,        // buffer for long name or NULL
  PSZ              pszAlias,           // buffer for alias or NULL
  PSZ              pszEditor,          // buffer for editor or NULL
  BOOL             fMsg,               // do-message-handling flag
  HWND             hwnd                // handle for error messages
);

__declspec(dllexport)
USHORT           DocQueryInfoEx
(
  PSZ              pszDocName,         // object name of document
  PSZ              pszMemory,          // buffer for translation memory or NULL
  PSZ              pszFormat,          // buffer for document format or NULL
  PSZ              pszSrcLng,          // buffer for source language or NULL
  PSZ              pszTrgLng,          // buffer for target language or NULL
  PSZ              pszLongName,        // buffer for long name or NULL
  PSZ              pszAlias,           // buffer for alias or NULL
  PSZ              pszEditor,          // buffer for editor or NULL
  PSZ              pszConversion,      // buffer for conversion or NULL
  PSZ              pszUnUsed1,         // not in use yet, always NULL
  PSZ              pszUnUsed2,         // not in use yet, always NULL
  BOOL             fMsg,               // do-message-handling flag
  HWND             hwnd                // handle for error messages
);

__declspec(dllexport)
USHORT           FolQueryInfo
(
  PSZ              pszFolName,         // object name of folder
  PSZ              pszMemory,          // buffer for translation memory or NULL
  PSZ              pszFormat,          // buffer for folder format or NULL
  PSZ              pszSrcLng,          // buffer for source language or NULL
  PSZ              pszTrgLng,          // buffer for target language or NULL
  BOOL             fMsg                // do-message-handling flag
);

__declspec(dllexport)
USHORT           FolQueryInfo2
(
  PSZ              pszFolName,         // object name of folder
  PSZ              pszMemory,          // buffer for translation memory or NULL
  PSZ              pszFormat,          // buffer for folder format or NULL
  PSZ              pszSrcLng,          // buffer for source language or NULL
  PSZ              pszTrgLng,          // buffer for target language or NULL
  PSZ              pszEditor,          // buffer for editor or NULL
  BOOL             fMsg                // do-message-handling flag
);

__declspec(dllexport)
USHORT           FolQueryInfoHwnd
(
  PSZ              pszFolName,         // object name of folder
  PSZ              pszMemory,          // buffer for translation memory or NULL
  PSZ              pszFormat,          // buffer for folder format or NULL
  PSZ              pszSrcLng,          // buffer for source language or NULL
  PSZ              pszTrgLng,          // buffer for target language or NULL
  BOOL             fMsg,               // do-message-handling flag
  HWND             hwnd                // handle for error messages
);

__declspec(dllexport)
USHORT           FolQueryInfo2Hwnd
(
  PSZ              pszFolName,         // object name of folder
  PSZ              pszMemory,          // buffer for translation memory or NULL
  PSZ              pszFormat,          // buffer for folder format or NULL
  PSZ              pszSrcLng,          // buffer for source language or NULL
  PSZ              pszTrgLng,          // buffer for target language or NULL
  PSZ              pszEditor,          // buffer for editor or NULL
  BOOL             fMsg,               // do-message-handling flag
  HWND             hwnd                // handle for error messages
);

USHORT FolQueryInfoEx
(
  PSZ              pszFolName,         // object name of folder
  PSZ              pszMemory,          // buffer for translation memory or NULL
  PSZ              pszFormat,          // buffer for folder format or NULL
  PSZ              pszSrcLng,          // buffer for source language or NULL
  PSZ              pszTrgLng,          // buffer for target language or NULL
  PSZ              pszEditor,          // buffer for editor or NULL
  PSZ              pszConversion,      // buffer for conversion or NULL
  PSZ              pszVendor,          // buffer for vendor name or NULL
  PSZ              pszVendorEMail,     // buffer for vendor eMail address or NULL
  PBOOL            pfDisabled,         // folder disabled flag
  PVOID            pvUnused1,
  PVOID            pvUnused2,
  BOOL             fMsg,               // do-message-handling flag
  HWND             hwnd                // owner handle for error messages
);

__declspec(dllexport)
USHORT           FolLongToShortDocName
(
  PSZ              pszFolObjName,      // object name of folder
  PSZ              pszLongName,        // long document name
  PSZ              pszShortName,       // buffer for short document name
  PBOOL            pfIsNew             // TRUE if document is a new one
);


// flags for LoadDocumentNames
#define LOADDOCNAMES_NAME       0
#define LOADDOCNAMES_ITEMTEXT   1
#define LOADDOCNAMES_OBJNAME    2
#define LOADDOCNAMES_LONGNAME   4
// flag to list the document names of included subfolders also
#define LOADDOCNAMES_INCLSUBFOLDERS 0x1000000L

__declspec(dllexport)
USHORT  LoadDocumentNames( PSZ search_path, HWND hlbox, LONG flg, PSZ pszBuffer);

// flags for FolLoadSubfolder modes
#define LOADSUBFOLNAMES_NAME       0
#define LOADSUBFOLNAMES_ITEMTEXT   1
#define LOADSUBFOLNAMES_PARENTINFO 2
// flag to list all subfolders of a folder without checking the parentage
#define LOADSUBFOLNAMES_ALLSUBFOLDERS 0x1000000L

__declspec(dllexport)
USHORT FolLoadSubFolderNames( PSZ pszParentObjName, HWND hwndLB, LONG fFlag, PSZ pszBuffer );

__declspec(dllexport)
BOOL SubFolObjectNameToName( PSZ pszObjName, PSZ pszName );
__declspec(dllexport)
BOOL SubFolNameToObjectName( PSZ pszNameName, PSZ pszObjName );
BOOL SubFolIdToObjectName( PSZ pszFolderName, ULONG ulID, PSZ pszObjName );
__declspec(dllexport)
BOOL SubFolderDelete( PSZ pszSubFolderObjName, BOOL fBatch );
__declspec(dllexport)
BOOL FolIsSubFolderItem( PSZ pszItem );
__declspec(dllexport)
BOOL FolIsSubFolderObject( PSZ pszObjName );
__declspec(dllexport)
ULONG FolGetSubFolderIdFromObjName( PSZ pszObjName );
BOOL EqfFolderPropsToHtml(PSZ pszFolderObj, HWND hwndParent);

typedef struct _SUBFOLINFO
{
  CHAR        szName[MAX_LONGFILESPEC];// name of subfolder
  ULONG       ulParentFolder;          // ID of parent folder
  ULONG       ulID;                    // ID of this subfolder
  ULONG       ulValue;                 // ULONG for free use
  BOOL        fFlag;                   // BOOL for free use
} SUBFOLINFO, *PSUBFOLINFO;

BOOL SubFolCreateInfoTable( PSZ pszFolderObjName, PSUBFOLINFO *ppInfo );
BOOL IsSubFolContainedInParent( ULONG ulParent, ULONG ulChild, PSUBFOLINFO pInfo );

__declspec(dllexport)
USHORT FolCreateIntFolder
(
PSZ         pszFolderName,           // name of folder
PSZ         pszDescription,          // folder description or NULL
CHAR        chTargetDrive,           // folder target drive
PSZ         pszMemName,              // folder Translation Memory or NULL
PSZ         pszMarkup,               // folder markup or NULL
PSZ         pszEditor,               // folder editor or NULL
PSZ         pszSourceLanguage,       // folder source language or NULL
PSZ         pszTargetLanguage        // folder target language or NULL
);
__declspec(dllexport)
USHORT FolDeleteIntFolder
(
  PSZ         pszFolderName            // name of folder being deleted
);
BOOL FolDeleteFolder                   // TRUE = delete successful
(
  HWND       hwndParent,               // handle of parent window (for error messages)
  PSZ        pszObj,                   // object name of folder being deleted
  PUSHORT    pusYesToAllMode           // points to usYesToAllMode variable or NULL
);
BOOL FolIsFolderEmpty( PSZ pszFolObjName );

USHORT FolCleanHistlog( PSZ pszHistLogFile, BOOL fMsg, HWND hwnd );
USHORT FolCleanHistlogEx( PSZ pszHistLogFile, BOOL fMsg, HWND hwnd, BOOL fClean );

// IDA for dictionary/memory selection dialog
typedef struct _SELECTDLGIDA
{
  BOOL        fMemSel;                           // TRUE = memory selection, FALSE = Dict selection
  HWND        hwndAvailLB;                       // listbox containing available mems or dicts
  CHAR        aSelected[40][MAX_LONGFILESPEC];   // array with selected names
  int         iMaxSelected;                      // max number of selected objects
  int         iSelected;                         // number of selected objects
  int         iDialogID;                         // ID to use for dialog
  CHAR        szTitleBar[1024];                  // titlebar text
  PVOID       pFolPropIda;                       // pointer to folder properties dialog IDA (only used for dicts)
  BOOL        fPIDSel;                           // TRUE = Dictionary PID selection
  //------------ temporary buffers used by dialog --------------------------------------
  CHAR        szBuffer[1024];                    // general purpose buffer used by dialog itself
  CHAR        szBuffer2[1024];                   // general purpose buffer used by dialog itself
} SELECTDLGIDA, *PSELECTDLGIDA;

INT_PTR APIENTRY FOLMEMSEL_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2 );


__declspec(dllexport)
BOOL FolderListGlobalMemoryCheckRequired( PSZ pszFolderObject );

__declspec(dllexport)
BOOL FolIsFolderDisabled( PSZ pszFolderObject );
