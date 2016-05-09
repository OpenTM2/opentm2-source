/*! \brief TMXFactory.H - Include file for the TMX factory class
	Copyright (c) 1999-2016, International Business Machines Corporation and others. All rights reserved.
	Description: This file contains the class definition for the JSONize helper class
*/

#ifndef _TMXFACTORY_H_
#define _TMXFACTORY_H_

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "string"


/*! \brief factory class for TMX related functions 
 
  This class is a singleton and provides functions for the conversion
  of OpenTM2 proposals to TMX and vice versa

*/


class TMXFactory
{

public:

/*! \brief Constructor
*/
TMXFactory();

/*! \brief Destructor
*/
~TMXFactory();

/*! \brief This static method returns a pointer to the JSON object.
	The first call of the method creates the JSON instance.
*/
	static TMXFactory* getInstance();


  /*! \brief return codes resturned by TMX factory
  */
  static const int ERROR_INVALIDPARSEHANDLE         = 3001;
  static const int INFO_ENDOFPARAMETERLISTREACHED   = 3002;
  static const int ERROR_INVALIDTMXSYNTAX           = 3003;
  static const int ERROR_INSUFFICIENTMEMORY         = 3004;

/*! \brief Converts a OtmProposal to a TMX formatted string

    \param Proposal reference to proposal data
    \param strTMX reference to a string receiving the TMX formatted string
  	\returns 0 or error code in case of errors
*/
  int ProposalToTMX
  (
    OtmProposal &Proposal,
    std::string &strTMX
  ); 

  int ProposalToTUString
  (
    OtmProposal &proposal,
    std::string &strTU,
	bool bWithHeader,
	bool bWithTail
  );

/*! \brief Converts a TMX formatted string into an OtmProposal 

    \param strTMX reference to a string containing the TMX formatted string
    \param Proposal reference to a OtmPorposal object receiving the extracted data
  	\returns 0 or error code in case of errors
*/
  int TMX2Proposal
  (
    std::string &strTMX,
	 std::vector<OtmProposal*> &proposals
    //OtmProposal &Proposal
  ); 

/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
  int getLastError
  (
    std::string &strError
  ); 
/*! \brief Get the error message for the last error occured

    \param pszError pointer to a buffer for the error text
    \param iBufSize size of error text buffer in number of characters
  	\returns last error code
*/
  int getLastError
  (
    char *pszError,
    int iBufSize
  ); 

private:

/*! \brief Copy a string to a buffer and return pointer to the end of the copied data

    \param pszString pointer to stringbeing copied
    \param pszBuffer pointer to buffer for the string data
  	\returns pointer to the end of the copied data
*/
  wchar_t *copyString
  (
    wchar_t *pszString,
    wchar_t *pszBuffer
  ); 
  wchar_t *copyString
  (
    char *pszString,
    wchar_t *pszBuffer
  ); 

/*! \brief replace xml escape charaters in pSrc

    \param pSrc  pointer to stringbeing formatted
    \param pOut  pointer to buffer for outputing
    \param outCapcity the capcity of pOut
  	\returns 0 if success
*/
  int xmlFormat(wchar_t *pSrc, wchar_t *pOut, size_t outCapcity);

/*! \brief whether the year is leap
    \param year  pointer to stringbeing formatted
  	\returns true if is leap
*/
  bool isLeapYear(int year);

/*! \brief get current time in millions
  	\returns current millions
*/
  long long  getCurrentMillions();

/*! \brief Load name list file for Tmgr <-> TMX name conversions
  	\returns 0 if successful otherwise error code
*/
  int TMXFactory::loadNames();

/*! \brief Initialize instants data
  	\returns 0 if successful otherwise error code
*/
  int init();

  static TMXFactory *instance;
  void *pvData;
};


#endif // ifndef _TMXFACTORY_H_
