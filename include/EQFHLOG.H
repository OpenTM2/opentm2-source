//+----------------------------------------------------------------------------+
//|EQFHLOG.H                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2013, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author:   QSoft (Regine Jornitz, Gerhard Queck)                             |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:  Defines and structures of the history log file                |
//+----------------------------------------------------------------------------+


//+----------------------------------------------------------------------------+
//|  Defines if used outside of EQF environment                                |
//+----------------------------------------------------------------------------+
#ifndef _EQF_H_INCLUDED
  #define MAX_FILESPEC            13     // max length of file name + extension
  #define MAX_PATH144             144    // max length of file name + extension
  #define MAX_SEGMENT_SIZE       2048
  #define MAX_MOSTUSEDDOCSPERSEG    5
#endif

//+----------------------------------------------------------------------------+
//|  Defines for the history log file                                          |
//+----------------------------------------------------------------------------+
// name of history log file
#define HISTLOGFILE     "HISTLOG.DAT"

// eye catcher for records of history log file
#define HISTLOGEYECATCHER 0xFF00EE11

// type of BOOL used in history log file records
#define HLOG_BOOL LONG

//+----------------------------------------------------------------------------+
//|  Task IDs for history log file                                             |
//+----------------------------------------------------------------------------+
typedef enum
{
  DOCIMPORT_LOGTASK = 1,               // document import
  DOCIMPNEWTARGET_LOGTASK,             // new target document due to import  --> EQFBHISTDOCSAVE
  ANALYSIS_LOGTASK,                    // analysis of document
  AUTOMATICSUBST_LOGTASK,              // automatic substitution of analysis --> EQFBHISTDOCSAVE
  DOCDELETE_LOGTASK,                   // delete of document
  DOCSAVE_LOGTASK,                     // save of document using TPRO
  DOCEXPORT_LOGTASK,                   // document export
  DOCPROP_LOGTASK,                     // document properties changed
  FOLPROP_LOGTASK,                     // folder properties changed
  LONGNAME_LOGTASK,                    // document with long name imported
  //<<NEW FORMAT>>
  DOCSAVE_LOGTASK2,                    // save of document using TPRO
  DOCIMPNEWTARGET_LOGTASK2,            // new target document due to import
  HISTDATA_INVALID_LOGTASK,            // histlog.dat is corrupted! for this file
  DOCAPI_LOGTASK,                      // histlog written vial API call
  HISTDATA_INCONSISTENT_LOGTASK,       // inconsistent histlog.dat
  VERSION_LOGTASK,                     // TMgr Version Info
  FOLPROPSHIPMENT_LOGTASK,             // folder shipment changed   (SHIPMENT_HANDLER)
  DOCIMPORT_LOGTASK2,                  // document import including (SHIPMENT_HANDLER)
  AUTOMATICSUBST_LOGTASK3,             // aut.substitution of analysis in DOCSAVEHIST3 format
  DOCSAVE_LOGTASK3,                    // save of document in DOCSAVEHIST3 format
  ANALYSIS_LOGTASK3,                   // analysis of document in DOCSAVEHIST3 format
  DOCAPI_LOGTASK3,                     // histlog written vial API call in DOCSAVEHIST3 format
  DOCIMPNEWTARGET_LOGTASK3,            // new target document due to import in DOCSAVEHIST3 format
  HISTLOGDELETE_LOGTASK,               // history delete log record
  HISTDATA_RESET_LOGTASK,              // reset/ignore any previous calculating infos
  FOLPROP_LOGTASK2,                    // folder properties changed record (new format)
  STARGET_CORRUPT_LOGTASK              // information in STARGET document was corrupted and has been corrected
} LOGTASK;


//+----------------------------------------------------------------------------+
//|  Sub types for export and import task                                      |
//+----------------------------------------------------------------------------+
typedef enum
{
  INTERN_SUBTYPE,
  EXTERN_SUBTYPE,
  FOLDER_SUBTYPE,
  NEWSOURCE_NOTIMPORTED_SUBTYPE,
  XLIFF_SUBTYPE
} IMPEXPSUBTYPE;

//+----------------------------------------------------------------------------+
//|  Layout of fixed length record part of history log file                    |
//+----------------------------------------------------------------------------+
// Note: The long name of the document is stored following the fixed length
//       part of the record. The length of the long name can be computed as
//       follows: usNameLen = usSize - usAddInfoLength - sizeof(HISTLOGRECORD)
//       In older records the Task field was defined as SHORT, it has now been
//       splitted into an Task UCHAR and 8 single bit values
//       Meaning of bit values:
//           fLongNameRecord  =  0 : this is an old histlog record, szDocName
//                                   may contain a short document name without
//                                   lond document name following
//                               1 : If there is no long name following the fixed
//                                   record part, the name in szDocName contains the
//                                   correct document name (no handling for short
//                                   documents required)

#pragma pack(1)
typedef struct _HISTLOGRECORD
{
   LONG    lEyeCatcher;                // bit pattern as start-of-record marker
   UCHAR   Task;                       // ID of task being logged
   unsigned char  fLongNameRecord :  1;// TRUE: Document long name may follow
   unsigned char  fUnused1  :  1;      // for future enhancements
   unsigned char  fUnused2  :  1;      // for future enhancements
   unsigned char  fUnused3  :  1;      // for future enhancements
   unsigned char  fUnused4  :  1;      // for future enhancements
   unsigned char  fUnused5  :  1;      // for future enhancements
   unsigned char  fUnused6  :  1;      // for future enhancements
   unsigned char  fUnused7  :  1;      // for future enhancements
   USHORT  usSize;                     // overall size of record
   LONG    lTime;                      // creation date/time of log record
   CHAR    szDocName[MAX_FILESPEC];    // name of document
   USHORT  usAddInfoLength;            // length of additional record data or 0
} HISTLOGRECORD, *PHISTLOGRECORD;

//+----------------------------------------------------------------------------+
//|  Layout of additional info part for task DOCSAVE                           |
//+----------------------------------------------------------------------------+

// summery of words per class
#pragma pack(1)
typedef struct _SumPerClass
{
   USHORT  usNumSegs;                  // number of segs in this class
   ULONG   ulSrcWords;                 // sum of all src words
   ULONG   ulModWords;                 // sum of all modified words
   ULONG   ulTgtWords;                 // sum of all target words
}  SUMPERCLASS, *PSUMPERCLASS;

// defines for the classes: simple sentences, medium sentences,
// complex sentences
#define SIMPLE_SENT_BOUND  5
#define MEDIUM_SENT_BOUND  15


// a criteria is, what the kind best proposal is:
#pragma pack(1)
typedef struct _CriteriaSum
{
   SUMPERCLASS  SimpleSum;             // number of segs in this class
   SUMPERCLASS  MediumSum;             // number of segs in this class
   SUMPERCLASS  ComplexSum;            // number of segs in this class
}  CRITERIASUM, *PCRITERIASUM;

#pragma pack(1)
typedef struct _DocSaveHist
{
   CRITERIASUM  AnalAutoSubst;         // sums for segs transl. by analauto
   CRITERIASUM  EditAutoSubst;         // sums for segs transl. by editauto
   CRITERIASUM  ExactExist;            // sums for segs with exact prop
   CRITERIASUM  ReplExist;             // sums for segs with repl.prop
   CRITERIASUM  FuzzyExist;            // sums for segs with fuzzy prop
   CRITERIASUM  MachExist;             // sums for segs with machine prop.
   CRITERIASUM  NoneExist;             // sums for segs with no proposal
   CRITERIASUM  NotXlated;             // sums for TOBE, ATTR, CURRENT
}  DOCSAVEHIST, *PDOCSAVEHIST;


// new structure for additional counting information
//--------------------------------------------------

#pragma pack(1)
typedef struct _DocSaveHist2
{
   CRITERIASUM  AnalAutoSubst;         // sums for segs transl. by analauto
   CRITERIASUM  EditAutoSubst;         // sums for segs transl. by editauto
   CRITERIASUM  ExactExist;            // sums for segs with exact prop
   CRITERIASUM  ExactUsed;             // sums for segs with exact prop used by translator
   CRITERIASUM  ReplExist;             // sums for segs with repl.prop
   CRITERIASUM  ReplUsed;              // sums for segs with repl.prop used by translator
   CRITERIASUM  FuzzyExist;            // sums for segs with fuzzy prop
   CRITERIASUM  FuzzyUsed;             // sums for segs with fuzzy prop used by translator
   CRITERIASUM  FuzzyExist_1;          // sums for segs with fuzzy prop
   CRITERIASUM  FuzzyUsed_1;           // sums for segs with fuzzy prop used by translator
   CRITERIASUM  FuzzyExist_2;          // sums for segs with fuzzy prop
   CRITERIASUM  FuzzyUsed_2;           // sums for segs with fuzzy prop used by translator
   CRITERIASUM  FuzzyExist_3;          // sums for segs with fuzzy prop
   CRITERIASUM  FuzzyUsed_3;           // sums for segs with fuzzy prop used by translator
   CRITERIASUM  MachExist;             // sums for segs with machine prop.
   CRITERIASUM  MachUsed;              // sums for segs with machine prop. used by translator
   CRITERIASUM  NoneExist;             // sums for segs with no proposal
   CRITERIASUM  NoneExist2;            // sums for segs with no proposal
   CRITERIASUM  NotXlated;             // sums for TOBE, ATTR, CURRENT
}  DOCSAVEHIST2, *PDOCSAVEHIST2;


// same structure as DOCSAVEHIST2 with fuzziness level info
#pragma pack(1)
typedef struct _DocSaveHist3
{
   CRITERIASUM  AnalAutoSubst;         // sums for segs transl. by analauto
   CRITERIASUM  EditAutoSubst;         // sums for segs transl. by editauto
   CRITERIASUM  ExactExist;            // sums for segs with exact prop
   CRITERIASUM  ExactUsed;             // sums for segs with exact prop used by translator
   CRITERIASUM  ReplExist;             // sums for segs with repl.prop
   CRITERIASUM  ReplUsed;              // sums for segs with repl.prop used by translator
   CRITERIASUM  FuzzyExist;            // sums for segs with fuzzy prop
   CRITERIASUM  FuzzyUsed;             // sums for segs with fuzzy prop used by translator
   CRITERIASUM  FuzzyExist_1;          // sums for segs with fuzzy prop
   CRITERIASUM  FuzzyUsed_1;           // sums for segs with fuzzy prop used by translator
   CRITERIASUM  FuzzyExist_2;          // sums for segs with fuzzy prop
   CRITERIASUM  FuzzyUsed_2;           // sums for segs with fuzzy prop used by translator
   CRITERIASUM  FuzzyExist_3;          // sums for segs with fuzzy prop
   CRITERIASUM  FuzzyUsed_3;           // sums for segs with fuzzy prop used by translator
   CRITERIASUM  MachExist;             // sums for segs with machine prop.
   CRITERIASUM  MachUsed;              // sums for segs with machine prop. used by translator
   CRITERIASUM  NoneExist;             // sums for segs with no proposal
   CRITERIASUM  NoneExist2;            // sums for segs with no proposal
   CRITERIASUM  NotXlated;             // sums for TOBE, ATTR, CURRENT
   // currently active fuzziness levels with two decimal places
   // (e.g. 50 is stored as 5000)
   LONG         lSmallFuzzLevel;       // fuzzines level for small segments
   LONG         lMediumFuzzLevel;      // fuzzines level for medium sized segments
   LONG         lLargeFuzzLevel;       // fuzzines level for large segments
}  DOCSAVEHIST3, *PDOCSAVEHIST3;



//+----------------------------------------------------------------------------+
//|  Layout of additional info part for task DOCIMPORT                         |
//+----------------------------------------------------------------------------+
typedef struct _DOCIMPORTHIST
{
  SHORT       sType;                   // type of import: INTERN, EXTERN, FOLDER
  CHAR        szPath[MAX_PATH144];     // input Path (EXTERN)
  CHAR        szFolder[MAX_FNAME];     // input folder (INTERN, FOLDER)
  CHAR        szFileName[MAX_PATH144]; // original file name (mixed-case and/or long file name) (for EXTERN type of import only)
  CHAR        szMarkup[MAX_FILESPEC];  // document markup table (EXTERN)
  CHAR        szMemory[MAX_FILESPEC];  // document TM or empty string (EXTERN)
  CHAR        szSourceLang[MAX_LANG_LENGTH]; //  doc source language (EXTERN)
  CHAR        szTargetLang[MAX_LANG_LENGTH]; //  doc target language (EXTERN)
  HLOG_BOOL   fSourceDocReplaced;      // TRUE = source document has been replaced
  HLOG_BOOL   fTargetDocReplaced;      // TRUE = target document has been replaced
} DOCIMPORTHIST, *PDOCIMPORTHIST;



//+----------------------------------------------------------------------------+
//|  SHIPMENT_HANDLER: Layout of additional info part for task DOCIMPORT
//+----------------------------------------------------------------------------+
typedef struct _DOCIMPORTHIST2
{
  SHORT       sType;                   // type of import: INTERN, EXTERN, FOLDER
  CHAR        szPath[MAX_PATH144];     // input Path (EXTERN)
  CHAR        szFolder[MAX_FNAME];     // input folder (INTERN, FOLDER)
  CHAR        szFileName[MAX_PATH144]; // original file name (mixed-case and/or long file name) (for EXTERN type of import only)
  CHAR        szMarkup[MAX_FILESPEC];  // document markup table (EXTERN)
  CHAR        szMemory[MAX_FILESPEC];  // document TM or empty string (EXTERN)
  CHAR        szSourceLang[MAX_LANG_LENGTH]; //  doc source language (EXTERN)
  CHAR        szTargetLang[MAX_LANG_LENGTH]; //  doc target language (EXTERN)
  HLOG_BOOL   fSourceDocReplaced;      // TRUE = source document has been replaced
  HLOG_BOOL   fTargetDocReplaced;      // TRUE = target document has been replaced
  CHAR        szShipment[MAX_DESCRIPTION];// folder description
} DOCIMPORTHIST2, *PDOCIMPORTHIST2;




//+----------------------------------------------------------------------------+
//|  Layout of additional info part for task DOCPROP                           |
//+----------------------------------------------------------------------------+
typedef struct _DOCPROPHIST
{
  CHAR        szMarkup[MAX_FILESPEC];  // document markup table or empty string
  CHAR        szMemory[MAX_FILESPEC];  // document TM or empty string
  CHAR        szSourceLang[MAX_LANG_LENGTH]; // doc source language
  CHAR        szTargetLang[MAX_LANG_LENGTH]; // doc target language
} DOCPROPHIST, *PDOCPROPHIST;

//+----------------------------------------------------------------------------+
//|  Layout of additional info part for task FOLPROP                           |
//+----------------------------------------------------------------------------+
typedef struct _FOLPROPHIST
{
  CHAR        szDescription[41];       // folder description
  CHAR        szEditor[MAX_FILESPEC];  // folder markup table
  CHAR        szMarkup[MAX_FILESPEC];  // folder editor
  CHAR        szMemory[MAX_FILESPEC];  // folder Translation Memory
  CHAR        szSourceLang[MAX_LANG_LENGTH];     // folder source language
  CHAR        szTargetLang[MAX_LANG_LENGTH];     // folder target language
  CHAR        DictTable[NUM_OF_FOLDER_DICS*MAX_FILESPEC]; // list of dicts
} FOLPROPHIST, *PFOLPROPHIST;

// object identifiers for the list of changes in FOLPROPHIST2 
#define FOLPROP_DESCRIPTION_OBJ 'C'
#define FOLPROP_EDITOR_OBJ      'E'
#define FOLPROP_MARKUP_OBJ      'A'
#define FOLPROP_MEMORY_OBJ      'M'
#define FOLPROP_SOURCELANG_OBJ  'S'
#define FOLPROP_TARGETLANG_OBJ  'T'
#define FOLPROP_DICTIONARY_OBJ  'D'
#define FOLPROP_RO_MEMORY_OBJ   'R'

// activity identifiers for the list of changes in FOLPROPHIST2 
#define FOLPROP_UNCHANGED_ACTION 'U'
#define FOLPROP_REMOVED_ACTION   'R'
#define FOLPROP_NEW_ACTION       'N'

//+----------------------------------------------------------------------------+
//|  Layout of additional info part for task FOLPROP2                          |
//|                                                                            |
//|  The list of changes is in the following format:                           |
//|     entry-1 entry-2 .... entry-n ZERO                                      |
//|                                                                            |
//|     each entry consists of                                                 |
//|     1 character object identifier                                          |
//|     1 character activity identifier                                        |
//|     n characters old/new/unchanged data (zero terminated)                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
typedef struct _FOLPROPHIST2
{
  BOOL        fOverFlow;               // list of changes did overflow szChanges buffer
  CHAR        szChanges[4000];         // list of changes in the folder properties
} FOLPROPHIST2, *PFOLPROPHIST2;


//+----------------------------------------------------------------------------+
//|  SHIPMENT_HANDLER                                                          |
//|  Layout of additional info part for task FOLPROPSHIPMENT                   |
//+----------------------------------------------------------------------------+
typedef struct _FOLPROPHISTSHIPMENT
{
  CHAR szShipment[MAX_DESCRIPTION];       // folder description
} FOLPROPHISTSHIPMENT, *PFOLPROPHISTSHIPMENT;




//+----------------------------------------------------------------------------+
//|  Layout of additional info part for task DOCEXPORT                         |
//+----------------------------------------------------------------------------+
typedef struct _DOCEXPORTHIST
{
  SHORT       sType;                   // Type of export: INTERN, EXTERN, FOLDER
  HLOG_BOOL   fSource;                 // TRUE = source has been exported (EXTERN)
  HLOG_BOOL   fTarget;                 // TRUE = target has been exported (EXTERN)
  HLOG_BOOL   fRevisionMarks;          // TRUE = target has been exported with revsion marks (EXTERN)
  HLOG_BOOL   fSNOMATCH;               // TRUE = SNOMATCH file created
  CHAR        szSourcePath[MAX_PATH144];// pPath for source document (EXTERN)
  CHAR        szTargetPath[MAX_PATH144];// path for target document (EXTERN)
  CHAR        szSNOMATCH[MAX_PATH144]; // path for SNOMATCH file (EXTERN)
} DOCEXPORTHIST, *PDOCEXPORTHIST;

//+----------------------------------------------------------------------------+
//|  Layout of additional info part for task ANALYSIS                          |
//+----------------------------------------------------------------------------+
typedef struct _ANALYSISHIST
{
  HLOG_BOOL   fAddToMem;               // TRUE = add segments to TM selected
  HLOG_BOOL   fAutoReplace;            // TRUE = substitute exact matches selected
  HLOG_BOOL   fLatestMatch;            // TRUE = use latest match selected
  HLOG_BOOL   fAutoJoin;               // TRUE = join segments selected
  CHAR        szMemory[MAX_FILESPEC];  // Translation Memory used for analysis
  CHAR        szMarkup[MAX_FILESPEC];  // Markup table used for analysis
} ANALYSISHIST, *PANALYSISHIST;


//+----------------------------------------------------------------------------+
//|  Layout of additional info part for task LONGNAME                          |
//+----------------------------------------------------------------------------+
#define MAX_LONGFILESPEC       256     // max length of long file name + extension
typedef struct _LONGNAMEHIST
{
  CHAR        szLongName[MAX_LONGFILESPEC];  // long document name
} LONGNAMEHIST, *PLONGNAMEHIST;

//+----------------------------------------------------------------------------+
//|  Function to add a new record to the history log file                      |
//|  (the function is located in EQFBFILE.C)                                   |
//+----------------------------------------------------------------------------+
__declspec(dllexport)
USHORT EQFBWriteHistLog
(
  PSZ      pszFolObjName,    // folder object name (e.g. "E:\\EQF\\SAMPLE1.F00")
  PSZ      pszDocName,       // name of document (e.g. "DEVICE.SCR")
  SHORT    TaskId,           // ID of task being logged
  USHORT   usAddInfoLength,  // length of additional info
  PVOID    pvAddInfo,        // ptr to data area containing additional info
  BOOL     fMsg,             // message handling flag, TRUE = show errors
  HWND     hwndErrMsg        // window handle to use for error messages
);

__declspec(dllexport)
USHORT EQFBWriteHistLog2
(
  PSZ      pszFolObjName,    // folder object name (e.g. "E:\\EQF\\SAMPLE1.F00")
  PSZ      pszDocName,       // short name of document (e.g. "DEVICE.SCR")
  SHORT    TaskId,           // ID of task being logged
  USHORT   usAddInfoLength,  // length of additional info
  PVOID    pvAddInfo,        // ptr to data area containing additional info
  BOOL     fMsg,             // message handling flag, TRUE = show errors
  HWND     hwndErrMsg,       // window handle to use for error messages
  PSZ      pszLongDocName    // document long name or NULL if none
);


// definitions for redundancy counting file

// name of redundancy count log file
#define REDUNDCOUNTFILE     "REDUND.LOG"

// header of redundancy count log file
typedef struct _REDCOUNTHEADER
{
  CHAR        szFolder[MAX_FNAME];     // name of folder containing documents
  LONG        lDocuments;              // number of documents in redundancy log
  LONG        lTime;                   // date/time of file creation
  COUNTSUMS   UniqueCount;
  LONG        lMustBeZero;             // identifier to recognize older headers
  LONG        lMostUsedOffset;         // offset of most used segments area
                                       // (relative to the begin of the file)
  LONG        lMostUsedSegments;       // number of segments in most used segments area
} REDCOUNTHEADER, *PREDCOUNTHEADER;

// record per document (followed by lDocuments COUNTSUMS records)
typedef struct _REDCOUNTDOC
{
  LONG        lSize;                   // size of document record (incl. COUNTSUMS array)
  CHAR        szDocName[MAX_LONGFILESPEC]; // document (long) name
  CHAR        szDocShortName[MAX_FILESPEC]; // document short name
  COUNTSUMS   TotalCount;              // total of segments/words in document
  COUNTSUMS   ExactCount;              // exact matches from selected TMs
} REDCOUNTDOC, *PREDCOUNTDOC;

// record layout for the document reference table
typedef struct _MOSTUSEDDOCANDSEG
{
  SHORT     sDocIndex;               // index of document
  ULONG     ulSegNo;                  // segment number within document
} MOSTUSEDDOCANDSEG, *PMOSTUSEDDOCANDSEG;

// record layout for a single segment in the most used segment area
typedef struct _MOSTUSEDSEG
{
  LONG   lFrequency;                   // frequency of segment
  CHAR_W   szSegment[MAX_SEGMENT_SIZE];  // segment data
  // array with document numbers and segment numbers
  MOSTUSEDDOCANDSEG aDocs[MAX_MOSTUSEDDOCSPERSEG];
} MOSTUSEDSEG, *PMOSTUSEDSEG;

// This is a helper structure for reading REDUND.LOG (V000096 quick fix)
typedef struct _MOSTUSEDSEGA {
  LONG lFrequency;
  CHAR szSegment[MAX_SEGMENT_SIZE];
  MOSTUSEDDOCANDSEG aDocs[MAX_MOSTUSEDDOCSPERSEG];
} MOSTUSEDSEGA, *PMOSTUSEDSEGA;

//+----------------------------------------------------------------------------+
//|  Layout of additional info part for task VERSION                           |
//+----------------------------------------------------------------------------+
typedef struct _VERSIONHIST
{
  BOOL        fWindows;                // TRUE = Windows, FALSE = OS/2
  LONG        lEqfstartDateTime;       // Date/Time of EQFSTART.EXE
  LONG        lEqfstartSize;           // Size of EQFSTART.EXE
  LONG        lEqfdllDateTime;         // Date/Time of EQFDLL.DLL
  LONG        lEqfdllSize;             // Size of EQFDLL.DLL
  LONG        lEqfresDateTime;         // Date/Time of EQFRES.DLL
  LONG        lEqfresSize;             // Size of EQFRES.DLL
  CHAR        szVersionString[20];     // Version string stored in EQFRES.DLL
  CHAR        szForFutureUse[20];      // for further information
} VERSIONHIST, *PVERSIONHIST;

//  Layout of additional info part for HISTLOGDELETE_LOGTASK                           
typedef struct _DELHISTLOG
{
  CHAR        szUser[80];              // User deleting the history log
  CHAR        szForFutureUse[20];      // for further information
} DELHISTLOG, *PDELHISTLOG;

// function to correct size values in older records
__declspec(dllexport) void HistLogCorrectRecSizes( PHISTLOGRECORD pRecord );

// union covering all types of the variable part of the histlog records
// union of variable parts of histlog record
typedef union _HISTLOGVARPART
{
  DOCIMPORTHIST  DocImport;
  DOCIMPORTHIST2 DocImport2;
  ANALYSISHIST   Analysis;
  DOCSAVEHIST    DocSave;
  DOCSAVEHIST2   DocSave2;
  DOCSAVEHIST3   DocSave3;
  DOCEXPORTHIST  DocExport;
  DOCPROPHIST    DocProp;
  FOLPROPHIST    FolProp;
  FOLPROPHIST2   FolProp2;
  FOLPROPHISTSHIPMENT   FolPropShipment; //SHIPMENT_HANDLER
  VERSIONHIST    VersionHist;
  BYTE           FillArea[12000];       // just to have enough free room in this union even for slightly larger variable parts ...
} HISTLOGVARPART, *PHISTLOGVARPART;

// max. size of history log records
// the required size is: sizeof HISTLOGRECORD + max. length of long names + max. size of variable part
#define MAXHISTLOGRECORDSIZE (sizeof(HISTLOGRECORD)+MAX_LONGFILESPEC+sizeof(HISTLOGVARPART))

__declspec(dllexport) USHORT FolReadLogRecord( HFILE hFile, PHISTLOGRECORD pRecord, PBOOL pfEOF, BOOL fMsg, HWND hwnd );
__declspec(dllexport) USHORT FolWriteLogRecord( HFILE hFile, PHISTLOGRECORD pRecord, BOOL fMsg, HWND hwnd );
