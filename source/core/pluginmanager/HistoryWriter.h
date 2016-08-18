//+----------------------------------------------------------------------------+
//|OtmLogWriter.h     OTM  Plugin Manager Parser function                      |
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

#pragma once

#include <string.h>
#include <stdarg.h>
#include <stdio.h>
#include "core\pluginmanager\OtmComm.h"

#define UNICODEFILEPREFIX                             "\xFF\xFE"

class __declspec(dllexport) HistoryWriter
{
public:
  /*! \brief Open mode */
  static const int OM_ANSI  = 1;
  static const int OM_UTF16 = 2;
  static const int OM_APPEND = 4;
  static const int OM_OVERWRITE = 8;

  /*! \brief Constructors */
  HistoryWriter::HistoryWriter(void);

  /*! \brief Denstructor */
  HistoryWriter::~HistoryWriter(void);

  /*! \brief open a log file
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

  int writef( const char *pszName, const char *pszFormat, ... );

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

