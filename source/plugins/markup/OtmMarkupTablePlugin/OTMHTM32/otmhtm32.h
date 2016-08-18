/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/* Header file for otmhtm32.dll          */

/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#ifndef _OTMHTM32_H_INCLUDE_
    #define _OTMHTM32_H_INCLUDE_
/*#include "htscript.h"*/



#define JS_TYPE_NONE          0           // File is not JS nor JSON
#define JS_TYPE_JS            1           // File is JS
#define JS_TYPE_JSON          2           // File is JSON
#define JS_TYPE_JSON_PRESEG   9           // File is JSON, PreSeg


short preSegScriptTag(char *srcFile,char *tmpFile,long *tagStartPos,long *tagEndPos);
short postUnSegScriptTag(char *srcFile,char *tmpFile,long *tagStartPos,long *tagEndPos);
short postUnSeg2kFix(char *,char *,long *,long *,wchar_t *,wchar_t *,wchar_t *);
short preSeg2kFix(char *,char *,long *,long *,wchar_t *);
short preSegDocType(char *SrcFile,char *TmpFile,long *tagStartPos,long *tagEndPos,
                    char *StartDocType,char *EndDocType,char *StartComment,char *EndComment);
short postUnSegDocType(char *SrcFile,char *TmpFile,long *tagStartPos,long *tagEndPos,
                       char *StartDocType,char *EndDocType,char *StartComment,char *EndComment);
BOOL PostSegScriptTagChar(char *sourceName, char *tempName);

#endif

