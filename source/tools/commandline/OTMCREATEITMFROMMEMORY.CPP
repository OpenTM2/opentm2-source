//+----------------------------------------------------------------------------+
//| OtmCreateITMFromMemory.C                                                   |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2015, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//+----------------------------------------------------------------------------+

#include <time.h>
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_DLGUTILS         // dialog utilities
#include <eqf.h>                  // General Translation Manager include file
#include <eqfserno.h>             // version number
#include "OTMFUNC.H"
#include "OtmProposal.h"
#include "core\pluginmanager\OtmMemory.h"
#include "core\memory\MemoryFactory.h"

// several constants used in the following code
#define TRUE  1
#define FALSE 0
#define NULC '\0'
#define WRONG_OPT 255

// some macros ...
#define SKIPSPACE(p)  while (*p == ' ' ) p++
#define SKIPCOMMA(p)  if (*p == ',' ) p++

char szMsg[2048];                      // buffer for TranslationManager messages


typedef struct _PROGRAM_DATA
{
  BOOL fProcessID;
  BOOL fQuiet;
  BOOL fDateTime;
  FILE *hfLog;
  char szLogFile[1024];                  // name of log file
  char szProgName[512];
  char szInMem[512];
  char szOutMem[512];
  char szBuffer[8096];
  OtmMemory *pInMem;
  OtmMemory *pOutMem;
  WCHAR szSegment[MAX_SEGMENT_SIZE+1];
  char szSourceLang[512];
} PROGRAM_DATA, *PPROGRAM_DATA;

void Out_FormString( PPROGRAM_DATA pData, PSZ pszFormat, PSZ pszParm );
void Out_Msg( PPROGRAM_DATA pData, HSESSION hSession );
void Out_RC( PPROGRAM_DATA pData, PSZ pszFunction, int iRC );
void Out_String( PPROGRAM_DATA pData, PSZ pszString );
void Out_TimeStamp( PPROGRAM_DATA pData, PSZ pszString );
void Out_Buffer( PPROGRAM_DATA pData );
void GetDateTime( PSZ pszBuffer );

static void showHelp();

char szLastSegDocName[256] = "";

static PROGRAM_DATA Data;

int main
(
  int argc,
  char *argv[],
  char *envp[]
)
{
  HSESSION hSession = 0L;
  int   iRC = 0;
  int fOK = TRUE;
  char *pszTemp;
  MemoryFactory *pFactory = NULL;

  envp;

  if(argc==2 && stricmp(argv[1],"-h")==0 )
  {
      showHelp();
      return 0;
  }

  memset( &Data, 0, sizeof(Data) );

  // get program name (for the use in our messages)
  pszTemp = strrchr( argv[0], '\\' );
  if ( pszTemp == NULL )
  {
    pszTemp = argv[0];
  }
  else
  {
    pszTemp++;
  } /* endif */
  Utlstrccpy( Data.szProgName, pszTemp, '.' );
  argc--;
  argv++;

  // process arguments
  Data.szLogFile[0] = '\0';

  while ( argc )
  {
    PSZ pszParm = argv[0];
    if ( (*pszParm == '-') || (*pszParm == '/') )
    {
      if ( strnicmp( pszParm+1, "in=", 3 ) == 0 )
      {
        strcpy( Data.szInMem, pszParm+4 );
      }
      else if ( strnicmp( pszParm+1, "inmem=", 6 ) == 0 )
      {
        strcpy( Data.szInMem, pszParm+7 );
      }
      else if ( strnicmp( pszParm+1, "out=", 4 ) == 0 )
      {
        strcpy( Data.szOutMem, pszParm+5 );
      }
      else if ( strnicmp( pszParm+1, "outmem=", 7) == 0 )
      {
        strcpy( Data.szOutMem, pszParm+8 );
      }
      else
      {
        Out_FormString( &Data, "WARNING==> unknown option \'%s\' is ignored\n", pszParm );
      } /* endif */
    }
    else
    {
      Out_FormString( &Data, "WARNING==> superfluos command line parameter \'%s\' is ignored\n", pszParm );
    } /* endif */
    argc--;
    argv++;
  } /* endwhile */

  // check manadatory parameters
  if ( fOK )
  {
    if ( Data.szInMem[0] == EOS )
    {
      Out_String( &Data, "ERROR==> missing name of input memory\n" );
      fOK = FALSE;
    } /* endif */

    if ( Data.szOutMem[0] == EOS )
    {
      Out_String( &Data, "ERROR==> missing name of output memory\n" );
      fOK = FALSE;
    } /* endif */

    if ( !fOK )
    {
      Out_String( &Data, "OtmCreateITMFromMemory.EXE : Create source-source memory\n" );
      Out_String( &Data, "Version                    : ");
      Out_String( &Data, STR_DRIVER_LEVEL_NUMBER );
      Out_String( &Data, "\n");
      Out_String( &Data, "Copyright                  : 1990-2015, International Business Machines Corporation and others. All rights reserved\n" );
      Out_String( &Data, "Purpose                    : Convert a bilingual memory into a source-source memory\n" );
      Out_String( &Data, "Syntax format              : OtmCreateITMFromMemory /in[mem]=inmemory /out[mem]=outmemory\n" );
      Out_String( &Data, "Options and parameters     :\n" );
      Out_String( &Data, "    /IN    the name of an existing memory\n" );
      Out_String( &Data, "            (already imported into TM)\n" );
      Out_String( &Data, "    /OUT   the name of a new output memory\n" );
      Out_String( &Data, "            (created as internal TM memory)\n" );
    } /* endif */
  } /* endif */

  // open log file (if any)
  if ( fOK && (Data.szLogFile[0] != '\0'))
  {
    Data.hfLog = fopen( Data.szLogFile, "w" );
    if ( Data.hfLog  == NULL )
    {
      Out_FormString( &Data, "ERROR - Log file %s could not be opened\n", Data.szLogFile );
    } /* endif */
  } /* endif */

  // show logo
  Out_FormString( &Data, "%s - The NLV memory to source-source memory converter\n", "" );

  // start a batch TM session
  if ( fOK )
  {
    iRC = EqfStartSession( &hSession );
    if ( iRC  )
    {
      Out_RC( &Data, "EqfStartSession", iRC );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( fOK)
  {
    pFactory = MemoryFactory::getInstance();
  } /* endif */



  // open input memory 
  if ( fOK)
  {
    int iRC = 0;
    Data.pInMem = pFactory->openMemory( NULL, Data.szInMem, NONEXCLUSIVE, &iRC );
    fOK = (iRC == 0);

    if ( !fOK )
    {
      Out_FormString( &Data, "ERROR==> Cannot open input memory %s\n", Data.szInMem );
    } /* endif */
  } /* endif */

  // get source language of input memory
  if ( fOK )
  {
    Data.pInMem->getSourceLanguage( Data.szSourceLang, sizeof(Data.szSourceLang) );
  } /* endif */

  // create output memory 
  if ( fOK)
  {
    int iRC = 0;

    OtmMemoryPlugin *pPlugin = (OtmMemoryPlugin *)Data.pInMem->getPlugin();

    Data.pOutMem = pFactory->createMemory( (PSZ)pPlugin->getName(), Data.szOutMem, "", Data.szSourceLang, &iRC );
    fOK = (iRC == 0);
    if ( !fOK ) Out_FormString( &Data, "ERROR==> Cannot create output memory %s\n", Data.szOutMem );
  } /* endif */

  // process input memory
  if ( fOK )
  {
    BOOL fFirstExtract = TRUE;
    OtmProposal Prop;
    int iRC = 0;
    LONG lInSegs = 0;
    LONG lOutSegs = 0;

    do
    {
      // extract next segment
      if ( fFirstExtract )
      {
        iRC = Data.pInMem->getFirstProposal( Prop );
        fFirstExtract = FALSE;
      }
      else
      {
        iRC = Data.pInMem->getNextProposal( Prop );
      } /* endif */

      // write segment to output memory
      if ( iRC == 0 )
      {
        lInSegs++;
        Prop.getSourceLanguage( Data.szSourceLang, sizeof(Data.szSourceLang) );
        Prop.setTargetLanguage( Data.szSourceLang );

        Prop.getSource( Data.szSegment, sizeof(Data.szSegment) / sizeof(CHAR_W) );
        Prop.setTarget( Data.szSegment );

        iRC = Data.pOutMem->putProposal( Prop );

        if ( iRC != 0 )
        {
          sprintf( Data.szBuffer, "ERROR==> Could not write segment %ld to the output memory %s\n", lInSegs, Data.szOutMem );
          Out_String( &Data, Data.szBuffer );
          fOK = FALSE;
        }
        else
        {
          lOutSegs++;
        } /* endif */
      } /* endif */
    } while ( fOK && (iRC == NO_ERROR) );
    sprintf( Data.szBuffer, "INFO==> %ld segments written to output memory %s\n", lOutSegs, Data.szOutMem );
    Out_String( &Data, Data.szBuffer );
  } /* endif */

  // cleanup

  if ( Data.pInMem != NULL ) pFactory->closeMemory( Data.pInMem );
  if ( Data.pOutMem != NULL ) pFactory->closeMemory( Data.pOutMem );
  if ( Data.hfLog ) fclose( Data.hfLog );
  if ( hSession ) EqfEndSession( hSession );

  return( iRC );
} /* end of function main */



// output functions

// output string with format
void Out_FormString( PPROGRAM_DATA pData, PSZ pszFormat, PSZ pszParm )
{
  sprintf( pData->szBuffer, pszFormat, pszParm );
  Out_Buffer( pData );
}

void Out_Msg( PPROGRAM_DATA pData, HSESSION hSession )
{
  USHORT usRC = 0;
  EqfGetLastError( hSession, &usRC, szMsg, sizeof(szMsg) );
  sprintf( pData->szBuffer, "\tMessage %u\n", usRC );
  Out_Buffer( pData );

  // write message text line by line to output
  {
    PSZ pszSource = szMsg;
    do
    {
      PSZ pszTarget = pData->szBuffer;
      *pszTarget++ = '\t';
      while ( (*pszSource != '\n') && *pszSource ) *pszTarget++ =* pszSource++;
      *pszTarget++ = '\n';
      *pszTarget = '\0';
      Out_Buffer( pData );
      if ( *pszSource == '\n') pszSource++;
    } while ( *pszSource );
  }
}

void Out_TimeStamp( PPROGRAM_DATA pData, PSZ pszString )
{
  CHAR szDateTime[20];
  GetDateTime( szDateTime );
  sprintf( pData->szBuffer, "%s %s\n", pszString, szDateTime );
  Out_Buffer( pData );
}

void Out_RC( PPROGRAM_DATA pData, PSZ pszFunction, int iRC )
{
  sprintf( pData->szBuffer, "\tInfo==>%s returned %ld\n", pszFunction, iRC );
  Out_Buffer( pData );
}

void Out_String( PPROGRAM_DATA pData, PSZ pszString )
{
  strcpy( pData->szBuffer, pszString );
  Out_Buffer( pData );
}

// write string prepared in buffer
void Out_Buffer( PPROGRAM_DATA pData )
{
  if ( pData->szLogFile[0] )
  {
    pData->hfLog = fopen( pData->szLogFile, "a" );
  }

  if ( pData->fDateTime )
  {
    CHAR szDateTime[20];
    GetDateTime( szDateTime );

    printf( szDateTime );
    if ( pData->hfLog  )
    {
      fprintf( pData->hfLog, szDateTime  );
    }
  } /* endif */

  if ( pData->fProcessID )
  {
    DWORD dwProcessID = GetCurrentProcessId();
    printf( "%5ld ", dwProcessID );
    if ( pData->hfLog  )
    {
      fprintf( pData->hfLog, "%5ld ", dwProcessID );
    }
  } /* endif */


  printf( "%s", pData->szBuffer );
  if ( pData->hfLog  )
  {
    fprintf( pData->hfLog, "%s", pData->szBuffer );
  }
}

void GetDateTime( PSZ pszBuffer )
{
  CHAR szDate[12], szDate2[12], szTime[12];

  _strtime( szTime );
  _strdate( szDate );

  // strdat formats the date in the form mm/dd/yy  so we have to exchange the fields
  szDate2[0] = szDate[6];
  szDate2[1] = szDate[7];
  szDate2[2] = szDate[2];
  szDate2[3] = szDate[0];
  szDate2[4] = szDate[1];
  szDate2[5] = szDate[5];
  szDate2[6] = szDate[3];
  szDate2[7] = szDate[4];
  szDate2[8] = '\0';

  sprintf( pszBuffer, "%s %s ", szDate2, szTime );
}

void showHelp()
{
    printf( "OtmCreateITMFromMemory.EXE : Create source-source memory\n");
    printf( "Version                    : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright                  : %s\n",STR_COPYRIGHT );
    printf( "Purpose                    : Convert a bilingual memory into a source-source memory\n" );
    printf( "Syntax format              : OtmCreateITMFromMemory /in[mem]=inmemory /out[mem]=outmemory\n" );
    printf( "Options and parameters     :\n" );
    printf( "    /IN    the name of an existing memory\n" );
    printf( "            (already imported into TM)\n" );
    printf( "    /OUT   the name of a new output memory\n" );
    printf( "            (created as internal TM memory)\n" );
}
