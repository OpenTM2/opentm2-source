/*! \file
	Filter code for the conversion of OpenTM2 XML validation documents into the internal proof read import XML format
	
	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/
#include "EQF.H"
#include "..\OtmProofReadFilter.h"


/*! \brief Convert a file returned from the proof reading process into the internal proof read XML document format
  \param pszProofReadInFile fully qualified input file selected by the user
  \param pszProofReadXMLOut name of the XML output file to be created
  \returns 0 if successful or an error code
*/
int convertToProofReadFormat( const char *pszProofReadInFile, const char *pszProofReadXMLOut )
{
  if ( !UtlFileExist( (PSZ)pszProofReadInFile ) )
  {
    char szMessage[100];
    PSZ pszParm = (PSZ)pszProofReadInFile;
    UtlError( ERROR_FILE_ACCESS_ERROR, MB_CANCEL, 1, &pszParm, EQF_ERROR );
    return( ERROR_FILE_ACCESS_ERROR );
  }

  // delete any existing target file
  DeleteFile( pszProofReadXMLOut );

  // convert document using XSLT conversion
  static CHAR szCommand[1024];
  UtlMakeEQFPath( szCommand, NULC, SYSTEM_PATH, NULL );
  strcat( szCommand, "\\WIN\\XalanTransform \"" );
  strcat( szCommand, pszProofReadInFile );
  strcat( szCommand, "\" \"" );
  UtlMakeEQFPath( szCommand + strlen(szCommand), NULC, PLUGIN_PATH, NULL );
  strcat( szCommand, "\\OtmProofReadImport\\filter\\ValDocXML\\ValDocXML.XSL\" \"" ); // TODO: use actual path of filter DLL
  strcat( szCommand, pszProofReadXMLOut );
  strcat( szCommand, "\"" );
  system( szCommand );

  // TODO: error handling for conversion errors

  return( 0 );
}


/*! \brief Return information about this filter
  \param pszFilterName buffer for the filter name to be displayed to the user, NULL if no filter name is to be returned
  \param iFilterNameBufSize size of the pszFilterName buffer
  \param pszFileExtension buffer for the file type extension processed by this filter, NULL if no file extension info is to be returned
  \param iFileExtensionBufSize size of the pszFileExtension buffer
  \param pszVersion buffer for the filter version, NULL if no version info is to be returned
  \param iVersionBufSize size of the pszVersion buffer
  \returns 0 if successful or an error code
*/
int getFilterInfo( PSZ pszFilterName, int iFilterNameBufSize, PSZ pszFileExtension, int iFileExtensionBufSize, PSZ pszVersion, int iVersionBufSize )
{
  if ( pszFilterName != NULL )
  {
    strncpy( pszFilterName, "OpenTM2 Validation Document XML", iFilterNameBufSize - 1);
    pszFilterName[iFilterNameBufSize - 1] = 0;
  }
  if ( pszFileExtension != NULL )
  {
    strncpy( pszFileExtension, ".XML", iFileExtensionBufSize - 1 );
    pszFileExtension[iFileExtensionBufSize - 1] = 0;
  }
  if ( pszVersion != NULL )
  {
    strncpy( pszVersion, ".XML", iVersionBufSize - 1 );
    pszVersion[iVersionBufSize - 1] = 0;
  }

  return( 0 );
}
