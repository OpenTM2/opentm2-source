//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#define WIN32
#define AXIS2_DECLARE_EXPORT


#include "axis2_stub_OpenTMSWebServiceImplementationService.h"

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

    pData->stub = axis2_stub_create_OpenTMSWebServiceImplementationService( pData->env, pData->client_home, pData->strEndPointURL.c_str() );
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
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  std::string response = "";
  int iRC = 0;
  List.clear();

  if ( pData == NULL )
  {
    return( setError( "Error: internal data not allocated", Error_PrivateDataNotSet ) );
  } /* endif */       

  pData->Log.write( "processing listMemories method" );

  // prepare command
  JSONFactory *factory = JSONFactory::getInstance();
  std::string command = "";
  factory->startJSON( command );
  factory->addParmToJSON( command, "method", "query" );
  factory->addParmToJSON( command, "user-id", pszUser );
  factory->addParmToJSON( command, "password", pszPassword );
  factory->addParmToJSON( command, "name", "" );
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
    pData->Log.write( "   error processing response string" );
    return( setError( "Error: internal function failed (parseJSONStart)", Error_InternalFunction_failed ) );
  } /* end */       

  std::string strErrorCode = "";
  std::string strErrorMessage = "";
  std::string strMemoryList = "";
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
      else if ( stricmp( name.c_str(), "memory-list" ) == 0 )
      {
        strMemoryList = value;
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

  // fill caller's vector with the names of the memories
  int iStart = 0;
  do 
  {
    int iEnd = strMemoryList.find( ",", iStart );
    int iLen = (iEnd == std::string::npos ) ? std::string::npos : iEnd - iStart;
    List.push_back( strMemoryList.substr( iStart, iLen ) );
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
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  std::string response = "";
  int iRC = 0;

  if ( pData == NULL )
  {
    return( setError( "Error: internal data not allocated", Error_PrivateDataNotSet ) );
  } /* endif */       

  pData->Log.write( "processing createMemory method" );

  // prepare command
  JSONFactory *factory = JSONFactory::getInstance();
  std::string command = "";
  factory->startJSON( command );
  factory->addParmToJSON( command, "method", "create" );
  factory->addParmToJSON( command, "user-id", (*pvOptions)[2] );
  factory->addParmToJSON( command, "password", (*pvOptions)[3] );
  factory->addParmToJSON( command, "name", (*pvOptions)[0] );
  std::string parameter = "dataSourceName=" + (*pvOptions)[4] + ";dataSourceGenericType=" + (*pvOptions)[5] + ";dataSourceType=" + (*pvOptions)[6] + 
    ";dataSourceServer=" + (*pvOptions)[7] + ";dataSourcePort=" + (*pvOptions)[8] + ";dataSourceUser=" + (*pvOptions)[9] + ";dataSourcePassword=" + (*pvOptions)[10];
  factory->addParmToJSON( command, "parameter", parameter );
  factory->addParmToJSON( command, "user-id-list", (*pvOptions)[11] );
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

  return( iRC );
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
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  std::string response = "";
  int iRC = 0;

  if ( pData == NULL )
  {
    return( setError( "Error: internal data not allocated", Error_PrivateDataNotSet ) );
  } /* endif */       

  pData->Log.write( "processing deleteMemory method" );

  // prepare command
  JSONFactory *factory = JSONFactory::getInstance();
  std::string command = "";
  factory->startJSON( command );
  factory->addParmToJSON( command, "method", "delete" );
  factory->addParmToJSON( command, "user-id", (*pvOptions)[1] );
  factory->addParmToJSON( command, "password", (*pvOptions)[2] );
  factory->addParmToJSON( command, "name", (*pvOptions)[0] );
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
  
  std::string strErrorMessage = "";
  std::string strErrorCode = "";
  std::string result = "";
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
      else if ( stricmp( name.c_str(), "errorstring" ) == 0 )
      {
        strErrorMessage = value;
      }
      else if ( stricmp( name.c_str(), "result" ) == 0 )
      {
        result = value;
      } 
      
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
    pData->Log.writef( "synchronize reported error %ld, message us %s", iRC, strErrorMessage.c_str() );
    strErrorMessage.insert( 0, "WebServiceError: " );
    return( setError( strErrorMessage.c_str(), iRC ) );
  } /* end */  
  
  return( iRC );
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
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  std::string response = "";
  int iRC = 0;


  if ( pData == NULL )
  {
    return( setError( "Error: internal data not allocated", Error_PrivateDataNotSet ) );
  } /* endif */       

  pData->Log.writef( "processing uploadProposal method, proposal is %s", strTMXProposal.c_str() );

  // prepare command
  JSONFactory *factory = JSONFactory::getInstance();
  std::string command = "";
  factory->startJSON( command );
  factory->addParmToJSON( command, "method", "upload" );
  factory->addParmToJSON( command, "user-id", pszUserID );
  factory->addParmToJSON( command, "password", pszPassword );
  factory->addParmToJSON( command, "name", pszName );
  factory->addParmToJSON( command, "parameter", "empty" );
  factory->addParmToJSON( command, "tmx-document", strTMXProposal );
  factory->addParmToJSON( command, "encoding", "UTF-8" );
  factory->addParmToJSON( command, "update-counter", "" );
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

  return( iRC );
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

        //strTMXProposal = value;
      } 
      else if ( stricmp( name.c_str(), "update-counter" ) == 0 )
      {
        strUpdateCounter = value;
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
    pData->Log.writef( "synchronize reported error %ld, message us %s", iRC, strErrorMessage.c_str() );
    strErrorMessage.insert( 0, "WebServiceError: " );
    return( setError( strErrorMessage.c_str(), iRC ) );
  } /* end */       

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
  factory->addParmToJSON( command, "method", "getcreator" );
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

  // extract some parameters from the response
  void *parseHandle = factory->parseJSONStart( response );
  if ( parseHandle == NULL )
  {
    return( setError( "Error: internal function failed (parseJSONStart)", Error_InternalFunction_failed ) );
  } /* end */       

  std::string result = "";
  std::string strErrorCode = "";
  std::string strErrorMessage = "";
  while ( iRC == 0 )
  {
    std::string name;
    std::string value;
    iRC = factory->parseJSONGetNext( parseHandle, name, value );
    if ( iRC == 0 )
    {
      if ( stricmp( name.c_str(), "result" ) == 0 )
      {
        result = value;
      } 
	  else if ( stricmp( name.c_str(), "ErrorCode" ) == 0 )
      {
        strErrorCode = value;
      } 
	  else if ( stricmp( name.c_str(), "errorstring" ) == 0 )
	  {
		strErrorMessage = value;
	  }
      else if ( stricmp( name.c_str(), "creator" ) == 0 )
      {
        creator = value;
      } 
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
    pData->Log.writef( "synchronize reported error %ld, message us %s", iRC, strErrorMessage.c_str() );
    strErrorMessage.insert( 0, "WebServiceError: " );
    return( setError( strErrorMessage.c_str(), iRC ) );
  }

  return( iRC );
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
  response = axis2_stub_op_OpenTMSWebServiceImplementationService_synchronize( pData->stub, pData->env, synch );

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

/* \brief add a new user to a shared memory user list
   \param pvOptions pointer to vector containing the  addUser option strings   
   \returns 0
*/
int MemoryWebServiceClient::addMemoryUser
(
    std::vector<std::string> *pvOptions
)
{
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  std::string response = "";
  int iRC = 0;

  if ( pData == NULL )
  {
    return( setError( "Error: internal data not allocated", Error_PrivateDataNotSet ) );
  } /* endif */       

  pData->Log.write( "processing addUser method" );

  // prepare command
  JSONFactory *factory = JSONFactory::getInstance();
  std::string command = "";
  factory->startJSON( command );
  factory->addParmToJSON( command, "method", "adduser" );
  factory->addParmToJSON( command, "user-id", (*pvOptions)[1] );
  factory->addParmToJSON( command, "password", (*pvOptions)[2] );
  factory->addParmToJSON( command, "name", (*pvOptions)[0] );
  factory->addParmToJSON( command, "newuser", (*pvOptions)[4] );
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

  std::string strErrorMessage = "";
  std::string strErrorCode = "";
  std::string strUerList = "";
  std::string result = "";
  while ( iRC == 0 )
  {
    std::string name;
    std::string value;
    iRC = factory->parseJSONGetNext( parseHandle, name, value );
    if ( iRC == 0 )
    {
      if ( stricmp( name.c_str(), "result" ) == 0 )
      {
        result = value;
      } 
	  else if ( stricmp( name.c_str(), "ErrorCode" ) == 0 )
      {
        strErrorCode = value;
      } 
      else if ( stricmp( name.c_str(), "errorstring" ) == 0 )
      {
          strErrorMessage = value;
      }
      else if ( stricmp( name.c_str(), "userlist" ) == 0 )
      {
        strUerList = value;
      }
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
    pData->Log.writef( "synchronize reported error %ld, message us %s", iRC, strErrorMessage.c_str() );
    strErrorMessage.insert( 0, "WebServiceError: " );
    return( setError( strErrorMessage.c_str(), iRC ) );
  } /* end */ 
  
  return( iRC );
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
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  std::string response = "";
  int iRC = 0;

  if ( pData == NULL )
  {
    return( setError( "Error: internal data not allocated", Error_PrivateDataNotSet ) );
  } /* endif */       

  pData->Log.write( "processing addUser method" );

  // prepare command
  JSONFactory *factory = JSONFactory::getInstance();
  std::string command = "";
  factory->startJSON( command );
  factory->addParmToJSON( command, "method", "removeuser" );
  factory->addParmToJSON( command, "user-id", (*pvOptions)[1] );
  factory->addParmToJSON( command, "password", (*pvOptions)[2] );
  factory->addParmToJSON( command, "name", (*pvOptions)[0] );
  factory->addParmToJSON( command, "removeuser", (*pvOptions)[4] );
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

  std::string strErrorMessage = "";
  std::string strErrorCode = "";
  std::string strUserList = "";
  std::string result = "";
  while ( iRC == 0 )
  {
    std::string name;
    std::string value;
    iRC = factory->parseJSONGetNext( parseHandle, name, value );
    if ( iRC == 0 )
    {
      if ( stricmp( name.c_str(), "result" ) == 0 )
      {
        result = value;
      } 
	  else if ( stricmp( name.c_str(), "ErrorCode" ) == 0 )
      {
        strErrorCode = value;
      } 
      else if ( stricmp( name.c_str(), "errorstring" ) == 0 )
      {
        strErrorMessage = value;
      }
      else if ( stricmp( name.c_str(), "userlist" ) == 0 )
      {
        strUserList = value;
      } 
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
    pData->Log.writef( "synchronize reported error %ld, message us %s", iRC, strErrorMessage.c_str() );
    strErrorMessage.insert( 0, "WebServiceError: " );
    return( setError( strErrorMessage.c_str(), iRC ) );
  } /* end */      

  return( iRC );
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
  PWEBCLIENTDATA pData = (PWEBCLIENTDATA)this->pvPrivateData;
  std::string response = "";
  int iRC = 0;
  users.clear();

  if ( pData == NULL )
  {
    return( setError( "Error: internal data not allocated", Error_PrivateDataNotSet ) );
  } /* endif */       

  pData->Log.write( "processing listMemories method" );

  // prepare command
  JSONFactory *factory = JSONFactory::getInstance();
  std::string command = "";
  factory->startJSON( command );
  factory->addParmToJSON( command, "method", "listuser" );
  factory->addParmToJSON( command, "user-id", options[1] );
  factory->addParmToJSON( command, "password", options[2] );
  factory->addParmToJSON( command, "name",options[0] );
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
    pData->Log.write( "   error processing response string" );
    return( setError( "Error: internal function failed (parseJSONStart)", Error_InternalFunction_failed ) );
  } /* end */       

  std::string strErrorMessage = "";
  std::string strErrorCode = "";
  std::string result = "";
  std::string strUserList = "";
  while ( iRC == 0 )
  {
    std::string name;
    std::string value;
    iRC = factory->parseJSONGetNext( parseHandle, name, value );
    if ( iRC == 0 )
    {
      if ( stricmp( name.c_str(), "result" ) == 0 )
      {
        result = value;
      } 
	  else if ( stricmp( name.c_str(), "ErrorCode" ) == 0 )
      {
        strErrorCode = value;
      } 
      else if (stricmp(name.c_str(),"errorstring") == 0)
      {
        strErrorMessage = value;
      }
      else if ( stricmp( name.c_str(), "userlist" ) == 0 )
      {
        strUserList = value;
      } 
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
    pData->Log.writef( "synchronize reported error %ld, message us %s", iRC, strErrorMessage.c_str() );
    strErrorMessage.insert( 0, "WebServiceError: " );
    return( setError( strErrorMessage.c_str(), iRC ) );
  } /* end */     

  if ( iRC == 0 )
  {
     // fill caller's vector with the names of the memories
     int iStart = 0;
     do 
     {
        int iEnd = strUserList.find( ",", iStart );
        int iLen = (iEnd == std::string::npos ) ? std::string::npos : iEnd - iStart;
        users.push_back( strUserList.substr( iStart, iLen ) );
        iStart = (iEnd != std::string::npos) ? iEnd + 1 : iEnd;
     } while( iStart != std::string::npos );

  } /* end */       

  return( iRC );
}