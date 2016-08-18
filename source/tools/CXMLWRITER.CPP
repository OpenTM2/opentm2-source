//+----------------------------------------------------------------------------+
//| CXMLWRITER.CPP                                                             |
//+----------------------------------------------------------------------------+
//| Copyright Notice:                                                          |
//|                                                                            |
//|          Copyright (C) 1990-2016, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: XML Writer based on the MS .NET XmlWriter API                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//| - UFT8 encoding not implemented yet                                        |
//| - UTF16 encoding not implemented yet                                       |
//| - character escaping not implemented yet                                   |
//| - XML Writer API only partially implemented                                |
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
// $Revision: 1.2 $ ----------- 1 Apr 2009
// GQ: - write BOM for UTF-8 encoding
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
// $Revision: 1.2 $ ----------- 16 Nov 2007
// GQ: - suppress ASCII control characters below 0x20 in XML output (fix for P033028)
//     - replace 0x16 character with blnak (fix for P032908)
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
// $Revision: 1.3 $ ----------- 7 Nov 2006
// GQ: - fixed P028556: Create detailed list for redundant segment list creates an eror by filtering
//       the character hex 1f when writing XML output
// 
// 
// $Revision: 1.2 $ ----------- 16 Aug 2006
// GQ: - added escaping of quote character
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.2 $ ----------- 12 Apr 2006
// GQ: - added methods WriteStartDocType, WriteEntity, WriteEndDocType
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.2 $ ----------- 9 Nov 2005
// GQ: - changed return code of WriteStartDocuemnt method from void to BOOL and added file open error checking
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.3 $ ----------- 19 Aug 2005
// GQ: - fixed type in style sheet name reference
// 
// 
// $Revision: 1.2 $ ----------- 19 Jul 2005
// GQ: - corrected class constructor handling
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.3 $ ----------- 11 Apr 2005
// GQ: - added WriteStylesheet method
//     - corrected bug in character escaping
// 
// 
// $Revision: 1.2 $ ----------- 7 Apr 2005
// GQ: - some fixes in UTF-8 part of WriteRaw method
// 
// 
// $Revision: 1.1 $ ----------- 30 Nov 2004
// GQ: - initial put
// 
// 

  // use non-ISO version of swprintf which was used up to VS2005, once older versions are not used anymore we
  // should use the new version of the function asap
  #define _CRT_NON_CONFORMING_SWPRINTFS


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "cxmlwriter.h"

CXmlWriter::CXmlWriter( const char *strFileName )
{
  this->Init();
  SetFileName( strFileName );
}

CXmlWriter::CXmlWriter()
{
  this->Init();
}

void CXmlWriter::Init( void )
{
  this->m_pRoot = NULL;
  this->m_iStackSize = 0;
  this->Indention = 0;
  this->Formatting = None; 
  this->Encoding = UTF16; 
  this->m_iCurIndention = 0;
  this->m_fOpenTag = FALSE;
  this->m_fLFBeforeEnd = TRUE;
}

void CXmlWriter::SetFileName( const char *strFileName )
{
  strcpy( m_strFileName, strFileName );
}


BOOL CXmlWriter::WriteStartDocument()
{
  BOOL fOK = TRUE;

  m_hf = fopen( m_strFileName, "wb" );

  if ( m_hf )
  {
    // write BOM 
    if ( Encoding == UTF16 )
    {
      fwrite( "\xff\xfe", 1, 2, m_hf );
    }
    else
    {
      fwrite( "\xEF\xBB\xBF", 1, 3, m_hf );
    } /* endif */

    WriteRaw( L"<?xml version=\"1.0\" encoding=\"" );
    if ( Encoding == UTF16 )
    {
      WriteRaw( L"UTF-16" );
    }
    else
    {
      WriteRaw( L"UTF-8" );
    } /* endif */
    WriteRaw( L"\" ?>\n" );
    Push( CXmlWriter::Document, L"doc" );

  }
  else
  {
    fOK = FALSE;
  } /* endif */

  return( fOK );
}

void CXmlWriter::WriteStylesheet( const char *stylesheet )
{
  // convert to UTF16 and use UTF16 method
  WCHAR *pUnicodeText;
  this->AnsiToUnicode( stylesheet, &pUnicodeText );
  WriteStylesheet( pUnicodeText );
  free( pUnicodeText );
}

void CXmlWriter::WriteStylesheet( const WCHAR *stylesheet )
{
  WriteRaw( L"<?xml-stylesheet type=\"text/xsl\" href=\"" );
  WriteRaw( stylesheet );
  WriteRaw( L"\" ?>\n" );
}

void CXmlWriter::WriteStartDocType( const WCHAR * type  )
{
  WriteRaw( L"<!DOCTYPE " );
  WriteRaw( type );
  WriteRaw( L"\n" );
  WriteRaw( L"  [\n" );
}

void CXmlWriter::WriteEndDocType()
{
  WriteRaw( L"  ]>\n" );
}

void CXmlWriter::WriteEntity( const WCHAR *name, const WCHAR *value )
{
  WriteRaw( L"       <!ENTITY " );
  WriteRaw( name );
  WriteRaw( L" \"" );

  WriteRaw( value );
  WriteRaw( L"\">\n" );
}


//void CXmlWriter::WriteString( const WCHAR * text )
void CXmlWriter::WriteString( const wchar_t *text )
{
  // close any open tag
  if ( GetCurElement() == CXmlWriter::Tag && m_fOpenTag )
  {
    WriteRaw( L">" );
    m_fOpenTag = FALSE;
  } /* endif */
  
  if ( GetCurElement() == CXmlWriter::Tag )
  {
    SetContentFlag();
  } /* endif */
  
  WriteStringInt( text );
}

void CXmlWriter::WriteString( const char * text )
{
  // convert to UTF16 and use UTF16 method
  WCHAR *pUnicodeText;
  this->AnsiToUnicode( text, &pUnicodeText );
  WriteString( pUnicodeText );
  free( pUnicodeText );
}

void CXmlWriter::WriteCDataString( const char * text )
{
  // convert to UTF16 and use UTF16 method
  WCHAR *pUnicodeText;
  this->AnsiToUnicode( text, &pUnicodeText );
  WriteCDataString( pUnicodeText );
  free( pUnicodeText );
}

void CXmlWriter::WriteCDataString( const WCHAR *text )
{
  // close any open tag
  if ( GetCurElement() == CXmlWriter::Tag && m_fOpenTag )
  {
    WriteRaw( L">" );
    m_fOpenTag = FALSE;
  } /* endif */
  
  if ( GetCurElement() == CXmlWriter::Tag )
  {
    SetContentFlag();
  } /* endif */
  
  WriteRaw( L"<![CDATA[" );
  WriteRaw( text );
  WriteRaw( L"]]>" );
}


// write integer value as string
void CXmlWriter::WriteInt( int iValue )
{
  WCHAR szValue[20];
  swprintf( szValue, L"%d", iValue );
  WriteString( szValue );
}

// internal write string methof with character escaping
void CXmlWriter::WriteStringInt( const WCHAR *text )
{
  WCHAR szEscapeChars[20];           
  BOOL fDone = FALSE;

  do
  {
    int iSpecialCharPos = 0;
    szEscapeChars[0] = 0;                        // no escape character sequence yet
    BOOL fEscapeChars = FALSE;

    // find next special character which requires an escape sequence
    do
    {
      switch ( text[iSpecialCharPos] )
      {
        case L'<':
          wcscpy( szEscapeChars, L"&lt;" );
          fEscapeChars = TRUE;
          break;
        case L'>':
          wcscpy( szEscapeChars, L"&gt;" );
          fEscapeChars = TRUE;
          break;
        case L'&':
          wcscpy( szEscapeChars, L"&amp;" );
          fEscapeChars = TRUE;
          break;
        case L'\"':
          wcscpy( szEscapeChars, L"&quot;" );
          fEscapeChars = TRUE;
          break;
        case 0x1F:
          wcscpy( szEscapeChars, L"" );
          fEscapeChars = TRUE;
          break;
        case 0x16:
          // required blank used by IBMA4 markup tables
          wcscpy( szEscapeChars, L" " );
          fEscapeChars = TRUE;
          break;
        case 0:
          // end of input string reached...
          fDone = TRUE;
          break;
        case L'\n':
        case L'\r':
        case L'\t':
          // allowed whitespaces, leave as is...
          iSpecialCharPos++;
          break;
        default:
          if ( text[iSpecialCharPos] < 32 )
          {
            // suppress characters below 0x20
            wcscpy( szEscapeChars, L"_" );
            fEscapeChars = TRUE;
          }
          else
          {
            // no spcial handling required
            iSpecialCharPos++;
          } /* endif */
          break;
      } /*endswitch */
    } while ( !fDone && !fEscapeChars  );

    // write characters up to special character
    if ( iSpecialCharPos ) WriteRaw( text, iSpecialCharPos );

    // write any escape sequence
    if ( fEscapeChars )
    {
      if ( szEscapeChars[0] != 0 )
      {
        WriteRaw( szEscapeChars );
      } /* endif */

      iSpecialCharPos++;
    } /* endif */

    // continue with next part of string
    text += iSpecialCharPos;

  } while ( !fDone && text[0] );
}

// write string as-is 
void CXmlWriter::WriteRaw( const WCHAR * text )
{
  int iLen = wcslen( text );
  WriteRaw( text, iLen );
}

// write string as-is 
void CXmlWriter::WriteRaw( const WCHAR * text, int iLen )
{
  // write string as-is except for LFs which are replaced by CRLF
  do
  {
    // find next LF 
    int iLFPos = 0;
    while ( (text[iLFPos] != L'\n') && iLen ) 
    {
      iLFPos++;
      iLen--;
    } /*endwhile */

    // write text up to LF
    if ( iLFPos )
    {
      if ( Encoding == UTF16 )
      {
        if ( m_hf )
        {
          fwrite( text, sizeof(WCHAR), iLFPos, m_hf );
        } /* endif */
      }
      else
      {
        int iBytes = WideCharToMultiByte( CP_UTF8, 0, text, iLFPos, m_Buffer, sizeof(m_Buffer), NULL, NULL );
        fwrite( m_Buffer, 1, iBytes, m_hf );
      } /* endif */
      m_iColumn += iLFPos;
    } /* endif */

    // handle any LF and prepare for next part of text
    if ( text[iLFPos] == L'\n' )
    {
      if ( Encoding == UTF16 )
      {
        fwrite( L"\r\n", sizeof(WCHAR), 2, m_hf );
      }
      else
      {
        fwrite( "\r\n", 1, 2, m_hf );
      } /* endif */
      iLFPos++;
      if ( iLen ) iLen--;
      m_iColumn = 0;
    } /* endif */
    text += iLFPos;
  } while ( iLen );
}

void CXmlWriter::WriteStartAttribute( const char * prefix, const char * localname, const char * ns )
{
  WCHAR *pUnicodePrefix = NULL, *pUnicodeLocalName = NULL, *pUnicodeNameSpace = NULL;

  if ( prefix )    AnsiToUnicode( prefix, &pUnicodePrefix );
  if ( localname ) AnsiToUnicode( localname, &pUnicodeLocalName );
  if ( ns )        AnsiToUnicode( ns, &pUnicodeNameSpace );

  WriteStartAttribute( pUnicodePrefix, pUnicodeLocalName, pUnicodeNameSpace );

  if ( pUnicodePrefix )    free( pUnicodePrefix );
  if ( pUnicodeLocalName ) free( pUnicodeLocalName );
  if ( pUnicodeNameSpace ) free( pUnicodeNameSpace );
}

void CXmlWriter::WriteStartAttribute( const WCHAR *prefix, const WCHAR *localname, const WCHAR *ns )
{
  ns; prefix; 

  if ( GetCurElement() == CXmlWriter::Tag && m_fOpenTag )
  {
    if ( GetCurElement() == CXmlWriter::Attribute )
    {
      WriteEndAttribute();
    } /* endif */
    WriteRaw( L" " );
    WriteRaw( localname );
    WriteRaw( L"=\"" );
    Push( CXmlWriter::Attribute, localname );
  }
  else
  {
    // wrong context!
  } /* endif */
}

void CXmlWriter::WriteEndAttribute()
{
  enum _ElementType Type;
  WCHAR *pName;

  if ( (GetCurElement() == CXmlWriter::Attribute) && m_iStackSize )
  {
    Pop( &Type, &pName );

    if ( Type == CXmlWriter::Attribute )
    {
      WriteRaw( L"\"" );
      free( pName );
    } /* endif */
  }
  else
  {
    // wrong context!
  } /* endif */
}

void CXmlWriter::WriteAttributeString( const char *localname, const char * ns, const char *value )
{
  WCHAR *pUnicodeLocalName = NULL;
  WCHAR *pUnicodeNameSpace = NULL;
  WCHAR *pUnicodeValue = NULL;

  if ( localname ) this->AnsiToUnicode( localname, &pUnicodeLocalName );
  if ( ns ) this->AnsiToUnicode( ns, &pUnicodeNameSpace );
  if ( value ) this->AnsiToUnicode( value, &pUnicodeValue );

  WriteAttributeString( pUnicodeLocalName, pUnicodeNameSpace, pUnicodeValue );

  if ( pUnicodeValue )     free( pUnicodeValue );
  if ( pUnicodeLocalName ) free( pUnicodeLocalName );
  if ( pUnicodeNameSpace ) free( pUnicodeNameSpace );

}

void CXmlWriter::WriteAttributeString( const WCHAR *localname, const WCHAR * ns, const WCHAR *value )
{
  ns;                                  // currently not used

  WriteStartAttribute( NULL, localname, ns );
  WriteStringInt( value );
  WriteEndAttribute();
}

void CXmlWriter::WriteIndention( int iIndention )
{
  if ( Formatting == Indented )
  {
    while ( m_iColumn < iIndention )
    {
      WriteRaw( L" " );
    } /*endwhile */
  } /* endif */
}

void CXmlWriter::WriteStartElement( const char * prefix, const char * localname, const char * ns )
{
  WCHAR *pUnicodePrefix = NULL;
  WCHAR *pUnicodeLocalName = NULL;
  WCHAR *pUnicodeNameSpace = NULL;

  if ( prefix )    AnsiToUnicode( prefix, &pUnicodePrefix );
  if ( localname ) AnsiToUnicode( localname, &pUnicodeLocalName );
  if ( ns )        AnsiToUnicode( ns, &pUnicodeNameSpace );

  WriteStartElement( pUnicodePrefix, pUnicodeLocalName, pUnicodeNameSpace );

  if ( pUnicodePrefix )    free( pUnicodePrefix );
  if ( pUnicodeLocalName ) free( pUnicodeLocalName );
  if ( pUnicodeNameSpace ) free( pUnicodeNameSpace );
}

void CXmlWriter::WriteStartElement( const WCHAR * prefix, const WCHAR * localname, const WCHAR * ns )
{
  ns; prefix;                          // currently not used

  // close any open attribute and start element tag
  if ( GetCurElement() == CXmlWriter::Attribute ) 
  {
    WriteEndAttribute();
  } /* endif */

  if ( GetCurElement() == CXmlWriter::Tag && m_fOpenTag ) 
  {
    WriteRaw( L">" );
    m_fOpenTag = FALSE;
  } /* endif */

  if ( Formatting == Indented )
  {
    WriteRaw( L"\n" );
    WriteIndention( m_iCurIndention ); 
  } /* endif */
  m_iCurIndention += Indention;
  WriteRaw( L"<" );
  WriteRaw( localname );
  SetContentFlag();
  Push( CXmlWriter::Tag, localname );
  m_fOpenTag = TRUE;
  m_fLFBeforeEnd = FALSE;

}

void CXmlWriter::WriteElementString( const char * localname, const char * ns, const char * value )
{
  WCHAR *pUnicodeLocalName = NULL, *pUnicodeNameSpace = NULL, *pUnicodeValue = NULL;

  if ( localname ) AnsiToUnicode( localname, &pUnicodeLocalName );
  if ( ns )        AnsiToUnicode( ns, &pUnicodeNameSpace );
  if ( value )     AnsiToUnicode( value, &pUnicodeValue );

  WriteElementString( pUnicodeLocalName, pUnicodeNameSpace, pUnicodeValue );

  if ( pUnicodeValue )     free( pUnicodeValue );
  if ( pUnicodeLocalName ) free( pUnicodeLocalName );
  if ( pUnicodeNameSpace ) free( pUnicodeNameSpace );
}

void CXmlWriter::WriteElementString( const WCHAR * localname, const WCHAR * ns, const WCHAR * value )
{
  WriteStartElement( NULL, localname, ns );
  WriteString( value );
  WriteEndElement();
}

void CXmlWriter::WriteComment( const char * text )
{
  WCHAR *pUnicodeText = NULL;

  if ( text ) AnsiToUnicode( text, &pUnicodeText );

  WriteComment( pUnicodeText );

  if ( pUnicodeText ) free( pUnicodeText );
}

void CXmlWriter::WriteComment( const WCHAR *text )
{
  WriteRaw( L"<!--" );
  WriteStringInt( text );
  WriteRaw( L"-->\n" );
}

void CXmlWriter::WriteEndElement()
{
  enum _ElementType Type;
  WCHAR *pName;
  BOOL fShortEnd = FALSE;

  if ( GetCurElement() == CXmlWriter::Attribute ) 
  {
    WriteEndAttribute();
  } /* endif */

  if ( GetCurElement() == CXmlWriter::Tag && m_fOpenTag ) 
  {
    if  ( GetContentFlag() )
    {
      WriteRaw( L">" );
      m_fOpenTag = FALSE;
    }
    else
    {
      // write short end element
      WriteRaw( L" />" );
      m_fOpenTag = FALSE;
      fShortEnd = TRUE;
    } /* endif */

  } /* endif */

  if ( m_iStackSize )
  {
    Pop( &Type, &pName );
    m_iCurIndention -= Indention;
    if ( m_iCurIndention < 0 ) m_iCurIndention = 0;
    if ( !fShortEnd )
    {
      if ( (Formatting == Indented) && m_fLFBeforeEnd )
      {
        WriteRaw( L"\n" );
        WriteIndention( m_iCurIndention );
      } /* endif */
      WriteRaw( L"</" );
      WriteRaw( pName );
      WriteRaw( L">" );
    } /* endif */
    free( pName );
  } /* endif */
  m_fLFBeforeEnd = TRUE;
}

void CXmlWriter::WriteEndDocument()
{
  // close all open elements...
  if ( GetCurElement() == CXmlWriter::Attribute )
  {
    WriteEndAttribute();
  } /* endif */

  while ( m_iStackSize && (GetCurElement() == CXmlWriter::Tag) )
  {
    WriteEndElement();
  } /*endwhile */

  if ( GetCurElement() == CXmlWriter::Document )
  {
    enum _ElementType Type;
    WCHAR *pName;

    Pop( &Type, &pName );
    free( pName );
  } /* endif */

}

void CXmlWriter::Close()
{
  if ( m_hf )
  {
    fclose( m_hf );
  } /* endif */
}

int CXmlWriter::Push( enum _ElementType type, const WCHAR *name )
{
  struct _Element *pNewElement;

  pNewElement = (struct _Element *)malloc( sizeof(struct _Element) );
  memset( pNewElement, 0, sizeof(struct _Element) );
  pNewElement->ElementType = type;
  pNewElement->pPrev = m_pRoot;
  pNewElement->pName = _wcsdup( name );
  m_iStackSize++;
  m_pRoot = pNewElement;

  return( 0 );
}

int CXmlWriter::Pop( enum _ElementType *pType, WCHAR **ppName )
{
  int iResult = 0;
  struct _Element *pElement;

  if ( m_iStackSize && m_pRoot )
  {
    pElement = m_pRoot;
    *pType   = pElement->ElementType;
    *ppName = pElement->pName;
    m_iStackSize--;
    m_pRoot = pElement->pPrev ;
    free( pElement );
  }
  else
  {
    iResult = -1;
  } /* endif */

  return( iResult );
}

CXmlWriter::ElementType CXmlWriter::GetCurElement( void )
{
  enum _ElementType Type;

  if ( m_iStackSize && m_pRoot )
  {
    struct _Element *pElement = m_pRoot;
    Type = pElement->ElementType;
  }
  else
  {
    Type = CXmlWriter::Undefined;
  } /* endif */

  return( Type );
}

// set content flag of current stack element
void CXmlWriter::SetContentFlag( void )
{
  if ( m_iStackSize && m_pRoot )
  {
    struct _Element *pElement = m_pRoot;
    pElement->fContent = TRUE;
  } /* endif */
}

// set content flag of current stack element
BOOL CXmlWriter::GetContentFlag( void )
{
  BOOL fElementContent = TRUE;
  if ( m_iStackSize && m_pRoot )
  {
    fElementContent = m_pRoot->fContent;
  } /* endif */
  return( fElementContent );
}

// convert given string to Unicode, dynamically allocate buffer for Unicode string 
void CXmlWriter::AnsiToUnicode( const char *pAnsiText, WCHAR **ppUnicodeText )
{
  int iUnicodeLen = sizeof(WCHAR) * strlen(pAnsiText) + 10;
  WCHAR *pBuffer = (WCHAR *)malloc( iUnicodeLen );
  MultiByteToWideChar( CP_ACP, 0, pAnsiText, -1, pBuffer, iUnicodeLen ); 
  *ppUnicodeText = pBuffer;
}
