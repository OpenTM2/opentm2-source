//+----------------------------------------------------------------------------+
//| TMXSource2Text.C                                                         |
//+----------------------------------------------------------------------------+
// Copyright (C) 2012-2015, International Business Machines
// Corporation and others.  All rights reserved.
//
#undef  DLL
#undef  _WINDLL
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <windows.h>
#include <eqfserno.h>

#define MAX_LINE_LEN 10000


char szInLineUTF8[MAX_LINE_LEN+1];   // buffer for input line in UTF8 encoding
wchar_t szInLine[MAX_LINE_LEN+1];   // buffer for input line
wchar_t szOrgLine[MAX_LINE_LEN+1];   // buffer for input line
wchar_t szBuffer[MAX_LINE_LEN+1];   // buffer for un-escaping special characters

static void showHelp();

/// un-escape data and write to output file
void WriteToOutput( FILE *hfOut, wchar_t *pszData )
{
  wchar_t *pszSource = pszData;
  wchar_t *pszTarget = szBuffer;

  while ( *pszSource )
  {
    if ( *pszSource == L'&' )
    {
      if ( wcsncmp( pszSource, L"&lt;", 4 ) == 0 )
      {
        *pszTarget++ = L'<';
        pszSource += 4;
      }
      else if ( wcsncmp( pszSource, L"&gt;", 4 ) == 0 )
      {
        *pszTarget++ = L'>';
        pszSource += 4;
      }
      else
      if ( wcsncmp( pszSource, L"&amp;", 5 ) == 0 )
      {
        *pszTarget++ = L'&';
        pszSource += 5;
      }
      else
      if ( wcsncmp( pszSource, L"&quot;", 6 ) == 0 )
      {
        *pszTarget++ = L'\"';
        pszSource += 6;
      }
      else
      {
        *pszTarget++ = *pszSource++;
      } /* endif */         
    }
    else
    {
      *pszTarget++ = *pszSource++;
    } /* endif */       
  } /* endwhile */     
  *pszTarget = 0;

  fwrite( szBuffer, wcslen(szBuffer), 2, hfOut );
}




int main
(
  int  argc,
  char *argv[],
  char *envp[]
)
{
  char *pszInFile = NULL;
  char *pszOutFile = NULL;
  char *pszLogFile = "TmxSource2Text.log";
  int iSegments = 0;
  int iIgnored = 0;
  int iLine = 0;
  BOOL fOK = TRUE;
  FILE *hf = NULL;
  FILE *hfOut = NULL;
  FILE *hfLog = NULL;
  int i = 0;
  BOOL fUTF8 = FALSE;

  envp;

  if(argc==2 && stricmp(argv[1],"-h")==0)
  {
      showHelp();
      return 0;
  }
  /* Show title line                                                  */
  printf( "TmxSource2Text v%s\nExtract source text of the segments in a TMX memory into a text file\n\n", STR_DRIVER_LEVEL_NUMBER );

  argc--;
  argv++;

  /********************************************************************/
  /* Check commandline arguments                                      */
  /********************************************************************/
  while ( fOK && argc )
  {
    PSZ pszArg;

    pszArg = *argv;
    if ( (pszArg[0] == '-') || (pszArg[0] == '/') )
    {
      /****************************************************************/
      /* Check for options                                            */
      /****************************************************************/
  //    if ( strnicmp( pszArg + 1, "CP=", 3 ) == 0 )
  //    {
  //      uiCP = atoi( pszArg+4 );
  //    }
  //    else
      {
        printf( "Warning: Unknown commandline switch '%s' is ignored.\n", pszArg );
      } /* endif */
    }
    else
    {
      if ( pszInFile == NULL )
      {
        pszInFile = pszArg;
      }
      else if ( pszOutFile == NULL )
      {
        pszOutFile = pszArg;
      }
      else
      {
        printf( "Warning: Superfluous commandline value '%s' is ignored.\n",
                pszArg );
      } /* endif */
    } /* endif */
    argc--;
    argv++;
  } /* endwhile */


  if ( pszInFile == NULL )
  {
    printf( "Error: No input file specified.\n\n" );
	printf( "TmxSource2Text : TMX Source text extractor\n" );
	printf( "Version        : %s\n", STR_DRIVER_LEVEL_NUMBER );
	printf( "Copyright      : %s\n",STR_COPYRIGHT );
	printf( "Purpose        : Extract source text of the segments in a TMX memory into a text file\n" );
	printf( "Syntax format  : TmxSource2Text infile [outfile]\n" );
	printf( "\n" );
	printf( "Options and parameters:\n" );
    printf( "   \'infile\' is the fully qualified name of the Translation Memory in TMX format\n" );
    printf( "   \'outfile\' is the fully qualified name of the output text file\n" );
    fOK = FALSE;
    return( 1 );
  } /* endif */

  if ( fOK && (pszOutFile == NULL ) )
  {
    pszOutFile = "OutFile.txt";
    printf( "Info: No output file specified, using %s as default\n\n", pszOutFile );
  } /* endif */

  // open files
  if ( fOK )
  {
    hf = fopen( pszInFile, "rb" );
    if ( hf == NULL )
    {
      printf( "Error: Could not open file %s\n", pszInFile );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    hfOut = fopen( pszOutFile, "wb" );
    if ( hfOut == NULL )
    {
      printf( "Error: Could not open output file %s\n", pszOutFile );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // open log file
  if ( fOK  )
  {
    hfLog = fopen( pszLogFile, "wb" );
    if ( hfLog != NULL )
    {
      struct tm *newtime;
      time_t aclock;

      fwprintf( hfLog, L"%cTmxSource2Text Log File\r\n\r\n", L'\xFEFF' );
      time( &aclock );
      newtime = localtime( &aclock );  
      fwprintf( hfLog, L"Date/Time  : %s\r\n", _wasctime(newtime) );
      fwprintf( hfLog, L"Input file : %S\r\n", pszInFile );
      fwprintf( hfLog, L"Output file: %S\r\n\r\n", pszOutFile );
    } /* endif */       
  } /* endif */     

  // check for UTF8 or UTF16 encoding of input file
  if ( fOK )
  {
    BYTE bTestBuffer[80];
    PSZ pszTest = (PSZ)bTestBuffer;
    wchar_t *pszTestW = (wchar_t *)bTestBuffer;

    fread( bTestBuffer, 1, 80, hf );
    bTestBuffer[78] = 0; 
    bTestBuffer[79] = 0; 
    if ( strstr( pszTest, "xml"  ) )
    {
      fUTF8 = TRUE;
    }
    else if ( wcsstr( pszTestW, L"xml"  ) )
    {
      fUTF8 = FALSE;
    }
    else
    {
      printf( "Error: Could not determine format of input file %s\n", pszInFile );
      fOK = FALSE;
    } /* endif */         
    fseek( hf, 0, SEEK_SET );
  } /* endif */     


  // process file contents
  if ( fOK )
  {
    enum { LookingForTu, LookingForSeg, LookingForEndSeg } mode = LookingForTu;

    // write BOM to output file
    fwrite( L"\xFEFF", 1, 2, hfOut );

    if ( fUTF8 )
    {
      memset( szInLineUTF8, 0, sizeof(szInLineUTF8) );
      fgets( szInLineUTF8, MAX_LINE_LEN, hf );
      MultiByteToWideChar( CP_UTF8, 0, szInLineUTF8, -1, szInLine, MAX_LINE_LEN );
    }
    else
    {
      memset( szInLine, 0, sizeof(szInLine) );
      fgetws( szInLine, MAX_LINE_LEN, hf );
    } /* endif */       


    // skip any BOM
    if ( szInLine[0] == L'\xFEFF' )
    {
      wchar_t *pszSource = szInLine + 1;
      wchar_t *pszTarget = szInLine;
      while( *pszSource != 0 ) *pszTarget++ = *pszSource++;
      *pszTarget = 0;
    } /* endif */    

    while ( !feof( hf ) )
    {
      int iLen = 0;

      iLine++;
      wcscpy( szOrgLine, szInLine );

      // remove trailing LF
      iLen = wcslen(szInLine); 
      if ( szInLine[iLen-1] == L'\n' )
      {
        iLen--;
        szInLine[iLen] = 0; 
      } /* endif */         
      if ( szInLine[iLen-1] == L'\r' )
      {
        iLen--;
        szInLine[iLen] = 0; 
      } /* endif */         

      // process line data
      {
        wchar_t *pszPos = szInLine;

        if ( mode == LookingForTu  )
        {
          wchar_t *pszFound = wcsstr( pszPos, L"<tu ");
          if ( pszFound != NULL )
          {
            pszPos = pszFound;
            mode = LookingForSeg;
          } /* endif */         
        } /* endif */         

        if ( mode == LookingForSeg  )
        {
          wchar_t *pszFound = wcsstr( pszPos, L"<seg>" );
          if ( pszFound != NULL )
          {
            iSegments++;
            pszPos = pszFound + wcslen(L"<seg>");
            mode = LookingForEndSeg;
          } /* endif */         
        } /* endif */         

        if ( mode == LookingForEndSeg  )
        {
          wchar_t *pszFound = wcsstr( pszPos, L"</seg>" );
          if ( pszFound != NULL )
          {
            // write data up to </seg> to output file
            *pszFound = 0;
            WriteToOutput( hfOut, pszPos );
            fputws( L"\r\n", hfOut );
            mode = LookingForTu;
          }
          else
          {
            WriteToOutput( hfOut, pszPos );
            fputws( L" ", hfOut );
          } /* endif */         
        } /* endif */         
      }


      // next line
      if ( fUTF8 )
      {
        memset( szInLineUTF8, 0, sizeof(szInLineUTF8) );
        fgets( szInLineUTF8, MAX_LINE_LEN, hf );
        MultiByteToWideChar( CP_UTF8, 0, szInLineUTF8, -1, szInLine, MAX_LINE_LEN );
      }
      else
      {
        memset( szInLine, 0, sizeof(szInLine) );
        fgetws( szInLine, MAX_LINE_LEN, hf );
      } /* endif */       
    } /* endwhile */       
  } /* endif */

 // 
 printf( "%ld segments processed\n", iSegments );
 //if ( iIgnored != 0 )
 //{
 //  printf( "see log file %s for details\n", pszLogFile );
 //} /* endif */    
 if ( hfLog != NULL )
 {
   fwprintf( hfLog, L"\r\n%ld segments processed\r\n", iSegments );
 } /* endif */           

 if ( hf != NULL ) fclose( hf );
 if ( hfOut != NULL ) fclose( hfOut );
 if ( hfLog != NULL ) fclose( hfLog );

  // return to system
  return( !fOK );
} /* end of main */

void showHelp()
{
    printf( "OtmTmxSource2Text : TMX Source text extractor\n" );
    printf( "Version        :%s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright      : %s\n", STR_COPYRIGHT);
    printf( "Purpose        : Extract source text of the segments in a TMX memory into a text file\n" );
    printf( "Syntax format  : OtmTmxSource2Text infile [outfile]\n" );
    printf( "Options and parameters:\n" );
    printf( "   infile     the fully qualified name of the Translation Memory in TMX format\n" );
    printf( "   outfile    the fully qualified name of the output text file\n" );
}
