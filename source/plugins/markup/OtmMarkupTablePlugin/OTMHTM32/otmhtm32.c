/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/

/*-------------------------------------------------------------------*/
/* otmhtm32.c Source file for a C DLL                                */
/*-------------------------------------------------------------------*/

/****************************************************************************/
/*                                                                          */
/* otmhtm32.c                                                               */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*    Description:                                                          */
/*         This function will examine an HTML 3.2 file and determine if     */
/*         any additional processing will be needed for segmentation to     */
/*         occur properly in TM2.  This DLL will have 4 entry points.       */
/*         These entry points are listed below and are defined by TM2.      */
/*                                                                          */
/*                                                                          */
/*    EQF_BOOL APIENTRY16 EQFPRESEG  (PSZ      pTagTable,                   */
/*                                    PSZ      pEdit,                       */
/*                                    PSZ      pProgPath,                   */
/*                                    PSZ      pSource,                     */
/*                                    PSZ      pTempSource,                 */
/*                                    PEQF_BOOL    pfNoSegment)             */
/*    EQF_BOOL APIENTRY16 EQFPOSTSEG  (PSZ      pTagTable,                  */
/*                                     PSZ      pEdit,                      */
/*                                     PSZ      pProgPath,                  */
/*                                     PSZ      pSource,                    */
/*                                     PSZ      pSegTarget,                 */
/*                                     PTATAG    pTATAG)                    */
/*    EQF_BOOL APIENTRY16 EQFPREUNSEG  (PSZ      pTagTable,                 */
/*                                      PSZ      pEdit,                     */
/*                                      PSZ      pProgPath,                 */
/*                                      PSZ      pSegTarget,                */
/*                                      PSZ      pTemp,                     */
/*                                      PTATAG   pTATAG,                    */
/*                                      PEQF_BOOL    pfNoUnseg)             */
/*    EQF_BOOL APIENTRY16 EQFPOSTUNSEG  (PSZ      pTagTable,                */
/*                                       PSZ      pEdit,                    */
/*                                       PSZ      pProgPath,                */
/*                                       PSZ      pTarget,                  */
/*                                       PTATAG   pTATAG)                   */
/*                                                                          */
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
#define MAX_PATH_SIZE      256
#define MAX_TEXT_SIZE     2048     

#define  CODEPAGE_ERROR                  "File Conversion Error"
#define  ICONV_CODEPAGE_ERROR            "Error occured during ICONV code page conversion"
#define  UNICODE_CODEPAGE_ERROR          "Error occured during code page conversion"

#define MAX_SEGMENT_LEN   1500                    /* Allow translation expansion */
#define  HTMLDOC_START_SCRIPT_STRING  L"<SCRIPT"
#define  HTMLDOC_END_SCRIPT_STRING    L"</SCRIPT"
#define  HTMLDOC_START_STYLE_STRING   L"<STYLE"
#define  HTMLDOC_END_STYLE_STRING     L"</STYLE"
#define  HTMLDOC_STYLE_CONT_STRING    L"</STYLE><STYLE ID=TWB>"
#define  HTMLDOC_SCRIPT_CONT_STRING1  L"µ<XSCRCONT:"
#define  HTMLDOC_SCRIPT_CONT_STRING2  L"{TWBSCR}<XSCRCONT:"
#define  HTMLDOC_SCRIPT_CONT_STRING3  L"<XSCRCONT:{TWBSCR}"
#define  HTMLDOC_SCRIPT_CONT_END      L"{TWBSCR}"
#define  HTMLDOC_SCRIPT_CONT_BEGIN    L"<XSCRCONT:"
#define  HTMLDOC_START_COMMENT_STRING L"<!--"
#define  HTMLDOC_END_COMMENT_STRING   L"-->"
#define  HTMLDOC_COMMENT_CONT_STRING  L"--><XCONT:"
#define  HTMLDOC_START_DOCTYPE_STRING L"<!DOCTYPE "
#define  HTMLDOC_2K_TAG               L"<TWB2K>"
#define  HTMLDOC_END_DOCTYPE_STRING   L">"
#define  HTMLDOC_JSON_START           L"<twbj--"
#define  HTMLDOC_JSON_END             L"--twbj>"
extern char * __cdecl getTemporaryFileName(char *nameBuf,short nameBufSize);

#ifndef _UNICODE
    #define TWBCNV_EOF                       '\x1A'
#else
    #define TWBCNV_EOF                       WEOF //'\x1A'  //'\x001A' //(wint_t)xFFFF
#endif


#ifndef CCHMAXPATHCOMP
     #define MAXPATHLEN 2024
     #define CCHMAXPATHCOMP MAXPATHLEN
#endif

#include <afxwin.h>
 #define DosCopy(File1, File2, BOOL)  CopyFileA(File1, File2, FALSE);



#include <ctype.h>
#include <sys\stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>                   
#include <locale.h>                  

#include "otmhtm32.h"
#include "scrptseg.h"
#include "entity.h"
#include "reseq.h"
#include "usrcalls.h"
#include "unicode.h"


using namespace mku ;                       


typedef struct tags {
    wchar_t start[15];
    wchar_t end[15];
} TAGS;

TAGS  *ptrsrchTags ;

TAGS srchTags[] = { 
    {  HTMLDOC_START_SCRIPT_STRING,HTMLDOC_END_SCRIPT_STRING},
    {  HTMLDOC_START_COMMENT_STRING,HTMLDOC_END_COMMENT_STRING},
    {  HTMLDOC_START_STYLE_STRING,HTMLDOC_END_STYLE_STRING},     
    {  L"<XCONT:",L"-->"},               
    {  L"[TWBSTART]",L"[TWBSTOP]"},                              
    {  L"",L""}
};

TAGS srchTags2[] = { 
    {  HTMLDOC_START_SCRIPT_STRING,HTMLDOC_END_SCRIPT_STRING},
    {  HTMLDOC_START_COMMENT_STRING,HTMLDOC_END_COMMENT_STRING},
    {  L"<XCONT:",L"-->"},  
    {  L"",L""}
};


extern   char    szDocTargetLanguage[80];   /* From USRCALLS.C  */
extern   char    szDocSourceLanguage[80];   /* From USRCALLS.C  */
extern   short   sTPVersion ;               /* From USRCALLS.C  */

long SearchStringListInFile(FILE **f_ptr,long startPos,long endPos,
                            wchar_t *untilStr,long *foundPos,short *list_idx);

void PostUnsegFont(char *InFile);
void UnsegSpecialTags(wchar_t *szIn, BOOL *SQuoted);
short chk_is_js_file(char *pathname, char *filename, char *markup, short Step );
long removeEndingOffendingChar(char *szTgt,wchar_t eofchar);
BOOL chk_is_inQuote( long tagPos, long st, FILE *fp );
BOOL ShowError( wchar_t *Title, wchar_t *Message, BOOL bOKCancel );




/****************************************************************************/
/*                                                                          */
/* EQFPRESEG                                                                */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*    Description:                                                          */
/*         called after text segmentation is invoked, to change the         */
/*         segmented source and target file before translation takes place. */
/*                                                                          */
/*    Arguments:   PSZ    ... pointer to markup table name.                 */
/*                                                                          */
/*                 PSZ    ... pointer to editor name.                       */
/*                                                                          */
/*                 PSZ    ... pointer to program path.                      */
/*                                                                          */
/*                 PSZ    ... pointer to segmented source file name.        */
/*                                                                          */
/*                 PSZ    ... pointer to the segmented target file name.    */
/*                                                                          */
/*                 PTATAG ... pointer to tags inserted by text segmentation,*/
/*                            ( see layout of tag structure ).              */
/*                                                                          */
/*    Return:      BOOL   ... TRUE:processing was OK.                       */
/*                            FALSE: an error occured during processing.    */
/*                                                                          */
/*    Note: It is vital that the name of the segmented source and target    */
/*          file is not changed!                                            */
/*                                                                          */
/*                                                                          */
/****************************************************************************/

__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPRESEG2(
                      PSZ      pTagTable,
                      PSZ      pEdit,
                      PSZ      pProgPath,
                      PSZ      pSource,
                      PSZ      pTempSource,
                      PEQF_BOOL    pfNoSegment,
                      HWND     hSlider,
                      PEQF_BOOL    pfKill)

{
    wchar_t  *cptr,*pszChr;
    char  szSrc[MAX_PATH_SIZE], pszBkSrcFile[512], pszWorkFile[512];
    wchar_t srchTag[50];
    FILE  *srcFp;
    short foundTag=0,i;
    long  srcPos = 0,startTagPos = 0,endTagPos = 0,rc,tagPos = -1;
    long  commentStartPos = 0, commentEndPos = 0;
    long  skipStartPos = 0, skipEndPos = 0;
    long  lFileSize ; 
    long  lastScriptStPos = 0, lastScriptEndPos = 0 ;
    float k_factor = 0.0;
    int   intk_factor = 0;
    wchar_t szErrTitle[128] ;
    wchar_t szErrText[4096] ;
    char szTempFile[512];
    char szMarkup[80];
    char *szAltTempExt1 = ".$$A";
    char *szAltTempExt2 = ".$$B";
    short sJS_FileType ;


    EQFSETSLIDER(hSlider, (0));   /* Initialize slider to zero */

    PrepDocLanguageInfo( pSource ) ;   /* Set language unqiue processing, like DBCS */
    if ( ! pEdit[0] )                  /* If ITM processing, make TARGET = SOURCE   */
       strcpy( szDocTargetLanguage, szDocSourceLanguage ) ;

    CreateTempFileName2( pszBkSrcFile, pSource, szAltTempExt1, TEMPNAME_SSOURCE ) ;

    strcpy(szSrc,pSource);
    DosCopy (szSrc,pszBkSrcFile,DCPY_EXISTING);

    CreateTempFileName2( pszWorkFile, pSource, szAltTempExt2, TEMPNAME_SSOURCE ) ;


    /* code page conversion routine */
    strcpy( szMarkup, pTagTable ) ;
    strupr( szMarkup ) ;
    if ( !strncmp(szMarkup, "OTMU", 4 ) ) {   
       DosCopy( pszBkSrcFile, pszWorkFile, DCPY_EXISTING ) ;
       rc = ConvertImport(pszWorkFile, pszBkSrcFile, EQF_UTF82UTF16 );
       if( !rc ) {
           MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
           return FALSE;
       }
       remove( pszWorkFile ) ;
    } else { //HTML files with ISO(HTML) input format.
       DosCopy( pszBkSrcFile, pszWorkFile, DCPY_EXISTING ) ;
       rc = ConvertImport(pszWorkFile, pszBkSrcFile, EQF_ANSI2UTF16);   // OpenTM2
       if( !rc ) {
           MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
           return FALSE;
       }
    }
    sJS_FileType = chk_is_js_file( pSource, pszBkSrcFile, szMarkup, 0);

    EntityTag(pszBkSrcFile, pszWorkFile);

    srcFp = fopen (pszBkSrcFile,"rb");

    srcPos = ftell(srcFp);

    if ( sJS_FileType == JS_TYPE_JSON ) {                             
       startTagPos = 0 ;
       fseek(srcFp,0L,SEEK_END);
       endTagPos= ftell(srcFp);
       fclose(srcFp);
       preSegJSON(pszBkSrcFile,pszWorkFile,&startTagPos,&endTagPos,sJS_FileType);

       *pfNoSegment = FALSE;
       strcpy(pTempSource,pszBkSrcFile);
       remove(pszWorkFile); 

       return(TRUE);
    }

// Get file length and calculate mult factor for TM/2 slider

    fseek(srcFp,0L,SEEK_END);
    intk_factor= ftell(srcFp);
    if ( intk_factor <= 0 ) {
        wcscpy( szErrText, L"File does not contain any information, 0 bytes in size.  Analysis terminated." ) ;
        wcscpy( szErrTitle, L"Invalid Source" ) ;
        ShowError( szErrTitle, szErrText, FALSE ) ;
        return(FALSE);
    }
    k_factor= 100.0/intk_factor;
    fseek(srcFp,srcPos,SEEK_CUR);

    memset(srchTag,'\0',sizeof(srchTag));
    ptrsrchTags = srchTags ; 
    for (rc = 0; ((rc >= 0) || (rc == -101)) ; ) {
        EQFSETSLIDER(hSlider, (int)(k_factor * srcPos)); /* Adjust slider by file pos * k_factor to get percent done */
        if (wcslen(srchTag) == 0) {
            rc = SearchStringListInFile(&srcFp,srcPos,-1,L"\n\r",&tagPos,&i);
        } else {
            if ( ( wcscmp(srchTag,HTMLDOC_END_SCRIPT_STRING) == 0 ) &&         
                 ( sJS_FileType ) ) {        /* Find last </SCRIPT> for JS file            */
               rc = StrSearchFile(&srcFp,    /* open file pointer                          */
                                  srchTag,   /* string to look for                         */
                                  -1,        /* Search backwards from end of file.         */
                                  srcPos,    /* The ending position < startPos if backward */
                                  L"",       /* a set of chars to indicated end of search  */
                                  &tagPos);  /* position the string was found in the file  */
            } else {
               rc = StrSearchFile(&srcFp,    /* open file pointer                          */
                                  srchTag,   /* string to look for                         */
                                  srcPos,    /* The starting position > endPos if backward */
                                  -1,        /* The ending position < startPos if backward */
                                  L"",        /* a set of chars to indicated end of search  */
                                  &tagPos);  /* position the string was found in the file  */
            }




            if (rc == -100) { /* there was no ending tag */


                rc = -101;
                srchTag[0] = '\0';
                foundTag = 0;
                fseek(srcFp,startTagPos+1*sizeof(wchar_t),SEEK_SET);
            }
        }

        if ((foundTag) && (tagPos >= 0)) {

            /* <SCRIPT> blah blah blah blah </SCRIPT>  */
            /*                                       */
            /* startTagPos                   endTagPos */
            endTagPos = tagPos + wcslen(srchTag)*sizeof(wchar_t);
            if (wcscmp(srchTag,HTMLDOC_END_SCRIPT_STRING) == 0) {

                if ( chk_is_inQuote( startTagPos, tagPos, srcFp ) ) {
                    fseek(srcFp,tagPos + 1*sizeof(wchar_t),SEEK_SET) ;
                    tagPos = -1 ;
                    srcPos = ftell(srcFp);
                    continue ;
                }
                wcscpy(srchTag, L">");
                srcPos = endTagPos;
                rc = StrSearchFile(&srcFp,    /* open file pointer                          */
                                   srchTag,   /* string to look for                         */
                                   srcPos,    /* The starting position > endPos if backward */
                                   -1,        /* The ending position < startPos if backward */
                                   L"",        /* a set of chars to indicated end of search  */
                                   &tagPos);  /* position the string was found in the file  */
                endTagPos = tagPos + 1*sizeof(wchar_t);
                lastScriptEndPos = endTagPos ;
                fclose (srcFp);              /* close to allow changes */
                skipStartPos = startTagPos;  /* Save the script start and end pos so we can check */
                skipEndPos = endTagPos;      /* if <-- is inside a <SCRIPT> section because we don't want to do the 2kseg fix for the comment. */
                                               /* since we are already doing it for the script.*/

                /* Check if <SCRIPT> tag is inside of a comment */
                if ((startTagPos < commentStartPos) || (startTagPos >= commentEndPos)) {  
                    preSegScriptTag(pszBkSrcFile,pszWorkFile,&startTagPos,&endTagPos);  /* YES then run script exit functions */
                    preSeg2kFix(pszBkSrcFile,pszWorkFile,&startTagPos,&endTagPos,       /* NO do nothing */
                                HTMLDOC_SCRIPT_CONT_STRING2);
                }
                openFileToPosition(pszBkSrcFile,startTagPos+1*sizeof(wchar_t),&srcFp);

            } else if (wcscmp(srchTag,HTMLDOC_END_COMMENT_STRING) == 0) {

                fclose (srcFp);              /* close to allow changes */
                commentStartPos = startTagPos;  /* Save the comment start and end pos so we can check */
                commentEndPos = endTagPos;      /* if <Script> is inside a comment. */
                if ((startTagPos < skipStartPos) || (startTagPos > skipEndPos)) {  /* Check if <SCRIPT> is inside a comment */
                    preSeg2kFix(pszBkSrcFile,pszWorkFile,&startTagPos,&endTagPos,
                                HTMLDOC_COMMENT_CONT_STRING);
                }
                openFileToPosition(pszBkSrcFile,startTagPos+1*sizeof(wchar_t),&srcFp);
            } else if (wcscmp(srchTag,HTMLDOC_END_STYLE_STRING) == 0) {
          
                fclose (srcFp);              /* close to allow changes */
                skipStartPos = startTagPos;  /* Save the style start and end pos so we can check */
                skipEndPos = endTagPos;      /* if <-- is inside a <STYLE> section because we don't want to do the 2kseg fix for the comment. */
                                             /* since we are already doing it for the script.*/

                /* Check if <STYLE> tag is inside of a comment */
                if ((startTagPos < commentStartPos) || (startTagPos >= commentEndPos)) { 
                   preSeg2kFix(pszBkSrcFile,pszWorkFile,&startTagPos,&endTagPos,      
                               HTMLDOC_STYLE_CONT_STRING);
                }
                openFileToPosition(pszBkSrcFile,startTagPos+1*sizeof(wchar_t),&srcFp);
            }

            srchTag[0] = '\0';
            foundTag = 0;
            tagPos = -1;
        } else if ((!foundTag) && (tagPos >= 0)) {

            if ( ( lastScriptStPos < tagPos ) && ( lastScriptEndPos > tagPos ) && 
                 ( wcscmp(srchTags[i].start,HTMLDOC_START_SCRIPT_STRING) == 0 ) ) {
                srchTag[0] = '\0';
                foundTag = 0;
                fseek(srcFp,tagPos + 1*sizeof(wchar_t),SEEK_SET);
                tagPos = -1;
            } else {
                if (wcscmp(srchTags[i].start,HTMLDOC_START_SCRIPT_STRING) == 0) {
                    lastScriptStPos = tagPos ;
                }
            foundTag = 1;
            wcscpy (srchTag,srchTags[i].end);
            startTagPos = tagPos;
            fseek(srcFp,startTagPos + 1*sizeof(wchar_t),SEEK_SET);
            tagPos = -1;
        }
        }

        srcPos = ftell(srcFp);
    }

    pTagTable;pEdit;pProgPath;pSource;pTempSource;pfNoSegment;

    *pfNoSegment = FALSE;

    strcpy(pTempSource,pszBkSrcFile);
    remove(pszWorkFile); 

    fclose (srcFp);

    EQFSETSLIDER(hSlider, (100)); /* all done so set slider to 100% */
    return(TRUE);

}


__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTSEGW(
                       PSZ pTagTable,
                       PSZ       pEdit,
                       PSZ       pProgPath,
                       PSZ       pSegSource,
                       PSZ       pSegTarget,
                       PTATAG_W  pTATAG,
                       HWND      hSlider,
                       PEQF_BOOL pfKill )

{

    char  szSrc[MAX_PATH_SIZE], szTgt[MAX_PATH_SIZE];
    char  pszBkSrcFile[512];
    char  pszWorkFile[512];
    wchar_t  *ScriptStart = L"<SCRIPT";
    wchar_t  *ScriptEnd = L"</SCRIPT";
    wchar_t srchTag[50];
    char   szMarkup[80];
    FILE   *srcFp;
    short  i;
    long   srcPos = 0,startTagPos = 0,endTagPos = 0,rc,tagPos = -1;
    BOOL   scriptMod = FALSE;
    BOOL   bResequence = FALSE;
    short  sJS_FileType ;
    float  k_factor = 0.0;
    char *szAltTempExt3 = ".$$C";
    char *szAltTempExt4 = ".$$D";


    EQFSETSLIDER(hSlider, (0)); /* Initialize slider to zero */

    PrepDocLanguageInfo( pSegSource ) ;   /* Set language unqiue processing, like DBCS */


    CreateTempFileName2( pszBkSrcFile, pSegSource, szAltTempExt3, TEMPNAME_SSOURCE ) ;

    strcpy(szSrc,pSegSource);
    strcpy(szTgt,pSegTarget);
    DosCopy (szSrc,pszBkSrcFile,DCPY_EXISTING);

    strcpy( szMarkup, pTagTable ) ;
    strupr( szMarkup ) ;
    sJS_FileType = chk_is_js_file(pSegSource,pszBkSrcFile, szMarkup, 1);


    CreateTempFileName2( pszWorkFile, pSegSource, szAltTempExt4, TEMPNAME_SSOURCE ) ;


    PostEntityTag(pszBkSrcFile, pszWorkFile, &bResequence );

    srcFp = fopen (pszBkSrcFile,"rb");

    // Get file length and calculate mult factor for TM/2 slider
    srcPos = ftell(srcFp);
    fseek(srcFp,0L,SEEK_END);
    k_factor= 100.0/ftell(srcFp);
    fseek(srcFp,srcPos,SEEK_CUR);

    rc = 0;


    rc = StrSearchFile(&srcFp,         /* open file pointer                          */  
                       ScriptStart,    /* string to look for                         */
                       srcPos,         /* The starting position > endPos if backward */
                       -1,             /* The ending position < startPos if backward */
                       L"",            /* a set of chars to indicated end of search  */
                       &tagPos);       /* position the string was found in the file  */
    if ( rc == 0 ) {                   /* If file contains 1+ <SCRIPT> sections      */
       fseek(srcFp,0L,SEEK_SET);
       srcPos = ftell(srcFp);
       ptrsrchTags = srchTags2 ; 
       tagPos = -1 ;
       i = 0 ;
       while (rc == 0) {
          EQFSETSLIDER(hSlider, (int)(k_factor * srcPos)); /* Adjust slider by file pos * k_factor to get percent done */

          tagPos = -1;
          rc = SearchStringListInFile(&srcFp,srcPos,-1,L"",&tagPos,&i);  /* Find next token in file */
          if (rc == 0 &&  tagPos >= 0 ) {                                /* If found token in file  */
             wcscpy (srchTag,ptrsrchTags[i].end);
             startTagPos = tagPos;
             fseek(srcFp,startTagPos + 1*sizeof(wchar_t),SEEK_SET);
             tagPos = -1;

             /* Look for end of that token */
             if ( ( wcscmp(srchTag,HTMLDOC_END_SCRIPT_STRING) == 0 ) &&         
                  ( sJS_FileType ) ) {        /* Find last </SCRIPT> for JS file            */
                rc = StrSearchFile(&srcFp,    /* open file pointer                          */
                                   srchTag,   /* string to look for                         */
                                   -1,        /* Search backwards from end of file          */
                                   srcPos,    /* The ending position < startPos if backward */
                                   L"",        /* a set of chars to indicated end of search */
                                   &tagPos);  /* position the string was found in the file  */
             } else {
                rc = StrSearchFile(&srcFp,         /* open file pointer                          */
                                   srchTag,        /* string to look for                         */
                                   srcPos,         /* The starting position > endPos if backward */
                                   -1,             /* The ending position < startPos if backward */
                                   L"",            /* a set of chars to indicated end of search  */
                                   &tagPos);       /* position the string was found in the file  */
             }
             if (rc == 0) {
                if (wcscmp(srchTag,HTMLDOC_END_COMMENT_STRING) == 0) { /* If comment, skip it */
                   srcPos =  tagPos + (wcslen(srchTag)-1)*sizeof(wchar_t);
                   fseek(srcFp,srcPos,SEEK_SET);
                } else {                                       /* If <SCRIPT>, handle section */
                   endTagPos = tagPos - 1*sizeof(wchar_t);
                   fclose (srcFp);
                   postSegScriptTag(pszBkSrcFile,pszWorkFile,&startTagPos,&endTagPos,sJS_FileType);
                   scriptMod = TRUE;
                   srcFp = fopen (pszBkSrcFile,"rb");
                   srcPos =  endTagPos+wcslen(ScriptEnd)*sizeof(wchar_t);
                   fseek(srcFp,srcPos,SEEK_SET);
                }
             } else {
                srcPos = ftell(srcFp);
             }
          } else {
             srcPos = ftell(srcFp);
          }
       }
    }




    fclose (srcFp);

    if (scriptMod||bResequence) {
        resequence_TM2(pszBkSrcFile);
    }
    
    PostSegScriptTagChar(pszBkSrcFile, pszWorkFile); 

    DosCopy (pszBkSrcFile,szSrc,DCPY_EXISTING);
    DosCopy (pszBkSrcFile,szTgt,DCPY_EXISTING);

    remove( pszBkSrcFile ) ;
    remove( pszWorkFile ) ;

    EQFSETSLIDER(hSlider, (100)); /* all done so set slider to 100% */
    return(TRUE);

}

__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPREUNSEGW  (PSZ      pTagTable,
                       PSZ       pEdit,
                       PSZ       pProgPath,
                       PSZ       pSegTarget,
                       PSZ       pTemp,
                       PTATAG_W  pTATAG,
                       PEQF_BOOL pfNoUnseg,
                       PEQF_BOOL pfKill )

{


    pTagTable;pEdit;pProgPath;pSegTarget;pTATAG;

    return(TRUE);

}

__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTUNSEGW(
                        PSZ      pTagTable,
                        PSZ      pEdit,
                        PSZ      pProgPath,
                        PSZ      pTarget,
                        PTATAG   pTATAG,
                        PEQF_BOOL  pfKill )

{
    char  *cptr,*pszChr;
    char  szTgt[MAX_PATH_SIZE],pszWorkFile[512];
    wchar_t srchTag[50];
    FILE  *tgtFp=NULL, *tmpFp=NULL ;
    short foundTag=0,i;
    long  tgtPos = 0,startTagPos = 0,endTagPos = 0,rc,tagPos = -1;
    char   szMarkup[80];
    char  szErrTitle[128] ;
    char  *szAltTempExt5 = ".$$E";
    short  sJS_FileType ;

    pTagTable;pEdit;pProgPath;pTarget;pTATAG;

    PrepDocLanguageInfo( pTarget ) ;   /* Set language unqiue processing, like DBCS */
    strcpy( szMarkup, pTagTable ) ;
    strupr( szMarkup ) ;

    strcpy(szTgt,pTarget);

    CreateTempFileName2( pszWorkFile, pTarget, szAltTempExt5, TEMPNAME_TARGET ) ;

    tgtFp = fopen (szTgt,"rb");

    tgtPos = ftell(tgtFp);
    memset(srchTag,'\0',sizeof(srchTag));
    ptrsrchTags = srchTags ; 
    for (rc = 0; ((rc >= 0) || (rc == -101)) ; ) {
        if (wcslen(srchTag) == 0) {
            rc = SearchStringListInFile(&tgtFp,tgtPos,-1,L"\n\r",&tagPos,&i);
        } else {
            rc = StrSearchFile(&tgtFp,    /* open file pointer                          */
                               srchTag,   /* string to look for                         */
                               tgtPos,    /* The starting position > endPos if backward */
                               -1,        /* The ending position < startPos if backward */
                               L"",        /* a set of chars to indicated end of search  */
                               &tagPos);  /* position the string was found in the file  */
            if (rc == -100) { /* there was no ending tag */
                rc = -101;
                srchTag[0] = '\0';
                foundTag = 0;
                fseek(tgtFp,startTagPos+1*sizeof(wchar_t),SEEK_SET);
            }
        }
        if ((foundTag) && (tagPos >= 0)) {
            /* <SCRIPT> blah blah blah blah </SCRIPT>  */
            /*                                       */
            /* startTagPos                   endTagPos */
            endTagPos = tagPos + wcslen(srchTag)*sizeof(wchar_t);
            if (wcscmp(srchTag,HTMLDOC_END_SCRIPT_STRING) == 0) {
                wcscpy(srchTag, L">");
                tgtPos = endTagPos;
                rc = StrSearchFile(&tgtFp,    /* open file pointer                          */
                                   srchTag,   /* string to look for                         */
                                   tgtPos,    /* The starting position > endPos if backward */
                                   -1,        /* The ending position < startPos if backward */
                                   L"",        /* a set of chars to indicated end of search  */
                                   &tagPos);  /* position the string was found in the file  */
                endTagPos = tagPos + 1*sizeof(wchar_t);
                fclose (tgtFp);              /* close to allow changes */
                postUnSegScriptTag(szTgt,pszWorkFile,&startTagPos,&endTagPos);
                postUnSeg2kFix(szTgt,pszWorkFile,&startTagPos,&endTagPos,
                               HTMLDOC_SCRIPT_CONT_STRING1,HTMLDOC_SCRIPT_CONT_STRING2,
                               HTMLDOC_END_SCRIPT_STRING);
                openFileToPosition(szTgt,startTagPos+1*sizeof(wchar_t),&tgtFp);
            } else if (wcscmp(srchTag,HTMLDOC_END_COMMENT_STRING) == 0) {
                fclose (tgtFp);
                postUnSeg2kFix(szTgt,pszWorkFile,&startTagPos,&endTagPos,
                               HTMLDOC_COMMENT_CONT_STRING,NULL,
                               HTMLDOC_END_COMMENT_STRING);
                openFileToPosition(szTgt,startTagPos+1*sizeof(wchar_t),&tgtFp);
            }
            srchTag[0] = '\0';
            foundTag = 0;
            tagPos = -1;
        } else if ((!foundTag) && (tagPos >= 0)) {
            foundTag = 1;
            wcscpy (srchTag,srchTags[i].end);
            startTagPos = tagPos;
            fseek(tgtFp,startTagPos + 1*sizeof(wchar_t),SEEK_SET);
            tagPos = -1;

            /*  If found "--><XCONT:" tag, then remove it and do not look */
            /*  for an end tag.                                           */
            if ( ! wcscmp(srchTags[i].start,L"<XCONT:") ) {  
               fclose (tgtFp);
               tgtFp = NULL ;
               if ( tmpFp != NULL) {
                  fclose(tmpFp) ;
                  tmpFp = NULL ;
               }
               copyPartialFile(szTgt, &tgtFp,
                               pszWorkFile, &tmpFp,
                               0,                    /* begining */
                               startTagPos-3*sizeof(wchar_t),
                               1 );                  /* create/overwrite file */
               fseek(tgtFp,0,SEEK_END);
               endTagPos = ftell(tgtFp) - startTagPos - 6*sizeof(wchar_t) ;            /* last position in the file */
               copyPartialFile(szTgt, &tgtFp,
                               pszWorkFile, &tmpFp,
                               startTagPos + 7*sizeof(wchar_t),
                               endTagPos,
                               0 );
               if ( tgtFp ) {
                  fclose( tgtFp );
                  tgtFp = NULL ;
               }
               if ( tmpFp ) {
                  fclose( tmpFp );
                  tmpFp = NULL ;
               }
               DosCopy (pszWorkFile,szTgt,DCPY_EXISTING);
               remove (pszWorkFile);
               foundTag = 0 ;
               srchTag[0] = 0 ;
               startTagPos -= 3*sizeof(wchar_t);
               openFileToPosition(szTgt,startTagPos,&tgtFp);
               tgtPos = ftell(tgtFp);
            }
        }

        tgtPos = ftell(tgtFp);
    }

    if (tgtFp) fclose (tgtFp);

    PostUnsegFont( pTarget ) ;     /* Correct neutral <FONT> tags */

    sJS_FileType = chk_is_js_file(pTarget,pTarget,szMarkup,2);
    removeEndingOffendingChar(szTgt,TWBCNV_EOF);
    /* code page conversion routine */
    strcpy( szErrTitle, pTagTable ) ;
    strupr( szErrTitle ) ;
    if ( ( ! strncmp( szErrTitle, "OTMU",   4 ) ) ||
         ( ! strcmp( szDocTargetLanguage, "Kazakh" ) ) ) { /* Kazakh always is UTF-8 */
       rc = ConvertExport(szTgt, EQF_UTF162UTF8);
       if( !rc ) {
           MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
           return FALSE;
       }
        i = 0 ;  
    } else { //HTML files coded in ISO, plus JSP files that uses markup OTMJSPHT 
       if ( IsDBCS( '\x90') ) {   
          rc = ConvertExport(szTgt, EQF_UTF162ASCII);
          if( !rc ) {
              MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
              return FALSE;
          }
          i=0;       
       } else {
///       rc = ConvertExport(szTgt, EQF_UTF162UTF8);
          rc = ConvertExport(szTgt, EQF_UTF162ANSI);     // OpenTM2
          if( !rc ) {
              MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
              return FALSE;
          }
          i=0;       
       }

    }

    remove (pszWorkFile);

    return((EQF_BOOL) !i);

}




/*******************************************************************************
*
*       function:       EQFSHOW
*
* -----------------------------------------------------------------------------
*       Description:
*               test and show the translation of some segments
*       Parameters:
*               LONG          // info needed
*               HWND          // handle
*       Return:
*               EQF_BOOL
*******************************************************************************/

__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFSHOW(
                   LONG     lInfo,
                   HWND     hwndParent)
{
  FILE     *fTemp ;
  char     szWorkFile[512] ;
  char      szTemp[512] ;
  wchar_t   *swTemp ;   
  wchar_t   *ptr ;
  USHORT   usBufSize ;
  ULONG    ulSegNum = 1;
  USHORT   usRc = 0;
  wchar_t  szTmp[512];
  char *szAltTempName = "\\otmhtm32.$$$";
  BOOL     bInSQuotedText = FALSE ;

#define    WM_EQF_SHOWHTML         2131


  CreateTempFileName(szTemp, szAltTempName);
  sprintf( szWorkFile, "%s.htm", szTemp ) ;

  swTemp = (wchar_t*)malloc( (EQF_SEGLEN+1)*sizeof(wchar_t) ) ;

  fTemp = fopen( szWorkFile, "wb" ) ;
  swprintf(szTmp, L"%c", BYTE_ORDER_MARK);
  fputws( szTmp, fTemp ) ;

  while ( !usRc ) {
     usBufSize = (EQF_SEGLEN+1)*sizeof(wchar_t);
     swTemp[0] = 0 ;

     usRc = EQFGETNEXTSEGW( lInfo, &ulSegNum, swTemp, &usBufSize );
     if ( !usRc ) { 
        ptr = wcsstr( swTemp, L"{TWB" ) ;
        if ( ptr ) {
           if ( ( ! wcsncmp( ptr, L"{TWBSCR}", 8 ) ) ||
                ( ! wcsncmp( ptr, L"{TWBENT}", 8 ) ) ) {
              wmemmove( ptr, ptr+8, wcslen(ptr+8)+1 ) ; 
           }
        }
        UnsegSpecialTags( swTemp, &bInSQuotedText ) ;
        fputws( swTemp, fTemp ) ;
     }
  } 
  fclose( fTemp ) ;

  SendMessage( GetParent(hwndParent), 
               WM_EQF_SHOWHTML, 
               (WPARAM)"Translation Preview",
               (LPARAM)szWorkFile );
  
  remove( szWorkFile ) ;
  free( swTemp ) ;

  return( usRc == 0 ) ;
}


/*******************************************************************************
*
*       function:       EQFQUERYEXITINFO
*
* -----------------------------------------------------------------------------
*       Description:
*               Determine the files which are required for this markup table.
*       Parameters:
*               PSZ           // name of the markup table, e.g. "OTMHTM32"
*               USHORT        // type of information being queried
*               PSZ           // buffer area receiving the information returned by the exit
*               USHORT        // length of buffer area
*       Return:
*               EQF_BOOL      // 0=Successful
*******************************************************************************/

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFQUERYEXITINFO(
                        PSZ pszTagTable, // name of the markup table, e.g. "OTMHTM32"
                        USHORT usMode,   // type of information being queried
                        PSZ pszBuffer,   // buffer area receiving the information returned by the exit
                        USHORT usBufLen  // length of buffer area
)
{
    
    if( usMode == QUERYEXIT_ADDFILES) {
        QueryExportFiles(pszTagTable, pszBuffer, usBufLen, FALSE);
    }

    return 0;
}


/****************************************************************************/
/*                                                                          */
/* SearchStringListInFile                                                   */
/*                                                                          */
/* Finds the first occurance of one string in a list of strings in a file.  */
/*                                                                          */
/* Returns: -1 = Error; -100 = EOF (not found); -101 = EOL (not found)      */
/*                                                                          */
/****************************************************************************/

long SearchStringListInFile(FILE **f_ptr,long startPos,long endPos,
                            wchar_t *untilStr,long *foundPos,short *list_idx)
{
    wchar_t    chrString[2],*tStr;
    long    curPos ;
    long    fndPos = -1 ;
    long    savePos = -1 ;
    long    resetPos = -1 ;
    long    rc = 0 ;
    short   i,j,k ;
    short   saveIdx = 0;


    /* check for any more search strings */

    for (i=0,curPos=-1; (fndPos == -1) && (wcslen(ptrsrchTags[i].start)); ++i) {
        chrString[0] = ptrsrchTags[i].start[0];  /* search for first character...first */
        chrString[1] = '\0';                  /* null terminator */
        /* search for first char of first string */
        rc = StrSearchFile(f_ptr,chrString,startPos,endPos,untilStr,&curPos);
        StrUpr (chrString);
        if ((rc == 0) && ((curPos < savePos) || (savePos == -1))) {
            if ( ( resetPos == -1 ) ||                         
                 ( curPos < resetPos ) ) 
               resetPos = curPos ; 

            /* match the rest of the word of search string */
            tStr = (wchar_t *) calloc (1,(wcslen(ptrsrchTags[i].start)+1)*sizeof(wchar_t));

            for( j=0, k=wcslen(ptrsrchTags[i].start) ;         
                 j<k && (j==0 || !wcschr(untilStr,tStr[j-1]) ) ;
                 tStr[j++]=fgetwc(*f_ptr) );
            StrUpr (tStr);
            if (!wcsncmp(tStr,(ptrsrchTags[i].start)+1, wcslen(ptrsrchTags[i].start)-1 ) ) {
               if ( ( ptrsrchTags[i].start[0] != '<'     ) ||  
                    ( ! iswalpha(ptrsrchTags[i].start[1]) ) ||
                    ( iswspace( tStr[wcslen(ptrsrchTags[i].start)-1] ) ) ||
                    ( tStr[wcslen(ptrsrchTags[i].start)-1] == '>' ) ) {
                  savePos = curPos;
                  saveIdx = i;
                  if (savePos == startPos) {  /* means there is no possible way to be before startPos */
                      fndPos = savePos;       /* allows us to stop looping */
                  }
               }
            } 
            free (tStr);
        }
    }

    if (savePos >= 0) {
        *foundPos = savePos;
        *list_idx = saveIdx;
        rc = 0;
    } else {
        /* let rc be...it will be either -100 EOF or -101 EOL */
       if ( resetPos > 1 ) {                
          fseek( *f_ptr, resetPos+1*sizeof(wchar_t), SEEK_SET ) ;
       }
    }
    return(rc);
}

/****************************************************************************/
/*                                                                          */
/* preSeg2kFix                                                              */
/*                                                                          */
/* Inserts the 2k fix extension characters.                                 */
/*                                                                          */
/* Returns: 0                                                               */
/*                                                                          */
/****************************************************************************/

short preSeg2kFix(char *srcFile,char *tmpFile,long *tagStartPos,long *tagEndPos,
                  wchar_t *ContString)
{
    FILE      *tmpfp,*srcfp, *mjc;
    wchar_t   szTemp[128] ;
    wchar_t   szTagName[80] ;
    wchar_t   c ;
    wchar_t   *cptr, *cptr2;
    long      tagLen ;
    long      fileLen ;
    long      i, j, k ;
    long      rc=0 ;
    long      begOffs ;
    long      AltSplit ;
    long      pos ;
    long      closeQuotePos=0 ;
    long      openQuotePos=0 ;
    long      quote_cnt=0 ;
    short     fileChanged;
    int       goodSpot = 0;
    BOOL      bInScript ;
    BOOL      bSkip2k ;

    static wchar_t  TAGLIST2_NEUTRAL[150] =
    L" A ABBR ACRONYM B BDO BIG BLINK CITE CODE DFN EM I ILAYER IMG KBD Q S SAMP SMALL SPAN STRIKE STRONG SUB SUP TT U VAR " ;

    tmpfp = srcfp = NULL;


    if (((*tagEndPos) - (*tagStartPos)) >= MAX_SEGMENT_LEN*sizeof(wchar_t)) {

        copyPartialFile(srcFile, &srcfp,
                        tmpFile, &tmpfp,
                        0,                    /* beginning */
                        *tagStartPos,
                        1 );                  /* create/overwrite file */

        if ((tmpfp != NULL) && (srcfp != NULL)) {
            tagLen = (*tagEndPos) - (*tagStartPos);
            if ( ! wcscmp( ContString, HTMLDOC_SCRIPT_CONT_STRING2 ) ) 
               bInScript = TRUE ;
            else 
               bInScript = FALSE ;
            bSkip2k = FALSE ;

            for (begOffs = (*tagStartPos);(begOffs < (*tagEndPos)) && rc == 0; ) {
                if (((*tagEndPos) - begOffs) > MAX_SEGMENT_LEN*sizeof(wchar_t)) {  /* need to insert comment continuation chars */
                    AltSplit = 0 ;
                    i = begOffs + MAX_SEGMENT_LEN*sizeof(wchar_t) - wcslen(ContString)*sizeof(wchar_t);
                    for (c = 'a' ; ((i > begOffs) && (c != '\n')); i -= sizeof(wchar_t)) {     
                        /* finding a place to break */
                        fseek (srcfp,i,SEEK_SET);
                        c = fgetwc (srcfp);

                        /* if double byte char skip next char  an look next for non DB char*/
                        if (IsDBCS(c) != 0) {
                            while (((IsDBCS(c) != 0) && (i > closeQuotePos))) {
                                c = fgetwc (srcfp); /* skip this char */
                                i -= sizeof(wchar_t);
                                c = fgetwc (srcfp); /* check this char */
                                i -= sizeof(wchar_t);
                            }
                        }
                        if ( bInScript ) {
                           if ( c == '{' ) {
                              wcscpy( szTemp, HTMLDOC_SCRIPT_CONT_END ) ;
                              for( j=0 ; 
                                   j<wcslen(szTemp) && c==szTemp[j]; 
                                   ++j, c=fgetwc(srcfp) ) ; 
                              if ( j == wcslen(szTemp) ) {
                                 i = begOffs ;
                                 bSkip2k = TRUE ;
                                 break ;
                              }
                           } else
                           if ( c == '<' ) {
//                            wcscpy( szTemp, HTMLDOC_SCRIPT_CONT_BEGIN ) ;
//                            for( j=0 ; 
//                                 j<wcslen(szTemp) && c==szTemp[j]; 
//                                 ++j, c=fgetwc(srcfp) ) ; 
//                            if ( j == wcslen(szTemp) ) {
//                               i +=  j*sizeof(wchar_t) ;
                              k = wcslen( HTMLDOC_SCRIPT_CONT_BEGIN ) ;          /* Longest tag to read */
                              for( j=0 ; j<k ; j++ ) {                          /* Read chars for tag   */
                                 szTemp[j] = c ;
                                 c = fgetwc(srcfp) ;
                              }
                              k = wcslen( HTMLDOC_SCRIPT_CONT_BEGIN ) ; 
                              if ( ! wcsncmp( szTemp, HTMLDOC_SCRIPT_CONT_BEGIN, k ) ) {
                                 i += k * sizeof(wchar_t) ;
                                 bSkip2k = FALSE ;
                                 break ;
                              }
                              k = wcslen( HTMLDOC_2K_TAG ) ;                  
                              if ( ! wcsncmp( szTemp, HTMLDOC_2K_TAG, k ) ) {
                                 bSkip2k = FALSE ;
                                 break ;
                              }
                              if ( ( iswalpha( szTemp[1] ) ) ||               
                                   ( ( szTemp[1] == '/'      ) &&
                                     ( iswalpha( szTemp[2] ) ) ) ||
                                   ( ( szTemp[1] == '\\'     ) &&          
                                     ( szTemp[2] == '/'      ) &&
                                     ( iswalpha( szTemp[3] ) ) ) ) { 
                                 if ( szTemp[1] == '/' ) 
                                    wcsncpy( szTagName, &szTemp[2], sizeof(szTagName)/sizeof(wchar_t) ) ;
                                 else
                                 if ( szTemp[1] == '\\' ) 
                                    wcsncpy( szTagName, &szTemp[3], sizeof(szTagName)/sizeof(wchar_t) ) ;
                                 else
                                    wcsncpy( szTagName, &szTemp[1], sizeof(szTagName)/sizeof(wchar_t) ) ;
                                 szTagName[sizeof(szTagName)/sizeof(wchar_t)-1] = 0 ;
                                 wcstok( szTagName, L" >\n\t\r" ) ;
                                 wcsupr( szTagName ) ;
                                 swprintf( szTemp, L" %s ", szTagName ) ;
                                 if ( ! wcsstr( TAGLIST2_NEUTRAL , szTemp ) ) { /* Next tag is a break */
                                    AltSplit = i ;
                                 }
                              }
                           } else
                           if ( c == '\n' ) {
                              j = i - 14*sizeof(wchar_t) ;
                              if ( j > ( begOffs + 1*sizeof(wchar_t) ) ) {
                                 fseek( srcfp, j, SEEK_SET ) ;
                                 fgetws( szTemp, 14+1, srcfp ) ;
                                 if ( ! wcsncmp( szTemp, L"<TWB2K>{TWB2K}", 14 ) ) {
                                    i = j + 1*sizeof(wchar_t) ;
                                 }
                              } else
                              if ( j == begOffs ) {
                                 bSkip2k = TRUE ;
                              }
                           }
                        }
                    }
                    if ( ( i == begOffs ) &&                 /* If split at break tag is available */
                         ( AltSplit > begOffs ) ) {                                  
                       i = AltSplit ;
                    }
                    if (i > begOffs) {
                        copyPartialFile(srcFile, &srcfp,
                                        tmpFile, &tmpfp,              
                                        begOffs,              /* beginning */
                                        i - begOffs,
                                        0 );                  /* create/overwrite file */
                        begOffs = i;
                    } else {  /* break at max segment...no whitespace found */
                        copyPartialFile(srcFile, &srcfp,
                                        tmpFile, &tmpfp,
                                        begOffs,              /* beginning */
                                        MAX_SEGMENT_LEN*sizeof(wchar_t) - wcslen(ContString)*sizeof(wchar_t),
                                        0 );                  /* create/overwrite file */
                        begOffs = begOffs + MAX_SEGMENT_LEN*sizeof(wchar_t) - wcslen(ContString)*sizeof(wchar_t);
                    }
                    if ( ! bSkip2k ) {
                       fputws (ContString,tmpfp);
                    }
                    bSkip2k = FALSE ;    /* Reset, since CONT_BEGIN was last tag  */



                } else {  /* all done write the rest of the tag to the file */
                    copyPartialFile(srcFile, &srcfp,
                                    tmpFile, &tmpfp,
                                    begOffs,              /* beggining */
                                    (*tagEndPos) - begOffs,
                                    0 );                  /* create/overwrite file */
                    begOffs = (*tagEndPos);
                }
            }

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
    }

    return(0);
}

/****************************************************************************/
/*                                                                          */
/* postUnSeg2kFix                                                           */
/*                                                                          */
/* Removes the 2k fix extension characters.                                 */
/*                                                                          */
/* Returns: 0                                                               */
/*                                                                          */
/****************************************************************************/

short postUnSeg2kFix(char *srcFile,char *tmpFile,long *tagStartPos,long *tagEndPos,
                     wchar_t *ContString1,wchar_t *ContString2,wchar_t *tagEndString)
{
    FILE *tmpfp,*srcfp;
    long fileLen,pos,rc,contPos;
    wchar_t *cptr,*cptr2;
    short fileChanged = 0;
    short sContStringLen;

    tmpfp = srcfp = NULL;

    if (wcslen(ContString1) > wcslen(tagEndString)) {
        if (!wcsncmp(ContString1,tagEndString,wcslen(tagEndString))) {  /* we may not have the last end tag position because of continuation */
            srcfp = fopen (srcFile,"rb");
            rc = StrSearchFile(&srcfp,ContString1,(*tagEndPos) - wcslen(tagEndString)*sizeof(wchar_t),
                               (*tagEndPos) + wcslen(ContString1)*sizeof(wchar_t),L"",&contPos);
            while ((rc == 0) && (contPos == ((*tagEndPos) - wcslen(tagEndString)*sizeof(wchar_t)))) {
                rc = StrSearchFile(&srcfp,tagEndString,(*tagEndPos),-1,L"",&pos);
                if (rc == 0) {
                    *tagEndPos = pos + wcslen(tagEndString)*sizeof(wchar_t);
                    rc = StrSearchFile(&srcfp,ContString1,(*tagEndPos) - wcslen(tagEndString)*sizeof(wchar_t),
                                       (*tagEndPos) + wcslen(ContString1)*sizeof(wchar_t),L"",&contPos);
                }
            }
        }
    }

    if (((*tagEndPos) - (*tagStartPos)) >= MAX_SEGMENT_LEN*sizeof(wchar_t)) {
        copyPartialFile(srcFile, &srcfp,
                        tmpFile, &tmpfp,
                        0,                    /* begining */
                        *tagStartPos,
                        1 );                  /* create/overwrite file */

        if ((tmpfp != NULL) && (srcfp != NULL)) {
            for (pos = *tagStartPos,rc = 0; (rc == 0) && (pos < (*tagEndPos)); ) {
                sContStringLen = wcslen(ContString1) ;
                rc = StrSearchFile (&srcfp,           /* open file pointer               */
                                    ContString1,      /* string to look for              */
                                    pos,              /* The starting position           */
                                    *tagEndPos,       /* The ending position             */
                                    L"",               /* Search until found or endTagPos */
                                    &contPos);
                if ( ( rc!= 0 ) &&
                     ( ContString2 ) ) {
                   sContStringLen = wcslen(ContString2) ;
                   rc = StrSearchFile (&srcfp,           /* open file pointer               */
                                       ContString2,      /* string to look for              */
                                       pos,              /* The starting position           */
                                       *tagEndPos,       /* The ending position             */
                                       L"",               /* Search until found or endTagPos */
                                       &contPos);
                }
                if (rc == 0) {  /* found one so lets remove it */
                    copyPartialFile(srcFile, &srcfp,
                                    tmpFile, &tmpfp,
                                    pos,
                                    contPos - pos,
                                    0 );                  /* create/overwrite file */
                    pos = contPos + sContStringLen*sizeof(wchar_t);
                }
            }
            fseek(srcfp,0,SEEK_END);
            fileLen = ftell(srcfp);            /* last position in the file */

            copyPartialFile(srcFile, &srcfp,
                            tmpFile, &tmpfp,
                            pos,
                            fileLen,
                            0 );                  /* create/overwrite file */
        }


        if (tmpfp != NULL) {
            rc = fclose (tmpfp);
            tmpfp = NULL;
        }
        if (srcfp != NULL) {
            rc = fclose (srcfp);
            srcfp = NULL;
        }


        rc = DosCopy (tmpFile,srcFile,DCPY_EXISTING);
        remove (tmpFile);
    } else {
        if (srcfp == NULL) {
            openFileToPosition(srcFile,*tagStartPos,&srcfp);
        }

        if (srcfp != NULL) {
            rc = StrSearchFile(&srcfp,HTMLDOC_START_DOCTYPE_STRING,*tagStartPos,
                               (*tagStartPos) + wcslen(HTMLDOC_START_DOCTYPE_STRING)*sizeof(wchar_t),
                               L"",&pos);

            fileChanged = 0;
            if ((rc == 0) && (pos == (*tagStartPos))) { /* found doctype tag */
                copyPartialFile (srcFile, &srcfp,
                                 tmpFile, &tmpfp,
                                 0, *tagStartPos,
                                 1 );              /* create the file */

                rc = StrSearchFile(&srcfp,HTMLDOC_END_COMMENT_STRING,*tagEndPos,-1,L"\n\r",&contPos);
                cptr = (wchar_t *) calloc (1,((contPos - pos) + 1)*sizeof(wchar_t));
                fseek(srcfp,*tagStartPos,SEEK_SET);
                fread (cptr,1*sizeof(wchar_t),(contPos - pos)/sizeof(wchar_t),srcfp);
                if ((cptr2 = wcsstr(&cptr[1],HTMLDOC_END_COMMENT_STRING)) != NULL) {  /* found first '>' */
                    strdel (cptr2,0,wcslen(HTMLDOC_END_COMMENT_STRING));
                    if ((cptr2 = wcsstr(&cptr[1],HTMLDOC_START_COMMENT_STRING)) != NULL) {
                        strdel (cptr2,0,wcslen(HTMLDOC_START_COMMENT_STRING));
                        fputws(cptr,tmpfp);

                        fseek(srcfp,0,SEEK_END);
                        fileLen = ftell(srcfp);            /* last position in the file */

                        copyPartialFile (srcFile, &srcfp,
                                         tmpFile, &tmpfp,
                                         contPos, fileLen,
                                         0 );              /* append to the file */
                        fileChanged = 1;
                    }
                }
                free (cptr);
            }

            if (tmpfp != NULL) {
                rc = fclose (tmpfp);
                tmpfp = NULL;
            }
            if (srcfp != NULL) {
                rc = fclose (srcfp);
                srcfp = NULL;
            }
            if (fileChanged) {
                DosCopy (tmpFile,srcFile,DCPY_EXISTING);
            }
            remove (tmpFile);
        }
    }

    if (srcfp != NULL) {
        rc = fclose (srcfp);
        srcfp = NULL;
    }

    return(0);
}





/********************************************************
/
/ This function replaces <FONTQ> tags with <FONT> tags.
/
********************************************************/

void PostUnsegFont(char *InFile )
{
    FILE     *fIn, *fOut ;
    wchar_t  szIn[MAX_TEXT_SIZE];
    char     tempFile[512];
    char     *szAltTempExt7 = ".$$F";
    BOOL      bInSQuotedText = FALSE ;

    CreateTempFileName2( tempFile, InFile, szAltTempExt7, TEMPNAME_TARGET ) ;

    fIn = fopen (InFile,"rb");
    fOut = fopen (tempFile,"wb");
    while( fgetws(szIn,sizeof(szIn)/sizeof(wchar_t),fIn) ) {
       UnsegSpecialTags(szIn, &bInSQuotedText );
       fputws(szIn,fOut);
    }
    fclose( fIn );
    fclose( fOut );

    DosCopy( tempFile, InFile, DCPY_EXISTING );
    remove( tempFile );
}


/********************************************************
/
/ This function replaces <FONTQ> tags with <FONT> tags.
/
********************************************************/

void UnsegSpecialTags(wchar_t *szRcd, BOOL *bInSQuotedText )
{
    wchar_t  szTemp[MAX_TEXT_SIZE];
    wchar_t  *ptrChar, *ptrChar2 ;
    wchar_t  cEuro = '\xAC\x20';

    wcscpy( szTemp, szRcd ) ;
    wcsupr( szTemp ) ;

    ptrChar = szTemp ;
    while( ptrChar ) {
       ptrChar = wcsstr(ptrChar, L"FONTQ") ;
       if ( ptrChar ) {
          ptrChar2 = szRcd + ( ptrChar - szTemp ) ;
          memmove( ptrChar+4, ptrChar+5, (wcslen(ptrChar+5)+1)*sizeof(wchar_t) ) ;
          memmove( ptrChar2+4, ptrChar2+5, (wcslen(ptrChar2+5)+1)*sizeof(wchar_t) ) ;
       }
    }

    ptrChar = szTemp ;                        
    while( ptrChar ) {
       ptrChar = wcsstr(ptrChar, L"NOBRQ") ;
       if ( ptrChar ) {
          ptrChar2 = szRcd + ( ptrChar - szTemp ) ;
          memmove( ptrChar+4, ptrChar+5, (wcslen(ptrChar+5)+1)*sizeof(wchar_t) ) ;
          memmove( ptrChar2+4, ptrChar2+5, (wcslen(ptrChar2+5)+1)*sizeof(wchar_t) ) ;
       }
    }

    ptrChar = szTemp ;                        
    while( ptrChar ) {
       ptrChar = wcsstr(ptrChar, L"MOD+VALUE") ;
       if ( ptrChar ) {
          ptrChar2 = szRcd + ( ptrChar - szTemp ) ;
          memmove( ptrChar, ptrChar+4, (wcslen(ptrChar+4)+1)*sizeof(wchar_t) ) ;
          memmove( ptrChar2, ptrChar2+4, (wcslen(ptrChar2+4)+1)*sizeof(wchar_t) ) ;
       }
    }

    ptrChar = szTemp ;                        
    while( ptrChar ) {
       ptrChar = wcsstr(ptrChar, L"MOD+DIR") ;
       if ( ptrChar ) {
          ptrChar2 = szRcd + ( ptrChar - szTemp ) ;
          memmove( ptrChar, ptrChar+4, (wcslen(ptrChar+4)+1)*sizeof(wchar_t) ) ;
          memmove( ptrChar2, ptrChar2+4, (wcslen(ptrChar2+4)+1)*sizeof(wchar_t) ) ;
       }
    }

    ptrChar = szTemp ;                        
    while( ptrChar ) {
       ptrChar = wcsstr(ptrChar, L"MOD+ALIGN") ;
       if ( ptrChar ) {
          ptrChar2 = szRcd + ( ptrChar - szTemp ) ;
          memmove( ptrChar, ptrChar+4, (wcslen(ptrChar+4)+1)*sizeof(wchar_t) ) ;
          memmove( ptrChar2, ptrChar2+4, (wcslen(ptrChar2+4)+1)*sizeof(wchar_t) ) ;
       }
    }

    ptrChar = szTemp ;                        
    while( ptrChar ) {
       ptrChar = wcsstr(ptrChar, L"{TWBSCR}") ;
       if ( ptrChar ) {
          ptrChar2 = szRcd + ( ptrChar - szTemp ) ;
          memmove( ptrChar, ptrChar+8, (wcslen(ptrChar+8)+1)*sizeof(wchar_t) ) ;
          memmove( ptrChar2, ptrChar2+8, (wcslen(ptrChar2+8)+1)*sizeof(wchar_t) ) ;
       }
    }

    ptrChar = szTemp ;                        
    while( ptrChar ) {
       ptrChar = wcschr( ptrChar, cEuro ) ;
       if ( ptrChar ) {
          ptrChar2 = szRcd + ( ptrChar - szTemp ) ;
          memmove( ptrChar+5, ptrChar, (wcslen(ptrChar)+1)*sizeof(wchar_t) ) ;
          wcsncpy( ptrChar, L"&euro;", 6 ) ;
          memmove( ptrChar2+5, ptrChar2, (wcslen(ptrChar2)+1)*sizeof(wchar_t) ) ;
          wcsncpy( ptrChar2, L"&euro;", 6 ) ;
       }
    }

    ptrChar = szTemp ;                        
    while( ptrChar ) {
       ptrChar = wcsstr(ptrChar, L"&TWBLT;") ;
       if ( ptrChar ) {
          ptrChar2 = szRcd + ( ptrChar - szTemp ) ;
          *ptrChar = L'<' ;
          *ptrChar2 = L'<' ;
          memmove( ptrChar+1, ptrChar+7, (wcslen(ptrChar+7)+1)*sizeof(wchar_t) ) ;
          memmove( ptrChar2+1, ptrChar2+7, (wcslen(ptrChar2+7)+1)*sizeof(wchar_t) ) ;
       }
    }
    if ( ( *bInSQuotedText ) ||               /* Handle single quotes in single quoted text */
         ( wcsstr(szRcd, L"<TWBSQ>" ) ) ) {
       for( ptrChar=szRcd ; *ptrChar!=NULL ; ++ptrChar ) {
          if ( *ptrChar == L'<' ) {     /* Check for start of quoted text */
             if ( ! wcsncmp( ptrChar, L"<TWBSQ>", 7 ) ) {
                wmemmove( ptrChar, ptrChar+7, wcslen(ptrChar+7)+1 ) ; /* Removed <TWBSQ>" */
                --ptrChar ;
                *bInSQuotedText = TRUE ;
                continue ;
             }
             if ( ! wcsncmp( ptrChar, L"</TWBSQ>", 8 ) ) {
                wmemmove( ptrChar, ptrChar+8, wcslen(ptrChar+8)+1 ) ; /* Removed </TWBSQ>" */
                *bInSQuotedText = FALSE ;
                continue ;
             }
          }
          if ( *bInSQuotedText ) {
             if ( *ptrChar == L'\'' ) {       /* Need to escape single quotes */
                wmemmove( ptrChar+1, ptrChar, wcslen(ptrChar)+1 ) ;
                *ptrChar = L'\\' ; 
                ++ptrChar ;
             } else 
             if ( *ptrChar == L'\\' ) {       /* Skip escaped characters */
                ++ptrChar ;
             }
          }
       }
    }

}


/*********************************************************/
//
// This function checks for a .JS (JAVASCRIPT) file type
// then modifies the file to allow proper processing
//
/********************************************************/

short chk_is_js_file(char *srcpath, char *tmpsrcfile, char *szMarkup, short Step)
{
    char szErrTitle[128] ;
    char szErrText[4096] ;
    char szErrString[80] ;


    char  tempFile[512];
    FILE  *srcFp, *tmpFp ;
    wchar_t charline[MAX_TEXT_SIZE];
    wchar_t charline1[MAX_TEXT_SIZE];
    char *token = NULL;
    wchar_t *hexZeroDogZeroAble = L"\x0D\0A";
    wchar_t *firstline = NULL;
    int  linelength = 0;
    wchar_t *endline = NULL;
    char fldrname[512] = {""};
    char shortfile[256] = {""};
    char longfile[256] = {""};
    char myname[256] = {""};
    char *fl;
    wchar_t *script_ptr;
    wchar_t *ptrChar, *ptrChar2, *ptrChar3 ;
    char *temp_ptr ;
    int mylength;
    int lrc = 0;
    int rc =0;
    BOOL bEOF ;
    char *szAltTempExt8 = ".$$G";

    wchar_t  *ptrToken ;
    USHORT   usNumMsg = 0 ;
    ULONG    ulNumNonBlankLines = 0 ;
    USHORT   usTemp = 0 ;
    BOOL     bInBlockComment = FALSE ;
    short sJS_FileType = JS_TYPE_NONE ; 


    // Get the folder path and name and the short filename to pass
    // to the TM/2 long filename conversion function
    strcpy(myname,srcpath);
    if ((fl = strstr(myname,".F00")) != NULL) {   // Test for folder name in path
        mylength = (int)(fl-myname);
        strncpy(fldrname,myname,mylength+4);
        token = strtok(myname, "\\");
        while ((token = strtok(NULL, "\\")) != NULL) {
            strcpy(shortfile,token);
        }

        temp_ptr = strrchr( shortfile, '.' ) ; 
        if ( ( temp_ptr ) &&                                                    
             ( strlen(temp_ptr) == 5 ) ) {
           *(temp_ptr+4) = 0 ;  // Remove trailing letter from name NAME.000A 
        }


        // Get the long file name from TM/2

        rc = EQFCONVERTFILENAMES( fldrname,longfile,shortfile ) ;
        if ( (rc == 0) && (longfile[0] != 0) ){
            strcpy( shortfile, longfile ) ;
        } else {
            strcpy( longfile, shortfile ) ;
        }

        // if the file extension is .JS then the file is a java script file
        // and we have to put <SCRIPT> </SCRIPT> tags around the entire file
        // so the OTMHTM32 markup will run the JAVASCRIPT parser


        if ( ( strstr(longfile + strlen(longfile) -3, ".JS")   != NULL ) ||
             ( strstr(longfile + strlen(longfile) -3, ".js")   != NULL ) )
           sJS_FileType = JS_TYPE_JS ;
        else 
        if ( ( strstr(longfile + strlen(longfile) -5, ".JSON") != NULL ) ||   
             ( strstr(longfile + strlen(longfile) -5, ".json") != NULL ) ) 
            sJS_FileType = JS_TYPE_JSON ;

        if ( sJS_FileType ) {
            CreateTempFileName2( tempFile, srcpath, szAltTempExt8, TEMPNAME_SSOURCE ) ;
            srcFp = fopen (tmpsrcfile,"rb");
            tmpFp = fopen (tempFile,"wb");
            if (Step == 0) {     /* PRESEG */
                wchar_t szTemp[512];
#ifdef _UNICODE
				fseek(srcFp, 2, SEEK_SET);	//skip BOM
#else
                fseek(srcFp, 0, SEEK_SET);
#endif

                // Need to determine what the end-of-line delimeter is for the file
                linelength = MAX_TEXT_SIZE - 1 ;
                while( linelength == MAX_TEXT_SIZE - 1 ) { /* Handle long first line */
                   firstline = fgetws(charline,sizeof(charline)/sizeof(wchar_t),srcFp);
                   linelength = wcslen(firstline);
                }

                if ((charline[linelength-2] == '\x0A') || (charline[linelength-2] == '\x0D')) {
                    endline = charline + linelength - 2;
                } else if ((charline[linelength-1] == '\x0A') || (charline[linelength-1] == '\x0D')) {
                    endline = charline + linelength - 1;
                } else { // choosing 0D0A arbitrarily could cause some problems and delimeter mismatch
                    endline = hexZeroDogZeroAble;
                }
#ifdef _UNICODE
				fseek(srcFp, 2, SEEK_SET); //skip BOM
#else
                fseek(srcFp, 0, SEEK_SET);
#endif 

                // add the <SCRIPT> tags to the file
                swprintf(szTemp, L"%c%s", BYTE_ORDER_MARK, L" <SCRIPT LANGUAGE=\"JAVASCRIPT\">");
                fputws(szTemp,tmpFp);
                fputws(endline,tmpFp);
                bEOF = FALSE ;

                while (fgetws(charline,sizeof(charline)/sizeof(wchar_t),srcFp) != NULL) {
                    fputws(charline,tmpFp);
                    rc = wcslen(charline) ;          
                    if ( ( rc > 0 ) &&
                         ( charline[rc-1] == TWBCNV_EOF/*''*/ ) )
                       bEOF = TRUE ;
                    else
                       bEOF = FALSE ;

                   /* ----------------------------------------------------- */
                   /*  Determine if processing JS or JSON file.             */
                   /*  Use CHKPII algorithm to determine this.              */
                   /* ----------------------------------------------------- */
                   if ( sJS_FileType != JS_TYPE_JSON ) {
                      ptrToken = wcsstr( charline, L"//"  ) ; 
                      if ( ptrToken ) 
                         *ptrToken = NULL ;
                      if ( bInBlockComment ) {
                         if ( wcsstr( charline, L"*/" ) ) 
                            bInBlockComment = FALSE ;
                         continue ;
                      }
                      if ( wcsstr( charline, L"/*" ) ) {
                         if ( ! wcsstr( charline, L"*/" ) ) 
                            bInBlockComment = TRUE ;
                         continue ;
                      }
                      if ( usTemp == 2 ) {  /* Looking for '[' for array of values */
                         for( ptrToken=charline ; *ptrToken && iswspace(*ptrToken) ; ++ptrToken ) ;
                         if ( *ptrToken == L'[' ) 
                            usTemp = 1 ;  /* Array of values */
                         else 
                            usTemp = 0 ;
                      }

                      ptrToken = wcschr( charline, L':' ) ;
                      if ( ( ptrToken ) ||
                           ( usTemp == 1 ) ) {
                         if ( ! ptrToken ) 
                            ptrToken = charline - 1 ;
                         for( ++ptrToken ; *ptrToken && iswspace(*ptrToken) ; ++ptrToken ) ;
                         if ( ( ( *ptrToken == L'\"' ) &&
                                ( wcschr(ptrToken+1,L'\"') != NULL ) ) ||
                              ( ( *ptrToken == L'\'' ) &&             
                                ( wcschr(ptrToken+1,L'\'') != NULL ) ) )
                            ++usNumMsg ;
                         else {
                            if ( usTemp != 1 ) {                      
                               if ( *ptrToken ) 
                                  if ( *ptrToken == L']' ) 
                                     usTemp = 1 ;  /* Array of values */
                                  else 
                                     usTemp = 0 ;
                               else 
                                  usTemp = 2 ;   /* Looking for next non-blank char */
                            }
                         }
                      }
                      if ( ( usTemp == 1 ) &&
                           ( wcschr( charline, L'[' ) ) ) 
                         usTemp = 0 ;

                      ptrToken = wcstok( charline, L" \n\r\t" ) ;
                      if ( ptrToken != NULL ) {
                         if ( ( ! wcscmp( charline, L"({" ) ) ||
                              ( ! wcscmp( charline, L"})" ) ) ) 
                            ++usNumMsg ; 
                         else
                           ++ulNumNonBlankLines ;
                      }
                   }

                }

                if ( ( sJS_FileType == JS_TYPE_JSON     ) ||
                     ( usNumMsg > ulNumNonBlankLines/2  ) ||
                     ( ! strcmp( szMarkup, "IBMJSUDJ" ) ) ) {     
                   sJS_FileType = JS_TYPE_JSON ;
                   fputws(L"<!--TWB_JSON-->",tmpFp);
                }

                if ( bEOF )                                       
                   fseek(tmpFp, (ftell(tmpFp)-1*sizeof(wchar_t)), SEEK_SET);
                fputws(L"</SCRIPT><!--TWB-->",tmpFp);
                fclose(srcFp);
                fclose (tmpFp);
                DosCopy (tempFile, tmpsrcfile, DCPY_EXISTING);
                remove (tempFile);
            } else {  // Delete tags
                wchar_t szTemp[512];
                fgetws(charline,sizeof(charline)/sizeof(wchar_t),srcFp); // Just get but don't write out the first line its the <SCRIPT> tag.
                if ( Step == 1 ) {          /* POSTSEG. Remove JSON tagging */
                   for( ptrChar=wcsstr(charline,HTMLDOC_JSON_START) ;
                        ptrChar ; 
                        ptrChar=wcsstr(charline,HTMLDOC_JSON_START) ) {
                      wmemmove( ptrChar, ptrChar+7, wcslen(ptrChar+7)+1 ) ;
                   }
                   for( ptrChar=wcsstr(charline,HTMLDOC_JSON_END) ;
                        ptrChar ; 
                        ptrChar=wcsstr(charline,HTMLDOC_JSON_END) ) {
                      wmemmove( ptrChar, ptrChar+7, wcslen(ptrChar+7)+1 ) ;
                   }
                   fputws( charline, tmpFp ) ; //Write to file 
                } else{                      /* POSTUNSEG. Remove all tags */
                swprintf(szTemp, L"%c", BYTE_ORDER_MARK);
                fputws( szTemp, tmpFp ) ; //Add BOM to the file
                }
                while (fgetws(charline,sizeof(charline)/sizeof(wchar_t),srcFp) != NULL) {
                   if ((ptrChar = wcsstr(charline,L"<!--TWB_JSON-->")) != NULL) {
                      sJS_FileType = JS_TYPE_JSON ;
                      if ( Step == 2 ) {                              /* POSTUNSEG */
                         wmemmove( ptrChar, ptrChar+15, wcslen(ptrChar+15)+1 ) ;
                      }
                   }
                   if ( Step == 1 ) {
                      for( ptrChar=wcsstr(charline,HTMLDOC_JSON_START) ;
                           ptrChar ; 
                           ptrChar=wcsstr(charline,HTMLDOC_JSON_START) ) {
                         wmemmove( ptrChar, ptrChar+7, wcslen(ptrChar+7)+1 ) ;
                      }
                      for( ptrChar=wcsstr(charline,HTMLDOC_JSON_END) ;
                           ptrChar ; 
                           ptrChar=wcsstr(charline,HTMLDOC_JSON_END) ) {
                         wmemmove( ptrChar, ptrChar+7, wcslen(ptrChar+7)+1 ) ;

                         /* Check for "${0}:" to make it translatable */
                         if ( ! wcsncmp( ptrChar, L":eqfn.:qfn ", 11 ) ) {
                            ptrChar2 = wcschr( ptrChar+11, L'.' ) ;
                            if ( ( ptrChar2 ) &&
                                 ( ! wcsncmp( ptrChar2+1, L"${", 2 ) ) ) {
                               ptrChar3 = wcsstr( ptrChar2, L"::eqfn." ) ;
                               if ( ptrChar3 ) {
                                  *(ptrChar+9) = L'x' ;
                                  *(ptrChar3+5) = L'x' ;
                               }
                            }
                         }
                      }
                   }
                   if ( ( (ptrChar = wcsstr(charline,L"</SCRIPT><!--TWB-->")) != NULL) &&
                        ( Step == 2 ) ) {
                        // added <!--TWB--> to the artificial </SCRIPT> tag to make it more unique to search for in removal
                        memset(charline1,'\0',sizeof(charline1));
                       wcsncpy(charline1,charline, (ptrChar-charline));
                       wcscat(charline1,ptrChar+19);
                        fputws(charline1,tmpFp);
                    } else {
                        fputws(charline,tmpFp);
                    }
                }
                fclose (srcFp);
                fclose (tmpFp);
                DosCopy (tempFile, tmpsrcfile, DCPY_EXISTING);
                remove (tempFile);
            }
        }

    }
    return( sJS_FileType ) ;
}

BOOL quickOffendCharCheck(FILE *fp,char offender)
{
    BOOL retVal = FALSE;
    wchar_t c;

//  fseek(fp,-(1*sizeof(wchar_t)),SEEK_END);                // position beginning.
    fseek(fp,0L,SEEK_SET);                // position beginning.

    c = fgetwc(fp);
    if (c == offender) {
        retVal = TRUE;
    }

    fseek(fp,0,SEEK_SET);                 // position back to the beginning.

    return(retVal);
}

long removeEndingOffendingChar(char *szTgt,wchar_t offender)
{
    FILE *ifp,*ofp;
    char tempFileName[512];
    wchar_t c,cPrev;
    BOOL keepit = FALSE;
    char *szAltTempExt9 = ".$$H";

    ifp = ofp = NULL;

    ifp = fopen(szTgt,"rb");

    if (ifp != NULL) {
        if (quickOffendCharCheck(ifp,offender)) {
            CreateTempFileName2( tempFileName, szTgt, szAltTempExt9, TEMPNAME_TARGET ) ;
            ofp = fopen (tempFileName,"wb");

            if ((ifp != NULL) && (ofp != NULL)) {
                c = cPrev = fgetwc(ifp);
                do {
                    c = fgetwc(ifp);
                    if (feof(ifp)) {  // do not want to actually write the EOF indicator.
                        if ((feof(ifp)) && (cPrev == offender)) {
                            keepit = TRUE;
                        }
                    } else {
                        fputwc(cPrev,ofp);
                    }
                    cPrev = c;
                } while ((!feof(ifp)) && (!ferror(ifp)) && (!ferror(ofp)));
            }
        }  // end if quick check.
    }  // end if open input file.

    if (ifp != NULL) fclose(ifp);
    if (ofp != NULL) fclose(ofp);

    if (keepit) {
        DosCopy (tempFileName,szTgt,DCPY_EXISTING);
    }

    remove(tempFileName);

    return(1);
}

/*****************************************************************************/
/*  ShowError                                                                */
/*                                                                           */
/*  Show an error to the user using the platform's message box function.     */
/*                                                                           */
/*  Return:  TRUE  - User pressed OK.                                        */
/*           FALSE - User pressed cancel.                                    */
/*****************************************************************************/

BOOL ShowError( wchar_t *Title, wchar_t *Message, BOOL bOKCancel )
{
   short  rc ;
   BOOL   bReturn = TRUE;


   if ( bOKCancel ) {

      rc = MessageBox( HWND_DESKTOP, Message, Title,
                       MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONSTOP | MB_SYSTEMMODAL ) ;
      if ( rc != IDOK )
         bReturn = FALSE ;

   } else {

      rc = MessageBox( HWND_DESKTOP, Message, Title,
                       MB_OK | MB_DEFBUTTON1 | MB_ICONSTOP | MB_SYSTEMMODAL ) ;

   }

   return bReturn ;
}

/***************************************************************************/
/* Function: getTemporaryFileName                                          */
/*                                                                         */
/* Purpose: Creates a temporary file name.                                 */
/*    NOTE: If NULL is passed as nameBuf, calloc is called...this memory   */
/*          MUST be freed when not in use.                                 */
/*                                                                         */
/* Returns:  new temporary file, otherwise NULL.                           */
/***************************************************************************/
extern char * __cdecl getTemporaryFileName(char *nameBuf,short nameBufSize)
{
char *tempFile;

   tempFile = tempnam (NULL,"");
   if (tempFile == NULL)
      {
      tempFile = strdup ("\\temp.tmp");
      }

   if (nameBuf == NULL)
      {
      nameBuf = tempFile;
      }
   else
      {
      memset (nameBuf,'\0',nameBufSize);
      if (strlen(tempFile) < nameBufSize)
         {
         strcpy (nameBuf,tempFile);
         }
      }

   return (nameBuf);
}

/****************************************************************************/
/*                                                                          */
/* chk_is_inQuote                                                           */
/*                                                                          */
/* Find whether or not one tag is in a Quoted Text                          */
/*                                                                          */
/* Returns: Ture=in quotedText  False=not in QuotedText                     */
/*                                                                          */
/****************************************************************************/
BOOL chk_is_inQuote( long st, long tagPos, FILE *fp ) {  

    BOOL bInQuote = FALSE ;
    wchar_t quoteChar;
    wchar_t curChar, prevChar ;
    long srcPos = -1 ;
    
    fseek( fp, st, SEEK_SET ) ;
    while( srcPos < tagPos ) {
        curChar = fgetwc( fp ) ;
        if ( !bInQuote && ( curChar == L'/' ) ) {
            curChar = fgetwc( fp ) ;
            if ( curChar == L'/' ) {
                while ( ( curChar != L'\n' ) && ( curChar != L'\r' ) && ( ftell(fp) < tagPos ) ) {
                    curChar = fgetwc( fp ) ;
                }
            } else if ( curChar == L'*' ) {
                while ( ftell(fp) < tagPos ) {
                    curChar = fgetwc( fp ) ;
                    if ( curChar == L'*' ) {
                        curChar = fgetwc( fp ) ;
                        if ( curChar == L'/' ) 
                            break ;
                    }
                }
            }
        }
        if ( bInQuote && ( curChar == L'\\' ) ) {                   
            curChar = fgetwc( fp ) ;
            continue ;
        }
        if ( ( curChar == L'\"' ) || ( curChar == L'\'' ) ) {
            if ( !bInQuote ) {                                      
                bInQuote = TRUE ;
                quoteChar = curChar ;
            } else if ( bInQuote && ( curChar == quoteChar ) ) {    
                bInQuote = FALSE ;
            }
        }
        srcPos = ftell( fp ) ;
    }
    return bInQuote ;
}
