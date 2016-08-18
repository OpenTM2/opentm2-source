/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/*
*
* ibmxmodc.h
*
* Define TranslationManager analysis program for OpenDocument XML files.
*
*/
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#ifndef _IBMXMODC_H_INCLUDE_
    #define _IBMXMODC_H_INCLUDE_

    #define MAXPATHLEN 2024

    #define INCL_BASE
    #define INCL_DOSFILEMGR

    #define DosCopy(a, b , value) CopyFileA(a, b, FALSE)

    #ifndef CCHMAXPATHCOMP
        #define CCHMAXPATHCOMP MAXPATHLEN       
    #endif

    #include "unicode.h"
    #include "parse.h"
    #include "export.h"
    #include "reseq.h"

#define MAX_RCD_LENGTH         12288       
#define ODC_TAG_SIZE              80

#define ODC_STYLE_STATE_BEFORE     0 
#define ODC_STYLE_STATE_IN         1 
#define ODC_STYLE_STATE_AFTER      2 

#endif





