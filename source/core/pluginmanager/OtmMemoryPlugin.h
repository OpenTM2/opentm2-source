/*! \file
	Copyright Notice:

	Copyright (C) 1990-2014, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _OTMMEMORYPLUGIN_H_
#define _OTMMEMORYPLUGIN_H_

#include "OtmPlugin.h"
#include "OtmMemory.h"

/*! \brief Abstract base-class for plugins handling translation memory
*/
class __declspec(dllexport) OtmMemoryPlugin: public OtmPlugin
{

public:

/*! \brief Constructor.
*/
	OtmMemoryPlugin()
		{pluginType = eTranslationMemoryType;};

/*! \brief Destructor.
*/
	virtual ~OtmMemoryPlugin()
		{};

	virtual bool isUsable()
		{return OtmPlugin::isUsable();};

	virtual const char* getName() = 0;

	virtual const char* getShortDescription() = 0;

	virtual const char* getLongDescription() = 0;

	virtual const char* getVersion() = 0;

	virtual const char* getSupplier() = 0;

	virtual const char* getDescriptiveMemType() = 0;
  
/*! \enum eRegRc
	Possible return values of OtmMemory and OtmMemoryPlugin methods
*/
	enum eRc
	{
		eSuccess = 0,			        /*!< method completed successfully */
		eUnknownPlugin,		        /*!< the specified memory plugin is not available */
//		eInvalidName,		          /*!< plugin-name is invalid */
//		eAlreadyRegistered,	      /*!< plugin with same name was already registered before */
//		eInvalidRequest,	        /*!< method may only be called from within registerPlugins call */
    eMemoryNotFound,          /*!< the specified memory does not exist or is not controlled by this memory plugin*/
		eUnknown,		            	/*!< plugin with that name was not registered before */
		eNotSupported,		        /*!< method is not supported by this plugin */
		eBufferTooSmall,          /*!< the provided buffer is too small */
    eNotEnoughMemory,         /*!< not enough system memory to process the request */
    eRepeat                   /*!< repeat calling this method until processing has been completed*/
	};


 
/*! \brief Create a new translation memory
  \param pszName name of the new memory
	\param pszSourceLang source language
	\param pszDescription description of the memory
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
  \param chDrive drive where new memory should be created, or 0 if memory should be created on primary drive
	\returns Pointer to created translation memory or NULL in case of errors
*/
	virtual OtmMemory* createMemory(
		PSZ pszName,			  
		PSZ pszSourceLang,
		PSZ pszDescription,
		BOOL bMsgHandling,
		HWND hwnd,
    CHAR chDrive
	) = 0;
	
/*! \brief Open an existing translation memory
  \param pszName name of the existing memory
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
  \param usAccessMode, special access mode for memory: FOR_ORGANIZE, EXCLUSIVE, READONLY
	\returns Pointer to translation memory or NULL in case of errors
*/
	virtual OtmMemory* openMemory(
		PSZ pszName,			  
		BOOL bMsgHandling,
		HWND hwnd,
    unsigned short usAccessMode = 0
	) = 0;

/*! \brief Physically delete a translation memory
  \param pszName name of the memory being deleted
	\returns 0 if successful or error return code
*/
	virtual int deleteMemory(
		PSZ pszName			  
	) = 0;

/*! \brief Clear (i.e. remove all entries) a translation memory
  \param pszName name of the memory being cleared
	\returns 0 if successful or error return code
*/
	virtual int clearMemory(
		PSZ pszName			  
	) = 0;

  /*! \brief Closes a translation memory
  \param pMemory pointer to a previously opened memory object
	\returns 0 if successful or error return code
*/
	virtual int closeMemory(
		OtmMemory *pMemory			  
	) = 0;


  /*! \brief structure for memory information */
  typedef struct _MEMORYINFO
  {
    char szPlugin[256];                          // name of the plugin controlling this memory
    char szName[256];                            // name of the memory
    char szDescription[256];                     // description of the memory
    char szFullPath[256];                        // full path to memory file(s) (if applicable only)
    char szSourceLanguage[MAX_LANG_LENGTH+1];    // memory source language
    char szOwner[256];                           // ID of the memory owner
    char szDescrMemoryType[256];                 // descriptive name of the memory type
    unsigned long ulSize;                        // size of the memory  
    BOOL fEnabled;                               // memory-is-enabled flag  
  } MEMORYINFO, *PMEMORYINFO;

/*! \brief type of callback function for ListMemories method */
typedef int (*PFN_LISTMEMORY_CALLBACK )(PVOID pvData, char *pszName, PMEMORYINFO pInfo  );

  /*! \brief Provide a list of all available memories
  \param pfnCallBack callback function to be called for each memory
	\param pvData caller's data pointetr, is passed to callback function
	\param fWithDetails TRUE = supply memory details, when this flag is set, 
  the pInfo parameter of the callback function is set otherwise it is NULL
	\returns number of provided memories
*/
	virtual int listMemories(
		PFN_LISTMEMORY_CALLBACK pfnCallBack,			  
		void *pvData,
		BOOL fWithDetails
	) = 0;

/*! \brief Get information about a memory
  \param pszName name of the memory
  \param pInfo pointer to buffer for memory information
	\returns 0 if successful or error return code
*/
	virtual int getMemoryInfo(
		PSZ pszName,
    PMEMORYINFO pInfo
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

// options for the importFromMemoryFiles method
static const int IMPORTFROMMEMFILES_COMPLETEINONECALL_OPT = 1;  // complete the import in one call, do not divide the processing into smaller steps

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
	  PSZ pszOldNae,
    PSZ pszNewName
  ) = 0;

  /*! \brief Create a temporary memory
  \param pszPrefix prefix to be used for name of the temporary memory
  \param pszName buffer for the name of the temporary memory
	\param pszSourceLang source language
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
	\returns Pointer to created translation memory or NULL in case of errors
*/
  virtual OtmMemory* createTempMemory(
	  PSZ pszPrefix,			  
	  PSZ pszName,			  
	  PSZ pszSourceLang,
	  BOOL bMsgHandling,
	  HWND hwnd
  ) = 0;

  /*! \brief Closes and deletes a temporary memory
  \param pMemory pointer to memory object
*/
  virtual void closeTempMemory(
	  OtmMemory *pMemory
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


/*! \brief Stops the plugin. 
	Terminating-function for the plugin, will be called directly before
	the DLL containing the plugin will be unloaded.\n
	The method should call PluginManager::deregisterPlugin() to tell the PluginManager
  that the plugin is not active anymore.
  Warning: THIS METHOD SHOULD BE CALLED BY THE PLUGINMANAGER ONLY!
	\param fForce, TRUE = force stop of the plugin even if functions are active, FALSE = only stop plugin when it is inactive
	\returns TRUE when successful */
	virtual bool stopPlugin( bool fForce = false ) = 0;

/*! \brief Returns list of supported drive letters. 
	The list of drives is used to allow the selection of a drive letter in the memory create indow
	The drive letters are retunred as a null-terminated string containing the letters of the 
  supported drives. When no drive selection should be possible, an empty string should be returned.\n
	\param pszDriveListBuffer points to the callers buffer for the list of drive letter characters
	\returns 0 if successful or error return code */
	virtual int getListOfSupportedDrives( char *pszDriveListBuffer ) = 0;

    /* \brief add a new memory information to memory list
       \param pszName memory name
       \param chToDrive drive letter
       \returns 0 if success
   */
    virtual int addMemoryToList( PSZ pszName, CHAR chDrive ) = 0;

    /* \brief remove a memory information from memory list
       \param  pszName memory name
       \returns 0 if success
    */
    virtual int removeMemoryFromList( PSZ pszName ) = 0;

 /* \brief Replace the data of one memory with the data of another memory and delete the remains of the second memory
    \param pszReplace name of the memory whose data is being replaced
    \param pszReplaceWith name of the memory whose data will be used to replace the data of the other memory
   \returns 0 if success
*/
 virtual int replaceMemory( PSZ pszReplace, PSZ pszReplaceWith ) = 0;


};

#endif // #ifndef _OTMMEMORYPLUGIN_H_
 