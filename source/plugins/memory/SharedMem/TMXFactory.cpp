/*! \brief TMXFactory.CPP - Module with TMX format related functions
	Copyright (c) 1999-2016, International Business Machines Corporation and others. All rights reserved.
	Description: This module contains functions to wrap parameters in the JSON format and to retrieve parameters from JSON strings
*/

#include "TMXFactory.h"
#include "time.h"
#include <vector>
/** Initialize the static instance variable */
TMXFactory* TMXFactory::instance = 0;


class CNameList
{
  public:
	  CNameList();
	  ~CNameList();
    BOOL AddName( const char *pszName, const char *pszValue );  // add a name/value pair to the list
    BOOL FindName( const char *pszName, char *pszValue, int iBufSize );  // find a name and returns its value
  protected:
    PSZ m_pList;                       // points to list data buffer
    int m_iListSize;                   // size of list in # of bytes
};

class SimpleBase64 
{
private:
    unsigned char* DECODETABLE;
	unsigned char *pDataEncoded;
	unsigned char* pDataDecoded;

public:
	SimpleBase64();
	~SimpleBase64();
	wchar_t*  SimpleBase64::decode(wchar_t* encodedStr);
};

SimpleBase64 g_sb64;


// the Win32 Xerces build requires the default structure packing...
#pragma pack( push )
#pragma pack(8)
#include <iostream>
#include <xercesc/util/XercesDefs.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/framework/XMLPScanToken.hpp>
#include <xercesc/parsers/SAXParser.hpp>
#include <xercesc/sax/HandlerBase.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/framework/MemBufInputSource.hpp>
#pragma pack( pop )

XERCES_CPP_NAMESPACE_USE

//////////////////////////////////////////////////////////////////
// part for TMX Sax parser

// name to use for undefined languages
#define OTHERLANGUAGES "OtherLanguages"

// name of TM specific prop elements
#define DESCRIPTION_PROP      "tmgr-description"
#define DESCRIPTION_PROP_W    L"tmgr-description"

#define MARKUP_PROP      "tmgr-markup"
#define MARKUP_PROP_W    L"tmgr-markup"

#define LANGUAGE_PROP    "tmgr-language"
#define LANGUAGE_PROP_W  L"tmgr-language"

#define DOCNAME_PROP    "tmgr-docname"
#define DOCNAME_PROP_W  L"tmgr-docname"

#define SHORTDOCNAME_PROP    "tmgr-short-docname"
#define SHORTDOCNAME_PROP_W  L"tmgr-short-docname"

#define MACHFLAG_PROP    "tmgr-MTflag"
#define MACHFLAG_PROP_W  L"tmgr-MTflag"

#define SEGNUM_PROP      "tmgr-segNum"
#define SEGNUM_PROP_W    L"tmgr-segNum"

#define NOTE_PROP      "tmgr-note"
#define NOTE_PROP_W    L"tmgr-note"

#define NOTESTYLE_PROP      "tmgr-notestyle"
#define NOTESTYLE_PROP_W    L"tmgr-notestyle"


#define LASTACCESSTIME_PROP   "lastAccessTime"
#define LASTACCESSTIME_PROP_W  L"lastAccessTime"

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
typedef enum { TMLANGUAGE_PROP, TMMARKUP_PROP, TMDOCNAME_PROP,TMSHORTDOCNAME_PROP, MACHINEFLAG_PROP, SEG_PROP, TMDESCRIPTION_PROP, TMNOTE_PROP, TMNOTESTYLE_PROP, TMLASTACCESSTIME_PROP,UNKNOWN_PROP } TMXPROPID;

// stack elements
typedef struct _TMXELEMENT
{
  ELEMENTID ID;                        // ID of element 
  BOOL      fInlineTagging;            // TRUE = we are processing inline tagging
  TMXPROPID PropID;                    // ID of prop element (only used for props)
  CHAR      szDataType[50];            // data type of current element
  CHAR      szTMXLanguage[50];         // TMX language of element
  CHAR      szTMLanguage[50];          // TM language of element
  CHAR      szTMMarkup[50];            // TM markup of element
  LONG      lSegNum;                   // TM segment number
} TMXELEMENT, *PTMXELEMENT;



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
  void SetNameLists( CNameList *pLangTmx2Tmgr, CNameList *pType2Markup );
 // void SetProposal( OtmProposal *pProposal );

  // getter functions 
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

  std::vector<OtmProposal*>&  getProposals();
  void clearProposals();
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

  // date conversion help functions
  BOOL IsLeapYear( const int iYear );
  int GetDaysOfMonth( const int iMonth, const int iYear );
  int GetDaysOfYear( const int iYear );
  int GetYearDay( const int iDay, const int iMonth, const int iYear );

  CHAR_W* TMXParseHandler::replace2NewLine(CHAR_W *pIn);

  // name list for name conversions
  CNameList *pLangTmx2Tmgr;
  CNameList *pType2Markup;

  // processing flags
  BOOL fSource;                        // TRUE = source data collected  
  BOOL fTarget;                        // TRUE = target data collected
  BOOL fCatchData;                     // TRUE = catch data
  BOOL fWithTagging;                   // TRUE = add tagging to data
  BOOL fError;                         // TRUE = parsing ended with error

  // segment data
  ULONG ulSegNo;                       // segmet number
  LONG  lTime;                         // segment date/time
  BOOL  fMachineTranslation;           // machine translation flag
  int   iNumOfTu;
  //OtmProposal *pProposal;              // proposal receiving the segment data;
  std::vector<OtmProposal*> proposals;
  // buffers
  #define DATABUFFERSIZE 4098

  typedef struct _BUFFERAREAS
  {
    CHAR_W   szData[DATABUFFERSIZE];  // buffer for collected data
    CHAR     szProp[DATABUFFERSIZE];  // buffer for collected prop values
    CHAR_W   szPropW[DATABUFFERSIZE];  // buffer for collected prop values
    CHAR     szLang[50];              // buffer for language
    CHAR     szDocument[512];          // buffer for document name
    CHAR     szShortDocument[512];          // buffer for document name
    CHAR     szLastAccessTime[13+1];   // last access time in mill
    CHAR     szDescription[1024];     // buffer for memory descripion
    CHAR     szMemSourceLang[50];     // buffer for memory source language
    CHAR     szErrorMessage[1024];    // buffer for error message text
    CHAR_W   szNote[MAX_SEGMENT_SIZE];// buffer for note text
    CHAR_W   szNoteStyle[100];        // buffer for note style
    CHAR     szMarkup[50];            // buffer for markup name
  } BUFFERAREAS, *PBUFFERAREAS;

  PBUFFERAREAS pBuf; 

  // TUV data area
  typedef struct _TMXTUV
  {
    CHAR_W   szText[DATABUFFERSIZE];  // buffer for TUV text
    CHAR     szLang[50];              // buffer for TUV language (converted to Tmgr language name)
    CHAR     szLastAccessTime[13+1];  //last access time in millions, every TUV have one
    BOOL     fInvalidChars;           // TRUE = contains invlaid characters 
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

  void fillSegmentInfo( PTMXTUV pSource, PTMXTUV pTarget, OtmProposal *pProposal );

};



// internal data area
typedef struct _TMXFACTORYDATA
{
  wchar_t szSegmentBuffer[4000];
  wchar_t szTMXBuffer[10000];
  char szUTF8Buffer[10000];
  wchar_t szUnicodeBuffer[1000];
  char szMarkup[40];
  char szSourceLanguage[40];
  char szTargetLanguage[40];
  char szDocName[512];
  CNameList LangTmgr2Tmx;
  CNameList LangTmx2Tmgr;
  CNameList Markup2Type;
  CNameList Type2Markup;
  TMXParseHandler *pHandler;          // our SAX handler 
  SAXParser* pParser;                  // SAX parser
  std::string strLastError;            // last error message
  int iLastError;                      // error code of last error
} TMXFACTORYDATA, *PTMXFACTORYDATA;


TMXFactory::TMXFactory()
{
  pvData = NULL;
}


TMXFactory::~TMXFactory()
{
}

int TMXFactory::init()
{
  this->pvData = new( TMXFACTORYDATA );
  if ( this->pvData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );
    
  PTMXFACTORYDATA pData = (PTMXFACTORYDATA) this->pvData;

  this->loadNames();

  pData->pHandler = NULL;
  pData->pParser = NULL;

  return( 0 );
}




/*! \brief This static method returns a pointer to the MemoryFactory object.
	The first call of the method creates the MemoryFactory instance.
*/
TMXFactory* TMXFactory::getInstance()
{
	if (instance == 0)
	{
		instance = new TMXFactory();
    if ( instance != 0 ) instance->init();
	}
	return instance;
}



/*! \brief Converts a OtmProposal to a TMX formatted string

  // note: this function uses are very simple and restricted solution for the implementation

    \param Proposal reference to proposal data
    \param strTMX reference to a string receiving the TMX formatted string
  	\returns 0 or error code in case of errors
*/
int TMXFactory::ProposalToTMX
(
  OtmProposal &Proposal,
  std::string &strTMX
)
{
    return ProposalToTUString(Proposal,strTMX,true,true);
}

int TMXFactory::ProposalToTUString
(
    OtmProposal &Proposal, 
	std::string &strTU, 
	bool bWithHeader, 
	bool bWithTail
)
{
  int iRC = 0;
  char szTMXLang[20];                  // buffer for TMX language name
  char szDataType[20];                 // buffer for TMX data type
  char szTMXTime[30];                  // buffer for date/time in TMY notation

  PTMXFACTORYDATA pData = (PTMXFACTORYDATA) this->pvData;

  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  // get some of the proposal data
  Proposal.getSourceLanguage( pData->szSourceLanguage, sizeof(pData->szSourceLanguage) );
  Proposal.getTargetLanguage( pData->szTargetLanguage, sizeof(pData->szTargetLanguage) );
  Proposal.getMarkup( pData->szMarkup, sizeof(pData->szMarkup) );
  Proposal.getDocName( pData->szDocName, sizeof(pData->szDocName) );

  // prepare TMX data type
  szDataType[0] = EOS;
  pData->Markup2Type.FindName( pData->szMarkup, szDataType, sizeof(szDataType) );
  if ( szDataType[0] == 0 )
  {
    strcpy( szDataType, "plaintext" ); 
  } /* endif */

  // prepare TMX creation date
  LONG lTime = Proposal.getUpdateTime();
  if ( lTime != 0L ) lTime += 10800L;// correction: + 3 hours
  struct tm *pTimeDate = gmtime( &lTime );
  if ( (lTime != 0L) && pTimeDate )   // if gmtime was successful ...
  {
      sprintf( szTMXTime, "%4.4d%2.2d%2.2dT%2.2d%2.2d%2.2dZ", pTimeDate->tm_year + 1900, pTimeDate->tm_mon + 1, pTimeDate->tm_mday,
               pTimeDate->tm_hour, pTimeDate->tm_min, pTimeDate->tm_sec );
  }
  else
  {
    strcpy( szTMXTime, "19800101T000000Z" );
  }
  // Now don't take accesstime with proposal in TU now,
  // it could let Server to decide time, the time is consistent for all clients
  // it's the best solution for download update counter control

  // get current time
  //char str[50+1]={0};
  //sprintf(str,"%lld",getCurrentMillions());
  //ltoa(getCurrentMillions(),str,10);

  // combine TMX string
  szTMXLang[0] = EOS;
  pData->LangTmgr2Tmx.FindName( pData->szSourceLanguage, szTMXLang, sizeof(szTMXLang) );

  wchar_t *pCurrent = NULL;
  if(bWithHeader) 
  {
	  pCurrent = this->copyString( L"<?xml version=\"1.0\" encoding=\"UTF-8\"?><tmx version=\"1.4\"><header srclang=\"", pData->szTMXBuffer );
	  pCurrent = this->copyString( szTMXLang, pCurrent );
      pCurrent = this->copyString( L"\" datatype=\"", pCurrent );
      pCurrent = this->copyString( szDataType, pCurrent );
      pCurrent = this->copyString( L"\"></header><body>",pCurrent);
  }
  else
  {
	  pCurrent = this->copyString( L"", pData->szTMXBuffer );
  }

 
  //pCurrent = this->copyString(L"<tu tuid=\"1\" datatype=\"", pCurrent );
  //pCurrent = this->copyString( szDataType, pCurrent );
  pCurrent = this->copyString(L"<tu tuid=\"1\" ", pCurrent );
  pCurrent = this->copyString( L"creationdate=\"", pCurrent );
  pCurrent = this->copyString( szTMXTime, pCurrent );
  pCurrent = this->copyString( L"\"><prop type=\"tmgr-segNum\">", pCurrent );
  _ltow( Proposal.getSegmentNum(), pData->szUnicodeBuffer, 10 );
  pCurrent = this->copyString( pData->szUnicodeBuffer, pCurrent );
  pCurrent = this->copyString( L"</prop><prop type=\"tmgr-markup\">", pCurrent );
  pCurrent = this->copyString( pData->szMarkup, pCurrent );
  pCurrent = this->copyString( L"</prop><prop type=\"tmgr-docname\">", pCurrent );
  pCurrent = this->copyString( pData->szDocName, pCurrent );

  // set short doc name
  char shortDocName[20+1]={0};
  Proposal.getDocShortName(shortDocName,sizeof(shortDocName)/sizeof(char));
  pCurrent = this->copyString( L"</prop><prop type=\"tmgr-short-docname\">", pCurrent );
  pCurrent = this->copyString( shortDocName, pCurrent );

  //set last accessed time
  //pCurrent = this->copyString( L"</prop><prop type=\"lastAccessTime\">", pCurrent );
  //pCurrent = this->copyString( str, pCurrent );


  pCurrent = this->copyString( L"</prop><tuv xml:lang=\"", pCurrent );
  szTMXLang[0] = EOS;
  pData->LangTmgr2Tmx.FindName( pData->szSourceLanguage, szTMXLang, sizeof(szTMXLang) );
  pCurrent = this->copyString( szTMXLang, pCurrent );
  pCurrent = this->copyString( L"\"><prop type=\"tmgr-language\">", pCurrent );
  pCurrent = this->copyString( pData->szSourceLanguage, pCurrent );

   // set last accessed time for monolingual
  //pCurrent = this->copyString( L"</prop><prop type=\"lastAccessTime\">", pCurrent );
  //pCurrent = this->copyString( str, pCurrent );

  pCurrent = this->copyString( L"</prop><seg>", pCurrent );
  Proposal.getSource( pData->szSegmentBuffer, sizeof(pData->szSegmentBuffer) / sizeof(wchar_t) );

  // format xml escape characters
  size_t outBufferSize = sizeof(pData->szSegmentBuffer) / sizeof(wchar_t)+20;
  wchar_t *pOutBuffer = new wchar_t[outBufferSize];
  iRC = -1;
  if(pOutBuffer != NULL)
  {
    memset(pOutBuffer,0,outBufferSize*sizeof(wchar_t));
    iRC = xmlFormat(pData->szSegmentBuffer,pOutBuffer,outBufferSize);
  }

  if(iRC == 0)
    pCurrent = this->copyString( pOutBuffer, pCurrent );
  else // if out of memory or format failed, still used the old
    pCurrent = this->copyString( pData->szSegmentBuffer, pCurrent );

  pCurrent = this->copyString( L"</seg></tuv><tuv xml:lang=\"", pCurrent );
  szTMXLang[0] = EOS;
  pData->LangTmgr2Tmx.FindName( pData->szTargetLanguage, szTMXLang, sizeof(szTMXLang) );
  pCurrent = this->copyString( szTMXLang, pCurrent );
  pCurrent = this->copyString( L"\"><prop type=\"tmgr-language\">", pCurrent );
  pCurrent = this->copyString( pData->szTargetLanguage, pCurrent );

   // set last accessed time for monolingual
  //pCurrent = this->copyString( L"</prop><prop type=\"lastAccessTime\">", pCurrent );
  //pCurrent = this->copyString( str, pCurrent );

  pCurrent = this->copyString( L"</prop><seg>", pCurrent );
  Proposal.getTarget( pData->szSegmentBuffer, sizeof(pData->szSegmentBuffer) / sizeof(wchar_t) );

  // format xml escape characters
  memset(pOutBuffer,0,outBufferSize*sizeof(wchar_t));
  iRC = xmlFormat(pData->szSegmentBuffer,pOutBuffer,outBufferSize);

  if(iRC == 0)
    pCurrent = this->copyString( pOutBuffer, pCurrent );
  else // if out of memory or format failed, still used the old
    pCurrent = this->copyString( pData->szSegmentBuffer, pCurrent );

  if(pOutBuffer != NULL) 
  {
    delete []pOutBuffer;
    pOutBuffer = NULL;
  }

  pCurrent = this->copyString( L"</seg></tuv></tu>",pCurrent);

  if(bWithTail)
      pCurrent = this->copyString(L"</body></tmx>", pCurrent );

  *pCurrent = 0;

  WideCharToMultiByte( CP_UTF8, 0, pData->szTMXBuffer, -1, pData->szUTF8Buffer, sizeof(pData->szUTF8Buffer), NULL, NULL );
  strTU.assign( pData->szUTF8Buffer );

  return( iRC );
}

/*! \brief Converts a TMX formatted string into an OtmProposal 

    \param strTMX reference to a string containing the TMX formatted string
    \param Proposal reference to a OtmPorposal object receiving the extracted data
  	\returns 0 or error code in case of errors
*/
int TMXFactory::TMX2Proposal
(
  std::string &strTMX,
  std::vector<OtmProposal*> &proposals
  //OtmProposal &Proposal
)
{
  int iRC = 0;

  PTMXFACTORYDATA pData = (PTMXFACTORYDATA) this->pvData;
  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  if ( pData->pParser == NULL )
  {
    try {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch) {
        toCatch;

        return( ERROR_NOT_READY );
    }

    pData->pParser = new SAXParser();      // Create a SAX parser object
  } /* endif */     

  if ( pData->pHandler == NULL )
  {
    pData->pHandler = new TMXParseHandler();
    if ( pData->pHandler == NULL ) return( ERROR_NOT_READY );

    // pass language and markup name list to handler
    pData->pHandler->SetNameLists( &(pData->LangTmx2Tmgr), &(pData->Type2Markup) ); 

    //  install our SAX handler as the document and error handler.
    pData->pParser->setDocumentHandler( pData->pHandler );
    pData->pParser->setErrorHandler( pData->pHandler );
    pData->pParser->setCalculateSrcOfs( TRUE );
    pData->pParser->setValidationSchemaFullChecking( FALSE );
    pData->pParser->setDoSchema( FALSE );
    pData->pParser->setLoadExternalDTD( FALSE );
    pData->pParser->setValidationScheme( SAXParser::Val_Never );
    pData->pParser->setExitOnFirstFatalError( FALSE );
  } /* endif */     

  // init the proposals vector
  pData->pHandler->clearProposals();

  // parse the TMX string
  if ( iRC == 0 )
  {
    InputSource* inSource = new MemBufInputSource((const unsigned char*)strTMX.c_str(), strTMX.length(), "1",  false);  
      
    try
    {
      pData->pParser->parse( *inSource );
    }
    catch (const OutOfMemoryException& )
    {
      pData->strLastError = "OutOfMemoryException in SAX parser";
      pData->iLastError = iRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    catch (const XMLException& toCatch)
    {
      toCatch;

      pData->strLastError = XMLString::transcode(toCatch.getMessage());
      pData->iLastError = iRC = ERROR_READ_FAULT;
    }

    delete( inSource );
  } /* endif */

   proposals = pData->pHandler->getProposals();

  return( iRC );
}

/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
int TMXFactory::getLastError
(
  std::string &strError
)
{
  PTMXFACTORYDATA pData = (PTMXFACTORYDATA) this->pvData;
  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  strError = pData->strLastError;
  return( pData->iLastError );
}

/*! \brief Get the error message for the last error occured

    \param pszError pointer to a buffer for the error text
    \param iBufSize size of error text buffer in number of characters
  	\returns last error code
*/
int TMXFactory::getLastError
(
  char *pszError,
  int iBufSize
)
{
  PTMXFACTORYDATA pData = (PTMXFACTORYDATA) this->pvData;
  if ( pData == NULL ) return( this->ERROR_INSUFFICIENTMEMORY );

  strncpy( pszError, pData->strLastError.c_str(), iBufSize );
  pszError[iBufSize-1] = 0;;
  return( pData->iLastError );
}




/*! \brief Copy a string to a buffer and return pointer to the end of the copied data

    \param pszString pointer to stringbeing copied
    \param pszBuffer pointer to buffer for the string data
  	\returns pointer to the end of the copied data
*/
wchar_t *TMXFactory::copyString
(
  char *pszString,
  wchar_t *pszBuffer
)
{
  PTMXFACTORYDATA pData = (PTMXFACTORYDATA) this->pvData;
  
  MultiByteToWideChar( CP_OEMCP, 0, pszString, -1, pData->szUnicodeBuffer, sizeof(pData->szUnicodeBuffer) / sizeof(wchar_t) );
  return( this->copyString( pData->szUnicodeBuffer, pszBuffer ) ); 
}

wchar_t *TMXFactory::copyString
(
  wchar_t *pszString,
  wchar_t *pszBuffer
)
{
  while ( *pszString != 0 ) *pszBuffer++ = *pszString++;
  return( pszBuffer );
}



/*! \brief Load name list file for Tmgr <-> TMX name conversions
  	\returns 0 if successful otherwise error code
*/
int TMXFactory::loadNames
( 
)
{
  FILE *hf = NULL;
  char szLine[512];
  CNameList *pActiveList = NULL;
  USHORT usRC = 0;
  PTMXFACTORYDATA pData = (PTMXFACTORYDATA) this->pvData;

  // setup list file name in our line buffer
  UtlMakeEQFPath( szLine, NULC, TABLE_PATH, NULL );
  strcat( szLine, "\\TMXNAMES.LST" );

  // load and process name file
  hf = fopen( szLine, "r" );
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
            pActiveList = &pData->LangTmgr2Tmx;
          }
          else if ( _stricmp( szLine + 1, "Language-TMX2Tmgr" ) == 0 )
          {
            pActiveList = &pData->LangTmx2Tmgr;
          }
          else if ( _stricmp( szLine + 1, "Markup2Type" ) == 0 )
          {
            pActiveList = &pData->Markup2Type;
          }
          else if ( _stricmp( szLine + 1, "Type2Markup" ) == 0 )
          {
            pActiveList = &pData->Type2Markup;
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

/*! \brief replace xml escape charaters in pSrc

    \param pSrc  pointer to stringbeing formatted
    \param pOut  pointer to buffer for outputing
    \param outCapcity the capcity of pOut
  	\returns 0 if success
*/
int TMXFactory::xmlFormat(wchar_t *pSrc, wchar_t *pOut,size_t outCapcity)
{
  if(pSrc==NULL || pOut==NULL)
    return -1;
  wchar_t  srcCh[] = {L'<', L'>', L'"', L'\'',L'\r',L'\n'};
  wchar_t  *replStr[] = {L"&lt;", L"&gt;", L"&quot;", L"&apos;",L"",L"@@"};

  wchar_t *pOutCurrent = pOut;

  for(size_t srcIndex = 0; srcIndex<wcslen(pSrc); srcIndex++)
  {
    size_t j;
    for(j=0; j<sizeof(srcCh)/sizeof(srcCh[0]); j++)
    {
      if(pSrc[srcIndex] == srcCh[j])
        break;
    }
     
    if(j < sizeof(srcCh)/sizeof(srcCh[0]))
    {
      wcsncpy(pOutCurrent,replStr[j],wcslen(replStr[j]));
      pOutCurrent += wcslen(replStr[j]);
    }
    else
    {
      *pOutCurrent = pSrc[srcIndex];
      pOutCurrent++;

      if(pSrc[srcIndex]==L'&')
      {
        if(wcsncmp(pSrc+srcIndex+1, L"lt;",3)!=0 &&
           wcsncmp(pSrc+srcIndex+1, L"gt;",3)!=0 &&
           wcsncmp(pSrc+srcIndex+1, L"quot;",5)!=0 &&
           wcsncmp(pSrc+srcIndex+1, L"apos;",5)!=0 &&
           wcsncmp(pSrc+srcIndex+1, L"amp;",4)!=0)
        {
          // it's &, and & has been copied previous
          wcsncpy(pOutCurrent,L"amp;",wcslen(L"amp;"));
          pOutCurrent += wcslen(L"amp;");
        }
      }
    }

    if((pOutCurrent-pOut) >= outCapcity)
      return -2;
  }//end for

  *pOutCurrent = 0;

  return 0;
}

/*! \brief whether the year is leap
    \param year  pointer to stringbeing formatted
  	\returns true if is leap
*/
bool TMXFactory::isLeapYear(int year) 
{
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

/*! \brief get current time in millions
  	\returns current millions
*/
long long  TMXFactory::getCurrentMillions()
{
  SYSTEMTIME st;
  GetSystemTime(&st);

  const int beginYear = 1970;
  
  // how much days during the previous years
  long totalDays = 0; 
  for(int iyear=beginYear; iyear!=st.wYear; iyear++)
  {
    if(isLeapYear(iyear))
      totalDays += 366;
    else
      totalDays += 365;

  }

  const int daysInMonths[] = {31,28,31,30,31,30,31,31,30,31,30,31};

  // how much days during the previous month in this year
  for(int imonth=1; imonth!=st.wMonth; imonth++)
  {
    totalDays += daysInMonths[imonth-1];
    if(imonth==2 && isLeapYear(st.wYear))
    {
      totalDays += 1;
    }
  }

  // how much days in this month
  for(int iday=1; iday!=st.wDay; iday++)
    totalDays++;

  long long totalSeconds = totalDays*24*60*60;

  // how much seconds in this day
  totalSeconds += st.wHour*60*60 + st.wMinute*60 + st.wSecond;
  // finally results 
  long long   toalMillionSeconds = totalSeconds*1000 + st.wMilliseconds;

  return toalMillionSeconds;
}

//
// implementation of name list class
//

// construct an empty name list
CNameList::CNameList()
{
  m_pList = NULL;
  m_iListSize = 0;
}

// destructor
CNameList::~CNameList() 
{
  if ( m_pList ) free( m_pList );
}

// add the given name and value to the list
BOOL CNameList::AddName( const char *pszName, const char *pszValue )
{
  BOOL fOK = TRUE;

  int iNameLen = strlen(pszName) + 1;
  int iValueLen = strlen(pszValue) + 1;  

  m_pList = (PSZ)realloc( m_pList, m_iListSize + iNameLen + iValueLen );

  if ( m_pList )
  {
    PSZ pszListEnd = m_pList + m_iListSize;
    strcpy( pszListEnd, pszName );
    pszListEnd  += iNameLen;
    strcpy( pszListEnd, pszValue );
    m_iListSize += iNameLen + iValueLen;
  }
  else
  {
    fOK = FALSE;
    m_pList = NULL;
    m_iListSize = 0;
  } /* endif */
  
  return( fOK );
}

// find the given name and return its value in the supplied buffer
BOOL CNameList::FindName( const char *pszName, char *pszValue, int iBufSize )
{
  BOOL fFound = FALSE;
  PSZ  pszEntry = m_pList;
  PSZ  pszListEnd = m_pList + m_iListSize;

  while ( !fFound && (pszEntry < pszListEnd) )
  {
    if ( _stricmp( pszEntry, pszName ) == 0 )
    {
      // entry found
      while ( *pszEntry ) pszEntry++; // skip name
      pszEntry++;
      memset( pszValue, 0, iBufSize );
      strncpy( pszValue, pszEntry, iBufSize - 1 );
      fFound = TRUE;
    }
    else
    {
      // try next entry
      while ( *pszEntry ) pszEntry++; // skip name
      pszEntry++;
      while ( *pszEntry ) pszEntry++; // skip value
      pszEntry++;
    } /* endif */
  } /*endwhile */

  return( fFound );
}



//
// Implementation of TMX SAX parser
//
TMXParseHandler::TMXParseHandler()
{
  hfLog = NULLHANDLE;

  // allocate buffer areas
  pBuf = (PBUFFERAREAS)malloc( sizeof(BUFFERAREAS) );
  if ( pBuf ) memset( pBuf, 0, sizeof(BUFFERAREAS) );
  fError = FALSE;

  // initialize element stack
  iStackSize = 0;
  iCurElement = 0;
  pStack = NULL;

  // initialize TUV array
  pTuvArray = NULL;
  iCurTuv = 0;
  iTuvArraySize = 0;

  //this->pProposal = NULL;
}

TMXParseHandler::~TMXParseHandler()
{
  if ( hfLog )  fclose( hfLog );
  if ( pStack ) free( pStack );
  if ( pBuf )   free( pBuf );
  if ( pTuvArray ) free( pTuvArray );
  // release the proposals
  clearProposals();
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
        this->pBuf->szMarkup[0] = 0;
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
              if ( _wcsicmp( pszValue, MARKUP_PROP_W ) == 0)
              {
                CurElement.PropID = TMMARKUP_PROP;
              }
              else if ( _wcsicmp( pszValue, LANGUAGE_PROP_W ) == 0)
              {
                CurElement.PropID = TMLANGUAGE_PROP;
              }
              else if ( _wcsicmp( pszValue, DOCNAME_PROP_W ) == 0)
              {
                CurElement.PropID = TMDOCNAME_PROP;
              }
              else if ( _wcsicmp( pszValue, SHORTDOCNAME_PROP_W ) == 0)
              {
                CurElement.PropID = TMSHORTDOCNAME_PROP;
              }
              else if ( _wcsicmp( pszValue, MACHFLAG_PROP_W ) == 0)
              {
                CurElement.PropID = MACHINEFLAG_PROP;
              } 
              else if ( _wcsicmp( pszValue, NOTE_PROP_W ) == 0)
              {
                CurElement.PropID = TMNOTE_PROP;
              } 
              else if ( _wcsicmp( pszValue, NOTESTYLE_PROP_W ) == 0)
              {
                CurElement.PropID = TMNOTESTYLE_PROP;
              } 
              else if ( _wcsicmp( pszValue, SEGNUM_PROP_W ) == 0)
              {
                CurElement.PropID = SEG_PROP;
              } 
              else if ( _wcsicmp( pszValue, DESCRIPTION_PROP_W ) == 0)
              {
                CurElement.PropID = TMDESCRIPTION_PROP;
              }
              else if ( _wcsicmp( pszValue, LASTACCESSTIME_PROP_W ) == 0)
              {
                CurElement.PropID = TMLASTACCESSTIME_PROP;
              }/* endif */
            } /* endif */
          } /* endif */
        } /* endfor */
        pBuf->szProp[0] = 0;                 // reset data buffer
        pBuf->szPropW[0] = 0;                 // reset data buffer
        break;

      case HEADER_ELEMENT:
        // reset element info
        CurElement.fInlineTagging = FALSE;
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
        fMachineTranslation = FALSE;

        pBuf->szNote[0] = 0;
        pBuf->szNoteStyle[0] = 0;

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
        if ( CurElement.szTMMarkup[0] )
        {
          // TM markup available: add inline tags as well
          fWithTagging = TRUE;
        }
        else if ( CurElement.szDataType[0] )
        {
          // check if TMX data type maps to a TM markup
          if ( pType2Markup->FindName( CurElement.szDataType, CurElement.szTMMarkup, sizeof(CurElement.szTMMarkup) ) )
          {
            fWithTagging = TRUE;
          }
          else if ( this->pBuf->szMarkup[0] != EOS )
          {
            strcpy( CurElement.szTMMarkup, this->pBuf->szMarkup );
            fWithTagging = TRUE;
          }
          else
          {
            strcpy( CurElement.szTMMarkup, "EQFASCII" );
            fWithTagging = FALSE;
          } /* endif */
        }
        else
        {
          if ( this->pBuf->szMarkup[0] != EOS )
          {
            strcpy( CurElement.szTMMarkup, this->pBuf->szMarkup );
            fWithTagging = TRUE;
          }
          else
          {
            // no data type or markup
            fWithTagging = FALSE;
          } /* endif */
        } /* endif */
        break;
      case BODY_ELEMENT:
        break;
      case SEG_ELEMENT:
        CurElement.fInlineTagging = FALSE;
        pBuf->szData[0] = 0;                 // reset data buffer
        fCatchData = TRUE; 
        break;
      case BPT_ELEMENT:
      case EPT_ELEMENT:
      case PH_ELEMENT:
      case HI_ELEMENT:
      case IT_ELEMENT:
        CurElement.fInlineTagging = TRUE;
        break;
      case SUB_ELEMENT:
        break;
      case UT_ELEMENT:
        break;
      case INVCHAR_ELEMENT:
        // segment contains invalid data
        this->fInvalidChars = TRUE;
        break;
      case UNKNOWN_ELEMENT:
        break;
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
  OtmProposal     *pProposal
)
{
  pProposal->clear();

  if ( CurElement.lSegNum != 0 )
  {
    pProposal->setSegmentNum( CurElement.lSegNum ); 
  }
  else
  {
    pProposal->setSegmentNum( (LONG)ulSegNo ); 
  } /* endif */     

  pProposal->setMarkup( "OTMANSI" );  // preset with default value...
  if ( CurElement.szTMMarkup[0] )
  {
    pProposal->setMarkup( CurElement.szTMMarkup );
  }
  else if ( this->pBuf->szMarkup[0] != EOS  ) 
  {
    pProposal->setMarkup( this->pBuf->szMarkup );
  }
  else if ( CurElement.szDataType[0] ) 
  {
    pType2Markup->FindName( CurElement.szDataType, this->pBuf->szMarkup, sizeof(this->pBuf->szMarkup) );
    pProposal->setMarkup( this->pBuf->szMarkup );
  } /* endif */

  if ( pBuf->szDocument[0] )
  {
    pProposal->setDocName( pBuf->szDocument );
  }
  else
  {
    pProposal->setDocName( "" );
  } /* endif */


  // also set short name, otherwise may cause repeated insert into memory
  if ( pBuf->szShortDocument[0] )
  {
    pProposal->setDocShortName(pBuf->szShortDocument);
  }
  else
  {
    pProposal->setDocShortName( "" );
  } /* endif */


  pProposal->setType( fMachineTranslation ? OtmProposal::eptMachine : OtmProposal::eptManual );

  //if ( lTime == 0 ) time( (time_t *)&lTime);
  //pProposal->setUpdateTime( lTime );

  // source info
  if ( pSource != NULL )
  {
    pProposal->setSourceLanguage( pSource->szLang );

	wchar_t* pDecoded = g_sb64.decode(pSource->szText);
	replace2NewLine(pDecoded);

    pProposal->setSource( pDecoded );
  } /* endif */

  // target info
  if ( pTarget != NULL )
  {
    pProposal->setTargetLanguage( pTarget->szLang );

	wchar_t*  pDecoded =  g_sb64.decode(pTarget->szText);
	replace2NewLine(pDecoded);

    pProposal->setTarget( pDecoded );

	// use target time to set updatetime
	char timeInSecs[10+1]={0};
	strncpy(timeInSecs, pTarget->szLastAccessTime,10);
	pProposal->setUpdateTime( atol(timeInSecs) );

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
      break;
    case PROP_ELEMENT:
      // is processed after current element has been removed from stack...
      break;
    case TU_ELEMENT:
      // TU is complete, check collected data and add memory segment if everything is OK
      {
        int iCurrent = 0;
        PTMXTUV pSourceTuv = NULL;

        // find <tuv> containing the source data
        iCurrent = 0;
        PTMXTUV pCurrentTuv = pTuvArray;
        while ( (iCurrent < iCurTuv) && (pSourceTuv == NULL) )
        {
          if ( strcmp( pCurrentTuv->szLang, pBuf->szMemSourceLang ) == 0 )
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
        }
        else
        {
          iCurrent = 0;
          pCurrentTuv = pTuvArray;
          while ( iCurrent < iCurTuv )
          {
            if ( pCurrentTuv != pSourceTuv )
            {
              // fill-in proposal data
              OtmProposal *pProposal = new OtmProposal;
              fillSegmentInfo( pSourceTuv, pCurrentTuv, pProposal );
              this->proposals.push_back(pProposal);
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
   
        if ( pBuf->szData[0] == 0 ) fTuvValid = FALSE;          // no data for <tuv>
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
            if ( !pLangTmx2Tmgr->FindName( CurElement.szTMXLanguage, pBuf->szLang, sizeof(pBuf->szLang) ) )
            {
              // language not found, strip language code and try again
              PSZ pszDelim = strchr( CurElement.szTMXLanguage, '-' );
              if ( pszDelim )
              {
                *pszDelim = '\0';
                if ( !pLangTmx2Tmgr->FindName( CurElement.szTMXLanguage, pBuf->szLang, sizeof(pBuf->szLang) ) )
                {
                  fTuvValid = FALSE;          // language not supported
                } /* endif */
              }
              else
              {
                fTuvValid = FALSE;          // language not supported
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
          // use tuv language as source language when no language specified yet
          if ( pBuf->szMemSourceLang[0] == 0 )
          {
            strcpy( pBuf->szMemSourceLang, pBuf->szLang );
          } /* end */           

          PTMXTUV pTuvData = pTuvArray + iCurTuv;

          wcscpy( pTuvData->szText, pBuf->szData );
          strcpy( pTuvData->szLang, pBuf->szLang );
          strcpy(pTuvData->szLastAccessTime, pBuf->szLastAccessTime);
          pTuvData->fInvalidChars = this->fInvalidChars;

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
      CurElement.fInlineTagging = FALSE;
      break;
    case SUB_ELEMENT:
      break;
    case UT_ELEMENT:
      break;
    case INVCHAR_ELEMENT:
      break;
    case UNKNOWN_ELEMENT:
      break;
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
	else if ( CurrentProp == TMSHORTDOCNAME_PROP )
	{
	  memset( pBuf->szShortDocument, 0, sizeof(pBuf->szShortDocument) );
      strncpy( pBuf->szShortDocument, pBuf->szProp, sizeof(pBuf->szShortDocument) - 1 );
	}
    else if ( CurrentProp == TMDESCRIPTION_PROP )
    {
      memset( pBuf->szDescription, 0, sizeof(pBuf->szDescription) );
      strncpy( pBuf->szDescription, pBuf->szProp, sizeof(pBuf->szDescription) - 1 );
    }
    else if ( CurrentProp == MACHINEFLAG_PROP )
    {
      fMachineTranslation = (pBuf->szProp[0] == '1') ? TRUE : FALSE;
    }
    else if ( CurrentProp == TMNOTE_PROP )
    {
      wcscpy( pBuf->szNote, pBuf->szPropW );
    }
    else if ( CurrentProp == TMNOTESTYLE_PROP )
    {
      wcscpy( pBuf->szNoteStyle, pBuf->szPropW );
    }
    else if ( CurrentProp == SEG_PROP )
    {
      CurElement.lSegNum = atol( pBuf->szProp );
    }
	else if ( CurrentProp == TMLASTACCESSTIME_PROP )
	{
      memset( pBuf->szLastAccessTime, 0, sizeof(pBuf->szLastAccessTime) );
      strncpy( pBuf->szLastAccessTime, pBuf->szProp, sizeof(pBuf->szLastAccessTime) - 1 );
	}/* endif */
  } /* endif */
}

void TMXParseHandler::characters(const XMLCh* const chars, const XMLSize_t length)
{
  PSZ_W pszChars = (PSZ_W)chars;

  if ( hfLog ) fprintf( hfLog, "%ld Characters; \"%S\"\n", length, pszChars );

  if ( (CurElement.ID == PROP_ELEMENT) && (CurElement.PropID != UNKNOWN_PROP) )
  {
    // store data in prop value buffer
    int iBytes = WideCharToMultiByte( CP_OEMCP, 0, pszChars, length, pBuf->szProp, sizeof(pBuf->szProp), NULL, NULL );
    pBuf->szProp[iBytes] = '\0';
    wcsncpy( pBuf->szPropW, pszChars, length );
    pBuf->szPropW[length] = 0;
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
      wcsncpy( pBuf->szData + iCurLen, pszChars, length );
      pBuf->szData[iCurLen+length] = 0;
    } /* endif */
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

void TMXParseHandler::SetNameLists( CNameList *pLangList, CNameList *pTypeList )
{
  pLangTmx2Tmgr = pLangList;
  pType2Markup = pTypeList;
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
    if ( !pLangTmx2Tmgr->FindName( pszTMXLanguage, pszResultingLanguage, 50 ) )
    {
      // try first part of language only
      PSZ pszDelim = strchr( pszTMXLanguage, '-' );
      if ( pszDelim )
      {
        *pszDelim = '\0';
        pLangTmx2Tmgr->FindName( pszTMXLanguage, pszResultingLanguage, 50 );
        *pszDelim = '-';
      } /* endif */
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of method TMXParseHandler::GeValue */


//void TMXParseHandler::SetProposal( OtmProposal *pInProposal )
//{
  // I think maybe write wrong, it should be pInProposal
  //this->pProposal = pInProposal;//pProposal;
//} /* end of method TMXParseHandler::SetProposal */


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

CHAR_W* TMXParseHandler::replace2NewLine(CHAR_W *pIn)
{
	if(pIn == NULL)
		return NULL;
	
	CHAR_W *ptrCur = pIn;
	CHAR_W *ptrKept = pIn;
	while(*(ptrCur+1) != '\0')
	{
		if(*ptrCur=='@' &&  *(ptrCur+1)=='@' )
		{
			*ptrKept = '\n';
			ptrKept++;
			ptrCur += 2;
			if(*ptrCur == '\0')
				break;
		}
		else
		{
			if(ptrCur == ptrKept )
			{
			    ptrCur++;
			    ptrKept++;
			}
			else
			{
				*ptrKept++ = *ptrCur++ ;
			}
		}
	}

	if(ptrCur!=ptrKept && ptrKept!= NULL)
	{
		*ptrKept++ = *ptrCur++;
		*ptrKept = '\0';
	}
	return pIn;
}

std::vector<OtmProposal*>&  TMXParseHandler::getProposals()
{
	return this->proposals;
}

void TMXParseHandler::clearProposals()
{
   for(std::vector<OtmProposal*>::iterator iter= this->proposals.begin();
    iter != this->proposals.end();
    iter++)
  {
    delete (*iter);
  }
  this->proposals.clear();
}

SimpleBase64::SimpleBase64()
{
	pDataEncoded = new unsigned char[DATABUFFERSIZE];
	pDataDecoded = new unsigned char[DATABUFFERSIZE];

	if(pDataEncoded!=NULL && pDataDecoded!=NULL )
	{
		DECODETABLE = new unsigned char[256];

		for (int i = 0; i < 256; i++)
			DECODETABLE[i] = 255;
		for (int i = 'A'; i <= 'Z'; i++)
			DECODETABLE[i] = (unsigned char) (i - 'A');
		for (int i = 'a'; i <= 'z'; i++)
			DECODETABLE[i] = (unsigned char) (26 + i - 'a');
		for (int i = '0'; i <= '9'; i++)
			DECODETABLE[i] = (unsigned char) (52 + i - '0');
		DECODETABLE['+'] = 62;
		DECODETABLE['/'] = 63;
	}
}

SimpleBase64::~SimpleBase64()
{
	if(pDataEncoded != NULL)
		delete []pDataEncoded;

	if(pDataDecoded != NULL)
		delete []pDataDecoded;

	pDataDecoded = NULL;
	pDataEncoded = NULL;
}

	
wchar_t* SimpleBase64::decode(wchar_t* encodedStr)
{
	memset(pDataEncoded,0,DATABUFFERSIZE*sizeof(unsigned char));
	int cnt = WideCharToMultiByte( CP_UTF8, 0, encodedStr, wcslen(encodedStr)+1,(LPSTR)(pDataEncoded), DATABUFFERSIZE*sizeof(unsigned char), NULL, NULL ) ;
	if(cnt == 0)
	{
		return encodedStr;
	}
	unsigned char *data = pDataEncoded;

	int orgLen = strlen((char*)data);
	int len = ((orgLen + 3) / 4) * 3;
	if (orgLen>0 && data[orgLen-1]=='=')
		--len;
	if (orgLen>1 && data[orgLen-2]=='=')
		--len;
	
	memset(pDataDecoded,0,DATABUFFERSIZE*sizeof(unsigned char));

	int shift = 0;
	int accum = 0;
	int index = 0;
	for (int ix = 0; ix < orgLen; ix++)
	{
		int value = DECODETABLE[data[ix] & 0xFF];
		if (value != 255) 
		{
			accum <<= 6;
			shift += 6;
			accum |= value;
			if (shift >= 8) 
			{
				shift -= 8;
				pDataDecoded[index++] =  (unsigned char) ((accum >> shift) & 0xff);
			}
		}
	}
	
	memset(encodedStr,0,DATABUFFERSIZE*sizeof(wchar_t));
	MultiByteToWideChar( CP_UTF8, 0, (LPSTR)pDataDecoded, -1, encodedStr, DATABUFFERSIZE*sizeof(wchar_t) );

	return encodedStr;
}