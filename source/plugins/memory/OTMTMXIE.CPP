//+----------------------------------------------------------------------------+
//| OTMTMXIE.CPP                                                               |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2017, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Memory Export/Import in TMX format                            |
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
  #include "EQFSERNO.H"
  #include "OTMFUNC.H"
  #include "TMXNames.H"

  #pragma pack( pop, TM2StructPacking )
#ifndef CPPTEST
}
#endif

#include "cxmlwriter.H"

// size of token buffer
#define TMXTOKBUFSIZE 32000

// size of file read buffer in preprocess step
#define TMX_BUFFER_SIZE 8096

// max length of a segment text

// name to use for undefined languages
#define OTHERLANGUAGES "OtherLanguages"

// name of TM specific prop elements
#define DESCRIPTION_PROP_OLD      "tmgr-description"
#define DESCRIPTION_PROP_W_OLD    L"tmgr-description"
#define DESCRIPTION_PROP          "tmgr:description"
#define DESCRIPTION_PROP_W        L"tmgr:description"

#define MARKUP_PROP_OLD      "tmgr-markup"
#define MARKUP_PROP_W_OLD    L"tmgr-markup"
#define MARKUP_PROP          "tmgr:markup"
#define MARKUP_PROP_W        L"tmgr:markup"

#define LANGUAGE_PROP_OLD    "tmgr-language"
#define LANGUAGE_PROP_W_OLD  L"tmgr-language"
#define LANGUAGE_PROP        "tmgr:language"
#define LANGUAGE_PROP_W      L"tmgr:language"

#define DOCNAME_PROP_OLD    "tmgr-docname"
#define DOCNAME_PROP_W_OLD  L"tmgr-docname"
#define DOCNAME_PROP        "tmgr:docname"
#define DOCNAME_PROP_W      L"tmgr:docname"

#define MACHFLAG_PROP_OLD    "tmgr-MTflag"
#define MACHFLAG_PROP_W_OLD  L"tmgr-MTflag"
#define MACHFLAG_PROP        "tmgr:MTflag"
#define MACHFLAG_PROP_W      L"tmgr:MTflag"

#define TRANSLFLAG_PROP_OLD    "tmgr-translFlag"
#define TRANSLFLAG_PROP_W_OLD  L"tmgr-translFlag"
#define TRANSLFLAG_PROP        "tmgr:translFlag"
#define TRANSLFLAG_PROP_W      L"tmgr:translFlag"

#define SEGNUM_PROP_OLD      "tmgr-segNum"
#define SEGNUM_PROP_W_OLD    L"tmgr-segNum"
#define SEGNUM_PROP          "tmgr:segNum"
#define SEGNUM_PROP_W        L"tmgr:segNum"

#define NOTE_PROP_OLD      "tmgr-note"
#define NOTE_PROP_W_OLD    L"tmgr-note"
#define NOTE_PROP          "tmgr:note"
#define NOTE_PROP_W        L"tmgr:note"

#define NOTESTYLE_PROP_OLD   "tmgr-notestyle"
#define NOTESTYLE_PROP_W_OLD L"tmgr-notestyle"
#define NOTESTYLE_PROP       "tmgr:notestyle"
#define NOTESTYLE_PROP_W     L"tmgr:notestyle"


#define TMMATCHTYPE_PROP            "TM:MatchType"
#define TMMATCHTYPE_PROP_W         L"TM:MatchType"
#define TMMATCHTYPE_PROP_W_OLD     L"TM-MatchType"

#define MTSERVICE_PROP              "MT:ServiceID"
#define MTSERVICE_PROP_W           L"MT:ServiceID"
#define MTSERVICE_PROP_W_OLD       L"MT-ServiceID"

#define MTMETRICNAME_PROP           "MT:MetricName"
#define MTMETRICNAME_PROP_W        L"MT:MetricName"
#define MTMETRICNAME_PROP_W_OLD    L"MT-MetricName"

#define MTMETRICVALUE_PROP          "MT:MetricValue"
#define MTMETRICVALUE_PROP_W       L"MT:MetricValue"
#define MTMETRICVALUE_PROP_W_OLD   L"MT-MetricValue"

#define PEEDITDISTANCECHARS_PROP    "PE:EditDistanceChars"
#define PEEDITDISTANCECHARS_PROP_W L"PE:EditDistanceChars"
#define PEEDITDISTANCECHARS_PROP_W_OLD L"PE-EditDistanceChars"

#define PEEDITDISTANCEWORDS_PROP    "PE:EditDistanceWords"
#define PEEDITDISTANCEWORDS_PROP_W L"PE:EditDistanceWords"
#define PEEDITDISTANCEWORDS_PROP_W_OLD L"PE-EditDistanceWords"

#define MTFIELDS_PROP               "MT:Fields"
#define MTFIELDS_PROP_W            L"MT:Fields"
#define MTFIELDS_PROP_W_OLD        L"MT-Fields"

#define WORDS_PROP                 "tmgr:words"
#define WORDS_PROP_W               L"tmgr:words"

#define MATCHSEGID_PROP           "tmgr:MatchSegID"
#define MATCHSEGID_PROP_W        L"tmgr:MatchSegID"


// IDs of TMX elelements
typedef enum { TMX_ELEMENT, PROP_ELEMENT, HEADER_ELEMENT, TU_ELEMENT, TUV_ELEMENT, BODY_ELEMENT,
               SEG_ELEMENT, BPT_ELEMENT, EPT_ELEMENT, PH_ELEMENT, HI_ELEMENT, IT_ELEMENT,
               SUB_ELEMENT, UT_ELEMENT, INVCHAR_ELEMENT, UNKNOWN_ELEMENT } ELEMENTID;
typedef struct _TMXNAMETOID
{
  CHAR_W   szName[30];                 // name of element
  ELEMENTID ID;                        // ID of element 
} TMXNAMETOID, *PTMXNAMETOID;

TMXNAMETOID TmxNameToID[] =
{ { L"tmx",    TMX_ELEMENT },
  { L"prop",   PROP_ELEMENT }, 
  { L"header", HEADER_ELEMENT }, 
  { L"tu",     TU_ELEMENT }, 
  { L"tuv",    TUV_ELEMENT }, 
  { L"body",   BODY_ELEMENT },
  { L"seg",    SEG_ELEMENT }, 
  { L"bpt",    BPT_ELEMENT }, 
  { L"ept",    EPT_ELEMENT }, 
  { L"ph",     PH_ELEMENT }, 
  { L"hi",     HI_ELEMENT }, 
  { L"it",     IT_ELEMENT },
  { L"sub",    SUB_ELEMENT }, 
  { L"ut",     UT_ELEMENT }, 
  { L"invchar", INVCHAR_ELEMENT }, 
  { L"",       UNKNOWN_ELEMENT } };

// PROP types
typedef enum { TMLANGUAGE_PROP, TMMARKUP_PROP, TMDOCNAME_PROP, MACHINEFLAG_PROP, SEG_PROP, TMDESCRIPTION_PROP, TMNOTE_PROP, TMNOTESTYLE_PROP,
  TRANSLATIONFLAG_PROP, TMTMMATCHTYPE_PROP, TMMTSERVICE_PROP, TMMTMETRICNAME_PROP, TMMTMETRICVALUE_PROP, TMPEEDITDISTANCECHARS_PROP, TMPEEDITDISTANCEWORDS_PROP,  
  TMMTFIELDS_PROP, TMWORDS_PROP, TMMATCHSEGID_PROP, UNKNOWN_PROP } TMXPROPID;

// stack elements
typedef struct _TMXELEMENT
{
  ELEMENTID ID;                        // ID of element 
  BOOL      fInlineTagging;            // TRUE = we are processing inline tagging
  BOOL      fInsideTagging;            // TRUE = we are currently inside inline tagging
  TMXPROPID PropID;                    // ID of prop element (only used for props)
  CHAR      szDataType[50];            // data type of current element
  CHAR      szTMXLanguage[50];         // TMX language of element
  CHAR      szTMLanguage[50];          // TM language of element
  CHAR      szTMMarkup[50];            // TM markup of element
  LONG      lSegNum;                   // TM segment number
} TMXELEMENT, *PTMXELEMENT;

// helper function for the access to the name mapping tables
BOOL FindName( PNAMETABLE pTable, const char *pszName, char *pszValue, int iBufSize );

void EscapeXMLChars( PSZ_W pszText, PSZ_W pszBuffer );

//
// class for our SAX handler
//
class TMXParseHandler : public HandlerBase
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  TMXParseHandler();
  virtual ~TMXParseHandler();

  // setter functions for import info
  void SetMemInfo( PMEMEXPIMPINFO m_pMemInfo ); 
  void SetMemInterface( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, PLOADEDTABLE pTable, PTOKENENTRY pTokBuf, int iTokBufSize ); 
  void SetSourceLanguage( char *pszSourceLang );

  // getter functions 
  void GetDescription( char *pszDescription, int iBufSize );
  void GetSourceLanguage( char *pszSourceLang, int iBufSize );
  BOOL IsHeaderDone( void );
  BOOL ErrorOccured( void );
  void GetErrorText( char *pszTextBuffer, int iBufSize );

  // -----------------------------------------------------------------------
  //  Handlers for the SAX DocumentHandler interface
  // -----------------------------------------------------------------------
  void startElement(const XMLCh* const name, AttributeList& attributes);
  void endElement(const XMLCh* const name );
  void characters(const XMLCh* const chars, const XMLSize_t length);
  //void ignorableWhitespace(const XMLCh* const chars, const unsigned int length);
  //void resetDocument();


  // -----------------------------------------------------------------------
  //  Handlers for the SAX ErrorHandler interface
  // -----------------------------------------------------------------------
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc );
  void fatalError(const SAXParseException& exc);
  //void resetErrors();


private:
  ELEMENTID GetElementID( PSZ_W pszName );
  void Push( PTMXELEMENT pElement );
  void Pop( PTMXELEMENT pElement );
  BOOL GetValue( PSZ pszString, int iLen, int *piResult );
  BOOL TMXLanguage2TMLanguage( PSZ pszTMLanguage, PSZ pszTMXLanguage, PSZ pszResultingLanguage );
  USHORT RemoveRTFTags( PSZ_W pszString, BOOL fTagsInCurlyBracesOnly );

  // date conversion help functions
  BOOL IsLeapYear( const int iYear );
  int GetDaysOfMonth( const int iMonth, const int iYear );
  int GetDaysOfYear( const int iYear );
  int GetYearDay( const int iDay, const int iMonth, const int iYear );

  // mem import interface data
  PMEMEXPIMPINFO m_pMemInfo;
  PFN_MEMINSERTSEGMENT pfnInsertSegment;
  LONG lMemHandle;

  // processing flags
  BOOL fSource;                        // TRUE = source data collected  
  BOOL fTarget;                        // TRUE = target data collected
  BOOL fCatchData;                     // TRUE = catch data
  BOOL fWithTagging;                   // TRUE = add tagging to data
  BOOL fWithTMXTags;                   // TRUE = segment contains TMX tags
  BOOL fTMXTagStarted;                 // TRUE = TMX inline tag started
  BOOL fHeaderDone;                    // TRUE = header has been processed
  BOOL fError;                         // TRUE = parsing ended with error

  // segment data
  ULONG ulSegNo;                       // segmet number
  LONG  lTime;                         // segment date/time
  USHORT usTranslationFlag;            // type of translation flag
  int   iNumOfTu;

  // buffers
  #define DATABUFFERSIZE 4098

  typedef struct _BUFFERAREAS
  {
    CHAR_W   szData[DATABUFFERSIZE];  // buffer for collected data
    CHAR     szProp[DATABUFFERSIZE];  // buffer for collected prop values
    CHAR_W   szPropW[DATABUFFERSIZE];  // buffer for collected prop values
    CHAR     szLang[50];              // buffer for language
    CHAR     szDocument[EQF_DOCNAMELEN];// buffer for document name
    MEMEXPIMPSEG SegmentData;         // buffer for segment data
    CHAR     szDescription[1024];     // buffer for memory descripion
    CHAR     szMemSourceLang[50];     // buffer for memory source language
    CHAR     szErrorMessage[1024];    // buffer for error message text
    CHAR_W   szNote[MAX_SEGMENT_SIZE];// buffer for note text
    CHAR_W   szNoteStyle[100];        // buffer for note style
    CHAR_W   szMTMetrics[MAX_SEGMENT_SIZE];// buffer for MT metrics data
    ULONG    ulWords;                 // number of words in the segment text
    CHAR_W   szMatchSegID[MAX_SEGMENT_SIZE];// buffer for match segment ID
  } BUFFERAREAS, *PBUFFERAREAS;

  PBUFFERAREAS pBuf; 

  // TUV data area
  typedef struct _TMXTUV
  {
    CHAR_W   szText[DATABUFFERSIZE];  // buffer for TUV text
    CHAR     szLang[50];              // buffer for TUV language (converted to Tmgr language name) or original language in case of fInvalidLang
    BOOL     fInvalidChars;           // TRUE = contains invalid characters 
    BOOL     fInlineTags;             // TRUE = contains inline tagging
    BOOL     fInvalidLang;            // TRUE = the language of the TUV is invalid
  } TMXTUV, *PTMXTUV;

  PTMXTUV pTuvArray;
  int iCurTuv;                        // current TUV index
  int iTuvArraySize;                  // current size of TUV array

  // element stack
  int iStackSize;
  int iCurElement;
  TMXELEMENT CurElement;
  PTMXELEMENT pStack;

  bool            fSawErrors;
  FILE *hfLog;
  BOOL      fInvalidChars;             // TRUE = current TUV contains invalid characters 
  BOOL      fInlineTags;               // TRUE = current TUV contains inline tagging

  // data for remove tag function
  PLOADEDTABLE pRTFTable;
  PTOKENENTRY pTokBuf;
  int iTokBufSize;
  ULONG ulCP;

  void fillSegmentInfo( PTMXTUV pSource, PTMXTUV pTarget, PMEMEXPIMPSEG pSegment );

};

BOOL MADGetNextAttr( HADDDATAKEY *ppKey, PSZ_W pszAttrNameBuffer, PSZ_W pszValueBuffer, int iBufSize  );

//+----------------------------------------------------------------------------+
//| Our TMX import export class                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
class CTMXExportImport
{
  public:
    // constructor/desctructor
	  CTMXExportImport();
	  ~CTMXExportImport();
    // export methods
    USHORT WriteHeader( const char *pszOutFile, PMEMEXPIMPINFO pMemInfo );
    USHORT WriteSegment( PMEMEXPIMPSEG pSegment  );
    USHORT WriteEnd();
    // import methods
    USHORT StartImport( const char *pszInFile, PMEMEXPIMPINFO pMemInfo ); 
    USHORT ImportNext( PFN_MEMINSERTSEGMENT pfnInsertSegment, LONG lMemHandle, LONG *plProgress ); 
    USHORT EndImport(); 
    USHORT getLastError( PSZ pszErrorBuffer, int iBufferLength );

  protected:
    USHORT WriteTUV( PSZ pszLanguage, PSZ pszMarkup, PSZ_W pszSegmentData );
    USHORT PreProcessInFile( const char *pszInFile, const char *pszOutFile );


    CXmlWriter m_xw;
    TMXParseHandler *m_handler;          // our SAX handler 
    SAXParser* m_parser;
    XMLPScanToken m_SaxToken; 
    unsigned int m_iSourceSize;          // size of source file
    PTOKENENTRY m_pTokBuf;               // buffer for TaTagTokenize tokens
    CHAR m_szActiveTagTable[50];         // buffer for name of currently loaded markup table
    PLOADEDTABLE m_pLoadedTable;         // pointer to currently loaded markup table
    PLOADEDTABLE m_pLoadedRTFTable;      // pointer to loaded RTF tag table
    CHAR m_szInFile[512];                // buffer for input file
    CHAR m_TempFile[540];                // buffer for temporary file name
    BYTE m_bBuffer[TMX_BUFFER_SIZE+1];
    PMEMEXPIMPINFO m_pMemInfo;
    CHAR_W m_szSegBuffer[MAX_SEGMENT_SIZE+1]; // buffer for the processing of segment data
    int  m_currentTu;                    // export: number of currently processed tu
};


//+----------------------------------------------------------------------------+
//| Interface functions called by TranslationManager                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
extern "C" 
__declspec(dllexport)
USHORT __cdecl EXTMEMEXPORTSTART
( 
  PLONG            plHandle,           // ptr to buffer receiving the handle for the external/import
  PSZ              pszOutFile,         // fully qualified file name for the exported memory
  PMEMEXPIMPINFO   pMemInfo            // pointer to structure containing general information for the exported memory
)
{
  USHORT           usRC = 0;           // function return code

  CTMXExportImport *pTMXExport = new CTMXExportImport;

  pTMXExport->WriteHeader( pszOutFile, pMemInfo ); 

  *plHandle = (LONG)pTMXExport;

  return( usRC );
} /* end of function EXTMEMEXPORTSTART */

extern "C" 
__declspec(dllexport)
USHORT __cdecl EXTMEMEXPORTPROCESS
( 
  LONG             lHandle,                 // export/import handle as set by ExtMemExportStart function
  PMEMEXPIMPSEG    pSegment                 // pointer to structure containing the segment data
)
{
  USHORT           usRC = 0;           // function return code

  CTMXExportImport *pTMXExport = (CTMXExportImport *)lHandle;

  pTMXExport->WriteSegment( pSegment ); 

  return( usRC );
} /* end of function EXTMEMEXPORTPROCESS */

//
// EXTMEMEXPORTEND
//
// This function is called once at the end of the memory export
extern "C" 
__declspec(dllexport)
USHORT __cdecl EXTMEMEXPORTEND
( 
  LONG             lHandle                  // export/import handle as set by ExtMemExportStart function
)
{
  USHORT           usRC = 0;           // function return code

  CTMXExportImport *pTMXExport = (CTMXExportImport *)lHandle;

  pTMXExport->WriteEnd(); 

  delete pTMXExport;

  return( usRC );
} /* end of function EXTMEMEXPORTEND */

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

  CTMXExportImport *pTMXExport = new CTMXExportImport;

  usRC = pTMXExport->StartImport( pszInFile, pMemInfo ); 

  *plHandle = (LONG)pTMXExport;

  return( usRC );
} /* end of function EXTMEMIMPORTSTART */

extern "C" 
__declspec(dllexport) USHORT __cdecl EXTMEMIMPORTPROCESS
( 
  LONG             lHandle,                 // Export/import handle as set by ExtMemImportStart function
  PFN_MEMINSERTSEGMENT pfnInsertSegment,    // callback function to insert segments into the memory
  LONG             lMemHandle,              // memory handle (has to be passed to pfnInsertSegment function)
  PLONG            plProgress               // caller's progress indicator
)
{
  USHORT           usRC = 0;           // function return code

  CTMXExportImport *pTMXExport = (CTMXExportImport *)lHandle;

  usRC = pTMXExport->ImportNext( pfnInsertSegment, lMemHandle, plProgress ); 

  return( usRC );
} /* end of function EXTMEMIMPORTPROCESS */

extern "C" 
__declspec(dllexport) USHORT __cdecl EXTMEMIMPORTEND
( 
  LONG             lHandle                  // Import/import handle as set by ExtMemImportStart function
)
{
  USHORT           usRC = 0;           // function return code

  CTMXExportImport *pTMXExport = (CTMXExportImport *)lHandle;

  usRC = pTMXExport->EndImport(); 

  delete pTMXExport;

  return( usRC );
} /* end of function EXTMEMIMPORTEND */

extern "C" 
__declspec(dllexport) USHORT __cdecl EXTMEMIMPORTGETLASTERROR
( 
  LONG             lHandle,                 // Export/import handle as set by ExtMemImportStart function
  PSZ              pszErrorBuffer,          // pointer to buffer for the error text
  int              iBufferLength            // sizeof buffer in number of characters
)
{
  USHORT           usRC = 0;           // function return code

  CTMXExportImport *pTMXExport = (CTMXExportImport *)lHandle;

  pTMXExport->getLastError( pszErrorBuffer, iBufferLength ); 

  return( usRC );
} /* end of function EXTMEMIMPORTGETLASTERROR */



//+----------------------------------------------------------------------------+
//| Implementation of TMXImportExport class                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
CTMXExportImport::CTMXExportImport()
{
  m_pTokBuf = (PTOKENENTRY)malloc( TMXTOKBUFSIZE );
  m_pLoadedTable = NULL;
  m_szActiveTagTable[0] = EOS;
}

CTMXExportImport::~CTMXExportImport()
{
  if ( m_pTokBuf ) free( m_pTokBuf );
  if ( m_pLoadedTable ) TAFreeTagTable( m_pLoadedTable );
  if ( m_pLoadedRTFTable ) TAFreeTagTable( m_pLoadedRTFTable );
}

USHORT CTMXExportImport::WriteHeader
( 
  const char *pszOutFile,
  PMEMEXPIMPINFO pMemInfo 
)
{
  this->m_pMemInfo = pMemInfo;
  this->m_currentTu = 1;

  // write TMX header
  m_xw.SetFileName( pszOutFile );
  m_xw.Formatting = CXmlWriter::Indented;
  m_xw.Indention = 2;
  if ( pMemInfo->fUTF16 )
  {
    m_xw.Encoding = CXmlWriter::UTF16;
  }
  else
  {
    m_xw.Encoding = CXmlWriter::UTF8;
  } /* endif */

  m_xw.WriteStartDocument();

  m_xw.WriteStartElement( "tmx" );
  m_xw.WriteAttributeString( "version", "1.4" ); 

  m_xw.WriteStartElement( "header" );
  char szVersion[20];
  sprintf( szVersion, "%d.%d.%d.%d", EQF_DRIVER_VERSION, EQF_DRIVER_RELEASE, EQF_DRIVER_SUBRELEASE, EQF_DRIVER_BUILD );
  m_xw.WriteAttributeString( "creationtoolversion", szVersion ); 
  m_xw.WriteAttributeString( "segtype", "sentence" ); 
  m_xw.WriteAttributeString( "adminlang", "en-us" ); 


  {
    // get TMX language name
    CHAR szTMXLang[20];                  // buffer for TMX language
    szTMXLang[0] = EOS;
    FindName( Language_Tmgr2TMX, pMemInfo->szSourceLang, szTMXLang, sizeof(szTMXLang) );
    m_xw.WriteAttributeString( "srclang", szTMXLang ); 
  }
 
  m_xw.WriteAttributeString( "o-tmf", "OpenTM2" ); 
  m_xw.WriteAttributeString( "creationtool", "OpenTM2" ); 

  // try to get data type of markup
  {
    CHAR  szDataType[40];

    szDataType[0] = EOS;
    FindName( Markup2Type, pMemInfo->szFormat, szDataType, sizeof(szDataType) );
    if ( szDataType[0] )
    {
      m_xw.WriteStartAttribute( "datatype" );
      m_xw.WriteString( szDataType ); 
      m_xw.WriteEndAttribute();
    }
    else
    {
      m_xw.WriteAttributeString( "datatype", "plaintext" ); 
    } /* endif */
  }

  // add memory description as as property
  if ( pMemInfo->szDescription[0] )
  {
    m_xw.WriteStartElement( "prop" );
    m_xw.WriteAttributeString( "type", DESCRIPTION_PROP );
    m_xw.WriteString( pMemInfo->szDescription ); 
    m_xw.WriteEndElement(); // prop
  } /* endif */

  m_xw.WriteEndElement(); // header

  m_xw.WriteStartElement( "body" );
  return( 0 );
}

USHORT CTMXExportImport::WriteSegment
( 
  PMEMEXPIMPSEG pSegment  
)
{
  //CHAR             szDocument[EQF_DOCNAMELEN];   // document  
  //BOOL             fMachineTranslation;          // TRUE = segment is a machine translation 
  //CHAR             szAuthor[EQF_AUTHORLEN];      // author of segment


  m_xw.WriteStartElement( "tu" );
  m_xw.WriteStartAttribute( "tuid" );
  m_xw.WriteInt( this->m_currentTu++ ); 
  m_xw.WriteEndAttribute();

  // try to get data type of markup
  {
    CHAR  szDataType[40];

    szDataType[0] = EOS;
    FindName( Markup2Type, pSegment->szFormat, szDataType, sizeof(szDataType) );
    if ( szDataType[0] )
    {
      m_xw.WriteStartAttribute( "datatype" );
      m_xw.WriteString( szDataType ); 
      m_xw.WriteEndAttribute();
    } /* endif */
  }
  
  // creation date/time
  {
    CHAR szTime[30];
    LONG lTime = pSegment->lTime;
    struct tm   *pTimeDate;            // time/date structure

    if ( lTime != 0L ) lTime += 10800L;// correction: + 3 hours

    pTimeDate = gmtime( &lTime );
    if ( (lTime != 0L) && pTimeDate )   // if gmtime was successful ...
    {
      sprintf( szTime, "%4.4d%2.2d%2.2dT%2.2d%2.2d%2.2dZ", 
               pTimeDate->tm_year + 1900, pTimeDate->tm_mon + 1, pTimeDate->tm_mday,
               pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec );
      m_xw.WriteStartAttribute( "creationdate" );
      m_xw.WriteString( szTime ); 
      m_xw.WriteEndAttribute();
    } /* endif */
  }

  // add segment number as property
  m_xw.WriteStartElement( "prop" );
  m_xw.WriteAttributeString( "type", SEGNUM_PROP );
  m_xw.WriteInt( pSegment->lSegNum );
  m_xw.WriteEndElement(); // prop

  // add original tmgr markup as property
  m_xw.WriteStartElement( "prop" );
  m_xw.WriteAttributeString( "type", MARKUP_PROP );
  m_xw.WriteString( pSegment->szFormat ); 
  m_xw.WriteEndElement(); // prop

  // add document name property
  m_xw.WriteStartElement( "prop" );
  m_xw.WriteAttributeString( "type", DOCNAME_PROP );
  m_xw.WriteString( pSegment->szDocument ); 
  m_xw.WriteEndElement(); // prop

  // add translation flag as property
  if ( pSegment->usTranslationFlag == TRANSLFLAG_MACHINE )
  {
    m_xw.WriteStartElement( "prop" );
    m_xw.WriteAttributeString( "type", TRANSLFLAG_PROP );
    m_xw.WriteString( "1" ); 
    m_xw.WriteEndElement(); // prop
  } 
  else if ( pSegment->usTranslationFlag == TRANSLFLAG_GLOBMEM )
  {
    m_xw.WriteStartElement( "prop" );
    m_xw.WriteAttributeString( "type", TRANSLFLAG_PROP );
    m_xw.WriteString( "2" ); 
    m_xw.WriteEndElement(); // prop
  } /* endif */

  // add segment note as property
  if ( pSegment->szAddInfo[0] != 0 )
  {
    static CHAR_W szBuffer[MAX_SEGMENT_SIZE];
    HADDDATAKEY hKey = MADSearchKey( pSegment->szAddInfo, L"Note" );
    if ( hKey != NULL )
    {
      MADGetAttr( hKey, L"style", szBuffer, sizeof(szBuffer) / sizeof(CHAR_W), L"" );
      if ( szBuffer[0] != EOS ) 
      {
        m_xw.WriteStartElement( "prop" );
        m_xw.WriteAttributeString( "type", NOTESTYLE_PROP );
        m_xw.WriteString( szBuffer); 
        m_xw.WriteEndElement(); // prop
      } /* endif */         

      MDAGetValueForKey( hKey, szBuffer, sizeof(szBuffer) / sizeof(CHAR_W), L"" );
      if ( szBuffer[0] != EOS ) 
      {
        m_xw.WriteStartElement( "prop" );
        m_xw.WriteAttributeString( "type", NOTE_PROP );
        m_xw.WriteString( szBuffer); 
        m_xw.WriteEndElement(); // prop
      } /* endif */         
    } /* endif */           

    hKey = MADSearchKey( pSegment->szAddInfo, L"MT" );
    if ( hKey != NULL )
    {
      BOOL fAttribute = TRUE;
      CHAR_W szAttrNameBuffer[40];
      while ( fAttribute )
      {
        fAttribute = MADGetNextAttr( &hKey, szAttrNameBuffer, szBuffer, sizeof(szBuffer) / sizeof(CHAR_W) );
        if ( fAttribute )
        {
          m_xw.WriteStartElement( "prop" );
          m_xw.WriteAttributeString( L"type", szAttrNameBuffer );
          m_xw.WriteString( szBuffer); 
          m_xw.WriteEndElement(); // prop
        } /* endif */           
      } /* endwhile */         
    } /* endif */

    hKey = MADSearchKey( pSegment->szAddInfo, L"wcnt" );
    if ( hKey != NULL )
    {
      BOOL fAttribute = TRUE;
      CHAR_W szAttrNameBuffer[40];
      while ( fAttribute )
      {
        fAttribute = MADGetNextAttr( &hKey, szAttrNameBuffer, szBuffer, sizeof(szBuffer) / sizeof(CHAR_W) );
        if ( fAttribute && (wcsicmp( szAttrNameBuffer, L"words" ) == 0) )
        {
          m_xw.WriteStartElement( "prop" );
          m_xw.WriteAttributeString( L"type", L"tmgr:words" );
          m_xw.WriteString( szBuffer); 
          m_xw.WriteEndElement(); // prop
        } /* endif */           
      } /* endwhile */         
    } /* endif */


    hKey = MADSearchKey( pSegment->szAddInfo, L"MatchSegID" );
    if ( hKey != NULL )
    {
      BOOL fAttribute = TRUE;
      CHAR_W szAttrNameBuffer[40];
      while ( fAttribute )
      {
        fAttribute = MADGetNextAttr( &hKey, szAttrNameBuffer, szBuffer, sizeof(szBuffer) / sizeof(CHAR_W) );
        if ( fAttribute && (wcsicmp( szAttrNameBuffer, L"ID" ) == 0) )
        {
          m_xw.WriteStartElement( "prop" );
          m_xw.WriteAttributeString( L"type", L"tmgr:MatchSegID" );
          m_xw.WriteString( szBuffer); 
          m_xw.WriteEndElement(); // prop
        } /* endif */           
      } /* endwhile */         
    } /* endif */
  } /* endif */


  // write source tuv
  WriteTUV( pSegment->szSourceLang, pSegment->szFormat, pSegment->szSource );

  // write target tuv
  WriteTUV( pSegment->szTargetLang, pSegment->szFormat, pSegment->szTarget );

  m_xw.WriteEndElement(); // tu

  return( 0 );
}

// write a TUV
USHORT CTMXExportImport::WriteTUV
( 
  PSZ   pszLanguage,                   // language to be used
  PSZ   pszMarkup,                     // markup of data   
  PSZ_W   pszSegmentData               // actual segment data (UTF-16!)
)
{
  CHAR szTMXLang[20];                  // buffer for TMX language


  // get TMX language name
  szTMXLang[0] = EOS;
  FindName( Language_Tmgr2TMX, pszLanguage, szTMXLang, sizeof(szTMXLang) );

  // start TUV
  m_xw.WriteStartElement( "tuv" );
  m_xw.WriteAttributeString( "xml:lang", szTMXLang ); 

  // add original Tmgr language as property
  m_xw.WriteStartElement( "prop" );
  m_xw.WriteAttributeString( "type", LANGUAGE_PROP );
  m_xw.WriteString( pszLanguage ); 
  m_xw.WriteEndElement(); // prop

  // write segment data and mask inline tags
  m_xw.WriteStartElement( "seg" );
  m_xw.Formatting = CXmlWriter::None;             // no indention within segment

  // write segment data as-is when one of the XLIFF markup tables is being used
  if ( (stricmp( pszMarkup, "OTMXUXLF" ) == 0) || (stricmp( pszMarkup, "OTMXMXLF" ) == 0 ) || (stricmp( pszMarkup, "OTMXAXLF" ) == 0 ) )
  {
    m_xw.WriteString( L"" ); // needed to properly close the start element tag
    m_xw.WriteRaw( pszSegmentData );
  }
  else
  {
    USHORT usRC = 0;

    if ( strcmp( m_szActiveTagTable, pszMarkup ) != 0 )
    {
      // free loaded tag table
      if ( m_pLoadedTable )
      {
        TAFreeTagTable( m_pLoadedTable );
        m_pLoadedTable = NULL;
      } /* endif */

      // load document tag table
      usRC = TALoadTagTableExHwnd( pszMarkup, (PLOADEDTABLE *)&m_pLoadedTable, FALSE, 
                                   TALOADUSEREXIT | TALOADPROTTABLEFUNC, FALSE, NULLHANDLE );
    } /* endif */

    // copy segment data to our buffer and use this buffer from now on
    wcscpy( this->m_szSegBuffer, pszSegmentData );
    pszSegmentData = this->m_szSegBuffer;

    // reduce CRLF when requested (and tag table allows CRLF changes)
    if ( (this->m_pMemInfo->fNoCRLF) && (m_pLoadedTable->fReflow != TAGREFLOW_NO) )
    {
      PSZ_W pszSource = pszSegmentData;
      PSZ_W pszTarget = pszSegmentData;
      CHAR_W chLastChar = L' ';
      while ( *pszSource != 0 )
      {
        if ( *pszSource == L'\r'  )
        {
          pszSource++;
          if ( *pszSource == L'\n' )
          {
            // skip following linefeed as well
            pszSource++;
          } /* endif */             

          if ( (chLastChar != L' ') && (*pszSource != L' ') && (*pszSource != 0 ))
          {
            // insert a blank as linebreak is not prefixed or suffixed by whitespace
            chLastChar = *pszTarget++ = L' ';
          } /* endif */             
        }
        else if ( *pszSource == L'\n'  )
        {
          pszSource++;
          if ( (chLastChar != L' ') && (*pszSource != L' ') && (*pszSource != 0 ))
          {
            // insert a blank as linebreak is not prefixed or suffixed by whitespace
            chLastChar = *pszTarget++ = L' ';
          } /* endif */             
        }
        else
        {
          chLastChar = *pszTarget++ = *pszSource++;
        } /* endif */           
      } /* endwhile */       
      *pszTarget = 0;
    } /* endif */       

      if ( !usRC )
      {
        ULONG ulCP = GetLangAnsiCP( pszLanguage );

        // build protect start/stop table for tag recognition
        PBYTE  pStartStop = NULL;
        usRC = TACreateProtectTableW( pszSegmentData, m_pLoadedTable, 0, (PTOKENENTRY)m_pTokBuf, TMXTOKBUFSIZE,
                                      (PSTARTSTOP *)&pStartStop, m_pLoadedTable->pfnProtTable, 
                                      m_pLoadedTable->pfnProtTableW, ulCP );

        if ( !usRC )
        {
          PSTARTSTOP pEntry = (PSTARTSTOP)pStartStop;
          while ( (pEntry->usStart != 0) || (pEntry->usStop != 0)  || (pEntry->usType != 0) )
          {
            switch ( pEntry->usType )
            {
              case UNPROTECTED_CHAR :
                // write translatable text
                {
                  CHAR_W chTemp = pszSegmentData[pEntry->usStop+1];
                  pszSegmentData[pEntry->usStop+1] = 0;
                  m_xw.WriteString( pszSegmentData + pEntry->usStart ); 
                  pszSegmentData[pEntry->usStop+1] = chTemp;
                }
                break;
              default :
                // handle not-translatable data

                // if next entry is also a protected one ...
                if ( ((pEntry+1)->usStart != 0) &&
                      ((pEntry+1)->usStop != 0)  &&
                      ((pEntry+1)->usType != UNPROTECTED_CHAR) )
                {
                  // combine this entry with the next one
                  (pEntry+1)->usStart = pEntry->usStart;
                }
                else
                {
                  // mask tagging
                  m_xw.WriteStartElement( "ph" );
                  CHAR_W chTemp = pszSegmentData[pEntry->usStop+1];
                  pszSegmentData[pEntry->usStop+1] = 0;
                  m_xw.WriteString( pszSegmentData + pEntry->usStart ); 
                  pszSegmentData[pEntry->usStop+1] = chTemp;
                  m_xw.WriteEndElement(); 
                } /* endif */
                break;
            } /* endswitch */
            pEntry++;
          } /* endwhile */
        } /* endif */
        UtlAlloc( (PVOID *)&pStartStop, 0L, 0L, NOMSG );
      } /* endif */
  }

  // end open elements
  m_xw.WriteEndElement(); // seg
  m_xw.Formatting = CXmlWriter::Indented;             // restart indention 
  m_xw.WriteEndElement(); // tuv

  return( 0 );
}

USHORT CTMXExportImport::WriteEnd()
{
  m_xw.WriteEndElement(); // body
  m_xw.WriteEndElement(); // tmx
  m_xw.WriteEndDocument();
  m_xw.Close();
  return( 0 );
}

USHORT CTMXExportImport::StartImport
( 
  const char *pszInFile, 
  PMEMEXPIMPINFO pMemInfo 
)
{
  USHORT usRC = 0;

  this->m_TempFile[0] = EOS;
  this->m_pMemInfo = pMemInfo;

  if ( pMemInfo->fCleanRTF )
  {
    //TALoadTagTableExHwnd( "EQFRTF", &m_pLoadedRTFTable, FALSE, TALOADUSEREXIT | TALOADPROTTABLEFUNC, FALSE, HWND_FUNCIF );
  } /* endif */

  // preprocess TMX file
  if ( !usRC )
  {
    // setup tempory file name
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

  // parse the TMX file
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
      m_handler = new TMXParseHandler();

      // pass memory info to handler
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
        // parse until header processed of no more data available
        BOOL fContinue  = TRUE;
        while (fContinue && !m_parser->getErrorCount() && !m_handler->IsHeaderDone() )
        {
          fContinue = m_parser->parseNext(m_SaxToken);
        } /*endwhile */

        if ( m_handler->ErrorOccured() )
        {
          m_handler->GetErrorText( pMemInfo->szError, sizeof(pMemInfo->szError) );
          pMemInfo->fError = TRUE;
        } /* endif */

        // stop import if no more data to follow
        if ( !fContinue && !m_handler->IsHeaderDone() )
        {
          m_parser->parseReset(m_SaxToken);
          usRC = ERROR_BAD_FORMAT;
        } /* endif */

        // get description and source language of memory
        if ( m_handler->IsHeaderDone() )
        {
          // save target memory source language
          CHAR szMemSourceLanguage[50]; 
          strcpy( szMemSourceLanguage, pMemInfo->szSourceLang );

          // get description and source language of imported memory
          m_handler->GetDescription( pMemInfo->szDescription, sizeof(pMemInfo->szDescription) );
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

USHORT CTMXExportImport::ImportNext
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

  m_handler->SetMemInterface( pfnInsertSegment, lMemHandle, m_pLoadedRTFTable, this->m_pTokBuf, TMXTOKBUFSIZE ); 
    
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
        if ( errorCount )
        {
          usRC = ERROR_MEMIMP_ERROR;
        }
        else
        {
          usRC = MEM_IMPORT_COMPLETE;
        }
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

USHORT CTMXExportImport::EndImport()
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

USHORT CTMXExportImport::getLastError
( 
  PSZ              pszErrorBuffer,          // pointer to buffer for the error text
  int              iBufferLength            // sizeof buffer in number of characters
)
{
  strncpy( pszErrorBuffer, this->m_pMemInfo->szError, iBufferLength - 1);
  pszErrorBuffer[iBufferLength-1] = 0;
  return( 0 );
}

// replace invalid characters in input file
USHORT CTMXExportImport::PreProcessInFile
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
      int iBytes = fread( m_bBuffer, 1, TMX_BUFFER_SIZE, hIn );
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




// find the given name and return its value in the supplied buffer
BOOL FindName( PNAMETABLE pTable, const char *pszName, char *pszValue, int iBufSize )
{
  BOOL fFound = FALSE;
  PNAMETABLE  pEntry = pTable;

  while ( !fFound && (pEntry->pszName != NULL) )
  {
    if ( _stricmp( pEntry->pszName, pszName ) == 0 )
    {
      memset( pszValue, 0, iBufSize );
      strncpy( pszValue, pEntry->pszValue, iBufSize - 1 );
      fFound = TRUE;
    }
    else
    {
      // try next entry
      pEntry++;
    } /* endif */
  } /*endwhile */

  return( fFound );
}

//
// Implementation of TMX SAX parser
//
TMXParseHandler::TMXParseHandler()
{
#ifdef _DEBUG
  //hfLog = fopen( "C:\\SAXTEST.LOG", "a" );
  //if ( hfLog )
  //{
  //  fprintf( hfLog, "**** SAXTEST ****\n" );
  //} /* endif */
  hfLog = NULLHANDLE;
#else
  hfLog = NULLHANDLE;
#endif 

  // allocate buffer areas
  pBuf = (PBUFFERAREAS)malloc( sizeof(BUFFERAREAS) );
  if ( pBuf ) memset( pBuf, 0, sizeof(BUFFERAREAS) );
  fError = FALSE;

  // initialize element stack
  iStackSize = 0;
  iCurElement = 0;
  pStack = NULL;

  pfnInsertSegment = NULL;
  lMemHandle = 0;
  fHeaderDone = FALSE;
  fSource = FALSE;
  fTarget= FALSE;
  fCatchData= FALSE;
  fWithTagging= FALSE;
  fWithTMXTags= FALSE;
  fTMXTagStarted= FALSE;

  // initialize TUV array
  pTuvArray = NULL;
  iCurTuv = 0;
  iTuvArraySize = 0;

  ulCP = GetLangOEMCP( NULL );

}

TMXParseHandler::~TMXParseHandler()
{
  if ( hfLog )  fclose( hfLog );
  if ( pStack ) free( pStack );
  if ( pBuf )   free( pBuf );
  if ( pTuvArray ) free( pTuvArray );
}

void TMXParseHandler::startElement(const XMLCh* const name, AttributeList& attributes)
{
    PSZ_W pszName = (PSZ_W)name;
    int iAttribs = attributes.getLength(); 

    if ( hfLog ) fprintf( hfLog, "StartElement: %S\n", pszName );

    Push( &CurElement );

    CurElement.ID = GetElementID( pszName );
    CurElement.PropID = UNKNOWN_PROP;            // reset prop ID of current element 

    switch ( CurElement.ID )
    {
      case TMX_ELEMENT:
        // reset element info
        memset( &CurElement, 0, sizeof(CurElement) );
        this->iNumOfTu = 0;
        break;
      case PROP_ELEMENT:
        // check if this is one of our own props
        CurElement.PropID = UNKNOWN_PROP;
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( _wcsicmp( pszName, L"type" ) == 0 )
          {
            PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
            if ( pszValue != NULL )
            {
              if ( (wcsicmp( pszValue, MARKUP_PROP_W ) == 0) || (wcsicmp( pszValue, MARKUP_PROP_W_OLD ) == 0) )
              {
                CurElement.PropID = TMMARKUP_PROP;
              }
              else if ( (wcsicmp( pszValue, LANGUAGE_PROP_W ) == 0) || (wcsicmp( pszValue, LANGUAGE_PROP_W_OLD ) == 0) )
              {
                CurElement.PropID = TMLANGUAGE_PROP;
              }
              else if ( (wcsicmp( pszValue, DOCNAME_PROP_W ) == 0) || (wcsicmp( pszValue, DOCNAME_PROP_W_OLD ) == 0) )
              {
                CurElement.PropID = TMDOCNAME_PROP;
              }
              else if ( (wcsicmp( pszValue, MACHFLAG_PROP_W ) == 0) || (wcsicmp( pszValue, MACHFLAG_PROP_W_OLD ) == 0) )
              {
                CurElement.PropID = MACHINEFLAG_PROP;
              } 
              else if ( (wcsicmp( pszValue, TRANSLFLAG_PROP_W ) == 0) || (wcsicmp( pszValue, TRANSLFLAG_PROP_W_OLD ) == 0) )
              {
                CurElement.PropID = TRANSLATIONFLAG_PROP;
              } 
              else if ( (wcsicmp( pszValue, NOTE_PROP_W ) == 0) || (wcsicmp( pszValue, NOTE_PROP_W_OLD ) == 0) )
              {
                CurElement.PropID = TMNOTE_PROP;
              } 
              else if ( (wcsicmp( pszValue, NOTESTYLE_PROP_W ) == 0) || (wcsicmp( pszValue, NOTE_PROP_W_OLD ) == 0) )
              {
                CurElement.PropID = TMNOTESTYLE_PROP;
              } 
              else if ( (wcsicmp( pszValue, SEGNUM_PROP_W ) == 0) || (wcsicmp( pszValue, SEGNUM_PROP_W_OLD ) == 0) )
              {
                CurElement.PropID = SEG_PROP;
              } 
              else if ( (wcsicmp( pszValue, DESCRIPTION_PROP_W ) == 0) || (wcsicmp( pszValue, DESCRIPTION_PROP_W_OLD ) == 0) )
              {
                CurElement.PropID = TMDESCRIPTION_PROP;
              } 
              else if ( (wcsicmp( pszValue, TMMATCHTYPE_PROP_W ) == 0 ) || (wcsicmp( pszValue, TMMATCHTYPE_PROP_W_OLD ) == 0 ) )
              {
                CurElement.PropID = TMTMMATCHTYPE_PROP;
              } 
              else if ( (wcsicmp( pszValue, MTSERVICE_PROP_W ) == 0 ) || (wcsicmp( pszValue, MTSERVICE_PROP_W_OLD ) == 0 ) )
              {
                CurElement.PropID = TMMTSERVICE_PROP;
              } 
              else if ( (wcsicmp( pszValue, MTMETRICNAME_PROP_W ) == 0 ) || (wcsicmp( pszValue, MTMETRICNAME_PROP_W_OLD ) == 0 ) )
              {
                CurElement.PropID = TMMTMETRICNAME_PROP;
              } 
              else if ( (wcsicmp( pszValue, MTMETRICVALUE_PROP_W ) == 0 ) || (wcsicmp( pszValue, MTMETRICVALUE_PROP_W_OLD ) == 0 ) )
              {
                CurElement.PropID = TMMTMETRICVALUE_PROP;
              } 
              else if ( (wcsicmp( pszValue, PEEDITDISTANCECHARS_PROP_W ) == 0 ) || (wcsicmp( pszValue, PEEDITDISTANCECHARS_PROP_W_OLD ) == 0 ) )
              {
                CurElement.PropID = TMPEEDITDISTANCECHARS_PROP;
              } 
              else if ( (wcsicmp( pszValue, PEEDITDISTANCEWORDS_PROP_W ) == 0 ) || (wcsicmp( pszValue, PEEDITDISTANCEWORDS_PROP_W_OLD ) == 0 ) )
              {
                CurElement.PropID = TMPEEDITDISTANCEWORDS_PROP;
              } 
              else if ( (wcsicmp( pszValue, MTFIELDS_PROP_W ) == 0 ) || (wcsicmp( pszValue, MTFIELDS_PROP_W_OLD ) == 0 ) )
              {
                CurElement.PropID = TMMTFIELDS_PROP;
              } 
              else if ( wcsicmp( pszValue, WORDS_PROP_W ) == 0 )
              {
                CurElement.PropID = TMWORDS_PROP;
              } 
              else if ( wcsicmp( pszValue, MATCHSEGID_PROP_W ) == 0 )
              {
                CurElement.PropID = TMMATCHSEGID_PROP;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endfor */
        pBuf->szProp[0] = 0;                 // reset data buffer
        pBuf->szPropW[0] = 0;                 // reset data buffer
        break;

      case HEADER_ELEMENT:
        // reset element info
        CurElement.fInlineTagging = FALSE;
        CurElement.fInsideTagging = FALSE;
        CurElement.PropID = UNKNOWN_PROP;
        CurElement.szDataType[0] = 0;
        CurElement.szDataType[0] = 0;
        CurElement.szTMLanguage[0] = 0;
        CurElement.szTMMarkup[0] = 0;
        CurElement.szTMXLanguage[0] = 0;

        // scan header attributes, currently only the source language and the data type are of interest
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( _wcsicmp( pszName, L"datatype" ) == 0 )
          {
            char *pszValue = XMLString::transcode(attributes.getValue( i ));
            if ( pszValue != NULL ) strcpy( CurElement.szDataType, pszValue );
            XMLString::release( &pszValue );

            // special handling for datatype in header: set datataype in all elements on the stack
            if ( pStack && iCurElement )
            {
              int i = iCurElement;
              while ( i )
              {
                i--;
                strcpy( pStack[i].szDataType, CurElement.szDataType );
              } /*endwhile */
            } /* endif */

          }
          else if ( _wcsicmp( pszName, L"srclang" ) == 0 )
          {
            char *pszValue = XMLString::transcode(attributes.getValue( i ));
            if ( pszValue != NULL )
            {
              if ( _stricmp( pszValue, "*all*" ) == 0 )
              {
                // use source language of memory
                pBuf->szMemSourceLang[0] = 0; 
              }
              else
              {
                TMXLanguage2TMLanguage( "", pszValue, pBuf->szMemSourceLang );
                if ( strcmp( pBuf->szMemSourceLang, OTHERLANGUAGES ) == 0 )
                {
                  pBuf->szMemSourceLang[0] = 0; 
                } /* endif */
              } /* endif */

              XMLString::release( &pszValue );
            } /* endif */
          } /* endif */
        } /* endfor */
        break;
      case TU_ELEMENT:
        // reset segment data 
        usTranslationFlag = TRANSLFLAG_NORMAL;

        pBuf->szNote[0] = 0;
        pBuf->szNoteStyle[0] = 0;
        pBuf->szMTMetrics[0] = 0;
        pBuf->szMatchSegID[0] = 0;
        pBuf->ulWords = 0;

        // reset TUV array
        iCurTuv = 0;
        this->iNumOfTu += 1;

        ulSegNo = this->iNumOfTu; // preset segment number

        // scan <tu> attributes, currently the data type, the tuid and the creationdate are of interest
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( _wcsicmp( pszName, L"tuid" ) == 0 )
          {
            char *pszValue = XMLString::transcode(attributes.getValue( i ));
            if ( pszValue != NULL ) ulSegNo = atol( pszValue );
            XMLString::release( &pszValue );
          } 
          else if ( _wcsicmp( pszName, L"datatype" ) == 0 )
          {
            char *pszValue = XMLString::transcode(attributes.getValue( i ));
            if ( pszValue != NULL ) strcpy( CurElement.szDataType, pszValue );
            XMLString::release( &pszValue );
          } 
          else if ( _wcsicmp( pszName, L"creationdate" ) == 0 )
          {
            char *pszDate = XMLString::transcode(attributes.getValue( i ));
            // we currently support only dates in the form YYYYMMDDThhmmssZ
            if ( (pszDate != NULL) && (strlen(pszDate) == 16) )
            {
              int iYear = 0, iMonth = 0, iDay = 0, iHour = 0, iMin = 0, iSec = 0, iDaysSince1970 = 0;

              // split string into date/time parts
              BOOL fOK = GetValue( pszDate, 4, &iYear );
              if ( fOK ) fOK = GetValue( pszDate + 4, 2, &iMonth );
              if ( fOK ) fOK = GetValue( pszDate + 6, 2, &iDay );
              if ( fOK ) fOK = GetValue( pszDate + 9, 2, &iHour );
              if ( fOK ) fOK = GetValue( pszDate + 11, 2, &iMin );
              if ( fOK ) fOK = GetValue( pszDate + 13, 2, &iSec );

              // convert date to number of days since 1.1.1970
              {
                iDaysSince1970 = GetYearDay( iDay, iMonth, iYear );
                for( int i = 1970; i < iYear; i++ )
                {
                  iDaysSince1970 += GetDaysOfYear( i );  
                } /* endfor */
                iDaysSince1970 -= 1;              // remove first day (1.1.1970)
              }

              // convert to a long value
              if ( fOK )
              {
                lTime =  iSec                      +
                        (iMin       *         60L) +
                        (iHour      *       3600L) +
                        ((iDaysSince1970) * 86400L);
                lTime -= 10800L;                  // correction for OS/2 format: - 3 hours
              } /* endif */
            } /* endif */
            XMLString::release( &pszDate );
          } /* endif */
        } /* endfor */
        break;
      case TUV_ELEMENT:
        // reset language info, any <tuv> needs its own language setting
        CurElement.szTMXLanguage[0] = 0;
        CurElement.szTMLanguage[0] = 0;
        this->fInvalidChars = FALSE;
        this->fInlineTags = FALSE;
        this->fTMXTagStarted = FALSE;
        this->fWithTMXTags = FALSE;
        this->fWithTagging = FALSE;
        // scan tuv attributes, currently only the language is of interest
        for( int i = 0; i < iAttribs; i++ )
        {
          PSZ_W pszName = (PSZ_W)attributes.getName( i );
          if ( (_wcsicmp( pszName, L"xml:lang" ) == 0) || (_wcsicmp( pszName, L"lang" ) == 0) )
          {
            char *pszValue = XMLString::transcode(attributes.getValue( i ));
            if ( pszValue != NULL )
            {
              strcpy( CurElement.szTMXLanguage, pszValue );
              strcpy( pBuf->szLang, pszValue );
            } /* endif */
            XMLString::release( &pszValue );
          } /* endif */
        } /* endfor */

        // set fWithTagging switch depending on datatype or TM markup
       fWithTagging = FALSE;
        break;
      case BODY_ELEMENT:
        break;
      case SEG_ELEMENT:
        CurElement.fInlineTagging = FALSE;
        CurElement.fInsideTagging = FALSE;
        pBuf->szData[0] = 0;                 // reset data buffer
        fCatchData = TRUE; 
        break;
      case BPT_ELEMENT:
      case EPT_ELEMENT:
      case PH_ELEMENT:
      case HI_ELEMENT:
      case IT_ELEMENT:
      case SUB_ELEMENT:
      case UT_ELEMENT:
        {
          CurElement.fInlineTagging = FALSE;
          CurElement.fInsideTagging = TRUE;
          this->fWithTMXTags = TRUE;
          this->fWithTagging = TRUE;

          int iCurLen = wcslen( pBuf->szData );

          // close any previous TMX tag
          if ( this->fTMXTagStarted )
          {
            // close open TMX inline tag
            if ( (iCurLen + 2) < DATABUFFERSIZE)
            {
              pBuf->szData[iCurLen++] = '>';
              pBuf->szData[iCurLen] = 0;
            } /* endif */
            this->fTMXTagStarted = FALSE;
          } /* endif */

          // add tag start and text of tag to current text buffer 
          this->fTMXTagStarted = TRUE;
          int iTagLen = wcslen( pszName );
          if ( (iCurLen + iTagLen + 1 + 1) < DATABUFFERSIZE)
          {
            pBuf->szData[iCurLen] = L'<';
            wcsncpy( pBuf->szData + iCurLen + 1, pszName, iTagLen );
            iCurLen += iTagLen + 1;
          } /* endif */

          // add any attributes to current text buffer 
          for( int i = 0; i < iAttribs; i++ )
          {
            PSZ_W pszAttrName = (PSZ_W)attributes.getName( i );
            PSZ_W pszAttrValue = (PSZ_W)attributes.getValue( i );
            int iAttrNameLen = wcslen( pszAttrName );
            int iAttrValueLen = wcslen( pszAttrValue );
            if ( (iCurLen + iAttrNameLen + iAttrValueLen + 1 + 4) < DATABUFFERSIZE)
            {
              pBuf->szData[iCurLen++] = L' ';
              wcsncpy( pBuf->szData + iCurLen, pszAttrName, iAttrNameLen );
              iCurLen += iAttrNameLen;
              pBuf->szData[iCurLen++] = L'=';
              pBuf->szData[iCurLen++] = L'\"';
              wcsncpy( pBuf->szData + iCurLen, pszAttrValue, iAttrValueLen );
              iCurLen += iAttrValueLen;
              pBuf->szData[iCurLen++] = L'\"';
            } /* endif */
          } /* endfor */
          pBuf->szData[iCurLen] = 0;
        }
        break;
      case INVCHAR_ELEMENT:
        // segment contains invalid data
        this->fInvalidChars = TRUE;
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

void TMXParseHandler::fillSegmentInfo
(
  TMXParseHandler::PTMXTUV pSource,
  TMXParseHandler::PTMXTUV pTarget,
  PMEMEXPIMPSEG    pSegment
)
{
  // fill-in header data
  memset( pSegment, 0, sizeof(MEMEXPIMPSEG) );
  if ( CurElement.lSegNum != 0 )
  {
    pSegment->lSegNum = CurElement.lSegNum; 
  }
  else
  {
    pSegment->lSegNum = (LONG)ulSegNo; 
  } /* endif */     

  if ( CurElement.szTMMarkup[0] )
  {
    strcpy( pSegment->szFormat, CurElement.szTMMarkup );
  }
  else if ( m_pMemInfo->pszMarkupList != NULL )
  {
    // do not set format, will be set later
  }
  else if ( m_pMemInfo->szFormat[0] != EOS )
  {
    // specified format overwrites info in datatype
    strcpy( pSegment->szFormat, m_pMemInfo->szFormat );
  }
  else if ( CurElement.szDataType[0] ) 
  {
    FindName(  Type2Markup, CurElement.szDataType, pSegment->szFormat, sizeof(pSegment->szFormat) );
  } /* endif */

  if ( pBuf->szDocument[0] )
  {
    strcpy( pSegment->szDocument, pBuf->szDocument );
  }
  else
  {
    strcpy( pSegment->szDocument, "none" );
  } /* endif */
  pSegment->usTranslationFlag = usTranslationFlag;
  if ( lTime )
  {
    pSegment->lTime = lTime;
  }
  else
  {
    time( &(pSegment->lTime) );
  } /* endif */

  // source info
  if ( pSource != NULL )
  {
    strcpy( pSegment->szSourceLang, pSource->szLang );
    wcsncpy( pSegment->szSource, pSource->szText, SEGDATABUFLEN );
  } /* endif */

  // target info
  if ( pTarget != NULL )
  {
    strcpy( pSegment->szTargetLang, pTarget->szLang );
    wcsncpy( pSegment->szTarget, pTarget->szText, SEGDATABUFLEN );
  } /* endif */

  // note style and text
  pSegment->szAddInfo[0] = 0;
  if ( (pBuf->szNote[0]!= 0) || (pBuf->szNoteStyle[0] != 0)  )
  {
    swprintf( pSegment->szAddInfo, L"<Note style=\"%s\">%s</Note>", pBuf->szNoteStyle, pBuf->szNote );
  } /* endif */     

  // MT meta data
  if ( (pBuf->szMTMetrics[0] != 0) )
  {
    swprintf( pSegment->szAddInfo + wcslen(pSegment->szAddInfo), L"<MT%s></MT>", pBuf->szMTMetrics );
  } /* endif */     

  // word count info
  if ( (pBuf->ulWords != 0) )
  {
    swprintf( pSegment->szAddInfo + wcslen(pSegment->szAddInfo), L"<wcnt words=\"%lu\"></wcnt>", pBuf->ulWords );
  } /* endif */     

  // Match segment ID
  if ( (pBuf->szMatchSegID[0] != 0) )
  {
    swprintf( pSegment->szAddInfo + wcslen(pSegment->szAddInfo), L"<MatchSegID ID=\"%s\"/>", pBuf->szMatchSegID );
  } /* endif */     
}

void TMXParseHandler::endElement(const XMLCh* const name )
{
  PSZ_W pszName = (PSZ_W)name;
  if ( hfLog ) fprintf( hfLog, "EndElement: %S\n", pszName );
  ELEMENTID CurrentID = CurElement.ID;
  PROPID    CurrentProp = CurElement.PropID;

  switch ( CurrentID )
  {
    case TMX_ELEMENT:
      // end of data reached...
      {
        int iTUs = this->iNumOfTu;
      }
      break;
    case PROP_ELEMENT:
      // is processed after current element has been removed from stack...
      break;
    case HEADER_ELEMENT:
      fHeaderDone = TRUE;
      break;
    case TU_ELEMENT:
      // TU is complete, check collected data and add memory segment if everything is OK
      {
        int iCurrent = 0;
        PTMXTUV pSourceTuv = NULL;

        // change markup table to XLIFF when segment data contains TMX inline tags
        if ( this->fWithTMXTags )
        {
          strcpy( CurElement.szTMMarkup, "OTMXUXLF" );
        } /* endif */
        // find <tuv> containing the source data
        iCurrent = 0;
        PTMXTUV pCurrentTuv = pTuvArray;
        while ( (iCurrent < iCurTuv) && (pSourceTuv == NULL) )
        {
          if ( stricmp( pCurrentTuv->szLang, pBuf->szMemSourceLang ) == 0 )
          {
            pSourceTuv = pCurrentTuv;
          }
          else
          {
            pCurrentTuv++;
            iCurrent++;
          } /* endif */
        } /*endwhile */

        // loop over all other <tuv>s and add the pairs to the memory
        if ( pSourceTuv == NULL )
        {
          // no TUV matching the memory source language

          // add one invalid segment
          fillSegmentInfo( pTuvArray, NULL, &(pBuf->SegmentData) );
          pBuf->SegmentData.fValid = FALSE;
          strcpy( pBuf->SegmentData.szReason, "No unit matching the memory source language" );
          pfnInsertSegment( lMemHandle, &(pBuf->SegmentData) );
        }
        else
        {
          iCurrent = 0;
          pCurrentTuv = pTuvArray;
          while ( iCurrent < iCurTuv )
          {
            BOOL fSkipTuv = FALSE;

            if ( pCurrentTuv == pSourceTuv )
            {
              fSkipTuv = TRUE; // skip this TUV as it contains the source text
            } /* endif */

            // GQ 2016-05-13 (fix for P403308): for empty TUVS only: check if there is another TUV for the same language and which contains some data and skip empty tuv in such a case
            if ( !fSkipTuv && (wcslen( pCurrentTuv->szText ) == 0) )
            {
              PTMXTUV pTestTuv = pCurrentTuv + 1;
              int iTestTuv = iCurrent + 1;
              while ( !fSkipTuv && (iTestTuv < iCurTuv) )
              {
                if ( (strcmp( pTestTuv->szLang, pCurrentTuv->szLang ) == 0) && (wcslen( pTestTuv->szText ) != 0) )
                {
                  fSkipTuv = TRUE; // skip this TUV as there is another tuv for the same language having text information
                } /* endif */
                iTestTuv++;
                pTestTuv++;
              } /* endwhile */
            } /* endif */

            if ( !fSkipTuv )
            {
              // do any tag removal if requested
              if ( m_pMemInfo->fCleanRTF )
              {
                RemoveRTFTags( pCurrentTuv->szText, m_pMemInfo->fTagsInCurlyBracesOnly );
                RemoveRTFTags( pSourceTuv->szText, m_pMemInfo->fTagsInCurlyBracesOnly );
              } /* endif */

              int iLenCurrent = wcslen( pCurrentTuv->szText );
              int iLenSource  = wcslen( pSourceTuv->szText );

              fillSegmentInfo( pSourceTuv, pCurrentTuv, &(pBuf->SegmentData) );


              if ( pCurrentTuv->fInvalidLang )
              {
                pBuf->SegmentData.fValid = FALSE;
                sprintf( pBuf->SegmentData.szReason, "The language \"%s\" is not supported by OpenTM2.", pCurrentTuv->szLang );
              }
              else if ( (iLenCurrent + 1) >= MAX_SEGMENT_SIZE)
              {
                pBuf->SegmentData.fValid = FALSE;
                sprintf( pBuf->SegmentData.szReason, "Segment too large. Length of target text is %ld characters.", iLenCurrent );
              }
              else if ( (iLenSource + 1) >= MAX_SEGMENT_SIZE )
              {
                pBuf->SegmentData.fValid = FALSE;
                sprintf( pBuf->SegmentData.szReason, "Segment too large. Length of source text is %ld characters.", iLenSource );
              }
              else if ( pSourceTuv->fInvalidChars )
              {
                pBuf->SegmentData.fValid = FALSE;
                sprintf( pBuf->SegmentData.szReason, "Segment source contains invalid characters." );
              }
              else if ( pCurrentTuv->fInvalidChars )
              {
                pBuf->SegmentData.fValid = FALSE;
                sprintf( pBuf->SegmentData.szReason, "Segment target contains invalid characters." );
              }
              else
              {
                pBuf->SegmentData.fValid = TRUE;
              } /* endif */

              // add segment to memory (if fValid set) or count as skipped segment
              if ( pfnInsertSegment != NULL )
              {
                if ( pBuf->SegmentData.fValid )
                {
                  if ( pBuf->SegmentData.szFormat[0] != EOS ) 
                  {
                    // markup table information is already available
                    pfnInsertSegment( lMemHandle, &(pBuf->SegmentData) );
                  }
                  else if ( m_pMemInfo->pszMarkupList != NULL )
                  {
                    if ( pSourceTuv->fInlineTags )
                    {
                      // insert segment data for all specified markups
                      PSZ pszCurrentMarkup = m_pMemInfo->pszMarkupList;
                      while ( *pszCurrentMarkup )
                      {
                        strcpy( pBuf->SegmentData.szFormat, pszCurrentMarkup );
                        pfnInsertSegment( lMemHandle, &(pBuf->SegmentData) );
                        pszCurrentMarkup += strlen(pszCurrentMarkup) + 1;
                      } /* endwhile */                       
                    }
                    else
                    {
                      // segment contains no inline tagging so add it only once using the first markup of the list
                      strcpy( pBuf->SegmentData.szFormat, m_pMemInfo->pszMarkupList );
                      pfnInsertSegment( lMemHandle, &(pBuf->SegmentData) );
                    } /* endif */                       
                  }
                  else if ( m_pMemInfo->szFormat[0] != EOS )
                  {
                    // use supplied markup table for the segment
                    strcpy( pBuf->SegmentData.szFormat, m_pMemInfo->szFormat );
                    pfnInsertSegment( lMemHandle, &(pBuf->SegmentData) );
                  }
                  else
                  {
                    // use default markup table for the segment
                    strcpy( pBuf->SegmentData.szFormat, "OTMANSI" );
                    pfnInsertSegment( lMemHandle, &(pBuf->SegmentData) );
                  } /* endif */                     
                }
                else
                {
                   if ( pBuf->SegmentData.szFormat[0] == EOS ) strcpy( pBuf->SegmentData.szFormat, "OTMANSI" );
                  pfnInsertSegment( lMemHandle, &(pBuf->SegmentData) );
                } /* endif */
              } /* endif */
            } /* endif */

            // continue with next tuv
            pCurrentTuv++;
            iCurrent++;
          } /*endwhile */
        } /* endif */
      }
      break;


    case TUV_ELEMENT:
      //
      // store collected data in TUV array
      //
      {
        // check if <tuv> contains all required info
        BOOL fTuvValid = TRUE;
        BOOL fTuvLangInvalid = FALSE;
   
        // GQ 2016/01/22 Allow empty data areas in order to allow conversion of memories with empty target segments
        // if ( pBuf->szData[0] == 0 ) fTuvValid = FALSE;          // no data for <tuv>
        if ( CurElement.szTMLanguage[0] == 0 )
        {
          // no Tmgr language available, check TMX language  
          if ( CurElement.szTMXLanguage[0] == 0 )
          {
            // no languag info at all, <tuv> is invalid
            fTuvValid = FALSE;
          }
          else
          {
            // convert TMX language to Tmgr language name
            if ( !FindName( Language_TMX2Tmgr, CurElement.szTMXLanguage, pBuf->szLang, sizeof(pBuf->szLang) ) )
            {
              // language not found, strip language code and try again
              PSZ pszDelim = strchr( CurElement.szTMXLanguage, '-' );
              if ( pszDelim )
              {
                *pszDelim = '\0';
                if ( !FindName( Language_TMX2Tmgr, CurElement.szTMXLanguage, pBuf->szLang, sizeof(pBuf->szLang) ) )
                {
                  fTuvLangInvalid = TRUE;          // language not supported
                } /* endif */
                *pszDelim = '-';
              }
              else
              {
                fTuvLangInvalid = TRUE;          // language not supported
              } /* endif */
            } /* endif */
          } /* endif */
        }
        else
        {
          // use Tmgr language name
          strcpy( pBuf->szLang, CurElement.szTMLanguage );
        } /* endif */
        
        // enlarge array if necessary
        if ( fTuvValid && (iCurTuv >= iTuvArraySize) )
        {
          int iNewArraySize = iCurTuv + 3;
          pTuvArray = (PTMXTUV)realloc( pTuvArray, iNewArraySize * sizeof(TMXTUV) );
          if ( pTuvArray )
          {
            iTuvArraySize = iNewArraySize;
          }
          else
          {
            // out of memory
          } /* endif */
        } /* endif */

        // store TUV data
        if ( fTuvValid )
        {
          PTMXTUV pTuvData = pTuvArray + iCurTuv;

          wcscpy( pTuvData->szText, pBuf->szData );
          strcpy( pTuvData->szLang, pBuf->szLang );
          pTuvData->fInvalidChars = this->fInvalidChars;
          pTuvData->fInlineTags = this->fInlineTags;
          pTuvData->fInvalidLang = fTuvLangInvalid;
          if ( fTuvLangInvalid )
          {
            // remember specified ISO language
            strcpy( pTuvData->szLang, CurElement.szTMXLanguage );
          } /* endif */

          iCurTuv++;
        } /* endif */
      }
      break;

    case BODY_ELEMENT:
      break;

    case SEG_ELEMENT:
      // data is complete, stop data catching
      CurElement.fInlineTagging = FALSE;
      fCatchData = FALSE; 
      break;
    case BPT_ELEMENT:
    case EPT_ELEMENT:
    case PH_ELEMENT:
    case HI_ELEMENT:
    case IT_ELEMENT:
    case SUB_ELEMENT:
    case UT_ELEMENT:
      {
        CurElement.fInsideTagging = FALSE;
        // close self contained tag or add end tag
        if ( this->fTMXTagStarted )
        {
          // close self contained inline tag
          int iCurLen = wcslen( pBuf->szData );
          if ( (iCurLen + 4) < DATABUFFERSIZE)
          {
            pBuf->szData[iCurLen++] = L' ';
            pBuf->szData[iCurLen++] = L'/';
            pBuf->szData[iCurLen++] = L'>';
            pBuf->szData[iCurLen++] = 0;
          } /* endif */
          this->fTMXTagStarted = FALSE;
        }
        else
        {
          // add end tag to current text buffer 
          int iCurLen = wcslen( pBuf->szData );
          int iTagLen = wcslen( pszName );
          if ( (iCurLen + iTagLen + 3) < DATABUFFERSIZE)
          {
            pBuf->szData[iCurLen++] = L'<';
            pBuf->szData[iCurLen++] = L'/';
            wcsncpy( pBuf->szData + iCurLen, pszName, iTagLen );
            iCurLen += iTagLen;
            pBuf->szData[iCurLen++] = L'>';
            pBuf->szData[iCurLen++] = 0;
          } /* endif */
        } /* endif */
        this->fWithTagging = FALSE;
      }
      break;
    case INVCHAR_ELEMENT:
      break;
    case UNKNOWN_ELEMENT:
    default:
      break;
  } /*endswitch */

  Pop( &CurElement );

  // for prop elements we have to set the data in the parent element (i.e. after the element has been removed from the stack)
  if ( CurrentID == PROP_ELEMENT )
  {
    if ( CurrentProp == TMLANGUAGE_PROP )
    {
      memset( CurElement.szTMLanguage, 0, sizeof(CurElement.szTMLanguage) );
      strncpy( CurElement.szTMLanguage, pBuf->szProp, sizeof(CurElement.szTMLanguage) - 1 );
    }
    else if ( CurrentProp == TMMARKUP_PROP )
    {
      memset( CurElement.szTMMarkup, 0, sizeof(CurElement.szTMMarkup) );
      strncpy( CurElement.szTMMarkup, pBuf->szProp, sizeof(CurElement.szTMMarkup) - 1 );
    }
    else if ( CurrentProp == TMDOCNAME_PROP )
    {
      memset( pBuf->szDocument, 0, sizeof(pBuf->szDocument) );
      strncpy( pBuf->szDocument, pBuf->szProp, sizeof(pBuf->szDocument) - 1 );
    }
    else if ( CurrentProp == TMDESCRIPTION_PROP )
    {
      memset( pBuf->szDescription, 0, sizeof(pBuf->szDescription) );
      strncpy( pBuf->szDescription, pBuf->szProp, sizeof(pBuf->szDescription) - 1 );
    }
    else if ( CurrentProp == MACHINEFLAG_PROP )
    {
       usTranslationFlag = TRANSLFLAG_MACHINE;
    }
    else if ( CurrentProp == TRANSLATIONFLAG_PROP )
    {
      if ( pBuf->szProp[0] == '1' )
      {
        usTranslationFlag = TRANSLFLAG_MACHINE;
      }
      else if ( pBuf->szProp[0] == '2' )
      {
        usTranslationFlag = TRANSLFLAG_GLOBMEM;
      } /* end */         
    }
    else if ( CurrentProp == TMNOTE_PROP )
    {
      wcscpy( pBuf->szNote, pBuf->szPropW );
    }
    else if ( CurrentProp == TMNOTESTYLE_PROP )
    {
      wcscpy( pBuf->szNoteStyle, pBuf->szPropW );
    }
    else if ( CurrentProp == TMTMMATCHTYPE_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, TMMATCHTYPE_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMMTSERVICE_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, MTSERVICE_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMMTMETRICNAME_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, MTMETRICNAME_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMMTMETRICVALUE_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, MTMETRICVALUE_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMPEEDITDISTANCECHARS_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, PEEDITDISTANCECHARS_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMPEEDITDISTANCEWORDS_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, PEEDITDISTANCEWORDS_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMMTFIELDS_PROP )
    {
      wcscat( pBuf->szMTMetrics, L" " );
      wcscat( pBuf->szMTMetrics, MTFIELDS_PROP_W );
      wcscat( pBuf->szMTMetrics, L"=\"" );
      wcscat( pBuf->szMTMetrics, pBuf->szPropW );
      wcscat( pBuf->szMTMetrics, L"\"" );
    }
    else if ( CurrentProp == TMWORDS_PROP )
    {
      pBuf->ulWords = atol( pBuf->szProp );
    }
    else if ( CurrentProp == TMMATCHSEGID_PROP )
    {
      wcscpy( pBuf->szMatchSegID, pBuf->szPropW );
    }
    else if ( CurrentProp == SEG_PROP )
    {
      CurElement.lSegNum = atol( pBuf->szProp );
    } /* endif */
  } /* endif */
}

void TMXParseHandler::characters(const XMLCh* const chars, const XMLSize_t length)
{
  PSZ_W pszChars = (PSZ_W)chars;
  int iLength = length;

  if ( this->fTMXTagStarted )
  {
    // close open TMX inline tag
    int iCurLen = wcslen( pBuf->szData );
    if ( (iCurLen + 2) < DATABUFFERSIZE)
    {
      pBuf->szData[iCurLen++] = '>';
      pBuf->szData[iCurLen] = 0;
    } /* endif */
    this->fTMXTagStarted = FALSE;
  } /* endif */

  if ( hfLog ) fprintf( hfLog, "%ld Characters; \"%S\"\n", length, pszChars );

  if ( (CurElement.ID == PROP_ELEMENT) && (CurElement.PropID != UNKNOWN_PROP) )
  {
    // append data in prop value buffer
    int iCurLength = wcslen( pBuf->szPropW );
    int iFree = (sizeof(pBuf->szPropW) / sizeof(CHAR_W)) - iCurLength - 1;
    if ( iLength > iFree ) iLength = iFree;

    wcsncpy( pBuf->szPropW + iCurLength, pszChars, iLength );
    pBuf->szPropW[iCurLength+iLength] = 0;

    int iBytes = WideCharToMultiByte( CP_OEMCP, 0, pBuf->szPropW, -1, pBuf->szProp, sizeof(pBuf->szProp), NULL, NULL );
    pBuf->szProp[iBytes] = '\0';
  }
  else if ( fCatchData && (CurElement.ID == INVCHAR_ELEMENT) )
  {
    // add invalid char
    int iCurLen = wcslen( pBuf->szData );
    if ( (iCurLen + 1 + 1) < DATABUFFERSIZE)
    {
      LONG lChar = _wtol( pszChars );
      if ( lChar != 0 )
      {
        USHORT usChar = (USHORT)lChar;
        pBuf->szData[iCurLen] = usChar;
        pBuf->szData[iCurLen+1] = 0;
      } /* endif */
    } /* endif */
  }
  else if ( fCatchData && (fWithTagging || !CurElement.fInlineTagging) )
  {
    // add data to current data buffer 
    int iCurLen = wcslen( pBuf->szData );
    if ( (iCurLen + length + 1) < DATABUFFERSIZE)
    {
      if ( CurElement.fInsideTagging )
      {
        // escape any special characters in inline tags
        EscapeXMLChars( pszChars, pBuf->szData + iCurLen );
      }
      else
      {
        wcsncpy( pBuf->szData + iCurLen, pszChars, length );
        pBuf->szData[iCurLen+length] = 0;
      }
    } /* endif */
    if ( CurElement.fInlineTagging ) this->fInlineTags = TRUE;
  } /* endif */
}

void TMXParseHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    if ( hfLog ) fprintf( hfLog, "Fatal Error: %s at column %ld in line %ld\n", message, col, line );
    this->fError = TRUE;
    sprintf( this->pBuf->szErrorMessage, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    XMLString::release( &message );
}

void TMXParseHandler::error(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    if ( hfLog ) fprintf( hfLog, "Error: %s at column %ld in line %ld\n", message, col, line  );
    this->fError = TRUE;
    sprintf( this->pBuf->szErrorMessage, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    XMLString::release( &message );
}

void TMXParseHandler::warning(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    if ( hfLog ) fprintf( hfLog, "Warning: %s at column %ld in line %ld\n", message, col, line  );
    XMLString::release( &message );
}


// get the ID for a TMX element
ELEMENTID TMXParseHandler::GetElementID( PSZ_W pszName )
{
  int i = 0;
  ELEMENTID IDFound = UNKNOWN_ELEMENT;

  while ( (IDFound == UNKNOWN_ELEMENT) && (TmxNameToID[i].szName[0] != 0) )
  {
    if ( _wcsicmp( pszName, TmxNameToID[i].szName ) == 0 )
    {
      IDFound = TmxNameToID[i].ID;
    }
    else
    {
      i++;
    } /* endif */
  } /*endwhile */
  return( IDFound );
} /* end of method TMXParseHandler::GetElementID */

void TMXParseHandler::Push( PTMXELEMENT pElement )
{
  // enlarge stack if necessary
  if ( iCurElement >= iStackSize )
  {
    pStack = (PTMXELEMENT)realloc( pStack, (iStackSize + 5) * sizeof(TMXELEMENT) );
    iStackSize += 5;
  } /* endif */

  // add element to stack
  if ( pStack )
  {
    memcpy( pStack + iCurElement, pElement, sizeof(TMXELEMENT) );
    iCurElement++;
  } /* endif */

  return;
} /* end of method TMXParseHandler::Push */

void TMXParseHandler::Pop( PTMXELEMENT pElement )
{
  if ( pStack && iCurElement )
  {
    iCurElement--;
    memcpy( pElement, pStack + iCurElement, sizeof(TMXELEMENT) );
  } /* endif */
  return;
} /* end of method TMXParseHandler::Pop */


void TMXParseHandler::SetMemInfo( PMEMEXPIMPINFO pMemInfo)
{
  this->m_pMemInfo = pMemInfo;
} /* end of method TMXParseHandler::SetNameLists */

// extract a numeric value from a string
BOOL TMXParseHandler::GetValue( PSZ pszString, int iLen, int *piResult )
{
  BOOL fOK = TRUE;
  char szNumber[10];
  char *pszNumber = szNumber;

  *piResult = 0;

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
    *pszNumber = '\0';
    *piResult = atoi( szNumber );
  } /* endif */

  return( fOK );
} /* end of method TMXParseHandler::GeValue */

// convert a TMX language name to an TM language name
BOOL TMXParseHandler::TMXLanguage2TMLanguage( PSZ pszTMLanguage, PSZ pszTMXLanguage, PSZ pszResultingLanguage )
{
  BOOL fOK = TRUE;

  // preset target buffer
  strcpy( pszResultingLanguage, OTHERLANGUAGES );

  if ( *pszTMLanguage )
  {
    strcpy( pszResultingLanguage, pszTMLanguage );
  }
  else if ( *pszTMXLanguage )
  {
    if ( !FindName( Language_TMX2Tmgr, pszTMXLanguage, pszResultingLanguage, 50 ) )
    {
      // try first part of language only
      PSZ pszDelim = strchr( pszTMXLanguage, '-' );
      if ( pszDelim )
      {
        *pszDelim = '\0';
        FindName( Language_TMX2Tmgr, pszTMXLanguage, pszResultingLanguage, 50 );
        *pszDelim = '-';
      } /* endif */
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of method TMXParseHandler::GeValue */

void TMXParseHandler::SetMemInterface( PFN_MEMINSERTSEGMENT pfnInsert, LONG lHandle, PLOADEDTABLE pTable, PTOKENENTRY pBuf, int iSize )
{
  pBuf;

  pfnInsertSegment = pfnInsert;
  lMemHandle = lHandle;
  pRTFTable = pTable;
  pTokBuf = pTokBuf;
  iTokBufSize = iSize;
} /* end of method TMXParseHandler::SetMemInterface */

void TMXParseHandler::GetDescription( char *pszDescription, int iBufSize )
{
  *pszDescription = 0;

  if ( pBuf && pBuf->szDescription[0] )
  {
    strncpy( pszDescription, pBuf->szDescription, iBufSize );
    pszDescription[iBufSize-1] = 0;
  } /* endif */
} /* end of method TMXParseHandler::GetDescription */

void TMXParseHandler::SetSourceLanguage( char *pszSourceLang)
{
  if ( pBuf )
  {
    strcpy( pBuf->szMemSourceLang, pszSourceLang);
  } /* endif */
} /* end of method TMXParseHandler::SetSourceLanguage */

void TMXParseHandler::GetSourceLanguage( char *pszSourceLang, int iBufSize )
{
  *pszSourceLang = 0;

  if ( pBuf && pBuf->szMemSourceLang[0] )
  {
    strncpy( pszSourceLang, pBuf->szMemSourceLang, iBufSize );
    pszSourceLang[iBufSize-1] = 0;
  } /* endif */
} /* end of method TMXParseHandler::GetSourceLanguage */

BOOL TMXParseHandler::IsHeaderDone()
{
 return( fHeaderDone );
}

// help functions for date conversion
BOOL TMXParseHandler::IsLeapYear( const int iYear )
{
  if ((iYear % 400) == 0)
    return true;
  else if ((iYear % 100) == 0)
    return false;
  else if ((iYear % 4) == 0)
    return true;
  return false;
}                   

int TMXParseHandler::GetDaysOfMonth( const int iMonth, const int iYear )
{
  int aiDaysOfMonth[13] = {  0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

  if (iMonth == 2)
  {
    if ( IsLeapYear( iYear ) )
      return 29;
    else
      return 28;
  }
  if (( iMonth >= 1) && (iMonth <= 12))
    return aiDaysOfMonth[iMonth];
  else
  {
    return 0;
  }
}                   

int TMXParseHandler::GetDaysOfYear( const int iYear )
{
  if ( IsLeapYear( iYear ) )
  {
    return 366;
  }
  else
  {
    return 365;
  } /* endif */
}                   

int TMXParseHandler::GetYearDay( const int iDay, const int iMonth, const int iYear )
{
  int iLocalDay = iDay;
  int iLocalMonth = iMonth;

  while ( iLocalMonth > 1)
  {
    iLocalMonth--;
    iLocalDay += GetDaysOfMonth( iLocalMonth, iYear );
  }
  return iLocalDay ;
}                   

BOOL TMXParseHandler::ErrorOccured( void )
{
  return( this->fError );
}

void TMXParseHandler::GetErrorText( char *pszTextBuffer, int iBufSize )
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

// remove RTFtags from string
//USHORT TMXParseHandler::RemoveRTFTags( PSZ_W pszString )
//{
//  USHORT usRC = 0;
//  PSTARTSTOP pStartStop = NULL;
//  PSZ_W  pszOut = pszString;
//
//  usRC = TACreateProtectTableW( pszString, pRTFTable, 0, (PTOKENENTRY)this->pTokBuf, iTokBufSize, 
//                                &pStartStop, pRTFTable->pfnProtTable, pRTFTable->pfnProtTableW, 
//                                ulCP );
//  if ( !usRC )
//  {
//      PSTARTSTOP pEntry = pStartStop;
//      while ( (pEntry->usStart != 0) || (pEntry->usStop != 0)  || (pEntry->usType != 0) )
//      {
//        if ( pEntry->usType == UNPROTECTED_CHAR )
//        {
//          for( int i = pEntry->usStart; i <= pEntry->usStop; i++ )
//          {
//            *pszOut++ = pszString[i];;
//          } /* endfor */
//        } /* endif */
//        pEntry++;
//      } /* endwhile */
//      *pszOut = 0;
//  } /* endif */
//  UtlAlloc( (PVOID *)&pStartStop, 0L, 0L, NOMSG );
//
//  return( usRC );
//} /* end of function RemoveRTFTags */
USHORT TMXParseHandler::RemoveRTFTags( PSZ_W pszString, BOOL fTagsInCurlyBracesOnly  )
{
  USHORT usRC = 0;
  PSZ_W  pszOut = pszString;
  BOOL fTagRemoveMode = !fTagsInCurlyBracesOnly;

  while ( *pszString )
  {
    if ( *pszString == L'\\' )
    {
      if ( *(pszString+1) == L'\\' )
      {
        // encoded backslash, convert to a single one
        pszString++;
        *pszOut++ = *pszString++;
      }
      if ( (*(pszString+1) == L'{') || (*(pszString+1) == L'}') )
      {
        // encoded curly brace, convert to a normal one
        pszString++;
        *pszOut++ = *pszString++;
      }
      else if ( iswalpha(*(pszString+1))  || (*(pszString+1) == L'*') )
      {
        if ( fTagRemoveMode  )
        {
        // skip RTF tag
        pszString++;
        if ( *pszString == L'*' ) pszString++;
        if ( *pszString == L'\\' ) pszString++;
        while ( iswalpha( *pszString ) ) pszString++;
        if ( *pszString == L'-') pszString++;
        while ( iswdigit( *pszString ) ) pszString++;
        if ( *pszString == L' ') pszString++;
      }
      else
      {
        // copy backslash to target
        *pszOut++ = *pszString++;
      } /* endif */
    }
      else
      {
        // copy backslash to target
        *pszOut++ = *pszString++;
      } /* endif */
    }
    else if ( *pszString == L'{' )
    {
      // skip curly brace, but leave parameters ( {n} ) as-is
      if ( (*pszString == L'{') && iswdigit(pszString[1]) && (pszString[2] == L'}')   )
      {
        *pszOut++ = *pszString++;
        *pszOut++ = *pszString++;
        *pszOut++ = *pszString++;
      }
      else
      {
        if ( fTagsInCurlyBracesOnly )
        {
          // check if curly brace is followed by RTF tags
          PSZ_W pszTest = pszString + 1;
          while (*pszTest == L' ') pszTest++;
          if ( (*pszTest == L'\\') && (iswalpha(*(pszTest+1))  || (*(pszTest+1) == L'*') ) )
          {
            // skip curly brace and start tag remove mode
            pszString++;
            fTagRemoveMode = TRUE;
          }
          else
          {
            // leave curly brace as-is
            *pszOut++ = *pszString++;
          } /* endif */           
        }
        else
        {
          // skip curly brace
          pszString++;
        } /* endif */           
      } /* endif */         
    }
    else if ( *pszString == L'}' )
    {
      if ( fTagRemoveMode  )
      {
        // skip closing curly brace
        pszString++;
      }
      else
      {
        // copy closing curly brace to target
        *pszOut++ = *pszString++;
      } /* endif */         
      if ( fTagsInCurlyBracesOnly )
      {
        fTagRemoveMode = FALSE;
      } /* endif */         
    }
    else
    {
      *pszOut++ = *pszString++;
    } /* endif */
  }/* endwhile */
  *pszOut = 0;
  return( usRC );
} /* end of function RemoveRTFTags */


//+----------------------------------------------------------------------------+
//| EXP2TMX and TMX2EXP converter                                              |
//+----------------------------------------------------------------------------+
#define BUF_SIZE 8096
#define TMXTOKBUFSIZE 32000

typedef struct _CONVERTERDATA
{
  MEMEXPIMPSEG     Segment;                      // segment data buffer
  MEMEXPIMPINFO    MemInfo;                      // mem import export data area
  FILE             *hfOut;                       // output file handle
  CHAR_W           szBufferW[16200];             // buffer for UTF6 strings
  CHAR             szASCIIBuffer[16200];         // buffer for UTF6 to ANSI/ASCII conversions
  BOOL             fHeaderWritten;               //TRUE = header has written to the memory
  LONG             lSegCounter;                  // valid segment counter
  LONG             lSkippedSegsCounter;          // invalid segment counter
  CHAR             szLogFile[MAX_LONGFILESPEC];  // buffer for log file name
  FILE             *hfLog;                       // log file handle

  //-- data for input memory processing --
  CHAR        szInSpec[MAX_LONGFILESPEC];        // input memory specification of user
  CHAR        szInputFile[MAX_LONGFILESPEC];     // buffer for preperation of input memory name
  CHAR        szInMemory[MAX_LONGFILESPEC];      // fully qualified name current input memory
  CHAR        szBuffer[4096];                    // general purpose buffer
  BOOL        fUnicode;                          // TRUE = input memory is in UTF-16 format
  FILE        *hInFile;                          // handle of input file
  CHAR_W      szLine[4096];                      // buffer for input line
  CHAR_W      szSegBuffer[32000];                // buffer for EXP segment (from <segment ..> up to </segment>)
  CHAR_W      chInBufW[BUF_SIZE];                // data buffer for read of Unicode data
  CHAR        chInBuf[BUF_SIZE];                 // data buffer for read of ANSI data
  int         iInBufProcessed;                   // number of processed characters in chInBuf
  int         iInBufRead;                        // number characters read into chInBuf
  ULONG       ulInputCP;                         // codepage to use for import when importing non-Unicode memory
  BOOL        fAnsi;                             // TRUE = input in ASCII format / FALSE = input in ASCII mode
  CHAR        szInMode[200];                     // buffer for input mode
  CHAR_W      szDescription[1024];               // buffer for memory description
  int         iLineNum;                          // current line number
  int         iSegBufferUsed;                    // number of characters used in szSegBuffer
  int         iSegBufferFree;                    // number of character free in szSegBuffer
  BOOL        fSegBufferFull;                    // TRUE = the segment buffer is full

  //-- data for output memory processing --
  CHAR        szOutMemory[MAX_LONGFILESPEC];     // fully qualified name of current output memory
  BOOL        fUTF16Output;                      // TRUE = UTF16, FALSE = UTF8 

  //-- segment data --
  CHAR_W      szSegStart[1024];                  // buffer for segment start string
  CHAR_W      szControl[1024];                   // buffer for control string
  CHAR        szControlAscii[1024];              // ASCII version of control string
  USHORT      usTranslationFlag;                 // type of translation flag
  CHAR_W      szDataType[100];                   // datatype of segment

  // - statistics --
  ULONG       ulSegments;                        // number of segments written to output memory

  // -- other stuff --
  //PLOADEDTABLE pLoadedTable;                     // pointer to currently loaded markup table
  //PTOKENENTRY  pTokBuf;                          // buffer for TaTagTokenize tokens
  //CHAR_W       szActiveTagTable[100];            // buffer for name of loaded tag table
  CHAR           szErrorText[2048];                // error message text in case of errors
  //BOOL         fLog;                             // true = write messages to log file
} CONVERTERDATA, *PCONVERTERDATA;

USHORT WriteStringToMemory( PCONVERTERDATA pData, PSZ_W pszString );
USHORT WriteMemHeader( PCONVERTERDATA pData);
USHORT WriteMemFooter( PCONVERTERDATA pData );

USHORT CheckOutputName( PCONVERTERDATA pData, PSZ pszInMemory, PSZ pszOutMemory );
USHORT CheckExistence( PCONVERTERDATA pData, PSZ pszInMemory );
USHORT GetEncoding( PCONVERTERDATA pData, PSZ pszInMemory, PSZ pszInMode );
USHORT GetEncodingEx( PCONVERTERDATA pData, PSZ pszInMemory, PSZ pszInMode, PSZ pszSourceLanguage );
USHORT GetNextSegment( PCONVERTERDATA pData, PBOOL pfSegmentAvailable );
USHORT CloseInput( PCONVERTERDATA pData );
USHORT FillBufferW( PCONVERTERDATA pData );
USHORT FillBuffer( PCONVERTERDATA pData );
USHORT ReadLineW( PCONVERTERDATA pData, PSZ_W pszLine, int iSize );
USHORT ReadLine( PCONVERTERDATA pData, PSZ pszLine, int iSize );
USHORT GetLine( PCONVERTERDATA pData, PSZ_W pszLine, int iSize, BOOL fUnicode, PBOOL pfEOF );
PSZ_W ParseX15W( PSZ_W pszX15String, SHORT sStringId );
BOOL StripTag( PSZ_W pszLine, PSZ_W pszTag );



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

// helper function: is character a list delimiter or space
static BOOL isDelimiter( CHAR chTest )
{
 return ( (chTest == '(') || (chTest == ')') || (chTest == ',') || (chTest == ' ') || (chTest == EOS)  );
}

// TMX to EXP converter
USHORT TMXTOEXP
( 
  PSZ              pszInMem,            // fully qualified name of input (TMX) memory
  PSZ              pszOutMem,           // fully qualified name of output (EXP) memory
  LONG             lOutMode,            // output mode ANSI/ASCII/UTF16
  LONG             lOptions,            // other processing options
  PSZ              pszMarkup,           // markup to use for segments
  PLONG            plValidSegs,         // number of segments written to EXP file
  PLONG            plSkippedSegs,       // number of skipped segments
  PSZ              pszErrorMsgBuffer,   // buffer for error message texts,
  int              iErrorMsgBufferSize  // size of buffer for error message texts
)
{
  USHORT           usRC = 0;           // function return code
  LONG             lHandle = 0;        // import handle
  LONG             lProgress = 0;
  PCONVERTERDATA   pData = NULL;

  if ( !UtlAlloc( (PVOID *)&pData, 0, sizeof(CONVERTERDATA), ERROR_STORAGE  ) )
  {
    usRC = ERROR_STORAGE;      
  } /* endif */

  if ( !usRC )
  {
    pData->MemInfo.lOutMode = lOutMode;
    if ( lOptions & CLEANRTF_OPT )
    {
      pData->MemInfo.fCleanRTF = TRUE;
    } /* endif */
    if ( lOptions & INCURLYBRACE_OPT )
    {
      pData->MemInfo.fTagsInCurlyBracesOnly = TRUE;
    } /* endif */
    if ( lOptions & SOURCESOURCEMEM_OPT )
    {
      pData->MemInfo.fSourceSource = TRUE;
    } /* endif */

    if ( pszMarkup ) 
    {
      // check if a list of markup tables has been specified
      int iLen = strlen(pszMarkup);
      if ( (*pszMarkup == '(') || (iLen  >= sizeof(pData->MemInfo.szFormat)) || (strchr( pszMarkup, ',' ) != NULL) )
      {
        // prepare list of markup tables

        // allocate buffer for markup table list
        if ( !UtlAlloc( (PVOID *)&(pData->MemInfo.pszMarkupList), 0, iLen + 10, ERROR_STORAGE  ) )
        {
          usRC = ERROR_STORAGE;      
        } /* endif */

        // setup markup table list
        if ( !usRC )
        {
          PSZ pszSource = pszMarkup;
          PSZ pszTarget = pData->MemInfo.pszMarkupList;
          while ( *pszSource != EOS )
          {
            if ( isDelimiter(*pszSource) )
            {
              // skip delimiters or space
              if ( *pszSource != EOS )pszSource++;
            }
            else
            {
              // copy markup table name up to next delimiter to list
              while ( !isDelimiter(*pszSource) )
              {
                *pszTarget++ = *pszSource++;
              } /* endwhile */             
              *pszTarget++ = EOS;
            } /* endif */               
          } /* endwhile */             
          *pszTarget++ = EOS;
        } /* endif */           
      }
      else
      {
        // a single markup has been specified
        strcpy( pData->MemInfo.szFormat, pszMarkup );
      } /* endif */         
    } /* endif */
      

    pData->hfOut = fopen( pszOutMem, "wb" );
    if ( pData->hfOut == NULL )
    {
      usRC = ERROR_WRITE_FAULT;
    }
    else
    {
        fwrite( "\xFF\xFE", 2, 1, pData->hfOut );
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
  }
  else if ( lHandle && (pszErrorMsgBuffer != NULL) )
  {
    EXTMEMIMPORTGETLASTERROR( lHandle, pszErrorMsgBuffer, iErrorMsgBufferSize );
    if ( pData->hfLog )
    {
      fprintf( pData->hfLog, "Error %u while converting memory from TMX to EXP, the error msg is \"%s\"\n", usRC, pszErrorMsgBuffer );
    } /* endif */

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
} /* end of function TMXTOEXP */

// EXP to TMX converter
USHORT EXPTOTMX
( 
  PSZ              pszInMem,            // fully qualified name of input (EXP) memory
  PSZ              pszOutMem,           // fully qualified name of output (TMX) memory
  PSZ              pszInMode,           // input mode "ANSI"/"ASCII"/"UTF16" or a codepage number
  LONG             lOutMode,            // output mode UTF16/UTF8
  PLONG            plSegs,              // number of written (=valid) segments
  PSZ              pszMsgBuffer,        // buffer for error messages
  PLONG            plInvalidSegs,       // number of skipped segments
  FILE             *hfLog               // handle of log file or NULL if no logging of invalid segments is to be done
)
{
  USHORT           usRC = 0;           // function return code
  LONG             lHandle = 0;        // import handle
  PCONVERTERDATA   pData = NULL;
  BOOL             fSegmentAvailable = FALSE;
  BOOL             fEOFReached = FALSE;

  if ( !UtlAlloc( (PVOID *)&pData, 0, sizeof(CONVERTERDATA), ERROR_STORAGE  ) )
  {
    usRC = ERROR_STORAGE;      
  } /* endif */

  // check existence of input file
  if ( usRC == NO_ERROR) usRC = CheckExistence( pData, pszInMem );

  // get input memory encoding
  if ( usRC == NO_ERROR) usRC = GetEncodingEx( pData, pszInMem, pszInMode, pData->MemInfo.szSourceLang );

    // prepare output memory
  if ( usRC == NO_ERROR) 
  {
    pData->hfLog = hfLog;
    pData->MemInfo.fUTF16 = (lOutMode & TMX_UTF16_OPT) != 0;
    pData->MemInfo.fNoCRLF = (lOutMode & TMX_NOCRLF_OPT) != 0;
    WideCharToMultiByte( CP_ACP, 0, pData->szDescription, -1, pData->MemInfo.szDescription, sizeof(pData->MemInfo.szDescription), NULL, NULL );

    usRC = EXTMEMEXPORTSTART( &lHandle, pszOutMem, &(pData->MemInfo) );
  } /* endif */

  // until not end of input memory...
  fSegmentAvailable = TRUE;
  while ( (usRC == NO_ERROR) && !fEOFReached )
  {
    // extract next segment from input memory
    usRC = GetNextSegment( pData, &fSegmentAvailable );

    // handle any error in input data
    if ( usRC != NO_ERROR )
    {
      usRC = 0;
      pData->lSkippedSegsCounter++;
    }
    else if ( !fSegmentAvailable )
    {
      // no error, but no segment available ==> we reached the end of the input file
      fEOFReached = TRUE;
    } /* endif */

    // write segment to output memory
    if ( fSegmentAvailable )
    {
      usRC = EXTMEMEXPORTPROCESS( lHandle, &(pData->Segment) );
      if ( usRC == NO_ERROR)
      {
        pData->ulSegments++;
      } /* endif */
    } /* endif */
  } /*endwhile */

  // close input memory
  CloseInput( pData );

  if ( lHandle ) EXTMEMEXPORTEND( lHandle );

  if ( plSegs ) *plSegs = (LONG)pData->ulSegments;
  if ( plInvalidSegs ) *plInvalidSegs = (LONG)pData->lSkippedSegsCounter;

  // pass any error message text to caller in case of errors
  if ( usRC && (pData->szErrorText[0] != EOS) )
  {
    strcpy( pszMsgBuffer, pData->szErrorText );
  } /* endif */

  if ( pData ) UtlAlloc( (PVOID *)&pData, 0, 0, NOMSG );


  return( usRC );
} /* end of function EXPTOTMX */


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
    WriteStringToMemory( pData, L"<Codepage>" );
    if ( pData->MemInfo.lOutMode & ANSI_OPT )
    {
      CHAR_W szCP[14];
      ULONG ulCP = GetLangAnsiCP( NULL );
      swprintf( szCP, L"ANSI=&lu", ulCP );
      WriteStringToMemory( pData, szCP );
    }
    else if ( pData->MemInfo.lOutMode & ASCII_OPT )
    {
      CHAR_W szCP[14];
      ULONG ulCP = GetLangOEMCP( NULL );
      swprintf( szCP, L"ASCII=&lu", ulCP );
      WriteStringToMemory( pData, szCP );
    }
    else
    {
      WriteStringToMemory( pData, L"UTF16" );
    } /* endif */
    WriteStringToMemory( pData, L"</Codepage>\r\n" );
    pData->fHeaderWritten = TRUE;
  } /* endif */
  return( 0 );
} /* end of function WriteMemHeader */

USHORT WriteMemFooter( PCONVERTERDATA pData )
{
  WriteStringToMemory( pData, L"</NTMMemoryDb>\r\n" );
  return( 0 );
} /* end of function WriteMemHeader */


// check output memory name 
USHORT CheckOutputName( PCONVERTERDATA pData, PSZ pszInMemory, PSZ pszOutMemory )
{
  USHORT      usRC = NO_ERROR;         // function return code

  pData;

  if ( *pszOutMemory == EOS )
  {
    PSZ pszExtension;

    strcpy( pData->szOutMemory, pszInMemory );

    pszExtension = strrchr( pData->szOutMemory, '\\' );
    if ( pszExtension == NULL ) pszExtension = pData->szOutMemory;
    pszExtension = strrchr( pszExtension, '.' );
    if ( pszExtension == NULL ) pszExtension = pData->szOutMemory + strlen(pData->szOutMemory);
    strcpy( pszExtension, ".TMX" );
  }
  else
  {
    strcpy( pData->szOutMemory, pszOutMemory );
  } /* endif */

  return( usRC );
} /* end of function */


  // check existence of input file
USHORT CheckExistence( PCONVERTERDATA pData, PSZ pszInMemory )
{
  USHORT      usRC = NO_ERROR;         // function return code
  HDIR hdir;
  static WIN32_FIND_DATA FindData;    // find data structure

  pData;                               // suppress 'unreferenced formal parameter' warning

  hdir = FindFirstFile( pszInMemory, &FindData );
  if ( hdir == INVALID_HANDLE_VALUE )
  {
     usRC = ERROR_FILE_NOT_FOUND;
     sprintf( pData->szErrorText, "Input memory %s could not be opened", pszInMemory );
  }
  else
  {
    FindClose( hdir );
  } /* endif */

  return( usRC );
} /* end of function CheckExistence */

// open input memory and get input memory encoding
USHORT GetEncoding( PCONVERTERDATA pData, PSZ pszInMemory, PSZ pszInMode )
{
  return( GetEncodingEx( pData, pszInMemory, pszInMode, NULL ) );
} /* end of function GetEncoding */

// get memory source language (UTF16 Version)
USHORT GetSourceLangW( PCONVERTERDATA pData, PSZ pszSourceLanguage )
{
  BOOL fWaitingForControlRecord = TRUE;
  fgetws( pData->szLine, sizeof(pData->szLine)/sizeof(CHAR_W), pData->hInFile );
  while ( fWaitingForControlRecord  && !feof(pData->hInFile) )
  {
    if ( wcsnicmp( pData->szLine, L"<control>", 9 ) == 0 )
    {
      // get control data
      fgetws( pData->szLine, sizeof(pData->szLine)/sizeof(CHAR_W), pData->hInFile );

      // retrieve source language
      wcscpy( pData->szBufferW, ParseX15W( pData->szLine, 3 ) );
      WideCharToMultiByte( CP_OEMCP, 0, pData->szBufferW, -1, pszSourceLanguage, MAX_LANG_LENGTH, NULL, NULL );

      fWaitingForControlRecord = FALSE;
    }
    else
    {
      fgetws( pData->szLine, sizeof(pData->szLine)/sizeof(CHAR_W), pData->hInFile );
    } /* endif */               
  } /* endwhile */           
  return( 0 );
} /* end of function GetSourceLangW */

// get memory source language 
USHORT GetSourceLang( PCONVERTERDATA pData, PSZ pszSourceLanguage )
{
  BOOL fWaitingForControlRecord = TRUE;
  fgets( (PSZ)pData->szLine, sizeof(pData->szLine), pData->hInFile );
  while ( fWaitingForControlRecord  && !feof(pData->hInFile) )
  {
    if ( strnicmp( (PSZ)pData->szLine, "<control>", 9 ) == 0 )
    {
      // get control data
      fgets( (PSZ)pData->szLine, sizeof(pData->szLine), pData->hInFile );

      // retrieve source language
      strcpy( pszSourceLanguage, UtlParseX15( (PSZ)pData->szLine, 3 ) );

      fWaitingForControlRecord = FALSE;
    }
    else
    {
      fgets( (PSZ)pData->szLine, sizeof(pData->szLine), pData->hInFile );
    } /* endif */               
  } /* endwhile */           
  return( 0 );
} /* end of function GetSourceLang */



// open input memory and get input memory encoding and source language
USHORT GetEncodingEx( PCONVERTERDATA pData, PSZ pszInMemory, PSZ pszInMode, PSZ pszSourceLanguage )
{
  USHORT      usRC = NO_ERROR;         // function return code

  pszInMode;

  strcpy( pData->szInMemory, pszInMemory );
  pData->hInFile = fopen( pData->szInMemory, "rb" );
  if ( pData->hInFile == NULL )
  {
     usRC = ERROR_FILE_NOT_FOUND;
     // ShowError( pData, "Error: Input memory %s could not be opened", pszInMemory );
  } /* endif */

  // get first bytes of memory and check for UTF-16 encoding
  if ( usRC == NO_ERROR )
  {
    // get first chunk of file and check for BOM and memory tag
    memset( pData->szBuffer, 0, sizeof(pData->szBuffer) );
    fread( pData->szBuffer, 1, sizeof(pData->szBuffer), pData->hInFile );

    if ( memcmp( pData->szBuffer, UNICODEFILEPREFIX, 2 ) == 0 )
    {
      PSZ_W pszTemp = (PSZ_W)pData->szBuffer + 1;
      pData->fUnicode = TRUE;

      if ( _wcsnicmp( pszTemp, L"<ntmmemorydb>", 13 ) != 0 )
      {
        sprintf( pData->szErrorText, "The format of the file %s is not supported", pData->szInMemory );
        usRC = ERROR_INVALID_DATA;
      } /* endif */

      // read ahead up to first control entry
      if ( usRC == 0 )
      {
        fseek( pData->hInFile, 2, SEEK_SET );
        if ( pszSourceLanguage != NULL ) GetSourceLangW( pData, pszSourceLanguage );
      } /* endif */         

      fseek( pData->hInFile, 2, SEEK_SET );

      usRC = FillBufferW( pData );
    }
    else
    {
      PSZ_W pszTemp = (PSZ_W)pData->szBuffer;

      fseek( pData->hInFile, 0, SEEK_SET );

      if ( _wcsnicmp( pszTemp, L"<ntmmemorydb>", 13 ) == 0 )
      {
        pData->fUnicode = TRUE;
        if ( pszSourceLanguage != NULL )
        {
          GetSourceLangW( pData, pszSourceLanguage );
          fseek( pData->hInFile, 0, SEEK_SET );
        } /* endif */         
       usRC = FillBufferW( pData );
      }
      else if ( _strnicmp( pData->szBuffer, "<ntmmemorydb>", 13 ) == 0 )
      {
        pData->fUnicode = FALSE;
        if ( pszSourceLanguage != NULL )
        {
          GetSourceLang( pData, pszSourceLanguage );
          fseek( pData->hInFile, 0, SEEK_SET );
        } /* endif */         
        usRC = FillBuffer( pData );
      }
      else
      {
        sprintf( pData->szErrorText, "The file %s is no memory in valid EXP format", pData->szInMemory );
        usRC = ERROR_INVALID_DATA;
      } /* endif */
    } /* endif */
  } /* endif */

  // read memory header up to first segment and check for codepage tag, store any found memory description
  if ( usRC == NO_ERROR )
  {
    BOOL fEOF = FALSE;
    BOOL fDescription = FALSE;

    pData->szLine[0] = 0;
    pData->szDescription[0] = 0;
    GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );

    while ( (usRC == NO_ERROR) && !fEOF && 
            (_wcsnicmp( pData->szLine, L"<segment>", 9 ) != 0) &&
            (_wcsnicmp( pData->szLine, L"</ntmmemorydb>", 14 ) != 0))
    {
      if ( _wcsnicmp( pData->szLine, L"<description>", 13 ) == 0 )
      {
        fDescription = TRUE;
      }
      else if ( _wcsnicmp( pData->szLine, L"</description>", 14 ) == 0 )
      {
        fDescription = FALSE;
      }
      else if ( _wcsnicmp( pData->szLine, L"<codepage>", 10 ) == 0 )
      {
        StripTag( pData->szLine + 10, L"</codepage>" );
        if ( _wcsnicmp( pData->szLine + 10, L"ANSI=", 5 ) == 0 ) 
        {
          pData->ulInputCP = _wtol( pData->szLine + 15 );
        }
        else if ( _wcsnicmp( pData->szLine + 10, L"ASCII=", 6 ) == 0 ) 
        {
          pData->ulInputCP = _wtol( pData->szLine + 16 );
        } /* endif */
      } 
      else if ( fDescription )
      {
        wcscat( pData->szDescription, pData->szLine );
      } /* endif */

      pData->szLine[0] = 0;
      usRC = GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
    } /*endwhile */
  } /* endif */

  return( usRC );
} /* end of function GetEncodingEx */

// clear the segment buffer
void ClearSegBuffer( PCONVERTERDATA pData )
{
  pData->iSegBufferUsed = 0;
  pData->iSegBufferFree = sizeof(pData->szSegBuffer)/sizeof(CHAR_W);
  pData->fSegBufferFull = FALSE;
}

// add the current line to the segment buffer
void AddLineToSegBuffer( PCONVERTERDATA pData )
{
  if ( !pData->fSegBufferFull )
  {
    int iAddLen = wcslen( pData->szLine );
    if ( (iAddLen + 5) < pData->iSegBufferFree )// always leave room for "..." and CRLF at the end of the buffer
    {
      wcscpy( pData->szSegBuffer + pData->iSegBufferUsed, pData->szLine );
      pData->iSegBufferUsed += iAddLen;
      pData->iSegBufferFree -= iAddLen;

      wcscpy( pData->szSegBuffer + pData->iSegBufferUsed, L"\r\n" );
      pData->iSegBufferUsed += 2;
      pData->iSegBufferFree -= 2;
    }
    else
    {
      wcscpy( pData->szSegBuffer + pData->iSegBufferUsed, L"...\r\n" );
      pData->iSegBufferUsed += 5;
      pData->fSegBufferFull = TRUE;
      pData->iSegBufferFree = 0;
    } /* endif */
  } /* endif */
}

// concatenate segment data and ensure that size limit is not exceeded
USHORT ConcatenateSegData( PSZ_W pszSegment, PSZ_W pszNewData, int iMaxSize)
{
  int iExistingLen = wcslen( pszSegment );
  int iAddLen = wcslen( pszNewData );
  if ( (iExistingLen + iAddLen) >= iMaxSize )
  {
    return( ERROR_INVALID_SEGMENT );
  }

  wcscpy( pszSegment + iExistingLen, pszNewData );
  return( 0 );
}

// extract next segment from input memory
USHORT GetNextSegment( PCONVERTERDATA pData, PBOOL pfSegmentAvailable )
{
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL        fEOF = FALSE;

  BOOL fControlString = FALSE;

  *pfSegmentAvailable = FALSE;

  // skip any lines until the start of a new segment is detected
  while ( !usRC && !fEOF && _wcsnicmp( pData->szLine, L"<segment>", 9 ) != 0 ) 
  {
    GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
  } /* enwhile */

  // read data until segment is complete
  if ( _wcsnicmp( pData->szLine, L"<segment>", 9 ) == 0 ) 
  {
    int iStartLine = pData->iLineNum;
    PSZ pszError = "";

    // copy all lines of the segment to our segment buffer (for logging in cas of errors)
    ClearSegBuffer( pData );
    AddLineToSegBuffer( pData );

    // reset segment data
    pData->Segment.szSource[0] = 0;
    pData->szControl[0] = 0;
    pData->Segment.szTarget[0] = 0;
    pData->Segment.szAddInfo[0] = 0;
    wcscpy( pData->szSegStart, pData->szLine );

    while ( !usRC && !fEOF && _wcsnicmp( pData->szLine, L"</segment>", 10 ) != 0 ) 
    {
      GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
      AddLineToSegBuffer( pData );

      if ( _wcsnicmp( pData->szLine, L"<control>", 9 ) == 0 )
      {
        fControlString = TRUE;

        // get the control string
        GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
        AddLineToSegBuffer( pData );
        wcscpy( pData->szControl, pData->szLine );

        if ( fEOF )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
          pszError = "Incomplete segment control string";
        }
        else if ( _wcsnicmp( pData->szLine, L"</control>", 10 ) == 0 )
        {
          // missing control string
          usRC = ERROR_INVALID_SEGMENT;
          pszError = "Missing control string";
        }
        else
        {
          // split control string into its elements
          pData->Segment.lSegNum = _wtol( ParseX15W( pData->szControl, 0 ) );
          if ( *(ParseX15W( pData->szControl, 1 )) == L'1' )
          {
            pData->Segment.usTranslationFlag = TRANSLFLAG_MACHINE;
          }
          else if ( *(ParseX15W( pData->szControl, 1 )) == L'2' )
          {
            pData->Segment.usTranslationFlag = TRANSLFLAG_GLOBMEM;
          }
          else
          {
            pData->Segment.usTranslationFlag = TRANSLFLAG_NORMAL;
          } /* endif */
          pData->Segment.lTime = _wtol( ParseX15W( pData->szControl, 2 ) );
          wcscpy( pData->szBufferW, ParseX15W( pData->szControl, 3 ) );
          WideCharToMultiByte( CP_OEMCP, 0, pData->szBufferW, -1, pData->Segment.szSourceLang, sizeof(pData->Segment.szSourceLang), NULL, NULL );
          wcscpy( pData->szBufferW, ParseX15W( pData->szControl, 4 ));
          WideCharToMultiByte( CP_OEMCP, 0, pData->szBufferW, -1, pData->Segment.szTargetLang, sizeof(pData->Segment.szTargetLang), NULL, NULL );
          wcscpy( pData->szBufferW, ParseX15W( pData->szControl, 6 ));
          WideCharToMultiByte( CP_OEMCP, 0, pData->szBufferW, -1, pData->Segment.szFormat, sizeof(pData->Segment.szFormat), NULL, NULL );
          wcscpy( pData->szBufferW, ParseX15W( pData->szControl, 8 ) );
          WideCharToMultiByte( CP_OEMCP, 0, pData->szBufferW, -1, pData->Segment.szDocument, sizeof(pData->Segment.szDocument), NULL, NULL );

          // get contol string end tag and check it
          GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF  );
          AddLineToSegBuffer( pData );
          if ( _wcsnicmp( pData->szLine, L"</control>", 10 ) != 0 )
          {
            // missing end control string
            usRC = ERROR_INVALID_SEGMENT;
            pszError = "Missing end control tag";
          }

        } /* endif */

      }
      else if ( _wcsnicmp( pData->szLine, L"<source>", 8 ) == 0  )
      {
        // get the segment source
        BOOL fEnd = StripTag( pData->szLine + 8, L"</source>" );
        usRC = ConcatenateSegData( pData->Segment.szSource, pData->szLine + 8, sizeof(pData->Segment.szSource)/sizeof(CHAR_W) );

        while ( !usRC && !fEnd && !fEOF )
        {
          usRC = ConcatenateSegData( pData->Segment.szSource, L"\n", MAX_SEGMENT_SIZE );
          if ( usRC )
          {
            pszError = "Segment source text too long";
          } /* endif */
          GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
          AddLineToSegBuffer( pData );
          if (_wcsnicmp( pData->szLine, L"</segment>", 10 ) == 0 ) 
          {
            usRC = ERROR_INVALID_SEGMENT;
            pszError = "Missing </source> tag";
          }
          else
          {
            fEnd = StripTag( pData->szLine, L"</source>" );
            if ( !usRC && !fEnd || (pData->szLine[0] != 0) )
            {
              usRC = ConcatenateSegData( pData->Segment.szSource, pData->szLine, sizeof(pData->Segment.szSource)/sizeof(CHAR_W) );
              if ( usRC )
              {
                pszError = "Segment source text too long";
              } /* endif */
            } /* endif */
          } /* endif */
        } /*endwhile */
        if ( !usRC && !fEnd )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
          pszError = "Missing </source> tag";
        } /* endif */
      }
      else if ( _wcsnicmp( pData->szLine, L"<target>", 8 ) == 0 )
      {
        // get the segment target
        BOOL fEnd = StripTag( pData->szLine + 8, L"</target>" );
        usRC = ConcatenateSegData( pData->Segment.szTarget, pData->szLine + 8, sizeof(pData->Segment.szTarget)/sizeof(CHAR_W) );

        while ( !usRC && !fEnd && !fEOF )
        {
          usRC = ConcatenateSegData( pData->Segment.szTarget, L"\n", MAX_SEGMENT_SIZE );
          if ( usRC )
          {
            pszError = "Segment target text too long";
          } /* endif */
          GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
          AddLineToSegBuffer( pData );
          if (_wcsnicmp( pData->szLine, L"</segment>", 10 ) == 0 ) 
          {
            usRC = ERROR_INVALID_SEGMENT;
            pszError = "Missing </target> tag";
          }
          else
          {
            fEnd = StripTag( pData->szLine, L"</target>" );
            if ( !usRC && !fEnd || (pData->szLine[0] != 0) )
            {
              usRC = ConcatenateSegData( pData->Segment.szTarget, pData->szLine, sizeof(pData->Segment.szTarget)/sizeof(CHAR_W) );
              if ( usRC )
              {
                pszError = "Segment target text too long";
              } /* endif */
            } /* endif */
          } /* endif */
        } /*endwhile */
        if ( !usRC && !fEnd )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
          pszError = "Missing </target> tag";
        } /* endif */
      } 
      else if ( wcsnicmp( pData->szLine, L"<adddata>", 9 ) == 0 )
      {
        // get the segment additional data
        BOOL fEnd = StripTag( pData->szLine + 9, L"</adddata>" );
        usRC = ConcatenateSegData( pData->Segment.szAddInfo, pData->szLine + 9, MAX_SEGMENT_SIZE );

        while ( !usRC && !fEnd && !fEOF )
        {
          usRC = ConcatenateSegData( pData->Segment.szAddInfo, L"\r\n", MAX_SEGMENT_SIZE );
          if ( usRC )
          {
            pszError = "Segment additional data text too long";
          } /* endif */
          GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );
          AddLineToSegBuffer( pData );
          if (_wcsnicmp( pData->szLine, L"</segment>", 10 ) == 0 ) 
          {
            usRC = ERROR_INVALID_SEGMENT;
            pszError = "Missing </adddata> tag";
          }
          else
          {
            fEnd = StripTag( pData->szLine, L"</adddata>" );
            if ( !usRC && !fEnd || (pData->szLine[0] != 0) )
            {
              usRC = ConcatenateSegData( pData->Segment.szAddInfo, pData->szLine, sizeof(pData->Segment.szAddInfo)/sizeof(CHAR_W) );
              if ( usRC )
              {
                pszError = "Segment additional data text too long";
              } /* endif */
            } /* endif */
          } /* endif */
        } /*endwhile */
        if ( !usRC && !fEnd )
        {
          // incomplete segment data
          usRC = ERROR_INVALID_SEGMENT;
          pszError = "Missing </adddata> tag";
        } /* endif */
      } /* endif */
    } /*endwhile */

    // read next line
    GetLine( pData, pData->szLine, sizeof(pData->szLine), pData->fUnicode, &fEOF );

    // test segment data
    if ( !usRC )
    {
      int iSourceLen = wcslen( pData->Segment.szSource );
      int iTargetLen = wcslen( pData->Segment.szTarget );

      if ( !fControlString )
      {
        pszError = "Missing <control>...</control> section";
        usRC = ERROR_INVALID_SEGMENT;
      }
      else if ( iSourceLen >= MAX_SEGMENT_SIZE )
      {
        pszError = "Segment source text is too long";
        usRC = ERROR_INVALID_SEGMENT;
      }
      else if (iTargetLen >= MAX_SEGMENT_SIZE )
      {
        pszError = "Segment target text is too long";
        usRC = ERROR_INVALID_SEGMENT;
      }
      else if ( iSourceLen == 0 )
      {
        pszError = "No segment source text found";
        usRC = ERROR_INVALID_SEGMENT;
      } /* endif */
    } /* endif */

    if ( usRC )
    {
      sprintf( pData->szErrorText, "The segment beginning in line %ld is invalid, the reason is \"%s\".", iStartLine, pszError );

      if ( pData->hfLog != NULL )
      {
        fwprintf( pData->hfLog, L"The segment beginning in line %ld is invalid, the reason is \"%S\".\r\n", iStartLine, pszError );
        fwprintf( pData->hfLog, L"Segment data:\r\n" );
        fwrite( pData->szSegBuffer, sizeof(CHAR_W), pData->iSegBufferUsed, pData->hfLog );
        fwprintf( pData->hfLog, L"\r\n" );
      } /* endif */
    }
    else
    {
        *pfSegmentAvailable = TRUE;
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function GetNextSegment */

// close input memory
USHORT CloseInput( PCONVERTERDATA pData )
{
  USHORT      usRC = NO_ERROR;         // function return code

  if ( pData->hInFile )
  {
    fclose( pData->hInFile );
    pData->hInFile = NULL;
  } /* endif */
  return( usRC );
} /* end of function CloseInput */

// (re)fill input buffer
USHORT FillBufferW
(
  PCONVERTERDATA pData
)
{
  USHORT      usRC = 0;
  PSZ_W       pTemp;
  int         iStillInBuf;
  ULONG       ulBytesRead = 0;

  if ( pData->iInBufProcessed )
  {
    // refill buffer
    pTemp = pData->chInBufW + pData->iInBufProcessed;
    iStillInBuf = pData->iInBufRead - pData->iInBufProcessed;
    memmove( pData->chInBufW, pTemp, iStillInBuf*sizeof(CHAR_W) );
    memset( &pData->chInBufW[iStillInBuf], 0, (BUF_SIZE-iStillInBuf)*sizeof(CHAR_W));
    ulBytesRead = fread( (pData->chInBufW + iStillInBuf), 1, (BUF_SIZE - iStillInBuf)*sizeof(CHAR_W), pData->hInFile );
    pData->iInBufProcessed = 0;
    pData->iInBufRead = (ulBytesRead / sizeof(CHAR_W)) + iStillInBuf;
  }
  else
  {
    // fill buffer for the first time
    ulBytesRead = fread( pData->chInBufW, 1, BUF_SIZE *sizeof(CHAR_W), pData->hInFile );

    pData->iInBufRead = ulBytesRead / sizeof(CHAR_W);
  } /* endif */

  return( usRC );
} /* end of function FillBufferW */

USHORT FillBuffer
(
  PCONVERTERDATA pData
)
{
  USHORT      usRC = 0;
  PSZ         pTemp;
  int         iStillInBuf;
  ULONG       ulBytesRead = 0;

  if ( pData->iInBufProcessed )
  {
    // refill buffer
    pTemp = pData->chInBuf + pData->iInBufProcessed;
    iStillInBuf = pData->iInBufRead - pData->iInBufProcessed;
    memmove( pData->chInBuf, pTemp, iStillInBuf );
    memset( &pData->chInBuf[iStillInBuf], 0, (BUF_SIZE-iStillInBuf) );
    ulBytesRead = fread( (pData->chInBuf + iStillInBuf), 1, (BUF_SIZE - iStillInBuf), pData->hInFile );

    pData->iInBufProcessed = 0;
    pData->iInBufRead = ulBytesRead + iStillInBuf;
  }
  else
  {
    // fill buffer for the first time
    ulBytesRead = fread( pData->chInBuf, 1, BUF_SIZE, pData->hInFile );

    pData->iInBufRead = ulBytesRead;
  } /* endif */

  return( usRC );
} /* end of function FillBuffer */

USHORT ReadLineW
(
  PCONVERTERDATA pData,
  PSZ_W    pszLine,
  int      iSize
)
{
  USHORT      usRC = 0;
  PSZ_W       pLF;
  PSZ_W       pTemp;
  int         iLen = 0;

  // check if buffer can be refilled
  if ( pData->iInBufRead == BUF_SIZE )
  {
      // there is more to read from infile (except it has exactly the size
      // of BUF_SIZE)
      if ( (pData->iInBufProcessed * 2) > pData->iInBufRead )
      {
          // half of buffer parsed, so refill it
          usRC = FillBufferW( pData );
      } /* endif */
  } /* endif */

  pTemp = pData->chInBufW + pData->iInBufProcessed;

  // get one line out of buffer
  pLF = wcschr( pTemp, L'\n' );
  if ( pLF )
  {
      iLen = pLF - pTemp;
      if ( iLen > iSize )
      {
        iLen = iSize - 1;
      } /* endif */

      memcpy( pszLine, pTemp, iLen * sizeof(CHAR_W) );
      pszLine[iLen] = 0;
      if ( (iLen > 1) &&  pszLine[iLen-1] == L'\r' )
      {
          pszLine[iLen-1] = 0;
      } /* endif */

      pData->iInBufProcessed += (iLen+1);
  }
  else
  {
      usRC = BTREE_EOF_REACHED;
  } /* endif */

  return( usRC );
} /* end of function ReadLineW */


USHORT ReadLine
(
  PCONVERTERDATA pData,
  PSZ      pszLine,
  int      iSize
)
{
  USHORT      usRC = 0;
  PSZ         pLF;
  PSZ         pTemp;
  int         iLen = 0;

  // check if buffer can be refilled
  if ( pData->iInBufRead == BUF_SIZE )
  {
      // there is more to read from infile (except it has exactly the size
      // of BUF_SIZE)
      if ( (pData->iInBufProcessed * 2) > pData->iInBufRead )
      {
          // half of buffer parsed, so refill it
          usRC = FillBuffer( pData );
      } /* endif */
  } /* endif */

  pTemp = pData->chInBuf + pData->iInBufProcessed;

  // get one line out of buffer
  pLF = strchr( pTemp, '\n' );
  if ( pLF )
  {
      iLen = pLF - pTemp;
      if ( iLen > iSize )
      {
        iLen = iSize - 1;
      } /* endif */

      memcpy( pszLine, pTemp, iLen );
      pszLine[iLen] = 0;
      if ( (iLen > 1) &&  pszLine[iLen-1] == '\r' )
      {
          pszLine[iLen-1] = 0;
      } /* endif */

      pData->iInBufProcessed += (iLen+1);
  }
  else
  {
      usRC = BTREE_EOF_REACHED;
  } /* endif */

  return( usRC );
} /* end of function ReadLine */


USHORT GetLine( PCONVERTERDATA pData, PSZ_W pszLine, int iSize, BOOL fUnicode, PBOOL pfEOF  )
{
  USHORT usRC = 0;

  *pszLine = 0;
  if ( fUnicode )
  {
    usRC = ReadLineW( pData, pszLine, iSize );
    if ( usRC == BTREE_EOF_REACHED )
    {
      *pfEOF = TRUE;
      usRC = 0;
    } /* endif */
  }
  else
  {
    static CHAR szAsciiLine[8096];  

    szAsciiLine[0] = EOS;

    usRC = ReadLine( pData, szAsciiLine, sizeof(szAsciiLine) );
    if ( usRC == BTREE_EOF_REACHED )
    {
      *pfEOF = TRUE;
      usRC = 0;
    } /* endif */

    MultiByteToWideChar( pData->ulInputCP, 0, szAsciiLine, -1, pszLine, iSize-1 );
  } /* endif */

  pData->iLineNum++;
  return( usRC );
}

// helper function to check for and remove the given tag from the end of the string
BOOL StripTag( PSZ_W pszLine, PSZ_W pszTag )
{
  BOOL fFound = FALSE;
  PSZ_W pszTest = NULL;

  int  iLen = wcslen( pszLine );
  int  iTagLen = wcslen( pszTag );

  if ( iLen >= iTagLen )
  {
    pszTest = pszLine + (iLen - iTagLen);
  } /* endif */

  if ( pszTest )
  {
    fFound = ( _wcsnicmp( pszTest, pszTag, iTagLen ) == 0 );

    if ( fFound )
    {
      *pszTest = 0;
    } /* endif */
  } /* endif */

  return( fFound );
} /* end of function StripTag */

PSZ_W ParseX15W( PSZ_W pszX15String, SHORT sStringId )
{
   PSZ_W   pszTemp = pszX15String;    // set temp. pointer to beginning of string

   while (sStringId--)
   {
      // search end of this part of the X15 string
      while ((*pszTemp != X15) && (*pszTemp != EOS))
      {
         pszTemp++;
      } /* endwhile */

      // position temp. pointer to beginning of next string part (now it points
      // to the delimiter character, i.e. either 0 or 0x15)
      pszTemp++;
   } /* endwhile */

   // remember the beginning of the result string
   pszX15String = pszTemp;

   // change the delimiter character to 0, so that this is a valid C string
   // search end of the result string
   while ((*pszTemp != X15) && (*pszTemp != EOS))
   {
      pszTemp++;
   } /* endwhile */
   // set end of string character
   *pszTemp = EOS;

   // return the pointer to the result string
   return (pszX15String);
} /* end of ParseX15 */


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
          pAddData++;
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

// escape characters in segment data to form valid XML
void EscapeXMLChars( PSZ_W pszText, PSZ_W pszBuffer )
{
  while ( *pszText )
  {
    if ( *pszText == L'\n' )
    {
      wcscpy( pszBuffer, L"\r\n" );
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