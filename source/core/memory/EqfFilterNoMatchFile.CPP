//+----------------------------------------------------------------------------+
//| EQFFilterNoMatchFile.CPP                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_EDITORAPI        // for EQFWORDCOUNTPERSEGW
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_MORPH            // morphologic functions
#include <eqf.h>                  // General Translation Manager include file

#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "core\memory\MemoryFactory.h"
#include "core\utilities\LanguageFactory.h"
#include "core\pluginmanager\OtmMorph.h"
#include "eqftag00.h"
#include "eqftmtag.h"
#include "eqfserno.h"

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


#include <time.h>                 // C library for time functions
#include "EQFDDE.H"               // Batch mode definitions
#define INCL_EQFMEM_DLGIDAS       // include dialog IDA definitions
#include "EQFTMI.H"               // Private header file of Translation Memory
#include "EQFMEM.ID"              // PM IDs for Translation Memory
#include <EQFQDAM.H>              // Low level TM access functions
  #include "OTMFUNC.H"            // function call interface public defines
  #include "eqffunci.h"           // function call interface private defines
#include "EQFHLOG.H"              // for word count category limits
#include <OTMGLOBMem.h> 
#include "cxmlwriter.H"


XERCES_CPP_NAMESPACE_USE

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  Code Implementing the processing for the EqfFilterNoMatchFile API call
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// data types 
// process phases
typedef enum _FILTERNOMATCH_TASKS
{
  FILTERNOMATCH_START_TASK,
  FILTERNOMATCH_PARSENEXT_TASK,
  FILTERNOMATCH_STOP_TASK,
  FILTERNOMATCH_COMPLETED_TASK
} FILTERNOMATCH_TASKS;

// number of best matches to be kept in match table
#define FILTERNOMATCH_MAX_MATCHES  3 

// size of input buffers
#define BUF_SIZE 8096

// best match structure, exists once per segment of the document and contains the reference for the 
// three best matches of this segment
typedef struct _FILTERNOMATCH_MATCH
{
  USHORT      usMatchLevel;                      // level of this match
  ULONG       ulPos;                             // position of this match in the output memory
  ULONG       ulLen;                             // length of this match in the output memory
} FILTERNOMATCH_MATCH, *PFILTERNOMATCH_MATCH;


// segment structure, exists once per segment of the document and contains the reference for the 
// three best matches of this segment
typedef struct _FILTERNOMATCH_SEGMENT
{
  FILTERNOMATCH_MATCH    aBestMatch[FILTERNOMATCH_MAX_MATCHES];     // best matches for this segment
  FILTERNOMATCH_MATCH    BestMatchGITR;                   // best exact match for this segment when applying generic inline tag replacement
} FILTERNOMATCH_SEGMENT, *PFILTERNOMATCH_SEGMENT;

// document structure, exists once per document
typedef struct _FILTERNOMATCH_DOCUMENT
{
  CHAR        szDocName[MAX_LONGFILESPEC];       // document name
  ULONG       ulSegments;                        // number of segments
  PFILTERNOMATCH_SEGMENT pSegment;                         // pointer to segment table
} FILTERNOMATCH_DOCUMENT, *PFILTERNOMATCH_DOCUMENT;


// IDs of nomatch file XML elelements
typedef enum { RQT_ELEMENT, DOCS_ELEMENT, D_ELEMENT, P_ELEMENT, S_ELEMENT, T_ELEMENT, UNKNOWN_ELEMENT } ELEMENTID;

typedef struct _XMLNAMETOID
{
  CHAR_W   szName[30];                 // name of element
  ELEMENTID ID;                        // ID of element 
} XMLNAMETOID, *PXMLNAMETOID;

XMLNAMETOID XmlNameToId[] =
{ { L"rqt",    RQT_ELEMENT },
  { L"docs",   DOCS_ELEMENT }, 
  { L"d",      D_ELEMENT }, 
  { L"p",      P_ELEMENT }, 
  { L"s",      S_ELEMENT }, 
  { L"t",      T_ELEMENT },
  { L"",       UNKNOWN_ELEMENT } };

//
// class for our nomatch parse SAX handler
//

#define MAX_CAPTURE_LEN 8096


class NomatchParseHandler : public HandlerBase
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  NomatchParseHandler( PVOID pvData );
  virtual ~NomatchParseHandler();

  // -----------------------------------------------------------------------
  //  Handlers for the SAX DocumentHandler interface
  // -----------------------------------------------------------------------
  void startElement(const XMLCh* const name, AttributeList& attributes);
  void endElement(const XMLCh* const name );
  void characters(const XMLCh* const chars, const XMLSize_t  length);

  // -----------------------------------------------------------------------
  //  Handlers for the SAX ErrorHandler interface
  // -----------------------------------------------------------------------
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc );
  void fatalError(const SAXParseException& exc);

  //  Error related methods
  BOOL ErrorOccured();
  void GetErrorText( char *pszTextBuffer, int iBufSize );

  // cleanup code
  void cleanup();

private:
  ELEMENTID GetElementID( PSZ_W pszName );
  void UTF16ToAscii( PSZ_W pszUnicode, PSZ pszAsciiBuffer, int iBufLen );
  void IsoToOpenTM2Language( PSZ_W pszISO, PSZ pszLanguage );
  void LookupInMemory();
  BOOL WriteSegmentXML();
  BOOL WriteSegmentEXP();
  BOOL WriteInUTF8( FILE *hfOut, PSZ_W pszUTF16 );
  void EscapeChars( PSZ_W pszText, PSZ_W pszBuffer );
  unsigned short CountWords( PSZ_W pszText, PULONG pulWords );
  void NomatchParseHandler::writeWordCount();

  ELEMENTID   m_eCurrentId;                      // ID of currently active element

  PVOID       m_pvData;                          // pointer to FILTERNOMATCHDATA area

  // variables used for error handling
  BOOL        m_fError;                          // TRUE = an error occured during parsing or processing
  char        m_szErrorMessage[1024];            // buffer for error message text
};



typedef struct _FILTERNOMATCH_DATA
{
  // passed parameters
  char          szInNoMatchXML[MAX_LONGFILESPEC];  // name of input nomatch file (XML format)
  char          szGlobMemOptionFile[MAX_LONGFILESPEC]; // name of global memory option file
  char          szMemory[MAX_LONGFILESPEC];        // name of memory for filtering
  char          szOutNoMatchXML[MAX_LONGFILESPEC]; // name of filtered nomatch file (XML format)  
  char          szOutNoMatchEXP[MAX_LONGFILESPEC]; // name of filtered nomatch file (EXP format)  
  char          szWordCountReport[MAX_LONGFILESPEC]; // name of word count report
  LONG          lOptions;

  // variables used during the processing
  PVOID         pvGlobMemFile;                     // pointer to global memory option file
  FILTERNOMATCH_TASKS   Task;                      // current task
  OtmMemory     *pInputMem;                        // input memory
  FILE          *hfNoMatchOutputXML;               // nomatch output file (XML)
  FILE          *hfNoMatchOutputExp;               // nomatch output file (EXP)
  FILE          *hfWordCount;                      // wordcount output file (XML)
  NomatchParseHandler *handler;                    // our SAX handler for the parsing of nomatch files 
  SAXParser*    parser;                            // SAX parser 
  XMLPScanToken SaxToken; 
  BOOL          fErrorStop;                        // TRUE = parsing ended by an error
  int           iSegmentsProcessed;                // number of segments processed in the current processing step
  std::vector<OtmProposal *> *pvMatchList;         // pointer to a vector receiving found memory proposals
  char          szErrorText[1024];                 // buffer for error messages
  CHAR_W        szUTF16Buffer[2500];               // buffer for UTF16 strings

  // word count sums
  ULONG         ulExactWords;                      // number of words in exact matches
  ULONG         ulFuzzyWords;                      // number of words in fuzzy matches
  ULONG         ulNomatchWords;                    // number of words having no match at all

  // text capturing
  BOOL        fCaptureText;                        // TRUE = capture text to our text buffer
  CHAR_W      szTextBuffer[MAX_CAPTURE_LEN];       // text capture buffer

  // attributes of current rqt element
  CHAR_W      szCurrentTargetLanguageIso[40];   // ISO name of target language
  char        szCurrentTargetLanguage[40];      // OpenTM2 name of target language
  CHAR_W      szCurrentAction[40];              // current action
  CHAR_W      szCurrentUID[40];                 // current user ID

  // attributes of current d element
  CHAR_W      szCurrentSourceLanguageIso[40];  // ISO name for source language
  char        szCurrentSourceLanguage[40];     // OpenTM2 name for source language
  CHAR_W      szCurrentDocType[40];            // document type
  CHAR_W      szCurrentDocument[MAX_LONGFILESPEC];  // current document name
  CHAR_W      szCurrentMarkup[40];             // current markup table name

  // attributes of current s element
  ULONG       ulCurrentSegNum;                 // segment number

  // attributes of current t element
  CHAR_W      szCurrentType[40];               // type of t element

  // variables used during parsing and processing of segments
  char        szBuffer[2100];                  // general purpose buffer
  CHAR_W      szLastDocument[MAX_LONGFILESPEC];// last processed document name
  ULONG       ulExpSegments;                   // number of segments written to EXP nomatch file
  ULONG       ulXmlSegments;                   // number of segments written to XML nomatch file
  CHAR_W      szLastMarkup[40];                // last loaded markup table
  char        szUTF8Buffer[2100];              // buffer for UTF8 strings
  PLOADEDTABLE  pLoadedTable;                  // ptr to loaded tag table  
  int         iCurrentXMLSegment;              // number of current segment in XML output file 
  int         iCurrentEXPSegment;              // number of current segment in XML output file 
  PTOKENENTRY  pTokenList;                     // token list buffer for inline tag recognition 
  LONG         lCurrentTokenListSize;          // current size of token list buffer
  ULONG        ulOemCP;                        // ASCII CP for current source language
  SHORT        sLangID;                        // language ID for current source language
  char         szLastSourceLanguage[40];       // last used source language
} FILTERNOMATCH_DATA, *PFILTERNOMATCH_DATA;



// prototypes 
USHORT FilterNoMatchPrepare( PFCTDATA pData, char *pszInNoMatchXML, char *pszGlobMemOptionFile, char *pszMemory, char *pszOutNoMatchXML, char *pszOutNoMatchEXP, char *pszWordCountReport, long lOptions );
USHORT FilterNoMatchProcess( PFCTDATA pData );
USHORT FilterNoMatchParseNext( PFILTERNOMATCH_DATA pData );
USHORT FilterNoMatchCleanup( PFILTERNOMATCH_DATA pData );
USHORT FilterNoMatchStart( PFILTERNOMATCH_DATA pData );

// function removed irrelevant segments from an external memory
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
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new process or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new process
    pData->usProgress = 0;
    usRC = FilterNoMatchPrepare( pData, pszInNoMatchXML, pszGlobMemOptionFile, pszMemory, pszOutNoMatchXML, pszOutNoMatchEXP, pszWordCountReport, lOptions );
    if ( usRC == 0 )
    {
      usRC = CONTINUE_RC;
    }
  }
  else
  {
    // continue current filter process
    usRC = FilterNoMatchProcess( pData );
  } /* endif */

  return( usRC );
} /* end of function FilterNoMatchFile */

// prepare a new process
USHORT FilterNoMatchPrepare
(
  PFCTDATA pFCTData,
  char *pszInNoMatchXML,           
  char *pszGlobMemOptionFile,      
  char *pszMemory,                 
  char *pszOutNoMatchXML,          
  char *pszOutNoMatchEXP,          
  char *pszWordCountReport,        
  LONG lOptions 
)
{
  BOOL        fOK = TRUE;              // internal O.K. flag
  USHORT      usRC = NO_ERROR;         // function return code
  PSZ         pszParm;                 // error parameter pointer

  PFILTERNOMATCH_DATA pData = NULL;   // Pointer to the load IDA

  fOK = UtlAllocHwnd( (PVOID *)&pData, 0L, (LONG)sizeof(FILTERNOMATCH_DATA), ERROR_STORAGE, HWND_FUNCIF );
  if ( fOK )
  {
    pData->sLangID = -1;
  }

  // check input nomatch file
  if ( fOK )
  {
    if ( (pszInNoMatchXML == NULL) || (*pszInNoMatchXML == EOS) )
    {
      fOK = FALSE;
      usRC = FUNC_MANDINFILE;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else if ( !UtlFileExist( pszInNoMatchXML ) )
    {
      fOK = FALSE;
      pszParm = pszInNoMatchXML;
      usRC = ERROR_FILENAME_NOT_VALID ;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check and load global memory option file
  if ( fOK )
  {
    if ( (pszGlobMemOptionFile == NULL) || (*pszGlobMemOptionFile == EOS) )
    {
      fOK = FALSE;
      usRC = EEA_MANDCMDLINE;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else if ( !GlobMemLoadOptionFile( pszGlobMemOptionFile, &(pData->pvGlobMemFile) ) ) 
    {
      fOK = FALSE;
      pszParm = pszGlobMemOptionFile;
      usRC = ERROR_PROFILE_LOAD ;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check memory
  if ( fOK )
  {
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    if ( (pszMemory == NULL) || (*pszMemory == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDTM;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    }
    else if ( !pFactory->exists( NULL, pszMemory ) )
    {
      fOK = FALSE;
      pszParm = pszMemory;
      usRC = ERROR_MEMORY_NOTFOUND ;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check options
  if ( fOK )
  {
    // use text output as default, when no output option has been used
    if ( ((lOptions & TEXT_OUTPUT_OPT) == 0) && ((lOptions & XML_OUTPUT_OPT) == 0) )
    {
      lOptions |= TEXT_OUTPUT_OPT;
    }
  } /* endif */

   // store input values and set options for processing
   if ( fOK )
   {
     strcpy( pData->szInNoMatchXML, pszInNoMatchXML );
     strcpy( pData->szGlobMemOptionFile, pszGlobMemOptionFile );
     strcpy( pData->szMemory, pszMemory );
     strcpy( pData->szOutNoMatchXML, pszOutNoMatchXML );
     strcpy( pData->szOutNoMatchEXP, pszOutNoMatchEXP );
     strcpy( pData->szWordCountReport, pszWordCountReport );
     pData->lOptions = lOptions;
   } /* endif */

   if ( !fOK )
   {
     if ( pData )
     {
       UtlAlloc( (PVOID *) &pData, 0L, 0L, NOMSG );
     } /* endif */
   } /* endif */

   // prepare the process
   if ( fOK )
   {
     pFCTData->pvFilterNoMatchFile = (PVOID)pData;
     pFCTData->fComplete = FALSE;
     pData->Task = FILTERNOMATCH_START_TASK;
   }
   else
   {
     usRC = UtlQueryUShort( QS_LASTERRORMSGID );
   } /* endif */

   return( usRC );

} /* end of function FilterNoMatchPrepare */

// function performaing the actual work of the CleanMemory function
USHORT FilterNoMatchProcess
(
  PFCTDATA    pFCTData                    // function I/F session data
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PFILTERNOMATCH_DATA pData = NULL;   // Pointer to the load IDA

  pData = (PFILTERNOMATCH_DATA)pFCTData->pvFilterNoMatchFile;
  UtlSetUShort( QS_LASTERRORMSGID, 0 );

  switch ( pData->Task )
  {
    case FILTERNOMATCH_START_TASK:
      usRC = FilterNoMatchStart( pData ); 
      break;
    case FILTERNOMATCH_PARSENEXT_TASK:    
      usRC = FilterNoMatchParseNext( pData ); 
      break;
    case FILTERNOMATCH_STOP_TASK:        
      usRC = FilterNoMatchCleanup( pData ); 
      break;
    case FILTERNOMATCH_COMPLETED_TASK:   
      break;
 } /* endswitch */

 if ( pData->Task == FILTERNOMATCH_COMPLETED_TASK )
 {
   pFCTData->fComplete = TRUE;
   UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG ); 
 } /* endif */

 return( usRC );

} /* end of function FilterNoMatchImportProcess */


// FilterNomatchFile start up code
USHORT FilterNoMatchStart
(
  PFILTERNOMATCH_DATA pData
)
{
  USHORT usRC = 0;

  // prepare proposal list
  if ( usRC == 0 )
  {
    pData->pvMatchList = new std::vector<OtmProposal *>;
    for ( int i = 0; i < 10; i++ )
    {
      pData->pvMatchList->push_back( new OtmProposal );
    } /* endfor */     
  } /* endif */

  // open input memory
  if ( usRC == 0 )
  {
    int iRC = 0;
    MemoryFactory *pFactory = MemoryFactory::getInstance();
    pData->pInputMem = pFactory->openMemory( NULL, pData->szMemory, 0, &iRC );
    if ( pData->pInputMem == NULL )
    {
      pFactory->showLastError( NULL, pData->szMemory, NULL, HWND_FUNCIF );
      usRC = (USHORT)iRC;
    }
    else
    {
      pData->pInputMem->setGlobalMemoryOptions( pData->pvGlobMemFile );
    }
  } /* endif */


  // open output files
  if ( (usRC == 0) && (pData->szOutNoMatchEXP[0] != EOS) )
  {
    pData->hfNoMatchOutputExp = fopen( pData->szOutNoMatchEXP, "wb" );
    if ( pData->hfNoMatchOutputExp == NULL )
    {
      PSZ pszParm[2];
      int iRC = GetLastError();
      char szRC[10];
      ltoa( iRC, szRC, 10 );
      pszParm[0] = pData->szOutNoMatchEXP;
      pszParm[1] = szRC;
      UtlErrorHwnd( ERROR_FILE_WRITE_ERROR, MB_CANCEL, 2, pszParm, EQF_ERROR, HWND_FUNCIF);
      usRC = ERROR_FILE_WRITE_ERROR;
    } /* endif */
  } /* endif */

  if ( (usRC == 0) && (pData->szOutNoMatchXML[0] != EOS) )
  {
    pData->hfNoMatchOutputXML = fopen( pData->szOutNoMatchXML, "wb" );
    if ( pData->hfNoMatchOutputXML == NULL )
    {
      PSZ pszParm[2];
      int iRC = GetLastError();
      char szRC[10];
      ltoa( iRC, szRC, 10 );
      pszParm[0] = pData->szOutNoMatchXML;
      pszParm[1] = szRC;
      UtlErrorHwnd( ERROR_FILE_WRITE_ERROR, MB_CANCEL, 2, pszParm, EQF_ERROR, HWND_FUNCIF);
      usRC = ERROR_FILE_WRITE_ERROR;
    } /* endif */
  } /* endif */

  if ( (usRC == 0) && (pData->szWordCountReport[0] != EOS) )
  {
    pData->hfWordCount = fopen( pData->szWordCountReport, "wb" );
    if ( pData->hfWordCount == NULL )
    {
      PSZ pszParm[2];
      int iRC = GetLastError();
      char szRC[10];
      ltoa( iRC, szRC, 10 );
      pszParm[0] = pData->szWordCountReport;
      pszParm[1] = szRC;
      UtlErrorHwnd( ERROR_FILE_WRITE_ERROR, MB_CANCEL, 2, pszParm, EQF_ERROR, HWND_FUNCIF);
      usRC = ERROR_FILE_WRITE_ERROR;
    } /* endif */
  } /* endif */

  // prepare parsing of input nomatch file
  if ( usRC == 0 )
  {
    try {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch) {
        toCatch;

        return( ERROR_NOT_READY );
    }

    pData->parser = new SAXParser();      // Create a SAX parser object

    // create an instance of our handler
    pData->handler = new NomatchParseHandler( (PVOID)pData );

    //  install our SAX handler as the document and error handler.
    pData->parser->setDocumentHandler( pData->handler );
    pData->parser->setErrorHandler( pData->handler );
    pData->parser->setCalculateSrcOfs( TRUE );
    pData->parser->setValidationSchemaFullChecking( FALSE );
    pData->parser->setDoSchema( FALSE );
    pData->parser->setLoadExternalDTD( FALSE );
    pData->parser->setValidationScheme( SAXParser::Val_Never );
    pData->parser->setExitOnFirstFatalError( FALSE );
      
    try
    {
      if ( !pData->parser->parseFirst( pData->szInNoMatchXML, pData->SaxToken) )
      {
          usRC = ERROR_READ_FAULT;
      } /* endif */
    }
    catch (const OutOfMemoryException& )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    catch (const XMLException& toCatch)
    {
      toCatch;
      usRC = ERROR_READ_FAULT;
    } /* endif */
  } /* endif */

  // prepare next processing step
  if ( !usRC )
  {
    pData->Task = FILTERNOMATCH_PARSENEXT_TASK;
  }
  else
  {
    pData->Task = FILTERNOMATCH_STOP_TASK;
    pData->fErrorStop = TRUE;
  } /* endif */

  return( usRC );
} /* end of function FilterNoMatchImportProcess */

// parse the next part of the input file
USHORT FilterNoMatchParseNext
(
  PFILTERNOMATCH_DATA pData
)
{
  USHORT usRC = 0;

  // parse next 10 entries in Position to next document, check if document has analyzed
  BOOL fContinue  = TRUE;

  pData->iSegmentsProcessed = 0;
  while ( fContinue && !pData->parser->getErrorCount() && (pData->iSegmentsProcessed < 10) )
  {
    fContinue = pData->parser->parseNext( pData->SaxToken);
  } /*endwhile */

  if ( pData->parser->getErrorCount() )
  {
    pData->fErrorStop = TRUE;
    //PSZ pszParms[3];

    //pszParms[0] = pData->szExportName ;
    //pszParms[1] = pData->szError;
    //pszParms[2] = "" ;
    //iRC = ERROR_XML_LOADERROR;
    //UtlErrorHwnd( (SHORT)iRC, MB_CANCEL, 3, pszParms, EQF_ERROR, pData->hwndErrMsg );
  } /* endif */

  if ( fContinue && !pData->fErrorStop )
  {
    pData->Task = FILTERNOMATCH_PARSENEXT_TASK;
  }
  else
  {
    // complete writing of output files
    pData->handler->cleanup();

    pData->Task = FILTERNOMATCH_STOP_TASK;
  } /* endif */

  return( usRC );
} /* end of function FilterNoMatchParseNext */

// do the cleanup
USHORT FilterNoMatchCleanup
(
  PFILTERNOMATCH_DATA pData
)
{
  USHORT usRC = 0;

  if ( pData != NULL )
  {
    if ( pData->handler != NULL ) delete( pData->handler ); ;
    if ( pData->sLangID != -1 ) MorphFreeLanguageID( pData->sLangID );
    if ( pData->pLoadedTable != NULL ) TAFreeTagTable( pData->pLoadedTable );
    if ( pData->pTokenList != NULL ) UtlAlloc( (PVOID *)&(pData->pTokenList), 0, 0, NOMSG );
    if ( pData->hfWordCount == NULL )
    {
      fclose( pData->hfWordCount );
    }
    if ( pData->hfNoMatchOutputExp != NULL )
    {
      fclose( pData->hfNoMatchOutputExp );
    }
    if ( pData->hfNoMatchOutputXML != NULL )
    {
      fclose( pData->hfNoMatchOutputXML );
    }
    if ( pData->pInputMem != NULL )
    {
      MemoryFactory *pFactory = MemoryFactory::getInstance();
      pFactory->closeMemory( pData->pInputMem );
    }

    pData->Task = FILTERNOMATCH_COMPLETED_TASK;
  }
  return( usRC );
} /* end of function FilterNoMatchCleanup */



//
// Implementation of Nomatch SAX parser
//
NomatchParseHandler::NomatchParseHandler( PVOID pvData )
{
  PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)pvData;

  m_pvData = pvData;
  pData->fCaptureText = FALSE;
  pData->szLastDocument[0] = EOS;
  pData->ulExpSegments = 0;
  pData->ulXmlSegments = 0;
}

NomatchParseHandler::~NomatchParseHandler()
{
}

void NomatchParseHandler::startElement(const XMLCh* const name, AttributeList& attributes)
{
    PSZ_W pszName = (PSZ_W)name;
    int iAttribs = attributes.getLength(); 
    PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)m_pvData;

    m_eCurrentId = GetElementID( pszName );

    switch ( m_eCurrentId )
    {
      case RQT_ELEMENT:
        // get target language
        pData->szCurrentTargetLanguageIso[0] = '\0';
        pData->szCurrentAction[0] = '\0';
        pData->szCurrentUID[0] = '\0';
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( _wcsicmp( pszName, L"tl" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            wcscpy( pData->szCurrentTargetLanguageIso, pszValue );
            IsoToOpenTM2Language( pData->szCurrentTargetLanguageIso, pData->szCurrentTargetLanguage );
          } 
          else if ( _wcsicmp( pszName, L"action" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            wcscpy( pData->szCurrentAction, pszValue );
          } 
          else if ( _wcsicmp( pszName, L"uid" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            wcscpy( pData->szCurrentUID, pszValue );
          } /* endif */
        } /* endfor */
        break;

      case D_ELEMENT:
        // get language, markup, and document name 
        pData->szCurrentDocument[0] = '\0';
        pData->szCurrentSourceLanguageIso[0] = '\0';
        pData->szCurrentMarkup[0] = '\0';
        pData->szCurrentDocType[0] = '\0';
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( _wcsicmp( pszName, L"url" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            wcscpy( pData->szCurrentDocument, pszValue );
          } 
          else if ( _wcsicmp( pszName, L"lang" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            wcscpy( pData->szCurrentSourceLanguageIso, pszValue );
            IsoToOpenTM2Language( pData->szCurrentSourceLanguageIso, pData->szCurrentSourceLanguage );
          } 
          else if ( _wcsicmp( pszName, L"markup" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            wcscpy( pData->szCurrentMarkup, pszValue );
          } 
          else if ( _wcsicmp( pszName, L"dt" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            wcscpy( pData->szCurrentDocType, pszValue );
          } /* endif */
        } /* endfor */
        break;

      case S_ELEMENT:
        // get segment number
        pData->ulCurrentSegNum = 0;
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( _wcsicmp( pszName, L"seg" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            pData->ulCurrentSegNum = _wtol( pszValue );
          } /* endif */
        } /* endfor */
        break;

      case T_ELEMENT:
        // start character recording
        pData->szTextBuffer[0] = EOS;
        pData->fCaptureText = TRUE;
        pData->szCurrentType[0] = EOS;
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( _wcsicmp( pszName, L"tt" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            wcscpy( pData->szCurrentType, pszValue );
          } /* endif */
        } /* endfor */
        break;

      case UNKNOWN_ELEMENT:
      default:
        break;
    } /*endswitch */
}


void NomatchParseHandler::endElement(const XMLCh* const name )
{
  PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)m_pvData;

  switch ( m_eCurrentId )
  {
    case T_ELEMENT:
      // T element is complete, so we cann process the collected data 
      pData->fCaptureText = FALSE;
      LookupInMemory();
      pData->iSegmentsProcessed++;
      m_eCurrentId = S_ELEMENT; // we don't care which element is active as long as it is no T_ELEMENT
      break;

    case UNKNOWN_ELEMENT:
    default:
      // nothing to do for these elements...
      break;
  } /*endswitch */
}
void NomatchParseHandler::characters(const XMLCh* const chars, const  XMLSize_t length )
{
  PSZ_W pszChars = (PSZ_W)chars;
  PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)m_pvData;

  if ( pData->fCaptureText )
  {
    // add characters to buffer
    int iCurLen = wcslen( pData->szTextBuffer );
    if ( (iCurLen + length + 1) < sizeof(pData->szTextBuffer) )
    {
      wcsncpy( pData->szTextBuffer + iCurLen, pszChars, length );
      pData->szTextBuffer[iCurLen+length] = 0;
    } /* endif */
  } /* endif */
}

void NomatchParseHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    m_fError = TRUE;
    sprintf( m_szErrorMessage, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    XMLString::release( &message );
}

void NomatchParseHandler::error(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    m_fError = TRUE;
    sprintf( m_szErrorMessage, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    XMLString::release( &message );
}

void NomatchParseHandler::warning(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    XMLString::release( &message );
}


// get the ID for a TMX element
ELEMENTID NomatchParseHandler::GetElementID( PSZ_W pszName )
{
  int i = 0;
  ELEMENTID IDFound = UNKNOWN_ELEMENT;

  while ( (IDFound == UNKNOWN_ELEMENT) && (XmlNameToId[i].szName[0] != 0) )
  {
    if ( _wcsicmp( pszName, XmlNameToId[i].szName ) == 0 )
    {
      IDFound = XmlNameToId[i].ID;
    }
    else
    {
      i++;
    } /* endif */
  } /*endwhile */
  return( IDFound );
} /* end of method NomatchParseHandler::GetElementID */


BOOL NomatchParseHandler::ErrorOccured( void )
{
  return( m_fError );
}

void NomatchParseHandler::GetErrorText( char *pszTextBuffer, int iBufSize )
{
  *pszTextBuffer = '\0';

  if ( m_szErrorMessage[0] != '\0' )
  {
    strncpy( pszTextBuffer, m_szErrorMessage, iBufSize );
    pszTextBuffer[iBufSize-1] = '\0';
  } /* endif */
}

void NomatchParseHandler::UTF16ToAscii( PSZ_W pszUnicode, PSZ pszAsciiBuffer, int iBufLen )
{
  PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)m_pvData;
  WideCharToMultiByte( CP_OEMCP, 0, pszUnicode, -1, pszAsciiBuffer, iBufLen, NULL, NULL );
}

void NomatchParseHandler::IsoToOpenTM2Language( PSZ_W pszISO, PSZ pszLanguage )
{
  PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)m_pvData;
  LanguageFactory *pLangFactory = LanguageFactory::getInstance();
  UTF16ToAscii( pszISO, pData->szBuffer, sizeof(pData->szBuffer) );
  pLangFactory->getOpenTM2NameFromISO( pData->szBuffer, pszLanguage );
}


void NomatchParseHandler::LookupInMemory()
{
  PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)m_pvData;

  // lookup segment in document memory
  OtmProposal searchKey;


  OtmProposal::clearAllProposals( *(pData->pvMatchList) );

  searchKey.setSegmentNum( pData->ulCurrentSegNum );
  searchKey.setDocShortName( "na" );
  UTF16ToAscii( pData->szCurrentDocument, pData->szBuffer, sizeof(pData->szBuffer) );
  searchKey.setDocName( pData->szBuffer );
  UTF16ToAscii( pData->szCurrentMarkup, pData->szBuffer, sizeof(pData->szBuffer) );
  searchKey.setMarkup( pData->szBuffer );
  searchKey.setSourceLanguage( pData->szCurrentSourceLanguage );
  searchKey.setTargetLanguage( pData->szCurrentTargetLanguage );
  searchKey.setSource( pData->szTextBuffer );

  pData->pInputMem->searchProposal( searchKey, *(pData->pvMatchList), GET_EXACT | GET_NO_GENERICREPLACE | GET_ALWAYS_WITH_TAGS );


  int iNumMatches = OtmProposal::getNumOfProposals( *(pData->pvMatchList) );
  int iFuzzyness =  (*pData->pvMatchList)[0]->getFuzziness();

  ULONG ulWords = 0;
  CountWords( pData->szTextBuffer, &ulWords );


  if ( (iNumMatches != 0) && ((*pData->pvMatchList)[0]->getType() == OtmProposal::eptManual) )
  {
    if ( iFuzzyness >= 100 )
    {
      // count as exact match
      pData->ulExactWords += ulWords;
    }
    else
    {
      // count as fuzzy match
      pData->ulFuzzyWords += ulWords;

      // add to output nomatch files
      WriteSegmentEXP();
      WriteSegmentXML();
    }
  }
  else
  {
    // count as nomatch
    pData->ulNomatchWords += ulWords;

    // add to output nomatch files
    WriteSegmentEXP();
    WriteSegmentXML();
  }
}

BOOL NomatchParseHandler::WriteSegmentEXP()
{
  BOOL fOK = TRUE;

  PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)m_pvData;

  if ( pData->hfNoMatchOutputExp != NULL )
  {
    if ( pData->iCurrentEXPSegment == 0 )
    {
      fwrite( UNICODEFILEPREFIX, strlen(UNICODEFILEPREFIX), 1, pData->hfNoMatchOutputExp );
      fputws( NTM_BEGIN_TAGW, pData->hfNoMatchOutputExp );
      fputws( NTM_CODEPAGE_BEGIN_TAGW, pData->hfNoMatchOutputExp );
      fputws( NTM_CODEPAGE_UTF16_VALUE, pData->hfNoMatchOutputExp );
      fputws( NTM_CODEPAGE_END_TAGW, pData->hfNoMatchOutputExp );
    }

    pData->iCurrentEXPSegment++;

    fputws( MEM_SEGMENT_BEGIN_TAGW, pData->hfNoMatchOutputExp );
    fwprintf( pData->hfNoMatchOutputExp, L"%ld\r\n", pData->iCurrentEXPSegment );
    fputws( MEM_CONTROL_BEGIN_TAGW, pData->hfNoMatchOutputExp );
    fwprintf( pData->hfNoMatchOutputExp, L"%06hu%s%c%s%016lu%s%S%s%S%s%S%s%s%s%s%s%s", pData->ulCurrentSegNum, X15_STRW, L'0', X15_STRW,
      0L, X15_STRW, pData->szCurrentSourceLanguage, X15_STRW, pData->szCurrentTargetLanguage, X15_STRW, "", X15_STRW,
      pData->szCurrentMarkup, X15_STRW, L"None", X15_STRW, pData->szCurrentDocument );
    fputws( MEM_CONTROL_END_TAGW, pData->hfNoMatchOutputExp );
    fputws( MEM_SOURCE_BEGIN_TAGW, pData->hfNoMatchOutputExp );
    fputws( pData->szTextBuffer, pData->hfNoMatchOutputExp );
    fputws( MEM_SOURCE_END_TAGW, pData->hfNoMatchOutputExp );
    fputws( MEM_TARGET_BEGIN_TAGW, pData->hfNoMatchOutputExp );
    fputws( MEM_TARGET_END_TAGW, pData->hfNoMatchOutputExp );
    fputws( MEM_SEGMENT_END_TAGW, pData->hfNoMatchOutputExp );

  }
  return( fOK );

} /* end of function TAWriteMTSegmentExpToFile */

// write the nomatch segment in XML format
BOOL NomatchParseHandler::WriteSegmentXML()
{
  BOOL fOK = TRUE;

  PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)m_pvData;

  if ( pData->hfNoMatchOutputXML != NULL )
  {
    // write document header if this is the first segment
    if ( pData->iCurrentXMLSegment == 0 )
    {
      fwrite( UTF8FILEPREFIX, strlen(UTF8FILEPREFIX), 1, pData->hfNoMatchOutputXML );
      if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, L"<?xml version=\'1.0\'?>\r\n" );
      swprintf( pData->szUTF16Buffer, L"<rqt action=\"trqt\" tl=\"%s\" uid=\"xyz@us.ibm.com\">\r\n", pData->szCurrentTargetLanguageIso );
      if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, pData->szUTF16Buffer );
      if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, L"   <docs>\r\n" );
    }

    // write document start if this is a new document
    if ( wcscmp( pData->szCurrentDocument, pData->szLastDocument ) != 0 )
    {
      // end current doc (if any) 
      if ( pData->szLastDocument[0] != 0)
      {
        if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, L"      </p>\r\n" );
        if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, L"    </d>\r\n" );
      } /* endif */

      wcscpy( pData->szLastDocument, pData->szCurrentDocument );

      // write start of new document
      if ( fOK )
      {
        // write d element up to document name
        swprintf ( pData->szUTF16Buffer, L"    <d lang=\"%s\" dt=\"ndoc\" url=\"", pData->szCurrentSourceLanguageIso );
        if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, pData->szUTF16Buffer );
      
        // escape and write document name 
        EscapeChars( pData->szCurrentDocument, pData->szUTF16Buffer );
        if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, pData->szUTF16Buffer );

       // write rest of d element data
        swprintf ( pData->szUTF16Buffer, L"\" markup=\"%s\">\r\n", pData->szCurrentMarkup );
        if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, pData->szUTF16Buffer );
        swprintf ( pData->szUTF16Buffer, L"      <p pid=\"%ld\">\r\n", 1L );
        if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, pData->szUTF16Buffer );
      } /* endif */
    } /* endif */

    pData->iCurrentXMLSegment++;

    // write segment data
    if ( fOK )
    {
      swprintf ( pData->szUTF16Buffer, L"        <s sid=\"%ld\" seg=\"%lu\"><t tid=\"1\" tt=\"orig\">", pData->iCurrentXMLSegment, pData->ulCurrentSegNum );
      if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, pData->szUTF16Buffer );

      EscapeChars( pData->szTextBuffer, pData->szUTF16Buffer );
      if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, pData->szUTF16Buffer );

      if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, L"</t></s>\r\n" );
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of function TAWriteMTSegmentToXmlFile */

// escape characters in segment data to form valid XML 
void NomatchParseHandler::EscapeChars( PSZ_W pszText, PSZ_W pszBuffer )
{
  while ( *pszText )
  {
    if ( *pszText == L'\n' )
    {
      wcscpy( pszBuffer, L" " );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'\r' )
    {
    }
    else if ( *pszText == L'&' )
    {
      wcscpy( pszBuffer, L"&amp;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'<' )
    {
      wcscpy( pszBuffer, L"&lt;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'>' )
    {
      wcscpy( pszBuffer, L"&gt;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'\"' )
    {
      wcscpy( pszBuffer, L"&quot;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( (*pszText == L'\x1F') || (*pszText == L'\t') )
    {
      // suppress some special characters
      *pszBuffer++ = L' ';
    }
    else
    {
      *pszBuffer++ = *pszText;
    } /* endif */
    pszText++;
  } /*endwhile */
  *pszBuffer = 0;
}


BOOL NomatchParseHandler::WriteInUTF8( FILE *hfOut, PSZ_W pszUTF16 )
{
  BOOL fOK = TRUE;
  PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)m_pvData;

  // convert to UTF-8
  int iOutLen =  WideCharToMultiByte( CP_UTF8, 0, (LPCWSTR)pszUTF16, -1, (LPSTR)pData->szUTF8Buffer, sizeof(pData->szUTF8Buffer), NULL, NULL );

  // write UTF-8 string to output file
  fwrite( pData->szUTF8Buffer, iOutLen - 1, 1, hfOut );

  return( fOK );
}

unsigned short NomatchParseHandler::CountWords( PSZ_W pszText, PULONG pulWords )
{
  unsigned short usRC = 0;
  PSTARTSTOP   pStartStop = NULL;
  unsigned long ulWords = 0;
  unsigned long ulTags = 0;
  PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)m_pvData;

  if ( pulWords != NULL ) *pulWords = 0;

  // get language ID and ASCII CP for language if not done yet
  if ( strcmp( pData->szCurrentSourceLanguage, pData->szLastSourceLanguage ) != 0 )
  {
    usRC = MorphGetLanguageID( pData->szCurrentSourceLanguage, &(pData->sLangID) );
    pData->ulOemCP = GetLangOEMCP( pData->szCurrentSourceLanguage );
    strcpy( pData->szLastSourceLanguage, pData->szCurrentSourceLanguage );
  }

  // perform inline tag recognition
  if ( usRC == 0 )
  {
    if ( wcscmp( pData->szCurrentMarkup, pData->szLastMarkup ) != 0 )
    {
      // free any previously loaded markup table
      if ( pData->pLoadedTable != NULL )
      {
        TAFreeTagTable( pData->pLoadedTable );
        pData->pLoadedTable = NULL;
      } /* endif */

      // load tag table for the markup
      UTF16ToAscii( pData->szCurrentMarkup, pData->szBuffer, sizeof(pData->szBuffer) );

      usRC = TALoadTagTableExHwnd( pData->szBuffer, &(pData->pLoadedTable), FALSE, TALOADUSEREXIT | TALOADPROTTABLEFUNC, TRUE, HWND_FUNCIF );

      wcscpy( pData->szLastMarkup, pData->szCurrentMarkup );
    }

    if ( pData->pLoadedTable != NULL )
    {
      // detect tags in input data
      if ( usRC == 0 )
      {
        int iIterations = 0;
        int          iAddEntries = 0;
        int          iIncrement = max( (wcslen(pszText) / 4), 20 );

        // allocate initial token buffer if not done yet
        if ( pData->lCurrentTokenListSize == 0 )
        {
          LONG lNewSize = (iAddEntries + iIncrement) * sizeof(TOKENENTRY);

          if (UtlAlloc((PVOID *) &(pData->pTokenList), pData->lCurrentTokenListSize, lNewSize, NOMSG) )
          {
            pData->lCurrentTokenListSize = lNewSize;
          }
        }

        usRC = TACreateProtectTableW( pszText, pData->pLoadedTable, 1, (PTOKENENTRY)(pData->pTokenList), (USHORT)pData->lCurrentTokenListSize, &pStartStop,  
                                      pData->pLoadedTable->pfnProtTable, pData->pLoadedTable->pfnProtTableW, pData->ulOemCP );

        while ((iIterations < 10) && (usRC == EQFRS_AREA_TOO_SMALL))
        {
          // (re)allocate token buffer
          LONG lNewSize = pData->lCurrentTokenListSize + ((iAddEntries + iIncrement) * sizeof(TOKENENTRY));

          if (UtlAlloc((PVOID *) &(pData->pTokenList), pData->lCurrentTokenListSize, lNewSize, NOMSG) )
          {
            iAddEntries += iIncrement;
            iIterations++;
            pData->lCurrentTokenListSize = lNewSize;
          }
          else
          {
            iIterations = 10;    // force end of loop
          } /* endif */

          // retry tokenization
          if (iIterations < 10 )
          {
            usRC = TACreateProtectTableW( pszText, pData->pLoadedTable, 1, (PTOKENENTRY)(pData->pTokenList), (USHORT)pData->lCurrentTokenListSize, &pStartStop,  
                                          pData->pLoadedTable->pfnProtTable, pData->pLoadedTable->pfnProtTableW, pData->ulOemCP );
          } /* endif */
        } /* endwhile */
      } /* endif */
    }
    else
    {
      // generate a start-stop entry for the text string
      UtlAlloc( (PVOID *)&pStartStop, 0, sizeof(STARTSTOP)*2, NOMSG );
      if ( pStartStop != NULL )
      {
        // entry for the text string
        pStartStop[0].usType = UNPROTECTED_CHAR;
        pStartStop[0].usStart = 0;
        pStartStop[0].usStop = (USHORT)(wcslen(pszText) - 1);

        // end-of-list entry 
        pStartStop[1].usType = 0;
        pStartStop[1].usStart = 0;
        pStartStop[1].usStop = 0;
      }
      else
      {
        usRC = ERROR_NOT_ENOUGH_MEMORY;
      } 
    }
  }

  // recognize words in the translatable parts of the start-stop list
  if ( usRC == 0 )
  {
    PSTARTSTOP pstCurrent = (PSTARTSTOP) pStartStop;

    while ( (pstCurrent->usType != 0) && (usRC == 0) )
    {
      if ( pstCurrent->usType == UNPROTECTED_CHAR )
      {
        PFLAGOFFSLIST pTermList = NULL;      // pt to created term list
        PFLAGOFFSLIST pActTerm;              // actual term
        CHAR_W chTemp = pszText[ pstCurrent->usStop+1 ];
        PSZ_W pszStart = pszText + pstCurrent->usStart;
        USHORT usListSize = 0;

        pszText[ pstCurrent->usStop+1 ] = EOS;
        usRC = MorphTokenizeW( pData->sLangID, pszStart, &usListSize, (PVOID *)&pTermList, MORPH_FLAG_OFFSLIST, pData->ulOemCP );
        pszText[ pstCurrent->usStop+1 ] = chTemp;

        if ( pTermList )
        {
          pActTerm = pTermList;
          while ( pActTerm->usLen )
          {
            if ( !(pActTerm->lFlags & OtmMorph::TERMTYPE_NEWSENTENCE ) && !(pActTerm->lFlags & OtmMorph::TERMTYPE_NOCOUNT) )
            {
              ulWords++;
            } /* endif */
            pActTerm++;
          } /* endwhile */
        } /* endif */
        UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
      }
      else
      {
        // count tags and attributes as one word
        ulTags++;
      } /* endif */
      pstCurrent++;
    } /*  endwhile */

    if ( pulWords != NULL ) *pulWords = ulWords;
  }

  // cleanup
  if ( pStartStop != NULL )   UtlAlloc( (PVOID *)&pStartStop, 0L, 0L, NOMSG );

  return( usRC );
}

// cleanup/termination code
void NomatchParseHandler::cleanup()
{
  PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)m_pvData;

  // write any termination text to our output files
  if ( pData->hfNoMatchOutputXML != NULL )
  {
    BOOL fOK = TRUE;

    // end current doc (if any) 
    if ( pData->szLastDocument[0] != 0)
    {
      if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, L"      </p>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, L"    </d>\r\n" );
    } /* endif */

    if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, L"  </docs>\r\n" );
    if ( fOK ) fOK = WriteInUTF8( pData->hfNoMatchOutputXML, L"</rqt>\r\n" );
  }
  if ( pData->hfNoMatchOutputExp != NULL )
  {
    fputws( NTM_END_TAGW, pData->hfNoMatchOutputExp );
  }

  // write word count report
  writeWordCount();
}

// cleanup/termination code
void NomatchParseHandler::writeWordCount()
{
  PFILTERNOMATCH_DATA pData = (PFILTERNOMATCH_DATA)m_pvData;

  if ( pData->hfWordCount != NULL ) 
  {
    if ( pData->lOptions & TEXT_OUTPUT_OPT )
    {
      // write text only wordcount 
      fputs( "Word Count Results - Translation\r\n", pData->hfWordCount );
      fputs( "Folder: n/a\r\n", pData->hfWordCount );
      fputs( "==========================================================================================\r\n", pData->hfWordCount );
      fputs( "Documents                                  Translated    Untranslated Target words  Markup\r\n", pData->hfWordCount );
      fputs( "------------------------------------------------------------------------------------------\r\n", pData->hfWordCount );
      fprintf( pData->hfWordCount, "%s\r\n", pData->szInNoMatchXML );
      fprintf( pData->hfWordCount, "                                           %10lu      %10lu   %10lu %7lu\r\n", pData->ulExactWords, pData->ulFuzzyWords + pData->ulNomatchWords, 0, 0 );
      fputs( "==========================================================================================\r\n", pData->hfWordCount );
      fprintf( pData->hfWordCount, "Total                                      %10lu      %10lu   %10lu %7lu\r\n", pData->ulExactWords, pData->ulFuzzyWords + pData->ulNomatchWords, 0, 0 );
    }
    else
    {
      // write output in XML
      BOOL fOK = TRUE;
      LONG lTime = 0;

      fwrite( UTF8FILEPREFIX, strlen(UTF8FILEPREFIX), 1, pData->hfWordCount );

      fOK = WriteInUTF8( pData->hfWordCount, L"<?xml version=\'1.0\' encoding=\"utf-8\"?>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"<?xml-stylesheet type=\"text/xsl\" href=\"CNT2HTML.XSL\" ?>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"<tmwordcount>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"  <description>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"    <report type=\"WordCount\" counted=\"Target\" generatedDate=\"" );

      UtlTime( &lTime );
      UtlLongToDateString( lTime, pData->szBuffer, 30 );
      UtlLongToTimeString( lTime, pData->szBuffer + 30, 30 );

      swprintf( pData->szTextBuffer, L"    <report type=\"WordCount\" counted=\"Target\" generatedDate=\"%S\" generatedTime=\"%S\" documentsSelected=\"1\" />\r\n", pData->szBuffer, pData->szBuffer + 30 );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, pData->szTextBuffer );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"    <folder nameLong=\"na\" nameShort=\"na\" documentsAmount=\"1\" />\r\n" );
      swprintf( pData->szTextBuffer, L"    <translationManager version=\"%d\" release=\"%d\" build=\"%d\" />\r\n", EQF_DRIVER_VERSION, EQF_DRIVER_RELEASE, EQF_DRIVER_SUBRELEASE );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, pData->szTextBuffer );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"  </description>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"  <documents>\r\n" );
      swprintf( pData->szTextBuffer, L"    <document nameLong=\"%S\" nameShort=\"na\">\r\n", pData->szInNoMatchXML );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, pData->szTextBuffer );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"      <wordcount>\r\n" );
      swprintf( pData->szTextBuffer, L"        <words type=\"Translated\" count=\"%lu\" />\r\n", pData->ulExactWords );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, pData->szTextBuffer );
      swprintf( pData->szTextBuffer, L"        <words type=\"Untranslated\" count=\"%lu\" />\r\n", pData->ulFuzzyWords + pData->ulNomatchWords );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, pData->szTextBuffer );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"        <words type=\"TargetWords\" count=\"0\" />\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"        <words type=\"Markup\" count=\"0\" />\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"      </wordcount>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"    </document>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"  </documents>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"  <summary>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"    <wordcount>\r\n" );
      swprintf( pData->szTextBuffer, L"        <words type=\"Translated\" count=\"%lu\" />\r\n", pData->ulExactWords );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, pData->szTextBuffer );
      swprintf( pData->szTextBuffer, L"        <words type=\"Untranslated\" count=\"%lu\" />\r\n", pData->ulFuzzyWords + pData->ulNomatchWords );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, pData->szTextBuffer );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"        <words type=\"TargetWords\" count=\"0\" />\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"        <words type=\"Markup\" count=\"0\" />\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"    </wordcount>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"  </summary>\r\n" );
      if ( fOK ) fOK = WriteInUTF8( pData->hfWordCount, L"</tmwordcount>\r\n" );
    }
  }
}