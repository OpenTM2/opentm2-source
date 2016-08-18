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

#ifndef _JAVAPRO_H_INCLUDE_
    #define _JAVAPRO_H_INCLUDE_


    #include "otmjdk11.h"

    #define MAX_RCD_LENGTH        20000    
    #define SEGMENT_NONE           0
    #define SEGMENT_NONTRANS       1
    #define SEGMENT_TRANS          2

USHORT ParseFile   ( PSZ, PSZ, SHORT, BOOL, HWND );
USHORT ExportFile  ( PSZ, PSZ );
USHORT ExportFile2 ( PSZ, PSZ, USHORT );
VOID   StartJDKSegment ( FILE *, USHORT, USHORT * ) ;
VOID   EndJDKSegment ( FILE *, USHORT ) ;
BOOL   TextContainsVariable( char * ) ;
void   HandleSingleQuotes( char *, BOOL, BOOL * ) ;
void   RemoveContextInfo( char *) ;  
BOOL   IsDBCSSentenceEnd( char *, SHORT ) ;
BOOL   IsUTF8DBCS( char *, SHORT, BOOL ) ;

#endif

