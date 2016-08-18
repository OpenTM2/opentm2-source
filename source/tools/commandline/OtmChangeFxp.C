//+----------------------------------------------------------------------------+
//| CHANGEFXP.C                                                         |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2015, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|   Change type of exported folder                                           |
//+----------------------------------------------------------------------------+
#undef  DLL
#undef  _WINDLL
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
#include <eqfserno.h>             // version number
#include "eqftmi.h"
#include <eqfdasdi.h>             // Private dictionary services defines ...
#include <eqfdde.h>               // batch mode definitions
#include <eqffol00.h>             // Private folder defines
#include "eqfutpck.h"             // our header file

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

PSZ pszInFile = NULL;
PSZ pszOutFile = NULL;
PSZ pszDelList = NULL;
PSZ pszDoc = "";
FILE     *hInput = NULL;               // input file handle
FILE     *hOutput = NULL;              // output file handle
PACKHEADER2 PackageHeader;
PACKHEADER OldPackHead;
BOOL fDetails = FALSE;

char szLongDocName[512];               // buffer for long document names
char szFolMarkup[MAX_FILESPEC];        // markup as defined in folder
char szInFile[1024];                   // buffer for input file name
char szDate[128];
char szBuffer[8096];
char szName[512];
char szPassword[512];

BYTE bPropBuffer[60000];               // buffer for property files
PROPFOLDER FolProp;
PROPDOCUMENT DocProp;

FILELIST   FileList;                   // package's file list

BOOL GetPackFile
(
   FILE           *hPackage,           // handle of package file
   PFILELISTENTRY pEntry,              // file to read
   PBYTE          pBuffer,             // buffer for file data
   int            iBufSize             // size of buffer area
);
BOOL UpdatePackFile
(
   FILE           *hPackage,           // handle of package file
   PFILELISTENTRY pEntry,              // file to read
   PBYTE          pBuffer,             // buffer for file data
   int            iBufSize             // size of buffer area
);

static void showHelp();

void LongToDateTime( LONG lTime, char *pszDate )
{
  struct tm   *pTimeDate;    // time/date structure

  // correction: + 3 hours
  if ( lTime != 0L )
  {
    lTime += 10800L;
  } /* endif */


  pTimeDate = localtime( &lTime );
  sprintf( pszDate, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d", pTimeDate->tm_year + 1900,
           pTimeDate->tm_mon + 1, pTimeDate->tm_mday, pTimeDate->tm_hour,
           pTimeDate->tm_min, pTimeDate->tm_sec );
}

PSZ Utlstrccpy( PSZ pszTarget, PSZ pszSource, CHAR chStop )
{
   PSZ pTemp;

   pTemp = pszTarget;
   while ( *pszSource && (*pszSource != chStop) )
   {
      *pszTarget++ = *pszSource++;
   } /* endwhile */
   *pszTarget = NULC;
   return( pTemp );
}

PSZ UtlGetFnameFromPath( PSZ path)
{
  PSZ   pszFileName;                   // ptr to begin of filename

  if( (pszFileName = strrchr( path, BACKSLASH)) != NULL )
    pszFileName++;
  return( pszFileName );
}

void ShowFolderType( PSZ pszPrefix, PPROPFOLDER pFolProp, PFOLEXPHEADER pFolderHeader )
{
  printf( "%s : ", pszPrefix );
  if ( pFolderHeader->BitFlags.fMasterFolder )
  {
    printf( "Master folder\n" );
  }
  else if ( pFolProp->fAFCFolder )
  {
    printf( "Child folder\n" );
  }
  else
  {
    printf( "Standard folder\n" );
  } /* endif */
}

BOOL UpdateFolderHeader
(
   FILE           *hPackage,           // handle of package file
   PFOLEXPHEADER pFolderHeader,        // new folder header
   int            iLen                 // size of folder header
)
{
  BOOL fOK = TRUE;

  // position to begin of user header
  if ( (memcmp( PackageHeader.bPackID, PACKHEADID, 4 ) == 0) ||
       (memcmp( PackageHeader.bPackID, PACKHEAD2ID, 4 ) == 0) )
  {
    // version 0 or 1 header
    fseek( hPackage, sizeof(PACKHEADER), SEEK_SET );
  }
  else
  {
    // version 2 header
    fseek( hPackage, sizeof(PACKHEADER2), SEEK_SET );
  } /* endif */

  // read user header into the given buffer (in 32k blocks)
  if ( fwrite( pFolderHeader, iLen, 1, hPackage) != 1 )
  {
    fOK = FALSE;
  } /* endif */
  return( fOK );
}

int main
(
  int  argc,
  char *argv[],
  char *envp[]
)
{
  BOOL fOK = TRUE;
  ULONG ulLength;                      // overall file length
  PFOLEXPHEADER pFolderHeader = NULL;
  PFILELISTENTRY pFileEntry;          // ptr to an entry in a file list
  BOOL fTypChange = FALSE;
  enum { MASTER_FOLDER_TYPE, CHILD_FOLDER_TYPE, STANDARD_FOLDER_TYPE } NewFolderType = STANDARD_FOLDER_TYPE;
  char szShipment[20];

  envp;

  // show help information if needed
  if(argc==2 && stricmp(argv[1],"-h")==0)
  {
      showHelp();
      return 0;
  }

  memset( &FileList, 0, sizeof(FileList ) );
  szPassword[0] = '\0';
  szShipment[0] = '\0'; 

  /* Show title line                                                  */
  printf( "OtmChangeFxp: Modify properties of exported TranslationManager folders\n\n" );

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
      if ( strnicmp( pszArg + 1, "TYPE=", 5 ) == 0 )
      {
        fTypChange = TRUE;
        if ( stricmp( pszArg + 6, "MASTER" ) == 0 )
        {
          NewFolderType = MASTER_FOLDER_TYPE;
        }
        else if ( stricmp( pszArg + 6, "CHILD" ) == 0 )
        {
          NewFolderType = CHILD_FOLDER_TYPE;
        }
        else if ( stricmp( pszArg + 6, "STANDARD" ) == 0 )
        {
          NewFolderType = STANDARD_FOLDER_TYPE;
        }
        else
        {
          printf( "Error: Unknown folder type \"%s\", valid are MASTER, CHILD, or STANDARD.\n", pszArg + 6);
          fOK = FALSE;
        } /* endif */

      }
      else if ( strnicmp( pszArg + 1, "PW=", 3 ) == 0 )
      {
        strcpy( szPassword, pszArg + 4 );
      }
      else if ( strnicmp( pszArg + 1, "SHIP=", 5 ) == 0 )
      {
        int iShipment = atol( pszArg + 6 );
        if ( (iShipment == 0) && (iShipment > 999) )
        {
          printf( "Error: Invalid value \"%s\" for shipment number, valid are numeric values from 1 to 999.\n", pszArg + 6);
          fOK = FALSE;
        }
        else
        {
          strcpy( szShipment, pszArg + 6 );
        } /* endif */           
      }
      else
      {
        printf( "Warning: Unknown commandline switch '%s' is ignored.\n", pszArg );
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


  if ( fOK )
  {
    if ( pszInFile == NULL )
    {
      printf( "Error: Missing file name of exported folder.\n" );
      showHelp();
      fOK = FALSE;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    if ( (NewFolderType == MASTER_FOLDER_TYPE) || (NewFolderType == CHILD_FOLDER_TYPE) )
    {
      if ( szPassword[0] == '\0')
      {
        printf( "Error: Missing password for change to MASTER or CHILD folder.\n" );
        printf( "   specify the password using the /PW= parameter\n" );
        fOK = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */


  /********************************************************************/
  /* Open input file                                                  */
  /********************************************************************/
  if ( fOK )
  {
    PSZ pszExtension;

    strcpy( szInFile, pszInFile );
    pszExtension = strrchr( szInFile, '\\' );
    if ( pszExtension == NULL )
    {
      pszExtension = szInFile;
    } /* endif */

    // append default extension check if none specified
    if ( strchr( pszExtension , '.' ) == NULL )
    {
      strcat( szInFile, ".FXP" );
    } /* endif */

    // open input file
    hInput = fopen( szInFile, "r+b" );
    if ( hInput == NULL )
    {
      printf( "Error: Exported folder file \"%s\" could not be opened.\n", szInFile );
      fOK = FALSE;
    }
    else
    {
      ulLength = _filelength( _fileno(hInput) );
    } /* endif */
  } /* endif */

  // read system package header ---
  if ( fOK )
  {
    int iRead = fread( &PackageHeader, 1, sizeof(PackageHeader), hInput );
    if ( iRead != sizeof(PackageHeader) )
    {
      printf( "Error: Read of package header failed\n" );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // convert package header to new format if necessary and correct
  // file pointer position
  if ( fOK )
  {
    if ( (memcmp( PackageHeader.bPackID, PACKHEADID, 4 ) == 0) ||
         (memcmp( PackageHeader.bPackID, PACKHEAD2ID, 4 ) == 0) )
    {
      if ( fDetails ) printf( "Info: Header has old format\n" );
      // old format header


      // convert to new layout
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

  // read user header
  if ( fOK )
  {
    pFolderHeader = (PFOLEXPHEADER) malloc( PackageHeader.ulUserHeaderSize );

    if ( !pFolderHeader )
    {
      fOK = FALSE;
      printf( "Error: Allocation of %lu bytes for folder header area failed.\n", PackageHeader.ulUserHeaderSize );
    }
    else
    {
      // position to begin of user header
      if ( (memcmp( PackageHeader.bPackID, PACKHEADID, 4 ) == 0) ||
           (memcmp( PackageHeader.bPackID, PACKHEAD2ID, 4 ) == 0) )
      {
        // version 0 or 1 header
        fseek( hInput, sizeof(PACKHEADER), SEEK_SET );
      }
      else
      {
        // version 2 header
        fseek( hInput, sizeof(PACKHEADER2), SEEK_SET );
      } /* endif */

      // read user header into the given buffer (in 32k blocks)
      if ( fread( pFolderHeader, PackageHeader.ulUserHeaderSize, 1, hInput ) != 1 )
      {
        fOK = FALSE;
        printf( "Error: Read of user header area failed,\n" );
      } /* endif */
    } /* endif */
  } /* endif */


  // read file list
  if ( fOK )
  {
    FileList.pEntries = (PFILELISTENTRY)malloc( PackageHeader.ulFileListSize );
    FileList.pBuffer  = (PSZ)malloc( PackageHeader.ulFileNameBufferSize );
    if ( (FileList.pEntries == NULL) || (FileList.pBuffer == NULL) )
    {
      printf("Error: Allocation of file list data area failed.\n" );
      fOK = FALSE;
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
        fOK = FALSE;
        printf( "Error: Read of file list failed,\n" );
      }
      else
      {
        FileList.ulListSize = FileList.ulListUsed = PackageHeader.ulFileListEntries;
        if ( fread( FileList.pBuffer, PackageHeader.ulFileNameBufferSize, 1, hInput ) != 1 )
        {
          fOK = FALSE;
          printf( "Error: Read of file name buffer failed,\n" );
        }
        else
        {
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

  // modify folder and document properties in package
  if ( fOK )
  {
    ULONG ulI = 0;
    PFILELISTENTRY pFileEntry = FileList.pEntries;

    memset( &FolProp, 0, sizeof(FolProp) );

    for ( ulI = 0; ulI < PackageHeader.ulFileListEntries; ulI++ )
    {
      if ( pFileEntry->usFileType == FOLDER_PROP_FILE )
      {
        BOOL fHeaderChanged = FALSE;
        BOOL fPropFileChanged = FALSE;

        GetPackFile( hInput, pFileEntry, (PBYTE)&FolProp, sizeof(FolProp) );

        if ( fTypChange)
        {
          printf( "Changing folder type\n" );
          ShowFolderType( "Old folder type", &FolProp, pFolderHeader );

          if ( NewFolderType == MASTER_FOLDER_TYPE )
          {
            if ( !pFolderHeader->BitFlags.fMasterFolder )
            {
              pFolderHeader->BitFlags.fMasterFolder = TRUE;
              fHeaderChanged = TRUE;
            } /* endif */
            if ( !FolProp.fAFCFolder )
            {
              FolProp.fAFCFolder = TRUE;
              strcpy( FolProp.szAFCPassword, szPassword );
              fPropFileChanged = TRUE;
            } /* endif */
            if ( !FolProp.fTCMasterFolder )
            {
              FolProp.fTCMasterFolder = TRUE;
              strcpy( FolProp.szAFCPassword, szPassword );
              fPropFileChanged = TRUE;
            } /* endif */
          }
          else if ( NewFolderType == CHILD_FOLDER_TYPE )
          {
            if ( pFolderHeader->BitFlags.fMasterFolder )
            {
              pFolderHeader->BitFlags.fMasterFolder = FALSE;
              fHeaderChanged = TRUE;
            } /* endif */
            if ( !FolProp.fAFCFolder )
            {
              FolProp.fAFCFolder = TRUE;
              strcpy( FolProp.szAFCPassword, szPassword );
              fPropFileChanged = TRUE;
            } /* endif */
          }
          else if ( NewFolderType == STANDARD_FOLDER_TYPE )
          {
            if ( pFolderHeader->BitFlags.fMasterFolder )
            {
              pFolderHeader->BitFlags.fMasterFolder = FALSE;
              fHeaderChanged = TRUE;
            } /* endif */
            if ( FolProp.fAFCFolder )
            {
              FolProp.fAFCFolder = FALSE;
              fPropFileChanged = TRUE;
            } /* endif */
          } /* endif */

          ShowFolderType( "New folder type", &FolProp, pFolderHeader );
        } /* endif */

        if ( szShipment != '\0' )
        {
          strcpy( FolProp.szShipment, szShipment );
          fPropFileChanged = TRUE;
        } /* endif */

        if ( fPropFileChanged ) UpdatePackFile( hInput, pFileEntry, (PBYTE)&FolProp, sizeof(FolProp) );
        if ( fHeaderChanged ) UpdateFolderHeader( hInput, pFolderHeader, (int)PackageHeader.ulUserHeaderSize );

        if ( szShipment != '\0' )
        {
          printf( "Done!\n" );
          break;
        } /* endif */           
      }
      else if ( (pFileEntry->usFileType == DOCUMENT_PROP_FILE) && (szShipment != '\0') )
      {
        GetPackFile( hInput, pFileEntry, (PBYTE)&DocProp, sizeof(DocProp) );
        strcpy( DocProp.szShipment, szShipment );
        UpdatePackFile( hInput, pFileEntry, (PBYTE)&DocProp, sizeof(DocProp) );
      } /* endif */
      pFileEntry++;
    } /* endfor */
  } /* endif */


  // cleanup

  if ( hInput )        fclose( hInput );
  if ( pFolderHeader ) free( pFolderHeader );
  if ( FileList.pEntries ) free( FileList.pEntries );
  if ( FileList.pBuffer ) free( FileList.pBuffer );

  // return to system
  return( !fOK );
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
   fseek( hPackage, pEntry->ulFilePos, SEEK_SET );

   // read file into buffer
   memset( pBuffer, 0, iBufSize );
   iLen = min( iBufSize, (int)pEntry->ulFileSize );
   fread( pBuffer, iLen, 1, hPackage );

   return( TRUE );
} /* end of GetPackFile */

// update a file in the package 
BOOL UpdatePackFile
(
   FILE           *hPackage,           // handle of package file
   PFILELISTENTRY pEntry,              // file to update
   PBYTE          pBuffer,             // buffer for file data
   int            iBufSize             // size of buffer area
)
{
   // position to start of requested file in package file
   fseek( hPackage, pEntry->ulFilePos, SEEK_SET );

   // update the file from the buffer
   fwrite( pBuffer, iBufSize, 1, hPackage );

   return( TRUE );
} /* end of GetPackFile */

void showHelp()
{
    printf("\n");
    printf( "OtmChangeFxp.EXE         : Folder type changer\n" );
    printf( "Version               : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright             : %s\n",STR_COPYRIGHT );
    printf( "Purpose               : Change the type of exported folders\n" );
    printf( "%-20s","Syntax format         : OtmChangeFxp folderfile [/TYPE=[MASTER|CHILD|STANDARD]] [/PW=password] [/SHIP=shipment]\n" );
    printf( "Options and parameters:\n" );
    printf( "    folderfile          the fully qualified name of an exported folder \n" );
    printf( "                        if no file extension is given \".FXP\" is assumed \n\n" );
    printf( "    /TYPE               the new type of the folder and can be MASTER, CHILD,\n");
    printf( "                        or STANDARD \n\n");
    printf( "    /PW                 the (new) password for the folder. This option has to \n" );
    printf( "                        be specified when the type of the folder is changed \n");
    printf( "                        to MASTER or CHILD \n\n" );
    printf( "    /SHIP               the shipment number for the documents in the folder.\n"); 
    printf( "                        is associated with all document in the folder,in the \n" );
    printf( "                        range from 1 to 999\n" );
}
