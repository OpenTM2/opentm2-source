/*! \brief JSON.H - Include file for the JSON factory class
	Copyright (c) 1999-2012, International Business Machines Corporation and others. All rights reserved.
	Description: This file contains the class definition for the JSONize helper class
*/

#ifndef _JSONFACTORY_H_
#define _JSONFACTORY_H_

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

/*! \brief Starts a JSON string

    This method starts a JSON string

    \param str reference to a string receiving the JSON start sequence

  	\returns 0 or error code in case of errors
*/
  int startJSON
  (
    std::string &str
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

/*! \brief Start parsing of a JSON string

    This method initializes the parsing of a JSON string 

    \param JSONString reference to a JSON string 

  	\returns handle of a JSON string parser instance or NULL in case of errors
*/
  void *parseJSONStart
  (
    std::string &JSONString
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

};

#endif // ifndef _JSONFACTORY_H_
