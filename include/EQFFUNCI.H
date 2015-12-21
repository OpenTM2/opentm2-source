/*! EQFFUNCI.H      internal header file for non-DDE batch functions
	Copyright (c) 1999-2015, International Business Machines Corporation and others. All rights reserved.
*/

#define FCTDATA_IDENTIFIER 0x1998A04FL
#define FCTSEGFILE_IDENTIFIER 0x2002F3ABL

// name delimiter for name lists (e.g. list of document names)
#define NAME_DELIMITER            ','

// the identifier for our functions
typedef enum _FCTFUNCTIONS
{
  FCT_NONE,
  FCT_EQFCREATEFOLDER,
  FCT_EQFEXPORTFOLDER,
  FCT_EQFCOUNTWORDS,
  FCT_EQFIMPORTDICT,
  FCT_EQFEXPORTDICT,
  FCT_EQFIMPORTDOC,
  FCT_EQFEXPORTDOC,
  FCT_EQFIMPORTMEM,
  FCT_EQFEXPORTMEM,
  FCT_EQFORGANIZEMEM,
  FCT_EQFCREATEMEM,
  FCT_EQFANALYZEDOC,
  FCT_EQFDELETEMEM,
  FCT_EQFIMPORTFOLDER,
  FCT_EQFDELETEFOLDER,
  FCT_EQFARCHIVETM,
  FCT_EQFDELETEDOC,
  FCT_EQFCLEANMEMORY,
  FCT_EQFRENAME,
  FCT_EQFPROCESSNOMATCH,
  FCT_EQFEXPORTSEGS,
  FCT_EQFCREATEMARKUP,
  FCT_EQFFILTERNOMATCHFILE
} FCTFUNCTIONS, *PFCTFUNCTIONS;

// processing step identifiers
typedef enum _FCTPHASES
{
  FCTPHASE_INIT,
  FCTPHASE_NEXT,
  FCTPHASE_PROCESS,
  FCTPHASE_CLEANUP,
  FCTPHASE_DONE,
  FCTPHASE_ABORT
} FCTPHASES, *PFCTPHASES;


// internal segmented file control area
typedef struct _FCTSEGFILE
{
  ULONG       lMagicWord;              // FUNCDATA area identifier
  HPARSER     hParser;                 // parser handle
  HPARSSEGFILE hParsSegFile;           // handle of loaded segmented file
  PVOID       pNext;                   // ptr to next FCTSEGFILE area
  HSESSION    hSession;                // session handle
} FCTSEGFILE, *PFCTSEGFILE;

// internal session data area
typedef struct _FCTDATA
{
  ULONG       lMagicWord;              // FUNCDATA area identifier
  LONG        lCheckSum;               // checksum of FUNCDATA area
  SHORT       sLastFunction;           // last function performed

  // area for session data
  BOOL        fComplete;               // current process is complete flag
  CHAR        szEqfResFile[MAX_EQF_PATH];// name of resource file
  CHAR        szSystemPropPath[MAX_EQF_PATH]; // system properties path
  CHAR        szMsgFile[MAX_EQF_PATH]; // message file for error messages
  DDEMSGBUFFER LastMessage;            // buffer for last error message
  HWND        hwndErrMsg;              // handle for error mesages (HWND_FUNCIF)

  // area used by batch analysis
  PVOID       pvAnaInputData;          // pointer to analysis input data
  USHORT      usAnalysisPhase;         // current/next phase of analysis process
  USHORT      usAnalysisFilePhase;     // current file processing phase
  USHORT      usAnalysisSegPhase;      // current segment processing phase
  USHORT      usAnalysisRedCountPhase; // current redundnacy counting phase

  // area used by mem import
  PVOID       pvMemLoadIda;            // pointer to analysis input data
  USHORT      usMemLoadPhase;          // current/next phase of TM import
  USHORT      usMemLoadRC;             // return code

  // area used by word count
  PVOID       pvWordCountData;         // ptr to word count data
  USHORT      usWordCountPhase;        // current word count phase

  // area used by folder import
  PVOID       pvFolderImportIda;       // pointer to folder import IDA

  // area used by folder export
  PVOID       pvFolderExportIda;       // pointer to folder export IDA

  // area used by mem organize
  PVOID       pvMemOrganizeCommArea;   // pointer to organize CommArea

  // area used by mem export
  PVOID       pvMemExportCommArea;

  // area used by dictionary import
  PVOID       pvDicImportIda;          // pointer to dictionary import IDA
  USHORT      DicImportNextTask;       // next task for dictionary import

  // area used by dictionary export
  PVOID       pvDicExportIda;          // pointer to dictionary export IDA

  // area used by document import and export
  PVOID       pvDocImpExpIda;          // pointer to batch document import/export IDA

  // area used by archive TM
  PVOID       pvArchTMIda;             // pointer to batch archive TM IDA
  // general use buffer area
  OBJNAME     szObjName;               // buffer for object names

  // start of segmented file control area chain
  PFCTSEGFILE pSegFile;

  // current progress of some nonDDE functions
  USHORT      usProgress;

  // area used by SimpleMatchLevel
  PVOID       pvGetSimpleMatchLevelData;

  // area used by clean memory function
  PVOID       pvCleanMemory;           // pointer to clean memory IDA

  // area used by process NOMATCH function
  PVOID       pvProcessNomatch;        // pointer to process NOMATCH IDA

  PVOID       pvExportSegsIda;        // pointer to export segments IDA

  PVOID       pvFilterNoMatchFile;    // for pointer to filter nomatch file IDA
  PVOID       pvUnused2;              // for future use
  PVOID       pvUnused3;              // for future use


} FCTDATA, *PFCTDATA;

// Internal functions

// FctValidateSession: check and convert a session handle to a FCTDATA pointer
USHORT FctValidateSession
(
  HSESSION    hSession,                // session handle being validated
  PFCTDATA   *ppData                   // address of to caller's FCTDATA pointer
);

USHORT FctBuildCheckSum
(
  PFCTDATA    pData,                   // FCTDATA area
  PLONG       plCheckSum               // ptr to checksum variable
);

// Function to enable Automatic Refresh of TM List Windows after Non-DDE call
void SetSharingFlag(ULONG ulRefreshFlag);

// Function call I/F specific functions in other modules
__declspec(dllexport)
USHORT TAFuncAnalyzeDoc
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // Translation Memory for analysis
  PSZ         pszProfile,              // analyiss profile name or NULL
  PVOID       pvReserved,              // reserved for futurre enhancements
  LONG        lOptions                 // options for analysis
);
USHORT TAFuncAnalysisProcess
(
  PFCTDATA    pData                    // ptr to function interface data area
);
USHORT TAFuncPrepAnalysis
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // Translation Memory for analysis
  PSZ         pszProfile,              // analyiss profile name or NULL
  PVOID       pvReserved,              // reserved for futurre enhancements
  LONG        lOptions                 // options for analysis
);

__declspec(dllexport)
USHORT MemFuncImportMem
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemname,              // name of Translation Memory
  PSZ         pszInFile,               // fully qualified name of input file
  LONG        lOptions                 // options for Translation Memory import
);

USHORT MemFuncPrepImport
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemname,              // name of Translation Memory
  PSZ         pszInFile,               // fully qualified name of input file
  LONG        lOptions                 // options for Translation Memory import
);

USHORT MemFuncImportProcess
(
  PFCTDATA    pData                    // function I/F session data
);

__declspec(dllexport)
USHORT FolFuncCreateFolder
(
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDescription,          // folder description or NULL
  CHAR        chTargetDrive,           // folder target drive
  PSZ         pszMemname,              // folder Translation Memory
  PSZ         pszMarkup,               // folder markup
  PSZ         pszEditor,               // folder editor
  PSZ         pszDictionaries,         // list of dictionaries or NULL
  PSZ         pszSourceLanguage,       // folder source language
  PSZ         pszTargetLanguage,       // folder target language
  PSZ         pszConversion,           // optional export conversion
  PSZ         pszReadOnlyMems          // list of readonly TMs or NULL
);

__declspec(dllexport)
USHORT FolFuncCreateControlledFolder
(
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDescription,          // folder description or NULL
  CHAR        chTargetDrive,           // folder target drive
  PSZ         pszMemname,              // folder Translation Memory
  PSZ         pszMarkup,               // folder markup
  PSZ         pszEditor,               // folder editor
  PSZ         pszDictionaries,         // list of dictionaries or NULL
  PSZ         pszSourceLanguage,       // folder source language
  PSZ         pszTargetLanguage,       // folder target language
  PSZ         pszConversion,           // optional export conversion
  PSZ         pszReadOnlyMems,         // list of readonly TMs or NULL
  PSZ         pszPassword,             // password
  PSZ         pszProjCoordName,        // name of the project coordinator
  PSZ         pszProjCoordMail,        // project coordinator's mail
  PSZ         pszTranslatorName,       // name of the translator
  PSZ         pszTranslatorMail,       // translator's mail
  PSZ         pszProductName,          // Name of the product
  PSZ         pszProductFamily,        // Product Famiily
  PSZ         pszSimilarProduct,       // Similar Product Family
  PSZ         pszProductDict,          // Product subject area dictionary
  PSZ         pszProductMem,           // Product subject area memory
  PSZ         pszPreviousVersion,      // Previous version of the product
  PSZ         pszVersion,              // Version of the Product
  PSZ         pszShipmentNumber        // Shipment number
);

__declspec(dllexport)
USHORT FolFuncChangeFolProps
(
  PSZ         pszFolder,               // name of folder
  CHAR        chTargetDrive,           // traget drive
  PSZ         pszTargetLanguage,       // target language or NULL
  PSZ         pszMemName,              // folder Translation Memory or NULL
  PSZ         pszDictionaries,         // list of dictionaries or NULL
  PSZ         pszROMemories,           // list of read-only search memories or NULL
  PSZ         pszDescription,          // folder description or NULL
  PSZ         pszProfile,              // calculation profile name or NULL
  PSZ         pszUnUsed2               // for future enhancements, currently NULL
);

__declspec(dllexport)
USHORT FolFuncGetFolProps
(
  PSZ         pszFolderName,           // name of the folder
  PEXTFOLPROP pExtFolProps             // structure to fill in folder properties
);


__declspec(dllexport)
USHORT FolFuncCreateSubFolder
(
  PSZ         pszParentFolder,         // name of parent (sub)folder
  PSZ         pszSubFolderName,        // subfolders name
  PSZ         pszMemName,              // subfolders Translation Memory or NULL
  PSZ         pszMarkup,               // name of Markup used for subfolder
  PSZ         pszSourceLanguage,       // Source Language used for subfolder
  PSZ         pszTargetLanguage,       // Target Language used for subfolder
  PSZ         pszEditor,
  PSZ         pszConversion,           // Conversion used for subfolder
  PSZ         pszTranslator,           // Name of translator
  PSZ         pszTranslatorMail        // Mail of translator
);

__declspec(dllexport)
USHORT TAFuncArchTM
(
  PFCTDATA    pData,
  PSZ         pszFolderName,
  CHAR        chToDrive,
  PSZ         pszDocuments,
  PSZ         pszMemName,
  LONG        lOptions
);

__declspec(dllexport)
USHORT CntFuncCountWords
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  LONG        lOptions,                // options for word count
  PSZ         pszOutFile               // name of output file
);

__declspec(dllexport)
USHORT FolFuncExportFolder
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszTargetPath,           // folder target path
  LONG        lOptions,                // options for the folder export or 0L
  PSZ         pszDocuments,            // list of documents or NULL
  PSZ         pszDescription,          // export description or NULL
  PSZ         pszExportAs,
  PSZ         pszMemoryExportAs        // export folder memory with this name
);

__declspec(dllexport)
USHORT DicFuncImportDict
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszInFile,               // fully qualified name of input file
  PSZ         pszDictName,             // name of dictionary
  PSZ         pszPassword,             // password of dictionary
  LONG        lOptions                 // dictionary import options
);

// export a dictionary
__declspec(dllexport)
USHORT DicFuncExportDict
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszDictName,             // name of dictionary
  LONG        lOptions,                // dictionary export options or 0L
  PSZ         pszOutFile               // fully qualified name of output file
);

// import a document
__declspec(dllexport)
USHORT DocFuncImportDoc
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder receiving the documents
  PSZ         pszFiles,                // list of input files (documents)
  PSZ         pszMemname,              // document Translation Memory or NULL
  PSZ         pszMarkup,               // document markup or NULL
  PSZ         pszEditor,               // document editor or NULL
  PSZ         pszSourceLanguage,       // document source language or NULL
  PSZ         pszTargetLanguage,       // document target language or NULL
  PSZ         pszAlias,                // alias for document name or NULL
  PSZ         pszStartPath,            // optional start path
  PSZ         pszConversion,           // optional export conversion
  LONG        lOptions                 // document import options or 0L
);

// export a document
__declspec(dllexport)
USHORT DocFuncExportDoc
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszFiles,                // list of documents with path information
  PSZ         pszStartPath,            // optional start path
  LONG        lOptions                 // options for document export
);

__declspec(dllexport)
USHORT MemFuncExportMem
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemname,              // name of Translation Memory
  PSZ         pszOutFile,              // fully qualified name of output file
  LONG        lOptions                 // options for Translation Memory export
);

// create a new Translation Memory
__declspec(dllexport)
USHORT MemFuncCreateMem
(
  PSZ         pszMemname,              // name of new Translation Memory
  PSZ         pszDescription,          // description for new Translation Memory or NULL
  CHAR        chToDrive,               // target drive for new Translation Memory
  PSZ         pszSourceLanguage,       // Translation Memory source language
  LONG        lOptions                 // type of new Translation Memory
                                       // LOCAL_TM_OPT is the default
);

// delete a Translation Memory
__declspec(dllexport)
USHORT MemFuncDeleteMem
(
  PSZ         pszMemName               // Translation Memory being deleted
);

__declspec(dllexport)
USHORT MemFuncClearMTFlag( PSZ pszInMemory, PSZ pszOutMemory );

// organize a Translation Memory
__declspec(dllexport)
USHORT MemFuncOrganizeMem
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemName               // Translation Memory being deleted
);

__declspec(dllexport)
USHORT FolFuncImportFolder
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder being imported
  PSZ         pszFromPath,             // path containing the imported folder
  CHAR        chToDrive,               // target drive for folder
  LONG        lOptions                 // folder import options
);

__declspec(dllexport)
USHORT FolFuncDeleteFolder
(
  PSZ         pszFolderName            // name of folder being deleted
);

__declspec(dllexport)
USHORT DocFuncDeleteDoc
(
  PSZ         pszFolderName,           // name of folder containing the documents
  PSZ         pszDocuments             // list of documents being deleted
);

__declspec(dllexport)
USHORT MemFuncCreateITM
(
    PSZ  pszMemName,
    PSZ  pszFilePairs,
    PSZ  pszMarkup,
    PSZ  pszSGMLMemFile,
    PSZ  pszSourceLanguage,
    PSZ  pszTargetLanguage,
    PSZ  pszSourceStartPath,
    PSZ  pszTargetStartPath,
    LONG lType
);


USHORT TAFuncArchiveTM
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // output Translation Memory
  LONG        lOptions                 // options for analysis
);
USHORT TAFuncArchiveTMProcess
(
  PFCTDATA    pData                    // ptr to function interface data area
);
USHORT TAFuncPrepArchTM
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  CHAR        chToDrive,               // drive letter
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // output Translation Memory
  LONG        lOptions                 // options for analysis
);

__declspec(dllexport)
USHORT MemFuncCleanMemory
(
  PFCTDATA         pData,              // function I/F session data
  PSZ              pszFolder,          // name of folder containing the translatable material
  PSZ              pszInMemory,        // fully qualified name of external input memory (encoding: UTF-16)
  PSZ              pszOutMemory,       // name of internal output memory or fully qualified name of external output memory
  LONG             lOptions            // options for processing
);

__declspec(dllexport)
USHORT MemFuncProcessNomatch
(
  PFCTDATA    pFuncData,              // function I/F session data
  PSZ         pszNomatch,              // fully qualified name of the SNOMATCh file
  PSZ         pszInMemory,             // name of the internal input memory
  PSZ         pszOutMemory,            // name of internal output memory (is created if it does not exist)
  PSZ         pszMemMatchReportText,   // fully qualified file name of the memory match count report (text format)
  PSZ         pszMemMatchReportXml,    // fully qualified file name of the memory match count report (XML format)
  PSZ         pszDupReportText,        // fully qualified file name of the duplicate word count report (text format)
  PSZ         pszDupReportXml,         // fully qualified file name of the duplicate word count report (XML format)
  PSZ         pszOutNomatchXml,        // fully qualified file name of the output nomatch file in nfluent XML format
  PSZ         pszUnused,               // additional parameter for future enhancements - currently no in use
  LONG        lOptions                 // options for processing
);

__declspec(dllexport)
USHORT FolFuncDeleteMTLog
(
  PSZ         pszFolderName            // name of the folder
);

__declspec(dllexport)
USHORT MarkupFuncCreateMarkup
(
  PSZ         pszInFile,               // fully qualified name of input file
  PSZ         pszOutFile,              // fully qualified name of output file
  LONG        lOptions                 // options for Markup Table create
);

__declspec(dllexport)
USHORT FolFuncOpenDoc
(
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszDocument,             // name of document
  ULONG       ulSegNum,                // segment number of segment to be activated
  ULONG       ulLine,                  // line to be activated (ulSegNum has to be 0)
  PSZ_W       pszSearch,               // search string 
  PSZ         pszTrackID               // TVT track ID to be activated
);

__declspec(dllexport)
USHORT FolFuncRemoveDocs
(
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszListFile              // name of list file 
);

__declspec(dllexport)
USHORT FolFuncRestoreDocs
(
  PSZ         pszFolderName            // name of the folder
);

__declspec(dllexport)
USHORT FolFuncAddCTIDList
(
  PSZ         pszFolderName,           // name of the folder
  PSZ         pszCTIDListFile          // fully qualified name of the CTID list file
);

__declspec(dllexport) 
unsigned short MorphFuncCheckSpelling
(
  char *pszLanguage,                   // name of the language being used for the spell checking
  char *pszInputTerms,                 // a comma seperated list of terms or NULL if a input file is being used
  char *pszInputFile,                  // the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTemrs is being used
  char *pszReport,                     // name of the report file receiving the results of the operation
  long  lOptions
);

__declspec(dllexport) 
unsigned short MorphFuncReduceToStemForm
(
  char *pszLanguage,                   // name of the language being used for the stem form reduction
  char *pszInputTerms,                 // a comma seperated list of terms or NULL if a input file is being used
  char *pszInputFile,                  // the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTemrs is being used
  char *pszReport,                     // name of the report file receiving the results of the operation, the report is in the XML format
  long  lOptions
);

// function removed irrelevant segments from an external memory
__declspec(dllexport) 
USHORT FilterNoMatchFile
(
  PFCTDATA         pData,                   // function I/F session data
  char             *pszInNoMatchXML,        // IN: input nomatch file (XML format)
  char             *pszGlobMemOptionFile,   // IN: global memory option file
  char             *pszMemory,              // IN: Memory for filtering
  char             *pszOutNoMatchXML,       // OUT: filtered nomatch file (XML format)  
  char             *pszOutNoMatchEXP,       // OUT: filtered nomatch file (EXP format)  
  char             *pszWordCountReport,     // OUT: Word count report
  LONG             lOptions                 // options for processing
);

// delete a dictionary
__declspec(dllexport)
USHORT DicFuncDeleteDict
(
  PSZ         pszMemName               // Translation Memory being deleted
);

// change the machine translation flags of memory proposals
__declspec(dllexport)
USHORT MemFuncChangeMFlag
(
    PSZ  pszMemName,
    LONG lAction
);

USHORT InternalExportFolder
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder
  PSZ         pszTargetPath,           // folder target path
  LONG        lOptions,                // options for the folder export or 0L
  PSZ         pszDocuments,            // list of documents or NULL
  PSZ         pszDescription,          // export description or NULL
  PSZ         pszExportAs,              // Export as - optional new name for exported folder
  PSZ         pszMemoryExportAs         // Export folder memory with this new name or NULL
);

/*! \brief Create a new folder
\param hSession the session handle returned by the EqfStartSession call
\returns 0 if successful or an error code in case of failures
*/
USHORT InternalImportFolder
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFolderName,           // name of folder being imported
  PSZ         pszFromPath,             // path containing the imported folder
  CHAR        chToDrive,               // target drive for folder
  LONG        lOptions                 // folder import options
);


