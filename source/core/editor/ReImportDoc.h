/*! \file
	Description: This file is the public header file for the re-import document function.

	Copyright Notice:

	Copyright (C) 1990-2015, International Business Machines
	Corporation and others. All rights reserved
*/

/*! \brief Checks if document can be re-imported 
  \param pszDocName fully qualified name of the STARGET document
\returns TRUE if external document is available for the re-import
*/
BOOL ReImport_IsAvailable( char *pszDocName );

/*! \brief Prepares the re-import of a document
  \param pszDocName fully qualified name of the STARGET document
  \param ulSegNum number of currently active segment within the document
  \param ppvReImportHandle address of a PVOID pointer which will receive the re-import handle on return
\returns TRUE if successful, FALSE in case of errors
*/
BOOL ReImport_Prepare( char *pszDocName, ULONG ulSegnum, PVOID *ppvReImportHandle );

/*! \brief Starts the re-import of a document
  \param pvReImportHandle the handle returned by a previous ReImport_Prepare call
\returns TRUE if successful, FALSE in case of errors
*/
BOOL ReImport_Start( PVOID pvReImportHandle );

/*! \brief Performs the actual re-import of a document
  \param hwnd window handle of caller
  \param pvReImportHandle the handle returned by a previous ReImport_Prepare call
\returns TRUE if successful, FALSE in case of errors
*/
__declspec(dllexport) BOOL ReImport_Process( HWND hwnd,  PVOID pvReImportHandle );
