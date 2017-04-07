/*! EQFFOL00.H - Internal header file for the EQF FolderHandler
	Copyright (c) 1990-2017, International Business Machines Corporation and others. All rights reserved.
*/
#ifndef INCL_EQFFOL00
#define INCL_EQFFOL00

#include "eqfxliff.h"     // this include file is a prereq for the following....

#define  FOL_ITEM_FORMAT       "%s\x15L%s\x15L%s\x15D%s\x15%s\x15%s\x15%s\x15%s\x15%s\x15%ld\x15"
typedef enum               //    ^      ^      ^      ^
{                          //
  FOL_OBJECT_IND,          //----+
  FOL_NAME_IND,            //-----------+
  FOL_FORMAT_IND,          //------------------+
  FOL_TRANSLATED_IND,      //-------------------------+
  FOL_SEGMENTED_IND,       //-------------------------
  FOL_EXPORTED_IND,        //-------------------------
  FOL_IMPORTED_IND,        //-------------------------
  FOL_TOUCHED_IND,         //-------------------------
  FOL_TOUCHEDTIME_IND,     //-------------------------
  FOL_SIZE_IND,            //-------------------------
  FOL_COMPLETE_IND,
  FOL_MODIFIED_IND,
  FOL_SCRATCH_IND,
  FOL_COPIED_IND,
  FOL_DOCFORMAT_IND,
  FOL_MEMORY_IND,
  FOL_SOURCELANG_IND,
  FOL_TARGETLANG_IND
} FOL_ITEMINDX;

/* FolderHandler Instance Data Area */
typedef struct _FOLDERIDA
{
    IDA_HEAD     IdaHead;              // common header of instance area
    OBJNAME      szPropName;           // object name for properties
    HWND         hLBox;                // list box wnd handle
    SHORT  sLastUsedViewList[MAX_VIEW];// last used view list
    SHORT  sDetailsViewList[MAX_VIEW]; // user's details view list
    SHORT  sSortList[MAX_VIEW];        // user's sort criteria list
    CHAR         szBuffer[4096];       // multi-purpose buffer
    CHAR szFormat[MAX_FILESPEC];        // folder format / Tag Table
    CHAR szMemory[MAX_FILESPEC];        // folder Translation Memory
    CHAR szSourceLang[MAX_LANG_LENGTH]; // folder source language
    CHAR szTargetLang[MAX_LANG_LENGTH]; // folder target language
} FOLDERIDA, *PFOLDERIDA;


/* Export window proc */
SHORT   DeleteDir( PSZ Path, PSZ Name, BOOL forceflg);
SHORT   DeleteFolder( PSZ objname, HWND hwndParent, PUSHORT pusYesToAllMode );
SHORT   CreateFolder( HWND hwnd, PBOOL pfMustNotClose, PSZ pszObjName);

// subfolder stuff
SHORT   FolCreateSubFolder( PSZ pszParentFolder );
INT_PTR CALLBACK SUBFOLDERDLGPROC( HWND, WINMSG, WPARAM, LPARAM);
__declspec(dllexport)
SHORT FolSubFolderProperties( PSZ pszObjName );

// reside in EQFFOL01.H
INT_PTR CALLBACK NEWFOLDERDLG( HWND, WINMSG, WPARAM, LPARAM);
INT_PTR CALLBACK FOLDERPROPSDLG( HWND, WINMSG, WPARAM, LPARAM);

//--- shortcut for enabling/disabling dialog controls
#define ENABLE( hwnd, id, flag ) WinEnableWindow( WinWindowFromID( hwnd, id ), \
                                                  flag )

#define IMPBUFFERSIZE 16000            // size of general import buffer
#define  SLIDER_INCR             101   // slider increment

#define FOLEXPOBJPREFIX    "Export:"   // prefix for folder export object name
#define FOLIMPOBJPREFIX    "Import:"   // prefix for folder import object name

#define EXT_OF_DLL         ".DLL"      // extension fo DLLs

#define SUBFOLDERPREFIX    "SUBF:"     // prefix to identify subfolder tasks

// extension of markup table info file
#define EXT_OF_TAGTABLEINFO ".MUTINFO"

// extension of memory info file
#define EXT_OF_MEMORYINFO ".MEMINFO"

typedef struct _FOLEXPBITFLAGS
{
      unsigned short fContainsMem : 1;   // package-contains-memory flag
      unsigned short fSelectedDocs: 1;   // package-contains-selected-docs only
      unsigned short fOldVers     : 1;   // package-is-in-version-1-format flag
      unsigned short fDeleted     : 1;   // folder-has-been-deleted-after-export flag
      unsigned short fOverWrite   : 1;   // overwrite-existing-folders flag
      unsigned short fWithNote    : 1;   // package-contains-an-export-note flag
      unsigned short fDocNameTable : 1;  // header area contains doc name table
      unsigned short fWithDocTMs  : 1;   // package contains document TMs
      unsigned short fNonUnicode  : 1;   // Documents are in non unicode
      unsigned short fMasterFolder: 1;   // Folder has been exported as master folder
      unsigned short fFree10      : 1;   // free bit flag
      unsigned short fFree11      : 1;   // free bit flag
      unsigned short fFree12      : 1;   // free bit flag
      unsigned short fHeaderType  : 3;   // type of header (= release)
} FOLEXPBITFLAGS;

//--- user header for exported folders ---
typedef struct _FOLEXPHEADER
{
   // the following variable is not used anymore as the size of
   // package headers may exceed 64k
   USHORT usHeaderSize;                // overall size of header structure

   EQF_BOOL fContainsDict;             // package-contains-dictionaries flag
   FOLEXPBITFLAGS BitFlags;            // folder flags
   union _Head
   {
     struct _Rel0                      // release 0 header type layout
     {
        CHAR szDescription[ 41];       // folder description
        CHAR szMemory[MAX_FILESPEC];   // folder's memory db
        CHAR szFormat[MAX_FILESPEC];   // folder's markup language table
        LONG lDateTime;                // date and time of export
        CHAR DicTbl[ NUM_OF_FOLDER_DICS*MAX_FILESPEC];  // folder's dictionaries
        CHAR UniqueID[21];             // folder's unique ID
        USHORT usNoteSize;             // size of followinf note buffer
        CHAR szNoteBuffer[1];          // start of variable length note buffer
     } Rel0;
     struct _Rel1                      // release 1 header type layout
     {
        CHAR szDescription[ 41];       // folder description
        CHAR szFormat[MAX_FILESPEC];   // folder's markup language table
        LONG lDateTime;                // date and time of export
        CHAR UniqueID[21];             // folder's unique ID
        USHORT usNoteSize;             // size of followinf note buffer
        CHAR szNoteBuffer[1];          // start of variable length note buffer
     } Rel1;
   } Head;
} FOLEXPHEADER, *PFOLEXPHEADER;

typedef enum _FOLEXPHEADERTYPES
{
  RELEASE0_HEADER,                     // header layout is of type Rel0
  RELEASE1_HEADER
} FOLEXPHEADERTYPES;

// types of folder import dialog (passed as flag in
typedef enum _FOLIMP_DIALOG_TYPES
{
   FOLDER_IMPORT,                      // behave as "Folder Import" dlg
   FOLDER_DELETE                       // bevave as "Delete exported folder" dlg
} FOLIMP_DIALOG_TYPES;

typedef struct _DICTLANG
{
  CHAR            szDict[MAX_LONGFILESPEC];  // name of dictionary
  CHAR            szLang[MAX_LANG_LENGTH];   // source language of dicionary
} DICTLANG, *PDICTLANG;

typedef struct _FOLPROPIDA
{
      HWND        hwndDlg;             // dialog window handle
      PPROPFOLDER pProp;               // ptr to folder properties
      BOOL        fNew;                // TRUE for New Folder dialog
      BOOL        fCBChange;           // TRUE if editor CB is changed by
                                       // program (to avoid handling of
                                       // WM_CONTROL message by dialog)
      HWND        hDicLBA;             // HWND of LB AVAILABLE Dics
      HWND        hDicLBS;             // HWND of LB SELECTED  Dics
      HWND        hwndFolLB;           // HWND of available folder listbox
      BOOL        fDisabled;           // dialog-is-disabled flag
      CHAR        szBuffer1[MAX_LONGPATH]; // buffer for object names
      CHAR        szBuffer2[MAX_LONGPATH]; // buffer for object names
      PDICTLANG   pDictLang;           // array for dictionary source languages
      HWND        hMemLBA;             // HWND of LB AVAILABLE memories
      HWND        hMemLBS;             // HWND of LB SELECTED  memories

      HWND        hwndTabCtrl;         // handle of tab control
      HWND        hwndPages[20];       // array with property pages
      BOOL        fMemSel;             // TRUE = selection dialog is called for memories
      // fields for delete history log dialog window
      CHAR        szUser[512];         // user name
      CHAR        szCaptcha[40];       // delete confirmation text
      CHAR        szDispCaptcha[80];   // delete confirmation text as displayed to the user
      CHAR        szUserCaptcha[40];   // confirmation text as entered by user
      CHAR        szObjName[MAX_EQF_PATH]; // buffer for object names
      PVOID       pvGMOptList;         // ptr to any loaded global memory option file
      PVOID       pvGMData;            // ptr to global memory tab private data
      BOOL        fInitComplete;       // TRUE = dialog initialization has been completed
      BOOL        fStartAnalysis;      // TRUE = start analysis when dialog is closed
      HWND        hDictPIDLBA;         // HWND of LB AVAILABLE Dict PID
} FOLPROPIDA, *PFOLPROPIDA;

typedef struct _FOLLANGIDA
{
  PPROPFOLDER pProp;                   // ptr to folder properties
  CHAR        szSource[MAX_LANG_LENGTH]; // buffer for source language
  CHAR        szTarget[MAX_LANG_LENGTH]; // buffer for target language
} FOLLANGIDA, *PFOLLANGIDA;

//--- IDA for folder export dialog ---
typedef struct _FOLEXPIDA
{
   PVOID     hFllProp;                 // handle of folder list properties
   CHAR      szName[ CCHMAXPATH ];     // name of folder (w/o path and ext )
   CHAR      szFolName[ CCHMAXPATH ];  // name of folder (w/o path )
   OBJNAME   szFolderObjName;          // complete folder name
   OBJNAME   szObjName;                // complete folder name
   HWND      hwndFll;                  // handle of active folder list
   OBJNAME   szFllObjName;             // object name of active folder list
   CHAR      chDrive;                  // currently selected drive button
   PVOID     pPackCtrl;                // ptr to package control structure
   ULONG     ulHeaderSize;             // size of package header
   PFOLEXPHEADER pHeader;              // ptr to package header area
   CHAR      szBuffer[ IMPBUFFERSIZE ]; // general use buffer
   CHAR      szTitle[80];              // title of export window
   HWND      hFrame;                   // frame handle of export window
   HWND      hClient;                  // client handle of export window
   HWND      hwndFolExpText;           // handle of export text field
   CHAR      szObjPath[ CCHMAXPATH ];  // path for object names
   CHAR      szSearchPath[ CCHMAXPATH ]; // path to search for objects
   CHAR      szFolder[ CCHMAXPATH ];   // target folder object name
   HWND      hwndSlider;               // handle of slider control
   USHORT    usCompleted;              // completion ratio, set by UtlPackWrite
   USHORT    usLastCompleted;          // last shown completion ratio
   BOOL      fDelete;                  // delete folder after export flag
   BOOL      fKill;                    // window has been closed
   BOOL      fDeleteInProgress;        // delete of folder is in progress
   BOOL      fOldVers;                 // export in old version
   HATOMTBL  hatomInError;             // atom table for in-error objects
   PDDEFOLEXP pDDEFolExp;              // DDE folder export area
   HWND      hwndErrMsg;               // parent handle for error messages
   BOOL      fWithROMem;               // export with R/O TMs
   BOOL      fFolderHasROMem;          // true if folder has R/O TMs
   HWND      hwndDocLB;                // listbox containing folde documents
   HWND      hwndLongNameLB;           // listbox containing document long names
   HWND      hwndTranslatorLB;         // listbox containing translator list
   CHAR      szDocName[MAX_FILESPEC];  // buffer for document short name
   CHAR      szLongName[MAX_LONGFILESPEC]; // buffer for document long name
   CHAR      szDocObjName[MAX_EQF_PATH];// buffer for document object name
   // new fields for "Export with document TMs" option
   CHAR      szFolMem[MAX_LONGFILESPEC];// buffer for name of folder TM
   BOOL      fDocMem;                  // TRUE if there are document TMs
   BOOL      fWithDocMem;              // TRUE if there are document TMs
   BOOL      fDict;                    // TRUE if there are dictionaries
   CHAR      szFolLongName[MAX_LONGFILESPEC]; // buffer for folder long name
   CHAR      szDocMem[MAX_LONGFILESPEC];  // buffer for document TM
   BOOL      fUseShortFolderName;      // use-short-folder-name-for-export flag
   BOOL      fExportAs;                // export using another name flag
   BOOL      fHistlog;                 // only histlog information
   BOOL      fTranslatorSelected;      // only histlog information
   CHAR      szExportAs[MAX_LONGFILESPEC]; // buffer for specified export-as name
   CHAR      szExportName[MAX_LONGFILESPEC]; // buffer for actual export name

   CHAR      szExpOriginator[MAX_DESCRIPTION];    // Originator defined during export
   CHAR      szExpEMail[MAX_DESCRIPTION];         // Originator Name e-mail
   CHAR      szVendor[MAX_DESCRIPTION];     // Originator defined during export

   HWND         hwndTabCtrl;                // handle of tab control
    HWND         hwndPages[20];              // array with property pages
    CHAR      szString[MAX_LONGFILESPEC];
    CHAR      szExportPath[MAX_LONGPATH];    // Export path
    BOOL     fOk;
    PSUBFOLINFO pSubFolInfo;                 // subfolder information table
    BOOL     fNonUnicode;                                        // export folder in non unicode
    PSZ      pszObjList;                    // list of folder objects for multiple export
    PSZ      pszActiveObj;                  // active object in pszObjList
    PSZ      pszNameList;                   // list of folder names for display in messages
    USHORT   usFolOverwriteMode;            // overwrite mode for exported folders
    BOOL     fWithFolderMem;                // export with folder TM
    BOOL     fIsMasterFolder;               // TRUE = folder is a master folder
    CHAR      szMemoryExportAs[MAX_LONGFILESPEC]; // buffer for specified export-as name
    BOOL      fMemoryExportAs;                // export using another name for folder mem.
    CHAR      szLongFolMem[MAX_LONGFILESPEC]; // old longname of folder memory
    USHORT    usExportFormat;                // folder format to use for folder export
    PFOLIMPEXPXLIFF pXLIFFData;              // data area for imprt/export in XLIFF format
    CHAR      szFullExportName[MAX_LONGPATH];// buffer for fully qualified export name
    BOOL      fExportAsMasterFolder;         // saved 'export as master folder'flag when exporting multiple folders
    BOOL      fWithDict;                     // saved 'exported with dict' flag
    CHAR      szMUTBuffer[ IMPBUFFERSIZE ];  // buffer for markup table processing 
    CHAR      szInfoFileName[MAX_LONGFILESPEC]; // buffer for infor file names
} FOLEXPIDA, *PFOLEXPIDA;

typedef enum _FOLIMPTASK
{
   LOADFILES,                           // load files to import directory
   PROCESSFOLDER,                       // process imported folder
   PROCESSDOCS,                         // process imported documents
   PROCESSTABLES,                       // process imported tag tables
   PROCESSMEMORY,                       // process imported memory
   PROCESSNEXTMEMORY,                   // process next imported memory
   PROCESSDICTIONARY,                   // process imported dictionary
   PROCESSNEXTDICTIONARY,               // process next imported dictionary
   IMPORTOK,                            // issue ok message
   IMPORTCANCEL,                        // issue import cancel message
   CLEANUP,                             // do a general cleanup
   PROCESSSUBFOLDERS,                   // process imported subfolders
} FOLIMPTASK;


//--- state and disposition values for the following structure
typedef enum _DOCSTAT
{
  //- document state values -------------------------------------------------
  NEWER_DOCSTAT,                       // imp doc is newer than existing one
  NEW_DOCSTAT,                         // imp doc is new, no existing one
  EQUAL_DOCSTAT,                       // imp doc is same date as existing one
  OLDER_DOCSTAT,                       // imp doc is older as existing one
  NONE_DOCSTAT,                        // no imp doc and no existing one
  OLDEXIST_DOCSTAT,                    // no imp doc but an existing one
  //- document processing disposition ----------------------------------------
  COPYSRCTGT_DOCDISP,                  // copy source and target document (+prop)
  COPYSRC_DOCDISP,                     // copy source document (+prop)
  COPYSRCDELTGT_DOCDISP,               // copy source and delete target document (+prop)
  COPYTGT_DOCDISP,                     // copy target, leave source as-is
  IGNORE_DOCDISP,                      // ignore imported document
} DOCSTAT;

//--- data for document processing within folder import ---
// Note: The size of this structure * MAX_DOCS_PER_FOLDER (=300) may not exceed
//       64K!
typedef struct _FOLIMPDOCDATA
{
  PSZ        pszDocObjName;            // ptr to document object name
  PSZ        pszLongName;              // ptr to document long name
  CHAR       szTargetShortName[MAX_FILESPEC]; // target short name
  BOOL       fIsNew;                   // TRUE if document is new
  FDATE      fdateSource;              // source doc date and time (2 bytes)
  FTIME      ftimeSource;
  FDATE      fdateImpSource;           // imported source doc date and time
  FTIME      ftimeImpSource;
  FDATE      fdateTarget;              // target doc date and time
  FTIME      ftimeTarget;
  FDATE      fdateImpTarget;           // imported target doc date and time
  FTIME      ftimeImpTarget;
  BYTE       bSourceState;             // state of source
  BYTE       bTargetState;             // state of target
  BYTE       bDisposition;             // how to process document
} FOLIMPDOCDATA, *PFOLIMPDOCDATA;

//--- IDA for folder import dialog ---
typedef struct _FOLIMPIDA
{
   USHORT    usDialogType;             // type of import dialog; see enum
                                       // FOLIMP_DIALOG_TYPES for valid values
   PVOID     hFllProp;                 // handle of folder list properties
   CHAR      szSourceFolName[MAX_LONGFILESPEC]; // name of source folder (no path+ext)
   CHAR      szSourceFolder[CCHMAXPATH];    // name of source folder (no path)
   OBJNAME   szObjName;                // complete folder name
   HWND      hwndFll;                  // handle of active folder list
   OBJNAME   szFllObjName;             // object name of active folder list
   CHAR      chFromDrive;              // currently selected 'From' drive button
   CHAR      chToDrive;                // currently selected 'To' drive button
   PVOID     pPackCtrl;                // ptr to package control structure
   PFOLEXPHEADER pHeader;              // ptr to package header area
   PSZ       pszBuffer;                // general use buffer
   ULONG     ulBufSize;                // current size of general use buffer
   CHAR      szTitle[80];              // title of import window
   HWND      hFrame;                   // frame handle of import window
   HWND      hClient;                  // client handle of import window
   HWND      hwndFolImpText;           // handle of import text field
   FOLIMPTASK NextTask;                // task todo if EQFN_TASKDONE is received
   CHAR      szObjPath[ MAX_LONGPATH * 2 ];  // path for object names
   CHAR      szSearchPath[ CCHMAXPATH ]; // path to search for objects
   CHAR      szTargetFolName[ MAX_LONGFILESPEC ];// target folder name (no path+ext)
   CHAR      szTargetFolder[ MAX_LONGFILESPEC ];// target folder name (no path)
   CHAR      szTargetFolObj[ CCHMAXPATH ];// target folder object name
   CHAR      szToDriveList[MAX_DRIVELIST];       // list of available target drives
   BOOL      fWithMem;                 // import with memory flag
   BOOL      fWithDict;                // import with dictionary flag
   BOOL      fKill;                    // window has been closed
   BOOL      fImportAs;                // "import as" checkbox is selected
   CHAR      szAction[ 256 ];          // action text displayed in window
   CHAR      chSysDrive;               // EQF system drive
   CHAR      szCurFile[ CCHMAXPATH ];  // currently processed memory or dict file
   BOOL      fNoFolderLanguage;        //  no-languages-for-folder set flag
   BOOL      fNoMsg;                   //  if TRUE no end message is displayed
   PDDEFOLIMP pDDEFolImp;              // DDE folder import area
   HWND      hwndErrMsg;               // parent handle for error messages
   OBJNAME   objCurProcess;            // object name of current process (TM
                                       // dictionary import or empty for no
                                       // seperate process active
   PFOLIMPDOCDATA pDocData;            // ptr to document info and processing table
   PPOOL     pPool;                    // string pool for strings in pDocData
   HWND      hwndShortNameLB;          // listbox for folder short names
   CHAR      szShortFolName[MAX_LONGFILESPEC];
   CHAR      szLongFolName[MAX_LONGFILESPEC];
   CHAR      szTargetShortFolName[MAX_LONGFILESPEC]; // short name of target folder
   CHAR      szLongBuffer[MAX_LONGFILESPEC];
   CHAR      szBuffer[1024];           // general purpose buffer
   HWND         hwndTabCtrl;                // handle of tab control
   BOOL      fOk;
   BOOL      fExists ;
   BOOL      fUpdated ;
   CHAR      szString[MAX_LONGFILESPEC];
   CHAR      szImportPath[MAX_LONGPATH];  // Import Path
  HWND         hwndPages[20];              // array with property pages
  PSUBFOLINFO pImpSubFolInfo;             // info regarding imported subfolders
  CHAR        szOrgFormat[MAX_FILESPEC];  // markup table of existing folder
  CHAR        szImpFormat[MAX_FILESPEC];  // markup table of imported folder
  HWND        hwndDlg;                    // handle of outer dialog window
  PSZ         pszFolderList;              // list of folders being imported
  PSZ         pszActiveFolder;            // ptr to active folder in pszFolderList
  CHAR        chOrgToDrive;               // 'To' drive as selected by user
  USHORT      usMaxObjects;               // number of objects processed during import
  USHORT      usProcessedObjects;         // number of processed objects
  PSZ         pAddTagTableFiles;          // list of additional user exit files being importet
  PSZ         pszNameList;                // list of folder long names
  CHAR        szFolderFormat[MAX_FILESPEC];  // markup table of folder after import
  CHAR        szFolderSourceLang[MAX_LANG_LENGTH];  // source language of folder after import
  CHAR        szFolderTargetLang[MAX_LANG_LENGTH];  // target language of folder after import
  CHAR        szImpSourceLang[MAX_LANG_LENGTH];  // source language of imported folder
  CHAR        szImpTargetLang[MAX_LANG_LENGTH];  // target language of imported folder
  CHAR        szOrgSourceLang[MAX_LANG_LENGTH];  // source language of existing folder
  CHAR        szOrgTargetLang[MAX_LANG_LENGTH];  // target language of existing folder
  USHORT      usImpMode;                   // folder import mode
  PFOLIMPEXPXLIFF pXLIFFData;              // data area for import/export in XLIFF format
  CHAR        szXliffFileName[MAX_LONGFILESPEC];// name of import XLIFF file
   BOOL      fWithMarkup;                 // import with markup tables and related stuff
  CHAR szImportDir[MAX_EQF_PATH];         // import directory name
  ULONG ulImportDirLength;                // length of import directory name
  CHAR        szGlobalMemOptFile[MAX_LONGFILESPEC];// name of global memory filter file
  CHAR        szOrgEditor[MAX_FILESPEC];     // editor of folder before import
  CHAR        szFolderEditor[MAX_FILESPEC];  // editor of folder after import
  BOOL        fUserExitCopyPending;        // show special folder import complete message
  BOOL        fFolderReserved;                   // TRUE = folder short name has been reserved
  CHAR        szInfoFileLine[512];         // buffer for lines of the markup table info file
  CHAR        fTranslationManagerFolder;   // TRUE = currently imported folder has been exported by TranslationManager
  PVOID       pvMemImportData;             // data area for the import of memory files
  USHORT      usImportError;               // error code of failed import or 0 if successful
  BOOL        fGlobalMemOptCheckRequired;  // state of fGlobalMemOptCheckRequired flag of the folder properties
} FOLIMPIDA, *PFOLIMPIDA;


// Subfolder dialog IDA
typedef struct _SUBFOLDERDLGIDA
{
  CHAR        szParentFolder[MAX_LONGFILESPEC];  // name of parent (sub)folder
  CHAR        szParentObjName[MAX_EQF_PATH];     // object name of parent (sub)folder
  CHAR        szObjectName[MAX_EQF_PATH];        // object name of subfolder
  CHAR        szMainFolderObjName[MAX_EQF_PATH]; // object name of main folder
  CHAR        szMainFolderPropName[MAX_EQF_PATH];// object name of main folder
  BOOL        fPropDlg;                          // TRUE  = Properties dialog
                                                 // FALSE = New subfolder dialog
  PPROPFOLDER pProp;                             // subfolder properties
  CHAR        szBuffer[2048];                    // general purpose buffer
  CHAR        szBuffer2[2048];                   // general purpose buffer
  CHAR        szFormat[MAX_LONGFILESPEC];        // currently selected format
  CHAR        szSourceLang[MAX_LANG_LENGTH];     // currently selected source language
  CHAR        szTargetLang[MAX_LANG_LENGTH];     // currently selected target language
  CHAR        szEditor[MAX_FILESPEC];            // currently selected target language
  CHAR        szConversion[MAX_DESCRIPTION];     // currently selected conversion
  CHAR        szMemory[MAX_LONGFILESPEC];        // currently selected TM
  CHAR        szNewMemory[MAX_LONGFILESPEC];     // buffer for new TM name
  CHAR        szVendor[MAX_DESCRIPTION];         // Vendor Name (Translator)
  CHAR        szVendorEMail[MAX_DESCRIPTION];    // Vendor e-mail
  BOOL        fDisabled;                         // dialog-is-disabled flag
} SUBFOLDERDLGIDA, *PSUBFOLDERDLGIDA;



// document name for the printout of the details of exported folders
#define FOLDER_DETAILS_PRINT_NAME "Folder Details"

INT_PTR CALLBACK NEWFOLDER2DLG( HWND, WINMSG, WPARAM, LPARAM);
SHORT CreateFolderDir( PPROPFOLDER pProp, BOOL fMsg );
SHORT CreateDir( PSZ Path, PSZ Name, BOOL msgflg);
void LoadFields_1( HWND, PFOLPROPIDA);
void LoadFields_2( HWND, PFOLPROPIDA);
BOOL FolDeleteSegDocuments( PFOLPROPIDA pIda );

MRESULT EXPENTRY FOLEXPDLGPROC ( HWND, WINMSG, WPARAM, LPARAM );

INT_PTR CALLBACK FOLEXPPROPDLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK FOLEXPPROP_GENERAL_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
);

INT_PTR CALLBACK FOLEXPPROP_OPTIONS_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
);

INT_PTR CALLBACK FOLIMPPROP_GENERAL_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
);


INT_PTR CALLBACK  FOLIMPPROP_DETAILS_DLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
);




INT_PTR CALLBACK FOLIMPPROPDLGPROC
(
HWND hwnd,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
);

MRESULT FolExpInit( HWND, WPARAM, LPARAM );
MRESULT FolExpControl( HWND, SHORT, SHORT );
MRESULT FolExpCommand( HWND, SHORT, SHORT );
MRESULT FolExpClose( HWND, WPARAM, LPARAM );

INT_PTR CALLBACK FOLIMPDLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
MRESULT FolImpInit( HWND, WPARAM, LPARAM );
MRESULT FolImpControl( HWND, SHORT, SHORT );
MRESULT FolImpCommand( HWND, SHORT, SHORT );
MRESULT FolImpClose( HWND, WPARAM, LPARAM );

INT_PTR CALLBACK FOLIMPINFODLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
MRESULT FolImpInfoInit( HWND, WPARAM, LPARAM );
MRESULT FolImpInfoCommand( HWND, WPARAM, LPARAM );
MRESULT FolImpInfoClose( HWND, WPARAM, LPARAM );

INT_PTR CALLBACK FOLEXPNOTEDLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
INT_PTR CALLBACK FOLMODELDLGPROC ( HWND, WINMSG, WPARAM, LPARAM );
MRESULT FolNewControl( HWND, SHORT, SHORT );
MRESULT FolNewCommand( HWND, SHORT, SHORT );

USHORT FolExpSetEnableState ( HWND, BOOL );
VOID   FolderImport( HWND, PSZ );
VOID   DeleteExportedFolder( VOID );
INT_PTR CALLBACK FOLIMPORTWP( HWND, WINMSG, WPARAM, LPARAM );
BOOL FolImpBuildObjList ( PSZ, PSZ, PSZ *, PULONG );
BOOL FolImpBuildObject( PSZ, PSZ, PSZ, ULONG, PSZ );
VOID FolMakeImportPath ( PSZ, CHAR, USHORT, PSZ );
VOID FolderExport( HWND hwnd, PSZ pSelFolderName, PSZ pszObjList );

BOOL FolderPropertySheetLoad( HWND hwndDlg, PFOLPROPIDA pFolIda);
MRESULT FolderPropsPropertySheetNotification( HWND hwndDlg, WPARAM , LPARAM );
MRESULT FolderPropsCommand( HWND, WPARAM, LPARAM );
INT_PTR CALLBACK FOLDERPROPS_GENERAL_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2);
INT_PTR CALLBACK FOLDERPROPS_AFC_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2);
MRESULT APIENTRY FOLDERPROPS_SHOWPROPS_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2);
INT_PTR CALLBACK FOLDERPROPS_PROJECTINFO_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2);
INT_PTR CALLBACK FOLDERPROPS_GLOBALMEMOPT_DLGPROC( HWND hwndDlg, WINMSG msg, WPARAM mp1, LPARAM mp2);

BOOL FolExpAddSelectedDocs( HWND hwndDlg, PFOLEXPIDA pIda, BOOL fWithMem, HWND hwnd );
BOOL FolExpAddMemory
(
  PFOLEXPIDA       pIda,               // ptr to folder export instance data area
  PSZ              pszMemory,          // name of translation memory
  HATOMTBL         hAtomTbl,           // atom table for in-error objects
  BYTE             bFormat,            // export in older TM format or not
  HWND             hwnd                // owner handle for error messages
);

BOOL FolExpAddDict
(
  PVOID            pPackCtrl,          // ptr to package control structure
  PSZ              pszDict,            // name of dictionary
  BYTE             bFormat,            // format of Dictionary in exported folder
  HWND             hwnd                // owner handle for error messages
);



VOID FolImpInfoPrintDetails ( PFOLIMPIDA  pIda, HWND hwndDlg );

PSZ  FolGetSourceLng( PFOLPROPIDA pIda, PSZ pszDictName );
USHORT FolUpdLangFromMemory
(
  HPROP            hFolProp,           // folder properties handle
  BOOL             fMsg                // do-message-handling flag
);
USHORT FolUpdLangFromMemoryHwnd
(
  HPROP            hFolProp,           // folder properties handle
  BOOL             fMsg,               // do-message-handling flag
  HWND             hwnd                // owner handle for error messages
);

INT_PTR CALLBACK FOLLANGDLGPROC
(
  HWND hwndDlg,
  WINMSG msg,
  WPARAM mp1,
  LPARAM mp2
);

VOID FolSetDefaultPos( EQF_PSWP );
BOOL FOLMakeDocListItem ( PPROPDOCUMENT, PSZ, PSZ, PSZ, PSZ, PSZ, PSZ );

BOOL FolGlobFindChange
(
  PSZ              pszFolObjName,      // folder object name
  BOOL             fFromFolderList,    // TRUE = called from folder list
  BOOL             fMultipleObjs       // TRUE = called with a list of folder object names
);

BOOL FolFuzzySearch
(
  PSZ              pszFolObjName,      // folder object name
  BOOL             fFromFolderList,    // TRUE = called from folder list
  BOOL             fMultipleObjs       // TRUE = called with a list of folder object names
);

BOOL FolSpellcheck
(
  PSZ              pszFolObjName,      // folder object name
  BOOL             fFromFolderList,    // TRUE = called from folder list
  BOOL             fMultipleObjs       // TRUE = called with a list of folder object names
);


USHORT FolBatchFolderCreate( HWND, PDDEFOLCRT );
USHORT FolBatchFolderExport( HWND, PDDEFOLEXP );
USHORT FolBatchFolderImport( HWND, PDDEFOLIMP );
USHORT FolBatchFolderDelete( PDDEFOLDEL );

USHORT FolMergeHistoryLog
(
  PSZ         pszMergeFrom,            // fully qualified name of input log
  PSZ         pszMergeInto,            // fully qualified name of existing log
  BOOL        fMsg,                    // message handling flag
  HWND        hwnd,                    // window handle for error messages
  PSZ         pszRenameFrom,           // rename from document name or NULL
  PSZ         pszRenameTo              // rename to document name or NULL
);
USHORT FolFilterHistoryLog
(
  PSZ         pszFromLog,              // fully qualified name of input log
  PSZ         pszToLog,                // fully qualified name of new log
  PSZ         pszDoc,                  // name of selected document
  BOOL        fMsg,                    // message handling flag
  HWND        hwnd                     // window handle for error messages
);
USHORT FolWriteNewTargetRec
(
  PSZ         pszFolObjName,           // object name of folder
  PSZ         pszDocument,             // name of document
  BOOL        fMsg,                    // TRUE = show errors directly
  HWND        hwnd                     // window to be used as parent for messages
);

INT_PTR CALLBACK FOLIMPDOCDLGPROC( HWND, WINMSG, WPARAM, LPARAM );

#endif

BOOL FolRenameFolder
(
  PSZ         pszOldAndNewName,        // old and new folder name (x15 seperated)
  BOOL        fMsg                     // show-error-messages flag
);

BOOL FolCorrectPropHead( PSZ pszPropName, CHAR chNewDrive, PSZ pszNewPath,
                         PSZ pszNewName, HWND hwnd );


// folder export/import formats
#define FOL_TMGR_FORMAT 0
#define FOL_XLIFF_FORMAT 2

#define FOLDEREXPORTXLIFF_EXT     ".XLF"            // ext. of an exported folder in XLIFF format

void FolFillFolderFormatCombobox( HWND hwnd, int id, USHORT usFormat );

USHORT FolIntRemoveDocs
(
  PSZ         pszFolObjName,           // folder object name
  PSZ         pszListFile,             // name of list file 
  int         *piNumOfDocs,            // callers buffer for the number of removed documents
  HWND        hwndErrMsg
);
USHORT FolIntRestoreDocs
(
  PSZ         pszFolObjName,           // folder object name
  int         *piNumOfDocs,            // callers buffer for the number of restored documents
  HWND        hwndErrMsg
);

// key values for markup table info file (folder export and folder import)
#define MUTINFOFILE_PLUGIN_KEY   "Plugin"
#define MUTINFOFILE_TABLE_KEY    "MarkupTable"
#define MUTINFOFILE_USEREXIT_KEY "UserExit"
#define MUTINFOFILE_ADDFILE_KEY  "AdditionalFile"
#define MUTINFOFILE_NAME_KEY     "Name"  
#define MUTINFOFILE_VERSION_KEY  "Version"
#define MUTINFOFILE_DESCRIPTION_KEY "Description"

// key values for memory info file (folder export and folder import)
#define MEMINFOFILE_PLUGIN_KEY   "Plugin"
#define MEMINFOFILE_FILES_KEY    "Files"
#define MEMINFOFILE_NAME_KEY     "Name"  


