//+----------------------------------------------------------------------------+
//| XLIFF2EXP.C                                                                |
//+----------------------------------------------------------------------------+
// Copyright (C) 2012-2015, International Business Machines
// Corporation and others.  All rights reserved.
//
//+----------------------------------------------------------------------------+
//| Author: David Walters                                                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Description: XLIFF (machine translation) format to EXP format converter    |
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
//#include "OTMFUNC.H"

#include <time.h>                 // C library for time functions
#include "EQFDDE.H"               // Batch mode definitions
#define INCL_EQFMEM_DLGIDAS       // include dialog IDA definitions
#include "EQFTMI.H"               // Private header file of Translation Memory
#include "EQFMEM.ID"              // PM IDs for Translation Memory
#include <EQFQDAM.H>              // Low level TM access functions
#include "OTMFUNC.H"              // function call interface public defines
#include "eqffunci.h"             // function call interface private defines
#include "EQFHLOG.H"              // for word count category limits

#include "EQFMEMIE.H"

#ifdef _DEBUG
#endif


// data types 

// size of input buffers
#define BUF_SIZE 8096

typedef struct _XLIFF2EXP_DATA
{
  //-- data for input memory processing --
  CHAR        szInSpec[MAX_LONGFILESPEC];        // input memory specification of user
  CHAR        szInputFile[MAX_LONGFILESPEC];     // buffer for preperation of input memory name
  CHAR        szInMemory[MAX_LONGFILESPEC];      // fully qualified name current input memory
  CHAR        szBuffer[4096];                    // general purpose buffer

  //-- data for output memory processing --
  CHAR        szOutSpec[MAX_LONGFILESPEC];       // output memory specification of user
  CHAR        szOutputFile[MAX_LONGFILESPEC];    // buffer for preperation of output memory name
  CHAR        szOutMemory[MAX_LONGFILESPEC];     // fully qualified name of current output memory
  CHAR        szOutMode[200];                    // buffer for output mode
  CHAR        szUtf8Buffer[16000];               // buffer for UTF8 conversion
  CHAR_W      szOutBuffer[16000];                // output buffer 
  LONG        lOutMode;                          // output mode ASNSI_OPT, ASCII_OPT, UTF16_OPT


  // -- other stuff --
  CHAR         szMsgBuffer[2048];                // buffer for message texts
  BOOL         fLog;                             // true = write messages to log file
} XLF2EXP_DATA, *PXLF2EXP_DATA;



// prototypes 
USHORT CheckExistence( PXLF2EXP_DATA pData, PSZ pszInMemory );
USHORT CheckOutputName( PXLF2EXP_DATA pData, PSZ pszInMemory, PSZ pszOutMemory );
USHORT ShowError( PXLF2EXP_DATA pData, PSZ pszMsg, PSZ pszParm );
USHORT ShowInfo( PXLF2EXP_DATA pData, PSZ pszMsg, PSZ pszParm );
USHORT ConvertSingleMemory( PXLF2EXP_DATA pData, PSZ pszInMemory, PSZ pszOutMemory );
USHORT WriteToOutMem( PXLF2EXP_DATA pData, PSZ_W pszString );

static void showHelp();

// main entry point
int main( int argc, char *argv[], char *envp[] )
{
  USHORT   usRC = 0;
  PXLF2EXP_DATA pData = NULL;
  HSESSION hSession = 0L;

  envp;

  if(argc==2 && stricmp(argv[1],"-h")==0)
  {
      showHelp();
      return 0;
  }
  // show logo
  printf( "XLIFF2EXP - The XLIFF (Machine Translation) to EXP memory converter\n\n" );


  // allocate our data area
  pData = (PXLF2EXP_DATA)malloc( sizeof(XLF2EXP_DATA) );
  if ( pData == NULL )
  {
    printf( "Error: Memory allocation error, program terminated" );
    return( -1 );
  } /* endif */

  memset( pData, 0, sizeof(XLF2EXP_DATA) );
  pData->lOutMode = UTF16_OPT;

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
      else if ( stricmp( pszParm+1, "log" ) == 0 )
      {
        pData->fLog = TRUE;
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
    free( pData );
  } /* endif */

  return( usRC );
} /* end of function main */


// function converting a single EXP memory into the TMX format
USHORT ConvertSingleMemory
(
  PXLF2EXP_DATA pData,                 // Pointer to our data area
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

  usRC = XLFTOEXP( pData->szInMemory, pData->szOutMemory, pData->lOutMode, lOptions, 
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
USHORT CheckExistence( PXLF2EXP_DATA pData, PSZ pszInMemory )
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
USHORT ShowError( PXLF2EXP_DATA pData, PSZ pszMsg, PSZ pszParm )
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

USHORT ShowInfo( PXLF2EXP_DATA pData, PSZ pszMsg, PSZ pszParm )
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
USHORT CheckOutputName( PXLF2EXP_DATA pData, PSZ pszInMemory, PSZ pszOutMemory )
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
    printf( "OtmXliff2Exp.EXE  : XLIFF (machine translation) to EXP memory convertor\n" );
    printf( "Version           : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright         : %s\n",STR_COPYRIGHT );
    printf( "Purpose           : Convert a memory in XLIFF (MT) format into the EXP format\n" );
    printf( "Syntax format     : OtmXliff2Exp /INPUTMEM=inputmem [/OUTPUTMEM=outputmem]\n" );
    printf( "Options and parameters:\n" );
    printf( "    /INPUTMEM     the fully qualified name of the input XLIFF memory\n     (this name can contain wildcards)\n" );
    printf( "    /OUTPUTMEM    the fully qualified name of the output EXP memory\n      (if not specified the input memory name\n" );
}
