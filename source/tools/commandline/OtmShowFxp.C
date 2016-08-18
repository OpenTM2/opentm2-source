//+----------------------------------------------------------------------------+
//| SHOWFXP.C                                                                  |
//+----------------------------------------------------------------------------+
// Copyright (C) 2012-2016, International Business Machines
// Corporation and others.  All rights reserved.
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//|                                                                            |
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

typedef struct _STRINGLIST
{
  ULONG ulEntries;
  ULONG ulBufSize;
  ULONG ulBufUsed;
  PSZ pszStrings;
} STRINGLIST, *PSTRINGLIST;

BOOL InitList( PSTRINGLIST pList );
BOOL FreeList( PSTRINGLIST pList );
BOOL AddToList( PSTRINGLIST pList, PSZ pszNewString, BOOL fCheckForDups );

static void showHelp();

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
char szBuffer[32000];
char szName[512];

BYTE bPropBuffer[60000];               // buffer for property files
PROPFOLDER FolProp;

FILELIST   FileList;                   // package's file list

STRINGLIST DocNames;
STRINGLIST DocComplete;
STRINGLIST DocWords;
STRINGLIST DocMarkups;
STRINGLIST MarkupList;
STRINGLIST DocLUDate;

// variables set during document scanning
BOOL fAllDocsCompleted = TRUE;         // all documents are completely translated
BOOL fTotalCountValid = TRUE;          // total word count is valid
int  iTotalCount = 0;                  // total word count



USHORT ShowFile( PFILELISTENTRY pEntry );
BOOL ScanDocumentProps
(
  FILE *hPackage,                    // package file
  PFILELIST pFileList,               // list of files in package
  int  *piDocuments                  // receives number of documents
);
BOOL GetPackFile
(
   FILE           *hPackage,           // handle of package file
   PFILELISTENTRY pEntry,              // file to read
   PBYTE          pBuffer,             // buffer for file data
   int            iBufSize             // size of buffer area
);

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
  BOOL fTMFolder = FALSE;             // TRUE for TranslationManager folder


  envp;

  if(argc==2 && stricmp(argv[1],"-h")==0 )
  {
      showHelp();
      return 0;
  }

  memset( &FileList, 0, sizeof(FileList ) );
  InitList( &DocNames );
  InitList( &DocComplete );
  InitList( &DocWords );
  InitList( &MarkupList );
  InitList( &DocMarkups );
  InitList( &DocLUDate );

  /* Show title line                                                  */
  printf( "OtmShowFxp: Show contents and info of exported OpenTM2 folder\n\n" );

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
      if ( strnicmp( pszArg + 1, "DETAILS", 7 ) == 0 )
      {
        fDetails = TRUE;
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
    printf( "Error: Missing file name of exported folder.\n\n" );
    showHelp();
    fOK = FALSE;
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
    hInput = fopen( szInFile, "rb" );
    if ( hInput == NULL )
    {
      printf( "Error: Exported folder file \"%s\" could not be opened.\n", szInFile );
      fOK = FALSE;
    }
    else
    {
      ulLength = _filelength( _fileno(hInput) );
      printf( "Listing contents of exported folder package %s\n\n", szInFile );
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

  // check header info
  if ( fOK )
  {
    if ( !PackageHeader.fCompleted )
    {
      printf( "Error: The folder export did not complete, folder package is not usable\n" );
      fOK = FALSE;
    }
    else if ( PackageHeader.usSequence != 1 )
    {
      printf( "Error: This is part %d of an exported foler, SHOWFXP only works with the first part of an exported folder.\n" );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // show header info
  if ( fOK )
  {
    PSZ pszOrigin = "unknown";

    // show origin of folder (TM2 or OpenTM2)
    if ( ((memcmp( PackageHeader.bPackID, PACKHEADID, 4 ) == 0) || (memcmp( PackageHeader.bPackID, PACKHEAD2ID, 4 ) == 0) || (memcmp( PackageHeader.bPackID, PACKHEAD3ID, 4 ) == 0)) &&
         (PackageHeader.usVersion >= 2) && (strcmp( PackageHeader.szToolID, "OTM" ) == 0 ) )
    {
      pszOrigin = "OpenTM2";
      fTMFolder = FALSE;
    }
    else
    {
      pszOrigin = "TM2";
      fTMFolder = TRUE;
    }
    printf( "Origin of folder       : %s\n", pszOrigin );


    LongToDateTime( PackageHeader.ulPackDate, szDate );
    if ( fDetails )
    {
      printf( "\nPackage header info\n" );
      printf( "   Internal version    : %u\n", PackageHeader.usVersion );
      printf( "   Date                : %s\n" , szDate );
      printf( "   Sequence            : %u\n", PackageHeader.usSequence );
      printf( "   Complete            : %s\n", ( PackageHeader.fCompleted ) ? "True" : "False" );
      printf( "   User header size    : %lu\n", PackageHeader.ulUserHeaderSize );
      printf( "   File list size      : %lu\n", PackageHeader.ulFileListSize );
      printf( "   File list entries   : %lu\n", PackageHeader.ulFileListEntries );
      printf( "   Name buffer size    : %lu\n", PackageHeader.ulFileNameBufferSize );
    }
    else
    {
//      printf( "Package date           : %s\n", szDate );
    } /* endif */
   } /* endif */

  // read user header
  if ( fOK )
  {
    pFolderHeader = (PFOLEXPHEADER)malloc( PackageHeader.ulUserHeaderSize );

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

  // show user header data
  if ( fOK )
  {
    if ( pFolderHeader->BitFlags.fHeaderType == RELEASE0_HEADER )
    {
      LongToDateTime( pFolderHeader->Head.Rel0.lDateTime, szDate );
    }
    else
    {
      LongToDateTime( pFolderHeader->Head.Rel1.lDateTime, szDate );
    } /* endif */

    if ( fDetails )
    {
      printf( "\nUser header info\n" );
      printf( "   Version             : %s\n", ( pFolderHeader->BitFlags.fHeaderType == RELEASE0_HEADER ) ? "Rel0" : "Rel1" );
      printf( "   Date of export      : %s\n" , szDate );
      printf( "   ContainsDict        : %s\n", ( pFolderHeader->fContainsDict ) ? "True" : "False" );
      printf( "   ContainsMem         : %s\n", ( pFolderHeader->BitFlags.fContainsMem ) ? "True" : "False" );
      printf( "   SelectedDocs        : %s\n", ( pFolderHeader->BitFlags.fSelectedDocs ) ? "True" : "False" );
      printf( "   OldVersion          : %s\n", ( pFolderHeader->BitFlags.fOldVers ) ? "True" : "False" );
      printf( "   Deleted             : %s\n", ( pFolderHeader->BitFlags.fDeleted ) ? "True" : "False" );
      printf( "   OverWrite           : %s\n", ( pFolderHeader->BitFlags.fOverWrite ) ? "True" : "False" );
      printf( "   WithNote            : %s\n", ( pFolderHeader->BitFlags.fWithNote ) ? "True" : "False" );
      printf( "   DocNameTable        : %s\n", ( pFolderHeader->BitFlags.fDocNameTable ) ? "True" : "False" );
      printf( "   WithDocTMs          : %s\n", ( pFolderHeader->BitFlags.fWithDocTMs ) ? "True" : "False" );
      printf( "   NonUnicode          : %s\n", ( pFolderHeader->BitFlags.fNonUnicode ) ? "True" : "False" );


      if ( pFolderHeader->BitFlags.fHeaderType == RELEASE0_HEADER )
      {
        printf( "   Description         : \"%s\"\n", pFolderHeader->Head.Rel0.szDescription );
        printf( "   Memory              : %s\n", pFolderHeader->Head.Rel0.szMemory );
        printf( "   Format              : %s\n", pFolderHeader->Head.Rel0.szFormat );
        printf( "   Dicts               : %s\n", pFolderHeader->Head.Rel0.DicTbl );
        printf( "   NoteSize            : %u\n", pFolderHeader->Head.Rel0.usNoteSize );
        if ( pFolderHeader->Head.Rel0.usNoteSize > 1 )
        {
          printf( "<<< Note >>>\n%s\n<< End of Note >>>\n", pFolderHeader->Head.Rel0.szNoteBuffer );
        } /* endif */
      }
      else
      {
        printf( "   Description         : \"%s\"\n", pFolderHeader->Head.Rel1.szDescription );
        printf( "   Format              : %s\n", pFolderHeader->Head.Rel1.szFormat );
        printf( "   NoteSize            : %u\n", pFolderHeader->Head.Rel1.usNoteSize );
        if ( pFolderHeader->Head.Rel1.usNoteSize > 1)
        {
          printf( "<<< Note >>>\n%s\n<< End of Note >>>\n", pFolderHeader->Head.Rel1.szNoteBuffer );
        } /* endif */
      } /* endif */
    }
    else
    {
      printf( "Date of export         : %s\n", szDate );

      szBuffer[0] = EOS;
      if ( pFolderHeader->fContainsDict )
        strcat( szBuffer, "With_Dictionaries " );
      if ( pFolderHeader->BitFlags.fContainsMem )
        strcat( szBuffer, "With_Memories " );
      if ( pFolderHeader->BitFlags.fSelectedDocs )
        strcat( szBuffer, "Selected_Documents_Only " );
      if ( pFolderHeader->BitFlags.fDeleted )
        strcat( szBuffer, "Delete_Folder_After_Export " );
      if ( pFolderHeader->BitFlags.fWithDocTMs )
        strcat( szBuffer, "With_Document_Memories " );
      if ( pFolderHeader->BitFlags.fNonUnicode )
        strcat( szBuffer, "In_nonUnicode_Format " );
      if ( szBuffer[0] != EOS )
        printf( "Export options         : %s\n", szBuffer );

      if ( pFolderHeader->BitFlags.fHeaderType == RELEASE0_HEADER )
      {
        printf( "Description            : %s\n", pFolderHeader->Head.Rel0.szDescription );
        printf( "Format                 : %s\n", pFolderHeader->Head.Rel0.szFormat );
        if ( pFolderHeader->Head.Rel0.usNoteSize > 1)
        {
          printf( "<<< Note >>>\n%s\n<< End of Note >>>\n", pFolderHeader->Head.Rel0.szNoteBuffer );
        } /* endif */
      }
      else
      {
        printf( "Description            : %s\n", pFolderHeader->Head.Rel1.szDescription );
        printf( "Format                 : %s\n", pFolderHeader->Head.Rel1.szFormat );
        if ( pFolderHeader->Head.Rel1.usNoteSize > 1 )
        {
          printf( "<<< Note >>>\n%s\n<< End of Note >>>\n", pFolderHeader->Head.Rel1.szNoteBuffer );
        } /* endif */
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

  // retrieve folder properties from package
  if ( fOK )
  {
    ULONG ulI = 0;
    PFILELISTENTRY pFileEntry = FileList.pEntries;

    memset( &FolProp, 0, sizeof(FolProp) );

    for ( ulI = 0; ulI < PackageHeader.ulFileListEntries; ulI++ )
    {
      if ( pFileEntry->usFileType == FOLDER_PROP_FILE )
      {
        GetPackFile( hInput, pFileEntry, (PBYTE)&FolProp, sizeof(FolProp) );
        break;
      } /* endif */
      pFileEntry++;
    } /* endfor */

    printf( "Folder source language : %s\n", FolProp.szSourceLang );
    printf( "Folder target language : %s\n", FolProp.szTargetLang );

    if ( pFolderHeader->BitFlags.fMasterFolder )
    {
      printf( "Folder type            : Master folder\n" );
    }
    else if ( FolProp.fAFCFolder )
    {
      printf( "Folder type            : Child folder\n" );
    }
    else
    {
      printf( "Folder type            : Standard folder\n" );
    } /* endif */
  } /* endif */

  // collect information on documents in exported folder
  if ( fOK )
  {
    int iDocuments = 0;

    fOK = ScanDocumentProps( hInput, &FileList, &iDocuments );

    // show document count and markups used by documents
    if ( fOK )
    {
      ULONG ulI = 0;
      BOOL fFirst = TRUE;
      PSZ pszCurMarkup;

      printf( "Number of documents    : %ld\n", iDocuments );
      if ( !fAllDocsCompleted )
      {
        printf( "All documents completed: No!\n" );
      } /* endif */
      if ( fTotalCountValid )
      {
        printf( "Total word count       : %ld\n", iTotalCount );
      }
      else
      {
        printf( "Total word count       : n/a (not all documents have been analyzed)\n" );
      } /* endif */

      pszCurMarkup = MarkupList.pszStrings;
      for( ulI = 0; ulI < MarkupList.ulEntries; ulI++ )
      {
        if ( fFirst )
        {
          printf( "Markups of documents   : %s\n", pszCurMarkup );
          fFirst = FALSE;
        }
        else
        {
          printf( "                         %s\n", pszCurMarkup );
        } /* endif */
        pszCurMarkup += strlen(pszCurMarkup) + 1;
      } /* endfor */
    } /* endif */
  }

  // show files
  if ( fOK )
  {
    ULONG ulI = 0;
    BOOL fFirst = TRUE;
    ULONG ulNoOfEntries = 0;

    if ( fDetails )
    {
      // attention: for TM folders we have to adjust the usFileType information as there are slight differences in the 
      // file type enumeration between OpenTM2 and TM: The types NTMMEMORY_OLD_MEMORY and NTMMEMORY_OLD_PROP do not exist in OpenTM2
      if ( fTMFolder ) 
      {
        pFileEntry = FileList.pEntries;
        for ( ulI = 0; ulI < PackageHeader.ulFileListEntries; ulI++ )
        {
          if ( pFileEntry->usFileType == HISTLOG_DATA_FILE ) // this is actually NTMMEMORY_OLD_MEMORY
          {
            pFileEntry->usFileType = MEMORY_DATA_FILE;
          }
          else if ( pFileEntry->usFileType == DOCUMENT_EADATA_FILE ) // this is actually NTMMEMORY_OLD_PROP
          {
            pFileEntry->usFileType = MEMORY_PROP_FILE;
          }
          else if ( pFileEntry->usFileType >= TAGTABLE_SETTINGS_FILE ) // this is actually HISTLOG_DATA_FILE or higher
          {
            pFileEntry->usFileType -= 2;
          } /* endif */
          pFileEntry++;
        } /* endfor */
      } /* endif */

      printf( "\nFiles contained in package:\n" );

      fFirst = TRUE;
      pFileEntry = FileList.pEntries;
      for ( ulI = 0; ulI < PackageHeader.ulFileListEntries; ulI++ )
      {
        if ( ( pFileEntry->usFileType == FOLDER_PROP_FILE ) ||
             ( pFileEntry->usFileType == HISTLOG_DATA_FILE ) ||
             ( pFileEntry->usFileType == REDSEGMENT_DATA_FILE ) ||
             ( pFileEntry->usFileType == BINCALCREPORT_FILE ) ||
             ( pFileEntry->usFileType == GLOBALMEMORYFILTER_FILE ) ||
             ( pFileEntry->usFileType == SUBFOLDER_PROP_FILE ) )
        {
          if ( fFirst )
          {
            printf( "\n   Folder files\n" );
            fFirst = FALSE;
          } /* endif */
          ShowFile( pFileEntry );
          pFileEntry->usProcessFlags = 1;
        } /* endif */
        pFileEntry++;
      } /* endfor */

      fFirst = TRUE;
      pFileEntry = FileList.pEntries;
      for ( ulI = 0; ulI < PackageHeader.ulFileListEntries; ulI++ )
      {
        if ( ( pFileEntry->usFileType == DOCUMENT_PROP_FILE ) ||
             ( pFileEntry->usFileType == DOCUMENT_SEGTGT_FILE ) ||
             ( pFileEntry->usFileType == DOCUMENT_SEGSRC_FILE ) ||
             ( pFileEntry->usFileType == DOCUMENT_EADATA_FILE ) ||
             ( pFileEntry->usFileType == DOCUMENT_MTLOG_FILE ) ||
             ( pFileEntry->usFileType == DOCUMENT_XLIFF_FILE ) ||
             ( pFileEntry->usFileType == DOCUMENT_METADATA_FILE ) ||
             ( pFileEntry->usFileType == DOCUMENT_MISC_FILE ) ||
             ( pFileEntry->usFileType == DOCUMENT_ENTITY_FILE ) ||
             ( pFileEntry->usFileType == DOCUMENT_SRC_FILE ) )
        {
          if ( fFirst )
          {
            printf( "\n   Document files\n" );
            fFirst = FALSE;
          } /* endif */
          ShowFile( pFileEntry );
          pFileEntry->usProcessFlags = 1;
        } /* endif */
        pFileEntry++;
      } /* endfor */

      fFirst = TRUE;
      pFileEntry = FileList.pEntries;
      for ( ulI = 0; ulI < PackageHeader.ulFileListEntries; ulI++ )
      {
        if ( ( pFileEntry->usFileType == MEMORY_PROP_FILE ) ||
              ( pFileEntry->usFileType == MEMORY_DATA_FILE ) ||
              ( pFileEntry->usFileType == MEMORY_TABLE_FILE ) ||
              ( pFileEntry->usFileType == MEMORY_INFO_FILE ) ||
              ( pFileEntry->usFileType == PLUGINMEMORY_DATA_FILE ) ||
              ( pFileEntry->usFileType == NTMMEMORY_INDEX_FILE ) ||
              ( pFileEntry->usFileType == NTMMEMORY_NON_UNICODE ) ||
              ( pFileEntry->usFileType == NTMMEMORY_NON_UNICODE_INDEX ) ||
              ( pFileEntry->usFileType == NTMMEMORY_DATA_FILE ) )
        {
          if ( fFirst )
          {
            printf( "\n   Memory files\n" );
            fFirst = FALSE;
          } /* endif */
          ShowFile( pFileEntry );
          pFileEntry->usProcessFlags = 1;
        } /* endif */
        pFileEntry++;
      } /* endfor */

      fFirst = TRUE;
      pFileEntry = FileList.pEntries;
      for ( ulI = 0; ulI < PackageHeader.ulFileListEntries; ulI++ )
      {
        if ( ( pFileEntry->usFileType == DICTIONARY_PROP_FILE ) ||
             ( pFileEntry->usFileType == DICTIONARY_DATA_FILE ) ||
             ( pFileEntry->usFileType == DICTIONARY_NON_UNICODE ) ||
             ( pFileEntry->usFileType == DICTIONARY_NON_UNICODE_INDEX ) ||
             ( pFileEntry->usFileType == DICTIONARY_INDEX_FILE ) )
        {
          if ( fFirst )
          {
            printf( "\n   Dictionary files\n" );
            fFirst = FALSE;
          } /* endif */
          ShowFile( pFileEntry );
          pFileEntry->usProcessFlags = 1;
        } /* endif */
        pFileEntry++;
      } /* endfor */

      fFirst = TRUE;
      pFileEntry = FileList.pEntries;
      for ( ulI = 0; ulI < PackageHeader.ulFileListEntries; ulI++ )
      {
        if ( ( pFileEntry->usFileType == TAGTABLE_DATA_FILE ) ||
             ( pFileEntry->usFileType == TAGTABLE_USEREXIT_FILE ) ||
             ( pFileEntry->usFileType == TAGTABLE_USEREXITWIN_FILE ) ||
             ( pFileEntry->usFileType == TAGTABLE_SETTINGS_FILE ) )
        {
          if ( fFirst )
          {
            printf( "\n   Markup table files\n" );
            fFirst = FALSE;
          } /* endif */
          ShowFile( pFileEntry );
          pFileEntry->usProcessFlags = 1;
        } /* endif */
        pFileEntry++;
      } /* endfor */

      fFirst = TRUE;
      pFileEntry = FileList.pEntries;
      for ( ulI = 0; ulI < PackageHeader.ulFileListEntries; ulI++ )
      {
        if ( pFileEntry->usProcessFlags == 0 )
        {
          if ( fFirst )
          {
            printf( "\n   Files of unknown type\n" );
            fFirst = FALSE;
          } /* endif */
          ShowFile( pFileEntry );
          pFileEntry->usProcessFlags = 1;
        } /* endif */
        pFileEntry++;
      } /* endfor */

    }
    else
    {
      ULONG ulI = 0;
      ULONG ulDate;
      CHAR  szDate[40];

      PSZ pszCurDoc = DocNames.pszStrings;
      PSZ pszCurWords = DocWords.pszStrings;
      PSZ pszCurComplete = DocComplete.pszStrings;
      PSZ pszCurLUDate = DocLUDate.pszStrings;
      PSZ pszCurMarkup = DocMarkups.pszStrings;

      printf("\nDocuments:\n" );
      printf("Name                                      Source Words Compl.  Last Modified        Markup Table\n" );
      for( ulI = 0; ulI < DocNames.ulEntries; ulI++ )
      {
        ulDate = atol(pszCurLUDate);
        if ( ulDate )
        {
          LongToDateTime( ulDate, szDate );
        }
        else
        {
          szDate[0] = EOS;
        } /* endif */

        if ( strlen(pszCurDoc) > 40 )
        {
          printf( "%s\n", pszCurDoc );
          printf( "%-40s  %12s %3s%%    %-20s %s\n", " ", pszCurWords, pszCurComplete, szDate, pszCurMarkup );
        }
        else
        {
          printf( "%-40s  %12s %3s%%    %-20s %s\n", pszCurDoc, pszCurWords, pszCurComplete, szDate, pszCurMarkup );
        } /* endif */

        pszCurDoc      = pszCurDoc + strlen(pszCurDoc) + 1;
        pszCurWords    = pszCurWords + strlen(pszCurWords) + 1;
        pszCurComplete = pszCurComplete + strlen(pszCurComplete) + 1;
        pszCurLUDate   = pszCurLUDate + strlen(pszCurLUDate) + 1;
        pszCurMarkup   = pszCurMarkup + strlen(pszCurMarkup) + 1;
      } /* endfor */


      printf("\nTranslationMemory databases:\n" );
      switch ( pFolderHeader->BitFlags.fHeaderType )
      {
        case RELEASE0_HEADER:
          // get memory file name from memory name variable in header
          printf( pFolderHeader->Head.Rel0.szMemory );
          break;
        case RELEASE1_HEADER:
          // get memory file name from list of packaged files
          {
            ulNoOfEntries = FileList.ulListUsed;
            pFileEntry    = FileList.pEntries;
            while ( ulNoOfEntries )
            {
              if ((pFileEntry->usFileType == MEMORY_DATA_FILE) ||
                  (pFileEntry->usFileType == NTMMEMORY_DATA_FILE) )
              {
                Utlstrccpy( szName, UtlGetFnameFromPath( pFileEntry->pszName ), DOT );
                printf( "   %s\n", szName );
              }
              else if ( pFileEntry->usFileType == MEMORY_INFO_FILE )
              {
                PSZ pszName = NULL;
                // GQ: show memory long name using the info from the .INFO file
                memset( szBuffer, 0, sizeof(szBuffer) );
                GetPackFile( hInput, pFileEntry, (PBYTE)szBuffer, sizeof(szBuffer) );
                pszName = strstr( szBuffer, "Name=" );
                if ( pszName == NULL )
                {
                  // no name info found, show short name instead
                  PSZ pszNamePos = UtlGetFnameFromPath( pFileEntry->pszName );
                  pszNamePos = strchr( pszNamePos, '-' );
                  if ( pszNamePos != NULL )
                  {
                    Utlstrccpy( szName, pszNamePos + 1, DOT );
                    printf( "   %s\n", szName );
                  } /* endif */
                }
                else
                {
                  // find end of name entry
                  PSZ pszEnd = strchr( pszName, '\r' );
                  if ( pszEnd == NULL ) pszEnd = strchr( pszName, '\n' );
                  if ( pszEnd != NULL ) *pszEnd = EOS;
                  strcpy( szName, pszName + 5 );
                  printf( "   %s\n", szName );
                } /* endif */

                //// info file name consists of foldername + ".F00" + "-" + memoryshortname + ".INFO"
              } /* endif */
              ulNoOfEntries--;            // skip to next entry in file list
              pFileEntry++;
            } /* endwhile */
          }
          break;
      } /* endswitch */

      printf( "\nDictionaries:\n" );
      switch ( pFolderHeader->BitFlags.fHeaderType )
      {
        case RELEASE0_HEADER:
          {
            PSZ pszItem;
            // get dictionary names from dictionary table in header
            pszItem = pFolderHeader->Head.Rel0.DicTbl;
            if ( *pszItem )
            {
              while ( pszItem )
              {
                Utlstrccpy( szName, pszItem, X15 );
                printf( "   %s\n", szName );
                pszItem = strchr( pszItem, X15 );
                if ( pszItem ) pszItem++;
              } /* endwhile */
            } /* endif */
          }
          break;
        case RELEASE1_HEADER:
          // get dictionary names from list of packaged files
          {
            ulNoOfEntries = FileList.ulListUsed;
            pFileEntry    = FileList.pEntries;
            while ( fOK && ulNoOfEntries )
            {
              if (pFileEntry->usFileType == DICTIONARY_PROP_FILE)
              {
                PPROPDICTIONARY pProp;

                // GQ: show dictionary long name using the info from the dictionary property file
                memset( szBuffer, 0, sizeof(szBuffer) );
                GetPackFile( hInput, pFileEntry, (PBYTE)szBuffer, sizeof(szBuffer) );
                pProp = (PPROPDICTIONARY)szBuffer;
                if ( pProp->szLongName[0] != EOS )
                {
                  strcpy( szName, pProp->szLongName );
                }
                else
                {
                  Utlstrccpy( szName, UtlGetFnameFromPath( pFileEntry->pszName ), DOT );
                } /* endif */

                printf( "   %s\n", szName );
              } /* endif */
              ulNoOfEntries--;            // skip to next entry in file list
              pFileEntry++;
            } /* endwhile */
          }
          break;
      } /* endswitch */
    } /* endif */
  } /* endif */

  // cleanup
  FreeList( &DocNames );
  FreeList( &DocComplete );
  FreeList( &DocWords );
  FreeList( &MarkupList );
  FreeList( &DocMarkups );
  FreeList( &DocLUDate );

  if ( hInput )        fclose( hInput );
  if ( pFolderHeader ) free( pFolderHeader );
  if ( FileList.pEntries ) free( FileList.pEntries );
  if ( FileList.pBuffer ) free( FileList.pBuffer );

  // return to system
  return( !fOK );
} /* end of main */

USHORT ShowFile( PFILELISTENTRY pEntry )
{
  PSZ pszType;

  switch( pEntry->usFileType )
  {
    case DICTIONARY_DATA_FILE:               pszType = "DICTDATA"; break;
    case DICTIONARY_INDEX_FILE:              pszType = "DICTINDEX"; break;
    case DICTIONARY_PROP_FILE:               pszType = "DICTPROP"; break;
    case DOCUMENT_PROP_FILE:                 pszType = "DOCPROP"; break;
    case DOCUMENT_SEGSRC_FILE:               pszType = "DOCSEGSOURCE"; break;
    case DOCUMENT_SEGTGT_FILE:               pszType = "DOCSEGTARGET"; break;
    case DOCUMENT_SRC_FILE:                  pszType = "DOCSOURCE"; break;
    case FOLDER_PROP_FILE:                   pszType = "FOLPROP"; break;
    case MEMORY_DATA_FILE:                   pszType = "MEMDATA(OLD)"; break;
    case MEMORY_PROP_FILE:                   pszType = "MEMPROP"; break;
    case MEMORY_TABLE_FILE:                  pszType = "MEMTABLE"; break;
    case TAGTABLE_DATA_FILE:                 pszType = "MARKUPTABLE"; break;
    case TAGTABLE_USEREXIT_FILE:             pszType = "MARKUPEXITOS2"; break;
    case NTMMEMORY_DATA_FILE:                pszType = "MEMDATA"; break;
    case NTMMEMORY_INDEX_FILE:               pszType = "MEMINDEX"; break;
    case TAGTABLE_USEREXITWIN_FILE:          pszType = "MARKUPEXITWIN"; break;
    case HISTLOG_DATA_FILE:                  pszType = "FOLHISTLOG"; break;
    case REDSEGMENT_DATA_FILE:               pszType = "REDSEGMENTDATA"; break;
    case BINCALCREPORT_FILE:                 pszType = "BINCALCREPORTFILE"; break;
    case GLOBALMEMORYFILTER_FILE:            pszType = "GLOBALMEMORYFILTER"; break;
    case DOCUMENT_MTLOG_FILE:                pszType = "DOCMTLOG"; break;
    case DOCUMENT_MISC_FILE:                 pszType = "DOCMISC"; break;
    case DOCUMENT_ENTITY_FILE:               pszType = "DOCENTITY"; break;
    case DOCUMENT_XLIFF_FILE:                pszType = "DOCXLIFF"; break;
    case DOCUMENT_METADATA_FILE:             pszType = "DOCMETADATA"; break;
    case TAGTABLE_SETTINGS_FILE:             pszType = "MARKUPSETTINGS"; break;
    case SUBFOLDER_PROP_FILE:                pszType = "SUBFOLPROP"; break;
    case NTMMEMORY_NON_UNICODE:              pszType = "MEMDATA(NONUNICODE)"; break;
    case NTMMEMORY_NON_UNICODE_INDEX:        pszType = "MEMINDEX(NONUNICODE)"; break;
    case DICTIONARY_NON_UNICODE:             pszType = "DICTDATA(NONUNICODE)"; break;
    case DICTIONARY_NON_UNICODE_INDEX:       pszType = "DICTINDEX(NONUNICODE)"; break;
    case PLUGINMEMORY_DATA_FILE:             pszType = "MEMORYDATA(FROMPLUGIN)"; break;
    case TAGTABLE_INFO_FILE:                 pszType = "TAGTABLEINFO"; break;
    case MEMORY_INFO_FILE:                   pszType = "MEMORYINFO"; break;
    default:                                 pszType = "Unknown"; break;
  } /* endswitch */

    printf( "      %-50s %4.4ld/%2.2d/%2.2d %2.2d:%2.2d:%2.2d %14lu %-30s\n", pEntry->pszName,
      pEntry->stFileDate.year + 1980, pEntry->stFileDate.month, pEntry->stFileDate.day,
      pEntry->stFileTime.hours, pEntry->stFileTime.minutes, pEntry->stFileTime.twosecs << 1,
      pEntry->ulFileSize, pszType );

  return( 0 );
}

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
   __int64  i64Pos =  (__int64)pEntry->ulFilePos;

   // position to start of requested file in package file
   _fseeki64( hPackage, i64Pos, SEEK_SET );

   // read file into buffer
   memset( pBuffer, 0, iBufSize );
   iLen = min( iBufSize, (int)pEntry->ulFileSize );
   fread( pBuffer, iLen, 1, hPackage );

   return( TRUE );
} /* end of GetPackFile */

// get number of documents and fill document markup buffer
BOOL ScanDocumentProps
(
  FILE *hPackage,                    // package file
  PFILELIST pFileList,               // list of files in package
  int  *piDocuments                  // receives number of documents
)
{
  BOOL fOK = TRUE;
  int iDocuments = 0;
  ULONG ulI;
  PFILELISTENTRY pFileEntry;          // ptr to an entry in a file list

  InitList( &DocNames );
  InitList( &DocComplete );
  InitList( &DocWords );
  InitList( &MarkupList );
  InitList( &DocLUDate );

  pFileEntry = pFileList->pEntries;
  for ( ulI = 0; ulI < PackageHeader.ulFileListEntries; ulI++ )
  {
    if ( pFileEntry->usFileType == DOCUMENT_PROP_FILE )
    {
      // get document property file
      PPROPDOCUMENT pProp = (PPROPDOCUMENT)bPropBuffer;
      GetPackFile( hPackage, pFileEntry, bPropBuffer, sizeof(bPropBuffer) );

      // add document info to lists
      {
        CHAR szComplete[20];
        CHAR szWords[20];
        CHAR szLUDate[20];

        sprintf( szComplete, "%u", pProp->usComplete );
        sprintf( szLUDate, "%lu", pProp->ulTouched );


        if ( pProp->ulSeg == 0 )
        {
          // document not analyzed yet..
          strcpy( szWords, "n/a" );
          fTotalCountValid = FALSE;
        }
        else
        {
          sprintf( szWords, "%lu", pProp->ulTotal );
        } /* endif */
        iTotalCount += pProp->ulTotal;

        if ( pProp->usComplete != 100 ) fAllDocsCompleted = FALSE;

        fOK = AddToList( &DocNames, (pProp->szLongName[0]) ? pProp->szLongName : pProp->PropHead.szName, FALSE );
        if ( fOK ) fOK = AddToList( &DocComplete, szComplete, FALSE );
        if ( fOK ) fOK = AddToList( &DocWords, szWords, FALSE );
        if ( fOK ) fOK = AddToList( &DocMarkups, (pProp->szFormat[0]) ? pProp->szFormat : FolProp.szFormat, FALSE );
        if ( fOK ) fOK = AddToList( &MarkupList, (pProp->szFormat[0]) ? pProp->szFormat : FolProp.szFormat, TRUE );
        if ( fOK ) fOK = AddToList( &DocLUDate, szLUDate, FALSE );
      }

      // increment number of documents
      iDocuments++;
    } /* endif */

    // next file
    pFileEntry++;
  } /* endfor */

  *piDocuments = iDocuments;

  return( fOK );
}

// functions for string lists
BOOL InitList
(
  PSTRINGLIST  pList
)
{
  memset( pList, 0, sizeof(STRINGLIST) );
  return( TRUE );
}

BOOL FreeList
(
  PSTRINGLIST  pList
)
{
  if ( pList->pszStrings ) free (pList->pszStrings );
  return( TRUE );
}

BOOL AddToList
(
  PSTRINGLIST  pList,
  PSZ          pszNewString,
  BOOL         fCheckForDups
)
{
  BOOL fFound = FALSE;
  BOOL fOK = TRUE;

  if ( fCheckForDups )
  {
    ULONG ulI = 0;
    PSZ pszCurrent = pList->pszStrings;
    while ( !fFound && (ulI < pList->ulEntries) )
    {
      if ( strcmp( pszCurrent, pszNewString ) == 0 )
      {
        fFound = TRUE;
      }
      else
      {
        ulI++;
        pszCurrent += strlen(pszCurrent) + 1;
      } /* endif */
    } /*endwhile */
  } /* endif */

  if ( !fFound )
  {
    ULONG ulLen = (ULONG)(strlen(pszNewString) + 1);
    if ( (pList->ulBufUsed + ulLen) > pList->ulBufSize )
    {
      ULONG ulNewSize = pList->ulBufSize + 32000;
      PSZ pszNew = (PSZ)malloc( ulNewSize );
      if ( pszNew != NULL )
      {
        memcpy( pszNew, pList->pszStrings, pList->ulBufSize );
        free ( pList->pszStrings );
        pList->pszStrings = pszNew;
        pList->ulBufSize = ulNewSize;
      }
      else
      {
        fOK = FALSE;
      } /* endif */
    }
    if ( fOK )
    {
      strcpy( pList->pszStrings + pList->ulBufUsed, pszNewString );
      pList->ulBufUsed += ulLen;
      pList->ulEntries += 1;
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */
  return( fOK );
} /* end of function AddToList */

void showHelp()
{
    printf( "OtmShowFxp.EXE   : Show FXP information\n" );
    printf( "Version          : %s\n",STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright        : %s\n",STR_COPYRIGHT );
    printf( "Purpose          : Show information about a exported folder\n" );
    printf( "Syntax format    : OtmShowFxp folderfile [/DETAILS]\n" );
    printf( "Options and parameters:\n" );
    printf( "    folderfile    the fully qualified name of an exported folder\n" );
    printf( "                  if no file extension is given \".FXP\" is assumed\n" );
    printf( "    /DETAILS      shows the output in greater details\n" );
}
