/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/* scrptseg.h file */

/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#ifndef _SCRPTSEG_H_INCLUDE_
    #define _SCRPTSEG_H_INCLUDE_

BOOL postSegScriptTag(char*,char*,long*,long*,short);
BOOL chkWhiteSpace(FILE*, FILE*, wchar_t *, long, BOOL);

BOOL preSegJSON(char*,char*,long*,long*,short);

#endif

