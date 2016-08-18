/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/*********************************************************************
 *
 * FUNCTION:
 *
 * Segmentation routine to segment JDK 1.1 Property files
 * for Translation in TM/2
 *
 ********************************************************************/

/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

/* Header file for otmjdk11.dll          */

#ifndef _OTMJDK11_H_INCLUDE_
    #define _OTMJDK11_H_INCLUDE_

    #define INCL_DOSNLS
    #define INCL_WINWINDOWMGR
    #define INCL_WININPUT
    #define INCL_DOSFILEMGR
    #define INCL_DOSERRORS
    #define INCL_BASE
    #define MAX_SIZE 144
    #define MAX_LEN 1024
    #define MAX_SEGMENT_SIZE 2048  
    #define MAX_CONTEXT_SIZE 2048 
    #define szFOLDER_EXT "00\\"

    #define LRB_NONE           0
    #define LRB_STANDARD       1

    #define PRB_STANDARD       1

    #define EXPORT_CP_NONE          0
    #define EXPORT_CP_DBCS_ASCII    1
    #define EXPORT_CP_UTF8          2

    #include <ctype.h>
    #include <sys\stat.h>
    #include <io.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include "usrcalls.h"
    #include "reseq.h"
    #include "javaseg.h"
    #include "javapro.h"

    #include <windows.h>
    #include <wtypes.h>



BOOL     chk_LRB_File(char *sourceName, char *shortName, short *Type, char*MarkupName );
USHORT   TM2ConvertShortToLong( UCHAR *szPath, UCHAR *szFileName );
USHORT   ConvertFromEscapedUnicode( UCHAR *szInFile, UCHAR *szOutFile );
USHORT   ConvertToEscapedUnicode( UCHAR *szFile );
SHORT    GetCharLength( UCHAR *szText );
BOOL     SetDBCSLang( UCHAR *szLanguage );
BOOL     IsHTMLTag( UCHAR *, USHORT * ) ;

#endif

