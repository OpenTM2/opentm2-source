/*! \brief TMXFactory.H - Include file for the LogWriter class
	Copyright (c) 1999-2015, International Business Machines Corporation and others. All rights reserved.
	Description: This file contains the class definition for the LogWriter tool
*/

#ifndef _LOGWRITER_H_
#define _LOGWRITER_H_

#include <eqf.h>
#include "string"
#include <stdarg.h>
#include <stdio.h>

class __declspec(dllexport) LogWriter
{
public:
  /*! \brief Open mode */
  static const int OM_ANSI  = 1;
  static const int OM_UTF16 = 2;
  static const int OM_APPEND = 4;
  static const int OM_OVERWRITE = 8;

  /*! \brief Constructors */
  LogWriter::LogWriter();

  /*! \brief Denstructor */
  LogWriter::~LogWriter();

  /*! \brief Register a log file
       
       After registering the log file will be displayed in the log selection panel and
       can be activated or deactivated by the user. The open method for registered
       log files will only open the log file when the user has activated the logging.
       For unregistered log files3 the open method will open the log file in any case.

  \param pszLogFileName name of the log file (without path and extention)
  \param pszDescription a description of the kind of logging done using this log file
	\returns 0 when successful or an error code
  */
  static int registerLogFile( const char *pszLogFileName, const char *pszDescription );

  /*! \brief open a log file

       The open method for registered log files will only open the log file when the 
       user has activated the logging.
       For unregistered log files the open method will open the log file in any case.
  \param pszName name of the log file (without path and extention)
  \param iOpenMode open mode for log file, default is OM_ANSI and OM_APPEND
	\returns 0 when successful or an error code
  */
  int open( const char *pszName, int iOpenMode = (OM_ANSI | OM_APPEND) );

  /*! \brief check if log file is open
	\returns TRUE when log file is open
  */
  bool isOpen();

  /*! \brief close a log file
  \param pszName name of the log file (without path and extention)
	\returns 0 when successful or an error code
  */
  int close();


  /*! \brief write formatted text to a log file
  \param pszFormat format string (printf syntax)
  \param ... variable argument list
	\returns 0 when successful or an error code
  */
  int writef( const char *pszFormat, ... );

  /*! \brief write formatted text to a log file (UTF16 version)
  \param pszFormat format string (printf syntax)
  \param ... variable argument list
	\returns 0 when successful or an error code
  */
  int writewf( const wchar_t *pszFormat, ... );

  /*! \brief write a single string to the a log file
  \param pszString string being written to log
	\returns 0 when successful or an error code
  */
  int write( const char *pszString );

  /*! \brief write a single string to the a log file
  \param strString reference to string being written to log
	\returns 0 when successful or an error code
  */
  int write( std::string &strString );

  /*! \brief set force write/flush mode of log file
  \param bForcedWriteIn TRUE = force immediate write of log output to disk, FALSE = normal (buffered) write mode
  */
  void setForcedWrite( bool fForcedWriteIn );

private:
  FILE *hfLog;
  bool fOpen;
  bool fActive;
  bool fUTF16;
  bool fForceWrite;
  char szLogFileName[512];
  char szOpenFlags[4];

  /*! \brief flush the log file
	\returns 0 when successful or an error code
  */
  int flush();

};

#endif // _LOGWRITER_H_
