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

#include "OtmLogWriterEnv.h"
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

void LogWriter::SetLogPath(const char * strPath)
{
  strcpy(this->szPath, strPath);
}


/*! \brief Denstructor */
LogWriter::~LogWriter()
{
  if ( this->hfLog != NULL ) fclose( this->hfLog );
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

int LogWriter::open2( const char *pszName )
{
    int iRC = 0;

    int iOpenMode = (OM_ANSI | OM_APPEND);

    strcpy( this->szOpenFlags, (iOpenMode & OM_APPEND ) ? "a" : "w" );
    if ( iOpenMode & OM_UTF16 )
    {
        strcat( this->szOpenFlags, "b" );
    } /* endif */

    strcpy ( this->szLogFileName, this->szPath);

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

    this->fForceWrite = TRUE; // temporarely active forced write always
    if ( this->fForceWrite ) this->flush();
    return( 0 );
}

int LogWriter::writef( const char *pszName, const char *pszFormat, ... )
{
  if (this->open2(pszName) < 0)
  {
      return (-1);
  }
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

  this->close();
  return( 0 );
}
