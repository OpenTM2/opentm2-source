/*! \brief JSONize.CPP - Module with JSON related functions
	Copyright (c) 1999-2014, International Business Machines Corporation and others. All rights reserved.
	Description: This module contains functions to wrap parameters in the JSON format and to retrieve parameters from JSON strings
*/

#include "JSONFactory.h"

/** Initialize the static instance variable */
JSONFactory* JSONFactory::instance = 0;


/*! \brief This static method returns a pointer to the MemoryFactory object.
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
    for ( iPos = 0; iPos < str.size(); iPos++ )
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
    if ( iStart < str.size() )
    {
      newString.append( str.substr( iStart ) );
    } /* endif */           
    newString.append( "\"" );
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

