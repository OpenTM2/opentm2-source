/*! \brief JSON.H - Include file for the JSON factory class
	Copyright (c) 1999-2012, International Business Machines Corporation and others. All rights reserved.
	Description: This file contains the class definition for the JSONize helper class
*/

#ifndef _OTMMSJSONFACTORY_H_
#define _OTMMSJSONFACTORY_H_

#include "string"

/*! \brief factory class for JSON related functions 
 
  This class is a singleton and provides functions for the processing
  of JSON formatted strings

*/
class JSONFactory
{

public:

/*! \brief This static method returns a pointer to the JSON object.
	The first call of the method creates the JSON instance.
*/
	static JSONFactory* getInstance();


  /*! \brief return codes resturned by JSON factory
  */
  static const int ERROR_INVALIDPARSEHANDLE         = 2001;
  static const int INFO_ENDOFPARAMETERLISTREACHED   = 2002;
  static const int ERROR_INVALIDJSONSYNTAX          = 2003;
  static const int ERROR_STARTPARSINGFAILED         = 2004;
  static const int INFO_BEGINOFELEMENTDETECTED      = 2005;

/*! \brief Starts a JSON string

    This method starts a JSON string

    \param str reference to a string receiving the JSON start sequence

  	\returns 0 or error code in case of errors
*/
  int startJSON
  (
    std::string &str
  ); 

  /*! \brief Starts a JSON string

    This method starts a JSON string

    \param str reference to a string receiving the JSON start sequence

  	\returns 0 or error code in case of errors
*/
  int startJSONW
  (
    std::wstring &str
  ); 

/*! \brief Terminates a JSON string

    This method terminates a JSON string

    \param JSONString reference to a string receiving the JSON end sequence

  	\returns 0 or error code in case of errors
*/
  static int terminateJSON
  (
    std::string &JSONString
  ); 

  /*! \brief Terminates a JSON string

    This method terminates a JSON string

    \param JSONString reference to a string receiving the JSON end sequence

  	\returns 0 or error code in case of errors
*/
  static int terminateJSONW
  (
    std::wstring &JSONString
  ); 

/*! \brief Adds a parameter to a JSON string

    This method adds a parameter to a JSON string

    \param JSONString reference to a string receiving the parameter
    \param name parameter name
    \param value parameter value

  	\returns 0 or error code in case of errors
*/
  int addParmToJSON
  (
    std::string &JSONString,
    const std::string &name,
    const std::string &value
  ); 

  int addParmToJSON
  (
    std::string &JSONString,
    const std::string &name,
    int iValue
  );

/*! \brief Adds a parameter to a JSON string

    This method adds a parameter to a JSON string

    \param JSONString reference to a string receiving the parameter
    \param name parameter name
    \param value parameter value

  	\returns 0 or error code in case of errors
*/
  int addParmToJSONW
  (
    std::wstring &JSONString,
    const std::wstring &name,
    const std::wstring &value
  ); 

  int addParmToJSONW
  (
    std::wstring &JSONString,
    const std::wstring &name,
    const std::string &value
  );

  int addParmToJSONW
  (
    std::wstring &JSONString,
    const std::wstring &name
  );

  int addParmToJSONW
  (
    std::wstring &JSONString,
    const std::wstring &name,
    int iValue
  ); 

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
    void *pvValue;           // pointer to a buffer receiving the value
    int iBufferLen;          // size of the buffer in number of characters
  } JSONPARSECONTROL, *PJSONPARSECONTROL;

/*! \brief Parses a JSON string using the supplied control table

    This method parses a JSON string using the provided control table

    \param JSONString reference to a JSON string 

    \param paParserControl pointer to table containing the information on the parameters to be extracted

  	\returns 0 when successful or an error code
*/
  int parseJSON
  (
    std::wstring &JSONString,
    PJSONPARSECONTROL paParserControl
  ); 


/*! \brief Start parsing of a JSON string

    This method initializes the parsing of a JSON string 

    \param JSONString reference to a JSON string 

  	\returns handle of a JSON string parser instance or NULL in case of errors
*/
  void *parseJSONStart
  (
    std::string &JSONString,
    int *piRC = NULL
  ); 

/*! \brief Start parsing of a JSON string

    This method initializes the parsing of a JSON string 

    \param JSONString reference to a JSON string 

  	\returns handle of a JSON string parser instance or NULL in case of errors
*/
  void *parseJSONStartW
  (
    std::wstring &JSONString,
    int *piRC = NULL
  ); 


/*! \brief Stop parsing of a JSON string

    This method ends the parsing of a JSON string and all internal resources are freed.
    The parser handle is not valid anymore.

    \param pvParseHandle JSON string parser handle as returned by parseJSONStart
*/
  void parseJSONStop
  (
    void *pvParseHandle
  ); 

  /*! \brief Stop parsing of a JSON string

    This method ends the parsing of a JSON string and all internal resources are freed.
    The parser handle is not valid anymore.

    \param pvParseHandle JSON string parser handle as returned by parseJSONStart
*/
  void parseJSONStopW
  (
    void *pvParseHandle
  ); 

/*! \brief Get the next parameter from a JSON string

    This method retrieves the next parameter and its value from a JSON string. When called
    for the first time first parameter of the JSON string is retrieved.

    \param pvParseHandle JSON string parser handle as returned by parseJSONStart
    \param strName reference to a string receiving the parameter name
    \param strValue reference to a string receiving the parameter value
    \returns 0 if successful or return code
*/
  int parseJSONGetNext
  (
    void *pvParseHandle,
    std::string &strName,
    std::string &strValue
  ); 

/*! \brief Get the next parameter from a JSON string

    This method retrieves the next parameter and its value from a JSON string. When called
    for the first time first parameter of the JSON string is retrieved.

    \param pvParseHandle JSON string parser handle as returned by parseJSONStart
    \param strName reference to a string receiving the parameter name
    \param strValue reference to a string receiving the parameter value
    \returns 0 if successful or return code
*/
  int parseJSONGetNextW
  (
    void *pvParseHandle,
    std::wstring &strName,
    std::wstring &strValue
  ); 

/*! \brief Add a element name to a JSON string
  	\returns 0 or error code in case of errors
*/
  int addNameToJSONW
  (
    std::wstring &JSONString,
    const std::wstring &name
  );

  /*! \brief Add the start identifier for an array to a JSON string
  	  \returns 0 or error code in case of errors
  */
  int addArrayStartToJSONW
  (
    std::wstring &JSONString
  );

  /*! \brief Add the end identifier for an array to a JSON string
  	  \returns 0 or error code in case of errors
  */
  int addArrayEndToJSONW
  (
    std::wstring &JSONString
  );

  /*! \brief Add the start identifier for an element to a JSON string
  	  \returns 0 or error code in case of errors
  */
  int addElementStartToJSONW
  (
    std::wstring &JSONString
  );

  /*! \brief Add the end identifier for an element to a JSON string
  	  \returns 0 or error code in case of errors
  */
  int addElementEndToJSONW
  (
    std::wstring &JSONString
  );


private:

/*! \brief adds a string to a JSON string and escapes double quotes

    This method adds a parameter to a JSON string

    \param JSONString reference to a string receiving the parameter
    \param str string being added to the JSON string

  	\returns 0 or error code in case of errors
*/
  int addString
  (
    std::string &JSONString,
    const std::string &str
  ); 

/*! \brief adds a string to a JSON string and escapes double quotes

    This method adds a parameter to a JSON string

    \param JSONString reference to a string receiving the parameter
    \param str string being added to the JSON string

  	\returns 0 or error code in case of errors
*/
  int addStringW
  (
    std::wstring &JSONString,
    const std::wstring &str
  ); 

  static JSONFactory *instance;

  /*! \brief Extract a string enclosed in double quotes

      This method extracts a string enclosed in double quotes and handles any
      escaped double quotes inside the string, the iPos parameter is increased
      to the next character following the ending double quote of the string

      \param pData pointer to JSON string parser data area
      \param string reference to a string receiving the parameter name
      \returns 0 if successful or return code
  */
  int extractString
  (
    void *pvData,
    std::string &string
  );

/*! \brief Extract a string enclosed in double quotes

      This method extracts a string enclosed in double quotes and handles any
      escaped double quotes inside the string, the iPos parameter is increased
      to the next character following the ending double quote of the string

      \param pData pointer to JSON string parser data area
      \param string reference to a string receiving the parameter name
      \returns 0 if successful or return code
  */
  int extractStringW
  (
    void *pvData,
    std::wstring &string
  );

  /*! \brief Add the delimiter (komma) to the JSON string when needed
  	  \returns 0 or error code in case of errors
  */
  int addDelimiterW
  (
    std::wstring &JSONString
  );

  /*! \brief Add the delimiter (komma) to the JSON string when needed
  \returns 0 or error code in case of errors
  */
  int addDelimiter
  (
    std::string &JSONString
  );

};


#endif // ifndef _OTMMSJSONFACTORY_H_
