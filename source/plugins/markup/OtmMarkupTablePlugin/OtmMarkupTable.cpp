/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#include "core\pluginmanager\PluginManager.h"

#include "OtmMarkupTable.h"


void      SaveValue2( char **, char * ) ;

/*! \brief Supplies the name of the markup table

The name of the markup table is the unique identifier
of a markup table within a markup table plugin

*/
int OtmMarkupTable::getName
(
    char *pszBuffer,
    int iBufSize
)
{
	int iLength = 0;
	
    *pszBuffer = '\0' ;
    if (  pInfo != NULL ) {
      iLength = CopyToBuffer( NULL, pInfo->pszName, pszBuffer, iBufSize, false ); 
    } /* end */     

    return( iLength );
};


/*! \brief Supplies the descriptive name of the markup table

The descriptive name is the name of the markup table 
as it is displayed to the user

*/
int OtmMarkupTable::getShortDescription(
    char *pszBuffer,
    int iBufSize
)
{
    int iLength = 0;

    *pszBuffer = '\0' ;
    if ( ( pInfo != NULL ) &&
         ( pInfo->pszShortDescription != NULL ) &&
         ( pInfo->pszShortDescription[0] != '\0' ) ) {
      iLength = CopyToBuffer( NULL, pInfo->pszShortDescription, pszBuffer, iBufSize, false ); 
    } /* end */     

    return( iLength );
};

	
/*! \brief Supplies the description of the markup table

The markup table description gives some information on
the markup table and the supported file formats					 

*/
int OtmMarkupTable::getLongDescription(
    char *pszBuffer,
    int iBufSize
)
{
    int iLength = 0;

    *pszBuffer = '\0' ;
    if ( ( pInfo != NULL ) &&
         ( pInfo->pszLongDescription != NULL ) &&
         ( pInfo->pszLongDescription[0] != '\0' ) ) {
       iLength = CopyToBuffer( NULL, pInfo->pszLongDescription, pszBuffer, iBufSize, false ); 
    } /* end */     

    return( iLength );
};

	
/*! \brief Supplies the name of the plugin supplier

The supplier is the developer of the markup table

*/
int OtmMarkupTable::getSupplier(
    char *pszBuffer,
    int iBufSize
)
{
    int iLength = 0;

    iBufSize = 0 ;
    *pszBuffer = '\0' ;  
    return( iLength );
};


/*! \brief Supplies the fully qualified name of the .TBL file

This method supplies the fully qualified file name of the
markup tables .TBL file

*/
int OtmMarkupTable::getTableFileName(
    char *pszBuffer,
    int iBufSize
)
{
    int iLength = 0;

    *pszBuffer = '\0' ;
    if (  pInfo != NULL ) {
       iLength = CopyToBuffer( pszBasePath, pInfo->pszTable, pszBuffer, iBufSize, false ); 
    } /* end */     

    return( iLength );
};
	
/*! \brief Supplies the version of the markup table

This method supplies the version of the markup

*/
int OtmMarkupTable::getVersion(
    char *pszBuffer,
    int iBufSize
)
{
    int iLength = 0;

    *pszBuffer = '\0' ;
    if ( ( pInfo != NULL ) &&
         ( pInfo->pszVersion != NULL ) &&
         ( pInfo->pszVersion[0] != '\0' ) ) {
       iLength = CopyToBuffer( NULL, pInfo->pszVersion, pszBuffer, iBufSize, false ); 
    } /* end */     

    return( iLength );
};

	
/*! \brief Supplies the fully qualified file name of the markup table user exit DLL

This method supplies the fully qualified file name of the
markup user exit DLL (if there is any)

*/
int OtmMarkupTable::getUserExitFileName(
    char *pszBuffer,
    int iBufSize
)
{
    int iLength = 0;

    *pszBuffer = '\0' ;
    if ( ( pInfo != NULL ) && 
         ( pInfo->pszUserExit != NULL ) && 
         ( pInfo->pszUserExit[0] != '\0' ) ) {
       iLength = CopyToBuffer( pszBasePath, pInfo->pszUserExit, pszBuffer, iBufSize, false ); 
    } /* end */     

    return( iLength );
}

	
/*! \brief Supplies a list of the files belonging to this markup table

This method supplies a list of the fully qualified file names of the
files which belong to this markup table and which should be exported
when exporting the markup table within a folder.

The list of files is stored in the buffer as a list of null-terminated
strings with an empty string as list end delimiter

*/
int OtmMarkupTable::getAllFiles(
    char *pszBuffer,
    int iBufSize
)
{
    char szTemp[8096] ;
    int iLength = 0;

    *pszBuffer = '\0' ;
    if (  pInfo != NULL ) {
      strcpy( szTemp, pInfo->pszTable ) ;
      if ( ( pInfo->pszUserExit    ) &&
           ( pInfo->pszUserExit[0] ) ) {
         strcat( szTemp, "," ) ;
         strcat( szTemp, pInfo->pszUserExit ) ;
      }
      if ( ( pInfo->pszFileList    ) &&
           ( pInfo->pszFileList[0] ) ) {
         strcat( szTemp, "," ) ;
         strcat( szTemp, pInfo->pszFileList ) ;
      }
      iLength = CopyToBuffer( pszBasePath, szTemp, pszBuffer, iBufSize, true ); 
    } /* end */     

    return( iLength );
}

/*! \brief Checks if markup table may be exported

	This method checks if this markup table may be exported in external format
	
	\returns TRUE when markup table may be exported

	*/
bool OtmMarkupTable::isExportable()
{
    return( true );
}

  /*! \brief Checks if markup table may be imported from an external file

	This method checks if this markup table may be imported from the external format
	
	\returns TRUE when markup table may be imported

	*/
bool OtmMarkupTable::isImportable()
{
    return( false );
}


  /*! \brief Checks if markup table may be deleted

	This method checks if this markup table can be deleted by the user
	
	\returns TRUE when markup table can be deleted

	*/
bool OtmMarkupTable::isDeletable()
{
    return( false );
}


  /*! \brief Checks if markup table is protected and cannot be changed

	This method checks if this markup table can be changed by the user
	
	\returns TRUE when markup table is protected

	*/
bool OtmMarkupTable::isProtected()
{
    return( true );
}

  /*! \brief Update interface for the markup table

	This method can update its internal table files with the files provided in the 
  comma separated update file list

  \param pszTableName   Pointer to name of markup table
  \param pszDescription   Pointer to markup table description or NULL
  \param pszVersion   Pointer to version of markup table or NULL
  \param pTableFileName   Pointer to name of TBL file
  \param pUserExitFileName   Pointer to name of user exit DLL file or NULL
  \param pFileList   Pointer to other markup table files (with TABLE\ or BIN\ prefix) or NULL
	
	\returns TRUE when the markup table files have been updated and FALSE when the update is 
  not possible

	*/
bool OtmMarkupTable::updateFiles(
    char   *pszTableName,
    char   *pszDescription,
    char   *pszVersion,
    char   *pszTableFileName,
    char   *pszUserExitFileName,
    char   *pszFileList )
{
   bool    bNewMarkup = false ;
   bool    bReturn = true ;
   

   if ( ( pszTableName ) &&
        ( ! pInfo->pszName )  ) {
      bNewMarkup = true ;
      SaveValue2( &(pInfo->pszName), pszTableName ) ;
   }
   if ( pszDescription ) {
      if ( ! pInfo->pszShortDescription ) 
         SaveValue2( &(pInfo->pszShortDescription), pszDescription ) ;
      if ( ! pInfo->pszLongDescription ) 
         SaveValue2( &(pInfo->pszLongDescription), pszDescription ) ;
   }
   if ( ( pszVersion ) &&
        ( ( ! pInfo->pszVersion ) ||
          ( stricmp( pszVersion, pInfo->pszVersion ) >= 0 ) ) ) {
      SaveValue2( &(pInfo->pszVersion), pszVersion ) ;
   }
   if ( ( pszTableFileName ) &&
        ( ! pInfo->pszTable ) ) {
      SaveValue2( &(pInfo->pszTable), pszTableFileName ) ;
   }
   if ( ( ( pszUserExitFileName ) &&
          ( ( ! pInfo->pszUserExit ) ||
            ( stricmp( pszUserExitFileName, pInfo->pszUserExit ) ) ) ) ||
        ( ( ! pszUserExitFileName ) &&
          ( pInfo->pszUserExit ) ) ) {
      SaveValue2( &(pInfo->pszUserExit), pszUserExitFileName ) ;
   }
   if ( ( ( pszFileList ) &&
          ( ( ! pInfo->pszFileList ) ||
            ( stricmp( pszFileList, pInfo->pszFileList ) ) ) ) ||
        ( ( ! pszFileList ) &&
          ( pInfo->pszFileList ) ) ) {
      SaveValue2( &(pInfo->pszFileList), pszFileList ) ;
   }

   return( bReturn ) ;
}


/* -------------------------------------------------------------- */
/*   deleteMarkup                                                 */
/* -------------------------------------------------------------- */
/*!     \brief Delete a markup table

  \param pszMarkupName   Pointer to name of markup table
	
	\returns TRUE when the markup table was deleted and FALSE when the delete is 
  not possible

*/
bool OtmMarkupTable::deleteMarkup(
   char   *pszMarkupName )
{
     bool   bReturn = false ;

     pszMarkupName ;
     return( bReturn ) ;
}


/*! 
  \brief copies a string to the supplied buffer
	
  \param pszPath Pointer a path which should prefix the string or NULL
  \param pszData Pointer to the string being copied to the buffer
  \param pszBuffer Pointer to buffer receiving the markup table string
  \param iBufSize Size of the buffer in number of characters
  \param fListOfFiles TRUE if pszData contains a list of files or FALSE if only a single file

  \returns Number of characters copied to pszBuffer including the terminating null character
  	
*/
int OtmMarkupTable::CopyToBuffer
 (
		char *pszPath,
		char *pszData,
		char *pszBuffer,
		int  iBufSize,
        bool fListOfFiles
)
{
    char  *pszTempPath ; 
    int iCopied = 0;
    bool fMoreNames = true;

    do {
      // prefix data with any path information
      if ( pszPath != NULL  ) {
        pszTempPath = pszPath ; 
		while ( (iBufSize >= 1) && (*pszTempPath != '\0') ) {
          *pszBuffer++ = *pszTempPath++;
          iCopied++;
          iBufSize--;
        } 
        if ( iBufSize >= 1 ) {
          *pszBuffer++ = '\\';
        } 
      }      

      // copy the string to the buffer area until end-of-data, end-of-buffer or list-of-files separator
      while ( ( iBufSize >= 1    ) && 
              ( *pszData != '\0' ) && 
              ( ( !fListOfFiles  ) || 
                (*pszData != ',' ) ) ) {
        *pszBuffer++ = *pszData++;
        iCopied++;
        iBufSize--;
      }
      if ( fListOfFiles && (*pszData == ',') ) {
        *pszBuffer++ = *pszData++;
        iCopied++;
        iBufSize--;
        fMoreNames = true;
      } else {
        fMoreNames = false;
      } 
    } while( fListOfFiles && fMoreNames ) ;

    *pszBuffer = '\0';
    iCopied++;

    return( iCopied );
}

/* -------------------------------------------------------------- */
/*   SaveValue2                                                   */
/*                                                                */
/*   Allocate storage and save the information there.             */
/* -------------------------------------------------------------- */
/*! 
  \brief Copies string to new allocated storage

  \param Output   Pointer to location to save new memory pointer
  \param Input   Pointer to string to allocate memory for and copy
  	
*/

void  SaveValue2( char **Output, char *Input )
{
   if ( *Output ) 
      free( *Output ) ;
   *Output = NULL ;
   if ( ( Input ) &&
        ( Input[0] ) ) {
      *Output = (char*)malloc( strlen(Input)+1 ) ;
      strcpy( *Output, Input ) ;
   }
}

 