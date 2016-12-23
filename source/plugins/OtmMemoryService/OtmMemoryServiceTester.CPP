//+----------------------------------------------------------------------------+
//| OtmMemoryServiceTester.CPP                                                 |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2016, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: Batch interface using the TM API                              |
//|                                                                            |
//|  Test tool for the OpenTM2 API                                             |
//+----------------------------------------------------------------------------+
//| To be done / known limitations / caveats:                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//

#include <windows.h>
#include <string.h>

#include "OtmMemoryService.h"

wchar_t szLine[8096];                // buffer for script lines
wchar_t szCommand[8096];             // buffer for current command
char szScriptFile[1024];             // name of script file
char szLogFile[1024];                // name of script file

int main( int argc, char *argv[], char *envp[] )
{
  // skip program name
  argc--;
  argv++;

  // process arguments
  szScriptFile[0] = '\0';
  szLogFile[0] = '\0';

  while ( argc )
  {
    PSZ pszParm = argv[0];
    while(*pszParm == ' ') ++pszParm;

    if ( (*pszParm == '-') || (*pszParm == '/') )
    {
      if ( _strnicmp( pszParm+1, "log=", 4 ) == 0 )
      {
        strcpy( szLogFile, pszParm+5 );
      }
      else
      {
        wprintf( L"Warning: unknown option \'%S\' is ignored\n", pszParm );
      } /* endif */
    }
    else if ( szScriptFile[0] == '\0' )
    {
      strcpy( szScriptFile, pszParm );
    }
    else
    {
      wprintf( L"Warning: superfluos command line parameter \'%S\' is ignored\n", pszParm);
    } /* endif */
    argc--;
    argv++;
  } /* endwhile */

  // return if no script file name has been specified
  if ( szScriptFile[0] == '\0' )
  {
    wprintf( L"Error: missing name of the script file\n" );
    return( -1 );
  }

  // open the script file
  FILE *hfScript = fopen( szScriptFile, "rb" );
  if ( hfScript == NULL  )
  {
    wprintf( L"Error: could not open script file \"%S\"\n", szScriptFile );
    return( -1 );
  }

  // check for unicode BOM
  byte bBOM[2];
  fread( bBOM, 2, 1, hfScript );
  if ( (bBOM[0] == 0xFF) && (bBOM[1] == 0xFE) ) 
  {
    // unicode BOM detected
  }
  else if ( (bBOM[0] != 0) && (bBOM[1] == 0) ) 
  {
    // could be unicode, reposition to begin of file
    fseek( hfScript, 0, SEEK_SET );
  }
  else
  {
    wprintf( L"Error: Script file \"%S\" does not seem to be in Unicode (UTF16) encoding\n", szScriptFile );
    fclose( hfScript );
    return( -1 );
  } /* endif */

  // start OtmMemoryService
  OtmMemoryService *pService = OtmMemoryService::getInstance();
  if ( pService == NULL )
  {
    wprintf( L"Error: Could not start OtmMemoryService\n" );
    fclose( hfScript );
    return( -1 );
  } /* endif */

  // process script file
  BOOL fInCommand = FALSE;
  std::wstring strInParms = L"";
  std::wstring strOutParms = L"";
  while( !feof( hfScript ) )
  {
    memset( szLine, 0, sizeof(szLine) );
    fgetws( szLine, sizeof(szLine)/sizeof(szLine[0]), hfScript );

    // remove leading blanks
    wchar_t *pszStart = szLine;
    while ( (*pszStart != 0) && (*pszStart == L' ') ) pszStart++;

    // remove any trailing CRLF
    int iLen = wcslen( pszStart );
    if ( (iLen != 0) && (pszStart[iLen-1] == L'\n') ) iLen--;
    if ( (iLen != 0) && (pszStart[iLen-1] == L'\r') ) iLen--;
    pszStart[iLen] = 0;

    if ( fInCommand )
    {
      strOutParms.clear();
      if ( (*pszStart == L'*') || (*pszStart == 0) || (*pszStart == L'!') )
      {
        int iRC = 0;
        // end of parameter data reached, execute the command
        if ( wcsicmp( szCommand, L"importMemoryFromPackage" ) == 0 )
        {
          iRC = pService->importMemoryFromPackage( strInParms, strOutParms );
        }
        else if ( wcsicmp( szCommand, L"deleteMemory" ) == 0 )
        {
          iRC = pService->deleteMemory( strInParms, strOutParms );
        }
        else if ( wcsicmp( szCommand, L"createMemory" ) == 0 )
        {
          iRC = pService->createMemory( strInParms, strOutParms );
        }
        else if ( wcsicmp( szCommand, L"importMemory" ) == 0 )
        {
          iRC = pService->importMemory( strInParms, strOutParms );
        }
        else if ( wcsicmp( szCommand, L"openMemory" ) == 0 )
        {
          iRC = pService->openMemory( strInParms, strOutParms );
        }
        else if ( wcsicmp( szCommand, L"closeMemory" ) == 0 )
        {
          iRC = pService->closeMemory( strInParms, strOutParms );
        }
        else if ( wcsicmp( szCommand, L"lookupInMemory" ) == 0 )
        {
          iRC = pService->lookupInMemory( strInParms, strOutParms );
        }
        else if ( wcsicmp( szCommand, L"searchMemory" ) == 0 )
        {
          iRC = pService->searchMemory( strInParms, strOutParms );
        }
        else if ( wcsicmp( szCommand, L"updateMemory" ) == 0 )
        {
          iRC = pService->updateMemory( strInParms, strOutParms );
        }
        else
        {
          wprintf( L"Warning: unknown command \"%s\" ignored\n", szCommand );
          fInCommand = FALSE;
        } /* endif */

        if ( fInCommand )
        {
          wprintf( L"Processing command %s\n", szCommand );
          wprintf( L"Input parameter:\n" );
          wprintf( L"%s\n", strInParms.c_str() );
          wprintf( L"Result = %ld\n", iRC );
          wprintf( L"Output:\n" );
          wprintf( L"%s\n", strOutParms.c_str() );
          fInCommand = FALSE;
        } /* endif */
      }
      else
      {
        // add current line to parameter data
        strInParms.append( pszStart );
      }
    } /* endif */

    if ( *pszStart == L'!') 
    {
      // switch to command mode
      wcscpy( szCommand, pszStart + 1 );
      strInParms.clear();
      fInCommand = TRUE;
    }
  } /* endwhile */

  fclose( hfScript );

  return( 0 );
} /* end of main */
