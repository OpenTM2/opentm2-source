//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#define WIN32
#define AXIS2_DECLARE_EXPORT


#include "axis2_stub_OtmTMServiceImplService.h"

#include <stdio.h>
#include <axiom.h>
#include <axis2_util.h>
#include <axiom_soap.h>
#include <axis2_client.h>

#include <axis2_client.h>

#include <string>
#include "MemoryWebServiceClient.h"
#include "JSONFactory.h"
#include "core\utilities\LogWriter.h"

static std::string   RESPONSESTATUSMSG  ="status-msg";

// private data area of Memory Web Service Client, this area will be 
typedef struct _WEBCLIENTDATA
{
  // input data
  std::string strEndPointURL;

  // axis2/c related variables
  axutil_env_t *env;
  axis2_char_t *client_home;
  axis2_char_t *endpoint_uri;
  axis2_stub_t *stub;

  // logging
  std::string strLogFile;
  LogWriter Log;


  // error handling
  std::string strLastError; // last error message
  int iLastReturnCode;      // last return code

} WEBCLIENTDATA, *PWEBCLIENTDATA;

/*! \brief Constructors
*/
MemoryWebServiceClient::MemoryWebServiceClient()
{
  PWEBCLIENTDATA pData = NULL;

  // allocate and initialize private data area
  pData = new(WEBCLIENTDATA);
  pData->env = NULL;
  pData->client_home = NULL;
  pData->endpoint_uri = "";
  pData->stub = NULL;
  pData->strLogFile = "WebServiceClient.log";
  this->pvPrivateData = pData;
  //pData->Log.open( "WebServiceClient" );
}

MemoryWebServiceClient::MemoryWebServiceClient( const char *pszLogFileName )
{
  PWEBCLIENTDATA pData = NULL;

  // allocate and initialize private data area
  pData = new(WEBCLIENTDATA);
  pData->env = NULL;
  pData->client_home = NULL;
  pData->endpoint_uri = "";
  pData->stub = NULL;
  pData->strLogFile = pszLogFileName;
  this->pvPrivateData = pData;
  pData->Log.open( pszLogFileName );
}




MemoryWebServiceClient::~MemoryWebServiceClient()
{
  if ( this->pvPrivateData != NULL ) 
  {
    PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;

    pData->Log.close();
    if ( pData->env != NULL ) axutil_env_free( pData->env );

    delete( this->pvPrivateData );
  } /* endif */
}


/*! \brief Set the endpoint URL for this web service
   \param pszEndPointURL endpoint URL for the communication with the web service
   \returns 0 when successful or error code
*/
int MemoryWebServiceClient::setEndpointUrl( const char *pszEndPointURL )
{
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  if ( pData != NULL )
  {
      if(pData->strEndPointURL != pszEndPointURL)
      {
          pData->strEndPointURL = pszEndPointURL;
          // if URL changed, stud must be re-created
          if(pData->stub != NULL)
          {
              //delete pData->stub;
              axis2_stub_free(pData->stub,pData->env);
              pData->stub = NULL;
          }
      }
  } /* endif */     
  return( 0 );
}

/*! \brief Set the endpoint URL for this web service
   \param pszEndPointURL endpoint URL for the communication with the web service
   \returns 0 when successful or error code
*/
int MemoryWebServiceClient::setEndpointUrl( std::string &strEndPointURL )
{
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  if ( pData != NULL )
  {
    pData->Log.writef( "EndPointURL has been set to %s", strEndPointURL.c_str() );
    if(pData->strEndPointURL != strEndPointURL)
    {
        pData->strEndPointURL = strEndPointURL;
        // if URL changed, stud must be re-created
        if(pData->stub != NULL)
        {
              //delete pData->stub;
              axis2_stub_free(pData->stub,pData->env);
              pData->stub = NULL;
        }
    }
  } /* endif */     
  return( 0 );
}



int MemoryWebServiceClient::createStub
(
)
{
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  if ( pData != NULL )
  {
    pData->Log.write( "creating stub..." );
    pData->env = axutil_env_create_all( pData->strLogFile.c_str(), AXIS2_LOG_LEVEL_TRACE);
    if ( pData->env == NULL )
    {
      pData->Log.write( "   axutil_env_create_all failed" );
      return( this->setError( "Error: internal function failed (axutil_env_create_all)", Error_AxutilEnvCreateAll_failed ) );
    } /* endif */     

    pData->client_home = AXIS2_GETENV("AXIS2C_HOME");
    if ( pData->client_home == NULL )
    {
      pData->Log.write( "   AXIS2_GETENV(AXIS2C_HOME) failed" );
      return( this->setError( "Error: AXIS2C_HOME environment variable not set", Error_Axis2CHome_notset ) );
    } /* end */       

    pData->stub = axis2_stub_create_OtmTMServiceImplService( pData->env, pData->client_home, pData->strEndPointURL.c_str() );
    if ( pData->stub == NULL )
    {
      pData->Log.write( "   axis2_stub_create_OpenTMSWebServiceImplementationService failed" );
      return( this->setError( "Error: could not create OpenTMSWebService stub", Error_Axis2CHome_notset ) );
    } /* endif */       
    pData->Log.write( "...stub created successfully" );

  } /* endif */ 
  return( 0 );
}

/* \brief Set error text and last error code
  \param pszErrorText error description
	\param iErrorCode error code
	\returns iErrorCode 
*/
int MemoryWebServiceClient::setError
(
    const char *pszErrorText,
    int iErrorCode
)
{
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  if ( pData != NULL )
  {
    pData->strLastError = pszErrorText;
    pData->iLastReturnCode = iErrorCode;
    pData->Log.writef( "   Error %ld (%s) returned by WebServiceClient", iErrorCode, pszErrorText );
  } /* endif */       
  return( iErrorCode );
}

  
/* \brief get a list of memories available for the given user
	 \param strUser user ID
	 \param strPassword password for the user ID
   \param MemList reference to a vector receiving the names of the memories
  \param strResponse Reference to a string receiving the response returned from web service
	\returns 0 when successful or error code 
*/
int MemoryWebServiceClient::listMemories
(
    char *pszUser,
    char *pszPassword,
    std::vector<std::string> &List
)
{

   std::map<std::string,std::string> parameters;
   parameters.insert(std::map<std::string,std::string>::value_type("method", "listallmemories"));
   parameters.insert(std::map<std::string,std::string>::value_type("user-id",  pszUser));
   parameters.insert(std::map<std::string,std::string>::value_type("password", pszPassword));

   std::string memoryList;
   int iRC = doSyncCall(parameters,"memory-list",memoryList);
   if(iRC!=0)
	   return iRC;

   int iStart = 0;
   do 
   {
     int iEnd = memoryList.find( ",", iStart );
     int iLen = (iEnd == std::string::npos ) ? std::string::npos : iEnd - iStart;
     List.push_back( memoryList.substr( iStart, iLen ) );
     iStart = (iEnd != std::string::npos) ? iEnd + 1 : iEnd;
   } while( iStart != std::string::npos );

   return( iRC );
}

/* \brief Get details of a memory
   \param pszUser user ID
	 \param pszPassword password for the user ID
	 \param pszMemory name of the memory
   \returns number of provided memories
*/
int MemoryWebServiceClient::getMemoryInfo
(
    char *pszUser,
    char *pszPassword,
    char *pszMemory,
    OtmMemoryPlugin::PMEMORYINFO pMemInfo
)
{
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  std::string response = "";
  int iRC = 0;

  if ( pData == NULL )
  {
    return( setError( "Error: internal data not allocated", Error_PrivateDataNotSet ) );
  } /* endif */       

  pData->Log.write( "processing getMemoryInfo method" );

  // prepare command
  JSONFactory *factory = JSONFactory::getInstance();
  std::string command = "";
  factory->startJSON( command );
  factory->addParmToJSON( command, "method", "query" );
  factory->addParmToJSON( command, "user-id", pszUser );
  factory->addParmToJSON( command, "password", pszPassword );
  factory->addParmToJSON( command, "name", pszMemory );
  factory->terminateJSON( command );
  pData->Log.writef( "   JSON string is %s", command.c_str() );
  
  // process command
  iRC = this->callSynchronize( command, response );
  if ( iRC != 0 )
  {
    pData->Log.writef( "   synchronize call failed with rc=%ld", iRC );
    return( iRC  );
  }
  else
  {
    pData->Log.writef( "   synchronize call successful, response is %s", response.c_str() );
  } /* endif */     

  // extract some parameters from the response
  void *parseHandle = factory->parseJSONStart( response );
  if ( parseHandle == NULL )
  {
    return( setError( "Error: internal function failed (parseJSONStart)", Error_InternalFunction_failed ) );
  } /* end */       

  std::string strErrorCode = "";
  std::string strErrorMessage = "";
  std::string strDescription = "";
  while ( iRC == 0 )
  {
    std::string name;
    std::string value;
    iRC = factory->parseJSONGetNext( parseHandle, name, value );
    if ( iRC == 0 )
    {
      if ( stricmp( name.c_str(), "ErrorCode" ) == 0 )
      {
        strErrorCode = value;
      } 
      else if ( stricmp( name.c_str(), "description" ) == 0 )
      {
        strDescription = value;
      } 
      else if ( stricmp( name.c_str(), "errorstring" ) == 0 )
      {
        strErrorMessage = value;
      } /* endif */         
    } /* endif */       
  } /* endwhile */     
  factory->parseJSONStop( parseHandle );
  if ( (iRC != 0) && (iRC != JSONFactory::INFO_ENDOFPARAMETERLISTREACHED) )
  {
    return( setError( "Error: internal function failed (parseJSONGetNext)", Error_InternalFunction_failed ) );
  } /* end */       
  if ( strErrorCode.empty() )
  {
    return( setError( "Error: incomplete response from web servive (ErrorCode is missing)", Error_IncompleteResponseFromWebService ) );
  } /* end */       

  // handle errors returned by web service
  iRC = atol( strErrorCode.c_str() );
  if ( iRC != 0 )
  {
    strErrorMessage.insert( 0, "WebServiceError: " );
    return( setError( strErrorMessage.c_str(), iRC ) );
  } /* end */       

  // fill caller's memory info with received data
  memset( pMemInfo, 0, sizeof(OtmMemoryPlugin::MEMORYINFO) );
  strcpy( pMemInfo->szDescription, strDescription.c_str() );
  pMemInfo->fEnabled = TRUE;
  strcpy( pMemInfo->szName, pszMemory );

  return( iRC );
}

/* \brief Create a shared memory
   \param pvOptions pointer to vector containing the create option strings
     Option 0 = memory name
     Option 1 = service URL
     Option 2 = user ID
     Option 3 = password
     Option 4 = data source name
     Option 5 = data source generic type
     Option 6 = data source type
     Option 7 = data source server
     Option 8 = data source port
     Option 9 = data source user ID
     Option 10 = data source user ID password
     Option 11 = comma separated user list for access to the shared memory
   \returns 0 when successful or error code
*/
int MemoryWebServiceClient::createMemory
(
    std::vector<std::string> *pvOptions
)
{

  std::map<std::string,std::string> parameters;
  parameters.insert(std::map<std::string,std::string>::value_type("method", "create"));
  parameters.insert(std::map<std::string,std::string>::value_type("user-id",  (*pvOptions)[2]));
  parameters.insert(std::map<std::string,std::string>::value_type("password", (*pvOptions)[3]));
  parameters.insert(std::map<std::string,std::string>::value_type("name", (*pvOptions)[0]));
  parameters.insert(std::map<std::string,std::string>::value_type("user-id-list", (*pvOptions)[4]));

  std::string notRequired;
  return doSyncCall(parameters,"",notRequired);
}


/* \brief Delete a shared memory
   \param pvOptions pointer to vector containing the create option strings
     Option 0 = memory name
     Option 1 = user ID
     Option 2 = password
   \returns 0 when successful or error code
*/
int MemoryWebServiceClient::deleteMemory
(
    std::vector<std::string> *pvOptions
)
{
   std::map<std::string,std::string> parameters;
   parameters.insert(std::map<std::string,std::string>::value_type("method", "delete"));
   parameters.insert(std::map<std::string,std::string>::value_type("user-id",  (*pvOptions)[1]));
   parameters.insert(std::map<std::string,std::string>::value_type("password", (*pvOptions)[2]));
   parameters.insert(std::map<std::string,std::string>::value_type("name", (*pvOptions)[0]));
   
   std::string notRequired;
   return doSyncCall(parameters,"",notRequired);
}

/* \brief Upload proposal data
   \param pszName name of memory
   \param pszUserID user ID uploading the proposal
   \param pszPassword password for user ID
   \param strTMXProposal proposal data in TMX format
   \returns 0 when successful or error code
*/
int MemoryWebServiceClient::uploadProposal
(
   char *pszName,
   char *pszUserID,
   char *pszPassword,
   std::string &strTMXProposal
)
{
   std::map<std::string,std::string> parameters;
   parameters.insert(std::map<std::string,std::string>::value_type("method", "upload"));
   parameters.insert(std::map<std::string,std::string>::value_type("user-id",  pszUserID));
   parameters.insert(std::map<std::string,std::string>::value_type("password", pszPassword));
   parameters.insert(std::map<std::string,std::string>::value_type("name",pszName));
   parameters.insert(std::map<std::string,std::string>::value_type("tmx-document",strTMXProposal));
   parameters.insert(std::map<std::string,std::string>::value_type("encoding","UTF-8"));

   std::string notRequired;
   return doSyncCall(parameters,"",notRequired);

}

/* \brief Download proposal data
   \param pszName name of memory
   \param pszUserID user ID uploading the proposal
   \param pszPassword password for user ID
   \param strTMXProposal reference to string receiving the proposal data in TMX format
   \param strUpdateCounter reference to string containing the current and receiving the new update counter
   \returns 0 when successful or error code
*/
int MemoryWebServiceClient::downloadProposal
(
   char *pszName,
   char *pszUserID,
   char *pszPassword,
   std::string &strTMXProposal,
   std::string &strUpdateCounter
)
{
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  std::string response = "";
  int iRC = 0;

  strTMXProposal = "";
  if ( pData == NULL )
  {
    return( setError( "Error: internal data not allocated", Error_PrivateDataNotSet ) );
  } /* endif */       

  // prepare command
  JSONFactory *factory = JSONFactory::getInstance();
  std::string command = "";
  factory->startJSON( command );
  factory->addParmToJSON( command, "method", "download" );
  factory->addParmToJSON( command, "user-id", pszUserID );
  factory->addParmToJSON( command, "password", pszPassword );
  factory->addParmToJSON( command, "name", pszName );
  factory->addParmToJSON( command, "parameter", "empty" );
  factory->addParmToJSON( command, "update-counter", strUpdateCounter );
  factory->terminateJSON( command );

  // process command
  iRC = this->callSynchronize( command, response );
  if ( iRC != 0 ) 
  {
    pData->Log.writef( "synchronize failed with rc=%ld", iRC );
    return( iRC  );
  } 

  // extract some parameters from the response
  void *parseHandle = factory->parseJSONStart( response );
  if ( parseHandle == NULL )
  {
    return( setError( "Error: internal function failed (parseJSONStart)", Error_InternalFunction_failed ) );
  } /* end */       

  std::string responseMsg = "";
  while ( iRC == 0 )
  {
    std::string name;
    std::string value;
    iRC = factory->parseJSONGetNext( parseHandle, name, value );
    if ( iRC == 0 )
    {
      if (stricmp( name.c_str(), RESPONSESTATUSMSG.c_str()  ) == 0 )
      {
        responseMsg = value;
      } 
      else if ( stricmp( name.c_str(), "tmx-document" ) == 0 )
      {
        // replace "\\" with "\"
        std::size_t iStart = 0;
        std::size_t idx = value.find("\\\\",iStart);
        while(idx != std::string::npos)
        {
            strTMXProposal.append(value.substr(iStart,idx-iStart));
            strTMXProposal.append("\\");
            iStart = idx+2;
            idx = value.find("\\\\",iStart);
        }
        strTMXProposal.append(value.substr(iStart,value.length()-iStart));
      } 
      else if ( stricmp( name.c_str(), "update-counter" ) == 0 )
      {
        strUpdateCounter = value;
      }       
    } /* endif */       
  } /* endwhile */     
  factory->parseJSONStop( parseHandle );

  if ( (iRC != 0) && (iRC != JSONFactory::INFO_ENDOFPARAMETERLISTREACHED) )
  {
    return( setError( "Error: internal function failed (parseJSONGetNext)", Error_InternalFunction_failed ) );
  }  

  if ( responseMsg.empty() )
  {
	  return( setError( "Error: incomplete response from web servive (ErrorCode is missing)", Error_IncompleteResponseFromWebService ) );
  }
  else 
  {
	 if(responseMsg!="success")
		return( setError( responseMsg.c_str(), 1 ) );
	 else
		iRC = 0;
   }

  return( iRC );
}


/* \brief Get memory creator
   \param pszUser user ID
   \param pszPassword password for the user ID
   \param pszMemory name of the memory
   \returns number of provided memories
*/
int MemoryWebServiceClient::getCreator
(
    char *pszUser,
    char *pszPassword,
    char *pszMemory,
    std::string &creator
)
{
   std::map<std::string,std::string> parameters;
   parameters.insert(std::map<std::string,std::string>::value_type("method", "getcreator"));
   parameters.insert(std::map<std::string,std::string>::value_type("user-id",  pszUser));
   parameters.insert(std::map<std::string,std::string>::value_type("password", pszPassword));
   parameters.insert(std::map<std::string,std::string>::value_type("name", pszMemory));
 
   return doSyncCall(parameters,"creator",creator);
}

/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
int MemoryWebServiceClient::getLastError
(
  std::string &strError
)
{
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  if ( pData != NULL )
  {
    strError = pData->strLastError;
    return( pData->iLastReturnCode );
  } /* endif */       

  return( Error_PrivateDataNotSet );
}
/*! \brief Get the error message for the last error occured

    \param pszError pointer to a buffer for the error text
    \param iBufSize size of error text buffer in number of characters
  	\returns last error code
*/
int MemoryWebServiceClient::getLastError
(
  char *pszError,
  int iBufSize
) 
{
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  if ( pData != NULL )
  {
    strncpy( pszError, pData->strLastError.c_str(), iBufSize );
    return( pData->iLastReturnCode );
  } /* endif */       

  return( Error_PrivateDataNotSet );
}



/* \brief Send a command to the synchronize function of the web service and return its response
  \param strCommand Reference to a string containing the command being send to web service
  \param strResponse Reference to a string receiving the response returned from web service
	\returns 0 when successful or error code 
*/
int MemoryWebServiceClient::callSynchronize
( 
  std::string &strCommand,
  std::string &strResponse
)
{
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  adb_synchronize_t *synch;
  adb_synchronizeResponse_t *response;
  axis2_char_t* returnedResponse;

  if ( pData == NULL )
  {
    return( setError( "Error: internal data not allocated", Error_PrivateDataNotSet ) );
  } /* endif */       

  // create stub if not done yet
  if ( pData->stub == NULL )
  {
    this->createStub();
  } /* endif */     

  // build the request adb
  synch = adb_synchronize_create(pData->env);
  if( synch == NULL)
  {
    return( setError( "Error: internal function failed (adb_synchronize_create)", Error_AdbSynchronizeCreate_failed ) );
  } /* endif */

  adb_synchronize_set_parameters( synch, pData->env, strCommand.c_str() );

  // call the service 
  response = axis2_stub_op_OtmTMServiceImplService_synchronize( pData->stub, pData->env, synch );

  // extract the response
  if( response == NULL )
  {
    adb_synchronize_free ( synch, pData->env );
    return( setError( "Error: No repsonse from web service", Error_NoResponseFromWebService ) );
  } /* endif */

  char *pszResponse = adb_synchronizeResponse_get_return(response, pData->env );
  if ( pszResponse != NULL )
  {
    strResponse = pszResponse;
  }
  else
  {
    strResponse = "";
  } /* end */     

  adb_synchronize_free ( synch, pData->env );

  return 0;
}

int MemoryWebServiceClient::doSyncCall(std::map<std::string,std::string> &parameters,const std::string requiredVal, std::string &retVal )
{
	PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
	std::string response = "";
	int iRC = 0;

	if ( pData == NULL )
	{
	    return( setError( "Error: internal data not allocated", Error_PrivateDataNotSet ) );
	}   

	pData->Log.write( "processing "+parameters["method"]+" method" );

	// prepare command
	JSONFactory *factory = JSONFactory::getInstance();
	std::string command = "";
	factory->startJSON( command );

	for(std::map<std::string,std::string>::iterator iter=parameters.begin(); iter!=parameters.end(); iter++) 
	{
		factory->addParmToJSON( command, iter->first, iter->second );
	}

	factory->terminateJSON( command );
	pData->Log.writef( "   JSON string is %s", command.c_str() );
  

	// process command
	iRC = this->callSynchronize( command, response );
	if ( iRC != 0 )
	{
	    pData->Log.writef( "   synchronize call failed with rc=%ld", iRC );
	    return( iRC  );
	}
	else
	{
	    pData->Log.writef( "   synchronize call successful, response is %s", response.c_str() );
	}

	// extract some parameters from the response
	void *parseHandle = factory->parseJSONStart( response );
	if ( parseHandle == NULL )
	{
	    return( setError( "Error: internal function failed (parseJSONStart)", Error_InternalFunction_failed ) );
	}

	std::string responseMsg = "";
	while ( iRC == 0 )
	{
		std::string name;
		std::string value;
		iRC = factory->parseJSONGetNext( parseHandle, name, value );
		if ( iRC == 0 )
		{
			if ( stricmp( name.c_str(), RESPONSESTATUSMSG.c_str() ) == 0 )
			{
			    responseMsg = value;
			} 
			else if(requiredVal!="" && stricmp( name.c_str(), requiredVal.c_str())==0 ) 
			{
				retVal = value;
			}
		}
	}
	factory->parseJSONStop( parseHandle );


	if ( (iRC != 0) && (iRC != JSONFactory::INFO_ENDOFPARAMETERLISTREACHED) )
	{
	    return( setError( "Error: internal function failed (parseJSONGetNext)", Error_InternalFunction_failed ) );
	}

	if ( responseMsg.empty() )
	{
	    return( setError( "Error: incomplete response from web servive (ErrorCode is missing)", Error_IncompleteResponseFromWebService ) );
	}
	else 
	{
		if(responseMsg!="success")
			return( setError( responseMsg.c_str(), 1 ) );
		else
			iRC = 0;
	}

	return iRC;
}

void MemoryWebServiceClient::makeUpdateCounterFileName
(
  std::string &strPropPath, 
  std::string &strPropFile, 
  std::string &strUpdateCounterFileName 
)
{
	 strUpdateCounterFileName = strPropPath + "\\" + strPropFile;
     strUpdateCounterFileName.erase( strUpdateCounterFileName.length() - 4  );
     strUpdateCounterFileName.append( ".UDC");
}

/* \brief add a new user to a shared memory user list
   \param pvOptions pointer to vector containing the  addUser option strings   
   \returns 0
*/
int MemoryWebServiceClient::addMemoryUser
(
    std::vector<std::string> *pvOptions
)
{
   std::map<std::string,std::string> parameters;
   parameters.insert(std::map<std::string,std::string>::value_type("method", "adduser"));
   parameters.insert(std::map<std::string,std::string>::value_type("user-id",  (*pvOptions)[1]));
   parameters.insert(std::map<std::string,std::string>::value_type("password", (*pvOptions)[2]));
   parameters.insert(std::map<std::string,std::string>::value_type("name", (*pvOptions)[0]));
   parameters.insert(std::map<std::string,std::string>::value_type("userToAdd", (*pvOptions)[4]));

   std::string notRequired;
   return doSyncCall(parameters,"",notRequired);
}

/* \brief delete a new user from a shared memory user list
   \param pvOptions pointer to vector containing the  removeUser option strings   
   \returns 0
*/
int MemoryWebServiceClient::removeMemoryUser
(
    std::vector<std::string> *pvOptions
)
{

   std::map<std::string,std::string> parameters;
   parameters.insert(std::map<std::string,std::string>::value_type("method", "removeuser"));
   parameters.insert(std::map<std::string,std::string>::value_type("user-id",  (*pvOptions)[1]));
   parameters.insert(std::map<std::string,std::string>::value_type("password", (*pvOptions)[2]));
   parameters.insert(std::map<std::string,std::string>::value_type("name", (*pvOptions)[0]));
   parameters.insert(std::map<std::string,std::string>::value_type("userToRemove", (*pvOptions)[4]));

   std::string notRequired;
   return doSyncCall(parameters,"",notRequired);
}


/* \brief   list shared memory users
   \param   pvOptions pointer to vector containing the  addUser option strings  
   \param   users  shared memory users returned
   \returns 0
*/
int MemoryWebServiceClient::listMemoryUsers
(
    std::vector<std::string> &options,
	std::vector<std::string> &users
)
{

   std::map<std::string,std::string> parameters;
   parameters.insert(std::map<std::string,std::string>::value_type("method", "listuser"));
   parameters.insert(std::map<std::string,std::string>::value_type("user-id",  options[1]));
   parameters.insert(std::map<std::string,std::string>::value_type("password", options[2]));
   parameters.insert(std::map<std::string,std::string>::value_type("name", options[0]));

   std::string usersList;
   int iRC = doSyncCall(parameters,"userIdList",usersList);

   if ( iRC == 0 )
   {
     // fill caller's vector with the names of the memories
     int iStart = 0;
     do 
     {
        int iEnd = usersList.find( ",", iStart );
        int iLen = (iEnd == std::string::npos ) ? std::string::npos : iEnd - iStart;
        users.push_back( usersList.substr( iStart, iLen ) );
        iStart = (iEnd != std::string::npos) ? iEnd + 1 : iEnd;
     } while( iStart != std::string::npos );

   }

  return( iRC );
}

// load current update counter value
void MemoryWebServiceClient::loadUpdateCounter( std::string &strPropPath, std::string &strPropFileName, std::string &strUpdateCounter )
{
  strUpdateCounter = "0"; // set default

  std::string strUpdateCounterFileName;
  makeUpdateCounterFileName( strPropPath, strPropFileName, strUpdateCounterFileName );

  FILE *hf = fopen( strUpdateCounterFileName.c_str(), "rb" );
  if ( hf == NULL ) return;

  char szBuffer[256];
  memset( szBuffer, 0, sizeof(szBuffer) );
  fread( szBuffer, sizeof(szBuffer), 1, hf );
  fclose( hf );

  strUpdateCounter = szBuffer;
  return;
}

// write new update counter value
void MemoryWebServiceClient::writeUpdateCounter( std::string &strPropPath, std::string &strPropFileName, std::string &strUpdateCounter )
{
  std::string strUpdateCounterFileName;
  makeUpdateCounterFileName( strPropPath, strPropFileName, strUpdateCounterFileName );

  FILE *hf = fopen( strUpdateCounterFileName.c_str(), "wb" );
  if ( hf == NULL ) return;

  fwrite( strUpdateCounter.c_str(), strUpdateCounter.length() + 1, 1, hf );
  fclose( hf );

  return;
}
