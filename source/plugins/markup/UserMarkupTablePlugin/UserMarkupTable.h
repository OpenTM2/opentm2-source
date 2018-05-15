//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
#ifndef _USERMARKUPTABLE_H_
#define _USERMARKUPTABLE_H_

#include "core\pluginmanager\OtmMarkup.h"


// structure of the elements in the static markup table list
typedef struct _MARKUPINFO
{
    char       *pszName;
	char       *pszShortDescription;
	char       *pszLongDescription;
	char       *pszSupplier;
	char       *pszVersion;
	char       *pszTable;
	char       *pszUserExit;
	char       *pszFileList;
} MARKUPINFO, *PMARKUPINFO;




/*! \brief Class for the markup table implementation for USER markup tables
*/

class __declspec(dllexport) UserMarkupTable: public OtmMarkup 
{
  private:

	/*! \brief Pointer to the markup table info */
  PMARKUPINFO pInfo;

	/*! \brief Pointer to the base directory of the markup table plugin  */
  char *pszBasePath;

  public:

  UserMarkupTable(){ pInfo = NULL; pszBasePath = NULL; }

  UserMarkupTable( PMARKUPINFO pMarkupInfo, char *pszPath )
  {
    pInfo = pMarkupInfo;
    pszBasePath = pszPath; 
  }

  ~UserMarkupTable() {
     if ( pInfo->pszName ) free( pInfo->pszName ) ;
     if ( pInfo->pszShortDescription ) free( pInfo->pszShortDescription ) ;
     if ( pInfo->pszLongDescription ) free( pInfo->pszLongDescription ) ;
     if ( pInfo->pszSupplier ) free( pInfo->pszSupplier ) ;
     if ( pInfo->pszVersion ) free( pInfo->pszVersion ) ;
     if ( pInfo->pszTable ) free ( pInfo->pszTable ) ;
     if ( pInfo->pszUserExit ) free( pInfo->pszUserExit ) ;
     if ( pInfo->pszFileList ) free( pInfo->pszFileList ) ;
  };
	
	/*! \brief Supplies the name of the markup table
	
	The name of the markup table is the unique identifier
	of a markup table within a markup table plugin
	
	*/
	int getName(
		char *pszBuffer,
		int iBufSize
	);

	/*! \brief Supplies the descriptive name of the markup table
	
	The descriptive name is the name of the markup table 
	as it is displayed to the user
	
	*/
	int getShortDescription(
		char *pszBuffer,
		int iBufSize
	);
	
	/*! \brief Supplies the description of the markup table

	The markup table description gives some information on
	the markup table and the supported file formats					 
	
	*/
	int getLongDescription(
		char *pszBuffer,
		int iBufSize
	);
	
	/*! \brief Supplies the fully qualified name of the .TBL file

	This method supplies the fully qualified file name of the
	markup tables .TBL file
	
	*/
	int getTableFileName(
		char *pszBuffer,
		int iBufSize
	);
	
	/*! \brief Supplies the version of the markup table

	This method supplies the version of the markup
	
	*/
	int getVersion(
		char *pszBuffer,
		int iBufSize
	);
	
	/*! \brief Supplies the name developer of the markup table
	
	The name of the markup table supplier is the developer
	of a markup table
	
	*/
	int getSupplier(
		char *pszBuffer,
		int iBufSize
	);
	
	/*! \brief Supplies the fully qualified file name of the markup table user exit DLL

	This method supplies the fully qualified file name of the
	markup user exit DLL (if there is any)
	
	*/
	int getUserExitFileName(
		char *pszBuffer,
		int iBufSize
	);
	
	/*! \brief Supplies a list of the files belonging to this markup table

	This method supplies a list of the fully qualified file names of the
	files which belong to this markup table and which should be exported
	when exporting the markup table within a folder.
	
	The list of files is stored in the buffer as a list of null-terminated
	strings with an empty string as list end delimiter
	
	*/
	int getAllFiles(
		char *pszBuffer,
		int iBufSize
	);

  /*! \brief Checks if markup table may be exported

	This method checks if this markup table may be exported in external format
	
	\returns TRUE when markup table may be exported

	*/
	bool isExportable();

  /*! \brief Checks if markup table may be imported from an external file

	This method checks if this markup table may be imported from the external format
	
	\returns TRUE when markup table may be imported

	*/
	bool isImportable();


  /*! \brief Checks if markup table may be deleted

	This method checks if this markup table can be deleted by the user
	
	\returns TRUE when markup table can be deleted

	*/
	bool isDeletable();


  /*! \brief Checks if markup table is protected and cannot be changed

    This method checks if this markup table can be changed by the user

    \returns TRUE when markup table is protected

	*/
	bool isProtected();


   /*! \brief Update interface for the markup table

	This method can update its internal table files with the files provided in the 
  comma separated update file list
	
	\returns  0 when the update failed
              1 when the markup table files have been updated
              2 when the update has been delayed and will occur at restart

	*/
	int updateFiles( 
           char   *pszMarkupName,
           char   *pszShortDescription,
           char   *pszLongDescription,
           char   *pszVersion,
           char   *pszTableFileName,
           char   *pszUserExitFileName,
           char   *pszFileList
    );


    /*! \brief Update XML information for the markup table

	This method can update the internal XML control file within new
  information about this markup tables.

	\returns  0 when the update failed
              1 when the markup table information has been updated

	*/
	int updateInfo( 
        char   *pszMarkupName,
        char   *pszShortDescription,
        char   *pszLongDescription,
        char   *pszVersion,
        char   *pszUserExitFileName
	);


/*!     \brief Delete a markup table

	This method can delete a markup table, if deletion is allowed
	
	\returns TRUE when the markup table was deleted and FALSE when the delete is 
  not possible

*/
    bool deleteMarkup(
           char   *pszMarkupName 
   );
  
    /*! \brief Get the markup family name for a markup

	This method looks for family name for a markup

	\returns Family name if have, otherwise a empty string
	*/
	std::string getFamilyName()
	{
		return "";
	}
	

private:

  /*! \brief copies a string to the supplied buffer
	
  \param pszPath Pointer a path which should prefix the string or NULL
  \param pszData Pointer to the string being copied to the buffer
  \param pszBuffer Pointer to buffer receiving the markup table string
  \param iBufSize Size of the buffer in number of characters
  \param fListOfFiles TRUE = process a comma delimited list of files
	\returns Number of characters copied to pszBuffer including the terminating null character
  	
	*/
	
	int CopyToBuffer
  (
		char *pszPath,
		char *pszData,
		char *pszBuffer,
		int iBufSize,
    bool fListOfFiles = false
	);

};
#endif // #ifndef _USERMARKUPTABLE_H_
 