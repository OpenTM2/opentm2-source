/*! \brief JSONize.CPP - Module with JSON related functions
	Copyright (c) 1999-2014, International Business Machines Corporation and others. All rights reserved.
	Description: This module contains functions to wrap parameters in the JSON format and to retrieve parameters from JSON strings
*/

#include <windows.h>
#include <winbase.h>
#include "JSONFactory.h"

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
          //escap the file seprator,otherwise JSON can't parse
          if ( iStart < iPos )
          {
              newString.append( str.substr( iStart, iPos - iStart ) );
              newString.append( "\\\\" );
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
          //escap the file seprator,otherwise JSON can't parse
          if ( iStart < iPos )
          {
              newString.append( str.substr( iStart, iPos - iStart ) );
              newString.append( L"\\\\" );
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
  std::wstring JSONString;   // string being parsed 
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
  std::string &JSONStringIn
)
{
  // check JSON string
  int iSize = JSONStringIn.size();
  if ( (iSize < 2) || (JSONStringIn[0] != '{') || (JSONStringIn[iSize-1] != '}')  )
  {
    return( NULL );
  } /* endif */     

  // prepare parsing
  PJSONPARSEDATA pData = new(JSONPARSEDATA);
  if ( pData != NULL )
  {
    pData->JSONString = JSONStringIn; 
    pData->iPos = 1;
    pData->iSize = iSize;
    while ( pData->JSONString[pData->iPos] == ' ' )  pData->iPos++;
  } /* endif */     
  return( (void *)pData );
}

void *JSONFactory::parseJSONStartW
(
  std::wstring &JSONStringIn
)
{
  // check JSON string
  int iSize = JSONStringIn.size();

  // check for enclosing curly braces
  int iPos = 0;
  while ( (iPos < iSize) && iswspace(JSONStringIn[iPos])) iPos++;
  if ( (iPos >= iSize) || (JSONStringIn[iPos] != L'{') )
  {
    return( NULL );
  } /* endif */     
  int iEnd = iSize - 1;
  while ( (iEnd > iPos) && iswspace(JSONStringIn[iEnd])) iEnd--;
  if ( (iEnd <= iPos) || (JSONStringIn[iEnd] != L'}') )
  {
    return( NULL );
  } /* endif */     

  // prepare parsing
  PJSONPARSEDATAW pData = new(JSONPARSEDATAW);
  if ( pData != NULL )
  {
    pData->JSONString = JSONStringIn; 
    pData->iPos = iPos + 1;
    pData->iSize = iEnd;
    pData->iBrackets = 0;
    pData->iCurlyBraces = 0;
    while ( (pData->iPos < pData->iSize) && iswspace(pData->JSONString[pData->iPos]) ) pData->iPos++;
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
  while ( pData->JSONString[pData->iPos] == ' ' )  pData->iPos++;

  // check for end of parameters
  if ( pData->JSONString[pData->iPos] == '}' ) return( INFO_ENDOFPARAMETERLISTREACHED );

  // iPos should now point to start of parameter name enclosed in double quotes
  if ( pData->JSONString[pData->iPos] != '\"' ) return( ERROR_INVALIDJSONSYNTAX );

  // extract parameter name
  iRC = this->extractString( pvParseHandle, strName );
  if ( iRC != 0 ) return( iRC );

  // iPos should now point to name/value separator 
  if ( pData->JSONString[pData->iPos] != ':' ) return( ERROR_INVALIDJSONSYNTAX );
  pData->iPos++;

  // extract parameter value
  iRC = this->extractString( pvParseHandle, strValue );

  // skip any parameter separator
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
  while ( iswspace(pData->JSONString[pData->iPos]) )  pData->iPos++;

  // handle ending curly braces
  while( pData->JSONString[pData->iPos] == L'}' )
  {
    if ( pData->iCurlyBraces == 0 )
    {
      return( INFO_ENDOFPARAMETERLISTREACHED );
    }
    else
    {
      pData->iCurlyBraces--;
      pData->iPos++;
      while ( iswspace(pData->JSONString[pData->iPos]) )  pData->iPos++;
    }
  } /* endwhile */

  // handle beginning curly brace
  if ( pData->JSONString[pData->iPos] == L'{' )
  {
    pData->iCurlyBraces++;
    pData->iPos++;
    return( INFO_BEGINOFELEMENTDETECTED );
  }

  // iPos should now point to start of parameter name enclosed in double quotes
  while ( iswspace(pData->JSONString[pData->iPos]) )  pData->iPos++;
  if ( pData->JSONString[pData->iPos] != L'\"' ) return( ERROR_INVALIDJSONSYNTAX );

  // extract parameter name
  iRC = this->extractStringW( pvParseHandle, strName );
  if ( iRC != 0 ) return( iRC );

  // iPos should now point to name/value separator 
  while ( iswspace(pData->JSONString[pData->iPos]) )  pData->iPos++;
  if ( pData->JSONString[pData->iPos] != L':' ) return( ERROR_INVALIDJSONSYNTAX );
  pData->iPos++;

  // handle beginning curly brace
  while ( iswspace(pData->JSONString[pData->iPos]) )  pData->iPos++;
  if ( pData->JSONString[pData->iPos] == L'{' )
  {
    pData->iCurlyBraces++;
    pData->iPos++;
    return( INFO_BEGINOFELEMENTDETECTED );
  }

  // extract parameter value
  iRC = this->extractStringW( pvParseHandle, strValue );

  // skip any parameter separator
  if ( pData->JSONString[pData->iPos] == L',' ) pData->iPos++;
  
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
      // check for escaped double quote
      if ( (pData->JSONString[iCurPos] == '\\') && (pData->JSONString[iCurPos+1] == '\"') )
      {
        // skip escaped double quote but count as one character
        iCurPos += 2;
        iTargetLen++;
      }
      else 
      {
        // treat as normal character
        iCurPos++; iTargetLen++; 
      } /* end */       
    } /* endwhile */ 

    // verify that string end is indicated by a double quote
    if ( pData->JSONString[iCurPos] != '\"' ) return( ERROR_INVALIDJSONSYNTAX );

    // copy characters to target string and unescape double quotes
    string.resize( iTargetLen, ' ' );
    int iTargetPos = 0;
    while ( pData->iPos < iCurPos )
    {
      if ( (pData->JSONString[pData->iPos] == '\\') && (pData->JSONString[pData->iPos+1] == '\"') )
      {
        // unescape escaped double quote 
        pData->iPos += 2;
        string[iTargetPos++] = '\"';
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

  if ( pData->JSONString[pData->iPos] == L'\"'  )
  {
    // skip starting double quote
    pData->iPos++;

    // find string end delimiter and evaluate length of string
    int iTargetLen = 0;
    int iCurPos = pData->iPos;
    while ( (iCurPos < pData->iSize) && (pData->JSONString[iCurPos] != L'\"') )
    {
      // check for escaped double quote
      if ( (pData->JSONString[iCurPos] == L'\\') && (pData->JSONString[iCurPos+1] == L'\"') )
      {
        // skip escaped double quote but count as one character
        iCurPos += 2;
        iTargetLen++;
      }
      else 
      {
        // treat as normal character
        iCurPos++; iTargetLen++; 
      } /* end */       
    } /* endwhile */ 

    // verify that string end is indicated by a double quote
    if ( pData->JSONString[iCurPos] != L'\"' ) return( ERROR_INVALIDJSONSYNTAX );

    // copy characters to target string and unescape double quotes
    string.resize( iTargetLen, ' ' );
    int iTargetPos = 0;
    while ( pData->iPos < iCurPos )
    {
      if ( (pData->JSONString[pData->iPos] == L'\\') && (pData->JSONString[pData->iPos+1] == L'\"') )
      {
        // unescape escaped double quote 
        pData->iPos += 2;
        string[iTargetPos++] = L'\"';
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

  void *parseHandle = parseJSONStartW( JSONString );
  if ( parseHandle == NULL )
  {
    return( ERROR_STARTPARSINGFAILED );
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
  JSONString.append( name );
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
    if ( (JSONString[iEnd-1] != L'{') && (JSONString[iEnd-1] != L'[') && (JSONString[iEnd-1] == L':'))
    {
      JSONString.append( L"," );
    } /* endif */
  } /* endif */
  return( 0 );
}
