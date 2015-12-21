/*! \file
	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

#include "string"
#include "vector"
#include "core\PluginManager\PluginManager.h"
#include "EqfSharedMemoryPlugin.h"
#include "EqfSharedMemory.h"
#include "eqftmi.h"
#include "MemoryWebServiceClient.h"
#include <TlHelp32.h>
#include <Shellapi.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

// name of the replicator exe
#define REPLICATOREXE "OtmMemReplicator.EXE"

// encryption password for option data
#define OPTIONSPASSWORD "poienmXCQM23Jiwpq?29jel"

// encryption password for properties data
#define PROPERTIESPASSWORD "pasdf09n456)dv!lxseq3=.Ö@"

// encryption password for access ata
#define ACCESSPASSWORD "asdfru14qq@pa68s08=df0<!j"


// data structure for inital and last used create options
typedef struct _CREATEOPTIONS
{
  char szWebServiceURL[256];
  char szUserID[256];
  char szPassword[256];
  char szDSGenericType[256];
  char szDSType[256];
  char szDSServer[256];
  char szDSPort[20];
  char szDSUser[256];
  char szDSPassword[256];
  char szUserList[1024];
} CREATEOPTIONS, *PCREATEOPTIONS;

// data structure for inital and last used create options
typedef struct _ACCESSOPTIONS
{
  char szWebServiceURL[256];
  char szUserID[256];
  char szPassword[256];
} ACCESSOPTIONS, *PACCESSOPTIONS;


// prototype for check create function
BOOL APIENTRY CheckCreateData( std::vector<std::string>&vFieldData, int *piIndex, char *pszErrorBuffer, int iBufSize, long lHandle, BOOL *pfOK );

// prototype for check access function
BOOL APIENTRY CheckAccessData( std::vector<std::string>&vFieldData, int *piIndex, char *pszErrorBuffer, int iBufSize, long lHandle, BOOL *pfOK );

// check if a program is running alread
BOOL isProgramRunning( char *pszProgram );

// function called by plugin manager to register this plugin
extern "C" {
__declspec(dllexport)
  USHORT registerPlugins()
  {
	  PluginManager::eRegRc eRc = PluginManager::eSuccess;
	  PluginManager *manager = PluginManager::getInstance();
	  EqfSharedMemoryPlugin* plugin = new EqfSharedMemoryPlugin();
	  eRc = manager->registerPlugin((OtmPlugin*) plugin);
      USHORT usRC = (USHORT) eRc;
      return usRC;
  }
}



EqfSharedMemoryPlugin::EqfSharedMemoryPlugin()
{
	name = "EqfSharedMemoryPlugin";
	shortDesc = "SharedTranslationMemoryPlugin";
	longDesc = "This is the standard shared Translation-Memory implementation";
	version = "1.0";
	supplier = "International Business Machines Corporation";
  descrType = "Shared Translation Memory (OpenTMS based)";
	pluginType = OtmPlugin::eSharedTranslationMemoryType;
	usableState = OtmPlugin::eUsable;

  // setup option fields for the memory create function
  this->vCreateLabels.resize( 12 );
  this->vCreateFieldData.resize( 12 );
  this->vCreateFieldDescr.resize( 12 );

  this->vCreateLabels[0] = "!Name";
  this->vCreateFieldDescr[0] = "(Name of the shared memory)";
  this->vCreateLabels[1] = "Service URL";
  this->vCreateFieldDescr[1] = "(URL of the Web Service)";
  this->vCreateLabels[2] = "User ID";
  this->vCreateFieldDescr[2] = "(User for the access to the memory)";
  this->vCreateLabels[3] = "@Password";
  this->vCreateFieldDescr[3] = "(Password for the user ID - Note: the password will not be displayed)";
  this->vCreateLabels[4] = "Datasource name";
  this->vCreateFieldDescr[4] = "(Data source name of the memory)";
  this->vCreateLabels[5] = "Data source generic type";
  this->vCreateFieldDescr[5] = "(generic type of the data source)";
  this->vCreateLabels[6] = "Data source type";
  this->vCreateFieldDescr[6] = "(Type of the data source)";
  this->vCreateLabels[7] = "Data source server";
  this->vCreateFieldDescr[7] = "(Server for the access to the data source)";
  this->vCreateLabels[8] = "Data source port";
  this->vCreateFieldDescr[8] = "(Port for the access to the data source)";
  this->vCreateLabels[9] = "Data source user ID";
  this->vCreateFieldDescr[9] = "(User ID for the access to the data source)";
  this->vCreateLabels[10] = "@Password";
  this->vCreateFieldDescr[10] = "(Password for the data source user ID -  Note: the password will not be displayed)";
  this->vCreateLabels[11] = "User list";
  this->vCreateFieldDescr[11] = "(comma separated list of users which may access the memory)";

  // setup option fields for the memory access 
  this->vAccessLabels.resize( 3 );
  this->vAccessFieldData.resize( 3 );
  this->vAccessFieldDescr.resize( 3 );

  this->vAccessLabels[0] = "Service URL";
  this->vAccessFieldDescr[0] = "(URL of the Web Service)";
  this->vAccessLabels[1] = "User ID";
  this->vAccessFieldDescr[1] = "(User for the access to the memory)";
  this->vAccessLabels[2] = "@Password";
  this->vAccessFieldDescr[2] = "(Password for the user ID - Note: the password will not be displayed)";

  // start logging
  //this->Log.open( "EqfSharedMemoryPlugin" );

  this->fReplicatorIsRunning = false;
}


EqfSharedMemoryPlugin::~EqfSharedMemoryPlugin()
{
  this->Log.close();
}

const char* EqfSharedMemoryPlugin::getName()
{
	return name.c_str();
}

const char* EqfSharedMemoryPlugin::getShortDescription()
{
	return shortDesc.c_str();
}

const char* EqfSharedMemoryPlugin::getLongDescription()
{
	return longDesc.c_str();
}

const char* EqfSharedMemoryPlugin::getVersion()
{
	return version.c_str();
}

const char* EqfSharedMemoryPlugin::getSupplier()
{
	return supplier.c_str();
}

/*! \brief Returns the descriptive type of the memories controlled by this plugin
*/
const char* EqfSharedMemoryPlugin::getDescriptiveMemType()
{
	return descrType.c_str();
}

/*! \brief Returns list of supported drive letters. 
	The list of drives is used to allow the selection of a drive letter in the memory create indow
	The drive letters are retunred as a null-terminated string containing the letters of the 
  supported drives. When no drive selection should be possible, an empty string should be returned.\n
	\param pszDriveListBuffer points to the callers buffer for the list of drive letter characters
	\returns 0 if successful or error return code */
int EqfSharedMemoryPlugin::getListOfSupportedDrives( char *pszDriveListBuffer )
{
  // no drives can be selected, so clear the list 
  *pszDriveListBuffer = '\0';
  return( 0 );
}

/*! \brief Check if the memory with the given name is owned by this plugin
\param pszName name of the memory
\returns TRUE if memory is owned by this plugin and FALSE if not */
BOOL EqfSharedMemoryPlugin::isMemoryOwnedByPlugin(
	PSZ pszName
)
{
  // check if there is one of our property files for this shared memory
  std::string strPropFile;
  this->makePropFileName( pszName, strPropFile );
  return( UtlFileExist( (char *)strPropFile.c_str() ) );
}

/*! \brief Tell if this shared memory plugin is using a local memory to keep a copy of the shared memory proposals
  \param pszName name of the memory
	\returns TRUE when a local memory is used by this plugin, FALSE is no local memory is used
*/
	BOOL EqfSharedMemoryPlugin::isLocalMemoryUsed()
  {
    // this shared memory plugin is using a local memory for the memory proposals and synchronizes this local copy
    // with the shared memory content
    return( TRUE );
  }

/*! \brief Create a new shared translation memory
  \param pszName name of the new memory
  \param pszSourceLanguage source language of the new memory
  \param pszDescription description for the new memory
  \param chDrive letter of the target drive for the memory (only used when drives can be selected)
	\param pvOptions pointer to a vector containing the create options
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
	\param pLocalMemory local version of shared memory being created
	\returns Pointer to created translation memory or NULL in case of errors
*/
OtmMemory* EqfSharedMemoryPlugin::createMemory(
	PSZ pszName,			  
	PSZ pszSourceLang,
	PSZ pszDescription,
  CHAR chDrive,
  std::vector<std::string> *pvOptions,
  OtmMemory *pLocalMemory
)
{
  int iRC = 0;

  pszSourceLang; pszDescription; chDrive;

  this->Log.writef( "Create the shared memory %s", pszName );
  this->Log.writef( "   Url=%s, UserID=%s", (*pvOptions)[1].c_str(), (*pvOptions)[2].c_str() );

  // write last used create options to disk
  this->writeLastUsedCreateValues( pvOptions );

  // set endpoint URL in WebServiceClient
  this->WebService.setEndpointUrl( (*pvOptions)[1] );

  // call web service to create the data source
  iRC = this->WebService.createMemory( pvOptions );
  if ( iRC != 0 )
  {
    this->iLastError = this->WebService.getLastError( this->strLastError );
    this->Log.writef( "   WebService method CreateMemory, WebService returned %s", this->strLastError.c_str() );
    return( NULL );
  }
  else
  {
    this->Log.write( "   WebService method CreateMemory completed successfully" );
  } /* endif */     

  // create shared memory properties
  iRC = this->createProperties( pszName, (*pvOptions)[1], (*pvOptions)[2], (*pvOptions)[3] );
  if ( iRC != 0 )
  {
    this->Log.writef( "   Property creation failed, error is %s", this->strLastError.c_str() );
    return( NULL );
  } /* endif */     

  // open the newly created shared memory
  return( this->openMemory( pszName, pLocalMemory, 0 ) );
}
	
/*! \brief Open an existing shared translation memory
  \param pszName name of the new memory
	\param pLocalMemory local version of shared memory being created
  \param usAccessMode, special access mode for memory: FOR_ORGANIZE, EXCLUSIVE, READONLY
	\returns Pointer to translation memory or NULL in case of errors
*/
OtmMemory* EqfSharedMemoryPlugin::openMemory(
	PSZ pszName,			  
  OtmMemory *pLocalMemory,
  USHORT usAccessMode
)
{
  int iRC = 0;

  usAccessMode; // acess mode not used for this type of memory

  this->Log.writef( "Open the shared memory %s", pszName );

  // load properties
  PSHAREDMEMPROP pProp = NULL;
  iRC = this->loadProperties( pszName, &pProp );
  if ( iRC != 0 )
  {
    this->iLastError = ERROR_WRITE_FAULT;
    this->strLastError = "Error opening shared memory properties";
    this->Log.writef( "   Load of properties failed, error is %s", this->strLastError.c_str() );
    return( NULL );
  } /* endif */     

  // create shared memory object
  EqfSharedMemory *pSharedMem = new( EqfSharedMemory );
  if ( pSharedMem != NULL )  
  {
    iRC = pSharedMem->initialize( this, pszName, pLocalMemory );
    if ( iRC != NULL )
    {
      this->iLastError = pSharedMem->getLastError( this->strLastError );
      this->Log.writef( "   Shared memory object could not be created, error is %s", this->strLastError.c_str() );
      delete( pSharedMem );
      pSharedMem = NULL;
    }
    else
    {
      this->Log.write( "   Shared memory opened successfully" );
    } /* endif */       
  }
  else
  {
    this->iLastError = ERROR_NOT_ENOUGH_MEMORY;
    this->strLastError = "Insufficient Memory, allocation of new memory obect failed";
    this->Log.writef( "   Shared memory object could not be created, error is %s", this->strLastError.c_str() );
  } /* end */     

  startReplicator();

  return( (OtmMemory *)pSharedMem );
  }

/*! \brief Close a memory
  \param pMemory pointer to memory object
*/
int EqfSharedMemoryPlugin::closeMemory(
	OtmMemory *pMemory			 
)
{
  int iRC = 0;

  if ( pMemory == NULL ) return( -1 );

  EqfSharedMemory *pSharedMem = (EqfSharedMemory *)pMemory;

  pSharedMem->terminate();

  delete( pSharedMem );

  return( iRC );
}

/*! \brief Connect to an existing shared translation memory
  \param pszName name of the memory being connected
     Option 0 = service URL
     Option 1 = user ID
     Option 2 = password
	\param pvOptions pointer to a vector containing the access options
	\returns 0 when successful or error return code
*/
int EqfSharedMemoryPlugin::connectToMemory(
	PSZ pszName,			  
  std::vector<std::string> *pvOptions
)
{
  startReplicator();
  return( this->createProperties( pszName,(*pvOptions)[0], (*pvOptions)[1], (*pvOptions)[2] ) ); 
}

/*! \brief Disconnect a shared translation memory
  \param pszName name of the memory being disconnected
	\returns 0 when successful or error return code
*/
int EqfSharedMemoryPlugin::disconnectMemory(
	PSZ pszName
)
{
  int iRC = 0;

  // load properties of shared memory
  PSHAREDMEMPROP pProp = NULL;
  iRC = this->loadProperties( pszName, &pProp );
  if ( iRC != 0 )
  {
    this->iLastError = ERROR_WRITE_FAULT;
    this->strLastError = "Error opening shared memory properties";
  } /* endif */     

  if(iRC == 0)
  {
    // delete property file
    std::string strPropFile;
    this->makePropFileName( pszName, strPropFile );
    UtlDelete( (char *)strPropFile.c_str(), 0, FALSE );

    // delete UDC file
    char szUDCName[MAX_LONGPATH];
    UtlMakeEQFPath( szUDCName, NULC, PROPERTY_PATH, NULL );
    strcat( szUDCName, BACKSLASH_STR );
    strcat( szUDCName, pszName );
    strcat( szUDCName, ".UDC" );
    UtlDelete( szUDCName, 0, FALSE );

    //remove in queue and out queue
  }
  
  if ( pProp ) UtlAlloc( (void **)&pProp, 0, 0, NOMSG );

  return( iRC );
}

/*! \brief Create properties for a shared memory
  \param pszName name of the memory being connected
  \param strServiceURL reference to a string with the service URL
  \param strUserID reference to a string with the User ID
  \param strPassword reference to a string with the password
	\returns 0 when successful or error return code
*/
int EqfSharedMemoryPlugin::createProperties(
	PSZ pszName,
  std::string strServiceURL,
  std::string strUserID,
  std::string strPassword
)
{
  int iRC = 0;

  // create shared memory properties
  PSHAREDMEMPROP pProp = new( SHAREDMEMPROP );
  memset( pProp, 0, sizeof( SHAREDMEMPROP ) );

  strcpy( pProp->szName, pszName );
  strcpy( pProp->szWebServiceURL, strServiceURL.c_str() );
  strcpy( pProp->szUserID, strUserID.c_str() );
  strcpy( pProp->szPassword, strPassword.c_str() );

  strcpy( pProp->szInQueueName, pszName );
  strcat( pProp->szInQueueName, ".IN" );
  strcpy( pProp->szOutQueueName, pszName );
  strcat( pProp->szOutQueueName, ".OUT" );
  
  // write properties to "pszName".SHP file
  iRC = this->writeProperties( pszName, pProp );
  delete( pProp );

  if ( iRC != 0 )
  {
    this->iLastError = ERROR_WRITE_FAULT;
    this->strLastError = "Error creating shared memory properties";
  } /* endif */     
  return( iRC );
}

  /*! \brief Provide a list of all available memories
  \param pfnCallBack callback function to be called for each memory
	\param pvData caller's data pointetr, is passed to callback function
	\param fWithDetails TRUE = supply memory details, when this flag is set, 
  the pInfo parameter of the callback function is set otherwise it is NULL
	\returns number of provided memories
*/
	int EqfSharedMemoryPlugin::listMemories(
		OtmMemoryPlugin::PFN_LISTMEMORY_CALLBACK pfnCallBack,			  
		void *pvData,
		BOOL fWithDetails
	)
  {
    // function is not supported by this plugin (all memories are already listed by lokal memory plugin)
    return( 0 );
  }

  /*! \brief Provide the names of shared memories
	\param pvOptions pointer to a vector containing the access options
  \param pvConnected pointer to a vector receiving the names of the connected memories
  \param pvNotConnected pointer to a vector receiving the names of the not connected memories
	\returns number of provided memories
*/
int EqfSharedMemoryPlugin::listSharedMemories(
  std::vector<std::string> *pvOptions,
  std::vector<std::string> *pvConnected,
  std::vector<std::string> *pvNotConnected
)
{
  int iMemories = 0;
  int iRC;
  std::vector<std::string> vMemList;

  this->Log.writef( "Retrieving available memories for the user %s", (*pvOptions)[1].c_str() );

  // get list of available shared translation memories for this user
  this->WebService.setEndpointUrl( (*pvOptions)[0] );
  iRC = WebService.listMemories( (char *)(*pvOptions)[1].c_str(), (char *)(*pvOptions)[2].c_str(), vMemList );
  if ( iRC != 0 )
  {
    this->iLastError = this->WebService.getLastError( this->strLastError );
    this->Log.writef( "   Web service client reported an error, error is %s", this->strLastError.c_str() );
    return( this->iLastError );
  }
  else
  {
    this->Log.writef( "   Web service client completed successfully, %ld memories are returned", vMemList.size() );
  } /* end */     

  std::string strPropFileName;
  for ( std::size_t i = 0; i < vMemList.size(); i++ )
  {
    this->makePropFileName( vMemList[i], strPropFileName );
    if ( UtlFileExist( (char *)strPropFileName.c_str() ) )
    {
      pvConnected->push_back( vMemList[i] );
      iMemories++;
    }
    else
    {
      pvNotConnected->push_back( vMemList[i] );
      iMemories++;
    } /* endif */       
  } /* end */     

  return( iMemories );
}

  /*! \brief Provide list of options required for the access to shared memories
  \param vLabels address of callers pointer to a vector containing the strings for the option field labels
       The first letter of the label text may define the type of the option field
       @ = password field
       ! = write protected field
       any other = text field
  \param ppFieldData address of callers pointer to a vector containing the inital values for the option fields
    this vector is passed to the creat method to create the shared memory
  \param ppFieldData address of callers pointer to a vector containing the inital values for the option fields
    this vector is passed to the creat method to create the shared memory
  \param ppFieldDescr address of callers pointer to a vector containing strings with descriptions for the option fields
  \param pfnCheckData pointer to a callback function checking the input data or NULL if not used
  \param plHandle pointer to a handle which is passed to the callback function
	\returns 0 if successful or error return code
*/
int EqfSharedMemoryPlugin::getAccessOptionFields(
  std::vector<std::string> **ppLabels,
  std::vector<std::string> **ppFieldData,
  std::vector<std::string> **ppFieldDescr,
  PFN_OPTIONSDIALOGCHECKDATA *ppfnCheckCallback,
  long *plHandle
)
{
  *ppLabels = &(this->vAccessLabels);
  *ppFieldData = &(this->vAccessFieldData);
  *ppFieldDescr = &(this->vAccessFieldDescr);

  PACCESSOPTIONS pOptions = NULL;

  this->vAccessFieldData[0] = "";
  this->vAccessFieldData[1] = "";
  this->vAccessFieldData[2] = "";

  // load last used options (if any)
  this->loadPropFile( "EqfSharedMemoryAccess.LastUsed", ACCESSPASSWORD, (void **)&pOptions, sizeof(ACCESSOPTIONS)  );

  // update data fields last used options
  if ( pOptions != NULL )
  {
    if ( pOptions->szWebServiceURL[0] != 0 ) this->vAccessFieldData[0] = pOptions->szWebServiceURL;  
    if ( pOptions->szUserID[0] != 0 )        this->vAccessFieldData[1] = pOptions->szUserID;
    if ( pOptions->szPassword[0] != 0 )      this->vAccessFieldData[2] = pOptions->szPassword;
    UtlAlloc( (void **)&pOptions, 0, 0, NOMSG );    
  } /* endif */     
 
  *ppfnCheckCallback = (PFN_OPTIONSDIALOGCHECKDATA)CheckAccessData;

  *plHandle = (long)this;


  return( 0 );
}


/*! \brief Get information about a memory
  \param pszName name of the memory
  \param pInfo pointer to buffer for memory information
	\returns 0 if successful or error return code
*/
int EqfSharedMemoryPlugin::getMemoryInfo(
	PSZ pszName,
  OtmMemoryPlugin::PMEMORYINFO pInfo
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
  pInfo; pszName;
  return( iRC );
}

   /*! \brief Physically rename a translation memory
  \param pszOldName name of the memory being rename
  \param pszNewName new name for the memory
	\returns 0 if successful or error return code
*/
int EqfSharedMemoryPlugin::renameMemory(
	  PSZ pszOldName,
    PSZ pszNewName
)
{
  int iRC = OtmMemoryPlugin::eNotSupported;
  pszOldName; pszNewName ;
  return( iRC );
}

/*! \brief provides a list of the memory data files

    \param pszName name of the memory
    \param pFileListBuffer  pointer to a buffer receiving the file names as a comma separated list
    \param iBufferSize      size of buffer in number of bytes

  	\returns 0 or error code in case of errors
*/
int EqfSharedMemoryPlugin::getMemoryFiles
(
  PSZ pszName,
  char *pFileListBuffer,
  int  iBufferSize
)
{
  int iRC = OtmMemoryPlugin::eNotSupported;
  pszName; pFileListBuffer; iBufferSize;
  return( iRC );
}

/*! \brief import a memory using a list of memory data files


   \param pszMemoryName    name of the memory 
   \param pFileList        pointer to a buffer containing the fully qualified memory data files as a comma separated list
   \param iOptions         processing options, one or more of the IMPORTFROMMEMFILES_..._OPT values ORed together
                           
   \param ppPrivateData    the address of a PVOID pointer which can be used to anchor private data. The
                           PVPOID pointer will be set to NULL on the initial call

  	\returns 0 if OK,
             OtmMemoryPlugin::eRepeat when the import needs more processing steps
             any other value is an error code
*/
int EqfSharedMemoryPlugin::importFromMemoryFiles
(
  char *pszMemoryName,
  char *pFileList,
  int  iOptions,
  PVOID *ppPrivateData
)
{
  int iRC = OtmMemoryPlugin::eNotSupported;
  pszMemoryName; pFileList; iOptions; ppPrivateData;
  return( iRC );
}


/*! \brief Physically delete a translation memory
  \param pszName name of the memory being deleted
	\returns 0 if successful or error return code
*/
int EqfSharedMemoryPlugin::deleteMemory(
	PSZ pszName			  
)
{
  PSHAREDMEMPROP pProp = NULL;
  int iRC = 0;

  this->Log.writef( "Load the shared memory %s", pszName );

  // load properties
  iRC = this->loadProperties( pszName, &pProp );
  if ( iRC != 0 )
  {
    this->iLastError = ERROR_WRITE_FAULT;
    this->strLastError = "Error loading shared memory properties";
    this->Log.writef( "   Load of properties failed, error is %s", this->strLastError.c_str() );
  } 

  if(iRC == 0)
  {
    // call web service to delete the data source
    std::vector<std::string> options;
    options.push_back(pszName);//name
    options.push_back(pProp->szUserID);//user-id
    options.push_back(pProp->szPassword);//password
    options.push_back(pszName);//dataSourceName

    this->WebService.setEndpointUrl(pProp->szWebServiceURL);
    // only the memory creator could delete the shared memory on server
    std::string creator;
    iRC = this->WebService.getCreator(pProp->szUserID, pProp->szPassword, pszName, creator);
    if(iRC == 0)
    {
	   this->Log.writef( "success when get creator for %s, creator name is %s\n", pszName,creator.c_str());
	   // if the user are just the creator, then could delete the shared memory on server
       int maxlen = creator.length()>strlen(pProp->szUserID)?creator.length():strlen(pProp->szUserID);
       if (strncmp(creator.c_str(),pProp->szUserID,maxlen) == 0)
       {
         iRC = this->WebService.deleteMemory(&options);
         if ( iRC != 0 )
         {
           this->iLastError = this->WebService.getLastError( this->strLastError );
           this->Log.writef( "   WebService method deleteMemory, WebService returned %s", this->strLastError.c_str() );
         }  
       }
    }
    else // get last error information
    {
        this->iLastError = this->WebService.getLastError( this->strLastError );
        this->Log.writef( "   WebService method deleteMemory, WebService returned %s", this->strLastError.c_str() );
    }
  }// end if
  
  //delete shared memory properties on local
  if(iRC == 0)
  {
    // delete shared memory properties
    std::string strPropFile;
    this->makePropFileName( pszName, strPropFile );
    iRC = UtlDelete( (char *)strPropFile.c_str(), 0, FALSE );
	
    // delete UDC file
    char szUDCName[MAX_LONGPATH];
    UtlMakeEQFPath( szUDCName, NULC, PROPERTY_PATH, NULL );
    strcat( szUDCName, BACKSLASH_STR );
    strcat( szUDCName, pszName );
    strcat( szUDCName, ".UDC" );
    if(UtlFileExist(szUDCName))
      iRC = UtlDelete( szUDCName, 0, FALSE );
  }

  if (pProp != NULL) UtlAlloc( (void **)&pProp, 0, 0, NOMSG );

  return( iRC );
}

/*! \brief Clear (i.e. remove all entries) a translation memory
  \param pszName name of the memory being cleared
	\returns 0 if successful or error return code
*/
int EqfSharedMemoryPlugin::clearMemory(
	PSZ pszName			  
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
  pszName;
  return( iRC );
}

/*! \brief Provide list of options required for the creation of shared memories
  \param pszName pointer to the name of the memory being created
  \param vLabels address of callers pointer to a vector containing the string for the option field labels
       The first letter of the label text may define the type of the option field
       @ = password field
       ! = write protected field
       any other = text field
  \param ppFieldData address of callers pointer to a vector containing the inital values for the option fields
    this vector is passed to the creat method to create the shared memory
  \param ppFieldData address of callers pointer to a vector containing the inital values for the option fields
    this vector is passed to the creat method to create the shared memory
  \param ppFieldDescr address of callers pointer to a vector containing strings with descriptions for the option fields
  \param pfnCheckData pointer to a function checking the input data or NULL if not used
    this vector is passed to the creat method to create the shared memory
  \param lHandle handle which is passed to the callback function
	\returns 0 if successful or error return code
*/
int EqfSharedMemoryPlugin::getCreateOptionFields(
	PSZ pszName,
  std::vector<std::string> **ppLabels,
  std::vector<std::string> **ppFieldData,
  std::vector<std::string> **ppFieldDescr,
  PFN_OPTIONSDIALOGCHECKDATA *ppfnCheckCallback,
  long *plHandle
)
{
  *ppLabels = &(this->vCreateLabels);
  *ppFieldData = &(this->vCreateFieldData);
  *ppFieldDescr = &(this->vCreateFieldDescr);

  // load default option values
  PCREATEOPTIONS pOptions = NULL;
  this->loadPropFile( "EqfSharedMemoryCreate.Defaults", OPTIONSPASSWORD, (void **)&pOptions, sizeof(CREATEOPTIONS)  );

  this->vCreateFieldData[0] = pszName;
  this->vCreateFieldData[2] = "";
  this->vCreateFieldData[3] = "";
  this->vCreateFieldData[4] = pszName;
  this->vCreateFieldData[9] = "";
  this->vCreateFieldData[10] = "";
  this->vCreateFieldData[11] = "";

  // update data fields with default options
  if ( pOptions != NULL )
  {
    if ( pOptions->szWebServiceURL[0] != 0 ) this->vCreateFieldData[1] = pOptions->szWebServiceURL;  
    if ( pOptions->szDSGenericType[0] != 0 ) this->vCreateFieldData[5] = pOptions->szDSGenericType;
    if ( pOptions->szDSType[0] != 0 )        this->vCreateFieldData[6] = pOptions->szDSType;
    if ( pOptions->szDSServer[0] != 0 )      this->vCreateFieldData[7] = pOptions->szDSServer;
    if ( pOptions->szDSPort[0] != 0 )        this->vCreateFieldData[8] = pOptions->szDSPort;
    UtlAlloc( (void **)&pOptions, 0, 0, NOMSG );    
  } /* endif */     

  // load last used options (if any)
  this->loadPropFile( "EqfSharedMemoryCreate.LastUsed", OPTIONSPASSWORD, (void **)&pOptions, sizeof(CREATEOPTIONS)  );

  // update data fields last used options
  if ( pOptions != NULL )
  {
    if ( pOptions->szWebServiceURL[0] != 0 ) this->vCreateFieldData[1] = pOptions->szWebServiceURL;  
    if ( pOptions->szUserID[0] != 0 )        this->vCreateFieldData[2] = pOptions->szUserID;
    if ( pOptions->szPassword[0] != 0 )      this->vCreateFieldData[3] = pOptions->szPassword;
    if ( pOptions->szDSGenericType[0] != 0 ) this->vCreateFieldData[5] = pOptions->szDSGenericType;
    if ( pOptions->szDSType[0] != 0 )        this->vCreateFieldData[6] = pOptions->szDSType;
    if ( pOptions->szDSServer[0] != 0 )      this->vCreateFieldData[7] = pOptions->szDSServer;
    if ( pOptions->szDSPort[0] != 0 )        this->vCreateFieldData[8] = pOptions->szDSPort;
    if ( pOptions->szDSUser[0] != 0 )        this->vCreateFieldData[9] = pOptions->szDSUser;
    if ( pOptions->szDSPassword[0] != 0 )    this->vCreateFieldData[10] = pOptions->szDSPassword;
    if ( pOptions->szUserList[0] != 0 )      this->vCreateFieldData[11] = pOptions->szUserList;
    UtlAlloc( (void **)&pOptions, 0, 0, NOMSG );    
  } /* endif */     
 
  *ppfnCheckCallback = (PFN_OPTIONSDIALOGCHECKDATA)CheckCreateData;

  *plHandle = (long)this;

  return( 0 );
}

/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
int EqfSharedMemoryPlugin::getLastError
(
  std::string &strError
)
{
  strError = this->strLastError;
  return( this->iLastError );
}

  /*! \brief Get the error message for the last error occured

    \param pszError pointer to a buffer for the error text
    \param iBufSize size of error text buffer in number of characters
  	\returns last error code
*/
int EqfSharedMemoryPlugin::getLastError
(
  char *pszError,
  int iBufSize
)
{
  strncpy( pszError, this->strLastError.c_str(), iBufSize );
  return( this->iLastError );
}



/* private methods */



/*! \brief Load memory properties file 
  \param pszName name of the property file
  \param pszPassword properties encryption password
  \param ppvProp adress of the pointer to the loaded properties file
  \param int iSize expected size of the loaded properties
	\returns 0 if successful or error return code
*/
int EqfSharedMemoryPlugin::loadPropFile(
	PSZ pszName,
  PSZ pszPassword,
  void **ppvProp,
  int iSize
)
{
  int iRC = 0;
  char szPath[512];
  unsigned short usSize = 0;

  UtlMakeEQFPath( szPath, NULC, PROPERTY_PATH, NULL );
  strcat( szPath, "\\" );
  strcat( szPath, pszName );

  *ppvProp = NULL;
  if ( !UtlLoadFile( szPath, ppvProp, &usSize, FALSE, FALSE ) )
  {
    return( GetLastError() );  
  } /* endif */     
  if ( usSize != (USHORT)iSize )
  {
    UtlAlloc( ppvProp, 0, 0, NOMSG );
    return( ERROR_READ_FAULT );  
  } /* endif */     


  // decrypt the data
  if ( pszPassword != NULL )
  {
    EqfSharedMemoryPlugin::encrypt( (PBYTE)*ppvProp, (int)usSize, pszPassword, FALSE );
  } /* endif */     

  return( iRC );
}

/*! \brief Write memory properties file 
  \param pszName name of the property file
  \param pszPassword properties encryption password
  \param pvProp pointer to the properties file
  \param iPropSize size o fthe properties in number of bytes
	\returns 0 if successful or error return code
*/
int EqfSharedMemoryPlugin::writePropFile(
	PSZ pszName,
  PSZ pszPassword,
  void *pvProp,
  int iPropSize
)
{
  int iRC = 0;
  char szPath[512];

  UtlMakeEQFPath( szPath, NULC, PROPERTY_PATH, NULL );
  strcat( szPath, "\\" );
  strcat( szPath, pszName );

  // encrypt the the data
  if ( pszPassword != NULL )
  {
    this->encrypt( (PBYTE)pvProp, iPropSize, pszPassword, TRUE );
  } /* endif */     

  if ( !UtlWriteFile( szPath, (USHORT)iPropSize, pvProp, FALSE ) )
  {
    return( GetLastError() );  
  } /* endif */     

  return( iRC );
}

/*! \brief Simple data decrypter/encrypter
  \param pbData pointer to data area being encrypter/decrypted
  \param iSize number of bytes in data area
  \param pszPassword password to be used for decryption/encryption
  \param fEncrypt true = encrypt, false = decrypt
	\returns 0
*/
int EqfSharedMemoryPlugin::encrypt( PBYTE pbData, int iSize, PSZ pszPassword, BOOL fEncrypt )
{
  if ( fEncrypt )
  {
    BYTE bLastValue = 0;
    int iLen = strlen(pszPassword);
    int iPW = 0;
    for ( int i = 0; i < iSize; i++ )
    {
      BYTE bValue = (BYTE)(pbData[i] + bLastValue);
      bValue = (BYTE)(bValue + (pszPassword[iPW] * (iPW+1) * 3));
      bLastValue = bValue;
      pbData[i] = bValue;
      iPW++;
      if ( iPW == iLen ) iPW = 0;
    } /* endfor */
  }
  else
  {
    BYTE bLastValue = 0;
    int iLen = strlen(pszPassword);
    int iPW = 0;
    for ( int i = 0; i < iSize; i++ )
    {
      BYTE bValue = pbData[i];
      bValue = (BYTE)(bValue - (pszPassword[iPW] * (iPW+1) * 3));
      bValue = (BYTE)(bValue - bLastValue);
      bLastValue = pbData[i];
      pbData[i] = bValue;
      iPW++;
      if ( iPW == iLen ) iPW = 0;
    } /*endfor */
  } /* endif */
  return( 0 );
}

/*! \brief prototype for a callback function checking the user input for the create method
  \param vFieldData reference to a vector with the user input
    for a list of the user input fields see createMemory method
  \param piIndex pointer to callers buffer for the index of the field in error
  \param pszErrorBuffer pointer to a buffer for the error message
  \param iBufSize size of error text buffer
  \param lHandle handle passed from the calling funciton (in ou case yued for a pointer to the plugin object)
	\returns true if input is correct otherwise false
*/
BOOL APIENTRY CheckCreateData( std::vector<std::string>&vFieldData, int *piIndex, char *pszErrorBuffer, int iBufSize, long lHandle, BOOL *pfOK )
{
  BOOL fOK = TRUE;
  EqfSharedMemoryPlugin *pPlugin = (EqfSharedMemoryPlugin *)lHandle;

  // check memory name (should not be empty)
  if ( fOK && vFieldData[0].empty() )
  {
    strncpy( pszErrorBuffer, "Memory name is required", iBufSize );
    *piIndex = 0;
    fOK = FALSE;
  } /* end */     

  // check web service URL (should not be empty)
  if ( fOK && vFieldData[1].empty() )
  {
    strncpy( pszErrorBuffer, "Web Service URL is required", iBufSize );
    *piIndex = 1;
    fOK = FALSE;
  } /* end */     

  // check user ID (should not be empty)
  if ( fOK && vFieldData[2].empty() )
  {
    strncpy( pszErrorBuffer, "User ID is required", iBufSize );
    *piIndex = 2;
    fOK = FALSE;
  } /* end */     

  // check password (should not be empty)
  if ( fOK && vFieldData[3].empty() )
  {
    strncpy( pszErrorBuffer, "Password is required", iBufSize );
    *piIndex = 3;
    fOK = FALSE;
  } /* end */     

  // check data source name (should not be empty)
  if ( fOK && vFieldData[4].empty() )
  {
    strncpy( pszErrorBuffer, "Data source name is required", iBufSize );
    *piIndex = 4;
    fOK = FALSE;
  } /* end */     

  // check generic type (should not be empty)
  if ( fOK && vFieldData[5].empty() )
  {
    strncpy( pszErrorBuffer, "Data source generic type is required", iBufSize );
    *piIndex = 5;
    fOK = FALSE;
  } /* end */     

  // check generic type (should not be empty)
  if ( fOK && vFieldData[6].empty() )
  {
    strncpy( pszErrorBuffer, "Data source type is required", iBufSize );
    *piIndex = 6;
    fOK = FALSE;
  } /* end */     

  // check generic type (should not be empty)
  if ( fOK && vFieldData[7].empty() )
  {
    strncpy( pszErrorBuffer, "Data source server is required", iBufSize );
    *piIndex = 7;
    fOK = FALSE;
  } /* end */     

  // check port (should not be empty)
  if ( fOK && vFieldData[8].empty() )
  {
    strncpy( pszErrorBuffer, "Data source port is required", iBufSize );
    *piIndex = 8;
    fOK = FALSE;
  } /* end */     

  // check port (should be numeric)
  if ( fOK && (vFieldData[8].find_first_not_of( "0123456789" ) != std::string::npos) )
  {
    strncpy( pszErrorBuffer, "Data source port has to be numeric", iBufSize );
    *piIndex = 8;
    fOK = FALSE;
  } /* end */     

  // check data source user ID (should not be empty)
  if ( fOK && vFieldData[9].empty() )
  {
    strncpy( pszErrorBuffer, "Data source user ID is required", iBufSize );
    *piIndex = 9;
    fOK = FALSE;
  } /* end */     

  // check data source user ID password (should not be empty)
  if ( fOK && vFieldData[10].empty() )
  {
    strncpy( pszErrorBuffer, "Password for data source user ID is required", iBufSize );
    *piIndex = 10;
    fOK = FALSE;
  } /* end */     

  // save current values as last last-used values
  pPlugin->writeLastUsedCreateValues( &vFieldData );
  *pfOK = fOK;

  return( fOK );
}


/*! \brief prototype for a callback function checking the user input for the access/list memories method
  \param vFieldData reference to a vector with the user input
    for a list of the user input fields see createMemory method
  \param piIndex pointer to callers buffer for the index of the field in error
  \param pszErrorBuffer pointer to a buffer for the error message
  \param iBufSize size of error text buffer
  \param lHandle handle passed from the calling funciton (in ou case yued for a pointer to the plugin object)
	\returns true if input is correct otherwise false
*/
BOOL APIENTRY CheckAccessData( std::vector<std::string>&vFieldData, int *piIndex, char *pszErrorBuffer, int iBufSize, long lHandle, BOOL *pfOK )
{
  BOOL fOK = TRUE;
  EqfSharedMemoryPlugin *pPlugin = (EqfSharedMemoryPlugin *)lHandle;

  // check web service URL (should not be empty)
  if ( fOK && vFieldData[0].empty() )
  {
    strncpy( pszErrorBuffer, "Web Service URL is required", iBufSize );
    *piIndex = 0;
    fOK = FALSE;
  } /* end */     

  // check user ID (should not be empty)
  if ( fOK && vFieldData[1].empty() )
  {
    strncpy( pszErrorBuffer, "User ID is required", iBufSize );
    *piIndex = 1;
    fOK = FALSE;
  } /* end */     

  // check password (should not be empty)
  if ( fOK && vFieldData[2].empty() )
  {
    strncpy( pszErrorBuffer, "Password is required", iBufSize );
    *piIndex = 2;
    fOK = FALSE;
  } /* end */     


  // save current values as last last-used values
  pPlugin->writeLastUsedAccessValues( &vFieldData );
  *pfOK = fOK;

  return( fOK );
}


/*! \brief write last used create values to disk
  \param pvOptions pointer to a vector with the user input
	\returns true if input is correct otherwise false
*/
int EqfSharedMemoryPlugin::writeLastUsedCreateValues( std::vector<std::string> *pvOptions )
{

  // write last used create options to disk
  PCREATEOPTIONS pOptions = NULL;
  UtlAlloc( (void **)&pOptions, 0, sizeof(CREATEOPTIONS), NOMSG );    
  if ( pOptions != NULL )
  {
    strcpy( pOptions->szWebServiceURL, (*pvOptions)[1].c_str() );
    strcpy( pOptions->szUserID,        (*pvOptions)[2].c_str() );
    strcpy( pOptions->szPassword,      (*pvOptions)[3].c_str() );
    strcpy( pOptions->szDSGenericType, (*pvOptions)[5].c_str() );
    strcpy( pOptions->szDSType,        (*pvOptions)[6].c_str() );
    strcpy( pOptions->szDSServer,      (*pvOptions)[7].c_str() );
    strcpy( pOptions->szDSPort,        (*pvOptions)[8].c_str() );
    strcpy( pOptions->szDSUser,        (*pvOptions)[9].c_str() );
    strcpy( pOptions->szDSPassword,    (*pvOptions)[10].c_str() );
    strcpy( pOptions->szUserList,      (*pvOptions)[11].c_str() );
    this->writePropFile( "EqfSharedMemoryCreate.LastUsed", OPTIONSPASSWORD, (void *)pOptions, sizeof(CREATEOPTIONS) );
    UtlAlloc( (void **)&pOptions, 0, 0, NOMSG );    
  } /* endif */     
  return( 0 );
}

/*! \brief write last used access values to disk
  \param pvOptions pointer to a vector with the user input
	\returns true if input is correct otherwise false
*/
int EqfSharedMemoryPlugin::writeLastUsedAccessValues( std::vector<std::string> *pvOptions )
{

  // write last used access options to disk
  PACCESSOPTIONS pOptions = NULL;
  UtlAlloc( (void **)&pOptions, 0, sizeof(ACCESSOPTIONS), NOMSG );    
  if ( pOptions != NULL )
  {
    strcpy( pOptions->szWebServiceURL, (*pvOptions)[0].c_str() );
    strcpy( pOptions->szUserID,        (*pvOptions)[1].c_str() );
    strcpy( pOptions->szPassword,      (*pvOptions)[2].c_str() );
    this->writePropFile( "EqfSharedMemoryAccess.LastUsed", ACCESSPASSWORD, (void *)pOptions, sizeof(ACCESSOPTIONS) );
    UtlAlloc( (void **)&pOptions, 0, 0, NOMSG );    
  } /* endif */     
  return( 0 );
}



/*! \brief write the properties of a shared memory to disk
  \param pszMemoryName name of the memory
  \param pProps pointer to property structure
	\returns true if input is correct otherwise false
*/
int EqfSharedMemoryPlugin::writeProperties( char *pszMemory, PSHAREDMEMPROP pProps )
{
  std::string strPropFile;
  this->makePropFileName( pszMemory, strPropFile );

  return( this->writePropFile( UtlGetFnameFromPath( (char *)strPropFile.c_str()), PROPERTIESPASSWORD, (void *)pProps, sizeof(SHAREDMEMPROP ) ) );

}

  /*! \brief write the properties of a shared memory to disk
  \param pszMemoryName name of the memory
  \param pProps address of callers property structure pointer
	\returns true if input is correct otherwise false
*/
int EqfSharedMemoryPlugin::loadProperties( char *pszMemory, PSHAREDMEMPROP *ppProps )
{
  std::string strPropFile;
  this->makePropFileName( pszMemory, strPropFile );

  return( this->loadPropFile( UtlGetFnameFromPath( (char *)strPropFile.c_str()), PROPERTIESPASSWORD, (void **)ppProps, sizeof(SHAREDMEMPROP)  ) );
}


/*! \brief make the fully qualified path name of a shared memory property file
  \param pszMemoryName name of the memory
  \param strPropFile reference to string receiving the property file name
	\returns true if input is correct otherwise false
*/
int EqfSharedMemoryPlugin::makePropFileName( char *pszMemory, std::string &strPropFile )
{
  char szPropName[MAX_LONGPATH];

  UtlMakeEQFPath( szPropName, NULC, PROPERTY_PATH, NULL );
  strcat( szPropName, BACKSLASH_STR );
  strcat( szPropName, pszMemory );
  strcat( szPropName, ".SHP" );
  strPropFile = szPropName;
  return( 0 );
}

  /*! \brief make the fully qualified path name of a shared memory property file
  \param strMemoryName reference to string containg the name of the memory
  \param strPropFile reference to string receiving the property file name
	\returns true if input is correct otherwise false
*/
int EqfSharedMemoryPlugin::makePropFileName( std::string strMemory, std::string &strPropFile )
{
  char szPropName[MAX_LONGPATH];

  UtlMakeEQFPath( szPropName, NULC, PROPERTY_PATH, NULL );
  strcat( szPropName, BACKSLASH_STR );
  strcat( szPropName, strMemory.c_str() );
  strcat( szPropName, ".SHP" );
  strPropFile = szPropName;
  return( 0 );
}

  /*! \brief load the properties of a shared memory from disk 
  This is a static method which can be called without having a 
  EqfSharedMemoryplugin object
  \param pszPropFileName fully qualified name of property file
  \param pProps address of callers property structure 
	\returns 0 when successful
*/
int EqfSharedMemoryPlugin::loadPropertyFile( const char *pszPropFile, EqfSharedMemoryPlugin::PSHAREDMEMPROP pProps )
{
  int iRC = 0;
  char szPath[512];
  unsigned short usSize = 0;

  iRC; szPath; usSize;
  memset( pProps, 0, sizeof(EqfSharedMemoryPlugin::SHAREDMEMPROP) );

  FILE *hfProp = fopen( pszPropFile, "rb" );
  if ( hfProp == NULL ) return( GetLastError() );

  if ( fread( pProps, sizeof(EqfSharedMemoryPlugin::SHAREDMEMPROP), 1, hfProp ) != 1 )
  {
    int iRC = GetLastError();  
    fclose( hfProp );
    return( iRC );
  } /* endif */     
  fclose( hfProp );

  // decrypt the data
  EqfSharedMemoryPlugin::encrypt( (PBYTE)pProps, sizeof(EqfSharedMemoryPlugin::SHAREDMEMPROP), PROPERTIESPASSWORD, FALSE );

  return( 0 );
}

BOOL isProgramRunning( char *pszProgram )
{
  HANDLE hSnapshot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
  if ( hSnapshot != INVALID_HANDLE_VALUE)
  {
    PROCESSENTRY32 ProcessEntry;
    BOOL fMore = FALSE;

    ProcessEntry.dwSize = sizeof(PROCESSENTRY32);       /* 2-24-14 */
    fMore = Process32First( hSnapshot, &ProcessEntry );
    while ( fMore )
    {
      if ( stricmp( ProcessEntry.szExeFile, pszProgram ) == 0 )
      {
        return( TRUE );
      } /* endif */         
      fMore = Process32Next( hSnapshot, &ProcessEntry );
    } /* endwhile */       
    CloseHandle( hSnapshot );
  } /* endif */     

  return( FALSE );

}

/* \brief add a new user to a shared memory user list
   \param pszMemName   memory name
   \param pszUserName  user name to add
   \returns 0
*/
int EqfSharedMemoryPlugin::addMemoryUser(
    PSZ pszName,
    PSZ userName
)
{
  PSHAREDMEMPROP pProp = NULL;
  int iRC = 0;

  this->Log.writef( "Load the shared memory %s", pszName );

  // load properties
  iRC = this->loadProperties( pszName, &pProp );
  if ( iRC != 0 )
  {
    this->iLastError = ERROR_WRITE_FAULT;
    this->strLastError = "Error loading shared memory properties";
    this->Log.writef( "   Load of properties failed, error is %s", this->strLastError.c_str() );
    // don't forget to realse memory
    if (pProp != NULL) UtlAlloc( (void **)&pProp, 0, 0, NOMSG );

    return( ERROR_WRITE_FAULT );
  } 
  
  // call web service to delete the data source
  std::vector<std::string> options;
  options.push_back(pszName);//name
  options.push_back(pProp->szUserID);//user-id
  options.push_back(pProp->szPassword);//password
  options.push_back(pszName);//memory name
  options.push_back(userName);//user name
  
  this->WebService.setEndpointUrl(pProp->szWebServiceURL);
  iRC = this->WebService.addMemoryUser(&options);
  if ( iRC != 0 )
  {
    this->Log.writef( "   WebService method addMemoryUser, WebService returned %s", this->strLastError.c_str() );
    this->iLastError = this->WebService.getLastError( this->strLastError );
    iRC = this->iLastError;
  }
  else
  {
    this->Log.write( "   WebService method addMemoryUser completed successfully" );
  }  
  
  if (pProp != NULL) UtlAlloc( (void **)&pProp, 0, 0, NOMSG );

  return( iRC );
}

/* \brief delete a new user from a shared memory user list
   \param pszMemName   memory name
   \param pszUserName  user name to add
   \returns 0
*/
int EqfSharedMemoryPlugin::removeMemoryUser(
    PSZ pszName,
    PSZ userName
)
{
  PSHAREDMEMPROP pProp = NULL;
  int iRC = 0;

  this->Log.writef( "Load the shared memory %s", pszName );

  // load properties
  iRC = this->loadProperties( pszName, &pProp );
  if ( iRC != 0 )
  {
    this->iLastError = ERROR_WRITE_FAULT;
    this->strLastError = "Error loading shared memory properties";
    this->Log.writef( "   Load of properties failed, error is %s", this->strLastError.c_str() );

    if (pProp != NULL)  UtlAlloc( (void **)&pProp, 0, 0, NOMSG );

    return( ERROR_WRITE_FAULT );
  } 
  
  // call web service to delete the data source
  std::vector<std::string> options;
  options.push_back(pszName);//name
  options.push_back(pProp->szUserID);//user-id
  options.push_back(pProp->szPassword);//password
  options.push_back(pszName);//memory name
  options.push_back(userName);//user name
  
  this->WebService.setEndpointUrl(pProp->szWebServiceURL);
  iRC = this->WebService.removeMemoryUser(&options);
  if ( iRC != 0 )
  {
    this->iLastError = this->WebService.getLastError( this->strLastError );
    iRC = this->iLastError;
    this->Log.writef( "   WebService method addMemoryUser, WebService returned %s", this->strLastError.c_str() );
  }
  else
  {
    this->Log.write( "   WebService method addMemoryUser completed successfully" );
  }  
  
	
  if (pProp != NULL) UtlAlloc( (void **)&pProp, 0, 0, NOMSG );

  return( iRC );
}

/* \brief list shared memory users
   \param pszMemName   memory name
   \param users        users name returned
   \returns 0
*/
int EqfSharedMemoryPlugin::listMemoryUsers(
    PSZ pszName,
    std::vector<std::string> &users
    )
{
  int iUsers = 0;
  int iRC;
  std::vector<std::string> vMemList;
  std::vector<std::string> options;
  PSHAREDMEMPROP pProp = NULL;

  iUsers;
  this->Log.writef( "Load the shared memory %s", pszName );

  // load properties
  iRC = this->loadProperties( pszName, &pProp );
  if ( iRC != 0 )
  {
    this->iLastError = ERROR_WRITE_FAULT;
    this->strLastError = "Error loading shared memory properties";
    this->Log.writef( "   Load of properties failed, error is %s", this->strLastError.c_str() );

    if (pProp != NULL) 
      UtlAlloc( (void **)&pProp, 0, 0, NOMSG );

    return( ERROR_WRITE_FAULT );
  } 
 
  options.push_back(pszName);//name
  options.push_back(pProp->szUserID);//user-id
  options.push_back(pProp->szPassword);//password
  options.push_back(pszName);//memory name

  // get list of available shared translation memories for this user
  this->WebService.setEndpointUrl(pProp->szWebServiceURL);
  iRC = this->WebService.listMemoryUsers( options, vMemList );
  if ( iRC != 0 )
  {
    this->iLastError = this->WebService.getLastError( this->strLastError );
    iRC = this->iLastError;
    this->Log.writef( "   Web service client reported an error, error is %s", this->strLastError.c_str() );
  }
  else
  {
    this->Log.writef( "   Web service client completed successfully, %ld memories are returned", vMemList.size() );

    std::string strPropFileName;
    for ( std::size_t i = 0; i < vMemList.size(); i++ )
    {
      users.push_back( vMemList[i] );
    }      
  } /* end */     

  if (pProp != NULL) UtlAlloc( (void **)&pProp, 0, 0, NOMSG );

  return iRC;
}

bool EqfSharedMemoryPlugin::stopPlugin( bool fForce  )
{

  // TODO: check for active objects..
  bool fActiveObjects = false;

  // decline stop if we have active objects
  if ( !fForce && fActiveObjects )
  {
    return( false );
  }

  // terminate memory replicator application
  stopReplicator();

  // TODO: terminate active objects, cleanup, free allocated resources

  // de-register plugin
	PluginManager *pPluginManager = PluginManager::getInstance();
	pPluginManager->deregisterPlugin( (OtmPlugin *)this );

  return( true );
}

/*! \brief Stop the memory replicator application
	\returns true when successful 
*/
BOOL EqfSharedMemoryPlugin::stopReplicator()
{
  bool fClosed = true;

  // find memory replicator main window
  HWND hwndMainWindow = FindWindow( NULL, "OpenTM2 Memory Replicator" );
  if ( hwndMainWindow != NULLHANDLE )
  {
    SendMessage(hwndMainWindow, WM_CLOSE, NULL, NULL);
    fClosed = true;
  }
  return( fClosed );
}

BOOL EqfSharedMemoryPlugin::startReplicator()
{
  // start our replicator program if it is not active yet
  if ( this->fReplicatorIsRunning ) return( true );

  // do not start replicator in API session mode
  if ( UtlQueryUShort( QS_RUNMODE ) == FUNCCALL_RUNMODE ) return( false );

  this->fReplicatorIsRunning = isProgramRunning( REPLICATOREXE );
  if ( !this->fReplicatorIsRunning ) 
  {
    char szProgramPath[MAX_LONGPATH];
    char szReplicator[MAX_LONGPATH];

    // // get fully qualified path name of our plugin
    //GetModuleFileName( (HINSTANCE)&__ImageBase, szProgramPath, MAX_LONGPATH );

    // // cut off plugin DLL name
    // char *pszEndPath = strrchr( szProgramPath, '\\' );
    // if ( pszEndPath != NULL ) *pszEndPath = '\0';

    // in the current version we use the program path!
    UtlMakeEQFPath( szProgramPath, NULC, PROGRAM_PATH, NULL );

    // build replicator exe path
    strcpy( szReplicator, szProgramPath );
    strcat( szReplicator, "\\" );
    strcat( szReplicator, REPLICATOREXE );

    // start our replicator exe
    ShellExecute( NULL, "open", szReplicator, NULL, szProgramPath, SW_SHOWMINNOACTIVE );

    this->fReplicatorIsRunning = true;
  } /* endif */       
  return( this->fReplicatorIsRunning );
}

/* \brief add a new memory information to memory list
   \param pszName memory name
   \param chToDrive drive letter
   \returns 0 if success
*/
int EqfSharedMemoryPlugin::addMemoryToList(PSZ pszName, CHAR chDrive)
{
    //user can't create such memory from commandline,so leave it here for future
  return OtmMemoryPlugin::eNotSupported;
}

/* \brief remove a memory information from memory list
   \param  pszName memory name
   \returns 0 if success
*/
int EqfSharedMemoryPlugin::removeMemoryFromList(PSZ pszName)
{
    //user can't create such memory from commandline,so leave it here for future
  return OtmMemoryPlugin::eNotSupported;
}

/* \brief set the owner of the memory
 \param pszMemoryName memory name
\param  pszOwner new owner for the memory
\returns 0 if success
*/
int EqfSharedMemoryPlugin::setOwner( PSZ pszMemoryName, PSZ pszOwner )
{
    // the setting of owner is not supported in this plugin
  return OtmMemoryPlugin::eNotSupported;
}

 /* \brief Replace the data of one memory with the data of another memory and delete the remains of the second memory
    \param pszReplace name of the memory whose data is being replaced
    \param pszReplaceWith name of the memory whose data will be used to replace the data of the other memory
   \returns 0 if success
*/
int EqfSharedMemoryPlugin::replaceMemory( PSZ pszReplace, PSZ pszReplaceWith )
{
    // the setting of owner is not supported in this plugin
  return OtmMemoryPlugin::eNotSupported;
}

