/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/******************************************************************************
*
* export.c
*
* Functions called to export OpenDocument XML files
*
*
* FUNCTIONS:
*   PostUnseg                called on post unsegmentation
*
******************************************************************************/
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#define INCL_DOSNLS
#define INCL_WINWINDOWMGR
#define INCL_WININPUT
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#define INCL_BASE

#include "otmxmodc.h"

#define  ODC_EXTRACT_ERROR_TITLE             "Extract XML from OpenDocument Error"
#define  ODC_EXTRACT_STYLE                   "Style information could not be replaced in the OpenDocument file."

extern  CHAR     szODC_ODCNL[9] ;
extern  CHAR     szODC_ODCNL_NL[10] ;
extern  CHAR     szODC_TWBSTYLE[13] ;

/****************************************************************************/
/*                                                                          */
/* PostUnseg                                                                */
/*                                                                          */
/* Function is called by EQFPOSTUNSEG2 during export processing.            */
/*                                                                          */
/* Input:      InFile        - Input file name to be updated.               */
/*             StyleFile     - Input file containing saved style info.      */
/* Output:     TRUE          - Processing successful.                       */
/*             FALSE         - Processing failed.                           */
/****************************************************************************/
BOOL PostUnseg(PSZ in, PSZ style)
{
    FILE     *fIn, *fOut, *fStyle ;
    CHAR     szIn[MAX_RCD_LENGTH*3] ;
    CHAR     szStyle[MAX_RCD_LENGTH] ;
    CHAR     szTempFile[512] ;
    char     *szAltTempExt1 = ".$8$";
    CHAR     *ptrChar, *ptrChar2 ;
    USHORT   usStyleState = ODC_STYLE_STATE_BEFORE ;
    USHORT   i ;
    BOOL     bReturn = TRUE;


    CreateTempFileName2( szTempFile, in, szAltTempExt1, TEMPNAME_TARGET ) ;

    fIn = fopen( in, "r" ) ;
    fOut = fopen( szTempFile, "w" ) ;
    fStyle = NULL ;
    if ( !fIn || !fOut ) {
       if ( fIn ) 
          fclose( fIn ) ;
       if ( fOut ) 
          fclose( fOut ) ;
       return( FALSE ) ;
    }

    /*------------------------------------------------------------------------*/
    /*  Remove any additional tags added during analysis processing.          */
    /*------------------------------------------------------------------------*/
    while ( GetRcd( szIn, MAX_RCD_LENGTH, fIn, TRUE ) != NULL ) {

       /*---------------------------------------------------------------------*/
       /*  Find each <ODCnl> tag and remove it and the following newline.    */
       /*---------------------------------------------------------------------*/
       for( ptrChar=strstr(szIn,szODC_ODCNL) ; 
            ptrChar ;
            ptrChar=strstr(ptrChar,szODC_ODCNL) ) {
          if ( *(ptrChar+8) == 0 ) {
             *(ptrChar+8) = fgetc(fIn) ;
             *(ptrChar+9) = 0 ;
          }
          if ( *(ptrChar+8) == '\n' ) {
             memmove( ptrChar, ptrChar+9, strlen(ptrChar+9)+1 ) ;
          } else {
             memmove( ptrChar, ptrChar+8, strlen(ptrChar+8)+1 ) ;
          }
       }

       /*---------------------------------------------------------------------*/
       /*  Replace all saved style information into the proper location.      */
       /*---------------------------------------------------------------------*/
       if ( usStyleState != ODC_STYLE_STATE_AFTER ) {
          ptrChar = strstr( szIn, szODC_TWBSTYLE ) ;
          if ( ptrChar ) {
             usStyleState = ODC_STYLE_STATE_AFTER ;
             *ptrChar = 0 ;
             fputs( szIn, fOut ) ;
             memmove( szIn, ptrChar+strlen(szODC_TWBSTYLE), strlen(ptrChar+strlen(szODC_TWBSTYLE))+1 ) ;
             if ( style[0] ) {
                fStyle = fopen( style, "r" ) ;
                if ( fStyle ) {
                   while ( fgets( szStyle, MAX_RCD_LENGTH, fStyle ) != NULL ) {
                      fputs( szStyle, fOut ) ;
                   }
                } else {
                   MessageBoxA(HWND_DESKTOP, ODC_EXTRACT_STYLE, ODC_EXTRACT_ERROR_TITLE, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
                   bReturn = FALSE ; 
                   break ;
                }
             }
          }
       }
   
       fputs( szIn, fOut ) ;
    }
    fclose( fIn ) ;
    fclose( fOut ) ;
    if ( fStyle ) 
       fclose( fStyle ) ;

    if ( bReturn ) {
       DosCopy( szTempFile, in, DCPY_EXISTING ) ;
    } else {
       remove( in ) ;
    }
    remove( szTempFile ) ;


    return(bReturn);

} /* PostUnseg */
