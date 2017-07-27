/*! match list processing functions
	Copyright (c) 1990-2017, International Business Machines Corporation and others. All rights reserved.
*/

#ifdef EQFTAML_MATCHLISTFUNCTIONS

__declspec(dllexport)
int TAMatchListInit
(
  PTAINPUT    pTAIn
);


__declspec(dllexport)
int TAMatchListTerminate
(
  PTAINPUT    pTAIn
);

__declspec(dllexport)
int TAMatchListProcess
(
  PTAINPUT    pTAIn,
  PSZ_W       pszSegment,
  PSZ_W       pszProposal,
  MATCHTYPE   MatchType,
  USHORT      usFuzzyness
);

__declspec(dllexport)
int TABalanceList
(
  PSZ         pszFolder,                         // folder object name
  PSZ         pszDocs,                           // ptr to list of documents
  USHORT      usFiles                            // number of source files
);

#endif

#ifdef EQFTAML_DOCEXPVALFORMAT

// name of sytelsheet for XML to HTML conversion of validation document
#define VALXMLTOHTML_STYLESHEET "valdoc2html.xsl"
#define VALXMLTOHTML_TRANSONLY_STYLESHEET "valdoc3html.xsl"
// IDs for the validation format export formats
typedef enum _VALFORMATID
{
  HTML_VALEXPFORMAT,
  XML_VALEXPFORMAT,
  DOC_VALEXPFORMAT,
  PLAINXML_VALEXPFORMAT,
  DOCX_VALEXPFORMAT,
  ODT_VALEXPFORMAT
} VALFORMATID;

// options for validation format export
typedef struct _DOCEXPVALOPTIONS
{
  VALFORMATID ValFormat;                         // selected export format
  BOOL fCombined;                                // TRUE = export in combined format
  BOOL fExportInValidationFormat;                // TRUE = validation format, FALSE = proofreading format

  // match options for proofreading format
  EQF_BOOL fNewMatch;
  EQF_BOOL fNoInlineTagRemoval;
  EQF_BOOL fProtMatch;
  EQF_BOOL fInclCount;
  EQF_BOOL fAutoMatch;
  EQF_BOOL fInclExisting;
  EQF_BOOL fNotTransl;
  EQF_BOOL fMismatchOnly;
  EQF_BOOL fFuzzyMatch;
  EQF_BOOL fModifiedExact;
  EQF_BOOL fExactMatch;
  EQF_BOOL fLinksImages;
  EQF_BOOL fMachMatch;
  EQF_BOOL fGlobMemMatch;
  EQF_BOOL fReplMatch;
  EQF_BOOL fModifiedAuto;
  
  CHAR szOutName[MAX_LONGFILESPEC];             // output file name as specified by user (w/o path information)
  
  CHAR_W       szTranslator[256];                // translator name for report header
  CHAR_W       szManager[256];                   // manager name for report header

  EQF_BOOL   fExactFromManual;                   // check for exact matches coming from manual translation switch
  EQF_BOOL   fTransOnly;                         // check for export trans text in simple format switch 

} DOCEXPVALOPTIONS, *PDOCEXPVALOPTIONS;

// document export in validation format functions

__declspec(dllexport)
int DocExpValInit
(
  PDOCEXPVALOPTIONS pOptions,                    // document options
  PSZ         pszDirectory,                      // target directory for output file
  PSZ         pszFolObjName,                     // folder object name
  PLONG       plHandle,                          // points to buffer for val format export handle
  BOOL        fOverwrite,                        // TRUE = overwrite files without confirmation message
  HWND        hwndParent                         // parent window handle (used as parent window for error messages)
);

__declspec(dllexport)
int DocExpValTerminate
(
  LONG        lHandle                            // val format export handle
);

__declspec(dllexport)
int DocExpValProcess
(
  LONG        lHandle,                            // val format export handle
  PSZ         pszDocObjName                       // object name of document being exported
);

__declspec(dllexport)
int DocExpValPreScan
(
  LONG        lHandle,                            // val format export handle
  PSZ         pszDocObjName                       // object name of document being processed
);

#endif
