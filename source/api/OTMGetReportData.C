/*! \brief OTMGetReportData.C 
	Copyright (c) 1999-2014, International Business Machines Corporation and others. All rights reserved.
*/

#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_PRINT            // print functions
#define INCL_FOLDER               // folder defines
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_ANALYSIS
#define INCL_EQF_DLGUTILS         // dialog utilities
#include <eqf.h>                  // General Translation Manager include file
#include "eqftmi.h"
#include <eqfdasdi.h>             // Private dictionary services defines ...
#include <eqfdde.h>               // batch mode definitions
#include <eqffol00.h>             // Private folder defines
#include "eqfutpck.h"             // our header file
#include "OtmGetReportData.H"

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

#define TRUE   1
#define FALSE  0

#define END_OF_STRING     '\0'
#define EOS               '\0'

#define MAX_FNAME                9
#define NUM_OF_FOLDER_DICS      10
#define MAX_LANG_LENGTH         20

PACKHEADER2 PackageHeader;
PACKHEADER OldPackHead;

FILELIST   FileList;                   // package's file list
CALCREPORTBINDATA CalcRepData;         // calculation report data

BOOL GetPackFile( FILE *hPackage, PFILELISTENTRY pEntry, PBYTE pBuffer,  int iBufSize );

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
  float flPayable = 0.0;

  envp; 

  argc--; argv++;     //skip program name

  if ( argc == 0 )
  {
    printf( "Error: Return code %ld (OTMGRD_ERROR_INVALID_PARMS)\n", OTMGRD_ERROR_INVALID_PARMS );
    return( OTMGRD_ERROR_INVALID_PARMS );
  }

  memset( szProfile, 0, sizeof(szProfile) );

  iRC = OtmGetReportData( argv[0], alWords, &flPayable, szProfile, sizeof(szProfile) );

  if ( iRC == 0 )
  {
    printf( "Success: TranslatedWords=%ld, PayableWords=%2.2f, Profile=%s\n", alWords[0], flPayable, szProfile ); 
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
    printf( "Error: Return code %ld (%s)\n", iRC, pszError );
  }

  return( iRC );
} /* end of main */

// extract calculation report data
int OtmGetReportData
(
  char       *pszFolderPackage,
  long       *plWords,
  float       *pflPayableWords,
  char        *pszProfileNameBuffer,
  int         iProfileNameBufferSize
)
{
  FILE *hInput = NULL;               // input file handle
  int iRC = 0;

  // check input parameters
  if ( iRC == 0 )
  {  
    if ( (pszFolderPackage == NULL) || (*pszFolderPackage == '\0') || 
         (plWords == NULL) || (pszProfileNameBuffer == NULL) || 
         (iProfileNameBufferSize <= 0) )
    {
      iRC = OTMGRD_ERROR_INVALID_PARMS;
    } /* endif */
  } /* endif */


  // Open package file
  if ( iRC == 0 )
  {
    hInput = fopen( pszFolderPackage, "rb" );
    if ( hInput == NULL )
    {
      iRC = OTMGRD_ERROR_FXP_NOTFOUND;
    } /* endif */
  } /* endif */

  // read system package header ---
  if ( iRC == 0 )
  {
    int iRead = fread( &PackageHeader, 1, sizeof(PackageHeader), hInput );
    if ( iRead != sizeof(PackageHeader) )
    {
      iRC = OTMGRD_ERROR_FXP_CORRUPTED;
    } /* endif */
  } /* endif */

  // convert package header to new format if necessary and correct file pointer position
  if ( iRC == 0 )
  {
    if ( (memcmp( PackageHeader.bPackID, PACKHEADID, 4 ) == 0) || (memcmp( PackageHeader.bPackID, PACKHEAD2ID, 4 ) == 0) )
    {
      memcpy( &OldPackHead, &PackageHeader, sizeof(OldPackHead) );
      memset( &PackageHeader, 0, sizeof(PackageHeader) );
      memcpy( &PackageHeader.bPackID, OldPackHead.bPackID, 4 );
      PackageHeader.usVersion              = OldPackHead.usVersion;
      PackageHeader.ulPackDate             = OldPackHead.ulPackDate;
      PackageHeader.usSequence             = OldPackHead.usSequence;
      PackageHeader.fCompleted             = OldPackHead.fCompleted;
      PackageHeader.fLastFileOfPackage     = OldPackHead.fLastFileOfPackage;
      PackageHeader.ulUserHeaderSize       = OldPackHead.usUserHeaderSize;
      PackageHeader.ulFileListSize         = OldPackHead.usFileListSize;
      PackageHeader.ulFileListEntries      = OldPackHead.usFileListEntries;
      PackageHeader.ulFileNameBufferSize   = OldPackHead.usFileNameBufferSize;

      // adjust file pointer position
      fseek( hInput, sizeof(PACKHEADER), SEEK_SET );
    } /* endif */
  } /* endif */

  // check header info
  if ( iRC == 0 )
  {
    if ( !PackageHeader.fCompleted )
    {
      iRC = OTMGRD_ERROR_FXP_CORRUPTED;
    }
    else if ( PackageHeader.usSequence != 1 )
    {
      iRC = OTMGRD_ERROR_FXP_CORRUPTED;
    } /* endif */
  } /* endif */

  // read file list
  if ( iRC == 0 )
  {
    memset( &FileList, 0, sizeof(FileList ) );
    FileList.pEntries = (PFILELISTENTRY)malloc( PackageHeader.ulFileListSize );
    FileList.pBuffer  = (PSZ)malloc( PackageHeader.ulFileNameBufferSize );
    if ( (FileList.pEntries == NULL) || (FileList.pBuffer == NULL) )
    {
      iRC = OTMGRD_ERROR_NOT_ENOUGH_MEMORY;
    }
    else
    {
      LONG lPos = 0;

      // position to file list
      if ( (memcmp( PackageHeader.bPackID, PACKHEADID, 4 ) == 0) ||
           (memcmp( PackageHeader.bPackID, PACKHEAD2ID, 4 ) == 0) )
      {
        // version 0 or 1 header
        lPos = sizeof(PACKHEADER);
      }
      else
      {
        // version 2 header
        lPos = sizeof(PACKHEADER2);
      } /* endif */
      lPos += PackageHeader.ulUserHeaderSize,
      fseek( hInput, lPos, SEEK_SET );

      // read file list and file names
      if ( fread( FileList.pEntries, PackageHeader.ulFileListSize, 1, hInput ) != 1 )
      {
        iRC = OTMGRD_ERROR_FXP_CORRUPTED;
      }
      else
      {
        FileList.ulListSize = FileList.ulListUsed = PackageHeader.ulFileListEntries;
        if ( fread( FileList.pBuffer, PackageHeader.ulFileNameBufferSize, 1, hInput ) != 1 )
        {
          iRC = OTMGRD_ERROR_FXP_CORRUPTED;
        }
        else
        {
          PFILELISTENTRY pFileEntry;          // ptr to an entry in a file list
          ULONG       ulNoOfEntries;         // no of file list entries to process

          FileList.ulBufferSize = FileList.ulBufferUsed = PackageHeader.ulFileNameBufferSize;

          // change offset of file names to pointers
          pFileEntry = FileList.pEntries;
          ulNoOfEntries = FileList.ulListUsed;
          while ( ulNoOfEntries )
          {
            ULONG ulOffs;
            ulOffs = (ULONG)pFileEntry->pszName;
            // fix for offsets corrupted bug!
            if ( ulOffs > FileList.ulBufferUsed )
            {
              ulOffs &= 0x0000FFFF;
            } /* endif */
            pFileEntry->pszName = FileList.pBuffer + ulOffs;
            pFileEntry->usProcessFlags = 0;
            pFileEntry++;
            ulNoOfEntries--;
          } /* endwhile */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  // retrieve calculation report data file from package
  if ( iRC == 0 )
  {
    ULONG ulI = 0;
    PFILELISTENTRY pFileEntry = FileList.pEntries;

    for ( ulI = 0; ulI < PackageHeader.ulFileListEntries; ulI++ )
    {
      if ( pFileEntry->usFileType == BINCALCREPORT_FILE )
      {
        break;
      } /* endif */
      pFileEntry++;
    } /* endfor */

    if ( pFileEntry->usFileType == BINCALCREPORT_FILE )
    {
      memset( (PVOID)&CalcRepData, 0, sizeof(CalcRepData) );
      GetPackFile( hInput, pFileEntry, (PBYTE)&CalcRepData, sizeof(CalcRepData) );

      // set caller's word count fields 
      // GQ 2015/04/30: Exclude all auto-subst columns 
      plWords[0] = /* CalcRepData.ulAnaAustoSubst + */
        /* CalcRepData.ulEditAustoSubst + */ CalcRepData.ulEditExact + CalcRepData.ulEditReplace + 
        CalcRepData.ulEditFuzzy5070 + CalcRepData.ulEditFuzzy7190 + CalcRepData.ulEditFuzzy91 + CalcRepData.ulEditMachine +
        CalcRepData.ulEditManual; //  + CalcRepData.ulNotTranslated;
      plWords[1] = CalcRepData.ulPayable;
      *pflPayableWords = CalcRepData.flPayable;
      
      // fill caller's profile name field
      if ( (int)strlen( CalcRepData.szProfile ) < iProfileNameBufferSize ) 
      {
        strcpy( pszProfileNameBuffer, CalcRepData.szProfile ); 
      }
      else
      {
        iRC = OTMGRD_ERROR_BUFFER_TOO_SMALL;
      } /* endif */
    }
    else
    {
      iRC = OTMGRD_ERROR_NO_CALCREPORTDATA;
    } /* endif */
  } /* endif */


  // cleanup
  if ( hInput )        fclose( hInput );
  if ( FileList.pEntries ) free( FileList.pEntries );
  if ( FileList.pBuffer ) free( FileList.pBuffer );

  // return to caller
  return( iRC );
} /* end of main */


// read a file from the package into an buffer area
BOOL GetPackFile
(
   FILE           *hPackage,           // handle of package file
   PFILELISTENTRY pEntry,              // file to read
   PBYTE          pBuffer,             // buffer for file data
   int            iBufSize             // size of buffer area
)
{
   int      iLen;

   // position to start of requested file in package file
   _int64 iPos= (_int64)pEntry->ulFilePos;
   _fseeki64( hPackage, iPos, SEEK_SET );

   // read file into buffer
   memset( pBuffer, 0, iBufSize );
   iLen = min( iBufSize, (int)pEntry->ulFileSize );
   fread( pBuffer, iLen, 1, hPackage );

   return( TRUE );
} /* end of GetPackFile */
