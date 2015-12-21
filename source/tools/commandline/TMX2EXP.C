//+----------------------------------------------------------------------------+
//| TMX2EXP.C                                                                 |
//+----------------------------------------------------------------------------+
// Copyright (C) 2012-2015, International Business Machines
// Corporation and others.  All rights reserved.
//
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: TMX format to EXP format converter                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_EDITORAPI        // for EQFWORDCOUNTPERSEGW
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#include <eqf.h>                  // General Translation Manager include file
#include <eqfserno.h>             // version number
#include "OTMFUNC.H"

#include <time.h>                 // C library for time functions
#include "EQFDDE.H"               // Batch mode definitions
#define INCL_EQFMEM_DLGIDAS       // include dialog IDA definitions
#include "EQFTMI.H"               // Private header file of Translation Memory
#include "EQFMEM.ID"              // PM IDs for Translation Memory
#include <EQFQDAM.H>              // Low level TM access functions
  #include "OTMFUNC.H"            // function call interface public defines
  #include "eqffunci.h"           // function call interface private defines
#include "EQFHLOG.H"              // for word count category limits

#include "EQFMEMIE.H"

#ifdef _DEBUG
#endif


// data types 

// size of input buffers
#define BUF_SIZE 8096
#define TMXTOKBUFSIZE 32000

typedef struct _TMX2EXP_DATA
{
  //-- data for input memory processing --
  CHAR        szInSpec[MAX_LONGFILESPEC];        // input memory specification of user
  CHAR        szInputFile[MAX_LONGFILESPEC];     // buffer for preperation of input memory name
  CHAR        szInMemory[MAX_LONGFILESPEC];      // fully qualified name current input memory
  CHAR        szBuffer[4096];                    // general purpose buffer
  BOOL        fUnicode;                          // TRUE = input memory is in UTF-16 format
  FILE        *hInFile;                          // handle of input file
  CHAR_W      szLine[4096];                      // buffer for input line
  BYTE        bReadBuffer[16000];                // file read/write buffer (must be large enough to contain complete memory segment)
  CHAR_W      chInBufW[BUF_SIZE];                // data buffer for read of Unicode data
  CHAR        chInBuf[BUF_SIZE];                 // data buffer for read of ANSI data
  int         iInBufProcessed;                   // number of processed characters in chInBuf
  int         iInBufRead;                        // number characters read into chInBuf
  ULONG       ulInputCP;                         // codepage to use for import when importing non-Unicode memory
  BOOL        fAnsi;                             // TRUE = input in ASCII format / FALSE = input in ASCII mode
  CHAR        szInMode[200];                     // buffer for input mode
  CHAR        szInMarkup[8000];                  // markup or list of markups to be used for segments
  BOOL        fCleanRTF;                         // TRUE = clean/remove RTF tags 
  BOOL        fTagsInCurlyBracesOnly;            // TRUE = remove RTF tags in curly braces only
  BOOL        fSourceSource;                     // TRUE = convert to source/source memory

  //-- data for output memory processing --
  CHAR        szOutSpec[MAX_LONGFILESPEC];       // output memory specification of user
  CHAR        szOutputFile[MAX_LONGFILESPEC];    // buffer for preperation of output memory name
  CHAR        szOutMemory[MAX_LONGFILESPEC];     // fully qualified name of current output memory
  FILE        *hOutFile;                         // handle of output file
  BOOL        fUTF16Output;                      // TRUE = UTF16, FALSE = UTF8 
  CHAR        szOutMode[200];                    // buffer for output mode
  CHAR        szUtf8Buffer[16000];               // buffer for UTF8 conversion
  CHAR_W      szOutBuffer[16000];                // output buffer 
  CHAR_W      szEscapeBuffer[16000];             // buffer for character escaping 
  LONG        lOutMode;                          // output mode ASNSI_OPT, ASCII_OPT, UTF16_OPT

  //-- segment data --
  ULONG       ulSegNum;                          // number of currently active segment
  LONG        lTime;                             // segment update time
  CHAR_W      szSegStart[1024];                  // buffer for segment start string
  CHAR_W      szControl[1024];                   // buffer for control string
  CHAR        szControlAscii[1024];              // ASCII version of control string
  CHAR_W      szSource[4096];                    // buffer for segment source
  CHAR_W      szTarget[4096];                    // buffer for segment target
  BOOL        fMachineTranslation;               // machine translation flag
  CHAR_W      szDataType[100];                   // datatype of segment
  CHAR_W      szDocName[1024];                   // document name 
  CHAR_W      szMarkup[4000];                    // TM markup or list of markup tables
  CHAR_W      szSourceLang[100];                 // TM source language
  CHAR_W      szTargetLang[100];                 // TM target language
  CHAR_W      szTime[100];                       // segment creation time in TMX format


  // - statistics --
  ULONG       ulSegments;                        // number of segments written to output memory

  // -- other stuff --
  PLOADEDTABLE pLoadedTable;                     // pointer to currently loaded markup table
  PTOKENENTRY  pTokBuf;                          // buffer for TaTagTokenize tokens
  CHAR_W       szActiveTagTable[100];            // buffer for name of loaded tag table
  CHAR         szMsgBuffer[2048];                // buffer for message texts
  BOOL         fLog;                             // true = write messages to log file
} TMX2EXP_DATA, *PTMX2EXP_DATA;



// prototypes 
USHORT CheckExistence( PTMX2EXP_DATA pData, PSZ pszInMemory );
USHORT GetEncoding( PTMX2EXP_DATA pData, PSZ pszInMemory, PSZ pszInMode );
USHORT CheckOutputName( PTMX2EXP_DATA pData, PSZ pszInMemory, PSZ pszOutMemory );
USHORT PrepareOutput( PTMX2EXP_DATA pData, PSZ pszOutMemory, PSZ pszOutMode );
USHORT GetNextSegment( PTMX2EXP_DATA pData, PBOOL pfSegmentAvailable );
USHORT WriteSegment( PTMX2EXP_DATA pData );
USHORT CloseInput( PTMX2EXP_DATA pData );
USHORT TerminateOutput( PTMX2EXP_DATA pData );
USHORT ShowError( PTMX2EXP_DATA pData, PSZ pszMsg, PSZ pszParm );
USHORT ShowInfo( PTMX2EXP_DATA pData, PSZ pszMsg, PSZ pszParm );
USHORT FillBufferW( PTMX2EXP_DATA pData );
USHORT FillBuffer( PTMX2EXP_DATA pData );
USHORT ReadLineW( PTMX2EXP_DATA pData, PSZ_W pszLine, int iSize );
USHORT ReadLine( PTMX2EXP_DATA pData, PSZ pszLine, int iSize );
USHORT GetLine( PTMX2EXP_DATA pData, PSZ_W pszLine, int iSize, BOOL fUnicode, PBOOL pfEOF );
void EscapeCharacters( PSZ_W pszName, PSZ_W pszBuffer );
PSZ_W ParseX15W( PSZ_W pszX15String, SHORT sStringId );
BOOL StripTag( PSZ_W pszLine, PSZ_W pszTag );
USHORT WriteTUV( PTMX2EXP_DATA pData, PSZ_W pszLanguage, PSZ_W pszMarkup, PSZ_W pszSegmentData );
void GetTypeForMarkup( PSZ_W pszMarkup, PSZ_W pszDataType );
void GetIsoLangForTMLang( PSZ_W pszTMLang, PSZ_W pszIsoLang );
USHORT ConvertSingleMemory( PTMX2EXP_DATA pData, PSZ pszInMemory, PSZ pszOutMemory );
USHORT WriteToOutMem( PTMX2EXP_DATA pData, PSZ_W pszString );

static void showHelp();

// main entry point
int main( int argc, char *argv[], char *envp[] )
{
  USHORT   usRC = 0;
  PTMX2EXP_DATA pData = NULL;
  HSESSION hSession = 0L;

  envp;

  if(argc==2 && stricmp(argv[1],"-h")==0)
  {
      showHelp();
      return 0;
  }
  // show logo
  printf( "TMX2EXP - The TMX to EXP memory converter\n\n" );


  // allocate our data area
  pData = (PTMX2EXP_DATA)malloc( sizeof(TMX2EXP_DATA) );
  if ( pData == NULL )
  {
    printf( "Error: Memory allocation error, program terminated" );
    return( -1 );
  } /* endif */

  memset( pData, 0, sizeof(TMX2EXP_DATA) );
  pData->pTokBuf = (PTOKENENTRY)malloc( TMXTOKBUFSIZE );
  if ( pData->pTokBuf == NULL )
  {
    printf( "Error: Memory allocation error, program terminated" );
    return( -1 );
  } /* endif */

  // skip program name
  argc--;
  argv++;

  // process arguments
  while ( argc )
  {
    PSZ pszParm = argv[0];

    if ( (*pszParm == '-') || (*pszParm == '/') )
    {
      if ( strnicmp( pszParm+1, "inputmem=", 9 ) == 0 )
      {
        PSZ pszValue = pszParm + 10;
        int len = sizeof(pData->szInSpec)/sizeof(pData->szInSpec[0])-1;
        strncpy( pData->szInSpec, pszValue, len);
        pData->szInSpec[len] = 0;
      }
      else  if ( strnicmp( pszParm+1, "outputmem=", 10 ) == 0 )
      {
        PSZ pszValue = pszParm + 11;
        int len = sizeof(pData->szOutSpec)/sizeof(pData->szOutSpec[0])-1;
        strncpy( pData->szOutSpec, pszValue, len);
        pData->szOutSpec[len] = 0;
      }
      else if ( strnicmp( pszParm+1, "outmode=", 8 ) == 0 )
      {
        PSZ pszValue = pszParm + 9;
        strcpy( pData->szInMode, pszValue );
        if ( stricmp( pszValue, "UTF16" ) == 0 )
        {
          pData->lOutMode = UTF16_OPT;
        } 
        else if ( stricmp( pszValue, "ASCII" ) == 0 )
        {
          pData->lOutMode = ASCII_OPT;
        } 
        else if ( stricmp( pszValue, "ANSI" ) == 0 )
        {
          pData->lOutMode = ANSI_OPT;
        } /* endif */
      }
      else if ( strnicmp( pszParm+1, "om=", 3 ) == 0 )
      {
        PSZ pszValue = pszParm + 4;
        strcpy( pData->szInMode, pszValue );
        if ( stricmp( pszValue, "UTF16" ) == 0 )
        {
          pData->lOutMode = UTF16_OPT;
        } 
        else if ( stricmp( pszValue, "ASCII" ) == 0 )
        {
          pData->lOutMode = ASCII_OPT;
        } 
        else if ( stricmp( pszValue, "ANSI" ) == 0 )
        {
          pData->lOutMode = ANSI_OPT;
        } /* endif */
      }
      else if ( strnicmp( pszParm+1, "markup=", 7 ) == 0 )
      {
        strcpy( pData->szInMarkup, pszParm + 8);
      }
      else if ( strnicmp( pszParm+1, "ma=", 3 ) == 0 )
      {
        strcpy( pData->szInMarkup, pszParm + 4);
      }
      else if ( stricmp( pszParm+1, "cleanrtf" ) == 0 )
      {
        pData->fCleanRTF = TRUE;
      }
      else if ( stricmp( pszParm+1, "incurlybrace" ) == 0 )
      {
        pData->fTagsInCurlyBracesOnly = TRUE;
      }
      else if ( stricmp( pszParm+1, "sourcesource" ) == 0 )
      {
        pData->fSourceSource = TRUE;
      }
      else if ( stricmp( pszParm+1, "ss" ) == 0 )
      {
        pData->fSourceSource = TRUE;
      }
      else if ( stricmp( pszParm+1, "log" ) == 0 )
      {
        pData->fLog = TRUE;
      }
      else
      {
        printf( "Warning: unknown option \'%s\' is ignored\n", pszParm );
      } /* endif */
    }
    //else if ( pData->szInSpec[0] == '\0' )
    //{
    //  strcpy( pData->szInSpec, pszParm );
    //}
    //else if ( pData->szOutSpec[0] == '\0' )
    //{
    //  strcpy( pData->szOutSpec, pszParm );
    //}
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

  // start the batch session
  if ( usRC == NO_ERROR )
  {
    usRC = EqfStartSession( &hSession );
    if ( usRC  )
    {
      printf( "Error: Could not start OpenTM2 session, rc = %u\n", usRC );
    } /* endif */
  } /* endif */


  // convert the memory and handle wildcards in input memory name
  if ( usRC == NO_ERROR )
  {
    if ( (strchr( pData->szInSpec, '?' ) != NULL) || (strchr( pData->szInSpec, '*' ) != NULL) )
    {
      HDIR hdir;
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
          if ( FindData.dwFileAttributes & FILE_DIRECTORY )
          {
            // ignore directories
          }
          else
          {
            strcpy( pData->szInputFile, pData->szInSpec );
            if ( strchr( pData->szInputFile, '\\') == NULL )
            {
              strcpy( pData->szInputFile, FindData.cFileName  );
            }
            else
            {
              UtlSplitFnameFromPath( pData->szInputFile );
              strcat( pData->szInputFile, "\\" );
              strcat( pData->szInputFile, FindData.cFileName  );
            } /* endif */

            // construct output file if something has been specified
            if ( pData->szOutSpec[0] )
            {
              strcpy( pData->szOutputFile, pData->szOutSpec );
              UtlSplitFnameFromPath( pData->szOutputFile );
              strcat( pData->szOutputFile, "\\" );
              strcat( pData->szOutputFile, FindData.cFileName  );
            }
            else
            {
              strcpy( pData->szOutputFile, "" );
            } /* endif */

            usRC = ConvertSingleMemory( pData, pData->szInputFile, pData->szOutputFile );
          } /* endif */

          fMoreFiles = FindNextFile( hdir, &FindData  );
        } /* endwhile */

        FindClose( hdir );
      } /* endif */

    }
    else
    {
      usRC = ConvertSingleMemory( pData, pData->szInSpec, pData->szOutSpec );
    } /* endif */
  } /* endif */

  if ( hSession != 0L )
  {
    EqfEndSession( hSession );
  }

  if ( pData )
  {
    if ( pData->pTokBuf ) free(pData->pTokBuf);
    free( pData );
  } /* endif */

  return( usRC );
} /* end of function main */


// function converting a single EXP memory into the TMX format
USHORT ConvertSingleMemory
(
  PTMX2EXP_DATA pData,                 // Pointer to our data area
  PSZ              pszInMemory,        // fully qualified name of input memory 
  PSZ              pszOutMemory        // fully qualified name of output memory
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  LONG lOptions = 0;
  LONG lSegments = 0;
  LONG lSkippedSegments = 0;


  // check existence of input file
  if ( usRC == NO_ERROR) usRC = CheckExistence( pData, pszInMemory );
  strcpy( pData->szInMemory, pszInMemory );

  // generate output memory name if not specified
  if ( usRC == NO_ERROR) usRC = CheckOutputName( pData, pData->szInMemory, pszOutMemory );

  if ( pData->fCleanRTF ) lOptions |= CLEANRTF_OPT;
  if ( pData->fTagsInCurlyBracesOnly ) lOptions |= INCURLYBRACE_OPT;
  if ( pData->fSourceSource ) lOptions |= SOURCESOURCEMEM_OPT;

  usRC = TMXTOEXP( pData->szInMemory, pData->szOutMemory, pData->lOutMode, lOptions, pData->szInMarkup,
                   &lSegments, &lSkippedSegments );

  if ( usRC == NO_ERROR)
  {
    sprintf( pData->szUtf8Buffer, "Memory %s converted into EXP memory %s\n   %ld segments written to EXP file\n   %ld segments skipped.",
      pData->szInMemory, pData->szOutMemory, lSegments, lSkippedSegments ); 
    ShowInfo( pData, "%s", pData->szUtf8Buffer );
  }
  else
  {
    sprintf( pData->szUtf8Buffer, "Conversion of memory %s into EXP memory %s failed, rc=%u.",
      pData->szInMemory, pData->szOutMemory, usRC ); 
    ShowError( pData, "%s", pData->szUtf8Buffer );
  } /* endif */
  return( usRC );
} /* end of function ConvertSingleMemory */

// check existence of input file
USHORT CheckExistence( PTMX2EXP_DATA pData, PSZ pszInMemory )
{
  USHORT      usRC = NO_ERROR;         // function return code
  HDIR hdir;
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
USHORT ShowError( PTMX2EXP_DATA pData, PSZ pszMsg, PSZ pszParm )
{
  USHORT      usRC = NO_ERROR;         // function return code

  sprintf( pData->szMsgBuffer, pszMsg, pszParm );

  printf( "Error: %s\n\n", pData->szMsgBuffer );

  if ( pData->fLog )
  {
    FILE *hfLog = fopen( "EXP2TMX.LOG", "a" );
    if ( hfLog )
    {
      fprintf( hfLog, "Error: %s\n\n", pData->szMsgBuffer );
      fclose( hfLog );
    } /* endif */
  } /* endif */
  return( usRC );
} /* end of function ShowError */

USHORT ShowInfo( PTMX2EXP_DATA pData, PSZ pszMsg, PSZ pszParm )
{
  USHORT      usRC = NO_ERROR;         // function return code

  sprintf( pData->szMsgBuffer, pszMsg, pszParm );

  printf( "Info: %s\n\n", pData->szMsgBuffer );

  if ( pData->fLog )
  {
    FILE *hfLog = fopen( "EXP2TMX.LOG", "a" );
    if ( hfLog )
    {
      fprintf( hfLog, "Info: %s\n\n", pData->szMsgBuffer );
      fclose( hfLog );
    } /* endif */
  } /* endif */
  return( usRC );
} /* end of function ShowInfo */

// check output memory name 
USHORT CheckOutputName( PTMX2EXP_DATA pData, PSZ pszInMemory, PSZ pszOutMemory )
{
  USHORT      usRC = NO_ERROR;         // function return code

  pData;

  if ( *pszOutMemory == EOS )
  {
    PSZ pszExtension;

    strcpy( pData->szOutMemory, pszInMemory );

    pszExtension = strrchr( pData->szOutMemory, '\\' );
    if ( pszExtension == NULL ) pszExtension = pData->szOutMemory;
    pszExtension = strrchr( pszExtension, '.' );
    if ( pszExtension == NULL ) pszExtension = pData->szOutMemory + strlen(pData->szOutMemory);
    strcpy( pszExtension, ".EXP" );
  }
  else
  {
    strcpy( pData->szOutMemory, pszOutMemory );
  } /* endif */

  return( usRC );
} /* end of function */

void showHelp()
{
    printf( "OtmTmx2Exp.EXE   : TMX to EXP memory convertor\n" );
    printf( "Version          : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright        : %s\n",STR_COPYRIGHT );
    printf( "Purpose          : Convert a memory in TMX format into the EXP format\n" );
    printf( "Syntax format    : OtmTmx2Exp /INPUTMEM=inputmem [/OUTPUTMEM=outputmem] [/OUTMODE|/OM=outmode] [/CLEANRTF] [/INCURLYBRACE] [/SOURCESOURCE|/SS] ][/MArkup=markup]\n" );
    printf( "Options and parameters:\n" );
    printf( "    /INPUTMEM     the fully qualified name of the input memory\n       (this name can contain wildcards)\n" );
    printf( "    /OUTPUTMEM    the fully qualified name of the output memory\n      (if not specified the input memory name\n" );
    printf( "    /OUTMODE      the output mode and can be UTF16, ASCII, or ANSI\n" );
    printf( "    /CLEANRTF     removes any RTF tags contained in the segments\n" );
    printf( "    /INCURLYBRACE restricts RTF tag removal to tags enclosed in curly braces (needs /CLEANRTF as well)\n" );
    printf( "    /SOURCESOURCE converts the memory to a source/source memory\n" );
    printf( "    /MArkup       the markup to use for the segments in the EXP file\n" );
    printf( "                  You can specify a comma separated list with different markup tables");
    printf("                   OtmTmx2Exp.exe creates a separate Translation Memory\n" ); 
    printf( "                  entry for each of the specified markup tables\n" );
}
