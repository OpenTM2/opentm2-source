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

#define INCL_DOSNLS
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS

#ifdef _WINDOWS

    #include <windows.h>
    #include <wtypes.h>
    #define DosCopy(File1, File2, BOOL)  CopyFileA(File1, File2, FALSE);

#else

    #include <os2.h>

#endif



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "otmhtm32.h"
#include "usrcalls.h"


#define  END_SCRIPT_TAG_STRING           L"{TWBSCR}"   

short preSegScriptTag(char *srcFile,char *tmpFile,long *tagStartPos,long *tagEndPos)
{
    FILE *tmpfp,*srcfp;
    wchar_t *pszBuf;
    long tagLen,fileLen;


    tmpfp = srcfp = NULL;
    copyPartialFile(srcFile, &srcfp,
                    tmpFile, &tmpfp,
                    0,                    /* beginning */
                    *tagStartPos,
                    1 );                  /* create/overwrite file */

    if ((tmpfp != NULL) && (srcfp != NULL)) {
        tagLen = (*tagEndPos) - (*tagStartPos);
        pszBuf = (wchar_t *) calloc((tagLen/sizeof(wchar_t)) + 1,1*sizeof(wchar_t));
        fread (pszBuf,2,tagLen/sizeof(wchar_t),srcfp);
        fwprintf(tmpfp,L"%s%s",pszBuf,END_SCRIPT_TAG_STRING);
        free (pszBuf);

        fseek(srcfp,0,SEEK_END);
        fileLen = ftell(srcfp);            /* last position in the file */

        copyPartialFile (srcFile, &srcfp,
                         tmpFile, &tmpfp,
                         *tagEndPos, fileLen,
                         0 );              /* append to the file */
    }

    if (tmpfp != NULL) {
        fclose (tmpfp);
    }
    if (srcfp != NULL) {
        fclose (srcfp);
    }

    DosCopy (tmpFile,srcFile,DCPY_EXISTING);
    remove (tmpFile);

    return(0);
}

short postUnSegScriptTag(char *srcFile,char *tmpFile,long *tagStartPos,long *tagEndPos)
{
    FILE *tmpfp,*srcfp;
    wchar_t cChar;
    long fileLen;
    long lCharCnt ;
    long lFileStart ;

    tmpfp = srcfp = NULL;
    copyPartialFile(srcFile, &srcfp,
                    tmpFile, &tmpfp,
                    0,                    /* beggining */
                    *tagEndPos,
                    1 );                  /* create/overwrite file */

    if ((tmpfp != NULL) && (srcfp != NULL)) {
       fseek(srcfp,0,SEEK_END);
       fileLen = ftell(srcfp);            /* last position in the file */

       fseek(srcfp,*tagEndPos,SEEK_SET);
       cChar = fgetwc(srcfp) ; 
       lFileStart = (*tagEndPos) ;        /* Do not remove any characters */
       if ( cChar == END_SCRIPT_TAG_STRING[0] ) {
          for( lCharCnt=0 ; 
               lCharCnt<wcslen(END_SCRIPT_TAG_STRING) && 
                 cChar==END_SCRIPT_TAG_STRING[lCharCnt] ;
               ++lCharCnt, cChar=fgetwc(srcfp)  ) ;
          if ( lCharCnt>=wcslen(END_SCRIPT_TAG_STRING) ) {
             lFileStart += lCharCnt*sizeof(wchar_t) ;     /* Past end string */
          }
       }
       
       copyPartialFile (srcFile, &srcfp,
                     tmpFile, &tmpfp,
                     lFileStart,          //starting point
                     fileLen,             //ending point
                     0 );                 // append to the file   
    }

    if (tmpfp != NULL) {
        fclose (tmpfp);
    }
    if (srcfp != NULL) {
        fclose (srcfp);
    }

    DosCopy (tmpFile,srcFile,DCPY_EXISTING);
    remove (tmpFile);

    return(0);
}

BOOL PostSegScriptTagChar(char *sourceName, char *tempName)
{  // routine for getting rid of special <XSCRCONT: tag

    FILE *sourceFile_ptr = NULL;
    FILE *tempFile_ptr = NULL;
    wchar_t SCRIPTCONT[11] = L"<XSCRCONT:" ;
    wchar_t SCRIPTCHAR1[13] = L"{TWBSCR}:EQF" ;        
    wchar_t TWBSCR[9] = L"{TWBSCR}" ;                    
    wchar_t EQF1[5] = L":EQF" ;                          
    wchar_t EQF2[5] = L":eqf" ;                          
    wchar_t char_str[1024+100];
    wchar_t *sub_str;
    wchar_t *end_str;
    wchar_t *ptrChar, *ptrChar2 ;
    int rc;
    int i;

    sourceFile_ptr = fopen(sourceName, "rb");
    tempFile_ptr = fopen(tempName, "wb");

          // get a line up to a newline character or 1024 bytes as input to parse
    while ((fgetws(char_str,1024,sourceFile_ptr)) != NULL) {
        if ( wcslen(char_str) == 1023 ) {                
           ptrChar = wcsrchr( char_str, '<' ) ;
           if ( ptrChar ) {
              for( i=0 ; 
                   *ptrChar && i<wcslen(SCRIPTCONT) && *ptrChar==SCRIPTCONT[i] ; 
                   ++ptrChar, ++i ) ;
           }
           ptrChar2 = NULL ;
           if ( ( ( ptrChar     ) &&
                  ( ! *ptrChar  ) ) ||
                ( ( ptrChar2    ) &&
                  ( ! *ptrChar2 ) ) ) {
              fgetws( &char_str[1023], 20, sourceFile_ptr ) ;
           }
        }

        // search for the SCRIPT continuation tag
        while ((sub_str = wcsstr(char_str,SCRIPTCONT)) != NULL) {

            end_str = sub_str + wcslen(SCRIPTCONT) ;
            if ( ( sub_str >= char_str + 8 ) &&
                 ( ! wcsncmp( (sub_str-8), END_SCRIPT_TAG_STRING, wcslen(END_SCRIPT_TAG_STRING) ) ) ) {
               sub_str -= wcslen(END_SCRIPT_TAG_STRING) ; 
            }
            memmove( sub_str, end_str, (wcslen(end_str)+1)*sizeof(wchar_t) ) ;
        }

        // search for the end SCRIPT tag character
                                                         
        for( sub_str=wcsstr(char_str,TWBSCR) ; sub_str ; sub_str=wcsstr(sub_str,TWBSCR) ) {
           for( end_str=sub_str+wcslen(TWBSCR) ; *end_str && iswspace(*end_str) ; ++end_str ) ;
           if ( ( ! wcsncmp( end_str, EQF1, wcslen(EQF1) ) ) ||
                ( ! wcsncmp( end_str, EQF2, wcslen(EQF2) ) ) ) {
              memmove( sub_str, sub_str+8, (wcslen(sub_str+8)+1)*sizeof(wchar_t) ) ;  /* Remove {TWBSCR} */
           } else {
              ++sub_str ;
           }

        }
        fputws(char_str,tempFile_ptr);
    }
    fclose(sourceFile_ptr);
    fclose(tempFile_ptr);

    rc = DosCopy(tempName, sourceName, DCPY_EXISTING); // COPIES MODIFIED TEMP BACK TO SOURCE
    remove(tempName);
    return(TRUE);
}

