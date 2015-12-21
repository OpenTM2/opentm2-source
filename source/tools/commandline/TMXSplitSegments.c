//+----------------------------------------------------------------------------+
//| TMXSplitSegments.C                                                         |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2015, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//+----------------------------------------------------------------------------+
#include <eqf.h>                  
#include <eqfserno.h>
#include <time.h>


// data types 

// size of input buffers
#define BUF_SIZE 8096
#define TMXTOKBUFSIZE 32000

PSZ pszDEAbbrev[] = {"0.", "1.", "2.", "3.", "4.", "6.", "7.", "8.", "9.", "a.",
                     "z.B.", "z.", "Benutzerspez.", "B.", "bzw.", "d.h.", "d.", "h.", "evtl.", "ggf.", "Inc.", "autom.", "max.", "nutzerspez.", 
                     "usw.", "u.A.", "u.", NULL };
PSZ pszITAbbrev[] = { "autom.", "ecc.", "es.", "imp.", "Inc.", "pag.", "predef.", "Oppure.", "dim.", "lungh.", "N.", "preimp.", "max.", "Tel.", "proc.", NULL};
PSZ pszESAbbrev[] = { "Desel.", "etc.", "Inc.", "p.ej.", "p.", "ej.", "U.S.", NULL};
PSZ pszFRAbbrev[] = {"0.", "1.", "2.", "3.", "4.", "6.", "7.", "8.", "9.", "Aj.", "Enreg.", "ex.", "param.", "Inc.", "util.", NULL};
PSZ pszENAbbrev[] = {"e.g.", "etc.", "i.e.", "Inc.", "vs.", "U.S.", NULL};

typedef struct _TMXSPLITSEG_DATA
{
  //-- data for input memory processing --
  CHAR        szInSpec[MAX_LONGFILESPEC];        // input memory specification of user
  CHAR        szInMemory[MAX_LONGFILESPEC];      // fully qualified name current input memory
  BOOL        fUnicode;                          // TRUE = input memory is in UTF-16 format
  FILE        *hInFile;                          // handle of input file
  CHAR        szInputFile[MAX_LONGFILESPEC];     // buffer for preperation of input memory name
  CHAR        szBuffer[4096];                    // general purpose buffer
  CHAR        szReadBuffer[16000];               // file read/write buffer (must be large enough to contain complete memory segment)

  //-- data for output memory processing --
  CHAR        szOutSpec[MAX_LONGFILESPEC];       // output memory specification of user
  CHAR        szOutMemory[MAX_LONGFILESPEC];     // fully qualified name of current output memory
  FILE        *hOutFile;                         // handle of output file

  //-- data for TU processing --
  CHAR        szTuBuffer[32000];                 // buffer for loaded translation unit
  int         iTuLine;                           // start line of TU
  int         iTuSize;                           // size of current TU
  int         iNumOfTUs;                         // number of TUs processed
  int         iNumOfAddTUs;                      // number of additional TUs
  int         iNumOfSplitTUs;                    // number of splitted TUs

  // -- other stuff --
  BOOL         fLog;                             // true = write messages to log file
  FILE         *hfLog;                           // log file handle
  CHAR         szLogFile[MAX_LONGFILESPEC];      // buffer for log file name
  BOOL         fErrorLog;                        // true = write messages to log file
  FILE         *hfErrorLog;                      // error log file handle
  CHAR         szErrorLogFile[MAX_LONGFILESPEC]; // buffer for error log file name
  BOOL         fWarningLog;                      // true = write messages to log file
  FILE         *hfWarningLog;                      // warning log file handle
  CHAR         szWarningLogFile[MAX_LONGFILESPEC]; // buffer for warning log file name
  CHAR         szMsgBuffer[2048];                // buffer for message texts
  CHAR         szLogBuffer[4096];                // buffer for logfile data
  int          iErrors;                          // number of errors
  int          iWarnings;                        // number of warnings
} TMXSPLITSEG_DATA, *PTMXSPLITSEG_DATA;



// prototypes 
USHORT ConvertMemory( PTMXSPLITSEG_DATA pData, PSZ pszInMemory, PSZ pszOutMemory );
USHORT CheckExistence( PTMXSPLITSEG_DATA pData, PSZ pszInMemory );
USHORT GetEncoding( PTMXSPLITSEG_DATA pData, PSZ pszInMemory );
USHORT ShowError( PTMXSPLITSEG_DATA pData, PSZ pszMsg, PSZ pszParm );
USHORT ShowWarning( PTMXSPLITSEG_DATA pData, PSZ pszMsg, PSZ pszParm );
USHORT ConvertMemoryAscii( PTMXSPLITSEG_DATA pData );
USHORT ConvertMemoryUTF16( PTMXSPLITSEG_DATA pData );
USHORT ShowInfo( PTMXSPLITSEG_DATA pData, PSZ pszMsg, PSZ pszParm );

static void showHelp();

// main entry point
int main( int argc, char *argv[], char *envp[] )
{
  USHORT   usRC = 0;
  PTMXSPLITSEG_DATA pData = NULL;

  envp;

  if(argc==2 && stricmp(argv[1],"-h")==0)
  {
      showHelp();
      return 0;
  }

  // show logo
  printf( "TMXSplitSegments - The TMX segment splitter\n\n" );


  // allocate our data area
  pData = (PTMXSPLITSEG_DATA) malloc( sizeof(TMXSPLITSEG_DATA) );
  if ( pData == NULL )
  {
    printf( "Error: Memory allocation error, program terminated" );
    return( -1 );
  } /* endif */

  memset( pData, 0, sizeof(TMXSPLITSEG_DATA) );

  // skip program name
  argc--;
  argv++;

  // process arguments
  pData->fLog = TRUE;
  while ( argc )
  {
    PSZ pszParm = argv[0];

    if ( (*pszParm == '-') || (*pszParm == '/') )
    {
      if ( strnicmp( pszParm + 1, "log=", 4 ) == 0 )
      {
        strcpy( pData->szLogFile, pszParm + 5 );
      }
      else if ( strnicmp( pszParm + 1, "logs=", 5 ) == 0 )
      {
        PSZ pszOpt = pszParm + 6;
        pData->fErrorLog = FALSE;
        pData->fWarningLog = FALSE;
        pData->fLog = FALSE;
        while ( *pszOpt )
        {
          if ( toupper(*pszOpt) == 'E' )
          {
            pData->fErrorLog = TRUE;
          } 
          else if ( toupper(*pszOpt) == 'W' )
          {
            pData->fWarningLog = TRUE;
          } 
          else if ( toupper(*pszOpt) == 'A' )
          {
            pData->fLog = TRUE;
          } /* endif */             
          pszOpt++;
        } /* endwhile */           
      }
      else
      {
        printf( "Warning: unknown option \'%s\' is ignored\n", pszParm );
      } /* endif */
    }
    else if ( pData->szInSpec[0] == '\0' )
    {
      strcpy( pData->szInSpec, pszParm );
    }
    else if ( pData->szOutSpec[0] == '\0' )
    {
      strcpy( pData->szOutSpec, pszParm );
    }
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

  // log file handling
  if ( usRC == NO_ERROR )
  {
    if ( pData->szLogFile[0] == EOS )
    {
      strcpy( pData->szLogFile, "TmxSplitSegments.log" );
    }
    else if ( stricmp( pData->szLogFile, "off" ) == 0)
    {
      pData->fLog = FALSE;
    } /* endif */       

    if ( pData->fLog )
    {
      pData->hfLog = fopen( pData->szLogFile, "a" );
      if ( pData->hfLog != NULL )
      {
        LONG lCurTime;
        time( &lCurTime );
        fprintf( pData->hfLog, "==== TmxSplitSegments Error/Warnings Log %s", asctime( localtime( &lCurTime ) ) );
        fprintf( pData->hfLog, "Input memory  = %s\n", pData->szInSpec );
        fprintf( pData->hfLog, "Output memory = %s\n", (pData->szOutSpec[0] != EOS) ? pData->szOutSpec : "None" );
      } 
      else
      {
        pData->fLog = FALSE;
        ShowError( pData, "Could not open log file %s", pData->szLogFile);
      } /* endif */         
    } /* endif */       

    if ( pData->fErrorLog )
    {
      PSZ pszTemp; 
      strcpy( pData->szErrorLogFile, pData->szLogFile );
      pszTemp = strrchr( pData->szErrorLogFile, '.' );
      if ( pszTemp != NULL )
      {
        strcpy( pData->szBuffer, pszTemp );
        strcpy( pszTemp, "_Error" );
        strcat( pData->szErrorLogFile, pData->szBuffer );
      }
      else
      {
        strcat( pData->szErrorLogFile, "_Error" );
      } /* endif */         
      pData->hfErrorLog = fopen( pData->szErrorLogFile, "a" );
      if ( pData->hfLog != NULL )
      {
        LONG lCurTime;
        time( &lCurTime );
        fprintf( pData->hfErrorLog, "==== TmxSplitSegments Error Log %s", asctime( localtime( &lCurTime ) ) );
        fprintf( pData->hfErrorLog, "Input memory  = %s\n", pData->szInSpec );
        fprintf( pData->hfErrorLog, "Output memory = %s\n", (pData->szOutSpec[0] != EOS) ? pData->szOutSpec : "None" );
      } 
      else
      {
        pData->fErrorLog = FALSE;
        ShowError( pData, "Could not open error log file %s", pData->szErrorLogFile);
      } /* endif */         
    } /* endif */       

    if ( pData->fWarningLog )
    {
      PSZ pszTemp; 
      strcpy( pData->szWarningLogFile, pData->szLogFile );
      pszTemp = strrchr( pData->szWarningLogFile, '.' );
      if ( pszTemp != NULL )
      {
        strcpy( pData->szBuffer, pszTemp );
        strcpy( pszTemp, "_Warning" );
        strcat( pData->szWarningLogFile, pData->szBuffer );
      }
      else
      {
        strcat( pData->szWarningLogFile, "_Warning" );
      } /* endif */         
      pData->hfWarningLog = fopen( pData->szWarningLogFile, "a" );
      if ( pData->hfLog != NULL )
      {
        LONG lCurTime;
        time( &lCurTime );
        fprintf( pData->hfWarningLog, "==== TmxSplitSegments Warning Log %s", asctime( localtime( &lCurTime ) ) );
        fprintf( pData->hfWarningLog, "Input memory  = %s\n", pData->szInSpec );
        fprintf( pData->hfWarningLog, "Output memory = %s\n", (pData->szOutSpec[0] != EOS) ? pData->szOutSpec : "None" );
      } 
      else
      {
        pData->fWarningLog = FALSE;
        ShowError( pData, "Could not open warning log file %s", pData->szWarningLogFile);
      } /* endif */         

    } /* endif */       

  } /* endif */

  if ( usRC == NO_ERROR )
  {
    usRC = ConvertMemory( pData, pData->szInSpec, pData->szOutSpec );
  } /* endif */

  if ( pData )
  {
    if ( pData->hfLog ) fclose( pData->hfLog );
    if ( pData->hfErrorLog ) fclose( pData->hfErrorLog );
    if ( pData->hfWarningLog ) fclose( pData->hfWarningLog );

    free( pData );
  } /* endif */

  return( usRC );
} /* end of function main */


// function converting a single EXP memory into the TMX format
USHORT ConvertMemory
(
  PTMXSPLITSEG_DATA pData,                 // Pointer to our data area
  PSZ              pszInMemory,        // fully qualified name of input memory 
  PSZ              pszOutMemory        // fully qualified name of output memory
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  LONG lOptions = 0;
  LONG lSegments = 0;
  LONG lSkippedSegments = 0;

  lOptions; lSegments; lSkippedSegments;

  // check existence of input file
  if ( usRC == NO_ERROR ) usRC = CheckExistence( pData, pszInMemory );
  strcpy( pData->szInMemory, pszInMemory );
  strcpy( pData->szOutMemory, pszOutMemory );

  // get encoding of input file
  if ( usRC == NO_ERROR ) usRC = GetEncoding( pData, pszInMemory );

  // process the memory
  if ( usRC == NO_ERROR )
  {
    if ( pData->fUnicode )
    {
      usRC = ConvertMemoryUTF16( pData );
    }
    else
    {
      usRC = ConvertMemoryAscii( pData );
    } /* endif */       
  } /* endif */     
  puts( "\n" );
  if ( pData->hfLog ) fputs( "\n", pData->hfLog );
  if ( pData->hfErrorLog ) fputs( "\n", pData->hfErrorLog );
  if ( pData->hfWarningLog ) fputs( "\n", pData->hfWarningLog );
  ShowInfo( pData, "Processing complete\n", pData->szInMemory );
  if ( pData->szOutSpec[0] != EOS )
  {
    sprintf( pData->szBuffer, "%ld translation units processed", pData->iNumOfTUs );
    ShowInfo( pData, pData->szBuffer, pData->szInMemory );
    sprintf( pData->szBuffer, "%ld translation units needed splitting", pData->iNumOfSplitTUs );
    ShowInfo( pData, pData->szBuffer, pData->szInMemory );
    sprintf( pData->szBuffer, "%ld additional translation units created\n", pData->iNumOfAddTUs );
    ShowInfo( pData, pData->szBuffer, pData->szInMemory );
  }
  else
  {
    sprintf( pData->szBuffer, "%ld translation units checked", pData->iNumOfTUs );
    ShowInfo( pData, pData->szBuffer, pData->szInMemory );
    sprintf( pData->szBuffer, "%ld translation units consist of more than one sentence", pData->iNumOfSplitTUs );
    ShowInfo( pData, pData->szBuffer, pData->szInMemory );
    sprintf( pData->szBuffer, "%ld additional TUs would be created by splitting\n", pData->iNumOfAddTUs );
    ShowInfo( pData, pData->szBuffer, pData->szInMemory );
  } /* endif */     
  sprintf( pData->szBuffer, "%ld possible ERRORS occurred", pData->iErrors );
  ShowInfo( pData, pData->szBuffer, pData->szInMemory );
  sprintf( pData->szBuffer, "%ld WARNINGS have been shown", pData->iWarnings);
  ShowInfo( pData, pData->szBuffer, pData->szInMemory );

  return( usRC );
} /* end of function ConvertSingleMemory */

// check existence of input file
USHORT CheckExistence( PTMXSPLITSEG_DATA pData, PSZ pszInMemory )
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

// open input memory and get input memory encoding
USHORT GetEncoding( PTMXSPLITSEG_DATA pData, PSZ pszInMemory )
{
  USHORT      usRC = NO_ERROR;         // function return code

  strcpy( pData->szInMemory, pszInMemory );
  pData->hInFile = fopen( pData->szInMemory, "rb" );
  if ( pData->hInFile == NULL )
  {
     usRC = ERROR_FILE_NOT_FOUND;
     ShowError( pData, "Error: Input memory %s could not be opened", pszInMemory );
  } /* endif */

  // get first bytes of memory and check for UTF-16 encoding
  if ( usRC == NO_ERROR )
  {
    // get first chunk of file and check for BOM and memory tag
    memset( pData->szBuffer, 0, sizeof(pData->szBuffer) );
    fread( pData->szBuffer, 1, sizeof(pData->szBuffer), pData->hInFile );

    if ( memcmp( pData->szBuffer, UNICODEFILEPREFIX, 2 ) == 0 )
    {
      pData->fUnicode = TRUE;
      fseek( pData->hInFile, 2, SEEK_SET );
    }
    else
    {
      pData->fUnicode = FALSE;
      fseek( pData->hInFile, 0, SEEK_SET );
    } /* endif */
  } /* endif */

  return( usRC );
} /* end of function GetEncoding */

BOOL StartsWith( PTMXSPLITSEG_DATA pData, PSZ pszLine, PSZ pszKey )
{
  pData; 

  // skip leading blanks
  while ( *pszLine == ' ' ) pszLine++;

  // test for key
  return( strnicmp( pszLine, pszKey, strlen(pszKey) ) == 0 );
} /* end of function StartsWith */

BOOL FindSegData( PSZ pszTuvStart, PSZ pszTuvEnd, PSZ *ppszSegStart, PSZ *ppszSegEnd )
{
  BOOL fOK = TRUE;
  CHAR chTemp = *pszTuvEnd;
  *pszTuvEnd = EOS;
  *ppszSegStart = NULL;
  *ppszSegEnd = NULL;

  *ppszSegStart = strstr( pszTuvStart, "<seg>" );
  if ( *ppszSegStart ) *ppszSegStart += strlen( "<seg>" ); 
  if ( *ppszSegStart ) *ppszSegEnd = strstr( *ppszSegStart , "</seg>" ); 

  *pszTuvEnd = chTemp;
 
  fOK = (*ppszSegStart != NULL) && (*ppszSegEnd != NULL ); 
  return( fOK ); 
}

int SplitSegData( PSZ pszStart, PSZ pszEnd, PSZ pszDelim[], PSZ *pszAbbrevStart )
{
  int iSegments = 1;
  CHAR chTemp = *pszEnd;
  *pszEnd = EOS;

  while ( *pszStart != EOS  )
  {
    if ( ((*pszStart == '.') || (*pszStart == '!') || (*pszStart == '?')) && (pszStart[1] == ' ') )
    {
      PSZ pszFound = NULL;

      if ( pszAbbrevStart != NULL )
      {
        PSZ *pszAbbrev = pszAbbrevStart;
        // check against list of abbreviations
        while ( (pszFound == NULL) && (*pszAbbrev != NULL) )
        {
          PSZ pszEntry = *pszAbbrev;
          int iLen = strlen(pszEntry);
          int iPos = 0;
          while ( (iPos < iLen) && ((toupper(pszStart[iPos-iLen+1]) == toupper(pszEntry[iPos]))) ) 
          {
            iPos++;
          } /* endwhile */             
          if ( iPos >= iLen )
          {
            if ( !isalpha(pszStart[-iLen]) )
            {
              pszFound = pszEntry;
            } /* endif */               
          } /* endif */             
          pszAbbrev++;
        } /* endwhile */           

      } /* endif */         

      if ( pszFound != NULL )
      {
        pszStart++;
      }
      else
      {
        pszStart += 2;
        while ( *pszStart == ' ') pszStart++;
        pszDelim[iSegments-1] = pszStart;
        iSegments++;
      } /* endif */         
    }
    else
    {
      pszStart++;
    } /* endif */       
  } /* endwhile */     
  pszDelim[iSegments-1] = pszStart;


  *pszEnd = chTemp;

  return( iSegments );
}

PSZ * GetAbbrevForTuv( PSZ pszTUVStart, PSZ pszTUVEnd )
{
  PSZ *pszAbbrev = NULL;
  PSZ pszLanguage = NULL;
  CHAR chTemp = *pszTUVEnd;
  *pszTUVEnd = EOS;

  pszLanguage = strstr( pszTUVStart, "xml:lang=\"" );
  if ( pszLanguage != NULL )
  {
    CHAR chID1 = (CHAR)(toupper(pszLanguage[10]));
    CHAR chID2 = (CHAR)(toupper(pszLanguage[11]));
    if ( (chID1 == 'D') && (chID2 == 'E') )
    {
      pszAbbrev = pszDEAbbrev;
    } 
    else if ( (chID1 == 'I') && (chID2 == 'T') )
    {
      pszAbbrev = pszITAbbrev;
    } 
    else if ( (chID1 == 'I') && (chID2 == 'T') )
    {
      pszAbbrev = pszITAbbrev;
    } 
    else if ( (chID1 == 'E') && (chID2 == 'S') )
    {
      pszAbbrev = pszESAbbrev;
    } 
    else if ( (chID1 == 'F') && (chID2 == 'R') )
    {
      pszAbbrev = pszFRAbbrev;
    } 
    else if ( (chID1 == 'E') && (chID2 == 'N') )
    {
      pszAbbrev = pszENAbbrev;
    } /* endif */       
  } /* endif */     
  *pszTUVEnd = chTemp;

  return( pszAbbrev );
}

USHORT ProcessTranslationUnit( PTMXSPLITSEG_DATA pData )
{
  USHORT      usRC = NO_ERROR;         // function return code
  PSZ   pszTUV1Start = NULL;
  PSZ   pszTUV1End = NULL;
  PSZ   pszTUV2Start = NULL;
  PSZ   pszTUV2End = NULL;
  PSZ   pszTUV3Start = NULL;
  PSZ   pszSeg1Start = NULL;
  PSZ   pszSeg1End = NULL;
  PSZ   pszSeg2Start = NULL;
  PSZ   pszSeg2End = NULL;
  PSZ   pszDelims1[20];
  PSZ   pszDelims2[20];

  pData->iNumOfTUs += 1;

  // find TUV delimiters
  pszTUV1Start = strstr( pData->szTuBuffer, "<tuv " );
  if ( pszTUV1Start != NULL ) pszTUV1End = strstr( pszTUV1Start, "</tuv>" );
  if ( pszTUV1End != NULL )   pszTUV2Start = strstr( pszTUV1End, "<tuv " );
  if ( pszTUV2Start != NULL ) pszTUV2End = strstr( pszTUV2Start, "</tuv>" );
  if ( pszTUV2End != NULL )   pszTUV3Start = strstr( pszTUV2End, "<tuv " );

  // check number of TUVs
  if ( pszTUV3Start != NULL )
  {
    sprintf( pData->szBuffer, "Translation unit starting in line %ld contains more than 2 TUVs and is copied as-is", pData->iTuLine );
    ShowError( pData, pData->szBuffer, pData->szInMemory );
    if ( pData->hfLog ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hfLog );
    if ( pData->hfLog ) fprintf( pData->hfLog, "%c", '\n' );
    if ( pData->hfErrorLog ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hfErrorLog );
    if ( pData->hfErrorLog ) fprintf( pData->hfErrorLog, "%c", '\n' );
    if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hOutFile );
    usRC = 1;
  }
  else if ( pszTUV2End == NULL )
  {
    sprintf( pData->szBuffer, "Translation unit starting in line %ld does not contain 2 TUVs and is copied as-is", pData->iTuLine );
    ShowError( pData, pData->szBuffer, pData->szInMemory );
    if ( pData->hfLog ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hfLog );
    if ( pData->hfLog ) fprintf( pData->hfLog, "%c", '\n' );
    if ( pData->hfErrorLog ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hfErrorLog );
    if ( pData->hfErrorLog ) fprintf( pData->hfErrorLog, "%c", '\n' );
    if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hOutFile );
    usRC = 2;
  } /* endif */     

  // find segment data in TUVs
  if ( usRC == NO_ERROR )
  {
    if ( !FindSegData( pszTUV1Start, pszTUV1End, &pszSeg1Start, &pszSeg1End ) || !FindSegData( pszTUV2Start, pszTUV2End, &pszSeg2Start, &pszSeg2End ) )
    {
      sprintf( pData->szBuffer, "Could not locate segment data in translation unit starting in line %ld, tu is copied as-is", pData->iTuLine );
      ShowError( pData, pData->szBuffer, pData->szInMemory );
      if ( pData->hfLog ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hfLog );
      if ( pData->hfLog ) fprintf( pData->hfLog, "%c", '\n' );
      if ( pData->hfErrorLog ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hfErrorLog );
      if ( pData->hfErrorLog ) fprintf( pData->hfErrorLog, "%c", '\n' );
      if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hOutFile );
      usRC = 3;
    } /* endif */     
  } /* endif */     

  // find sentence delimiters in segment data
  if ( usRC == NO_ERROR )
  {
    int iSegs1 = 0;
    int iSegs2 = 0;

    // get any abreviation lists for TUV language
    PSZ *pszAbbrev1 = GetAbbrevForTuv( pszTUV1Start, pszTUV1End );
    PSZ *pszAbbrev2 = GetAbbrevForTuv( pszTUV2Start, pszTUV2End );

    iSegs1 = SplitSegData( pszSeg1Start, pszSeg1End, pszDelims1, pszAbbrev1 );
    iSegs2 = SplitSegData( pszSeg2Start, pszSeg2End, pszDelims2, pszAbbrev2 );

    if ( (iSegs1 == 1) && (iSegs2 > 1)  )
    {
      sprintf( pData->szBuffer, "The translation of translation unit starting in line %ld has more than one sentence, splitting is not required, tu is copied as-is", pData->iTuLine );
      ShowWarning( pData, pData->szBuffer, pData->szInMemory );
      if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hOutFile );
      if ( pData->hfLog ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hfLog );
      if ( pData->hfLog ) fprintf( pData->hfLog, "%c", '\n' );
      if ( pData->hfWarningLog ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hfWarningLog );
      if ( pData->hfWarningLog ) fprintf( pData->hfWarningLog, "%c", '\n' );
    }
    else if ( iSegs1 != iSegs2 )
    {
      sprintf( pData->szBuffer, "Number of sentences in <tuvs> of translation unit starting in line %ld do not match, tu is copied as-is", pData->iTuLine );
      ShowError( pData, pData->szBuffer, pData->szInMemory );
      if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hOutFile );
      //if ( iSegs1 > 1 )
      //{
      //  int i = 0;
      //  for ( i = 0; i < (iSegs1 - 1); i++ )
      //  {
      //    pszDelims1[i][-1] = '#';
      //  } /* endfor */           
      //} /* endif */         
      //if ( iSegs2 > 1 )
      //{
      //  int i = 0;
      //  for ( i = 0; i < (iSegs2 - 1); i++ )
      //  {
      //    pszDelims2[i][-1] = '#';
      //  } /* endfor */           
      //} /* endif */         
      if ( pData->hfLog ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hfLog );
      if ( pData->hfLog ) fprintf( pData->hfLog, "%c", '\n' );
      if ( pData->hfErrorLog ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hfErrorLog );
      if ( pData->hfErrorLog ) fprintf( pData->hfErrorLog, "%c", '\n' );
      usRC = 4;
    }
    else if ( iSegs1 == 1 )
    {
      // nothing to split, write TU as-is
      if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pData->iTuSize, 1, pData->hOutFile );
    }
    else
    {
      // split segment data
      int i = 0;
      PSZ pszStart1 = pszSeg1Start;
      PSZ pszStart2 = pszSeg2Start;
      pData->iNumOfAddTUs -= 1;
      pData->iNumOfSplitTUs += 1;
      for ( i = 0; i < iSegs1; i++ )
      {
        // write tuv up to start of first sentence
        if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pszSeg1Start - pData->szTuBuffer, 1, pData->hOutFile );

        // write current sentence 1st tuv
        if ( pData->hOutFile ) fwrite( pszStart1, pszDelims1[i] - pszStart1, 1, pData->hOutFile );
        pszStart1 = pszDelims1[i];

        // write data up to next tuv seg data
        if ( pData->hOutFile ) fwrite( pszSeg1End, pszSeg2Start - pszSeg1End, 1, pData->hOutFile );

        // write current sentence 2nd tuv
        if ( pData->hOutFile ) fwrite( pszStart2, pszDelims2[i] - pszStart2, 1, pData->hOutFile );
        pszStart2 = pszDelims2[i];

        // write rest of tuv
        if ( pData->hOutFile ) fwrite( pszSeg2End, pData->iTuSize - (pszSeg2End - pData->szTuBuffer), 1, pData->hOutFile );
        if ( pData->hOutFile ) fputs( "\r\n", pData->hOutFile );

        pData->iNumOfAddTUs += 1;
      } /* endfor */         
    } /* endif */   
  } /* endif */     


  return( usRC );
} /* end of function ProcessTranslationUnit */

USHORT ConvertMemoryAscii
(
  PTMXSPLITSEG_DATA pData              // Pointer to our data area
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  int iLineNum = 0;

  // open output memory
  if ( pData->szOutSpec[0] != EOS )
  {
    pData->hOutFile = fopen( pData->szOutSpec, "wb" );
    if ( pData->hOutFile == NULL )
    {
      usRC = ERROR_FILE_NOT_FOUND;
      ShowError( pData, "Error: Output memory %s could not be opened", pData->szOutSpec );
    } /* endif */
  } /* endif */     

  // get/write memory lines until done
  fgets( pData->szReadBuffer, sizeof(pData->szReadBuffer), pData->hInFile );
  while ( !feof( pData->hInFile ) )
  {
    iLineNum++;
    if ( StartsWith( pData, pData->szReadBuffer, "<tu " ) )
    {
      // read complete TU into buffer and process it
      int iFilled = 0;
      pData->iTuLine = iLineNum;
      do
      {
        int iLen = strlen( pData->szReadBuffer );
        memcpy( pData->szTuBuffer + iFilled, pData->szReadBuffer, iLen );
        iFilled += iLen;
        fgets( pData->szReadBuffer, sizeof(pData->szReadBuffer), pData->hInFile );
        iLineNum++;
      } while ( !feof( pData->hInFile ) && !StartsWith( pData, pData->szReadBuffer, "</tu>" ) ); /* enddo */       
      if ( !feof( pData->hInFile  ) )
      {
        int iLen = strlen( pData->szReadBuffer );
        memcpy( pData->szTuBuffer + iFilled, pData->szReadBuffer, iLen );
        pData->iTuSize = iFilled + iLen;
        pData->szTuBuffer[pData->iTuSize]= EOS;

        // process the TU
        ProcessTranslationUnit( pData );
      } /* endif */         
    }
    else
    {
      if ( pData->hOutFile ) fputs( pData->szReadBuffer, pData->hOutFile );
    } /* endif */
    fgets( pData->szReadBuffer, sizeof(pData->szReadBuffer), pData->hInFile );
  } /* endwhile */     

  // close files
  if ( pData->hInFile ) fclose( pData->hInFile );
  if ( pData->hOutFile ) fclose( pData->hOutFile );

  return( usRC );
} /* end of function ConvertMemoryAscii */

BOOL StartsWithW( PTMXSPLITSEG_DATA pData, PSZ_W pszLine, PSZ_W pszKey )
{
  pData; 

  // skip leading blanks
  while ( *pszLine == L' ' ) pszLine++;

  // test for key
  return( _wcsnicmp( pszLine, pszKey, wcslen(pszKey) ) == 0 );
} /* end of function StartsWith */

BOOL FindSegDataW( PSZ_W pszTuvStart, PSZ_W pszTuvEnd, PSZ_W *ppszSegStart, PSZ_W *ppszSegEnd )
{
  BOOL fOK = TRUE;
  CHAR_W chTemp = *pszTuvEnd;
  *pszTuvEnd = EOS;
  *ppszSegStart = NULL;
  *ppszSegEnd = NULL;

  *ppszSegStart = wcsstr( pszTuvStart, L"<seg>" );
  if ( *ppszSegStart ) *ppszSegStart += wcslen( L"<seg>" ); 
  if ( *ppszSegStart ) *ppszSegEnd = wcsstr( *ppszSegStart , L"</seg>" ); 

  *pszTuvEnd = chTemp;
 
  fOK = (*ppszSegStart != NULL) && (*ppszSegEnd != NULL ); 
  return( fOK ); 
}

int SplitSegDataW( PSZ_W pszStart, PSZ_W pszEnd, PSZ_W pszDelim[] )
{
  int iSegments = 1;
  CHAR_W chTemp = *pszEnd;
  *pszEnd = EOS;

  while ( *pszStart != EOS  )
  {
    if ( ((*pszStart == L'.') || (*pszStart == L'!') || (*pszStart == L'?')) && (pszStart[1] == L' ') )
    {
      pszStart += 2;
      while ( *pszStart == L' ') pszStart++;
      pszDelim[iSegments-1] = pszStart;
      iSegments++;
    }
    else
    {
      pszStart++;
    } /* endif */       
  } /* endwhile */     
  pszDelim[iSegments-1] = pszStart;


  *pszEnd = chTemp;

  return( iSegments );
}

USHORT ProcessTranslationUnitW( PTMXSPLITSEG_DATA pData )
{
  USHORT      usRC = NO_ERROR;         // function return code
  PSZ_W   pszTUV1Start = NULL;
  PSZ_W   pszTUV1End = NULL;
  PSZ_W   pszTUV2Start = NULL;
  PSZ_W   pszTUV2End = NULL;
  PSZ_W   pszTUV3Start = NULL;
  PSZ_W   pszSeg1Start = NULL;
  PSZ_W   pszSeg1End = NULL;
  PSZ_W   pszSeg2Start = NULL;
  PSZ_W   pszSeg2End = NULL;
  PSZ_W   pszDelims1[20];
  PSZ_W   pszDelims2[20];

  pData->iNumOfTUs += 1;

  // find TUV delimiters
  pszTUV1Start = wcsstr( (PSZ_W)pData->szTuBuffer, L"<tuv " );
  if ( pszTUV1Start != NULL ) pszTUV1End = wcsstr( pszTUV1Start, L"</tuv>" );
  if ( pszTUV1End != NULL )   pszTUV2Start = wcsstr( pszTUV1End, L"<tuv " );
  if ( pszTUV2Start != NULL ) pszTUV2End = wcsstr( pszTUV2Start, L"</tuv>" );
  if ( pszTUV2End != NULL )   pszTUV3Start = wcsstr( pszTUV2End, L"<tuv " );

  // check number of TUVs
  if ( pszTUV3Start != NULL )
  {
    sprintf( pData->szBuffer, "Translation unit starting in line %ld contains more than 2 TUVs and is ignored", pData->iTuLine );
    ShowError( pData, pData->szBuffer, pData->szInMemory );
    if ( pData->hfLog ) fwrite( pData->szTuBuffer, pData->iTuSize, sizeof(CHAR_W), pData->hfLog );
    if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pData->iTuSize, sizeof(CHAR_W), pData->hOutFile );
    usRC = 1;
  }
  else if ( pszTUV2End == NULL )
  {
    sprintf( pData->szBuffer, "Translation unit starting in line %ld does not contain 2 TUVs and is copied as-is", pData->iTuLine );
    ShowError( pData, pData->szBuffer, pData->szInMemory );
    if ( pData->hfLog ) fwrite( pData->szTuBuffer, pData->iTuSize, sizeof(CHAR_W), pData->hfLog );
    if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pData->iTuSize, sizeof(CHAR_W), pData->hOutFile );
    usRC = 2;
  } /* endif */     

  // find segment data in TUVs
  if ( usRC == NO_ERROR )
  {
    if ( !FindSegDataW( pszTUV1Start, pszTUV1End, &pszSeg1Start, &pszSeg1End ) || !FindSegDataW( pszTUV2Start, pszTUV2End, &pszSeg2Start, &pszSeg2End ) )
    {
      sprintf( pData->szBuffer, "Could not locate segment data in translation unit starting in line %ld, tu is copied as-is", pData->iTuLine );
      ShowError( pData, pData->szBuffer, pData->szInMemory );
      if ( pData->hfLog ) fwrite( pData->szTuBuffer, pData->iTuSize, sizeof(CHAR_W), pData->hfLog );
      if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pData->iTuSize, sizeof(CHAR_W), pData->hOutFile );
      usRC = 3;
    } /* endif */     
  } /* endif */     

  // find sentence delimiters in segment data
  if ( usRC == NO_ERROR )
  {
    int iSegs1 = SplitSegDataW( pszSeg1Start, pszSeg1End, pszDelims1 );
    int iSegs2 = SplitSegDataW( pszSeg2Start, pszSeg2End, pszDelims2 );
    if ( iSegs1 != iSegs2 )
    {
      sprintf( pData->szBuffer, "Number of sentences in <tuvs> of translation unit starting in line %ld do not match, tu is copied as-is", pData->iTuLine );
      ShowError( pData, pData->szBuffer, pData->szInMemory );
      if ( pData->hfLog ) fwrite( pData->szTuBuffer, pData->iTuSize, sizeof(CHAR_W), pData->hfLog );
      if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pData->iTuSize, sizeof(CHAR_W), pData->hOutFile );
      usRC = 4;
    }
    else if ( iSegs1 == 1 )
    {
      // nothing to split, write TU as-is
      if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pData->iTuSize, sizeof(CHAR_W), pData->hOutFile );
    }
    else
    {
      // split segment data
      int i = 0;
      PSZ_W pszStart1 = pszSeg1Start;
      PSZ_W pszStart2 = pszSeg2Start;
      pData->iNumOfAddTUs -= 1;

      for ( i = 0; i < iSegs1; i++ )
      {
        // write tuv up to start of first sentence
        if ( pData->hOutFile ) fwrite( pData->szTuBuffer, pszSeg1Start - ((PSZ_W)pData->szTuBuffer), sizeof(CHAR_W), pData->hOutFile );

        // write current sentence 1st tuv
        if ( pData->hOutFile ) fwrite( pszStart1, pszDelims1[i] - pszStart1, sizeof(CHAR_W), pData->hOutFile );
        pszStart1 = pszDelims1[i];

        // write data up to next tuv seg data
        if ( pData->hOutFile ) fwrite( pszSeg1End, pszSeg2Start - pszSeg1End, sizeof(CHAR_W), pData->hOutFile );

        // write current sentence 2nd tuv
        if ( pData->hOutFile ) fwrite( pszStart2, pszDelims2[i] - pszStart2, sizeof(CHAR_W), pData->hOutFile );
        pszStart2 = pszDelims2[i];

        // write rest of tuv
        if ( pData->hOutFile ) fwrite( pszSeg2End, pData->iTuSize - (pszSeg2End - (PSZ_W)pData->szTuBuffer), sizeof(CHAR_W), pData->hOutFile );
        if ( pData->hOutFile ) fputs( "\r\n", pData->hOutFile );

        pData->iNumOfAddTUs += 1;
      } /* endfor */         
    } /* endif */   
  } /* endif */     


  return( usRC );
} /* end of function ProcessTranslationUnitW */

USHORT ConvertMemoryUTF16
(
  PTMXSPLITSEG_DATA pData              // Pointer to our data area
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  int iLineNum = 0;

  // open output memory
  if ( pData->szOutSpec[0]!= EOS )  
  {
    pData->hOutFile = fopen( pData->szOutSpec, "wb" );
    if ( pData->hOutFile == NULL )
    {
      usRC = ERROR_FILE_NOT_FOUND;
      ShowError( pData, "Error: Output memory %s could not be opened", pData->szOutSpec );
    } /* endif */
  } /* endif */     

  // get/write memory lines until done
  fgetws( (PSZ_W)pData->szReadBuffer, sizeof(pData->szReadBuffer)/sizeof(CHAR_W), pData->hInFile );
  while ( !feof( pData->hInFile ) )
  {
    iLineNum++;
    if ( StartsWithW( pData, (PSZ_W)pData->szReadBuffer, L"<tu " ) )
    {
      // read complete TU into buffer and process it
      int iFilled = 0;
      pData->iTuLine = iLineNum;
      do
      {
        int iLen = wcslen( (PSZ_W)pData->szReadBuffer ) * sizeof(CHAR_W);
        memcpy( pData->szTuBuffer + iFilled, pData->szReadBuffer, iLen );
        iFilled += iLen;
        fgetws( (PSZ_W)pData->szReadBuffer, sizeof(pData->szReadBuffer)/sizeof(CHAR_W), pData->hInFile );
        iLineNum++;
      } while ( !feof( pData->hInFile ) && !StartsWithW( pData, (PSZ_W)pData->szReadBuffer, L"</tu>" ) ); /* enddo */       
      if ( !feof( pData->hInFile  ) )
      {
        int iLen = wcslen( (PSZ_W)pData->szReadBuffer ) * sizeof(CHAR_W);
        memcpy( pData->szTuBuffer + iFilled, pData->szReadBuffer, iLen );
        pData->iTuSize = (iFilled + iLen) / sizeof(CHAR_W);
        ((PSZ_W)pData->szTuBuffer)[pData->iTuSize] = EOS;

        // process the TU
        ProcessTranslationUnitW( pData );
      } /* endif */         
    }
    else
    {
      if ( pData->hOutFile ) fputws( (PSZ_W)pData->szReadBuffer, pData->hOutFile );
    } /* endif */
    fgetws( (PSZ_W)pData->szReadBuffer, sizeof(pData->szReadBuffer)/sizeof(CHAR_W), pData->hInFile );
  } /* endwhile */     

  // close files
  if ( pData->hInFile ) fclose( pData->hInFile );
  if ( pData->hOutFile ) fclose( pData->hOutFile );

  return( usRC );
} /* end of function ConvertMemoryUTF16 */

// show error message
USHORT ShowError( PTMXSPLITSEG_DATA pData, PSZ pszMsg, PSZ pszParm )
{
  USHORT      usRC = NO_ERROR;         // function return code

  sprintf( pData->szMsgBuffer, pszMsg, pszParm );

  printf( "Error: %s\n", pData->szMsgBuffer );

  if ( pData->fLog )
  {
    fprintf( pData->hfLog, "Error: %s\n", pData->szMsgBuffer );
  } /* endif */
  if ( pData->fErrorLog )
  {
    fprintf( pData->hfErrorLog, "Error: %s\n", pData->szMsgBuffer );
  } /* endif */

  pData->iErrors += 1;
  return( usRC );
} /* end of function ShowError */

// show warning message
USHORT ShowWarning( PTMXSPLITSEG_DATA pData, PSZ pszMsg, PSZ pszParm )
{
  USHORT      usRC = NO_ERROR;         // function return code

  sprintf( pData->szMsgBuffer, pszMsg, pszParm );

  printf( "Warning: %s\n", pData->szMsgBuffer );

  if ( pData->fLog )
  {
    fprintf( pData->hfLog, "Warning: %s\n", pData->szMsgBuffer );
  } /* endif */
  if ( pData->fWarningLog )
  {
    fprintf( pData->hfWarningLog, "Warning: %s\n", pData->szMsgBuffer );
  } /* endif */

  pData->iWarnings += 1;
  return( usRC );
} /* end of function ShowWarning */


USHORT ShowInfo( PTMXSPLITSEG_DATA pData, PSZ pszMsg, PSZ pszParm )
{
  USHORT      usRC = NO_ERROR;         // function return code

  sprintf( pData->szMsgBuffer, pszMsg, pszParm );

  printf( "Info: %s\n", pData->szMsgBuffer );

  if ( pData->fLog )
  {
    fprintf( pData->hfLog, "Info: %s\n", pData->szMsgBuffer );
  } /* endif */
  if ( pData->fErrorLog )
  {
    fprintf( pData->hfErrorLog, "Info: %s\n", pData->szMsgBuffer );
  } /* endif */
  if ( pData->fWarningLog )
  {
    fprintf( pData->hfWarningLog, "Info: %s\n", pData->szMsgBuffer );
  } /* endif */
  return( usRC );
} /* end of function ShowInfo */

void showHelp()
{
    printf( "OtmTmxSplitSegments.EXE : TMX segment splitter\n" );
    printf( "Version                 : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright               : %s\n",STR_COPYRIGHT );
    printf( "Purpose                 : Split segments in a TMX memory at sentence boundaries\n" );
    printf( "Syntax format           : OtmTmxSplitSegments inputmem [outputmem] [/logs=[E|W|A] [/log=logfile]\n" );
    printf( "\n" );
    printf( "Options and parameters:\n" );
    printf( "    inputmem    the fully qualified name of the TMX input memory\n" );
    printf( "    outputmem   the fully qualified name of the TMX output memory\n" );
    //printf( "      The output memory contains the same data as the input memory except for segments\n" );
    //printf( "      consisting of more than one sentence, these segments are splitted into\n" );
    //printf( "      indiviudal sentences.\n" );
    //printf( "      When no output memory is specified, the tool checks the input memory but\n " );
    //printf( "      does not create an output memory.\n" );
    printf( "   /LOGS       allows to select which log files should be produced\n" );
    printf( "               Specify one or more of the following values:\n" );
    printf( "               E to create seperate error log file\n" );
    printf( "               W to create seperate warnings log file\n" );
    printf( "               A to create combined error/warning log file\n" ); 
    printf( "   /log        the fully qualified name for the log file. If not specified a log file\n" );
    printf( "               named TmxSplitSegments.LOG is written to the current directory. To switch off logging specify \"OFF\" as logfile name\n" );
}
