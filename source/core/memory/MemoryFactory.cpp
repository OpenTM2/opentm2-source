/*! \file

 \brief Memory factory class wrapper 

 This class provides static methods for the creation of OtmMemory objects and
 access to standard memory functions

Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TAGTABLE         // tag table and format functions
  #define INCL_EQF_MORPH            // morphologic functions
  #define INCL_EQF_ANALYSIS         // analysis functions
  #define INCL_EQF_TM               // general Transl. Memory functions


//#define LOGGING

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "core\PluginManager\OtmSharedMemory.h"
#include "MemoryUtil.h"
#include "core\memory\MemoryFactory.h"
#include "OptionsDialog.h"

extern "C"
{
#include <OTMFUNC.H>               // header file of OpenTM2 API functions
}

#include "vector"


// default memory plugins
#define DEFAULTMEMORYPLUGIN "EqfMemoryPlugin"
#define DEFAULTSHAREDMEMORYPLUGIN "EqfSharedMemPlugin"

/** Initialize the static instance variable */
MemoryFactory* MemoryFactory::instance = 0;


/*! \brief This static method returns a pointer to the MemoryFactory object.
	The first call of the method creates the MemoryFactory instance.
*/
MemoryFactory* MemoryFactory::getInstance()
{
	if (instance == 0)
	{
		instance = new MemoryFactory();
    instance->pluginList = NULL;
    instance->pSharedMemPluginList = NULL;
    instance->refreshPluginList();
#ifdef LOGGING
    instance->Log.open( "MemoryFactory" );
#endif
  }
	return instance;
}


/* \brief Open an existing memory
   \param pszPluginName Name of the plugin or NULL
   \param pszMemoryName Name of the memory
   \param usOpenFlags Open mode flags
   \param pMemory pointer to the opened memory object
   \paran piErrorCode optional pointer to a buffer for error codes
	 \returns pointer to memory object or NULL in case of errors
*/

OtmMemory *MemoryFactory::openMemory
(
  char *pszPluginName,
  char *pszMemoryName,
  unsigned short usOpenFlags,
  int *piErrorCode
)
{
  OtmMemory *pMemory = NULL;
  OtmPlugin *pluginSelected = NULL;
  this->strLastError = "";
  this->iLastError = 0;
  this->Log.writef( "Open memory %s", pszMemoryName );

  pluginSelected = this->findPlugin( pszPluginName, pszMemoryName );
  if ( pluginSelected == NULL ) 
  {
    this->Log.write( "   Could not identify plugin processing the memory" );
    *piErrorCode = this->iLastError;
    return( NULL );
  } /* endif */

  std::string strMemoryName;
  this->getMemoryName( pszMemoryName, strMemoryName );

  OtmSharedMemoryPlugin *pSharedMemPlugin = NULL;
  BOOL fIsShared = this->isSharedMemory( strMemoryName, &pSharedMemPlugin );

  // use the plugin to open the memory
  if ( pluginSelected->getType() == OtmPlugin::eTranslationMemoryType )
  {
    pMemory = ((OtmMemoryPlugin *)pluginSelected)->openMemory( (char *)strMemoryName.c_str() , FALSE, NULLHANDLE, usOpenFlags );
    if ( pMemory == NULL ) *piErrorCode = this->iLastError = ((OtmMemoryPlugin *)pluginSelected)->getLastError( this->strLastError );
  }
  else if ( pluginSelected->getType() == OtmPlugin::eSharedTranslationMemoryType )
  {
    pMemory = ((OtmSharedMemoryPlugin *)pluginSelected)->openMemory( (char *)strMemoryName.c_str() , NULL, (usOpenFlags == 0) ? NONEXCLUSIVE : usOpenFlags );
    if ( pMemory == NULL ) *piErrorCode = this->iLastError = ((OtmSharedMemoryPlugin *)pluginSelected)->getLastError( this->strLastError );
  }
  if ( pMemory == NULL )
  {
    this->Log.writef( "   Open of local memory %s using plugin %s failed, the return code is %ld", strMemoryName.c_str(), pluginSelected->getName(), this->iLastError );
    return( NULL );
  } /* end */     

  // for shared memories using a local memory as a copy: open the associated shared memory
  if ( fIsShared && pSharedMemPlugin->isLocalMemoryUsed() )
  {
    this->Log.writef( "  Open shared component of memory using plugin %s", pSharedMemPlugin->getName(), this->iLastError );
    OtmMemory *pSharedMem = pSharedMemPlugin->openMemory( (char *)strMemoryName.c_str(), pMemory, 0 );
    if ( pSharedMem == NULL )
    {
      // close local memory
      this->closeMemory( pMemory );
      this->iLastError = pSharedMemPlugin->getLastError( this->strLastError );
      this->Log.writef( "  Open failed, return code is %ld, error message is %s", this->iLastError, this->strLastError.c_str() );
    }
    else
    {
      // use shared memory object from now on
      pMemory = pSharedMem;
      this->Log.write( "  Open successful" );
    } /* endif */
  } /* endif */

  return( pMemory );
}

/* \brief Get information from an existing memory
   \param pszPlugin plugin-name or NULL if not available or memory object name is used
   \param pszMemoryName name of the memory or memory object name (pluginname + colon + memory name)
   \param pInfo pointer to caller MemoryInfo structure
   \returns 0 when successful or error code
*/
int MemoryFactory::getMemoryInfo
(
  char *pszPluginName,
  char *pszMemoryName,
  OtmMemoryPlugin::PMEMORYINFO pInfo
)
{
  OtmPlugin *pluginSelected = NULL;
  int iRC = 0;
  this->strLastError = "";
  this->iLastError = 0;
  this->Log.writef( "Get info for memory %s", pszMemoryName );

  pluginSelected = this->findPlugin( pszPluginName, pszMemoryName );
  if ( pluginSelected == NULL ) 
  {
    this->Log.write( "   Could not identify plugin processing the memory" );
    return OtmMemoryPlugin::eUnknownPlugin;
  } /* endif */

  std::string strMemoryName;
  this->getMemoryName( pszMemoryName, strMemoryName );

  // use the plugin to get the memory info
  if ( pluginSelected->getType() == OtmPlugin::eTranslationMemoryType )
  {
    iRC = ((OtmMemoryPlugin *)pluginSelected)->getMemoryInfo( (char *)strMemoryName.c_str(), pInfo );
    if ( iRC != 0 ) iRC = this->iLastError = ((OtmMemoryPlugin *)pluginSelected)->getLastError( this->strLastError );
  }
  else if ( pluginSelected->getType() == OtmPlugin::eSharedTranslationMemoryType )
  {
    iRC = ((OtmSharedMemoryPlugin *)pluginSelected)->getMemoryInfo( (char *)strMemoryName.c_str(), pInfo );
    if ( iRC != 0 ) iRC = this->iLastError = ((OtmSharedMemoryPlugin *)pluginSelected)->getLastError( this->strLastError );
  }
  if ( iRC != 0 )
  {
    this->Log.writef( "   Could not retrieve information for local memory %s using plugin %s, the return code is %ld", strMemoryName.c_str(), pluginSelected->getName(), this->iLastError );
  }
  else
  {
    this->Log.write( "   info retrieval was successful" );
  } /* end */     

  return( iRC );
}

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
int MemoryFactory::getMemoryFiles
(
  char *pszPluginName,
  char *pszMemoryName,
  char *pFileListBuffer,
  int  iBufferSize,
  char *pszPluginNameOut
)
{
  OtmPlugin *pluginSelected = NULL;
  int iRC = 0;
  this->strLastError = "";
  this->iLastError = 0;
  this->Log.writef( "Get memory data files for memory %s", pszMemoryName );

  pluginSelected = this->findPlugin( pszPluginName, pszMemoryName );
  if ( pluginSelected == NULL ) 
  {
    this->Log.write( "   Could not identify plugin processing the memory" );
    return OtmMemoryPlugin::eUnknownPlugin;
  } /* endif */

  std::string strMemoryName;
  this->getMemoryName( pszMemoryName, strMemoryName );

  if ( pszPluginNameOut != NULL ) strcpy( pszPluginNameOut, pluginSelected->getName() );

  // use the plugin to get the memory files
  if ( pluginSelected->getType() == OtmPlugin::eTranslationMemoryType )
  {
    iRC = ((OtmMemoryPlugin *)pluginSelected)->getMemoryFiles( (char *)strMemoryName.c_str(), pFileListBuffer, iBufferSize );
    if ( iRC != 0 ) iRC = this->iLastError = ((OtmMemoryPlugin *)pluginSelected)->getLastError( this->strLastError );
  }
  else if ( pluginSelected->getType() == OtmPlugin::eSharedTranslationMemoryType )
  {
    iRC = ((OtmSharedMemoryPlugin *)pluginSelected)->getMemoryFiles( (char *)strMemoryName.c_str(), pFileListBuffer, iBufferSize );
    if ( iRC != 0 ) iRC = this->iLastError = ((OtmSharedMemoryPlugin *)pluginSelected)->getLastError( this->strLastError );
  }
  if ( iRC != 0 )
  {
    this->Log.writef( "   Could not retrieve data file names for local memory %s using plugin %s, the return code is %ld", strMemoryName.c_str(), pluginSelected->getName(), this->iLastError );
  }
  else
  {
    this->Log.write( "   data file names retrieval was successful" );
  } /* end */     

  return( iRC );
}


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
int MemoryFactory::importFromMemoryFiles
(
  char *pszPluginName,
  char *pszMemoryName,
  char *pFileListBuffer,
  int  iOptions,
  PVOID *ppPrivateData
)
{
  OtmPlugin *pluginSelected = NULL;
  int iRC = 0;
  this->strLastError = "";
  this->iLastError = 0;
  this->Log.writef( "import memory from memory data files %s", pszMemoryName );

  pluginSelected = this->findPlugin( pszPluginName, pszMemoryName );
  if ( pluginSelected == NULL ) 
  {
    this->Log.write( "   Could not identify plugin processing the memory" );
    return OtmMemoryPlugin::eUnknownPlugin;
  } /* endif */

  std::string strMemoryName;
  this->getMemoryName( pszMemoryName, strMemoryName );

  // use the plugin to import the memory files
  if ( pluginSelected->getType() == OtmPlugin::eTranslationMemoryType )
  {
    iRC = ((OtmMemoryPlugin *)pluginSelected)->importFromMemoryFiles( (char *)strMemoryName.c_str(), pFileListBuffer, iOptions, ppPrivateData );
    if ( iRC != 0 ) iRC = this->iLastError = ((OtmMemoryPlugin *)pluginSelected)->getLastError( this->strLastError );
  }
  if ( iRC == OtmMemoryPlugin::eRepeat )
  {
    // nothing to log here
  }
  else if ( iRC != 0 )
  {
    this->Log.writef( "   Could not import data file names for local memory %s using plugin %s, the return code is %ld", strMemoryName.c_str(), pluginSelected->getName(), this->iLastError );
  }
  else
  {
    this->Log.write( "   memory has been imported from memory data files successful" );
  } /* end */     

  return( iRC );
}


/* \brief Create a memory 
   \param pszPlugin plugin-name or NULL if not available or memory object name is used
   \param pszMemoryName name of the memory being created or
    memory object name (pluginname + colon + memory name)
   \param pszDescription description of the memory
   \param pszSourceLanguage source language of the memory
   \param piErrorCode pointer to a int varaibel receiving any error code when function fails
   \returns pointer to created memory object 
*/
OtmMemory *MemoryFactory::createMemory
(
  char *pszPluginName,
  char *pszMemoryName,
  char *pszDescription,
  char *pszSourceLanguage,
  int *piErrorCode
)
{
  return( this->createMemory( pszPluginName, pszMemoryName, pszDescription, pszSourceLanguage, '\0', NULL, false, piErrorCode ) );
}


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
OtmMemory *MemoryFactory::createMemory
(
  char *pszPluginName,
  char *pszMemoryName,
  char *pszDescription,
  char *pszSourceLanguage,
  char chDrive,
  int *piErrorCode
)
{
  return( this->createMemory( pszPluginName, pszMemoryName, pszDescription, pszSourceLanguage, chDrive, NULL, false, piErrorCode ) );
}

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
OtmMemory *MemoryFactory::createMemory
(
  char *pszPluginName,
  char *pszMemoryName,
  char *pszDescription,
  char *pszSourceLanguage,
  char chDrive,
  char *pszOwner,
  boolean bInvisible,
  int *piErrorCode
)
{
  OtmMemory *pMemory = NULL;
  OtmPlugin *pluginSelected = NULL;
  this->strLastError = "";
  this->iLastError = 0;
  this->Log.writef( "Create memory %s", pszMemoryName );

  if ( piErrorCode != NULL ) *piErrorCode = 0;

  pluginSelected = this->findPlugin( pszPluginName, pszMemoryName );
  if ( pluginSelected == NULL ) 
  {
    // use first available memory plugin for the new memory
    pluginSelected = (*pluginList)[0];
	// reset the iLastError
	// because the flag can be set in findPlugin if not plug exist
	this->iLastError = 0;
  } /* endif */

  if ( pluginSelected == NULL )
  {
    this->iLastError = *piErrorCode = ERROR_PLUGINNOTAVAILABLE;
    this->strLastError = "No memory plugin available for the creation of the memory";
    this->Log.writef( "   Create failed, with message \"%s\"", this->strLastError.c_str() );
    return( NULL );
  } /* endif */       

  // use the plugin to create the memory
  std::string strMemoryName;
  this->getMemoryName( pszMemoryName, strMemoryName );
  if ( pluginSelected->getType() == OtmPlugin::eTranslationMemoryType )
  {
    pMemory = ((OtmMemoryPlugin *)pluginSelected)->createMemory( (char *)strMemoryName.c_str(), pszSourceLanguage, pszDescription, FALSE, NULLHANDLE, chDrive );
    if ( pMemory == NULL ) this->iLastError = ((OtmMemoryPlugin *)pluginSelected)->getLastError( this->strLastError );
  }
  else   if ( pluginSelected->getType() == OtmPlugin::eSharedTranslationMemoryType )
  {
    pMemory = ((OtmSharedMemoryPlugin *)pluginSelected)->createMemory( (char *)strMemoryName.c_str(), pszSourceLanguage, pszDescription, chDrive, NULL, NULL );
    if ( pMemory == NULL )
    {
      this->iLastError = ((OtmSharedMemoryPlugin *)pluginSelected)->getLastError( this->strLastError );
    }
    else if ( (pszOwner != NULL) && (*pszOwner != EOS) ) 
    {
      ((OtmSharedMemoryPlugin *)pluginSelected)->setOwner( (char *)strMemoryName.c_str(), pszOwner );
    }
  }

  if ( pMemory == NULL)
  {
    this->Log.writef( "   Create failed, with message \"%s\"", this->strLastError.c_str() );
    if ( piErrorCode != NULL ) *piErrorCode = this->iLastError;
  }
  else if ( !bInvisible )
  {
    // send created notifcation
    PSZ pszObjName = NULL;
    UtlAlloc( (PVOID *)&pszObjName, 0L, MAX_LONGFILESPEC + MAX_LONGFILESPEC + 2, NOMSG );
    strcpy( pszObjName, pluginSelected->getName() );
    strcat( pszObjName, ":" );
    strcat( pszObjName, pszMemoryName );
    EqfSend2Handler( MEMORYHANDLER, WM_EQFN_CREATED, MP1FROMSHORT(  clsMEMORYDB  ), MP2FROMP( pszObjName ));
    if ( pszObjName != NULL ) UtlAlloc( (PVOID *)&pszObjName, 0L, 0L, NOMSG );
    this->Log.write( "   Create successful" );
  } /* endif */     

  return( pMemory );
}


/* \brief List memories from all memory plugins
   \param pfnCallBack callback function to be called for each memory
	 \param pvData caller's data pointetr, is passed to callback function
	 \param fWithDetails TRUE = supply memory details, when this flag is set, 
   the pInfo parameter of the callback function is set otherwise it is NULL
 	 \returns number of provided memories
*/
int MemoryFactory::listMemories
(
	OtmMemoryPlugin::PFN_LISTMEMORY_CALLBACK pfnCallBack,			  
	void *pvData,
	BOOL fWithDetails
)
{
  int iMemories = 0;

  for ( std::size_t i = 0; i < pluginList->size(); i++ )
  {
    OtmMemoryPlugin *pluginCurrent = (*pluginList)[i];

    iMemories += pluginCurrent->listMemories( pfnCallBack, pvData, fWithDetails );
  } /* endfor */      

  for ( std::size_t i = 0; i < pSharedMemPluginList->size(); i++ )
  {
    OtmSharedMemoryPlugin *pluginCurrent = (*pSharedMemPluginList)[i];

    iMemories += pluginCurrent->listMemories( pfnCallBack, pvData, fWithDetails );
  } /* endfor */      


  return( iMemories );
}

/* \brief Get a list of the active memory plugins
   \param vPluginList reference to caller's vector receiving the list of memory plugins
   \param vPluginList reference to caller's vector receiving the list of shared memory plugins
 	 \returns number of provided plugins
*/
int MemoryFactory::getMemoryPlugins
(
	std::vector<OtmMemoryPlugin *>&vMemPluginList,
	std::vector<OtmSharedMemoryPlugin *>&vSharedMemPluginList
)
{
  int iPlugins = 0;
  vMemPluginList.clear();
  for ( std::size_t i = 0; i < pluginList->size(); i++ )
  {
    OtmMemoryPlugin *pluginCurrent = (*pluginList)[i];
    vMemPluginList.push_back( pluginCurrent );
    iPlugins++;
  } /* endfor */    

  vSharedMemPluginList.clear();
  for ( std::size_t i = 0; i < pSharedMemPluginList->size(); i++ )
  {
    OtmSharedMemoryPlugin *pluginCurrent = (*pSharedMemPluginList)[i];
    vSharedMemPluginList.push_back( pluginCurrent );
    iPlugins++;
  } /* endfor */    

  return( iPlugins );
}

/* \brief Closed a previously opened memory
   \param pMemory pointer to memory object beign closed
	 \returns 0 when successful or error code
*/
int MemoryFactory::closeMemory
(
  OtmMemory *pMemory
)
{
  int iRC = 0;

  if ( pMemory == NULL  ) return( -1 );

  OtmMemoryPlugin *pPlugin = (OtmMemoryPlugin *)pMemory->getPlugin();
  if ( pPlugin == NULL  ) return( -2 );

  // build memory object name
  PSZ pszObjName = NULL;
  UtlAlloc( (PVOID *)&pszObjName, 0L, MAX_LONGFILESPEC + MAX_LONGFILESPEC + 2, NOMSG );
  strcpy( pszObjName, pPlugin->getName() );
  strcat( pszObjName, ":" );
  pMemory->getName( pszObjName + strlen(pszObjName), MAX_LONGFILESPEC );

  // close the memory
  if ( pPlugin->getType() == OtmMemoryPlugin::eSharedTranslationMemoryType )
  {
    OtmSharedMemoryPlugin *pSharedPlugin = (OtmSharedMemoryPlugin *)pPlugin;
    OtmSharedMemory *pSharedMem = (OtmSharedMemory *)pMemory;
    OtmMemory *pLocalMemory = pSharedMem->getLocalMemory();
    OtmMemoryPlugin *pLocalPlugin = (OtmMemoryPlugin *)pSharedMem->getLocalPlugin();
    iRC = pSharedPlugin->closeMemory( pMemory );
    if ( (pLocalMemory != NULL) && (pLocalPlugin != NULL) )
    {
      pLocalPlugin->closeMemory( pLocalMemory );
    }
  }
  else
  {
    iRC = pPlugin->closeMemory( pMemory );
  }

  // send a properties changed msg to memory handler
  EqfSend2Handler( MEMORYHANDLER, WM_EQFN_PROPERTIESCHANGED, MP1FROMSHORT( PROP_CLASS_MEMORY ), MP2FROMP( pszObjName ));

  if ( pszObjName != NULL ) UtlAlloc( (PVOID *)&pszObjName, 0L, 0L, NOMSG );
 
  return( iRC );
}

/*! \brief Rename a translation memory
  \param pszPluginName name of the memory being deleted
  \param pszOldMemoryName name of the memory being renamed or
  memory object name (pluginname + colon + memoryname)
  \param pszNewMemoryName new name for the memory 
	\returns 0 if successful or error return code
*/
int MemoryFactory::renameMemory(
  char *pszPluginName,
  char *pszOldMemoryName,
  char *pszNewMemoryName
)
{
  int iRC = OtmMemoryPlugin::eSuccess;

  OtmPlugin *plugin = this->findPlugin( pszPluginName, pszOldMemoryName );

  std::string strMemoryName;
  this->getMemoryName( pszOldMemoryName, strMemoryName );

  if ( plugin != NULL )
  {
    if ( plugin->getType() == OtmMemoryPlugin::eTranslationMemoryType )
    {
      iRC = ((OtmMemoryPlugin *)plugin)->renameMemory( (char *)strMemoryName.c_str(), pszNewMemoryName);
    }
    else if ( plugin->getType() == OtmMemoryPlugin::eSharedTranslationMemoryType )
    {
      iRC = ((OtmSharedMemoryPlugin *)plugin)->renameMemory( (char *)strMemoryName.c_str(), pszNewMemoryName);
    }
  }
  else
  {
    iRC = OtmMemoryPlugin::eMemoryNotFound;
  } /* endif */

  // broadcast change memory name
  if ( iRC == OtmMemoryPlugin::eSuccess )
  {
    strcpy( this->szMemObjName, plugin->getName() );
    strcat( this->szMemObjName,  ":" ); 
		strcat( this->szMemObjName, pszOldMemoryName );
    EqfSend2AllHandlers( WM_EQFN_DELETED, MP1FROMSHORT(clsMEMORYDB), MP2FROMP(this->szMemObjName) );
							 
    strcpy( this->szMemObjName, plugin->getName() );
    strcat( this->szMemObjName,  ":" ); 
		strcat( this->szMemObjName, pszNewMemoryName );
    EqfSend2AllHandlers( WM_EQFN_CREATED, MP1FROMSHORT(clsMEMORYDB), MP2FROMP(this->szMemObjName) );
	}

  return( iRC );
}

/*! \brief Physically delete a translation memory
  \param pszPluginName name of the memory being deleted
  \param pszMemoryName name of the memory being deleted or
  memory object name (pluginname + colon + memoryname)
	\returns 0 if successful or error return code
*/
int MemoryFactory::deleteMemory(
  char *pszPluginName,
  char *pszMemoryName
)
{
  std::string strError;
  int iRC = deleteMemory(pszPluginName,pszMemoryName, strError);

  return iRC;
}

/*! \brief Physically delete a translation memory
  \param pszPluginName name of the memory being deleted
  \param pszMemoryName name of the memory being deleted or
  memory object name (pluginname + colon + memoryname)
  \param strError return error message with it
	returns 0 if successful or error return code
*/
int MemoryFactory::deleteMemory(
  char *pszPluginName,
  char *pszMemoryName,
  std::string &strError
)
{
  int iRC = OtmMemoryPlugin::eSuccess;

  OtmPlugin *plugin = this->findPlugin( pszPluginName, pszMemoryName );

  if ( plugin != NULL )
  {
    std::string strMemoryName;
    this->getMemoryName( pszMemoryName, strMemoryName );

    // delete the shared memory on server
    OtmSharedMemoryPlugin *pSharedMemPlugin = NULL;
    if ( this->isSharedMemory( (char *)strMemoryName.c_str(), &pSharedMemPlugin ) && pSharedMemPlugin->isLocalMemoryUsed() )
    {
      this->Log.writef( " Delete shared memory using plugin %s", 
          pSharedMemPlugin->getName(), this->iLastError );
      iRC = pSharedMemPlugin->deleteMemory((char *)strMemoryName.c_str());
      if(iRC != 0)
      {
        pSharedMemPlugin->getLastError(strError);
        this->Log.writef( " Delete shared memory failed ");
        return iRC;
      }
    } 

    // use the given plugin to delete local memory
    if ( plugin->getType() == OtmMemoryPlugin::eTranslationMemoryType )
    {
      iRC = ((OtmMemoryPlugin *)plugin)->deleteMemory( (char *)strMemoryName.c_str());
      if ( iRC != 0 ) ((OtmMemoryPlugin *)plugin)->getLastError(strError);
    }
    else if ( plugin->getType() == OtmMemoryPlugin::eSharedTranslationMemoryType )
    {
      iRC = ((OtmSharedMemoryPlugin *)plugin)->deleteMemory((char *)strMemoryName.c_str());
      if ( iRC != 0 ) ((OtmSharedMemoryPlugin *)plugin)->getLastError(strError);
    }

    // broadcast deleted memory name
    if ( iRC == OtmMemoryPlugin::eSuccess )
    {
      strcpy( this->szMemObjName, plugin->getName() );
      strcat( this->szMemObjName,  ":" ); 
		  strcat( this->szMemObjName, pszMemoryName );
      EqfSend2AllHandlers( WM_EQFN_DELETED, MP1FROMSHORT(clsMEMORYDB), MP2FROMP(this->szMemObjName) );
	  }
  }
  else
  {
    strError = "Memory Not Found";
    iRC = OtmMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}


/*! \brief Delete all entries contained in a translation memory
  \param pszPluginName name of the memory being cleared
  \param pszMemoryName name of the memory being cleared or
  memory object name (pluginname + colon + memoryname)
	\returns 0 if successful or error return code
*/
int MemoryFactory::clearMemory(
  char *pszPluginName,
  char *pszMemoryName
)
{
  int iRC = OtmMemoryPlugin::eSuccess;

  OtmPlugin *plugin = this->findPlugin( pszPluginName, pszMemoryName );

  if ( plugin != NULL )
  {
    // use the given plugin
    if ( plugin->getType() == OtmMemoryPlugin::eTranslationMemoryType )
    {
      iRC = ((OtmMemoryPlugin *)plugin)->clearMemory( pszMemoryName );
    }
    else if ( plugin->getType() == OtmMemoryPlugin::eSharedTranslationMemoryType )
    {
    }
  }
  else
  {
    iRC = OtmMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}

/*! \brief Check if memory exists
  \param pszPlugin plugin-name or NULL if not available or memory object name is used
  \param pszMemoryName name of the memory being cleared or
  memory object name (pluginname + colon + memoryname)
	\returns 0 if successful or error return code
*/
BOOL MemoryFactory::exists(
  char *pszPluginName,
  char *pszMemoryName
)
{
  BOOL fExists = FALSE;

  OtmPlugin *plugin = this->findPlugin( pszPluginName, pszMemoryName );

  if ( plugin != NULL )
  {
    std::string strMemoryName;
    this->getMemoryName( pszMemoryName, strMemoryName );

    // get memory info using found plugin
    if ( plugin->getType() == OtmMemoryPlugin::eTranslationMemoryType )
    {
      OtmMemoryPlugin::MEMORYINFO *pInfo = new(OtmMemoryPlugin::MEMORYINFO);
      if ( ((OtmMemoryPlugin *)plugin)->getMemoryInfo( (char *)strMemoryName.c_str(), pInfo ) == 0 )
      {
        fExists = TRUE;
      } /* endif */         
      delete( pInfo );
    }
    else if ( plugin->getType() == OtmMemoryPlugin::eSharedTranslationMemoryType )
    {
      fExists = ((OtmSharedMemoryPlugin *)plugin)->isMemoryOwnedByPlugin((char *)strMemoryName.c_str());
    }

  } /* endif */
  return( fExists );
}

/*! \brief Check if memory is a shared/synchronized memory
  \param pMemory pointer to memory object
	\returns TRUE is memory is shared/synchronized
*/
BOOL MemoryFactory::isSharedMemory
( 
  OtmMemory *pMemory
)
{
  if( pMemory == NULL ) return( FALSE );

  OtmPlugin *pPlugin = (OtmPlugin *)pMemory->getPlugin();

  if( pPlugin == NULL ) return( FALSE );

  OtmPlugin::ePluginType type = pPlugin->getType();
  return( ( type == OtmMemoryPlugin::eSharedTranslationMemoryType ) ? TRUE : FALSE );
}

/*! \brief Check if memory is a shared/synchronized memory
  \param pszMemory Name of the memory
  \param pPlugin adress of a variable receiving the pointer to the plugin of the memory
	\returns TRUE is memory is shared/synchronized
*/
BOOL MemoryFactory::isSharedMemory
(
  char *pszMemory,
  OtmSharedMemoryPlugin **ppPlugin
)
{
  BOOL isShared = FALSE;

  // loop over all plugins for shared memories and test if memory is controlled by this plugin
  for ( std::size_t i = 0; (i < pSharedMemPluginList->size()) && !isShared; i++ )
  {
    OtmSharedMemoryPlugin *pluginCurrent = (*pSharedMemPluginList)[i];

    if ( pluginCurrent->isMemoryOwnedByPlugin( pszMemory ) )
    {
      isShared = TRUE;
      if ( ppPlugin != NULL ) *ppPlugin = pluginCurrent; 
    }
  } /* endfor */       
  return( isShared );
}

/*! \brief Check if memory is a shared/synchronized memory
  \param strMemory Name of the memory
  \param pPlugin adress of a variable receiving the pointer to the plugin of the memory
	\returns TRUE is memory is shared/synchronized
*/
BOOL MemoryFactory::isSharedMemory(
  std::string &strMemory,
  OtmSharedMemoryPlugin **ppPlugin
)
{
  return( this->isSharedMemory( (char *)strMemory.c_str(), ppPlugin ) );
}

  /*! \brief Create a temporary memory
  \param pszPrefix prefix to be used for name of the temporary memory
  \param pszName buffer for the name of the temporary memory
	\param pszSourceLang source language
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
	\returns Pointer to created translation memory or NULL in case of errors
*/
OtmMemory* MemoryFactory::createTempMemory(
	  PSZ pszPrefix,			  
	  PSZ pszName,			  
	  PSZ pszSourceLang
  )
{
  OtmMemory *pMemory = NULL;

  // always use first available plugin 
  OtmMemoryPlugin *plugin = (*pluginList)[0];

  pMemory = plugin->createTempMemory( pszPrefix, pszName, pszSourceLang, FALSE, NULLHANDLE );

  return( pMemory );
}

  /*! \brief Closes and deletes a temporary memory
  \param pszName name of the temporary memory
*/
void MemoryFactory::closeTempMemory(
	  OtmMemory *pMemory
)
{
  // always use first available plugin 
  OtmMemoryPlugin *plugin = (*pluginList)[0];

  plugin->closeTempMemory( pMemory );
}

 

/*! \brief Show error message for the last error
  \param pszPlugin plugin-name or NULL if not available or memory object name is used
  \param pszMemoryName name of the memory causing the problem
  memory object name (pluginname + colon + memoryname)
  \param pMemory pointer to existing memory object or NULL if not available
  \param hwndErrMsg handle of parent window message box
*/
void MemoryFactory::showLastError(
  char *pszPluginName,
  char *pszMemoryName,
  OtmMemory *pMemory,
  HWND hwndErrMsg
)
{
  pszPluginName; pszMemoryName;

  if ( pMemory != NULL )
  {
    // retrieve last error from memory plugin
    this->iLastError = pMemory->getLastError( this->strLastError );
  } /* endif */     

 // show error message
  PSZ pszParm = (PSZ)this->strLastError.c_str();
  UtlErrorHwnd( (USHORT)this->iLastError, MB_CANCEL, 1, &pszParm, SHOW_ERROR, hwndErrMsg );
}

std::string& MemoryFactory::getLastError( OtmMemory *pMemory, int& iLastError, std::string& strError)
{
    if ( pMemory != NULL )
        this->iLastError = pMemory->getLastError( this->strLastError );

    iLastError = this->iLastError;
    strError = this->strLastError;
    return strError;
}

/*! \brief Copy best matches from one proposal vector into another
  and sort the proposals
  \param SourceProposals refernce to a vector containing the source proposals
  \param TargetProposals reference to a vector receiving the copied proposals
  the vector may already contain proposals. The proposals are
  inserted on their relevance
  \param iMaxProposals maximum number of proposals to be filled in TargetProposals
  When there are more proposals available proposals with lesser relevance will be replaced
  \param iMTDisplayFactor factor for the placement of machine matches within the table
*/
void MemoryFactory::copyBestMatches(
  std::vector<OtmProposal *> &SourceProposals,
  std::vector<OtmProposal *> &TargetProposals,
  int iMaxProposals,
  int iMTDisplayFactor
)
{
  iMTDisplayFactor; 

  for ( std::size_t i = 0; i < SourceProposals.size(); i++ )
  {
    if ( !SourceProposals[i]->isEmpty() )
    {
      this->insertProposalData( SourceProposals[i], TargetProposals, iMaxProposals, (i+1) == SourceProposals.size() );
    } /* end */       
  } /* end */     

  // ignore all fuzzy matches when there are exact matchegetVs available
  if ( (TargetProposals.size() > 1) && TargetProposals[0]->isExactMatch()  )
  {
    for ( std::size_t i = 0; i < TargetProposals.size(); i++ )
    {
      if ( !TargetProposals[i]->isEmpty() && !TargetProposals[i]->isExactMatch() )
      {
        TargetProposals[i]->clear();
      } /* end */       
    } /* end */     
  } /* end */

  // ignore all normal exact matches when there are exact-exact matches available
  if ( (TargetProposals.size() > 1) && (TargetProposals[1]->getMatchType() == OtmProposal::emtExactExact)  )
  {
    for ( std::size_t i = 0; i < TargetProposals.size(); i++ )
    {
      if ( !TargetProposals[i]->isEmpty() && !(TargetProposals[i]->getMatchType() == OtmProposal::emtExactExact) )
      {
        TargetProposals[i]->clear();
      } /* end */       
    } /* end */     
  } /* end */
}

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
void MemoryFactory::insertProposalData(
  OtmProposal *newProposal,
  std::vector<OtmProposal *> &Proposals,
  int iMaxProposals,
  BOOL fLastEntry, 
  int iMTDisplayFactor
)
{
  int iNewProposalSortKey = this->getProposalSortKey( *newProposal, iMTDisplayFactor, 0, false );

  fLastEntry;

  // find correct place for the new proposal
  for ( std::size_t i = 0; (i < Proposals.size()) && (i < (std::size_t)iMaxProposals); i++ )
  {
    BOOL fEndOfTable = ((i + 1) == Proposals.size()) || ((i +1) == (std::size_t)iMaxProposals);
    int iExistingProposalSortKey = this->getProposalSortKey( *(Proposals[i]), iMTDisplayFactor, 0, fEndOfTable );
    if ( iNewProposalSortKey == iExistingProposalSortKey )
    {
      // check for same target string 
      if ( Proposals[i]->isSameTarget( newProposal ) )
      {
        // same target exists already, leaver newer proposal in list

        if ( Proposals[i]->getUpdateTime() >= newProposal->getUpdateTime() )
        {
          // existing proposal is newer, so ignore this one
          return;
        } /* endif */

        // replace existing proposal with newer one
        *Proposals[i] = *newProposal;
        return;
      }
      else
      {
        if ( Proposals[i]->getUpdateTime() < newProposal->getUpdateTime() )
        {
          // new proposal is a newer one, so insert it here

          // make room for new proposal
//        int j = i;
//        while ( j < (iMaxProposals - 1))
//        {
//          *Proposals[j+1] = *Proposals[j];
//          j++;
//        } /* endwhile */         

//    Change to copy proposals starting at the end to avoid
//    creating duplicate entries.    15-07-17
      int j = iMaxProposals - 1;
      while ( j > (int)i )
      {
        *Proposals[j] = *Proposals[j-1];
        j--;
      } /* endwhile */         

          // insert proposal here
          *Proposals[i] = *newProposal;

          return;
        } /* endif */
      } /* end */         
    }
    else if ( iNewProposalSortKey > iExistingProposalSortKey )
    {
      // make room for new proposal
//    int j = i;
//    while ( j < (iMaxProposals - 1))
//    {
//      *Proposals[j+1] = *Proposals[j];
//      j++;
//    } /* endwhile */         

//    Change to copy proposals starting at the end to avoid
//    creating duplicate entries.    15-07-17
      int j = iMaxProposals - 1;
      while ( j > (int)i )
      {
        *Proposals[j] = *Proposals[j-1];
        j--;
      } /* endwhile */         

      // insert proposal here
      *Proposals[i] = *newProposal;

      return;
    } /* endif */       
  } /* end */     
  return;
}


/*! \brief Get the sort order key for a memory match

  \param Proposal reference to a proposal for which the sort key is evaluated
  \returns the proposal sort key
*/
int MemoryFactory::getProposalSortKey(  OtmProposal &Proposal )
{
  return( getProposalSortKey( Proposal, -1, 0, FALSE ) ); 
}


/*! \brief Get the sort order key for a memory match

 the current order is: Exact-Exact -> Exact -> Global memory(Exact) -> Replace -> Fuzzy -> Machine
 We use the folloging sort key values
 - Exact-Exact = 1200
 - Exact-ContextSameDoc =  1000
 - Exact-Context     =  900-999
 - Exact-SameDoc     =  801
 - Exact             =  800
 - Global memory (Exact)    =  700
 - Global memory Star(Exact)= 600
 - Fuzzy             =  301-399
 - Machine           =  200 (400 at the end of the table)
 - empty entry       =  0

  Special handling for MT display factor (if given):
  - 0%  Machine =  200
  - 100%  Machine =  400
  - 1%-99% Machine =  3xx (xx = MT display factor)

  Special mode at the end of the match table:

  When the end of the table is reached MT proposals get a key higher than fuzzy ones to keep at least one MT match in the table 
  
  \param Proposal reference to a proposal for which the sort key is evaluated
  \param iMTDisplayFactor the machine translation display factor, -1 to ignore the factor
  \param usContextRanking the context ranking for the proposal
  \param TargetProposals reference to a vector receiving the copied proposals
  the vector may already contain proposals. The proposals are
  inserted on their relevance
  \param iMaxProposals maximum number of proposals to be filled in TargetProposals
  When there are more proposals available proposals with lesser relevance will be replaced
  \returns the proposal sort key
*/
int MemoryFactory::getProposalSortKey(  OtmProposal &Proposal, int iMTDisplayFactor, USHORT usContextRanking, BOOL fEndOfTable )
{
  return( getProposalSortKey( Proposal.getMatchType(), Proposal.getType(), Proposal.getFuzziness(), iMTDisplayFactor, usContextRanking, fEndOfTable ) ); 
}

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
int MemoryFactory::getProposalSortKey(  OtmProposal::eMatchType MatchType, OtmProposal::eProposalType ProposalType, 
    int iFuzziness, int iMTDisplayFactor, USHORT usContextRanking, BOOL fEndOfTable )
{

  // dummy (=empty) entries
  if ( (MatchType == OtmProposal::emtUndefined) || (iFuzziness == 0) )
  {
    return( 0 );  
  } /* endif */  

  // use system MT display factor if none has been specified
  if ( iMTDisplayFactor == -1 )
  {
    iMTDisplayFactor = UtlQueryULong( QL_MTDISPLAYFACTOR );
  }

  // normal matches
  if ( ProposalType == OtmProposal::eptManual )
  {
    // exact matches with correct context have the highest priority
    if ( (usContextRanking == 100) && 
         ( (MatchType == OtmProposal::emtExact) || 
           (MatchType == OtmProposal::emtExactContext) ||
           (MatchType == OtmProposal::emtExactExact) ) )
    {
      return( 1300 );
    }
    else if ( (usContextRanking > 0) && (usContextRanking < 100) )
    {
      if ( MatchType == OtmProposal::emtExactExact )
      {
        return( 1200 );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtExactContext )
      {
        return( 900 + usContextRanking  );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtExact )
      {
        return( 900 + usContextRanking -1  );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtFuzzy )
      {
        return( 300 + iFuzziness );  
      } /* endif */  
      // anything else...
      return( 0 );
    }
    else
    {
      if ( MatchType == OtmProposal::emtExactExact )
      {
        return( 1200 );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtExactContext )
      {
        return( 801 );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtExact )
      {
        return( 800 );  
      } /* endif */  
      // fuzzy and replace matches
      if ( MatchType == OtmProposal::emtReplace )
      {
        return( 400 );  
      } /* endif */  
      if ( MatchType == OtmProposal::emtFuzzy )
      {
        return( 300 + iFuzziness );  
      } /* endif */  
      // anything else...
      return( 0 );
    } /* endif */       
  } /* endif */  

  // machine matches
  if ( ProposalType == OtmProposal::eptMachine )
  {
    if ( (iMTDisplayFactor <= 0) || (iMTDisplayFactor > 100) ) // invalid (>100)  / not specified (-1) or 0
    {
      if ( fEndOfTable )
      {
        return( 400 );  
      }
      else
      {
        return( 200 );  
      } /* endif */         
    } 
    else if ( iMTDisplayFactor == 100 )
    {
      return( 400 );  
    }
    else 
    {
      return( 300 + iMTDisplayFactor );  
    } /* endif */       
  } /* endif */  

  // global memory matches
  if ( ProposalType == OtmProposal::eptGlobalMemory )
  {
    if ( iFuzziness >= 100 )
    {
      return( 700 );  
    }
    else
    {
      // fuzzy global memory matches: treat as normal fuzzies
      return( 300 + iFuzziness );  
    } /* endif */       
  } /* endif */  

  // global memory star matches
  if ( ProposalType == OtmProposal::eptGlobalMemoryStar )
  {
    if ( iFuzziness >= 100 )
    {
      return( 600 );  
    }
    else
    {
      // fuzzy global memory matches: treat as normal fuzzies
      return( 300 + iFuzziness );  
    } /* endif */       
  } /* endif */  

  // fuzzy and replace matches
  if ( ProposalType == OtmProposal::emtReplace )
  {
    return( 400 );  
  } /* endif */  
  if ( ProposalType == OtmProposal::emtFuzzy )
  {
    return( 300 + iFuzziness );  
  } /* endif */  

  // anything else...
  return( 0 );
} /* end of method MemoryFactory::getProposalSortKey */


/*! \brief Check if first proposal in the list can be used for automatic substitution 
  \param Proposals reference to a vector containing the proposals
  \returns true when automatic substitution can be performed otherwise false
*/
bool MemoryFactory::isAutoSubstCandidate(
  std::vector<OtmProposal *> &Proposals
)
{
  bool fAutoSubst = true;
              
  // currently only manual translations are allowed for auto-substitution
  if ( Proposals[0]->getType() != OtmProposal::eptManual )
  {
    return( false );  
  } /* endif */

  //  check if best match is an exact match or "better" (exact exact) 
  if ( Proposals[0]->isExactMatch() )
  {
    int iProposals = OtmProposal::getNumOfProposals( Proposals );

    // more than one exact match?
    if ( (iProposals > 1) && Proposals[1]->isExactMatch() )
    {
        // when second match is a global memory match allow autosubstitution
        if ( Proposals[1]->getType() == OtmProposal::eptGlobalMemory )
        {
          fAutoSubst = true;
        }
        else if ( (Proposals[0]->getMatchType() == OtmProposal::emtExactExact) && (Proposals[1]->getMatchType() != OtmProposal::emtExactExact) )
        {
          // allow auto-substitution for single exact-exact matches 
          fAutoSubst = true;
        }
        else if ( Proposals[0]->getContextRanking() > Proposals[1]->getContextRanking() )
        {
          // allow auto-substitutiun for exact match with better context ranking 
          fAutoSubst = true;
        }
        else
        {
          // no auto substitution because of multiple exact matches
          fAutoSubst = false;
        }
    }
    else
    {
      // allow auto-substitution for single exact-exact matches 
      fAutoSubst = true;
    } /* endif */   
  }
  else
  {
    /* best match is an fuzzy match do not use for auto replace       */
    fAutoSubst = false;
  } /* endif */

  return fAutoSubst;  
} /* end of method MemoryFactory::isAutoSubstCandidate */


/*! \brief Create the shared part of a memory
  \param hwndOwner owner handle for dialog windows
  \param pszPluginName name of the memory plugin to be used
  \param pszMemoryName name of the memory
  \param pLocalMemory pointer to local version of the created memory
  \returns pointer to shared memory object or NULL in case of errors
*/
OtmMemory *MemoryFactory::createSharedMemory(
  HWND hwndOwner,
  char *pszPluginName,
  char *pszMemoryName,
  OtmMemory *pLocalMemory
)
{
  std::vector<std::string> *pvLabels;
  std::vector<std::string> *pvData;
  std::vector<std::string> *pvDescr;
  PFN_OPTIONSDIALOGCHECKDATA pfnCheckCallback;
  OtmMemory* pSharedMem = NULL;
  long lHandle = 0;
  int iRC = 0;
  BOOL fOK = TRUE;

  this->Log.writef( "Create shared memory %s", pszMemoryName );

  // check if a shared memory plugin is available
  if ( (this->pSharedMemPluginList == NULL) || (this->pSharedMemPluginList->size() == 0))
  {
    this->iLastError = ERROR_NOSHAREDMEMORYPLUGIN;
    this->strLastError = "No plugin for shared memory handling is available";
    if ( (hwndOwner != NULLHANDLE) && (hwndOwner != HWND_FUNCIF) )
    {
      this->Log.writef( "   %s", this->strLastError.c_str() );
      MessageBox( hwndOwner, this->strLastError.c_str(), "Error", MB_OK );
    } /* end */       

    return( NULL );
  } /* end */     

  OtmSharedMemoryPlugin *plugin = this->getSharedMemoryPlugin( pszPluginName ); 

  // get list of required options for a shared memory
  this->Log.write( "   Getting list of required options" );
  iRC = plugin->getCreateOptionFields( pszMemoryName, &pvLabels, &pvData, &pvDescr, &pfnCheckCallback, &lHandle );
   
  // show options dialog
  if ( iRC == 0 )
  {
    this->Log.write( "   Showing option dialog" );
    fOK = UtlOptionsDlg( hwndOwner, 1000, pvLabels, pvData, pvDescr, "Create a shared memory", pfnCheckCallback, lHandle );
  }
  
  // create the shared memory
  if ( fOK )
  {
    // get some data from local memory if available
    static char szDescription[100];
    static char szLanguage[40];

    if ( pLocalMemory != NULL )
    {
      pLocalMemory->getDescription( szDescription, sizeof(szDescription) );
      pLocalMemory->getSourceLanguage( szLanguage, sizeof(szLanguage) );
    }
    else
    {
      szDescription[0] = '\0';
      szLanguage[0] = '\0';
    }
    
    Log.write( "   calling actual memory create function" );
    pSharedMem = plugin->createMemory( pszMemoryName, szLanguage, szDescription, '\0', pvData, pLocalMemory );
    
    if ( pSharedMem == NULL )
    {
      this->iLastError = plugin->getLastError( this->strLastError );
      return( NULL );
    } /* end */       
  }
  else
  {
    this->Log.write( "   Option dialog returned FALSE" );
  } /* end */     

  return( pSharedMem );
}

/*! \brief Provide the names of shared memories available for a given user
	\param pszPlugin  name of the shared memory plugin to be used
	\param pvOptions pointer to a vector containing the access options
  \param pvConnected pointer to a vector receiving the names of the connected memories
  \param pvNotConnected pointer to a vector receiving the names of the not connected memories
	\returns number of provided memories
*/
int MemoryFactory::listSharedMemories(
  char *pszPlugin,
  std::vector<std::string> *pvOptions,
  std::vector<std::string> *pvConnected,
  std::vector<std::string> *pvNotConnected
)
{
  this->Log.write( "List shared memory databases" );

  OtmSharedMemoryPlugin *plugin = this->getSharedMemoryPlugin( pszPlugin ); 

  if ( plugin == NULL )
  {
    this->iLastError = ERROR_NOSHAREDMEMORYPLUGIN;
    this->strLastError = "No plugin for shared memory handling is available";
    this->Log.writef( "   %s", this->strLastError.c_str() );
    return( 0 );
  } /* end */     

  return( plugin->listSharedMemories( pvOptions, pvConnected, pvNotConnected ) );
}


/*! \brief Connect to a shared memory
	\param pszPlugin  name of the shared memory plugin to be used
  \param hwndOwner owner handle for dialog windows
  \param pszMemoryName name of the memory
  \param pvOptions ptr to a vector containing the access options
  \returns 0 when successful or error return code
*/
int MemoryFactory::connectToMemory(
	char *pszPlugin,
  HWND hwndOwner,
  char *pszMemoryName,
  std::vector<std::string> *pvOptions
)
{
  OtmMemory* pLocalMem = NULL;
  int iRC = 0;
  this->iLastError = 0;
  this->strLastError = "";

  OtmSharedMemoryPlugin *plugin = this->getSharedMemoryPlugin( pszPlugin ); 

  // check if a shared memory plugin is available
  if ( plugin == NULL )
  {
    this->iLastError = ERROR_NOSHAREDMEMORYPLUGIN;
    this->strLastError = "No plugin for shared memory handling is available";
    if ( (hwndOwner != NULLHANDLE) && (hwndOwner != HWND_FUNCIF) )
    {
      MessageBox( hwndOwner, this->strLastError.c_str(), "Error", MB_OK );
    } /* end */       

    return( this->iLastError );
  } /* end */     

     
  // create local memory (use any memory plugin for this)
  if ( plugin->isLocalMemoryUsed() )
  {
    pLocalMem = this->createMemory( NULL, pszMemoryName, "", "English(U.S.)", &iRC );
    if ( pLocalMem == NULL )
    {
      if ( (hwndOwner != NULLHANDLE) && (hwndOwner != HWND_FUNCIF) )
      {
        MessageBox( hwndOwner, this->strLastError.c_str(), "Error", MB_OK );
      } /* end */       
      return( iRC );
    }
    else
    {
      this->closeMemory( pLocalMem );
      pLocalMem = NULL;
    } /* end */     
  }

   // First to create memory, then connect to server, otherwise have problem with replicator
   // connect to the shared memory
  iRC = plugin->connectToMemory( pszMemoryName, pvOptions );
  if ( iRC != 0 )
  {
    this->iLastError = plugin->getLastError( this->strLastError );
    if ( (hwndOwner != NULLHANDLE) && (hwndOwner != HWND_FUNCIF) )
    {
      MessageBox( hwndOwner, this->strLastError.c_str(), "Error", MB_OK );
    } /* end */       
    return( this->iLastError );
  } /* end */

  return( this->iLastError );
}

/*! \brief Disconnect a connected memory
	\param pszPlugin  name of the shared memory plugin to be used
  \param hwndOwner owner handle for dialog windows and error messages
  \param pszMemoryName name of the memory
  \returns 0 when successful or error return code
*/
int MemoryFactory::disconnectMemory(
	char *pszPlugin,
  HWND hwndOwner,
  char *pszMemoryName
)
{
  int iRC = 0;
  this->iLastError = 0;
  this->strLastError = "";

  OtmSharedMemoryPlugin *plugin = this->getSharedMemoryPlugin( pszPlugin ); 

  // check if a shared memory plugin is available
  if ( plugin == NULL )
  {
    this->iLastError = ERROR_NOSHAREDMEMORYPLUGIN;
    this->strLastError = "No plugin for shared memory handling is available";
    if ( (hwndOwner != NULLHANDLE) && (hwndOwner != HWND_FUNCIF) )
    {
      MessageBox( hwndOwner, this->strLastError.c_str(), "Error", MB_OK );
    } /* end */       

    return( this->iLastError );
  } /* end */     

  // disconnect the shared memory
  iRC = plugin->disconnectMemory( pszMemoryName );
  if ( iRC != 0 )
  {
    this->iLastError = plugin->getLastError( this->strLastError );
    if ( (hwndOwner != NULLHANDLE) && (hwndOwner != HWND_FUNCIF) )
    {
      MessageBox( hwndOwner, this->strLastError.c_str(), "Error", MB_OK );
    } /* end */       
    return( this->iLastError );
  } /* end */     

  // delete the local memory (if any)
  if ( plugin->isLocalMemoryUsed() )
  {
    this->deleteMemory( NULL, pszMemoryName );
  }

  return( this->iLastError );
}





/*! \brief get the options required to connect to a sharedmemory
	\param pszPlugin  name of the shared memory plugin to be used
  \param hwndOwner owner handle for dialog windows
  \param pvOptions pointer to a vector receiving the connect options
  \returns 0 when successful, -1 when user has cancelled the options dialog or an error code
*/
int MemoryFactory::getConnectOptions(
	char *pszPlugin,
  HWND hwndOwner,
  std::vector<std::string> *pvOptions
)
{
  std::vector<std::string> *pvLabels;
  std::vector<std::string> *pvData;
  std::vector<std::string> *pvDescr;
  PFN_OPTIONSDIALOGCHECKDATA pfnCheckCallback;
  long lHandle = 0;
  int iRC = 0;

  OtmSharedMemoryPlugin *plugin = this->getSharedMemoryPlugin( pszPlugin ); 

  // check if a shared memory plugin is available
  if ( plugin == NULL )
  {
    this->iLastError = ERROR_NOSHAREDMEMORYPLUGIN;
    this->strLastError = "No plugin for shared memory handling is available";
    if ( (hwndOwner != NULLHANDLE) && (hwndOwner != HWND_FUNCIF) )
    {
      MessageBox( hwndOwner, this->strLastError.c_str(), "Error", MB_OK );
    } /* end */       

    return( -1 );
  } /* end */     

  // get list of required options to connect to a shared memory
  iRC = plugin->getAccessOptionFields( &pvLabels, &pvData, &pvDescr, &pfnCheckCallback, &lHandle );
  if ( iRC != 0 )
  {
    // no options required for this plugin
    pvOptions->clear();
    return( 0 );
  }

  // show options dialog
  std::string strTitle = "Options to connect to a shared memory (";
  strTitle.append( plugin->getDescriptiveMemType() );
  strTitle.append( ")" );
  BOOL fOK = UtlOptionsDlg( hwndOwner, 1000, pvLabels, pvData, pvDescr, strTitle.c_str(), pfnCheckCallback, lHandle );
  if ( !fOK )
  {
    return( -1 );
  } /* endif */     

  // copy options to callers vector
  pvOptions->resize( pvData->size() );
  for ( std::size_t i = 0; i < pvData->size(); i++ )
  {
    pvOptions->at(i) = pvData->at(i);
  } /* endif */     

  return( 0 );
}

/*! \brief Get name of default memory plugin
	\returns pointer to name of default memory plugin
*/
const char * MemoryFactory::getDefaultMemoryPlugin(
)
{
  return( this->szDefaultMemoryPlugin );
}

/*! \brief Get name of default shared memory plugin
	\returns pointer to name of default shared memory plugin
*/
const char *MemoryFactory::getDefaultSharedMemoryPlugin(
)
{
  return( this->szDefaultSharedMemoryPlugin );
}



/*! \brief Get the object name for the memory
  \param pMemory pointer to the memory object
  \param pszObjName pointer to a buffer for the object name
  \param iBufSize size of the object name buffer
  \returns 0 when successful or the error code
*/
int MemoryFactory::getObjectName( OtmMemory *pMemory, char *pszObjName, int iBufSize )
{
  OtmMemoryPlugin *pPlugin;

  if ( pMemory == NULL )
  {
      this->iLastError = ERROR_MEMORYOBJECTISNULL;
      this->strLastError = "Internal error, supplied memory object is null";
      return( this->iLastError );
  } /* endif */     

  pPlugin = (OtmMemoryPlugin *)pMemory->getPlugin();
  if ( (pPlugin->getType() == OtmPlugin::eSharedTranslationMemoryType) && ((OtmSharedMemoryPlugin *)pPlugin)->isLocalMemoryUsed() )
  {
    OtmSharedMemory *pSharedMem = (OtmSharedMemory *)pMemory;
    pPlugin = (OtmMemoryPlugin *)pSharedMem->getLocalPlugin();
  } /* endif */     

  if ( pPlugin == NULL )
  {
      this->iLastError = ERROR_PLUGINNOTAVAILABLE;
      this->strLastError = "Internal error: No plugin found for memory object";
      return( this->iLastError );
  } /* endif */     

  const char *pszPluginName = pPlugin->getName();
  std::string strMemName;
  pMemory->getName( strMemName );
  if ( ((int)strlen( pszPluginName ) + (int)strMemName.length() + 2) > iBufSize)
  {
    return( ERROR_BUFFERTOOSMALL );
  } /* endif */     

  strcpy( pszObjName, pszPluginName );
  strcat( pszObjName, ":" );
  strcat( pszObjName, strMemName.c_str() );

  return( 0 );
}

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
int MemoryFactory::splitObjName( char *pszObjName, char *pszPluginName, int iPluginBufSize, char *pszMemoryName, int iNameBufSize  )
{
  if ( pszObjName == NULL )
  {
    this->iLastError = ERROR_MEMORYOBJECTISNULL;
    this->strLastError = "Internal error: No memory object name specified";
    return( this->iLastError );
  } /* endif */     

  char *pszNamePos = strchr( pszObjName, ':' );
  if ( pszNamePos == NULL )
  {
    this->iLastError = ERROR_INVALIDOBJNAME;
    this->strLastError = "Internal error: Invalid memory object name";
    return( this->iLastError );
  } /* endif */     

  if ( pszPluginName != NULL )
  {
    int iPluginNameLen = pszNamePos - pszObjName;
    if ( iPluginNameLen >= iPluginBufSize )
    {
      this->iLastError = ERROR_BUFFERTOOSMALL;
      this->strLastError = "Internal error: Buffer too small";
      return( this->iLastError );
    } /* endif */     
    strncpy( pszPluginName, pszObjName, iPluginNameLen );
    pszPluginName[iPluginNameLen] = 0;
  } /* end */     

  if ( pszMemoryName != NULL )
  {
    pszNamePos += 1;
    int iMemoryNameLen = strlen( pszNamePos );
    if ( iMemoryNameLen >= iNameBufSize )
    {
      this->iLastError = ERROR_BUFFERTOOSMALL;
      this->strLastError = "Internal error: Buffer too small";
      return( this->iLastError );
    } /* endif */     
    strcpy( pszMemoryName, pszNamePos );
  } /* end */     
  return( 0 );
}


// helper functions 

/* \brief Get memory plugin with the given pkugin name
   \param pszPlugin plugin-name
   \returns pointer to plugin or NULL if no memory pluging with the given name is specified
*/
OtmPlugin *MemoryFactory::getPlugin
(
  const char *pszPluginName
)
{
  // find plugin using plugin name
  if ( (pszPluginName != NULL) && (*pszPluginName != '\0') )
  {
    for ( std::size_t i = 0; i < pluginList->size(); i++ )
    {
      OtmPlugin *pluginCurrent = (OtmPlugin *)(*pluginList)[i];
      if ( strcmp( pluginCurrent->getName(), pszPluginName ) == 0 )
      {
        return ( pluginCurrent );
      } /* endif */         
    } /* endfor */       
  } /* endif */     

  if ( (pszPluginName != NULL) && (*pszPluginName != '\0') )
  {
    for ( std::size_t i = 0; i < pSharedMemPluginList->size(); i++ )
    {
      OtmPlugin *pluginCurrent = (OtmPlugin *)(*pSharedMemPluginList)[i];
      if ( strcmp( pluginCurrent->getName(), pszPluginName ) == 0 )
      {
        return ( pluginCurrent );
      } /* endif */         
    } /* endfor */       
  } /* endif */     

  return( NULL );
}


  /* \brief Find memory plugin for this memory using input
    data or find memory testing all memory plugins
   \param pszPlugin plugin-name or NULL if not available
   \param pszMemoryName memory name or memory object name (pluginname + colon + memoryname)
   \returns pointer to plugin or NULL if no memory pluging with the given name is specified
*/
OtmPlugin *MemoryFactory::findPlugin
(
  char *pszPluginName,
  char *pszMemoryName
)
{
  OtmPlugin *plugin = NULL;

  // find plugin using plugin name
  if ( (pszPluginName != NULL) && (*pszPluginName != '\0') )
  {
    plugin = this->getPlugin( pszPluginName );
    if ( plugin != NULL )
    {
      return( plugin );
    }
    else
    {
      this->iLastError = ERROR_PLUGINNOTAVAILABLE;
      this->strLastError = "Error: selected plugin " + std::string( pszPluginName ) + "is not available";
    } /* endif */       
  } /* endif */     

  // check for memory object names
  if ( (pszMemoryName != NULL) && (*pszMemoryName != '\0') )
  {
    char *pszColon  = strchr( pszMemoryName, ':' );
    if ( pszColon != NULL )
    {
      // split object name into plugin name and memory name
      std::string strPlugin = pszMemoryName;
      strPlugin.erase( pszColon - pszMemoryName );
      plugin = this->getPlugin( strPlugin.c_str() );
      if ( plugin != NULL )
      {
        return( plugin );
      }
      else
      {
        this->iLastError = ERROR_PLUGINNOTAVAILABLE;
        this->strLastError = "Error: selected plugin " + strPlugin + "is not available";
      } /* endif */       
    }
    else
    { 
      OtmMemoryPlugin::MEMORYINFO *pInfo = new(OtmMemoryPlugin::MEMORYINFO);

      // find plugin by querying memory info from all available plugins
      for ( std::size_t i = 0; i < pluginList->size(); i++ )
      {
        OtmMemoryPlugin *pluginCurrent = (*pluginList)[i];
        if ( pluginCurrent->getMemoryInfo( pszMemoryName, pInfo ) == 0 )
        {
          plugin = (OtmPlugin *)pluginCurrent;
          break;
        } /* endif */         
      } /* endfor */       
      delete( pInfo );

      // try shared memory plugins
      if ( plugin == NULL )
      {
        for ( std::size_t i = 0; i < pSharedMemPluginList->size(); i++ )
        {
          OtmSharedMemoryPlugin *pluginCurrent = (*pSharedMemPluginList)[i];
          if ( pluginCurrent->isMemoryOwnedByPlugin( pszMemoryName ) )
          {
            plugin = (OtmPlugin *)pluginCurrent;
            break;
          } /* endif */         
        } /* endfor */       
      }

      if ( plugin == NULL )
      {
        this->iLastError = ERROR_MEMORY_NOTFOUND;//ERROR_PLUGINNOTAVAILABLE
        this->strLastError = "Translation Memory "+std::string(pszMemoryName)+" was not found.";//"Error: no memory plugin for memory " + std::string(pszMemoryName) + " found or memory does not exist";
      } /* endif */       
    } /* endif */       
  }
  else
  {
    this->iLastError = ERROR_MISSINGPARAMETER;
    this->strLastError = "Error: Missing memory name";
  } /* endif */     

  return( plugin );
}

/* \brief Get the memory name from a potential memory object name
   \param pszMemoryName memory name or memory object name (pluginname + colon + memoryname)
   \param strMemoryName reference to string receiving memory name without any plugin name
   \returns 0
*/
int MemoryFactory::getMemoryName
(
  char *pszMemoryName,
  std::string &strMemoryName
)
{
 if ( (pszMemoryName == NULL) || (*pszMemoryName == '\0') ) return( -1 );

 char *pszColon  = strchr( pszMemoryName, ':' );
 if ( pszColon != NULL )
 {
   strMemoryName = pszColon + 1;
 }
 else
 {
   strMemoryName = pszMemoryName;
 }
 return( 0 );
}

void MemoryFactory::refreshPluginList() 
{
  if ( this->pluginList != NULL ) delete( this->pluginList );
  if ( this->pSharedMemPluginList != NULL ) delete( this->pSharedMemPluginList );

  this->szDefaultMemoryPlugin[0] = '\0';
  this->szDefaultSharedMemoryPlugin [0] = '\0';

  // access plugin manager
	PluginManager* thePluginManager = PluginManager::getInstance();

  // iterate through all memory plugins and store found plugins in pluginList
  OtmMemoryPlugin * curPlugin = NULL;
  pluginList = new std::vector<OtmMemoryPlugin *>;
  int i = 0;
  do
  {
    i++;
    curPlugin = (OtmMemoryPlugin*) thePluginManager->getPlugin(OtmPlugin::eTranslationMemoryType, i );
    if ( curPlugin != NULL ) 
    {
      pluginList->push_back( curPlugin );
      if ( stricmp( curPlugin->getName(), DEFAULTMEMORYPLUGIN ) == 0 )
      {
        strcpy( this->szDefaultMemoryPlugin, curPlugin->getName() );
      }
    }
  }  while ( curPlugin != NULL ); /* end */     

  // if default plugin is not available use first plugin as default
  if ( (this->szDefaultMemoryPlugin[0] == '\0') && (pluginList->size() != 0) )
  {
    strcpy( this->szDefaultMemoryPlugin, (*pluginList)[0]->getName() );
  }

  OtmSharedMemoryPlugin *curSharedPlugin = NULL;
  pSharedMemPluginList = new std::vector<OtmSharedMemoryPlugin *>;
  i = 0;
  do
  {
    i++;
    curSharedPlugin = (OtmSharedMemoryPlugin*) thePluginManager->getPlugin(OtmPlugin::eSharedTranslationMemoryType, i );
    if ( curSharedPlugin != NULL ) 
    {
      pSharedMemPluginList->push_back( curSharedPlugin );
      if ( stricmp( curSharedPlugin->getName(), DEFAULTSHAREDMEMORYPLUGIN ) == 0 )
      {
        strcpy( this->szDefaultMemoryPlugin, curSharedPlugin->getName() );
      }
    }
  }  while ( curSharedPlugin != NULL ); /* end */     

  // if default plugin is not available use first plugin as default
  if ( (this->szDefaultSharedMemoryPlugin[0] == '\0') && (pSharedMemPluginList->size() != 0) )
  {
    strcpy( this->szDefaultSharedMemoryPlugin, (*pSharedMemPluginList)[0]->getName() );
  }

}

/* \brief add a new user to a shared memory user list
   \param pszPlugin  name of the shared memory plugin to be used
   \param pszMemName   memory name
   \param pszUserName  user name to add
   \param strError     return error message with it
   \returns 0
*/
int MemoryFactory::addSharedMemoryUser( char *pszPlugin, char *pszMemName, char *pszUserName,std::string &strError)
{
  strError = "";

  OtmSharedMemoryPlugin *plugin = getSharedMemoryPlugin( pszPlugin );

  if( pszMemName==NULL || pszUserName==NULL || plugin == NULL )
  {
    strError = "Invalidate object name";
    return  MemoryFactory::ERROR_INVALIDOBJNAME;
  }

  int iRC = 0;

  iRC = plugin->addMemoryUser( pszMemName,pszUserName);
  if(iRC != 0)
    iRC = plugin->getLastError(strError);

  return iRC;
}

/* \brief delete a user from a shared memory user list
   \param pszPlugin  name of the shared memory plugin to be used
   \param pszMemName   memory name
   \param pszUserName  user name to add
   \param strError     return error message with it
   \returns 0
*/
int MemoryFactory::removeSharedMemoryUser( char *pszPlugin, char *pszMemName, char *pszUserName,std::string &strError)
{
  strError = "";

  OtmSharedMemoryPlugin *plugin = getSharedMemoryPlugin( pszPlugin );

  if(pszMemName==NULL || pszUserName==NULL || plugin==NULL )
  {
    strError = "Invalidate object name";
    return  MemoryFactory::ERROR_INVALIDOBJNAME;
  }

  int iRC = 0;
  iRC = plugin->removeMemoryUser( pszMemName,pszUserName);
  if(iRC != 0)
    iRC = plugin->getLastError(strError);

  return iRC;
}

/* \brief list shared memory users
   \param pszPlugin  name of the shared memory plugin to be used
   \param pszMemName   memory name
   \param users        users name returned
   \param strError     return error message with it
   \returns 0
*/
int MemoryFactory::listSharedMemoryUsers(
    char *pszPlugin,
    char *pszMemName,
    std::vector<std::string> &users,
    std::string &strError
    )
{
  strError = "";

  OtmSharedMemoryPlugin *plugin = getSharedMemoryPlugin( pszPlugin );

  if(pszMemName==NULL || plugin==NULL )
  {
    strError = "Invalidate object name";
    return  MemoryFactory::ERROR_INVALIDOBJNAME;
  }

  int iRC = plugin->listMemoryUsers(pszMemName,users);
  if(iRC != 0)
    iRC = plugin->getLastError(strError);

  return iRC;
}

/* \brief Get a pointer to the shared memory plugin with the give name
   \param pszPlugin plugin name
   \returns pointer to the plugin
*/
OtmSharedMemoryPlugin *MemoryFactory::getSharedMemoryPlugin( char *pszPlugin )
{
  if ( (pszPlugin == NULL) || (*pszPlugin == '\0') ) pszPlugin = this->szDefaultSharedMemoryPlugin;


  for( int i = 0; i < (int)this->pSharedMemPluginList->size(); i++ )
  {
    if ( strcmp( (*this->pSharedMemPluginList)[i]->getName(), pszPlugin ) == 0 )
    {
      return( (*this->pSharedMemPluginList)[i] );
    }
  }
  return( NULL );
}

/* \brief add a new memory information to memory list
   \param pszName memory name, format as "pluginName:memoryName"
   \param chToDrive drive letter
   \returns 0 if success
*/
int MemoryFactory::addMemoryToList(PSZ pszName, CHAR chToDrive)
{
    if(pszName==NULL)
        return -1;

    char* pCol = strchr(pszName,':');
    if(pCol == NULL)
        return -1;

    char* pMemName = pCol+1;
    char temp = *pCol;
    *pCol = EOS;
    char* pPlgName = pszName;

    OtmPlugin* pPlugin = findPlugin(pPlgName,pMemName);
    *pCol = temp;
    if(pPlugin == NULL)
       return -1;
    if(pPlugin->getType() ==  OtmPlugin::eTranslationMemoryType)
    {
        ((OtmMemoryPlugin *)pPlugin)->addMemoryToList(pMemName,chToDrive);
    }
    else if(pPlugin->getType() == OtmPlugin::eSharedTranslationMemoryType)
    {
        ((OtmSharedMemoryPlugin *)pPlugin)->addMemoryToList(pMemName,chToDrive);
    }
    
    return 0;
}

/* \brief remove a memory information from memory list
   \param  pszName format as "pluginName:memoryName"
   \returns 0 if success
*/
int MemoryFactory::removeMemoryFromList(PSZ pszName)
{
    if(pszName==NULL)
        return -1;

     char* pCol = strchr(pszName,':');
    if(pCol == NULL)
        return -1;

    char* pMemName = pCol+1;
    char temp = *pCol;
    *pCol = EOS;
    char* pPlgName = pszName;

    OtmPlugin* pPlugin = findPlugin(pPlgName,pMemName);
    *pCol = temp;
    if(pPlugin == NULL)
       return -1;

    if(pPlugin->getType() ==  OtmPlugin::eTranslationMemoryType)
    {
        ((OtmMemoryPlugin *)pPlugin)->removeMemoryFromList(pMemName);
    }
    else if(pPlugin->getType() == OtmPlugin::eSharedTranslationMemoryType)
    {
        ((OtmSharedMemoryPlugin *)pPlugin)->removeMemoryFromList(pMemName);
    }
    
    return 0;
}

/*! \brief Replace a memory with the data from another memory
  This method bevaves like deleting the replace memory and renaming the
  replaceWith memory to the name of the replace memory without the overhead of the
  actual delete and rename operations
  \param pszPluginName name of plugin of the memory
  \param pszReplace name of the memory being replaced
  \param pszReplaceWith name of the memory replacing the pszReplace memory
	returns 0 if successful or error return code
*/
int MemoryFactory::replaceMemory
(
  char *pszPluginName,
  char *pszReplace,
  char *pszReplaceWith
)
{
  int iRC = OtmMemoryPlugin::eSuccess;

  OtmPlugin *plugin = this->findPlugin( pszPluginName, pszReplace );

  if ( plugin != NULL )
  {
    // use the given plugin to replace the memory, when not supported try the delete-and-rename approach
    if ( plugin->getType() == OtmMemoryPlugin::eTranslationMemoryType )
    {
      iRC = ((OtmMemoryPlugin *)plugin)->replaceMemory( pszReplace, pszReplaceWith );
      if ( iRC == OtmMemoryPlugin::eNotSupported )
      {
        iRC = ((OtmMemoryPlugin *)plugin)->deleteMemory( pszReplace );
        if ( iRC == 0 ) iRC = ((OtmMemoryPlugin *)plugin)->renameMemory( pszReplaceWith, pszReplace );
      }
      if ( iRC != 0 ) ((OtmMemoryPlugin *)plugin)->getLastError(this->strLastError);
    }
    else if ( plugin->getType() == OtmMemoryPlugin::eSharedTranslationMemoryType )
    {
      iRC = ((OtmSharedMemoryPlugin *)plugin)->replaceMemory( pszReplace, pszReplaceWith );
      if ( iRC == OtmMemoryPlugin::eNotSupported )
      {
        iRC = ((OtmSharedMemoryPlugin *)plugin)->deleteMemory( pszReplace );
        if ( iRC == 0 ) iRC = ((OtmSharedMemoryPlugin *)plugin)->renameMemory( pszReplaceWith, pszReplace );
      }
      if ( iRC != 0 ) ((OtmSharedMemoryPlugin *)plugin)->getLastError(this->strLastError);
    }

    // broadcast deleted memory name for replaceWith memory
    if ( iRC == OtmMemoryPlugin::eSuccess )
    {
      strcpy( this->szMemObjName, plugin->getName() );
      strcat( this->szMemObjName,  ":" ); 
		  strcat( this->szMemObjName, pszReplaceWith );
      EqfSend2AllHandlers( WM_EQFN_DELETED, MP1FROMSHORT(clsMEMORYDB), MP2FROMP(this->szMemObjName) );
	  }
  }
  else
  {
    iRC = OtmMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}

