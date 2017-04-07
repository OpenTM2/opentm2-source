/*! \file
	Function prototypes for the OtmProofReadImport plugin filters
	
	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved
*/

#define GETNFOFUNC_NAME "getFilterInfo"

#define CONVERTFUNC_NAME "convertToProofReadFormat"


extern "C" {
/*! \brief Convert a file returned from the proof reading process into the internal proof read XML document format
  \param pszProofReadInFile fully qualified input file selected by the user
  \param pszProofReadXMLOut name of the XML output file to be created
  \returns 0 if successful or an error code
*/
__declspec(dllexport) 
int convertToProofReadFormat( const char *pszProofReadInFile, const char *pszProofReadXMLOut );  

/*! \typedef for Convert function
*/
typedef int (__cdecl *CONVERTFUNC)( const char *, const char * );

/*! \brief Return information about this filter
  \param pszFilterName buffer for the filter name to be displayed to the user, NULL if no filter name is to be returned
  \param iFilterNameBufSize size of the pszFilterName buffer
  \param pszFileExtension buffer for the file type extension processed by this filter, NULL if no file extension info is to be returned
  \param iFileExtensionBufSize size of the pszFileExtension buffer
  \param pszVersion buffer for the filter version, NULL if no version info is to be returned
  \param iVersionBufSize size of the pszVersion buffer
  \returns 0 if successful or an error code
*/
__declspec(dllexport) 
int getFilterInfo( PSZ pszFilterName, int iFilterNameBufSize, PSZ pszFileExtension, int iFileExtensionBufSize, PSZ pszVersion, int iVersionBufSize );

/*! \typedef for GetFilterInfo function
*/
typedef int (__cdecl *GETINFOFUNC)( PSZ, int, PSZ, int, PSZ, int );


}
