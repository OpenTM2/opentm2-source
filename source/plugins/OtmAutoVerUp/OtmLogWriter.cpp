//+----------------------------------------------------------------------------+
//|OtmLogWriter.CPP     OTM  Plugin Manager Parser function                    |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2014, International Business Machines          |
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

#include "OtmLogWriter.h"
#include <time.h>
#include <stdarg.h>


// application name in the registry
#define APPL_NAME "OpenTM2"

// key for the OpenTM2 syste path
#define KEY_PATH    "Path"

// key for the OpenTM2 syste path
#define KEY_DRIVE    "Drive"

/*! \brief Implementation of the LogWriter class
 *
 * 
 */



/*! \brief Constructors */
LogWriter::LogWriter()
{
  this->hfLog = NULL;
  this->fOpen = FALSE;
  this->fActive = FALSE;
}

/*! \brief Denstructor */
LogWriter::~LogWriter()
{
  if ( this->hfLog != NULL ) fclose( this->hfLog );
}

/*! \brief open a log file
\param pszName name of the log file (without path and extention)
\param iOpenMode open mode for log file, default is OM_ANSI and OM_APPEND
\returns 0 when successful or an error code
*/
int LogWriter::open( const char *pszName, int iOpenMode )
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
  return( 0 );
}

/*! \brief check if log file is open
\returns TRUE when log file is open
*/
bool LogWriter::isOpen()
{
  return( this->fOpen );
}


/*! \brief close a log file
\param pszName name of the log file (without path and extention)
\returns 0 when successful or an error code
*/
int LogWriter::close()
{
  if ( this->hfLog != NULL ) fclose( hfLog );
  this->hfLog = NULL;
  this->fOpen = FALSE;
  return( 0 );
}

/*! \brief flush the log file
\returns 0 when successful or an error code
*/
int LogWriter::flush()
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
int LogWriter::writef( const char *pszFormat, ... )
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
int LogWriter::writewf( const wchar_t *pszFormat, ... )
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
int LogWriter::write( const char *pszString )
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
int LogWriter::write( std::string &strString )
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


