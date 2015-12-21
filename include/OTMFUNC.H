/**
* \file OTMFUNC.H
*
* \brief External header file for OpenTM2 API functions
*
*	Copyright Notice:
*
*	Copyright (C) 1990-2015, International Business Machines
*	Corporation and others. All rights reserved
**/

#ifndef _OTMFUNC_H_INCLUDED
#define _OTMFUNC_H_INCLUDED

// include EQFPAPI.H for segment definitions
#ifndef _EQFPAPI_H_INCLUDED
  #include "EQFPAPI.h"
#endif 

#ifdef __cplusplus
  extern "C" {
#endif

#ifndef _EQF_H_INCLUDED
  typedef long LONG;
  typedef unsigned short USHORT;
  typedef unsigned short *PUSHORT;
  typedef char * PSZ;
  typedef char CHAR;
  #define MAX_LANG_LENGTH           20   // length of the name of a language
  #define MAX_LONGFILESPEC         256   // max length of long file name + extension
  #define NUM_OF_FOLDER_DICS        10   // number of dictionaries per folder
  #define MAX_NUM_OF_READONLY_MDB   25   // folder's R/O memory dbs
  #define MAX_FILESPEC              13   // max length of file name + extension
#endif

typedef LONG HSESSION;
typedef HSESSION *PHSESSION;

typedef int BOOL;

typedef struct _REPORTTYPE
{
  PSZ  pszReport;
  LONG lRepType;
  PSZ  pszDescription;
} REPORTTYPE, *PREPORTTYPE;


typedef struct _REPORTSETTINGS
{
  PSZ  pszCountType;
  BOOL bShow;
  BOOL bSummary;
  PSZ  pszRepLayout;
  BOOL bShrink;
  PSZ  pszStatisticType;
  BOOL bExProposal;
} REPORTSETTINGS, *PREPORTSETTINGS;


typedef struct _FACTSHEET
{
  float lComplexity;
  float lPayFactor;
} FACTSHEET,*PFACTSHEET;


typedef struct _FINALFACTORS
{
  LONG  lUnit;
  float lCurrFactor;
  PSZ   pszLocalCurrency;
} FINALFACTORS,*PFINALFACTORS;


typedef struct _EXTFOLPROP
{
  CHAR  chDrive;
  CHAR  szTargetLang[MAX_LANG_LENGTH];
  CHAR  szRWMemory[MAX_LONGFILESPEC];
  CHAR  szROMemTbl[MAX_NUM_OF_READONLY_MDB][MAX_LONGFILESPEC];
  CHAR  szDicTbl[ NUM_OF_FOLDER_DICS][MAX_FILESPEC];
} EXTFOLPROP, *PEXTFOLPROP;
//
// option flags used by the function call interface
//

// location flags for TMs and dictionaries
#define LOCAL_OPT         0x00000001L
#define SHARED_OPT        0x00000002L

// analysis flags 
#define TMMATCH_OPT            0x00000010L
#define ADDTOMEM_OPT           0x00000020L
#define AUTOSUBST_OPT          0x00000040L
#define UNTRANSLATED_OPT       0x00000080L
#define AUTOLAST_OPT           0x00000100L
#define AUTOJOIN_OPT           0x00000200L
#define AUTOCONTEXT_OPT        0x00000400L
#define REDUNDCOUNT_OPT        0x00000800L
#define IGNOREPATH_OPT         0x00001000L
#define ADJUSTLEADWS_OPT       0x00002000L
#define ADJUSTTRAILWS_OPT      0x00004000L
#define RESPECTCRLF_OPT        0x00008000L
#define NOBLANKATSEGEND_OPT    0x00010000L
#define NOSUBSTIFIDENTICAL_OPT 0x00020000L
#define SENDTOMT_OPT           0x00080000L
#define STOPATFIRSTEXACT_OPT   0x00100000L
// ignore memory proposals which are flagged with a comment

#define IGNORECOMMENTED_OPT    0x00200000L

/** @brief protect text inside XMP sections, MsgNum sections, Meta sections, screen sections, and code block sections */
#define PROTECTXMPSCREEN_OPT   0x00040000L

/** @brief protect text inside XMP sections */
#define PROTECTXMP_OPT         0x00400000L
/** @brief protect text inside MsgNum sections */
#define PROTECTMSGNUM_OPT      0x00800000L
/** @brief protect text inside Meta sections */
#define PROTECTMETA_OPT        0x01000000L
/** @brief protect text inside screen sections */
#define PROTECTSCREEN_OPT      0x02000000L
/** @brief protect text inside CodeBlock sections */
#define PROTECTCODEBLOCK_OPT   0x04000000L


// folder export and folder import flags
#define WITHDICT_OPT           0x00001000L
#define WITHMEM_OPT            0x00002000L
#define DELETE_OPT             0x00004000L
#define WITHREADONLYMEM_OPT    0x00008000L
#define WITHDOCMEM_OPT         0x00010000L
#define MASTERFOLDER_OPT       0x00020000L
#define XLIFF_OPT              0x00040000L
#define NOMARKUP_UPDATE_OPT    0x00080000L

//
// document import specific options
//

// do not search in sub-directories for specified document names
#define NOSUBDIRSEARCH_OPT  0x00001000L

//
// flags for document export and wordcount
//

// Count replace matches separately option (can only be used in conjunction with TMMATCH_OPT)
#define SEPERATEREPLMATCH_OPT  0x02000000L

// create word count without duplicates (R009560)
#define DUPMEMMATCH_OPT   0x01000000L

// create word count without duplicates (R009560)
#define DUPLICATE_OPT     0x00010000L

// create source word count / export source document
#define SOURCE_OPT        0x00020000L

// create translated/untranslated word count / export target document
#define TARGET_OPT        0x00040000L

// export SNOMATCH document
#define SNOMATCH_OPT      0x00080000L

// count fuzzy matches (only EqfCountWords)
#define FUZZYMATCH_OPT         0x00080000L

//

// flags for dictionary import merge options
// (IGNORE_OPT also used in EqfImportMem to skip invalid segments)
//
#define IGNORE_OPT        0x00000020L
#define REPLACE_OPT       0x00000040L
#define COMBINE_OPT       0x00000080L

// flags for dictionary import/export in DXT UTF8 format
#define DXT_UTF8_OPT     0x00200000L

// external/internal flags
#define EXTERNAL_OPT      0x01000000L
#define INTERNAL_OPT      0x02000000L

// flags for build archive TM function EqfArchiveTM
#define USEASFOLDERTM_OPT   0x20000000L
#define SOURCESOURCEMEM_OPT 0x40000000L
#define SETMFLAG_OPT        0x80000000L

// general flags
#define OVERWRITE_OPT     0x10000000L

// characterset flags for dictionary/memory import and export
#define ASCII_OPT        0x00020000L
#define ANSI_OPT         0x00040000L
#define UTF16_OPT        0x00080000L

// import in TMX format (UTF-16 and UTF-8)
#define TMX_OPT          0x00100000L

// export in TMX format
#define TMX_UTF16_OPT    0x00100000L
#define TMX_UTF8_OPT     0x00200000L

// clean/remove RTF tags
#define CLEANRTF_OPT     0x00400000L

// restrict RTF tag removal to tags enclosed in curly braces only
#define INCURLYBRACE_OPT   0x00800000L

// remove linebreaks in text of translation units
#define TMX_NOCRLF_OPT     0x01000000L

// import in XLIFF machine translation (MT) format
#define XLIFF_MT_OPT       0x02000000L

// how to handle unknown markup table when importing memory
#define CANCEL_UNKNOWN_MARKUP_OPT      0x10000000L
#define SKIP_UNKNOWN_MARKUP_OPT        0x20000000L
#define GENERIC_UNKNOWN_MARKUP_OPT     0x40000000L


// flags for document export in validation format

// export documents in validation format (cannot be used together with the other export options)
// export validation document in XML format
#define VALFORMAT_XML_OPT    0x00200000L  
// export validation document in HML format
#define VALFORMAT_HTML_OPT   0x00400000L  
//export validation document in DOC format
#define VALFORMAT_DOC_OPT    0x00800000L  
//export validation document in ODT format
#define VALFORMAT_ODT_OPT    0x00100000L  

// flags which can be used together with the export options above

// combine documents into a single file 
#define VALFORMAT_COMBINE_OPT  0x01000000L  
// include protected segments in the validation document
#define VALFORMAT_PROTSEGS_OPT 0x02000000L  

// export documents as plain XML
#define PLAINXML_OPT           0x04000000L

// relative path document export options
#define WITHRELATIVEPATH_OPT      0x20000000L
#define WITHOUTRELATIVEPATH_OPT   0x40000000L

// document export/import in internal format
#define OPENTM2FORMAT_OPT 0x80000000L

// include TVT tracking ID document export option
#define WITHTRACKID_OPT 0x08000000L

//
// output type options for EqfCountWords and EqfCreateCountReport
//
#define TEXT_OUTPUT_OPT   0x00100000L  
#define XML_OUTPUT_OPT    0x00200000L  
#define HTML_OUTPUT_OPT   0x00400000L  

// report IDs for function EqfCreateCountReport
#define HISTORY_REP             1
#define COUNTING_REP            2
#define CALCULATING_REP         3
#define PREANALYSIS_REP         4
#define REDUNDANCY_REP          5
#define REDUNDANCYSEGMENT_REP   6

// report types for function EqfCreateCountReport

// types for HISTORY_REPORT
#define BRIEF_SORTBYDATE_REPTYPE  1
#define BRIEF_SORTBYDOC_REPTYPE   2
#define DETAIL_REPTYPE            3

// types for COUNTING_REPORT
#define WITHTOTALS_REPTYPE        4
#define WITHOUTTOTALS_REPTYPE     5

// types for CALCULATING_REPORT, PREANALYSIS_REPORT, REDUNDANCY_REPORT,
//           and REDUNDANCYSEGMENT_REPORT
#define BASE_REPTYPE                    6
#define BASE_SUMMARY_REPTYPE            7
#define BASE_SUMMARY_FACTSHEET_REPTYPE  8
#define SUMMARY_FACTSHEET_REPTYPE       9
#define FACTSHEET_REPTYPE              10


//options for EqfCreateITM lType
#define NOANA_TYP           0x1
#define NOTM_TYP            0x2
#define PREPARE_TYP         0x4

//options for EqfChangeMFlag lAction
#define CLEAR_MMOPT         0
#define SET_MMOPT           1

// options for non-dde interface (calculating report)
#define BASE_TYP        1
#define FACT_TYP        2
#define SUM_TYP         4

#define PLAUS_OPT   1             // (Plausibilty check)
#define LOST_OPT    2             // (Lost Data: Force new shipment)
#define LIST_OPT    4             // (List of Documents)


// return when all activitiy has been completed (instead of iterative processing)
#define COMPLETE_IN_ONE_CALL_OPT 0x01000000

// non-DDE function interface specific return codes

// Info: return code for functions not implemented yet
#define NOTIMPLEMENTED_RC               10001

// Error: passed session handle is invalid
#define ERROR_INVALID_SESSION_HANDLE    10002

// Notification: current function has not been completed yet
//               re-call function until a non-CONTINUE_RC
//               return code is retuned
#define CONTINUE_RC                     10003

// Error: Wrong or conflicting options specified
#define WRONG_OPTIONS_RC                10004

// Error: different function has been called although
//        current task has not been completed yet
#define LASTTASK_INCOMPLETE_RC          10005

// Error: Segment has been joined to a previous one and is not
//        part of the visible document. It cannot be used
//        in function EqfGetSourceLine
#define SEGMENTISJOINED_RC               10006

// Error: Given segment number is invalid
//        (probably not contained in document)
#define INVALIDSEGMENT_RC                10007

// Error: Given handle of loaded file is invalid
#define INVALIDFILEHANDLE_RC             10008

// Error: There is no segment matching the given position
#define NOMATCHINGSEGMENT_RC             10009


/*! \brief Create a new folder
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName pointer to the buffer containing the folder name
	\param pszDescription pointer to the buffer containing the folder description or NULL if not used
	\param chTargetDrive the drive letter of the drive the folder should reside on, if left empty the folder is created on the OpenTM2 installation drive
	\param pszMemname pointer to the buffer containing the folder Translation Memory
	\param pszMarkup pointer to the buffer containing the name of the folder markup (e.g. IBMDITA)
	\param pszEditor pointer to the buffer containing the name of the editor to be used for documents of this folder
	\param pszDictionaries pointer to the buffer containing a comma separated list of dictionaries or NULL if no dictionaries should be used
	\param pszSourceLanguage pointer to the buffer containing the folder source language (OpenTM2 language name)
	\param pszTargetLanguage pointer to the buffer containing the folder target language (OpenTM2 language name)
	\param pszConversion pointer to the buffer containing the export conversion - currently not in use and should be NULL 
	\param pszReadOnlyMems pointer to the buffer containing the a comma separated list of readonly Translation Memory databases or NULL if not used
	\returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/  
__declspec(dllexport)
USHORT EqfCreateFolder
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  PSZ         pszDescription,          // opt: folder description or NULL
  CHAR        chTargetDrive,           // opt: folder target drive
  PSZ         pszMemname,              // mand: folder Translation Memory
  PSZ         pszMarkup,               // mand: folder markup
  PSZ         pszEditor,               // opt: folder editor
  PSZ         pszDictionaries,         // opt: list of dictionaries or NULL
  PSZ         pszSourceLanguage,       // mand: folder source language
  PSZ         pszTargetLanguage,       // mand: folder target language
  PSZ         pszConversion,           // opt: export conversion or NULL
  PSZ         pszReadOnlyMems          // opt: list of readonly TMs or NULL
);


// create a new controlled folder
/*! \brief Create a new cntrolled/protected folder
    A controlled folder has some protected properties 
	\param hSession the session handle returned by the EqfStartSession call
	\param pszFolderName pointer to the buffer containing the folder name
	\param pszDescription pointer to the buffer containing the folder description or NULL if not used
	\param chTargetDrive the drive letter of the drive the folder should reside on, if left empty the folder is created on the OpenTM2 installation drive
	\param pszMemname pointer to the buffer containing the folder Translation Memory
	\param pszMarkup pointer to the buffer containing the name of the folder markup (e.g. IBMDITA)
	\param pszEditor pointer to the buffer containing the name of the editor to be used for documents of this folder
	\param pszDictionaries pointer to the buffer containing a comma separated list of dictionaries or NULL if no dictionaries should be used
	\param pszSourceLanguage pointer to the buffer containing the folder source language (OpenTM2 language name)
	\param pszTargetLanguage pointer to the buffer containing the folder target language (OpenTM2 language name)
	\param pszConversion pointer to the buffer containing the export conversion - currently not in use and should be NULL
	\param pszReadOnlyMems pointer to the buffer containing the a comma separated list of read only Translation Memory databases or NULL if not used
	\param pszPassword pointer to the buffer containing the password to be used for the folder
	\param pszProjCoordName pointer to the buffer containing the name of the project coordinator or NULL if not used
	\param pszProjCoordMail pointer to the buffer containing the project coordinator's mail or NULL if not used
	\param pszTranslatorName pointer to the buffer containing the name of the translator
	\param pszTranslatorMail pointer to the buffer containing the translator's mail
	\param pszProductName pointer to the buffer containing the name of the product
	\param pszProductFamily pointer to the buffer containing the product family
	\param pszSimilarProduct pointer to the buffer containing the similar product family
	\param pszProductDict pointer to the buffer containing the product subject area dictionary
	\param pszProductMem pointer to the buffer containing the product subject area memory
	\param pszPreviousVersion pointer to the buffer containing the previous version of the product
	\param pszVersion pointer to the buffer containing the Version of the product
	\param pszShipmentNumber pointer to the buffer containing the shipment number
	\returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/  
__declspec(dllexport)
USHORT EqfCreateControlledFolder
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  PSZ         pszDescription,          // opt: folder description or NULL
  CHAR        chTargetDrive,           // opt: folder target drive
  PSZ         pszMemname,              // mand: folder Translation Memory
  PSZ         pszMarkup,               // mand: folder markup
  PSZ         pszEditor,               // opt: folder editor
  PSZ         pszDictionaries,         // opt: list of dictionaries or NULL
  PSZ         pszSourceLanguage,       // mand: folder source language
  PSZ         pszTargetLanguage,       // mand: folder target language
  PSZ         pszConversion,           // opt: export conversion or NULL
  PSZ         pszReadOnlyMems,         // opt: list of readonly TMs or NULL
  PSZ         pszPassword,             // mand: password
  PSZ         pszProjCoordName,        // opt: name of the project coordinator
  PSZ         pszProjCoordMail,        // opt: project coordinator's mail
  PSZ         pszTranslatorName,       // opt: name of the translator
  PSZ         pszTranslatorMail,       // opt: translator's mail
  PSZ         pszProductName,          // opt: Name of the product
  PSZ         pszProductFamily,        // opt: Product Famiily
  PSZ         pszSimilarProduct,       // opt: Similar Product Family
  PSZ         pszProductDict,          // opt: Product subject area dictionary
  PSZ         pszProductMem,           // opt: Product subject area memory
  PSZ         pszPreviousVersion,      // opt: Previous version of the product
  PSZ         pszVersion,              // opt: Version of the Product
  PSZ         pszShipmentNumber        // opt: Shipment number
 );
// change Folder Properties
/*! \brief Change the properties of a folder
	\param hSession the session handle returned by the EqfStartSession call
	\param pszFolderName pointer to the buffer containing the folder name
	\param chTargetDrive the drive letter of the drive the folder resides on (is not required anymore and should be left empty
	\param pszTargetLanguage pointer to the buffer containing the new folder target language (OpenTM2 language name) or NULL when the target language should not be changed
	\param pszMemName pointer to the buffer containing the name of the new folder Translation Memory or NULL when the target language should not be changed
	\param pszDictionaries pointer to the buffer containing the comma separated new list of dictionaries or NULL if dictionaries should not be changed
	\returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfChangeFolProps
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  CHAR        chTargetDrive,           // opt: target drive
  PSZ         pszTargetLanguage,       // opt: target language or NULL
  PSZ         pszMemName,              // opt: folder Translation Memory or NULL
  PSZ         pszDictionaries          // opt: list of dictionaries or NULL
  );
/*! \brief Change the properties of a folder (extended version)
  \param hSession the session handle returned by the EqfStartSession call
  \param pszFolderName pointer to the buffer containing the folder name
  \param chTargetDrive the drive letter of the drive the folder resides on (is not required anymore and should be left empty
  \param pszTargetLanguage pointer to the buffer containing the new folder target language (OpenTM2 language name) or NULL for no change
  \param pszMemName pointer to the buffer containing the name of the new folder Translation Memory or NULL for no change
  \param pszDictionaries pointer to the buffer containing the comma separated new list of dictionaries or NULL for no change
  \param pszROMemories pointer to the buffer containing the new list of read-only Translation Memory databases or NULL for no change
  \param pszDescription pointer to the buffer containing the new folder description or NULL for no change
  \param pszProfile pointer to the buffer containing the new calculation profile name or NULL for no change
  \param pszUnUsed2 parameter currently not in use and reserved for future enhancements, should be NULL
  \returns 0 if successful or an error code in case of failures
  */
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfChangeFolPropsEx
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  CHAR        chTargetDrive,           // opt: target drive
  PSZ         pszTargetLanguage,       // opt: target language or NULL
  PSZ         pszMemName,              // opt: folder Translation Memory or NULL
  PSZ         pszDictionaries,         // opt: list of dictionaries or NULL
  PSZ         pszROMemories,           // opt: list of read-only search memories or NULL
  PSZ         pszDescription,          // opt: folder description or NULL
  PSZ         pszProfile,              // opt: calculation profile name or NULL
  PSZ         pszUnUsed2               // opt: for future enhancements, currently NULL
  );
/*! \brief Create a sub-folder
	\param hSession the session handle returned by the EqfStartSession call
	\param pszParentFolder pointer to the buffer containing the name of the parent folder
	\param pszSubFolderName pointer to the buffer containing the name for the new sub-folder
	\param pszMemname pointer to the buffer containing the folder Translation Memory
	\param pszMarkup pointer to the buffer containing the name of the folder markup (e.g. IBMDITA)
	\param pszSourceLanguage pointer to the buffer containing the sub-folder source language (OpenTM2 language name)
	\param pszTargetLanguage pointer to the buffer containing the sub-folder target language (OpenTM2 language name)
	\param pszEditor pointer to the buffer containing the name of the editor to be used for documents of this sub-folder
	\param pszConversion pointer to the buffer containing the export conversion - currently not in use and should be NULL
	\param pszTranslatorName pointer to the buffer containing the name of the translator
	\param pszTranslatorMail pointer to the buffer containing the translator's mail
	\returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfCreateSubFolder
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszParentFolder,         // mand: name of parent (sub)folder
  PSZ         pszSubFolderName,        // mand: subfolders name
  PSZ         pszMemName,              // opt: subfolders Translation Memory or NULL
  PSZ         pszMarkup,               // opt: name of Markup used for subfolder
  PSZ         pszSourceLanguage,       // opt: Source Language used for subfolder or NULL
  PSZ         pszTargetLanguage,       // opt: Target Language used for subfolder or NULL
  PSZ         pszEditor,               // opt: editor used by sub folder  or NULL
  PSZ         pszConversion,           // opt: Conversion used for subfolder or NULL
  PSZ         pszTranslator,           // opt: Name of translator or NULL
  PSZ         pszTranslatorMail        // opt: Mail of translator or NULL
);


/*! \brief Export a folder to the \OTM\Export directory of the specified drive
  \param hSession the session handle returned by the EqfStartSession call
  \param pszFolderName pointer to the buffer containing the name of folder being exported
  \param chTargetDrive folder export target drive
  \param lOptions options for the folder export or 0L
    - DELETE_OPT delete the folder after successful export
    - WITHDICT_OPT add the dictionaries used by the folder
    - WITHMEM_OPT include the folder Translation Memory
    - WITHREADONLYMEM_OPT include the read-only Translation Memory databases
    - WITHDOCMEM_OPT include the Translation Memory databases specified in the document properties
    - OVERWRITE_OPT overwrite any existing exported folder
    - MASTERFOLDER_OPT export as master folder
    - XLIFF_OPT export as XLIFF folder
  \param pszDocuments pointer to the buffer containing the list of documents or NULL to export all documents of the folder
  \param pszDescription pointer to the buffer containing the export description or NULL if not used
  \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfExportFolder
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  CHAR        chTargetDrive,           // mand: folder target drive
  LONG        lOptions,                // opt: options for the folder export or 0L
                                       // @Contain Memory: {WITHMEM_OPT,WITHREADONLYMEM_OPT,WITHDOCMEM_OPT}
									   // @Other: {DELETE_OPT,WITHDICT_OPT,MASTERFOLDER_OPT, XLIFF_OPT, OVERWRITE_OPT}
  PSZ         pszDocuments,            // opt: list of documents or NULL
  PSZ         pszDescription           // opt: export description or NULL
);

/*! \brief Export a folder to the specified directory
  \param hSession the session handle returned by the EqfStartSession call
  \param pszFolderName pointer to the buffer containing the name of folder being exported
  \param pszTargetPath pointer to the buffer containing target directory for the folder export
  \param lOptions options for the folder export or 0L
    - DELETE_OPT delete the folder after successful export
    - WITHDICT_OPT add the dictionaries used by the folder
    - WITHMEM_OPT include the folder Translation Memory
    - WITHREADONLYMEM_OPT include the read-only Translation Memory databases
    - WITHDOCMEM_OPT include the Translation Memory databases specified in the document properties
    - OVERWRITE_OPT overwrite any existing exported folder
    - MASTERFOLDER_OPT export as master folder
    - XLIFF_OPT export as XLIFF folder
  \param pszDocuments pointer to the buffer containing the list of documents or NULL to export all documents of the folder
  \param pszDescription pointer to the buffer containing the export description or NULL if not used
  \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfExportFolderFP
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  PSZ         pszTargetPath,           // mand: folder target path
  LONG        lOptions,                // opt: options for the folder export or 0L
                                       // @Contain Memory: {WITHMEM_OPT,WITHREADONLYMEM_OPT,WITHDOCMEM_OPT}
									   // @Other: {DELETE_OPT,WITHDICT_OPT,MASTERFOLDER_OPT, XLIFF_OPT, OVERWRITE_OPT}
  PSZ         pszDocuments,            // opt: list of documents or NULL
  PSZ         pszDescription           // opt: export description or NULL          
);

/*! \brief Export a folder to the specified directory using a different folder name
  \param hSession the session handle returned by the EqfStartSession call
  \param pszFolderName pointer to the buffer containing the name of folder being exported
  \param pszTargetPath pointer to the buffer containing target directory for the folder export
  \param pszExportAs pointer to the buffer containing name to be use for the exported folder or NULL if not used
  \param lOptions options for the folder export or 0L
    - DELETE_OPT delete the folder after successful export
    - WITHDICT_OPT add the dictionaries used by the folder
    - WITHMEM_OPT include the folder Translation Memory
    - WITHREADONLYMEM_OPT include the read-only Translation Memory databases
    - WITHDOCMEM_OPT include the Translation Memory databases specified in the document properties
    - OVERWRITE_OPT overwrite any existing exported folder
    - MASTERFOLDER_OPT export as master folder
    - XLIFF_OPT export as XLIFF folder
  \param pszDocuments pointer to the buffer containing the list of documents or NULL to export all documents of the folder
  \param pszDescription pointer to the buffer containing the export description or NULL if not used
  \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfExportFolderFPas
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  PSZ         pszTargetPath,           // mand: folder target path
  PSZ         pszExportAs,             // opt: Export as - optional new name for exported folder
  LONG        lOptions,                // opt: options for the folder export or 0L
                                       // @Contain Memory: {WITHMEM_OPT,WITHREADONLYMEM_OPT,WITHDOCMEM_OPT}
									   // @Other: {DELETE_OPT,WITHDICT_OPT,MASTERFOLDER_OPT, XLIFF_OPT, OVERWRITE_OPT}
  PSZ         pszDocuments,            // opt: list of documents or NULL
  PSZ         pszDescription,          // opt: export description or NULL
  PSZ         pszMemoryExportAs        // opt: export name for memory in folder
);

/**
 \brief Count the number of words in documents

 This API call counts the words of the documents in a folder or of a subset of
 the documents in a folder.
 The word count report can be a source word count (SOURCE_OPT), a target word count (TARGET_OPT), a 
 duplicate word count (DUPLICATE_OPT), or a memory match word count (TMMATCH_OPT)
 The format of the word count report can be either text format (TEXT_OUTPUT_OPT),
 HTML format (HTML_OUTPUT_OPT), or XML format (XML_OUTPUT_OPT). 

 \param hSession OpenTM2 session handle returned by EqfStartSession
 \param pszFolderName the name of the folder containing the documents being counted
 \param pszDocuments list of documents or NULL if all documents of the folder are to be counted
 \param lOptions options for the word counting
    - TMMATCH_OPT create a memory match word count
    - DUPLICATE_OPT create a duplicates word count
    - SOURCE_OPT create a source word count
    - TARGET_OPT create a target word count
    - FUZZYMATCH_OPT include fuzzy match information in memory match word count 
    - SEPERATEREPLMATCH_OPT count replace matches separately in memory match word count
    - DUPMEMMATCH_OPT include memory match information in duplicates word count 
    - TEXT_OUTPUT_OPT create a plain text word count
    - XML_OUTPUT_OPT create a word count in XML format
    - HTML_OUTPUT_OPT create a word count in HTML format
 \param pszOutFile fully qualified name of the file receiving the word count output
	\returns 0 if successful or an error code
	
**/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfCountWords
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  PSZ         pszDocuments,            // opt: list of documents or NULL
  LONG        lOptions,                // opt: options for the word count
                                       // @Wordount:      TMMATCH_OPT{SEPERATEREPLMATCH_OPT},
									   //                 SOURCE_OPT, TARGET_OPT(default), DUPLICATE_OPT, DUPMEMMATCH_OPT 
                                       // @Output Format: XML_OUTPUT_OPT, HTML_OUTPUT_OPT, TEXT_OUTPUT_OPT(default)
									   // @Other:         {SEPERATEREPLMATCH_OPT,OVERWRITE_OPT}
  PSZ         pszOutFile               // mand: fully qualified output file
);

/*! \brief Import a dictionary
  \param hSession the session handle returned by the EqfStartSession call
  \param pszInFile fully qualified name of input file
  \param pszDictName name of the dictionary
  \param pszPassword password of the dictionary or NULL if not used
  \param lOptions dictionary import options
    one of the format options
    - ASCII_OPT import from SGML (ASCII encoded)
    - ANSI_OPT import from SGML (ANSI encoded)
    - UTF16_OPT import from SGML (Unicode UTF-16 encoded)
    - DXT_UTF8_OPT import from DXT XML (UTF-8 encoded)
    .
    and one of the handling options
    - COMBINE_OPT combine existing entries and imported entries
    - REPLACE_OPT replace existing entries with the and imported entries
    - IGNORE_OPT ignore imported entries when the entry already exists
  \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfImportDict
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszInFile,               // mand: fully qualified name of input file
  PSZ         pszDictName,             // mand: name of dictionary
  PSZ         pszPassword,             // opt: password of dictionary
  LONG        lOptions                 // opt: dictionary import options
                                       // @Import Mode: ASCII_OPT,ANSI_OPT,DXT_UTF8_OPT,UTF16_OPT(default)
									   // @Merge Mode:  IGNORE_OPT, REPLACE_OPT, COMBINE_OPT
);
/*! \brief Export a dictionary
  \param hSession the session handle returned by the EqfStartSession call
  \param pszDictName name of dictionary
  \param lOptions dictionary export options or 0L
    - OVERWRITE_OPT to overwrite existing files
    .
    and one of the format options
    - ASCII_OPT import from SGML (ASCII encoded)
    - ANSI_OPT import from SGML (ANSI encoded)
    - UTF16_OPT import from SGML (Unicode UTF-16 encoded)
    - DXT_UTF8_OPT import from DXT XML  (UTF-8 encoded)
  \param pszOutFile fully qualified name of output file
  \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfExportDict
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszDictName,             // mand: name of dictionary
  LONG        lOptions,                // opt: dictionary export options or 0L
                                       // @Export Mode: ASCII_OPT, ANSI_OPT, DXT_UTF8_OPT, UTF16_OPT(default)
                                       // @Other: OVERWRITE_OPT
  PSZ         pszOutFile               // mand: fully qualified name of output file
);
/*! \brief Import a document
  \param hSession the session handle returned by the EqfStartSession call
  \param pszFolderName name of folder receiving the documents
  \param pszFiles list of input files (documents) in form of a comma separated list enclosed in parentheses
  \param pszMemname document Translation Memory or NULL
  \param pszMarkup document markup or NULL
  \param pszEditor document editor or NULL
  \param pszSourceLanguage document source language or NULL
  \param pszTargetLanguage document target language or NULL
  \param pszAlias alias for document name or NULL
  \param pszStartPath optional start path
  \param pszConversion document conversion - not used anymore ans should be NULL
  \param lOptions document import options or 0
         - OVERWRITE_OPT to overwrite any existing document with the imported one 
  \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfImportDoc
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder receiving the documents
  PSZ         pszFiles,                // mand: list of input files (documents)
  PSZ         pszMemname,              // opt: document Translation Memory or NULL
  PSZ         pszMarkup,               // opt: document markup or NULL
  PSZ         pszEditor,               // opt: document editor or NULL
  PSZ         pszSourceLanguage,       // opt: document source language or NULL
  PSZ         pszTargetLanguage,       // opt: document target language or NULL
  PSZ         pszAlias,                // opt: alias for document name or NULL
  PSZ         pszStartPath,            // opt: optional start path
  PSZ         pszConversion,           // opt: optional document conversion
  LONG        lOptions                 // opt: document import options or 0L
                                       // @Option: OVERWRITE_OPT
);


/*! \brief Export a document
  \param hSession the session handle returned by the EqfStartSession call
  \param pszFolderName name of folder receiving the documents
  \param pszFiles list of input files (documents) in form of a comma separated list enclosed in parentheses
  \param pszStartPath optional start path
  \param lOptions document import options or 0
         - OVERWRITE_OPT to overwrite any existing file
         .
         and one of the export format options
         - SOURCE_OPT export the document source file
         - TARGET_OPT export the document target (=translated) file
         - SNOMATCH_OPT export the list of segments with no matching memory proposal
         - VALFORMAT_ODT_OPT export as validation document (ODT format)
         - VALFORMAT_XML_OPT export as validation document (XML format)
         - VALFORMAT_HTML_OPT export as validation document (HTML format)
         - VALFORMAT_DOC_OPT export as validation document (MS DOC format)
         - PLAINXML_OPT export document as plain XML file
         - OPENTM2FORMAT_OPT export the document in the internal OpenTM2 format
         .
         for the validation formats the following two additional options can specified
         - VALFORMAT_COMBINE_OPT combine all documents into a single one
         - VALFORMAT_PROTSEGS_OPT include protected segments in the exported document
         .
         for the SOURCE_OPT, TARGET_OPT, and SNOMATCH_OPT export formats the following relative path options can be used
         - WITHRELATIVEPATH_OPT export the documents with their relative path (this is the default)
         - WITHOUTRELATIVEPATH_OPT export the documents without their relative path
         for the TARGET_OPT export format the following option can be used
         - WITHTRACKID_OPT export the documents with TVT tracking info
  \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfExportDoc
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  PSZ         pszFiles,                // mand: list of documents with path information
  PSZ         pszStartPath,            // opt: optional start path
  LONG        lOptions                 // opt: options for document export
                                       // @Export Format: SOURCE_OPT,TARGET_OPT(default),SNOMATCH_OPT, PLAINXML_OPT, OPENTM2FORMAT_OPT,
									   //                 VALFORMAT_XML_OPT{VALFORMAT_COMBINE_OPT  VALFORMAT_PROTSEGS_OPT}, 
									   //                 VALFORMAT_DOC_OPT{VALFORMAT_COMBINE_OPT  VALFORMAT_PROTSEGS_OPT},
									   //                 VALFORMAT_ODT_OPT{VALFORMAT_COMBINE_OPT  VALFORMAT_PROTSEGS_OPT},
									   //                 VALFORMAT_HTML_OPT{VALFORMAT_COMBINE_OPT VALFORMAT_PROTSEGS_OPT},
                                       //                 WITHTRACKID_OPT
                                       // @RelativePath: WITHRELATIVEPATH_OPT(default),WITHOUTRELATIVEPATH_OPT
									   // @Other: {VALFORMAT_COMBINE_OPT,VALFORMAT_PROTSEGS_OPT,OVERWRITE_OPT}
);     


/*! \brief Import a Translation Memory
  \param hSession the session handle returned by the EqfStartSession call
  \param pszMemname name of the Translation Memory
  \param pszInFile fully qualified name of the input file
  \param lOptions options for Translation Memory import
         one of the format options 
         - TMX_OPT import in TMX format
         - XLIFF_MT_OPT import in XLIFF format
         - UTF16_OPT import in the EXP format (Unicode UTF-16 encoded)
         - ANSI_OPT import in the EXP format (ANSI encoded)
         - ASCII_OPT import in the EXP format (ASCII encoded)
         .
         and one of the markup table handling functions
         - CANCEL_UNKNOWN_MARKUP_OPT stop import when an unknown markup is detected
         - SKIP_UNKNOWN_MARKUP_OPT skip segments with unknown markup
         - GENERIC_UNKNOWN_MARKUP_OPT use the default markup table for segments with unknown markup
         .
         additional options which can be used
         - CLEANRTF_OPT to remove RTF inline tags (only in combination with TMX_OPT)
         - IGNORE_OPT ignore invalid segments and continue the import 
  \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfImportMem
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszMemname,              // mand: name of Translation Memory
  PSZ         pszInFile,               // mand: fully qualified name of input file
  LONG        lOptions                 // opt: options for Translation Memory import
                                       // @Import Mode: TMX_OPT{CLEANRTF_OPT}, XLIFF_MT_OPT,UTF16_OPT,ANSI_OPT,ASCII_OPT(default)
                                       // @Markup Table Handling: CANCEL_UNKNOWN_MARKUP_OPT(default), SKIP_UNKNOWN_MARKUP_OPT, GENERIC_UNKNOWN_MARKUP_OPT
									                     // @Other: {CLEANRTF_OPT,IGNORE_OPT}
);
/*! \brief Export a Translation Memory
  \param hSession the session handle returned by the EqfStartSession call
  \param pszMemname name of the Translation Memory
  \param pszOutFile fully qualified name of the output file
  \param lOptions options for Translation Memory export
         - OVERWRITE_OPT to overwrite existing files
         .
         and one of the format options 
         - TMX_OPT import in TMX format
         - XLIFF_MT_OPT import in XLIFF format
         - UTF16_OPT export in the EXP format (Unicode UTF-16 encoded)
         - ANSI_OPT export in the EXP format (ANSI encoded)
         - ASCII_OPT export in the EXP format (ASCII encoded)
         .
         for the TMX_UTF8_OPT and TMX_UTF16_OPT also the following option can be specified
         - TMX_NOCRLF_OPT to remove line breaks within the segment data
\returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfExportMem
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszMemname,              // mand: name of Translation Memory
  PSZ         pszOutFile,              // mand: fully qualified name of output file
  LONG        lOptions                 // opt: options for Translation Memory export
                                       // @Export Mode: UTF16_OPT,ANSI_OPT,TMX_UTF8_OPT{TMX_NOCRLF_OPT},TMX_UTF16_OPT{TMX_NOCRLF_OPT},ASCII_OPT(default)
									   // @Other: OVERWRITE_OPT
);

/*! \brief Organize a Translation Memory
  \param hSession the session handle returned by the EqfStartSession call
  \param pszMemname name of the Translation Memory
  \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfOrganizeMem
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszMemname               // mand: name of Translation Memory
);

/*! \brief Create a new Translation Memory
  \param hSession the session handle returned by the EqfStartSession call
  \param pszMemname name of the Translation Memory
  \param pszDescription description for new Translation Memory or NULL
  \param chToDrive target drive for new Translation Memory
  \param pszSourceLanguage Translation Memory source language
  \param lOptions type of new Translation Memory
         - LOCAL_TM_OPT create a local memory using the EqfMemoryPlugin
\returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfCreateMem
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszMemname,              // mand: name of new Translation Memory
  PSZ         pszDescription,          // opt: description for new Translation Memory or NULL
  CHAR        chToDrive,               // opt: target drive for new Translation Memory
  PSZ         pszSourceLanguage,       // mand: Translation Memory source language
  LONG        lOptions                 // opt: type of new Translation Memory
                                       // @Location:SHARED_OPT,LOCAL_OPT(default)
);
/*! \brief Create a Initial Translation Memory
    \param hSession the session handle returned by the EqfStartSession call
    \param pszMemName name of the Translation Memory
    \param pszFilePairs list of file pairs as a comma separated list enclosed in parentheses
    \param pszMarkup markup table to be used
    \param pszSGMLMemFile external memory name or NUULL
    \param pszSourceLanguage Translation Memory source language
    \param pszTargetLanguage Translation Memory target language
    \param pszSourceStartPath startpath not stored for source files or NULL
    \param pszTargetStartPath startpath not stored for target files or NULL
    \param lType type of creation
           - NOANA_TYP no analysis is necessary
           - NOTM_TYP no translation memory database
           - PREPARE_TYP only prepare the alignment
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfCreateITM
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszMemName,              // mand: name of new Translation Memory
  PSZ         pszFilePairs,            // opt: list of filepairs or NULL
  PSZ         pszMarkup,               // mand: use markup table
  PSZ         pszSGMLMemFile,          // opt: external memory name or NULL
  PSZ         pszSourceLanguage,       // mand: Translation Memory source language
  PSZ         pszTargetLanguage,       // mand: Translation Memory target language
  PSZ         pszSourceStartPath,      // opt: Startpath not stored for source files or NULL
  PSZ         pszTargetStartPath,      // opt: Startpath not stored for target files or NULL
  LONG        lType                    // opt: type of creation
                                       // @Type:{NOANA_TYP,NOTM_TYP,PREPARE_TYP}
);
// change the machine translation flags
/*! \brief Create a new folder
    \param hSession the session handle returned by the EqfStartSession call
    \param pszMemName name of the Translation Memory
    \param lAction type of action
    - CLEAR_MMOPT clear the machine translation flag
    - SET_MMOPT set the machine translation flag
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfChangeMFlag
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszMemName,              // mand: name of new Translation Memory
  LONG        lAction                  // mand: type of action
                                       // @Action:CLEAR_MMOPT,SET_MMOPT
);

/*! \brief Analyze one or more documents
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of folder
    \param pszDocuments comma separated list with document names or NULL to analyze all documents of the folder
    \param pszMemname Translation Memory to be used for the analysis
    \param lOptions analysis options
           - TMMATCH_OPT prepare memory match word count
           - ADDTOMEM_OPT write found proposals to folder/document Translation Memory
           - AUTOSUBST_OPT auto-substitute memory matches
           - UNTRANSLATED_OPT write segments without proposals to an external memory (EXP)
           - AUTOLAST_OPT when more exaxct proposals are available use newest proposal for auto-substitution
           - AUTOJOIN_OPT auto-join segments
           - AUTOCONTEXT_OPT when more exact proposals are available use proposal from same document for auto-substitution
           - REDUNDCOUNT_OPT perform redundancy counting
           - IGNOREPATH_OPT ignore path of document names when looking for exact matches
           - ADJUSTLEADWS_OPT adjust leading white space of auto-substituted segments 
           - ADJUSTTRAILWS_OPT adjust trailing white space of auto-substituted segments 
           - RESPECTCRLF_OPT do not auto-substitute segments when number of line breaks are different
           - NOBLANKATSEGEND_OPT remove blank following a period at the end of the segment
           - NOSUBSTIFIDENTICAL_OPT do not auto-substitute segments when source and target of the proposal is identical
           - PROTECTXMPSCREEN_OPT protect xmp, screen, msgnum, meta, and codeblock sections
           - SENDTOMT_OPT send to / prepare data for MT systems
           - STOPATFIRSTEXACT_OPT stop searching Translation Memory databases at first exact match found
           - IGNORECOMMENTED_OPT ignore memory proposals with comments in the additional info area
           - PROTECTXMP_OPT protect xmp sections
           - PROTECTMSGNUM_OPT protect msgnum sections 
           - PROTECTMETA_OPT protect meta sections
           - PROTECTSCREEN_OPT protect screen sections
           - PROTECTCODEBLOCK_OPT  protect codeblock sections
           - OVERWRITE_OPT overwrite documents in 
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfAnalyzeDoc
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  PSZ         pszDocuments,            // opt: list with document names or NULL
  PSZ         pszMemname,              // opt: Translation Memory for analysis
  LONG        lOptions                 // opt: options for analysis
                                       //@:{TMMATCH_OPT,ADDTOMEM_OPT,AUTOSUBST_OPT,UNTRANSLATED_OPT,
									   //   AUTOLAST_OPT,AUTOJOIN_OPT,AUTOCONTEXT_OPT,REDUNDCOUNT_OPT,
									   //   OVERWRITE_OPT,IGNOREPATH_OPT,ADJUSTLEADWS_OPT,ADJUSTTRAILWS_OPT,
									   //   NOBLANKATSEGEND_OPT,NOSUBSTIFIDENTICAL_OPT,PROTECTXMPSCREEN_OPT,
									   //   RESPECTCRLF_OPT,STOPATFIRSTEXACT_OPT,IGNORECOMMENTED_OPT}
);

/*! \brief Analyze one or more documents (with additional parameters)
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of folder
    \param pszDocuments comma separated list with document names or NULL to analyze all documents of the folder
    \param pszMemname Translation Memory to be used for the analysis
    \param pszProfile analyis profile to be used for the analysis or NULL if not used
    \param pvReserved reserved for future enhancements, has to be NULL
    \param lOptions analysis options
           - TMMATCH_OPT prepare memory match word count
           - ADDTOMEM_OPT write found proposals to folder/document Translation Memory
           - AUTOSUBST_OPT auto-substitute memory matches
           - UNTRANSLATED_OPT write segments without proposals to an external memory (EXP)
           - AUTOLAST_OPT when more exact proposals are available use newest proposal for auto-substitution
           - AUTOJOIN_OPT auto-join segments
           - AUTOCONTEXT_OPT when more exaxct proposals are available use proposal from same document for auto-substitution
           - REDUNDCOUNT_OPT perform redundancy counting
           - IGNOREPATH_OPT ignore path of document names when looking for exact matches
           - ADJUSTLEADWS_OPT adjust leading white space of auto-substituted segments 
           - ADJUSTTRAILWS_OPT adjust trailing white space of auto-substituted segments 
           - RESPECTCRLF_OPT do not auto-substitute segments when number of line breaks are different
           - NOBLANKATSEGEND_OPT remove blank following a period at the end of the segment
           - NOSUBSTIFIDENTICAL_OPT do not auto-substitute segments when source and target of the proposal is identical
           - PROTECTXMPSCREEN_OPT protect xmp, screen, msgnum, meta, and codeblock sections
           - SENDTOMT_OPT send to / prepare data for MT systems
           - STOPATFIRSTEXACT_OPT stop searching Translation Memory databases at first exact match found
           - IGNORECOMMENTED_OPT ignore memory proposals with comments in the additional info area
           - PROTECTXMP_OPT protect xmp sections
           - PROTECTMSGNUM_OPT protect msgnum sections 
           - PROTECTMETA_OPT protect meta sections
           - PROTECTSCREEN_OPT protect screen sections
           - PROTECTCODEBLOCK_OPT  protect codeblock sections
           - OVERWRITE_OPT overwrite documents in 
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfAnalyzeDocEx
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  PSZ         pszDocuments,            // opt: list with document names or NULL
  PSZ         pszMemname,              // opt: Translation Memory for analysis
  PSZ         pszProfile,              // opt: analyis profile name or NULL
  PVOID       pvReserved,              // opt: reserved for future enhancements, has to be NULL
  LONG        lOptions                 // opt: options for analysis
                                       //@:{TMMATCH_OPT,ADDTOMEM_OPT,AUTOSUBST_OPT,UNTRANSLATED_OPT,
									   //   AUTOLAST_OPT,AUTOJOIN_OPT,AUTOCONTEXT_OPT,REDUNDCOUNT_OPT,
									   //   OVERWRITE_OPT,IGNOREPATH_OPT,ADJUSTLEADWS_OPT,ADJUSTTRAILWS_OPT,
									   //   NOBLANKATSEGEND_OPT,NOSUBSTIFIDENTICAL_OPT,PROTECTXMPSCREEN_OPT,SENDTOMT_OPT,
									   //   RESPECTCRLF_OPT,STOPATFIRSTEXACT_OPT,IGNORECOMMENTED_OPT}
);
/*! \brief Delete a Translation Memory
    \param hSession the session handle returned by the EqfStartSession call
    \param pszMemName name of the memory being deleted
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfDeleteMem
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszMemName               // mand: Translation Memory being deleted
);

/*! \brief Import a folder from the \OTM\EXPORT directory
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder being imported
    \param chFromDrive drive containing the imported folder
    \param chToDrive target drive for imported folder
    \param lOptions folder import options
           - XLIFF_OPT import a XLIFF folder (when not specified the import of a standard FXP folder is assumed)
           - NOMARKUP_UPDATE_OPT do not import any markup tables contained in the exported folder
           - WITHDICT_OPT import any dictionaries contained in the exported folder
           - WITHMEM_OPT  import any Translation Memory databases contained in the exported folder
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfImportFolder
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder being imported
  CHAR        chFromDrive,             // mand: drive containing the imported folder
  CHAR        chToDrive,               // opt: target drive for folder
  LONG        lOptions                 // opt: folder import options
                                       // @Import Mode: XLIFF_OPT
									   // @Other: {WITHDICT_OPT, WITHMEM_OPT}
);
/*! \brief Import a folder from the given directory
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder being imported
    \param pszFromPath the fully qualified path name of the directory containing the imported folder
    \param chToDrive target drive for imported folder
    \param lOptions folder import options
           - XLIFF_OPT import a XLIFF folder (when not specified the import of a standard FXP folder is assumed)
           - NOMARKUP_UPDATE_OPT do not import any markup tables contained in the exported folder
           - WITHDICT_OPT import any dictionaries contained in the exported folder
           - WITHMEM_OPT  import any Translation Memory databases contained in the exported folder
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfImportFolderFP
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder being imported
  PSZ         pszFromPath,             // mand: path containing the imported folder
  CHAR        chToDrive,               // opt: target drive for folder
  LONG        lOptions                 // opt: folder import options
                                       // @Import Mode: XLIFF_OPT
									   // @Other: {WITHDICT_OPT, WITHMEM_OPT}
);

/*! \brief Delete a folder
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder being deleted
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfDeleteFolder
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName            // mand: name of folder being deleted
);


/*! \brief Get the properties of a folder
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder to get the properties from
    \param pExtFolProp pointer to a EXTFOLPROP buffer receiving the properties of the folder on return
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfGetFolderProp
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder to get properties from
  PEXTFOLPROP pExtFolProp              // mand: structure of Folderproperties to be returned
);
/*! \brief Delete one or more documents
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder containing the documents
    \param pszDocuments a comma separeted list of documents being deleted
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfDeleteDoc
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder containing the documents
  PSZ         pszDocuments             // mand: list of documents being deleted
);


/*! \brief Create a counting report
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder containing the documents
    \param pszDocuments comma separeted list of documents being counted or NULL if all documents of the folder should be counted
    \param pszOutFile fully qualified name of the output file
    \param usReport ID of the report being created
           - HISTORY_REP create a history report
           - COUNTING_REP create a counting report
           - CALCULATING_REP create a calculating report
           - PREANALYSIS_REP create a pre-analysis report
           - REDUNDANCY_REP create a redundancy report
           - REDUNDANCYSEGMENT_REP create a redundant segment list
    \param usType type of the report being created
           valid for a history report are
           - BRIEF_SORTBYDATE_REPTYPE create a brief report sorted by date
           - BRIEF_SORTBYDOC_REPTYPE create a brief report sorted by documents
           - DETAIL_REPTYPE create a detailed report
           valid types for counting report
           - WITHTOTALS_REPTYPE counting report with totals
           - WITHOUTTOTALS_REPTYPE counting report without totals
           valid types for calculating report, pre-analysis report, redundancy report, and redundant segment list
           - BASE_REPTYPE only base 
           - BASE_SUMMARY_REPTYPE only summary 
           - BASE_SUMMARY_FACTSHEET_REPTYPE base, summary, and factsheet
           - SUMMARY_FACTSHEET_REPTYPE only summary and factsheet
           - FACTSHEET_REPTYPE only factsheet
    \param pszProfile name of the counting report profile to be applied
    \param lOptions options for the counting report
           - OVERWRITE_OPT overwrite any existing output file
           .
           one of the output format options
           - TEXT_OUTPUT_OPT create a plain text report
           - XML_OUTPUT_OPT create a XML report
           - HTML_OUTPUT_OPT create a HTML report
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfCreateCountReport
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder containing the documents
  PSZ         pszDocuments,            // opt: list of documents being counted
  PSZ         pszOutFile,              // mand: fully qualified name of output file
  USHORT      usReport,                // mand: ID of report being created
                                       // @Report ID:CALCULATING_REP,PREANALYSIS_REP,REDUNDANCY_REP,REDUNDANCYSEGMENT_REP
  USHORT      usType,                  // mand: type of report being created
                                       // @Type:BASE_REPTYPE,BASE_SUMMARY_REPTYPE,BASE_SUMMARY_FACTSHEET_REPTYPE,SUMMARY_FACTSHEET_REPTYPE,FACTSHEET_REPTYPE
  PSZ         pszProfile,              // opt: name of profile
  LONG        lOptions                 // opt: output format
                                       // @Output Foramt: HTML_OUTPUT_OPT, XML_OUTPUT_OPT,TEXT_OUTPUT_OPT(default)
									   // @Other: OVERWRITE_OPT
);
/*! \brief Create a archive memory from translated documents
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder
    \param chToDrive folder drive, can be ignored
    \param pszDocuments comma separated list with document names or NULL to include all folder documents
    \param pszMemname name of output Translation Memory
    \param lOptions options for archive memory function
           - OVERWRITE_OPT delete all existing proposals in the Translation Memory before starting
           - USEASFOLDERTM_OPT use specified Translation Memory as folder memory
           - SOURCESOURCEMEM_OPT create a source-source Translation Memory
           - SETMFLAG_OPT set machine translation flag in the proposals written to the Translation Memory
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfArchiveTM
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  CHAR        chToDrive,               // opt: drive
  PSZ         pszDocuments,            // opt: list with document names or NULL
  PSZ         pszMemname,              // mand: output Translation Memory
  LONG        lOptions                 // opt: options for analysis
                                       // @Options: {USEASFOLDERTM_OPT,SOURCESOURCEMEM_OPT,SETMFLAG_OPT,OVERWRITE_OPT}
);
/*! \brief Create a internal markup table (.TBL) from an external markup table (.TBX)
    \param hSession the session handle returned by the EqfStartSession call
    \param pszInFile fully qualified name of input file (.TBX format)
    \param pszOutFile fully qualified name of output file (.TBL format)
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfCreateMarkup
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszInFile,               // mand: fully qualified name of input file
  PSZ         pszOutFile               // mand: fully qualified name of output file
);

/*! \brief Start a OpenTM2 API call session
    \param phSession pointer to a buffer receiving the session handle
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfStartSession
(
  PHSESSION   phSession                // mand: ptr to callers Eqf session handle variable
);

/*! \brief End a OpenTM2 API call session
    \param hSession the session handle returned by the EqfStartSession call
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfEndSession
(
  HSESSION    hSession                 // mand: Eqf session handle
);

/*! \brief Get information about the last occured error
    \param hSession the session handle returned by the EqfStartSession call
    \param pusRC pointer to the buffer for last return code
    \param pszMsgBuffer pointer to a buffer receiving the error message text 
    \param usBufSize size of message buffer in number of bytes
    \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfGetLastError
(
  HSESSION    hSession,                // Eqf session handle
  PUSHORT     pusRC,                   // ptr to buffer for last return code
  PSZ         pszMsgBuffer,            // ptr to buffer receiving the message
  USHORT      usBufSize                // size of message buffer in bytes
);

// 
/*! \brief Create a counting report (old interface, please use EqfCreateCountReport instead)
    \param hSession the session handle returned by the EqfStartSession call
    \param chDriveLetter drive letter of the folder location, not used anymore and should be empty
    \param pszFolderName name of the folder
    \param pszDocuments comma separated list of document names or NULL if complete folder should be counted
    \param pReportType pointer to a REPORTTYPE structure containing the report type information
    \param pszOutfileName fully qualified name of the output report file
    \param pszFormat format of the output file
           - "ASCII" for a plain text report
           - "HTML" for a HTML report
           - "RTF" for a RTF report
           - "XML" for a XML report
    \param pszProfile name of the calculation profile to be used, when specified the following parameters are not used and
           all information is retrieved from the profile instead
    \param pRepSettings pointer to a REPORTSETTINGS structure containing the report settings
    \param pFactSheet pointer to a FACTSHEET structure containing the factsheet infos
    \param usColumn number of columns to in the report
    \param usCategory number of categories in the report
    \param pFinalFactors pointer to a FINALFACTORS structure containing the payment factors
    \param lOptSecurity security check level
    \param bSingleShipment 1 = count single shipments, 0 = count all shipments
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfCreateCntReport
(
  HSESSION    hSession,                // mand: Eqf session handle
  CHAR        chDriveLetter,           // opt: target drive letter
  PSZ         pszFolderName,           // mand: name of folder containing the documents
  PSZ         pszDocuments,            // opt: list of documents being counted
  PREPORTTYPE pReportType,             // mand: type of report
  PSZ         pszOutfileName,              // mand: fully qualified name of output file
  PSZ         pszFormat,               // mand: outpur format
  PSZ         pszProfile,              // opt: name of profile
  PREPORTSETTINGS pRepSettings,        // mand: report settings
  PFACTSHEET pFactSheet,               // mand: factsheet settings
  USHORT      usColumn,                // mand: column handling
  USHORT      usCategory,              // mand: category setting
  PFINALFACTORS pFinalFactors,         // mand: final factors
  LONG        lOptSecurity,            // opt: security settings 
                                       // @Security:{PLAUS_OPT,LOST_OPT,LIST_OPT}
  BOOL        bSingleShipment          // opt: shipment handlign
);
/*! \brief Load a segmented document into memory 
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFullDocName fully qualified name of segmented document
    \param hLoadedFile pointer to buffer for handle of loaded file
    \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfLoadSegFile
(
  HSESSION    hSession,                // Eqf session handle
  PSZ         pszFullDocName,          // fully qualified name of segmented document
  HPARSSEGFILE *phLoadedFile           // pointer to buffer for handle of loaded file
);

/*! \brief Get number of segments in loaded file
    \param hLoadedFile the loaded file handle returned by the EqfLoadSegFile call
    \param plNumOfSegs pointer to a buffer receiving the number of segments in the loaded file
    \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfGetSegNum
(
  HPARSSEGFILE hLoadedFile,            // handle of loaded segmented file
  PLONG       plNumOfSegs              // pointer to buffer for number of segments in file
);

/*! \brief Free all memory occupied by a loaded file
    \param hLoadedFile the loaded file handle returned by the EqfLoadSegFile call
    \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfFreeSegFile
(
  HPARSSEGFILE hLoadedFile             // handle of loaded segmented file
);

// write a segmented document to disk
/*! \brief Create a new folder
    \param hLoadedFile the loaded file handle returned by the EqfLoadSegFile call
    \param pszFullDocName fully qualified name of the segmented document file
    \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfWriteSegFile
(
  HPARSSEGFILE hLoadedFile,            // handle of loaded segmented file
  PSZ         pszFullDocName           // fully qualified name of segmented document
);

/*! \brief Get the data of the specified segment
    \param hLoadedFile the loaded file handle returned by the EqfLoadSegFile call
    \param lSegNum number of the segment
    \param pSeg pointer to a PARSSEGMENTW structure receiving the segment data
    \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfGetSegW
(
  HPARSSEGFILE  hSegFile,              // handle of loaded segmented file
  LONG          lSegNum,               // number of requested segment
  PPARSSEGMENTW pSeg                   // ptr to buffer for segment data
);

/*! \brief Update the data of a specific segment
    \param hLoadedFile the loaded file handle returned by the EqfLoadSegFile call
    \param lSegNum number of the segment
    \param pSeg pointer to a PARSSEGMENTW structure containing the new segment data
    \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfUpdateSegW
(
  HPARSSEGFILE hSegFile,               // handle of loaded segmented file
  LONG         lSegNum,                // number of segment to update
  PPARSSEGMENTW pSeg                   // ptr new segment data
);

/*! \brief Construct the fully qualified name of a segmented document from long folder name and document name
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName pointer to name of the folder
    \param pszDocumentName pointer to document name
    \param fSource when TRUE the name of the segmented source file name is constructed, when FALSE the name of the 
      segmented target file name is constructed
    \param pszSegFile points to buffer receiving the fully qualified document name
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfBuildSegDocName
(
  HSESSION         hSession,           // mand: non-DDE session handle
  PSZ              pszFolderName,      // mand: pointer to long folder name w/p drive, path and ext
  PSZ              pszDocumentName,    // mand: pointer to long document name
  USHORT           fSource,            // mand: TRUE (1)  = build segmented source file name
                                       //       FALSE (0) = build segmented target file name
  PSZ              pszSegFile          // mand: points to buffer for full file name
                                       //       must have a size of at least 60 characters
);

/*! \brief Set the default target language (aka system language)
    \param hSession the session handle returned by the EqfStartSession call
    \param pSystemPropLang the new system language (OpenTM2 language name)
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfSetSysLanguage
(
  HSESSION    hSession,                // mand: ptr to callers Eqf session handle variable
  PSZ         pSystemPropLang          // mand: System language to be set
);

/*! \brief Get the default target language (aka system language)
    \param hSession the session handle returned by the EqfStartSession call
    \param pSystemPropLang buffer receiving the actual system language (OpenTM2 language name)
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfGetSysLanguage
(
  HSESSION   hSession,                // mand: callers Eqf session handle variable
  PSZ        pSystemPropLang          // mand: System language filled at output
);

/*! \brief Get the progress of currently running process
    \param hSession the session handle returned by the EqfStartSession call
    \param pusProgress pointer to a buffer receiving the current progress value
    \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfGetProgress
(
  HSESSION   hSession,                // callers Eqf session handle variable
  PUSHORT    pusProgress              // pointer to buffer for current progress value
);

/*! \brief Get the line information for a specific segment
    \param hSession the session handle returned by the EqfStartSession call
    \param lSegNum number of the requested segment
    \param plStartLine points to a buffer for the segment start line
    \param plEndLine points to a buffer for the segment end line
    \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfGetSourceLine
(
  HPARSSEGFILE hLoadedFile,           // handle of loaded segmented file
  LONG         lSegNum,               // number of requested segment
  PLONG        plStartLine,           // points to buffer for segment start line
  PLONG        plEndLine              // points to buffer for segment end line
);

/*! \brief Get the segment number for a given line/column position
    \param hSession the session handle returned by the EqfStartSession call
    \param lLine line position of segment
    \param lColumn columns position of segment
    \param plSegNum number of segment found at given position
    \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfGetSegmentNumber
(
  HPARSSEGFILE hLoadedFile,           // handle of loaded segmented file
  LONG          lLine,                 // line position of segment
  LONG          lColumn,               // columns position of segment
  PLONG         plSegNum               // number of segment found at given position
);



/*! \brief Structure containing the segment or memory match information
     This structure is used by the EqfGetMatchLevel API for the segment from the
     document and for the memory proposal
*/
#pragma pack( push, TM2StructPacking, 1 )
typedef struct _EQFSEGINFO
{
  WCHAR           szSource[EQF_SEGLEN];          // segment source text (UTF-16 encoded)
  WCHAR           szTarget[EQF_SEGLEN];          // segment target text (UTF-16 encoded) (empty for document segment)
  LONG            lSegNumber;                    // segment number
  CHAR            szDocument[MAX_LONGFILESPEC];  // name of document
  CHAR            szSourceLanguage[MAX_LANG_LENGTH];   // source language of segment
  CHAR            szTargetLanguage[MAX_LANG_LENGTH];   // target language of segment
  CHAR            szMarkup[MAX_FILESPEC];        // markup of segment
} EQFSEGINFO, *PEQFSEGINFO;
#pragma pack( pop, TM2StructPacking )

//
// options for EqfGetMatchLevel API
//

// suppress generic inline tag replacement
#define NO_GENERIC_INLINETAG_REPL_OPT    0x0001

// use generic inline tag replacement function
#define USE_GENERIC_INLINETAG_REPL_OPT   0x0002

/*! \brief State/type of matches used by EqfGetMatchLevel API
*/
typedef enum _EQFMATCHSTATE
{
  REPLACE_MATCHSTATE,                            // replace match
  FUZZYREPLACE_MATCHSTATE,                       // fuzzy replace match
  FUZZY_MATCHSTATE,                              // fuzzy match 
  NONE_MATCHSTATE,                               // no match
  EXACT_MATCHSTATE,                              // exact match 
  EXACTEXACT_MATCHSTATE                          // exact-exact match (match from same document and same segment)      
} EQFMATCHSTATE, *PEQFMATCHSTATE;

// retrieve the match level of a memory proposal
/*! \brief Create a new folder
    \param hSession the session handle returned by the EqfStartSession call
    \param pSegment pointer to the segment info
    \param pProposal pointer to the memory proposal info
    \param psMatchLevel pointer to caller's match level field
    \param psMatchState pointer to caller's match state field
    \param lOptions options to be used by the API call
           - NO_GENERIC_INLINETAG_REPL_OPT suppress inline tag replacement
           - USE_GENERIC_INLINETAG_REPL_OPT perform generic inline tag replacement
    \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
USHORT EqfGetMatchLevel
(
  HSESSION         hSession,           // non-DDE session handle
  PEQFSEGINFO      pSegment,           // pointer to segment info
  PEQFSEGINFO      pProposal,          // pointer to memory proposal info
  SHORT            *psMatchLevel,      // pointer to caller's match level field
  SHORT            *psMatchState,        // pointer to caller's match state field
  LONG             lOptions            // options to be used by the API call
);

/*! \brief Retrieve the match level of a memory proposal (simplified calling syntax)
    \param hSession the session handle returned by the EqfStartSession call
    \param pszSegmentText segment text (Unicode UTF-16 encoded)
    \param pszProposalText memory proposal text (Unicode UTF-16 encoded)
    \param pszLanguage source language of segment and memory proposal (OpenTM2 language name)
    \param pszSegmentMarkup markup table name of the segment
    \param pszProposalMarkup markup table name of the memory proposal
    \param lOptions options to be used by the API call
           - NO_GENERIC_INLINETAG_REPL_OPT suppress inline tag replacement
           - USE_GENERIC_INLINETAG_REPL_OPT perform generic inline tag replacement
    \returns 0 if successful or an error code in case of failures
*/
__declspec(dllexport)
SHORT EqfSimpleGetMatchLevel
(
  HSESSION         hSession,           // non-DDE session handle
  WCHAR            *pszSegmentText,    // segment text
  WCHAR            *pszProposalText,   // memory proposal text
  PSZ              pszLanguage,        // source language of segment and memory proposal
  PSZ              pszSegmentMarkup,   // markup table name of segment
  PSZ              pszProposalMarkup,  // markup table name of memory proposal
  LONG             lOptions            // options to be used by the API call

);



// options fpr EqfCleanMatch API call

// create internal memory
#define CLEANMEM_INTERNAL_MEMORY_OPT  0x00000001 

// create external memory
#define CLEANMEM_EXTERNAL_MEMORY_OPT  0x00000002

// return when all activitiy has been completed (instead of iterative processing)
#define CLEANMEM_COMPLETE_IN_ONE_CALL_OPT 0x01000000
#define COMPLETE_IN_ONE_CALL_OPT 0x01000000


// activate logging (will append log info to the log file \EQF\LOGS\CLEANMEMORY.LOG)
#define CLEANMEM_LOGGING_OPT          0x00010000

// if specified only the best match will be written to the output memory
// if not specified the best 3 fuzzy matches will be written to the output memory
#define CLEANMEM_BESTMATCH_OPT        0x00020000

// merge into existing internal memory
#define CLEANMEM_MERGE_OPT            0x20000000

// keep duplicate exact matches and fuzzy matches in the memory
#define CLEANMEM_KEEP_DUPS_OPT       0x40000000

/*! \brief Remove irrelevant (for the given folder) proposals from an external memory
    The relevant segments are either stored in an internal or external memory. The external memory format
    is EXP in UTF-16 encoding.
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolder name of folder containing the translatable material
    \param pszInMemory fully qualified name of external input memory (EXP format, encoding: UTF-16)
    \param pszOutMemory name of internal output memory or fully qualified name of external output memory
    \param lOptions options for processing
           - CLEANMEM_INTERNAL_MEMORY_OPT write the relevant proposals to an internal memory
           - CLEANMEM_EXTERNAL_MEMORY_OPT write the relevant proposals to an external memory
           - COMPLETE_IN_ONE_CALL_OPT complete the process in one call, without this option the function processes a small unit and
             returns to the calling process with a return code of CONTINUE_RC, the calling process has to re-call the API call until
             any other return code is returned
           - CLEANMEM_LOGGING_OPT activate logging (will append log info to the log file \OTM\LOGS\CLEANMEMORY.LOG)
           - CLEANMEM_BESTMATCH_OPT when specified only the best match will be written to the output memory, without this option
             the best 3 matches will be written to the output memory
           - CLEANMEM_MERGE_OPT merge the relevant proposal into an existing internal memory, without this option the memory is cleared
             before adding the relevant proposals
           - CLEANMEM_KEEP_DUPS_OPT keep duplicate exact matches and fuzzy matches in the memory
		   - OVERWRITE_OPT to overwirte any existing output memory
    \returns 0 if successful, CONTINUE_RC to indicate that the processing is not finished yet, or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/  
__declspec(dllexport)
USHORT EqfCleanMemory
(
  HSESSION         hSession,           // mand: Eqf session handle
  PSZ              pszFolder,          // mand: name of folder containing the translatable material
  PSZ              pszInMemory,        // mand: fully qualified name of external input memory (encoding: UTF-16)
  PSZ              pszOutMemory,       // mand: name of internal output memory or fully qualified name of external output memory
  LONG             lOptions            // opt: options for processing
                                       // @Output:CLEANMEM_INTERNAL_MEMORY_OPT,CLEANMEM_EXTERNAL_MEMORY_OPT
                                       // @Other: {CLEANMEM_LOGGING_OPT, CLEANMEM_BESTMATCH_OPT, CLEANMEM_MERGE_OPT, CLEANMEM_KEEP_DUPS_OPT,COMPLETE_IN_ONE_CALL_OPT, OVERWRITE_OPT}
);

// rename modes
#define RENAME_FOLDER        1
#define RENAME_MEMORY        2
#define RENAME_DICTIONARY    4

// options
#define ADJUSTREFERENCES_OPT  0x04000000

/*! \brief Rename a folder, Translation Memory, or a dictionary
    \param hSession the session handle returned by the EqfStartSession call
    \param usMode rename mode
           - RENAME_FOLDER to rename a folder
           - RENAME_MEMORY to rename a Translation Memory
           - RENAME_DICTIONARY to rename a dictionary
    \param pszName name of the object being renamed
    \param pszNewName new name of the object
    \param lOptions options for processing
           - ADJUSTREFERENCES_OPT adjust any references to the renamed object
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/  
__declspec(dllexport)
USHORT EqfRename
(
  HSESSION         hSession,           // mand: Eqf session handle
  USHORT           usMode,             // mand: rename mode: 
                                       // @Mode:RENAME_FOLDER, RENAME_MEMORY, RENAME_DICTIONARY
  PSZ              pszName,            // mand: name of object being renamed
  PSZ              pszNewName,         // mand: new name of object
  LONG             lOptions            // opt: options for processing
                                       // @Option:ADJUSTREFERENCES_OPT
);


/*! \brief Process a SNOMATCH file
        Run one or more SNOMATCH (Segments with NO MATCH in the searched Translation Memory databases) files against a Translation Memory and copy all relevant memory proposals
        to the output memory, for the segments contained in the SNOMATCH files a memory match count report and a duplicate word count report is created.
        When the option COMPLETE_IN_ONE_CALL_OPT The processing is performed in small units and the API has to be called repetively until a return code other than CONTINUE_RC uis returned
    \param hSession the session handle returned by the EqfStartSession call
    \param pszNomatch fully qualified name of a single SNOMATCH file, the name of a folder containing SNOMATCH files or a search path (using wild-card characters)
           for a group of SNOMATCH files
    \param pszInMemory name of the internal input memory
    \param pszOutMemory name of internal output memory (is created if it does not exist)
    \param pszMemMatchReportText fully qualified file name of the memory match count report (text format)
    \param pszMemMatchReportXml fully qualified file name of the memory match count report (XML format)
    \param pszDupReportText fully qualified file name of the duplicate word count report (text format)
    \param pszDupReportXml fully qualified file name of the duplicate word count report (XML format)
    \param lOptions options for processing
           - COMPLETE_IN_ONE_CALL_OPT complete the processing in one call to the API
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfProcessNomatch
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszNomatch,              // mand: fully qualified name of the SNOMATCh file, folder name or search path
  PSZ         pszInMemory,             // mand: name of the internal input memory
  PSZ         pszOutMemory,            // mand: name of internal output memory (is created if it does not exist)
  PSZ         pszMemMatchReportText,   // opt: fully qualified file name of the memory match count report (text format)
  PSZ         pszMemMatchReportXml,    // opt: fully qualified file name of the memory match count report (XML format)
  PSZ         pszDupReportText,        // opt: fully qualified file name of the duplicate word count report (text format)
  PSZ         pszDupReportXml,         // opt: fully qualified file name of the duplicate word count report (XML format)
  LONG        lOptions                 // opt: options for processing
                                       // @Options: {COMPLETE_IN_ONE_CALL_OPT,RESPECTCRLF_OPT}
);
/*! \brief Process a SNOMATCH file (version with additional parameters)
        Run one or more SNOMATCH (Segments with NO MATCH in the searched Translation Memory databases) files against a Translation Memory and copy all relevant memory proposals
        to the output memory, for the segments contained in the SNOMATCH files a memory match count report and a duplicate word count report is created.
        When the option COMPLETE_IN_ONE_CALL_OPT The processing is performed in small units and the API has to be called repetively until a return code other than CONTINUE_RC uis returned
    \param hSession the session handle returned by the EqfStartSession call
    \param pszNomatch fully qualified name of a single SNOMATCH file, the name of a folder containing SNOMATCH files or a search path (using wild-card characters)
           for a group of SNOMATCH files
    \param pszInMemory name of the internal input memory
    \param pszOutMemory name of internal output memory (is created if it does not exist)
    \param pszMemMatchReportText fully qualified file name of the memory match count report (text format)
    \param pszMemMatchReportXml fully qualified file name of the memory match count report (XML format)
    \param pszDupReportText fully qualified file name of the duplicate word count report (text format)
    \param pszDupReportXml fully qualified file name of the duplicate word count report (XML format)
    \param pszOutNomatchXml fully qualified file name of the output nomatch file in nfluent XML format
    \param pszOutNomatchExp fully qualified file name of the output nomatch file in OpenTM2 EXP format
    \param lOptions options for processing
           - COMPLETE_IN_ONE_CALL_OPT complete the processing in one call to the API
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfProcessNomatchEx
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszNomatch,              // mand: fully qualified name of the SNOMATCh file, folder name or search path
  PSZ         pszInMemory,             // mand: name of the internal input memory
  PSZ         pszOutMemory,            // mand: name of internal output memory (is created if it does not exist)
  PSZ         pszMemMatchReportText,   // opt:  fully qualified file name of the memory match count report (text format)
  PSZ         pszMemMatchReportXml,    // opt:  fully qualified file name of the memory match count report (XML format)
  PSZ         pszDupReportText,        // opt:  fully qualified file name of the duplicate word count report (text format)
  PSZ         pszDupReportXml,         // opt:  fully qualified file name of the duplicate word count report (XML format)
  PSZ         pszOutNomatchXml,        // opt:  fully qualified file name of the output nomatch file in nfluent XML format
  PSZ         pszOutNomatchExp,        // opt:  fully qualified file name of the output nomatch file in TM EXP format
  LONG        lOptions                 // opt:  options for processing
                                       // @Options: {COMPLETE_IN_ONE_CALL_OPT,RESPECTCRLF_OPT}
);
/*! \brief Open a document in OpenTM2 at the given location
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder
    \param pszDocument name of the document
    \param ulSegNum segment number of the segment to be activated
    \param ulLine line to be activated (ulSegNum has to be 0 to use the line number)
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfOpenDoc
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of the folder
  PSZ         pszDocument,             // mand: name of document
  ULONG       ulSegNum,                // opt: segment number of segment to be activated
  ULONG       ulLine                   // opt: line to be activated (ulSegNum has to be 0)
);
/*! \brief Open a document in OpenTM2 at the given location (extented version)
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder
    \param pszDocument name of the document
    \param ulSegNum segment number of the segment to be activated
    \param ulLine line to be activated (ulSegNum has to be 0 to use the line number)
    \param pszSearch UTF-16 encode search string (ulSegNum and ulLine have to be 0 to search for a string)
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfOpenDocEx
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of the folder
  PSZ         pszDocument,             // mand: name of document
  ULONG       ulSegNum,                // opt:  segment number of segment to be activated
  ULONG       ulLine,                  // opt:  line to be activated (ulSegNum has to be 0)
  wchar_t*    pszSearch                // opt:  UTF-16 search string (ulSegNum and ulLine have to be 0)
);

/*! \brief Open a document in OpenTM2 at the given location
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder
    \param pszTrackID TVT tracking ID of the segment to be activated
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfOpenDocByTrack
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of the folder
  PSZ         pszTrackID               // TVT tracking ID of segment to be activated
);

/*! \brief Delete the MT log files of a folder
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfDeleteMTLog
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName            // mand: name of the folder
);

// constants for API call EqfGetShortName

#define FOLDER_OBJ    1
#define MEMORY_OBJ    2
#define DICT_OBJ      3
#define DOCUMENT_OBJ  4 

// 

/*! \brief Get the internally used short name for a folder, dictionary, Translation Memory, or document
      Attention: this API function will only work for the older OpenTM2 plugins. Newer plugins will (hopefully)
      not use short names anymore
    \param hSession the session handle returned by the EqfStartSession call
    \param ObjectType type of the object being processed
           - FOLDER_OBJ object is a folder
           - MEMORY_OBJ object is a Translation Memory
           - DICT_OBJ object is a dictionary
           - DOCUMENT_OBJ object is a document
    \param pszLongName long name of the object
           for documents also the folder name has to be specified in the form foldername:documentname
    \param pszShortName pointer to a buffer for the returned short name
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfGetShortName
(
  HSESSION    hSession,                // mand: Eqf session handle
  USHORT      ObjectType,              // mand: type of object being processed,
                                       // @Type: FOLDER_OBJ, MEMORY_OBJ, DICT_OBJ, DOCUMENT_OBJ
  PSZ         pszLongName,             // mand: long name of the object, for DOCUMENT_OBJ, 
                                       //       use a colon between folder-name and document-name as "folder-name:document-name"
  PSZ         pszShortName             // mand: buffer for returned short name
);

/*! \brief Remove a group of documents based on a list of document names in a plain text file
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder containing the documents
    \param pszListFile fully qualified name of text file containing the list of document names
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfRemoveDocs
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of the folder
  PSZ         pszListFile              // mand: name of text file containing the list of document names
);
/*! \brief Restore a group of documents removed using the EqfRemoveDocs API call
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder containing the documents
    \returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfRestoreDocs
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName            // mand: name of the folder
);

/*! \brief Associate a HAMSTER/Global memory CTID options file to a folder
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder
    \param pszCTIDListFile fully qualified name of the CTID list file
    \returns 0 if successful or an error code in case of failures
*/
 /*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfAddCTIDList
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of the folder
  PSZ         pszCTIDListFile          // mand: fully qualified name of the CTID list file
);

/*! \brief Export segments contained in specific tag groups
\param hSession the session handle returned by the EqfStartSession call
\param pszFolderName name of the folder
\param pszDocuments list with document names or NULL to process all documents of the folder
\param pszStartStopFile file containing the start/stop tag list
\param pszOutFile fully qualified name of the output file receiving the segment data
\param lOptions options for the processing
       - OVERWRITE_OPT overwrite any existing output file
       - COMPLETE_IN_ONE_CALL_OPT complete processing in one single call to the API
\returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfExportSegs
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder
  PSZ         pszDocuments,            // opt: list with document names or NULL
  PSZ         pszStartStopFile,        // mand: file containing start/stop tag list
  PSZ         pszOutFile,              // mand: name of output file
  LONG        lOptions                 // opt:  options 
                                       // @Other: {COMPLETE_IN_ONE_CALL_OPT, OVERWRITE_OPT}
);
/*! \brief Check the existence of a folder
    \param hSession the session handle returned by the EqfStartSession call
    \param pszFolderName name of the folder
    \returns 0 when the folder exists or an error code
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfFolderExists
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName            // mand: name of folder
);

/*! \brief Check the existence of a Translation Memory
    \param hSession the session handle returned by the EqfStartSession call
    \param pszMemoryName name of the Translation Memory
    \returns 0 when the Translation Memory exists or an error code
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfMemoryExists
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszMemoryName            // mand: name of the memory
);

/*! \brief Check the existence of a dictionary
    \param hSession the session handle returned by the EqfStartSession call
    \param pszDictionaryName name of the dictionary
    \returns 0 when the dictionary exists or an error code
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfDictionaryExists
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszDictionaryName        // mand: name of the dictionary
);

/*! \brief Check the existence of a document
    \param hSession the session handle returned by the EqfStartSession call
    \param pszDocumentName name of the document
    \returns 0 when the document exists or an error code
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfDocumentExists
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszFolderName,           // mand: name of folder containing the document
  PSZ         pszDocumentName          // mand: name of the document
);

/**
  \brief Count the number of words in a given string
 
  \param hSession OpenTM2 session handle returned by EqfStartSession
 	\param pszMarkup name of the markup table to be used for the recognition of in-line tags,
   if this parameter is NULL no in-line tag recognition will be performed
 	\param pszLanguage OpenTM2 name for the language of the given text
 	\param pszText null-terminated string containing the text to be counted, the encoding is UTF-16
  \param pulWords points to an unsigned long value receiving the number of words in the text
  \param pulInlineTags points to an unsigned long value receiving the number of inline tags in the text
 	\returns 0 if successful or an error code
 	
**/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport) 
unsigned short EqfCountWordsInString
(
  HSESSION    hSession,                // mand: Eqf session handle
  char*       pszMarkup,               // opt: name of the markup table to be used for the recognition of in-line tags,if this parameter is NULL no in-line tag recognition will be performed
  char*       pszLanguage,             // mand: OpenTM2 name for the language of the given text
  wchar_t*    pszText,                 // mand: null-terminated string containing the text to be counted, the encoding is UTF-16
  ULONG*      pulWords,                // mand: points to an unsigned long value receiving the number of words in the text
  ULONG*      pulInlineTags            // mand: points to an unsigned long value receiving the number of inline tags in the text
);
/**
 \brief Check the spelling of a list of words

 \param hSession OpenTM2 session handle returned by EqfStartSession
	\param pszLanguage name of the language being used for the spell checking
	\param pszInputTerms a comma separated list of terms or NULL if a input file is being used
	\param pszInputFile the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTerms is being used
  \param pszReport name of the report file receiving the results of the operation
  \param lOption options for the output of the report: TEXT_OUTPUT_OPT for plain text output (CSV) or XML_OUTPUT_OPT (default) for XML output
	\returns 0 if successful or an error code
	
**/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport) unsigned short EqfCheckSpelling
(
  HSESSION    hSession,                // mand: Eqf session handle
  char*       pszLanguage,             // mand: name of the language being used for the spell checking
  char*       pszInputTerms,           // opt:  a comma seperated list of terms or NULL if a input file is being used
  char*       pszInputFile,            // opt:  the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTemrs is being used
  char*       pszReport,               // mand: name of the report file receiving the results of the operation, the report is in the XML format
  LONG        lOptions                 // opt:options 
                                       // @Output Format: TEXT_OUTPUT_OPT,XML_OUTPUT_OPT(default)
);
/**
 \brief Reduce a list of words to their stem form

 \param hSession OpenTM2 session handle returned by EqfStartSession
 \param pszLanguage name of the language being used for the spell checking
 \param pszInputTerms a comma separated list of terms or NULL if a input file is being used
 \param pszInputFile the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTerms is being used
 \param pszReport name of the report file receiving the results of the operation
 \param lOption options for the output of the report
     - TEXT_OUTPUT_OPT for plain text output (CSV) or 
     - XML_OUTPUT_OPT (= default) for XML output
	\returns 0 if successful or an error code
	
**/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport) 
unsigned short EqfReduceToStemForm
(
  HSESSION    hSession,                // mand: Eqf session handle
  char*       pszLanguage,             // mand: name of the language being used for the stem form reduction
  char*       pszInputTerms,           // mand: a comma seperated list of terms or NULL if a input file is being used
  char*       pszInputFile,            // mand: the fully qualified name of a plain text file containing the terms, one term per line or NULL if pszInputTemrs is being used
  char*       pszReport,               // mand: name of the report file receiving the results of the operation, the report is in the XML format
  LONG        lOptions                 // opt: options 
                                       // @Output Format:TEXT_OUTPUT_OPT,XML_OUTPUT_OPT(default)
);
/*! \brief Clears the MT flag of an external memory in the EXP format
  
  This API function processes a memory in the .EXP format (encoding UTF-16, ANSI. or ASCII) and
  clears any machine translation flag (MT flags) of the memory proposals.
  When an output memory is specified the processed memory is written to the specified output file
  otherwise the input memory is overwritten with the modified memory.

  \param pszInMemory fully qualified file name of the input memory
  \param pszOutMemory fully qualified file name of the output memory, if not specified the output memory overwrites the input memory
	\returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
unsigned short EqfClearMTFlag
(
  HSESSION    hSession,                // mand: Eqf session handle
  char*       pszInMemory,             // mand: fully qualified file name of the input memory (the file is in EXP format)
  char*       pszOutMemory             // opt:  fully qualified file name of the output memory, if not specified the input memory is changed directly
);

/*! \brief Checks matches from a NOMATCH file against a memory and applies any Global Memory option file.
  
  This API function looks up all matches contained in a NOMATCH file (in XML format)
  in the given memory and applies the specified Global Memory option file on the memory proposals.
  The function creates a memory match word count and writes any matches not found in the input
  memory to a new NOMATCH file. The new NOMATCH file can be in the XML format and/or the .EXP format.
  The processing is done in small units and the API call is to be called repetitively as long as 
  the return code CONTINUE_RC is returned. To do the processing in one block specify the option
  COMPLETE_IN_ONE_CALL_OPT. The word count report can be created in the XML format (use the option XML_OUTPUT_OPT
  ) or in plain text format (use the option TEXT_OUTPUT_OPT). The word count report creation in plain text format is the default. 

  \param hSession the session handle returned by the EqfStartSession call
  \param pszInNoMatchXML fully qualified file name of the input NOMATCH file in XML format
  \param pszGlobMemOptionFile fully qualified file name of the Global Memory option file
  \param pszMemory Name of the internal memory being used for the look-up. 
  \param pszOutNoMatchXML fully qualified file name of the new NOMATCH file in the XML format (can be NULL when not used)
  \param pszOutNoMatchEXP fully qualified file name of the new NOMATCH file in the EXP format (can be NULL when not used)
  \param pszWordCountReport fully qualified file name of the created memory match word count report (can be NULL when not used)
  \param lOptions options for the processing
    - COMPLETE_IN_ONE_CALL_OPT to do the processing in one call (rather than doing the processing in small units)
    - TEXT_OUTPUT_OPT to create the word count report in plain text format (=default)
    - XML_OUTPUT_OPT to create the word count report in XML format
	\returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
unsigned short EqfFilterNoMatchFile
(
  HSESSION    hSession,               // mand: Eqf session handle   
  char*       pszInNoMatchXML,        // mand: fully qualified file name of the input NOMATCH file in XML format   
  char*       pszGlobMemOptionFile,   // mand: fully qualified file name of the Global Memory option file  
  char*       pszMemory,              // mand: Name of the internal memory being used for the lookup.    
  char*       pszOutNoMatchXML,       // opt:  fully qualified file name of the new NOMATCH file in the XML format
  char*       pszOutNoMatchEXP,       // opt:  fully qualified file name of the new NOMATCH file in the EXP format 
  char*       pszWordCountReport,     // opt:  fully qualified file name of the created memory match word count report
  long        lOptions                // opt:  options for the processing
                                      // @Output Format: XML_OUTPUT_OPT,TEXT_OUTPUT_OPT(default) 
                                      // @Other: COMPLETE_IN_ONE_CALL_OPT								  
);

/*! \brief Deletes the given dictionary.
  \param hSession the session handle returned by the EqfStartSession call
  \param pszDict name of the dictionary being deleted
	\returns 0 if successful or an error code in case of failures
*/
/*@ADDTOSCRIPTER*/ 
__declspec(dllexport)
USHORT EqfDeleteDict
(
  HSESSION    hSession,                // mand: Eqf session handle
  PSZ         pszDictName              // mand: name of dictionary
);

/*! \brief Get OpenTM2 version information
    \param  pszVersion pointer to a buffer for the version string
    \param  length size of the buffer for the version string
    \returns version information
*/
/*@ADDTOSCRIPTER*/
__declspec(dllexport)
USHORT EqfGetVersionEx
(
  PSZ  pszVersion,                    // mand: to retrieve version information
  int  length                         // /*@Ignore*/ mand: legnth of the retrive buffer  
);


#ifdef __cplusplus
}
#endif

#endif
