//+----------------------------------------------------------------------------+
//| EQFXLIF.CPP                                                               |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Parser for XLIFF Files                                        |
//|                                                                            |
//+----------------------------------------------------------------------------+

#pragma pack( push, TM2StructPacking, 1 )

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include "eqf.h"                  // General .H for EQF
#include "EQFMEMIE.H"
#include "EQFXLIFF.H"
#include "EQFMETADATA.H"

#pragma pack( pop, TM2StructPacking )

#include "core\pluginmanager\OtmMemory.h"
#include "core\pluginmanager\OtmMemoryPlugin.h"
#include "core\memory\MemoryFactory.h"

#include "cxmlwriter.H"

// the Win32 Xerces build requires the default structure packing...
#pragma pack( push )
#pragma pack(8)

#include <iostream>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>

#pragma pack( pop )

XERCES_CPP_NAMESPACE_USE



#include "eqfxlifi.h"


//+----------------------------------------------------------------------------+
//| Our XLIFF import export class                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
class CXLIFFExportImport
{
  public:
    // constructor/desctructor
	  CXLIFFExportImport();
	  ~CXLIFFExportImport();

    // import methods
    USHORT StartImport( const char *pszInFile, PMEMEXPIMPINFO pMemInfo ); 
    USHORT ImportNext( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, LONG *plProgress ); 
    USHORT EndImport(); 

    int Import( CHAR chDrive, PSZ pszFolShortName, PSZ pszDocShortName, PSZ pszSourceLanguage, PSZ pszTargetLanguage );
    int Export( PSZ pszSegTargetFile, PSZ pszExportTarget, PSZ pszFolShortName, PSZ pszDocShortName );

    // folder export methods
    int FolderExportStart( PFOLIMPEXPXLIFF pData );
    int FolderExportContinue( PFOLIMPEXPXLIFF pData );
    int FolderExportStop( PFOLIMPEXPXLIFF pData );

    // folder import methods
    int FolderImportStart( PFOLIMPEXPXLIFF pData );
    int FolderImportContinue( PFOLIMPEXPXLIFF pData );
    int FolderImportStop( PFOLIMPEXPXLIFF pData );

  protected:
    USHORT LoadNames( PSZ pszNameFile );
//    USHORT WriteTUV( PSZ pszLanguage, PSZ pszMarkup, PSZ_W pszSegmentData );
    int GetFolderProperties();
    int WriteDocument( PSZ pszDocShortName); 
    int WriteSegmentText( PSZ_W pszSegText, PLOADEDTABLE pTagTable, BOOL fProtected, ULONG ulCP );
    int WriteNonEmptyElement( PSZ pszElement, PSZ pszValue );

    // memory related stuff 
    int OpenMem( PSZ pszMemory );
    int CloseMem();
    int WriteMemProposal( PTBSEGMENT pSeg, PLOADEDTABLE pTagTable, ULONG ulCP );

    CXmlWriter *m_xw;
    IdiomXLIFFParseHandler *m_handler;          // our SAX handler for Idiom XLIFF files 
    FolXliffParseHandler   *m_handlerf;         // our SAX handler for XLIFF folders

    SAXParser* m_parser;
    XMLPScanToken m_SaxToken; 
    unsigned int m_iSourceSize;          // size of source file

    CNameList m_LangTmgr2Tmx;
    CNameList m_LangTmx2Tmgr;
    CNameList m_Markup2Type;
    CNameList m_Type2Markup;
    PTOKENENTRY m_pTokBuf;               // buffer for TaTagTokenize tokens
    CHAR m_szActiveTagTable[50];         // buffer for name of currently loaded markup table
    PLOADEDTABLE m_pLoadedTable;         // pointer to currently loaded markup table

    CHAR m_szXLIFFFile[512];             // buffer for input file name
    CHAR m_szSourceFile[MAX_EQF_PATH];   // buffer for document SOURCE file name
    CHAR m_szSegSourceFile[MAX_EQF_PATH];// buffer for document SSOURCE file name
    CHAR m_szSegTargetFile[MAX_EQF_PATH];// buffer for document STARGET file name
    CHAR m_szMetaDataFile[MAX_EQF_PATH]; // buffer for document METADATA file name
    CHAR m_szLongDocName[MAX_LONGFILESPEC]; // buffer for document long name
    CHAR m_szLongFolName[MAX_LONGFILESPEC]; // buffer for folder long name
 
    FILE *m_hfSource;                    // handle of source file
    FILE *m_hfSegSource;                 // handle of segmented source file
    FILE *m_hfSegTarget;                 // handle of segmented target file
    FILE *m_hfMetaData;                  // handle of metadata file

    // fields for the folder export/import in XLIFF mode
    int m_iSliderMovement;              // slider movement value
    int m_iCurrentDoc;                  // number of currently processed document (zero based)
    CXLIFFUtil *m_Util;                 // instance of out utilitiy class 
    typedef enum _XLIFFEXPSTATE { XLIFF_WRITEFOLDER, XLIFF_WRITEDOC, XLIFF_NEXTDOC, XLIFF_TERMINATE} XLIFFEXPSTATE;
    XLIFFEXPSTATE m_iState;
    PFOLIMPEXPXLIFF m_pData;            // pointer to XLIFF folder import/export data area
    PSZ   m_pszCurDoc;                  // ptr to name of current document
    CHAR  m_szFolMarkup[20];            // markup table used by folder
    CHAR  m_szBuffer[16032];            // general purpose buffer
    CHAR  m_szFolSourceLang[MAX_LANG_LENGTH];
    CHAR  m_szFolTargetLang[MAX_LANG_LENGTH];
    CHAR  m_szFolMemory[MAX_LONGFILESPEC]; // name of folder memory
    CHAR  m_szDocMemory[MAX_LONGFILESPEC]; // name of document memory
    CHAR  m_szCurMemory[MAX_LONGFILESPEC]; // name of current memory
    CHAR  m_szCurMemPath[MAX_EQF_PATH]; // path name of current memory
    CHAR  m_szDocMarkup[20];            // markup used by document
    CHAR  m_szDocSourceLang[MAX_LANG_LENGTH];  // source language used by document
    CHAR  m_szDocTargetLang[MAX_LANG_LENGTH];  // target language used by document
    ULONG m_ulSourceCP;                  // ASCII codepage for document source language
    ULONG m_ulTargetCP;                  // ASCII codepage for document target language
    CHAR  m_szShortDocName[MAX_FILESPEC]; // buffer for document short name
    OtmMemory *m_pMem;                  // current memory 
    CHAR  m_szPropMarkup[20];            // proposal markup 
    CHAR  m_szPropDocument[MAX_LONGFILESPEC]; // proposal document name 
    CHAR_W  m_szPropSource[MAX_SEGMENT_SIZE+1]; // buffer for proposal source
    CHAR_W  m_szPropTarget[MAX_SEGMENT_SIZE+1]; // buffer for proposal target

    CHAR   m_szROMemNames[MAX_NUM_OF_FOLDER_MDB+1][MAX_LONGFILESPEC]; // name of memories 
    int    m_iNumOfROMems ;                      // number of read-only memories
    std::vector<OtmMemory *> m_MemoryDBs;        // list of memory databases
    std::vector<OtmProposal *> m_FoundProposals; // list of found proposals
    std::vector<OtmProposal *> m_BestProposals;  // list of best proposals

};

// tools
static BOOL IsUnicodeFile( char *pszFile );
static PSZ ScanForKeyword( FILE *hf, PSZ pszLine, int iSize, PSZ pszKeyword, int iMaxLines );
static PSZ_W ScanForKeywordW( FILE *hf, PSZ_W pszLine, int iSize, PSZ_W pszKeyword, int iMaxLines );


//+----------------------------------------------------------------------------+
//| Interface functions called by TranslationManager                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
int XLIFFIMPORT
( 
  CHAR             chDrive,            // drive letter of drive containing the folder
  PSZ              pszFolShortName,    // folder short name 
  PSZ              pszDocShortName,    // document short name = name of file in XLIFF directory of folder
  PSZ              pszSourceLanguage,  // buffer for document source language
  PSZ              pszTargetLanguage   // buffer for document target language
)
{
  int              iRC = 0;           // function return code

  CXLIFFExportImport *pXLIFFImport = new CXLIFFExportImport;

  iRC = pXLIFFImport->Import( chDrive, pszFolShortName, pszDocShortName, pszSourceLanguage, pszTargetLanguage );

  return( iRC );
} /* end of function XLIFFIMPORT */

int XLIFFEXPORT
( 
  PSZ              pszSegTargetFile,   // fully qualified name of segmented target file
  PSZ              pszExportTarget,    // fully qualified name of XLIFF target file
  PSZ              pszFolShortName,    // folder short name 
  PSZ              pszDocShortName     // document short name = name of file in XLIFF directory of folder
)
{
  int              iRC = 0;           // function return code

  CXLIFFExportImport *pXLIFFImport = new CXLIFFExportImport;

  iRC = pXLIFFImport->Export( pszSegTargetFile, pszExportTarget, pszFolShortName, pszDocShortName );

  return( iRC );
} /* end of function XLIFFEXPORT */

//
// FOLXLIFFIMPORT
//
extern "C" 
__declspec(dllexport)
int __cdecl FOLXLIFFIMPORT
(
  PFOLIMPEXPXLIFF  pData                // pointer to XLIFF folder import/export data area
)
{
  int              iRC = 0;            // function return code
  CXLIFFExportImport *pXLIFFImport = NULL;

  if ( pData->lHandle == 0 )           // first call?
  {
    pXLIFFImport = new CXLIFFExportImport;
    pData->lHandle = (LONG)pXLIFFImport;
    iRC  = pXLIFFImport->FolderImportStart( pData );
  }
  else
  {
    pXLIFFImport = (CXLIFFExportImport *)pData->lHandle;
    iRC = pXLIFFImport->FolderImportContinue( pData );
  } /* endif */


  if ( iRC && (pXLIFFImport != NULL) )
  {
    pData->usComplete = 100;
    pXLIFFImport->FolderImportStop( pData );
  } /* endif */

  return( iRC );
} /* end of function XLIFFIMPORT */

//
// FOLXLIFFEXPORT
//

extern "C" 
__declspec(dllexport)
int __cdecl FOLXLIFFEXPORT
(
  PFOLIMPEXPXLIFF  pData                // pointer to XLIFF folder import/export data area
)
{
  int              iRC = 0;            // function return code
  CXLIFFExportImport *pXLIFFImport = NULL;

  if ( pData->lHandle == 0 )           // first call?
  {
    pXLIFFImport = new CXLIFFExportImport;
    iRC = pXLIFFImport->FolderExportStart( pData );
    pData->lHandle = (LONG)pXLIFFImport;
  }
  else 
  {
    pXLIFFImport = (CXLIFFExportImport *)pData->lHandle;
    iRC = pXLIFFImport->FolderExportContinue( pData );
  } /* endif */


  if ( iRC && (pXLIFFImport != NULL) )
  {
    pXLIFFImport->FolderExportStop( pData );
  } /* endif */

  return( iRC );
} /* end of function XLIFFEXPORT */

                    
extern "C" 
__declspec(dllexport)
int __cdecl FOLXLIFFCHECKFILE
(
  PSZ              pszFile,            // fully qualified name of file being checked
  PSZ              pszFolName,         // pointer to buffer for folder name
  HWND             hwndErrMsg          // window handle for error messages
)
{
  int              iRC = 0;            // function return code
  BOOL             fUnicode = FALSE;   // TRUE = input file is in Unicode

  typedef struct _XCHECKFILEDATA
  {
    CHAR szLine[2048];                 // buffer for current line

  } XCHECKFILEDATA, *PXCHECKFILEDATA;

  PXCHECKFILEDATA pData = NULL;
  FILE *hfIn = NULL;

  // allocate buffer area
  if ( !UtlAllocHwnd( (PVOID *)&pData, 0, sizeof(XCHECKFILEDATA), ERROR_STORAGE, hwndErrMsg ) )
  {
    iRC = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  // get type of encoding
  if ( !iRC )
  {
    fUnicode = IsUnicodeFile( pszFile );
  } /* endif */

  // open file
  if ( !iRC )
  {
    hfIn = fopen( pszFile, fUnicode ? "rb" : "r" );
    if ( hfIn == NULL )
    {
      iRC = ERROR_FILE_NOT_FOUND; 
    } /* endif */
  } /* endif */

  // scan file for our key words
  if ( !iRC )
  {
    if ( fUnicode )
    {
      PSZ_W pszLine = (PSZ_W)pData->szLine;
      int iSize = sizeof(pData->szLine)/sizeof(CHAR_W);
      PSZ_W pszFound = ScanForKeywordW( hfIn, pszLine, iSize, L"<?xml", 5 );
      if ( pszFound ) pszFound = ScanForKeywordW( hfIn, pszLine, iSize, L"<xliff", 10 );
      if ( pszFound ) pszFound = ScanForKeywordW( hfIn, pszLine, iSize, L"<tmgr:folder>", 60 );
      if ( pszFound )
      {
        PSZ_W pszEnd = wcschr( pszFound, L'<' );
        if ( pszEnd )
        {
          *pszEnd = 0;
          Unicode2ASCII( pszFound + 13, pszFolName, CP_OEMCP );
          *pszEnd = L'<' ;
        } /* endif */
      }
      else
      {
        iRC = ERROR_FILE_NOT_FOUND;
      } /* endif */
    }
    else
    {
      PSZ pszFound = ScanForKeyword( hfIn, pData->szLine, sizeof(pData->szLine), "<?xml", 5 );
      if ( pszFound ) pszFound = ScanForKeyword( hfIn, pData->szLine, sizeof(pData->szLine), "<xliff", 10 );
      if ( pszFound ) pszFound = ScanForKeyword( hfIn, pData->szLine, sizeof(pData->szLine), "<tmgr:folder>", 60 );
      if ( pszFound )
      {
        Utlstrccpy( pszFolName, pszFound + 13, '<' );
      }
      else
      {
        iRC = ERROR_FILE_NOT_FOUND;
      } /* endif */
    } /* endif */
  } /* endif */

  // cleanup
  if ( pData) UtlAlloc( (PVOID *)&pData, 0, 0, NOMSG );
  if ( hfIn ) fclose( hfIn );

  return( iRC );
} /* end of function FOLXLIFFCHECKFILE */


//+----------------------------------------------------------------------------+
//| Implementation of XLIFFImportExport class                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
CXLIFFExportImport::CXLIFFExportImport()
{
  m_pTokBuf = (PTOKENENTRY)malloc( TMXTOKBUFSIZE );
  m_pLoadedTable = NULL;
  m_szActiveTagTable[0] = EOS;

  m_hfSource = NULL;
  m_hfSegSource = NULL;
  m_hfSegTarget = NULL;
  m_hfMetaData = NULL;

  m_handler = NULL;
  m_handlerf = NULL;
  m_parser = NULL;

  m_szCurMemory[0] = '\0';
  m_pMem = NULL; 
  m_iNumOfROMems = 0 ;

  // prepare proposal vectors
  for ( int i=0; i < 3; i++ ) m_FoundProposals.push_back( new OtmProposal );
  for ( int i=0; i < 3; i++ ) m_BestProposals.push_back( new OtmProposal );
}

CXLIFFExportImport::~CXLIFFExportImport()
{
  if ( m_pTokBuf ) free( m_pTokBuf );
  if ( m_pLoadedTable ) TAFreeTagTable( m_pLoadedTable );
  for ( int i=0; i < 3; i++ ) m_FoundProposals.pop_back();
  for ( int i=0; i < 3; i++ ) m_BestProposals.pop_back();
  for ( int i=0; i < m_iNumOfROMems; i++ ) m_MemoryDBs.pop_back();
}

int CXLIFFExportImport::Import
(
  CHAR             chDrive,            // drive letter of drive containing the folder
  PSZ              pszFolShortName,    // folder short name 
  PSZ              pszDocShortName,    // document short name = name of file in XLIFF directory of folder
  PSZ              pszSourceLanguage,  // buffer for document source language
  PSZ              pszTargetLanguage   // buffer for document target language
)
{
  int iRC = 0;

  // load names file
  {
    CHAR szNameFile[128];
  
    UtlMakeEQFPath( szNameFile, NULC, TABLE_PATH, NULL );
    strcat( szNameFile, "\\tmxnames.lst" );
    LoadNames( szNameFile );
  }

  // get document long name
  m_Util->BuildTMFileName( m_szSourceFile, chDrive, SYSTEM_PATH, pszFolShortName, pszDocShortName ); 
  DocQueryInfo2( m_szSourceFile, NULL,NULL,NULL,NULL, m_szLongDocName, NULL,NULL,FALSE);

  // setup file names
  m_Util->BuildTMFileName( m_szXLIFFFile, chDrive, XLIFF_PATH, pszFolShortName, pszDocShortName ); 
  m_Util->BuildTMFileName( m_szSourceFile, chDrive, DIRSOURCEDOC_PATH, pszFolShortName, pszDocShortName ); 
  m_Util->BuildTMFileName( m_szSegSourceFile, chDrive, DIRSEGSOURCEDOC_PATH, pszFolShortName, pszDocShortName ); 
  m_Util->BuildTMFileName( m_szSegTargetFile, chDrive, DIRSEGTARGETDOC_PATH, pszFolShortName, pszDocShortName ); 
  m_Util->BuildTMFileName( m_szMetaDataFile, chDrive, METADATA_PATH, pszFolShortName, pszDocShortName ); 

  // open output files
  iRC = m_Util->OpenOutputFile( m_szSourceFile,    &m_hfSource );
  if ( !iRC ) iRC = m_Util->OpenOutputFile( m_szSegSourceFile, &m_hfSegSource );
  if ( !iRC ) iRC = m_Util->OpenOutputFile( m_szSegTargetFile, &m_hfSegTarget );
  if ( !iRC ) iRC = m_Util->OpenOutputFile( m_szMetaDataFile, &m_hfMetaData );

  // write BOMs to Unicode output files
  if ( !iRC )
  {
    m_Util->WriteBOM( m_hfSource );
    m_Util->WriteBOM( m_hfSegSource );
    m_Util->WriteBOM( m_hfSegTarget );
  } /* endif */

  if ( !iRC )
  {
      try 
      {
        XMLPlatformUtils::Initialize();
      }
      catch (const XMLException& toCatch) 
      {
        toCatch;

        return( ERROR_NOT_READY );
      }

      m_parser = new SAXParser();      // Create a SAX parser object

      // create an instance of our handler
      m_handler = new IdiomXLIFFParseHandler();

      // pass output files to handler
      m_handler->SetFile( DIRSOURCEDOC_PATH, m_hfSource );
      m_handler->SetFile( DIRSEGSOURCEDOC_PATH, m_hfSegSource );
      m_handler->SetFile( DIRSEGTARGETDOC_PATH, m_hfSegTarget );
      m_handler->SetFile( METADATA_PATH, m_hfMetaData );
      m_handler->SetLanguagePointer( pszSourceLanguage, pszTargetLanguage );
      m_handler->SetNameLists( &m_LangTmx2Tmgr, &m_Type2Markup );
      m_handler->SetSourceFileName( this->m_szLongDocName );

      //  install our SAX handler as the document and error handler.
      m_parser->setDocumentHandler( m_handler );
      m_parser->setErrorHandler( m_handler );
      m_parser->setCalculateSrcOfs( TRUE );
      m_parser->setValidationSchemaFullChecking( FALSE );
      m_parser->setDoSchema( FALSE );
      m_parser->setLoadExternalDTD( FALSE );
      m_parser->setValidationScheme( SAXParser::Val_Never );

    try
    {
      m_parser->parse( m_szXLIFFFile );
    }
    catch (const OutOfMemoryException& )
    {
//        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    catch (const XMLException& toCatch)
    {
      toCatch;

        //XERCES_STD_QUALIFIER cerr << "\nAn error occurred: '" << xmlFile << "'\n"
        //     << "Exception message is: \n"
        //     << StrX(toCatch.getMessage())
        //     << "\n" << XERCES_STD_QUALIFIER endl;
        iRC = ERROR_READ_FAULT;
    }

  } /* endif */

  m_Util->CloseFile( m_hfSource );
  m_hfSource = NULL;
  m_Util->CloseFile( m_hfSegSource );
  m_hfSegSource = NULL;
  m_Util->CloseFile( m_hfSegTarget );
  m_hfSegTarget = NULL;
  m_Util->CloseFile( m_hfMetaData );
  m_hfMetaData = NULL;

  // set completion rate in document properties
  if ( !iRC )
  {
    HPROP           hProp;                        // handle of document properties
    PPROPDOCUMENT   pProp = NULL;                 // pointer to document properties
    ULONG           ulErrorInfo;                  // error indicator from PRHA

    // update properties
    m_Util->BuildTMFileName( m_szSourceFile, chDrive, SYSTEM_PATH, pszFolShortName, pszDocShortName ); 
    hProp = OpenProperties( m_szSourceFile, NULL, PROP_ACCESS_READ, &ulErrorInfo);
    if ( hProp )
    {
      if ( SetPropAccess( hProp, PROP_ACCESS_WRITE ) )
      {
        LONG lTotalSegs = m_handler->GetTotalSegs();
        LONG lTranslatedSegs = m_handler->GetTranslatedSegs();
 
        pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hProp );
 
        if ( lTotalSegs == 0 )
        {
          pProp->usComplete = (USHORT)100;
        }
        else
        {
          pProp->usComplete = (USHORT)( (lTranslatedSegs * 100L) / lTotalSegs );
          if ( !pProp->usComplete && lTranslatedSegs ) pProp->usComplete = 1;
        } /* endif */
 
        SaveProperties( hProp, &ulErrorInfo );
        ResetPropAccess( hProp, PROP_ACCESS_WRITE );
      } /* endif */
      CloseProperties( hProp, PROP_FILE, &ulErrorInfo);
    } /* endif */
  } /* endif */

  return( iRC );
}

int CXLIFFExportImport::Export
(
  PSZ              pszSegTargetFile,   // fully qualified name of segmented target file
  PSZ              pszExportTarget,    // fully qualified name of XLIFF target file
  PSZ              pszFolShortName,    // folder short name 
  PSZ              pszDocShortName     // document short name = name of file in XLIFF directory of folder
)
{
  int iRC = 0;
  PTBDOCUMENT      pSourceDoc = NULL;
  PTBDOCUMENT      pTargetDoc = NULL;

  // setup file names
  m_Util->BuildTMFileName( m_szXLIFFFile, pszSegTargetFile[0], XLIFF_PATH, pszFolShortName, pszDocShortName ); 
  //BuildTMFileName( m_szSourceFile, chDrive, DIRSOURCEDOC_PATH, pszFolShortName, pszDocShortName ); 
  m_Util->BuildTMFileName( m_szSegSourceFile, pszSegTargetFile[0], DIRSEGSOURCEDOC_PATH, pszFolShortName, pszDocShortName ); 
  //BuildTMFileName( m_szSegTargetFile, chDrive, DIRSEGTARGETDOC_PATH, pszFolShortName, pszDocShortName ); 
  //BuildTMFileName( m_szMetaDataFile, chDrive, METADATA_PATH, pszFolShortName, pszDocShortName ); 

  // open output files
  //iRC = OpenOutputFile( m_szSourceFile,    &m_hfSource );
  //if ( !iRC ) iRC = OpenOutputFile( m_szSegSourceFile, &m_hfSegSource );
  //if ( !iRC ) iRC = OpenOutputFile( m_szSegTargetFile, &m_hfSegTarget );
  //if ( !iRC ) iRC = OpenOutputFile( m_szMetaDataFile, &m_hfMetaData );

  //// write BOMs to Unicode output files
  //if ( !iRC )
  //{
  //  WriteBOM( m_hfSource );
  //  WriteBOM( m_hfSegSource );
  //  WriteBOM( m_hfSegTarget );
  //} /* endif */

  // load segmented target document
  if ( !iRC )
  {
    iRC = m_Util->LoadSegmentedFile( pszSegTargetFile, &pTargetDoc, TRUE );
  } /* endif */

  // load segmented source document
  if ( !iRC )
  {
    iRC = m_Util->LoadSegmentedFile( m_szSegSourceFile, &pSourceDoc, FALSE );
  } /* endif */

  if ( !iRC )
  {
      try 
      {
        XMLPlatformUtils::Initialize();
      }
      catch (const XMLException& toCatch) 
      {
        toCatch;

        return( ERROR_NOT_READY );
      }

      // create an XML writer object for the output XLIFF file
      m_xw = new CXmlWriter( pszExportTarget );
      m_xw->Formatting = CXmlWriter::Indented;
      m_xw->Encoding = CXmlWriter::UTF8;
      m_xw->Indention = 2;
      m_xw->WriteStartDocument();

      m_parser = new SAXParser();      // Create a SAX parser object

      // create an instance of our handler
      m_handler = new IdiomXLIFFParseHandler();

      // pass output files to handler
      //m_handler->SetFile( DIRSOURCEDOC_PATH, m_hfSource );
      //m_handler->SetFile( DIRSEGSOURCEDOC_PATH, m_hfSegSource );
      //m_handler->SetFile( DIRSEGTARGETDOC_PATH, m_hfSegTarget );
      //m_handler->SetFile( METADATA_PATH, m_hfMetaData );

      m_handler->SetMode( IdiomXLIFFParseHandler::EXPORT_MODE );

      m_handler->SetXMLWriter( m_xw );
      m_handler->SetLoadedDocs( pTargetDoc, pSourceDoc );

      //  install our SAX handler as the document and error handler.
      m_parser->setDocumentHandler( m_handler );
      m_parser->setErrorHandler( m_handler );
      m_parser->setCalculateSrcOfs( TRUE );
      m_parser->setValidationSchemaFullChecking( FALSE );
      m_parser->setDoSchema( FALSE );
      m_parser->setLoadExternalDTD( FALSE );
      m_parser->setValidationScheme( SAXParser::Val_Never );

    try
    {
      m_parser->parse( m_szXLIFFFile );
    }
    catch (const OutOfMemoryException& )
    {
//        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    catch (const XMLException& toCatch)
    {
      toCatch;

        //XERCES_STD_QUALIFIER cerr << "\nAn error occurred: '" << xmlFile << "'\n"
        //     << "Exception message is: \n"
        //     << StrX(toCatch.getMessage())
        //     << "\n" << XERCES_STD_QUALIFIER endl;
        iRC = ERROR_READ_FAULT;
    }

  } /* endif */

  m_xw->WriteEndDocument();
  m_xw->Close();

  delete m_xw;
  m_xw = NULL;

  if ( pSourceDoc ) m_Util->FreeSegmentedFile( &pSourceDoc );
  if ( pTargetDoc ) m_Util->FreeSegmentedFile( &pTargetDoc );


  //CloseFile( m_hfSource );
  //m_hfSource = NULL;
  //CloseFile( m_hfSegSource );
  //m_hfSegSource = NULL;
  //CloseFile( m_hfSegTarget );
  //m_hfSegTarget = NULL;
  //CloseFile( m_hfMetaData );
  //m_hfMetaData = NULL;

  return( iRC );
}

//USHORT CXLIFFExportImport::WriteHeader
//( 
//  const char *pszOutFile,
//  PMEMEXPIMPINFO pMemInfo 
//)
//{
//  // load tmgr<->TMX name tables
//  {
//    CHAR szNameFile[128];
//
//    UtlMakeEQFPath( szNameFile, NULC, TABLE_PATH, NULL );
//    strcat( szNameFile, "\\tmxnames.lst" );
//    LoadNames( szNameFile );
//  }
//
//  // write TMX header
//  m_xw.SetFileName( pszOutFile );
//  m_xw.Formatting = CXmlWriter::Indented;
//  m_xw.Indention = 2;
//  if ( pMemInfo->fUTF16 )
//  {
//    m_xw.Encoding = CXmlWriter::UTF16;
//  }
//  else
//  {
//    m_xw.Encoding = CXmlWriter::UTF8;
//  } /* endif */
//
//  m_xw.WriteStartDocument();
//
//  m_xw.WriteStartElement( "tmx" );
//  m_xw.WriteAttributeString( "version", "1.4" ); 
//
//  m_xw.WriteStartElement( "header" );
//  m_xw.WriteAttributeString( "creationtoolversion", "607" ); 
//  //m_xw.WriteAttributeString( "datatype", "todo" );  // no data type on memory level
//  m_xw.WriteAttributeString( "segtype", "sentence" ); 
//  m_xw.WriteAttributeString( "adminlang", "en-us" ); 
//
//
//  {
//    // get TMX language name
//    CHAR szTMXLang[20];                  // buffer for TMX language
//    szTMXLang[0] = EOS;
//    m_LangTmgr2Tmx.FindName( pMemInfo->szSourceLang, szTMXLang, sizeof(szTMXLang) );
//    m_xw.WriteAttributeString( "srclang", szTMXLang ); 
//  }
// 
//  m_xw.WriteAttributeString( "o-tmf", "TranslationManager" ); 
//  m_xw.WriteAttributeString( "creationtool", "TranslationManager" ); 
//
//  // add memory description as as property
//  if ( pMemInfo->szDescription[0] )
//  {
//    m_xw.WriteStartElement( "prop" );
//    m_xw.WriteAttributeString( "type", DESCRIPTION_PROP );
//    m_xw.WriteString( pMemInfo->szDescription ); 
//    m_xw.WriteEndElement(); // prop
//  } /* endif */
//
//  m_xw.WriteEndElement(); // header
//
//  m_xw.WriteStartElement( "body" );
//  return( 0 );
//}


// load name list file for Tmgr <-> TMX name conversions
USHORT CXLIFFExportImport::LoadNames
( 
  PSZ pszNameFile
)
{
  FILE *hf = NULL;
  char szLine[512];
  CNameList *pActiveList = NULL;
  USHORT usRC = 0;

  // load and process name file
  hf = fopen( pszNameFile, "r" );
  if ( hf )
  {
    // read first line
    fgets( szLine, sizeof(szLine), hf );

    do
    {
      // strip-off LF at end of line
      int iLen = strlen(szLine);
      if ( iLen && (szLine[iLen-1] == '\n') ) szLine[iLen-1] = '\0';

      // handle line
      if ( szLine[0] == '*' )
      {
        // ignore comment line
      }
      else
      {
        // strip leading and trailing blanks
        {
          PSZ pszSource = szLine;
          PSZ pszTarget = szLine;

          while ( *pszSource == ' ') pszSource++;
          while ( *pszSource ) *pszTarget++ = *pszSource++; 
          *pszTarget = '\0';
          
          if ( szLine[0] )
          {
            pszSource = szLine + strlen(szLine) - 1;
            while ( (pszSource > szLine) && (*pszSource == ' ') )
            {
              *pszSource = '\0';
              pszSource--;
            } /*endwhile */
          } /* endif */
        }

        // handle data line
        if ( szLine[0] == '\0' )
        {
          // ignore empty line
        }
        else if ( szLine[0] == '[' )
        {
          // process new group header

          // find end of group name and cut-off rest of line
          char *pszEndBracket = strchr( szLine, ']' );
          if ( pszEndBracket ) *pszEndBracket = '\0';

          // check name of group
          if ( _stricmp( szLine + 1, "Language-Tmgr2TMX" ) == 0 )
          {
            // init name groups
            pActiveList = &m_LangTmgr2Tmx;
          }
          else if ( _stricmp( szLine + 1, "Language-TMX2Tmgr" ) == 0 )
          {
            pActiveList = &m_LangTmx2Tmgr;
          }
          else if ( _stricmp( szLine + 1, "Markup2Type" ) == 0 )
          {
            pActiveList = &m_Markup2Type;
          }
          else if ( _stricmp( szLine + 1, "Type2Markup" ) == 0 )
          {
            pActiveList = &m_Type2Markup;
          }
          else 
          {
            // no or no supported group
            pActiveList = NULL;
          } /* endif */
        }
        else if ( pActiveList != NULL  )
        {
          // handle text line
          char *pszValue = strchr( szLine, '=' );
          if ( *pszValue )
          {
            *pszValue = '\0';          // split line
            pszValue++;                // position to start of value
//TODO            UtlStripBlanks( pszValue );//  
//TODO            UtlStripBlanks( szLine );
            pActiveList->AddName( szLine, pszValue );
          } /* endif */
        } 
        else 
        {
          // ignore text line outside of a group
        } /* endif */
      } /* endif */

      // read next line
      fgets( szLine, sizeof(szLine), hf );

    } while ( !feof(hf) );
  }
  else
  {
    usRC = 123; // ERROR_FILE_OPEN_FAILED;
  } /* endif */

  return( usRC );
}

//
// start XLIFF export of a folder
//
int CXLIFFExportImport::FolderExportStart
(
  PFOLIMPEXPXLIFF  pData                // pointer to XLIFF folder import/export data area
)
{
  int              iRC = 0;            // function return code

  // anchor folder import/export area
  m_pData = pData;

  // inialize our variables
  m_iState = XLIFF_WRITEFOLDER;
  m_iCurrentDoc = 0;

  // set slider step range (on step per document or folder)
  m_iSliderMovement = 100 / (pData->iDocuments + 1);

  // get instance of XLIFF utils 
  m_Util = new CXLIFFUtil();

  // load names file
  {
    CHAR szNameFile[128];
  
    UtlMakeEQFPath( szNameFile, NULC, TABLE_PATH, NULL );
    strcat( szNameFile, "\\tmxnames.lst" );
    LoadNames( szNameFile );
  }

  // create an XML writer object for the output XLIFF file
  m_xw = new CXmlWriter( pData->szExportName );
  m_xw->Formatting = CXmlWriter::Indented;
  m_xw->Encoding = CXmlWriter::UTF8;
  m_xw->Indention = 2;

  return( iRC );
} /* end of CXLIFFExportImport::FolderExportStart */ 

//
// continue XLIFF export of a folder
//
int CXLIFFExportImport::FolderExportContinue
(
  PFOLIMPEXPXLIFF  pData                // pointer to XLIFF folder import/export data area
)
{
  int              iRC = 0;            // function return code

  switch ( m_iState )
  {
    case XLIFF_WRITEFOLDER:
      // start XLIFF file
      m_xw->WriteStartDocument();
      m_xw->WriteStartElement( "xliff" );
      m_xw->WriteAttributeString( "version", "1.1" ); 
      m_xw->WriteAttributeString( "xmlns", "urn:oasis:names:tc:xliff:document:1.1"  ); 
      m_xw->WriteAttributeString( "xmlns:tmgr", "http://www.ibm.com"  ); 

      // get folder info
      GetFolderProperties();

      // continue with documents
      m_iState = ( m_iCurrentDoc >= pData->iDocuments ) ? XLIFF_TERMINATE : XLIFF_WRITEDOC;
      m_pszCurDoc = m_pData->pszDocuments;
      pData->usComplete = pData->usComplete + (USHORT)m_iSliderMovement;
      break;

    case XLIFF_WRITEDOC:
      // write current document
      iRC = WriteDocument( m_pszCurDoc ); 
      m_iState = XLIFF_NEXTDOC;
      pData->usComplete = pData->usComplete + (USHORT)m_iSliderMovement;
      break;

    case XLIFF_NEXTDOC:
      m_iCurrentDoc += 1;
      m_pszCurDoc += strlen( m_pszCurDoc ) + 1;
      m_iState = ( m_iCurrentDoc >= pData->iDocuments ) ? XLIFF_TERMINATE : XLIFF_WRITEDOC;
      break;

    case XLIFF_TERMINATE:
      m_xw->WriteEndElement(); // "xliff"
      m_xw->WriteEndDocument();
      iRC = FOL_EXPORT_IMPORT_COMPLETE;
      break;

    default:
      break;
  } /*endswitch */
  return( iRC );
} /* end of CXLIFFExportImport::FolderExportContinue */ 

//
// end XLIFF export of a folder
//
int CXLIFFExportImport::FolderExportStop
(
  PFOLIMPEXPXLIFF  pData                // pointer to XLIFF folder import/export data area
)
{
  int              iRC = 0;            // function return code

  pData;

  if ( m_iNumOfROMems > 0  )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    int i = 0 ;
    while ( (i < m_iNumOfROMems) && (m_MemoryDBs[i] != NULL) )
    {         
      pFactory->closeMemory( m_MemoryDBs[i] );
      i++;
    } 
  } 

  if ( m_xw != NULL )
  {
    m_xw->WriteEndDocument();
    m_xw->Close();

    delete m_xw;
    m_xw = NULL;
  } /* endif */

  return( iRC );
} /* end of CXLIFFExportImport::FolderExportStop */ 

//
// start XLIFF import of a folder
//
int CXLIFFExportImport::FolderImportStart
(
  PFOLIMPEXPXLIFF  pData                // pointer to XLIFF folder import/export data area
)
{
  int              iRC = 0;            // function return code

  // anchor folder import/export area
  m_pData = pData;

  // load names file
  {
    CHAR szNameFile[128];
  
    UtlMakeEQFPath( szNameFile, NULC, TABLE_PATH, NULL );
    strcat( szNameFile, "\\tmxnames.lst" );
    LoadNames( szNameFile );
  }

  // get size of input file
  if ( !iRC )
  {
    WIN32_FIND_DATA FindData;
    HANDLE hFind = FindFirstFile( pData->szExportName, &FindData );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
      m_iSourceSize = FindData.nFileSizeLow;
      FindClose( hFind );
    }
    else
    {
      PSZ pszParm = pData->szExportName;
      iRC = ERROR_FILE_ACCESS_ERROR;
      UtlErrorHwnd( (SHORT)iRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, m_pData->hwndErrMsg );
    } /* endif */
  }

  // parse the XLIFF file
  if ( !iRC )
  {
      try {
          XMLPlatformUtils::Initialize();
      }
      catch (const XMLException& toCatch) {
          toCatch;

          return( ERROR_NOT_READY );
      }

      m_parser = new SAXParser();      // Create a SAX parser object

      // create an instance of our handler
      m_handlerf = new FolXliffParseHandler();

      // pass language and markup name list to handler
      m_handlerf->SetNameLists( &m_LangTmx2Tmgr, &m_Type2Markup ); 
      m_handlerf->SetImpExpData( pData ); 

      //  install our SAX handler as the document and error handler.
      m_parser->setDocumentHandler( m_handlerf );
      m_parser->setErrorHandler( m_handlerf );
      m_parser->setCalculateSrcOfs( TRUE );
      m_parser->setValidationSchemaFullChecking( FALSE );
      m_parser->setDoSchema( FALSE );
      m_parser->setLoadExternalDTD( FALSE );
      m_parser->setValidationScheme( SAXParser::Val_Never );
      m_parser->setExitOnFirstFatalError( FALSE );
      
    // start parsing
    try
    {
      if (!m_parser->parseFirst( pData->szExportName, m_SaxToken))
      {
        PSZ pszParm = pData->szExportName;
        iRC = ERROR_FILE_READ_ERROR;
        UtlErrorHwnd( (SHORT)iRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, m_pData->hwndErrMsg );
      } /* endif */
    }
    catch (const OutOfMemoryException& )
    {
//        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
      iRC = ERROR_STORAGE;
      UtlErrorHwnd( (SHORT)iRC, MB_CANCEL, 0, NULL, EQF_ERROR, m_pData->hwndErrMsg );
    }
    catch (const XMLException& toCatch)
    {
      toCatch;

        //XERCES_STD_QUALIFIER cerr << "\nAn error occurred: '" << xmlFile << "'\n"
        //     << "Exception message is: \n"
        //     << StrX(toCatch.getMessage())
        //     << "\n" << XERCES_STD_QUALIFIER endl;
        PSZ pszParm = pData->szExportName;
        iRC = ERROR_FILE_READ_ERROR;
        UtlErrorHwnd( (SHORT)iRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, m_pData->hwndErrMsg );
    }
  } /* endif */

  return( iRC );
} /* end of CXLIFFExportImport::FolderImportStart */ 

//
// continue XLIFF import of a folder
//
int CXLIFFExportImport::FolderImportContinue
(
  PFOLIMPEXPXLIFF  pData                // pointer to XLIFF folder import/export data area
)
{
  int              iRC = 0;            // function return code
  int              iSteps = 100;

  // parse until header processed of no more data available
  BOOL fContinue  = TRUE;
  while (fContinue && !m_parser->getErrorCount() && iSteps )
  {
    fContinue = m_parser->parseNext(m_SaxToken);
    iSteps--;
  } /*endwhile */

  if ( fContinue )
  {
    int iPos = (int)m_parser->getSrcOffset();

    INT64 iComplete = (INT64)iPos;
    INT64 iTotal = (INT64)m_iSourceSize;
    INT64 iRatio = iComplete * (INT64)100 / iTotal;
    m_pData->usComplete = (USHORT)iRatio;

  } /* endif */

  if ( !fContinue )
  {
    iRC = FOL_EXPORT_IMPORT_COMPLETE;
  } /* endif */

  if ( m_handlerf->ErrorOccured() )
  {
    m_handlerf->GetErrorText( pData->szError, sizeof(pData->szError) );
    pData->fError = TRUE;
    PSZ pszParms[3];

    pszParms[0] = pData->szExportName ;
    pszParms[1] = pData->szError;
    pszParms[2] = "" ;
    iRC = ERROR_XML_LOADERROR;
    UtlErrorHwnd( (SHORT)iRC, MB_CANCEL, 3, pszParms, EQF_ERROR, pData->hwndErrMsg );
  } /* endif */

  return( iRC );
} /* end of CXLIFFExportImport::FolderImportContinue */ 

//
// end XLIFF import of a folder
//
int CXLIFFExportImport::FolderImportStop
(
  PFOLIMPEXPXLIFF  pData                // pointer to XLIFF folder import/export data area
)
{
  int              iRC = 0;            // function return code

  pData;

  // end SAX parser and environment
  if ( m_parser != NULL )   delete m_parser;                     // Delete the parser itself
  if ( m_handlerf != NULL ) delete m_handlerf;                   // delete the handler  
  XMLPlatformUtils::Terminate();       // And call the termination method

  return( iRC );
} /* end of CXLIFFExportImport::FolderImpStop */ 

int CXLIFFExportImport::GetFolderProperties
(
)
{
  int iRC = 0;

  // get folder properties
  PPROPFOLDER     pProp = NULL;                 // pointer to folder properties
  ULONG           ulErrorInfo;                  // error indicator from PRHA

  HPROP hProp = OpenProperties( m_pData->szFolObjName, NULL, PROP_ACCESS_READ, &ulErrorInfo);
  if ( hProp )
  {
    pProp = (PPROPFOLDER)MakePropPtrFromHnd( hProp );

    // remember some folder setings
    strcpy( m_szFolMarkup,     pProp->szFormat );
    strcpy( m_szFolSourceLang, pProp->szSourceLang );
    strcpy( m_szFolTargetLang, pProp->szTargetLang );
    strcpy( m_szFolMemory, pProp->szLongMemory[0] ? pProp->szLongMemory : pProp->szMemory );

    if ( pProp->szLongName[0] )
    {
      strcpy( m_szLongFolName, pProp->szLongName );
    }
    else
    {
      Utlstrccpy( m_szLongFolName, pProp->PropHead.szName, DOT );
    } /* endif */


    // add folder R/O memories to the list
    if ( ( m_pData->fWithROMem ) &&
         ( pProp->aLongMemTbl[0][0] != EOS ) ) 
    {
      int i = 0;
      MemoryFactory *pFactory = MemoryFactory::getInstance();
      while ( ( ! iRC ) && 
              ( i < MAX_NUM_OF_READONLY_MDB ) && 
              ( pProp->aLongMemTbl[i][0] != EOS ) )
      {
         // open the read-only memories
         OtmMemory *pMem = pFactory->openMemory( NULL, pProp->aLongMemTbl[i], READONLYACCESS, &iRC );
         if ( !iRC )
         {
           strcpy( m_szROMemNames[m_iNumOfROMems], pProp->aLongMemTbl[i] );
           m_MemoryDBs.push_back( pMem );
           m_iNumOfROMems++;
         }
         else
         {
           // memory not accessible or not found
           PSZ  pszMemory = pProp->aLongMemTbl[i] ;
           UtlErrorHwnd( EQFS_TM_NOT_FOUND, MB_CANCEL, 1, &pszMemory, EQF_ERROR, this->m_pData->hwndErrMsg );
           iRC = EQFS_TM_NOT_FOUND;
         } /* endif */
         i++;
      } 
    }

    CloseProperties( hProp, PROP_QUIT, &ulErrorInfo);
  } /* endif */

  return( iRC );
}

int CXLIFFExportImport::WriteDocument
(
  PSZ               pszDocShortName    // pointer to document short name
)
{
  int iRC = 0;
  PTBDOCUMENT      pSourceDoc = NULL;
  PTBDOCUMENT      pTargetDoc = NULL;
  ULONG ulCP = 0;                      // ansi codepage for document language
  SHORT sLangID = -1;                  // morph language ID for source language

  // start document
  m_xw->WriteStartElement( "file" );

  // setup file names
  PSZ pszFolName = UtlGetFnameFromPath( m_pData->szFolObjName );
  m_Util->BuildTMFileName( m_szSegSourceFile, m_pData->szFolObjName[0], DIRSEGSOURCEDOC_PATH, pszFolName, pszDocShortName ); 
  m_Util->BuildTMFileName( m_szSegTargetFile, m_pData->szFolObjName[0], DIRSEGTARGETDOC_PATH, pszFolName, pszDocShortName ); 

  // get/write document properties
  {
    PPROPDOCUMENT   pProp = NULL;                 // pointer to document properties
    ULONG           ulErrorInfo;                  // error indicator from PRHA

    HPROP hProp = OpenProperties( pszDocShortName, m_pData->szFolObjName, PROP_ACCESS_READ, &ulErrorInfo);
    if ( hProp )
    {
      CHAR szName[20];

      pProp = (PPROPDOCUMENT)MakePropPtrFromHnd( hProp );

      strcpy( m_szShortDocName, pProp->PropHead.szName ); 
      strcpy( m_szDocMarkup, pProp->szFormat[0] ? pProp->szFormat : m_szFolMarkup );
      strcpy( m_szDocSourceLang, pProp->szSourceLang[0] ? pProp->szSourceLang : m_szFolSourceLang );
      strcpy( m_szDocTargetLang, pProp->szTargetLang[0] ? pProp->szTargetLang : m_szFolTargetLang );

      if ( pProp->szLongMemory[0] )
      {
        strcpy( m_szDocMemory, pProp->szLongMemory ); 
      }
      else if ( pProp->szMemory[0] )
      {
        strcpy( m_szDocMemory, pProp->szMemory ); 
      }
      else
      {
        strcpy( m_szDocMemory, m_szFolMemory ); 
      } /* endif */

      
      ulCP = GetLangAnsiCP( m_szDocTargetLang );
      m_ulSourceCP = GetLangOEMCP( m_szDocSourceLang );
      m_ulTargetCP = GetLangOEMCP( m_szDocTargetLang );
      MorphGetLanguageID( m_szDocSourceLang, &sLangID );

      if ( !m_Markup2Type.FindName( m_szDocMarkup, szName, sizeof(szName) ) )
      {
        strcpy( szName, "plaintext" );
      } /* endif */
      m_xw->WriteAttributeString( "datatype", szName ); 

      OEMTOANSI( pProp->szLongName ) ;
      m_xw->WriteAttributeString( "original", pProp->szLongName[0] ? pProp->szLongName : pProp->PropHead.szName ); 
      ANSITOOEM( pProp->szLongName ) ;

      m_LangTmgr2Tmx.FindName( m_szDocSourceLang, szName, sizeof(szName) );
      m_xw->WriteAttributeString( "source-language", szName ); 

      m_LangTmgr2Tmx.FindName( m_szDocTargetLang, szName, sizeof(szName) );
      m_xw->WriteAttributeString( "target-language", szName ); 

      m_xw->WriteAttributeString( "tool", "OpenTM2" ); 


      // other document properties
      m_xw->WriteStartElement( "header" );
      m_xw->WriteStartElement( TM_PROPERTIES );
      WriteNonEmptyElement( TM_FOLDER,      this->m_szLongFolName ); 
      WriteNonEmptyElement( TM_MARKUP,      m_szDocMarkup ); 
      WriteNonEmptyElement( TM_MEMORY,      pProp->szLongMemory );
      WriteNonEmptyElement( TM_SOURCELANG,  m_szDocSourceLang );
      WriteNonEmptyElement( TM_TARGETLANG,  m_szDocTargetLang );

      m_xw->WriteElementString( TM_SHORTNAME, m_szShortDocName );
      WriteNonEmptyElement( TM_ALIAS,       pProp->szAlias );
      WriteNonEmptyElement( TM_EDITOR,      pProp->szEditor );
      WriteNonEmptyElement( TM_CONVERSION,  pProp->szConversion );
      WriteNonEmptyElement( TM_VENDOR,      pProp->szVendor );
      WriteNonEmptyElement( TM_VENDOREMAIL, pProp->szVendorEMail );
      WriteNonEmptyElement( TM_SHIPMENT,    pProp->szShipment );
      m_xw->WriteEndElement(); // TM_PROPERTIES
      m_xw->WriteEndElement(); // "header" 

      CloseProperties( hProp, PROP_QUIT, &ulErrorInfo);
    } /* endif */
  } /* endif */


  // load segments source and target file
  iRC = m_Util->LoadSegmentedFile( m_szSegTargetFile, &pTargetDoc, TRUE );
  if ( !iRC ) iRC = m_Util->LoadSegmentedFile( m_szSegSourceFile, &pSourceDoc, FALSE );

  // load document tag table
  PLOADEDTABLE pTagTable = NULL;
  if ( !iRC ) 
  {
    iRC = (SHORT)TALoadTagTableExHwnd( m_szDocMarkup, (PLOADEDTABLE *)&pTagTable, FALSE, 
                                       TALOADUSEREXIT | TALOADPROTTABLEFUNC, FALSE, NULLHANDLE );
    if ( iRC ) {                                         /* 7-9-14 */
       PSZ pszParm = m_szDocMarkup;
       iRC = ERROR_FORMAT_NOTFOUND;
       UtlErrorHwnd( (SHORT)iRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, m_pData->hwndErrMsg );
    }
  }

  // open memory (if requested)
  if ( !iRC && m_pData->fWithFolderMem )
  {
    OpenMem( m_szDocMemory );
  } /* endif */

  // write translation units
  m_xw->WriteStartElement( "body" );
  if ( !iRC )
  {
    ULONG ulSegNum = 1;
    char  szMatchInfo[20] ;
    char  szMatchRatio[6] ;
    PTBSEGMENT pTargetSeg = EQFBGetSegW( pTargetDoc, ulSegNum );
    while ( pTargetSeg )
    {
      PTBSEGMENT pSourceSeg = EQFBGetSegW( pSourceDoc, ulSegNum );

      if ( pSourceSeg )
      {
        if ( pTargetSeg->SegFlags.Joined && !pTargetSeg->SegFlags.JoinStart )
        {
          // ignore joined segments
        }
        else if ( (pTargetSeg->qStatus == QF_XLATED) || (pTargetSeg->qStatus == QF_TOBE) || 
                  (pTargetSeg->qStatus == QF_ATTR) || (pTargetSeg->qStatus == QF_CURRENT) )

        {
          CHAR szSegNum[10];
          m_xw->WriteStartElement( "trans-unit" );
          sprintf( szSegNum, "%lu", ulSegNum );
          m_xw->WriteAttributeString( "id", szSegNum );
          m_xw->WriteAttributeString( "translate", "yes" );

          switch ( pTargetSeg->qStatus )
          {
            case QF_XLATED:
              m_xw->WriteAttributeString( TM_SEGSTATUS, "XLATED" );
              break;
            case QF_TOBE:
              m_xw->WriteAttributeString( TM_SEGSTATUS, "TOBE" );
              break;
            case QF_NOP:
              m_xw->WriteAttributeString( TM_SEGSTATUS, "NOP" );
              break;
            case QF_ATTR:
              m_xw->WriteAttributeString( TM_SEGSTATUS, "ATTR" );
              break;
            case QF_CURRENT:
              m_xw->WriteAttributeString( TM_SEGSTATUS, "CURRENT" );
              break;
            default:
                break;
          } /*endswitch */

///       if ( pTargetSeg->CountFlag.AnalAutoSubst || pTargetSeg->CountFlag.EditAutoSubst )
///       {
///          m_xw->WriteAttributeString( TM_MATCHINFO, "EXIST" );
///       }
///       else if ( pTargetSeg->CountFlag.ExactExist )
///       {
///          m_xw->WriteAttributeString( TM_MATCHINFO, "EXACT" );
///       }
///       else if ( pTargetSeg->CountFlag.GlobMemExist )
///       {
///          m_xw->WriteAttributeString( TM_MATCHINFO, "GLOBALMEMORY" );
///       }
///       else if ( pTargetSeg->CountFlag.FuzzyExist )
///       {
///          m_xw->WriteAttributeString( TM_MATCHINFO, "FUZZY" );
///       }
///       else if ( pTargetSeg->CountFlag.MachExist )
///       {
///          m_xw->WriteAttributeString( TM_MATCHINFO, "MACHINE" );
///       }
///       else if ( pTargetSeg->CountFlag.ReplExist )
///       {
///          m_xw->WriteAttributeString( TM_MATCHINFO, "REPLACE" );
///       }
///       else
///       {
///          m_xw->WriteAttributeString( TM_MATCHINFO, "NEW" );
///       } /* endif */


          if ( pTargetSeg->qStatus == QF_NOP )
          {
            strcpy( szMatchInfo, "PROTECTED" );
          }
          else if ( pTargetSeg->qStatus == QF_XLATED )
          {
            if ( (pTargetSeg->CountFlag.AnalAutoSubst || pTargetSeg->CountFlag.EditAutoSubst) && pTargetSeg->CountFlag.PropChanged )
            {
              strcpy( szMatchInfo, "MODIFIEDAUTOSUBST" );
              strcpy( szMatchRatio, "100" ); 
            }
            else if ( pTargetSeg->CountFlag.AnalAutoSubst || pTargetSeg->CountFlag.EditAutoSubst )
            {
          /// if ( pData->Options.fExactFromManual && IsFromManualTranslation( pData, pSourceSeg->pDataW, pTargetSeg->pDataW ) )
          /// {
          ///   strcpy( szMatchInfo, "ExactFromManual" );
          /// }
          /// else
              {
                strcpy( szMatchInfo, "AUTOSUBST" );
              }
              strcpy( szMatchRatio, "100" ); 
            }
         ///else if ( pTargetSeg->CountFlag.ExactCopy && pTargetSeg->CountFlag.PropChanged )
         ///{
         ///  strcpy( szMatchInfo, "MODIFIEDEXACT" );
         ///  strcpy( szMatchRatio, "100" );
         ///}
            else if ( pTargetSeg->CountFlag.ExactCopy )
            {
          /// if ( pData->Options.fExactFromManual && IsFromManualTranslation( pData, pSourceSeg->pDataW, pTargetSeg->pDataW ) )
          /// {
          ///   strcpy( szMatchInfo, "ExactFromManual" );
          /// }
          /// else
              {
                strcpy( szMatchInfo, "EXACT" );
              }
              strcpy( szMatchRatio, "100" );
            }
            else if ( pTargetSeg->CountFlag.FuzzyCopy )
            {
              strcpy( szMatchInfo, "FUZZY" );
              if ( pTargetSeg->CountFlag.Fuzzy9199 )
              {
              //strcpy( szMatchRatio, "91-99" );
                strcpy( szMatchRatio, "95" );
              }
              else if ( pTargetSeg->CountFlag.Fuzzy7190 )
              {
              //strcpy( szMatchRatio, "71-90" );
                strcpy( szMatchRatio, "80" );
              }
              else if ( pTargetSeg->CountFlag.Fuzzy5070 )
              {
              //strcpy( szMatchRatio, "50-70" );
              strcpy( szMatchRatio, "60" );
              } /* endif */
            }
         // else if ( pTargetSeg->CountFlag.GlobMemCopy && pTargetSeg->CountFlag.PropChanged )
         // {
         //   strcpy( szMatchInfo, "GLOBALMEMORYMODIFIED" );
         //   strcpy( szMatchRatio, "100" ); 
         // }
            else if ( pTargetSeg->CountFlag.GlobMemCopy )
            {
              strcpy( szMatchInfo, "GLOBALMEMORY" );
              strcpy( szMatchRatio, "100" ); 
            }
            else if ( pTargetSeg->CountFlag.MachCopy )
            {
              strcpy( szMatchInfo, "MACHINE" );
              strcpy( szMatchRatio, "100" );
            }
            else if ( pTargetSeg->CountFlag.ReplExist )
            {
              strcpy( szMatchInfo, "REPLACE" );
              strcpy( szMatchRatio, "100" );
            }
            else
            {
              strcpy( szMatchInfo, "NEW" );
              strcpy( szMatchRatio, "0" );
            } /* endif */
          }
          else
          {
            strcpy( szMatchInfo, "NOTTRANSLATED" );
            strcpy( szMatchRatio, "0" );
          } /* endif */
          m_xw->WriteAttributeString( TM_MATCHINFO, szMatchInfo );
          m_xw->WriteAttributeString( TM_MATCHRATIO, szMatchRatio );



          // source
          m_xw->WriteStartElement( "source" );
          m_xw->Formatting = CXmlWriter::None;
          WriteSegmentText( pSourceSeg->pDataW, pTagTable, (pTargetSeg->qStatus == QF_NOP), ulCP );
          m_xw->WriteEndElement(); // "source" 
          m_xw->Formatting = CXmlWriter::Indented;

          // target
          if ( pTargetSeg->qStatus == QF_XLATED ) 
          {
            m_xw->WriteStartElement( "target" );
            m_xw->WriteAttributeString( "state", "translated" );
            m_xw->Formatting = CXmlWriter::None;
            WriteSegmentText( pTargetSeg->pDataW, pTagTable, (pTargetSeg->qStatus == QF_NOP), ulCP );
            m_xw->WriteEndElement(); // "target" 
            m_xw->Formatting = CXmlWriter::Indented;
          }
          else
          {
            // GQ: currently no target for not-translated segments
            //m_xw->WriteStartElement( "target" );
            //m_xw->WriteAttributeString( "state", "needs-translation" );
            //m_xw->Formatting = CXmlWriter::None;
            //WriteSegmentText( pTargetSeg->pDataW, pTagTable, (pTargetSeg->qStatus == QF_NOP), ulCP );
            //m_xw->WriteEndElement(); // "target" 
            //m_xw->Formatting = CXmlWriter::Indented;
          } /* endif */

          // count source words and write count group
          {
            ULONG ulWords = 0;
            ULONG ulMarkup = 0; 
            CHAR szNumber[20];

            EQFBWordCntPerSeg( pTagTable, (PTOKENENTRY)m_szBuffer, pSourceSeg->pDataW, sLangID, &ulWords, &ulMarkup, m_ulSourceCP );

            m_xw->WriteStartElement( "count-group" );
            m_xw->WriteAttributeString( "name", "word count" );
            m_xw->WriteStartElement( "count" );
            m_xw->WriteAttributeString( "count-type", "word count" );
            m_xw->WriteAttributeString( "unit", "word" );
            sprintf( szNumber, "%lu", ulWords );
            m_xw->WriteString( szNumber );
            m_xw->WriteEndElement(); //  "count"
            m_xw->WriteEndElement(); // "count-group" 
          } /* endif */

          // memory proposals
          if ( ( m_pData->fWithFolderMem ) ||
               ( m_pData->fWithROMem     ) ) 
          {
            WriteMemProposal( pSourceSeg, pTagTable, ulCP );
          } /* endif */

          m_xw->WriteEndElement(); // "trans-unit" 
        } /* endif */
      } /* endif */

      // next segment
      ulSegNum++;
      pTargetSeg = EQFBGetSegW( pTargetDoc, ulSegNum );
    } /*endwhile */
  } /* endif */
  m_xw->WriteEndElement(); // "body" 

  // end document
  m_xw->WriteEndElement(); // "file" 

  // cleanup
  if ( pTagTable ) TAFreeTagTable( pTagTable );
  if ( sLangID >= 0 ) MorphFreeLanguageID( sLangID );
  if ( pSourceDoc ) m_Util->FreeSegmentedFile( &pSourceDoc );
  if ( pTargetDoc ) m_Util->FreeSegmentedFile( &pTargetDoc );

  // open memory (if requested)
  if ( m_pData->fWithFolderMem )
  {
    CloseMem();
  } /* endif */

  return( iRC );
}

int CXLIFFExportImport::WriteSegmentText
(
  PSZ_W             pszSegmentData,   // pointer to document short name
  PLOADEDTABLE      pTagTable,        // tag table to use for inline tag recognition
  BOOL              fProtected,
  ULONG             ulCP
)
{
  int iRC = 0;

  if ( fProtected )
  {
    m_xw->WriteString( pszSegmentData ); 
  }
  else
  {
    // build protect start/stop table for tag recognition
    PBYTE  pStartStop = NULL;
    //iRC = (int)TACreateProtectTableWEx( pszSegmentData, pTagTable, 0, (PTOKENENTRY)m_pTokBuf, TMXTOKBUFSIZE,
    //                                   (PSTARTSTOP *)&pStartStop, pTagTable->pfnProtTable, 
    //                                   pTagTable->pfnProtTableW, ulCP, CREATEPROTTABLE_MARKATTR );
    iRC = (int)TACreateProtectTableW( pszSegmentData, pTagTable, 0, (PTOKENENTRY)m_pTokBuf, TMXTOKBUFSIZE,
                                       (PSTARTSTOP *)&pStartStop, pTagTable->pfnProtTable, 
                                       pTagTable->pfnProtTableW, ulCP);


    if ( !iRC )
    {
      PSTARTSTOP pEntry = (PSTARTSTOP)pStartStop;
      BOOL fSubFlow = FALSE;
      BOOL fNewSubFlow = FALSE;
      while ( (pEntry->usStart != 0) || (pEntry->usStop != 0)  || (pEntry->usType != 0) )
      {
        switch ( pEntry->usType )
        {
          case UNPROTECTED_CHAR :
            // write translatable text
            {
              CHAR_W chTemp = pszSegmentData[pEntry->usStop+1];
              pszSegmentData[pEntry->usStop+1] = 0;
              m_xw->WriteString( pszSegmentData + pEntry->usStart ); 
              pszSegmentData[pEntry->usStop+1] = chTemp;
              pEntry++;
            }
            break;
          default :
            // handle not-translatable data
            // loop over all protected entries and handle subflows
            m_xw->WriteStartElement( "ph" );
            fNewSubFlow = fSubFlow = FALSE;
            BOOL fEnd = FALSE;
            while ( !fEnd && (pEntry->usType != UNPROTECTED_CHAR) )
            {
              // get subflow flag
              fNewSubFlow = ((pEntry->usType & 0x8000) != 0);
              if ( fNewSubFlow != fSubFlow )
              {
                if ( fNewSubFlow )
                {
                  m_xw->WriteStartElement( "sub" );
                }
                else
                {
                  m_xw->WriteEndElement(); // "sub" 
                } /* endif */
                fSubFlow = fNewSubFlow;
              } /* endif */
              pEntry->usType = pEntry->usType & 0x7FFF;

              // write text of current element
              CHAR_W chTemp = pszSegmentData[pEntry->usStop+1];
              pszSegmentData[pEntry->usStop+1] = 0;
              m_xw->WriteString( pszSegmentData + pEntry->usStart ); 
              pszSegmentData[pEntry->usStop+1] = chTemp;

              // next entry
              pEntry++;
              fEnd = (pEntry->usStart == 0) && (pEntry->usStop == 0) && (pEntry->usType == 0);
            } /*endwhile */
            if ( fSubFlow )
            {
              m_xw->WriteEndElement(); // "sub" 
            } /* endif */
            m_xw->WriteEndElement(); // "ph" 
            break;
        } /* endswitch */
      } /* endwhile */
    } /* endif */
    UtlAlloc( (PVOID *)&pStartStop, 0L, 0L, NOMSG );
  } /* endif */

  return( iRC );
}

int CXLIFFExportImport::WriteNonEmptyElement
(
  PSZ  pszElement,
  PSZ  pszValue
)
{
  int iRC = 0;
  if ( *pszValue )
  {
    m_xw->WriteElementString( pszElement, pszValue );
  } /* endif */
  return( iRC );
}

int CXLIFFExportImport::OpenMem
( 
  PSZ              pszMemory           // name of memory being opened
)
{
  int              iRC = 0;

  if ( strcmp( m_szCurMemory, pszMemory ) != 0 )
  {
    // close any open document memory database                     
    CloseMem();

    // open memory 
    {
      MemoryFactory *pFactory = MemoryFactory::getInstance();
      m_pMem = pFactory->openMemory( NULL, pszMemory, NONEXCLUSIVE, &iRC );

      if ( !iRC )
      {
        strcpy( m_szCurMemory, pszMemory );
      }
      else
      {
        // memory not accessible or not found
        UtlErrorHwnd( EQFS_TM_NOT_FOUND, MB_CANCEL, 1, &pszMemory, EQF_ERROR, this->m_pData->hwndErrMsg );
        iRC = EQFS_TM_NOT_FOUND;
      } /* endif */
    }
  } /* endif */

  return( iRC );
}

int CXLIFFExportImport::CloseMem()
{
  int              iRC = 0;

  if ( m_pMem != NULL )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    pFactory->closeMemory( m_pMem );
    m_pMem = NULL;
    m_szCurMemory[0] = '\0';
  } /* endif */
  return( iRC );
}

int CXLIFFExportImport::WriteMemProposal
(
  PTBSEGMENT   pSeg,                             // segment being looked up 
  PLOADEDTABLE pTagTable,                        // document tag table 
  ULONG        ulCP                              // document target language ANSi CP 
  )
{
  int              iRC = 0;
  PLOADEDTABLE     pPropTagTable = NULL;

  if ( ( ( m_pMem == NULL ) &&
         ( ! m_pData->fWithROMem ) ) ||
       ( ( m_iNumOfROMems == 0 ) &&
         ( ! m_pData->fWithFolderMem ) ) )
  {
    return( 0 );
  } /* endif */

  // init and fill tm data
  OtmProposal SearchKey;
  SearchKey.setDocName( m_szLongDocName );
  SearchKey.setDocShortName( m_szShortDocName );
  SearchKey.setSegmentNum( pSeg->ulSegNum );
  SearchKey.setSource( pSeg->pDataW );
  SearchKey.setMarkup( m_szDocMarkup );
  SearchKey.setSourceLanguage( m_szDocSourceLang );
  SearchKey.setTargetLanguage( m_szDocTargetLang );

  // get memory proposals
  MemoryFactory *pFactory = MemoryFactory::getInstance();
  OtmProposal::clearAllProposals( m_BestProposals );

  if ( m_pData->fWithFolderMem ) 
  {
     iRC = m_pMem->searchProposal( SearchKey, m_BestProposals, GET_NO_GENERICREPLACE | GET_ALWAYS_WITH_TAGS );
  }

  if ( !iRC && m_iNumOfROMems > 0 ) 
  {
    int i = 0 ;
    while ( !iRC && (i < m_iNumOfROMems) && (m_MemoryDBs[i] != NULL) )
    {
      OtmProposal::clearAllProposals( m_FoundProposals );
      iRC = m_MemoryDBs[i]->searchProposal( SearchKey, m_FoundProposals, GET_NO_GENERICREPLACE | GET_ALWAYS_WITH_TAGS );
      pFactory->copyBestMatches( m_FoundProposals, m_BestProposals, 3 );
      i++;
    } 
  }

  // write proposals to output file
  int iNumOfProposals = OtmProposal::getNumOfProposals( m_BestProposals );

  if ( !iRC && iNumOfProposals )
  {
    CHAR szTemp[10];
    int iMatch = 0;

    while ( iMatch < iNumOfProposals )
    {
      m_BestProposals[iMatch]->getMarkup( m_szPropMarkup, sizeof(m_szPropMarkup) );
      m_BestProposals[iMatch]->getMarkup( m_szPropMarkup, sizeof(m_szPropMarkup) );
      m_BestProposals[iMatch]->getDocName( m_szPropDocument, sizeof(m_szPropDocument) );
      m_BestProposals[iMatch]->getSource( m_szPropSource, sizeof(m_szPropSource)/sizeof(CHAR_W) );
      m_BestProposals[iMatch]->getTarget( m_szPropTarget, sizeof(m_szPropTarget)/sizeof(CHAR_W) );

      if ( strcmp( m_szDocMarkup, m_szPropMarkup ) != 0 ) 
      {
         TALoadTagTableExHwnd( m_szPropMarkup, (PLOADEDTABLE *)&pPropTagTable, FALSE, 
                               TALOADUSEREXIT | TALOADPROTTABLEFUNC, FALSE, NULLHANDLE );

      } /* endif */

      m_xw->WriteStartElement( "alt-trans" );
      if ( m_BestProposals[iMatch]->getFuzziness() >= 100 )
      {
        sprintf( szTemp, "1.%2.2ld", m_BestProposals[iMatch]->getFuzziness()-100 );
        m_xw->WriteAttributeString( "match-quality", szTemp );
      }
      else
      {
        sprintf( szTemp, "0.%2.2ld", m_BestProposals[iMatch]->getFuzziness() );
        m_xw->WriteAttributeString( "match-quality", szTemp );
      } /* endif */

      m_xw->WriteAttributeString( "origin", m_szPropDocument );
      m_xw->WriteAttributeString( TM_MARKUP, m_szPropMarkup );
      if ( m_BestProposals[iMatch]->getType() == OtmProposal::eptMachine ) m_xw->WriteAttributeString( TM_MACHMATCH, "true" );
      sprintf( szTemp, "%ld", m_BestProposals[iMatch]->getSegmentNum() );
      m_xw->WriteAttributeString( TM_SEGNUM, szTemp );
      sprintf( szTemp, "%ld", m_BestProposals[iMatch]->getUpdateTime() );
      m_xw->WriteAttributeString( TM_DATETIME, szTemp );


      m_xw->WriteStartElement( "source" );
      m_xw->Formatting = CXmlWriter::None;
      WriteSegmentText( m_szPropSource, ( pPropTagTable ) ? pPropTagTable : pTagTable, FALSE, ulCP );
      m_xw->WriteEndElement(); // "source" 
      m_xw->Formatting = CXmlWriter::Indented;

      m_xw->WriteStartElement( "target" );
      m_xw->Formatting = CXmlWriter::None;
      WriteSegmentText( m_szPropTarget, ( pPropTagTable ) ? pPropTagTable : pTagTable, FALSE, ulCP );
      m_xw->WriteEndElement(); // "target" 
      m_xw->Formatting = CXmlWriter::Indented;

      m_xw->WriteEndElement(); // "alt-trans" 

      iMatch++;
    } /*endwhile */

  } /* endif */

  return( iRC );
}

// check if file is in Unicode encoding
static BOOL IsUnicodeFile( char *pszFile )
{
  BOOL fIsUnicode = FALSE;
  FILE *hf = NULLHANDLE;

  hf = fopen( pszFile, "rb" );

  if ( hf )
  {
    // get first two bytes and check for BOM
    BYTE chBOM[2];

    fread( chBOM, 1, 2, hf );

    if ( memcmp( chBOM, UNICODEFILEPREFIX, 2 ) == 0 )
    {
      fIsUnicode = TRUE;
    }
    fclose( hf );
  } /* endif */
  return( fIsUnicode );
}

static PSZ ScanForKeyword
( 
  FILE        *hf,                     // handle of file
  PSZ         pszLine,                 // ptr to line buffer
  int         iSize,                   // size of line buffer
  PSZ         pszKeyword,              // keyword being looked for
  int         iMaxLines                // max number of lines to check
)
{
  PSZ          pszFound = NULL;

  fgets( pszLine, iSize, hf );
  while ( !feof( hf ) && iMaxLines && (pszFound == NULL) )
  {
    PSZ pszTest = pszLine;
    PSZ pszCurChar = pszKeyword;
    PSZ pszStart = pszTest;
    CHAR chCurChar = (CHAR)toupper(*pszCurChar);
    while ( (*pszTest != '\0') && (*pszCurChar != '\0') )
    {
      if ( toupper(*pszTest) == chCurChar )
      {
        // try next character
        pszTest++;
        pszCurChar++;
        chCurChar = (CHAR)toupper(*pszCurChar);
      }
      else
      {
        // continue with next position
        pszStart++;
        pszTest = pszStart;
        pszCurChar = pszKeyword;
      } /* endif */
    } /*endwhile */

    if ( *pszCurChar == '\0' )
    {
      // we found our keyword 
      pszFound = pszStart;
    }
    else
    {
      // continue with next line
      fgets( pszLine, iSize, hf );
      iMaxLines--; 
    } /* endif */
  } /*endwhile */

  return( pszFound );
}

static PSZ_W ScanForKeywordW
( 
  FILE        *hf,                     // handle of file
  PSZ_W       pszLine,                 // ptr to line buffer
  int         iSize,                   // size of line buffer
  PSZ_W       pszKeyword,              // keyword being looked for
  int         iMaxLines                // max number of lines to check
)
{
  PSZ_W       pszFound = NULL;

  fgetws( pszLine, iSize, hf );
  while ( !feof( hf ) && iMaxLines && (pszFound == NULL) )
  {
    PSZ_W pszTest = pszLine;
    PSZ_W pszCurChar = pszKeyword;
    PSZ_W pszStart = pszTest;
    CHAR_W chCurChar = (CHAR_W)towupper(*pszCurChar);
    while ( (*pszTest != L'\0') && (*pszCurChar != L'\0') )
    {
      if ( towupper(*pszTest) == chCurChar )
      {
        // try next character
        pszTest++;
        pszCurChar++;
        chCurChar = (CHAR_W)towupper(*pszCurChar);
      }
      else
      {
        // continue with next position
        pszStart++;
        pszTest = pszStart;
        pszCurChar = pszKeyword;
      } /* endif */
    } /*endwhile */

    if ( *pszCurChar == L'\0' )
    {
      // we found our keyword 
      pszFound = pszStart;
    }
    else
    {
      // continue with next line
      fgetws( pszLine, iSize, hf );
      iMaxLines--; 
    } /* endif */
  } /*endwhile */

  return( pszFound );
}
