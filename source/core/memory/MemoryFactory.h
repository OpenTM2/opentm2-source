/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#ifndef _MemoryFactory_H_
#define _MemoryFactory_H_

#include <string>
#include <vector>
#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmSharedMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "core\utilities\LogWriter.h"

class __declspec(dllexport) MemoryFactory
/*! \brief   This class provides factory methods for OtmMemory objects 

*/

{
public:


/*! \brief Error code definition
*/
  static const int ERROR_NOSHAREDMEMORYPLUGIN = 1001;
  static const int ERROR_PLUGINNOTAVAILABLE   = 1002;
  static const int ERROR_MEMORYOBJECTISNULL   = 1003;
  static const int ERROR_BUFFERTOOSMALL       = 1004;
  static const int ERROR_INVALIDOBJNAME       = 1005;
  static const int ERROR_MISSINGPARAMETER     = 1006;

/*! \brief This static method returns a pointer to the MemoryFactory object.
	The first call of the method creates the MemoryFactory instance.
*/
	static MemoryFactory* getInstance();

/* \brief Open a memory 
   \param pszPlugin plugin-name or NULL if not available or memory object name is used
   \param pszMemoryName name of the memory being deleted or
    memory object name (pluginname + colon + memory name)
   \param piErrorCode pointer to a int varaibel receiving any error code when function fails
   \returns pointer to opened memory object 
*/
OtmMemory *openMemory
(
  char *pszPluginName,
  char *pszMemoryName,
  unsigned short usOpenFlags,
  int *piErrorCode
);

/* \brief Get information from an existing memory
   \param pszPlugin plugin-name or NULL if not available or memory object name is used
   \param pszMemoryName name of the memory or memory object name (pluginname + colon + memory name)
   \param pInfo pointer to caller MemoryInfo structure
   \returns 0 when successful or error code
*/
int getMemoryInfo
(
  char *pszPluginName,
  char *pszMemoryName,
  OtmMemoryPlugin::PMEMORYINFO pInfo
);


/* \brief Get the names of the actual memory data files

   These files are passed to the getMemoryPart method of the memory to retrieve the memory 
   data files in binary format (e.g. for folder export)

   \param pszPlugin plugin-name or NULL if not available or memory object name is used
   \param pszMemoryName name of the memory or memory object name (pluginname + colon + memory name)
   \param pFileListBuffer  pointer to a buffer receiving the file names as a comma separated list
   \param iBufferSize      size of buffer in number of bytes
   \param pszPluginNameOut buffer for the plugin of the memory or NULL, if not used
   \returns 0 when successful or error code
*/
int getMemoryFiles
(
  char *pszPluginName,
  char *pszMemoryName,
  char *pFileListBuffer,
  int  iBufferSize,
  char *pszPluginNameOut
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

   \param pszPlugin plugin-name or NULL if not available or memory object name is used
   \param pszMemoryName name of the memory or memory object name (pluginname + colon + memory name)
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
  char *pszPluginName,
  char *pszMemoryName,
  char *pFileListBuffer,
  int  iOptions,
  PVOID *ppPrivateData
);


/* \brief Create a memory 
   \param pszPlugin plugin-name or NULL if not available or memory object name is used
   \param pszMemoryName name of the memory being created or
    memory object name (pluginname + colon + memory name)
   \param pszDescription description of the memory
   \param pszSourceLanguage source language of the memory
   \param piErrorCode pointer to a int varaibel receiving any error code when function fails
   \returns pointer to created memory object 
*/
OtmMemory *createMemory
(
  char *pszPluginName,
  char *pszMemoryName,
  char *pszDescription,
  char *pszSourceLanguage,
  int *piErrorCode
);

/* \brief Create a memory 
   \param pszPlugin plugin-name or NULL if not available or memory object name is used
   \param pszMemoryName name of the memory being created or
    memory object name (pluginname + colon + memory name)
   \param pszDescription description of the memory
   \param pszSourceLanguage source language of the memory
   \param chDrive drive where new memory should be created, or 0 if memory should be created on primary drive
   \param piErrorCode pointer to a int varaibel receiving any error code when function fails
   \returns pointer to created memory object 
*/
OtmMemory *createMemory
(
  char *pszPluginName,
  char *pszMemoryName,
  char *pszDescription,
  char *pszSourceLanguage,
  char chDrive,
  int *piErrorCode
);

/* \brief Create a memory 
   \param pszPlugin plugin-name or NULL if not available or memory object name is used
   \param pszMemoryName name of the memory being created or
    memory object name (pluginname + colon + memory name)
   \param pszDescription description of the memory
   \param pszSourceLanguage source language of the memory
   \param chDrive drive where new memory should be created, or 0 if memory should be created on primary drive
   \param pszOwner owner of the newly created memory
   \param bInvisible don't display memory in memory loist window when true, 
   \param piErrorCode pointer to a int varaibel receiving any error code when function fails
   \returns pointer to created memory object 
*/
OtmMemory *createMemory
(
  char *pszPluginName,
  char *pszMemoryName,
  char *pszDescription,
  char *pszSourceLanguage,
  char chDrive,
  char *pszOwner,
  boolean bInvisible,
  int *piErrorCode
);


/* \brief List all available memories from all installed memory plugins
   \param pfnCallBack callback function to be called for each memory found
	 \param pvData caller's data pointetr, is passed to callback function
	 \param fWithDetails TRUE = supply memory details, when this flag is set, 
   the pInfo parameter of the callback function is set otherwise it is NULL
 	 \returns number of provided memories
*/
int listMemories
(
	OtmMemoryPlugin::PFN_LISTMEMORY_CALLBACK pfnCallBack,			  
	void *pvData,
	BOOL fWithDetails
);

/* \brief Get a list of the active memory plugins
   \param vPluginList reference to caller's vector receiving the list of memory plugins
   \param vPluginList reference to caller's vector receiving the list of shared memory plugins
 	 \returns number of provided plugins
*/
int getMemoryPlugins
(
	std::vector<OtmMemoryPlugin *>&vMemPluginList,
	std::vector<OtmSharedMemoryPlugin *>&vSharedMemPluginList
);

/*! \brief Provide the names of shared memories available for a given user
	\param pszPlugin  name of the shared memory plugin to be used
	\param pvOptions pointer to a vector containing the access options
  \param pvConnected pointer to a vector receiving the names of the connected memories
  \param pvNotConnected pointer to a vector receiving the names of the not connected memories
	\returns number of provided memories
*/
	int listSharedMemories(
    char *pszPlugin,
    std::vector<std::string> *pvOptions,
    std::vector<std::string> *pvConnected,
    std::vector<std::string> *pvNotConnected
	);

/* \brief Close a memory 
   Close the memory object and free all memory related resources.
   The memory object is not valid anymore.
   \param pMemory pointer to the memory object being closes
   \returns 0 when successful or a error return code
*/
int closeMemory
(
  OtmMemory *pMemory
);

/*! \brief Rename a translation memory
  \param pszPluginName name of the memory being deleted
  \param pszOldMemoryName name of the memory being renamed or
  memory object name (pluginname + colon + memoryname)
  \param pszNewMemoryName new name for the memory 
	\returns 0 if successful or error return code
*/
int renameMemory(
  char *pszPluginName,
  char *pszOldMemoryName,
  char *pszNewMemoryName
);

/*! \brief Physically delete a translation memory
   \param pszPlugin plugin-name or NULL if not available or memory object name is used
  \param pszMemoryName name of the memory being deleted or
  memory object name (pluginname + colon + memoryname)
	\returns 0 if successful or error return code
*/
int deleteMemory(
  char *pszPluginName,
  char *pszMemoryName
);


/*! \brief Physically delete a translation memory
   \param pszPlugin plugin-name or NULL if not available or memory object name is used
  \param pszMemoryName name of the memory being deleted or
  memory object name (pluginname + colon + memoryname)
  \param strError  return error message with it
	\returns 0 if successful or error return code
*/
int deleteMemory(
  char *pszPluginName,
  char *pszMemoryName,
  std::string &strError
);
/*! \brief Delete all entries contained in a translation memory
  \param pszPlugin plugin-name or NULL if not available or memory object name is used
  \param pszMemoryName name of the memory being cleared or
  memory object name (pluginname + colon + memoryname)
	\returns 0 if successful or error return code
*/
int clearMemory(
  char *pszPluginName,
  char *pszMemoryName
);

/*! \brief Check if memory exists
  \param pszPlugin plugin-name or NULL if not available or memory object name is used
  \param pszMemoryName name of the memory being cleared or
  memory object name (pluginname + colon + memoryname)
	\returns 0 if successful or error return code
*/
int exists(
  char *pszPluginName,
  char *pszMemoryName
);

/*! \brief Check if memory is a shared/synchronized memory
  \param pMemory pointer to memory object
	\returns TRUE is memory is shared/synchronized
*/
BOOL isSharedMemory(
  OtmMemory *pMemory
);

/*! \brief Check if memory is a shared/synchronized memory
  \param pszMemory Name of the memory
  \param pPlugin adress of a variable receiving the pointer to the plugin of the memory
	\returns TRUE is memory is shared/synchronized
*/
BOOL isSharedMemory(
  char *pszMemory,
  OtmSharedMemoryPlugin **ppPlugin = NULL
);

/*! \brief Check if memory is a shared/synchronized memory
  \param strMemory Name of the memory
  \param pPlugin adress of a variable receiving the pointer to the plugin of the memory
	\returns TRUE is memory is shared/synchronized
*/
BOOL isSharedMemory(
  std::string &strMemory,
  OtmSharedMemoryPlugin **ppPlugin = NULL
);

/*! \brief Get name of default memory plugin
	\returns pointer to name of default memory plugin
*/
const char *getDefaultMemoryPlugin();

/*! \brief Get name of default shared memory plugin
	\returns pointer to name of default shared memory plugin
*/
const char *getDefaultSharedMemoryPlugin();


/*! \brief Show error message for the last error
  \param pszPlugin plugin-name or NULL if not available or memory object name is used
  \param pszMemoryName name of the memory causing the problem
  memory object name (pluginname + colon + memoryname)
  \param pMemory pointer to existing memory object or NULL if not available
  \param hwndErrMsg handle of parent window message box
*/
void showLastError(
  char *pszPluginName,
  char *pszMemoryName,
  OtmMemory *pMemory,
  HWND hwndErrMsg
);

/*! \brief   get error message for the last error
  \param   pMemory pointer to existing memory object or NULL if not available
  \param   iLastError the last error number
  \param   strError the error string returned with
  \returns the last error string
*/
std::string& MemoryFactory::getLastError(
    OtmMemory *pMemory,
    int& iLastError,
    std::string& strError);

/*! \brief Copy best matches from one proposal vector into another
  and sort the proposals
  \param SourceProposals refernce to a vector containing the source proposals
  \param TargetProposals reference to a vector receiving the copied proposals
  the vector may already contain proposals. The proposals are
  inserted on their relevance
  \param iMaxProposals maximum number of proposals to be filled in TargetProposals
  When there are more proposals available proposals with lesser relevance will be replaced
*/
void copyBestMatches(
  std::vector<OtmProposal *> &SourceProposals,
  std::vector<OtmProposal *> &TargetProposals,
  int iMaxProposals
);

/*! \brief Copy best matches from one proposal vector into another
  and sort the proposals
  \param SourceProposals refernce to a vector containing the source proposals
  \param TargetProposals reference to a vector receiving the copied proposals
  the vector may already contain proposals. The proposals are
  inserted on their relevance
  \param iMaxProposals maximum number of proposals to be filled in TargetProposals
  When there are more proposals available proposals with lesser relevance will be replaced
  \param iMTDisplayFactor factor for the placement of machine matches within the table
  \param fExactAndFuzzies switch to control the handling of fuzzy matches when exact matches exist, TRUE = keep fuzzy matches even when exact matches exist
*/
void copyBestMatches(
  std::vector<OtmProposal *> &SourceProposals,
  std::vector<OtmProposal *> &TargetProposals,
  int iMaxProposals, 
  int iMTDisplayFactor,
  BOOL fExactAndFuzzies
);

/*! \brief Insert proposal into proposal vector at the correct position and
  remove a proposal with lesser relevance when iMaxPropoals have already been filled
  \param NewProposal pointer to proposal being inserted
  \param SourceProposals refernce to a vector containing the source proposals
  \param TargetProposals reference to a vector receiving the copied proposals
  the vector may already contain proposals. The proposals are
  inserted on their relevance
  \param iMaxProposals maximum number of proposals to be filled in TargetProposals
  When there are more proposals available proposals with lesser relevance will be replaced
  \param fLastEntry true = this is the last entry in the table
  \param iMTDisplayFactor factor for the placement of machine matches within the table
*/
void insertProposalData(
  OtmProposal *newProposal,
  std::vector<OtmProposal *> &Proposals,
  int iMaxProposals,
  BOOL fLastEntry, 
  int iMTDisplayFactor = -1
);

/*! \brief Check if first proposal in the list can be used for automatic substitution 
  \param Proposals reference to a vector containing the proposals
  \returns true when automatic substitution can be performed otherwise false
*/
bool isAutoSubstCandidate(
  std::vector<OtmProposal *> &Proposals
);

/*! \brief Create the shared part of a memory
  \param hwndOwner owner handle for dialog windows
  \param pszPluginName name of the memory plugin
  \param pszMemoryName name of the memory
  \param pLocalMemory pointer to local version of the created memory
  \returns pointer to shared memory object or NULL in case of errors
*/
OtmMemory *createSharedMemory(
  HWND hwndOwner,
  char *pszPluginName,
  char *pszMemoryName,
  OtmMemory *pLocalMemory
);


/*! \brief Connect a shared memory
	\param pszPlugin  name of the shared memory plugin to be used
  \param hwndOwner owner handle for dialog windows
  \param pszMemoryName name of the memory
  \param pvOptions ptr to a vector containing the access options
  \returns 0 when successful or error return code
*/
int connectToMemory(
  char *pszPlugin,
  HWND hwndOwner,
  char *pszMemoryName,
  std::vector<std::string> *pvOptions
);

/*! \brief Disconnect a connected memory
	\param pszPlugin  name of the shared memory plugin to be used
  \param hwndOwner owner handle for dialog windows and error messages
  \param pszMemoryName name of the memory
  \returns 0 when successful or error return code
*/
int disconnectMemory(
  char *pszPlugin,
  HWND hwndOwner,
  char *pszMemoryName
);

/*! \brief get the options required to connect to a sharedmemory
	\param pszPlugin  name of the shared memory plugin to be used
  \param hwndOwner owner handle for dialog windows
  \param pvOptions pointer to a vector receiving the connect options
  \returns 0 when successful, -1 when user has cancelled the options dialog or an error code
*/
  int getConnectOptions(
    char *pszPlugin,
    HWND hwndOwner,
    std::vector<std::string> *pvOptions
  );

/*! \brief Get the object name for the memory
  \param pMemory pointer to the memory object
  \param pszObjName pointer to a buffer for the object name
  \param iBufSize size of the object name buffer
  \returns 0 when successful or the error code
*/
int getObjectName( OtmMemory *pMemory, char *pszObjName, int iBufSize );

/*! \brief Get the plugin name and the memory name from a memory object name
  \param pszObjName pointer to the memory object name
  \param pszPluginName pointer to the buffer for the plugin name or 
    NULL if no plugin name is requested
  \param iPluginBufSize size of the buffer for the plugin name
  \param pszluginName pointer to the buffer for the plugin name or
    NULL if no memory name is requested
  \param iNameBufSize size of the buffer for the memory name
  \returns 0 when successful or the error code
*/
int splitObjName( char *pszObjName, char *pszPluginName, int iPluginBufSize, char *pszMemoryName, int iNameBufSize  );

 /*! \brief Get the sort order key for a memory match
  \param Proposal reference to a proposal for which the sort key is evaluated
  \param iMTDisplayFactor the machine translation display factor, -1 to ignore the factor
  \param usContextRanking the context ranking for the proposal
  \param fEndIfTable TRUE when this proposal is the last in a proposal table
  When there are more proposals available proposals with lesser relevance will be replaced
  \returns the proposal sort key
*/
 int getProposalSortKey(  OtmProposal &Proposal, int iMTDisplayFactor, USHORT usContextRanking, BOOL fEndOfTable );

 /*! \brief Get the sort order key for a memory match
  \param Proposal reference to a proposal for which the sort key is evaluated
  \returns the proposal sort key
*/
int getProposalSortKey(  OtmProposal &Proposal );

 /*! \brief Get the sort order key for a memory match
  \param MatchType match type of the match
  \param ProposalType type of the proposal
  \param iFuzziness fuzziness of the proposal
  \param iMTDisplayFactor the machine translation display factor, -1 to use the system MT display factor
  \param usContextRanking the context ranking for the proposal
  \param fEndIfTable TRUE when this proposal is the last in a proposal table
  When there are more proposals available proposals with lesser relevance will be replaced
  \returns the proposal sort key
*/
int getProposalSortKey(  OtmProposal::eMatchType MatchType, OtmProposal::eProposalType ProposalType, int iFuzzyness, int iMTDisplayFactor, USHORT usContextRanking, BOOL fEndOfTable );

/* \brief add a new user to a shared memory user list
   \param pszPlugin  name of the shared memory plugin to be used
   \param pszMemName   memory name
   \param pszUserName  user name to add
   \param strError     return error message with it
   \returns 0
*/
int addSharedMemoryUser( char *pszPlugin, char *pszMemName, char *pszUserName, std::string &strError);

/* \brief delete a user from a shared memory user list
   \param pszPlugin  name of the shared memory plugin to be used
   \param pszMemName   memory name
   \param pszUserName  user name to add
   \param strError     return error message with it
   \returns 0
*/
int removeSharedMemoryUser( char *pszPlugin, char *pszMemName, char *pszUserName, std::string &strError);

/* \brief list shared memory users
   \param pszPlugin  name of the shared memory plugin to be used
   \param pszMemName   memory name
   \param users        users name returned
   \param strError     return error message with it
   \returns 0
*/
int listSharedMemoryUsers( char *pszPlugin, char  *pszMemName, std::vector<std::string> &users, std::string &strError);

/*! \brief Create a temporary memory
  \param pszPrefix prefix to be used for name of the temporary memory
  \param pszName buffer for the name of the temporary memory
	\param pszSourceLang source language
	\returns Pointer to created translation memory or NULL in case of errors
*/
 OtmMemory* createTempMemory(
	  PSZ pszPrefix,			  
	  PSZ pszName,			  
	  PSZ pszSourceLang
  );

  /*! \brief Close and delete a temporary memory
  \param pMemory pointer to memory obhect
*/
 void closeTempMemory(
	  OtmMemory *pMemory
);

/* \brief add a new memory information to memory list
   \param pszName memory name, format as "pluginName:memoryName"
   \param chToDrive drive letter
   \returns 0 if success
*/
int addMemoryToList(PSZ pszName, CHAR chToDrive);

/* \brief remove a memory information from memory list
   \param  pszName format as "pluginName:memoryName"
   \returns 0 if success
*/
int removeMemoryFromList(PSZ pszName);

  /*! \brief Replace a memory with the data from another memory
    This method bevaves like deleting the replace memory and renaming the
    replaceWith memory to the name of the replace memory without the overhead of the
    actual delete and rename operations
    \param pszPluginName name of plugin of the memory
    \param pszReplace name of the memory being replaced
    \param pszReplaceWith name of the memory replacing the pszReplace memory
	  returns 0 if successful or error return code
  */
  int replaceMemory
  (
    char *pszPluginName,
    char *pszReplace,
    char *pszReplaceWith
  );


  /*! \brief process the API call: EqfImportMemInInternalFormat and import a memory using the internal memory files
    \param pszMemory name of the memory being imported
    \param pszMemoryPackage name of a ZIP archive containing the memory files
    \param lOptions options for searching fuzzy segments
           - OVERWRITE_OPT overwrite any existing memory with the given name
    \returns 0 if successful or an error code in case of failures
  */
  USHORT APIImportMemInInternalFormat
  (
    PSZ         pszMemoryName,
    PSZ         pszMemoryPackage,
    LONG        lOptions 
  );

  /*! \brief process the API call: EqfOpenMem and open the specified translation memory
    \param pszMemory name of the memory being opened
    \param plHandle buffer to a long value receiving the handle of the opened memory or -1 in case of failures
    \param lOptions processing options 
    \returns 0 if successful or an error code in case of failures
  */
  USHORT APIOpenMem
  (
    PSZ         pszMemoryName, 
    LONG        *plHandle,
    LONG        lOptions 
  );

  /*! \brief process the API call: EqfCloseMem and close the translation memory referred by the handle
    \param lHandle handle of a previously opened memory
    \param lOptions processing options 
    \returns 0 if successful or an error code in case of failures
  */
  USHORT APICloseMem
  (
    LONG        lHandle,
    LONG        lOptions 
  );

  /*! \brief process the API call: EqfQueryMem and lookup a segment in the memory
    \param lHandle handle of a previously opened memory
    \param pvSearchKey pointer to an OtmProposal object containing the searched segment
    \param pvProposals pointer to a vector of OtmProposal objects receiving the search results
    \param lOptions processing options 
    \returns 0 if successful or an error code in case of failures
  */
  USHORT APIQueryMem
  (
    LONG        lHandle,          
    void       *pvSearchKey, 
    void       *pvProposals, 
    LONG        lOptions     
  );


  /*! \brief process the API call: EqfSearchMem and search the given text string in the memory
    \param lHandle handle of a previously opened memory
    \param pszSearchString pointer to the search string (in UTF-16 encoding)
    \param pszStartPosition pointer to a buffer (min size = 20 charachters) containing the start position, on completion this buffer is filled with the next search position
    \param pvProposal pointer to an OtmProposal object receiving the next matching segment
    \param lOptions processing options 
    \returns 0 if successful or an error code in case of failures
  */
  USHORT APISearchMem
  (
    LONG        lHandle,                 
    __wchar_t  *pszSearchString,
    PSZ         pszStartPosition,
    void       *pvProposal,
    LONG        lOptions
  );

  /*! \brief process the API call: EqfUpdateMem and update a segment in the memory
    \param lHandle handle of a previously opened memory
    \param pvNewProposal pointer to an OtmProposal object containing the segment data
    \param lOptions processing options 
    \returns 0 if successful or an error code in case of failures
  */
  USHORT APIUpdateMem
  (
    LONG        lHandle, 
    void        *pvNewProposal,
    LONG        lOptions
  );

private:
  
  /* \brief Get memory plugin with the given plugin name
   \param pszPlugin plugin-name
   \returns pointer to plugin or NULL if no memory pluging with the given name is specified
*/
OtmPlugin *MemoryFactory::getPlugin
(
  const char *pszPluginName
);

  /* \brief Find memory plugin for this memory using input
    data or find memory testing all memory plugins
   \param pszPlugin plugin-name or NULL if not available
   \param pszMemoryName memory name or memory object name (pluginname + colon + memoryname)
   \returns pointer to plugin or NULL if no memory pluging with the given name is specified
*/
OtmPlugin *findPlugin
(
  char *pszPluginName,
  char *pszMemoryName
);

/* \brief Get the memory name from a potential memory object name
   \param pszMemoryName memory name or memory object name (pluginname + colon + memoryname)
   \param strMemoryName reference to string receiving memory name without any plugin name
   \returns 0
*/
int getMemoryName
(
  char *pszMemoryName,
  std::string &strMemoryName
);

/* \brief Get a pointer to the shared memory plugin with the give name
   \param pszPlugin plugin name
   \returns pointer to the plugin
*/
OtmSharedMemoryPlugin *getSharedMemoryPlugin( char *pszPlugin ); 


/* \brief Refresh internal list of memory plugins 
*/
void MemoryFactory::refreshPluginList();

/*! \brief get the index into the memory object table from a memory handle
  \param lHandle handle of a previously opened memory
  \returns index into the memory object table
*/
LONG getIndexFromHandle( LONG );

/*! \brief get the checksum from a memory handle
  \param lHandle handle of a previously opened memory
  \returns checksum of the memory object
*/
LONG getCheckSumFromHandle
(
  LONG        lHandle
);

/*! \brief compute the checksum for a memory object
  \param pMemory pointer to a memory object
  \returns memory object checksum
*/
LONG computeMemoryObjectChecksum( OtmMemory *pMemory );

/*! \brief convert a memory object and the index into the memory oject table to a memory handle
  \param lIndex index into the memory object table
  \param pMemory pointer to a memory object
  \returns index into the memory object table
*/
LONG createHandle
(
  LONG        lIndex,
  OtmMemory   *pMemory
);

/*! \brief compute the checksum for a memory object
  \param lHandle handel referring to the memory object
  \returns memory object pointer or NULL if the given handle is invalid
*/
OtmMemory *handleToMemoryObject
(
  LONG lHandle
);


/*! \brief search option flags
*/
#define SEARCH_SOURCE  1
#define SEARCH_TARGET  2
#define SEARCH_CASEINSENSITIVE 4
#define SEARCH_WHITESPACETOLERANT 8

/*! \brief search a string in a proposal
  \param pProposal pointer to the proposal 
  \param pszSearch pointer to the search string (when fIngoreCase is being used, this strign has to be in uppercase)
  \param lSearchOptions combination of search options
  \returns TRUE if the proposal contains the searched string otherwise FALSE is returned
*/
BOOL searchInProposal
( 
  OtmProposal *pProposal,
  PSZ_W pszSearch,
  LONG lSearchOptions
);

/*! \brief find the given string in the provided data
  \param pszData pointer to the data being searched
  \param pszSearch pointer to the search string
  \returns TRUE if the data contains the searched string otherwise FALSE is returned
*/
BOOL findString
( 
  PSZ_W pszData,
  PSZ_W pszSearch
);

/*! \brief check if search string matches current data
  \param pData pointer to current position in data area
  \param pSearch pointer to search string
  \returns 0 if search string matches data
*/
SHORT compareString
(
  PSZ_W   pData,
  PSZ_W   pSearch
);

/*! \brief normalize white space within a string by replacing single or multiple white space occurences to a single blank
  \param pszString pointer to the string being normalized
  \returns 0 in any case
*/
SHORT normalizeWhiteSpace
(
  PSZ_W   pszData
);

/*! \brief Pointer to the instance of the MemoryFactory object (singleton).
*/
	static MemoryFactory* instance;

/*! \brief Pointer to the list of installed memory plugins
*/
 std::vector<OtmMemoryPlugin *> *pluginList;

/*! \brief Pointer to the list of installed plugins for shared memories
*/
  std::vector<OtmSharedMemoryPlugin *> *pSharedMemPluginList;

 /*! \brief error code of last error occured
*/
 int iLastError;

 /*! \brief error text for last error occured
*/
 std::string strLastError;

/*! \brief LogWriter object for logging
*/
 LogWriter Log;

/*! \brief buffer for memory object names
*/
 char szMemObjName[512];

/*! \brief buffer for name of default memory plugin
*/
 char szDefaultMemoryPlugin[512];

/*! \brief buffer for name of default shared memory plugin
*/
 char szDefaultSharedMemoryPlugin[512];

/*! \brief array containing the memory objects referred to by a handle
*/
std::vector<OtmMemory *> *pHandleToMemoryList;

/*! \brief Buffer for segment text
*/
  CHAR_W m_szSegmentText[MAX_SEGMENT_SIZE+1];


};


#endif