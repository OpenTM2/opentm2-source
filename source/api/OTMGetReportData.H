/*! \brief OTMGetReportData.H
	Copyright (c) 1999-2014, International Business Machines Corporation and others. All rights reserved.
*/



/*! \brief error codes used by OTMGetReportData API call
*/
#define OTMGRD_ERROR_FXP_NOTFOUND      3001  // the specified exported folder package could not be accessed
#define OTMGRD_ERROR_FXP_CORRUPTED     3002  // the specified exported folder package is not valid FXP file or is corrupted#
#define OTMGRD_ERROR_NO_CALCREPORTDATA 3003  // the exported folder package does not containn binary calculation report data
#define OTMGRD_ERROR_INVALID_PARMS     3004  // the exported folder package does not containn binary calculation report data
#define OTMGRD_ERROR_BUFFER_TOO_SMALL  3005  // the provided buffer area is not large enough for the calculation profile name
#define OTMGRD_ERROR_NOT_ENOUGH_MEMORY 3006  // failed to allocate memory for temporary data
 
#ifdef __cplusplus
  extern "C" {
#endif

__declspec(dllexport) 
int OtmGetReportData
(
  char        *pszFolderPackage,
  long        *plWords,
  float       *pflPayableWords,
  char        *pszProfileNameBuffer,
  int         iProfileNameBufferSize

);

#ifdef __cplusplus
  }
#endif

