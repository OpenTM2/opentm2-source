/*! \brief EQFTAML.CPP
	Copyright (c) 1990-2017, International Business Machines Corporation and others. All rights reserved.
	Description: XML Output related functions of TranslationManager
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_SLIDER           // slider utility functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_EDITORAPI        // editor API (required for EQFTPI.H)

#pragma pack( push, TM2StructPacking, 1 )

  #include "eqf.h"                  // General .H for EQF

#include <direct.h>
#include "OtmProposal.h"
#include "AnalysisMemLookupData.h"
#include "core\memory\MemoryFactory.h"

  #include "eqftai.h"
  #include "eqftpi.h"
  #include "ValDocExp.h"
  #include "eqfserno.h"
  #include <time.h>
#pragma pack( pop, TM2StructPacking )


#include "cxmlwriter.h"

// includes for OLE interface
#include <ole2.h>
#include <oleauto.h>

// activate the following define to log the setting of the track changes flag for DOCX files
//#define SETTRACKFLAGLOG

//////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Document Export in Validation Format
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////


// size of token buffer
#define VALTOKBUFSIZE 32000

// instruction strings
static CHAR szValDocInstructions[][200] =
{
  "Instructions for review:",
  " ",
  "This document shows several columns of which two require your attention. ",
  "One column contains the source text (Source) and the other one the translated text (Translation). The translated text is to be reviewed by you.",
  " ",
  "For review purposes, please:",
  "- Always use the function \"Track Changes\" for all changes (validation process).",
  "- Enter your changes in the Translation column only. Do not edit the source text in the left column.",
  "- Make necessary changes only - avoid purely stylistic changes.",
  "- Be aware that one sentence in the Source column might be split over more than one cell due to segmentation. This would then apply to the Translation column as well.",
  " ",
  "Please do not:",
  "- Insert question marks, mark your changes clearly.",
  "- Move text from one cell to another, as this would have an impact on text formatting.",
  "- Delete or add any cells to the table.",
  "- Edit any text outside the given table borders.",
  " ",
  "Many thanks in advance.",
  " ",
  "The IBM Translation Services Center",
  ""                                             // end of array indicator
};

// match description strings
static CHAR szValDocMatchDescription[][80] =
{
  ""                                             // end of array indicator
};

// replace list entry definition
typedef struct _VALTAGREPLACEENTRY
{
  CHAR_W      chFrom[40];
  int         iLen;
  CHAR_W      chTo[40];
} VALTAGREPLACEENTRY, *PVALTAGREPLACEENTRY;

// tag replace lists for some markup table
VALTAGREPLACEENTRY IBMXMSTI_ReplList[] =
{
  { L"<nobr>&amp;nbsp;</nobr>", 23, L" " },
  { L"<hyphen/>",                9, L"-" },
  { L"<ldquote/>",              10, L"\"" },
  { L"<rdquote/>",              10, L"\"" },
  { L"<eserver/>",              10, L"eServer" },
  { L"<inch/>",                  7, L"\"" },
  { L"<foot/>",                  7, L"\'" },
  { L"<THSpace/>",              10, L" " },
  { L"<EMSpace/>",              10, L" " },
  { L"<ENSpace/>",              10, L" " },
  { L"",                         0, L"" }
};

VALTAGREPLACEENTRY HTML_ReplList[] =
{
  { L"&nbrsp;",                  7, L" " },
  { L"",                         0, L"" }
};

// defintion of markup table to replace list table
typedef struct _VALMARKUPLIST
{
  CHAR        chMarkup[13];
  PVALTAGREPLACEENTRY pReplaceList;
} VALMARKUPLIST, *PVALMARKUPLIST;


// markup table to replace list table
VALMARKUPLIST MarkupToReplaceList[] =
{
  { "IBMXMSTI", IBMXMSTI_ReplList },
  { "IBMHTM32", HTML_ReplList },
  { "IBMHTM32", HTML_ReplList },
  { "EQFHTML2", HTML_ReplList },
  { "EQFHTML4", HTML_ReplList },
  { "",         NULL }
};

//
// definition of in-memory copy of replacement list
//

// max length of a markup table name
#define MUT_NAME_LEN  20

// max markup table names per section
#define MUT_NAME_ENTRIES   40

// max length of a replacement tag
#define REPL_TAG_LEN  40

// max length of the replacement string
#define REPL_STRING_LEN  40

// replace entry
typedef struct _VALDOCREPLENTRY
{
  CHAR_W szFrom[REPL_TAG_LEN];         // tag name
  CHAR_W szTo[REPL_STRING_LEN];        // string replacing the tag
  int    iLen;                         // tag name length in characters
} VALDOCREPLACEENTRY, *PVALDOCREPLACEENTRY;


// markup table group
typedef struct _VALDOCREPLMU
{
  int    iSize;                        // overall size of this structure
  int    iEntries;                     // number of entries in markup table group
  CHAR   szMarkup[MUT_NAME_ENTRIES][MUT_NAME_LEN];       // markup table names
  VALDOCREPLACEENTRY Entry[1];         // dynamic array of replace entries
} VALDOCREPLACEMU, *PVALDOCREPLACEMU;

// replace list containing all markup table groups
typedef struct _VALDOCREPLACELIST
{
  int    iSize;                        // overall size of this structure
  int    iEntries;                     // number of markup groups in replace list
  VALDOCREPLACEMU MarkupGroup;         // markup group
} VALDOCREPLACELIST, *PVALDOCREPLACELIST;

// static replacement list, only loaded once
static PVALDOCREPLACELIST pReplList = NULL;
static BOOL               fReplListLoadError = FALSE;

BOOL ValExpAddReplaceEntry( PVALDOCREPLACELIST *ppReplList, PSZ_W pszReplEntry, PSZ_W pszValue );
BOOL ValExpStartMarkupGroup( PVALDOCREPLACELIST *ppReplList, PSZ_W pszMarkup );
PVALDOCREPLACELIST ValExpGetReplacementList();
PVALDOCREPLACEMU ValExpGetMarkupGroup( PSZ pszMarkup );
BOOL ValExpApplyReplaceList( PVALDOCREPLACEMU pMU, PSZ_W pszInString, PSZ_W pszOutString, PBOOL isTag, BOOL fLinksImages );
BOOL ValIsTaxonomyDoc( PTBDOCUMENT pDoc );
BOOL ValSetTrackingFlag( PSZ pszDocument );


// segment list entry for manually translated segments
typedef struct _VALSEGENTRY
{
  PSZ_W pszSource;                     // source of segment
  PSZ_W pszTarget;                     // target of segment
} VALSEGENTRY, *PVALSEGENTRY;

// our data area
typedef struct _DOCEXPVALFORMATDATA
{
  // values as specified in the init call
  CHAR        szTargetDir[MAX_LONGFILESPEC];     // target directory for output file
  CHAR        szFolObjName[MAX_LONGFILESPEC];    // folder object name
  DOCEXPVALOPTIONS Options;                      // document options
  HWND        hwndParent;                        // parent window handle (used as parent window for error messages)
  // values evaluated during init
  CXmlWriter *xw;                                // ptr to active XmlWriter instance
  CHAR        szXmlOutFile[MAX_LONGFILESPEC];    // name of XML output file
  CHAR        szFinalOutFile[MAX_LONGFILESPEC];  // name of final output file
  BOOL        fMsg;                              // TRUE = show error messages
  CHAR        szHtmlFile[MAX_LONGFILESPEC];      // name of intermediate HTML file
  CHAR        szFolderDir[MAX_LONGFILESPEC];     // folder directory
  CHAR        szFolderName[MAX_LONGFILESPEC];    // folder (long) name
  // values from  current document
  CHAR        szLongName[MAX_LONGFILESPEC];      // lon
  CHAR        szDocFormat[MAX_FILESPEC];         // format of document
  CHAR        szDocSourceLang[MAX_LANG_LENGTH];  // document source language
  CHAR        szDocTargetLang[MAX_LANG_LENGTH];  // --   target language
  PTBDOCUMENT pSourceDoc;                        // ptr to source document
  PTBDOCUMENT pTargetDoc;                        // ptr to target document
  PLOADEDTABLE pLoadedQFTable;                   // ptr to loaded QF tag table
  CHAR        szDocName[MAX_LONGFILESPEC];       // document (long) name
  ULONG       ulCP;                              // codepage to use for current language
  // general purpose buffers
  CHAR        szFileName1[MAX_LONGFILESPEC];     // buffer for file/path names
  CHAR        szFileName2[MAX_LONGFILESPEC];     // buffer for file/path names
  CHAR        szBuffer[MAX_SEGMENT_SIZE*2];      // buffer for smaller text strings
  CHAR_W      szSegBuffer[MAX_SEGMENT_SIZE*2];   // buffer for segment text processing
  CHAR_W      szSegBuffer2[MAX_SEGMENT_SIZE*2];  // buffer for segment text processing
  BYTE        bTokBuffer[VALTOKBUFSIZE];         // buffer for segment text tokenization
  // user choices
  BOOL        fOverwrite;                        // TRUE = overwrite existing files
  BOOL        fCreateAllDirs;                    // TRUE = create all directories without asking for permission
  BOOL        fTransOnly;                        // TRUE = Output translatable text only
  BOOL        fThai;                             // TRUE = Handling Thai text               
  //
  PVALDOCREPLACEMU pReplaceList;                 // active tag replace list for current markup or NULL if none
  BOOL        isTag[MAX_SEGMENT_SIZE*2];                // buffer for segment text processing
  // data for attribute and tag processing
  CHAR_W      szAttrName[MAX_SEGMENT_SIZE];      // buffer for attribute name
  CHAR_W      szAttrData[MAX_SEGMENT_SIZE];      // buffer for attribute data
  CHAR_W      szTagBuffer[MAX_SEGMENT_SIZE];     // buffer for createdor modified tags
  // data for automatic recognition of manually translated exact matches
  PPOOL       pPool;                             // string pool for manually translated segments
  std::vector<VALSEGENTRY> *pvSegTable;          // vector containing list of manually translated segments
} DOCEXPVALFORMATDATA, *PDOCEXPVALFORMATDATA;

// internal helper functions


static int MakeTempFile
(
  PSZ         pszTempFile,                       // buffer for temp file name
  PSZ         pszDir,                            // directory to use for temp file
  PSZ         pszExt                             // extension to use for temp file
)
{
  int iRC = 0;
  long lCounter = 1;

  do
  {
    sprintf( pszTempFile, "%s$$$$%ld%s", pszDir, lCounter++, pszExt );
  } while ( UtlFileExist( pszTempFile ) );

  return iRC;
} /* end of function MakeTempFile */

static int MakeOutFileName
(
  PSZ         pszFileName,                       // buffer for fully qualified file name
  PSZ         pszDir,                            // directory to use for file
  PSZ         pszName,                           // file name w/o path
  VALFORMATID Format,                            // format of output file
  BOOL        fUserName                          // TRUE: pszName has been specified by the user
)
{
  int iRC = 0;
  PSZ pszFormatExtension = NULL;

  strcpy( pszFileName, pszDir );
  strcat( pszFileName, pszName );


  switch ( Format )
  {
    case HTML_VALEXPFORMAT     : pszFormatExtension = ".HTM"; break;
    case XML_VALEXPFORMAT      : pszFormatExtension = ".XML"; break;
    case DOC_VALEXPFORMAT      : pszFormatExtension = ".DOC"; break;
    case PLAINXML_VALEXPFORMAT : pszFormatExtension = ".XML"; break;
    case ODT_VALEXPFORMAT      : pszFormatExtension = ".ODT"; break;
    case DOCX_VALEXPFORMAT     : pszFormatExtension = ".DOCX"; break;
  } /*endswitch */
  if ( !fUserName || (strchr( pszName, '.' ) == NULL) )
  {
    strcat( pszFileName, pszFormatExtension );
  }
  else
  {
    // check if correct extension has been used
    PSZ pszExt = strrchr( pszName, '.' );

    switch ( Format )
    {
      case HTML_VALEXPFORMAT :
        if ( (stricmp( pszExt, ".HTML") != 0) && (stricmp( pszExt, pszFormatExtension ) != 0) )
        {
          strcat( pszExt, pszFormatExtension );
        } /* endif */
        break;
      case PLAINXML_VALEXPFORMAT :
      case XML_VALEXPFORMAT  :
      case DOC_VALEXPFORMAT  :
      case DOCX_VALEXPFORMAT  :
        if ( stricmp( pszExt, pszFormatExtension ) != 0 )
        {
          strcat( pszExt, pszFormatExtension );
        } /* endif */
        break;
    } /*endswitch */
  } /* endif */

  return iRC;
} /* end of function MakeOutFileName */

// write instructions
static int WriteInstructions( CXmlWriter *xw )
{
  int iRC = 0;

  if ( szValDocInstructions[0][0] != 0 )
  {
    int i = 0;
    xw->WriteStartElement( "Instructions" );
    while ( szValDocInstructions[i][0] != 0 )
    {
      xw->WriteElementString( "Line", szValDocInstructions[i] );
      i++;
    } /*endwhile */
    xw->WriteEndElement();   // Instructions
  } /* endif */

  return iRC;
} /* end of function WriteInstructions */

// function currently not used
#ifdef WriteMatchDescription
// write match descriptions
static int WriteMatchDescription( CXmlWriter *xw )
{
  int iRC = 0;

  if ( szValDocMatchDescription[0][0] != 0 )
  {
    int i = 0;
    xw->WriteStartElement( "MatchDescription" );
    while ( szValDocMatchDescription[i][0] != 0 )
    {
      xw->WriteElementString( "Line", szValDocMatchDescription[i] );
      i++;
    } /*endwhile */
    xw->WriteEndElement();   // MatchDescription
  } /* endif */

  return iRC;
} /* end of function WriteMatchDescription */
#endif

static int StartXmlOutput
(
  PDOCEXPVALFORMATDATA pData,                    // data area
  PSZ         pszOutFile                         // fully qualified name of output file
)
{
  int iRC = 0;

  CXmlWriter *xw = new CXmlWriter( pszOutFile );
  xw->Formatting = CXmlWriter::Indented;
  xw->Encoding = CXmlWriter::UTF8;
  xw->Indention = 2;
  if ( xw->WriteStartDocument() )
  {
    if ( pData->Options.ValFormat == PLAINXML_VALEXPFORMAT)
    {
      xw->WriteStartElement( "XmlExport" );
    }
    else
    {
      xw->WriteStartDocType( L"Proof" );
      xw->WriteEntity( L"shy", L"&#173;" );
      xw->WriteEndDocType();

      if ( pData->fTransOnly ) 
         xw->WriteStylesheet( VALXMLTOHTML_TRANSONLY_STYLESHEET );
      else
         xw->WriteStylesheet( VALXMLTOHTML_STYLESHEET );
      xw->WriteStartElement( "Proof" );
      xw->WriteAttributeString( "Type", pData->Options.fExportInValidationFormat ? "Validation" : "Proof" );
      if ( pData->Options.fExportInValidationFormat )
      {
        iRC = WriteInstructions( xw );
      } /* endif */
    } /* endif */
  }
  else
  {
    if ( pData->fMsg ) UtlErrorHwnd( ERROR_FILENAME_NOT_VALID, MB_CANCEL, 1, &pszOutFile, EQF_ERROR, pData->hwndParent );
    iRC = ERROR_FILENAME_NOT_VALID;
  } /* endif */
  pData->xw = xw;

  return iRC;
} /* end of function StartXmlOutput */

// end XML output
static int EndXmlOutput
(
  PDOCEXPVALFORMATDATA pData                     // data area
)
{
  int iRC = 0;

  // terminate XML output
  CXmlWriter *xw = pData->xw;

  xw->WriteEndElement(); // "Proof"
  xw->WriteEndDocument();
  xw->Close();

  delete xw;

  return iRC;
} /* end of function EndXmlOutput */

// convert XML file to HTML using XSLT
static int ConvertXml2Html
(
  PDOCEXPVALFORMATDATA pData,                    // data area
  PSZ         pszXmlFile,                        // name of file containing the XML data
  PSZ         pszHtmlFile                        // name of file receiving the HTML output
)
{
  int iRC = 0;

  // setup fully qualified name of XSL style sheet
  UtlMakeEQFPath( pData->szFileName1, NULC, TABLE_PATH, NULL );
  strcat( pData->szFileName1, BACKSLASH_STR );
  if ( pData->fTransOnly )
     strcat( pData->szFileName1, VALXMLTOHTML_TRANSONLY_STYLESHEET );
  else
     strcat( pData->szFileName1, VALXMLTOHTML_STYLESHEET );

  // call Xalan converter to do the conversion
  iRC = XSLTConversion( pszXmlFile, pszHtmlFile, pData->szFileName1, pData->fMsg, pData->hwndParent );

  return iRC;
} /* end of function ConvertXml2Html */

LPOLESTR EQFAnsiToUnicode(LPCSTR pszA)
{
  int           iNewLength;
  int           iInputLength;
  LPOLESTR      lpOleStr = NULL;

  iInputLength = strlen(pszA) + 1;
  iNewLength = MultiByteToWideChar( CP_ACP, 0, pszA, iInputLength, NULL, 0 );

  if ( iNewLength )
  {
    UtlAlloc( (PVOID*)&lpOleStr,0L, (LONG)(iNewLength * 2 + 2),ERROR_STORAGE);
    if ( lpOleStr )
    {
      if ( MultiByteToWideChar( CP_ACP, 0, pszA, iInputLength, lpOleStr, iNewLength ) == 0 )
      {
          /* conversion failed */
          UtlAlloc( (PVOID*)&lpOleStr,0L, 0L, NOMSG);
          lpOleStr = NULL;
      } /* endif */
    } /* endif */
  } /* endif */

  return lpOleStr;
} // end of function EQFAnsiToUnicode

//+----------------------------------------------------------------------------+
//| EQFAutoWrap                                                                |
//+----------------------------------------------------------------------------+                                                       |
//| Wrapper function for making generic 16-bit Automation calls...             |
//|                                                                            |
//|   from HOWTO: Do 16-bit Automation in C++ Using VC 1.52                    |
//|               Article ID: Q194656                                          |
//|                                                                            |
//|                                                                            |
//| Prereqs:                                                                   |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
BOOL EQFAutoWrap
(
  int autoType,
  VARIANT *pvResult,
  IDispatch *pDisp,
  char *ptName,
  int cArgs...
)
{
    va_list marker;
    BOOL    fOk = TRUE;
    // Variables used...
    DISPPARAMS dp = { NULL, NULL, 0, 0};
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    DISPID dispID = 0;
    HRESULT hr;

    CHAR szTemp[100];
    PSZ pszMsgTable[2];

    // Allocate memory for arguments...
    VARIANT *pArgs = new VARIANT[cArgs+1];

    va_start(marker, cArgs);

    if ( !pDisp )
    {
        UtlError(ERROR_OLE_WRAPPER_1, MB_CANCEL, 1, &ptName, EQF_ERROR);
        fOk = FALSE;
    } // end if

    if ( fOk )
    {

        LPOLESTR   lpUniBuffer;
        lpUniBuffer = EQFAnsiToUnicode( ptName );

        // Get DISPID for name passed...
        hr = pDisp->GetIDsOfNames( IID_NULL, &lpUniBuffer, 1, LOCALE_USER_DEFAULT, &dispID);

        if ( FAILED(hr) )
        {
            UtlGetWinError(hr, szTemp);

            pszMsgTable[0] = ptName;
            pszMsgTable[1] = szTemp;

            UtlError(ERROR_OLE_WRAPPER_RC, MB_CANCEL, 2, pszMsgTable, EQF_ERROR);

            fOk = FALSE;

        } // end if

    } // end if fOk

    if ( fOk )
    {
        // Extract arguments...
        for ( int i=0; i<cArgs; i++ )
        {
            pArgs[i] = va_arg(marker, VARIANT);
        }

        // Build DISPPARAMS
        dp.cArgs = cArgs;
        dp.rgvarg = pArgs;

        // Handle special-case for property-puts!
        if ( autoType & DISPATCH_PROPERTYPUT )
        {
            dp.cNamedArgs = 1;
            dp.rgdispidNamedArgs = &dispidNamed;
        } // end if

        // Make the call!
        hr = pDisp->Invoke( dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT, (WORD)autoType, &dp, pvResult, NULL, NULL);
        if ( FAILED(hr) )
        {

            UtlGetWinError(hr, szTemp);

            pszMsgTable[0] = ptName;
            pszMsgTable[1] = szTemp;

            UtlError(ERROR_OLE_WRAPPER_RC, MB_CANCEL, 2, pszMsgTable, EQF_ERROR);

            fOk = FALSE;

        } // end if

    } // end if fOk

    // End variable-argument section...
    va_end(marker);

    delete [] pArgs;

    return fOk;
} // end of function EqfAutoWrap



// convert HTML file to DOC using MS Word
static int ConvertHtml2Doc
(
  PDOCEXPVALFORMATDATA pData,                    // data area
  PSZ         pszHtmlFile,                       // name of file containing the HTML document
  PSZ         pszDocFile,                        // name of file receiving the document in DOC format
  int         DocFormat                          // type of document to create (DOC or DOCX)
)
{
  int iRC = 0;
  BOOL       fOk = TRUE;

  BOOL       fOleInit = FALSE;
  BOOL       fOleOK = TRUE;

  HRESULT    hr;
  CLSID      clsid;
  IUnknown   *pUnk = NULL;
  IDispatch  *pDispRoot = NULL;
  IDispatch  *pDispApp  = NULL;
  IDispatch  *pDispDocs = NULL;
  IDispatch  *pDispDoc = NULL;

  BOOL       fSnapshot;
  DWORD      dwWinWordOldPids[5];
  DWORD      dwWinWordNewPids[5];

  // Make a snapshot of current running WinWord.exe processes
  fSnapshot = UtlGetPidMultipleInstances( "winword.exe", dwWinWordOldPids );

  if ( !iRC )
  {
    OleInitialize( NULL );
    fOleInit = TRUE;

//  AfxOleInit();
//  COleMessageFilter *pFilter;
//  pFilter = AfxOleGetMessageFilter();
//  ASSERT_VALID(pFilter);
//  pFilter->EnableBusyDialog(FALSE);
//  pFilter->EnableNotRespondingDialog(FALSE);

    // Get CLSID for our server...
    hr = CLSIDFromProgID( L"Word.Application", &clsid );
    if ( FAILED(hr) )
    {
      iRC = ERROR_CALL_WORD_APPLICATION;
      if ( pData->fMsg )UtlErrorHwnd( ERROR_CALL_WORD_APPLICATION, MB_CANCEL, 0, NULL, EQF_ERROR, pData->hwndParent );
    } // end if
  }// end if fOk

  // Start the server...
  if ( !iRC )
  {
    hr = CoCreateInstance( clsid, NULL, CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**)&pUnk );
    if ( FAILED(hr) )
    {
      fOleOK = FALSE;
      iRC = ERROR_CALL_WORD_APPLICATION;
      if ( pData->fMsg ) UtlErrorHwnd( ERROR_CALL_WORD_APPLICATION, MB_CANCEL, 0, NULL, EQF_ERROR, pData->hwndParent );
    }// end if
  } // end if fOk

  // Query for IDispatch
  if ( !iRC )
  {
    hr = pUnk->QueryInterface( IID_IDispatch, (void**)&pDispRoot );
    if ( FAILED(hr) )
    {
      pUnk->Release();
      fOleOK = FALSE;
      iRC = ERROR_CALL_WORD_APPLICATION;
      if ( pData->fMsg )UtlErrorHwnd( ERROR_CALL_WORD_APPLICATION, MB_CANCEL, 0, NULL, EQF_ERROR, pData->hwndParent );
    }
  } // end if

  // GET App IDispatch
  if ( !iRC )
  {
    VARIANT Result;
    VariantInit( &Result );
    fOleOK = EQFAutoWrap( DISPATCH_PROPERTYGET, &Result, pDispRoot, "Application", 0);
    if ( !fOleOK ) iRC = ERROR_CALL_WORD_APPLICATION;
    pDispApp = Result.pdispVal;
  }

  // Get Documents collection...         [WordApp~Documents]
  if ( !iRC )
  {
    VARIANT result;
    VariantInit(&result);
    fOleOK = EQFAutoWrap( DISPATCH_PROPERTYGET, &result, pDispApp, "Documents", 0);
    if ( !fOleOK ) iRC = ERROR_CALL_WORD_APPLICATION;
    pDispDocs = result.pdispVal;
  }// end if fOk

  // Call Documents::Add()...            [WordApp~Add]
  if ( !iRC )
  {
    VARIANT result;
    VariantInit(&result);
    VARIANT parm;
    parm.vt = VT_BSTR;
    parm.bstrVal = EQFAnsiToUnicode( pszHtmlFile );
    fOleOK = EQFAutoWrap(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &result, pDispDocs, "Add", 1, parm);
    if ( !fOleOK ) iRC = ERROR_CALL_WORD_APPLICATION;
    pDispDoc = result.pdispVal;
    UtlAlloc( (PVOID*)&(parm.bstrVal),0L, 0L, NOMSG);
  }// end if fOk

  // SaveAs DOC                           [WordApp~SaveAs]
  if ( !iRC )
  {
    int num = 0;       // use DOC format for save-as
    VARIANT result;
    VariantInit(&result);
    VARIANT parm;
    parm.vt = VT_BSTR;
    parm.bstrVal = EQFAnsiToUnicode(pszDocFile);
    VARIANT parm1;
    parm1.vt = VT_I4;
    if ( DocFormat == DOCX_VALEXPFORMAT) 
       num = 16;       // use DOCX format for save-as
    parm1.lVal = num;
    fOleOK = EQFAutoWrap( DISPATCH_METHOD, &result, pDispDoc, "SaveAs", 2,  parm1, parm);
    if ( !fOleOK ) iRC = ERROR_CALL_WORD_APPLICATION;
    UtlAlloc( (PVOID*)&(parm.bstrVal),0L, 0L, NOMSG);
  } // end if fOk

  //Quit                               [WordApp~Quit]
  if ( !iRC )
  {
    VARIANT result;
    VariantInit( &result );
    fOleOK = EQFAutoWrap(DISPATCH_METHOD, 0, pDispApp, "Quit", 0);
    if ( !fOleOK ) iRC = ERROR_CALL_WORD_APPLICATION;
  }

  if ( !fOleOK )
  {
    // kill hanging process - doc is locked
    DWORD dwKillPid = 0;
    int i, j;

    fSnapshot = UtlGetPidMultipleInstances( "winword.exe", dwWinWordNewPids );

    for (i = 0; dwWinWordNewPids[i] != 0; i ++)
    {
      j = 0;
      do
      {
        if (dwWinWordNewPids[i] != dwWinWordOldPids[j])
        {
          dwKillPid = dwWinWordNewPids[i];
          break;
        }
      } while (dwWinWordOldPids[j++] != 0);
    }

    if (!UtlKillProcess(dwKillPid))
    {
      fOk = FALSE;
      if ( pData->fMsg ) UtlErrorHwnd(ERROR_KILL_HANGING_APP, MB_CANCEL, 0, NULL, EQF_ERROR, pData->hwndParent );
    }
  }

  // Clean up...
  if (pDispDoc) pDispDoc->Release();
  if (pDispDocs) pDispDocs->Release();
  if (pDispApp) pDispApp->Release();
  if (pDispRoot) pDispRoot->Release();
  if (pUnk) pUnk->Release();

  // Uninitialize OLE Libraries...
  if (fOleInit) OleUninitialize();

  return iRC;
} /* end of function ConvertHtml2Doc */


// restore tagging of anchors and images in HTML files
void RestoreTaging
(
  PDOCEXPVALFORMATDATA pData,                    // data area
  PSZ         pszFile                         // name of file containing the output in XML format
)
{
  FILE        *hfIn = NULL;
  FILE        *hfOut = NULL;
  BOOL        fDone = FALSE;
  BOOL        fChanged = FALSE;

  // setup temp file name
  strcpy( pData->szFileName1, pszFile );
  strcat( pData->szFileName1, ".temp" );

  // open input and output file
  hfIn = fopen( pszFile, "r" );
  hfOut = fopen( pData->szFileName1, "w" );
  if ( (hfIn == NULL) || (hfOut == NULL) )
  {
    fDone = TRUE;                                // do no report errors here, leave it to calling procedure
  } /* endif */

  // read/write files until complete
  if ( !fDone )
  {
    fgets( (PSZ)pData->bTokBuffer, sizeof(pData->bTokBuffer), hfIn );

    while ( !feof(hfIn) )
    {
      PSZ pszSource = (PSZ)pData->bTokBuffer;
      PSZ pszTarget = (PSZ)pData->bTokBuffer;
      BOOL fTagClosePending = FALSE;

      while ( *pszSource )
      {
        if ( *pszSource == '&' )
        {
          if ( strncmp( pszSource, "&lt;a ", 6 ) == 0 )
          {
            *pszTarget++ = '<';
            pszSource += 4;
            fTagClosePending = TRUE;
            fChanged = TRUE;
          }
          else if ( strncmp( pszSource, "&lt;/a&gt;", 10 ) == 0 )
          {
            *pszTarget++ = '<';
            pszSource += 4;
            fTagClosePending = TRUE;
            fChanged = TRUE;
          }
          else if ( fTagClosePending && (strncmp( pszSource, "&gt;", 4 ) == 0 ) )
          {
            *pszTarget++ = '>';
            pszSource += 4;
            fTagClosePending = FALSE;
            fChanged = TRUE;
          }
          else
          {
            *pszTarget++ = *pszSource++;
          } /* endif */
        }
        else if ( *pszSource == '#' )
        {
          // ignore end delimiter of ##twb tags
          if ( strncmp( pszSource, "##twb&gt;", 9 ) == 0 )
          {
            int i = 0;
            for ( i = 0; i < 9; i++ )
            {
              *pszTarget++ = *pszSource++;
            } /* endfor */               
          }
          else
          {
            *pszTarget++ = *pszSource++;
          } /* endif */
        }
        else
        {
          *pszTarget++ = *pszSource++;
        } /* endif */
      } /*endwhile */
      *pszTarget = EOS;

      fputs( (PSZ)pData->bTokBuffer, hfOut );
      fgets( (PSZ)pData->bTokBuffer, sizeof(pData->bTokBuffer), hfIn );
    } /*endwhile */
  } /* endif */

  // cleanup
  if ( hfIn ) fclose( hfIn );
  if ( hfOut ) fclose( hfOut );

  // rename input file and rename output file
  UtlDelete( pszFile, 0L, FALSE );
  UtlMove( pData->szFileName1, pszFile, 0L, TRUE );

} /* end of function RestoreTaging */


// create output file in the requested format
static int CreateFinalOutputFile
(
  PDOCEXPVALFORMATDATA pData,                    // data area
  PSZ         pszXmlFile                         // name of file containing the output in XML format
)
{
  int iRC = 0;

  // create output directory if necessary
  if ( !iRC )
  {
    USHORT usResponse = 0;

    strcpy( pData->szBuffer, pData->szFinalOutFile );
    UtlSplitFnameFromPath( pData->szBuffer );

    if ( !UtlDirExist(pData->szBuffer) )
    {
      if ( pData->fCreateAllDirs )
      {
        usResponse = MBID_YES;
      }
      else
      {
        PSZ pszParm = pData->szBuffer + 3;
        usResponse = UtlErrorHwnd( PATH_NOTEXIST_CREATEIT, MB_EQF_YESTOALL | MB_DEFBUTTON3, 1, &pszParm, EQF_QUERY, pData->hwndParent );
        if ( usResponse == MBID_EQF_YESTOALL )
        {
          pData->fCreateAllDirs = TRUE;
          usResponse = MBID_YES;
        } /* endif */
      } /* endif */

      if ( usResponse == MBID_YES )
      {
        iRC = UtlMkMultDir( pData->szBuffer, TRUE );
      }
      else if ( usResponse == MBID_CANCEL )
      {
        iRC = ERROR_DOCEXP_CANCEL;
      }
      else
      {
        iRC = PATH_NOTEXIST_CREATEIT;
      } /* endif */
    } /* endif */
  } /* endif */

  // if output file exists, get overwrite confirmation and deelete the file
  if ( !iRC && UtlFileExist( pData->szFinalOutFile ) )
  {
    USHORT usResponse = 0;

    if ( pData->fOverwrite )
    {
      usResponse = MBID_YES;
    }
    else
    {
      PSZ pszParm = pData->szFinalOutFile;
      usResponse = UtlErrorHwnd( ERROR_FILE_EXISTS_ALREADY, MB_EQF_YESTOALL | MB_DEFBUTTON3, 1, &pszParm, EQF_WARNING, pData->hwndParent );
      if ( usResponse == MBID_EQF_YESTOALL )
      {
        pData->fOverwrite = TRUE;
        usResponse = MBID_YES;
      } /* endif */
    } /* endif */

    if ( usResponse == MBID_YES )
    {
      UtlDelete( pData->szFinalOutFile, 0L, FALSE );
    }
    else
    {
      iRC = ERROR_FILE_EXISTS_ALREADY;
    } /* endif */
  } /* endif */

  if ( !iRC )
  {
    if ( (pData->Options.ValFormat == XML_VALEXPFORMAT) || (pData->Options.ValFormat == PLAINXML_VALEXPFORMAT) )
    {
      //
      // XML output
      //

      //   rename XML file to final name
      iRC = (int)UtlSmartMoveHwnd( pszXmlFile, pData->szFinalOutFile, pData->fMsg, pData->hwndParent );

      // copy stylesheet to target directory
      if ( pData->Options.ValFormat == XML_VALEXPFORMAT )
      {
        UtlMakeEQFPath( pData->szFileName1, NULC, TABLE_PATH, NULL );
        strcat( pData->szFileName1, BACKSLASH_STR );
        if ( pData->fTransOnly ) 
           strcat( pData->szFileName1, VALXMLTOHTML_TRANSONLY_STYLESHEET );
        else
           strcat( pData->szFileName1, VALXMLTOHTML_STYLESHEET );
        strcpy( pData->szFileName2, pData->szTargetDir );
        if ( pData->fTransOnly ) 
           strcat( pData->szFileName2, VALXMLTOHTML_TRANSONLY_STYLESHEET );
        else
           strcat( pData->szFileName2, VALXMLTOHTML_STYLESHEET );
        if ( !iRC ) iRC = (int)UtlCopyHwnd( pData->szFileName1, pData->szFileName2, 1, 0L, pData->fMsg, pData->hwndParent );
      } /* endif */
    }
    else if ( (pData->Options.ValFormat == HTML_VALEXPFORMAT) || (pData->Options.ValFormat == ODT_VALEXPFORMAT) )
    {
      //
      // HTML output
      //

      // convert XML to HTML
      iRC = ConvertXml2Html( pData, pszXmlFile,  pData->szFinalOutFile );

      // delete XML temp file
      UtlDelete( pszXmlFile, 0L, FALSE );

      // do any postprocessing to restore link and image tags
      if ( pData->Options.fLinksImages )
      {
        RestoreTaging( pData, pData->szFinalOutFile );
      } /* endif */

    }
    else if ( (pData->Options.ValFormat == DOC_VALEXPFORMAT) || (pData->Options.ValFormat == DOCX_VALEXPFORMAT) )
    {
      //
      // DOC output
      //

      // convert XML to HTML
      MakeTempFile( pData->szHtmlFile, pData->szFolderDir, ".HTM" );
      iRC = ConvertXml2Html( pData, pszXmlFile,  pData->szHtmlFile );

      // do any postprocessing to restore link and image tags
      if ( pData->Options.fLinksImages )
      {
        RestoreTaging( pData, pData->szHtmlFile );
      } /* endif */

      // convert HTML to DOC
      if ( !iRC ) iRC = ConvertHtml2Doc( pData, pData->szHtmlFile, pData->szFinalOutFile, pData->Options.ValFormat );

      // set tracking flag in DOCX file
      if ( !iRC && (pData->Options.ValFormat == DOCX_VALEXPFORMAT) ) ValSetTrackingFlag( pData->szFinalOutFile );

      // delete temp files
      UtlDelete( pszXmlFile, 0L, FALSE );
      UtlDelete( pData->szHtmlFile, 0L, FALSE );
    }
    else
    {
      // not supported output format
    } /* endif */
  } /* endif */

  return iRC;
} /* end of function CreateFinalOutputFile */

// create the fully qualified file name for a document file
int MakeDocFileName
(
  PSZ         pszBuffer,                         // buffer for created file name
  PSZ         pszDocObjName,                     // document object name
  SHORT       sPathID                            // ID of the directory to use
)
{
  int iRC = 0;
  CHAR szTempBuffer[MAX_EQF_PATH];

  strcpy( szTempBuffer, pszDocObjName );
  PSZ pszDocName = UtlSplitFnameFromPath( szTempBuffer );  // cut-off document name
  UtlMakeEQFPath( pszBuffer, *pszDocObjName, sPathID, UtlGetFnameFromPath( szTempBuffer ) );
  strcat( pszBuffer, BACKSLASH_STR );
  strcat( pszBuffer, pszDocName );

  return iRC;
} /* end of function MakeDocFileName */

// load document properties file
static int LoadDocPropFile
(
  PDOCEXPVALFORMATDATA pData,                    // data area
  PSZ         pszDocObjName,                     // object name of document
  PPROPDOCUMENT *ppProp                          // ptr to caller property pointer
)
{
  int iRC = 0;
  PPROPDOCUMENT pProp = NULL;
  ULONG ulLength = 0;

  // setup document property file name
  MakeDocFileName( pData->szFileName2, pszDocObjName, PROPERTY_PATH );

  // load property file into memory
  if ( !UtlLoadFileHwnd( pData->szFileName2, (PVOID *)&pProp, &ulLength, TRUE, pData->fMsg, pData->hwndParent ) )
  {
    iRC = ERROR_FILE_NOT_FOUND;
  }
  else
  {
    *ppProp = pProp;
  } /* endif */

  return iRC;
} /* end of function LoadDocPropFile */


static int WriteDocHeader
(
  PDOCEXPVALFORMATDATA pData,                    // data area
  PPROPDOCUMENT  pProp                           // ptr to document properties
)
{
  int iRC = 0;
  LONG lTime = 0;

  pData->xw->WriteStartElement( "Header" );
  pData->xw->WriteElementString( "Folder", pData->szFolderName );
  pData->xw->WriteElementString( "DocumentName", pData->szDocName );
  time( &lTime );
  UtlLongToDateString( lTime, pData->szBuffer, sizeof(pData->szBuffer) );
  pData->xw->WriteElementString( "CreationDate", pData->szBuffer );
  if ( pData->Options.ValFormat != PLAINXML_VALEXPFORMAT)
  {
    pData->xw->WriteElementString( "Creator", "" );
    if ( pData->Options.szManager[0] != 0 )
    {
      pData->xw->WriteElementString( L"IBMPL", pData->Options.szManager );
    }
    else
    {
      pData->xw->WriteElementString( "IBMPL", "" );
    } /* endif */       
    pData->xw->WriteElementString( "Project", "" );
  } /* endif */
  pData->xw->WriteElementString( "SourceLanguage", pData->szDocSourceLang );
  pData->xw->WriteElementString( "TargetLanguage", pData->szDocTargetLang );
  pData->xw->WriteElementString( "Markup", pData->szDocFormat );
  if ( pData->Options.ValFormat != PLAINXML_VALEXPFORMAT)
  {
    pData->xw->WriteStartElement( "TotalWords" );
    pData->xw->WriteInt( (int)pProp->ulTotal );
    pData->xw->WriteEndElement();
    if ( pData->Options.szTranslator[0] != 0 )
    {
      pData->xw->WriteElementString( L"Translator", pData->Options.szTranslator );
    }
    else
    {
      pData->xw->WriteElementString( "Translator", pProp->szVendor );
    } /* endif */       
    pData->xw->WriteElementString( "Shipment", pProp->szShipment );
  } /* endif */
  pData->xw->WriteEndElement(); // "Header"

  return iRC;
} /* end of function WriteDocHeader */



// write the given text and insert soft hyphens into longer words
static int WriteTextAndInsertSoftHyphens
(
  PDOCEXPVALFORMATDATA pData,                    // data area
  PSZ_W                pszTextToWrite            // text being written
)
{
  int iRC = 0;

  // insert soft hyphens when words get too long
  PSZ_W pszText = pszTextToWrite;
  while ( *pszText )
  {
    PSZ_W pszTemp = pszText;
    int iWordLen = 0;
    BOOL fSplit = FALSE;

    while ( *pszTemp && !fSplit )
    {
      if ( iswspace( *pszTemp ) ||
//           (*pszTemp == L'.')   ||   a single point is no place for an automatic break of column text..
           (*pszTemp == L',')   ||
           (*pszTemp == L';')   ||
           (*pszTemp == L'!')   ||
           ((*pszTemp >= 0xFE00) && (*pszTemp <= 0xFF65))  ||       // Unicode punctuation characters
           (*pszTemp == L'?') )

      {
        // found word delimiter, reset word length
        iWordLen = 0;
      }
      else if ( EQFIsDBCSChar( *pszTemp, pData->ulCP ) )
      {
        // found DBCS character, data can be split here
        iWordLen = 0;
      }
      else if ( ( pData->fThai) &&
                ( *pszTemp >= 0x00FF ) )  
      {
        // found Thai character, data can be split here
        iWordLen = 0;
      }
      else if ( (iWordLen > 20) && (iWordLen < 30) )
      {
        // word limit exceeded, insert hyphen at next non-alpha character
        if ( !iswalpha( *pszTemp ) )
        {
          // split here
          fSplit = TRUE;
        }
        else
        {
          // continue search for non-apha character
          iWordLen++;
        } /* endif */
      }
      else if ( iWordLen >= 30 )
      {
        // word limit exceeded, split here without caring for non-alpha characters
        fSplit = TRUE;
      }
      else
      {
        // continue
        iWordLen++;
      } /* endif */
      if ( !fSplit )
      {
        pszTemp++;
      } /* endif */
    } /*endwhile */

    if ( fSplit )
    {
      // write text up to split position
      if ( pszTemp != pszText )
      {
        CHAR_W chTemp = *pszTemp;
        *pszTemp = 0;
        pData->xw->WriteString( pszText );
        *pszTemp = chTemp;
      } /* endif */

      // insert soft hyphen
      pData->xw->WriteRaw( L"&shy;", 5 );

      // continue at split position
      pszText = pszTemp;
    }
    else
    {
      // nothing to split, write remaining text
      pData->xw->WriteString( pszText );
      pszText += wcslen(pszText);
    } /* endif */
  } /*endwhile */
  return iRC;
} /* end of function WriteDocHeader */

// get the attribute at the current position
BOOL GetTagAttribute
(
  PSZ_W        *ppszTag,                          // data pointer  to tag data
  PSZ_W        pszName,                           // buffer for attribute name
  PSZ_W        pszData                            // buffer for attribute data
)
{
  BOOL         fAttrFound = FALSE;
  PSZ_W        pszTag = *ppszTag;

  // skip leading whitespace
  while ( iswspace(*pszTag) ) pszTag++;

  if ( (*pszTag != L'>') && (*pszTag != L'/') )
  {
    PSZ_W pszTest = pszTag;

    fAttrFound = TRUE;

    // extract attribute name
    while( iswalnum(*pszTag) || (*pszTag == L'_') || (*pszTag == L'-') ) *pszName++ = *pszTag++;

    // problems with name scanning?
    if ( pszTag == pszTest )
    {
      int iLen = 0;

      // use all characters up to the equal sign as attribute name
      while( *pszTag && (*pszTag != L'=') && (*pszTag != L'/') && (*pszTag != L'>') && (iLen < MAX_SEGMENT_SIZE) )
      {
        *pszName++ = *pszTag++;
        iLen++;
      } /*endwhile */
    } /* endif */

    // extract any attribute data
    if ( *pszTag == L'=' )
    {
      pszTag++;
      do
      {
        if ( *pszTag == L'\"' )
        {
          // add data up to closing double-quote
          pszTag++;
          while( (*pszTag != L'\"') && (*pszTag != 0))  *pszData++ = *pszTag++;
          if ( *pszTag == L'\"' ) *pszTag++;
        }
        else
        {
          // add single character
          *pszData++ = *pszTag++;
        } /* endif */
      } while ( !iswspace(*pszTag) && (*pszTag != 0) && (*pszTag != L'>') && (*pszTag != L'/') );
    } /* endif */
  } /* endif */

  // skip trailing whitespace
  while ( iswspace(*pszTag) ) pszTag++;

  // terminate buffer areas
  *pszName = 0;
  *pszData = 0;

  // skip tag end delimiter
  if ( !fAttrFound )
  {
    if ( *pszTag == L'/' ) pszTag++;
    if ( *pszTag == L'>' ) pszTag++;
  } /* endif */

  *ppszTag = pszTag;

  return( fAttrFound );
} /* end of GetTagAttribute */


// write the text of the segment
static int WriteSegmentText
(
  PDOCEXPVALFORMATDATA pData,                    // data area
  PSZ_W                pszText                   // text of segment being written
)
{
  int iRC = 0;

  if ( pData->Options.ValFormat == PLAINXML_VALEXPFORMAT)
  {
    pData->xw->WriteString( pszText );
  }
  else
  {

    // preprocess segment text
    if ( pData->Options.fNoInlineTagRemoval )
    {
      // use segment text as is
      wcscpy( pData->szSegBuffer, pszText );
    }
    else
    {
      // build protect start/stop table for tag recognition
      USHORT usRC = 0;
      PBYTE  pStartStop = NULL;
      PLOADEDTABLE pLoadedTable = (PLOADEDTABLE)pData->pSourceDoc->pDocTagTable;
      PSZ_W  pszTarget = pData->szSegBuffer;

      // build is-inline-tag-flag array
      usRC = TACreateProtectTableW( pszText, pLoadedTable, 0, (PTOKENENTRY)pData->bTokBuffer, VALTOKBUFSIZE, (PSTARTSTOP *)&pStartStop,
                                    pLoadedTable->pfnProtTable, pLoadedTable->pfnProtTableW, pData->ulCP );
      if ( !usRC )
      {
        PSTARTSTOP pEntry = (PSTARTSTOP)pStartStop;
        memset( pData->isTag, 0, sizeof(pData->isTag) );
        while ( (pEntry->usStart != 0) || (pEntry->usStop != 0)  || (pEntry->usType != 0) )
        {
          if ( pEntry->usType != UNPROTECTED_CHAR )
          {
            for( int i = pEntry->usStart; i <= pEntry->usStop; i++ )
            {
              pData->isTag[i] = 1;
            } /* endfor */
          } /* endif */
          pEntry++;
        } /* endwhile */
      } /* endif */
      UtlAlloc( (PVOID *)&pStartStop, 0L, 0L, NOMSG );
      *pszTarget = 0;

      // apply any tag replacements and suppress remaining inline tags
      ValExpApplyReplaceList( pData->pReplaceList, pszText, pData->szSegBuffer, pData->isTag, pData->Options.fLinksImages );
    } /* endif */

    // mask
    // remove any remaining softline feeds and other invalid characters in the text
    {
      PSZ_W pszSource = pData->szSegBuffer;
      PSZ_W pszTarget = pData->szSegBuffer;
      while ( *pszSource )
      {
        if ( *pszSource == 0x07 )
        {
          pszSource++;
        }
        else if ( *pszSource == 0x1A )
        {
          // replace 1A with a normal dash character
          pszSource++;
          *pszTarget++ = '-';
        }
        else
        {
          *pszTarget++ = *pszSource++;
        } /* endif */
      } /*endwhile */
      *pszTarget = 0;
    }


    if ( pData->Options.fLinksImages )
    {
      // process any  anchors and images contained in the text
      // scan for <a and <img, <content_guid; for the end anchor tag we have nothing to do here
      PSZ_W pszText = pData->szSegBuffer;
      do
      {
        enum { ANCHOR_TAG, IMAGE_TAG, CONTENT_GUID_TAG, NO_TAG } TagFound = NO_TAG;

        PSZ_W pszTag = pszText;
        do
        {
          pszTag = wcschr( pszTag, L'<' );
          if ( pszTag != NULL )
          {
            if ( wcsnicmp( pszTag, L"<a ", 3 ) == 0 )
            {
              TagFound = ANCHOR_TAG;
            }
            else if ( wcsnicmp( pszTag, L"<img ", 5 ) == 0 )
            {
              TagFound = IMAGE_TAG;
            }
            else if ( wcsnicmp( pszTag, L"<content_guid ", 14 ) == 0 )
            {
              TagFound = CONTENT_GUID_TAG;
            }
            else
            {
              pszTag++; // skip possible tag start character
            } /* endif */
          } /* endif */
        } while ( (pszTag != NULL) && (TagFound == NO_TAG) );

        if ( TagFound != NO_TAG )
        {
          // write text up to tag
          if ( pszTag != pszText )
          {
            CHAR_W c = *pszTag;
            *pszTag = 0;
            iRC = WriteTextAndInsertSoftHyphens( pData, pszText );
            *pszTag = c;
          } /* endif */

          if ( TagFound == IMAGE_TAG )
          {
            // convert image to a link
            pszTag = pszTag + 5;
            while ( *pszTag == L' ' ) pszTag++;
            while ( (*pszTag != 0) && (*pszTag != L'>' ) && (*pszTag != L'/' ))
            {
              BOOL fFound = GetTagAttribute( &pszTag, pData->szAttrName, pData->szAttrData );
              if ( fFound )
              {
                if ( wcsicmp( pData->szAttrName, L"src" ) == 0 )
                {
                  PSZ_W pszName = wcsrchr( pData->szAttrData, L'/' );
                  pszName = (pszName == NULL) ? pData->szAttrData : pszName + 1;
                  wcscpy( pData->szTagBuffer, L"<a href=\"" );
                  wcscat( pData->szTagBuffer, pData->szAttrData );
                  wcscat( pData->szTagBuffer, L"\" target=\"_blank\">" );
                  wcscat( pData->szTagBuffer, pszName );
                  wcscat( pData->szTagBuffer, L"</a>" );
                } /* endif */
              } /* endif */
            } /*endwhile */

            // write our link without softhyphen handling
            pData->xw->WriteString( pData->szTagBuffer );
          }
          else if ( TagFound == ANCHOR_TAG )
          {
            // find end of anchor and add target= attribute when not contained in anchor
            PSZ_W pszTagStart = pszTag;
            BOOL fAnchor = FALSE;
            pszTag = pszTag + 3;
            while ( *pszTag == L' ' ) pszTag++;
            while ( (*pszTag != 0) && (*pszTag != L'>' ) )
            {
              BOOL fFound = GetTagAttribute( &pszTag, pData->szAttrName, pData->szAttrData );
              if ( fFound )
              {
                if ( wcsicmp(pData->szAttrName, L"target" ) == 0 )
                {
                  fAnchor = TRUE;
                } /* endif */
              } /* endif */
            } /*endwhile */

            // copy the anchor tag to the tag buffer and add any missing target= attribute
            CHAR_W c = *pszTag;
            *pszTag = EOS;
            wcscpy( pData->szTagBuffer, pszTagStart );
            *pszTag = c;
            if ( !fAnchor )
            {
              wcscat( pData->szTagBuffer, L" target=\"_blank\"" );
            } /* endif */
            wcscat( pData->szTagBuffer, L">" );
            pData->xw->WriteString( pData->szTagBuffer );
          }
          else if ( TagFound == CONTENT_GUID_TAG )
          {
            // process any url= attribute
            PSZ_W pszTagStart = pszTag;
            pszTag = pszTag + 13;
            while ( *pszTag == L' ' ) pszTag++;
            while ( (*pszTag != 0) && (*pszTag != L'>' ) )
            {
              PSZ_W pszAttr = pszTag;
              BOOL fFound = GetTagAttribute( &pszTag, pData->szAttrName, pData->szAttrData );
              if ( fFound )
              {
                if ( wcsicmp(pData->szAttrName, L"url" ) == 0 )
                {
                  // write text up to url= attribute
                  CHAR_W c = *pszAttr;
                  *pszAttr = 0;
                  iRC = WriteTextAndInsertSoftHyphens( pData, pszTagStart );
                  *pszAttr = c;

                  // write url as anchor
                  wcscpy( pData->szTagBuffer, L" url=\"<a href=\"" );
                  wcscat( pData->szTagBuffer, pData->szAttrData );
                  wcscat( pData->szTagBuffer, L"\" target=\"_blank\">" );
                  wcscat( pData->szTagBuffer, pData->szAttrData );
                  wcscat( pData->szTagBuffer, L"</a>\" " );
                  pData->xw->WriteString( pData->szTagBuffer );

                  // shift pzzTagStart to area following url
                  pszTagStart = pszTag;
                } /* endif */
              } /* endif */
            } /*endwhile */

            // write remaining or all text
            if ( pszTag != NULL  )
            {
              CHAR_W c = *pszTag;
              *pszTag = 0;
              iRC = WriteTextAndInsertSoftHyphens( pData, pszTagStart );
              *pszTag = c;
            } /* endif */
          } /* endif */

          // continue processing right after tag data
          if ( *pszTag == L'/' ) pszTag++;
          if ( *pszTag == L'>' ) pszTag++;
          pszText = pszTag;
        }
        else
        {
          // write rest of segment text
          iRC = WriteTextAndInsertSoftHyphens( pData, pszText );
          pszText = pszText + wcslen( pszText );
        } /* endif */
      } while ( *pszText);

    }
    else
    {
      // write segments text directly
      iRC = WriteTextAndInsertSoftHyphens( pData, pData->szSegBuffer );
    } /* endif */
  } /* endif */

  return iRC;
} /* end of function WriteSegmentText */

BOOL IsFromManualTranslation
(
  PDOCEXPVALFORMATDATA pData,                    // data area
  PSZ_W    pszSource,                            // segment source
  PSZ_W    pszTarget                             // segment target
)
{
  int iEntries = pData->pvSegTable->size();
  int i = 0;
  while ( i < iEntries )
  {
    if ( (wcscmp( pszSource, (*pData->pvSegTable)[i].pszSource ) == 0 ) && (wcscmp( pszTarget, (*pData->pvSegTable)[i].pszTarget ) == 0 ) )
    {
      return( TRUE );
    }
    i++;
  }
  return( FALSE );
}

static int WriteSegmentList
(
  PDOCEXPVALFORMATDATA pData,                    // data area
  PPROPDOCUMENT  pProp,                          // ptr to document properties
  PSZ         pszDocObjName                      // object name of document
)
{
  int iRC = 0;
  BOOL fTaxonomy = FALSE;                        // TRUE = work in Taxonomy mode

  pProp;                                         // currently not used

  // load document
  {
    // allocate structure for segmented source document
    if ( !UtlAllocHwnd( (PVOID *) &pData->pSourceDoc, 0L, sizeof(TBDOCUMENT), ERROR_STORAGE, pData->hwndParent ) )
    {
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */

    // load document tag table
    if ( !iRC )
    {
      iRC = TALoadTagTableExHwnd( pData->szDocFormat, (PLOADEDTABLE *)&(pData->pSourceDoc->pDocTagTable),
                                  FALSE, TALOADUSEREXIT | TALOADGETSEGCONTEXTFUNC | TALOADPROTTABLEFUNC,
                                  pData->fMsg, pData->hwndParent  );
    } /* endif */

    // load QF tag table
    if ( !iRC )
    {
      iRC = TALoadTagTableExHwnd( QFTAG_TABLE, (PLOADEDTABLE *)&(pData->pSourceDoc->pQFTagTable),
                                  TRUE, 0, pData->fMsg, pData->hwndParent );
    } /* endif */


    // load segmented source document
    if ( !iRC )
    {
      MakeDocFileName( pData->szFileName1, pszDocObjName, DIRSEGSOURCEDOC_PATH );

      // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to SrcLang
      pData->pSourceDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pData->szDocSourceLang);
      pData->pSourceDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pData->szDocSourceLang);
      pData->ulCP =  GetLangAnsiCP( pData->szDocTargetLang );

      iRC = EQFBFileReadExW( pData->szFileName1, pData->pSourceDoc, 0 );

      // allocate structure for segmented target document
      if ( !iRC )
      {
        if ( !UtlAllocHwnd( (PVOID *) &pData->pTargetDoc, 0L, sizeof(TBDOCUMENT), ERROR_STORAGE, pData->hwndParent ) )
        {
          iRC = ERROR_NOT_ENOUGH_MEMORY;
        } /* endif */
      } /* endif */

      // load segmented target document
      if ( !iRC )
      {
        pData->pTargetDoc->pQFTagTable = pData->pSourceDoc->pQFTagTable;
        pData->pTargetDoc->pDocTagTable = pData->pSourceDoc->pDocTagTable;
        MakeDocFileName( pData->szFileName1, pszDocObjName, DIRSEGTARGETDOC_PATH );

        // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to SrcLang
        pData->pTargetDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pData->szDocTargetLang);
        pData->pTargetDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pData->szDocTargetLang);

        iRC = EQFBFileReadExW( pData->szFileName1, pData->pTargetDoc, 0 );
      } /* endif */
    } /* endif */
  }

  // check for Taxonomy mode
  if ( !iRC)
  {
    fTaxonomy = ValIsTaxonomyDoc( pData->pSourceDoc );
  } /* endif */

  // process document segments
  if ( !iRC )
  {
    ULONG ulSegNum = 1;
    ULONG ulAddSegNum = 1;
    ULONG ulActiveTable = STANDARDTABLE;
    CHAR_W szTaxCode[40];              // buffer for taxonomy codes

    pData->xw->WriteStartElement( "SegmentList" );
    wcscpy( szTaxCode, L" " );
    while ( !iRC && (ulSegNum <= pData->pSourceDoc->ulMaxSeg) )
    {
      ULONG ulTempTable = ulActiveTable;
      ULONG ulTempSeg   = ulSegNum;
      ULONG ulTempAddSeg = ulAddSegNum;

      // get segment pointers
      PTBSEGMENT pSourceSeg = EQFBGetFromBothTables( pData->pSourceDoc, &ulSegNum, &ulAddSegNum, &ulActiveTable );
      PTBSEGMENT pTargetSeg = EQFBGetFromBothTables( pData->pTargetDoc, &ulTempSeg, &ulTempAddSeg, &ulTempTable );

      // write segment entry
      if ( (pSourceSeg == NULL) || (pTargetSeg == NULL) )
      {
        // ignore not accessible segments
      }
      else if ( pTargetSeg->SegFlags.Joined && !pTargetSeg->SegFlags.JoinStart )
      {
        // ignore joinded segments
      }
      else if ( pTargetSeg->SegFlags.NoCount )
      {
        // GQ: Fix for P400573
        // ignore nocount segments
      }
      else
      {
        CHAR szFuzziness[20];
        BOOL fSelected = FALSE;

        // get any Taxonomy code
        if ( fTaxonomy )
        {
          if ( wcsnicmp( pSourceSeg->pDataW, L"<TaxoNode", 9 ) == 0 )
          {
            PSZ_W pszCode = wcsstr( pSourceSeg->pDataW + 9, L"code=\"" );
            wcscpy( szTaxCode, L" " );
            if ( pszCode )
            {
              PSZ_W pszEnd = wcschr( pszCode + 6, L'\"' );
              if ( pszEnd )
              {
                *pszEnd = 0;
                wcsncpy( szTaxCode, pszCode + 6, sizeof(szTaxCode)/sizeof(CHAR_W) );
                *pszEnd = L'\"';
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */

        strcpy( szFuzziness, "0" );                      // set default

     // avaialable count flags (as reference)
     //unsigned short AnalAutoSubst    : 1;     // Count flag
     //unsigned short EditAutoSubst    : 1;     // Count flag
     //unsigned short ExactCopy        : 1;     // Count flag
     //unsigned short MachCopy         : 1;     // Count flag
     //unsigned short FuzzyCopy        : 1;     // Count flag
     //unsigned short ReplCopy         : 1;     // Count flag
     //unsigned short ExactExist       : 1;     // Count flag
     //unsigned short MachExist        : 1;     // Count flag
     //unsigned short FuzzyExist       : 1;     // Count flag
     //unsigned short ReplExist        : 1;     // Count flag
     //unsigned short Fuzzy5070   : 1;     // Count flag
     //unsigned short Fuzzy7190   : 1;     // Count flag
     //unsigned short Fuzzy9199   : 1;     // Count flag

        // check if match type of segment has been selected for the validation document
        if ( pData->Options.fMismatchOnly )
        {
          if ( pTargetSeg->CountFlag.ExactExist && (!pTargetSeg->CountFlag.ExactCopy || pTargetSeg->CountFlag.PropChanged))
          {
            fSelected = TRUE;
          }
          else if ( pTargetSeg->CountFlag.GlobMemExist && (!pTargetSeg->CountFlag.GlobMemCopy || pTargetSeg->CountFlag.PropChanged))
          {
            fSelected = TRUE;
          }
          else if ( pTargetSeg->CountFlag.FuzzyExist && !pTargetSeg->CountFlag.FuzzyCopy )
          {
            fSelected = TRUE;
          } /* endif */
        }
        else if ( pTargetSeg->qStatus == QF_NOP)
        {
          fSelected = pData->Options.fProtMatch && !pData->Options.fExportInValidationFormat;
        }
        else if ( pTargetSeg->qStatus == QF_XLATED )
        {
          if ( pTargetSeg->CountFlag.AnalAutoSubst || pTargetSeg->CountFlag.EditAutoSubst )
          {
            if ( pTargetSeg->CountFlag.PropChanged )
            {
              fSelected = pData->Options.fModifiedAuto || pData->Options.fExportInValidationFormat;
            }
            else
            {
              fSelected = pData->Options.fAutoMatch || pData->Options.fExportInValidationFormat;
            } /* endif */
          }
          else if ( pTargetSeg->CountFlag.ExactCopy )
          {
            if ( pTargetSeg->CountFlag.PropChanged )
            {
              fSelected = pData->Options.fModifiedExact || pData->Options.fExportInValidationFormat;
            }
            else
            {
              fSelected = pData->Options.fExactMatch || pData->Options.fExportInValidationFormat;
            } /* endif */
          }
          else if ( pTargetSeg->CountFlag.FuzzyCopy )
          {
            fSelected = pData->Options.fFuzzyMatch || pData->Options.fExportInValidationFormat;
          }
          else if ( pTargetSeg->CountFlag.GlobMemCopy )
          {
            fSelected = pData->Options.fGlobMemMatch || pData->Options.fExportInValidationFormat;
          }
          else if ( pTargetSeg->CountFlag.MachCopy )
          {
            fSelected = pData->Options.fMachMatch || pData->Options.fExportInValidationFormat;
          }
          else if ( pTargetSeg->CountFlag.ReplExist )
          {
            fSelected = pData->Options.fReplMatch || pData->Options.fExportInValidationFormat;
          }
          else
          {
            fSelected = pData->Options.fNewMatch || pData->Options.fExportInValidationFormat;
          } /* endif */

          // include all global memory proposals if a global memory proposal was available...
          if ( !fSelected && pTargetSeg->CountFlag.GlobMemExist )
          {
            if ( !pTargetSeg->CountFlag.AnalAutoSubst && !pTargetSeg->CountFlag.EditAutoSubst && !pTargetSeg->CountFlag.ExactExist )
            {
              fSelected = pData->Options.fGlobMemMatch || pData->Options.fExportInValidationFormat;
            } /* endif */               
          } /* endif */

        }
        else
        {
          fSelected = pData->Options.fNotTransl || pData->Options.fExportInValidationFormat;
        } /* endif */

        if ( fSelected )
        {

          pData->xw->WriteStartElement( "Segment" );
          pData->xw->WriteStartAttribute( "Number" );
          pData->xw->WriteInt( (int)(pTargetSeg->ulSegNum));
          pData->xw->WriteEndAttribute();

          // existing match info
          if ( pData->Options.fInclExisting )
          {
            if ( pTargetSeg->CountFlag.AnalAutoSubst || pTargetSeg->CountFlag.EditAutoSubst )
            {
              strcpy( pData->szBuffer, "Exist" );
            }
            else if ( pTargetSeg->CountFlag.ExactExist )
            {
              strcpy( pData->szBuffer, "Exact" );
            }
            else if ( pTargetSeg->CountFlag.GlobMemExist )
            {
              strcpy( pData->szBuffer, "GlobalMemory" );
            }
            else if ( pTargetSeg->CountFlag.FuzzyExist )
            {
              strcpy( pData->szBuffer, "Fuzzy" );
            }
            else if ( pTargetSeg->CountFlag.MachExist )
            {
              strcpy( pData->szBuffer, "Machine" );
            }
            else if ( pTargetSeg->CountFlag.ReplExist )
            {
              strcpy( pData->szBuffer, "Replace" );
            }
            else
            {
              strcpy( pData->szBuffer, "No proposal" );
            } /* endif */
            pData->xw->WriteAttributeString( "ExistMatch", pData->szBuffer );
          } /* endif */

          if ( pTargetSeg->qStatus == QF_NOP )
          {
            strcpy( pData->szBuffer, "Protected" );
          }
          else if ( pTargetSeg->qStatus == QF_XLATED )
          {
            if ( (pTargetSeg->CountFlag.AnalAutoSubst || pTargetSeg->CountFlag.EditAutoSubst) && pTargetSeg->CountFlag.PropChanged )
            {
              strcpy( pData->szBuffer, "ModifiedAutoSubst" );
              strcpy( szFuzziness, "100" ); 
            }
            else if ( pTargetSeg->CountFlag.AnalAutoSubst || pTargetSeg->CountFlag.EditAutoSubst )
            {
              if ( pData->Options.fExactFromManual && IsFromManualTranslation( pData, pSourceSeg->pDataW, pTargetSeg->pDataW ) )
              {
                strcpy( pData->szBuffer, "ExactFromManual" );
              }
              else
              {
                strcpy( pData->szBuffer, "AutoSubst" );
              }
              strcpy( szFuzziness, "100" ); 
            }
            else if ( pTargetSeg->CountFlag.ExactCopy && pTargetSeg->CountFlag.PropChanged )
            {
              strcpy( pData->szBuffer, "ModifiedExact" );
              strcpy( szFuzziness, "100" );
            }
            else if ( pTargetSeg->CountFlag.ExactCopy )
            {
              if ( pData->Options.fExactFromManual && IsFromManualTranslation( pData, pSourceSeg->pDataW, pTargetSeg->pDataW ) )
              {
                strcpy( pData->szBuffer, "ExactFromManual" );
              }
              else
              {
                strcpy( pData->szBuffer, "Exact" );
              }
              strcpy( szFuzziness, "100" );
            }
            else if ( pTargetSeg->CountFlag.FuzzyCopy )
            {
              strcpy( pData->szBuffer, "Fuzzy" );
              if ( pTargetSeg->CountFlag.Fuzzy9199 )
              {
                strcpy( szFuzziness, "91-99" );
              }
              else if ( pTargetSeg->CountFlag.Fuzzy7190 )
              {
                strcpy( szFuzziness, "71-90" );
              }
              else if ( pTargetSeg->CountFlag.Fuzzy5070 )
              {
                strcpy( szFuzziness, "50-70" );
              } /* endif */
            }
            else if ( pTargetSeg->CountFlag.GlobMemCopy && pTargetSeg->CountFlag.PropChanged )
            {
              strcpy( pData->szBuffer, "GlobalMemoryModified" );
              strcpy( szFuzziness, "100" ); 
            }
            else if ( pTargetSeg->CountFlag.GlobMemCopy )
            {
              strcpy( pData->szBuffer, "GlobalMemory" );
              strcpy( szFuzziness, "100" ); 
            }
            else if ( pTargetSeg->CountFlag.MachCopy )
            {
              strcpy( pData->szBuffer, "Machine" );
              strcpy( szFuzziness, "100" );
            }
            else if ( pTargetSeg->CountFlag.ReplExist )
            {
              strcpy( pData->szBuffer, "Replace" );
              strcpy( szFuzziness, "100" );
            }
            else
            {
              strcpy( pData->szBuffer, "New" );
            } /* endif */
          }
          else
          {
            strcpy( pData->szBuffer, "NotTranslated" );
          } /* endif */
          pData->xw->WriteAttributeString( "Match", pData->szBuffer );

          pData->xw->WriteAttributeString( "Fuzziness", szFuzziness );

          // word count info
          if ( pData->Options.fInclCount )
          {
            sprintf( pData->szBuffer, "%u", pTargetSeg->usSrcWords );
            pData->xw->WriteAttributeString( "WordCount", pData->szBuffer );
          } /* endif */

          if ( fTaxonomy )
          {
            if ( pTargetSeg->qStatus == QF_NOP )
            {
              pData->xw->WriteAttributeString( "TaxoCode", "-" );
            }
            else
            {
              pData->xw->WriteStartAttribute( "TaxoCode" );
              pData->xw->WriteString( szTaxCode );
              pData->xw->WriteEndAttribute();
            } /* endif */
          } /* endif */

          pData->xw->WriteStartElement( "Source" );

          WriteSegmentText( pData, pSourceSeg->pDataW );

          pData->xw->WriteEndElement();   // "Source"

          pData->xw->WriteStartElement( "Target" );

          WriteSegmentText( pData, pTargetSeg->pDataW );

          pData->xw->WriteEndElement();   // "Target"

          pData->xw->WriteEndElement();   //  "Segment"
        } /* endif */
      } /* endif */
    } /*endwhile */
    pData->xw->WriteEndElement();         // "SegmentList"
  } /* endif */

  // cleanup
  if ( pData->pSourceDoc )
  {
    if ( pData->pSourceDoc->pDocTagTable ) TAFreeTagTable( (PLOADEDTABLE)pData->pSourceDoc->pDocTagTable  );
    if ( pData->pSourceDoc->pQFTagTable ) TAFreeTagTable( (PLOADEDTABLE)pData->pSourceDoc->pQFTagTable  );
    SegFileFreeDoc( (PVOID *)&(pData->pSourceDoc) );
    pData->pSourceDoc = NULL;
  } /* endif */
  if ( pData->pTargetDoc )
  {
    SegFileFreeDoc( (PVOID *)&(pData->pTargetDoc) );
    pData->pTargetDoc = NULL;
  } /* endif */

  return iRC;
} /* end of function WriteSegmentList */

//
// Eport in Validation format interface functions
//


int DocExpValInit
(
  PDOCEXPVALOPTIONS pOptions,                    // document options
  PSZ         pszDirectory,                      // target directory for output file
  PSZ         pszFolObjName,                     // folder long name, = name of output file when fCombine = TRUE
  PLONG       plHandle,                          // points to buffer for val format export handle
  BOOL        fOverwrite,                        // TRUE = overwrite files without confirmation message
  HWND        hwndParent                         // parent window handle (used as parent window for error messages)
)
{
  int         iRC = 0;
  PDOCEXPVALFORMATDATA pData = NULL;

  // check input parameter
  if ( (pOptions == NULL) || (pszDirectory == NULL) || (pszFolObjName == NULL) || (plHandle == NULL) )
  {
    iRC = ERROR_INVALID_DATA;
  } /* endif */

  // allocated data area
  if ( !iRC )
  {
    if ( !UtlAlloc( (PVOID *)&pData, 0L, sizeof(DOCEXPVALFORMATDATA), NOMSG ) )
    {
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
      *plHandle = (LONG)pData;
    } /* endif */
  } /* endif */

  // save input parameters to our data area
  if ( !iRC )
  {
    strcpy( pData->szTargetDir, pszDirectory );
    strcpy( pData->szFolObjName, pszFolObjName );
    SubFolObjectNameToName( pData->szFolObjName, pData->szFolderName );
    if ( pData->szTargetDir[strlen(pData->szTargetDir)-1] != BACKSLASH )
    {
      strcat( pData->szTargetDir, BACKSLASH_STR );
    } /* endif */
    pData->hwndParent = hwndParent;
    pData->fMsg = (pData->hwndParent != NULLHANDLE);
    strcpy( pData->szFolderDir, pData->szFolObjName );
    UtlSplitFnameFromPath( pData->szFolderDir );
    strcat( pData->szFolderDir, BACKSLASH_STR );
    memcpy( &(pData->Options), pOptions, sizeof(pData->Options) );
    pData->fOverwrite = fOverwrite;
    pData->fCreateAllDirs = (hwndParent == HWND_FUNCIF);
    pData->fTransOnly = pOptions->fTransOnly;

    pData->pvSegTable = new( std::vector<VALSEGENTRY>);
    pData->pPool = PoolCreate( 32000 );
  } /* endif */

  // in combined mode start XML output and use folder name as output file name
  if ( !iRC )
  {
    if ( pData->Options.szOutName[0] )
    {
      MakeTempFile( pData->szXmlOutFile, pData->szFolderDir, ".XML" );
      StartXmlOutput( pData, pData->szXmlOutFile );
      MakeOutFileName( pData->szFinalOutFile, pData->szTargetDir, pData->Options.szOutName, pData->Options.ValFormat, TRUE );
    }
    else if ( pData->Options.fCombined )
    {
      MakeTempFile( pData->szXmlOutFile, pData->szFolderDir, ".XML" );
      StartXmlOutput( pData, pData->szXmlOutFile );
      MakeOutFileName( pData->szFinalOutFile, pData->szTargetDir, pData->szFolderName, pData->Options.ValFormat, FALSE );
    } /* endif */
  } /* endif */

  return ( iRC );
} /* end of function DocExpValInit */

int DocExpValTerminate
(
  LONG        lHandle                            // val format export handle
)
{
  int         iRC = 0;
  PDOCEXPVALFORMATDATA pData = (PDOCEXPVALFORMATDATA)lHandle;

  // check input parameter
  if ( lHandle == 0 )
  {
    iRC = ERROR_INVALID_DATA;
  } /* endif */

  // in combined mode closed XML file and perform XML postprocessing
  if ( !iRC )
  {
    if ( pData->Options.fCombined )
    {
      iRC = EndXmlOutput( pData );
      if ( !iRC ) iRC = CreateFinalOutputFile( pData, pData->szXmlOutFile );
    } /* endif */
  } /* endif */

  // free allocated resources
  if ( pData != NULL )
  {
    if ( pData->pvSegTable != NULL ) free( pData->pvSegTable );
    if ( pData->pPool != NULL ) PoolDestroy( pData->pPool );
    UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );
  }

  return ( iRC );

} /* end of function DocExpValTerminate */

int DocExpValProcess
(
  LONG        lHandle,                            // val format export handle
  PSZ         pszDocObjName                       // object name of document being exported
)
{
  int         iRC = 0;
  ULONG       ulSrcCP, ulTgtCP ;
  PDOCEXPVALFORMATDATA pData = (PDOCEXPVALFORMATDATA)lHandle;
  PPROPDOCUMENT pProp = NULL;

  // check input parameter
  if ( (lHandle == 0) || (pszDocObjName == 0) )
  {
    iRC = ERROR_INVALID_DATA;
  } /* endif */

  // load document properties file
  if ( !iRC )
  {
    iRC = LoadDocPropFile( pData, pszDocObjName, &pProp );
  } /* endif */

  // get document info
  if ( !iRC )
  {
    DocQueryInfo2Hwnd( pszDocObjName, NULL, pData->szDocFormat, pData->szDocSourceLang, pData->szDocTargetLang,
                       pData->szDocName, NULL, NULL, pData->fMsg, pData->hwndParent );
    strcpy( pData->szDocName, pProp->szLongName[0] ? pProp->szLongName : pProp->PropHead.szName );

    ulSrcCP =  GetLangAnsiCP( pData->szDocSourceLang );
    ulTgtCP =  GetLangAnsiCP( pData->szDocTargetLang );
    if ( ( ulSrcCP == 874 ) ||   /* Set Thai processing */
         ( ulTgtCP == 874 ) ) 
       pData->fThai = TRUE;
  } /* endif */

  // get tag replacement list for current markup
  if ( !iRC )
  {
    pData->pReplaceList = ValExpGetMarkupGroup( pData->szDocFormat );
  } /* endif */

  // start XML output if not in combined mode
  if ( !iRC && !pData->Options.fCombined )
  {
    MakeTempFile( pData->szXmlOutFile, pData->szFolderDir, ".XML" );
    StartXmlOutput( pData, pData->szXmlOutFile );
    if ( pData->Options.szOutName[0] != EOS )
    {
      MakeOutFileName( pData->szFinalOutFile, pData->szTargetDir, pData->Options.szOutName, pData->Options.ValFormat, TRUE );
    }
    else
    {
      MakeOutFileName( pData->szFinalOutFile, pData->szTargetDir, pData->szDocName, pData->Options.ValFormat, FALSE );
    } /* endif */
  } /* endif */


  // start document section
  if ( !iRC )
  {
    pData->xw->WriteStartElement( "Document" );
  } /* endif */

  // write document header
  if ( !iRC )
  {
    iRC = WriteDocHeader( pData, pProp );
  } /* endif */

  // write segment list
  if ( !iRC )
  {
    iRC = WriteSegmentList( pData, pProp, pszDocObjName );
  } /* endif */

  // end document section
  if ( !iRC )
  {
    pData->xw->WriteEndElement();  // "Document"
  } /* endif */

  // in not combined mode close XML file and perform XML postprocessing
  if ( !iRC )
  {
    if ( !pData->Options.fCombined )
    {
      EndXmlOutput( pData );
      if ( CreateFinalOutputFile( pData, pData->szXmlOutFile ) == ERROR_DOCEXP_CANCEL )
      {
        iRC = ERROR_DOCEXP_CANCEL;
      } /* endif */
    } /* endif */
  } /* endif */

  // cleanup
  if ( pProp ) UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );

  return ( iRC );
} /* end of function DocExpValProcess */

int DocExpValPreScan
(
  LONG        lHandle,                            // val format export handle
  PSZ         pszDocObjName                       // object name of document being scanned
)
{
  int         iRC = 0;
  PDOCEXPVALFORMATDATA pData = (PDOCEXPVALFORMATDATA)lHandle;
  //PPROPDOCUMENT pProp = NULL;

  // get document info
  if ( !iRC )
  {
    DocQueryInfo2Hwnd( pszDocObjName, NULL, pData->szDocFormat, pData->szDocSourceLang, pData->szDocTargetLang,
                       pData->szDocName, NULL, NULL, pData->fMsg, pData->hwndParent );
  } /* endif */

  // load document
  {
    // allocate structure for segmented source document
    if ( !UtlAllocHwnd( (PVOID *) &pData->pSourceDoc, 0L, sizeof(TBDOCUMENT), ERROR_STORAGE, pData->hwndParent ) )
    {
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */

    // load document tag table
    if ( !iRC )
    {
      iRC = TALoadTagTableExHwnd( pData->szDocFormat, (PLOADEDTABLE *)&(pData->pSourceDoc->pDocTagTable),
                                  FALSE, TALOADUSEREXIT | TALOADGETSEGCONTEXTFUNC | TALOADPROTTABLEFUNC,
                                  pData->fMsg, pData->hwndParent  );
    } /* endif */

    // load QF tag table
    if ( !iRC )
    {
      iRC = TALoadTagTableExHwnd( QFTAG_TABLE, (PLOADEDTABLE *)&(pData->pSourceDoc->pQFTagTable),
                                  TRUE, 0, pData->fMsg, pData->hwndParent );
    } /* endif */

    // load segmented source document
    if ( !iRC )
    {
      MakeDocFileName( pData->szFileName1, pszDocObjName, DIRSEGSOURCEDOC_PATH );

      // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to SrcLang
      pData->pSourceDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pData->szDocSourceLang);
      pData->pSourceDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pData->szDocSourceLang);
      pData->ulCP =  GetLangAnsiCP( pData->szDocTargetLang );

      iRC = EQFBFileReadExW( pData->szFileName1, pData->pSourceDoc, 0 );

      // allocate structure for segmented target document
      if ( !iRC )
      {
        if ( !UtlAllocHwnd( (PVOID *) &pData->pTargetDoc, 0L, sizeof(TBDOCUMENT), ERROR_STORAGE, pData->hwndParent ) )
        {
          iRC = ERROR_NOT_ENOUGH_MEMORY;
        } /* endif */
      } /* endif */

      // load segmented target document
      if ( !iRC )
      {
        pData->pTargetDoc->pQFTagTable = pData->pSourceDoc->pQFTagTable;
        pData->pTargetDoc->pDocTagTable = pData->pSourceDoc->pDocTagTable;
        MakeDocFileName( pData->szFileName1, pszDocObjName, DIRSEGTARGETDOC_PATH );

        // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to SrcLang
        pData->pTargetDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pData->szDocTargetLang);
        pData->pTargetDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pData->szDocTargetLang);

        iRC = EQFBFileReadExW( pData->szFileName1, pData->pTargetDoc, 0 );
      } /* endif */
    } /* endif */
  }

  // scan document segments
  if ( !iRC )
  {
    ULONG ulSegNum = 1;
    ULONG ulAddSegNum = 1;
    ULONG ulActiveTable = STANDARDTABLE;

    while ( !iRC && (ulSegNum <= pData->pSourceDoc->ulMaxSeg) )
    {
      ULONG ulTempTable = ulActiveTable;
      ULONG ulTempSeg   = ulSegNum;
      ULONG ulTempAddSeg = ulAddSegNum;

      // get segment pointers
      PTBSEGMENT pSourceSeg = EQFBGetFromBothTables( pData->pSourceDoc, &ulSegNum, &ulAddSegNum, &ulActiveTable );
      PTBSEGMENT pTargetSeg = EQFBGetFromBothTables( pData->pTargetDoc, &ulTempSeg, &ulTempAddSeg, &ulTempTable );

      // write segment entry
      if ( (pSourceSeg == NULL) || (pTargetSeg == NULL) )
      {
        // ignore not accessible segments
      }
      else if ( pTargetSeg->SegFlags.Joined && !pTargetSeg->SegFlags.JoinStart )
      {
        // ignore joinded segments
      }
      else if ( pTargetSeg->SegFlags.NoCount )
      {
        // GQ: Fix for P400573
        // ignore nocount segments
      }
      else
      {
        BOOL fManuallyTranslated = FALSE;

        if ( pTargetSeg->qStatus == QF_XLATED )
        {
          if ( pTargetSeg->CountFlag.AnalAutoSubst || pTargetSeg->CountFlag.ExactCopy || pTargetSeg->CountFlag.FuzzyCopy  ||
               pTargetSeg->CountFlag.GlobMemCopy || pTargetSeg->CountFlag.MachCopy || pTargetSeg->CountFlag.EditAutoSubst )
          {
            fManuallyTranslated = pTargetSeg->CountFlag.PropChanged;
          }
          else
          {
            fManuallyTranslated = TRUE;
          } /* endif */
        } /* endif */

        if ( fManuallyTranslated )
        {
          // add source and translation to our segment list
          VALSEGENTRY NewEntry;
          NewEntry.pszTarget = PoolAddStringW( pData->pPool, pTargetSeg->pDataW );
          NewEntry.pszSource = PoolAddStringW( pData->pPool, pSourceSeg->pDataW );
          pData->pvSegTable->push_back( NewEntry );
        } /* endif */
      } /* endif */
    } /*endwhile */
  } /* endif */

  // cleanup
  if ( pData->pSourceDoc )
  {
    if ( pData->pSourceDoc->pDocTagTable ) TAFreeTagTable( (PLOADEDTABLE)pData->pSourceDoc->pDocTagTable  );
    if ( pData->pSourceDoc->pQFTagTable ) TAFreeTagTable( (PLOADEDTABLE)pData->pSourceDoc->pQFTagTable  );
    SegFileFreeDoc( (PVOID *)&(pData->pSourceDoc) );
    pData->pSourceDoc = NULL;
  } /* endif */
  if ( pData->pTargetDoc )
  {
    SegFileFreeDoc( (PVOID *)&(pData->pTargetDoc) );
    pData->pTargetDoc = NULL;
  } /* endif */

  return ( iRC );
} /* end of function DocExpPreScan */

//
// load balance list creation
//

typedef struct _TABALANCELISTDATA
{
  CHAR        szFolObjName[MAX_EQF_PATH];        // folder object name
  CHAR        szFolPropName[MAX_EQF_PATH];       // folder properties name
  CHAR        szFolderLongName[MAX_LONGFILESPEC];// folder long name
  CHAR        szFolderShortName[MAX_FILESPEC];    // folder short name
  CHAR        szSubFolObjName[MAX_EQF_PATH];     // folder object name
  CHAR        szMarkup[MAX_FILESPEC];            // markup of folder
  CHAR        szMemory[MAX_LONGFILESPEC];        // folder's memory db
  CHAR        szSourceLang[60];                  // folder source language
  CHAR        szTargetLang[60];                  // folder target language
  CHAR        szBalanceFile[MAX_LONGFILESPEC];   // fully qualified balance file name
  int         iNumOfDocs;                        // number of documents in the folder
  ULONG       ulTotalWords;                      // total number of words in documents
  ULONG       ulTotalUntranslated;               // total number of untranslated words
  PVOID       pvXW;                              // active XmlWriter object
  PSZ         pszDocList;                        // ptr to list of documents
  CHAR        szBuffer[4096];                    // general use working area
} TABALANCELISTDATA, *PTABALANCELISTDATA;


// write folder info part to balance file
int TABalanceWriteFolderInfo
(
  PTABALANCELISTDATA pData
)
{
  int iRC = 0;
  PPROPFOLDER pProp = NULL;
  CXmlWriter *xw = (CXmlWriter *)pData->pvXW;

  // setup property file name
  UtlMakeEQFPath( pData->szFolPropName, NULC, PROPERTY_PATH, NULL );
  strcat( pData->szFolPropName, BACKSLASH_STR );
  strcat( pData->szFolPropName, UtlGetFnameFromPath( pData->szFolObjName ) );

  // load properties into memory
  {
    ULONG ulPropLen = 0;

    // load document property file
    if ( !UtlLoadFileL( pData->szFolPropName, (PVOID *)&pProp, &ulPropLen, FALSE, FALSE ) )
    {
      iRC = ERROR_PROPERTY_ACCESS;
    }
  }

  // write folder info
  if ( pProp )
  {
    xw->WriteAttributeString( "Type", "Original" );

    xw->WriteElementString( "Name", pData->szFolderLongName );

    sprintf( pData->szBuffer, "%ld", pData->iNumOfDocs );
    xw->WriteElementString( "TotalDocuments", pData->szBuffer );

    sprintf( pData->szBuffer, "%lu", pData->ulTotalWords );
    xw->WriteElementString( "TotalWords", pData->szBuffer );

    sprintf( pData->szBuffer, "%lu", pData->ulTotalUntranslated );
    xw->WriteElementString( "UntranslatedWords", pData->szBuffer );

    xw->WriteElementString( "Markup", pData->szMarkup );

    xw->WriteElementString( "Memory", pData->szMemory );

    sprintf( pData->szBuffer, "%c", pProp->chDrive );
    xw->WriteElementString( "Drive", pData->szBuffer );

    xw->WriteElementString( "Editor", pProp->szEditor );

    strcpy( pData->szSourceLang, pProp->szSourceLang );
    xw->WriteElementString( "SourceLanguage", pProp->szSourceLang );

    strcpy( pData->szTargetLang, pProp->szTargetLang );
    xw->WriteElementString( "TargetLanguage", pProp->szTargetLang );

    if ( pProp->aLongDicTbl[0][0] != EOS )
    {
      xw->WriteStartElement( "DictionaryList" );

      int i = 0;
      while ( (i < MAX_NUM_OF_FOLDER_DICS) && (pProp->aLongDicTbl[i][0] != EOS) )
      {
        xw->WriteElementString( "Name", pProp->aLongDicTbl[i] );
        i++;
      }     /*end while */
      xw->WriteEndElement();   // DictionaryList
    }
    else if (pProp->DicTbl[0] != EOS)
    {
      PSZ pszTemp = pProp->DicTbl;
      PSZ pszEnd = NULL;

      xw->WriteStartElement( "DictionaryList" );

      do
      {
        pszEnd = strchr( pszTemp, X15 );
        if ( pszEnd ) *pszEnd = EOS;
        xw->WriteElementString( "Name", pszTemp );
        if ( pszEnd ) *pszEnd = X15;
        if ( pszEnd ) pszTemp = pszEnd + 1;
      } while ( pszEnd );

      xw->WriteEndElement();   // DictionaryList
    } /* endif */


    if ( pProp->aLongMemTbl[0][0] != EOS )
    {
      int i = 0;
      xw->WriteStartElement( "SearchMemoryList" );
      while ( (i < MAX_NUM_OF_READONLY_MDB) && (pProp->aLongMemTbl[i][0] != EOS) )
      {
        xw->WriteElementString( "Name", pProp->aLongMemTbl[i] );
        i++;
      } /* endwhile */
      xw->WriteEndElement();                  // SearchMemoryList
    }
    else if ( pProp->MemTbl[0] != EOS )
    {
      PSZ pszTemp = pProp->MemTbl;
      PSZ pszEnd = NULL;

      xw->WriteStartElement( "SearchMemoryList" );

      do
      {
        pszEnd = strchr( pszTemp, X15 );
        if ( pszEnd ) *pszEnd = EOS;
        xw->WriteElementString( "Name", pszTemp );
        if ( pszEnd ) *pszEnd = X15;
        if ( pszEnd ) pszTemp = pszEnd + 1;
      } while ( pszEnd );
      xw->WriteEndElement();                  // SearchMemoryList
    } /* endif */
  } /* endif */

  // cleanup
  if ( pProp )  UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );

  return( iRC );
} /* end of function TABalanceWriteFolderInfo */


// write document info part to balance file
int TABalanceWriteDocInfo
(
  PTABALANCELISTDATA pData,                      // our data area
  PPROPDOCUMENT      pProp                       // document properties
)
{
  int iRC = 0;
  CXmlWriter *xw = (CXmlWriter *)pData->pvXW;

  xw->WriteStartElement( "Document" );

  // write document path, name and extension
  {
    PSZ pszName = ( pProp->szLongName[0] ) ? pProp->szLongName : pProp->PropHead.szName;
    PSZ pszPath = NULL;
    strcpy( pData->szBuffer, pszName );
    pszName = pData->szBuffer;

    xw->WriteElementString( "FullName", pszName );

    PSZ pszNamePos = strrchr( pszName,  BACKSLASH );
    if ( pszNamePos )
    {
      pszPath = pszName;
      pszName = pszNamePos + 1;
      *pszNamePos = EOS;
    } /* endif */
    PSZ pszExtension = strrchr( pszName, DOT );

    if ( pszPath ) xw->WriteElementString( "Path", pszPath );
    xw->WriteElementString( "Name", pszName );
    if ( pszExtension ) xw->WriteElementString( "Extension", pszExtension + 1 );
  }

  // show any subfolder name for document
  if ( pProp->ulParentFolder )
  {
    SubFolIdToObjectName( pProp->PropHead.szPath, pProp->ulParentFolder, pData->szSubFolObjName );
    SubFolObjectNameToName( pData->szSubFolObjName, pData->szBuffer );
    xw->WriteElementString( "SubFolder", pData->szBuffer );
  } /* endif */

  sprintf( pData->szBuffer, "%lu", pProp->ulTotal );
  xw->WriteElementString( "TotalWords", pData->szBuffer );

  sprintf( pData->szBuffer, "%lu", pProp->ulNotTranslated );
  xw->WriteElementString( "UntranslatedWords", pData->szBuffer );

  sprintf( pData->szBuffer, "%u", pProp->usComplete );
  xw->WriteElementString( "CompletionRate", pData->szBuffer );

  if ( pProp->szVendor[0] ) xw->WriteElementString( "Translator", pProp->szVendor );

  xw->WriteElementString( "Markup", pProp->szFormat[0] ? pProp->szFormat : pData->szMarkup );

  // document size
  {
    char        szBuffer[MAX_EQF_PATH];
    FILEFINDBUF stFile;
    USHORT      usCount = 1;
    HDIR        hSearch = HDIR_CREATE;

    strcpy( szBuffer, pProp->PropHead.szPath );
    strcat( szBuffer, BACKSLASH_STR );
    UtlQueryString( QST_SOURCEDIR, szBuffer + strlen(szBuffer), MAX_FILESPEC );
    strcat( szBuffer, BACKSLASH_STR );
    strcat( szBuffer, pProp->PropHead.szName );

    memset( &stFile, 0, sizeof(stFile) );
    UtlFindFirst( szBuffer, &hSearch, 0, &stFile, sizeof(stFile), &usCount, 0L, FALSE );
    UtlFindClose( hSearch, FALSE );

    sprintf( pData->szBuffer, "%ld", stFile.nFileSizeLow );
    xw->WriteElementString( "Size", pData->szBuffer );
  }

  xw->WriteEndElement();                         // Document

  return( iRC );
} /* end of function TABalanceWriteDocInfo */



// main function for balance list creation
int TABalanceList
(
  PSZ         pszFolObjName,                     // folder object name
  PSZ         pszDocs,                           // ptr to list of documents
  USHORT      usFiles                            // number of source files
)
{
  int         iRC = 0;                           // function return code
  PTABALANCELISTDATA pData = NULL;               // ptr to our data area
  PSZ         pDocNameBuffer = NULL;             // document name buffer
  CXmlWriter *xw = NULL;                         // ptr to XmlWriter object

  // check input parameter
  if ( pszFolObjName == NULL )
  {
    iRC = ERROR_INVALID_DATA;
  } /* endif */

  // allocated data area
  if ( !iRC )
  {
    if ( !UtlAlloc( (PVOID *)&pData, 0L, sizeof(TABALANCELISTDATA ), NOMSG ) )
    {
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // get folder data, setup file names
  if ( !iRC )
  {
    pData->iNumOfDocs = (int)usFiles;
    pData->pszDocList = pszDocs;

    strcpy( pData->szFolObjName, pszFolObjName );
    SubFolObjectNameToName( pData->szFolObjName, pData->szFolderLongName );
    strcpy( pData->szFolderShortName, UtlGetFnameFromPath( pData->szFolObjName ) );

    UtlMakeEQFPath( pData->szBalanceFile, NULC, SYSTEM_PATH, NULL );
    strcat( pData->szBalanceFile, "\\LOADBALANCE\\" );
    strcat( pData->szBalanceFile, pData->szFolderLongName );
    strcat( pData->szBalanceFile, ".XML" );

    FolQueryInfo2( pData->szFolObjName, pData->szMemory, pData->szMarkup, NULL, NULL, NULL, FALSE );
  } /* endif */

  // start XML output
  if ( !iRC )
  {
    xw = new CXmlWriter( pData->szBalanceFile );
    xw->Formatting = CXmlWriter::Indented;
    xw->Encoding = CXmlWriter::UTF8;
    xw->Indention = 2;

    xw->WriteStartDocument();

    //xw->WriteStylesheet( "matchlist.xsl" );
    xw->WriteStartElement( "LoadBalance" );

    pData->pvXW = (PVOID)xw;
  } /* endif */

  // writer header info
  if ( !iRC )
  {
    LONG lTime = 0;

    xw->WriteStartElement( "Header" );

    xw->WriteElementString( "Generator", "OpenTM2" );

    sprintf( pData->szBuffer, "%d.%d.%d", EQF_DRIVER_VERSION, EQF_DRIVER_RELEASE, EQF_DRIVER_SUBRELEASE );
    xw->WriteElementString( "GeneratorVersion", pData->szBuffer );

    time( &lTime );
    UtlLongToDateString( lTime, pData->szBuffer, sizeof(pData->szBuffer) );
    xw->WriteElementString( "GeneratedDate", pData->szBuffer );

    UtlLongToTimeString( lTime, pData->szBuffer, sizeof(pData->szBuffer) );
    xw->WriteElementString( "GeneratedTime", pData->szBuffer );

    xw->WriteEndElement();                       // Header

  } /* endif */

  // loop documents to compute total number of untranslated words
  if ( !iRC )
  {
    PSZ pszCurDoc = pData->pszDocList;
    int iDocItem = 0;
    while ( !iRC && (iDocItem < pData->iNumOfDocs) )
    {
      CHAR szDocProp[MAX_EQF_PATH];
      PPROPDOCUMENT pDocProp = NULL;
      ULONG ulPropLen = 0L;                    // size of loaded doc property file

      // setup document property file name
      UtlMakeEQFPath( szDocProp, pData->szFolObjName[0], PROPERTY_PATH, pData->szFolderShortName );
      strcat( szDocProp, BACKSLASH_STR );
      strcat( szDocProp, pszCurDoc );

      // load document property file
      if ( UtlLoadFileL( szDocProp, (PVOID *)&pDocProp, &ulPropLen, FALSE, FALSE ) )
      {
        pData->ulTotalWords        += pDocProp->ulTotal;
        pData->ulTotalUntranslated += pDocProp->ulNotTranslated;
        UtlAlloc( (PVOID *)&pDocProp, 0L, 0L, NOMSG );
      }
      else
      {
        // TODO: error handling: load of document property file failed
      } /* endif */

      // next document
      pszCurDoc += strlen(pszCurDoc) + 1;
      iDocItem++;
    } /*endwhile */
  } /* endif */

  // writer folder info
  if ( !iRC )
  {
    xw->WriteStartElement( "Folder" );
    iRC = TABalanceWriteFolderInfo( pData );
  } /* endif */

  // write document info(s)
  if ( !iRC )
  {
    PSZ pszCurDoc = pData->pszDocList;
    int iDocItem = 0;
    xw->WriteStartElement( "DocumentList" );
    while ( !iRC && (iDocItem < pData->iNumOfDocs) )
    {
      CHAR szDocProp[MAX_EQF_PATH];
      PPROPDOCUMENT pDocProp = NULL;
      ULONG ulPropLen = 0L;                    // size of loaded doc property file

      // setup document property file name
      UtlMakeEQFPath( szDocProp, pData->szFolObjName[0], PROPERTY_PATH, pData->szFolderShortName );
      strcat( szDocProp, BACKSLASH_STR );
      strcat( szDocProp, pszCurDoc );

      // load document property file
      if ( UtlLoadFileL( szDocProp, (PVOID *)&pDocProp, &ulPropLen, FALSE, FALSE ) )
      {
        iRC = TABalanceWriteDocInfo( pData, pDocProp );
        UtlAlloc( (PVOID *)&pDocProp, 0L, 0L, NOMSG );
      }
      else
      {
        // TODO: error handling: load of document property file failed
      } /* endif */

      // next document
      pszCurDoc += strlen(pszCurDoc) + 1;
      iDocItem++;
    } /*endwhile */
    xw->WriteEndElement();                         // DocumentList
    xw->WriteEndElement();                         // Folder
  } /* endif */

  // terminate XML output
  if ( xw != NULL )
  {
    xw->WriteEndElement(); // LoadBalance
    xw->WriteEndDocument();
    xw->Close();
    delete xw;
  } /* endif */

  // cleanup
  if ( pData ) UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );
  if ( pDocNameBuffer ) UtlAlloc( (PVOID *)&pDocNameBuffer, 0L, 0L, NOMSG );

  return( 0 );
} /* end of function TABalanceList */



// helper functions

// remove CR/LF at the end of the line
static void StripCRLF( PSZ_W pszLine )
{
  int iLen = wcslen( pszLine );
  while( iLen && ( (pszLine[iLen-1] == L'\n') || (pszLine[iLen-1] == L'\r') ) )
  {
    iLen--;
    pszLine[iLen] = 0;
  } /* endwhile */
}

// remove BOM at the begin of the line
static void StripBOM( PSZ_W pszLine )
{
  if ( memcmp( pszLine, UNICODEFILEPREFIX, 2 ) == 0 )
  {
    PSZ_W pszSource = pszLine + 2;
    PSZ_W pszTarget = pszLine;
    while ( *pszSource )
    {
      *pszTarget++ = *pszSource++;
    } /*endwhile */
    *pszTarget = 0;
  } /* endif */
}


// remove leading and trailing blanks
static void StripBlanks( PSZ_W pszString )
{
  PSZ_W  pszSource;                    // source position pointer
  PSZ_W  pszTarget;                    // target position pointer
  PSZ_W  pszLastNonBlank;              // last non-blank position

  /* Skip leading blanks                                              */
  pszSource = pszTarget = pszString;
  pszLastNonBlank = pszString - 1;
  while ( *pszSource == BLANK ) pszSource++;

  /* Copy remaining characters                                        */
  while ( *pszSource )
  {
    if ( *pszSource != BLANK )
    {
      pszLastNonBlank = pszTarget;
    } /* endif */
    *pszTarget++ = *pszSource++;
  } /* endwhile */

  /* Truncate string at last non-blank position                       */
  pszLastNonBlank++;
  *pszLastNonBlank = EOS;
}

// add a replace entry to the replace list table
BOOL ValExpAddReplaceEntry( PVALDOCREPLACELIST *ppReplList, PSZ_W pszReplEntry, PSZ_W pszValue )
{
  int    iMU = 1;
  BOOL   fOK = TRUE;                             // internal OK flag
  PVALDOCREPLACELIST pReplList = *ppReplList;
  PVALDOCREPLACEMU pMU = &(pReplList->MarkupGroup);

  // return asap when there are no markup groups in the list
  if ( pReplList->iEntries == 0 )
  {
    return( FALSE );
  } /* endif */

  // enlarge replacement list area
  fOK = UtlAlloc( (PVOID *)&pReplList, pReplList->iSize, pReplList->iSize + sizeof(VALDOCREPLACEENTRY), ERROR_STORAGE );

  // position to last (=active) markup group in replacement list
  if ( fOK )
  {
    pMU = &(pReplList->MarkupGroup);
    while ( iMU < pReplList->iEntries )
    {
      pMU = (PVALDOCREPLACEMU)( ((PBYTE)pMU) + pMU->iSize);
      iMU++;
    } /*endwhile */
  } /* endif */

  // add new entry
  if ( fOK )
  {
    wcsncpy( pMU->Entry[pMU->iEntries].szTo, pszValue, (sizeof(pMU->Entry[pMU->iEntries].szTo)/ sizeof(CHAR_W))-1 );
    wcsncpy( pMU->Entry[pMU->iEntries].szFrom, pszReplEntry, (sizeof(pMU->Entry[pMU->iEntries].szFrom) / sizeof(CHAR_W)) - 1   );
    pMU->Entry[pMU->iEntries].iLen = wcslen( pszReplEntry );
  } /* endif */

  // adjust sizes and counters
  if ( fOK )
  {
    pMU->iEntries++;
    pMU->iSize += sizeof(VALDOCREPLACEENTRY);
    pReplList->iSize += sizeof(VALDOCREPLACEENTRY);
  } /* endif */

  *ppReplList = pReplList;

  return( fOK );
} /* end of function ValExpAddReplaceEntry */

// start a new markup group within the replace list table
BOOL ValExpStartMarkupGroup( PVALDOCREPLACELIST *ppReplList, PSZ_W pszMarkup )
{
  BOOL   fOK = TRUE;                             // internal OK flag
  PVALDOCREPLACELIST pReplList = *ppReplList;
  PVALDOCREPLACEMU pMU = NULL;

  // resize replacement list
  int iOldSize = ( pReplList == NULL ) ? 0 : pReplList->iSize;
  int iNewSize = ( iOldSize ) ? (iOldSize + sizeof(VALDOCREPLACEMU)) : sizeof(VALDOCREPLACELIST);
  fOK = UtlAlloc( (PVOID *)&pReplList, iOldSize, iNewSize, ERROR_STORAGE );

  // adjust sizes
  if ( fOK )
  {
    pReplList->iEntries++;
    pReplList->iSize = iNewSize;
  } /* endif */

  // position to last markup group in replacement list
  if ( fOK )
  {
    int iMU = 1;
    pMU = &(pReplList->MarkupGroup);
    while ( iMU < pReplList->iEntries )
    {
      pMU = (PVALDOCREPLACEMU)( ((PBYTE)pMU) + pMU->iSize);
      iMU++;
    } /*endwhile */
  } /* endif */

  // fill entry
  if ( fOK )
  {
    int iMutNames = 0;
    PSZ_W  pszMUNameEnd = NULL;
    pMU->iEntries = 0;
    pMU->iSize = sizeof(VALDOCREPLACEMU);
    memset( pMU->szMarkup, 0, sizeof(pMU->szMarkup) );
    do
    {
      pszMUNameEnd = wcschr( pszMarkup, L',' );
      if ( pszMUNameEnd != NULL ) *pszMUNameEnd = 0;
      WideCharToMultiByte( CP_OEMCP, 0, pszMarkup, -1, pMU->szMarkup[iMutNames], sizeof(pMU->szMarkup[iMutNames]), NULL, NULL );
      pszMarkup = pszMUNameEnd + 1;
      iMutNames++;
    } while ( (pszMUNameEnd != NULL) && (iMutNames < MUT_NAME_ENTRIES) );
  } /* endif */

  *ppReplList = pReplList;

  return( fOK );
} /* end of function ValExpStartMarkupGroup */

//
// Load validation document export tag replacement list into memory
// if not already loaded into memory
//
PVALDOCREPLACELIST ValExpGetReplacementList()
{
  BOOL   fOK = TRUE;                             // internal OK flag
  FILE   *hfList = NULL;                         // handle of list file

  // return asap if the replacement has been loaded already
  if ( (pReplList != NULL) || fReplListLoadError )
  {
    return( pReplList );
  } /* endif */

  // open replacement list file
  {
    CHAR  szFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szFile, NULC, TABLE_PATH, NULL );
    strcat( szFile, "\\VALDOCREPL.LST" );

    hfList = fopen( szFile, "rb" );

    if ( hfList == NULL )
    {
      // open failed...
      fOK = FALSE;
      fReplListLoadError = TRUE;
    } /* endif */
  }

  // read and process file until complete
  if ( fOK )
  {
    static CHAR_W szLine[1024];

    fgetws( szLine, sizeof(szLine), hfList );

    StripBOM( szLine );

    while ( fOK && !feof(hfList) )
    {
      // remove CR/LF at the end of the line
      StripCRLF( szLine );

      // ignore comment lines, process all other lines
      if ( szLine[0] != L'*' )
      {
        StripBlanks( szLine );
        if ( szLine[0] == 0 )
        {
          // ignore empty lines
        }
        else if ( szLine[0] == L'[' )
        {
          // isolate markup table name and start a new markup group
          PSZ_W pszEnd = wcschr( szLine, L']' );
          if ( pszEnd != NULL )
          {
            *pszEnd = 0;

            fOK = ValExpStartMarkupGroup( &pReplList, szLine + 1 );
          } /* endif */

        }
        else
        {
          // handle replace line
          PSZ_W pValue = wcschr( szLine, L'=' );
          if ( pValue )
          {
            *pValue = 0;
            pValue++;
            StripBlanks( pValue );
            if ( pValue[0] == L'\'' )
            {
              PSZ_W pszEnd = wcschr( pValue + 1, L'\'' );
              if ( pszEnd )
              {
                *pszEnd = 0;
                fOK = ValExpAddReplaceEntry( &pReplList, szLine, pValue + 1 );
              } /* endif */
            }
            else if ( pValue[0] == L'\"' )
            {
              PSZ_W pszEnd = wcschr( pValue + 1, L'\"' );
              if ( pszEnd )
              {
                *pszEnd = 0;
                fOK = ValExpAddReplaceEntry( &pReplList, szLine, pValue + 1 );
              } /* endif */
            }
            else if ( iswdigit( pValue[0] ) )
            {
              CHAR_W szValue[2];
              szValue[0] = (CHAR_W)_wtoi( pValue );
              szValue[1] = 0;
              if ( szValue[0] )
              {
                fOK = ValExpAddReplaceEntry( &pReplList, szLine, szValue );
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */

      } /* endif */
      // continue with next line
      fgetws( szLine, sizeof(szLine), hfList );
    } /*endwhile */
  } /* endif */

  // close replacement list file
  if ( hfList ) fclose( hfList );

  fReplListLoadError = !fOK;

  return( pReplList );
} /* end of function ValExpGetReplacementList */

// find the markup group belonging to the specified markup table
PVALDOCREPLACEMU ValExpGetMarkupGroup( PSZ pszMarkup )
{
  PVALDOCREPLACELIST pReplList = NULL;
  PVALDOCREPLACEMU pMU = NULL;

  // get replacement list
  pReplList = ValExpGetReplacementList();

  // find markup table group
  if ( pReplList && (pReplList->iEntries != 0))
  {
    int iMU = 1;
    PVALDOCREPLACEMU pTempMU = &(pReplList->MarkupGroup);
    while ( (iMU < pReplList->iEntries) && (pMU == NULL) )
    {
      BOOL fFound = FALSE;
      int iMutName = 0;

      while ( !fFound && (iMutName < MUT_NAME_ENTRIES) && (pTempMU->szMarkup[iMutName][0] != EOS) )
      {
        UtlMatchStrings( pszMarkup, pTempMU->szMarkup[iMutName], &fFound );
        iMutName++;
      } /*endwhile */

      if ( fFound )
      {
        pMU = pTempMU;
      }
      else
      {
        pTempMU = (PVALDOCREPLACEMU)( ((PBYTE)pTempMU) + pTempMU->iSize);
        iMU++;
      } /* endif */
    } /*endwhile */
  } /* endif */

  return( pMU );
} /* end of function ValExpGetMarkupGroup */

// helper function to check for character entities of type "&#ddd;"

BOOL isCharEntity( PSZ_W pszTest, CHAR_W *pValue, int *pLen )
{
  BOOL   fEntity = FALSE;
  *pValue = 0;
  *pLen = 0;

  if ( (pszTest[0] == L'&') && (pszTest[1] == L'#'))
  {
    int iLen = 2;
    while ( (pszTest[iLen] >= L'0') && (pszTest[iLen] <= L'9'))
    {
      iLen++;
    } /*endwhile */

    if ( (pszTest[iLen] == L';') && (iLen > 2))
    {
      fEntity = TRUE;
      *pLen = iLen + 1;
      *pValue = (CHAR_W)_wtol( pszTest + 2 );
    } /* endif */
  } /* endif */

  return( fEntity );
}

// apply the replacement list on the given string
BOOL ValExpApplyReplaceList( PVALDOCREPLACEMU pMU, PSZ_W pszInString, PSZ_W pszOutString, PBOOL isTag, BOOL fLinksImages )
{
  BOOL   fOK = TRUE;                             // internal OK flag
  int iInPos = 0;

  // remove tagging info for links and images when links and images should be preserved
  // the tags to preserve are "<a...>", "</a>", and "<img....>"
  if ( fLinksImages )
  {
    PSZ_W pszPos = pszInString;

    // loop over the string
    while ( *pszPos  != 0 )
    {
      BOOL fFound = FALSE;
      PSZ_W pszTagEnd = NULL;

      // check for link or image tag
      if ( *pszPos == L'<' )
      {
        fFound = ( (wcsnicmp( pszPos, L"<a ", 3 ) == 0) || (wcsnicmp( pszPos, L"</a", 3 ) == 0) ||
                   (wcsnicmp( pszPos, L"<img ", 5 ) == 0) );
      } /* endif */

      // search end of tag
      if ( fFound )
      {
        pszTagEnd = pszPos + 1;
        while ( (*pszTagEnd != 0) && (*pszTagEnd != L'>') )
        {
          // skip parts enclosed in double quotes
          if ( *pszTagEnd == L'\"')
          {
            pszTagEnd++;
            while ( (*pszTagEnd != 0) && (*pszTagEnd != L'\"') )
            {
              pszTagEnd++;
            } /*endwhile */
          } /* endif */

          if ( *pszTagEnd != 0)
          {
            pszTagEnd++;
          } /* endif */
        } /*endwhile */
      } /* endif */

      // remove tagging info from tag or test next character
      if ( fFound && pszTagEnd )
      {
        int iStartPos = pszPos - pszInString;
        int iEndPos = pszTagEnd - pszInString;
        while ( iStartPos <= iEndPos )
        {
          isTag[iStartPos++] = FALSE;
        } /*endwhile */
        pszPos = pszTagEnd;
      }
      else
      {
        pszPos++;
      } /* endif */
    } /*endwhile */
  } /* endif */



  while ( pszInString[iInPos] != 0 )
  {
    if ( pMU == NULL )
    {
      if ( !isTag[iInPos] )
      {
        *pszOutString++ = pszInString[iInPos];
      } /* endif */
      iInPos++;
    }
    else
    {
      // check against replacement strings
      int iReplTag = 0;
      BOOL fFound = FALSE;
      CHAR_W chTest = pszInString[iInPos];

      while ( !fFound && (iReplTag < pMU->iEntries) )
      {
        if ( chTest == pMU->Entry[iReplTag].szFrom[0] )
        {
          if ( wcsncmp( pszInString + iInPos, pMU->Entry[iReplTag].szFrom, pMU->Entry[iReplTag].iLen ) == 0 )
          {
            fFound = TRUE;
          }
          else
          {
            iReplTag++;
          } /* endif */
        }
        else
        {
          iReplTag++;
        } /* endif */
      } /*endwhile */

      if ( fFound )
      {
        wcscpy( pszOutString, pMU->Entry[iReplTag].szTo );
        pszOutString += wcslen( pszOutString );
        iInPos += pMU->Entry[iReplTag].iLen;
      }
      else
      {
        // check for encoded unicode character
        CHAR_W value = 0;
        int iLen = 0;
        if ( isCharEntity( pszInString + iInPos, &value, &iLen ) )
        {
          iInPos += iLen;
          *pszOutString++ = value;
        }
        else if ( !isTag[iInPos] )
        {
          *pszOutString++ = pszInString[iInPos];
          iInPos++;
        } /* endif */
        else
        {
          iInPos++;
        } /* endif */
      } /* endif */
    } /* endif */
  } /*endwhile */
  *pszOutString = 0;

  return( fOK );
} /* end of function ValExpApplyReplaceList */

// check if given docment is a Taxonomy document
BOOL ValIsTaxonomyDoc
(
  PTBDOCUMENT pDoc
)
{
  ULONG ulSegNum = 1;
  ULONG ulAddSegNum = 1;
  ULONG ulActiveTable = STANDARDTABLE;
  int  iSegsToCheck = 30;
  BOOL  fIsTaxonomy = FALSE;

  while ( !fIsTaxonomy && (iSegsToCheck > 0) && (ulSegNum <= pDoc->ulMaxSeg) )
  {
    // get segment
    PTBSEGMENT pSeg = EQFBGetFromBothTables( pDoc, &ulSegNum, &ulAddSegNum, &ulActiveTable );
    if ( pSeg )
    {
      if ( wcsnicmp( L"<TaxoFile", pSeg->pDataW, 9 ) == 0 )
      {
        fIsTaxonomy = TRUE;
      } /* endif */
      else if ( wcsnicmp( L"<Taxonomy", pSeg->pDataW, 9 ) == 0 )
      {
        fIsTaxonomy = TRUE;
      } /* endif */
    } /* endif */
    iSegsToCheck--;
  } /*endwhile */
  return( fIsTaxonomy );
}

// execute command
USHORT ValExecuteCommand( PSZ pszCommand )
{
    STARTUPINFOA           StartupInfo ;
    PROCESS_INFORMATION    piProcessInfo ; 
    DWORD                  dwCode = 0;

    GetStartupInfoA( &StartupInfo ) ;
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW ;
    StartupInfo.wShowWindow = SW_HIDE ;
    CreateProcessA( NULL, pszCommand, NULL, NULL, FALSE, (DWORD)0, NULL, NULL, &StartupInfo, &piProcessInfo ) ; 
    WaitForSingleObject( piProcessInfo.hProcess, INFINITE ) ;
    GetExitCodeProcess(piProcessInfo.hProcess, &dwCode);
    return( (USHORT)dwCode ) ;
}

// set tracking flag in MS Word document
BOOL ValSetTrackingFlag( PSZ pszDocument )
{
  char      szCommand[1024] ;
  char      szTempDir[256] ;
  BOOL      fOK = TRUE;
  PSZ       pszSettings = NULL;
  USHORT    usSize = 0;
  #define WordSettingsFile "word\\settings.xml"
#ifdef SETTRACKFLAGLOG
  CHAR szLogFile[MAX_EQF_PATH];
  FILE *hfLog = NULL;

  UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
  UtlMkDir( szLogFile, 0L, FALSE );
  strcat( szLogFile, "\\ValDocExportSetTrackFlag.LOG" );
  hfLog = fopen( szLogFile, "a" );
  if ( hfLog )
  {
    time_t lTime;
    time( &lTime );
    fprintf( hfLog, "ValDocExport Set Track Changes Flag log %s", asctime( localtime( &lTime ) ) );
  } /* endif */
#endif

  // create temporary directory
  UtlMakeEQFPath( szTempDir, NULC, EXPORT_PATH, NULL );
  strcat( szTempDir, "\\" );
  strcat( szTempDir, UtlGetFnameFromPath( pszDocument ) );
  UtlMkMultDir( szTempDir, FALSE );

  // check existance of UNZIP.EXE
  UtlMakeEQFPath( szCommand, NULC, WIN_PATH, NULL );
  strcat( szCommand, "\\UNZIP.EXE" );
  fOK = UtlFileExist( szCommand );
#ifdef SETTRACKFLAGLOG
  if ( hfLog ) fprintf( hfLog, "  Access to %s %s\n", szCommand, fOK ? "successful" : "failed" );
#endif

  // unpack DOCX file
  if ( fOK )
  {
    sprintf( szCommand+strlen(szCommand), " -q -o %s %s -d %s", pszDocument, WordSettingsFile, szTempDir );
    fOK = ValExecuteCommand( szCommand ) == 0;
#ifdef SETTRACKFLAGLOG
    if ( hfLog ) fprintf( hfLog, "  Unpacking of word settings file from document %s %s\n", pszDocument, fOK ? "successful" : "failed" );
#endif
  }

  // set tracking flag
  if ( fOK )
  {
    // load settings.xml
    sprintf( szCommand, "%s\\%s", szTempDir, WordSettingsFile );
    fOK = UtlLoadFile( szCommand, (PVOID *)&pszSettings, &usSize, FALSE, FALSE );
#ifdef SETTRACKFLAGLOG
    if ( hfLog ) fprintf( hfLog, "  Load of settings file from temp directory %s %s\n", szCommand, fOK ? "successful" : "failed" );
#endif
  }

  // resize settings buffer
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pszSettings, usSize, usSize + 20, FALSE );
    if ( fOK )
    {
      pszSettings[usSize] = 0; // ensure buffer is null terminated
      usSize += 19;
    }
#ifdef SETTRACKFLAGLOG
    if ( hfLog ) fprintf( hfLog, "  Reallocation of file buffer %s\n", fOK ? "successful" : "failed" );
#endif
  }

  // add tracking flag
  if ( fOK )
  {
    PSZ pszEndTag = strstr( pszSettings, "</w:settings>" );
    if ( pszEndTag != NULL )
    {
#ifdef SETTRACKFLAGLOG
    if ( hfLog ) fprintf( hfLog, "  End of settings tag </w:settings> found\n" );
#endif
      strcpy( pszEndTag, "<w:trackRevisions/></w:settings>" );
    }
    else
    {
#ifdef SETTRACKFLAGLOG
    if ( hfLog ) fprintf( hfLog, "  Could not find end of settings tag </w:settings>\n" );
#endif
      fOK = FALSE;
    }
  }

  // re-write settings.xml
  if ( fOK )
  {
    fOK = UtlWriteFile( szCommand, usSize, (PVOID)pszSettings, FALSE ) == 0;
#ifdef SETTRACKFLAGLOG
    if ( hfLog ) fprintf( hfLog, "  Rewrite of settings file %s from file buffer %s\n", szCommand, fOK ? "successful" : "failed" );
#endif
  }

  // check existance of ZIP.EXE
  if ( fOK )
  {
    UtlMakeEQFPath( szCommand, NULC, WIN_PATH, NULL );
    strcat( szCommand, "\\ZIP.EXE" );
    fOK = UtlFileExist( szCommand );
#ifdef SETTRACKFLAGLOG
  if ( hfLog ) fprintf( hfLog, "  Access to %s %s\n", szCommand, fOK ? "successful" : "failed" );
#endif
  }

  // freshen files in DOCX file from temporary directory
  if ( fOK )
  {
    char szSaveDir[MAX_PATH];
    _getcwd( szSaveDir, sizeof(szSaveDir) ) ;
    _chdir( szTempDir ) ;
    sprintf( szCommand + strlen(szCommand), " %s -f %s", pszDocument, WordSettingsFile );
    ValExecuteCommand( szCommand ) ;
    _chdir( szSaveDir ) ;
#ifdef SETTRACKFLAGLOG
  if ( hfLog ) fprintf( hfLog, "  Command to freshen settings file executed\n" );
#endif
  }

  // cleanup
  UtlRemoveDir( szTempDir, FALSE );
  if ( pszSettings ) UtlAlloc( (PVOID *)&pszSettings, 0, 0, NOMSG );

#ifdef SETTRACKFLAGLOG
  if ( hfLog )
  {
    fclose( hfLog );
  } /* endif */
#endif

  return( fOK );
}



