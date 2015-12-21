//+---------------------------------------------------------------------------+
//| OTMXLFMT.CPP                                                               |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: David Walters                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Memory import for XLIFF machine translation returns.          |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//

// use non-ISO version of swprintf which was used up to VS2005, once older versions are not used anymore we
// should use the new version of the function asap
#define _CRT_NON_CONFORMING_SWPRINTFS


#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

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


#ifndef CPPTEST
extern "C"
{
#endif
  #pragma pack( push, TM2StructPacking, 1 )

  #define INCL_EQF_TM               // general Transl. Memory functions
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_TAGTABLE         // tagtable defines and functions
  #define INCL_EQF_MORPH            // morphologic functions
  #include "eqf.h"                  // General .H for EQF
  #include "EQFMEMIE.H"
  #include "OTMFUNC.H"

  #pragma pack( pop, TM2StructPacking )
#ifndef CPPTEST
}
#endif

#include "cxmlwriter.H"

// size of token buffer
#define XLFTOKBUFSIZE 32000

// size of file read buffer in preprocess step
#define XLF_BUFFER_SIZE 8096

// names of XLIFF attributes

#define CATEGORY_ATTR         L"category"
#define ID_ATTR               L"id"
#define NAME_ATTR             L"name"
#define MATCHQUALITY_ATTR     L"matchQuality"
#define ORIGIN_ATTR           L"origin"
#define ORIGINAL_ATTR         L"original"
#define SRCLANG_ATTR          L"srcLang"
#define STATE_ATTR            L"state"
#define TGTLANG_ATTR          L"tgtLang"
#define TYPE_ATTR             L"type"
#define VERSION_ATTR          L"version"

// names of XLIFF attribute values

#define OPENTM2_VAL           L"OpenTM2"
#define OTM_VAL               L"otm"
#define CONTEXT_VAL           L"context"
#define DATETIME_VAL          L"datetime"
#define DOCNAME_VAL           L"docname"
#define DOCSEGNUM_VAL         L"docsegnum"
//#define DOCSHORTNAME_VAL      L"docshortname"
#define FOLDER_VAL            L"folder"
#define MARKUP_VAL            L"markup"
#define SEGNUM_VAL            L"segnum"
//#define SHORTNAME_VAL         L"shortname"
#define SRCLANG_VAL           L"srclang"
#define TGTLANG_VAL           L"tgtlang"
#define TRANSLATED_VAL        L"translated"
#define INITIAL_VAL           L"initial"
#define ICM_VAL               L"icm"
#define IDM_VAL               L"idm"
#define MT_VAL                L"mt"

// IDs of XLIFF elements
typedef enum { XLIFF_ELEMENT, FILE_ELEMENT, UNIT_ELEMENT, SEGMENT_ELEMENT, SOURCE_ELEMENT, TARGET_ELEMENT,
               META_ELEMENT, METADATA_ELEMENT, METAGROUP_ELEMENT,
               MATCH_ELEMENT, MATCHES_ELEMENT, 
               INVCHAR_ELEMENT, UNKNOWN_ELEMENT } ELEMENTID;

typedef struct _XLFNAMETOID
{
  CHAR_W   szName[30];                 // name of element
  ELEMENTID ID;                        // ID of element 
} XLFNAMETOID, *PXLFNAMETOID;

XLFNAMETOID XlfNameToID[] =
{ { L"xliff",          XLIFF_ELEMENT },
  { L"file",           FILE_ELEMENT }, 
  { L"unit",           UNIT_ELEMENT }, 
  { L"segment",        SEGMENT_ELEMENT }, 
  { L"source",         SOURCE_ELEMENT }, 
  { L"target",         TARGET_ELEMENT }, 
  { L"mda:meta",       META_ELEMENT }, 
  { L"mda:metadata",   METADATA_ELEMENT }, 
  { L"mda:metagroup",  METAGROUP_ELEMENT },
  { L"mtc:match",      MATCH_ELEMENT }, 
  { L"mtc:matches",    MATCHES_ELEMENT }, 
  { L"invchar",        INVCHAR_ELEMENT }, 
  { L"",               UNKNOWN_ELEMENT } };


//
// class for our SAX handler
//
class XLFParseHandler : public HandlerBase
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  XLFParseHandler();
  virtual ~XLFParseHandler();

  // setter functions for import info
  void SetMemInfo( PMEMEXPIMPINFO m_pMemInfo ); 
  void SetMemInterface( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, PTOKENENTRY pTokBuf, int iTokBufSize ); 
  void SetSourceLanguage( char *pszSourceLang );

  // getter functions 
  void GetSourceLanguage( char *pszSourceLang, int iBufSize );
  BOOL IsValidFormat( void );
  BOOL IsHeaderDone( void );
  BOOL ErrorOccured( void );
  void GetErrorText( char *pszTextBuffer, int iBufSize );

  // -----------------------------------------------------------------------
  //  Handlers for the SAX DocumentHandler interface
  // -----------------------------------------------------------------------
  void startElement(const XMLCh* const name, AttributeList& attributes);
  void endElement(const XMLCh* const name );
  void characters(const XMLCh* const chars, const XMLSize_t length);


  // -----------------------------------------------------------------------
  //  Handlers for the SAX ErrorHandler interface
  // -----------------------------------------------------------------------
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc );
  void fatalError(const SAXParseException& exc);


private:
  ELEMENTID GetElementID( PSZ_W pszName );
  BOOL GetValueW( PCHAR_W pszString, int iLen, LONG *piResult );

  // mem import interface data
  PMEMEXPIMPINFO m_pMemInfo;
  PFN_MEMINSERTSEGMENT pfnInsertSegment;
  LONG lMemHandle;

  // processing flags
  BOOL fSource;                        // TRUE = source data collected  
  BOOL fTarget;                        // TRUE = target data collected
  BOOL fCatchData;                     // TRUE = catch data
  BOOL fValidFormat;                   // TRUE = <meta> for OpenTM2 has been processed
  BOOL fHeaderDone;                    // TRUE = <xliff> element is done
  BOOL fKeepMeta;                      // TRUE = "OTM" meta info found
  BOOL fError;                         // TRUE = parsing ended with error

  // segment data
  LONG  lTime;                         // segment date/time
  int   iLastElement;                  // type of last XLIFF parent element

  // buffers
  #define DATABUFFERSIZE 4098

  typedef struct _FOLDER_INFO
  {
    CHAR_W   szName[512];             // Folder name
//  CHAR_W   szShortName[13];         // Folder short name
    CHAR_W   szSrcLocale[10];         // XLIFF source locale (en-US)
    CHAR_W   szTgtLocale[10];         // XLIFF target locale (de-DE)
    CHAR_W   szSrcLang[50];           // XLIFF source language
    CHAR_W   szTgtLang[50];           // XLIFF target language
    CHAR_W   szOTMVersion[50];        // Version of OpenTM2 which created this file
  } FOLDER_INFO, *PFOLDER_INFO;

  typedef struct _FILE_INFO
  {
    CHAR_W   szName[512];             // File name
//  CHAR_W   szShortName[13];         // File short name
    CHAR_W   szMarkup[40];            // OpenTM2 markup table
    CHAR_W   szSrcLang[50];           // OpenTM2 source language
    CHAR_W   szTgtLang[50];           // OpenTM2 target language
  } FILE_INFO, *PFILE_INFO;

  typedef struct _UNIT_INFO
  {
    CHAR_W   szId[10];                // XLIFF sequential ID
    CHAR_W   szSegmentNum[10];        // Original OpenTM2 segment number
    BOOL     bMatches;                // TRUE=Memory matches are available
  } UNIT_INFO, *PUNIT_INFO;

  typedef struct _SEGMENT_INFO
  {
    CHAR_W   szState[40];               // Translation state
    CHAR_W   szSource[DATABUFFERSIZE];  // Source text
    CHAR_W   szTarget[DATABUFFERSIZE];  // Target text
    BOOL     bInvalidSourceChar ;       // TRUE=Invalid source character
    BOOL     bInvalidTargetChar ;       // TRUE=Invalid target character
  } SEGMENT_INFO, *PSEGMENT_INFO;


  PFOLDER_INFO    pFolderInfo; 
  PFILE_INFO      pFileInfo; 
  PUNIT_INFO      pUnitInfo; 
  PSEGMENT_INFO   pSegmentInfo; 

  ELEMENTID       CurElementId ;
  CHAR_W          *pInfo ;  
  int             iInfoSize ;
  int             iLenSource ;
  int             iLenTarget ;

  typedef struct _BUFFERAREAS
  {
    CHAR_W   szData[DATABUFFERSIZE];  // buffer for collected data
    CHAR     szDocument[EQF_DOCNAMELEN];// buffer for document name
    MEMEXPIMPSEG SegmentData;         // buffer for segment data
    CHAR     szDescription[1024];     // buffer for memory descripion
    CHAR     szMemSourceLang[50];     // buffer for memory source language
    CHAR     szErrorMessage[1024];    // buffer for error message text
    CHAR_W   szNote[MAX_SEGMENT_SIZE];// buffer for note text
    CHAR_W   szNoteStyle[100];        // buffer for note style
    CHAR_W   szMTMetrics[MAX_SEGMENT_SIZE];// buffer for MT metrics data
  } BUFFERAREAS, *PBUFFERAREAS;

  PBUFFERAREAS pBuf; 

  bool      fSawErrors;
  FILE      *hfLog;

  // data for remove tag function
  PTOKENENTRY pTokBuf;
  int iTokBufSize;

  void fillSegmentInfo( PFOLDER_INFO pFolderInfo, PFILE_INFO pFileInfo, 
                        PUNIT_INFO pUnitInfo, PSEGMENT_INFO pSegmentInfo,
                        PMEMEXPIMPSEG pSegment );

};


//+----------------------------------------------------------------------------+
//| Our XLIFF import class                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
class CXLFImport
{
  public:
    // constructor/destructor
	  CXLFImport();
	  ~CXLFImport();
    // import methods
    USHORT StartImport( const char *pszInFile, PMEMEXPIMPINFO pMemInfo ); 
    USHORT ImportNext( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, LONG *plProgress ); 
    USHORT EndImport(); 

  protected:
    USHORT PreProcessInFile( const char *pszInFile, const char *pszOutFile );


    CXmlWriter       m_xw;
    XLFParseHandler  *m_handler;                     // our SAX handler 
    SAXParser*       m_parser;
    XMLPScanToken    m_SaxToken; 
    unsigned int     m_iSourceSize;                  // size of source file
    CHAR             m_szActiveTagTable[50];         // buffer for name of currently loaded markup table
    PTOKENENTRY      m_pTokBuf;                      // buffer for TaTagTokenize tokens
    CHAR             m_szInFile[512];                // buffer for input file
    CHAR             m_TempFile[540];                // buffer for temporary file name
    BYTE             m_bBuffer[XLF_BUFFER_SIZE+1];
    PMEMEXPIMPINFO   m_pMemInfo;
    CHAR_W           m_szSegBuffer[MAX_SEGMENT_SIZE+1]; // buffer for the processing of segment data
    int              m_currentTu;                    // export: number of currently processed tu
};


//+----------------------------------------------------------------------------+
//| Interface functions called by OpenTM2                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+

extern "C" 
__declspec(dllexport) USHORT __cdecl EXTMEMIMPORTSTART
( 
  PLONG            plHandle,           // ptr to buffer receiving the handle for the external/import
  PSZ              pszInFile,          // fully qualified file name for the memory being imported
  PMEMEXPIMPINFO   pMemInfo            // pointer to structure receiving general information for the imported memory
)
{
  USHORT           usRC = 0;           // function return code

  plHandle; pszInFile; pMemInfo;

  CXLFImport *pXLFImport = new CXLFImport;

  usRC = pXLFImport->StartImport( pszInFile, pMemInfo ); 

  *plHandle = (LONG)pXLFImport;

  return( usRC );
} /* end of function EXTMEMIMPORTSTART */


extern "C" 
__declspec(dllexport) USHORT __cdecl EXTMEMIMPORTPROCESS
( 
  LONG             lHandle,                 // Import handle as set by ExtMemImportStart function
  PFN_MEMINSERTSEGMENT pfnInsertSegment,    // callback function to insert segments into the memory
  LONG             lMemHandle,              // memory handle (has to be passed to pfnInsertSegment function)
  PLONG            plProgress               // caller's progress indicator
)
{
  USHORT           usRC = 0;           // function return code

  CXLFImport *pXLFImport = (CXLFImport *)lHandle;

  usRC = pXLFImport->ImportNext( pfnInsertSegment, lMemHandle, plProgress ); 

  return( usRC );
} /* end of function EXTMEMIMPORTPROCESS */


extern "C" 
__declspec(dllexport) USHORT __cdecl EXTMEMIMPORTEND
( 
  LONG             lHandle                  // Import/import handle as set by ExtMemImportStart function
)
{
  USHORT           usRC = 0;           // function return code

  CXLFImport *pXLFImport = (CXLFImport *)lHandle;

  usRC = pXLFImport->EndImport(); 

  delete pXLFImport;

  return( usRC );
} /* end of function EXTMEMIMPORTEND */



//+----------------------------------------------------------------------------+
//| Implementation of XLFImport class                                          |
//|                                                                            |
//+----------------------------------------------------------------------------+
CXLFImport::CXLFImport()
{
  m_szActiveTagTable[0] = EOS;
}

CXLFImport::~CXLFImport()
{
}


USHORT CXLFImport::StartImport
( 
  const char *pszInFile, 
  PMEMEXPIMPINFO pMemInfo 
)
{
  USHORT usRC = 0;

  this->m_TempFile[0] = EOS;
  this->m_pMemInfo = pMemInfo;


  // preprocess XLIFF file
  if ( !usRC )
  {
    // setup temporary file name
    UtlMakeEQFPath( this->m_TempFile, NULC, MEM_PATH, NULL );
    strcat( this->m_TempFile, BACKSLASH_STR );
    if ( strchr( (PSZ)pszInFile, '\\' ) == NULL )
    {
      strcat( this->m_TempFile, (PSZ)pszInFile );
    }
    else
    {
      strcat( this->m_TempFile, UtlGetFnameFromPath( (PSZ)pszInFile ) );
    } /* endif */
    strcat( this->m_TempFile, ".Temp" );

    // call preprocess method
    usRC = this->PreProcessInFile( pszInFile, this->m_TempFile );
  } /* endif */

  // get size of input file
  if ( !usRC )
  {
    WIN32_FIND_DATA FindData;
    HANDLE hFind = FindFirstFile( this->m_TempFile, &FindData );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
      m_iSourceSize = FindData.nFileSizeLow;
      FindClose( hFind );
    }
    else
    {
      usRC = ERROR_FILE_NOT_FOUND;
    } /* endif */
  }

  // parse the XLIFF file
  if ( !usRC )
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
      m_handler = new XLFParseHandler();

      // pass language and markup name list to handler
      m_handler->SetMemInfo( m_pMemInfo ); 

      //  install our SAX handler as the document and error handler.
      m_parser->setDocumentHandler( m_handler );
      m_parser->setErrorHandler( m_handler );
      m_parser->setCalculateSrcOfs( TRUE );
      m_parser->setValidationSchemaFullChecking( FALSE );
      m_parser->setDoSchema( FALSE );
      m_parser->setLoadExternalDTD( FALSE );
      m_parser->setValidationScheme( SAXParser::Val_Never );
      m_parser->setExitOnFirstFatalError( FALSE );
      
      strcpy( m_szInFile, pszInFile );

    try
    {
      if (!m_parser->parseFirst( this->m_TempFile, m_SaxToken))
      {
          //XERCES_STD_QUALIFIER cerr << "scanFirst() failed\n" << XERCES_STD_QUALIFIER endl;
          //XMLPlatformUtils::Terminate();
          usRC = ERROR_READ_FAULT;
      }
      else
      {
        // parse until header processed or no more data available
        BOOL fContinue  = TRUE;
        while( fContinue && !m_parser->getErrorCount() && 
               ( !m_handler->IsValidFormat() || !m_handler->IsHeaderDone() ) )
        {
          fContinue = m_parser->parseNext(m_SaxToken);
        } /*endwhile */

        if ( m_handler->ErrorOccured() )
        {
          m_handler->GetErrorText( pMemInfo->szError, sizeof(pMemInfo->szError) );
          pMemInfo->fError = TRUE;
        } /* endif */

        // stop import if no more data to follow
        if ( !fContinue && !m_handler->IsValidFormat() )
        {
          usRC = ERROR_BAD_FORMAT;
        } /* endif */

        // get description and source language of memory
        if ( m_handler->IsValidFormat() )
        {
          // save target memory source language
          CHAR szMemSourceLanguage[50]; 
          strcpy( szMemSourceLanguage, pMemInfo->szSourceLang );

          // get source language of imported memory
          m_handler->GetSourceLanguage( pMemInfo->szSourceLang, sizeof(pMemInfo->szSourceLang)  );

          // always use source language of target memory
          if ( szMemSourceLanguage[0] != EOS)
          {
            m_handler->SetSourceLanguage( szMemSourceLanguage );
          } /* endif */
        } /* endif */
      } /* endif */
    }
    catch (const OutOfMemoryException& )
    {
//        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    catch (const XMLException& toCatch)
    {
      toCatch;

        //XERCES_STD_QUALIFIER cerr << "\nAn error occurred: '" << xmlFile << "'\n"
        //     << "Exception message is: \n"
        //     << StrX(toCatch.getMessage())
        //     << "\n" << XERCES_STD_QUALIFIER endl;
        usRC = ERROR_READ_FAULT;
    }

  } /* endif */


  return( usRC );
}

USHORT CXLFImport::ImportNext
( 
  PFN_MEMINSERTSEGMENT pfnInsertSegment, 
  LONG lMemHandle, 
  LONG *plProgress 
)
{
  USHORT usRC = 0;
  int  iIteration = 10;
  BOOL fContinue  = TRUE;
  int errorCode = 0;
  int errorCount = 0;

  plProgress; 

  m_handler->SetMemInterface( pfnInsertSegment, lMemHandle, this->m_pTokBuf, XLFTOKBUFSIZE ); 
    
    try
    {
      while (fContinue && !m_parser->getErrorCount() && iIteration )
      {
        fContinue = m_parser->parseNext(m_SaxToken);
        iIteration--;
      } /*endwhile */

      errorCount = m_parser->getErrorCount();

      // compute current progress
      if ( !errorCount && fContinue )
      {
        int iPos = (int)m_parser->getSrcOffset();

        INT64 iComplete = (INT64)iPos;
        INT64 iTotal = (INT64)m_iSourceSize;
        INT64 iRatio = iComplete * (INT64)100 / iTotal;
        *plProgress = (USHORT)iRatio;

      } /* endif */

      if ( m_handler->ErrorOccured() )
      {
        m_handler->GetErrorText( m_pMemInfo->szError, sizeof(m_pMemInfo->szError) );
        m_pMemInfo->fError = TRUE;
      } /* endif */

      if ( errorCount || !fContinue )
      {
        m_parser->parseReset(m_SaxToken);
        usRC = MEM_IMPORT_COMPLETE;
      } /* endif */
    }
    catch (const OutOfMemoryException& )
    {
//        XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
        errorCode = 5;
    }
    catch (const XMLException& toCatch)
    {
      toCatch;

        //XERCES_STD_QUALIFIER cerr << "\nAn error occurred: '" << xmlFile << "'\n"
        //     << "Exception message is: \n"
        //     << StrX(toCatch.getMessage())
        //     << "\n" << XERCES_STD_QUALIFIER endl;
        errorCode = 4;
    }
  
  return( usRC );
}

USHORT CXLFImport::EndImport()
{
  USHORT usRC = 0;

  delete m_parser;                     // Delete the parser itself

  delete m_handler;                    // delete the handler  

  XMLPlatformUtils::Terminate();       // And call the termination method

  // remove temporary file
  if ( this->m_TempFile[0] != EOS )
  {
    UtlDelete( this->m_TempFile, 0, FALSE );
  } /* endif */

  return( usRC );
}

// replace invalid characters in input file
USHORT CXLFImport::PreProcessInFile
( 
  const char *pszInFile, 
  const char *pszOutFile
)
{
  USHORT      usRC = 0;
  FILE        *hIn = NULL;
  FILE        *hOut = NULL;

  hIn = fopen( pszInFile, "rb" );
  if ( hIn == NULL )
  {
    usRC = ERROR_FILE_NOT_FOUND;
  } /* endif */

  if ( !usRC )
  {
    hOut = fopen( pszOutFile, "wb" );
    if ( hOut == NULL )
    {
      usRC = ERROR_FILE_NOT_FOUND;
    } /* endif */
  } /* endif */

  // read/write files until done
  if ( !usRC )
  {
    BOOL fFirstRead = TRUE;
    BOOL fUTF16 = FALSE;
    PBYTE pCurPos = m_bBuffer;

    do
    {
      int iBytes = fread( m_bBuffer, 1, XLF_BUFFER_SIZE, hIn );
      pCurPos = m_bBuffer;

      // check encoding 
      if ( (iBytes >= 2) && fFirstRead )
      {
        fFirstRead = FALSE;
        if ( (m_bBuffer[0] == 0xFF) && (m_bBuffer[1] == 0xFE) ) 
        {
          // file starts with BOM
          fUTF16 = TRUE;

          // write BOM to output file and advance buffer pointer
          fwrite( m_bBuffer, 1, 2, hOut );
          pCurPos += 2;
          iBytes  -= 2;
        }
        else if ( (m_bBuffer[0] != 0) && (m_bBuffer[1] == 0) ) 
        {
          // assume UTF16
          fUTF16 = TRUE;
        } /* endif */
      } /* endif */

      // check and write data 
      if ( fUTF16 )
      {
        PSZ_W pszStartPos = (PSZ_W)pCurPos;
        PSZ_W pszCurPos = pszStartPos;
        int iLen = iBytes / 2;
        while ( iLen )
        {
          CHAR_W c = *pszCurPos;

          // is valid XML char
          if ( (c == 0x9) || 
               (c == 0xA) || 
               (c == 0xD) || 
               ((c >= 0x20) && (c <= 0xD7FF)) || 
               ((c >= 0xE000) && (c <= 0xFFFD)) || 
               ((c >= 0x10000) && (c <= 0x10FFFF)))
          {
            pszCurPos++;
            iLen--;
          }
          else
          {
            // write chars up to invalid character
            fwrite( pszStartPos, 2, pszCurPos - pszStartPos, hOut );

            // write invalid character
            fwprintf( hOut, L"<invchar>%u</invchar>", c );

            // set new start position
            pszCurPos++;
            pszStartPos = pszCurPos;
            iLen--;
          } /* endif */
        } /*endwhile */

        // write remaining characters
        if ( pszCurPos != pszStartPos )
        {
          fwrite( pszStartPos, 2, pszCurPos - pszStartPos, hOut );
        } /* endif */
      }
      else
      {
        fwrite( m_bBuffer, 1, iBytes, hOut );
      } /* endif */
    } while ( !feof( hIn ) );
  } /* endif */


  //cleanup
  if ( hIn )  fclose( hIn );
  if ( hOut ) fclose( hOut );

  return( usRC );
}


//
// Implementation of XLIFF SAX parser
//
XLFParseHandler::XLFParseHandler()
{
#ifdef _DEBUG
    hfLog = fopen( "C:\\SAXTEST.LOG", "a" );
    if ( hfLog )
    {
      fprintf( hfLog, "**** SAXTEST ****\n" );
    } /* endif */
  //hfLog = NULLHANDLE;
#else
  hfLog = NULLHANDLE;
#endif 

  // allocate buffer areas
  pFolderInfo = (PFOLDER_INFO)malloc( sizeof(FOLDER_INFO) );
  pFileInfo = (PFILE_INFO)malloc( sizeof(FILE_INFO) );
  pUnitInfo = (PUNIT_INFO)malloc( sizeof(UNIT_INFO) );
  pSegmentInfo = (PSEGMENT_INFO)malloc( sizeof(SEGMENT_INFO) );
  pBuf = (PBUFFERAREAS)malloc( sizeof(BUFFERAREAS) );
  if ( pBuf ) memset( pBuf, 0, sizeof(BUFFERAREAS) );
  fError = FALSE;

  pfnInsertSegment = NULL;
  lMemHandle = 0;
  fValidFormat = FALSE;
  fHeaderDone = FALSE;
  pInfo = NULL ;

}

XLFParseHandler::~XLFParseHandler()
{
  if ( hfLog )  fclose( hfLog );
  if ( pFolderInfo )   free( pFolderInfo );
  if ( pFileInfo )     free( pFileInfo );
  if ( pUnitInfo )     free( pUnitInfo );
  if ( pSegmentInfo )  free( pSegmentInfo );
  if ( pBuf)           free( pBuf );
}

void XLFParseHandler::startElement(const XMLCh* const name, AttributeList& attributes)
{
    PSZ_W pszName = (PSZ_W)name;
    int iAttribs = attributes.getLength(); 
    CurElementId = GetElementID( pszName );

    if ( hfLog ) fprintf( hfLog, "StartElement: %S\n", pszName );


    switch ( CurElementId )
    {

      case XLIFF_ELEMENT:
         memset( pFolderInfo, 0, sizeof(FOLDER_INFO) );
         memset( pFileInfo, 0, sizeof(FILE_INFO) );
         memset( pUnitInfo, 0, sizeof(UNIT_INFO) );
         memset( pSegmentInfo, 0, sizeof(SEGMENT_INFO) );
         for( int i = 0; i < iAttribs; i++ ) {
           PSZ_W pszName = (PSZ_W)attributes.getName( i );
           if ( _wcsicmp( pszName, SRCLANG_ATTR ) == 0 ) {
             PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
             if ( pszValue ) 
                wcscpy( pFolderInfo->szSrcLocale, pszValue ) ;
           } else
           if ( _wcsicmp( pszName, TGTLANG_ATTR ) == 0 ) {
             PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
             if ( pszValue ) 
                wcscpy( pFolderInfo->szTgtLocale, pszValue ) ;
           }
         }
         this->iLastElement = XLIFF_ELEMENT ;
         break;

      case FILE_ELEMENT:
         memset( pFileInfo, 0, sizeof(FILE_INFO) );
         for( int i = 0; i < iAttribs; i++ ) {
           PSZ_W pszName = (PSZ_W)attributes.getName( i );
           if ( _wcsicmp( pszName, ORIGINAL_ATTR ) == 0 ) {
             PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
             if ( pszValue ) 
                wcscpy( pFileInfo->szName, pszValue ) ;
           }
         }
         this->iLastElement = FILE_ELEMENT ;
         fHeaderDone = TRUE ;
         break;

      case UNIT_ELEMENT:
         memset( pUnitInfo, 0, sizeof(UNIT_INFO) );
         for( int i = 0; i < iAttribs; i++ ) {
           PSZ_W pszName = (PSZ_W)attributes.getName( i );
           if ( _wcsicmp( pszName, ID_ATTR ) == 0 ) {
             PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
             if ( pszValue ) 
                wcscpy( pUnitInfo->szId, pszValue ) ;
           }
           if ( _wcsicmp( pszName, NAME_ATTR ) == 0 ) {
             PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
             if ( pszValue ) 
                wcscpy( pUnitInfo->szSegmentNum, pszValue ) ;
           }
         }
         this->iLastElement = UNIT_ELEMENT ;
         break;

      case SEGMENT_ELEMENT:
         memset( pSegmentInfo, 0, sizeof(SEGMENT_INFO) );
         for( int i = 0; i < iAttribs; i++ ) {
           PSZ_W pszName = (PSZ_W)attributes.getName( i );
           if ( _wcsicmp( pszName, STATE_ATTR ) == 0 ) {
             PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
             if ( pszValue )  
                wcscpy( pSegmentInfo->szState, pszValue ) ;
           }
         }
         this->iLastElement = SEGMENT_ELEMENT ;
         break;

      case SOURCE_ELEMENT:
         if ( this->iLastElement == SEGMENT_ELEMENT ) {
            pSegmentInfo->szSource[0] = NULL;  
            pInfo = pSegmentInfo->szSource ;
            iInfoSize = sizeof( pSegmentInfo->szSource ) ;
            fCatchData = TRUE; 
            this->iLastElement = SOURCE_ELEMENT ;
         }
         break;

      case TARGET_ELEMENT:
         if ( this->iLastElement == SEGMENT_ELEMENT ) {
            pSegmentInfo->szTarget[0] = NULL;  
            pInfo = pSegmentInfo->szTarget ;
            iInfoSize = sizeof( pSegmentInfo->szTarget ) ;
            fCatchData = TRUE; 
            this->iLastElement = TARGET_ELEMENT ;
         }
         break;

      case META_ELEMENT:
         if ( this->fKeepMeta ) {
           for( int i = 0; i < iAttribs; i++ ) {
             PSZ_W pszName = (PSZ_W)attributes.getName( i );
             if ( _wcsicmp( pszName, TYPE_ATTR ) == 0 ) {
                PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
                if ( pszValue ) {
                   switch( this->iLastElement ) {
                     case XLIFF_ELEMENT:
                        if ( _wcsicmp( pszValue, OPENTM2_VAL ) == 0 ) {
                           fValidFormat = TRUE ;
                           pFolderInfo->szOTMVersion[0] = NULL;  
                           pInfo = pFolderInfo->szOTMVersion ;
                           iInfoSize = sizeof( pFolderInfo->szOTMVersion ) ;
                           fCatchData = TRUE; 
                        } else
                        if ( _wcsicmp( pszValue, FOLDER_VAL ) == 0 ) {
                           pFolderInfo->szName[0] = NULL;  
                           pInfo = pFolderInfo->szName ;
                           iInfoSize = sizeof( pFolderInfo->szName ) ;
                           fCatchData = TRUE; 
                        } else
//                      if ( _wcsicmp( pszValue, SHORTNAME_VAL ) == 0 ) {
//                         pFolderInfo->szShortName[0] = NULL;  
//                         pInfo = pFolderInfo->szShortName ;
//                         iInfoSize = sizeof( pFolderInfo->szShortName ) ;
//                         fCatchData = TRUE; 
//                      }
                        if ( _wcsicmp( pszValue, SRCLANG_VAL ) == 0 ) {
                           pFolderInfo->szSrcLang[0] = NULL;  
                           pInfo = pFolderInfo->szSrcLang ;
                           iInfoSize = sizeof( pFolderInfo->szSrcLang ) ;
                           fCatchData = TRUE; 
                        } else
                        if ( _wcsicmp( pszValue, TGTLANG_VAL ) == 0 ) {
                           fHeaderDone = TRUE ;
                           pFolderInfo->szTgtLang[0] = NULL;  
                           pInfo = pFolderInfo->szTgtLang ;
                           iInfoSize = sizeof( pFolderInfo->szTgtLang ) ;
                           fCatchData = TRUE; 
                        }
                        break;
                     case FILE_ELEMENT:
                        if ( _wcsicmp( pszValue, MARKUP_VAL ) == 0 ) {
                           pFileInfo->szMarkup[0] = NULL;  
                           pInfo = pFileInfo->szMarkup ;
                           iInfoSize = sizeof( pFileInfo->szMarkup ) ;
                           fCatchData = TRUE; 
                        } else
//                      if ( _wcsicmp( pszValue, DOCSHORTNAME_VAL ) == 0 ) {
//                         pFileInfo->szShortName[0] = NULL;  
//                         pInfo = pFileInfo->szShortName ;
//                         iInfoSize = sizeof( pFileInfo->szShortName ) ;
//                         fCatchData = TRUE; 
//                      } else
                        if ( _wcsicmp( pszValue, SRCLANG_VAL ) == 0 ) {
                           pFileInfo->szSrcLang[0] = NULL;  
                           pInfo = pFileInfo->szSrcLang ;
                           iInfoSize = sizeof( pFileInfo->szSrcLang ) ;
                           fCatchData = TRUE; 
                        } else
                        if ( _wcsicmp( pszValue, TGTLANG_VAL ) == 0 ) {
                           pFileInfo->szTgtLang[0] = NULL;  
                           pInfo = pFileInfo->szTgtLang ;
                           iInfoSize = sizeof( pFileInfo->szTgtLang ) ;
                           fCatchData = TRUE; 
                        }
                        break;
                     case UNIT_ELEMENT:
                        if ( _wcsicmp( pszValue, SEGNUM_VAL ) == 0 ) {
                           if ( ! pUnitInfo->szSegmentNum[0] ) {
                              pInfo = pUnitInfo->szSegmentNum ;
                              iInfoSize = sizeof( pUnitInfo->szSegmentNum ) ;
                              fCatchData = TRUE; 
                           }
                        }
                        break;
                     default:
                       break;
                   } /*endswitch */
                }
             }
           }
         }
         break;
              
      case METADATA_ELEMENT:
         this->fKeepMeta = FALSE ;
         break;

      case METAGROUP_ELEMENT:
         this->fKeepMeta = FALSE ;
         for( int i = 0; i < iAttribs; i++ ) {
           PSZ_W pszName = (PSZ_W)attributes.getName( i );
           if ( _wcsicmp( pszName, CATEGORY_ATTR ) == 0 ) {
             PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
             if ( _wcsicmp( pszValue, OTM_VAL ) == 0 ) {  
                this->fKeepMeta = TRUE ;
             }
           }
         }
         break;

      case MATCHES_ELEMENT:
         pUnitInfo->bMatches = TRUE ;
         this->iLastElement = MATCHES_ELEMENT ;
         break;

      case INVCHAR_ELEMENT:
        // segment contains invalid data
        if ( this->iLastElement == SOURCE_ELEMENT ) {
           pSegmentInfo->bInvalidSourceChar = TRUE ;
        }
        if ( this->iLastElement == TARGET_ELEMENT ) {
           pSegmentInfo->bInvalidTargetChar = TRUE ;
        }
        break;

      case UNKNOWN_ELEMENT:
      default:
        break;
    } /*endswitch */

    if ( hfLog )
    {
      for( int i = 0; i < iAttribs; i++ )
      {
        PSZ_W pszName = (PSZ_W)attributes.getName( i );
        PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
        fprintf( hfLog, "Attribute %ld: Name=%S, Value=%S\n", i, pszName, pszValue );
      } /* endfor */
    } /* endif */
}


void XLFParseHandler::endElement(const XMLCh* const name )
{
  PSZ_W pszName = (PSZ_W)name;
  if ( hfLog ) fprintf( hfLog, "EndElement: %S\n", pszName );
  CurElementId = GetElementID( pszName );

  switch ( CurElementId )
  {

    case FILE_ELEMENT:
       memset( pFileInfo, 0, sizeof(FILE_INFO) );
       this->iLastElement = XLIFF_ELEMENT ;
       break;
  
    case UNIT_ELEMENT:
       memset( pUnitInfo, 0, sizeof(UNIT_INFO) );
       this->iLastElement = FILE_ELEMENT ;
       break;
  
    case SEGMENT_ELEMENT:
       /* Save this segment if all of the information is properly set */
       if ( hfLog ) {
          fprintf( hfLog, "New Seg...\n" );
          if ( pUnitInfo->szSegmentNum )fprintf( hfLog, "    Num=%S\n", pUnitInfo->szSegmentNum);
          if ( pSegmentInfo->szState )fprintf( hfLog, "    State=%S\n", pSegmentInfo->szState);
          fprintf( hfLog, "    Matches=%d\n", pUnitInfo->bMatches);
          if ( pFileInfo->szSrcLang )fprintf( hfLog, "    SrcLang=%S\n", pFileInfo->szSrcLang);
          if ( pFileInfo->szTgtLang )fprintf( hfLog, "    SrcLang=%S\n", pFileInfo->szTgtLang);
          if ( pFileInfo->szMarkup )fprintf( hfLog, "    Markup=%S\n", pFileInfo->szMarkup);
//        if ( pFileInfo->szShortName )fprintf( hfLog, "    Short=%S\n", pFileInfo->szShortName);
          if ( pFileInfo->szName )fprintf( hfLog, "    Name=%S\n", pFileInfo->szName);
          if ( pSegmentInfo->szSource )fprintf( hfLog, "    SRC=[[%S]]\n", pSegmentInfo->szSource);
          if ( pSegmentInfo->szTarget )fprintf( hfLog, "    TGT=[[%S]]\n", pSegmentInfo->szTarget);
       }
//     if ( ( pUnitInfo->szSegmentNum ) &&
//          ( pSegmentInfo->szState   ) &&
//          ( ! pUnitInfo->bMatches   ) &&
//          ( pFileInfo->szSrcLang    ) &&
//          ( pFileInfo->szTgtLang    ) && 
//          ( pFileInfo->szMarkup     ) &&
//          ( pFileInfo->szShortName  ) &&
//          ( pFileInfo->szName       ) &&
//          ( pSegmentInfo->szSource  ) &&
//          ( pSegmentInfo->szTarget  ) ) {

       /* Import only segments which have no other memory matches and have target text */
       if ( ( ( _wcsicmp( pSegmentInfo->szState, TRANSLATED_VAL) == 0 ) ||     /* State=translated */
              ( pSegmentInfo->szTarget[0]                             ) ) &&   /* Target text exists */
            ( ! pUnitInfo->bMatches   ) ) {                                    /* No matches */

          fillSegmentInfo( pFolderInfo, pFileInfo, pUnitInfo, pSegmentInfo, &(pBuf->SegmentData) );

          iLenSource = wcslen( pBuf->SegmentData.szSource );
          iLenTarget = wcslen( pBuf->SegmentData.szTarget );
          if ( (iLenTarget + 1) >= MAX_SEGMENT_SIZE) {
            pBuf->SegmentData.fValid = FALSE;
            sprintf( pBuf->SegmentData.szReason, "Segment too large. Length of target text is %ld characters.", iLenTarget );
          }
          else 
          if ( (iLenSource + 1) >= MAX_SEGMENT_SIZE ) {
            pBuf->SegmentData.fValid = FALSE;
            sprintf( pBuf->SegmentData.szReason, "Segment too large. Length of source text is %ld characters.", iLenSource );
          }
          else
          if ( iLenTarget == 0 ) {
            pBuf->SegmentData.fValid = FALSE;
            sprintf( pBuf->SegmentData.szReason, "Segment target text is missing.  Contains 0 characters." );
          }
          else 
          if ( iLenSource == 0 ) {
            pBuf->SegmentData.fValid = FALSE;
            sprintf( pBuf->SegmentData.szReason, "Segment source text is missing.  Contains 0 characters." );
          }
          else 
          if ( pSegmentInfo->bInvalidSourceChar ) {
            pBuf->SegmentData.fValid = FALSE;
            sprintf( pBuf->SegmentData.szReason, "Segment source contains invalid characters." );
          }
          else 
          if ( pSegmentInfo->bInvalidTargetChar ) {
            pBuf->SegmentData.fValid = FALSE;
            sprintf( pBuf->SegmentData.szReason, "Segment target contains invalid characters." );
          }
          else 
          if ( pBuf->SegmentData.szSourceLang[0] == 0 ) {
            pBuf->SegmentData.fValid = FALSE;
            sprintf( pBuf->SegmentData.szReason, "Segment source language is not specified." );
          }
          else 
          if ( pBuf->SegmentData.szTargetLang[0] == 0 ) {
            pBuf->SegmentData.fValid = FALSE;
            sprintf( pBuf->SegmentData.szReason, "Segment target language is not specified." );
          }
          else 
          if ( _wcsicmp( pSegmentInfo->szState, TRANSLATED_VAL) != 0 ) {
            pBuf->SegmentData.fValid = FALSE;
            sprintf( pBuf->SegmentData.szReason, "Segment has target text but the translation state is \"%S\"", pSegmentInfo->szState );
          }
          else
          {
            pBuf->SegmentData.fValid = TRUE;
          } /* endif */

          // add segment to memory (if fValid set) or count as skipped segment
          if ( pfnInsertSegment != NULL ) {
            pfnInsertSegment( lMemHandle, &(pBuf->SegmentData) );
          } /* endif */

       }

       memset( pSegmentInfo, 0, sizeof(SEGMENT_INFO) );
       this->iLastElement = UNIT_ELEMENT ;
       pInfo = NULL ;
       iInfoSize = 0 ;
       break;

    case SOURCE_ELEMENT:
    case TARGET_ELEMENT:
      if ( ( this->iLastElement == SOURCE_ELEMENT ) ||
           ( this->iLastElement == TARGET_ELEMENT ) ) {
         this->iLastElement = SEGMENT_ELEMENT ;
      }
      fCatchData = FALSE ;
      pInfo = NULL ;
      iInfoSize = 0 ;
      break;
  
    case METAGROUP_ELEMENT:
      this->fKeepMeta = FALSE ;
      break;

    case META_ELEMENT:
      fCatchData = FALSE ;
      pInfo = NULL ;
      iInfoSize = 0 ;
      break;

    case MATCHES_ELEMENT:
      this->iLastElement = UNIT_ELEMENT ;
      break;

    case INVCHAR_ELEMENT:
      break;

    case UNKNOWN_ELEMENT:
    default:
      break;
  } /*endswitch */

}

void XLFParseHandler::characters(const XMLCh* const chars, const XMLSize_t length)
{
  PSZ_W pszChars = (PSZ_W)chars;

  if ( hfLog ) fprintf( hfLog, "%ld Characters; \"%S\"\n", length, pszChars );

  if ( fCatchData && pInfo && iInfoSize && (CurElementId != UNKNOWN_ELEMENT) ) 
  {
    // add data to current data buffer 
    int iCurLen = wcslen( pInfo );
    if ( (INT)(iCurLen + length + 1) < iInfoSize/sizeof(CHAR_W) )
    {
      wcsncpy( pInfo + iCurLen, pszChars, length );
      pInfo[iCurLen+length] = 0;
    } /* endif */
  } /* endif */
}


void XLFParseHandler::fillSegmentInfo
(
  PFOLDER_INFO     pFolderInfo,
  PFILE_INFO       pFileInfo,
  PUNIT_INFO       pUnitInfo,
  PSEGMENT_INFO    pSegmentInfo,
  PMEMEXPIMPSEG    pSegment
)
{
  pFolderInfo;

  // fill-in header data
  memset( pSegment, 0, sizeof(MEMEXPIMPSEG) );

  GetValueW( pUnitInfo->szSegmentNum, wcslen(pUnitInfo->szSegmentNum), &(pSegment->lSegNum) ) ;
  wcstombs( pSegment->szFormat, pFileInfo->szMarkup, sizeof(pSegment->szFormat) ) ;
  if ( pFileInfo->szName[0] ) {
     wcstombs( pSegment->szDocument, pFileInfo->szName, sizeof(pFileInfo->szName) ) ;
  } else {
     strcpy( pSegment->szDocument, "none" );
  }
  if ( pFileInfo->szName[0] ) {
     wcstombs( pSegment->szDocument, pFileInfo->szName, sizeof(pFileInfo->szName) ) ;
  } else {
     strcpy( pSegment->szDocument, "none" );
  }
  pSegment->usTranslationFlag = TRANSLFLAG_MACHINE;
  time( &(pSegment->lTime) );

  wcstombs( pSegment->szSourceLang, pFileInfo->szSrcLang, sizeof(pSegment->szSourceLang) ) ;
  wcstombs( pSegment->szTargetLang, pFileInfo->szTgtLang, sizeof(pSegment->szTargetLang) ) ;
  wcsncpy( pSegment->szSource, pSegmentInfo->szSource, SEGDATABUFLEN );
  wcsncpy( pSegment->szTarget, pSegmentInfo->szTarget, SEGDATABUFLEN );

//// note style and text
//pSegment->szAddInfo[0] = 0;
//if ( (pBuf->szNote[0]!= 0) || (pBuf->szNoteStyle[0] != 0)  )
//{
//  swprintf( pSegment->szAddInfo, L"<Note style=\"%s\">%s</Note>", pBuf->szNoteStyle, pBuf->szNote );
//} /* endif */     
//
//// MT meta data
//if ( (pBuf->szMTMetrics[0] != 0) )
//{
//  swprintf( pSegment->szAddInfo + wcslen(pSegment->szAddInfo), L"<MT%s></MT>", pBuf->szMTMetrics );
//} /* endif */     
}

void XLFParseHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    if ( hfLog ) fprintf( hfLog, "Fatal Error: %s at column %ld in line %ld\n", message, col, line );
    this->fError = TRUE;
    sprintf( this->pBuf->szErrorMessage, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    XMLString::release( &message );
}

void XLFParseHandler::error(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    if ( hfLog ) fprintf( hfLog, "Error: %s at column %ld in line %ld\n", message, col, line  );
    this->fError = TRUE;
    sprintf( this->pBuf->szErrorMessage, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    XMLString::release( &message );
}

void XLFParseHandler::warning(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    if ( hfLog ) fprintf( hfLog, "Warning: %s at column %ld in line %ld\n", message, col, line  );
    XMLString::release( &message );
}


// get the ID for a XLIFF element
ELEMENTID XLFParseHandler::GetElementID( PSZ_W pszName )
{
  int i = 0;
  ELEMENTID IDFound = UNKNOWN_ELEMENT;

  while ( (IDFound == UNKNOWN_ELEMENT) && (XlfNameToID[i].szName[0] != 0) )
  {
    if ( _wcsicmp( pszName, XlfNameToID[i].szName ) == 0 )
    {
      IDFound = XlfNameToID[i].ID;
    }
    else
    {
      i++;
    } /* endif */
  } /*endwhile */
  return( IDFound );
} /* end of method XLFParseHandler::GetElementID */


void XLFParseHandler::SetMemInfo( PMEMEXPIMPINFO pMemInfo)
{
  this->m_pMemInfo = pMemInfo;
} /* end of method XLFParseHandler::SetMemInfo */

// extract a numeric value from a string
BOOL XLFParseHandler::GetValueW( PCHAR_W pszString, int iLen, LONG *plResult )
{
  BOOL fOK = TRUE;
  CHAR_W szNumber[10];
  CHAR_W *pszNumber = szNumber;

  *plResult = 0;

  while ( iLen && fOK )
  {
    if ( isdigit(*pszString) )
    {
      *pszNumber++ = *pszString++;
      iLen--;
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /*endwhile */

  if ( fOK )
  {
    *pszNumber = NULL;
    *plResult = _wtoi( szNumber );
  } /* endif */

  return( fOK );
} /* end of method XLFParseHandler::GetValue */


void XLFParseHandler::SetMemInterface( PFN_MEMINSERTSEGMENT pfnInsert, LONG lHandle, PTOKENENTRY pTokBuf, int iSize )
{
  pfnInsertSegment = pfnInsert;
  lMemHandle = lHandle;
  pTokBuf = pTokBuf;
  iTokBufSize = iSize;
} /* end of method XLFParseHandler::SetMemInterface */


BOOL XLFParseHandler::IsValidFormat()
{
 return( fValidFormat );
}


BOOL XLFParseHandler::IsHeaderDone()
{
 return( fHeaderDone );
}


void XLFParseHandler::SetSourceLanguage( char *pszSourceLang)
{
  if ( pBuf )
  {
    strcpy( pBuf->szMemSourceLang, pszSourceLang);
  } /* endif */
} /* end of method XLFParseHandler::SetSourceLanguage */

void XLFParseHandler::GetSourceLanguage( char *pszSourceLang, int iBufSize )
{
  *pszSourceLang = 0;

  if ( pFolderInfo->szSrcLang[0] )
  {
    wcstombs( pszSourceLang, pFolderInfo->szSrcLang, iBufSize ) ;
    if ( pBuf ) {
       strncpy( pBuf->szMemSourceLang, pszSourceLang, sizeof(pBuf->szMemSourceLang) );
       pBuf->szMemSourceLang[sizeof(pBuf->szMemSourceLang)-1] = 0;
    }
  } else
  if ( pBuf && pBuf->szMemSourceLang[0] )
  {
    strncpy( pszSourceLang, pBuf->szMemSourceLang, iBufSize );
    pszSourceLang[iBufSize-1] = 0;
  } /* endif */
} /* end of method XLFParseHandler::GetSourceLanguage */


BOOL XLFParseHandler::ErrorOccured( void )
{
  return( this->fError );
}

void XLFParseHandler::GetErrorText( char *pszTextBuffer, int iBufSize )
{
  *pszTextBuffer = '\0';

  if ( this->pBuf != NULL )
  {
    if ( this->pBuf->szErrorMessage[0] != '\0' )
    {
      strncpy( pszTextBuffer, this->pBuf->szErrorMessage, iBufSize );
      pszTextBuffer[iBufSize-1] = '\0';
    } /* endif */
  } /* endif */
}


//+----------------------------------------------------------------------------+
//| XLF2EXP converter                                                          |
//+----------------------------------------------------------------------------+
#define BUF_SIZE 8096

typedef struct _CONVERTERDATA
{
  MEMEXPIMPSEG     Segment;                      // segment data buffer
  MEMEXPIMPINFO    MemInfo;                      // mem import export data area
  FILE             *hfOut;                       // output file handle
  CHAR_W           szBufferW[16200];             // buffer for UTF16 strings
  CHAR             szASCIIBuffer[16200];         // buffer for UTF16 to ANSI/ASCII conversions
  BOOL             fHeaderWritten;               //TRUE = header has written to the memory
  LONG             lSegCounter;                  // valid segment counter
  LONG             lSkippedSegsCounter;          // invalid segment counter
  CHAR             szLogFile[MAX_LONGFILESPEC];  // buffer for log file name
  FILE             *hfLog;                       // log file handle

  //-- data for input memory processing --
  CHAR             szBuffer[4096];                    // general purpose buffer

} CONVERTERDATA, *PCONVERTERDATA;

USHORT WriteStringToMemory( PCONVERTERDATA pData, PSZ_W pszString );
USHORT WriteMemHeader( PCONVERTERDATA pData);
USHORT WriteMemFooter( PCONVERTERDATA pData );



// write segment to output file
USHORT APIENTRY WRITEEXPSEGMENT( LONG lMemHandle, PMEMEXPIMPSEG pSegment )
{
  USHORT           usRC = 0;           // function return code

  PCONVERTERDATA   pData = (PCONVERTERDATA)lMemHandle;

  if ( pSegment->fValid )
  {
    if ( !pData->fHeaderWritten )
    {
      WriteMemHeader( pData );
    } /* endif */

    pData->lSegCounter++;
    swprintf( pData->szBufferW, L"<Segment>%10.10ld\r\n", pData->lSegCounter );
    WriteStringToMemory( pData, pData->szBufferW );
    WriteStringToMemory( pData, L"<Control>\r\n" );
    swprintf( pData->szBufferW, L"%06ld%s%1.1u%s%016lu%s%S%s%S%s%S%s%S%s%s%s%S",
      pSegment->lSegNum, X15_STRW, pSegment->usTranslationFlag, X15_STRW,
      pSegment->lTime, X15_STRW, pSegment->szSourceLang, X15_STRW, 
      (pData->MemInfo.fSourceSource) ? pSegment->szSourceLang : pSegment->szTargetLang, X15_STRW,
      pSegment->szAuthor, X15_STRW, pSegment->szFormat, X15_STRW, L"na", X15_STRW, pSegment->szDocument );
    WriteStringToMemory( pData, pData->szBufferW );
    WriteStringToMemory( pData, L"\r\n</Control>\r\n" );
    if ( pSegment->szAddInfo[0] != 0 )
    {
      WriteStringToMemory( pData, L"<AddData>" );
      WriteStringToMemory( pData, pSegment->szAddInfo );
      WriteStringToMemory( pData, L"</AddData>\r\n" );
    } /* endif */       
    WriteStringToMemory( pData, L"<Source>" );
    WriteStringToMemory( pData, pSegment->szSource );
    WriteStringToMemory( pData, L"</Source>\r\n" );
    WriteStringToMemory( pData, L"<Target>" );
    if ( pData->MemInfo.fSourceSource )
    {
      WriteStringToMemory( pData, pSegment->szSource );
    }
    else
    {
      WriteStringToMemory( pData, pSegment->szTarget );
    } /* endif */
    WriteStringToMemory( pData, L"</Target>\r\n" );
    WriteStringToMemory( pData, L"</Segment>\r\n" );
  }
  else
  {
    if ( pData->hfLog )
    {
      fwprintf( pData->hfLog, L"\r\nSegment %ld skipped, reason = %S\r\n", pSegment->lSegNum, pSegment->szReason );
      fwprintf( pData->hfLog, L"Source=%s\r\n", pSegment->szSource );

    } /* endif */
    pData->lSkippedSegsCounter++;
  } /* endif */

  return( usRC );
}

// XLIFF to EXP converter
USHORT XLFTOEXP
( 
  PSZ              pszInMem,            // fully qualified name of input (XLIFF) memory
  PSZ              pszOutMem,           // fully qualified name of output (EXP) memory
  LONG             lOutMode,            // output mode ANSI/ASCII/UTF16
  LONG             lOptions,            // other processing options
  PLONG            plValidSegs,         // number of segments written to EXP file
  PLONG            plSkippedSegs        // number of skipped segments
)
{
  USHORT           usRC = 0;           // function return code
  LONG             lHandle = 0;        // import handle
  LONG             lProgress = 0;
  PCONVERTERDATA   pData = NULL;

  lOptions;

  if ( !UtlAlloc( (PVOID *)&pData, 0, sizeof(CONVERTERDATA), ERROR_STORAGE  ) )
  {
    usRC = ERROR_STORAGE;      
  } /* endif */

  if ( !usRC )
  {
    pData->MemInfo.lOutMode = lOutMode;

    pData->hfOut = fopen( pszOutMem, "wb" );
    if ( pData->hfOut == NULL )
    {
      usRC = ERROR_WRITE_FAULT;
    }
    else
    {
        fwrite( "\xFF\xFE", 2, 1, pData->hfOut );    /* Always UTF-16 */
    } /* endif */

    if ( !usRC )
    {
      Utlstrccpy( pData->szLogFile, pszInMem, '.' );
      strcat( pData->szLogFile, ".LOG" );
      pData->hfLog = fopen( pData->szLogFile, "wb" );
      if ( pData->hfLog )
      {
        fwrite( "\xFF\xFE", 2, 1, pData->hfLog );
        fwprintf( pData->hfLog, L"*** Skipped segments log ****\r\n\r\n" );
      } /* endif */
    } /* endif */
  } /* endif */

  if ( !usRC )
  {
    usRC = EXTMEMIMPORTSTART( &lHandle, pszInMem, &(pData->MemInfo) );
  } /* endif */

  while ( !usRC )
  {
    usRC = EXTMEMIMPORTPROCESS( lHandle, WRITEEXPSEGMENT, (LONG)pData, &lProgress );
  } /*endwhile */
  if ( usRC == MEM_IMPORT_COMPLETE )
  {
    usRC = 0;
  } /* endif */

  if ( lHandle ) EXTMEMIMPORTEND( lHandle );

  if ( pData )
  {
    if ( plValidSegs ) *plValidSegs = pData->lSegCounter;
    if ( plSkippedSegs ) *plSkippedSegs = pData->lSkippedSegsCounter;

    if ( pData->hfOut )
    {
      WriteMemFooter( pData );
      fclose( pData->hfOut );
    } /* endif */
    if ( pData->hfLog )
    {
      fclose( pData->hfLog );
    } /* endif */
    if ( pData->MemInfo.pszMarkupList != NULL  ) UtlAlloc( (PVOID *)&(pData->MemInfo.pszMarkupList), 0, 0, NOMSG );
    UtlAlloc( (PVOID *)&pData, 0, 0, NOMSG );
  } /* endif */

  return( usRC );
} /* end of function XLFTOEXP */


// write string to output memory
USHORT WriteStringToMemory
( 
  PCONVERTERDATA   pData,
  PSZ_W            pszString
)
{
  if ( pData->MemInfo.lOutMode & ASCII_OPT )
  {
    WideCharToMultiByte( CP_OEMCP, 0, pszString, -1, pData->szASCIIBuffer, sizeof(pData->szASCIIBuffer), NULL, NULL );
    fwrite( pData->szASCIIBuffer, strlen(pData->szASCIIBuffer), 1, pData->hfOut );
  }
  else if ( pData->MemInfo.lOutMode & ANSI_OPT )
  {
    WideCharToMultiByte( CP_ACP, 0, pszString, -1, pData->szASCIIBuffer, sizeof(pData->szASCIIBuffer), NULL, NULL );
    fwrite( pData->szASCIIBuffer, strlen(pData->szASCIIBuffer), 1, pData->hfOut );
  }
  else
  {
    fwrite( pszString, wcslen(pszString), sizeof(CHAR_W), pData->hfOut );
  } /* endif */
  return( 0 );
} /* end of function WriteStringToMemory */


USHORT WriteMemHeader( PCONVERTERDATA pData )
{
  if ( !pData->fHeaderWritten )
  {
    WriteStringToMemory( pData, L"<NTMMemoryDb>\r\n" );
    WriteStringToMemory( pData, L"<Description>\r\n" );
    MultiByteToWideChar( CP_ACP, 0, pData->MemInfo.szDescription, -1, pData->szBufferW, sizeof(pData->szBufferW)/sizeof(CHAR_W) );
    WriteStringToMemory( pData, pData->szBufferW );
    WriteStringToMemory( pData, L"\r\n</Description>\r\n" );
    WriteStringToMemory( pData, L"<Codepage>UTF16</Codepage>\r\n" );
    pData->fHeaderWritten = TRUE;
  } /* endif */
  return( 0 );
} /* end of function WriteMemHeader */


USHORT WriteMemFooter( PCONVERTERDATA pData )
{
  WriteStringToMemory( pData, L"</NTMMemoryDb>\r\n" );
  return( 0 );
} /* end of function WriteMemHeader */







// code borrowed from EQFMEMUT.C as the EQFDLL currently does not export this functions
// functions and defines for the access to additional data of proposals (MAD)

// prototypes for private functions
PSZ_W MADSkipName( PSZ_W pszName );
PSZ_W MADSkipWhitespace( PSZ_W pszData );
PSZ_W MADSkipAttrValue( PSZ_W pszValue );
BOOL  MADCompareKey( PSZ_W pszKey1, PSZ_W pzKey2 );
BOOL  MADCompareAttr( PSZ_W pszAttr1, PSZ_W pszAttr2 );
BOOL  MADNextAttr( PSZ_W *ppszAttr );
PSZ_W MADSkipAttrValue( PSZ_W pszValue );
BOOL  MADGetAttrValue( PSZ_W pszAttr, PSZ_W pszBuffer, int iBufSize );


//
// Search a specific key in the additional memory data
//
HADDDATAKEY MADSearchKey( PSZ_W pAddData, PSZ_W pszKey )
{
  HADDDATAKEY pKey = NULL;
  if ( (pAddData != NULL) && (*pAddData != 0) )
  {
    while ( (pKey == NULL) && (*pAddData != 0) )
    {
      // move to begin of next key
      while ( (*pAddData != 0) && (*pAddData != L'<') ) pAddData++;

      // check key
      if ( (*pAddData == L'<')  )
      {
        if ( MADCompareKey( pAddData + 1, pszKey ) )
        {
          pKey = pAddData;
        }
        else
        {
          pAddData += wcslen(pszKey);
        } /* endif */           
      } /* endif */         
    } /* endwhile */       
  } /* endif */     

  return( pKey );
}

//
// retrieve the data associated with a specific key
//
BOOL MDAGetValueForKey( HADDDATAKEY pKey, PSZ_W pszBuffer, int iBufSize, PSZ_W pszDefault )
{
  PSZ_W pszAttr = pKey;
  BOOL fValue = FALSE;
  BOOL fAttrAvailable = FALSE; 

  *pszBuffer = 0;

  // skip any attributes
  if ( *pszAttr == L'<' ) pszAttr++;
  pszAttr = MADSkipName( pszAttr );
  do
  {
    fAttrAvailable = MADNextAttr( &pszAttr );
  } while ( fAttrAvailable ); /* enddo */   

  if ( *pszAttr == L'>' )
  {
    PSZ_W pszValue = pszAttr + 1;
    while ( (iBufSize > 1) && (*pszValue != 0) && (*pszValue != L'<') )
    {
      *pszBuffer++ = *pszValue++;
      iBufSize--;
    }
    *pszBuffer = 0;
    fValue = TRUE;
  } /* endif */     

  if ( !fValue && (pszDefault != NULL) ) wcscpy( pszBuffer, pszDefault );

  return( FALSE );
}

//
// get the value for a specific attribute 
//
BOOL MADGetAttr( HADDDATAKEY pKey, PSZ_W pszAttrName, PSZ_W pszBuffer, int iBufSize, PSZ_W pszDefault )
{
  PSZ_W pszAttr = pKey;
  BOOL fAttrAvailable = FALSE;

  if ( *pszAttr == L'<' ) pszAttr++;
  pszAttr = MADSkipName( pszAttr );

  do
  {
    fAttrAvailable = MADNextAttr( &pszAttr );
    if ( fAttrAvailable )
    {
      if ( MADCompareAttr( pszAttr, pszAttrName ) )
      {
        MADGetAttrValue( pszAttr, pszBuffer, iBufSize );
        return( TRUE );
      } /* endif */         
    } /* endif */       

  } while ( fAttrAvailable ); /* enddo */   
  if ( pszDefault != NULL ) wcscpy( pszBuffer, pszDefault );
  return( FALSE);
}

//
// get the next attribute/value pair
//
BOOL MADGetNextAttr( HADDDATAKEY *ppKey, PSZ_W pszAttrNameBuffer, PSZ_W pszValueBuffer, int iBufSize  )
{
  PSZ_W pszAttr = *ppKey;
  BOOL fAttrAvailable = FALSE;

  *pszAttrNameBuffer = 0;
  *pszValueBuffer =0;
  if ( *pszAttr == L'<' ) 
  {
    pszAttr++;
    pszAttr = MADSkipName( pszAttr );
  } /* endif */     

  fAttrAvailable = MADNextAttr( &pszAttr );
  if ( fAttrAvailable )
  {
    CHAR_W chTemp;
    PSZ_W pszNameEnd = pszAttr;
    while ( (*pszNameEnd != 0) && (*pszNameEnd != L' ') && (*pszNameEnd != L'>') && (*pszNameEnd != L'=') )  pszNameEnd++;
    chTemp = *pszNameEnd;
    *pszNameEnd = 0;
    wcscpy( pszAttrNameBuffer, pszAttr );
    *pszNameEnd = chTemp;
    MADGetAttrValue( pszAttr, pszValueBuffer, iBufSize );
  } /* endif */   
  *ppKey = pszAttr;
  return( fAttrAvailable );
}

// ======== internal functions for Memory Additional Data processing

// position to next attribute
BOOL MADNextAttr( PSZ_W *ppszAttr )
{
  PSZ_W pszAttr = *ppszAttr;

  // if we are at an attribute already we skip this one
  if ( iswalpha( *pszAttr) || (*pszAttr == L'-') || (*pszAttr == L'_') || (*pszAttr == L':') )
  {
    pszAttr = MADSkipName( pszAttr );
    pszAttr = MADSkipAttrValue( pszAttr );
  } /* endif */     

  pszAttr = MADSkipWhitespace( pszAttr );
  while ( *pszAttr == L' ' ) pszAttr++;

  // set caller's pointer
  *ppszAttr = pszAttr;

  return( iswalpha( *pszAttr ) );
}


// retrieve attribute value
BOOL MADGetAttrValue( PSZ_W pszAttr, PSZ_W pszBuffer, int iBufSize )
{
  PSZ_W pszValue = MADSkipName( pszAttr );
  if ( *pszValue == L'=' )
  {
    pszValue++;
    if ( *pszValue == L'\"' )
    {
      pszValue++;
      while ( (iBufSize > 1) && (*pszValue != 0) && (*pszValue != L'\"') )
      {
        *pszBuffer++ = *pszValue++;
        iBufSize--;
      }
      if ( *pszValue == L'\"' ) pszValue++;
    }
    else
    {
      while ( (iBufSize > 1) && iswalpha(*pszValue) || iswdigit(*pszValue) )
      {
        *pszBuffer++ = *pszValue++;
        iBufSize--;
      }
    } /* endif */       

  } /* endif */     
  *pszBuffer = 0;

  return( TRUE );
}

// compare attribute names
BOOL MADCompareAttr( PSZ_W pszAttr1, PSZ_W pszAttr2 )
{
  BOOL fMatch = FALSE;

  while ( (*pszAttr1 != 0) && (*pszAttr2 != 0) && (*pszAttr1 == *pszAttr2) )
  {
    pszAttr1++; pszAttr2++;
  } /* endwhile */     

  fMatch = ( ((*pszAttr1 == 0) || (*pszAttr1 == L' ') || (*pszAttr1 == L'>') || (*pszAttr1 == L'=')) && 
             ((*pszAttr2 == 0) || (*pszAttr2 == L' ') || (*pszAttr2 == L'>') || (*pszAttr2 == L'=')) );

  return( fMatch );
}

// compare two key names
BOOL MADCompareKey( PSZ_W pszKey1, PSZ_W pszKey2 )
{
  BOOL fMatch = FALSE;

  while ( (*pszKey1 != 0) && (*pszKey2 != 0) && (*pszKey1 == *pszKey2) )
  {
    pszKey1++; pszKey2++;
  } /* endwhile */     

  fMatch = ( ((*pszKey1 == 0) || (*pszKey1 == L' ') || (*pszKey1 == L'>')) && ((*pszKey2 == 0) || (*pszKey2 == L' ') || (*pszKey2 == L'>')) );

  return( fMatch );
}

// skip attribute value: loop until end of current attribute value
PSZ_W MADSkipAttrValue( PSZ_W pszValue )
{
  // pszValuze points to first character following the attribute name, eithere the name is followed
  // by an equal sign and the value or the attribute has no value at all
  if ( *pszValue == L'=' )
  {
    pszValue++;
    if ( *pszValue == L'\"' )
    {
      pszValue++;
      while ( (*pszValue != 0) && (*pszValue != L'\"') ) pszValue++;
      if ( *pszValue == L'\"' ) pszValue++;
    }
    else
    {
      while ( iswalpha(*pszValue) || iswdigit(*pszValue) ) pszValue++;
    } /* endif */       

  } /* endif */     
  return( pszValue );
}

// skip name: loop until end of current name
PSZ_W MADSkipName( PSZ_W pszName )
{
  while ( iswalpha(*pszName) || iswdigit(*pszName) || (*pszName == L'-') || (*pszName == L'_') || (*pszName == L':') ) pszName++;
  return( pszName );
}

// skip any whitespace: increase given pointer until non-blank character reached
PSZ_W MADSkipWhitespace( PSZ_W pszData )
{
  while ( *pszData == L' ' ) pszData++;
  return( pszData );
}
