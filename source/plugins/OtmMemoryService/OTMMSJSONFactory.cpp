/*! \brief JSONize.CPP - Module with JSON related functions
	Copyright (c) 1999-2014, International Business Machines Corporation and others. All rights reserved.
	Description: This module contains functions to wrap parameters in the JSON format and to retrieve parameters from JSON strings
*/

#include <windows.h>
#include <winbase.h>
#include "OTMMSJSONFactory.h"

/** Initialize the static instance variable */
JSONFactory* JSONFactory::instance = 0;


/*! \brief This static method returns a pointer to the JSONFactory object.
	The first call of the method creates the MemoryFactory instance.
*/
JSONFactory* JSONFactory::getInstance()
{
	if (instance == 0)
	{
		instance = new JSONFactory();
	}
	return instance;
}

/*! \brief Starts a JSON string

    This method starts a JSON string

    \param JSONString reference to a string receiving the JSON start sequence

  	\returns 0 or error code in case of errors
*/
  int JSONFactory::startJSON
  (
    std::string &JSONString
  )
  {
    JSONString = "{";
    return( 0 );
  }

  int JSONFactory::startJSONW
  (
    std::wstring &JSONString
  )
  {
    JSONString = L"{";
    return( 0 );
  }

/*! \brief Terminates a JSON string

    This method terminates a JSON string

    \param JSONString reference to a string receiving the JSON end sequence

  	\returns 0 or error code in case of errors
*/
  int JSONFactory::terminateJSON
  (
    std::string &JSONString
  )
  {
    JSONString.append( "}" );
    return( 0 );
  }

  int JSONFactory::terminateJSONW
  (
    std::wstring &JSONString
  )
  {
    JSONString.append( L"}" );
    return( 0 );
  }


/*! \brief Adds a parameter to a JSON string

    This method adds a parameter to a JSON string

    \param JSONString reference to a string receiving the parameter
    \param name parameter name
    \param value parameter value

  	\returns 0 or error code in case of errors
*/
  int JSONFactory::addParmToJSON
  (
    std::string &JSONString,
    const std::string &name,
    const std::string &value
  )
  {
    // add delimiter to string when this is not the first parameter added
    if ( JSONString.size() > 1 )
    {
      JSONString.append( "," );
    } /* end */       

    // add parameter to string
    this->addString( JSONString, name );
    JSONString.append( ":" );
    this->addString( JSONString, value );
    return( 0 );
  }

  int JSONFactory::addParmToJSON
  (
    std::string &JSONString,
    const std::string &name,
    int iValue
  )
  {
    char szValue[20];

    _itoa( iValue, szValue, 10 );

    addDelimiter( JSONString );

    // add parameter to string
    this->addString( JSONString, name );
    JSONString.append( ":" );
    JSONString.append( szValue );

    return( 0 );
  }

  /*! \brief convert a ASCII std::string to a UTF16 std::wstring
  \param strUTF8String string in UTF8 encoding
  \returns string converted to UTF16
  */
  std::wstring convertToUTF16( const std::string& strASCII )
  {
    int iUTF16Len;
    int iLen = (int)strASCII.length() + 1;
    iUTF16Len = MultiByteToWideChar( CP_OEMCP, 0, strASCII.c_str(), iLen, 0, 0 );
    wchar_t *pszUtf16String = new ( wchar_t[iUTF16Len] );
    MultiByteToWideChar( CP_OEMCP, 0, strASCII.c_str(), iLen, pszUtf16String, iUTF16Len );
    std::wstring strUTF16 = pszUtf16String;
    delete( pszUtf16String );
    return strUTF16;
  }


  int JSONFactory::addParmToJSONW
  (
    std::wstring &JSONString,
    const std::wstring &name,
    const std::string &value
  )
  {
    std::wstring strValueW = convertToUTF16( value );
    int iRC = addParmToJSONW( JSONString, name, strValueW );
    return( iRC );
  }

  int JSONFactory::addParmToJSONW
  (
    std::wstring &JSONString,
    const std::wstring &name,
    const std::wstring &value
  )
  {
    addDelimiterW( JSONString );

    // add parameter to string
    this->addStringW( JSONString, name );
    JSONString.append( L":" );
    this->addStringW( JSONString, value );
    return( 0 );
  }

  int JSONFactory::addParmToJSONW
  (
    std::wstring &JSONString,
    const std::wstring &name
  )
  {
    addDelimiterW( JSONString );

    // add parameter to string
    this->addStringW( JSONString, name );
    JSONString.append( L":" );
    JSONString.append( L"null" );
    return( 0 );
  }

  int JSONFactory::addParmToJSONW
  (
    std::wstring &JSONString,
    const std::wstring &name,
    int iValue
  )
  {
    wchar_t szValue[20];

    _itow( iValue, szValue, 10 );

    addDelimiterW( JSONString );

    // add parameter to string
    this->addStringW( JSONString, name );
    JSONString.append( L":" );
    JSONString.append( szValue );

    return( 0 );
  }




  /*! \brief adds a string to a JSON string and escapes double quotes

    This method adds a parameter to a JSON string

    \param JSONString reference to a string receiving the parameter
    \param str string being added to the JSON string

  	\returns 0 or error code in case of errors
*/
  int JSONFactory::addString
  (
    std::string &JSONString,
    const std::string &str
  )
  {
    std::string newString = "\"";
    int iStart = 0;
    int iPos = 0;
    for ( iPos = 0; iPos < (int)str.size(); iPos++ )
    {
      if ( str[iPos] == '\"' )
      {
        // add data up to double quote
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( "\\\"" );
          iStart = iPos + 1;
        } /* endif */           
      } /* endif */         
      else if(str[iPos] == '\\')
      {   
        // escape the backslash
        if ( iStart < iPos )
        {
            newString.append( str.substr( iStart, iPos - iStart ) );
            newString.append( "\\\\" );
            iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == '\n' )
      {
        // escape the lf
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( "\\n" );
          iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == '\r' )
      {
        // escape the cr
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( "\\r" );
          iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == '/' )
      {
        // escape the slash
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( "\\/" );
          iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == '\t' )
      {
        // escape the tab
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( "\\t" );
          iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == '\f' )
      {
        // escape the formfeed
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( "\\f" );
          iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == (char)0xA0 )
      {
        // escape the non breaking space
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( "\\u00A0" );
          iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == '\b' )
      {
        // escape the backspace
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( "\\b" );
          iStart = iPos + 1;
        }
      }
    } /* endfor */
    // add remaining data
    if ( iStart < (int)str.size() )
    {
      newString.append( str.substr( iStart ) );
    } /* endif */           
    newString.append( "\"" );
    JSONString.append( newString );

    return( 0 );
  }

  int JSONFactory::addStringW
  (
    std::wstring &JSONString,
    const std::wstring &str
  )
  {
    std::wstring newString = L"\"";
    int iStart = 0;
    int iPos = 0;
    for ( iPos = 0; iPos < (int)str.size(); iPos++ )
    {
      if ( str[iPos] == L'\"' )
      {
        // add data up to double quote
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( L"\\\"" );
          iStart = iPos + 1;
        } /* endif */           
      } /* endif */         
      else if(str[iPos] == L'\\')
      {   
        // escape the backslash
        if ( iStart < iPos )
        {
            newString.append( str.substr( iStart, iPos - iStart ) );
            newString.append( L"\\\\" );
            iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == L'\n' )
      {
        // escape the lf
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( L"\\n" );
          iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == L'\r' )
      {
        // escape the cr
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( L"\\r" );
          iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == L'/' )
      {
        // escape the slash
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( L"\\/" );
          iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == L'\t' )
      {
        // escape the tab
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( L"\\t" );
          iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == L'\f' )
      {
        // escape the formfeed
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( L"\\f" );
          iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == (wchar_t)0xA0 )
      {
        // escape the non breaking space
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( L"\\u00A0" );
          iStart = iPos + 1;
        }
      }
      else if ( str[iPos] == L'\b' )
      {
        // escape the backspace
        if ( iStart < iPos )
        {
          newString.append( str.substr( iStart, iPos - iStart ) );
          newString.append( L"\\b" );
          iStart = iPos + 1;
        }
      }
    } /* endfor */
    // add remaining data
    if ( iStart < (int)str.size() )
    {
      newString.append( str.substr( iStart ) );
    } /* endif */           
    newString.append( L"\"" );
    JSONString.append( newString );

    return( 0 );
  }


/*! \brief Data area for parsing JSON strings
*/
typedef struct _JSONPARSEDATA
{
  std::string JSONString;    // string being parsed
  int         iPos;          // current position within the string
  int         iSize;         // overall length of JSON string
} JSONPARSEDATA, *PJSONPARSEDATA;

/*! \brief Data area for parsing JSON strings (widechar version)
*/
typedef struct _JSONPARSEDATAW
{
  wchar_t     *pszJSONString;// string being parsed 
  int         iPos;          // current position within the string
  int         iSize;         // overall length of JSON string
  int         iCurlyBraces;  // curly brace nesting level
  int         iBrackets;     // brackets nesting level
} JSONPARSEDATAW, *PJSONPARSEDATAW;

/*! \brief Start parsing of a JSON string

    This method initializes the parsing of a JSON string 

    \param JSONStringIn reference to a JSON string 

  	\returns handle of a JSON string parser instance or NULL in case of errors
*/
void *JSONFactory::parseJSONStart
(
  std::string &JSONStringIn,
  int *piRC
)
{
  // check JSON string
  int iSize = JSONStringIn.size();

  // check for enclosing curly braces
  int iPos = 0;
  while ( ( iPos < iSize ) && isspace( JSONStringIn[iPos] ) ) iPos++;
  if ( ( iPos >= iSize ) || ( JSONStringIn[iPos] != '{' ) )
  {
    if ( piRC != NULL ) *piRC = 1;
    return( NULL );
  } /* endif */
  int iEnd = iSize - 1;
  while ( ( iEnd > iPos ) && (isspace( JSONStringIn[iEnd]) || (JSONStringIn[iEnd] == 0)) ) iEnd--;
  if ( ( iEnd <= iPos ) || ( JSONStringIn[iEnd] != '}' ) )
  {
    if ( piRC != NULL ) *piRC = 2;
    return( NULL );
  } /* endif */


  // prepare parsing
  PJSONPARSEDATA pData = new(JSONPARSEDATA);
  if ( pData != NULL )
  {
    pData->JSONString = JSONStringIn; 
    pData->iPos = iPos + 1;
    pData->iSize = iSize;
    while ( isspace( pData->JSONString[pData->iPos] )  ) pData->iPos++;
  } /* endif */     
  return( (void *)pData );
}

void *JSONFactory::parseJSONStartW
(
  std::wstring &JSONStringIn,
  int *piRC
)
{
  // check JSON string
  int iSize = JSONStringIn.length();

  // check for enclosing curly braces
  int iPos = 0;
  while ( (iPos < iSize) && iswspace(JSONStringIn[iPos])) iPos++;
  if ( (iPos >= iSize) || (JSONStringIn[iPos] != L'{') )
  {
    if ( piRC != NULL ) *piRC = 1;
    return( NULL );
  } /* endif */     
  int iEnd = iSize - 1;
  while ( ( iEnd > iPos ) && ( iswspace( JSONStringIn[iEnd] ) || ( JSONStringIn[iEnd] == 0 ) ) ) iEnd--;
  if ( (iEnd <= iPos) || (JSONStringIn[iEnd] != L'}') )
  {
    if ( piRC != NULL ) *piRC = 2;
    return( NULL );
  } /* endif */     

  // prepare parsing
  PJSONPARSEDATAW pData = new(JSONPARSEDATAW);
  if ( pData != NULL )
  {
    pData->pszJSONString = new( wchar_t[iSize + 1] );
    wcscpy( pData->pszJSONString, JSONStringIn.c_str() );
    pData->iPos = iPos + 1;
    pData->iSize = iEnd;
    pData->iBrackets = 0;
    pData->iCurlyBraces = 0;
    while ( (pData->iPos < pData->iSize) && iswspace(pData->pszJSONString[pData->iPos]) ) pData->iPos++;
  } /* endif */     
  return( (void *)pData );
}


/*! \brief Stop parsing of a JSON string

    This method ends the parsing of a JSON string and all internal resources are freed.
    The parser handle is not valid anymore.

    \param pvParseHandle JSON string parser handle as returned by parseJSONStart
*/
void JSONFactory::parseJSONStop
(
  void *pvParseHandle
)
{
  if ( pvParseHandle != NULL )
  {
    PJSONPARSEDATA pData = (PJSONPARSEDATA)pvParseHandle;
    free( pData );
  } /* endif */     
}

void JSONFactory::parseJSONStopW
(
  void *pvParseHandle
)
{
  if ( pvParseHandle != NULL )
  {
    PJSONPARSEDATAW pData = (PJSONPARSEDATAW)pvParseHandle;
    if ( pData->pszJSONString ) free( pData->pszJSONString );
    free( pData );
  } /* endif */     
}


/*! \brief Get the next parameter from a JSON string

    This method retrieves the next parameter and its value from a JSON string. When called
    for the first time first parameter of the JSON string is retrieved.

    \param pvParseHandle JSON string parser handle as returned by parseJSONStart
    \param strName reference to a string receiving the parameter name
    \param strValue reference to a string receiving the parameter value
    \returns 0 if successful or return code
*/
int JSONFactory::parseJSONGetNext
(
  void *pvParseHandle,
  std::string &strName,
  std::string &strValue
)
{
  int iRC = 0;

  if ( pvParseHandle == NULL ) return( ERROR_INVALIDPARSEHANDLE );

  PJSONPARSEDATA pData = (PJSONPARSEDATA)pvParseHandle;

  // skip whitespace up to start of name
  while ( isspace( pData->JSONString[pData->iPos] ) )  pData->iPos++;

  // check for end of parameters
  if ( pData->JSONString[pData->iPos] == '}' ) return( INFO_ENDOFPARAMETERLISTREACHED );

  // iPos should now point to start of parameter name enclosed in double quotes
  if ( pData->JSONString[pData->iPos] != '\"' ) return( ERROR_INVALIDJSONSYNTAX );

  // extract parameter name
  iRC = this->extractString( pvParseHandle, strName );
  if ( iRC != 0 ) return( iRC );

  // iPos should now point to name/value separator 
  while ( isspace( pData->JSONString[pData->iPos] ) )  pData->iPos++;
  if ( pData->JSONString[pData->iPos] != ':' ) return( ERROR_INVALIDJSONSYNTAX );
  pData->iPos++;
  while ( isspace( pData->JSONString[pData->iPos] ) )  pData->iPos++;

  // extract parameter value
  iRC = this->extractString( pvParseHandle, strValue );

  // skip any parameter separator
  while ( isspace( pData->JSONString[pData->iPos] ) )  pData->iPos++;
  if ( pData->JSONString[pData->iPos] == ',' ) pData->iPos++;
  
  return( iRC );
}

int JSONFactory::parseJSONGetNextW
(
  void *pvParseHandle,
  std::wstring &strName,
  std::wstring &strValue
)
{
  int iRC = 0;

  if ( pvParseHandle == NULL ) return( ERROR_INVALIDPARSEHANDLE );

  PJSONPARSEDATAW pData = (PJSONPARSEDATAW)pvParseHandle;

  // skip whitespace up to start of name
  while ( iswspace(pData->pszJSONString[pData->iPos]) )  pData->iPos++;

  // handle ending curly braces
  while( pData->pszJSONString[pData->iPos] == L'}' )
  {
    if ( pData->iCurlyBraces == 0 )
    {
      return( INFO_ENDOFPARAMETERLISTREACHED );
    }
    else
    {
      pData->iCurlyBraces--;
      pData->iPos++;
      while ( iswspace(pData->pszJSONString[pData->iPos]) )  pData->iPos++;

      // skip any following comma
      if ( pData->pszJSONString[pData->iPos] == ',' )  pData->iPos++;
      while ( iswspace( pData->pszJSONString[pData->iPos] ) )  pData->iPos++;
    }
  } /* endwhile */

  // handle beginning curly brace
  if ( pData->pszJSONString[pData->iPos] == L'{' )
  {
    pData->iCurlyBraces++;
    pData->iPos++;
    return( INFO_BEGINOFELEMENTDETECTED );
  }

  // iPos should now point to start of parameter name enclosed in double quotes
  while ( iswspace(pData->pszJSONString[pData->iPos]) )  pData->iPos++;
  if ( pData->pszJSONString[pData->iPos] != L'\"' ) return( ERROR_INVALIDJSONSYNTAX );

  // extract parameter name
  iRC = this->extractStringW( pvParseHandle, strName );
  if ( iRC != 0 ) return( iRC );

  // iPos should now point to name/value separator 
  while ( iswspace(pData->pszJSONString[pData->iPos]) )  pData->iPos++;
  if ( pData->pszJSONString[pData->iPos] != L':' ) return( ERROR_INVALIDJSONSYNTAX );
  pData->iPos++;

  // handle beginning curly brace
  while ( iswspace(pData->pszJSONString[pData->iPos]) )  pData->iPos++;
  if ( pData->pszJSONString[pData->iPos] == L'{' )
  {
    pData->iCurlyBraces++;
    pData->iPos++;
    return( INFO_BEGINOFELEMENTDETECTED );
  }

  // extract parameter value
  iRC = this->extractStringW( pvParseHandle, strValue );

  // skip any parameter separator
  while ( iswspace( pData->pszJSONString[pData->iPos] ) )  pData->iPos++;
  if ( pData->pszJSONString[pData->iPos] == L',' ) pData->iPos++;
  
  return( iRC );
}


/*! \brief Extract a string enclosed in double quotes or delimited by comma or curly brace

    This method extracts a string enclosed in double quotes and handles any
    escaped double quotes inside the string, the iPos parameter is increased
    to the next character following the ending double quote of the string

    \param pData pointer to JSON string parser data area
    \param string reference to a string receiving the parameter name
    \returns 0 if successful or return code
*/
int JSONFactory::extractString
(
  void *pvData,
  std::string &string
)
{
  PJSONPARSEDATA pData = (PJSONPARSEDATA)pvData;

  if ( pData->JSONString[pData->iPos] == '\"'  )
  {
    // skip starting double quote
    pData->iPos++;

    // find string end delimiter and evaluate length of string
    int iTargetLen = 0;
    int iCurPos = pData->iPos;
    while ( (iCurPos < pData->iSize) && (pData->JSONString[iCurPos] != '\"') )
    {
      if ( pData->JSONString[iCurPos] == '\\' )
      {
        char chNextChar = pData->JSONString[iCurPos + 1];
        switch ( chNextChar )
        {
          case '\\':
          case '\"':
          case 'n':
          case 'r':
          case '/':
          case 'b':
          case 't':
          case 'f':
            iCurPos += 2;
            iTargetLen++;
            break;
          case 'u':
            iCurPos += 6;
            iTargetLen += 6; // the UTF8 character may need up to 6 bytes
            break;
          default:
            // treat as normal character
            iCurPos++; iTargetLen++;
        }
      }
      else
      {
        // treat as normal character
        iCurPos++; iTargetLen++;
      }
    } /* endwhile */ 

    // verify that string end is indicated by a double quote
    if ( pData->JSONString[iCurPos] != '\"' ) return( ERROR_INVALIDJSONSYNTAX );

    // copy characters to target string and unescape escaped characters
    string.resize( iTargetLen, ' ' );
    int iTargetPos = 0;
    while ( pData->iPos < iCurPos )
    {
      if ( pData->JSONString[pData->iPos] == '\\' )
      {
        char chNextChar = pData->JSONString[pData->iPos + 1];
        switch ( chNextChar )
        {
          case '\\':
            pData->iPos += 2;
            string[iTargetPos++] = '\\';
            break;
          case '\"':
            pData->iPos += 2;
            string[iTargetPos++] = '\"';
            break;
          case 'n':
            pData->iPos += 2;
            string[iTargetPos++] = '\n';
            break;
          case 'r':
            pData->iPos += 2;
            string[iTargetPos++] = '\r';
            break;
          case '/':
            pData->iPos += 2;
            string[iTargetPos++] = '/';
            break;
          case 'b':
            pData->iPos += 2;
            string[iTargetPos++] = '\b';
            break;
          case 't':
            pData->iPos += 2;
            string[iTargetPos++] = '\t';
            break;
          case 'f':
            pData->iPos += 2;
            string[iTargetPos++] = '\f';
            break;
          case 'u':
            {
              // build wchar string with this and any following code point and convert the string to UTF8
              std::wstring strUTF16;
              int iLen = 0;

              while ( ((pData->iPos + 3) < iCurPos) && ( pData->JSONString[pData->iPos] == '\\' ) && ( pData->JSONString[pData->iPos + 1] == 'u' ) )
              {
                char szHexNumber[20];
                int iHexNumberLen = 0;
                pData->iPos += 2;
                while ( ( pData->iPos < iCurPos ) && ( iHexNumberLen < 4) )
                {
                  szHexNumber[iHexNumberLen++] = pData->JSONString[pData->iPos++];
                }
                szHexNumber[iHexNumberLen] = 0;
                wchar_t c = (wchar_t)strtol( szHexNumber, 0, 16 );
                strUTF16.resize( iLen + 1);
                strUTF16[iLen++] = c;
              }

              // convert to UTF8
              int iUTF8Len;
              iUTF8Len = WideCharToMultiByte( CP_UTF8, 0, strUTF16.c_str(), iLen, 0, 0, 0, 0 );
              std::string strUTF8( iUTF8Len, '\0' );
              WideCharToMultiByte( CP_UTF8, 0, strUTF16.c_str(), iLen, &strUTF8[0], iUTF8Len, 0, 0 );

              // add UTF8 character to target string
              for ( int i = 0; i < iUTF8Len; i++ ) string[iTargetPos++] = strUTF8[i];
            }
            break;
          default:
            // treat as normal character
            string[iTargetPos++] = pData->JSONString[pData->iPos++];
        }
      }
      else
      {
        // handle normal character
        string[iTargetPos++] = pData->JSONString[pData->iPos++];
      } /* end */       
    } /* endwhile */ 

    // skip closing double quote
    pData->iPos++;
  }
  else
  {
    // find end delimiter and evaluate length of string
    int iTargetLen = 0;
    int iCurPos = pData->iPos;
    while ( (iCurPos < pData->iSize) && (pData->JSONString[iCurPos] != '}') && (pData->JSONString[iCurPos] != ','))
    {
      iCurPos++; iTargetLen++; 
    } /* endwhile */ 

    // copy characters to target string
    string.resize( iTargetLen, ' ' );
    int iTargetPos = 0;
    while ( pData->iPos < iCurPos )
    {
      string[iTargetPos++] = pData->JSONString[pData->iPos++];
    } /* endwhile */ 
  } /* endif */     

  return( 0 );
}

int JSONFactory::extractStringW
(
  void *pvData,
  std::wstring &string
)
{
  PJSONPARSEDATAW pData = (PJSONPARSEDATAW)pvData;

  if ( pData->pszJSONString[pData->iPos] == L'\"'  )
  {
    // skip starting double quote
    pData->iPos++;

    // find string end delimiter and evaluate length of string
    int iTargetLen = 0;
    int iCurPos = pData->iPos;
    while ( ( iCurPos < pData->iSize ) && ( pData->pszJSONString[iCurPos] != L'\"' ) )
    {
      if ( pData->pszJSONString[iCurPos] == L'\\' )
      {
        wchar_t chNextChar = pData->pszJSONString[iCurPos + 1];
        switch ( chNextChar )
        {
          case L'\\':
          case L'\"':
          case L'n':
          case L'r':
          case L'/':
          case L'b':
          case L't':
          case L'f':
            iCurPos += 2;
            iTargetLen++;
            break;
          case L'u':
            iCurPos += 6;
            iTargetLen++; // the UTF16 code point needs one wchar
            break;
          default:
            // treat as normal character
            iCurPos++; iTargetLen++;
        }
      }
      else
      {
        // treat as normal character
        iCurPos++; iTargetLen++;
      }
    } /* endwhile */ 

    // verify that string end is indicated by a double quote
    if ( pData->pszJSONString[iCurPos] != L'\"' ) return( ERROR_INVALIDJSONSYNTAX );

    // copy characters to target string and unescape escaped characters
    string.resize( iTargetLen, ' ' );
    int iTargetPos = 0;
    while ( pData->iPos < iCurPos )
    {
      if ( pData->pszJSONString[pData->iPos] == L'\\' )
      {
        wchar_t chNextChar = pData->pszJSONString[pData->iPos + 1];
        switch ( chNextChar )
        {
          case L'\\':
            pData->iPos += 2;
            string[iTargetPos++] = L'\\';
            break;
          case L'\"':
            pData->iPos += 2;
            string[iTargetPos++] = L'\"';
            break;
          case L'n':
            pData->iPos += 2;
            string[iTargetPos++] = L'\n';
            break;
          case L'r':
            pData->iPos += 2;
            string[iTargetPos++] = L'\r';
            break;
          case L'/':
            pData->iPos += 2;
            string[iTargetPos++] = '/';
            break;
          case L'b':
            pData->iPos += 2;
            string[iTargetPos++] = L'\b';
            break;
          case L't':
            pData->iPos += 2;
            string[iTargetPos++] = L'\t';
            break;
          case L'f':
            pData->iPos += 2;
            string[iTargetPos++] = L'\f';
            break;
          case L'u':
          {
            wchar_t szHexNumber[20];
            int iHexNumberLen = 0;
            pData->iPos += 2;
            while ( ( pData->iPos < iCurPos ) && ( iHexNumberLen < 4 ) )
            {
              szHexNumber[iHexNumberLen++] = pData->pszJSONString[pData->iPos++];
            }
            szHexNumber[iHexNumberLen] = 0;
            wchar_t c = (wchar_t)wcstol( szHexNumber, 0, 16 );
            string[iTargetPos++] = c;
          }
          break;
          default:
            // treat as normal character
            string[iTargetPos++] = pData->pszJSONString[pData->iPos++];
        }
      }
      else
      {
        // handle normal character
        string[iTargetPos++] = pData->pszJSONString[pData->iPos++];
      } /* end */
    } /* endwhile */

    // skip closing double quote
    pData->iPos++;
  }
  else
  {
    // find end delimiter and evaluate length of string
    int iTargetLen = 0;
    int iCurPos = pData->iPos;
    while ( (iCurPos < pData->iSize) && (pData->pszJSONString[iCurPos] != '}') && (pData->pszJSONString[iCurPos] != ','))
    {
      iCurPos++; iTargetLen++; 
    } /* endwhile */ 

    // copy characters to target string
    string.resize( iTargetLen, ' ' );
    int iTargetPos = 0;
    while ( pData->iPos < iCurPos )
    {
      string[iTargetPos++] = pData->pszJSONString[pData->iPos++];
    } /* endwhile */ 

    // special handling for the value "null"
    if ( wcsicmp( string.c_str(), L"null" ) == 0 )
    {
      string = L"";
    }

  } /* endif */     

  return( 0 );
}

/*! \brief Parameter types for the entries of a JSON parse control table
*/
  typedef enum 
  {
    ASCII_STRING_PARM_TYPE,
    UTF8_STRING_PARM_TYPE,
    UTF16_STRING_PARM_TYPE,
    INT_PARM_TYPE
  } PARMTYPE;

/*! \brief Entry of a JSON parse control table
*/
  typedef struct _JSONPARSECONTROL
  {
    wchar_t szName[40];      // name of the parameter
    PARMTYPE type;           // type of the parameter
    void *pvValue;           // pointer to a variable receiving the value
  } JSONPARSECONTROL, *PJSONPARSECONTROL;

/*! \brief Parses a JSON string using the supplied control table

    This method parses a JSON string using the provided control table

    \param JSONString reference to a JSON string 

    \param pParserControl pointer to table containing the information on the parameters to be extracted

  	\returns 0 when successful or an error code
*/
int JSONFactory::parseJSON
(
  std::wstring &JSONString,
  PJSONPARSECONTROL pParserControl
)
{
  int iRC = 0;

  void *parseHandle = parseJSONStartW( JSONString, &iRC );
  if ( parseHandle == NULL )
  {
    return( iRC );
  } /* end */       

  

  while ( iRC == 0 )
  {
    std::wstring name;
    std::wstring value;
    iRC = parseJSONGetNextW( parseHandle, name, value );
    if ( iRC == 0 )
    {
      PJSONPARSECONTROL pParm = pParserControl;
      bool fFound = false;

      while( !fFound && (pParm->szName[0] != 0) )
      {
        if ( wcsicmp( name.c_str(), pParm->szName ) == 0 )
        {
          fFound = true;
        }
        else
        {
          pParm++;
        } /* endif */
      } /* endwhile */

      if ( fFound )
      {
        switch ( pParm->type )
        {
          case ASCII_STRING_PARM_TYPE:
            WideCharToMultiByte( CP_OEMCP, 0, value.c_str(), -1, (char *)pParm->pvValue, pParm->iBufferLen - 1, NULL, NULL );
            *(((char *)pParm->pvValue) + (pParm->iBufferLen - 1)) = 0;
            break;

          case UTF8_STRING_PARM_TYPE:
            WideCharToMultiByte( CP_UTF8, 0, value.c_str(), -1, (char *)pParm->pvValue, pParm->iBufferLen - 1, NULL, NULL );
            *(((char *)pParm->pvValue) + (pParm->iBufferLen - 1)) = 0;
            break;

          case UTF16_STRING_PARM_TYPE:
            wcsncpy( (wchar_t *)pParm->pvValue, value.c_str(), pParm->iBufferLen - 1 );
            *(((wchar_t *)pParm->pvValue) + (pParm->iBufferLen - 1)) = 0;
            break;

          case INT_PARM_TYPE:
            *((int *)pParm->pvValue) = _wtol( value.c_str() );
            break;
        } /* endswitch */
      } /* endif */
    } /* endif */       

    // reset iRC for specific codes
    if ( iRC == INFO_BEGINOFELEMENTDETECTED )
    {
      iRC = 0;
    } /* endif */
  } /* endwhile */    

  parseJSONStopW( parseHandle );

  return( (iRC == INFO_ENDOFPARAMETERLISTREACHED) ? 0 : iRC );
}

/*! \brief Add a element name to a JSON string
  	\returns 0 or error code in case of errors
*/
int JSONFactory::addNameToJSONW
(
  std::wstring &JSONString,
  const std::wstring &name
)
{
  addDelimiterW( JSONString );
  this->addStringW( JSONString, name );
  JSONString.append( L":" );
  return( 0 );
}


/*! \brief Add the start identifier for an array to a JSON string
  	\returns 0 or error code in case of errors
*/
int JSONFactory::addArrayStartToJSONW
(
  std::wstring &JSONString
)
{
  addDelimiterW( JSONString );
  JSONString.append( L"[" );

  return( 0 );
}

/*! \brief Add the end identifier for an array to a JSON string
  	\returns 0 or error code in case of errors
*/
int JSONFactory::addArrayEndToJSONW
(
  std::wstring &JSONString
)
{
  JSONString.append( L"]" );

  return( 0 );
}

/*! \brief Add the start identifier for an element to a JSON string
  	\returns 0 or error code in case of errors
*/
int JSONFactory::addElementStartToJSONW
(
  std::wstring &JSONString
)
{
  addDelimiterW( JSONString );
  JSONString.append( L"{" );

  return( 0 );
}


/*! \brief Add the end identifier for an element to a JSON string
  	\returns 0 or error code in case of errors
*/
int JSONFactory::addElementEndToJSONW
(
  std::wstring &JSONString
)
{
  JSONString.append( L"}" );

  return( 0 );
}


/*! \brief Add the delimiter (komma) to the JSON string when needed
  	\returns 0 or error code in case of errors
*/
int JSONFactory::addDelimiterW
(
  std::wstring &JSONString
)
{
  // add a komma delimiter to the JSON string if previous character is not a curly brace, a bracket, or a colon
  int iEnd = JSONString.size();
  while ( (iEnd > 0) && (JSONString[iEnd-1] == L' ') ) iEnd--;
  if ( iEnd > 0 )
  {
    if ( (JSONString[iEnd-1] != L'{') && (JSONString[iEnd-1] != L'[') && (JSONString[iEnd-1] != L':'))
    {
      JSONString.append( L"," );
    } /* endif */
  } /* endif */
  return( 0 );
}

/*! \brief Add the delimiter (komma) to the JSON string when needed
\returns 0 or error code in case of errors
*/
int JSONFactory::addDelimiter
(
  std::string &JSONString
)
{
  // add a komma delimiter to the JSON string if previous character is not a curly brace, a bracket, or a colon
  int iEnd = JSONString.size();
  while ( ( iEnd > 0 ) && ( JSONString[iEnd - 1] == ' ' ) ) iEnd--;
  if ( iEnd > 0 )
  {
    if ( ( JSONString[iEnd - 1] != '{' ) && ( JSONString[iEnd - 1] != '[' ) && ( JSONString[iEnd - 1] != ':' ) )
    {
      JSONString.append( "," );
    } /* endif */
  } /* endif */
  return( 0 );
}


