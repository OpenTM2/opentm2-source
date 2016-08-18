//+----------------------------------------------------------------------------+
//| OTMADL.C                                                                   |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2015, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//+----------------------------------------------------------------------------+
//| Utility to correct the drive letters in property files                     |
//+----------------------------------------------------------------------------+
#undef  DLL
#undef  _WINDLL
#define INCL_EQF_FOLDER                // folder functions
#define INCL_EQF_ANALYSIS              // analysis functions
#define INCL_EQF_DLGUTILS              // dialog utilities (for EQFTMI.H!)
#define INCL_EQF_TM                    // general Transl. Memory functions
#define DLLIMPORTRESMOD              // resource module handle imported from DLL
#include "EQF.H"
#include "EQFSERNO.H"
#include "eqftops.h"
#include "EQFSTART.H"                  // for TwbGetCheckProfileData
#define INCL_EQFMEM_DLGIDAS            // include dialog IDA definitions
#include <EQFTMI.H>                    // Private header file of Translation Memory
#include "eqfsetup.h"                  // directory names

char szFolder[MAX_LONGPATH] = "";      // buffer for folder name
char szMemory[MAX_LONGPATH] = "";      // buffer for memory name
char szDictionary[MAX_LONGPATH] = "";  // buffer for dictionary name
char szPrimaryDrive[10];
char szSecDrives[40];
char szMsgFile[MAX_LONGFILESPEC];
char szSystemPropPath[MAX_LONGFILESPEC];
char szEqfResFile[MAX_LONGFILESPEC];
DDEMSGBUFFER LastMessage;            // buffer for last error message

char szExeStartPath[MAX_LONGPATH] = "";

// total number of processed objects
int iFolders = 0;
int iMemories = 0;
int iDictionaries = 0;

int StartEnvironment();
BOOL AdjustFolder( PSZ pszFolder, BOOL fMayBeLongName );
BOOL AdjustMemory( PSZ pszMemory, BOOL fMayBeLongName  );
BOOL AdjustDictionary( PSZ pszDictionary, BOOL fMayBeLongName  );
BOOL LoadFile( PSZ pszFile, PLONG plLength, PBYTE *ppBuffer, BOOL fMsg );
BOOL SaveFile( PSZ pszFile, LONG lLength, PBYTE pBuffer, BOOL fMsg );
BOOL SearchLongName( PSZ pszName, PSZ pszPattern, int iOffset, PSZ pszShortName );
BOOL ContainsWildCards( PSZ pszName );
static void showHelp();

int main
(
  int  argc,
  char *argv[],
  char *envp[]
)
{
  BOOL fOK = TRUE;                     // O.K. flag

  envp;

  // get fully qualified path of our EXE
  HMODULE hModule = GetModuleHandleW(NULL);
  GetModuleFileName ( hModule, szExeStartPath, sizeof(szExeStartPath) );

  // build system property file name
  UtlSplitFnameFromPath( szExeStartPath );
  UtlSplitFnameFromPath( szExeStartPath );
  strcat( szExeStartPath, "\\PROPERTY\\" );
  strcat( szExeStartPath, SYSTEM_PROPERTIES_NAME );

  // run setup utility when necessary
  if ( GetFileAttributes( szExeStartPath ) == INVALID_FILE_ATTRIBUTES )
  {
    SetupMAT( (HAB)0, szExeStartPath[0], "", ' ' );
  }

   // show help information if needed
  if(argc==2 && stricmp(argv[1],"-h")==0)
  {
      showHelp();
      return 0;
  }

    // Show title line
  printf( "OtmAdl    the TranslationManager drive letter adjust utility\n" );

  StartEnvironment();
  UtlQueryString( QST_ORGEQFDRIVES, szSecDrives, sizeof(szSecDrives) );
  UtlQueryString( QST_PRIMARYDRIVE, szPrimaryDrive, sizeof(szPrimaryDrive)  );

  /* Skip first commandline argument (program name)                   */
  argc--;
  argv++;

  /* Check commandline arguments                                      */
  while ( fOK && argc )
  {
    PSZ pszArg;

    pszArg = *argv;
    if ( (pszArg[0] == '-') || (pszArg[0] == '/') )
    {
      // Check for options
      if ( strnicmp( pszArg + 1, "FOL=", 4 ) == 0 )
      {
        strcpy( szFolder, pszArg + 5 );
      }
      else if ( strnicmp( pszArg + 1, "MEM=", 4 ) == 0 )
      {
        strcpy( szMemory, pszArg + 5 );
      }
      else if ( strnicmp( pszArg + 1, "DIC=", 4 ) == 0 )
      {
        strcpy( szDictionary, pszArg + 5 );
      }
      else
      {
        printf( "Warning: Unknown commandline switch '%s' is ignored.\n",
                pszArg );
      } /* endif */
    }
    else
    {
      printf( "Warning: Superfluous commandline value '%s' is ignored.\n",
                pszArg );
    } /* endif */
    argc--;
    argv++;
  } /* endwhile */


  if ( (szFolder[0] == EOS) && (szMemory[0] == EOS) && (szDictionary[0] == EOS) )
  {
    printf( "Error: Missig input parameters.\n\n" );
    showHelp();
    fOK = FALSE;
  } /* endif */

  if ( fOK && szFolder[0] )
  {
    // check for wildcards within the given name
    if ( ContainsWildCards( szFolder ) )
    {
      WIN32_FIND_DATA FindData;
      HANDLE hDir;
      CHAR  szSearch[256];

      // setup search path name
      UtlMakeEQFPath( szSearch, NULC, PROPERTY_PATH, NULL );
      strcat( szSearch, BACKSLASH_STR );
      Utlstrccpy( szSearch + strlen(szSearch), szFolder, '.' );
      strcat( szSearch, EXT_FOLDER_MAIN );

      // loop over all memory short names using specified string as search mask
      hDir = FindFirstFile( szSearch, &FindData );
      if ( hDir != INVALID_HANDLE_VALUE )
      {
        do
        {
          // if found file is no directory ...
          if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
          {
            // process the folder
            CHAR szShortName[MAX_FILESPEC];
            Utlstrccpy( szShortName, FindData.cFileName, '.' );
            fOK  = AdjustFolder( szShortName, FALSE );
          } /* endif */
        } while ( FindNextFile( hDir, &FindData  ) );
        FindClose( hDir );
      } /* endif */

    }
    else
    {
      fOK  = AdjustFolder( szFolder, TRUE );
    } /* endif */
  } /* endif */

  if ( fOK && szMemory[0] )
  {
    // check for wildcards within the given name
    if ( ContainsWildCards( szMemory ) )
    {
      WIN32_FIND_DATA FindData;
      HANDLE hDir;
      CHAR  szSearch[256];

      // setup search path name
      UtlMakeEQFPath( szSearch, NULC, PROPERTY_PATH, NULL );
      strcat( szSearch, BACKSLASH_STR );
      Utlstrccpy( szSearch + strlen(szSearch), szMemory, '.' );
      strcat( szSearch, EXT_OF_MEM );

      // loop over all folder short names using specified string as search mask
      hDir = FindFirstFile( szSearch, &FindData );
      if ( hDir != INVALID_HANDLE_VALUE )
      {
        do
        {
          // if found file is no directory ...
          if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
          {
            // process the memory
            CHAR szShortName[MAX_FILESPEC];
            Utlstrccpy( szShortName, FindData.cFileName, '.' );
            fOK  = AdjustMemory( szShortName, FALSE );
          } /* endif */
        } while ( FindNextFile( hDir, &FindData  ) );
        FindClose( hDir );
      } /* endif */

    }
    else
    {
      fOK  = AdjustMemory( szMemory, TRUE );
    } /* endif */
  } /* endif */

  if ( fOK && szDictionary[0] )
  {
    // check for wildcards within the given name
    if ( ContainsWildCards( szDictionary ) )
    {
      WIN32_FIND_DATA FindData;
      HANDLE hDir;
      CHAR  szSearch[256];

      // setup search path name
      UtlMakeEQFPath( szSearch, NULC, PROPERTY_PATH, NULL );
      strcat( szSearch, BACKSLASH_STR );
      Utlstrccpy( szSearch + strlen(szSearch), szDictionary, '.' );
      strcat( szSearch, ".PRO" );

      // loop over all dictionary short names using specified string as search mask
      hDir = FindFirstFile( szSearch, &FindData );
      if ( hDir != INVALID_HANDLE_VALUE )
      {
        do
        {
          // if found file is no directory ...
          if ( !(FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
          {
            // process the memory
            CHAR szShortName[MAX_FILESPEC];
            Utlstrccpy( szShortName, FindData.cFileName, '.' );
            fOK  = AdjustDictionary( szShortName, FALSE );
          } /* endif */
        } while ( FindNextFile( hDir, &FindData  ) );
        FindClose( hDir );
      } /* endif */

    }
    else
    {
      fOK  = AdjustDictionary( szDictionary, TRUE );
    } /* endif */
  } /* endif */

  // show summary if something has been changed
  if ( iFolders || iMemories || iDictionaries )
  {
    printf( "Info: Processing complete, %ld folders, %ld memories, and %ld dictionaries have been processed\n",  iFolders, iMemories, iDictionaries );
  }
  else
  {
    printf( "Info: Processing complete, nothing has been processed\n"  );
  } /* endif */

  /* Cleanup                                                          */
  UtlTerminateUtils();
  UtlTerminateError();

  /* return to system                                                 */
  return( !fOK );
} /* end of main */

BOOL AdjustFolder( PSZ pszFolder, BOOL fMayBeLongName  )
{
  BOOL        fOK = TRUE;
  LONG        lLength = 0;
  PBYTE       pbBuffer = NULL;
  PPROPFOLDER pProp = NULL;
  CHAR szShortName[MAX_FILESPEC];
  CHAR szLongName[MAX_LONGPATH];
  CHAR chDrive = szPrimaryDrive[0];

  CHAR szPropPath[MAX_LONGPATH+MAX_LONGPATH];

  // setup property file name
  UtlMakeEQFPath( szPropPath, NULC, PROPERTY_PATH, NULL );
  strcat( szPropPath, BACKSLASH_STR );
  strcat( szPropPath, pszFolder );
  strcat( szPropPath, EXT_FOLDER_MAIN );

  // check if there is a folder with the given name 
  fOK = LoadFile( szPropPath, &lLength, &pbBuffer, FALSE );
  if ( fOK )
  {
    PPROPFOLDER pProp = (PPROPFOLDER)pbBuffer;
    if ( pProp->szLongName[0] != EOS)
    {
      strcpy( szLongName, pProp->szLongName );
    }
    else
    {
      szLongName[0] = EOS;
    } /* endif */
    strcpy( szShortName, pszFolder );
  } /* endif */

  // search using name as folder long name
  if ( !fOK && fMayBeLongName )
  {
    if ( SearchLongName( pszFolder, "*.F00", ((PBYTE)&(pProp->szLongName) - (PBYTE)pProp), szShortName ) )
    {
      UtlMakeEQFPath( szPropPath, NULC, PROPERTY_PATH, NULL );
      strcat( szPropPath, BACKSLASH_STR );
      strcat( szPropPath, szShortName );
      strcat( szPropPath, EXT_FOLDER_MAIN );
      fOK = LoadFile( szPropPath, &lLength, &pbBuffer, FALSE );
      strcpy( szLongName, pszFolder );
    } /* endif */

    if ( !fOK )
    {
      printf( "Error: Folder \'%s\' not found searching in long and short folder names\n", pszFolder );
      printf( "Info: No drive letters have been adjusted\n" );
    } /* endif */
  } /* endif */

  // check if folder directory exists and get drive letter
  if ( fOK )
  {
    PSZ pszDrives = szSecDrives;
    CHAR szFolderDir[MAX_EQF_PATH];
    BOOL fFound = FALSE;

    UtlMakeEQFPath( szFolderDir, NULC, SYSTEM_PATH, NULL );
    strcat( szFolderDir, BACKSLASH_STR );
    strcat( szFolderDir, szShortName );
    strcat( szFolderDir, EXT_FOLDER_MAIN );

    while ( *pszDrives && !fFound )
    {
      szFolderDir[0] = *pszDrives;
      if ( UtlDirExist( szFolderDir ) )
      {
        fFound = TRUE;
        chDrive = szFolderDir[0];
      }
      else
      {
        pszDrives++;
      } /* endif */
    } /* endwhile */

    if ( !fFound )
    {
      printf( "Error: Folder directory \'%s\' not found in primary and secondary drives.\n", szFolderDir + 2 );
      printf( "Info: No drive letters have been adjusted\n" );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // adjust folder properties
  if ( fOK )
  {
    PPROPFOLDER pProp = (PPROPFOLDER)pbBuffer;
    pProp->PropHead.szPath[0] = szPrimaryDrive[0];
    pProp->chDrive = chDrive;

    fOK = SaveFile( szPropPath, lLength, pbBuffer, FALSE );
    if ( fOK )
    {
      printf( "Info: Drive letters of property file of folder %s have been adjusted\n", (szLongName[0] != EOS) ? szLongName : szShortName );
      iFolders++;
    }
    else
    {
      printf( "Error: Save of folder property file \'%s\' failed\n", szPropPath );
      printf( "Info: No drive letters have been adjusted\n" );
      fOK = FALSE;
    } /* endif */
    free( pbBuffer );
    pbBuffer = NULL;
  } /* endif */

  // loop over all document / subfolder properties and adjust drive letter
  if ( fOK )
  {
    BOOL fFinished = FALSE;
    WIN32_FIND_DATA FileData; 
    HANDLE hSearch; 
    char szSearchPath[MAX_LONGPATH]; 
    char szDocPropFile[MAX_LONGPATH]; 
    char szFolder[MAX_FILESPEC];
    int iDocsChanged = 0;
   
    // setup search and file path
    strcpy( szFolder, szShortName );
    strcat( szFolder, EXT_FOLDER_MAIN );
    UtlMakeEQFPath( szSearchPath, chDrive, PROPERTY_PATH, szFolder );
    strcat( szSearchPath, BACKSLASH_STR ); 
    strcat( szSearchPath, "*.*" ); 

    // start searching for doc property files 
    hSearch = FindFirstFile( szSearchPath, &FileData); 
    if (hSearch != INVALID_HANDLE_VALUE) 
    { 
      while ( !fFinished ) 
      { 
        PBYTE pbBuffer = NULL;
        LONG  lLength = 0;

        UtlMakeEQFPath( szDocPropFile, chDrive, PROPERTY_PATH, szFolder );
        strcat( szDocPropFile, BACKSLASH_STR ); 
        strcat( szDocPropFile, FileData.cFileName); 

        if ( (strcmp( FileData.cFileName, "." ) != 0) &&
             (strcmp( FileData.cFileName, ".." ) != 0) &&
             (stricmp( FileData.cFileName, "HISTLOG.DAT" ) != 0) )
        {
          if ( LoadFile( szDocPropFile, &lLength, &pbBuffer, FALSE ) )
          {
            PPROPDOCUMENT pDocProp = (PPROPDOCUMENT)pbBuffer;

            if ( pDocProp->PropHead.szName[0] != EOS )
            {
              pDocProp->PropHead.szPath[0] = szDocPropFile[0];
              SaveFile( szDocPropFile, lLength, pbBuffer, FALSE ),
              iDocsChanged++;
            }
            else
            {
              // a subfolder nothing to do here ...
            } /* endif */

            free( pbBuffer );
            pbBuffer = NULL;
          } /* endif */
        } /* endif */

        if ( !FindNextFile(hSearch, &FileData) ) 
        {
          fFinished = TRUE; 
        } /* endif */
      } /* endwhile */
    } /* endif */

    if ( iDocsChanged )
    {
      printf("Info: Drive letter of %ld documents has been adjusted\n", iDocsChanged );
    }
    else
    {
      printf("Info: No documents found in folder\n" );
    } /* endif */
  } /* endif */

  return( fOK );
}

BOOL AdjustMemory( PSZ pszMemory, BOOL fMayBeLongName  )
{
  BOOL        fOK = TRUE;
  LONG        lLength = 0;
  PBYTE       pbBuffer = NULL;
  CHAR szShortName[MAX_FILESPEC];
  CHAR szLongName[MAX_LONGPATH];
  CHAR chDrive = szPrimaryDrive[0];

  CHAR szPropPath[MAX_LONGPATH+MAX_LONGPATH];

  // setup property file name
  UtlMakeEQFPath( szPropPath, NULC, PROPERTY_PATH, NULL );
  strcat( szPropPath, BACKSLASH_STR );
  strcat( szPropPath, pszMemory );
  strcat( szPropPath, EXT_OF_MEM );

  // check if there is a memory with the given name 
  fOK = LoadFile( szPropPath, &lLength, &pbBuffer, FALSE );
  if ( fOK )
  {
    PPROP_NTM pProp = (PPROP_NTM)pbBuffer;
    if ( pProp->szLongName[0] != EOS)
    {
      strcpy( szLongName, pProp->szLongName );
    }
    else
    {
      szLongName[0] = EOS;
    } /* endif */
    strcpy( szShortName, pszMemory );
  } /* endif */

  // search using name as memory long name
  if ( !fOK && fMayBeLongName )
  {
    PPROP_NTM pProp = NULL;
    if ( SearchLongName( pszMemory, "*.MEM", ((PBYTE)&(pProp->szLongName) - (PBYTE)pProp), szShortName ) )
    {
      UtlMakeEQFPath( szPropPath, NULC, PROPERTY_PATH, NULL );
      strcat( szPropPath, BACKSLASH_STR );
      strcat( szPropPath, szShortName );
      strcat( szPropPath, EXT_OF_MEM );
      fOK = LoadFile( szPropPath, &lLength, &pbBuffer, FALSE );
      strcpy( szLongName, pszMemory );
    } /* endif */

    if ( !fOK )
    {
      printf( "Error: Memory \'%s\' not found searching in long and short memory names\n", pszMemory );
      printf( "Info: No drive letters have been adjusted\n" );
    } /* endif */
  } /* endif */

  // check if memory files exist and get drive letter
  if ( fOK )
  {
    PSZ pszDrives = szSecDrives;
    CHAR szMemFile[MAX_EQF_PATH];
    BOOL fFound = FALSE;
    PPROP_NTM pProp = (PPROP_NTM)pbBuffer;

    while ( *pszDrives && !fFound )
    {
      strcpy( szMemFile, pProp->szFullMemName );
      szMemFile[0] = *pszDrives;
      if ( UtlFileExist( szMemFile ) )
      {
        fFound = TRUE;
        chDrive = szMemFile[0];
      }
      else
      {
        pszDrives++;
      } /* endif */
    } /* endwhile */

    if ( !fFound )
    {
      printf( "Error: Memory data files not found on primary and secondary drives.\n" );
      printf( "Info: No drive letters have been adjusted\n" );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // adjust memory properties
  if ( fOK )
  {
    PPROP_NTM pProp = (PPROP_NTM)pbBuffer;
    pProp->stPropHead.szPath[0] = szPrimaryDrive[0];
    pProp->szFullMemName[0] = chDrive;

    fOK = SaveFile( szPropPath, lLength, pbBuffer, FALSE );
    if ( fOK )
    {
      printf( "Info: Drive letters of property file of memory %s have been adjusted\n", (szLongName[0] != EOS) ? szLongName : szShortName  );
      iMemories++;
    }
    else
    {
      printf( "Error: Save of memory property file \'%s\' failed\n", szPropPath );
      printf( "Info: No drive letters have been adjusted\n" );
      fOK = FALSE;
    } /* endif */
    free( pbBuffer );
    pbBuffer = NULL;
  } /* endif */

  return( fOK );
}

BOOL AdjustDictionary( PSZ pszDictionary, BOOL fMayBeLongName  )
{
  BOOL        fOK = TRUE;
  LONG        lLength = 0;
  PBYTE       pbBuffer = NULL;
  CHAR szShortName[MAX_FILESPEC];
  CHAR szLongName[MAX_LONGPATH];
  CHAR chDrive = szPrimaryDrive[0];

  CHAR szPropPath[MAX_LONGPATH+MAX_LONGPATH];

  // setup property file name
  UtlMakeEQFPath( szPropPath, NULC, PROPERTY_PATH, NULL );
  strcat( szPropPath, BACKSLASH_STR );
  strcat( szPropPath, pszDictionary );
  strcat( szPropPath, ".PRO" );

  // check if there is a dictionary with the given name 
  fOK = LoadFile( szPropPath, &lLength, &pbBuffer, FALSE );
  if ( fOK )
  {
    PPROPDICTIONARY pProp = (PPROPDICTIONARY)pbBuffer;
    if ( pProp->szLongName[0] != EOS)
    {
      strcpy( szLongName, pProp->szLongName );
    }
    else
    {
      szLongName[0] = EOS;
    } /* endif */
    strcpy( szShortName, pszDictionary );
  } /* endif */

  // search using name as dictionary long name
  if ( !fOK && fMayBeLongName )
  {
    PPROPDICTIONARY pProp = NULL;
    if ( SearchLongName( pszDictionary, "*.PRO", ((PBYTE)&(pProp->szLongName) - (PBYTE)pProp), szShortName ) )
    {
      UtlMakeEQFPath( szPropPath, NULC, PROPERTY_PATH, NULL );
      strcat( szPropPath, BACKSLASH_STR );
      strcat( szPropPath, szShortName );
      strcat( szPropPath, EXT_OF_DICTPROP );
      fOK = LoadFile( szPropPath, &lLength, &pbBuffer, FALSE );
      strcpy( szLongName, pszDictionary );
    } /* endif */

    if ( !fOK )
    {
      printf( "Error: Dictionary \'%s\' not found searching in long and short dictionary names\n", pszDictionary );
      printf( "Info: No drive letters have been adjusted\n" );
    } /* endif */
  } /* endif */

  // check if dictionary files exist and get drive letter
  if ( fOK )
  {
    PSZ pszDrives = szSecDrives;
    CHAR szDictFile[MAX_EQF_PATH];
    BOOL fFound = FALSE;
    PPROPDICTIONARY pProp = (PPROPDICTIONARY)pbBuffer;

    while ( *pszDrives && !fFound )
    {
      strcpy( szDictFile, pProp->szDictPath );
      szDictFile[0] = *pszDrives;
      if ( UtlFileExist( szDictFile ) )
      {
        fFound = TRUE;
        chDrive = szDictFile[0];
      }
      else
      {
        pszDrives++;
      } /* endif */
    } /* endwhile */

    if ( !fFound )
    {
      printf( "Error: Dictionary data files not found on primary and secondary drives.\n" );
      printf( "Info: No drive letters have been adjusted\n" );
      fOK = FALSE;
    } /* endif */
  } /* endif */

  // adjust dictionary properties
  if ( fOK )
  {
    PPROPDICTIONARY pProp = (PPROPDICTIONARY)pbBuffer;
    pProp->PropHead.szPath[0] = szPrimaryDrive[0];
    pProp->szDictPath[0] = chDrive;
    pProp->szIndexPath[0] = chDrive;

    fOK = SaveFile( szPropPath, lLength, pbBuffer, FALSE );
    if ( fOK )
    {
      printf( "Info: Drive letters of property file of dictionary %s have been adjusted\n", (szLongName[0] != EOS) ? szLongName : szShortName  );
      iDictionaries++;
    }
    else
    {
      printf( "Error: Save of dictionary property file \'%s\' failed\n", szPropPath );
      printf( "Info: No drive letters have been adjusted\n" );
      fOK = FALSE;
    } /* endif */
    free( pbBuffer );
    pbBuffer = NULL;
  } /* endif */

  return( fOK );
}



int StartEnvironment( )
{
  USHORT usRC = NO_ERROR;

  // check if we have a system property file, run the setup utility if not


  if ( usRC == NO_ERROR )
  {
    UtlSetUShort( QS_RUNMODE, FUNCCALL_RUNMODE );
    UtlInitUtils( NULLHANDLE );
  } /* endif */

  // get profile data and initialize error handler
  if ( usRC == NO_ERROR )
  {
    TwbGetCheckProfileData( szMsgFile, szSystemPropPath, szEqfResFile );

    //  Initialize error handler to allow handling of error messages
    UtlInitError( NULLHANDLE, HWND_FUNCIF, (HWND)&(LastMessage),
                  szMsgFile );
  } /* endif */

  // set directory strings and main drive
  if ( usRC == NO_ERROR )
  {
    PPROPSYSTEM  pPropSys = GetSystemPropPtr();

    UtlSetString( QST_PRIMARYDRIVE, pPropSys->szPrimaryDrive );
    UtlSetString( QST_LANDRIVE,     pPropSys->szLanDrive );
    UtlSetString( QST_PROPDIR,      pPropSys->szPropertyPath );
    UtlSetString( QST_CONTROLDIR,   pPropSys->szCtrlPath );
    UtlSetString( QST_PROGRAMDIR,   pPropSys->szProgramPath );
    UtlSetString( QST_DICDIR,       pPropSys->szDicPath );
    UtlSetString( QST_MEMDIR,       pPropSys->szMemPath );
    UtlSetString( QST_TABLEDIR,     pPropSys->szTablePath );
    UtlSetString( QST_LISTDIR,      pPropSys->szListPath );
    UtlSetString( QST_SOURCEDIR,    pPropSys->szDirSourceDoc );
    UtlSetString( QST_SEGSOURCEDIR, pPropSys->szDirSegSourceDoc );
    UtlSetString( QST_SEGTARGETDIR, pPropSys->szDirSegTargetDoc );
    UtlSetString( QST_TARGETDIR,    pPropSys->szDirTargetDoc );
    UtlSetString( QST_DLLDIR,       pPropSys->szDllPath );
    UtlSetString( QST_MSGDIR,       pPropSys->szMsgPath );
    UtlSetString( QST_EXPORTDIR,    pPropSys->szExportPath );
    UtlSetString( QST_BACKUPDIR,    pPropSys->szBackupPath );
    UtlSetString( QST_IMPORTDIR,    pPropSys->szDirImport );
    UtlSetString( QST_COMMEMDIR,    pPropSys->szDirComMem );
    UtlSetString( QST_COMPROPDIR,   pPropSys->szDirComProp );
    UtlSetString( QST_COMDICTDIR,   pPropSys->szDirComDict );
    UtlSetString( QST_SYSTEMDIR,    pPropSys->PropHead.szPath + 3 );
    UtlSetString( QST_DIRSEGNOMATCHDIR, "SNOMATCH" );
    UtlSetString( QST_DIRSEGMTDIR, "MT" );
    UtlSetString( QST_DIRSEGRTFDIR, "RTF" );
    UtlSetString( QST_PRTPATH,      pPropSys->szPrtPath );
    if ( pPropSys->szWinPath[0] )
    {
      UtlSetString( QST_WINPATH,    pPropSys->szWinPath );
    }
    else
    {
      UtlSetString( QST_WINPATH,      WINDIR );
    } /* endif */
    if ( pPropSys->szEADataPath[0] )
    {
      UtlSetString( QST_EADATAPATH,   pPropSys->szEADataPath );
    }
    else
    {
      UtlSetString( QST_EADATAPATH,   EADATADIR );
    } /* endif */

    UtlSetString( QST_ORGEQFDRIVES, pPropSys->szDriveList );
  } /* endif */
  return( 0 );
}

BOOL LoadFile( PSZ pszFile, PLONG plLength, PBYTE *ppBuffer, BOOL fMsg )
{
  BOOL fOK = TRUE;

  FILE *hInput = fopen( pszFile, "rb" );
  if ( hInput == NULL )
  {
    if ( fMsg ) printf( "Error: Property file \"%s\" could not be opened.\n", pszFile );
    fOK = FALSE;
  }
  else
  {
    *plLength = _filelength( _fileno(hInput) );
    *ppBuffer = (PBYTE)malloc( *plLength ); 
    fread( *ppBuffer, 1, *plLength, hInput );
    fclose( hInput );
  } /* endif */

  return( fOK );
}

BOOL SaveFile( PSZ pszFile, LONG lLength, PBYTE pBuffer, BOOL fMsg )
{
  BOOL fOK = TRUE;

  FILE *hInput = fopen( pszFile, "wb" );
  if ( hInput == NULL )
  {
    if ( fMsg ) printf( "Error: Rewrite of property file \"%s\" failed.\n", pszFile );
    fOK = FALSE;
  }
  else
  {
    fwrite( pBuffer, 1, lLength, hInput );
    fclose( hInput );
  } /* endif */

  return( fOK );
}

// search property files for given long name
BOOL SearchLongName( PSZ pszName, PSZ pszPattern, int iOffset, PSZ pszShortName )
{
  BOOL fFound = FALSE;
  BOOL fFinished = FALSE;
  WIN32_FIND_DATA FileData; 
  HANDLE hSearch; 
  char szSearchPath[MAX_LONGPATH]; 
  char szPropFile[MAX_LONGPATH]; 
 
  // setup search and file path
  UtlMakeEQFPath( szSearchPath, NULC, PROPERTY_PATH, NULL );
  strcat( szSearchPath, BACKSLASH_STR ); 
  strcat( szSearchPath, pszPattern ); 

  // start searchg for property files 
  hSearch = FindFirstFile( szSearchPath, &FileData); 
  if (hSearch != INVALID_HANDLE_VALUE) 
  { 
    while ( !fFinished ) 
    { 
      PBYTE pbBuffer = NULL;
      LONG  lLength = 0;

      UtlMakeEQFPath( szPropFile, NULC, PROPERTY_PATH, NULL );
      strcat( szPropFile, BACKSLASH_STR ); 
      strcat( szPropFile, FileData.cFileName); 

      if ( LoadFile( szPropFile, &lLength, &pbBuffer, FALSE ) )
      {
        PSZ pszLongName = (PSZ)pbBuffer + iOffset;

        if ( stricmp( pszLongName, pszName ) == 0 )
        {
          fFound = TRUE;
          fFinished = TRUE;
          Utlstrccpy( pszShortName, FileData.cFileName, DOT );
        } /* endif */

        free( pbBuffer );
        pbBuffer = NULL;
      } /* endif */

      if ( !FindNextFile(hSearch, &FileData) ) 
      {
        fFinished = TRUE; 
      } /* endif */
    } /* endwhile */
  } /* endif */
  return( fFound ); 
} 
 
static void showHelp()
{
    printf( "OtmAdl.EXE            : OpenTM2 drive letter adjuster\n" );
    printf( "Version               : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright             : %s\n",STR_COPYRIGHT );
    printf( "Purpose               : Adjust driver letter of OpenTM2 property files\n" );
    printf( "Syntax format         : OtmAdl [/mem=memname] [/fol=folname] [/DIC=dicname]\n" );
    printf( "Options and parameters:\n" );
    printf( "    /MEM                the short or long memory name (short names may contain wildcards, e.g.  /MEM=s*)\n" );
    printf( "    /FOL                the short or long folder name (short names may contain wildcards, e.g.  /FOL=*)\n" );
    printf( "    /DIC                the short or long dictionary name (short names may contain wildcarsds, e.g.  /DIC=A*)\n" );
}

BOOL ContainsWildCards( PSZ pszName )
{
  if ( strchr( pszName, '*' ) != NULL ) return( TRUE );
  if ( strchr( pszName, '?' ) != NULL ) return( TRUE );
  return( FALSE );
}
