//+----------------------------------------------------------------------------+
//| OTMCleanMem.C                                                              |
//+----------------------------------------------------------------------------+
// Copyright (C) 2012-2015, International Business Machines
// Corporation and others.  All rights reserved.
//
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: External Memory Cleanup Tool                                  |
//+----------------------------------------------------------------------------+

#include <windows.h>
#include <winbase.h>
#include <stdio.h>
#include <string.h>

#include <eqfserno.h>             // version number

wchar_t szSourceTagW[] = L"<Source>";
wchar_t szEndSourceTagW[] = L"</Source>";
wchar_t szTargetTagW[] = L"<Target";
wchar_t szEndTargetTagW[] = L"</Target";
char szSourceTag[] = "<Source>";
char szEndSourceTag[] = "</Source>";
char szTargetTag[] = "<Target";
char szEndTargetTag[] = "</Target";

// data types 
typedef struct WORKAREA
{
  //-- data for input memory processing --
  CHAR        szInSpec[MAX_PATH];                // input memory specification of user
  CHAR        szOutSpec[MAX_PATH];               // output memory specification of user
  CHAR        szInputMem[MAX_PATH];              // buffer for preperation of input memory name
  CHAR        szOutputMem[MAX_PATH];             // buffer for preperation of output memory name
  CHAR        szOutputPath[MAX_PATH];            // buffer for output path
  CHAR        szBuffer[4096];                    // general purpose buffer
  BOOL        fUnicode;                          // TRUE = input memory is in UTF-16 format
  FILE        *hInFile;                          // handle of input file
  FILE        *hOutFile;                         // handle of output file
  wchar_t     szLineW[4096];                     // buffer for input line 
  char        szLine[4096];                      // buffer for input line 
  wchar_t     szSegDataW[8192];                  // buffer for segment data
  char        szSegData[8192];                   // buffer for segment data
  BOOL        fRemoveTM;                         // TRUE = remove trademark tags
  BOOL        fReduceLF;                         // TRUE = reduce line breaks
  CHAR        szMsgBuffer[4096];                 // buffer for message texts


  // - statistics --
  ULONG       ulSegments;                        // number of segments written to output memory

  // -- other stuff --
} WORKAREA, *PWORKAREA;


static void showHelp();
static void GetOptions( PWORKAREA pData, PSZ pszValue );
static PSZ SplitFnameFromPath( PSZ path);
static USHORT MakeOutputName( PWORKAREA pData, PSZ pszInMemory, PSZ pszOutputPath, PSZ pszOutMemory );
static USHORT CheckExistence( PWORKAREA pData, PSZ pszInMemory );
static USHORT ShowError( PWORKAREA pData, PSZ pszMsg, PSZ pszParm );
static USHORT ShowInfo(PWORKAREA pData, PSZ pszMsg, PSZ pszParm );
static USHORT CleanSingleMemory( PWORKAREA pData, PSZ pszInMemory, PSZ pszOutMemory );
static void GetMemEncoding( FILE *hf, PBOOL pfUnicode );
static USHORT ProcessMemoryW( PWORKAREA pData );
static USHORT ProcessMemory( PWORKAREA pData );
static USHORT ProcessSegmentDataW( PWORKAREA pData );
static USHORT ProcessSegmentData( PWORKAREA pData );

// main entry point
int main( int argc, char *argv[], char *envp[] )
{
  USHORT   usRC = 0;
  PWORKAREA pData = NULL;

  envp;

  if( (argc == 2) && (stricmp(argv[1],"-h") == 0) || (argc == 1) )
  {
      showHelp();
      return 0;
  }

  // show logo
  printf( "OtmCleanMem EXP Memory Cleanup Tool\n\n" );


  // allocate our data area
  pData = (PWORKAREA)malloc( sizeof(WORKAREA) );
  if ( pData == NULL )
  {
    printf( "Error: Memory allocation error, program terminated" );
    return( -1 );
  } /* endif */

  memset( pData, 0, sizeof(WORKAREA) );

  // skip program name
  argc--;
  argv++;

  // process arguments
  while ( argc )
  {
    PSZ pszParm = argv[0];

    if ( (*pszParm == '-') || (*pszParm == '/') )
    {
      if ( strnicmp( pszParm+1, "in=", 3 ) == 0 )
      {
        PSZ pszValue = pszParm + 4;
        int len = sizeof(pData->szInSpec)/sizeof(pData->szInSpec[0])-1;
        strncpy( pData->szInSpec, pszValue, len);
        pData->szInSpec[len] = 0;
      }
      else if ( strnicmp( pszParm+1, "inputmem=", 9 ) == 0 )
      {
        PSZ pszValue = pszParm + 10;
        int len = sizeof(pData->szInSpec)/sizeof(pData->szInSpec[0])-1;
        strncpy( pData->szInSpec, pszValue, len);
        pData->szInSpec[len] = 0;
      }
       else  if ( strnicmp( pszParm+1, "out=", 4 ) == 0 )
      {
        PSZ pszValue = pszParm + 5;
        int len = sizeof(pData->szOutSpec)/sizeof(pData->szOutSpec[0])-1;
        strncpy( pData->szOutSpec, pszValue, len);
        pData->szOutSpec[len] = 0;
      }
      else  if ( strnicmp( pszParm+1, "outputmem=", 10 ) == 0 )
      {
        PSZ pszValue = pszParm + 11;
        int len = sizeof(pData->szOutSpec)/sizeof(pData->szOutSpec[0])-1;
        strncpy( pData->szOutSpec, pszValue, len);
        pData->szOutSpec[len] = 0;
      }
      else if ( strnicmp( pszParm+1, "op=", 3 ) == 0 )
      {
        GetOptions( pData, pszParm + 4 );
      }
      else if ( strnicmp( pszParm+1, "options=", 8 ) == 0 )
      {
        GetOptions( pData, pszParm + 9 );
      }
      else
      {
        printf( "Warning: unknown option \'%s\' is ignored\n", pszParm );
      } /* endif */
    }
    else
    {
      printf( "Warning: superfluous command line parameter \'%s\' is ignored\n", pszParm );
    } /* endif */

    argc--;
    argv++;
  } /* endwhile */

  if ( usRC == NO_ERROR )
  {
    if ( pData->szInSpec[0] == '\0' )
    {
        printf( "Error: No input memory specified\n\n" );
        showHelp();
        usRC = (USHORT)-2;
    } /* endif */
  } /* endif */


  if ( usRC == NO_ERROR )
  {
    if ( !pData->fReduceLF && !pData->fRemoveTM  )
    {
        printf( "Info: No or no valid options specified, using 1L (reduce to one line) and NOTM (Remove trademarks) as default\n\n" );
        pData->fReduceLF = TRUE;
        pData->fRemoveTM = TRUE;
    } /* endif */
  } /* endif */

  // clean the memory and handle wildcards in input memory name
  if ( usRC == NO_ERROR )
  {
    if ( (strchr( pData->szInSpec, '?' ) != NULL) || (strchr( pData->szInSpec, '*' ) != NULL) )
    {
      HANDLE hdir;
      static WIN32_FIND_DATA FindData;   

      hdir = FindFirstFile( pData->szInSpec, &FindData );
      if ( hdir == INVALID_HANDLE_VALUE )
      {
        usRC = ERROR_FILE_NOT_FOUND;
        ShowError( pData, "No files found matching the input pattern %s", pData->szInSpec );
      }
      else
      {
        BOOL fMoreFiles = TRUE;

        while ( !usRC && fMoreFiles )
        {
          if ( FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
          {
            // ignore directories
          }
          else
          {
            strcpy( pData->szInputMem, pData->szInSpec );
            if ( strchr( pData->szInputMem, '\\') == NULL )
            {
              strcpy( pData->szInputMem, FindData.cFileName  );
            }
            else
            {
              SplitFnameFromPath( pData->szInputMem );
              strcat( pData->szInputMem, "\\" );
              strcat( pData->szInputMem, FindData.cFileName  );
            } /* endif */

            // construct output file if something has been specified
            if ( pData->szOutSpec[0] )
            {
              strcpy( pData->szOutputPath, pData->szOutSpec );
              SplitFnameFromPath( pData->szOutputPath );
              strcat( pData->szOutputPath, "\\" );
            }
            else
            {
              strcpy( pData->szOutputPath, "" );
            } /* endif */
            pData->szOutputMem[0] = 0;
            MakeOutputName( pData, pData->szInputMem, pData->szOutputPath, pData->szOutputMem );

            usRC = CleanSingleMemory( pData, pData->szInputMem, pData->szOutputMem );
          } /* endif */

          fMoreFiles = FindNextFile( hdir, &FindData  );
        } /* endwhile */

        FindClose( hdir );
      } /* endif */

    }
    else
    {
      if ( pData->szOutSpec[0]  )
      {
        strcpy( pData->szOutputMem, pData->szOutSpec );
      }
      else
      {
        strcpy( pData->szOutputPath, "" );
        MakeOutputName( pData, pData->szInSpec, pData->szOutputPath, pData->szOutputMem );
      } /* endif */

      usRC = CleanSingleMemory( pData, pData->szInSpec, pData->szOutputMem );
    } /* endif */
  } /* endif */

  if ( pData )
  {
    free( pData );
  } /* endif */

  return( usRC );
} /* end of function main */


// function cleaning a single EXP memory 
USHORT CleanSingleMemory
(
  PWORKAREA        pData,              // Pointer to our data area
  PSZ              pszInMemory,        // fully qualified name of input memory 
  PSZ              pszOutMemory        // fully qualified name of output memory
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  pData->ulSegments = 0;

  // check existence of input file
  if ( usRC == NO_ERROR) usRC = CheckExistence( pData, pszInMemory );

  // open input memory
  if ( usRC == NO_ERROR )
  {
    pData->hInFile = fopen( pszInMemory, "rb" );
    if ( pData->hInFile == NULL )
    {
      int iRC = 0;
      _get_errno( &iRC );
      sprintf( pData->szBuffer, "Can't open input memory %s, return code is %ld\n.", pszInMemory, iRC ); 
      ShowError( pData, "%s", pData->szBuffer );
    } /* endif */
  }/* endif */

  // open output memory
  if ( usRC == NO_ERROR )
  {
    pData->hOutFile = fopen( pszOutMemory, "wb" );
    if ( pData->hOutFile == NULL )
    {
      int iRC = 0;
      _get_errno( &iRC );
      sprintf( pData->szBuffer, "Can't open output memory %s, return code is %ld\n.", pszOutMemory, iRC ); 
      ShowError( pData, "%s", pData->szBuffer );
    } /* endif */
  }/* endif */

  // get encoding of memory
  if ( usRC == NO_ERROR ) GetMemEncoding( pData->hInFile, &(pData->fUnicode) );

  // process the memory
  if ( usRC == NO_ERROR )
  {
    if ( pData->fUnicode )
    {
      usRC = ProcessMemoryW( pData );
    }
    else
    {
      usRC = ProcessMemory( pData );
    } /* endif */
  }/* endif */

  // close our files
  if ( pData->hInFile != NULL )
  {
    fclose( pData->hInFile );
    pData->hInFile = NULL;
  } /* endif */
  if ( pData->hOutFile != NULL )
  {
    fclose( pData->hOutFile );
    pData->hOutFile = NULL;
  } /* endif */

  if ( usRC == NO_ERROR)
  {
    sprintf( pData->szBuffer, "Cleaned memory %s, %lu segments have been processed and written to the output memory %s.",
      pszInMemory, pData->ulSegments, pszOutMemory ); 
    ShowInfo( pData, "%s", pData->szBuffer );
  }
  else
  {
    sprintf( pData->szBuffer, "Cleanup of memory %s into EXP memory %s failed, rc=%u.", pszInMemory, pszOutMemory, usRC ); 
    ShowError( pData, "%s", pData->szBuffer );
  } /* endif */
  return( usRC );
} /* end of function CleanSingleMemory */

// check existence of input file
USHORT CheckExistence( PWORKAREA pData, PSZ pszInMemory )
{
  USHORT      usRC = NO_ERROR;         // function return code
  HANDLE hdir;
  static WIN32_FIND_DATA FindData;    // find data structure

  hdir = FindFirstFile( pszInMemory, &FindData );
  if ( hdir == INVALID_HANDLE_VALUE )
  {
     usRC = ERROR_FILE_NOT_FOUND;
     ShowError( pData, "Input memory %s could not be opened", pszInMemory );
  }
  else
  {
    FindClose( hdir );
  } /* endif */

  return( usRC );
} /* end of function CheckExistence */

// show error message
USHORT ShowError( PWORKAREA pData, PSZ pszMsg, PSZ pszParm )
{
  USHORT      usRC = NO_ERROR;         // function return code

  sprintf( pData->szMsgBuffer, pszMsg, pszParm );

  printf( "Error: %s\n\n", pData->szMsgBuffer );

  return( usRC );
} /* end of function ShowError */

USHORT ShowInfo(PWORKAREA pData, PSZ pszMsg, PSZ pszParm )
{
  USHORT      usRC = NO_ERROR;         // function return code

  sprintf( pData->szMsgBuffer, pszMsg, pszParm );

  printf( "Info: %s\n", pData->szMsgBuffer );

  return( usRC );
} /* end of function ShowInfo */

// check output memory name 
USHORT MakeOutputName( PWORKAREA pData, PSZ pszInMemory, PSZ pszOutputPath, PSZ pszOutMemory )
{
  USHORT      usRC = NO_ERROR;         // function return code

  pData;

  if ( (pszOutMemory == NULL) || (*pszOutMemory == 0) )
  {
    if ( pszOutputPath && *pszOutputPath )
    {
      // use provided output path
      strcpy( pszOutMemory, pszOutputPath );
      if ( pszOutMemory[strlen(pszOutMemory)-1] != '\\' ) strcat ( pszOutMemory, "\\" );

      // append input memory name
      {
        PSZ pszName = strrchr( pszInMemory, '\\' );
        strcat( pszOutMemory, (pszName == NULL) ? pszInMemory : pszName + 1 );
      }
    }
    else
    {
      // use input memory name
      strcpy( pszOutMemory, pszInMemory );
    } /* endif */

    // cut of file extension
    {
      PSZ pszExt = strrchr( pszOutMemory, '.' );
      if ( pszExt != NULL ) *pszExt = 0;
    }

    // add name suffix depending on selected options
    if ( pData->fReduceLF ) strcat( pszOutMemory, "_1L" ); 
    if ( pData->fRemoveTM ) strcat( pszOutMemory, "_NOTM" ); 

    // add file extension
    strcat( pszOutMemory, ".EXP" );
  }
  else
  {
    strcpy( pszOutMemory, pszOutMemory );
  } /* endif */

  return( usRC );
} /* end of function */

void showHelp()
{
    printf( "OtmCleanMem.EXE  : EXP Memory cleanup tool\n" );
    printf( "Version          : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright        : %s\n",STR_COPYRIGHT );
    printf( "Purpose          : Reduce line breaks within segment data and/or remove trademark tags in an external memory in EXP format\n" );
    printf( "Syntax format    : OtmCleanMem /IN=|INPUTMEM=inputmem [/OUT=|/OUTPPUTMEM=outputmem] [/OP=|/OPTIONS=options]\n" );
    printf( "Options and parameters:\n" );
    printf( "    inputmem      the fully qualified name of the input memory (this name can contain wildcards)\n" );
    printf( "    outputmem     the fully qualified name of the output memory\n" );
    printf( "                  if not specified the input memory name suffixed with the selected options is being used\n" );
    printf( "    options       one or more of the following processing options\n" );
    printf( "        1L        reduce linebreaks in the segment text thus forming a single line of text\n" );
    printf( "        NOTM      remove trademark tags (\"<tm ...>\" and \"</tm>\") from segment text\n" );
    printf( "                  when more than one option is being used, the options have to be separated using a comma or the plus sign (e.g. \"1L+NOTM\" or \"1L,NOTM\")\n" );
    printf( "                  when no options are specified \"1L+NOTM\" is used as default\n" );
}

void GetOptions( PWORKAREA pData, PSZ pszValue )
{
  pData->fReduceLF = FALSE;
  pData->fRemoveTM = FALSE;

  while ( *pszValue )
  {
    // skip whitespace
    while (  iswspace( *pszValue ) ) pszValue++;

    // find end of current option
    PSZ pszEnd = pszValue;
    while ( *pszEnd && (*pszEnd != ',') && (*pszEnd != '+') ) pszEnd++;

    // handle current option
    if ( pszEnd > pszValue )
    {
      int iLen = pszEnd - pszValue;
      strncpy( pData->szBuffer, pszValue, iLen );
      pData->szBuffer[iLen] = 0;

      if ( stricmp( pData->szBuffer, "1L" ) == 0 )
      {
        pData->fReduceLF = TRUE;
      }
      else if ( stricmp( pData->szBuffer, "NOTM" ) == 0 )
      {
        pData->fRemoveTM = TRUE;
      }
      else
      {
        printf( "Info: unknown option \"%s\" is ignored\n", pData->szBuffer );
      } /* endif */
    } /* endif */

    pszValue = (*pszEnd) ? (pszEnd + 1) : pszEnd;

    // skip whitespace
    while (  iswspace( *pszValue ) ) pszValue++;
  } /* endwhile */

  return;
}

PSZ SplitFnameFromPath( PSZ path)
{
    char *p;
    if( (p = strrchr( path, '\\')) != NULL )
       *p++ = '\0';
    return( p);
}

void GetMemEncoding( FILE *hf, PBOOL pfUnicode)
{
  BYTE bBuffer[20];

  memset( bBuffer, 0, sizeof(bBuffer) );
  fread( bBuffer, 20, 1, hf );
  fseek( hf, 0, SEEK_SET );

  *pfUnicode = (memcmp( bBuffer, "\xFF\xFE", 2 ) == 0) || (memcmp( bBuffer, L"<ntm", 8 ) == 0) ;
}

USHORT ProcessMemoryW( PWORKAREA pData )
{
  USHORT usRC = 0;

  while ( !usRC && !feof( pData->hInFile ) )
  {
    wchar_t *pszTag;

    memset( pData->szLineW, 0, sizeof(pData->szLineW) );
    fgetws( pData->szLineW, sizeof(pData->szLineW) / sizeof(wchar_t), pData->hInFile );

    pszTag = wcsstr( pData->szLineW, szSourceTagW );
    if ( pszTag != NULL )
    {
      // handle source text 
      wchar_t *pszLineStart;


      // write line up to and including source tag
      fwrite( pData->szLineW, (pszTag - pData->szLineW) + wcslen(szSourceTagW), sizeof(wchar_t), pData->hOutFile );

      // collect segment text up to source end tag
      pszLineStart = pszTag + wcslen(szSourceTagW);
      pszTag = NULL;
      pData->szSegDataW[0] = 0;
      while ( !feof( pData->hInFile ) && (pszTag == NULL) )
      {
        pszTag = wcsstr( pszLineStart, szEndSourceTagW );
        if ( pszTag == NULL )
        {
          wcscat( pData->szSegDataW, pszLineStart );
          memset( pData->szLineW, 0, sizeof(pData->szLineW) );
          fgetws( pData->szLineW, sizeof(pData->szLineW) / sizeof(wchar_t), pData->hInFile );
          pszLineStart = pData->szLineW;
        }
        else
        {
          wchar_t chTemp = *pszTag;
          *pszTag = 0;
          wcscat( pData->szSegDataW, pszLineStart );
          *pszTag = chTemp;
        } /* endif */
      } /* endwhile */

      // process segment text
      ProcessSegmentDataW( pData );

      // write segment text to output file
      fwrite( pData->szSegDataW, wcslen(pData->szSegDataW), sizeof(wchar_t), pData->hOutFile );

      // write source end tag
      if ( pszTag != NULL ) fputws( pszTag, pData->hOutFile );

      pData->ulSegments++;
    }
    else
    {
      pszTag = wcsstr( pData->szLineW, szTargetTagW );
      if ( pszTag != NULL )
      {
        // handle target text 

        wchar_t *pszLineStart;


        // write line up to and including target tag
        fwrite( pData->szLineW, (pszTag - pData->szLineW) + wcslen(szTargetTagW), sizeof(wchar_t), pData->hOutFile );

        // collect segment text up to target end tag
        pszLineStart = pszTag + wcslen(szTargetTagW);
        pszTag = NULL;
        pData->szSegDataW[0] = 0;
        while ( !feof( pData->hInFile ) && (pszTag == NULL) )
        {
          pszTag = wcsstr( pszLineStart, szEndTargetTagW );
          if ( pszTag == NULL )
          {
            wcscat( pData->szSegDataW, pszLineStart );
            memset( pData->szLineW, 0, sizeof(pData->szLineW) );
            fgetws( pData->szLineW, sizeof(pData->szLineW) / sizeof(wchar_t), pData->hInFile );
            pszLineStart = pData->szLineW;
          }
          else
          {
            wchar_t chTemp = *pszTag;
            *pszTag = 0;
            wcscat( pData->szSegDataW, pszLineStart );
            *pszTag = chTemp;
          } /* endif */
        } /* endwhile */

        // process segment text
        ProcessSegmentDataW( pData );

        // write segment text to output file
        fwrite( pData->szSegDataW, wcslen(pData->szSegDataW), sizeof(wchar_t), pData->hOutFile );

        // write target end tag
        if ( pszTag != NULL ) fputws( pszTag, pData->hOutFile );
      }
      else
      {
        // hande any other data
        if ( pData->szLineW[0] != 0 ) fputws( pData->szLineW, pData->hOutFile );
      } /* endif */
    } /* endif */

  } /* endwhile */
  return( usRC );
}

USHORT ProcessMemory( PWORKAREA pData )
{
  USHORT usRC = 0;

  while ( !usRC && !feof( pData->hInFile ) )
  {
    char *pszTag;

    memset( pData->szLine, 0, sizeof(pData->szLineW) );
    fgets( pData->szLine, sizeof(pData->szLine), pData->hInFile );

    pszTag = strstr( pData->szLine, szSourceTag );
    if ( pszTag != NULL )
    {
      // handle source text 

      BOOL fDone = FALSE;
      char *pszLineStart;


      // write line up to and including source tag
      fwrite( pData->szLine, (pszTag - pData->szLine) + strlen(szSourceTag), 1, pData->hOutFile );

      // collect segment text up to source end tag
      pszLineStart = pszTag + strlen(szSourceTag);
      pszTag = NULL;
      pData->szSegData[0] = 0;
      while ( !feof( pData->hInFile ) && (pszTag == NULL) )
      {
        pszTag = strstr( pszLineStart, szEndSourceTag );
        if ( pszTag == NULL )
        {
          strcat( pData->szSegData, pszLineStart );
          memset( pData->szLine, 0, sizeof(pData->szLine) );
          fgets( pData->szLine, sizeof(pData->szLine), pData->hInFile );
          pszLineStart = pData->szLine;
        }
        else
        {
          char chTemp = *pszTag;
          *pszTag = 0;
          strcat( pData->szSegData, pszLineStart );
          *pszTag = chTemp;
        } /* endif */
      } /* endwhile */

      // process segment text
      ProcessSegmentData( pData );

      // write segment text to output file
      fwrite( pData->szSegData, strlen(pData->szSegData), 1, pData->hOutFile );

      // write source end tag
      if ( pszTag != NULL ) fputs( pszTag, pData->hOutFile );

      pData->ulSegments++;
    }
    else
    {
      pszTag = strstr( pData->szLine, szTargetTag );
      if ( pszTag != NULL )
      {
        // handle target text 

        BOOL fDone = FALSE;
        char *pszLineStart;


        // write line up to and including target tag
        fwrite( pData->szLine, (pszTag - pData->szLine) + strlen(szTargetTag), 1, pData->hOutFile );

        // collect segment text up to target end tag
        pszLineStart = pszTag + strlen(szTargetTag);
        pszTag = NULL;
        pData->szSegData[0] = 0;
        while ( !feof( pData->hInFile ) && (pszTag == NULL) )
        {
          pszTag = strstr( pszLineStart, szEndTargetTag );
          if ( pszTag == NULL )
          {
            strcat( pData->szSegData, pszLineStart );
            memset( pData->szLine, 0, sizeof(pData->szLine) );
            fgets( pData->szLine, sizeof(pData->szLine), pData->hInFile );
            pszLineStart = pData->szLine;
          }
          else
          {
            char chTemp = *pszTag;
            *pszTag = 0;
            strcat( pData->szSegData, pszLineStart );
            *pszTag = chTemp;
          } /* endif */
        } /* endwhile */

        // process segment text
        ProcessSegmentData( pData );

        // write segment text to output file
        fwrite( pData->szSegData, strlen(pData->szSegData), 1, pData->hOutFile );

        // write target end tag
        if ( pszTag != NULL ) fputs( pszTag, pData->hOutFile );
      }
      else
      {
        // hande any other data
        if ( pData->szLine[0] != 0 ) fputs( pData->szLine, pData->hOutFile );
      } /* endif */
    } /* endif */

  } /* endwhile */
  return( usRC );
}

USHORT ProcessSegmentDataW( PWORKAREA pData )
{
  USHORT usRC = 0;

  if ( pData->fReduceLF )
  {
    wchar_t *pszSource = pData->szSegDataW;
    wchar_t *pszTarget = pData->szSegDataW;

    while ( *pszSource )
    {
      if ( *pszSource == L'\r' )
      {
        // ignore CR
        pszSource++;
      }
      else if ( *pszSource == L'\n' )
      {
        // replace LF by blank when not at the end of the segment data
        if ( pszSource[1] != 0 ) *pszTarget++ = L' ';
        pszSource++;
      }
      else
      {
        // copy character to target
        *pszTarget++ = *pszSource++;
      } /* endif */
    } /* endwhile */
    *pszTarget = 0;
  } /* endif */

  if ( pData->fRemoveTM )
  {
    wchar_t *pszSource = pData->szSegDataW;
    wchar_t *pszTarget = pData->szSegDataW;
    BOOL fIgnore = FALSE;

    while ( *pszSource )
    {
      if ( fIgnore )
      {
        if ( *pszSource == L'>' ) fIgnore = FALSE;
        pszSource++;
      }
      else if ( wcsnicmp( pszSource, L"<tm ", 4 ) == 0 )
      {
        fIgnore = TRUE;
        pszSource += 4;
      }
      else if ( wcsnicmp( pszSource, L"</tm", 4 ) == 0 )
      {
        fIgnore = TRUE;
        pszSource += 4;
      }
      else 
      {
        // copy character to target
        *pszTarget++ = *pszSource++;
      } /* endif */


    } /* endwhile */
    *pszTarget = 0;
  } /* endif */

  return( usRC );
}

USHORT ProcessSegmentData( PWORKAREA pData )
{
  USHORT usRC = 0;

  if ( pData->fReduceLF )
  {
    char *pszSource = pData->szSegData;
    char *pszTarget = pData->szSegData;

    while ( *pszSource )
    {
      if ( *pszSource == '\r' )
      {
        // ignore CR
        pszSource++;
      }
      else if ( *pszSource == '\n' )
      {
        // replace LF by blank when not at the end of the segment data
        if ( pszSource[1] != 0 ) *pszTarget++ = ' ';
        pszSource++;
      }
      else
      {
        // copy character to target
        *pszTarget++ = *pszSource++;
      } /* endif */
    } /* endwhile */
    *pszTarget = 0;
  } /* endif */

  if ( pData->fRemoveTM )
  {
    char *pszSource = pData->szSegData;
    char *pszTarget = pData->szSegData;
    BOOL fIgnore = FALSE;

    while ( *pszSource )
    {
      if ( fIgnore )
      {
        if ( *pszSource == '>' ) fIgnore = FALSE;
        pszSource++;
      }
      else if ( strnicmp( pszSource, "<tm ", 4 ) == 0 )
      {
        fIgnore = TRUE;
        pszSource += 4;
      }
      else if ( strnicmp( pszSource, "</tm", 4 ) == 0 )
      {
        fIgnore = TRUE;
        pszSource += 4;
      }
      else 
      {
        // copy character to target
        *pszTarget++ = *pszSource++;
      } /* endif */


    } /* endwhile */
    *pszTarget = 0;
  } /* endif */

  return( usRC );
}