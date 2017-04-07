/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _EqfMemoryPlugin_H_
#define _EqfMemoryPlugin_H_

#include <string>
#include <vector>
#include <memory>
#include "core\PluginManager\OtmMemoryPlugin.h"

class OtmMemory;

class EqfMemoryPlugin: public OtmMemoryPlugin
/*! \brief This class implements the standard translation memory plugin (EQF) for OpenTM2.
*/

{
public:
/*! \brief Constructor
*/
	EqfMemoryPlugin();
/*! \brief Destructor
*/
	~EqfMemoryPlugin();
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


/*! \brief Create a new translation memory
  \param pszName name of the new memory
	\param pszSourceLang source language
	\param pszDescription description of the memory
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
  \param chDrive drive where new memory should be created, or 0 if memory should be created on primary drive
	\returns Pointer to created translation memory or NULL in case of errors
*/
	OtmMemory* createMemory(
		PSZ pszName,			  
		PSZ pszSourceLang,
		PSZ pszDescription,
		BOOL bMsgHandling,
		HWND hwnd,
    CHAR chDrive
	);
	
/*! \brief Open an existing translation memory
  \param pszName name of the existing memory
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
  \param usAccessMode, special access mode for memory: FOR_ORGANIZE, EXCLUSIVE, READONLY
	\returns Pointer to translation memory or NULL in case of errors
*/
	OtmMemory* openMemory(
		PSZ pszName,			  
		BOOL bMsgHandling,
		HWND hwnd,
    unsigned short usAccessMode = 0
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
    PMEMORYINFO pInfo
	);

/*! \brief set description of a memory
  \param pszName name of the memory
  \param pszDesc description information
	\returns 0 if successful or error return code
*/
    int setDescription(
        PSZ pszName,
        PSZ pszDesc);

  /*! \brief provides a list of the memory data files

     This method returns a list of the files which together form the specific memory.
     If there are no real physical files also a dummy name can be and
     the contents of this dummy file can be generated dynamically when the getMemoryPart
     method is applied on this memory.

     The file names are passed to the getMemoryPart method to extract the memory data in
     binary form.

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

    This method imports the binary files of a memory. The files have been created and
    filled using the getMemoryPart method.

    This method should delete the memory data files at the end of the processing- 

    When the processing of the memory files needs more time, the method
    should process the task in small units in order to prevent blocking of the
    calling application. To do this the method should return
    OtmMemoryPugin::eRepeat and should use the pPrivData pointer to anchor
    a private data area to keep track of the current processing step. The method will
    be called repetetively until the import has been completed.

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
  char *pFileListBuffer,
  int  iOptions,
  PVOID *ppPrivateData
);

  /*! \brief Physically rename a translation memory
  \param pszOldName name of the memory being rename
  \param pszNewName new name for the memory
	\returns 0 if successful or error return code
*/
  int renameMemory(
	  PSZ pszOldNae,
    PSZ pszNewName
  );

  /*! \brief Create a temporary memory
  \param pszPrefix prefix to be used for name of the temporary memory
  \param pszName buffer for the name of the temporary memory
	\param pszSourceLang source language
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
	\returns Pointer to created translation memory or NULL in case of errors
*/
  OtmMemory* createTempMemory(
	  PSZ pszPrefix,			  
	  PSZ pszName,			  
	  PSZ pszSourceLang,
	  BOOL bMsgHandling,
	  HWND hwnd
  );

  /*! \brief Close and delete a temporary memory
  \param pMemory pointr to memory object
*/
  void closeTempMemory(
	  OtmMemory *pMemory
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
  

/*! \brief Stops the plugin. 
	Terminating-function for the plugin, will be called directly before
	the DLL containing the plugin will be unloaded.\n
	The method should call PluginManager::deregisterPlugin() to tell the PluginManager
  that the plugin is not active anymore.
  Warning: THIS METHOD SHOULD BE CALLED BY THE PLUGINMANAGER ONLY!
	\param fForce, TRUE = force stop of the plugin even if functions are active, FALSE = only stop plugin when it is inactive
	\returns TRUE when successful */
	bool stopPlugin( bool fForce = false );

/*! \brief Handle a return code from the memory functions and create the approbriate error message text for it
    \param iRC return code from memory function
    \param pszMemName long memory name
    \param pszMarkup markup table name or NULL if not available
    \param pszMemPath fully qualified memory path name or NULL if not available
    \param strLastError reference to string object receiving the message text
    \param iLastError reference to a integer variable receiving the error code
  	\returns original or modified error return code
*/
static int handleError( int iRC, char *pszMemName, char *pszMarkup, char *pszMemPath, std::string &strLastError, int &iLastError );
 
/*! \brief Returns list of supported drive letters. 
	The list of drives is used to allow the selection of a drive letter in the memory create indow
	The drive letters are retunred as a null-terminated string containing the letters of the 
  supported drives. When no drive selection should be possible, an empty string should be returned.\n
	\param pszDriveListBuffer points to the callers buffer for the list of drive letter characters
	\returns 0 if successful or error return code */
	int getListOfSupportedDrives( char *pszDriveListBuffer );

    /* \brief add a new memory information to memory list
       \param pszName memory name
       \param chToDrive drive letter
       \returns 0 if success
    */
    int addMemoryToList( PSZ pszName, CHAR chDrive );

    /* \brief remove a memory information from memory list
       \param  pszName memory name
       \returns 0 if success
    */
    int removeMemoryFromList(PSZ pszName);

    /* \brief Replace the data of one memory with the data of another memory and delete the remains of the second memory
      \param pszReplace name of the memory whose data is being replaced
      \param pszReplaceWith name of the memory whose data will be used to replace the data of the other memory
     \returns 0 if success
    */
    int replaceMemory( PSZ pszReplace, PSZ pszReplaceWith );

private:

  BOOL makeMemoryPath( PSZ pszName, CHAR chDrive, std::string &strPathName, BOOL fReserve = FALSE, PBOOL pfReserved = NULL );
  void refreshMemoryList();
  OtmMemoryPlugin::PMEMORYINFO findMemory( char *pszName );
  int findMemoryIndex(char *pszName);
/*! \brief Create memory properties
  \param pszName long name of the memory
  \param strPathName memory path name
	\param pszDescription memory description
	\param pszSourceLanguage memory source language
	\returns TRUE when successful, FALSE in case of errors
*/
BOOL createMemoryProperties( PSZ pszName, std::string &strPathName, PSZ pszDescription, PSZ pszSourceLanguage );
/*! \brief Create memory properties
  \param pszName long name of the memory
  \param strPathName memory path name
	\param pvOldProperties existing property file to be used for the fields of the new properties
	\returns TRUE when successful, FALSE in case of errors
*/
BOOL createMemoryProperties( PSZ pszName, std::string &strPathName, void *pvOldProperties );

/*! \brief Make the fully qualified property file name for a memory
  \param strPathName reference to the memory path name
  \param strPropName reference to the string receiving the property file name
	\returns 0 when successful
*/
int makePropName( std::string &strPathName, std::string &strPropName );

/*! \brief Add memory to our internal memory lisst
  \param strPathName reference to the memory path name
	\returns 0 when successful
*/
  int addToList( std::string &strPathName );

/*! \brief Add memory to our internal memory lisst
  \param pszPropname pointer to property file name
	\returns 0 when successful
*/
  int addToList( char *pszPropName );

/*! \brief Fill memory info structure from memory properties
  \param pszPropName name of the memory property file (w/o path) 
	\param pInfo pointer to memory info structure
	\returns TRUE when successful, FALSE in case of errors
*/
  BOOL fillInfoStructure( char *pszPropName, PMEMORYINFO pInfo );

	std::string name;
	std::string shortDesc;
	std::string longDesc;
	std::string version;
	std::string supplier;
	std::string descrType;
	std::string strLastError;
	int iLastError;
  std::vector< std::shared_ptr<MEMORYINFO> > m_MemInfoVector;
  char szBuffer[4000];                         // general purpose buffer area
  char szSupportedDrives[27]; // list of supported drives


/*! \brief make Index filename from memory data file name
  \param pszMemPath pointer to memory data file name
  \param pszIndexFileName pointer to a buffer for the memory index file name
	\returns 0 when successful
*/
int makeIndexFileName( char *pszMemPath, char *pszIndexFileName );

/*! \brief make Index filename from memory data file name
  \param strMemPath reference to a string containing the memory data file name
  \param strIndexFileName reference to a string receiving the memory index file name
	\returns 0 when successful
*/
int makeIndexFileName( std::string &strMemPath, std::string &strIndexFileName );

/*! \brief Initialize the import of a memory using a list of memory data files


   \param pszMemoryName    name of the memory 
   \param pFileList        pointer to a buffer containing the fully qualified memory data files as a comma separated list
   \param iOptions         processing options, one or more of the IMPORTFROMMEMFILES_..._OPT values ORed together
                           
   \param ppPrivateData    the address of a PVOID pointer which can be used to anchor private data. The
                           PVPOID pointer will be set to NULL on the initial call

  	\returns 0 if OK,
             OtmMemoryPlugin::eRepeat when the import needs more processing steps
             any other value is an error code
*/
int importFromMemFilesInitialize
(
  char *pszMemoryName,
  char *pFileList,
  int  iOptions,
  PVOID *ppPrivateData
);

  
/*! \brief Continue the import of a memory using a list of memory data files


   \param ppPrivateData    the address of a PVOID pointer which can be used to anchor private data. The
                           PVPOID pointer will be set to NULL on the initial call

  	\returns 0 if OK,
             OtmMemoryPlugin::eRepeat when the import needs more processing steps
             any other value is an error code
 */
  int importFromMemFilesContinueProcessing
  (
    PVOID *ppPrivateData
  );

  /*! \brief Terminate the import of a memory using a list of memory data files and do a cleanup


   \param ppPrivateData    the address of a PVOID pointer which can be used to anchor private data. The
                           PVPOID pointer will be set to NULL on the initial call

  	\returns 0 if OK,
             any other value is an error code
 */
  int importFromMemFilesEndProcessing
  (
    PVOID *ppPrivateData
  );

};


#endif // #ifndef _EqfMemoryPlugin_H_