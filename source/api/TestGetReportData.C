//+----------------------------------------------------------------------------+
//| TestGetReportData.C                                                        |
//|   Program to run tests of the OtmGetReportData API call                    |
//+----------------------------------------------------------------------------+
// Copyright (c) 2014, International Business Machines
// Corporation and others.  All rights reserved.
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
#include <stdio.h>
#include <string.h>
#include "OtmGetReportData.h" 

int main
(
  int  argc,
  char *argv[],
  char *envp[]
)
{
  int iRC = 0;
  long alWords[2];
  char szProfile[40];

  envp; 

  argc--; argv++;     //skip program name

  if ( argc == 0 )
  {
    printf( "%s\n", "Missing fully qualified name of the folder package" );
    return( -1 );
  }

  memset( szProfile, 0, sizeof(szProfile) );

  printf( "Specified folder package is \"%s\"\n", argv[0] ); 

  iRC = OtmGetReportData( argv[0], alWords, szProfile, sizeof(szProfile) );

  if ( iRC == 0 )
  {
    printf( "OtmGetReportData completed successfully\n" );
    printf( "%20s : %ld\n",  "Actual words", alWords[0] ); 
    printf( "%20s : %ld\n",  "Payable words", alWords[1] ); 
    printf( "%20s : \"%s\"\n",  "Profile name", szProfile ); 
  }
  else
  {
    char *pszError = "undefined";
    switch ( iRC )
    {
      case OTMGRD_ERROR_FXP_NOTFOUND:      pszError = "OTMGRD_ERROR_FXP_NOTFOUND"; break;
      case OTMGRD_ERROR_FXP_CORRUPTED:     pszError = "OTMGRD_ERROR_FXP_CORRUPTED"; break;
      case OTMGRD_ERROR_NO_CALCREPORTDATA: pszError = "OTMGRD_ERROR_NO_CALCREPORTDATA"; break;
      case OTMGRD_ERROR_INVALID_PARMS:     pszError = "OTMGRD_ERROR_INVALID_PARMS"; break;
      case OTMGRD_ERROR_BUFFER_TOO_SMALL:  pszError = "OTMGRD_ERROR_BUFFER_TOO_SMALL"; break;
      case OTMGRD_ERROR_NOT_ENOUGH_MEMORY: pszError = "OTMGRD_ERROR_NOT_ENOUGH_MEMORY"; break;
    }
    printf( "OtmGetReportData failed with a return code of %ld (%s)\n", iRC, pszError );
  }

  return( iRC );
} /* end of main */

