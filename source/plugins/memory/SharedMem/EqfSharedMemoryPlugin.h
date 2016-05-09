/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _EqfSharedMemoryPlugin_H_
#define _EqfSharedMemoryPlugin_H_

#include <string>
#include <vector>
#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "core\PluginManager\OtmSharedMemoryPlugin.h"
#include "core\Utilities\LogWriter.h"
#include "MemoryWebServiceClient.h"

class OtmMemory;

class EqfSharedMemoryPlugin: public OtmSharedMemoryPlugin
/*! \brief This class implements the standard shared translation memory plugin (EQF) for OpenTM2.
*/

{
public:
/*! \brief Constructor
*/
	EqfSharedMemoryPlugin();
/*! \brief Destructor
*/
	~EqfSharedMemoryPlugin();
/*! \brief Returns the name of the plugin
*/
	const char* getName();
/*! \brief Returns a short plugin-Description
*/
	const char* getShortDescription();
/*! \brief Returns a verbose plugin-Description
*/
	const char* getLongDescription();
/*! \brief Returns the version of the plugin
*/
	const char* getVersion();
/*! \brief Returns the name of the plugin-supplier
*/
	const char* getSupplier();

/*! \brief Returns the descriptive type of the memories controlled by this plugin
*/
	const char* getDescriptiveMemType();

  /*! \brief Check if the memory with the given name is owned by this plugin
  \param pszName name of the memory
	\returns TRUE if memory is owned by this plugin and FALSE if not
*/
	BOOL isMemoryOwnedByPlugin(
		PSZ pszName
	);



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
	OtmMemory* createMemory(
		PSZ pszName,			  
	  PSZ pszSourceLang,
	  PSZ pszDescription,
    CHAR chDrive,
    std::vector<std::string> *pvOptions,
    OtmMemory *pLocalMemory
	);

/*! \brief Connect to an existing shared translation memory
  \param pszName name of the memory being connected
     Option 0 = service URL
     Option 1 = user ID
     Option 2 = password
	\param pvOptions pointer to a vector containing the access options
	\returns 0 when successful or error return code
*/
	int connectToMemory(
		PSZ pszName,			  
    std::vector<std::string> *pvOptions
	);
	
/*! \brief Disconnect a shared translation memory
  \param pszName name of the memory being disconnected
	\returns 0 when successful or error return code
*/
	int disconnectMemory(
		PSZ pszName
	);


/*! \brief Open an existing shared translation memory
  \param pszName name of the new memory
	\param pLocalMemory local version of shared memory being created
  \param usAccessMode, special access mode for memory: FOR_ORGANIZE, EXCLUSIVE, READONLY
	\returns Pointer to translation memory or NULL in case of errors
*/
	OtmMemory* openMemory(
		PSZ pszName,			  
    OtmMemory *pLocalMemory,
    USHORT usAccessMode
	);

  /*! \brief Provide a list of all available memories
  \param pfnCallBack callback function to be called for each memory
	\param pvData caller's data pointetr, is passed to callback function
	\param fWithDetails TRUE = supply memory details, when this flag is set, 
  the pInfo parameter of the callback function is set otherwise it is NULL
	\returns number of provided memories
*/
	int listMemories(
		OtmMemoryPlugin::PFN_LISTMEMORY_CALLBACK pfnCallBack,			  
		void *pvData,
		BOOL fWithDetails
	);

/*! \brief Provide the names of shared memories
	\param pvOptions pointer to a vector containing the access options
  \param pvConnected pointer to a vector receiving the names of the connected memories
  \param pvNotConnected pointer to a vector receiving the names of the not connected memories
	\returns number of provided memories
*/
	int listSharedMemories(
    std::vector<std::string> *pvOptions,
    std::vector<std::string> *pvConnected,
    std::vector<std::string> *pvNotConnected
	);

/*! \brief Close a memory
  \param pMemory pointer to memory object
*/
  int closeMemory(
	  OtmMemory *pMemory			 
  );

/*! \brief Physically delete a translation memory
  \param pszName name of the memory being deleted
	\returns 0 if successful or error return code
*/
	int deleteMemory(
		PSZ pszName			  
	);

/*! \brief Clear (i.e. remove all entries) a translation memory
  \param pszName name of the memory being cleared
	\returns 0 if successful or error return code
*/
	int clearMemory(
		PSZ pszName			  
	);

/*! \brief Get information about a memory
  \param pszName name of the memory
  \param pInfo pointer to buffer for memory information
	\returns 0 if successful or error return code
*/
	int getMemoryInfo(
		PSZ pszName,
    OtmMemoryPlugin::PMEMORYINFO pInfo
	);

/*! \brief provides a list of the memory data files

    \param pszName name of the memory
    \param pFileListBuffer  pointer to a buffer receiving the file names as a comma separated list
    \param iBufferSize      size of buffer in number of bytes

  	\returns 0 or error code in case of errors
*/
  int getMemoryFiles
  (
    PSZ pszName,
    char *pFileListBuffer,
    int  iBufferSize
  );

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
  int importFromMemoryFiles
  (
    char *pszMemoryName,
    char *pFileList,
    int  iOptions,
    PVOID *ppPrivateData
  );

/*! \brief Physically rename a translation memory
  \param pszOldName name of the memory being rename
  \param pszNewName new name for the memory
	\returns 0 if successful or error return code
*/
int renameMemory(
	  PSZ pszOldName,
    PSZ pszNewName
);

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
  \param plHandle pointer to a handle which is passed to the callback function
	\returns 0 if successful or error return code
*/
	int getCreateOptionFields(
		PSZ pszName,
    std::vector<std::string> **ppLabels,
    std::vector<std::string> **ppFieldData,
    std::vector<std::string> **ppFieldDescr,
    PFN_OPTIONSDIALOGCHECKDATA *ppfnCheckCallback,
    long *plHandle
	);


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
	int getAccessOptionFields(
    std::vector<std::string> **ppLabels,
    std::vector<std::string> **ppFieldData,
    std::vector<std::string> **ppFieldDescr,
    PFN_OPTIONSDIALOGCHECKDATA *ppfnCheckCallback,
    long *plHandle
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


/*! \brief Returns list of supported drive letters. 
	The list of drives is used to allow the selection of a drive letter in the memory create indow
	The drive letters are retunred as a null-terminated string containing the letters of the 
  supported drives. When no drive selection should be possible, an empty string should be returned.\n
	\param pszDriveListBuffer points to the callers buffer for the list of drive letter characters
	\returns 0 if successful or error return code */
	int getListOfSupportedDrives( char *pszDriveListBuffer );

/*! \brief write last used create values to disk
  \param pvOptions pointer to a vector with the user input
	\returns true if input is correct otherwise false
*/
  int writeLastUsedCreateValues( std::vector<std::string> *pvOptions );

/*! \brief write last used access values to disk
  \param pvOptions pointer to a vector with the user input
	\returns true if input is correct otherwise false
*/
  int writeLastUsedAccessValues( std::vector<std::string> *pvOptions );


/*! \brief Property data of shared memories
*/
  #define NAME_LEN 512
  typedef struct _SHAREDMEMPROP
  {
    char szName[NAME_LEN];           // name of memory
    char szWebServiceURL[NAME_LEN];  // URL of web service
    char szUserID[NAME_LEN];         // user ID for the access to the memory
    char szPassword[NAME_LEN];       // password for the user ID
    char szInQueueName[NAME_LEN];    // name of input queue
    char szOutQueueName[NAME_LEN];   // name of input queue
    char szUnUsed[8096];             // room for enhancements
  } SHAREDMEMPROP, *PSHAREDMEMPROP;


/*! \brief write the properties of a shared memory to disk
  \param pszMemoryName name of the memory
  \param pProps pointer to property structure
	\returns true if input is correct otherwise false
*/
  int writeProperties( char *pszMemory, PSHAREDMEMPROP pProps );

  /*! \brief write the properties of a shared memory to disk
  \param pszMemoryName name of the memory
  \param pProps address of callers property structure pointer
	\returns true if input is correct otherwise false
*/
  int loadProperties( char *pszMemory, PSHAREDMEMPROP *ppProps );


  /*! \brief Provide a list of all available but not yet connected shared memories
  \param pfnCallBack callback function to be called for each memory
	\param pvData caller's data pointetr, is passed to callback function
	\param fWithDetails TRUE = supply memory details, when this flag is set, 
  the pInfo parameter of the callback function is set otherwise it is NULL
	\param pvOptions pointer to a vector containing the access options
	\returns number of provided memories
*/
	int listNotConnectedMemories(
		OtmMemoryPlugin::PFN_LISTMEMORY_CALLBACK pfnCallBack,			  
		void *pvData,
		BOOL fWithDetails,
    std::vector<std::string> *pvOptions
	);

  /*! \brief load the properties of a shared memory from disk 
  This is a static method which can be called without having a 
  EqfSharedMemoryplugin object
  \param pszPropFileName fully qualified name of property file
  \param pProps address of callers property structure 
	\returns 0 when successful
*/
static int EqfSharedMemoryPlugin::loadPropertyFile( const char *pszPropFile, EqfSharedMemoryPlugin::PSHAREDMEMPROP pProps );

/* \brief add a new user to a shared memory user list
   \param pszMemName   memory name
   \param pszUserName  user name to add
   \returns 0
*/
int addMemoryUser(PSZ pszName, PSZ userName);

/* \brief delete a user from a shared memory user list
   \param pszMemName   memory name
   \param pszUserName  user name to add
   \returns 0
*/
int removeMemoryUser(PSZ pszName, PSZ userName);

/* \brief list shared memory users
   \param pszMemName   memory name
   \param users        users name returned
   \returns 0
*/
int listMemoryUsers(PSZ pszName, std::vector<std::string> &users);

int replicateWithServer(PSZ pszName,OtmMemory* pLocalMem, bool isUpload);

/*! \brief Stops the plugin. 
	Terminating-function for the plugin, will be called directly before
	the DLL containing the plugin will be unloaded.\n
	The method should call PluginManager::deregisterPlugin() to tell the PluginManager
  that the plugin is not active anymore.
  Warning: THIS METHOD SHOULD BE CALLED BY THE PLUGINMANAGER ONLY!
	\param fForce, TRUE = force stop of the plugin even if functions are active, FALSE = only stop plugin when it is inactive
	\returns TRUE when successful */
	bool stopPlugin( bool fForce = FALSE );

  /*! \brief Tell if this shared memory plugin is using a local memory to keep a copy of the shared memory proposals
  \param pszName name of the memory
	\returns TRUE when a local memory is used by this plugin, FALSE is no local memory is used
*/
	BOOL isLocalMemoryUsed();

  /* \brief add a new memory information to memory list
   \param pszName memory name
   \param chToDrive drive letter
   \returns 0 if success
  */
  int addMemoryToList(PSZ pszName, CHAR chToDrive);

  /* \brief remove a memory information from memory list
   \param  pszName memory name
   \returns 0 if success
  */
  int removeMemoryFromList(PSZ pszName);

   /* \brief set the owner of the memory
    \param pszMemoryName memory name
    \param  pszOwner new owner for the memory
   \returns 0 if success
   */
   int setOwner( PSZ pszMemoryName, PSZ pszOwner);

 /* \brief Replace the data of one memory with the data of another memory and delete the remains of the second memory
    \param pszReplace name of the memory whose data is being replaced
    \param pszReplaceWith name of the memory whose data will be used to replace the data of the other memory
   \returns 0 if success
*/
 int replaceMemory( PSZ pszReplace, PSZ pszReplaceWith );

private:
  std::string name;
	std::string shortDesc;
	std::string longDesc;
	std::string version;
	std::string supplier;
	std::string descrType;
  std::vector<std::string> vCreateLabels;
  std::vector<std::string> vCreateFieldData;
  std::vector<std::string> vCreateFieldDescr;
  std::vector<std::string> vAccessLabels;
  std::vector<std::string> vAccessFieldData;
  std::vector<std::string> vAccessFieldDescr;
	std::string strLastError;
	int iLastError;
  MemoryWebServiceClient WebService;
  LogWriter Log;
  BOOL fReplicatorIsRunning;


/*! \brief Load memory properties file 
  \param pszName name of the property file
  \param pszPassword properties encryption password
  \param ppvProp adress of the pointer to the loaded properties file
  \param int iSize expected size of the loaded properties
	\returns 0 if successful or error return code
*/
	int loadPropFile(
		PSZ pszName,
    PSZ pszPassword,
    void **ppvProp,
    int iSize
	);

/*! \brief Write memory properties file 
  \param pszName name of the property file
  \param pszPassword properties encryption password
  \param pvProp pointer to the properties file
  \param iPropSize size o fthe properties in number of bytes
	\returns 0 if successful or error return code
*/
  int writePropFile(
	  PSZ pszName,
    PSZ pszPassword,
    void *pvProp,
    int iPropSize
  );

/*! \brief make the fully qualified path name of a shared memory property file
  \param pszMemoryName name of the memory
  \param strPropFile reference to string receiving the property file name
	\returns true if input is correct otherwise false
*/
  int makePropFileName( char *pszMemory, std::string &strPropFile );

  /*! \brief make the fully qualified path name of a shared memory property file
  \param strMemoryName reference to string containg the name of the memory
  \param strPropFile reference to string receiving the property file name
	\returns true if input is correct otherwise false
*/
  int makePropFileName( std::string strMemory, std::string &strPropFile );

/*! \brief Simple data decrypter/encrypter
  \param pbData pointer to data area being encrypter/decrypted
  \param iSize number of bytes in data area
  \param pszPassword password to be used for decryption/encryption
  \param fEncrypt true = encrypt, false = decrypt
	\returns 0
*/
static int encrypt( PBYTE pbData, int iSize, PSZ pszPassword, BOOL fEncrypt );

/*! \brief Create properties for a shared memory
  \param pszName name of the memory being connected
  \param strServiceURL reference to a string with the service URL
  \param strUserID reference to a string with the User ID
  \param strPassword reference to a string with the password
	\returns 0 when successful or error return code
*/
int createProperties(
	PSZ pszName,			  
  std::string strServiceURL,
  std::string strUserID,
  std::string strPassword
);


/*! \brief Stop the memory replicator application
	\returns true when successful 
*/
BOOL stopReplicator();

/*! \brief Starts the memory replicator application if it is not active yet
	\returns true when successful 
*/
BOOL startReplicator();

void initCreationDatas();
int batchUpload(PSZ pszName, PSZ userID, PSZ password, OtmMemory* pLocalMem);
int batchDownload(PSZ pszName, PSZ userID, PSZ password, OtmMemory* pLocalMem);

};

#endif // #ifndef _EqfSharedMemoryPlugin_H_