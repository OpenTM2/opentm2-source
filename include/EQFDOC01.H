/*! EQFDOCOH.H - header file for EQFDOCOH.
	document functions belonging to document handler
	Copyright (c) 1990-2015, International Business Machines Corporation and others. All rights reserved.
*/

#ifndef INCL_EQFDOC01
#define INCL_EQFDOC01



#define MAX_BUFFER        2048         // general work buffer size

#define NUMCHAR         256                   //number of character to check if
                                              //file is valid (DocumentLoad)
#define FIMPO_DEFAULT_DRIVE "C:"
#define FIMPO_DEFAULT_PATTERN_NAME "*"
#define FIMPO_DEFAULT_PATTERN_EXT ".*"
#define EXT_OF_REVISION_MARK      ".REV"
#define REVMARK_LF_TAG            "<LF>"      // line feed tag in revision marks

#define BUTTON_LENGTH 35
#define BUTTON_HEIGHT 25

#define CHECKED     1                       //check for buttons

#define LOAD_OK     10                      //retuns ...
#define LOAD_NOTOK  11                      //  .. from ...
#define LOAD_CANCEL 12                      //      DocumentLoad
#define LOAD_CLOSED 13                      //      DocumentLoad    /*KIT0891A*/

#define UNLOAD_OK      14                   //retuns ...            /*KIT0891C*/
#define UNLOAD_NOTOK   15                   //  .. from ...         /*KIT0891C*/
#define UNLOAD_CANCEL  16                   //      DocumentUnload  /*KIT0891C*/
#define UNLOAD_CLOSED  17                   //      DocumentUnload  /*KIT0891A*/




//--- user header for exported documents ---
typedef struct _DOCEXPHEADER
{
   USHORT usHeaderSize;                // overall size of header structure
   USHORT usHeaderRel;                 // release/version of header
   LONG   lDateTime;                   // date/time of export
   CHAR   szLongName[MAX_LONGFILESPEC];// long name of document
} DOCEXPHEADER, *PDOCEXPHEADER;

typedef struct _FILESPEC                    /*full specification of a file    */
{
   CHAR     szDrive[MAX_DRIVE];             /*drive letter with :             */
   CHAR     szPath[MAX_LONGPATH];           /*directory incl. trailing \      */
   CHAR     szName[MAX_FILESPEC];           /*filename                        */
   CHAR     szLongName[MAX_LONGFILESPEC];   /* buffer for long file name      */
   CHAR     szExt[MAX_FEXT];                /*file extension with leading dot */
   BOOL     fIsLongName;                    /* TRUE if a long fil name is used*/
} FILESPEC;


#define EXTRAPAGE_START       3

typedef struct _EXTRAPAGES
{
   HWND     hwndDlg;                   // dialog handle of extra page
   CHAR     szText[ 40 ];              // tab to be displayed
   CHAR     szToolTip[ 256 ];          // tooltip
   CHAR     szDLLName[ MAX_FILESPEC ]; // dll name
   CHAR     szResName[ MAX_FILESPEC ]; // name of resource dll
   CHAR     szImportProcName[ 40 ];    // name of import dlg procedure
   CHAR     szExportProcName[ 40 ];    // name of import dlg procedure
   USHORT   usResIDImp;                // resource id for import dlg
   USHORT   usResIDExp;                // resource id for export dlg
   CHAR     chMarkUp[ MAX_FILESPEC ];  // tagtable to be used
   HMODULE  hmodSegmentDll;            // handle of segment exit.

   EXTRAIDA ExtraIda;                 // format dependent stuff
   INT_PTR  (CALLBACK  *pfnImportProc) ( HWND, UINT, WPARAM, LPARAM );
   INT_PTR  (CALLBACK  *pfnExportProc) ( HWND, UINT, WPARAM, LPARAM );
//   BOOL CALLBACK pfnExportProcDialogProc(
//  HWND hwndDlg,  // handle to dialog box
//  UINT uMsg,     // message
//  WPARAM wParam, // first message parameter
//  LPARAM lParam  // second message parameter
//    DLGPROC  pfnExportProc;

   LRESULT  (APIENTRY  *pfnDoImportProc) ( PSZ, PSZ, PSZ, PSZ,
                                           PSZ, PSZ, PSZ, PVOID, HWND );
   LRESULT  (APIENTRY  *pfnDoExportProc) ( PSZ, PSZ, PSZ, PSZ,
                                           PSZ, PSZ, PSZ, PVOID, HWND );
} EXTRAPAGES, * PEXTRAPAGES;
/**********************************************************************/
/* helper macros                                                      */
/**********************************************************************/
#define GET_EXTRA_IDA( pExtra ) (pExtra->pIda)
#define GET_EXTRA_FILENAME( pExtra ) (pExtra->szFileName)

typedef struct _DOCIMPIDA                   //instance area for dialog
{                                           //procedure FImpoDlgProc
   IDA_HEAD  IdaHead;                       //common IDA header
   HWND      hwndDlg;                       // handle of dialog window
   HPROP     hProp;                         //handle of import/export properties
   CHAR      szDrives[MAX_DRIVELIST];       //list of available drives
   BOOL      fDictionary;                   //dictionary flag
   BOOL      fMemorydb;                     //memory database flag
   OBJNAME   szTargetFolder;                //object name of target folder
   CHAR      szToFolder[MAX_FILESPEC];      // target folder (name only)
   CHAR      szFromFolder[MAX_FILESPEC];    // source folder if importing in
                                            // internal format

   BOOL      fEnChanged;                    //flag for path entryfield changed
   BOOL      fCancel;                       //flag for CANCEL BUTTON
   FILESPEC  stFs;                          //file to be loaded
   CHAR      szPatternName[_MAX_FNAME];     //active pattern name
   CHAR      szPatternExt[MAX_FEXT];        //active pattern extension
   CHAR      szSource[MAX_LONGPATH];        // document source path
   CHAR      szTarget[MAX_LONGPATH];        // document target path

   CHAR      szProperties[MAX_PATH144];//work buffer for property path
   CHAR      szSysDir[MAX_PATH144];    //EQF system directory
   CHAR  szDummy[MAX_PATH144+MAX_FILESPEC];    //work buffer
   OBJNAME szObjName;                       // imex properies object name
   PVOID       pPackCtrl;              // ptr to package control structure
   CHAR        szImportPath[MAX_EQF_PATH];  // directory with imported files
  USHORT       usFormatID;             // ID of currently selected format RB
  CHAR         szString[1024];         // multi purose string buffer
  CONTROLSIDA  Controls;               // IDA for dialog utilities
  BOOL         fInitInProgress;        // initialization-is-in-progress flag
  BOOL         fImporting;             // import-of-files-in-progress flag
  BOOL         fBeingClosed;           // is dialog being closed  /*KIT0891A*/
  USHORT       usDocsInFolder;         // number of docs already in fldr /*KIT0925A*/
  CHAR         szDocObjName[MAX_PATH144]; // document object name for locking /*KIT1173A*/
  DOCUMENTINFO DocInfo;                // document property info structure
  BOOL         fYesToAll;              // yes-to-all flag
  USHORT       usSelDocs;              // number of selected docs for import
  CHAR         szBuffer[MAX_BUFFER];   // general work area
  HWND         hwndShortNameLB;        // listbox for document short names
  DOCEXPHEADER Header;                 // buffer for package header
  CHAR         szEAPath[MAX_EQF_PATH]; // buffer for name of doc EA data file
  PROPDOCUMENT stDocProp;              // buffer for old document properties
  CHAR         szToDoc[MAX_FILESPEC];  // target document short name of
  SHORT        sThreadStatus;          // flag to stop the scan directory thread
  CHAR         szStartPath[MAX_LONGPATH]; // buffer for start path
  HWND         hwndPropertySheet;      // handle of propertysheet
  EXTRAPAGES   ExtraPages[ 10 ];       // additional pages
  // subfolder stuff
  ULONG        ulParentID;             // ID of parent folder
  CHAR         szParentObjName[MAX_EQF_PATH]; // object name of parent folder
  HWND         hwndFolderListBox;      // listbox containing folder short names
  CHAR         szFolLongName[MAX_LONGPATH]; // buffer for long folder name
  BOOL         fDocExists;             // TRUE = document exists already
} DOCIMPIDA, *PDOCIMPIDA;


/******************************************************************************/
#define IDA_DLG_PTR 0

#define REVMARKID "TM/2\r\n\x1aREVMARK" // identifier for revision mark files

typedef struct _REVMARK
{
  CHAR            szID[20];            // identifier for revision mark files
  CHAR            szName[MAX_FNAME];   // revision mark name
  CHAR            szDescr[MAX_DESCRIPTION]; // revision mark description
  USHORT          usOffsCat1Beg;       // offset to rev mark begin for cat1
  USHORT          usOffsCat1End;       // offset to rev mark end for cat1
  USHORT          usOffsCat2Beg;       // offset to rev mark begin for cat2
  USHORT          usOffsCat2End;       // offset to rev mark end for cat2
  USHORT          usOffsCat3Beg;       // offset to rev mark begin for cat3
  USHORT          usOffsCat3End;       // offset to rev mark end for cat3
} REVMARK, *PREVMARK;

typedef struct _DOCEXPIDA                   //instance area for dialog
{                                           //procedure FExportDlgProc
   HWND        hwndDlg;                     //handle of export dialog
   IDA_HEAD    IdaHead;                     //common IDA header
   HWND        hwndExportListbox;           //handle of exportlistbox
   OBJNAME     szFolderObjName;             //full qualified folder obj name
   CHAR        szFolderName[MAX_FILESPEC];  //folder name
   CHAR        szDrives[27];                //list of available drives
   CHAR        szSelectedDrive[MAX_DRIVE];  //actual selected drive
   CHAR        szSelectedTransDrive[MAX_DRIVE];  //actual selected drive
   CHAR        szSelectedOrgDrive[MAX_DRIVE];  //actual selected drive
   CHAR        szSelectedSnoDrive[MAX_DRIVE];  //actual selected drive
   SHORT       sExportFormatID;             // format of export (ID of RB)
   BOOL        fTarget;                     //target files flag
   BOOL        fSource;                     //source files flag
   BOOL        fSNoMatch;                   //SNOMATCH segments flag
   BOOL        fWithRevMark;                // export with revision marks
   BOOL        fWithTrackID;                // export with TVT tracking IDs
   BOOL        fCancel;                     //flag for CANCEL BUTTON
   CHAR        szRevMarkFile[MAX_EQF_PATH]; // path name of revision mark file
   CHAR        szTargetDir[MAX_LONGPATH];   //target path with leading and
                                            //trailing backslash
   CHAR        szSourceDir[MAX_LONGPATH];   //source path with leading and
                                            //trailing backslash
   CHAR        szEAPath[MAX_EQF_PATH];      // buffer for name of doc EA data file
   CHAR        szSNoMatchDir[MAX_PATH144];  //SNOMATCH path with leading and
                                            //trailing backslash
   CHAR        szName[MAX_FILESPEC];        // selected document
   CHAR        szNewName[MAX_FILESPEC];     // new name for selected document
   CHAR        szSource[MAX_LONGPATH];      //work buffer for DosCopy FindFirst
   CHAR        szTarget[MAX_LONGPATH];      //work buffer for DosCopy FindFirst
   CHAR        szSegTargetFile[MAX_PATH144];//full qualified segmented target file
   FILEFINDBUF stResultBuf;                 //buffer for FindFirst
   CHAR        szPathContent[MAX_PATH144];  //string buffer for path entry field
   OBJNAME     szObjName;                   //object name of imex properties
   CHAR        szSysDir[MAX_PATH144];       //system directory
   OBJNAME     szDocObjName;                //object name of document properties
   CHAR        szString[MAX_LONGPATH +MAX_FILESPEC];  //work buffer for file names
   PVOID       pPackCtrl;              // ptr to package control structure
   PDOCEXPHEADER pHeader;              // ptr to header of export docs
   PREVMARK    pRevMark;                    // ptr to revision marks
   USHORT      usRevMarkSize;               // current size of revision mark buf
   CHAR        szActRevMark[MAX_FILESPEC];  // name of currently displayed revmark
   CHAR        szBuffer[MAX_BUFFER];        // general work area
   BOOL        fExporting;                  // still in export process /*KIT0891A*/
   BOOL        fSNoMatchExist;              // do SNOMATCH docs exist  /*KIT0891A*/
   BOOL        fBeingClosed;                // is dialog being closed  /*KIT0891A*/
   BOOL        fExportInProcess;            // export is still processed /*KIT0891A*/
   BOOL        fOverwrite;                  // overrite existing files flag
   CHAR        szLongName[MAX_LONGFILESPEC];// long name of current document
   CHAR        szExpName[MAX_LONGFILESPEC]; // name to be used for export
   BOOL        fIsLongName;                 // TRUE if document has a long file name
   BOOL        fRelPathInfo;                // TRUE = exported documents contain
                                            // relative path information
  HWND         hwndPropertySheet;           // handle of propertysheet
  EXTRAPAGES   ExtraPages[ 10 ];            // additional pages
  CHAR         szMemName[MAX_LONGFILESPEC]; // buffer for TM name
  CHAR         szPackName[MAX_FILESPEC];    // name of package file
  DOCEXPHEADER Header;                      // buffer for package header
  BOOL         fCreateAllDirs;              // TRUE = create all required directories
                                            // without further user confirmation
  // subfolder stuff
  ULONG        ulParentID;                  // ID of parent folder
  CHAR         szParentObjName[MAX_EQF_PATH]; // object name of parent folder

  CHAR         szConversion[MAX_DESCRIPTION]; // selected output conversion
  CHAR         szUseDocConv[MAX_DESCRIPTION]; // text 'use document conversion'
  CHAR         szDocConversion[MAX_DESCRIPTION]; // conversion from doc props
  CHAR         szFolderLongName[MAX_LONGFILESPEC];// long name of current folder
  // new fields for export in validation format
  CHAR         szValDir[MAX_LONGPATH];           // directory for export
  SHORT        sValFormat;                       // selected export format
  BOOL         fValExportWithProtSegs;           // TRUE = export with protected segments
  BOOL         fValExportCombine;                // TRUE = create combined output file
  BOOL         fValExportValFormat;
  BOOL         fValExportProofFormat;
  BOOL         fValExportAllMatch;
  BOOL         fValExportNewMatch;
  BOOL         fValExportProtMatch;
  BOOL         fValExportAutoMatch;
  BOOL         fValExportModAutoMatch;
  BOOL         fValExportNotTransl;
  BOOL         fValExportFuzzyMatch;
  BOOL         fValExportExactMatch;
  BOOL         fValExportMachMatch;
  BOOL         fValExportReplMatch;
  BOOL         fValExportRemoveTagging;
  CHAR         szValFormatOutputName[MAX_LONGFILESPEC]; // buffer for val format output name
  BOOL         fValExportInclCount;            // state of "Include word count info" checkbox
  BOOL         fValExportInclExisting;         // state of Include existing match info" checkbox
  BOOL         fValExportMismatchOnly;         // state of "Mismatches only" checkbox
  BOOL         fInit;                          // TRUE = currently processing INITDLG message
  BOOL         fValExportModExactMatch;        // state of modified exact match checkbox
  BOOL         fValExportLinksImages;          // state of preserve links and images
  BOOL         fValExportGlobMemMatch;
  CHAR_W       szValTranslator[256];           // translator name for report header
  CHAR_W       szValManager[256];              // manager name for report header
  BOOL         fValExportExactFromManual;      // check for exact matches coming from manual translation switch
  BOOL         fValExportTransOnly;            // check for export trans text in simple format switch
} DOCEXPIDA, *PDOCEXPIDA;

// modes for doc property fields
typedef enum _FIELDMODE
{
  SAMEVALUE,        // all documents have the same value in this field
  DIFFERENTVALUE,   // the value of this field is different in the documents
  VALUECHANGED      // this field has been changed by the user
} FIELDMODE;

/**********************************************************************/
/* Document property dialog IDA                                       */
/**********************************************************************/
typedef struct _DOCPROPIDA
{
  HPROP            hDocProp;           // handle of document properties
  BOOL             fPropDlg;           // property or new doc dialog flag
  PDOCUMENTINFO    pDocInfo;           // communication area of document import
  OBJNAME          szDocObjName;       // document object name
  CHAR             szBuffer[1024];     // general purpose buffer
  CHAR             szBuffer2[1024];    // second general purpose buffer
  CHAR             szBuffer1[MAX_LONGPATH];                // buffer for object names
  CHAR             szSourceLangBuffer[MAX_LANG_LENGTH];    // document source language buffer
  CHAR             szTargetLangBuffer[MAX_LANG_LENGTH];    // document target language buffer
  CHAR             szSourceLangFolBuffer[MAX_LANG_LENGTH]; // folder source language buffer
  CHAR             szTargetLangFolBuffer[MAX_LANG_LENGTH]; // folder target language buffer
  CHAR             szTransMemBuffer[MAX_LONGFILESPEC];     // document translation memory buffer
  CHAR             szTransMemFolBuffer[MAX_LONGFILESPEC];  // folder translation memory buffer
  CHAR             szMarkupBuffer[1024];                   // document markup table buffer
  CHAR             szMarkupFolBuffer[1024];                // folder markup table buffer
  CHAR             szEditorBuffer[MAX_FILESPEC];           // document markup table buffer
  CHAR             szEditorFolBuffer[MAX_FILESPEC];        // folder markup table buffer
  CHAR             szConvBuffer[MAX_DESCRIPTION];          // document conversion buffer
  CHAR             szConvFolBuffer[MAX_DESCRIPTION];       // folder conversion buffer
  CHAR             szChangedAliasBuffer[MAX_LONGFILESPEC]; // alias buffer
  CHAR             szFormat[MAX_FILESPEC];        // format / Tag Table
  CHAR             szMemory[MAX_LONGFILESPEC];    // Translation Memory
  CHAR             szSourceLang[MAX_LANG_LENGTH]; // source language
  CHAR             szTargetLang[MAX_LANG_LENGTH]; // target language
  BOOL             fDisabled;                     // dialog is in disabled state
  CHAR             szEditor[MAX_FILESPEC];        // editor
  CHAR             szConversion[MAX_DESCRIPTION]; // conversion
  CHAR             szNewMemory[MAX_LONGFILESPEC]; // Translation Memory
  CHAR             szFolVendorName[MAX_DESCRIPTION];      // translator's name of folder
  CHAR             szFolVendorEMail[MAX_DESCRIPTION];     // translator's email of folder
  CHAR             szFolShipment[MAX_DESCRIPTION];        // shipment number of folder
  HWND             hwndPages[20];              // AH array with property pages
  HWND             hwndTabCtrl;                // AH tabs
  // variables used for the handling of properties from multiple documents
  PSZ              pszObjNameList;               // object name list or NULL
  CHAR             szAllNameBuffer[4096];        // buffer for concatenated names
  FIELDMODE        modeEditor;                   // mode of editor field
  FIELDMODE        modeFormat;                   // mode of markup field
  FIELDMODE        modeMemory;                   // mode of memory field
  FIELDMODE        modeConversion;               // mode of conversion field
  FIELDMODE        modeVendorName;               // mode of vendor name field
  FIELDMODE        modeVendorEMail;              // mode of vendor email field
  FIELDMODE        modeSourceLang;               // mode of source language field
  FIELDMODE        modeTargetLang;               // mode of target language field
  FIELDMODE        modeShipment;                 // mode of shipment field
  CHAR             szDocFormat[MAX_FILESPEC];        // format / Tag Table of document
  CHAR             szDocMemory[MAX_LONGFILESPEC];    // Translation Memory of document
  CHAR             szDocEditor[MAX_FILESPEC];        // editor of document
  CHAR             szDocSourceLang[MAX_LANG_LENGTH]; // source language of document
  CHAR             szDocTargetLang[MAX_LANG_LENGTH]; // target language of document
  CHAR             szDocConversion[MAX_DESCRIPTION]; // conversion of document
  CHAR             szDocVendorName[MAX_DESCRIPTION]; // translator's name of document
  CHAR             szDocVendorEMail[MAX_DESCRIPTION];// translator's email of document
  CHAR             szDocShipment[MAX_DESCRIPTION];   // shipment number of document
  BOOL             fParentIsSubfolder;            // TRUE = parent of document is a subfolder
  CHAR             szParentObject[MAX_EQF_PATH];  // parent object name
} DOCPROPIDA, *PDOCPROPIDA;



/**********************************************************************/
/* Function prototypes                                                */
/**********************************************************************/
BOOL CheckExistence( PSZ, BOOL, USHORT, PUSHORT );
PSZ EqfMakePath( PSZ, PSZ, PSZ, PSZ );
PSZ EqfMakeDir( PSZ, PSZ, PSZ, PSZ );
PSZ EqfMakeFindPath( PSZ, PSZ, PSZ, PSZ, PSZ );
USHORT UtlCreateDir( PSZ );
SHORT CopyListBox( HWND, HWND );

USHORT DocumentLoad( PDOCIMPIDA, BOOL );     //load a document in external  format
USHORT DocumentUnload( PDOCEXPIDA, HWND );  //unload a document in external format
USHORT UnloadDocument( PDOCEXPIDA, BOOL );
BOOL   DocExpUpdateDosProps( PDOCEXPIDA pIda );
VOID RevMarkGetRevisionMarkText( HWND, USHORT, PDOCEXPIDA, PULONG, PUSHORT );
VOID RevMarkSetRevisionMarkText( HWND, USHORT, PDOCEXPIDA, USHORT );
USHORT ImportXLIFFDocument( PDOCIMPIDA, BOOL );     //load a document in external  format


MRESULT DocImpControl( HWND, SHORT, SHORT );

INT_PTR CALLBACK DocPropDlgProc
(
  HWND             hwnd,               // dialog window handle
  WINMSG           msg,                // message
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
);

BOOL DocPropPropertySheetLoad
(
HWND            hwnd,
PDOCPROPIDA     pIda
);

MRESULT DocPropPropertySheetNotification
(
HWND hwnd,
WPARAM  mp1,
LPARAM  mp2
);

INT_PTR CALLBACK DocPropSheetsDlgProc
(
HWND             hwnd,               // dialog window handle
WINMSG           msg,                // message
WPARAM           mp1,                // first message parameter
LPARAM           mp2                 // second message parameter
);

INT_PTR CALLBACK DocPropSettingsDlgProc
(
 HWND hwnd,                        //handle of dialog window
 WINMSG msg,                       // message
 WPARAM mp1,                       // first message parameter
 LPARAM mp2                        // second message parameter
 );


INT_PTR CALLBACK DocPropAdminDlgProc
(
 HWND hwnd,                        //handle of dialog window
 WINMSG msg,                       // message
 WPARAM mp1,                       // first message parameter
 LPARAM mp2                        // second message parameter
 );


INT_PTR CALLBACK DocPropStatisticsDlgProc
 (
 HWND hwnd,                       // handle of dialog window
 WINMSG msg,                      // message
 WPARAM mp1,                      // first message parameter
 LPARAM mp2                       // second message parameter
 );


INT_PTR CALLBACK LONGTOSHORTDLGPROC
(
  HWND             hwnd,               // dialog window handle
  WINMSG           msg,                // message
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
);

USHORT usGetNumberOfDocsInFolder (PSZ pszFolderObjectName);

/**********************************************************************/
/* prototypes for document import/export functions                    */
/**********************************************************************/

__declspec(dllexport) USHORT DocImpInternal( PDOCIMPIDA pIda );
__declspec(dllexport) int DocImpInternalDeletePackage( PSZ pszPackage );
VOID DocImpSourceName( PSZ, PSZ, PSZ, USHORT, PSZ );
VOID DocImpTargetName( PSZ pszTarget, PSZ pszFolder, USHORT usPathType,
                       PSZ pszDocName );
__declspec(dllexport) USHORT DocExpExportInternal
( 
  PSZ        pszFolderObjName,       // folder object name  
  PSZ        pszDocLongName,         // long name of exported document
  PSZ        pszDocShortName,         // long name of exported document
  CHAR       chTargetDrive,          // target drive letter
  HWND       hwndErrMsg,             // parent window handle for error messages
  PBOOL      pfOverwrite,            // pointer to "overwrite existing files" flag, is changed to TRUE when user selects yes-to-all
  PSZ        pszDocPackName          // NULL or pointer to a buffer receiving the package name actually being used for the export (may be different from document short name)
);

USHORT DocImpScanDir
(
  PSZ    pszStartDir,                  // pointer to start directory
  PSZ    pszPattern,                   // search pattern
  PSZ    pszPathFilter,                // pattern for path filter or NULL
  SHORT  sStartLen,                    // length of start path
  HWND   hwndLB,                       // handle of listbox being filled
  PSHORT psStatus                      // pointer to status flag
);
USHORT DocScanDirStartThread
(
  PSZ    pszStartDir,                  // pointer to start directory
  PSZ    pszPattern,                   // search pattern
  PSZ    pszPathFilter,                // pattern for path filter or NULL
  SHORT  sStartPathLen,                // length of original start path
  HWND   hwndLB,                       // handle of listbox being filled
  PSHORT psStatus                      // pointer to status flag
);
void DocScanDirThread
(
  void   *pvData                       // pointer to thread data
);

USHORT DocImpHandlePathInput
(
  HWND        hwndDlg                  // dialog window handle
);

USHORT DocExpHandlePathInput
(
  HWND        hwndDlg                  // dialog window handle
);

// handle ENTER key in Document import dialog
MRESULT DocImpDMGETDEFID
(
  HWND             hwndDlg,            // dialog window handle
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
);

USHORT DocImpStopThread
(
  PDOCIMPIDA  pIda                    // document import instance data area
);

// thread status values
#define THREAD_INACTIVE  0
#define THREAD_RUNNING   1
#define STOP_THREAD      2
#define THREAD_STOPPED   3


#endif
