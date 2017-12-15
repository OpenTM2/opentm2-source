/*! \file
	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/

#include "eqf.h"
#include "eqfserno.h"
#include "windows.h"

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

#include "CXmlWriter.h"
#include "OtmProofReadList.h"

//
// class for our SAX handler which parses the XML file containing the proof read entries
//
class ProofReadParseHandler : public HandlerBase
{
public:
  // -----------------------------------------------------------------------
  //  Constructors and Destructor
  // -----------------------------------------------------------------------
  ProofReadParseHandler();
  virtual ~ProofReadParseHandler();

  // setter functions 
  void setProofReadList( OtmProofReadList *pList );

  // getter functions 
  bool ErrorOccured( void );
  void GetErrorText( char *pszTextBuffer, int iBufSize );

  //  handlers for the SAX DocumentHandler interface
  void startElement(const XMLCh* const name, AttributeList& attributes);
  void endElement(const XMLCh* const name );
  void characters(const XMLCh* const chars, const XMLSize_t length);

  //  handlers for the SAX ErrorHandler interface
  void warning(const SAXParseException& exc);
  void error(const SAXParseException& exc );
  void fatalError(const SAXParseException& exc);

private:
  // IDs of XML elelements
  typedef enum { HEADER_ELEMENT, CREATIONDATE_ELEMENT, PROOFREADDATE_ELEMENT, FOLDER_ELEMENT, TRANSLATOR_ELEMENT, PROOFREADER_ELEMENT, DOCUMENT_ELEMENT, SEGMENTLIST_ELEMENT,
                 SEGMENT_ELEMENT, SOURCE_ELEMENT, TARGET_ELEMENT, MODTARGET_ELEMENT, NEWTARGET_ELEMENT, COMMENT_ELEMENT, UNKNOWN_ELEMENT
               } ELEMENTID;

  typedef struct _XMLNAMETOID
  {
    CHAR_W   szName[30];                 // name of element
    ProofReadParseHandler::ELEMENTID ID;                        // ID of element 
  } XMLNAMETOID, *PXMLNAMETOID;

  ELEMENTID CurID;
  std::vector<ELEMENTID> vELementStack;
  std::wstring strSource;
  std::wstring strTarget;
  std::wstring strModTarget;
  std::wstring strNewTarget;
  std::wstring strComment;
  CHAR_W szDataW[4048];
  CHAR szData[4048];
  char szErrorText[256];
  OtmProofReadList *pList;
  bool fError;                         // TRUE = parsing ended with error
  int m_iCurFol;                       // number (=index) of current folder
  int m_iCurDoc;                       // number (=index) of current document
  OtmProofReadEntry *pCurEntry;        // current proof read entry
  ELEMENTID GetElementID( PSZ_W pszName );
};


// constructor
OtmProofReadList::OtmProofReadList()
{
}



/*! \brief Loads the proof read document entry list from the specified file
  \param pszInFile fully qualified name of the XML file containing the proof read entries being loaded
  \param hwndError handle of parent window for error messages
  \returns 0 if successful or an error code
*/
int OtmProofReadList::load( const char *pszInFile, HWND hwndError )
{
  int iRC = 0;

  // parse the XML file containing the language properties
  if ( !iRC )
  {
    try {
        XMLPlatformUtils::Initialize();
    }
    catch (const XMLException& toCatch) {
        toCatch;

        return( ERROR_NOT_READY );
    }

    SAXParser* parser = new SAXParser();      // Create a SAX parser object

    // create an instance of our handler
    ProofReadParseHandler *handler = new ProofReadParseHandler();


    //  install our SAX handler as the document and error handler.
    parser->setDocumentHandler( handler );
    parser->setErrorHandler( handler );
    parser->setValidationSchemaFullChecking( false );
    parser->setDoSchema( false );
    parser->setLoadExternalDTD( false );
    parser->setValidationScheme( SAXParser::Val_Never );
    parser->setExitOnFirstFatalError( false );
    handler->setProofReadList( this );

    try
    {
      parser->parse( pszInFile );
    }
    catch (const OutOfMemoryException& )
    {
      iRC = ERROR_NOT_ENOUGH_MEMORY;
    }
    catch (const XMLException& toCatch)
    {
      toCatch;

      iRC = ERROR_READ_FAULT;
    }
  } /* endif */


  return( iRC );
}

/*! \brief Saves the proof read document entry list to the specified file
  \param pszOutFile fully qualified name of the XML file receiving the proof read entries
  \param hwndError handle of parent window for error messages
  \returns 0 if successful, an error code in case of errors
*/
int OtmProofReadList::save( char *pszOutFile, HWND hwndEror )
{
  CXmlWriter *xw = new CXmlWriter( pszOutFile );
  xw->Formatting = CXmlWriter::Indented;
  xw->Encoding = CXmlWriter::UTF8;
  xw->Indention = 2;

  xw->WriteStartDocument();

  xw->WriteStartElement( "ProofReadingResults" );

  xw->WriteStartElement( "Header" );
  xw->WriteElementString( "Folder", this->m_vFolderList[0].c_str() );
  xw->WriteElementString( "CreationDate", this->m_strCreationDate.c_str() );
  xw->WriteElementString( "ProofReadDate", this->m_strProofReadDate.c_str() );
  xw->WriteElementString( "Translator", this->m_strTranslator.c_str() );
  xw->WriteElementString( "ProofReader", this->m_strProofReader.c_str() );
  xw->WriteEndElement(); // "Header"

  int iCurDoc = -1;
  for( size_t i = 0; i < this->m_vList.size(); i++ )
  {
    OtmProofReadEntry *pEntry = m_vList[i];
    int iDocNum = pEntry->getDocumentNumber();
    if ( iCurDoc != iDocNum )
    {
      if ( iCurDoc != -1 )
      {
        xw->WriteEndElement(); // "SegmentList"
        xw->WriteEndElement(); // "Document"
      } /* endif */

      xw->WriteStartElement( "Document" );
      xw->WriteAttributeString( "Name", this->m_vDocumentList[iDocNum].c_str() );
      xw->WriteAttributeString( "SourceLang", this->m_vDocSourceLangList[iDocNum].c_str() );
      xw->WriteAttributeString( "TargetLang", this->m_vDocTargetLangList[iDocNum].c_str() );
      xw->WriteAttributeString( "Markup", this->m_vDocmarkupList[iDocNum].c_str() );

      xw->WriteStartElement( "SegmentList" );

      iCurDoc = iDocNum;
    } /* endif */

    xw->WriteStartElement( "Segment" );
    char szBuffer[40];
    _snprintf_s( szBuffer, sizeof(szBuffer), 20, "%ld", pEntry->getSegmentNumber() );
    xw->WriteAttributeString( "Number", szBuffer );
    xw->WriteAttributeString( "Selected", pEntry->getSelected() ? "yes" : "no" );
    xw->WriteAttributeString( "Processed", pEntry->getProcessed() ? "yes" : "no" );

    xw->WriteElementString( L"Source", pEntry->getSource()  );
    xw->WriteElementString( L"OrgTarget", pEntry->getTarget()  );
    xw->WriteElementString( L"ModTarget", pEntry->getModTarget()  );
    xw->WriteElementString( L"NewTarget", pEntry->getNewTarget()  );
    xw->WriteElementString( L"Comment", pEntry->getComment()  );

    xw->WriteEndElement(); // "Segment"

  } /* endfor */

  if ( iCurDoc != -1 )
  {
    xw->WriteEndElement(); // "SegmentList"
    xw->WriteEndElement(); // "Document"
  } /* endif */

  xw->WriteEndElement(); // "ProofReadingResults" 
  xw->Close();

  return( 0 );
}

/*! \brief Clears the content of the current list
*/
void OtmProofReadList::clear()
{
  for( size_t i = 0; i < m_vList.size(); i++ )
  {
    PVOID pvTargetChangeList = m_vList[i]->getTargetChangeList();
    if ( pvTargetChangeList ) delete( pvTargetChangeList );
    PVOID pvModTargetChangeList = m_vList[i]->getModTargetChangeList();
    if ( pvModTargetChangeList ) delete( pvModTargetChangeList );
    delete m_vList[i];
  }
  m_vList.clear();
	m_vDocumentList.clear();
  m_vFolderList.clear();
  m_strCreationDate.clear();
  m_strProofReadDate.clear();
  m_strTranslator.clear();
  m_strProofReader.clear();
  m_vDocSourceLangList.clear();
  m_vDocTargetLangList.clear();
  m_vDocmarkupList.clear();

}

/*! \brief Add a document to our document list
  \param strName name of the document
  \param strSourceLang source language of the document
  \param strTargetLang target language of the document
  \param strMarkup markup of the document
  \returns index of the added document
*/
int OtmProofReadList::addDocument( std::string strName, std::string strSourceLang, std::string strTargetLang, std::string strMarkup )
{
  int iCurDoc = m_vDocumentList.size();

  m_vDocumentList.push_back( strName );
  m_vDocSourceLangList.push_back( strSourceLang );
  m_vDocTargetLangList.push_back( strTargetLang );
  m_vDocmarkupList.push_back( strMarkup );

  return( iCurDoc );
}

/*! \brief Add a folder to our folder list
  \param strName name of the folder
  \returns index of the added document
*/
int OtmProofReadList::addFolder( std::string strName )
{

  // check if folder name is already contained in our folder list
  int iEntries = m_vFolderList.size();
  int i = 0;
  while( (i < iEntries) && (m_vFolderList[i].compare( strName ) != 0) ) i++;

  if ( i < iEntries )
  {
    return( i );
  }
  else
  {
    m_vFolderList.push_back( strName );
    return( iEntries );
  } /* endif */
}

/*! \brief get the number of selected entries
  \returns number of selected entries
*/
int OtmProofReadList::getNumOfSelected()
{
  int iSelected = 0;

  for( size_t i = 0; i < m_vList.size(); i++ )
  {
    if ( (m_vList[i])->getSelected() )
    {
      iSelected += 1;
    } /* endif */
  }
  return( iSelected );
}


//
// implementation of SAX parser for the parsing of the language control file
//
ProofReadParseHandler::ProofReadParseHandler()
{
  fError = FALSE;
  pList = NULL;
}

ProofReadParseHandler::~ProofReadParseHandler()
{
}

void ProofReadParseHandler::startElement(const XMLCh* const name, AttributeList& attributes)
{
    PSZ_W pszName = (PSZ_W)name;

    attributes;

    CurID = GetElementID( pszName );
    vELementStack.push_back( CurID );
    szDataW[0] = 0;
    switch ( CurID )
    {
      case DOCUMENT_ELEMENT:
        {
          std::string strDocName;
          std::string strDocSourceLang;
          std::string strDocTargetLang;
          std::string strDocMarkup;

          int iAttribs = attributes.getLength(); 

          for( int i = 0; i < iAttribs; i++ )
          {
            PSZ_W pszName = (PSZ_W)attributes.getName( i );
            if ( _wcsicmp( pszName, L"name" ) == 0 )
            {
              PSZ_W pszValue = (PSZ_W)attributes.getValue( i );

              if ( pszValue != NULL )
              {
                WideCharToMultiByte( CP_OEMCP, 0, pszValue, -1, this->szData, sizeof(this->szData), NULL, NULL );
                strDocName = this->szData;
              } /* endif */
            } 
            else if ( _wcsicmp( pszName, L"sourcelang" ) == 0 )
            {
              PSZ_W pszValue = (PSZ_W)attributes.getValue( i );

              if ( pszValue != NULL )
              {
                WideCharToMultiByte( CP_OEMCP, 0, pszValue, -1, this->szData, sizeof(this->szData), NULL, NULL );
                strDocSourceLang = this->szData;
              } /* endif */
            } 
            else if ( _wcsicmp( pszName, L"targetlang" ) == 0 )
            {
              PSZ_W pszValue = (PSZ_W)attributes.getValue( i );

              if ( pszValue != NULL )
              {
                WideCharToMultiByte( CP_OEMCP, 0, pszValue, -1, this->szData, sizeof(this->szData), NULL, NULL );
                strDocTargetLang = this->szData;
              } /* endif */
            } 
            else if ( _wcsicmp( pszName, L"markup" ) == 0 )
            {
              PSZ_W pszValue = (PSZ_W)attributes.getValue( i );

              if ( pszValue != NULL )
              {
                WideCharToMultiByte( CP_OEMCP, 0, pszValue, -1, this->szData, sizeof(this->szData), NULL, NULL );
                strDocMarkup = this->szData;
              } /* endif */
            } /* endif */
          } /* endfor */

          m_iCurDoc = this->pList->addDocument( strDocName, strDocSourceLang, strDocTargetLang, strDocMarkup );
        }
        break;
      case SEGMENT_ELEMENT:
        {
          std::string strDocName;
          std::string strDocSourceLang;
          std::string strDocTargetLang;
          std::string strDocMarkup;

          int iAttribs = attributes.getLength(); 

          this->pCurEntry = new OtmProofReadEntry();
          pCurEntry->setDocumentNumber( this->m_iCurDoc );

          this->strModTarget.clear();
          this->strNewTarget.clear();
          this->strTarget.clear();
          this->strSource.clear();
          this->strComment.clear();

          BOOL fSelected = FALSE;
          BOOL fProcessed = FALSE;

          for( int i = 0; i < iAttribs; i++ )
          {
            PSZ_W pszName = (PSZ_W)attributes.getName( i );
            if ( _wcsicmp( pszName, L"number" ) == 0 )
            {
              PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
              if ( pszValue ) pCurEntry->setSegmentNumber( (unsigned long)_wtol( pszValue ) ); 
            } 
            else if ( _wcsicmp( pszName, L"selected" ) == 0 )
            {
              PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
              if ( pszValue != NULL )
              {
                fSelected = wcsicmp( pszValue, L"yes" ) == 0;
              } /* endif */
            } 
            else if ( _wcsicmp( pszName, L"processed" ) == 0 )
            {
              PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
              if ( pszValue != NULL )
              {
                fProcessed = wcsicmp( pszValue, L"yes" ) == 0;
              } /* endif */
            } /* endif */
          } /* endfor */
          this->strModTarget.clear();
          this->strNewTarget.clear();
          this->strTarget.clear();
          this->strSource.clear();
          this->strComment.clear();

          pCurEntry->setSelected( fSelected );
          pCurEntry->setProcessed( fProcessed );
        }
        break;
      case MODTARGET_ELEMENT:
        {
          int iAttribs = attributes.getLength(); 
          pCurEntry->setModTargetDeletedFlag( FALSE );
          for( int i = 0; i < iAttribs; i++ )
          {
            PSZ_W pszName = (PSZ_W)attributes.getName( i );
            if ( _wcsicmp( pszName, L"type" ) == 0 )
            {
              PSZ_W pszValue = (PSZ_W)attributes.getValue( i );
              if ( _wcsicmp( pszValue, L"deleted" ) == 0 )
              {
                pCurEntry->setModTargetDeletedFlag( TRUE );
              } /* endif */
            } /* endif */
          } /* endfor */
        }
        break;
      case UNKNOWN_ELEMENT:
      default:
        break;
    } /*endswitch */
}

void ProofReadParseHandler::endElement(const XMLCh* const name )
{
  CurID = vELementStack.back();
  vELementStack.pop_back();

  name;

  // convert collected data to ASCII
  WideCharToMultiByte( CP_OEMCP, 0, this->szDataW, -1, this->szData, sizeof(this->szData), NULL, NULL );

  switch ( CurID )
  {
    case CREATIONDATE_ELEMENT:
      this->pList->setCreationDate( this->szData );
      break;
    case PROOFREADDATE_ELEMENT:
      this->pList->setProofReadDate( this->szData );
      break;
    case FOLDER_ELEMENT:
        m_iCurFol = this->pList->addFolder( this->szData );
      break;
    case TRANSLATOR_ELEMENT:
      this->pList->setTranslator( this->szData );
      break;
    case PROOFREADER_ELEMENT:
      this->pList->setProofReader( this->szData );
      break;
    case DOCUMENT_ELEMENT:
      break;
    case SEGMENT_ELEMENT:
      pCurEntry->setSource( strSource.c_str() );
      pCurEntry->setModTarget( strModTarget.c_str() );
      pCurEntry->setNewTarget( strNewTarget.c_str() );
      pCurEntry->setTarget( strTarget.c_str() );
      pCurEntry->setComment( strComment.c_str() );
      pCurEntry->setDocumentNumber( m_iCurDoc );
      pCurEntry->setFolderNumber( m_iCurFol );
      pList->addEntry( pCurEntry );
      pCurEntry = NULL;
      break;
    case SOURCE_ELEMENT:
      this->strSource = this->szDataW;
      break;
    case TARGET_ELEMENT:
      this->strTarget = this->szDataW;
      break;
    case MODTARGET_ELEMENT:
      this->strModTarget = this->szDataW;
      break;
    case NEWTARGET_ELEMENT:
      this->strNewTarget = this->szDataW;
      break;
    case COMMENT_ELEMENT:
      this->strComment = this->szDataW;
      break;
    case UNKNOWN_ELEMENT:
    default:
      break;
  } /*endswitch */
}

void ProofReadParseHandler::characters(const XMLCh* const chars, const XMLSize_t length)
{
  PSZ_W pszChars = (PSZ_W)chars;

  // add data to current data buffer 
  int iCurLen = wcslen( szDataW );
  if ( (iCurLen + length + 1) < (sizeof(szData)/sizeof(CHAR_W)))
  {
    wcsncpy( szDataW + iCurLen, pszChars, length );
    szDataW[iCurLen+length] = 0;
  } /* endif */
}

void ProofReadParseHandler::fatalError(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    this->fError = TRUE;
    sprintf( this->szErrorText, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    XMLString::release( &message );
}

void ProofReadParseHandler::error(const SAXParseException& exception)
{
    char* message = XMLString::transcode(exception.getMessage());
    long line = (long)exception.getLineNumber();
    long col = (long)exception.getColumnNumber();
    this->fError = TRUE;
    sprintf( this->szErrorText, "Fatal Error: %s at column %ld in line %ld", message, col, line );
    XMLString::release( &message );
}

void ProofReadParseHandler::warning(const SAXParseException& exception)
{
  exception;
}


// get the ID for a XML element
ProofReadParseHandler::ELEMENTID ProofReadParseHandler::GetElementID( PSZ_W pszName )
{
  int i = 0;
  ELEMENTID IDFound = UNKNOWN_ELEMENT;

static XMLNAMETOID XmlNameToID[] =
{ { L"folder",        FOLDER_ELEMENT },
  { L"creationdate",  CREATIONDATE_ELEMENT }, 
  { L"proofreaddate", PROOFREADDATE_ELEMENT }, 
  { L"translator",    TRANSLATOR_ELEMENT }, 
  { L"proofreader",   PROOFREADER_ELEMENT }, 
  { L"header",        HEADER_ELEMENT }, 
  { L"document",      DOCUMENT_ELEMENT },
  { L"segmentlist",   SEGMENTLIST_ELEMENT }, 
  { L"segment",       SEGMENT_ELEMENT }, 
  { L"source",        SOURCE_ELEMENT }, 
  { L"orgtarget",     TARGET_ELEMENT }, 
  { L"modtarget",     MODTARGET_ELEMENT }, 
  { L"newtarget",     NEWTARGET_ELEMENT },
  { L"comment",       COMMENT_ELEMENT }, 
  { L"",              UNKNOWN_ELEMENT } };

while ( (IDFound == UNKNOWN_ELEMENT) && (XmlNameToID[i].szName[0] != 0) )
  {
    if ( _wcsicmp( pszName, XmlNameToID[i].szName ) == 0 )
    {
      IDFound = XmlNameToID[i].ID;
    }
    else
    {
      i++;
    } /* endif */
  } /*endwhile */
  return( IDFound );
} /* end of method ProofReadParseHandler::GetElementID */

bool ProofReadParseHandler::ErrorOccured( void )
{
  return( this->fError );
}

void ProofReadParseHandler::GetErrorText( char *pszTextBuffer, int iBufSize )
{
  *pszTextBuffer = '\0';

  if ( this->szErrorText[0] != '\0' )
  {
    strncpy( pszTextBuffer, this->szErrorText, iBufSize );
    pszTextBuffer[iBufSize-1] = '\0';
  } /* endif */
}

void ProofReadParseHandler::setProofReadList( OtmProofReadList *pList )
{
  this->pList = pList;
}

