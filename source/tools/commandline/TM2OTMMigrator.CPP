//+----------------------------------------------------------------------------+
//| TM2OTMMigrator.C                                                           |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2012-2015, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//+----------------------------------------------------------------------------+
//| Tool to migrate TranslationManager data to OpenTM2                         |
//+----------------------------------------------------------------------------+
#undef  DLL
#undef  _WINDLL
#define INCL_EQF_FOLDER                // folder functions
#define INCL_EQF_ANALYSIS              // analysis functions
#define INCL_EQF_TM                    // general Transl. Memory functions
#define INCL_EQF_EDITORAPI             // include editor API stuff
#include "EQF.H"
#include "EQFSERNO.H"

typedef struct _MIGRATIONDATA
{
  BOOL        fMem;                    // TRUE = process memories
  BOOL        fDict;                   // TRUE = process dictionaries
  BOOL        fFolder;                 // TRUE = process folder
  BOOL        fCalcProfiles;           // TRUE = process calculation profiles
  BOOL        fAnalysisProfiles;       // TRUE = process analysis profiles
  BOOL        fSettings;               // TRUE = process settings
  BOOL        fSharedMem;              // TRUE = process shared memories
  char        szSearchDir[MAX_LONGFILESPEC]; // buffer for search directory
  char        szSearchPath[MAX_LONGFILESPEC]; // buffer for search path
  char        szFileName1[MAX_LONGFILESPEC]; // buffer for file names 1
  char        szFileName2[MAX_LONGFILESPEC]; // buffer for file names 2
  char        szTMPath[MAX_LONGFILESPEC]; // TranslationManager system path
  char        szOTMPath[MAX_LONGFILESPEC]; // OpenTM2 system path
  char        szNameBuffer[MAX_LONGFILESPEC]; // buffer for single file names
  char        szLongName[MAX_LONGFILESPEC]; // buffer for long names of memories, folders and dictionaries
  FILE        *hfLog;                  // migration log file
  int         iFolderSuccess;          // number of successfully migrated folders
  int         iFolderFailed;           // number of failed folders
  int         iMemSuccess;             // number of successfully migrated memories
  int         iMemFailed;              // number of failed memories
  int         iSharedMemSuccess;       // number of successfully migrated shared memories
  int         iSharedMemFailed;        // number of failed shared memories
  int         iDictSuccess;            // number of successfully migrated dictionaries
  int         iDictFailed;             // number of failed dictionaries
  int         iCalcSuccess;            // number of successfully migrated calculation profiles
  int         iCalcFailed;             // number of failed calculation profiles
  int         iAnalysisSuccess;        // number of successfully migrated analysis profiles
  int         iAnalysisFailed;         // number of failed analysis profiles
  int         iPropsSuccess;           // number of successfully migrated setting files
  int         iPropsFailed;            // number of failed setting files
  char        chOTMPrimaryDrive;       // primary drive of OpenTM2 
  char        chTMPrimaryDrive;        // primary drive of Tmgr
  char        szSecondaryOTMDrives[26];// list of secondary OpenTM2 drives
  BOOL        fSecondaryDriveAdded;    // TRUE = a drive has been added to the list of OpenTM2 secondary drives
  char        szSecondaryTMDrives[26]; // list of secondary Tmgr drives
  char        szGlobMemFile[MAX_LONGFILESPEC]; // buffer for name of gliobal memory file
  BOOL        fTMDriveSpecified;       // TRUE when the Tmgr drive has been specified
  CHAR        chTMDrive;               // TMGR drive specified by the user
} MIGRATIONDATA, *PMIGRATIONDATA;

#define HISTLOGFILE     "HISTLOG.DAT"
#define REDUNDCOUNTFILE     "REDUND.LOG"

void showHelp();
BOOL checkArguments( int argc, char **argv, PMIGRATIONDATA pData );
BOOL getTMRoot( PMIGRATIONDATA pData );
BOOL getOTMRoot( PMIGRATIONDATA pData );
BOOL getStringFromRegistry( PSZ pszAppl, PSZ pszKey, PSZ pszBuffer, int iBufSize, PSZ pszDefault );
int migrateFolders( PMIGRATIONDATA pData );
int migrateMemories( PMIGRATIONDATA pData );
int migrateSharedMemories( PMIGRATIONDATA pData );
int migrateDicts( PMIGRATIONDATA pData );
int migrateCalcProfiles( PMIGRATIONDATA pData );
int migrateAnalysisProfiles( PMIGRATIONDATA pData );
int migrateSettings( PMIGRATIONDATA pData );
BOOL copyFile( PMIGRATIONDATA pData, char chFromDrive, char chToDrive, char *pszDirectory, char *pszFileName, BOOL fAdjustHeader, BOOL fReplaceExisting );
BOOL doesExistInOtm( PMIGRATIONDATA pData, char *pszDirectory, char *pszFileName );
void copyStringUpToChar( char *pszTarget, char *pszSource, char chTruncateHere);
long loadFile( char * pszFile, void **ppvLoadedFile );
BOOL writeFile( char * pszFile, void *pvLoadedFile, long lLength );
BOOL getOTMSysPropInfo( PMIGRATIONDATA pData );
BOOL prepareDrive( PMIGRATIONDATA pData, char chDrive );
BOOL prepareSharedDrive( PMIGRATIONDATA pData, char chDrive );
BOOL updateSysProps( PMIGRATIONDATA pData );
BOOL doesFileExistInTM( PMIGRATIONDATA pData, char chDrive, char *pszDirectory, char *pszFileName );

MIGRATIONDATA MigrationData;

int main( int argc, char **argv )
{
  int iErrors = 0;
  char *pszLogFile = "TM2OTMMigrator.LOG";

  argc--; argv++;    // skip program name
  if ( argc == 0 )   // no arguments given? 
  {
    showHelp();
    return( 0 ); 
  }

  memset( &MigrationData, 0, sizeof(MigrationData) );

  if ( !checkArguments( argc, argv, &MigrationData ) )
  {
    showHelp();
    return( 0 );
  }

  if ( !getTMRoot( &MigrationData ) )
  {
    printf( "Error: Did not find TranslationManager installation! Program is terminated\n" );
    return( 0 );
  }

  if ( !getOTMRoot( &MigrationData ) )
  {
    printf( "Error: Did not find OpenTM2 installation! Program is terminated\n" );
    return( 0 );
  }

  // get some infos from OpenTM2 system properties
  if ( !getOTMSysPropInfo( &MigrationData ) )
  {
    printf( "Error: Did not find OpenTM2 installation! Program is terminated\n" );
    return( 0 );
  }


  MigrationData.hfLog = fopen( pszLogFile, "w" );
  if ( MigrationData.hfLog == NULL )
  {
    printf( "Error: could not open migration log file %s! Program is terminated\n", pszLogFile );
    return( 0 );
  }
  fprintf( MigrationData.hfLog, "Results of the TranslationManager to OpenTM2 migration\n" );
  fprintf( MigrationData.hfLog, "Tool version: %s\n\n", STR_DRIVER_LEVEL_NUMBER );

  if ( MigrationData.fFolder ) iErrors += migrateFolders( &MigrationData );
  if ( MigrationData.fMem ) iErrors += migrateMemories( &MigrationData );
  if ( MigrationData.fDict ) iErrors += migrateDicts( &MigrationData );
  if ( MigrationData.fCalcProfiles ) iErrors += migrateCalcProfiles( &MigrationData );
  if ( MigrationData.fSettings ) iErrors += migrateSettings( &MigrationData );
  if ( MigrationData.fAnalysisProfiles ) iErrors += migrateAnalysisProfiles( &MigrationData );
  if ( MigrationData.fSharedMem ) iErrors += migrateSharedMemories( &MigrationData );

  // do any update of secondary drive list
  updateSysProps( &MigrationData );

  // show summary
  fprintf( MigrationData.hfLog, "\nSummary\n" );
  fprintf( MigrationData.hfLog, "=======\n" );
  if ( MigrationData.fFolder ) fprintf( MigrationData.hfLog, "%ld folders migrated successfully, %ld folders failed\n", MigrationData.iFolderSuccess, MigrationData.iFolderFailed );
  if ( MigrationData.fMem ) fprintf( MigrationData.hfLog, "%ld Translation Memory databases migrated successfully, %ld Translation Memory databases failed\n", MigrationData.iMemSuccess, MigrationData.iMemFailed );
  if ( MigrationData.fSharedMem ) fprintf( MigrationData.hfLog, "%ld Shared Translation Memory databases migrated successfully, %ld Shared Translation Memory databases failed\n", MigrationData.iSharedMemSuccess, MigrationData.iSharedMemFailed );
  if ( MigrationData.fDict ) fprintf( MigrationData.hfLog, "%ld dictionaries migrated successfully, %ld dictionaries failed\n", MigrationData.iDictSuccess, MigrationData.iDictFailed );
  if ( MigrationData.fCalcProfiles ) fprintf( MigrationData.hfLog, "%ld calculation profiles migrated successfully, %ld calculation profiles failed\n", MigrationData.iCalcSuccess, MigrationData.iCalcFailed );
  if ( MigrationData.fSettings ) fprintf( MigrationData.hfLog, "%ld setting files migrated successfully, %ld setting files failed\n", MigrationData.iPropsSuccess, MigrationData.iPropsFailed );
  if ( MigrationData.fAnalysisProfiles ) fprintf( MigrationData.hfLog, "%ld analysis profiles migrated successfully, %ld calculation profiles failed\n", MigrationData.iAnalysisSuccess, MigrationData.iAnalysisFailed );
  
  fclose( MigrationData.hfLog );

  if ( iErrors != 0 )
  {
    printf( "Errors occured during migration. See migration log file %s for details.\n", pszLogFile );
    return( 0 );
  }
  else
  {
    printf( "Migration completed successfully. See migration log file %s for details.\n", pszLogFile );
    return( 1 );
  }
}

BOOL checkArguments( int argc, char **argv, PMIGRATIONDATA pData )
{
  BOOL fValidCommand = FALSE;

  while ( argc )
  {
    if ( (stricmp( *argv, "/F" ) == 0) || (stricmp( *argv, "/Folder" ) == 0) )
    {
      pData->fFolder = TRUE;
      fValidCommand = TRUE;
    }
    else if ( (stricmp( *argv, "/D" ) == 0) || (stricmp( *argv, "/Dict" ) == 0) )
    {
      pData->fDict = TRUE;
      fValidCommand = TRUE;
    }
    else if ( (stricmp( *argv, "/M" ) == 0) || (stricmp( *argv, "/Mem" ) == 0) )
    {
      pData->fMem = TRUE;
      fValidCommand = TRUE;
    }
    else if ( (stricmp( *argv, "/SH" ) == 0) || (stricmp( *argv, "/SharedMem" ) == 0) )
    {
      pData->fSharedMem = TRUE;
      fValidCommand = TRUE;
    }
    else if ( (stricmp( *argv, "/C" ) == 0) || (stricmp( *argv, "/CalcProfile" ) == 0) )
    {
      pData->fCalcProfiles = TRUE;
      fValidCommand = TRUE;
    }
    else if ( (stricmp( *argv, "/P" ) == 0) || (stricmp( *argv, "/AnalysisProfile" ) == 0) )
    {
      pData->fAnalysisProfiles = TRUE;
      fValidCommand = TRUE;
    }
    else if ( (stricmp( *argv, "/S" ) == 0) || (stricmp( *argv, "/Settings" ) == 0) )
    {
      pData->fSettings = TRUE;
      fValidCommand = TRUE;
    }
    else if ( (strnicmp( *argv, "/TMGR=",6 ) == 0) )
    {
      pData->fTMDriveSpecified = TRUE;
      pData->chTMDrive = (*argv)[6]; 
    }
    else if ( (stricmp( *argv, "/A" ) == 0) || (stricmp( *argv, "/All" ) == 0) )
    {
      pData->fFolder = TRUE;
      pData->fDict = TRUE;
      pData->fMem = TRUE;
      pData->fCalcProfiles = TRUE;
      pData->fAnalysisProfiles = TRUE;
      pData->fSettings = TRUE;
      pData->fSharedMem = TRUE;
      fValidCommand = TRUE;
    }
    else 
    {
      printf( "Warning: Unknown option %s is ignored\n", *argv );
    }

    argc--, argv++;
  }

  return( fValidCommand );
}

void showHelp()
{
    printf( "TM2OTMMigrator.EXE    : TranslationManager to OpenTM2 migration utility\n" );
    printf( "Version               : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright             : %s\n",STR_COPYRIGHT );
    printf( "Purpose               : Migrate TranslationManager folder, memories, dictionaries and profile settings to OpenTM2\n" );
    printf( "Syntax format         : TM2OTMMigrator [/Mem] [/SHaredMem] [/Folder] [/Dict] [/Settings] [/Calcprofiles] [/P|/ANALYSISPROFILE] [/All] [/TMGR=d]\n" );
    printf( "Options and parameters:\n" );
    printf( "    /Mem or /M           migrate TranslationManager translation memory databases\n" );
    printf( "    /SharedMem or /SH    migrate Shared TranslationManager translation memory databases\n" );
    printf( "    /Folder or /F        migrate TranslationManager folder\n" );
    printf( "    /Dict or /D          migrate TranslationManager dictionaries\n" );
    printf( "    /Settings or /S      migrate TranslationManager settings\n" );
    printf( "    /CalcProfile or /C   migrate TranslationManager calculation profiles\n" );
    printf( "    /AnalysisProfile or /P   migrate TranslationManager analysis profiles\n" );
    printf( "    /ALL or /A           migrate all TranslationManager data\n" );
    printf( "    /TMGR=d              specify the drive letter of the TranslationManager installation, d is the TranslationManager installation drive\n" );
}

BOOL getTMRoot( PMIGRATIONDATA pData )
{
  char *pszApplName = "EqfStart";
  char *pszDriveKey = "Drive";
  char *pszPathKey = "Path";

  if ( pData->fTMDriveSpecified )
  {
    sprintf( pData->szTMPath, "%c:\\EQF",  pData->chTMDrive );
  }
  else
  {
    GetProfileString( pszApplName, pszDriveKey, "", pData->szFileName1, sizeof(pData->szFileName1) );
    GetProfileString( pszApplName, pszPathKey, "", pData->szFileName2, sizeof(pData->szFileName2) );
    if ( (pData->szFileName1[0] == '\0') || (pData->szFileName2 == '\0') ) return( FALSE );

    sprintf( pData->szTMPath, "%s\\%s",  pData->szFileName1, pData->szFileName2 );
  }
  return( TRUE );
}

BOOL getOTMRoot( PMIGRATIONDATA pData )
{
  char *pszApplName = "OpenTM2";
  char *pszDriveKey = "Drive";
  char *pszPathKey = "Path";

  getStringFromRegistry( pszApplName, pszDriveKey, pData->szFileName1, sizeof(pData->szFileName1), "" );
  getStringFromRegistry( pszApplName, pszPathKey, pData->szFileName2, sizeof(pData->szFileName2), "" );

  if ( (pData->szFileName1[0] == '\0') || (pData->szFileName2 == '\0') ) return( FALSE );

  sprintf( pData->szOTMPath, "%s\\%s",  pData->szFileName1, pData->szFileName2 );

  return( TRUE );
}

// get a string from the registry
BOOL getStringFromRegistry( PSZ pszAppl, PSZ pszKey, PSZ pszBuffer, int iBufSize, PSZ pszDefault )
{
  BOOL fOK = FALSE;
  HKEY hKey = NULL;

  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "Software", 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
  {
    HKEY hSubKey = NULL;
    if ( RegOpenKeyEx( hKey, pszAppl, 0, KEY_ALL_ACCESS, &hSubKey ) == ERROR_SUCCESS )
    {
      DWORD dwType = REG_SZ;
      DWORD iSize = iBufSize;
      int iSuccess = RegQueryValueEx( hSubKey, pszKey, 0, &dwType, (LPBYTE)pszBuffer, &iSize );
      fOK = (iSuccess == ERROR_SUCCESS);
      RegCloseKey(hSubKey);
    } /* endif */        
    RegCloseKey( hKey );
  } /* endif */     

  if ( !fOK )
  {
    strcpy( pszBuffer, pszDefault );
  } /* endif */     
  return( fOK );
} /* end of function GetStringFromRegistry */

// migrate a single TranslationManager memory
BOOL migrateSingleFolder( PMIGRATIONDATA pData, char *pszFolShortName )
{
  char chFromDrive;
  char chToDrive = '\0';
  char *pszDirectory = "PROPERTY";
  char szName[60];
  BOOL fOK = TRUE;

  pData->szGlobMemFile[0] = '\0';

  // check if a folder with the given short name exists
  sprintf( szName, "%s%s", pszFolShortName, ".F00" );
  if ( doesExistInOtm( pData, pszDirectory, szName ) )
  {
    fprintf( pData->hfLog, "Error: A folder with the short name %s exists in OpenTM2, migration of folder skipped.\n", pszFolShortName );
    return( FALSE );
  }

  // get drive letter and other stuff from property file
  {
    PPROPFOLDER pProp = NULL;
    sprintf( pData->szFileName1, "%s\\%s\\%s", pData->szTMPath, pszDirectory, szName );
    long lLength = loadFile( pData->szFileName1, (void **)&pProp );
    if ( lLength == 0 )
    {
      fprintf( pData->hfLog, "Error: Access to folder properties of memory %s failed, migration of folder skipped.\n", pszFolShortName );
      return( FALSE );
    }
    else
    {
      if ( pProp->szLongName[0] != '\0' )
      {
        strcpy( pData->szLongName, pProp->szLongName );
      }
      else
      {
        strcpy( pData->szLongName, pszFolShortName );
      }
      chToDrive = chFromDrive = pProp->chDrive;
      strcpy( pData->szGlobMemFile, pProp->szGlobalMemOptFile );
      free( pProp );
    }
  }

  // copy property file
  if ( !copyFile( pData, '\0', '\0', pszDirectory, szName, TRUE, FALSE ) )
  {
    fprintf( pData->hfLog, "Error: Migration of folder %s failed, an error occured while copying property file, the error code is %ld.\n", pData->szLongName, GetLastError() );
    return( FALSE );
  }

  // prepare drive it is not the OpenTM2 main drive
  if ( chFromDrive == '\0' ) chFromDrive = pData->szTMPath[0];
  if ( chToDrive == '\0' ) chToDrive = pData->szTMPath[0];  // use Tmgr drive to keep object on same drive
  if ( (chToDrive != pData->szOTMPath[0]) && !prepareDrive( pData, chToDrive ) )
  {
    fprintf( pData->hfLog, "Error: Migration of folder %s failed, could not create directories on secondary drive %c:, the error code is %ld.\n", pData->szLongName, chToDrive, GetLastError() );
    return( FALSE );
  }

  // adjust drive letter in property file
  {
    PPROPFOLDER pProp = NULL;
    pProp->szGlobalMemOptFile;
    sprintf( pData->szFileName1, "%s\\%s\\%s", pData->szOTMPath, pszDirectory, szName );
    long lLength = loadFile( pData->szFileName1, (void **)&pProp );
    if ( lLength == 0 )
    {
      fprintf( pData->hfLog, "Error: Migration of folder %s failed, access to copied property file failed, the error code is %ld.\n", pData->szLongName, GetLastError() );
      return( FALSE );
    }
    else
    {
      pProp->chDrive = chToDrive;
      writeFile( pData->szFileName1, (void *)pProp, lLength );
      free( pProp );
    }
  }

  // create folder directory
  {
    sprintf( pData->szFileName1, "%c%s\\%s", chToDrive, pData->szOTMPath + 1, szName );
    CreateDirectory( pData->szFileName1, NULL );
  }

  // migrate additional folder files
  {
    char szDirectory[60];
    sprintf( szDirectory, "%s\\%s", szName, "PROPERTY" );
    sprintf( pData->szFileName1, "%c%s\\%s", chToDrive, pData->szOTMPath + 1, szDirectory);
    CreateDirectory( pData->szFileName1, NULL );

    // global memory file
    if ( pData->szGlobMemFile[0] != '\0' )
    {
      if ( doesFileExistInTM( pData, chFromDrive, szName, pData->szGlobMemFile ) )
      {
        copyFile( pData, chFromDrive, chToDrive, szName, pData->szGlobMemFile, FALSE, FALSE );
      }
    }
  }

  // loop over all folder directories and process document files (and histlog.dat and redund.log in the property directory)
  {
    WIN32_FIND_DATA FindData;
    HANDLE hFindDir;
  
    sprintf( pData->szSearchPath, "%c%s\\%s\\%s", chFromDrive, pData->szTMPath + 1, szName, "*." );
    pData->szSearchPath[0] = chFromDrive; 

    hFindDir = FindFirstFile( pData->szSearchPath, &FindData);
    if (hFindDir != INVALID_HANDLE_VALUE)
    {
      do
      {
        if ( (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) && (FindData.cFileName[0] != '.') )
        {
          BOOL isProperty = FALSE;
          char szDirectory[60];

          // create directory in OpenTM2 folder
          sprintf( pData->szFileName1, "%c%s\\%s\\%s", chToDrive, pData->szOTMPath + 1, szName, FindData.cFileName );
          CreateDirectory( pData->szFileName1, NULL );
          isProperty = (stricmp( FindData.cFileName, "PROPERTY" ) == 0 );
          sprintf( szDirectory, "%s\\%s", szName, FindData.cFileName );

          // copy all document files
          {
            HANDLE hFindDocs;
            sprintf( pData->szSearchPath, "%c%s\\%s\\%s\\*.*", chFromDrive, pData->szTMPath + 1, szName, FindData.cFileName );
            hFindDocs = FindFirstFile( pData->szSearchPath, &FindData);
            if (hFindDocs != INVALID_HANDLE_VALUE)
            {
              do
              {
                if ( (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0  )
                {
                  if ( !copyFile( pData, chFromDrive, chToDrive, szDirectory, FindData.cFileName, isProperty, FALSE ) )
                  {
                    fprintf( pData->hfLog, "Error: Migration of folder %s failed, error when copying file %s\\%s, migration of folder skipped.\n", pData->szLongName, szDirectory, FindData.cFileName );
                    fOK = FALSE;
                  }
                }
              }
              while ( fOK && FindNextFile( hFindDocs, &FindData ) );
              FindClose(hFindDocs);
            }
          }
        }
      }
      while ( fOK && FindNextFile( hFindDir, &FindData ) );
      FindClose(hFindDir);
    }
  }
  if ( fOK )
  {
    fprintf( pData->hfLog, "       Migration of folder %s completed successfully.\n", pData->szLongName );
  }

  return( fOK );
}

// migrate all existing TranslationManager folders
int migrateFolders( PMIGRATIONDATA pData )
{
  WIN32_FIND_DATA FindData;
  HANDLE hFind;
  int iErrors = 0;
  int iFiles = 0;
  
  fprintf( pData->hfLog, "\nMigrating folders\n" );
  fprintf( pData->hfLog, "=================\n" );

  char *pszSearchDir = "PROPERTY";

  sprintf( pData->szSearchPath, "%s\\%s\\%s", pData->szTMPath, pszSearchDir, "*.F00" );

  hFind = FindFirstFile( pData->szSearchPath, &FindData);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do
    {
      iFiles++;
      copyStringUpToChar( pData->szNameBuffer, FindData.cFileName, '.' );
      if ( !migrateSingleFolder( pData, pData->szNameBuffer ) ) iErrors++;
    }
    while ( FindNextFile( hFind, &FindData ) );
    FindClose(hFind);
    pData->iFolderSuccess = iFiles - iErrors;
    pData->iFolderFailed = iErrors;
  }
  return( iErrors );
}

// migrate a single TranslationManager memory
int migrateSingleMemory( PMIGRATIONDATA pData, char *pszMemShortName )
{
  char chFromDrive;
  char chToDrive = '\0';
  char *pszDirectory = "PROPERTY";
  char szMemName[60];
  BOOL fIsShared = FALSE;

  // check if a memory with the given short name exists
  sprintf( szMemName, "%s%s", pszMemShortName, ".MEM" );
  if ( doesExistInOtm( pData, pszDirectory, szMemName ) )
  {
    fprintf( pData->hfLog, "Error: A Translation Memory with the short name %s exists in OpenTM2, migration of Translation Memory skipped.\n", pszMemShortName );
    return( FALSE );
  }

  // get drive letter from property file
  {
    PPROP_NTM pProp = NULL;
    sprintf( pData->szFileName1, "%s\\%s\\%s", pData->szTMPath, pszDirectory, szMemName );
    long lLength = loadFile( pData->szFileName1, (void **)&pProp );
    if ( lLength == 0 )
    {
      fprintf( pData->hfLog, "Error: Access to Translation Memory properties of memory %s failed, migration of Translation Memory skipped.\n", pszMemShortName );
      return( FALSE );
    }
    else
    {
      if ( pProp->szLongName[0] != '\0' )
      {
        strcpy( pData->szLongName, pProp->szLongName );
      }
      else
      {
        strcpy( pData->szLongName, pszMemShortName );
      }
      chToDrive = chFromDrive = pProp->szFullMemName[0];
      char *pszExt = strrchr( pProp->szFullMemName, DOT );

      fIsShared = ( pszExt && (stricmp( pszExt, EXT_OF_SHARED_MEM ) == 0 ) );

      free( pProp );
    }
  }

  // currently we can't migrate shared memories
  if ( fIsShared )
  {
    return( 2 );
  }

  // copy property file
  if ( !copyFile( pData, '\0', '\0', pszDirectory, szMemName, TRUE, FALSE ) )
  {
    fprintf( pData->hfLog, "Error: Migration of Translation Memory %s failed, an error occured while copying property file, the error code is %ld.\n", pData->szLongName, GetLastError() );
    return( FALSE );
  }

  // prepare drive it is not the OpenTM2 main drive
  if ( chFromDrive == '\0' ) chFromDrive = pData->szTMPath[0];
  if ( chToDrive == '\0' ) chToDrive = pData->szTMPath[0];  // use Tmgr drive to keep object on same drive
  if ( (chToDrive != pData->szOTMPath[0]) && !prepareDrive( pData, chToDrive ) )
  {
    fprintf( pData->hfLog, "Error: Migration of Translation Memory %s failed, could not create directories on secondary drive %c:, the error code is %ld.\n", pData->szLongName, chToDrive, GetLastError() );
    return( FALSE );
  }

  // adjust drive letter in property file
  {
    PPROP_NTM pProp = NULL;
    sprintf( pData->szFileName1, "%s\\%s\\%s", pData->szOTMPath, pszDirectory, szMemName );
    long lLength = loadFile( pData->szFileName1, (void **)&pProp );
    if ( lLength == 0 )
    {
      fprintf( pData->hfLog, "Error: Migration of Translation Memory %s failed, access to copied property file failed, the error code is %ld.\n", pData->szLongName, GetLastError() );
      return( FALSE );
    }
    else
    {
      sprintf( pProp->szFullMemName, "%c%s\\MEM\\%s.TMD", chToDrive, pData->szOTMPath + 1, pszMemShortName );
      writeFile( pData->szFileName1, (void *)pProp, lLength );
      free( pProp );
    }
  }

  // copy data file
  pszDirectory = "MEM";
  sprintf( szMemName, "%s%s", pszMemShortName, ".TMD" );
  if ( !copyFile( pData, chFromDrive, chToDrive, pszDirectory, szMemName, FALSE, FALSE ) )
  {
    fprintf( pData->hfLog, "Error: Migration of Translation Memory %s failed, an error occured while copying the data file, the error code is %ld.\n", pData->szLongName, GetLastError() );
    return( FALSE );
  }

  // copy index file
  sprintf( szMemName, "%s%s", pszMemShortName, ".TMI" );
  if ( !copyFile( pData, chFromDrive, chToDrive, pszDirectory, szMemName, FALSE, FALSE ) )
  {
    fprintf( pData->hfLog, "Error: Migration of Translation Memory %s failed, an error occured while copying the index file, the error code is %ld.\n", pData->szLongName, GetLastError() );
    return( FALSE );
  }
  else
  {
    fprintf( pData->hfLog, "       Migration of Translation Memory %s completed successfully.\n", pData->szLongName );
    return( TRUE );
  }
}

// migrate all existing TranslationManager memories
int migrateMemories( PMIGRATIONDATA pData )
{
  WIN32_FIND_DATA FindData;
  HANDLE hFind;
  int iErrors = 0;
  int iFiles = 0;
  
  fprintf( pData->hfLog, "\nMigrating Translation Memory databases\n" );
  fprintf( pData->hfLog, "======================================\n" );

  char *pszSearchDir = "PROPERTY";

  sprintf( pData->szSearchPath, "%s\\%s\\%s", pData->szTMPath, pszSearchDir, "*.MEM" );

  hFind = FindFirstFile( pData->szSearchPath, &FindData);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do
    {
      iFiles++;
      copyStringUpToChar( pData->szNameBuffer, FindData.cFileName, '.' );
      int iResult = migrateSingleMemory( pData, pData->szNameBuffer );
      if ( iResult == FALSE ) 
        iErrors++;
      else if ( iResult == 2 )
        iFiles--; // ignore this file
    }
    while ( FindNextFile( hFind, &FindData ) );
    FindClose(hFind);
  }
  pData->iMemSuccess = iFiles - iErrors;
  pData->iMemFailed = iErrors;

  return( iErrors );
}

// migrate a single Shared TranslationManager memory
int migrateSingleSharedMemory( PMIGRATIONDATA pData, char *pszMemShortName )
{
  char chFromDrive;
  char chToDrive = '\0';
  char *pszDirectory = "PROPERTY";
  char szMemName[60];
  BOOL fIsShared = FALSE;

  // check if a memory with the given short name exists
  sprintf( szMemName, "%s%s", pszMemShortName, ".SLM" );
  if ( doesExistInOtm( pData, pszDirectory, szMemName ) )
  {
    fprintf( pData->hfLog, "Error: A Shared Translation Memory with the short name %s exists in OpenTM2, migration of Translation Memory skipped.\n", pszMemShortName );
    return( FALSE );
  }

  // get drive letter from property file
  {
    PPROP_NTM pProp = NULL;
    sprintf( szMemName, "%s%s", pszMemShortName, ".MEM" );
    sprintf( pData->szFileName1, "%s\\%s\\%s", pData->szTMPath, pszDirectory, szMemName );
    long lLength = loadFile( pData->szFileName1, (void **)&pProp );
    if ( lLength == 0 )
    {
      fprintf( pData->hfLog, "Error: Access to Translation Memory properties of memory %s failed, migration of Translation Memory skipped.\n", pszMemShortName );
      return( FALSE );
    }
    else
    {
      if ( pProp->szLongName[0] != '\0' )
      {
        strcpy( pData->szLongName, pProp->szLongName );
      }
      else
      {
        strcpy( pData->szLongName, pszMemShortName );
      }
      chToDrive = chFromDrive = pProp->szFullMemName[0];
      char *pszExt = strrchr( pProp->szFullMemName, DOT );

      fIsShared = ( pszExt && (stricmp( pszExt, EXT_OF_SHARED_MEM ) == 0 ) );

      free( pProp );
    }
  }

  // ignore local memories
  if ( !fIsShared )
  {
    return( 2 );
  }

  // copy property file
  if ( !copyFile( pData, '\0', '\0', pszDirectory, szMemName, TRUE, FALSE ) )
  {
    fprintf( pData->hfLog, "Error: Migration of Translation Memory %s failed, an error occured while copying property file, the error code is %ld.\n", pData->szLongName, GetLastError() );
    return( FALSE );
  }

  // prepare drive it is not the OpenTM2 main drive
  if ( chFromDrive == '\0' ) chFromDrive = pData->szTMPath[0];
  if ( chToDrive == '\0' ) chToDrive = pData->szTMPath[0];  // use Tmgr drive to keep object on same drive
  if ( (chToDrive != pData->szOTMPath[0]) && !prepareSharedDrive( pData, chToDrive ) )
  {
    fprintf( pData->hfLog, "Error: Migration of Translation Memory %s failed, could not create directories on secondary drive %c:, the error code is %ld.\n", pData->szLongName, chToDrive, GetLastError() );
    return( FALSE );
  }

  // rename property file, adjust drive letter and file extension in property file
  {
    PPROP_NTM pProp = NULL;
    sprintf( szMemName, "%s%s", pszMemShortName, ".SLM" );
    sprintf( pData->szFileName1, "%s\\%s\\%s", pData->szOTMPath, pszDirectory, szMemName );
    sprintf( pData->szFileName2, "%s\\%s\\%s.MEM", pData->szOTMPath, pszDirectory,  pszMemShortName );
    rename( pData->szFileName2, pData->szFileName1 );
    
    long lLength = loadFile( pData->szFileName1, (void **)&pProp );
    if ( lLength == 0 )
    {
      fprintf( pData->hfLog, "Error: Migration of Shared Translation Memory %s failed, access to copied property file failed, the error code is %ld.\n", pData->szLongName, GetLastError() );
      return( FALSE );
    }
    else
    {
      sprintf( pProp->szFullMemName, "%c%s\\MEM\\%s.RMD", chToDrive, pData->szOTMPath + 1, pszMemShortName );
      sprintf( pProp->stPropHead.szName, "%s.SLM", pszMemShortName );
      writeFile( pData->szFileName1, (void *)pProp, lLength );
      free( pProp );
    }
  }

  // copy remote property file
  pszDirectory = "MEM";
  sprintf( szMemName, "%s%s", pszMemShortName, ".RMP" );
  if ( !copyFile( pData, chFromDrive, chToDrive, pszDirectory, szMemName, FALSE, FALSE ) )
  {
    fprintf( pData->hfLog, "Error: Migration of Shared Translation Memory %s failed, an error occured while copying the remote property file, the error code is %ld.\n", pData->szLongName, GetLastError() );
    return( FALSE );
  }

  // copy data file
  pszDirectory = "MEM";
  sprintf( szMemName, "%s%s", pszMemShortName, ".RMD" );
  if ( !copyFile( pData, chFromDrive, chToDrive, pszDirectory, szMemName, FALSE, FALSE ) )
  {
    fprintf( pData->hfLog, "Error: Migration of Shared Translation Memory %s failed, an error occured while copying the data file, the error code is %ld.\n", pData->szLongName, GetLastError() );
    return( FALSE );
  }

  // copy index file
  sprintf( szMemName, "%s%s", pszMemShortName, ".RMI" );
  if ( !copyFile( pData, chFromDrive, chToDrive, pszDirectory, szMemName, FALSE, FALSE ) )
  {
    fprintf( pData->hfLog, "Error: Migration of Shared Translation Memory %s failed, an error occured while copying the index file, the error code is %ld.\n", pData->szLongName, GetLastError() );
    return( FALSE );
  }
  else
  {
    fprintf( pData->hfLog, "       Migration of Shared Translation Memory %s completed successfully.\n", pData->szLongName );
    return( TRUE );
  }
}

// migrate all existing Shared TranslationManager memories
int migrateSharedMemories( PMIGRATIONDATA pData )
{
  WIN32_FIND_DATA FindData;
  HANDLE hFind;
  int iErrors = 0;
  int iFiles = 0;
  
  fprintf( pData->hfLog, "\nMigrating Shared Translation Memory databases\n" );
  fprintf( pData->hfLog, "===============================================\n" );

  char *pszSearchDir = "PROPERTY";

  sprintf( pData->szSearchPath, "%s\\%s\\%s", pData->szTMPath, pszSearchDir, "*.MEM" );

  hFind = FindFirstFile( pData->szSearchPath, &FindData);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do
    {
      iFiles++;
      copyStringUpToChar( pData->szNameBuffer, FindData.cFileName, '.' );
      int iResult = migrateSingleSharedMemory( pData, pData->szNameBuffer );
      if ( iResult == FALSE ) 
        iErrors++;
      else if ( iResult == 2 )
        iFiles--; // ignore this file
    }
    while ( FindNextFile( hFind, &FindData ) );
    FindClose(hFind);
  }
  pData->iSharedMemSuccess = iFiles - iErrors;
  pData->iSharedMemFailed = iErrors;

  return( iErrors );
}


// migrate a single TranslationManager dictionary
BOOL migrateSingleDict( PMIGRATIONDATA pData, char *pszDictShortName )
{
  char chFromDrive;
  char chToDrive = '\0';
  char *pszDirectory = "PROPERTY";
  char szName[60];
  BOOL fIsShared = FALSE;

  // check if a dictionary with the given short name exists
  sprintf( szName, "%s%s", pszDictShortName, ".PRO" );
  if ( doesExistInOtm( pData, pszDirectory, szName ) )
  {
    fprintf( pData->hfLog, "Error: A dictionary with the short name %s exists in OpenTM2, migration of dictionary skipped.\n", pszDictShortName );
    return( FALSE );
  }

  // get drive letter from property file
  {
    PPROPDICTIONARY pProp = NULL;
    sprintf( pData->szFileName1, "%s\\%s\\%s", pData->szTMPath, pszDirectory, szName );
    long lLength = loadFile( pData->szFileName1, (void **)&pProp );
    if ( lLength == 0 )
    {
      fprintf( pData->hfLog, "Error: Access to properties of dictionary %s failed, migration of dictionary skipped.\n", pszDictShortName );
      return( FALSE );
    }
    else
    {
      if ( pProp->szLongName[0] != '\0' )
      {
        strcpy( pData->szLongName, pProp->szLongName );
      }
      else
      {
        strcpy( pData->szLongName, pszDictShortName );
      }
      chToDrive = chFromDrive = pProp->szDictPath[0];
      char *pszExt = strrchr( pProp->szDictPath, DOT );
      fIsShared = ( pszExt && (stricmp( pszExt, EXT_OF_SHARED_DIC ) == 0 ) );
      free( pProp );
    }
  }

  // copy property file
  if ( !copyFile( pData, '\0', '\0', pszDirectory, szName, TRUE, FALSE ) )
  {
    fprintf( pData->hfLog, "Error: Migration of dictionary %s failed, an error occured while copying property file, the error code is %ld.\n", pData->szLongName, GetLastError() );
    return( FALSE );
  }

  // prepare drive it is not the OpenTM2 main drive
  if ( chFromDrive == '\0' ) chFromDrive = pData->szTMPath[0];
  if ( chToDrive == '\0' ) chToDrive = pData->szTMPath[0];  // use Tmgr drive to keep object on same drive
  if ( !fIsShared && (chToDrive != pData->szOTMPath[0]) && !prepareDrive( pData, chToDrive ) )
  {
    fprintf( pData->hfLog, "Error: Migration of dictionary %s failed, could not create directories on secondary drive %c:, the error code is %ld.\n", pData->szLongName, chToDrive, GetLastError() );
    return( FALSE );
  }

  // for shared dictionaries only: create directories on LAN drive (don't care if they exist already)
  if ( fIsShared )
  {
    sprintf( pData->szFileName1, "%c%s", chToDrive, pData->szOTMPath + 1 );
    CreateDirectory( pData->szFileName1, NULL );
    sprintf( pData->szFileName1, "%c%s\\DICT", chToDrive, pData->szOTMPath + 1 );
    CreateDirectory( pData->szFileName1, NULL );
  }


  // adjust drive letter in property file
  {
    PPROPDICTIONARY pProp = NULL;
    sprintf( pData->szFileName1, "%s\\%s\\%s", pData->szOTMPath, pszDirectory, szName );
    long lLength = loadFile( pData->szFileName1, (void **)&pProp );
    if ( lLength == 0 )
    {
      fprintf( pData->hfLog, "Error: Migration of dictionary %s failed, access to copied property file failed, the error code is %ld.\n", pData->szLongName, GetLastError() );
      return( FALSE );
    }
    else
    {
      if ( fIsShared )
      {
        sprintf( pProp->szDictPath, "%c%s\\DICT\\%s.RAD", chToDrive, pData->szOTMPath + 1, pszDictShortName );
        sprintf( pProp->szIndexPath, "%c%s\\DICT\\%s.RAI",  chToDrive, pData->szOTMPath + 1, pszDictShortName );
      }
      else
      {
        sprintf( pProp->szDictPath, "%c%s\\DICT\\%s.ASD", chToDrive, pData->szOTMPath + 1, pszDictShortName );
        sprintf( pProp->szIndexPath, "%c%s\\DICT\\%s.ASI",  chToDrive, pData->szOTMPath + 1, pszDictShortName );
      }
      writeFile( pData->szFileName1, (void *)pProp, lLength );
      free( pProp );
    }
  }

  // copy data file
  pszDirectory = "DICT";
  sprintf( szName, "%s%s", pszDictShortName, fIsShared ? ".RAD" : ".ASD" );
  if ( !copyFile( pData, chFromDrive, chToDrive, pszDirectory, szName, FALSE, FALSE ) )
  {
    fprintf( pData->hfLog, "Error: Migration of dictionary %s failed, an error occured while copying the data file, the error code is %ld.\n", pData->szLongName, GetLastError() );
    return( FALSE );
  }

  // copy index file
  sprintf( szName, "%s%s", pszDictShortName, fIsShared ? ".RAI" : ".ASI" );
  if ( !copyFile( pData, chFromDrive, chToDrive, pszDirectory, szName, FALSE, FALSE ) )
  {
    fprintf( pData->hfLog, "Error: Migration of dictionary %s failed, an error occured while copying the index file, the error code is %ld.\n", pData->szLongName, GetLastError() );
    return( FALSE );
  }

  // for shared dictionaries only: copy shared property file
  if ( fIsShared )
  {
    sprintf( szName, "%s%s", pszDictShortName, ".RPR" );
    if ( !copyFile( pData, chFromDrive, chToDrive, pszDirectory, szName, TRUE, FALSE ) )
    {
      fprintf( pData->hfLog, "Error: Migration of dictionary %s failed, an error occured while copying the shared property file, the error code is %ld.\n", pData->szLongName, GetLastError() );
      return( FALSE );
   }
  }

  fprintf( pData->hfLog, "       Migration of dictionary %s completed successfully.\n", pData->szLongName );
  return( TRUE );
}


// migrate all existing TranslationManager dictionaries
int migrateDicts( PMIGRATIONDATA pData )
{
  WIN32_FIND_DATA FindData;
  HANDLE hFind;
  int iErrors = 0;
  int iFiles = 0;
  
  fprintf( pData->hfLog, "\nMigrating dictionaries\n" );
  fprintf( pData->hfLog, "======================\n" );

  char *pszSearchDir = "PROPERTY";

  sprintf( pData->szSearchPath, "%s\\%s\\%s", pData->szTMPath, pszSearchDir, "*.PRO" );

  hFind = FindFirstFile( pData->szSearchPath, &FindData);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do
    {
      iFiles++;
      copyStringUpToChar( pData->szNameBuffer, FindData.cFileName, '.' );
      if ( !migrateSingleDict( pData, pData->szNameBuffer ) ) iErrors++;
    }
    while ( FindNextFile( hFind, &FindData ) );
    FindClose(hFind);
  }
  pData->iDictSuccess = iFiles - iErrors;
  pData->iDictFailed = iErrors;
  return( iErrors );
}

// migrate all existing TranslationManager calculation profiles
int migrateCalcProfiles( PMIGRATIONDATA pData )
{
  WIN32_FIND_DATA FindData;
  HANDLE hFind;
  int iErrors = 0;
  int iFiles = 0;
  
  fprintf( pData->hfLog, "\nMigrating calculation profiles\n" );
  fprintf( pData->hfLog, "==============================\n" );

  char *pszSearchDir = "PROPERTY";

  sprintf( pData->szSearchPath, "%s\\%s\\%s", pData->szTMPath, pszSearchDir, "*.R00" );

  hFind = FindFirstFile( pData->szSearchPath, &FindData);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do
    {
      iFiles++;
      if ( doesExistInOtm( pData, pszSearchDir, FindData.cFileName ) )
      {
        copyStringUpToChar( pData->szFileName1, FindData.cFileName, '.' );
        fprintf( pData->hfLog, "Error: Calculation profile %s exists in OpenTM2, migration of TM profile skipped.\n", pData->szFileName1 );
        iErrors++;
      }
      else
      {
        if ( copyFile( pData, '\0', '\0', pszSearchDir, FindData.cFileName, TRUE, FALSE ) )
        {
          copyStringUpToChar( pData->szFileName1, FindData.cFileName, '.' );
          fprintf( pData->hfLog, "       Migration of calculation profile %s successfully.\n", pData->szFileName1 );
        }
        else
        {
          copyStringUpToChar( pData->szFileName1, FindData.cFileName, '.' );
          fprintf( pData->hfLog, "Error: Migration of calculation profile %s failed, the error code is %ld.\n", pData->szFileName1, GetLastError() );
          iErrors++;
        }
      }
    }
    while ( FindNextFile( hFind, &FindData ) );
    FindClose(hFind);
  }
  pData->iCalcSuccess = iFiles - iErrors;
  pData->iCalcFailed = iErrors;

  return( iErrors );
}


// migrate all existing TranslationManager analysis profiles
int migrateAnalysisProfiles( PMIGRATIONDATA pData )
{
  WIN32_FIND_DATA FindData;
  HANDLE hFind;
  int iErrors = 0;
  int iFiles = 0;
  
  fprintf( pData->hfLog, "\nMigrating analysis profiles\n" );
  fprintf( pData->hfLog, "==============================\n" );

  char *pszSearchDir = "PROPERTY";

  sprintf( pData->szSearchPath, "%s\\%s\\%s", pData->szTMPath, pszSearchDir, "*.APR" );

  hFind = FindFirstFile( pData->szSearchPath, &FindData);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do
    {
      iFiles++;
      if ( doesExistInOtm( pData, pszSearchDir, FindData.cFileName ) )
      {
        copyStringUpToChar( pData->szFileName1, FindData.cFileName, '.' );
        fprintf( pData->hfLog, "Error: Analysis profile %s exists in OpenTM2, migration of TM analysis profile skipped.\n", pData->szFileName1 );
        iErrors++;
      }
      else
      {
        if ( copyFile( pData, '\0', '\0', pszSearchDir, FindData.cFileName, FALSE, FALSE ) )
        {
          copyStringUpToChar( pData->szFileName1, FindData.cFileName, '.' );
          fprintf( pData->hfLog, "       Migration of analysis profile %s successfully.\n", pData->szFileName1 );
        }
        else
        {
          copyStringUpToChar( pData->szFileName1, FindData.cFileName, '.' );
          fprintf( pData->hfLog, "Error: Migration of analysis profile %s failed, the error code is %ld.\n", pData->szFileName1, GetLastError() );
          iErrors++;
        }
      }
    }
    while ( FindNextFile( hFind, &FindData ) );
    FindClose(hFind);
  }
  pData->iAnalysisSuccess = iFiles - iErrors;
  pData->iAnalysisFailed = iErrors;

  return( iErrors );
}


// migrate a single TranslationManager setting file
BOOL migrateSettingsFile( PMIGRATIONDATA pData, char *pszDir, char *pszFile )
{
  if ( copyFile( pData, '\0', '\0', pszDir, pszFile, TRUE, TRUE ) )
  {
    fprintf( pData->hfLog, "       Settings file %s migrated successfully.\n", pszFile );
    pData->iPropsSuccess++;
    return( TRUE );
  }
  else
  {
    fprintf( pData->hfLog, "Error: Migration of settings file %s failed, the error code is %ld.\n", pszFile, GetLastError() );
    pData->iPropsFailed++;
    return( FALSE );
  }
}

// Structure of TranslationManager system properties
typedef struct _PROPSYSTEM_TMGR
{
   //-------------------------- property header --------------------------------
   PROPHEAD  PropHead;                      // common property header
   //------------------ drive and path information -----------------------------
   CHAR szPrimaryDrive[MAX_DRIVE];          // primary TWB drive( e.g. "X:\0")
   CHAR szDriveList[MAX_DRIVELIST];         // list of EQF drives
   CHAR szPropertyPath[MAX_FILESPEC];       // directory for properties
   CHAR szProgramPath[MAX_FILESPEC];        // directory for program files
   CHAR szDicPath[MAX_FILESPEC];            // directory for dictionaries
   CHAR szMemPath[MAX_FILESPEC];            // directory for memory data bases
   CHAR szTablePath[MAX_FILESPEC];          // directory for translation tables
   CHAR szListPath[MAX_FILESPEC];           // directory for lists
   CHAR szExportPath[MAX_FILESPEC];         // directory for exported files
   CHAR szBackupPath[MAX_FILESPEC];         // directory for backup files
   CHAR szDirSourceDoc[MAX_FILESPEC];       // directory for source documents
   CHAR szDirSegSourceDoc[MAX_FILESPEC];    // directory for segm. source docs
   CHAR szDirSegTargetDoc[MAX_FILESPEC];    // directory for segm. target docs
   CHAR szDirTargetDoc[MAX_FILESPEC];       // directory for target docs
   CHAR szDirDocWordLists[MAX_FILESPEC];    // directory for document lists
   CHAR szDllPath[MAX_FILESPEC];            // directory for DLLs
   CHAR szMsgPath[MAX_FILESPEC];            // directory for msg and help files
   CHAR szCtrlPath[MAX_FILESPEC];           // directory for control information
   //----------------------- restart information -------------------------------
   CHAR RestartFolderLists[ 128];           // list of folderlists for restart
   CHAR RestartFolders[ 256];               // list of folder for restart
   EQF_SWP  SwpDef;                         // TWB default window size
   EQF_SWP  Swp;                            // TWB window position
   CHAR RestartMemory[ 256];                // list of mem windows for restart
   CHAR RestartDicts[ 256];                 // list for dict windows for restart
   OBJNAME FocusObject;                     // object which had the focus
   CHAR RestartTagTables[ 128];             // list of tag tables for restart
   CHAR szDefaultEditor[MAX_FILESPEC];      // default editor
   //---------------------- TM server code values ------------------------------
   CHAR szServerList[MAX_SERVER_LIST_SIZE]; // list of servers
   //------------------ drive and path information (2) -------------------------
   CHAR szDirComMem[MAX_FILESPEC];          // directory for server TMs
   CHAR szDirComProp[MAX_FILESPEC];         // directory for server TM props
   CHAR szDirImport[MAX_FILESPEC];          // directory for imported files
   //----------------------- restart information (2) ---------------------------
   CHAR RestartLists[ 256];                 // list of list handlers for restart
   CHAR RestartDocs[ 1000];                 // list of documents for restart
   //----------------------- new paths and directories -------------------------
   CHAR szPrtPath[MAX_FILESPEC];            // directory for print formats
   CHAR szDirComDict[MAX_FILESPEC];         // directory for server Dicts
   CHAR szWinPath[MAX_FILESPEC];            // directory for Windows DLLs
   //------------------------- general options ---------------------------------
   EQF_BOOL  fFolImpNoDocExistsQuery;       // FolImp no query for existing docs
   EQF_BOOL  fReserveOption01;              // reserve option
   EQF_BOOL  fReserveOption02;              // reserve option
   EQF_BOOL  fReserveOption03;              // reserve option
   EQF_BOOL  fReserveOption04;              // reserve option
   EQF_BOOL  fReserveOption05;              // reserve option
   EQF_BOOL  fReserveOption06;              // reserve option
   EQF_BOOL  fReserveOption07;              // reserve option
   EQF_BOOL  fReserveOption08;              // reserve option
   EQF_BOOL  fReserveOption09;              // reserve option
   EQF_BOOL  fReserveOption10;              // reserve option
   EQF_BOOL  fReserveOption11;              // reserve option
   EQF_BOOL  fReserveOption12;              // reserve option
   EQF_BOOL  fReserveOption13;              // reserve option
   EQF_BOOL  fReserveOption14;              // reserve option
   EQF_BOOL  fReserveOption15;              // reserve option
   EQF_BOOL  fReserveOption16;              // reserve option
   EQF_BOOL  fReserveOption17;              // reserve option
   EQF_BOOL  fReserveOption18;              // reserve option
   EQF_BOOL  fReserveOption19;              // reserve option
   //---------------------------------------------------------------------------
   CHAR szEADataPath[MAX_FILESPEC];         // directory for EA data
   //----------------------------- LAN drive letter ----------------------------
   CHAR szLanDrive[MAX_DRIVE];    // primary TWB LAN drive( e.g. "X:\0")
   //-------------------- values in system property dialog ---------------------
   EQF_BOOL  fNoShutDownConf;               // no shutdown confirmation flag
   EQF_BOOL  fNoDeleteConf;                 // no delete confirmation flag
   EQF_BOOL  fSaveAlways;                   // always save workbench during shutdown
   EQF_BOOL  fUseSymbolicMarkupNames;       // use symbolic markup names flag
   EQF_BOOL  fNoGenericMarkup;              // suppress generic markup handling
   CHAR      szWebBrowser[MAX_LONGPATH];    // buffer for web browser name/path

   //---------------------------- TQM stuff --------------------------------------
   CHAR      szTQMProjectPath[MAX_EQF_PATH];// name of TQM project directory
   CHAR      szTQMEvalPath[MAX_EQF_PATH];   // name of TQM evalutation profile directory
   CHAR      szTQMArchivePath[MAX_EQF_PATH];// name of TQM archived projects directory
   CHAR      szTQMReportPath[MAX_EQF_PATH]; // name of TQM report directory
   CHAR      szTQMVendorPath[MAX_EQF_PATH]; // name of TQM vendor directory

   //-------------------- more values in system property dialog ---------------------
   EQF_BOOL  fUseIELikeListWindows;         // use IE like folder list

   // currently active fuzziness levels with two desimal places
   // (e.g. 50 is stored as 5000)
   LONG         lSmallFuzzLevel;       // fuzzines level for small segments
   LONG         lMediumFuzzLevel;      // fuzzines level for medium sized segments
   LONG         lLargeFuzzLevel;       // fuzzines level for large segments

   // internal code version number, used to special preprocessing if a new
   // code version is about to be started for the first time (e.g. delete
   // old stem form cache files if format haas been changed)
   LONG         lCodeVersion;          // current EQFD/EQFDLL code version
   CHAR         szSystemPrefLang[MAX_LANG_LENGTH]; // System Preferences LANGUAGE
   ULONG        ulSystemPrefCP;

   CHAR         RestartMTList[80];    // list of MT job lists to restart (only one!)
   LONG         lSmallLkupFuzzLevel;       // fuzzines level for small segments(1LONG=3CHAR)
   LONG         lMediumLkupFuzzLevel;      // fuzzines level for medium sized segments
   LONG         lLargeLkupFuzzLevel;       // fuzzines level for large segments

   SHORT  		sCalcReportWinX;
   SHORT        sCalcReportWinY;
   SHORT        sCalcReportWinCX;
   SHORT        sCalcReportWinCY;
   SHORT        sLeftPaneWidth;           // P021036

   EQF_BOOL     fNoSgmlDitaProcessing;// true = disable preprocessing of IDDOC proposals for DITA documents
   SHORT        sXSLTEngine;          // XSLT engine to be used
   EQF_BOOL     fEntityProcessing;    // true = perform special processing for IDDOC and DITA entities

   //----------------------------- filler --------------------------------------
   CHAR         chReserve[257];       //  reserve space/ filles props to 4K)
} PROPSYSTEM_TMGR, *PPROPSYSTEM_TMGR;


// migrate some values of the system properties file
BOOL migrateSysProps( PMIGRATIONDATA pData )
{
  PPROPSYSTEM_TMGR pSysPropTmgr = NULL;
  long lTmgrPropLen = 0;
  PPROPSYSTEM pSysProp = NULL;
  long lOtmPropLen = 0;


  // load Tmgr system properties
  sprintf( pData->szFileName1, "%s\\PROPERTY\\EQFSYSW.PRP", pData->szTMPath );
  lTmgrPropLen = loadFile( pData->szFileName1, (void **)&pSysPropTmgr );
  if ( lTmgrPropLen == 0 )
  {
    fprintf( pData->hfLog, "Error: Migration of system properties failed, could not load TranslationManager property file, the error code is %ld.\n", GetLastError() );
    pData->iPropsFailed++;
    return( FALSE );
  }

  // load OpenTM2 system properties
  sprintf( pData->szFileName2, "%s\\PROPERTY\\EQFSYSW.PRP", pData->szOTMPath );
  lOtmPropLen = loadFile( pData->szFileName2, (void **)&pSysProp );
  if ( lOtmPropLen == 0 )
  {
    fprintf( pData->hfLog, "Error: Migration of system properties failed, could not load OpenTM2 property file, the error code is %ld.\n", GetLastError() );
    free( pSysPropTmgr );
    pData->iPropsFailed++;
    return( FALSE );
  }

  // copy selected values
  memcpy( &(pSysProp->SwpDef), &(pSysPropTmgr->SwpDef), sizeof(pSysProp->SwpDef) );
  memcpy( &(pSysProp->Swp), &(pSysPropTmgr->Swp), sizeof(pSysProp->Swp) );
  strcpy( pSysProp->szDefaultEditor, pSysPropTmgr->szDefaultEditor );
  pSysProp->fFolImpNoDocExistsQuery = pSysPropTmgr->fFolImpNoDocExistsQuery;
  pSysProp->fNoShutDownConf = pSysPropTmgr->fNoShutDownConf;
  pSysProp->fNoDeleteConf = pSysPropTmgr->fNoDeleteConf;
  pSysProp->fSaveAlways = pSysPropTmgr->fSaveAlways;
  pSysProp->fNoGenericMarkup = pSysPropTmgr->fNoGenericMarkup;
  strcpy( pSysProp->szWebBrowser, pSysPropTmgr->szWebBrowser );
  pSysProp->fUseIELikeListWindows = pSysPropTmgr->fUseIELikeListWindows;
  pSysProp->lSmallFuzzLevel = pSysPropTmgr->lSmallFuzzLevel;
  pSysProp->lMediumFuzzLevel = pSysPropTmgr->lMediumFuzzLevel;
  pSysProp->lLargeFuzzLevel = pSysPropTmgr->lLargeFuzzLevel;
  strcpy( pSysProp->szSystemPrefLang, pSysPropTmgr->szSystemPrefLang );
  pSysProp->ulSystemPrefCP = pSysPropTmgr->ulSystemPrefCP;
  pSysProp->lSmallLkupFuzzLevel = pSysPropTmgr->lSmallLkupFuzzLevel;
  pSysProp->lMediumLkupFuzzLevel = pSysPropTmgr->lMediumLkupFuzzLevel;
  pSysProp->lLargeLkupFuzzLevel = pSysPropTmgr->lLargeLkupFuzzLevel;
  pSysProp->sCalcReportWinX = pSysPropTmgr->sCalcReportWinX;
  pSysProp->sCalcReportWinY = pSysPropTmgr->sCalcReportWinY;
  pSysProp->sCalcReportWinCX = pSysPropTmgr->sCalcReportWinCX;
  pSysProp->sCalcReportWinCY = pSysPropTmgr->sCalcReportWinCY;
  pSysProp->sLeftPaneWidth = pSysPropTmgr->sLeftPaneWidth;
  pSysProp->fNoSgmlDitaProcessing = pSysPropTmgr->fNoSgmlDitaProcessing;
  pSysProp->fEntityProcessing = pSysPropTmgr->fEntityProcessing;

  // save OpenTM2 system properties
  if ( writeFile( pData->szFileName2, (void *)pSysProp, lOtmPropLen ) )
  {
    fprintf( pData->hfLog, "       Migration of system properties EQFSYSW.PRP completed successfully.\n" );
    free( pSysProp );
    free( pSysPropTmgr );
    pData->iPropsSuccess++;
    return( TRUE );
  }
  else
  {
    fprintf( pData->hfLog, "Error: Migration of system properties EQFSYSW.PRP failed, could not update OpenTM2 property file, the error code is %ld.\n", GetLastError() );
    free( pSysProp );
    free( pSysPropTmgr );
    pData->iPropsFailed++;
    return( FALSE );
  }
}


// migrate all TranslationManager setting files
int migrateSettings( PMIGRATIONDATA pData)
{
  int iErrors = 0;

  fprintf( pData->hfLog, "\nMigrating setting files\n" );
  fprintf( pData->hfLog, "=======================\n" );

  if ( !migrateSettingsFile( pData, "PROPERTY", "STANDARD.EDI" ) ) iErrors++;
  if ( !migrateSettingsFile( pData, "PROPERTY", "RTFEDIT.EDI" ) ) iErrors++;
  if ( !migrateSettingsFile( pData, "PROPERTY", "XLATE.EDI" ) ) 
  {
    iErrors++;
  }
  else
  {
    // adjust DLL name in XLATE.EDI
    PPROPEDIT pProp = NULL;
    sprintf( pData->szFileName1, "%s\\%s\\%s", pData->szOTMPath, "PROPERTY", "XLATE.EDI" );
    long lLength = loadFile( pData->szFileName1, (void **)&pProp );
    if ( lLength != 0 )
    {
      strcpy( pProp->szMFEStartProc, "XLATEOTM" );
      writeFile( pData->szFileName1, (void *)pProp, lLength );
      free( pProp );
    }
  }

  if ( !migrateSettingsFile( pData, "PROPERTY", "EQFPROW.PRP" ) ) iErrors++;
  if ( !migrateSettingsFile( pData, "PROPERTY", "FLIST.L00" ) ) iErrors++;

  if ( !migrateSettingsFile( pData, "PROPERTY", "EQFDICT.PRP" ) ) iErrors++;
  if ( !migrateSettingsFile( pData, "PROPERTY", "EQFIMEX.PRP" ) ) iErrors++;
  if ( !migrateSettingsFile( pData, "PROPERTY", "EQFMEMLU.PRP" ) ) iErrors++;
  if ( !migrateSettingsFile( pData, "PROPERTY", "EQFMEMN.PRP" ) ) iErrors++;

  if ( !migrateSysProps( pData ) ) iErrors++;

  return( iErrors );
}

// copy file to OpenTM2 and adjust header info when requested
BOOL copyFile( PMIGRATIONDATA pData, char chFromDrive, char chToDrive, char *pszDirectory, char *pszFileName, BOOL fAdjustHeader, BOOL fReplaceExisting )
{
  BOOL fOK = TRUE;

  sprintf( pData->szFileName1, "%s\\%s\\%s", pData->szTMPath, pszDirectory, pszFileName );
  if ( chFromDrive != '\0' ) pData->szFileName1[0] = chFromDrive;
  sprintf( pData->szFileName2, "%s\\%s\\%s", pData->szOTMPath, pszDirectory, pszFileName );
  if ( chFromDrive != '\0' ) pData->szFileName2[0] = chFromDrive;

  fOK = CopyFile( pData->szFileName1, pData->szFileName2, !fReplaceExisting );

  // clear any read-only attribute
  if ( fOK )
  {
    DWORD dwAttrs = GetFileAttributes( pData->szFileName1 ); 
    if ( (dwAttrs != INVALID_FILE_ATTRIBUTES) && (dwAttrs & FILE_ATTRIBUTE_READONLY) ) 
    { 
      dwAttrs &= ~FILE_ATTRIBUTE_READONLY;
      fOK = SetFileAttributes( pData->szFileName1, dwAttrs ); 
      if ( !fOK )
      {
        int iRC = GetLastError();
        iRC += 1;
      }
    }
  }

  if ( fOK && fAdjustHeader )
  {
    FILE *hf = fopen( pData->szFileName2, "r+b" );
    if ( hf != NULL )
    {
      PROPHEAD  PropHead; 
      fread( &PropHead, sizeof(PropHead), 1, hf );
      memcpy( PropHead.szPath, pData->szOTMPath, strlen(pData->szOTMPath) );
      if ( chToDrive != '\0' ) PropHead.szPath[0] = chToDrive;
      fseek( hf, 0, SEEK_SET );
      fwrite( &PropHead, sizeof(PropHead), 1, hf );
      fclose( hf );
    }
    else
    {
      fOK = FALSE;
    }
  }

  return( fOK );
}

// check if the given file exists in OpenTM2
BOOL doesExistInOtm( PMIGRATIONDATA pData, char *pszDirectory, char *pszFileName )
{
  BOOL fFound = FALSE;
  WIN32_FIND_DATA ResultBuf;               
  HANDLE hDirHandle;

  sprintf( pData->szFileName1, "%s\\%s\\%s", pData->szOTMPath, pszDirectory, pszFileName );
  hDirHandle = FindFirstFile( pData->szFileName1, &ResultBuf );
  if ( hDirHandle == INVALID_HANDLE_VALUE )
  {
    fFound = FALSE;
  }
  else
  {
    fFound = TRUE;
    FindClose( hDirHandle );
  } /* endif */

  return( fFound );
}

// truncate string at given character
void copyStringUpToChar( char *pszTarget, char *pszSource, char chTruncateHere )
{
  while( *pszSource && (*pszSource != chTruncateHere) ) *pszTarget++ = *pszSource++;
  *pszTarget = '\0';
}

// load a file
long loadFile( char * pszFile, void **ppvLoadedFile ) 
{
    long lLength = 0;

    FILE *hf = NULL;
    
    hf = fopen( pszFile, "rb" );
    if ( hf != NULL )
    {
      lLength = _filelength( _fileno( hf ) );
      *ppvLoadedFile = malloc( lLength );
      if ( *ppvLoadedFile != NULL )
      {
        fread( *ppvLoadedFile, 1, lLength, hf ); 
      }
      else
      {
        lLength = 0;
      }
      fclose( hf );
    }
    return( lLength );
 } 

// write a file
BOOL writeFile( char * pszFile, void *pvLoadedFile, long lLength ) 
{
    BOOL fOK = TRUE;

    FILE *hf = NULL;
    
    hf = fopen( pszFile, "wb" );
    if ( hf == NULL ) return( FALSE );

    fwrite( pvLoadedFile, 1, lLength, hf ); 
    fclose( hf );
    
    return( fOK );
 } 

// prepare secondary drive for OpenTM2
BOOL prepareDrive( PMIGRATIONDATA pData, char chDrive ) 
{
  // leave function when drive is already in list of secondary drives
  if ( strchr( pData->szSecondaryOTMDrives, chDrive ) != NULL ) return( TRUE );

  // this is a new drive so create the approbriate directories
  sprintf( pData->szFileName1, "%c%s", chDrive, pData->szOTMPath + 1 );
  CreateDirectory( pData->szFileName1, NULL );
  sprintf( pData->szFileName1, "%c%s\\DICT", chDrive, pData->szOTMPath + 1 );
  CreateDirectory( pData->szFileName1, NULL );
  sprintf( pData->szFileName1, "%c%s\\LIST", chDrive, pData->szOTMPath + 1 );
  CreateDirectory( pData->szFileName1, NULL );
  sprintf( pData->szFileName1, "%c%s\\MEM", chDrive, pData->szOTMPath + 1 );
  CreateDirectory( pData->szFileName1, NULL );

  // add new drive to the drive list
  size_t len = strlen( pData->szSecondaryOTMDrives );
  pData->szSecondaryOTMDrives[len] = chDrive;
  pData->szSecondaryOTMDrives[len+1] = '\0';
  pData->fSecondaryDriveAdded = TRUE;
    
  return( TRUE );
} 

// prepare a shared drive for OpenTM2
BOOL prepareSharedDrive( PMIGRATIONDATA pData, char chDrive ) 
{
  sprintf( pData->szFileName1, "%c%s", chDrive, pData->szOTMPath + 1 );
  CreateDirectory( pData->szFileName1, NULL );
  sprintf( pData->szFileName1, "%c%s\\DICT", chDrive, pData->szOTMPath + 1 );
  CreateDirectory( pData->szFileName1, NULL );
  sprintf( pData->szFileName1, "%c%s\\MEM", chDrive, pData->szOTMPath + 1 );
  CreateDirectory( pData->szFileName1, NULL );
  return( TRUE );
} 


// exctract some infor from openTM2 system properties
BOOL getOTMSysPropInfo( PMIGRATIONDATA pData )
{
  PPROPSYSTEM pSysProp = NULL;
  long lOtmPropLen = 0;


  // load OpenTM2 system properties
  sprintf( pData->szFileName2, "%s\\PROPERTY\\EQFSYSW.PRP", pData->szOTMPath );
  lOtmPropLen = loadFile( pData->szFileName2, (void **)&pSysProp );
  if ( lOtmPropLen == 0 ) return( FALSE );

  pData->chOTMPrimaryDrive = pSysProp->szPrimaryDrive[0];
  strcpy( pData->szSecondaryOTMDrives, pSysProp->szDriveList );

  free( pSysProp );
  return( TRUE );
}

// compare character function (se for sorting the drive letter list)
int iCompare( const void *arg1, const void *arg2 )
{
   char *pchArg1 = (char *)arg1;
   char *pchArg2 = (char *)arg2;

   int intRc;

   if ( *pchArg1 < *pchArg2 ) return( -1 );
   if ( *pchArg1 == *pchArg2 ) return( 0 );
   return( 1 );
}

// update secondary drive list in system properties when required
BOOL updateSysProps( PMIGRATIONDATA pData )
{
  PPROPSYSTEM pSysProp = NULL;
  long lOtmPropLen = 0;

  // return when no update is required
  if ( !pData->fSecondaryDriveAdded ) return( TRUE );

  // load OpenTM2 system properties
  sprintf( pData->szFileName2, "%s\\PROPERTY\\EQFSYSW.PRP", pData->szOTMPath );
  lOtmPropLen = loadFile( pData->szFileName2, (void **)&pSysProp );
  if ( lOtmPropLen == 0 ) return( FALSE );

  // update list of secondary drives
  qsort( (void *)pData->szSecondaryOTMDrives, (size_t)strlen(pData->szSecondaryOTMDrives), (size_t)sizeof(char), iCompare );
  strcpy( pSysProp->szDriveList, pData->szSecondaryOTMDrives );

  // save OpenTM2 system properties
  if ( writeFile( pData->szFileName2, (void *)pSysProp, lOtmPropLen ) )
  {
    free( pSysProp );
    return( TRUE );
  }
  else
  {
    free( pSysProp );
    return( FALSE );
  }
  return( TRUE );
}

// check existence of a file
BOOL doesFileExistInTM( PMIGRATIONDATA pData, char chDrive, char *pszDirectory, char *pszFileName )
{
  DWORD dwAttrib = 0;

  sprintf( pData->szFileName1, "%s\\%s\\%s", pData->szTMPath, pszDirectory, pszFileName );
  if ( chDrive != '\0' ) pData->szFileName1[0] = chDrive;

  dwAttrib = GetFileAttributes( pData->szFileName1 );

  return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}
