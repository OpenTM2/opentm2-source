//	Copyright (C) 1990-2016, International Business Machines
//	Corporation and others. All rights reserved
#ifndef _OTMMARKUP_H_
#define _OTMMARKUP_H_

#include <string>

#define  UPDATE_MARKUP_ERROR    0
#define  UPDATE_MARKUP_OK       1
#define  UPDATE_MARKUP_INUSE    2

/*! \brief Abstract base-class for markup table objects
*/
class __declspec(dllexport) OtmMarkup
{

public:
   
	OtmMarkup(){};

	virtual ~OtmMarkup() {};
	
	/*! \brief Supplies the name of the markup table
	
	The name of the markup table is the unique identifier
	of a markup table within a markup table plugin

  \param pszBuffer Pointer to buffer receiving the markup table name
  \param iBufSize Size of the buffer in number of characters
	\returns Number of characters copied to pszBuffer including the terminating null character

	
	*/
	virtual int getName
    (
		char *pszBuffer,
		int iBufSize
	) = 0;

	/*! \brief Supplies the short description (descriptive name) of the markup table
	
	The short description of the markup table.  It is displayed to the user
    as the descriptive name.
	
  \param pszBuffer Pointer to buffer receiving the short description
  \param iBufSize Size of the buffer in number of characters
	\returns Number of characters copied to pszBuffer including the terminating null character

	*/
	virtual int getShortDescription
  (
		char *pszBuffer,
		int iBufSize
	) = 0;
	
	/*! \brief Supplies the long description of the markup table

	The long description gives some information on the markup table and 
    the supported file formats					 
	
  \param pszBuffer Pointer to buffer receiving the long description
  \param iBufSize Size of the buffer in number of characters
	\returns Number of characters copied to pszBuffer including the terminating null character

	*/
	virtual int getLongDescription
  (
		char *pszBuffer,
		int iBufSize
	) = 0;
	
	/*! \brief Supplies the name of the supplier of the markup table

	This supplier gives information about who supplied this markup table.
	
  \param pszBuffer Pointer to buffer receiving the supplier name
  \param iBufSize Size of the buffer in number of characters
	\returns Number of characters copied to pszBuffer including the terminating null character

	*/
	virtual int getSupplier
  (
		char *pszBuffer,
		int iBufSize
	) = 0;

	/*! \brief Supplies the version of the markup table

	This method supplies the version of the markup
	
  \param pszBuffer Pointer to buffer receiving the version string
  \param iBufSize Size of the buffer in number of characters
	\returns Number of characters copied to pszBuffer including the terminating null character

	*/
	virtual int getVersion
  (
		char *pszBuffer,
		int iBufSize
	) = 0;

	/*! \brief Supplies the fully qualified name of the .TBL file

	This method supplies the fully qualified file name of the
	markup tables .TBL file
	
  \param pszBuffer Pointer to buffer receiving the table file name
  \param iBufSize Size of the buffer in number of characters
	\returns Number of characters copied to pszBuffer including the terminating null character

	*/
	virtual int getTableFileName
  (
		char *pszBuffer,
		int iBufSize
	) = 0;
	
	/*! \brief Supplies the fully qualified file name of the markup table user exit DLL

	This method supplies the fully qualified file name of the
	markup user exit DLL (if there is any)
	
  \param pszBuffer Pointer to buffer receiving the user exit DLL
  \param iBufSize Size of the buffer in number of characters
	\returns Number of characters copied to pszBuffer including the terminating null character

	*/
	virtual int getUserExitFileName
  (
		char *pszBuffer,
		int iBufSize
	) = 0;
	
	/*! \brief Supplies a list of the files belonging to this markup table

	This method supplies a list of the fully qualified file names of the
	files which belong to this markup table and which should be exported
	when exporting the markup table within a folder.
	
	The list of files is stored in the buffer as a list of null-terminated
	strings with an empty string as list end delimiter
	
  \param pszBuffer Pointer to buffer receiving the file names
  \param iBufSize Size of the buffer in number of characters
	\returns Number of characters copied to pszBuffer including the terminating null character

	*/

	virtual int getAllFiles
  (
		char *pszBuffer,
		int iBufSize
	) = 0;
	
  /*! \brief Checks if markup table may be exported

	This method checks if this markup table may be exported in external format
	
	\returns TRUE when markup table may be exported

	*/
	virtual bool isExportable
  (
	) = 0;

  /*! \brief Checks if markup table may be imported from an external file

	This method checks if this markup table may be imported from the external format
	
	\returns TRUE when markup table may be imported

	*/
	virtual bool isImportable
  (
	) = 0;

  /*! \brief Checks if markup table may be deleted

	This method checks if this markup table can be deleted by the user
	
	\returns TRUE when markup table can be deleted

	*/
	virtual bool isDeletable
  (
	) = 0;

  /*! \brief Checks if markup table is protected and cannot be changed

	This method checks if this markup table can be changed by the user
	
	\returns TRUE when markup table is protected

	*/
	virtual bool isProtected
  (
	) = 0;

    /*! \brief Update interface for the markup table

	This method can update its internal table files with the files provided in the 
  comma separated update file list

  \param pszMarkupName   Pointer to name of markup table
  \param pszShortDescription   Pointer to markup table short description or NULL
  \param pszLongDescription   Pointer to markup table long description or NULL
  \param pszVersion   Pointer to version of markup table or NULL
  \param pTableFileName   Pointer to name of TBL file
  \param pUserExitFileName   Pointer to name of user exit DLL file or NULL
  \param pFileList   Pointer to other markup table files (with TABLE\ or BIN\ prefix) or NULL
	
	\returns  0 when the update failed
              1 when the markup table files have been updated
              2 when the update has been delayed and will occur at restart

	*/
	virtual int updateFiles
  ( 
        char   *pszMarkupName,
        char   *pszShortDescription,
        char   *pszLongDescription,
        char   *pszVersion,
        char   *pszTableFileName,
        char   *pszUserExitFileName,
        char   *pszFileList
	) = 0;

    /*! \brief Update XML information for the markup table

	This method can update the internal XML control file with new
  information about this markup tables.

  \param pszMarkupName   Pointer to name of markup table (input only)
  \param pszShortDescription   Pointer to markup table description or NULL
  \param pszLongDescription   Pointer to markup table description or NULL
  \param pszVersion   Pointer to version of markup table or NULL
  \param pUserExitFileName   Pointer to name of user exit DLL file or NULL
	
	\returns  0 when the update failed
              1 when the markup table information has been updated

	*/
	virtual int updateInfo
  ( 
        char   *pszMarkupName,
        char   *pszShortDescription,
        char   *pszLongDescription,
        char   *pszVersion,
        char   *pszUserExitFileName
	) = 0;

    /*! \brief Delete a markup table

	This method will delete a markup table if deletion is allowed

  \param pszMarkupName   Pointer to name of markup table
	
	\returns TRUE when the markup table was deleted and FALSE when the delete is 
  not possible

	*/
	virtual bool deleteMarkup
  ( 
        char   *pszMarkupName
	) = 0;

	/*! \brief Get the markup family name for a markup

	This method looks for family name for a markup

	\returns Family name if have, otherwise a empty string
	*/
	// For R012645
	virtual std::string getFamilyName() = 0;
};
#endif // #ifndef _OTMMARKUP_H_
 