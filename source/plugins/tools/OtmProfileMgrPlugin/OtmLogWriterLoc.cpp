//+----------------------------------------------------------------------------+
//|OtmLogWriter.CPP     OTM  Plugin Manager Parser function                    |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2015, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:             Flora Lee                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:        This is module contains some functions which are used   |
//|                    during plugin manager parser                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+

#include "OtmLogWriterLoc.h"
#include <time.h>
#include <stdarg.h>


// application name in the registry
#define APPL_NAME "OpenTM2"

// key for the OpenTM2 syste path
#define KEY_PATH    "Path"

// key for the OpenTM2 syste path
#define KEY_DRIVE    "Drive"

/*! \brief Implementation of the OtmLogWriter class
 *
 * 
 */



/*! \brief Constructors */
OtmLogWriter::OtmLogWriter()
{
  this->hfLog = NULL;
  this->fOpen = FALSE;
  this->fActive = FALSE;
}

/*! \brief Denstructor */
OtmLogWriter::~OtmLogWriter()
{
  if ( this->hfLog != NULL ) fclose( this->hfLog );
}

/*! \brief open a log file
\param pszName name of the log file (without path and extention)
\param iOpenMode open mode for log file, default is OM_ANSI and OM_APPEND
\returns 0 when successful or an error code
*/
int OtmLogWriter::open( const char *pszName, int iOpenMode )
{
  int iRC = 0;

  strcpy( this->szOpenFlags, (iOpenMode & OM_APPEND ) ? "a" : "w" );
  if ( iOpenMode & OM_UTF16 )
  {
    strcat( this->szOpenFlags, "b" );
  } /* endif */     

  // get system path from the registry
  HKEY hKey = NULL;
  this->szLogFileName[0] = 0;
  if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, APP_SOFTWARE_STR, 0, KEY_READ, &hKey ) == ERROR_SUCCESS )
  {
    HKEY hSubKey = NULL;
    if ( RegOpenKeyEx( hKey, APPL_NAME, 0, KEY_READ, &hSubKey ) == ERROR_SUCCESS )
    {
      DWORD dwType = REG_SZ;
      DWORD iSize = sizeof( this->szLogFileName );
      int iSuccess = RegQueryValueEx( hSubKey, KEY_DRIVE, 0, &dwType, (LPBYTE)this->szLogFileName, &iSize );
      if ( iSuccess == ERROR_SUCCESS )
      {
        dwType = REG_SZ;
        iSize = sizeof( this->szLogFileName );
        strcat( this->szLogFileName, "\\" );
        iSuccess = RegQueryValueEx( hSubKey, KEY_PATH, 0, &dwType, (LPBYTE)(this->szLogFileName+strlen(this->szLogFileName)), &iSize );
        if ( iSuccess != ERROR_SUCCESS )
        {
          this->szLogFileName[0] = 0;
        } /* endif */         
      } /* endif */         
      RegCloseKey(hSubKey);
    } /* endif */        
    RegCloseKey( hKey );
  } /* endif */     

  // temp add start
  if ((NULL == this->szLogFileName) || (this->szLogFileName[0] == 0))
  {
      char strModule[MAX_PATH];
      memset(strModule, 0x00, sizeof(strModule));

      GetModuleFileName(NULL, strModule, sizeof(strModule));
      GetModuleAppPath(strModule, this->szLogFileName);
  }
  // temp add end

  if ( this->szLogFileName[0] == 0 )
  {
    this->fOpen = FALSE;
    this->fActive = FALSE;
    return( -1 );
  } /* end */     
  
  // create directory but don't care for errors
  strcat ( this->szLogFileName, "\\LOGS" );
  CreateDirectory( this->szLogFileName, NULL );

  // setup complete log file name
  strcat( this->szLogFileName, "\\" );
  strcat( this->szLogFileName, pszName );
  strcat( this->szLogFileName, ".LOG" );


  this->hfLog = fopen( this->szLogFileName, this->szOpenFlags );
  if ( this->hfLog == NULL )
  {
    return( GetLastError() );
  } /* endif */     

  this->fOpen = TRUE;

  time_t lCurTime = 0;  
  time( &lCurTime );
  if ( iOpenMode & OM_UTF16 )
  {
    this->fUTF16 = TRUE;
    fwrite( UNICODEFILEPREFIX, 1, 2, this->hfLog );
    fwprintf( hfLog, L"------ Loggin started at %s", _wasctime( localtime( &lCurTime ) ) );
  }
  else
  {
    this->fUTF16 = FALSE;
    fprintf( hfLog, "------ Logging started at %s", asctime( localtime( &lCurTime ) ) );
  } /* endif */     
  this->fForceWrite = TRUE; // temporarely active forced write always
  if ( this->fForceWrite ) this->flush();
  return( iRC );
}

/*! \brief check if log file is open
\returns TRUE when log file is open
*/
bool OtmLogWriter::isOpen()
{
  return( this->fOpen );
}


/*! \brief close a log file
\param pszName name of the log file (without path and extention)
\returns 0 when successful or an error code
*/
int OtmLogWriter::close()
{
  if ( this->hfLog != NULL ) fclose( hfLog );
  this->hfLog = NULL;
  this->fOpen = FALSE;
  return( 0 );
}

/*! \brief flush the log file
\returns 0 when successful or an error code
*/
int OtmLogWriter::flush()
{
  if ( this->hfLog != NULL )
  {
    // brute force flush method...
    //fclose( this->hfLog );
    //this->hfLog = fopen( this->szLogFileName, this->szOpenFlags );

    fflush( this->hfLog );
  }
  return( 0 );
}


/*! \brief write formatted text to a log file
\param pszFormat format string (printf syntax)
\param ... variable argument list
\returns 0 when successful or an error code
*/
int OtmLogWriter::writef( const char *pszFormat, ... )
{
  va_list vArgs;

  if ( this->fOpen )
  {
    va_start( vArgs, pszFormat );
    vfprintf( this->hfLog, pszFormat, vArgs );
    va_end( vArgs );

    // add CRLF when pszFormat does not contain one at the end
    if ( *pszFormat )
    {
      int iLen = strlen(pszFormat);
      if ( (pszFormat[iLen-1] != '\n') && (pszFormat[iLen-1] != '\r') )
      {
        this->write( "" );      
      } /* end */         
    } /* endif */       
    if ( this->fForceWrite ) this->flush();
  } /* endif */
  return( 0 );
}

/*! \brief write formatted text to a log file (UTF16 version)
  \param pszFormat format string (printf syntax)
  \param ... variable argument list
	\returns 0 when successful or an error code
  */
int OtmLogWriter::writewf( const wchar_t *pszFormat, ... )
{
  va_list vArgs;

  if ( this->fOpen )
  {
    va_start( vArgs, pszFormat );
    vfwprintf( this->hfLog, pszFormat, vArgs );
    va_end( vArgs );
    if ( this->fForceWrite ) this->flush();
  } /* endif */
  return( 0 );
}

/*! \brief write a single string to the a log file
\param pszString string being written to log
\returns 0 when successful or an error code
*/
int OtmLogWriter::write( const char *pszString )
{
  if ( this->fOpen )
  {
    if ( this->fUTF16 )
    {
      fwprintf( this->hfLog, L"%S\r\n", pszString );
    }
    else
    {
      fprintf( this->hfLog, "%s\r\n", pszString );
    } /* end */       
    if ( this->fForceWrite ) this->flush();
  } /* endif */
  return( 0 );
}

/*! \brief write a single string to the a log file
\param strString reference to string being written to log
\returns 0 when successful or an error code
*/
int OtmLogWriter::write( std::string &strString )
{
  if ( this->fOpen )
  {
    if ( this->fUTF16 )
    {
      fwprintf( this->hfLog, L"%S\r\n", strString.c_str() );
    }
    else
    {
      fprintf( this->hfLog, "%s\r\n", strString.c_str() );
    } /* end */       
    if ( this->fForceWrite ) this->flush();
  } /* endif */
  return( 0 ); 
}

int OtmLogWriter::open2(const char *pszName, char * szLogFileName, FILE * & hfLog, int iOpenMode)
{
    int iRC = 0;

    char szOpenFlags[4];
    strcpy(szOpenFlags, (iOpenMode & OM_APPEND ) ? "a" : "w");
    if (iOpenMode & OM_UTF16)
    {
        strcat(szOpenFlags, "b");
    }

    REGSAM regOption = NULL;
    char strSubKey[MAX_PATH];
    memset(strSubKey, 0x00, sizeof(strSubKey));

    if (OtmIsWow64())
    {
        regOption = KEY_READ | KEY_WOW64_64KEY;
        sprintf(strSubKey, "%s\\%s", APP_SOFTWARE_STR, REG_KEY_WOW_6432_NODE_STR);
    }
    else
    {
        regOption = KEY_READ | KEY_WOW64_32KEY;
        sprintf(strSubKey, "%s", APP_SOFTWARE_STR);
    }

    // get system path from the registry
    HKEY hKey = NULL;
    szLogFileName[0] = 0;
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, strSubKey, 0, regOption, &hKey) == ERROR_SUCCESS)
    {
        HKEY hSubKey = NULL;
        if (RegOpenKeyEx(hKey, APPL_NAME, 0, KEY_ALL_ACCESS, &hSubKey) == ERROR_SUCCESS)
        {
            DWORD dwType = REG_SZ;
            DWORD iSize = sizeof(szLogFileName);
            iRC = RegQueryValueEx(hSubKey, KEY_DRIVE, 0, &dwType, (LPBYTE)szLogFileName, &iSize);
            if (iRC == ERROR_SUCCESS)
            {
                dwType = REG_SZ;
                iSize = sizeof(szLogFileName);
                strcat(szLogFileName, "\\");
                iRC = RegQueryValueEx( hSubKey, KEY_PATH, 0, &dwType, (LPBYTE)(szLogFileName+strlen(szLogFileName)), &iSize );
                if (iRC != ERROR_SUCCESS)
                {
                    szLogFileName[0] = 0;
                }
            }
            RegCloseKey(hSubKey);
        }
        RegCloseKey( hKey );
    }

    if ((NULL == szLogFileName) || (szLogFileName[0] == 0))
    {
        char strModule[MAX_PATH];
        memset(strModule, 0x00, sizeof(strModule));

        GetModuleFileName(NULL, strModule, sizeof(strModule));
        GetModuleAppPath(strModule, szLogFileName);
    }

    if (szLogFileName[0] == 0)
    {
        iRC = -1;
        return iRC;
    }

    // create directory but don't care for errors
    strcat (szLogFileName, "\\LOGS" );
    CreateDirectory(szLogFileName, NULL);

    // setup complete log file name
    strcat(szLogFileName, "\\");
    strcat(szLogFileName, pszName);
    strcat(szLogFileName, ".LOG");

    BOOL bNew = FALSE;
    if (-1 != access(szLogFileName, 0))
    {
        bNew = TRUE;
    }

    hfLog = fopen(szLogFileName, szOpenFlags);
    if (hfLog == NULL)
    {
        iRC = GetLastError();
        return iRC;
    }

    if ((iOpenMode & OM_UTF16) && bNew)
    {
        // add unicode prefix
        fwrite(UNICODEFILEPREFIX, 1, 2, hfLog);
    }

    return iRC;
}

int OtmLogWriter::writef(const char *pszName, int iOpenMode, const char *pszFormat, ...)
{
    int iRC = NO_ERROR;

    char szLogFileName[512];
    memset(szLogFileName, 0x00, sizeof(szLogFileName));

    FILE *hfLog = NULL;

    if (0 == iOpenMode)
    {
        iRC = open2(pszName, szLogFileName, hfLog);
    }
    else
    {
        iRC = open2(pszName, szLogFileName, hfLog, iOpenMode);
    }

    if (iRC < 0)
    {
        return iRC;
    }

    va_list vArgs;

    va_start(vArgs, pszFormat);
    vfprintf(hfLog, pszFormat, vArgs);
    va_end(vArgs);

    // add CRLF when pszFormat does not contain one at the end
    if (*pszFormat)
    {
        int iLen = strlen(pszFormat);
        if ((pszFormat[iLen-1] != '\n') && (pszFormat[iLen-1] != '\r'))
        {
            if (iOpenMode & OM_UTF16)
            {
                fwprintf(hfLog, L"%S\n", "");
            }
            else
            {
                fprintf(hfLog, "%s\n", "");
            }
        }
    }

    if (hfLog != NULL)
    {
        fclose(hfLog);
    }

    return iRC;
}

int OtmLogWriter::timestamp(const char *pszName, int iOpenMode)
{
    int iRC = NO_ERROR;

    char szLogFileName[512];
    memset(szLogFileName, 0x00, sizeof(szLogFileName));

    FILE *hfLog = NULL;

    if (0 == iOpenMode)
    {
        iRC = open2(pszName, szLogFileName, hfLog);
    }
    else
    {
        iRC = open2(pszName, szLogFileName, hfLog, iOpenMode);
    }

    if (NO_ERROR != iRC)
    {
        return iRC;
    }

    time_t lCurTime = 0;
    time(&lCurTime);

    if (iOpenMode & OM_UTF16)
    {
        fwprintf(hfLog, L"------ Loggin started at %s", _wasctime(localtime(&lCurTime)));
    }
    else
    {
        fprintf(hfLog, "------ Logging started at %s", asctime(localtime(&lCurTime)));
    }

    if (hfLog != NULL)
    {
        fclose(hfLog);
    }

    return iRC;
}
