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

#include <windows.h>
#include <wtypes.h>
#include <ctype.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>                

#ifdef __cplusplus
extern "C" {
#endif
#include "otmapi.h"
#ifdef __cplusplus
}
#endif


HINSTANCE    hiOTMAPI = NULL ;


/****************************************************************************/
/* EQFSETSLIDER                                                             */
/*                                                                          */
/* Set status slider.                                                       */
/****************************************************************************/
VOID __cdecl /*APIENTRY*/ EQFSETSLIDER (HWND hSlider, USHORT fsFlags) {
    
    typedef VOID (__cdecl /*APIENTRY*/ /*__stdcall*/ *EQFSETSLIDER) ( HWND hSlider, USHORT fsFlags);
    static EQFSETSLIDER EQFSETSLIDER_addr;

    if ( EQFSETSLIDER_addr!=NULL) {
        (*EQFSETSLIDER_addr)( hSlider, fsFlags ) ;
        return ;
    }

    if ( hiOTMAPI == NULL )
       hiOTMAPI = LoadLibraryA("OTMAPI.DLL");
    if (hiOTMAPI != NULL ) {
        EQFSETSLIDER_addr = (EQFSETSLIDER) GetProcAddress(hiOTMAPI, "EQFSETSLIDER");
        if (EQFSETSLIDER_addr != NULL ) {
            (*EQFSETSLIDER_addr)( hSlider, fsFlags ) ;
        }
//      FreeLibrary(hiOTMAPI);
//      hiOTMAPI = NULL ;
    }
    return ;
}  

/****************************************************************************/
/* EQFCONVERTFILENAMES                                                      */
/*                                                                          */
/* Convert between TM/2 internal (short) and external (long) file names.    */
/****************************************************************************/
USHORT __cdecl /*APIENTRY*/ EQFCONVERTFILENAMES ( PSZ pszFolder, PSZ pszLongFileName, PSZ pszShortFileName ) {
    USHORT   rc=1;

    typedef USHORT (__cdecl /*APIENTRY*/ *EQFCONVERTFILENAMES) ( PSZ pszFolder,PSZ pszLongFileName,PSZ pszShortFileName);
    static EQFCONVERTFILENAMES EQFCONVERTFILENAMES_addr;

    if ( EQFCONVERTFILENAMES_addr!=NULL)
        return(*EQFCONVERTFILENAMES_addr)( pszFolder, pszLongFileName, pszShortFileName ) ;


    if ( hiOTMAPI == NULL )
        hiOTMAPI =  LoadLibraryA( "OTMAPI.DLL" );
    if (hiOTMAPI) {
        EQFCONVERTFILENAMES_addr = (EQFCONVERTFILENAMES)GetProcAddress(hiOTMAPI, "EQFCONVERTFILENAMES");
        if (EQFCONVERTFILENAMES_addr) {
            rc = (*EQFCONVERTFILENAMES_addr)( pszFolder, pszLongFileName, pszShortFileName ) ;
        }
//      FreeLibrary(hiOTMAPI);
//      hiOTMAPI = NULL ;
    }
    return rc;
}

/****************************************************************************/
/* EQFFILECONVERSIONEX                                                      */
/*                                                                          */
/* Convert the contents of a file into a different code page.               */
/****************************************************************************/
USHORT __cdecl /*APIENTRY*/ EQFFILECONVERSIONEX ( PSZ pInput, PSZ pOutput, PSZ szDocLanguage, USHORT usConvertType ) {
    USHORT   rc=1 ;

    typedef USHORT (__cdecl /*APIENTRY*/ *EQFFILECONVERSIONEX) ( PSZ pInput, PSZ pOutput, PSZ szDocLanguage, USHORT usConvertType ) ;
    static EQFFILECONVERSIONEX EQFFILECONVERSIONEX_addr;

    if (EQFFILECONVERSIONEX_addr) 
        return(*EQFFILECONVERSIONEX_addr)( pInput, pOutput, szDocLanguage, usConvertType ) ;

    if ( hiOTMAPI == NULL )
        hiOTMAPI =  LoadLibraryA( "OTMAPI.DLL" );
    if (hiOTMAPI) {
        EQFFILECONVERSIONEX_addr = (EQFFILECONVERSIONEX)GetProcAddress(hiOTMAPI, "EQFFILECONVERSIONEX");
        if (EQFFILECONVERSIONEX_addr) {
            rc = (*EQFFILECONVERSIONEX_addr)( pInput, pOutput, szDocLanguage, usConvertType ) ;
        }
//      FreeLibrary(hiOTMAPI);
//      hiOTMAPI = NULL ;
    }
    return rc;
}

/****************************************************************************/
/* EQFGETSOURCELANG                                                         */
/*                                                                          */
/* Get source language of document.                                         */
/****************************************************************************/
USHORT __cdecl /*APIENTRY*/ EQFGETSOURCELANG ( PSZ pszFolder, PSZ pszFileName, PSZ pszSrcLang ) {
    USHORT   rc=1 ;
    typedef USHORT (__cdecl /*APIENTRY*/ *EQFGETSOURCELANG) ( PSZ pszFolder, PSZ pszFileName, PSZ pszSrcLang ) ;
    static EQFGETSOURCELANG EQFGETSOURCELANG_addr;

    if (EQFGETSOURCELANG_addr) 
       return(*EQFGETSOURCELANG_addr)( pszFolder, pszFileName, pszSrcLang ) ;


    if ( hiOTMAPI == NULL )
        hiOTMAPI =  LoadLibraryA( "OTMAPI.DLL" );
    if (hiOTMAPI) {
        EQFGETSOURCELANG_addr = (EQFGETSOURCELANG)GetProcAddress(hiOTMAPI, "EQFGETSOURCELANG");
        if (EQFGETSOURCELANG_addr) {
            rc = (*EQFGETSOURCELANG_addr)( pszFolder, pszFileName, pszSrcLang ) ;
        }
//      FreeLibrary(hiOTMAPI);
//      hiOTMAPI = NULL ;
    }
    return rc;
}

/****************************************************************************/
/* EQFGETTARGETLANG                                                         */
/*                                                                          */
/* Get target language of document.                                         */
/****************************************************************************/
USHORT __cdecl /*APIENTRY*/ EQFGETTARGETLANG ( PSZ pszFolder, PSZ pszFileName, PSZ pszTgtLang ) {
    USHORT   rc=1;
    typedef USHORT (__cdecl /*APIENTRY*/ *EQFGETTARGETLANG) ( PSZ pszFolder, PSZ pszFileName, PSZ pszTgtLang ) ;
    static EQFGETTARGETLANG EQFGETTARGETLANG_addr;

    if (EQFGETTARGETLANG_addr) 
       return(*EQFGETTARGETLANG_addr)( pszFolder, pszFileName, pszTgtLang ) ;

    if ( hiOTMAPI == NULL )
        hiOTMAPI =  LoadLibraryA( "OTMAPI.DLL" );
    if (hiOTMAPI) {
        EQFGETTARGETLANG_addr = (EQFGETTARGETLANG)GetProcAddress(hiOTMAPI, "EQFGETTARGETLANG");
        if (EQFGETTARGETLANG_addr) {
            rc = (*EQFGETTARGETLANG_addr)( pszFolder, pszFileName, pszTgtLang ) ;
        }
//      FreeLibrary(hiOTMAPI);
//      hiOTMAPI = NULL ;
    }
    return rc;
}

/****************************************************************************/
/* EQFGETNEXTSEG                                                            */
/*                                                                          */
/* Get next segment.                                                        */
/****************************************************************************/
USHORT __cdecl /*APIENTRY*/ EQFGETNEXTSEG( LONG lInfo, PUSHORT ulSegNum,  PCHAR swTemp, PUSHORT usBufSize) {
	USHORT   rc=1;
    typedef USHORT (__cdecl /*APIENTRY*/ *EQFGETNEXTSEG) (LONG lInfo, PUSHORT ulSegNum, PCHAR swTemp, PUSHORT usBufSize);
    static EQFGETNEXTSEG EQFGETNEXTSEG_addr;

    if ( EQFGETNEXTSEG_addr!=NULL) 
        return(*EQFGETNEXTSEG_addr)(lInfo, ulSegNum, swTemp, usBufSize) ;

    if ( hiOTMAPI == NULL )
       hiOTMAPI = LoadLibraryA("OTMAPI.DLL");
    if (hiOTMAPI != NULL ) {
        EQFGETNEXTSEG_addr = (EQFGETNEXTSEG) GetProcAddress(hiOTMAPI, "EQFGETNEXTSEG");
        if (EQFGETNEXTSEG_addr != NULL ) {
            rc = (*EQFGETNEXTSEG_addr)(lInfo, ulSegNum, swTemp, usBufSize ) ;
        }
//      FreeLibrary(hiOTMAPI);
//      hiOTMAPI = NULL ;
    }
    return rc;
}  


/****************************************************************************/
/* EQFGETNEXTSEGW                                                           */
/*                                                                          */
/* Get next segment.                                                        */
/****************************************************************************/
USHORT __cdecl /*APIENTRY*/ EQFGETNEXTSEGW( LONG lInfo, PULONG ulSegNum, PCHAR_W swTemp, PUSHORT usBufSize) {
	USHORT   rc=1;
    typedef USHORT (__cdecl /*APIENTRY*/ *EQFGETNEXTSEGW) (LONG lInfo, PULONG ulSegNum, PCHAR_W swTemp, PUSHORT usBufSize);
    static EQFGETNEXTSEGW EQFGETNEXTSEGW_addr;

    if ( EQFGETNEXTSEGW_addr!=NULL) 
        return(*EQFGETNEXTSEGW_addr)(lInfo, ulSegNum, swTemp, usBufSize) ;

    if ( hiOTMAPI == NULL )
       hiOTMAPI = LoadLibraryA("OTMAPI.DLL");
    if (hiOTMAPI != NULL ) {
        EQFGETNEXTSEGW_addr = (EQFGETNEXTSEGW) GetProcAddress(hiOTMAPI, "EQFGETNEXTSEGW");
        if (EQFGETNEXTSEGW_addr != NULL ) {
            rc = (*EQFGETNEXTSEGW_addr)(lInfo, ulSegNum, swTemp, usBufSize ) ;
        }
//      FreeLibrary(hiOTMAPI);
//      hiOTMAPI = NULL ;
    }
    return rc;
}  


/****************************************************************************/
/* EQFGETPREVSEG                                                            */
/*                                                                          */
/* Get previous segment.                                                    */
/****************************************************************************/
USHORT __cdecl /*APIENTRY*/ EQFGETPREVSEG( LONG lInfo, PUSHORT ulSegNum,  PCHAR swTemp, PUSHORT usBufSize) {
	USHORT   rc=1;
    typedef USHORT (__cdecl /*APIENTRY*/ *EQFGETPREVSEG) (LONG lInfo, PUSHORT ulSegNum, PCHAR swTemp, PUSHORT usBufSize);
    static EQFGETPREVSEG EQFGETPREVSEG_addr;

    if ( EQFGETPREVSEG_addr!=NULL) 
        return(*EQFGETPREVSEG_addr)(lInfo, ulSegNum, swTemp, usBufSize) ;

    if ( hiOTMAPI == NULL )
       hiOTMAPI = LoadLibraryA("OTMAPI.DLL");
    if (hiOTMAPI != NULL ) {
        EQFGETPREVSEG_addr = (EQFGETPREVSEG) GetProcAddress(hiOTMAPI, "EQFGETPREVSEG");
        if (EQFGETPREVSEG_addr != NULL ) {
            rc = (*EQFGETPREVSEG_addr)(lInfo, ulSegNum, swTemp, usBufSize ) ;
        }
//      FreeLibrary(hiOTMAPI);
//      hiOTMAPI = NULL ;
    }
    return rc;
}  


/****************************************************************************/
/* EQFGETPREVSEGW                                                           */
/*                                                                          */
/* Get next segment.                                                        */
/****************************************************************************/
USHORT __cdecl /*APIENTRY*/ __cdecl /*APIENTRY*/ EQFGETPREVSEGW( LONG lInfo, PULONG ulSegNum, PCHAR_W swTemp, PUSHORT usBufSize) {
	USHORT   rc=1;
    typedef USHORT (__cdecl /*APIENTRY*/ *EQFGETPREVSEGW) (LONG lInfo, PULONG ulSegNum, PCHAR_W swTemp, PUSHORT usBufSize);
    static EQFGETPREVSEGW EQFGETPREVSEGW_addr;

    if ( EQFGETPREVSEGW_addr!=NULL) 
        return(*EQFGETPREVSEGW_addr)(lInfo, ulSegNum, swTemp, usBufSize) ;

    if ( hiOTMAPI == NULL )
       hiOTMAPI = LoadLibraryA("OTMAPI.DLL");
    if (hiOTMAPI != NULL ) {
        EQFGETPREVSEGW_addr = (EQFGETPREVSEGW) GetProcAddress(hiOTMAPI, "EQFGETPREVSEGW");
        if (EQFGETPREVSEGW_addr != NULL ) {
            rc = (*EQFGETPREVSEGW_addr)(lInfo, ulSegNum, swTemp, usBufSize ) ;
        }
//      FreeLibrary(hiOTMAPI);
//      hiOTMAPI = NULL ;
    }
    return rc;
}  
