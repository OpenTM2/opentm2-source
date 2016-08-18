//+----------------------------------------------------------------------------+
//| STAMPPROFILE.C                                                             |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2015, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  Stamp calculation profiles                                                |
//+----------------------------------------------------------------------------+

#undef  DLL
#undef  _WINDLL
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#define INCL_EQF_DLGUTILS         // set output file dialog
#define INCL_EQF_FOLDER           // folder list and document list functions

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_TP               // public translation processor functions


#include "eqf.h"                  // general TranslationManager include file
#include "eqfhlog.h"              // defines and structures of history log file
#include "eqfserno.h"             // version number

//typedef int BOOL;
typedef char CHAR;
typedef char * PCHAR;
typedef char * PSZ;
typedef unsigned char UCHAR;
typedef unsigned char BYTE;
typedef unsigned char * PBYTE;
typedef short SHORT;
typedef unsigned short USHORT;
typedef long LONG;
typedef unsigned long ULONG;
typedef void * PVOID;
//typedef unsigned long HWND;

#define TRUE   1
#define FALSE  0

#define END_OF_STRING     '\0'
#define EOS               '\0'

#define MAX_FNAME                9
#define NUM_OF_FOLDER_DICS      10
#define MAX_LANG_LENGTH         20
//#define MAX_PATH               144

#include "eqfdde.h"
#include "EQFRPT.H"
#include "EQFRPT00.H"


PSZ pszInFile = NULL;
PSZ pszOutFile = NULL;
PSZ pszDelList = NULL;
PSZ pszDoc = "";
FILE     *hInput = NULL;               // input file handle
FILE     *hOutput = NULL;              // output file handle
CHAR  szName[9];                       // internal profile name
PROPFOLDER Folder;

static void showHelp();

int main
(
  int  argc,
  char *argv[],
  char *envp[]
)
{
  BOOL fOK = TRUE;                     // O.K. flag
  BOOL fClear = FALSE;                 // clear protection stamp
  PSZ p;

  if(argc==2 && stricmp(argv[1],"-h")==0)
  {
      showHelp();
      return 0;
  }

  szName[0] = '\0';

  /********************************************************************/
  /* Show title line                                                  */
  /********************************************************************/
  printf( "Calculation Profile Stamp Utility\n" );

  /********************************************************************/
  /* Skip first commandline argument (program name)                   */
  /********************************************************************/
  argc--;
  argv++;

  /********************************************************************/
  /* Check commandline arguments                                      */
  /********************************************************************/
  pszInFile = NULL;
  while ( fOK && argc )
  {
    PSZ pszArg;

    pszArg = *argv;
    if ( (pszArg[0] == '-') || (pszArg[0] == '/') )
    {
      /****************************************************************/
      /* Check for options                                            */
      /****************************************************************/
      if ( strnicmp( pszArg + 1, "CLEAR", 5) == 0 )
      {
        fClear = TRUE;
      }
      else if ( strnicmp( pszArg + 1, "NAME=", 5) == 0 )
      {
        if ( strlen( pszArg + 6 ) > 8 )
        {
          printf( "Warning: specified internal name \"%s\" is too long, name is ignored.\n",
                  pszArg + 6 );
        }
        else
        {
          strcpy( szName, pszArg + 6 );
        } /* endif */
      }
      else
      {
        printf( "Warning: Unknown commandline switch '%s' is ignored.\n",
                pszArg );
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* Treat as file name                                           */
      /****************************************************************/
      if ( pszInFile == NULL )
      {
        pszInFile = pszArg;
      }
      else
      {
        /**************************************************************/
        /* Invalid option or superfluous file name ...                */
        /**************************************************************/
        printf( "Warning: Superfluous commandline value '%s' is ignored.\n",
                pszArg );
      } /* endif */
    } /* endif */
    argc--;
    argv++;
  } /* endwhile */


  if ( pszInFile == NULL )
  {
    printf( "Error: Missing name of profile.\n\n" );
    showHelp();
    fOK = FALSE;
  } /* endif */


  /********************************************************************/
  /* Open profile                                                     */
  /********************************************************************/
  if ( fOK )
  {
    hInput = fopen( pszInFile, "r+b" );
    if ( hInput == NULL )
    {
      printf( "Error: CalcReport Profile \"%s\" could not be opened.\n", pszInFile );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Read profile                                                     */
  /********************************************************************/
  if ( fOK )
  {
    memset( &Folder, 0, sizeof(Folder) );
    fread( &Folder, 1, sizeof(Folder), hInput );
  } /* endif */

  /********************************************************************/
  /* Set or clear protection                                          */
  /********************************************************************/
  if ( fOK )
  {
    if ( fClear )
    {
      memset( Folder.szProtectStamp, 0, sizeof(Folder.szProtectStamp) );
      memset( Folder.szIntProfileName, 0, sizeof(Folder.szIntProfileName) );
      Folder.lCheckSum = 0;
    }
    else
    {
     strcpy( Folder.szIntProfileName, szName );
     memcpy( Folder.szProtectStamp, RPT_PROTECTSTAMP, sizeof(Folder.szProtectStamp) );
     Folder.lCheckSum = RptCalcProfileCheckSum( &Folder );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Set or clear protection                                          */
  /********************************************************************/
  if ( fOK )
  {
    fseek( hInput, 0, SEEK_SET );
    fwrite( &Folder, 1, sizeof(Folder), hInput );
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( hInput )       fclose( hInput );

  /********************************************************************/
  /* return to system                                                 */
  /********************************************************************/
  return( !fOK );
} /* end of main */

// checks stamp and checksum of a calculation profile
BOOL RptCheckProfileStamp( PPROPFOLDER pProp )
{
  BOOL fProtected = FALSE;

  LONG lCheckSum = RptCalcProfileCheckSum( pProp );

  if ( (lCheckSum == pProp->lCheckSum) && (memcmp( pProp->szProtectStamp, RPT_PROTECTSTAMP, sizeof(pProp->szProtectStamp)) == 0) )
  {
    fProtected = TRUE;
  } /* endif */

  return( fProtected );
} /* end of function RptCheckProfileStamp */


// computes the checksum for a calculation profile
LONG RptCalcProfileCheckSum( PPROPFOLDER pProp )
{
  LONG lFactor = 1;
  LONG lCheckSum = 0;
  int  iIndex = 0;
  LONG lOrgCheckSum = pProp->lCheckSum;
  PBYTE pbProp = (PBYTE)pProp;

  pProp->lCheckSum = 0;

  for( iIndex = sizeof(PROPHEAD); iIndex < sizeof(PROPFOLDER); iIndex++ )
  {
    lCheckSum += pbProp[iIndex] * lFactor;
    lFactor++;
    if ( lFactor > 16 ) lFactor = 1;
  } /* endfor */

  pProp->lCheckSum = lOrgCheckSum;

  return( lCheckSum  );
} /* end of function RptCalcProfileCheckSum */

void showHelp()
{
    printf( "OtmSampProfile.EXE : Calculation Profile Stamp Utility\n" );
    printf( "Version          : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright        : %s\n",STR_COPYRIGHT );
    printf( "Purpose          : Set a hidden stamp in a calculation profile to prevent manipulations\n" );
    printf( "Syntax format    : OtmSampProfile profname [/CLEAR]\n" );
    printf( "Options and parameters:\n" );
    printf( "   profname    the fully qualified calculation profile name\n" );
}
