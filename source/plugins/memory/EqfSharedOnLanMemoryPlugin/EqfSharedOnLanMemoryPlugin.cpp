/*! \file
	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#include "string"
#include "vector"
#include "core\PluginManager\PluginManager.h"
#include "EqfSharedOnLanMemoryPlugin.h"
#include "EqfSharedOnLanMemory.h"
#include "eqftmi.h"
#include <TlHelp32.h>
#include <Shellapi.h>

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

// the static plugin infos
static char *pszPluginName = "EqfSharedOnLanMemoryPlugin";
static char *pszShortDescription = "SharedMemoryPlugin using LAN drives";
static char *pszLongDescription	= "Translation-Memory plugin sharing Translation Memory databases using LAN drives";
static char *pszVersion = "1.0";
static char *pszSupplier = "International Business Machines Corporation";

EqfSharedOnLanMemoryPlugin::EqfSharedOnLanMemoryPlugin()
{
    
	  name = pszPluginName;
	  shortDesc = pszShortDescription;
	  longDesc = pszLongDescription;
	  version = pszVersion;
	  supplier = pszSupplier;
    descrType   = "Shared Translation Memory (MS Win LAN based)";

    iLastError  = 0;
    pluginType  = OtmPlugin::eSharedTranslationMemoryType;
    usableState = OtmPlugin::eUsable;

    UtlGetLANDriveList( (PBYTE)szSupportedDrives );

    this->refreshMemoryList();
}


EqfSharedOnLanMemoryPlugin::~EqfSharedOnLanMemoryPlugin()
{
}

const char* EqfSharedOnLanMemoryPlugin::getName()
{
	return name.c_str();
}

const char* EqfSharedOnLanMemoryPlugin::getShortDescription()
{
	return shortDesc.c_str();
}

const char* EqfSharedOnLanMemoryPlugin::getLongDescription()
{
	return longDesc.c_str();
}

const char* EqfSharedOnLanMemoryPlugin::getVersion()
{
	return version.c_str();
}

const char* EqfSharedOnLanMemoryPlugin::getSupplier()
{
	return supplier.c_str();
}

const char* EqfSharedOnLanMemoryPlugin::getDescriptiveMemType()
{
	return descrType.c_str();
}

/*! \brief Returns list of supported drive letters. 
	The list of drives is used to allow the selection of a drive letter in the memory create indow
	The drive letters are retunred as a null-terminated string containing the letters of the 
  supported drives. When no drive selection should be possible, an empty string should be returned.\n
	\param pszDriveListBuffer points to the callers buffer for the list of drive letter characters
	\returns 0 if successful or error return code */
int EqfSharedOnLanMemoryPlugin::getListOfSupportedDrives( char *pszDriveListBuffer )
{
  refreshDriveList();

  strcpy( pszDriveListBuffer, szSupportedDrives );
  return( 0 );
}

/*! \brief Check if the memory with the given name is owned by this plugin
\param pszName name of the memory
\returns TRUE if memory is owned by this plugin and FALSE if not */
BOOL EqfSharedOnLanMemoryPlugin::isMemoryOwnedByPlugin(
	PSZ pszName
)
{
  // build memory path
  std::string strMemPath;
  this->makeMemoryPath( pszName, '\0', strMemPath );

  // build property file name
  std::string strPropName;
  this->makePropName( strMemPath, strPropName );

  // check if there is one of our property files for this shared memory
  return( UtlFileExist( (char *)strPropName.c_str() ) );
}

/*! \brief Tell if this shared memory plugin is using a local memory to keep a copy of the shared memory proposals
  \param pszName name of the memory
	\returns TRUE when a local memory is used by this plugin, FALSE is no local memory is used
*/
	BOOL EqfSharedOnLanMemoryPlugin::isLocalMemoryUsed()
  {
    // this shared memory plugin does not use a local memory for the memory proposals 
    return( FALSE );
  }


/*! \brief Create a new translation memory
  \param pszName name of the new memory
	\param pszSourceLang source language
	\param pszDescription description of the memory
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
  \param chDrive drive where new memory should be created, or 0 if memory should be created on primary drive
	\returns Pointer to created translation memory or NULL in case of errors
*/
OtmMemory* EqfSharedOnLanMemoryPlugin::createMemory(
	PSZ pszName,			  
	PSZ pszSourceLang,
	PSZ pszDescription,
  CHAR chDrive,
  std::vector<std::string> *pvOptions,
  OtmMemory *pLocalMemory
)
{
  EqfSharedOnLanMemory *pNewMemory = NULL;        // new memory object
  HTM htm = NULL;                      // memory handle 
  std::string strMemPath;
  pvOptions; pLocalMemory;

  // get ID of current user
  char szUserID[40];
  USHORT usUserPriv = 0;
  UtlGetLANUserID( szUserID, &usUserPriv, FALSE );

  // create otm\mem directory on target drive if it does not exist yet
  char szPathName[MAX_LONGFILESPEC];
  UtlMakeEQFPath( szPathName, chDrive, MEM_PATH, NULL );
  UtlMkMultDir( szPathName, FALSE );

  // get a new short name and check if the memory exists already
  char szLanDrives[27];
  char szShortName[MAX_FILESPEC];
  BOOL fReserved = FALSE;
  UtlGetLANDriveList( (BYTE *)szLanDrives );
  int iExists = getShortName( pszName, szLanDrives, szShortName, TRUE, &fReserved );

  if ( iExists == 1 ) // there is already a memory with the given name
  {
    handleError( ERROR_MEMORY_EXISTS, pszName, NULL, szPathName, this->strLastError, this->iLastError );
    return( NULL );
  }
  else if ( iExists == 2 ) // there is a not-connect shared memory having the same name
  {
    handleError( ERROR_SHAREDMEM_EXISTS, pszName, NULL, szLanDrives, this->strLastError, this->iLastError );
    return( NULL );
  }
  else if ( iExists == -1 ) // an error occured, most likely a memory allocation error
  {
    handleError( ERROR_STORAGE, pszName, NULL, szPathName, this->strLastError, this->iLastError );
    return( NULL );
  }

  // build memory path
  this->buildMemoryPath( szShortName, chDrive, strMemPath );

  // use old memory create code
  USHORT usRC = TmCreate(  (PSZ)strMemPath.c_str(), &htm,  NULL, "",  szUserID,  pszSourceLang,  pszDescription,  0, NULLHANDLE );
  if ( usRC != 0 )
  {
    handleError( (int)usRC, pszName, NULL, szPathName, this->strLastError, this->iLastError );

    // delete reserved short name
    if ( fReserved )
    {
      char szRempropFile[MAX_LONGFILESPEC];
      strcpy( szRempropFile, strMemPath.c_str() );
      PSZ pszExt = strrchr( szRempropFile, DOT );
      strcpy( pszExt, EXT_OF_SHARED_MEMPROP );
      UtlDelete( szRempropFile, 0L, FALSE );
    }
  }

  // setup memory properties
  this->createMemoryProperties( pszName, strMemPath, pszDescription, pszSourceLang, szUserID );
  
  // create memory object if create function completed successfully
  pNewMemory = new EqfSharedOnLanMemory( this, htm, pszName );

  // add memory info to our internal memory list
  this->addToList( strMemPath );
  
  return( (OtmMemory *)pNewMemory );
}
	
/*! \brief Open an existing shared translation memory
  \param pszName name of the new memory
	\param pLocalMemory local version of shared memory being created
  \param usAccessMode, special access mode for memory: FOR_ORGANIZE, EXCLUSIVE, READONLY
	\returns Pointer to translation memory or NULL in case of errors
*/
OtmMemory* EqfSharedOnLanMemoryPlugin::openMemory(
	PSZ pszName,			  
  OtmMemory *pLocalMemory,
  USHORT usAccessMode
)
{
  OtmMemory *pMemory = NULL;
  HTM htm = NULL;                      // memory handle 
  std::string strMemPath;

  pLocalMemory; // unused

  // find memory in our list
  OtmMemoryPlugin::PMEMORYINFO pInfo = this->findMemory( pszName );
  if ( pInfo != NULL )
  {
    // use old memory open code
    USHORT usRC = TmOpen(  pInfo->szFullPath, &htm, usAccessMode, 0, 0,  NULLHANDLE );

    // create memory object if create function completed successfully
    if ( (usRC == 0) || ((usRC == BTREE_CORRUPTED) && (usAccessMode == FOR_ORGANIZE)) )
    {
      pMemory = new EqfSharedOnLanMemory( this, htm, pszName );
    }
    else
    {
      handleError( (int)usRC, pszName, NULL, pInfo->szFullPath, this->strLastError, this->iLastError );
      if ( htm != 0 ) TmClose( htm, NULL,  FALSE,  NULL );
    } /* end */       
  } /* end */     
  else
  {
    // no memory found
    handleError( ERROR_MEMORY_NOTFOUND, pszName, NULL, pInfo->szFullPath, this->strLastError, this->iLastError );
  }

  return( pMemory );
}

/*! \brief Close a memory
  \param pMemory pointer to memory object
*/
int EqfSharedOnLanMemoryPlugin::closeMemory(
	OtmMemory *pMemory			 
)
{
  int iRC = 0;

  if ( pMemory == NULL ) return( -1 );

  EqfSharedOnLanMemory *pMem = (EqfSharedOnLanMemory *)pMemory;
  HTM htm = pMem->getHTM();

	iRC = TmClose( htm, NULL,  FALSE,  NULL );


  // refresh memory info
  std::string strMemName;
  pMem->getName( strMemName );
  OtmMemoryPlugin::PMEMORYINFO pMemInfo = this->findMemory( (char *)strMemName.c_str() );
  if ( pMemInfo != NULL )
  {
    std::string strPropName;
    std::string strMemPath = pMemInfo->szFullPath;
    this->makePropName( strMemPath, strPropName );
    this->fillInfoStructure( UtlGetFnameFromPath( (char *)strPropName.c_str() ), pMemInfo );
  }

  delete( pMemory );  

  return( iRC );
}


/*! \brief Provide a list of all available memories
  \param pfnCallBack callback function to be called for each memory
	\param pvData caller's data pointetr, is passed to callback function
	\param fWithDetails TRUE = supply memory details, when this flag is set, 
  the pInfo parameter of the callback function is set otherwise it is NULL
	\returns number of provided memories
*/
int EqfSharedOnLanMemoryPlugin::listMemories(
	OtmMemoryPlugin::PFN_LISTMEMORY_CALLBACK pfnCallBack,			  
	void *pvData,
	BOOL fWithDetails
)
{
  for ( std::size_t i = 0; i < m_MemInfoVector.size(); i++ )
  {
    OtmMemoryPlugin::PMEMORYINFO pInfo = m_MemInfoVector[i];

    pfnCallBack( pvData, pInfo->szName, fWithDetails ? pInfo : NULL );
  } /* end */     
  return( m_MemInfoVector.size() );
}

/*! \brief Get information about a memory
  \param pszName name of the memory
  \param pInfo pointer to buffer for memory information
	\returns 0 if successful or error return code
*/
int EqfSharedOnLanMemoryPlugin::getMemoryInfo(
	PSZ pszName,
  OtmMemoryPlugin::PMEMORYINFO pInfo
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
  OtmMemoryPlugin::PMEMORYINFO pMemInfo = this->findMemory( pszName );
  if ( pMemInfo != NULL )
  {
    memcpy( pInfo, pMemInfo, sizeof(OtmMemoryPlugin::MEMORYINFO) );
  }
  else
  {
    iRC = OtmMemoryPlugin::eMemoryNotFound;
    memset( pInfo, 0, sizeof(OtmMemoryPlugin::MEMORYINFO) );
  } /* endif */
  return( iRC );
}

///*! \brief set description of a memory
//  \param pszName name of the memory
//  \param pszDesc description information
//	\returns 0 if successful or error return code
//*/
int EqfSharedOnLanMemoryPlugin::setDescription(PSZ pszName, PSZ pszDesc)
{
    if(pszName==NULL || pszDesc==NULL)
        return 0;

    OtmMemoryPlugin::PMEMORYINFO pMemInfo = this->findMemory( pszName );
    if ( pMemInfo == NULL )
    {
      // no memory found
      handleError( ERROR_MEMORY_NOTFOUND, pszName, NULL, NULL, this->strLastError, this->iLastError );
      return( ERROR_MEMORY_NOTFOUND );
    }

    // firstly change in memory
    int tLen = sizeof(pMemInfo->szDescription)/sizeof(pMemInfo->szDescription[0])-1;
    strncpy( pMemInfo->szDescription, pszDesc, tLen );
    pMemInfo->szDescription[tLen] = '\0';

    // then change in disk
    char szPathMem[512];
    
    UtlMakeEQFPath( szPathMem, NULC, PROPERTY_PATH, NULL );
    strcat( szPathMem, "\\" );
    Utlstrccpy( szPathMem+strlen(szPathMem), UtlGetFnameFromPath( pMemInfo->szFullPath ), DOT );
    strcat( szPathMem, LANSHARED_MEM_PROP );

    ULONG ulRead;
    PPROP_NTM pProp = NULL;
    BOOL fOK = UtlLoadFileL( szPathMem, (PVOID*)&pProp, &ulRead, FALSE, FALSE );

    if(!fOK || pProp == NULL)
        return -1;

    int length = sizeof(pProp->stTMSignature.szDescription)/sizeof(pProp->stTMSignature.szDescription[0])-1;
    strncpy(pProp->stTMSignature.szDescription, pszDesc, length);
    pProp->stTMSignature.szDescription[length]='\0';

    int res = UtlWriteFileL( szPathMem, ulRead, (PVOID)pProp, FALSE );

    // relase memory
    UtlAlloc( (PVOID *)&pProp, 0, 0, NOMSG );

    return res;
}

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
int EqfSharedOnLanMemoryPlugin::getMemoryFiles
(
  PSZ pszName,
  char *pFileListBuffer,
  int  iBufferSize
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
  *pFileListBuffer = '\0';

  OtmMemoryPlugin::PMEMORYINFO pMemInfo = this->findMemory( pszName );

  if ( pMemInfo != NULL )
  {
    // we need room for two full path names (data + index) plus room for the separator
    int iRequiredLength = (2 * strlen(pMemInfo->szFullPath)) + 2 + 1;

    // get memory property file name
    std::string strPropName;
    std::string strMemPath = pMemInfo->szFullPath;
    this->makePropName( strMemPath, strPropName ); 

    // now add the length of the property file name
    iRequiredLength += strPropName.length() + 1;

    if ( iRequiredLength <= iBufferSize )
    {
      // add property name to buffer
      strcat( pFileListBuffer, strPropName.c_str() ); 
      strcat( pFileListBuffer, "," ); 

      // add data file name to buffer
      strcat( pFileListBuffer, pMemInfo->szFullPath ); 
      strcat( pFileListBuffer, "," ); 

      // setup index file name
      char szIndexPath[MAX_LONGFILESPEC];
      this->makeIndexFileName( pMemInfo->szFullPath, szIndexPath );

      // add index file name to buffer
      strcat( pFileListBuffer, szIndexPath ); 

    }
    else
    {
      iRC = OtmMemoryPlugin::eBufferTooSmall;
    } /* endif */
  }
  else
  {
    iRC = OtmMemoryPlugin::eMemoryNotFound;
  } /* endif */
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

   \param pszMemoryName    name of the memory 
   \param pFileList        pointer to a buffer containing the fully qualified memory data files as a comma separated list
   \param iOptions         processing options, one or more of the IMPORTFROMMEMFILES_..._OPT values ORed together
                           
   \param ppPrivateData    the address of a PVOID pointer which can be used to anchor private data. The
                           PVPOID pointer will be set to NULL on the initial call

  	\returns 0 if OK,
             OtmMemoryPlugin::eRepeat when the import needs more processing steps
             any other value is an error code
*/
int EqfSharedOnLanMemoryPlugin::importFromMemoryFiles
(
  char *pszMemoryName,
  char *pFileList,
  int  iOptions,
  PVOID *ppPrivateData
)
{
  int iRC = OtmMemoryPlugin::eSuccess;

  // check the type of method call
  if ( *ppPrivateData == NULL )
  {
    // this is an inital call to import a memory using its data files
    iRC = this->importFromMemFilesInitialize( pszMemoryName, pFileList, iOptions, ppPrivateData );
  }
  else
  {
    // this is a continuation call
    iRC = this->importFromMemFilesContinueProcessing( ppPrivateData );
  } /* endif */     

  return( iRC );
}



/*! \brief Physically rename a translation memory
  \param pszOldName name of the memory being rename
  \param pszNewName new name for the memory
	\returns 0 if successful or error return code
*/
int EqfSharedOnLanMemoryPlugin::renameMemory(
	PSZ pszOldName,
  PSZ pszNewName
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
  OtmMemoryPlugin::PMEMORYINFO pMemInfo = this->findMemory( pszOldName );
  if ( pMemInfo != NULL )
  {
    // get new short name for memory
    BOOL fIsNew = FALSE;
    char szShortName[MAX_FILESPEC];

    ObjLongToShortName( pszNewName, szShortName, TM_OBJECT, &fIsNew );
    if ( !fIsNew ) return( OtmMemory::ERROR_MEMORYEXISTS );

    // get memory property file name
    std::string strPropName;
    std::string strMemPath = pMemInfo->szFullPath;
    this->makePropName( strMemPath, strPropName ); 

    // rename index file
    char szIndexPath[MAX_LONGFILESPEC];
    char szNewPath[MAX_LONGFILESPEC];
    strcpy( szIndexPath, pMemInfo->szFullPath );
    char *pszExt = strrchr( szIndexPath, DOT );
    strcpy( szNewPath, pMemInfo->szFullPath );
    UtlSplitFnameFromPath( szNewPath );
    strcat( szNewPath, "\\" );
    strcat( szNewPath, szShortName );
    if ( strcmp( pszExt, EXT_OF_TMDATA ) == 0 )
    {
      strcpy( strrchr( szIndexPath, DOT ), EXT_OF_TMINDEX );
      strcat( szNewPath, EXT_OF_TMINDEX );
    }
    else
    {
      strcpy( strrchr( szIndexPath, DOT ), EXT_OF_SHARED_MEMINDEX );
      strcat( szNewPath, EXT_OF_SHARED_MEMINDEX );
    } /* endif */
    rename( szIndexPath, szNewPath ) ;

    // rename data file
    strcpy( szNewPath, pMemInfo->szFullPath );
    UtlSplitFnameFromPath( szNewPath );
    strcat( szNewPath, "\\" );
    strcat( szNewPath, szShortName );
    strcat( szNewPath, EXT_OF_SHARED_MEM );
    rename( pMemInfo->szFullPath, szNewPath ) ;

    // adjust data file name in memory info area
    strcpy( pMemInfo->szFullPath, szNewPath ) ;

    // rename the property file
    strcpy( szNewPath, strPropName.c_str() ); 
    UtlSplitFnameFromPath( szNewPath );
    strcat( szNewPath, "\\" );
    strcat( szNewPath, szShortName );
    strcat( szNewPath, LANSHARED_MEM_PROP  );
    rename( strPropName.c_str(), szNewPath ) ;

    // update property file
    PPROP_NTM pstMemProp = NULL;
    ULONG ulRead = 0;
    if ( UtlLoadFileL( szNewPath, (PVOID *)&pstMemProp, &ulRead, FALSE, FALSE ) )
    {
      // adjust name in property header
      strcpy( pstMemProp->stPropHead.szName, szShortName );
      strcat( pstMemProp->stPropHead.szName, LANSHARED_MEM_PROP );

      // adjust long name
      if ( strcmp( pszNewName, szShortName ) != 0 )
      {
        strcpy( pstMemProp->szLongName, pszNewName );
      }
      else
      {
        pstMemProp->szLongName[0] = EOS;
      } /* endif */

      // adjust fully qualified TM name
      strcpy( pstMemProp->szFullMemName, pMemInfo->szFullPath );

      // update name in signature structure
      strcpy( pstMemProp->stTMSignature.szName, szShortName );

      // re-write property file
      UtlWriteFileL( szNewPath, ulRead, pstMemProp, FALSE );


      // delete old shared property file
      strcpy( szNewPath, strMemPath.c_str() );
      pszExt = strrchr( szNewPath, DOT );
      strcpy( pszExt, EXT_OF_SHARED_MEMPROP );
      UtlDelete( szNewPath, 0L, FALSE );

      // re-write new shared property file 
      strcpy( szNewPath, strMemPath.c_str() );
      UtlSplitFnameFromPath( szNewPath );
      strcat( szNewPath, "\\" );
      strcat( szNewPath, szShortName );
      strcat( szNewPath, EXT_OF_SHARED_MEMPROP );
      UtlWriteFileL( szNewPath, ulRead, pstMemProp, FALSE );

      // delete the update counter files
      strcpy( szNewPath, strMemPath.c_str() );
      pszExt = strrchr( szNewPath, DOT );
      strcpy( pszExt, ".R-I" );
      UtlDelete( szNewPath, 0L, FALSE );
      strcpy( pszExt, ".R-D" );
      UtlDelete( szNewPath, 0L, FALSE );

      // free property area
      UtlAlloc( (PVOID *)&pstMemProp, 0, 0, NOMSG );
    } /* endif */

    // update memory name in info data
    strcpy( pMemInfo->szName, pszNewName );
  }
  else
  {
    iRC = OtmMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}

/*! \brief Physically delete a translation memory
  \param pszName name of the memory being deleted
	\returns 0 if successful or error return code
*/
int EqfSharedOnLanMemoryPlugin::deleteMemory(
	PSZ pszName			  
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
  OtmMemoryPlugin::PMEMORYINFO pMemInfo = NULL;

  // get the memory info index int the vector and use it to get memory info
  int idx = this->findMemoryIndex(pszName);
  if( idx>=0 && idx<((int)m_MemInfoVector.size()) )
      pMemInfo = m_MemInfoVector[idx];

  if ( (pMemInfo != NULL) && (pMemInfo->szFullPath[0] != EOS) )
  {
    // delete data and index file
    UtlDelete( pMemInfo->szFullPath, 0L, FALSE );

    char szIndexPath[MAX_LONGFILESPEC];
    strcpy( szIndexPath, pMemInfo->szFullPath );
    char *pszExt = strrchr( szIndexPath, DOT );
    if ( strcmp( pszExt, EXT_OF_TMDATA ) == 0 )
    {
      strcpy( strrchr( szIndexPath, DOT ), EXT_OF_TMINDEX );
    }
    else
    {
      strcpy( strrchr( szIndexPath, DOT ), EXT_OF_SHARED_MEMINDEX );
    } /* endif */
    UtlDelete( szIndexPath, 0L, FALSE );

    // delete the property file
    std::string strPropName;
    std::string strMemPath = pMemInfo->szFullPath;
    this->makePropName( strMemPath, strPropName ); 
    UtlDelete( (char *)strPropName.c_str(), 0L, FALSE );

    // delete the shared property file
    strPropName = pMemInfo->szFullPath;
    strPropName.erase( strPropName.end()-4, strPropName.end() );
    strPropName.append( EXT_OF_SHARED_MEMPROP );
    UtlDelete( (char *)strPropName.c_str(), 0L, FALSE );

    // delete the update counter files
    strPropName.erase( strPropName.end()-4, strPropName.end() );
    strPropName.append( ".R-I" );
    UtlDelete( (char *)strPropName.c_str(), 0L, FALSE );
    strPropName.erase( strPropName.end()-4, strPropName.end() );
    strPropName.append( ".R-D" );
    UtlDelete( (char *)strPropName.c_str(), 0L, FALSE );

    // remove memory info from our memory info vector
    delete( pMemInfo );
    m_MemInfoVector.erase(m_MemInfoVector.begin( )+idx);
  }
  else
  {
    iRC = OtmMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}

/*! \brief Clear (i.e. remove all entries) a translation memory
  \param pszName name of the memory being cleared
	\returns 0 if successful or error return code
*/
int EqfSharedOnLanMemoryPlugin::clearMemory(
	PSZ pszName			  
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
  OtmMemoryPlugin::PMEMORYINFO pMemInfo = this->findMemory( pszName );
  if ( pMemInfo != NULL )
  {
    // delete data and index file
    UtlDelete( pMemInfo->szFullPath, 0L, FALSE );

    char szIndexPath[MAX_LONGFILESPEC];
    strcpy( szIndexPath, pMemInfo->szFullPath );
    char *pszExt = strrchr( szIndexPath, DOT );
    if ( strcmp( pszExt, EXT_OF_TMDATA ) == 0 )
    {
      strcpy( strrchr( szIndexPath, DOT ), EXT_OF_TMINDEX );
    }
    else
    {
      strcpy( strrchr( szIndexPath, DOT ), EXT_OF_SHARED_MEMINDEX );
    } /* endif */
    UtlDelete( szIndexPath, 0L, FALSE );

    // use TmtXCreate to create new data and index file
    PTMX_CREATE_IN pTmCreateIn = new (TMX_CREATE_IN);
    memset( pTmCreateIn, 0, sizeof(TMX_CREATE_IN) );

    strcpy( pTmCreateIn->stTmCreate.szDataName, pMemInfo->szFullPath );
    strcpy( pTmCreateIn->stTmCreate.szSourceLanguage, pMemInfo->szSourceLanguage );
    strcpy( pTmCreateIn->stTmCreate.szSourceLanguage, pMemInfo->szSourceLanguage );
    strcpy( pTmCreateIn->stTmCreate.szDescription, pMemInfo->szDescription );

    PTMX_CREATE_OUT pTmCreateOut = new (TMX_CREATE_OUT);
    memset( pTmCreateOut, 0, sizeof(TMX_CREATE_OUT) );

    iRC = (int)TmtXCreate( pTmCreateIn, pTmCreateOut );

    free( pTmCreateIn );
    free( pTmCreateOut );
  }
  else
  {
    iRC = OtmMemoryPlugin::eMemoryNotFound;
  } /* endif */
  return( iRC );
}

#ifdef CODEFOROTMMEMORYPLUGINONLY
/*! \brief Create a temporary memory
  \param pszPrefix prefix to be used for name of the temporary memory
  \param pszName buffer for the name of the temporary memory
	\param pszSourceLang source language
	\param bMsgHandling true/false: display errors or not
	\param hwnd owner-window needed for modal error-message
	\returns Pointer to created translation memory or NULL in case of errors
*/
OtmMemory* EqfSharedOnLanMemoryPlugin::createTempMemory(
	PSZ pszPrefix,			  
	PSZ pszName,			  
	PSZ pszSourceLang,
	BOOL bMsgHandling,
	HWND hwnd
)
{
  EqfMemory *pNewMemory = NULL;        // new memory object
  HTM htm = NULL;                      // memory handle 
  USHORT usRC = 0;

  bMsgHandling;

  // use old temporary memory create code
  usRC = TMCreateTempMem( pszPrefix, pszName, &htm, NULL, pszSourceLang, hwnd );
  
  // create memory object if create function completed successfully
  if ( usRC == 0 )
  {
    pNewMemory = new EqfMemory( this, htm, pszName );
  } /* end */     

  return( (OtmMemory *)pNewMemory );
}
#endif

#ifdef CODEFOROTMMEMORYPLUGINONLY
/*! \brief close and delete a temporary memory
  \param pMemory pointer to memory objject
*/
void EqfSharedOnLanMemoryPlugin::closeTempMemory(
	OtmMemory *pMemory
)
{
  std::string strName;

  // get the memory name
  pMemory->getName( strName );

  // close the memory
  this->closeMemory( pMemory );

  // use old temporary memory delete code
  TMDeleteTempMem( (PSZ)strName.c_str() );
  
  return;
}
#endif

/*! \brief Get the error message for the last error occured

    \param strError reference to a string receiving the error mesage text
  	\returns last error code
*/
int EqfSharedOnLanMemoryPlugin::getLastError
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
int EqfSharedOnLanMemoryPlugin::getLastError
(
  char *pszError,
  int iBufSize
)
{
  strncpy( pszError, this->strLastError.c_str(), iBufSize );
  return( this->iLastError );
}

/* private methods */

/*! \brief Refresh the internal list of translation memory dbs
*/
void EqfSharedOnLanMemoryPlugin::refreshMemoryList()
{
  FILEFINDBUF     ResultBuf;           // DOS file find struct
  USHORT          usCount;
  USHORT          usRC;                // return value of Utl/Dos calls
  HDIR            hDir = HDIR_CREATE;  // DosFind routine handle
  PSZ             pszName = RESBUFNAME(ResultBuf);

  // free any existing memory info data
  for( int i = 0; i < (int)m_MemInfoVector.size(); i++ )
  {
    delete( m_MemInfoVector[i] );
  }
  m_MemInfoVector.clear();

  UtlMakeEQFPath( this->szBuffer, NULC, PROPERTY_PATH, NULL );
  sprintf( this->szBuffer + strlen(szBuffer), "%c%s%s", BACKSLASH, DEFAULT_PATTERN_NAME, LANSHARED_MEM_PROP );

  // loop over all memory property files
  usCount = 1;
  usRC = UtlFindFirst( this->szBuffer, &hDir, 0, &ResultBuf, sizeof(ResultBuf), &usCount, 0L, 0 );
  usCount = ( usRC ) ? 0 : usCount;
  while( usCount)
  {
    this->addToList( pszName );

    usCount = 1;
    usRC = UtlFindNext( hDir, &ResultBuf, sizeof(ResultBuf), &usCount, FALSE );
    usCount = ( usRC ) ? 0 : usCount;
  } /* endwhile */

  // close file search handle
  if ( hDir != HDIR_CREATE ) UtlFindClose( hDir, FALSE );

  return;
} /* end of method RefreshMemoryList */

/*! \brief Fill memory info structure from memory properties
  \param pszPropName name of the memory property file (w/o path) 
	\param pInfo pointer to memory info structure
	\returns TRUE when successful, FALSE in case of errors
*/
BOOL EqfSharedOnLanMemoryPlugin::fillInfoStructure
(
   char *pszPropName,
   OtmMemoryPlugin::PMEMORYINFO pInfo
)
{
  if(pInfo==0 || pszPropName==0)
      return FALSE;

  BOOL fOK = TRUE;
  char szFullPropName[MAX_LONGFILESPEC];
  PPROP_NTM pProp = NULL;
  USHORT usLen = 0;

  memset( pInfo, 0, sizeof(OtmMemoryPlugin::MEMORYINFO) );

  // init it, if not meet some condition ,it will be set to false
  pInfo->fEnabled = TRUE;

  UtlMakeEQFPath( szFullPropName, NULC, PROPERTY_PATH, NULL );
  strcat( szFullPropName, BACKSLASH_STR );
  strcat( szFullPropName, pszPropName );
  fOK = UtlLoadFile( szFullPropName, (PVOID *)&pProp, &usLen, FALSE, FALSE );
  if ( fOK )
  {
    if ( usLen >= sizeof(PROP_NTM) && (pProp->szFullMemName[0] != EOS))
    {
      FILESTATUS      StatusBuf;           // DOS file find struct
      USHORT          usRC;                // return value of Utl/Dos calls

      /* get size of memory data file */
      usRC = UtlQPathInfo(  pProp->szFullMemName, 1, (PBYTE)&StatusBuf, sizeof(StatusBuf), 0L, FALSE );
      if ( usRC == 0 )
      {
        pInfo->ulSize = StatusBuf.cbFile;
      }
      else
      {
        pInfo->ulSize = 0;
        pInfo->fEnabled = FALSE;
      } /* end */             

      // add size of memory index
      strcpy( szFullPropName, pProp->szFullMemName );
      char *pszExt = strrchr( szFullPropName, DOT );
      if ( pszExt != NULL ) strcpy( pszExt, (strcmp( pszExt, EXT_OF_SHARED_MEM ) == 0 ) ? EXT_OF_SHARED_MEMINDEX : EXT_OF_TMINDEX );
      usRC = UtlQPathInfo( szFullPropName, 1, (PBYTE)&StatusBuf, sizeof(StatusBuf), 0L, FALSE );
      if ( usRC == 0 )
      {
        pInfo->ulSize += StatusBuf.cbFile;
      }
      else
      {
        pInfo->ulSize = 0;
        pInfo->fEnabled = FALSE;
      } /* endif */

      strcpy( pInfo->szName, pProp->szLongName );
      if ( pInfo->szName[0] == EOS )
      {
        Utlstrccpy( pInfo->szName, pProp->stTMSignature.szName, DOT );
      } /* endif */
      strcpy( pInfo->szDescription, pProp->stTMSignature.szDescription );
      strcpy( pInfo->szSourceLanguage, pProp->stTMSignature.szSourceLanguage );
      strcpy( pInfo->szFullPath, pProp->szFullMemName );
      strcpy( pInfo->szPlugin, this->name.c_str() );
      strcpy( pInfo->szDescrMemoryType, this->descrType.c_str() );
      strcpy( pInfo->szOwner, pProp->stTMSignature.szUserid );
    }
    else
    {
      fOK = FALSE;

      // automatically delete defective property files
      UtlDelete( szFullPropName, 0, FALSE );

    } /* end */         
    UtlAlloc( (PVOID *)&pProp, 0, 0, NOMSG );
  } /* endif */
  return( fOK );
}

/*! \brief Find memory in our memory list and return pointer to memory info 
  \param pszName name of the memory  
	\returns PMEMORYINFO pInfo pointer to memory info or NULL in case of errors
*/
OtmMemoryPlugin::PMEMORYINFO EqfSharedOnLanMemoryPlugin::findMemory
(
   char *pszName
)
{
  int idx = findMemoryIndex(pszName);
  if( idx>=0 && idx<((int)m_MemInfoVector.size()) )
  {
      return( m_MemInfoVector[idx] );
  }
  return NULL;
}

// return the memory index of the specified memory name
int EqfSharedOnLanMemoryPlugin::findMemoryIndex
(
   char *pszName
)
{
  for ( int i = 0; i < (int)m_MemInfoVector.size(); i++ )
  {
      OtmMemoryPlugin::PMEMORYINFO pInfo = m_MemInfoVector[i];
      if ( strcmpi( pszName, pInfo->szName) == 0 )
      {
          return i;
      }
  }
  
  return (-1);
}


bool EqfSharedOnLanMemoryPlugin::stopPlugin( bool fForce  )
{

  // TODO: check for active objects..
  bool fActiveObjects = false;

  // decline stop if we have active objects
  if ( !fForce && fActiveObjects )
  {
    return( false );
  }

  // TODO: terminate active objects, cleanup, free allocated resources

  // de-register plugin
	PluginManager *pPluginManager = PluginManager::getInstance();
	pPluginManager->deregisterPlugin( (OtmPlugin *)this );

  return( true );
}

extern "C" {
__declspec(dllexport)
USHORT registerPlugins()
{
	PluginManager::eRegRc eRc = PluginManager::eSuccess;
	PluginManager *manager = PluginManager::getInstance();
	EqfSharedOnLanMemoryPlugin* plugin = new EqfSharedOnLanMemoryPlugin();
	eRc = manager->registerPlugin((OtmPlugin*) plugin);
    USHORT usRC = (USHORT) eRc;
    return usRC;
}
}

/*! \brief Build the path name for a memory
  \param pszName (long) name of the new memory
	\param string receiving the memory path name
	\returns TRUE when successful 
*/
BOOL EqfSharedOnLanMemoryPlugin::makeMemoryPath( PSZ pszName, CHAR chDrive, std::string &strPathName, PSZ pszLanDrives, BOOL fReserveName, PBOOL pfReserved )
{
  char szShortName[MAX_FILESPEC];

  // build short name
  int iExist = getShortName( pszName, pszLanDrives, szShortName, fReserveName, pfReserved );

  return( buildMemoryPath( szShortName, chDrive, strPathName ) );
}

/*! \brief Construct the path name for a memory from a short name
  \param pszShortName short name of the memory
  \param chDrive target drive for the memory
	\param string receiving the memory path name
	\returns TRUE when successful 
*/
BOOL EqfSharedOnLanMemoryPlugin::buildMemoryPath( PSZ pszShortName, CHAR chDrive, std::string &strPathName )
{
  char szPathName[MAX_LONGFILESPEC];

  UtlMakeEQFPath( szPathName, chDrive, MEM_PATH, NULL );
  strcat( szPathName, "\\" );
  strcat( szPathName, pszShortName );
  strcat( szPathName, EXT_OF_SHARED_MEM  );

  strPathName.assign( szPathName );

  return( TRUE );
}



/*! \brief Create memory properties
  \param pszName long name of the memory
  \param strPathName memory path name
	\param pszDescription memory description
	\param pszSourceLanguage memory source language
	\param pszOwner owner of the memory
	\returns TRUE when successful, FALSE in case of errors
*/
BOOL EqfSharedOnLanMemoryPlugin::createMemoryProperties( PSZ pszName, std::string &strPathName, PSZ pszDescription, PSZ pszSourceLanguage, PSZ pszOwner )
{
  BOOL fOK = TRUE;
  PPROP_NTM pProp = NULL;
  USHORT usPropSize = max( sizeof(PROP_NTM), MEM_PROP_SIZE );

  fOK = UtlAlloc( (void **)&pProp, 0, usPropSize, NOMSG );

  // init memory
  memset((void*)pProp,0,usPropSize);
  if ( fOK )
  {
    // fill properties file
    pProp->stPropHead.usClass = PROP_CLASS_MEMORY;
    pProp->stPropHead.chType = PROP_TYPE_NEW;
    pProp->usLocation = TM_SHARED;
    strncpy( pProp->stPropHead.szPath, strPathName.c_str(), 
             sizeof(pProp->stPropHead.szPath)/sizeof(pProp->stPropHead.szPath[0]));
    Utlstrccpy( pProp->stPropHead.szName, UtlSplitFnameFromPath( pProp->stPropHead.szPath ), DOT );
    strcat( pProp->stPropHead.szName, LANSHARED_MEM_PROP );
    UtlSplitFnameFromPath( pProp->stPropHead.szPath );

    //in case of overflow. change these strcpy to strncpy
    strncpy( pProp->szFullMemName, strPathName.c_str(), sizeof(pProp->szFullMemName)/sizeof(pProp->szFullMemName[0])-1);
    strncpy( pProp->szLongName, pszName, sizeof(pProp->szLongName)/sizeof(pProp->szLongName[0])-1);
    strncpy( pProp->stTMSignature.szDescription, 
             pszDescription, 
             sizeof(pProp->stTMSignature.szDescription)/sizeof(pProp->stTMSignature.szDescription[0])-1);
    strncpy( pProp->stTMSignature.szSourceLanguage, pszSourceLanguage, 
             sizeof(pProp->stTMSignature.szSourceLanguage)/sizeof(pProp->stTMSignature.szSourceLanguage[0])-1);

    strcpy( pProp->stTMSignature.szUserid, pszOwner );
    pProp->stTMSignature.bMajorVersion = TM_MAJ_VERSION;
    pProp->stTMSignature.bMinorVersion = TM_MIN_VERSION;
    strcpy( pProp->szNTMMarker, NTM_MARKER );
    UtlTime( &pProp->stTMSignature.lTime );
    pProp->usThreshold = TM_DEFAULT_THRESHOLD;

    // write properties to disk
    std::string strPropName;
    this->makePropName( strPathName, strPropName );
    fOK = UtlWriteFile( (char *)strPropName.c_str() , usPropSize, (PVOID)pProp, FALSE );

    // create shared copy of property file
    strPropName = pProp->szFullMemName;
    strPropName.erase( strPropName.end()-4, strPropName.end() );
    strPropName.append( EXT_OF_SHARED_MEMPROP );
    UtlWriteFile( (char *)strPropName.c_str() , usPropSize, (PVOID)pProp, FALSE );

    // cleanup
    UtlAlloc( (void **)&pProp, 0, 0, NOMSG );

  } /* endif */     
  return( fOK );
}

/*! \brief Create memory properties
  \param pszName long name of the memory
  \param strPathName memory path name
	\param pvOldProperties existing property file to be used for the fields of the new properties
	\returns TRUE when successful, FALSE in case of errors
*/
BOOL EqfSharedOnLanMemoryPlugin::createMemoryProperties( PSZ pszName, std::string &strPathName, void *pvOldProperties )
{
  BOOL fOK = TRUE;
  PPROP_NTM pProp = NULL;
  PPROP_NTM pOldProp = (PPROP_NTM)pvOldProperties;
  USHORT usPropSize = max( sizeof(PROP_NTM), MEM_PROP_SIZE );

  fOK = UtlAlloc( (void **)&pProp, 0, usPropSize, NOMSG );

  // init memory
  memset((void*)pProp,0,usPropSize);
  if ( fOK )
  {
    // fill properties file
    pProp->stPropHead.usClass = PROP_CLASS_MEMORY;
    pProp->stPropHead.chType = PROP_TYPE_NEW;
    strncpy( pProp->stPropHead.szPath, strPathName.c_str(), 
             sizeof(pProp->stPropHead.szPath)/sizeof(pProp->stPropHead.szPath[0]));
    Utlstrccpy( pProp->stPropHead.szName, UtlSplitFnameFromPath( pProp->stPropHead.szPath ), DOT );
    strcat( pProp->stPropHead.szName, LANSHARED_MEM_PROP );
    UtlSplitFnameFromPath( pProp->stPropHead.szPath );

    //in case of overflow. change these strcpy to strncpy
    strncpy( pProp->szFullMemName, strPathName.c_str(), sizeof(pProp->szFullMemName)/sizeof(pProp->szFullMemName[0])-1);
    strncpy( pProp->szLongName, pszName, sizeof(pProp->szLongName)/sizeof(pProp->szLongName[0])-1);
    strncpy( pProp->stTMSignature.szDescription, pOldProp->stTMSignature.szDescription, 
             sizeof(pProp->stTMSignature.szDescription)/sizeof(pProp->stTMSignature.szDescription[0])-1);
    strncpy( pProp->stTMSignature.szSourceLanguage, pOldProp->stTMSignature.szSourceLanguage, 
             sizeof(pProp->stTMSignature.szSourceLanguage)/sizeof(pProp->stTMSignature.szSourceLanguage[0])-1);

    strcpy( pProp->stTMSignature.szUserid, pOldProp->stTMSignature.szUserid );
    pProp->stTMSignature.bMajorVersion = pOldProp->stTMSignature.bMajorVersion;
    pProp->stTMSignature.bMinorVersion = pOldProp->stTMSignature.bMinorVersion;
    strcpy( pProp->szNTMMarker, NTM_MARKER );
    pProp->stTMSignature.lTime = pOldProp->stTMSignature.lTime;
    pProp->usThreshold = pOldProp->usThreshold ;

    // write properties to disk
    std::string strPropName;
    this->makePropName( strPathName, strPropName );
    fOK = UtlWriteFile( (char *)strPropName.c_str() , usPropSize, (PVOID)pProp, FALSE );
    UtlAlloc( (void **)&pProp, 0, 0, NOMSG );
  } /* endif */     
  return( fOK );
}



/*! \brief Add memory to our internal memory lisst
  \param strPathName reference to the memory path name
	\returns 0 when successful
*/
int EqfSharedOnLanMemoryPlugin::addToList( std::string &strPathName )
{
  std::string strPropName;

  this->makePropName( strPathName, strPropName );

  return( this->addToList( (PSZ)strPropName.c_str() ) );
}

/*! \brief Add memory to our internal memory lisst
  \param pszPropname pointer to property file name
	\returns 0 when successful
*/
int EqfSharedOnLanMemoryPlugin::addToList( char *pszPropName )
{
  OtmMemoryPlugin::PMEMORYINFO pInfo = new( OtmMemoryPlugin::MEMORYINFO );
  if(pInfo != 0)
  {
    // find the property name when pszPropName is fully qualified
    PSZ pszName = strrchr( pszPropName, '\\' );
    pszName = (pszName == NULL) ? pszPropName : pszName + 1;
    if ( this->fillInfoStructure( pszName, pInfo ) )
    {
      m_MemInfoVector.push_back( pInfo );
    }
    else
    {
      delete( pInfo );
    }
  }
  return( 0 );
}

/*! \brief make Index filename from memory data file name
  \param pszMemPath pointer to memory data file name
  \param pszIndexFileName pointer to a buffer for the memory index file name
	\returns 0 when successful
*/
int EqfSharedOnLanMemoryPlugin::makeIndexFileName( char *pszMemPath, char *pszIndexFileName )
{
  char *pszExt = NULL;
  strcpy( pszIndexFileName, pszMemPath );
  pszExt = strrchr( pszIndexFileName, DOT );
  if ( strcmp( pszExt, EXT_OF_TMDATA ) == 0 )
  {
    strcpy( pszExt, EXT_OF_TMINDEX );
  }
  else
  {
    strcpy( pszExt, EXT_OF_SHARED_MEMINDEX );
  } /* endif */
  return( 0 );
}

/*! \brief make Index filename from memory data file name
  \param strMemPath reference to a string containing the memory data file name
  \param strIndexFileName reference to a string receiving the memory index file name
	\returns 0 when successful
*/
int EqfSharedOnLanMemoryPlugin::makeIndexFileName( std::string &strMemPath, std::string &strIndexFileName )
{
  size_t ExtPos = strMemPath.find_last_of( '.' );
  strIndexFileName =strMemPath.substr( 0, ExtPos );
  if ( strMemPath.compare( ExtPos, std::string::npos, EXT_OF_TMDATA ) == 0 )
  {
    strIndexFileName.append( EXT_OF_TMINDEX );
  }
  else
  {
    strIndexFileName.append( EXT_OF_SHARED_MEMINDEX );
  } /* endif */
  return( 0 );
}

/*! \brief Private data area for the memory import from data files */
typedef struct _MEMIMPORTFROMFILEDATA
{
  EqfSharedOnLanMemory *pInputMemory;             // input memory
  EqfSharedOnLanMemory *pOutputMemory;            // output memory
  OtmProposal  Proposal;               // currently processed proposal
  std::string strPropFile;             // property file of the imorted memory
  std::string strDataFile;             // data file of the imorted memory
  std::string strIndexFile;            // index file of the imorted memory
  char  szBuffer[1014];                // general purpose buffer

} MEMIMPORTFROMFILEDATA, *PMEMIMPORTFROMFILEDATA;

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
int EqfSharedOnLanMemoryPlugin::importFromMemFilesInitialize
(
  char *pszMemoryName,
  char *pFileList,
  int  iOptions,
  PVOID *ppPrivateData
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
  OtmMemoryPlugin::PMEMORYINFO pMemInfo = this->findMemory( pszMemoryName );
  std::string strPropFile = "";
  std::string strDataFile = "";
  std::string strIndexFile = "";

  iOptions;

  // extract names of the imported files from the file list
  {
     // loop over imported files and extract them
     std::string strFiles = pFileList;
     size_t Pos = 0;
     BOOL fComplete = FALSE;
     while ( !fComplete )
     {
       // extract next file name from list
       std::string strCurFile;
       size_t End = strFiles.find_first_of( ',', Pos );
       if ( End == std::string::npos )
       {
         strCurFile = strFiles.substr( Pos, std::string::npos );
         fComplete = TRUE;
       }
       else
       {
         strCurFile = strFiles.substr( Pos, End - Pos );
         Pos = End + 1;
       } /* endif */     

       // due to a bug in previous versions the files in the list may use the EQF directory instead of 
       // the OTM directory, so correct this when necessary
       size_t EQFPos = strCurFile.find( "\\EQF\\" );
       if ( EQFPos != std::string::npos )
       {
         strCurFile.replace( EQFPos, 5, "\\OTM\\" );
       }

       // get position of file extention in current file
       size_t ExtPos = strCurFile.find_last_of( '.' );

       // process current file name
       if ( strCurFile.compare( ExtPos, std::string::npos, LANSHARED_MEM_PROP ) == 0 )
       {
          strPropFile = strCurFile;   
       }
       else if ( strCurFile.compare( ExtPos, std::string::npos, EXT_OF_SHARED_MEM ) == 0 )
       {
          strDataFile = strCurFile;   
       }
       else if ( strCurFile.compare( ExtPos, std::string::npos, EXT_OF_SHARED_MEMINDEX ) == 0 )
       {
          strIndexFile = strCurFile;   
       }
       else
       {
         // delete unknown file type
         UtlDelete( (PSZ)strCurFile.c_str(), 0L, FALSE );
       } /* endif */
     } /* endwhile */        
  }


  if ( pMemInfo != NULL )
  {
    PMEMIMPORTFROMFILEDATA pData = NULL;

    // this memory exists already and we have to merge the data from the memory data files into
    // the existing memory, so prepare the merge of the memory

    // allocate and anchor our memory merge data structure
    pData = new( MEMIMPORTFROMFILEDATA );
    if ( pData == NULL ) return( OtmMemoryPlugin::eNotEnoughMemory );
    *ppPrivateData = (PVOID)pData;

    // store memory file names in private data area
    pData->strPropFile = strPropFile;
    pData->strDataFile = strDataFile;
    pData->strIndexFile = strIndexFile;

    // open the target memory
    pData->pOutputMemory = (EqfSharedOnLanMemory *)this->openMemory( pszMemoryName, NULL, NONEXCLUSIVE );
    if ( pData->pOutputMemory == NULL )
    {
      return( this->iLastError );
    } /* end */       

    // open the data file of the imported memory file
    // use old memory open code
    HTM htm = 0;
    USHORT usRC = TmOpen( (PSZ)strDataFile.c_str(), &htm,  EXCLUSIVE, 0, FALSE,  NULLHANDLE );
    if ( usRC != 0 )
    {
      handleError( (int)usRC, (char *)strDataFile.c_str(), NULL, (char *)strDataFile.c_str(), this->strLastError, this->iLastError );
      return( this->iLastError );      
    } /* end */   

    pData->pInputMemory = new EqfSharedOnLanMemory( this, htm, pszMemoryName  );

    // get first proposal and copy it to output memory
    int iMemRC = pData->pInputMemory->getFirstProposal( pData->Proposal );
    if ( iMemRC == NO_ERROR)
    {
      pData->pOutputMemory->putProposal( pData->Proposal );
      iRC = OtmMemoryPlugin::eRepeat;
    }
    else if ( iMemRC == BTREE_EOF_REACHED )
    {
      // nothing more to do
      this->importFromMemFilesEndProcessing( ppPrivateData );
    }
  }
  else
  {
    // this is a new memory, so we can move the files to the approbriate location and create a new property file for it
     std::string strMemPath;

     // build memory path for the imported memory
     this->makeMemoryPath( pszMemoryName, '\0', strMemPath );

      // process property file
      {
        PPROP_NTM pProp = NULL;
        USHORT usLen = 0;
        UtlLoadFile( (PSZ)strPropFile.c_str(), (PVOID *)&pProp, &usLen, FALSE, FALSE );
        if ( pProp != NULL )
        {
          // setup new memory properties based on imported memory property file
          this->createMemoryProperties( pszMemoryName, strMemPath, (void *)pProp );

          // delete imported property file
          UtlDelete( (PSZ)strPropFile.c_str(), 0L, FALSE );

          // free memory of loaded property file
          UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );
        } /* end */             
       }

       // process data file
       {
         UtlMove( (PSZ)strDataFile.c_str(), (PSZ)strMemPath.c_str(), 0L, FALSE );
       }

       // process index file
       {
         std::string strNewIndexFile;
         this->makeIndexFileName( strMemPath, strNewIndexFile );
         UtlMove( (PSZ)strIndexFile.c_str(), (PSZ)strNewIndexFile.c_str(), 0L, FALSE );
       } /* endif */

       // add memory to our memory list
       this->addToList( strMemPath );
  } /* endif */
  return( iRC );
}

/*! \brief Continue the import of a memory using a list of memory data files


   \param ppPrivateData    the address of a PVOID pointer which can be used to anchor private data. The
                           PVPOID pointer will be set to NULL on the initial call

  	\returns 0 if OK,
             OtmMemoryPlugin::eRepeat when the import needs more processing steps
             any other value is an error code
*/
int EqfSharedOnLanMemoryPlugin::importFromMemFilesContinueProcessing
(
  PVOID *ppPrivateData
)
{
  int iRC = OtmMemoryPlugin::eSuccess;
  int iMemRC = NO_ERROR;
  PMEMIMPORTFROMFILEDATA pData = (PMEMIMPORTFROMFILEDATA)*ppPrivateData;

  for ( int i = 1; i < 10; i++ )
  {
    iMemRC = pData->pInputMemory->getNextProposal( pData->Proposal );
    if ( iMemRC == NO_ERROR )
    {
      pData->pOutputMemory->putProposal( pData->Proposal );
    }
    else
    {
      break;
    }
  } /* endfor */     

  if ( iMemRC == NO_ERROR)
  {
    iRC = OtmMemoryPlugin::eRepeat;
  }
  else if ( iMemRC == OtmMemory::INFO_ENDREACHED )
  {
    this->importFromMemFilesEndProcessing( ppPrivateData );
    iRC = OtmMemoryPlugin::eSuccess;
  }
  else
  {
    this->importFromMemFilesEndProcessing( ppPrivateData );
    iRC = iMemRC;
  }
  return( iRC );
}

  /*! \brief Terminate the import of a memory using a list of memory data files and do a cleanup


   \param ppPrivateData    the address of a PVOID pointer which can be used to anchor private data. The
                           PVPOID pointer will be set to NULL on the initial call

  	\returns 0 if OK,
             any other value is an error code
 */
int EqfSharedOnLanMemoryPlugin::importFromMemFilesEndProcessing
(
  PVOID *ppPrivateData
)
{
  int iRC = OtmMemoryPlugin::eSuccess;

  PMEMIMPORTFROMFILEDATA pData = (PMEMIMPORTFROMFILEDATA)*ppPrivateData;

  if ( pData != NULL )
  {
    // close memories
    if ( pData->pOutputMemory != NULL ) this->closeMemory( pData->pOutputMemory );

    if ( pData->pInputMemory != NULL ) 
    {
      HTM htm = pData->pInputMemory->getHTM();
      TmClose( htm, NULL,  FALSE,  NULL );

      delete( pData->pInputMemory );  
    } /* end */       

    // delete memory data files
    if ( pData->strPropFile.length() != 0 ) UtlDelete( (PSZ)pData->strPropFile.c_str(), 0L, FALSE );
    if ( pData->strDataFile.length() != 0 ) UtlDelete( (PSZ)pData->strDataFile.c_str(), 0L, FALSE );
    if ( pData->strIndexFile.length() != 0 ) UtlDelete( (PSZ)pData->strIndexFile.c_str(), 0L, FALSE );

    // free data area
    free( pData );
  } /* end */     

  return( iRC );
}

/*! \brief Handle a return code from the memory functions and create the approbriate error message text for it
    \param iRC return code from memory function
    \param pszMemName long memory name
    \param pszMarkup markup table name or NULL if not available
    \param pszMemPath fully qualified memory path name or NULL if not available
    \param strLastError reference to string object receiving the message text
    \param iLastError reference to a integer variable receiving the error code
  	\returns original or modified error return code
*/
int EqfSharedOnLanMemoryPlugin::handleError( int iRC, char *pszMemName, char *pszMarkup, char *pszMemPath, std::string &strLastError, int &iLastError )
{
  PSZ          pReplAddr[2];

  if ( iRC == 0 ) return( iRC );

  char *pszErrorTextBuffer = (char *)malloc(8096);
  if ( pszErrorTextBuffer == NULL ) return( iRC );

  pReplAddr[0] = pszMemName;

  switch ( iRC )
  {
   case ERROR_TA_ACC_TAGTABLE:
     pReplAddr[0] = (pszMarkup != NULL) ? pszMarkup : "n/a";
     UtlGetMsgTxt( ERROR_TA_ACC_TAGTABLE, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case ERROR_SHARING_VIOLATION:
   case BTREE_DICT_LOCKED:
   case BTREE_ENTRY_LOCKED:
   case BTREE_ACCESS_ERROR:
     UtlGetMsgTxt( ERROR_MEM_NOT_ACCESSIBLE, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TMERR_NO_MORE_MEMORY_AVAILABLE:
   case ERROR_NOT_ENOUGH_MEMORY:
   case BTREE_NO_ROOM:
   case BTREE_NO_BUFFER:
     UtlGetMsgTxt( ERROR_STORAGE, pszErrorTextBuffer, 0, NULL );
     break;

   case VERSION_MISMATCH:
   case CORRUPT_VERSION_MISMATCH:
   case ERROR_VERSION_NOT_SUPPORTED:
     UtlGetMsgTxt( ERROR_MEM_VERSION_NOT_SUPPORTED, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case FILE_MIGHT_BE_CORRUPTED:
   case BTREE_CORRUPTED:
   case BTREE_USERDATA:
   case ERROR_OLD_PROPERTY_FILE:
     UtlGetMsgTxt( ITM_TM_NEEDS_ORGANIZE, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case BTREE_DISK_FULL:
   case BTREE_WRITE_ERROR   :
   case DISK_FULL:
     {
       char szDrive[2];
       if ( pszMemPath != NULL )
       {
         szDrive[0] = *pszMemPath;
         szDrive[1] = '\0';
       }
       pReplAddr[0] = ( pszMemPath != NULL ) ? szDrive : "n/a";
       UtlGetMsgTxt( ERROR_DISK_FULL_MSG, pszErrorTextBuffer, 1, pReplAddr );
     }
     break;

   case DB_FULL:
   case BTREE_LOOKUPTABLE_TOO_SMALL:
     UtlGetMsgTxt( ERROR_MEM_DB_FULL, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case FILE_ALREADY_EXISTS:
     UtlGetMsgTxt( ERROR_MEM_NAME_INVALID, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case NOT_REPLACED_OLD_SEGMENT:
     UtlGetMsgTxt( ERROR_MEM_NOT_REPLACED, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TM_FILE_SCREWED_UP:
   case NOT_A_MEMORY_DATABASE:
   case BTREE_ILLEGAL_FILE:
     UtlGetMsgTxt( ERROR_MEM_DESTROYED, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TM_FILE_NOT_FOUND:
   case BTREE_FILE_NOTFOUND:
     UtlGetMsgTxt( ERROR_TM_FILE_NOT_FOUND, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TMERR_TM_OPENED_EXCLUSIVELY:
     UtlGetMsgTxt( ERROR_TM_OPENED_EXCLUSIVELY, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TMERR_PROP_EXIST:
     UtlGetMsgTxt( ERROR_PROP_EXIST, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TMERR_PROP_WRITE_ERROR:
     UtlGetMsgTxt( ERROR_PROP_WRITE, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TM_PROPERTIES_NOT_OPENED:
     UtlGetMsgTxt( ERROR_OPEN_TM_PROPERTIES, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case ERROR_NETWORK_ACCESS_DENIED :
   case BTREE_NETWORK_ACCESS_DENIED:
     UtlGetMsgTxt( ERROR_ACCESS_DENIED_MSG, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case SEGMENT_BUFFER_FULL:
   case BTREE_BUFFER_SMALL:
     UtlGetMsgTxt( ERROR_MEM_SEGMENT_TOO_LARGE, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case ERROR_INTERNAL :                                      
     UtlGetMsgTxt( ERROR_INTERNAL, pszErrorTextBuffer, 0, NULL );
     break;                                                   

   case ERROR_INTERNAL_PARM:
     UtlGetMsgTxt( ERROR_INTERNAL_PARM, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case  BTREE_IN_USE:
     UtlGetMsgTxt( ERROR_MEM_NOT_ACCESSIBLE, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case ERROR_SHAREDMEM_EXISTS: // parm 1 = memory, parm 2 = drive character)
     {
       char szDrive[2];
       if ( pszMemPath != NULL )
       {
         szDrive[0] = *pszMemPath;
         szDrive[1] = '\0';
       }
       pReplAddr[0] = pszMemName;
       pReplAddr[1] = szDrive;

       UtlGetMsgTxt( ERROR_SHAREDMEM_EXISTS, pszErrorTextBuffer, 2, pReplAddr );
     }
     break;

   case ERROR_MEMORY_EXISTS: // parm 1 = memory
     UtlGetMsgTxt( ERROR_MEMORY_EXISTS, pszErrorTextBuffer, 1, pReplAddr );
     break;

   case TMERR_TM_OPENED_SHARED:
   case ERROR_PATH_NOT_FOUND:
   case ERROR_INVALID_DRIVE:
   case BTREE_INVALID_DRIVE :
   default:
     {
       char szError[20];
       sprintf( szError, "%ld", iRC );
       pReplAddr[1] = szError;
       UtlGetMsgTxt( ERROR_MEM_UNDEFINED, pszErrorTextBuffer, 2, pReplAddr );
     }
 }

 strLastError = pszErrorTextBuffer;
 iLastError = iRC;
 free( pszErrorTextBuffer );

 return( iRC );
}


/* \brief add a new memory information to memory list
   \param pszName memory name
   \param chToDrive drive letter
   \returns 0 if success
*/
int EqfSharedOnLanMemoryPlugin::addMemoryToList(PSZ pszName, CHAR chDrive)
{
    if(pszName==NULL)
        return -1;
     
   // build memory path
    char szShortName[MAX_FILESPEC];
    char szPathName[MAX_LONGFILESPEC];
    BOOL fIsNew = FALSE;

    // already exist, just return
    if( findMemory(pszName) != NULL)
        return -1;

    ObjLongToShortName( pszName, szShortName, TM_OBJECT, &fIsNew );
    // only could be added when its property exists
    if(!fIsNew)
    {
        UtlMakeEQFPath( szPathName, chDrive, MEM_PATH, NULL );
        strcat( szPathName, "\\" );
        strcat( szPathName, szShortName );
        strcat( szPathName, EXT_OF_SHARED_MEM  );
        std::string strPath = szPathName;
        addToList( strPath );
    }

    return 0;
}

/* \brief remove a memory information from memory list
   \param  pszName memory name
   \returns 0 if success
*/
int EqfSharedOnLanMemoryPlugin::removeMemoryFromList(PSZ pszName)
{
    if(pszName==NULL)
        return -1;

     int idx = findMemoryIndex(pszName);
     if(idx == -1)
         return -1;
    // don't exist, just return
    m_MemInfoVector.erase(m_MemInfoVector.begin( )+idx);

    return 0;

}

/*! \brief Make the fully qualified property file name for a memory
  \param strPathName reference to the memory path name
  \param strPropName reference to the string receiving the property file name
	\returns 0 when successful
*/
int EqfSharedOnLanMemoryPlugin::makePropName( std::string &strPathName, std::string &strPropName )
{
  char szFullPropName[MAX_LONGFILESPEC];

  UtlMakeEQFPath( szFullPropName, NULC, PROPERTY_PATH, NULL );
  strcat( szFullPropName, BACKSLASH_STR );
  Utlstrccpy( szFullPropName + strlen(szFullPropName), UtlGetFnameFromPath( (char *)strPathName.c_str() ), DOT );
  strcat( szFullPropName, LANSHARED_MEM_PROP );
  strPropName = szFullPropName;
  return( 0 );
}

/*! \brief Provide the names of shared memories
	\param pvOptions pointer to a vector containing the access options
  \param pvConnected pointer to a vector receiving the names of the connected memories
  \param pvNotConnected pointer to a vector receiving the names of the not connected memories
	\returns number of provided memories
*/
int EqfSharedOnLanMemoryPlugin::listSharedMemories(
  std::vector<std::string> *pvOptions,
  std::vector<std::string> *pvConnected,
  std::vector<std::string> *pvNotConnected
)
{
  pvOptions; pvConnected; pvNotConnected;

  refreshDriveList();

  m_RemoteLongToShortNameList.clear();

  char *pszDrive = this->szSupportedDrives;        // start with first drive

  while ( *pszDrive != NULC )       // while not end of drive list ...
  {
    WIN32_FIND_DATA FindData;
    std::string strMemory;

    // Setup search path and scan current drive for remote resources                                                   */
    UtlMakeEQFPath( szBuffer, *pszDrive, MEM_PATH, NULL );
    strcat( szBuffer, BACKSLASH_STR );
    strcat( szBuffer, "*" );
    strcat( szBuffer, EXT_OF_SHARED_MEMPROP );

    HANDLE hFind = FindFirstFile( this->szBuffer, &FindData );
    if ( hFind != INVALID_HANDLE_VALUE )
    {
      do
      {
        // Get any long name from shared property file
        PPROP_NTM pProp = NULL;         // ptr to TM properties
        ULONG ulLen;
        UtlMakeEQFPath( szBuffer, *pszDrive, MEM_PATH, NULL );
        strcat( szBuffer, BACKSLASH_STR );
        strcat( szBuffer, FindData.cFileName  );

        if ( UtlLoadFileL( szBuffer, (PVOID *)&pProp, &ulLen, TRUE, FALSE ) )
        {
          szBuffer[0] = *pszDrive;
          szBuffer[1] = ':';
          szBuffer[2] = ' ';
          if ( pProp->szLongName[0] != EOS )
          {
            strcpy( szBuffer + 3, pProp->szLongName );
            OEMTOANSI( szBuffer );
          }
          else
          {
            Utlstrccpy( szBuffer + 3, FindData.cFileName, '.' );
          } /* endif */
          UtlAlloc( (PVOID *)&pProp, 0L, 0L, NOMSG );

          // add memory to approbriate list
          strMemory = szBuffer;
          if ( findMemoryIndex( szBuffer + 3 ) >= 0 )
          {
            pvConnected->push_back( strMemory );
          }
          else
          {
            pvNotConnected->push_back( strMemory );
          }
        } /* endif */

        // add memory name and asscoiated property file name to our remote resources list
        std::string strNewEntry = szBuffer;
        strNewEntry.append( ">" );
        strNewEntry.append( FindData.cFileName ) ;
        m_RemoteLongToShortNameList.push_back( strNewEntry );

      } while ( FindNextFile( hFind, &FindData ) );
      FindClose( hFind );
    }
    pszDrive++;                     // continue with next drive
  } /* endwhile */

  return( 0 );
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
  \param plHandle pointer to a handle which is passed to the callback function
	\returns 0 if successful or error return code
*/
int EqfSharedOnLanMemoryPlugin::getCreateOptionFields(
	PSZ pszName,
  std::vector<std::string> **ppLabels,
  std::vector<std::string> **ppFieldData,
  std::vector<std::string> **ppFieldDescr,
  PFN_OPTIONSDIALOGCHECKDATA *ppfnCheckCallback,
  long *plHandle
)
{
	pszName; ppLabels; ppFieldData; ppFieldDescr; ppfnCheckCallback; plHandle;

  // this function is not supported 
  return( OtmMemoryPlugin::eNotSupported );
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
int EqfSharedOnLanMemoryPlugin::getAccessOptionFields(
  std::vector<std::string> **ppLabels,
  std::vector<std::string> **ppFieldData,
  std::vector<std::string> **ppFieldDescr,
  PFN_OPTIONSDIALOGCHECKDATA *ppfnCheckCallback,
  long *plHandle
)
{
	ppLabels; ppFieldData; ppFieldDescr; ppfnCheckCallback; plHandle;

  // this function is not supported 
  return( OtmMemoryPlugin::eNotSupported );
}

/*! \brief Connect to an existing shared translation memory
  \param pszName name of the memory being connected
	\param pvOptions pointer to a vector containing the access options
	\returns 0 when successful or error return code
*/
int EqfSharedOnLanMemoryPlugin::connectToMemory(
	PSZ pszName,			  
  std::vector<std::string> *pvOptions
)
{
	pszName; pvOptions;

  int iRC = 0;

  // find memory in our list of remote resource
  int iLen = strlen( pszName );
  char szShortName[MAX_FILESPEC];

  szShortName[0] = '\0';
  for( int i = 0; i < (int)m_RemoteLongToShortNameList.size(); i++ )
  {
    if (  m_RemoteLongToShortNameList[i].compare( 0, iLen, pszName ) == 0 )
    {
      strcpy( szShortName, m_RemoteLongToShortNameList[i].c_str() + iLen + 1 );
      break;
    }
  }

  if ( szShortName[0] != '\0' )
  {
    BOOL fOK = TRUE;                // internal O.K. flag
    PPROP_NTM pProp = NULL;         // ptr to TM properties
    USHORT    usRC;                 // buffer for return codes
    ULONG     ulLen = 0;            // length of properties

    // Setup path of remote property file
    UtlMakeEQFPath( szBuffer, pszName[0], MEM_PATH, NULL );
    strcat( szBuffer, BACKSLASH_STR );
    strcat( szBuffer, szShortName ); 

    // load property file into memory
    fOK = UtlLoadFileL( szBuffer, (PVOID *)&pProp, &ulLen, TRUE, FALSE );
    if ( !fOK )
    {
      handleError( TM_PROPERTIES_NOT_OPENED, pszName, NULL, NULL, this->strLastError, this->iLastError );
    }

    // Correct property header and drive letters for TM paths (drive letters may be assigned differently)
    if ( fOK )
    {
      pProp->szFullMemName[0] = pszName[0];
      UtlMakeEQFPath( pProp->stPropHead.szPath, NULC, SYSTEM_PATH, NULL );
      Utlstrccpy( pProp->stPropHead.szName, szShortName, '.' );
      strcat( pProp->stPropHead.szName, LANSHARED_MEM_PROP );

      // GQ 2015/10/18: ensure that szFullMemName points to the OpenTM2 root directory
      memcpy( pProp->szFullMemName + 3, "OTM", 3 );
    } /* endif */

    // Write local property file
    if ( fOK )
    {
      UtlMakeEQFPath( szBuffer, NULC, PROPERTY_PATH, NULL );
      strcat( szBuffer, BACKSLASH_STR );
      strcat( szBuffer, pProp->stPropHead.szName );

      usRC = UtlWriteFileL( szBuffer, ulLen, (PVOID)pProp, TRUE );
      if ( usRC != NO_ERROR )
      {
        handleError( TMERR_PROP_WRITE_ERROR, pszName, NULL, NULL, this->strLastError, this->iLastError );
        fOK = FALSE;
      }
    } /* endif */

    // add to our internal list
    if ( fOK ) this->addToList( pProp->stPropHead.szName ); 

    // free property file
    if ( pProp != NULL ) UtlAlloc( (PVOID *)&pProp, 0, 0, NOMSG );
  }
  else
  {
    // memory not in the our available memories list ...
    handleError( TM_PROPERTIES_NOT_OPENED, pszName, NULL, NULL, this->strLastError, this->iLastError );
  }

  return( iRC );
}
	
/*! \brief Disconnect a shared translation memory
  \param pszName name of the memory being disconnected
	\returns 0 when successful or error return code
*/
int EqfSharedOnLanMemoryPlugin::disconnectMemory(
	PSZ pszName
)
{
  // find memory in our list of remote resource
  int iLen = strlen( pszName );
  char szShortName[MAX_FILESPEC];

  szShortName[0] = '\0';
  for( int i = 0; i < (int)m_RemoteLongToShortNameList.size(); i++ )
  {
    if (  m_RemoteLongToShortNameList[i].compare( 0, iLen, pszName ) == 0 )
    {
      strcpy( szShortName, m_RemoteLongToShortNameList[i].c_str() + iLen + 1 );
      break;
    }
  }

  if ( szShortName[0] != '\0' )
  {
    // Setup property file name
    UtlMakeEQFPath( szBuffer, NULC, PROPERTY_PATH, NULL );
    strcat( szBuffer, BACKSLASH_STR );
    Utlstrccpy( szBuffer + strlen(szBuffer), szShortName, '.' ) ; 
    strcat( szBuffer, LANSHARED_MEM_PROP );

    // Delete local property file
    UtlDelete( szBuffer, 0L, TRUE );

    // remove memory from our internal list
    int iEntry = findMemoryIndex( pszName + 3 );
    if ( iEntry >= 0 )
    {
      delete( m_MemInfoVector[iEntry] );
      m_MemInfoVector.erase( m_MemInfoVector.begin() + iEntry );
    }
  }

  return( 0 );
}

/* \brief add a new user to a shared memory user list
   \param pszMemName   memory name
   \param pszUserName  user name to add
   \returns 0
*/
int EqfSharedOnLanMemoryPlugin::addMemoryUser(PSZ pszName, PSZ userName)
{
	pszName; userName;

  // this function is not supported 
  return( OtmMemoryPlugin::eNotSupported );
}

/* \brief delete a user from a shared memory user list
   \param pszMemName   memory name
   \param pszUserName  user name to add
   \returns 0
*/
int EqfSharedOnLanMemoryPlugin::removeMemoryUser(PSZ pszName, PSZ userName)
{
	pszName; userName;

  // this function is not supported 
  return( OtmMemoryPlugin::eNotSupported );
}

/* \brief list shared memory users
   \param pszMemName   memory name
   \param users        users name returned
   \returns 0
*/
int EqfSharedOnLanMemoryPlugin::listMemoryUsers(PSZ pszName, std::vector<std::string> &users)
{
	pszName; 
  
  users.clear();

  // this function is not supported 
  return( 0 );
}


/* \brief refresh the list of supported drives and also adjust the enabled state of our memories
   \returns 0
*/
int EqfSharedOnLanMemoryPlugin::refreshDriveList()
{
  UtlGetLANDriveList( (PBYTE)szSupportedDrives );

  // check our memories and disable all residing not on drives contained in the drive list
  char szUpperCaseDrives[27];
  strcpy( szUpperCaseDrives, szSupportedDrives );
  strupr( szUpperCaseDrives );
  for( int i = 0; i < (int)m_MemInfoVector.size(); i++ )
  {
    OtmMemoryPlugin::MEMORYINFO *pInfo = m_MemInfoVector[i];
    
    pInfo->fEnabled = ( strchr( szUpperCaseDrives, _toupper(pInfo->szFullPath[0]) )  != 0 );
  }

  return( 0 );
}

/* \brief set the owner of the given memory
 \param pszMemoryName memory name
 \param  pszOwner new owner for the memory
\returns 0 if success
*/
int EqfSharedOnLanMemoryPlugin::setOwner( PSZ pszMemoryName, PSZ pszOwner)
{
  OtmMemoryPlugin::PMEMORYINFO pInfo = this->findMemory( pszMemoryName );
  if ( pInfo == NULL )
  {
    handleError( ERROR_MEMORY_NOTFOUND, pszMemoryName, NULL, NULL, this->strLastError, this->iLastError );
    return( ERROR_MEMORY_NOTFOUND );
  }

  // update info structure
  strcpy( pInfo->szOwner, pszOwner );

  // get memory property file name
  std::string strPropName;
  std::string strMemPath = pInfo->szFullPath;
  this->makePropName( strMemPath, strPropName ); 

  // update property file
  PPROP_NTM pstMemProp = NULL;
  ULONG ulRead = 0;
  if ( UtlLoadFileL( (PSZ)strPropName.c_str(), (PVOID *)&pstMemProp, &ulRead, FALSE, FALSE ) )
  {
    // update owner in property file
    strcpy( pstMemProp->stTMSignature.szUserid, pszOwner );

    // re-write property file
    UtlWriteFileL( (PSZ)strPropName.c_str(), ulRead, pstMemProp, FALSE );

    // free property area
    UtlAlloc( (PVOID *)&pstMemProp, 0, 0, NOMSG );
  } /* endif */

  return( 0 );
}

 /* \brief Replace the data of one memory with the data of another memory and delete the remains of the second memory
    \param pszReplace name of the memory whose data is being replaced
    \param pszReplaceWith name of the memory whose data will be used to replace the data of the other memory
   \returns 0 if success
*/
int EqfSharedOnLanMemoryPlugin::replaceMemory( PSZ pszReplace, PSZ pszReplaceWith )
{
  OtmMemoryPlugin::PMEMORYINFO pInfoReplace = this->findMemory( pszReplace );
  if ( pszReplace == NULL )
  {
    handleError( ERROR_MEMORY_NOTFOUND, pszReplace, NULL, NULL, this->strLastError, this->iLastError );
    return( ERROR_MEMORY_NOTFOUND );
  }

  OtmMemoryPlugin::PMEMORYINFO pInfoReplaceWith = this->findMemory( pszReplaceWith );
  if ( pszReplaceWith == NULL )
  {
    handleError( ERROR_MEMORY_NOTFOUND, pszReplaceWith, NULL, NULL, this->strLastError, this->iLastError );
    return( ERROR_MEMORY_NOTFOUND );
  }

  // delete and move data file
  UtlDelete( pInfoReplace->szFullPath, 0L, FALSE );
  UtlMove( pInfoReplaceWith->szFullPath, pInfoReplace->szFullPath, 0L, FALSE );

  char szSource[MAX_LONGFILESPEC];
  char szTarget[MAX_LONGFILESPEC];

  // delete and move index file
  this->makeIndexFileName( pInfoReplaceWith->szFullPath, szSource );
  this->makeIndexFileName( pInfoReplace->szFullPath, szTarget );
  UtlDelete( szTarget, 0L, FALSE );
  UtlMove( szSource, szTarget, 0L, FALSE );

  // delete all remaining files using the normal memory delete
  deleteMemory( pszReplaceWith );

  return( 0 );
}


/* \brief Memory long to short name conversion with check for shared resources and reservation of files
 \param pszLongName ptr to long name
 \param pszDrive list of drives to use for shared resource checking (or NULL if not used)
   Attention: when there is a memory with the given name on one of the drives, the list is replaced with this drive letter
 \param pszShortName ptr to buffer for short name
 \param pObjState ptr to buffer for returned object state
 \param fReserveName TRUE = reserve created short name; FALSE = only look for existing memories
 \param pfReserved points to callers fReserved flag
  \returns processing result
    * 0 when no memory with the long name exist
    * 1 when a memory exists
    * 2 when a shared but not connected memory with the given name exists
    * -1 in case of errors
*/
int EqfSharedOnLanMemoryPlugin::getShortName
(
  PSZ         pszLongName,             // ptr to long name
  char        *pszDrive,               // list of drives to use for shared resource checking (or NULL if not used)
  PSZ pszShortName,                    // ptr to buffer for short name
  BOOL        fReserveName,            // TRUE = reserve created short name
  PBOOL       pfReserved               // points to callers fReserved flag
)
{
  // our private data area
  typedef struct _SHORTNAMEDATA
  {
    CHAR      szShortName[MAX_FILESPEC];    // buffer for short name
    CHAR      szFolder[MAX_FILESPEC];       // buffer for folder name
    CHAR      szSearchPath[MAX_EQF_PATH];   // object search path
    CHAR      szFullPath[MAX_EQF_PATH];     // fully qualified object name
    CHAR      szSharedSearchPath[MAX_EQF_PATH];// object search path for shared resources
    CHAR      szSharedFullPath[MAX_EQF_PATH];  // fully qualified object name for shared resources
    FILEFINDBUF stResultBuf;                // DOS file find structure
    CHAR      szExt[20];                    // buffer for object extention
    CHAR      szSharedExt[20];              // buffer for shared object extention
    CHAR      szInLongName[MAX_LONGFILESPEC];// buffer for input long name
    CHAR      szPropLongName[MAX_LONGFILESPEC];// buffer for long name in property file
    CHAR      szPropFileName[MAX_LONGFILESPEC];// buffer for access to property file
  } SHORTNAMEDATA, *PSHORTNAMEDATA;

  // local variables
  PSZ         pszOrgLongName = pszLongName; // original start of long name
  PSHORTNAMEDATA pData = NULL;            // ptr to private data area
  enum { SHORTNAME, LONGNAME, MAYBELONGNAME } NameType = SHORTNAME;
  int iResult = 0;                 // function return code
  SHORT i = 0;                         // number of characters in short name
  ULONG   ulCP = 0L;

  ulCP = GetCodePage( OEM_CP );       // use CP of installed OS

  // ignore any leading blanks
  while ( *pszOrgLongName == ' ' ) pszOrgLongName++;

  // preset callers's variables
  pszShortName[0] = EOS;

  // use our list of shared drives if no drive list has been specified
  if ( (pszDrive == NULL ) || (*pszDrive == '\0') )
  {
    pszDrive = szSupportedDrives;
  }

  // allocate our private data area
  if ( !UtlAlloc( (PVOID *)&pData, 0L, (LONG)sizeof(SHORTNAMEDATA), ERROR_STORAGE ) )
  {
    return( -1 );
  } /* endif */

  // scan long name and add valid characters to short file name
  {
    while ( (i < 5) && (*pszLongName != EOS) )
    {
      CHAR chTest;
      chTest = *pszLongName;
      if ( ((chTest >= '0') && (chTest <= '9')) || // digit or
           ((chTest >= 'a') && (chTest <= 'z')) || // lowercase char or
           ((chTest >= 'A') && (chTest <= 'Z')) )  // uppercase char ?
      {
        CHAR c = *pszLongName++;
        pData->szShortName[i++] = (CHAR)toupper(c);
      }
      else
      {
        NameType = LONGNAME;
        if ( IsDBCSLeadByteEx(ulCP,  *pszLongName ) && (pszLongName[1] != EOS) )
        {
          pszLongName++;                 // skip second byte of DBCS char
        }
        pszLongName++;                   // try next character
      } /* endif */
    } /* endwhile */

    // scan rest of long file name for long object name recognition
    if ( NameType != LONGNAME )        // no non-alphanumeric chars yet?
    {
      int iLen = i;                    // remember current length
      while ( (iLen <= 8) && (NameType != LONGNAME) && (*pszLongName != EOS) )
      {
        CHAR chTest;
        chTest = *pszLongName;
        if ( ((chTest >= '0') && (chTest <= '9')) || // digit or
             ((chTest >= 'a') && (chTest <= 'z')) || // lowercase char or
             ((chTest >= 'A') && (chTest <= 'Z')) )  // uppercase char ?
        {
          pszLongName++;
          iLen++;
        }
        else
        {
          NameType = LONGNAME;
        } /* endif */
      } /* endwhile */

      // due to a bug in a previous version short names with exactly
      // 8 characters were treated as long names - so we have to
      // check for it...
      if ( (iLen == 8) && (NameType != LONGNAME) )
      {
        NameType = MAYBELONGNAME;
      }
      else if ( iLen > 8 )
      {
        NameType = LONGNAME;
      } /* endif */
    } /* endif */

    // complete short name or use long name if it is a short one
    if ( NameType != SHORTNAME )
    {
      // padd short name with A's up to a length of 5 characters
      while ( i < 5 )
      {
        pData->szShortName[i++] = 'A';
      } /* endif */

      // terminate short file name
      pData->szShortName[i] = EOS;
    }
    else
    {
      strcpy( pData->szShortName, pszOrgLongName );
      UtlUpper( pData->szShortName );
    } /* endif */
  } /* endif */


  // setup search path and path of full object name 
  {
    UtlMakeEQFPath( pData->szSearchPath, NULC, PROPERTY_PATH, NULL );
    strcat( pData->szSearchPath, BACKSLASH_STR );
    strcpy( pData->szFullPath, pData->szSearchPath );
    strcat( pData->szSearchPath, pData->szShortName );
    if ( NameType == LONGNAME ) strcat( pData->szSearchPath, DEFAULT_PATTERN_NAME );
    strcpy( pData->szExt, LANSHARED_MEM_PROP );
    strcat( pData->szSearchPath, LANSHARED_MEM_PROP );

    if ( pszDrive != NULL )
    {
      UtlMakeEQFPath( pData->szSharedSearchPath, *pszDrive, MEM_PATH, NULL );
      strcat( pData->szSharedSearchPath, BACKSLASH_STR );
      strcpy( pData->szSharedFullPath, pData->szSharedSearchPath );
      strcat( pData->szSharedSearchPath, pData->szShortName );
      if ( NameType == LONGNAME ) strcat( pData->szSharedSearchPath, DEFAULT_PATTERN_NAME );
      strcpy( pData->szSharedExt, EXT_OF_SHARED_MEMPROP );
      strcat( pData->szSharedSearchPath, pData->szSharedExt );
    } /* endif */
  } /* endif */

  // look for objects having the same short name
  {
    USHORT usDosRC = NO_ERROR;         // return code of called DOS functions
    USHORT usCount = 1;                // number of files requested
    HDIR   hDir = HDIR_CREATE;         // file find handle
    BOOL   fOK;
    HANDLE hMutexSem = NULL;

    GETMUTEX(hMutexSem);

    // if specified long name has exactly 8 alphanumeric characters
    // check if there is already an object with the given name
    // if not we have to use the long name search method as previous
    // versions of TMgr may have created this object using a long name
    if ( NameType == MAYBELONGNAME )
    {
      strcpy( pData->szSearchPath, pData->szFullPath );
      strcat( pData->szSearchPath, pszOrgLongName );
      strcat( pData->szSearchPath, pData->szExt );
      UtlUpper( pData->szSearchPath );

      if ( UtlFileExist(pData->szSearchPath) )
      {
        // switch to short file mode as there is already an object with this
        // short name
        NameType = SHORTNAME;
        strcpy( pData->szShortName, pszOrgLongName );
      }
      else
      {
        // restore long name search path
        strcpy( pData->szSearchPath, pData->szFullPath );
        strcat( pData->szSearchPath, pData->szShortName );
        strcat( pData->szSearchPath, DEFAULT_PATTERN_NAME );
        strcat( pData->szSearchPath, pData->szExt );
      } /* endif */
    } /* endif */

    // if specified long name is a short name use this name for search
    if ( NameType == SHORTNAME )
    {
      strcpy( pData->szSearchPath, pData->szFullPath );
      strcat( pData->szSearchPath, pData->szShortName );
      strcat( pData->szSearchPath, pData->szExt );
      strcpy( pszShortName, pData->szShortName );

      if ( (pszDrive != NULL) && (*pszDrive != EOS) )
      {
        strcpy( pData->szSharedSearchPath, pData->szSharedFullPath );
        strcat( pData->szSharedSearchPath, pData->szShortName );
        strcat( pData->szSharedSearchPath, pData->szSharedExt );
      } /* endif */
    } /* endif */

    usDosRC = UtlFindFirst( pData->szSearchPath, &hDir, FILE_NORMAL,
                            &(pData->stResultBuf), sizeof(pData->stResultBuf),
                            &usCount, 0L, FALSE );

    while ( (iResult == 0) && (usDosRC == NO_ERROR) && usCount )
    {
      PVOID pProp = NULL;      // ptr to object properties
      ULONG ulBytesRead;

      // in TM_OBJECT mode only: ignore all files but files with the .MEM and .SLM extension
      {
        strcpy( pData->szSearchPath, pData->szFullPath );
        strcat( pData->szSearchPath, RESBUFNAME(pData->stResultBuf) );

        fOK = UtlLoadFileL( pData->szSearchPath, &pProp, &ulBytesRead, FALSE, FALSE );
        if ( !fOK )
        {
          usDosRC = ERROR_PROPERTY_ACCESS;
        } /* endif */

        if ( usDosRC == NO_ERROR && (ulBytesRead > sizeof(PROPHEAD) ))
        {
          BOOL fFound;
          PSZ pszPropName = NULL;
          CHAR szShortName[MAX_FILESPEC];
          PPROP_NTM pMemProp = (PPROP_NTM)pProp;

          // get position of long name in property file
          pszPropName = pMemProp->szLongName;
          if ( *pszPropName == EOS )
          {
            pszPropName = szShortName;
            Utlstrccpy( szShortName, RESBUFNAME(pData->stResultBuf), DOT );
          } /* endif */

          // compare long name of found object with given long name
          // use case-insensitive compare

          strcpy( pData->szInLongName, pszOrgLongName );
          OEMTOANSI( pData->szInLongName );
          CharUpper( pData->szInLongName  );
          strcpy( pData->szPropLongName, pszPropName  );
          OEMTOANSI( pData->szPropLongName );
          CharUpper( pData->szPropLongName );

          fFound = ( strcmp( pData->szInLongName, pData->szPropLongName ) == 0);

          if ( fFound )
          {
            iResult = 1;
            Utlstrccpy( pszShortName, RESBUFNAME( pData->stResultBuf ), DOT );
            strcpy( pszOrgLongName, pszPropName );
          } /* endif */

          UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
        } /* endif */
      } /* endif */

      // try next object
      if ( iResult == 0 )
      {
        usCount = 1;
        usDosRC = UtlFindNext( hDir, &pData->stResultBuf, sizeof(pData->stResultBuf), &usCount, FALSE );
      } /* endif */
    } /* endwhile */

    UtlFindClose( hDir, FALSE );

    // release Mutex
    RELEASEMUTEX(hMutexSem);
  } /* endif */

  // check if there is a shared memory on one of the LAN drives having the given long name
  if ((iResult == 0) && (pszDrive != NULL) && (*pszDrive != EOS) )
  {
    char *pszCurDrive = pszDrive;
    BOOL fSharedMemExists = FALSE;

    while ( (*pszCurDrive != EOS) && (iResult == 0)&& !fSharedMemExists )
    {
      USHORT usDosRC = NO_ERROR;         // return code of called DOS functions+-
      USHORT usCount = 1;                // number of files requested
      HDIR   hDir = HDIR_CREATE;         // file find handle
      BOOL   fOK;

      pData->szSharedSearchPath[0] = *pszCurDrive;
      usDosRC = UtlFindFirst( pData->szSharedSearchPath, &hDir, FILE_NORMAL,
                              &(pData->stResultBuf), sizeof(pData->stResultBuf),
                              &usCount, 0L, FALSE );

      while ( (iResult == 0) && (usDosRC == NO_ERROR) && usCount )
      {
        PVOID pProp = NULL;      // ptr to object properties
        ULONG ulBytesRead;

        strcpy( pData->szPropFileName, pData->szSharedFullPath );
        strcat( pData->szPropFileName, RESBUFNAME(pData->stResultBuf) );
        pData->szPropFileName[0] = *pszCurDrive;

        fOK = UtlLoadFileL( pData->szPropFileName, &pProp, &ulBytesRead, FALSE, FALSE );
        if ( !fOK )
        {
          usDosRC = ERROR_PROPERTY_ACCESS;
        } /* endif */

        // if load successful and file size is at least size of property header..
        if ( (usDosRC == NO_ERROR) && (ulBytesRead > sizeof(PROPHEAD) ) )
        {
          BOOL fFound;
          PSZ pszPropName = NULL;
          PPROP_NTM pMemProp = (PPROP_NTM)pProp;

          // compare long name of found object with given long name
          // use case-insensitive compare
          fFound = ( _stricmp( pszOrgLongName, pMemProp->szLongName ) == 0);

          if ( fFound )
          {
            iResult = 2;
            fSharedMemExists = TRUE;
            Utlstrccpy( pszShortName, RESBUFNAME( pData->stResultBuf ), DOT );
            strcpy( pszOrgLongName, pMemProp->szLongName );
            if ( pszDrive != NULL )
            {
              pszDrive[0] = pData->szSharedSearchPath[0];
              pszDrive[1] = EOS;
            }
          } /* endif */

          UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
        } /* endif */

        // try next object
        if ( iResult == 0 )
        {
          usCount = 1;
          usDosRC = UtlFindNext( hDir, &pData->stResultBuf, sizeof(pData->stResultBuf), &usCount, FALSE );
        } /* endif */
      } /* endwhile */

      UtlFindClose( hDir, FALSE );

      // next drive letter
      pszCurDrive++;
    }
  } /* endif */

  // find a unique name if memory does not exist
  if ( (iResult == 0) && (NameType != SHORTNAME) )
  {
    if ( NameType == MAYBELONGNAME  )
    {
      // use long name as short name to create the new object
      strcpy( pszShortName, pszOrgLongName );
      UtlUpper( pszShortName );
    }
    else
    {
      CHAR szCounter[4];                 // counter for object name
      BOOL fNameIsInUse = FALSE;
      HANDLE hMutexSem = NULL;

      strcpy( szCounter, "000" );        // counter start value

      // do not allow other process to search while we search for a new name
      GETMUTEX(hMutexSem);

      do
      {
        // build object name
        int iNameLenWithoutExt = 0;
        strcpy( pData->szSearchPath, pData->szFullPath );
        strcat( pData->szSearchPath, pData->szShortName );
        strcat( pData->szSearchPath, szCounter );
        iNameLenWithoutExt = strlen( pData->szSearchPath );
        strcat( pData->szSearchPath, LANSHARED_MEM_PROP );

        if ( *pszDrive != NULL )
        {
          strcpy( pData->szSharedSearchPath, pData->szSharedFullPath );
          strcat( pData->szSharedSearchPath, pData->szShortName );
          strcat( pData->szSharedSearchPath, szCounter );
          strcat( pData->szSharedSearchPath, EXT_OF_SHARED_MEMPROP );
        } /* endif */

        // increment alphanumeric 'counter' (counts from '000' to 'ZZZ')
        {
          if ( szCounter[2] == '9' ) szCounter[2] = 'A';
          else if ( szCounter[2] == 'Z' )
          {
            szCounter[2] = '0';
            if ( szCounter[1] == '9' )
            {
              szCounter[1] = 'A';
            }
            else if ( szCounter[1] == 'Z' )
            {
              szCounter[1] = '0';
              if ( szCounter[0] == '9' )
              {
                szCounter[0] = 'A';
              }
              else if ( szCounter[0] == 'Z' )
              {
                // overflow of our counter ... restart with 000
                UtlError( ERROR_INTERNAL, MB_CANCEL, 0, NULL, INTERNAL_ERROR );
                szCounter[0] = '0';
                szCounter[1] = '0';
                szCounter[2] = '0';
              }
              else
              {
                szCounter[0] += 1;
              } /* endif */
            }
            else
            {
              szCounter[1] += 1;
            } /* endif */
          }
          else
          {
            szCounter[2] += 1;
          } /* endif */
        }

        fNameIsInUse = UtlFileExist( pData->szSearchPath );

        // check shared drives as well
        char *pszSharedDrive = pszDrive;
        while ( !fNameIsInUse && (*pszSharedDrive != EOS) )
        {
          pData->szSharedSearchPath[0] = *pszSharedDrive;
          fNameIsInUse = UtlFileExist( pData->szSharedSearchPath );
          if ( !fNameIsInUse ) pszSharedDrive++;
        } /* endif */
      } while ( fNameIsInUse ); /* enddo */

      Utlstrccpy( pszShortName, UtlGetFnameFromPath( pData->szSearchPath ), DOT );

      // reserve name if requested
      if ( fReserveName )
      {
        PPROP_NTM pProp = NULL;
        if ( UtlAlloc( (PVOID *)&pProp, 0L, sizeof(PROP_NTM ), NOMSG ) )
        {
          strcpy( pProp->szLongName, pszOrgLongName );
          strcpy( pProp->stPropHead.szPath, pData->szSearchPath );
          strcpy( pProp->stPropHead.szName, UtlSplitFnameFromPath(pProp->stPropHead.szPath) );
          UtlSplitFnameFromPath( pProp->stPropHead.szPath ); 
          UtlWriteFile( pData->szSearchPath, sizeof(PROP_NTM ), (PVOID)pProp, FALSE );
          UtlAlloc( (PVOID *) &pProp, 0L, 0L, NOMSG );
          if ( pfReserved ) *pfReserved = TRUE;
        } /* endif */
      } /* endif */

      // release Mutex
      RELEASEMUTEX(hMutexSem);
    } /* endif */
  } /* endif */

  // cleanup
  if ( pData != NULL ) UtlAlloc( (PVOID *)&pData, 0L, 0L,NOMSG );

  return( iResult );
} /* end of method GetShortName */


extern "C" {
  __declspec(dllexport)
  unsigned short getPluginInfo( POTMPLUGININFO pPluginInfo )
  {
    strcpy( pPluginInfo->szName, pszPluginName );
    strcpy( pPluginInfo->szShortDescription, pszShortDescription );
    strcpy( pPluginInfo->szLongDescription, pszLongDescription );
    strcpy( pPluginInfo->szVersion, pszVersion );
    strcpy( pPluginInfo->szSupplier, pszSupplier );
    pPluginInfo->eType = OtmPlugin::eSharedTranslationMemoryType;
    strcpy( pPluginInfo->szDependencies, "" );
    pPluginInfo->iMinOpenTM2Version= -1;
    return( 0 );
  }
}
