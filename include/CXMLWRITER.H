//+----------------------------------------------------------------------------+
//| CXMLWRITER.H                                                               |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Header file for the XML Writer class                          |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//
// PVCS Section
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.2 $ ----------- 12 Apr 2006
// GQ: - added methods WriteStartDocType, WriteEntity, WriteEndDocType
//     - made WriteRaw method public
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.2 $ ----------- 9 Nov 2005
// GQ: -changed return code of WriteStartDocuemnt method from void to BOOL
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.2 $ ----------- 19 Jul 2005
// GQ: - added Init prototype
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.3 $ ----------- 11 Apr 2005
// GQ: - added some protected member variables to class
//     - added WriteStylesheet method
// 
// 
// $Revision: 1.2 $ ----------- 7 Apr 2005
// GQ: - added UTF-16 versions of methods
// 
// 
// $Revision: 1.1 $ ----------- 30 Nov 2004
// GQ: - initial put
// 
// 
class CXmlWriter
{
public:
	CXmlWriter();
  CXmlWriter( const char *strFileName );
	virtual ~CXmlWriter() {};
	void SetFileName( const char *strFileName );


	BOOL WriteStartDocument();

  void WriteStartDocType( const WCHAR * type );
  void WriteEntity( const WCHAR *name, const WCHAR *value );
  void WriteEndDocType();

	void WriteStylesheet( const char *stylesheet );
	void WriteStylesheet( const WCHAR *stylesheet );

  // write string, escape characters
  void WriteString( const char * text );
  void WriteString( const WCHAR *text );

  // write string, enclose in CDATA section
  void WriteCDataString( const char * text );
  void WriteCDataString( const WCHAR *text );

  // write integer value as string
  void WriteInt( int iValue );

  // write the start of an attribute
  void WriteStartAttribute( const char * prefix, const char * localName, const char * ns );
  void WriteStartAttribute( const WCHAR *prefix, const WCHAR * localName, const WCHAR * ns );
  void WriteStartAttribute( const char * localName, const char * ns )
    { WriteStartAttribute( NULL, localName, ns ); };
  void WriteStartAttribute( const char * localName)
    { WriteStartAttribute( NULL, localName, NULL ); };
  void WriteStartAttribute( const WCHAR * localName)
    { WriteStartAttribute( NULL, localName, NULL ); };

  // write complete attribute string
	void WriteAttributeString( const WCHAR *localname, const WCHAR *value )
    { WriteAttributeString( localname, NULL, value ); };
	void WriteAttributeString( const char *localname, const char *value )
    { WriteAttributeString( localname, NULL, value ); };
  void WriteAttributeString( const WCHAR *localName, const WCHAR * ns, const WCHAR *value );
  void WriteAttributeString( const char *localName, const char * ns, const char *value );

  // write the end of an attrbute
  void WriteEndAttribute();

  // write start of a new element
  void WriteStartElement( const WCHAR * localName )
    { WriteStartElement( NULL, localName, NULL ); };
  void WriteStartElement( const char * localName )
    { WriteStartElement( NULL, localName, NULL ); };
  void WriteStartElement( const WCHAR * localName, const WCHAR * ns )
    { WriteStartElement( NULL, localName, ns ); };
  void WriteStartElement( const char * localName, const char * ns )
    { WriteStartElement( NULL, localName, ns ); };
  void WriteStartElement( const WCHAR * prefix, const WCHAR * localName, const WCHAR * ns );
  void WriteStartElement( const char * prefix, const char * localName, const char * ns );


  // write the end of an element
  void WriteEndElement();

  // write a complete element string
  void WriteElementString( const char * localName, const char * value )
    { WriteElementString( localName, NULL, value ); };
  void WriteElementString( const char * localName, const char * ns, const char * value );
  void WriteElementString( const WCHAR * localName, const WCHAR * value )
    { WriteElementString( localName, NULL, value ); };
  void WriteElementString( const WCHAR * localName, const WCHAR * ns, const WCHAR * value );

  // write end of the document
	void WriteEndDocument();

  // write a comment 
  void WriteComment( const WCHAR * text );
  void WriteComment( const char * text );

  // write text without escaping characters
  void WriteRaw( const WCHAR * text );
  void WriteRaw( const WCHAR * text, int iLen );

  // close output file
  void Close();
    
  // size of indention
  int Indention;

  // formatting
  enum { Indented, None } Formatting; 

  // encoding
  enum { UTF8, UTF16 } Encoding; 


protected:
  FILE *m_hf;                          // handle of output file
  int m_iStackSize;
  int m_iCurIndention;
  int m_iColumn;
  char m_strFileName[_MAX_PATH];
  char m_Buffer[2048];
  BOOL m_fLFBeforeEnd;                 // TRUE = insert LF before adding end element
  typedef enum _ElementType { Tag, Attribute, Document, Undefined } ElementType;
  struct _Element
  {
    enum _ElementType ElementType;
    WCHAR *pName;                      // name of element
    struct _Element *pPrev;            // previous element 
    BOOL  fContent;                    // TRUE = contains other elements
  };
  struct _Element * m_pRoot;
  int Push( enum _ElementType type, const WCHAR *name );
  int Pop( enum _ElementType *pType, WCHAR **ppName );
  void WriteIndention( int iIndention );

  void WriteStringInt( const WCHAR * text );

  // conversion utilities
  void AnsiToUnicode( const char *pAnsiText, WCHAR **pUnicodeText );

  ElementType GetCurElement( void );
  void SetContentFlag( void );
  BOOL GetContentFlag( void );
  BOOL m_fOpenTag;
  void Init( void );
};
