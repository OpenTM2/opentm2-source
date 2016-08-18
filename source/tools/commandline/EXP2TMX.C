//+----------------------------------------------------------------------------+
//| EXP2TMX.C                                                                  |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2016, International Business Machines                   |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//+----------------------------------------------------------------------------+
//| Description: EXP format to TMX format converter                            |
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
#include "EQFMEMIE.H"             // for memory import/export and convert functions


#ifdef _DEBUG
#endif

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///
///  CleanMemory API
///
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// data types 

// size of input buffers
#define BUF_SIZE 8096
#define TMXTOKBUFSIZE 32000

typedef struct _EXP2TMX_DATA
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

  //-- data for output memory processing --
  CHAR        szOutSpec[MAX_LONGFILESPEC];       // output memory specification of user
  CHAR        szOutputFile[MAX_LONGFILESPEC];    // buffer for preperation of output memory name
  CHAR        szOutMemory[MAX_LONGFILESPEC];     // fully qualified name of current output memory
  FILE        *hOutFile;                         // handle of output file
  BOOL        fUTF16Output;                      // TRUE = UTF16, FALSE = UTF8 
  CHAR        szOutMode[200];                    // buffer for output mode
  CHAR        szUtf8Buffer[16000];               // buffer for UTF8 conversion
  CHAR_W      szOutBuffer[16000];                // output buffer 
  CHAR_W      szEscapeBuffer[32000];             // buffer for character escaping and to save EXP segment
  BOOL        fNoCRLF;                           // true = remove linebreaks from segment data

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
  CHAR_W      szMarkup[100];                     // TM markup
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
  CHAR         szErrorMsgBuffer[4000];           // buffer for error messages
  CHAR         szLogFile[MAX_LONGFILESPEC];      // fully qualified name of log file (if not specified name of input memory is used as log)
  FILE        *hfLogFile;                        // log file handle 
} EXP2TMX_DATA, *PEXP2TMX_DATA;



// prototypes 
USHORT CheckExistence( PEXP2TMX_DATA pData, PSZ pszInMemory );
USHORT GetEncoding( PEXP2TMX_DATA pData, PSZ pszInMemory, PSZ pszInMode );
USHORT CheckOutputName( PEXP2TMX_DATA pData, PSZ pszInMemory, PSZ pszOutMemory );
USHORT PrepareOutput( PEXP2TMX_DATA pData, PSZ pszOutMemory, PSZ pszOutMode );
USHORT GetNextSegment( PEXP2TMX_DATA pData, PBOOL pfSegmentAvailable );
USHORT WriteSegment( PEXP2TMX_DATA pData );
USHORT CloseInput( PEXP2TMX_DATA pData );
USHORT TerminateOutput( PEXP2TMX_DATA pData );
USHORT ShowError( PEXP2TMX_DATA pData, PSZ pszMsg, PSZ pszParm );
USHORT ShowInfo( PEXP2TMX_DATA pData, PSZ pszMsg, PSZ pszParm );
USHORT FillBufferW( PEXP2TMX_DATA pData );
USHORT FillBuffer( PEXP2TMX_DATA pData );
USHORT ReadLineW( PEXP2TMX_DATA pData, PSZ_W pszLine, int iSize );
USHORT ReadLine( PEXP2TMX_DATA pData, PSZ pszLine, int iSize );
USHORT GetLine( PEXP2TMX_DATA pData, PSZ_W pszLine, int iSize, BOOL fUnicode, PBOOL pfEOF );
void EscapeCharacters( PSZ_W pszName, PSZ_W pszBuffer );
PSZ_W ParseX15W( PSZ_W pszX15String, SHORT sStringId );
BOOL StripTag( PSZ_W pszLine, PSZ_W pszTag );
USHORT WriteTUV( PEXP2TMX_DATA pData, PSZ_W pszLanguage, PSZ_W pszMarkup, PSZ_W pszSegmentData );
void GetTypeForMarkup( PSZ_W pszMarkup, PSZ_W pszDataType );
void GetIsoLangForTMLang( PSZ_W pszTMLang, PSZ_W pszIsoLang );
USHORT ConvertSingleMemory( PEXP2TMX_DATA pData, PSZ pszInMemory, PSZ pszOutMemory );
USHORT WriteToOutMem( PEXP2TMX_DATA pData, PSZ_W pszString );
PSZ SplitFnameFromPath( PSZ pszPath );

static void showHelp();

// main entry point
int main( int argc, char *argv[], char *envp[] )
{
  USHORT   usRC = 0;
  PEXP2TMX_DATA pData = NULL;
  HSESSION hSession = 0L;

  envp;

  if(argc==2 && stricmp(argv[1],"-h")==0)
  {
      showHelp();
      return 0;
  }

  // show logo
  printf( "EXP2TMX - The EXP to TMX memory converter\n\n" );


  // allocate our data area
  pData = (PEXP2TMX_DATA)malloc( sizeof(EXP2TMX_DATA) );
  if ( pData == NULL )
  {
    printf( "Error: Memory allocation error, program terminated" );
    return( -1 );
  } /* endif */

  memset( pData, 0, sizeof(EXP2TMX_DATA) );
  pData->pTokBuf = (PTOKENENTRY)malloc( TMXTOKBUFSIZE );
  if ( pData->pTokBuf == NULL )
  {
    printf( "Error: Memory allocation error, program terminated" );
    return( -1 );
  } /* endif */
  pData->fLog = TRUE;

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
      else if ( strnicmp( pszParm+1, "in=", 3 ) == 0 )
      {
        PSZ pszValue = pszParm + 4;
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
      else  if ( strnicmp( pszParm+1, "out=", 4 ) == 0 )
      {
        PSZ pszValue = pszParm + 5;
        int len = sizeof(pData->szOutSpec)/sizeof(pData->szOutSpec[0])-1;
        strncpy( pData->szOutSpec, pszValue, len);
        pData->szOutSpec[len] = 0;
      }
      else if ( strnicmp( pszParm+1, "inmode=", 7 ) == 0 )
      {
        PSZ pszValue = pszParm + 8;
        strcpy( pData->szInMode, pszValue );
        if ( stricmp( pszValue, "UTF16" ) == 0 )
        {
        } 
        else if ( stricmp( pszValue, "ASCII" ) == 0 )
        {
          pData->ulInputCP = CP_OEMCP;
        } 
        else if ( stricmp( pszValue, "ANSI" ) == 0 )
        {
          pData->ulInputCP = CP_ACP;
        } 
        else 
        {
          PSZ pszTest = pszValue;
          while ( *pszTest && isdigit(*pszTest) ) pszTest++;

          if ( *pszTest )
          {
            printf( "Error: Invalid input mode specified, valid values are \"UTF16\", \"ANSI\", \"ASCII\" or anumeric codepage number\n" );
            usRC = (USHORT)-2;
          }
          else
          {
            pData->ulInputCP = atol( pszValue );
          } /* endif */
        } /* endif */
      }
      else if ( strnicmp( pszParm+1, "im=", 3 ) == 0 )
      {
        PSZ pszValue = pszParm + 4;
        strcpy( pData->szInMode, pszValue );
        if ( stricmp( pszValue, "UTF16" ) == 0 )
        {
        } 
        else if ( stricmp( pszValue, "ASCII" ) == 0 )
        {
          pData->ulInputCP = CP_OEMCP;
        } 
        else if ( stricmp( pszValue, "ANSI" ) == 0 )
        {
          pData->ulInputCP = CP_ACP;
        } 
        else 
        {
          PSZ pszTest = pszValue;
          while ( *pszTest && isdigit(*pszTest) ) pszTest++;

          if ( *pszTest )
          {
            printf( "Error: Invalid input mode specified, valid values are \"UTF16\", \"ANSI\", \"ASCII\" or anumeric codepage number\n" );
            usRC = (USHORT)-2;
          }
          else
          {
            pData->ulInputCP = atol( pszValue );
          } /* endif */
        } /* endif */
      }
      else if ( strnicmp( pszParm+1, "LOG=", 4 ) == 0 )
      {
        pData->fLog = TRUE;
        PSZ pszValue = pszParm + 5;
        strcpy( pData->szLogFile, pszValue );
      }
      else if ( stricmp( pszParm+1, "NOLOG" ) == 0 )
      {
        pData->fLog = FALSE;
        PSZ pszValue = pszParm + 5;
        strcpy( pData->szOutMode, pszValue );
      }
      else if ( strnicmp( pszParm+1, "outmode=", 8 ) == 0 )
      {
        PSZ pszValue = pszParm + 9;
        strcpy( pData->szOutMode, pszValue );
        if ( stricmp( pszValue, "UTF16" ) == 0 )
        {
          pData->fUTF16Output = TRUE;
        } 
        else if ( stricmp( pszValue, "UTF8" ) == 0 )
        {
          pData->fUTF16Output = FALSE;
        } 
        else 
        {
          printf( "Error: Invalid output mode specified, valid values are \"UTF16\" or \"UTF8\".\n" );
          usRC = (USHORT)-2;
        } /* endif */
      }
      else if ( strnicmp( pszParm+1, "om=", 3 ) == 0 )
      {
        PSZ pszValue = pszParm + 4;
        strcpy( pData->szOutMode, pszValue );
        if ( stricmp( pszValue, "UTF16" ) == 0 )
        {
          pData->fUTF16Output = TRUE;
        } 
        else if ( stricmp( pszValue, "UTF8" ) == 0 )
        {
          pData->fUTF16Output = FALSE;
        } 
        else 
        {
          printf( "Error: Invalid output mode specified, valid values are \"UTF16\" or \"UTF8\".\n" );
          usRC = (USHORT)-2;
        } /* endif */
      }
      else if ( stricmp( pszParm+1, "nocrlf" ) == 0 )
      {
        pData->fNoCRLF = TRUE;
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
      printf( "Warning: superfluos command line parameter \'%s\' is ignored\n", pszParm );
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
      printf( "Error: Could not start TranslationManager session, rc = %u\n", usRC );
    } /* endif */
  } /* endif */


  // convert the memory and handle wildcards in input memory name
  if ( usRC == NO_ERROR )
  {
    // open any user specified log file 
    if ( pData->fLog && (pData->szLogFile[0] != EOS) )
    {
      pData->hfLogFile = fopen( pData->szLogFile, "wb" );
      if ( pData->hfLogFile )
      {
        fwrite( UNICODEFILEPREFIX, 2, 1, pData->hfLogFile );
        fwprintf( pData->hfLogFile, L"OtmExp2Tmx Log File\r\n\r\n" );
        fwprintf( pData->hfLogFile, L"Specified input memory : %S\r\n", pData->szInSpec );
        fwprintf( pData->hfLogFile, L"Specified output memory: %S\r\n\r\n", pData->szOutSpec );
      } /* endif */
    } /* endif */

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
              SplitFnameFromPath( pData->szInputFile );
              strcat( pData->szInputFile, "\\" );
              strcat( pData->szInputFile, FindData.cFileName  );
            } /* endif */

            // construct output file if something has been specified
            if ( pData->szOutSpec[0] )
            {
              strcpy( pData->szOutputFile, pData->szOutSpec );
              SplitFnameFromPath( pData->szOutputFile );
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

  // open any user specified log file 
  if ( pData->hfLogFile )
  {
    fclose( pData->hfLogFile );
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
  PEXP2TMX_DATA pData,                 // Pointer to our data area
  PSZ              pszInMemory,        // fully qualified name of input memory 
  PSZ              pszOutMemory        // fully qualified name of output memory
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  LONG        lOutMode = 0;
  BOOL        fMemoryLogFile = FALSE;

  LONG lSegments = 0;
  LONG lInvalidSegments = 0;

  usRC = CheckExistence( pData, pszInMemory );

  // generate output memory name if not specified
  if ( usRC == NO_ERROR) usRC = CheckOutputName( pData, pszInMemory, pszOutMemory );

  if ( (usRC == NO_ERROR) && pData->fLog )
  {
    if ( pData->hfLogFile == NULL )
    {
      // create memory specifiy log file
      strcpy( pData->szBuffer, pszInMemory );
      strcat( pData->szBuffer, ".log" );
      pData->hfLogFile = fopen( pData->szBuffer, "wb" );
      if ( pData->hfLogFile )
      {
        fMemoryLogFile = TRUE;
        fwrite( UNICODEFILEPREFIX, 2, 1, pData->hfLogFile );
        fwprintf( pData->hfLogFile, L"OtmExp2Tmx Log File\r\n\r\n" );
        fwprintf( pData->hfLogFile, L"Converting EXP memory %S into TMX memory %S\r\n\r\n", pszInMemory, pszOutMemory );
      } /* endif */
    }
    else
    {
      fwprintf( pData->hfLogFile, L"Converting EXP memory %S into TMX memory %S\r\n\r\n", pszInMemory, pszOutMemory );
    } /* endif */
  } /* endif */

  lOutMode = (stricmp( pData->szOutMode, "UTF16" ) == 0 ) ? TMX_UTF16_OPT : TMX_UTF8_OPT;
  if ( pData->fNoCRLF ) lOutMode = lOutMode | TMX_NOCRLF_OPT; 

  pData->szErrorMsgBuffer[0] = EOS;
  usRC = EXPTOTMX( pszInMemory, pData->szOutMemory, pData->szInMode, lOutMode, &lSegments, pData->szErrorMsgBuffer, &lInvalidSegments, pData->hfLogFile ); 

  if ( usRC == NO_ERROR)
  {
    sprintf( pData->szUtf8Buffer, "Memory %s converted into TMX memory %s\n   %ld segments written to TMX file\n   %ld segments skipped.",
      pszInMemory, pData->szOutMemory, lSegments, lInvalidSegments ); 
    ShowInfo( pData, "%s", pData->szUtf8Buffer );
  }
  else
  {
    sprintf( pData->szUtf8Buffer, "Conversion of memory %s into TMX memory %s failed, rc=%u.",
      pszInMemory, pData->szOutMemory, usRC ); 
    ShowError( pData, "%s", pData->szUtf8Buffer );
    if ( pData->szErrorMsgBuffer[0] != EOS )
    {
      ShowInfo( pData, "%s", pData->szErrorMsgBuffer );
    } /* endif */
  } /* endif */

  // close any local log file
  if ( fMemoryLogFile )
  {
    fclose( pData->hfLogFile );
    pData->hfLogFile = NULL;
  } /* endif */

  return( usRC );
} /* end of function ConvertSingleMemory */

// check existence of input file
USHORT CheckExistence( PEXP2TMX_DATA pData, PSZ pszInMemory )
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


// check output memory name 
USHORT CheckOutputName( PEXP2TMX_DATA pData, PSZ pszInMemory, PSZ pszOutMemory )
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
    strcpy( pszExtension, ".TMX" );
  }
  else
  {
    strcpy( pData->szOutMemory, pszOutMemory );
  } /* endif */

  return( usRC );
} /* end of function */

// show error message
USHORT ShowError( PEXP2TMX_DATA pData, PSZ pszMsg, PSZ pszParm )
{
  USHORT      usRC = NO_ERROR;         // function return code

  sprintf( pData->szMsgBuffer, pszMsg, pszParm );

  printf( "Error: %s\n\n", pData->szMsgBuffer );

  if ( pData->hfLogFile )
  {
    fwprintf( pData->hfLogFile, L"Error: %S\r\n\r\n", pData->szMsgBuffer );
  } /* endif */
  return( usRC );
} /* end of function ShowError */

USHORT ShowInfo( PEXP2TMX_DATA pData, PSZ pszMsg, PSZ pszParm )
{
  USHORT      usRC = NO_ERROR;         // function return code

  sprintf( pData->szMsgBuffer, pszMsg, pszParm );

  printf( "Info: %s\n\n", pData->szMsgBuffer );

  if ( pData->hfLogFile )
  {
    fwprintf( pData->hfLogFile, L"Info: %S\r\n\r\n", pData->szMsgBuffer );
  } /* endif */
  return( usRC );
} /* end of function ShowInfo */

PSZ SplitFnameFromPath( PSZ pszPath )
{
    char *p;
    if( (p = strrchr( pszPath, BACKSLASH)) != NULL )
       *p++ = EOS;
    return( p);
}

void showHelp()
{
    printf( "OtmExp2Tmx.EXE        : EXP to TMX convertor\n" );
    printf( "Version               : %s\n", "1.3.0.1" /*STR_DRIVER_LEVEL_NUMBER*/ );
    printf( "Copyright             : %s\n",STR_COPYRIGHT);
    printf( "Purpose               : Converts a memory from the EXP format into the TMX format\n" );
    printf( "Syntax format         : OtmExp2Tmx /INPUTMEM=inputmem [/OUTPUTMEM=outputmem] [/INMODE=inmode] [/OUTMODE=outmode] [/NOCRLF] [/NOLOG] [LOG=logfile]\n" );
    printf( "Options and parameters:\n" );
    printf( "   /INPUTMEM or /IN   the fully qualified name of the input memory\n       (this name can contain wildcards)\n" );
    printf( "   /OUTPUTMEM or /OUT the fully qualified name of the output memory\n       (if not specified the input memory name\n" ); 
    printf( "   /INMODE or /IM     the input mode and can be UTF16, ASCII, ANSI or \n       the number of the codepage\n" );
    printf( "                      this parameter is only required for non-UTF16 memories without\n       <codepage> tag\n" );
    printf( "   /OUTMODE           selects the output format of the TMX memory and can be\n       UTF8 or UTF16 (UTF8 is the default)\n" );
    printf( "   /NOCRLF            suppress any line breaks within the segment data\n" );
    printf( "   /NOLOG             do not write additional info and error data to a log file\n" );
    printf( "   /LOG=              the fully qualified name of the log file\n" );
    printf( "                      when no log file is specified, the name of the input memory suffixed with .log is used as log file name\n" );
}

