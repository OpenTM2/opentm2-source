/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OTMSHAREDMEMORYPLUGIN_H_
#define _OTMSHAREDMEMORYPLUGIN_H_

#include "OtmPlugin.h"
#include "OtmMemoryPlugin.h"
#include "OtmMemory.h"
#include "OptionsDialog.h"


/*! \brief Abstract base-class for shared memory plugins 
*/
class __declspec(dllexport) OtmSharedMemoryPlugin: public OtmPlugin
{

public:

/*! \brief Constructor.
*/
	OtmSharedMemoryPlugin()
		{pluginType = eSharedTranslationMemoryType;};

/*! \brief Destructor.
*/
	virtual ~OtmSharedMemoryPlugin()
		{};

/*! \brief Returns the descriptive type of the memories controlled by this plugin
*/
	virtual const char* getDescriptiveMemType() = 0;

/*! \brief Check if the memory with the given name is owned by this plugin
  \param pszName name of the memory
	\returns TRUE if memory is owned by this plugin and FALSE if not
*/
	virtual BOOL isMemoryOwnedByPlugin(
		PSZ pszName
	) = 0;

/*! \brief Tell if this shared memory plugin is using a local memory to keep a copy of the shared memory proposals
  \param pszName name of the memory
	\returns TRUE when a local memory is used by this plugin, FALSE is no local memory is used
*/
	virtual BOOL isLocalMemoryUsed() = 0;

/*! \brief Returns list of supported drive letters. 
	The list of drives is used to allow the selection of a drive letter in the memory create indow
	The drive letters are retunred as a null-terminated string containing the letters of the 
  supported drives. When no drive selection should be possible, an empty string should be returned.\n
	\param pszDriveListBuffer points to the callers buffer for the list of drive letter characters
	\returns 0 if successful or error return code */
  virtual int getListOfSupportedDrives( char *pszDriveListBuffer ) = 0;

  /*! \brief Provide a list of all available memories
  \param pfnCallBack callback function to be called for each memory
	\param pvData caller's data pointetr, is passed to callback function
	\param fWithDetails TRUE = supply memory details, when this flag is set, 
  the pInfo parameter of the callback function is set otherwise it is NULL
	\returns number of provided memories
*/
	virtual int listMemories(
		OtmMemoryPlugin::PFN_LISTMEMORY_CALLBACK pfnCallBack,			  
		void *pvData,
		BOOL fWithDetails
	) = 0;
  
/*! \brief Create a new shared translation memory
  \param pszName name of the new memory
  \param pszSourceLanguage source language of the new memory
  \param pszDescription description for the new memory
  \param chDrive letter of the target drive for the memory (only used when drives can be selected)
	\param pvOptions pointer to a vector containing the create options
	\param pLocalMemory local version of shared memory being created
	\returns Pointer to created translation memory or NULL in case of errors
*/
	virtual OtmMemory* createMemory(
		PSZ pszName,			  
	  PSZ pszSourceLang,
	  PSZ pszDescription,
    CHAR chDrive,
    std::vector<std::string> *pvOptions,
    OtmMemory *pLocalMemory
	) = 0;
	
/*! \brief Open an existing shared translation memory
  \param pszName name of the new memory
	\param pLocalMemory local version of shared memory being created
  \param usAccessMode, special access mode for memory: FOR_ORGANIZE, EXCLUSIVE, READONLY
	\returns Pointer to translation memory or NULL in case of errors
*/
	virtual OtmMemory* openMemory(
		PSZ pszName,			  
    OtmMemory *pLocalMemory,
    USHORT usAccessMode
	) = 0;

/*! \brief Physically delete a translation memory
  \param pszName name of the memory being deleted
	\returns 0 if successful or error return code
*/
	virtual int deleteMemory(
		PSZ pszName			  
	) = 0;

  /*! \brief Closes a shared translation memory
  \param pMemory pointer to a previously opened memory object
	\returns 0 if successful or error return code
*/
	virtual int closeMemory(
		OtmMemory *pMemory			  
	) = 0;

  /*! \brief Provide the names of shared memories
	\param pvOptions pointer to a vector containing the access options
  \param pvConnected pointer to a vector receiving the names of the connected memories
  \param pvNotConnected pointer to a vector receiving the names of the not connected memories
	\returns number of provided memories
*/
	virtual int listSharedMemories(
    std::vector<std::string> *pvOptions,
    std::vector<std::string> *pvConnected,
    std::vector<std::string> *pvNotConnected
	) = 0;

/*! \brief Get information about a memory
  \param pszName name of the memory
  \param pInfo pointer to buffer for memory information
	\returns 0 if successful or error return code
*/
	virtual int getMemoryInfo(
		PSZ pszName,
    OtmMemoryPlugin::PMEMORYINFO pInfo
	) = 0;

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
  virtual int getMemoryFiles
  (
    PSZ pszName,
    char *pFileListBuffer,
    int  iBufferSize
  ) = 0;

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
virtual int importFromMemoryFiles
(
  char *pszMemoryName,
  char *pFileListBuffer,
  int  iOptions,
  PVOID *ppPrivateData
) = 0;

   /*! \brief Physically rename a translation memory
  \param pszOldName name of the memory being rename
  \param pszNewName new name for the memory
	\returns 0 if successful or error return code
*/
  virtual int renameMemory(
	  PSZ pszOldName,
    PSZ pszNewName
  ) = 0;

/*! \brief Provide list of options required for the creation of shared memories
  \param pszName pointer to the name of the memory being created
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
	virtual int getCreateOptionFields(
		PSZ pszName,
    std::vector<std::string> **ppLabels,
    std::vector<std::string> **ppFieldData,
    std::vector<std::string> **ppFieldDescr,
    PFN_OPTIONSDIALOGCHECKDATA *ppfnCheckCallback,
    long *plHandle
	) = 0;

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
	virtual int getAccessOptionFields(
    std::vector<std::string> **ppLabels,
    std::vector<std::string> **ppFieldData,
    std::vector<std::string> **ppFieldDescr,
    PFN_OPTIONSDIALOGCHECKDATA *ppfnCheckCallback,
    long *plHandle
	) = 0;

/*! \brief Connect to an existing shared translation memory
  \param pszName name of the memory being connected
     Option 0 = service URL
     Option 1 = user ID
     Option 2 = password
	\param pvOptions pointer to a vector containing the access options
	\returns 0 when successful or error return code
*/
	virtual int connectToMemory(
		PSZ pszName,			  
    std::vector<std::string> *pvOptions
	) = 0;

/*! \brief Disconnect a shared translation memory
  \param pszName name of the memory being disconnected
	\returns 0 when successful or error return code
*/
	virtual int disconnectMemory(
		PSZ pszName
	) = 0;

/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
	virtual int getLastError
  (
    std::string &strError
  ) = 0; 
/*! \brief Get the error message for the last error occured

    \param pszError pointer to a buffer for the error text
    \param iBufSize size of error text buffer in number of characters
  	\returns last error code
*/
	virtual int getLastError
  (
    char *pszError,
    int iBufSize
  ) = 0; 

  virtual int addMemoryUser(
      PSZ pszName,
      PSZ userName
    ) = 0;

  virtual int removeMemoryUser(
      PSZ pszName,
      PSZ userName
    ) = 0;

  virtual int listMemoryUsers(
      PSZ pszName,
      std::vector<std::string> &users
    ) = 0;

/* \brief add a new memory information to memory list
    \param pszName memory name
    \param chToDrive drive letter
    \returns 0 if success
*/
 virtual int addMemoryToList(PSZ pszName, CHAR chToDrive)=0;

/* \brief remove a memory information from memory list
   \param  pszName memory name
   \returns 0 if success
*/
 virtual int removeMemoryFromList(PSZ pszName)=0;

 /* \brief set the owner of the memory
    \param pszMemoryName memory name
    \param  pszOwner new owner for the memory
   \returns 0 if success
*/
 virtual int setOwner( PSZ pszMemoryName, PSZ pszOwner)=0;

 /* \brief Replace the data of one memory with the data of another memory and delete the remains of the second memory
    \param pszReplace name of the memory whose data is being replaced
    \param pszReplaceWith name of the memory whose data will be used to replace the data of the other memory
   \returns 0 if success
*/
 virtual int replaceMemory( PSZ pszReplace, PSZ pszReplaceWith ) = 0;

};

#endif // #ifndef _OTMSHAREDMEMORYPLUGIN_H_
 