//+----------------------------------------------------------------------------+
//| OtmCheckRegistry.C                                                         |
//+----------------------------------------------------------------------------+
//| Copyright (C) 2018, International Business Machines                        |
//| Corporation and others.  All rights reserved.                              |
//+----------------------------------------------------------------------------+
//| Author: Gerhard Queck                                                      |
//+----------------------------------------------------------------------------+
//| Tool to check and correct the registry settings of OpenTM2                 |
//+----------------------------------------------------------------------------+
#undef  DLL
#undef  _WINDLL
#include "EQF.H"
#include "EQFSERNO.H"


typedef struct _OPENTM2REGENTRY
{
  CHAR szRegistryKey[512];
  CHAR szEntry[512];
  CHAR szExpectedValue[80];  // expected registry value 
} OPENTM2REGENTRY, *POPENTM2REGENTRY;

OPENTM2REGENTRY RegistryEntries[] = {
  { "SOFTWARE\\WOW6432Node\\OpenTM2", "Drive", "C:" },
  { "SOFTWARE\\WOW6432Node\\OpenTM2", "Path", "OTM" },
  { "SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall\\OpenTM2", "DisplayVersion", "1.4.2" },
  { "", "", "" } 
};

BOOL fTestOnly = FALSE;
BOOL fVerbose = FALSE;

void showHelp();
BOOL checkAndCorrectRegistryEntry( PSZ pszKey, PSZ pszEntry, PSZ pszValue, BOOL fTest, BOOL fVerbose, int *piErrors, int *piCorrected );
BOOL updateExpectedValues( BOOL fVerbose );
void showSystemError( DWORD dwRC );


int main( int argc, char **argv )
{
  int iErrors = 0;
  int iCorrected = 0;

  argc--; argv++;    // skip program name
  while ( argc )
  {
    if ( (stricmp( argv[0], "/TEST" ) == 0) || (stricmp( argv[0], "/T" ) == 0) )
    {
      fTestOnly = TRUE;
    }
    else if ( (stricmp( argv[0], "/VERBOSE" ) == 0) || (stricmp( argv[0], "/V" ) == 0) )
    {
      fVerbose = TRUE;
    }
    else
    {
      showHelp();
      return( 0 ); 
    } /* endif */
    argc--;
    argv++;
  } /* endwhile */

  updateExpectedValues( fVerbose );

  printf( "Checking OpenTM2 registry entries....\n" );

  POPENTM2REGENTRY pEntry = RegistryEntries;

  while( pEntry->szRegistryKey[0] != 0 )
  {
    checkAndCorrectRegistryEntry( pEntry->szRegistryKey, pEntry->szEntry, pEntry->szExpectedValue, fTestOnly, fVerbose, &iErrors, &iCorrected );
    pEntry++;
  } /* endwhile */

  if ( iErrors != 0 )
  {
    printf( "%ld registry entries were missing or incorrect, %ld entries have been corrected.\n", iErrors, iCorrected );
    return( 0 );
  }
  else
  {
    printf( "Found no incorrect registry entries.\n" );
    return( 1 );
  }
}

// get a string from the registry
BOOL checkAndCorrectRegistryEntry( PSZ pszKey, PSZ pszEntry, PSZ pszValue, BOOL fTest, BOOL fVerbose, int *piErrors, int *piCorrected )
{
  BOOL fOK = FALSE;
  HKEY hKey = NULL;
  int iRC = 0;

  if ( fVerbose )
  {
    printf( "Info: Checking entry HKEY_LOCAL_MACHINE\\%s:%s\n", pszKey, pszEntry );
  } /* endif */
  iRC = RegOpenKeyEx( HKEY_LOCAL_MACHINE, pszKey, 0, KEY_READ | KEY_WRITE, &hKey );
  if ( iRC == ERROR_SUCCESS )
  {
    CHAR szValue[512];
    DWORD dwType = REG_SZ;
    DWORD iSize = sizeof(szValue);
    iRC = RegQueryValueEx( hKey, pszEntry, 0, &dwType, (LPBYTE)szValue, &iSize );
    if ( iRC == ERROR_SUCCESS)
    {
      if ( fVerbose )
      {
        printf( "Info: Current value of HKEY_LOCAL_MACHINE\\%s:%s is %s\n", pszKey, pszEntry, szValue );
      } /* endif */
      if ( strcmp( szValue, pszValue ) != 0 )
      {
        printf( "Error: Value of entry HKEY_LOCAL_MACHINE\\%s:%s is %s, expected value is %s\n", pszKey, pszEntry, szValue, pszValue );
        (*piErrors)++;
        if ( !fTest )
        {
          iRC = RegSetKeyValue( hKey, NULL, pszEntry, REG_SZ, (LPBYTE)pszValue, strlen(pszValue) + 1 );
          if ( iRC == ERROR_SUCCESS  )
          {
            printf( "Info: Successfully corrected value of entry HKEY_LOCAL_MACHINE\\%s:%s, the new value is %s\n", pszKey, pszEntry, pszValue );
            (*piCorrected)++;
          }
          else
          {
            printf( "Error: Failed to update value of entry HKEY_LOCAL_MACHINE\\%s:%s (RC=%ld)\n", pszKey, pszEntry, iRC );
            showSystemError( iRC );
          } /* endif */
        } /* endif */
      } /* endif */
    }
    else 
    {
      printf( "Error: Entry HKEY_LOCAL_MACHINE\\%s:%s does not exist\n", pszKey, pszEntry );
      if ( !fTest )
      {
        iRC = RegSetKeyValue( hKey, NULL, pszEntry, REG_SZ, (LPBYTE)pszValue, strlen(pszValue) + 1 );
        if ( iRC == ERROR_SUCCESS  )
        {
          printf( "Info: Successfully added missing entry HKEY_LOCAL_MACHINE\\%s:%s, the new value is %s\n", pszKey, pszEntry, pszValue );
          (*piCorrected)++;
        }
        else
        {
          printf( "Error: Failed to add missing entry HKEY_LOCAL_MACHINE\\%s:%s (RC=%ld)\n", pszKey, pszEntry, iRC );
          showSystemError( iRC );
        } /* endif */
      } /* endif */
    } /* endif */
    RegCloseKey( hKey );
  }
  else
  {
    printf( "Error: Key HKEY_LOCAL_MACHINE\\%s not found in registry\n", pszKey );
    (*piErrors)++;

    // create the missing key
    if ( !fTest )
    {
      iRC = RegCreateKeyEx( HKEY_LOCAL_MACHINE, pszKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE, NULL, &hKey, NULL );
      if ( iRC == ERROR_SUCCESS  )
      {
        iRC = RegSetKeyValue( hKey, NULL, pszEntry, REG_SZ, (LPBYTE)pszValue, strlen(pszValue) + 1 );
        if ( iRC == ERROR_SUCCESS  )
        {
          printf( "Info: Successfully added missing entry HKEY_LOCAL_MACHINE\\%s:%s, the new value is %s\n", pszKey, pszEntry, pszValue );
          (*piCorrected)++;
        }
        else
        {
          printf( "Error: Failed to add missing entry HKEY_LOCAL_MACHINE\\%s:%s (RC=%ld)\n", pszKey, pszEntry, iRC );
          showSystemError( iRC );
        } /* endif */
        RegCloseKey( hKey );
      }
      else
      {
        printf( "Error: Key HKEY_LOCAL_MACHINE\\%s could not be created in the registry (RC=%ld)\n", pszKey, iRC );
        showSystemError( iRC );
      }
    } /* endif */
  } /* endif */        
  return( fOK );
} /* end of function GetStringFromRegistry */

void showHelp()
{
    printf( "OtmCheckRegistry.EXE  : OpenTM2 registry settings utility\n" );
    printf( "Version               : %s\n", STR_DRIVER_LEVEL_NUMBER );
    printf( "Copyright             : %s\n", STR_COPYRIGHT );
    printf( "Purpose               : Check and correct the OpenTM2 registry settings\n" );
    printf( "Syntax format         : OtmCheckRegistry [/Test]  [/Verbose]\n" );
    printf( "Options and parameters:\n" );
    printf( "    /Test or /T          test registry settings without correcting them\n" );
    printf( "    /Verbose or /V       show all registry entries being tested\n" );
}

// update expected registry values
BOOL updateExpectedValues( BOOL fVerbose)
{
  CHAR szDrive[3];

  // find OTM installation on available fixed disks
  szDrive[0] = 0;
  for ( int i = 0; (i < 26) && (szDrive[0] == 0); i++)
  {
    CHAR szRoot[4] = "C:\\";
    szRoot[0] = (CHAR)('A' + i);

    WORD wDriveInfo = GetDriveType( szRoot );
    if ( (wDriveInfo == DRIVE_FIXED) || (wDriveInfo == DRIVE_REMOTE) )
    {
      char szOpenTM2Exe[60];
      strcpy( szOpenTM2Exe, szRoot );
      strcat( szOpenTM2Exe, "OTM\\WIN\\OpenTM2.EXE" );
      if ( GetFileAttributes( szOpenTM2Exe ) != INVALID_FILE_ATTRIBUTES )
      {
        szDrive[0] = szRoot[0];
        szDrive[1] = ':';
        szDrive[2] = 0;
        if ( fVerbose )
        {
          printf( "Info: Found OpenTM2 installation on drive %s\n", szDrive );
        } /* endif */
      }
    } /* endif */
  } /* endfor */


  // update the drive entry
  POPENTM2REGENTRY pEntry = RegistryEntries;
  while( pEntry->szRegistryKey[0] != 0 )
  {
    if ( stricmp( pEntry->szEntry, "Drive" ) == 0 )
    {
      strcpy( pEntry->szExpectedValue, szDrive );
    } /* endif */
    pEntry++;
  } /* endwhile */

  


  return( TRUE );
}

void showSystemError( DWORD dwRC )
{
  LPTSTR pErrorText = NULL;
  FormatMessage( FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS, NULL, dwRC, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&pErrorText, 0, NULL);   
  if ( pErrorText != NULL )
  {
    printf( "      Reason: %s", pErrorText );
    LocalFree( pErrorText );
  }
  if ( dwRC == 5 )
  {
    printf( "      Try to run this program in administrator mode!\n" );
  } /* endif */
}