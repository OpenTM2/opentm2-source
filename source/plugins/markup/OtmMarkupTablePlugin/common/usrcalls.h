/*
*
*  Copyright (C) 1998-2013, International Business Machines         
*         Corporation and others. All rights reserved 
*
*/
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#ifndef _USRCALLS_H_INCLUDE_
#define _USRCALLS_H_INCLUDE_

#define INCL_BASE
#define INCL_DOSFILEMGRA



#include <windows.h>
#include <wtypes.h>
#include <tchar.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <malloc.h>
#include <ctype.h>
#include <stdlib.h>
#include <io.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif
#include "otmapi.h"
#ifdef __cplusplus
}
#endif



#define  TP_UNKNOWN   0
#define  TP_55        1
#define  TP_60        2
#define  TP_602       3
#define  TP_OTM       4

#define  TP_ANSI      1
#define  TP_HTML      2
#define  TP_ASCII     3
#define  TP_UTF8	  4

#define  TEMPNAME_SOURCE   1
#define  TEMPNAME_SSOURCE  2
#define  TEMPNAME_STARGET  3
#define  TEMPNAME_TARGET   4

#define  OTM_MARKUPPLUGIN_TABLE_DIR   "PLUGINS\\OTMMARKUPTABLEPLUGIN\\TABLE"
#define  OTM_MARKUPPLUGIN_DLL_DIR     "PLUGINS\\OTMMARKUPTABLEPLUGIN\\BIN"


extern _TCHAR * __cdecl cleanString( _TCHAR * tempString );
extern _TCHAR * __cdecl deleteSubString( _TCHAR * string1, _TCHAR * string2 );
extern int __cdecl strloc( _TCHAR * string1, _TCHAR * string2 );
extern int __cdecl strloclast( _TCHAR * string1, _TCHAR * string2 );
extern _TCHAR * __cdecl strsub( _TCHAR * string1, int subStart, int subLength );
extern _TCHAR * __cdecl strdel( _TCHAR * string1, int delStart, int delLength );
extern _TCHAR * __cdecl strins( _TCHAR * string1, _TCHAR * string2, int insIndex );
extern void __cdecl strCOPY( _TCHAR * string1, _TCHAR * string2 );
extern int __cdecl strlocnext( _TCHAR * string1, _TCHAR * string2, int next );
extern int __cdecl strnextnonblank( _TCHAR * string1, int next );
extern _TCHAR * __cdecl StrUpr( _TCHAR * Str );
extern _TCHAR * __cdecl StrnUpr( _TCHAR * Str, short starting, short numBytes );
extern short __cdecl searchAndReplaceString(_TCHAR *buffer,_TCHAR *search_str,_TCHAR *replace_str,short maxLen);

extern void __cdecl openFileToPosition(char *file,long filePos,FILE **fp);
extern long __cdecl copyPartialFile (char *srcFile, FILE **sfp,
                      char *tmpFile, FILE **tfp,
                      long startPos, long numBytes,
                      short newFile);
extern long  __cdecl StrSearchFile (FILE  **fp,         /* open file pointer                          */
                     _TCHAR  *str,         /* string to look for                         */
                     long  startPos,     /* The starting position > endPos if backward */
                     long  endPos,       /* The ending position < startPos if backward */
                     _TCHAR  *delims,      /* a set of chars to indicated end of search  */
                     long  *foundPos);    /* position the string was found in the file  */

extern void   __cdecl PrepDocLanguageInfo( char * filename );
extern int    __cdecl IsDBCS(UCHAR);
extern void   __cdecl QueryExportFiles(char *szMarkup, char *szExportList, USHORT usBufLen, BOOL bRecursive );
extern int    __cdecl StripNewline( char * text, char * newline );

extern void   __cdecl CreateTempFileName ( PSZ pTempName, PSZ pAltName ) ;
extern void   __cdecl CreateTempFileName2 ( PSZ pTempName, PSZ pBase, PSZ pAltExt, USHORT Type ) ;
extern int    __cdecl ConvertImport ( PSZ pInput, PSZ pOutput, USHORT usConvertType ) ;
extern int    __cdecl ConvertExport ( PSZ pReplace, USHORT usConvertType ) ;

extern int    __cdecl  EqfFileConversionEX(PSZ pszInFile, PSZ pszOutFile, PSZ pszLanguage, SHORT usConversionType);

extern void   __cdecl  GetOTMTablePath( char * basename, char * OTMname ) ;
extern void   __cdecl  GetOTMDllPath( char * basename, char * OTMname ) ;


#endif
