/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _MEMORYWEBSERVICECLIENT_H_
#define _MEMORYWEBSERVICECLIENT_H_

#include <string>
#include <vector>
#include <map>
#include "core\pluginmanager\OtmMemoryPlugin.h"

class __declspec(dllexport) MemoryWebServiceClient
/*! \brief This class implements the interface to the memory web service of OpenTM2.
*/
{
public:

 /*! \brief Constructor
  */
  MemoryWebServiceClient::MemoryWebServiceClient();

  MemoryWebServiceClient::MemoryWebServiceClient( const char *pszLogFileName );


  /*! \brief destructor
  */
  MemoryWebServiceClient::~MemoryWebServiceClient();

/*! \brief Set the endpoint URL for this web service
   \param pszEndPointURL endpoint URL for the communication with the web service
   \returns 0 when successful or error code
*/
	int setEndpointUrl( const char *pszEndPointURL );

/*! \brief Set the endpoint URL for this web service
   \param pszEndPointURL endpoint URL for the communication with the web service
   \returns 0 when successful or error code
*/
	int setEndpointUrl( std::string &strEndPointURL );

/* \brief List available memories
   \param pszUser user ID
	 \param pszPassword password for the user ID
	 \param List reference to vector receiving the strings with the memory names
   \returns number of provided memories
*/
int listMemories
(
    char *pszUser,
    char *pszPassword,
    std::vector<std::string> &List
);

/* \brief Get details of a memory
   \param pszUser user ID
	 \param pszPassword password for the user ID
	 \param pszMemory name of the memory
   \returns number of provided memories
*/
int getMemoryInfo
(
    char *pszUser,
    char *pszPassword,
    char *pszMemory,
    OtmMemoryPlugin::PMEMORYINFO pMemInfo
);

/* \brief Create a shared memory
   \param pvOptions pointer to vector containing the create option strings
     Option 0 = memory name
     Option 1 = service URL
     Option 2 = user ID
     Option 3 = password
     Option 4 = data source name
     Option 5 = data source generic type
     Option 6 = data source server
     Option 7 = data source port
     Option 8 = data source port
     Option 9 = data source user ID
     Option 10 = data source user ID password
     Option 11 = comma separated user list for access to the shared memory
   \returns 0 when successful or error code
*/
int createMemory
(
    std::vector<std::string> *pvOptions
);

/* \brief Delete a shared memory
   \param pvOptions pointer to vector containing the create option strings
     Option 0 = memory name
     Option 1 = user ID
     Option 2 = password
   \returns 0 when successful or error code
*/
int deleteMemory
(
    std::vector<std::string> *pvOptions
);

/* \brief Upload proposal data
   \param pszName name of memory
   \param pszUserID user ID uploading the proposal
   \param pszPassword password for user ID
   \param strTMXProposal proposal data in TMX format
   \returns 0 when successful or error code
*/
  int uploadProposal
  (
     char *pszName,
     char *pszUserID,
     char *pszPassword,
     std::string &strTMXProposal
  );

/* \brief Download proposal data
   \param pszName name of memory
   \param pszUserID user ID uploading the proposal
   \param pszPassword password for user ID
   \param strTMXProposal reference to string receiving the proposal data in TMX format
   \param strUpdateCounter reference to string receiving the update counter
   \returns 0 when successful or error code
*/
  int downloadProposal
  (
     char *pszName,
     char *pszUserID,
     char *pszPassword,
     std::string &strTMXProposal,
     std::string &strUpdateCounter
  );

  /* \brief Get memory creator
   \param pszUser user ID
   \param pszPassword password for the user ID
   \param pszMemory name of the memory
   \param creator the name of the memory creator
   \returns 0 if success
*/
int getCreator
(
    char *pszUser,
    char *pszPassword,
    char *pszMemory,
    std::string &creator
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

/* \brief add a new user to a shared memory user list
   \param pvOptions pointer to vector containing the  addUser option strings 
   \returns 0
*/
int addMemoryUser
(
    std::vector<std::string> *pvOptions
);

/* \brief delete a new user from a shared memory user list
   \param pvOptions pointer to vector containing the  removeUser option strings   
   \returns 0
*/
int removeMemoryUser
(
    std::vector<std::string> *pvOptions
);

/* \brief   list shared memory users
   \param   pvOptions pointer to vector containing the  listUser option strings  
   \param   users  shared memory users returned
   \returns 0
*/
int listMemoryUsers
(
    std::vector<std::string> &options,
	std::vector<std::string> &users
);

// load current update counter value
void loadUpdateCounter( std::string &strPropPath, std::string &strPropFileName, std::string &strUpdateCounter );
// write new update counter value
void writeUpdateCounter( std::string &strPropPath, std::string &strPropFileName, std::string &strUpdateCounter );


/*! \brief Error codes returned by MemoryWebServiceClient.
*/
static const int Error_AxutilEnvCreateAll_failed         = 9001;    
static const int Error_Axis2CHome_notset                 = 9001;    
static const int Error_PrivateDataNotSet                 = 9001;    
static const int Error_AdbSynchronizeCreate_failed       = 9001;    
static const int Error_NoResponseFromWebService          = 9001;    
static const int Error_IncompleteResponseFromWebService  = 9001;    
static const int Error_InternalFunction_failed           = 9001;    

private:

  /*! \brief Pointer to private data area of MemoryWebServiceClient object 
  */
  void *pvPrivateData;

  /* \brief create the stub for the access to the web service
     \returns 0 when successful or error return code
  */
  int createStub
  (
  );

  /* \brief Set error text and last error code
   \param pszErrorText error description
	 \param iErrorCode error code
	\returns iErrorCode 
  */
  int setError
  (
    const char *pszErrorText,
    int iErrorCode
  );

  /* \brief Send a command to the synchronize function of the web service and return its response
    \param strCommand Reference to a string containing the command being send to web service
    \param strResponse Reference to a string receiving the response returned from web service
	  \returns 0 when successful or error code 
  */
  int callSynchronize
  ( 
    std::string &strCommand,
    std::string &strResponse
  );

  int  doSyncCall(std::map<std::string,std::string> &parameters, const std::string requiredVal, std::string &retVal );

  void makeUpdateCounterFileName( std::string &strPropPath, std::string &strPropFile, std::string &strUpdateCounterFileName );
};

#endif // #ifndef _MEMORYWEBSERVICECLIENT_H_