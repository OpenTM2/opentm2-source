/*
*
*  Copyright (C) 1998-2013 International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/****************************************************************************/
/*                                                                          */
/* otmjdk11.c                                                               */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*    Description:                                                          */
/*         This function will examine a Java Resource Bundle and do the     */
/*         segmentation processing.  This DLL will have 4 entry points.       */
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
/****************************************************************************/

#include "otmjdk11.h"


extern   char    szDocTargetLanguage[80];   /* From USRCALLS.C  */
extern   char    szDocSourceLanguage[80];   /* From USRCALLS.C  */
extern   short   sTPVersion ;               /* From USRCALLS.C  */
         BOOL    bDBCSSrc ;
         BOOL    bDBCSTgt ;
         BOOL    bDBCSInfo ;
         BOOL    bJapanese ;
         BOOL    bKorean ;
         BOOL    bSChinese ;
         BOOL    bTChinese ;
         BOOL    bNoDBCS ;



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


/*************************************************************************/
/*  Pre-segmentation, including access to progress window.               */
/*************************************************************************/
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPRESEG2(
                     PSZ    pTagTable,
                     PSZ    pEdit,
                     PSZ    pProgPath,
                     PSZ    pSource,
                     PSZ    pTempSource,
                     PEQF_BOOL     pfNoSegment,
                     HWND        hSlider,
                     PEQF_BOOL     pfKill )
{
    char      szADummy1[1024] ;
    char      szADummy2[1024] ={0};
    char      szTemp[256] ;
    char      szMarkupTable[80] ;
    PSZ       pName;                     /* Pointer to file name          */
    UCHAR     longfilename[1024];
    char      *pszWorkFile ;
    char      szWorkFile[256] ;
    char      szSourceFile[256] ;
    short     sType ;
    BOOL      bReturn = TRUE ; 
    BOOL      bLRB = FALSE ; 
    BOOL      bCheckMessageFormat = FALSE ; 
    BOOL      bUTF8 = FALSE ;
    char *szAltTempExt1 = ".$$1";
    char *szAltTempExt2 = ".$$2";


    PrepDocLanguageInfo( pSource ) ;   /* Set language unique processing, like DBCS */
    bDBCSSrc = SetDBCSLang( szDocSourceLanguage ) ;
    bDBCSTgt = SetDBCSLang( szDocTargetLanguage ) ;
    bDBCSInfo = FALSE ;
    bJapanese = FALSE ;
    bKorean = FALSE ;
    bSChinese = FALSE ;
    bTChinese = FALSE ;
    bNoDBCS = FALSE ;


    /*************************************************************************/
    /*  Create temporary file name by changing extension to "$$$".           */
    /*************************************************************************/
    CreateTempFileName2( szWorkFile,  pSource, szAltTempExt1, TEMPNAME_SSOURCE ) ;
    CreateTempFileName2( pTempSource, pSource, szAltTempExt2, TEMPNAME_SSOURCE ) ;

    TM2ConvertShortToLong(pSource, longfilename);
    if ( longfilename[0] == 0 ) 
        strcpy( longfilename, pSource ) ;

    strcpy( szMarkupTable, pTagTable ) ;
    strupr( szMarkupTable ) ;

    strcpy( szSourceFile, pSource ) ;


    /*************************************************************************/
    /*  Convert escaped Unicode to UTF-8 for processing.                     */
    /*************************************************************************/
    if ( ! strncmp( szMarkupTable, "OTMNJDK", 7 ) ) {
       bReturn = ConvertFromEscapedUnicode( szSourceFile, szWorkFile ) ; 
       if ( bReturn ) 
          bReturn = ConvertExport( szWorkFile, EQF_UTF162UTF8 ) ;
       strcpy( szSourceFile, szWorkFile ) ;
    } else

    /*************************************************************************/
    /*  Convert UTF-8 to UTF-8 for processing.                               */
    /*************************************************************************/
    if ( ! strncmp( szMarkupTable, "OTMU", 4 ) ) { 
       CopyFile(szSourceFile, szWorkFile, FALSE); 
    } else

    /*************************************************************************/
    /*  Convert ANSI to UTF-8 for processing.                                */
    /*************************************************************************/
    if ( ! strncmp( szMarkupTable, "OTMA", 4 ) ) {      /* ANSI markup table */
       bReturn = ConvertImport(szSourceFile, szWorkFile, EQF_ANSI2UTF8);
       strcpy( szSourceFile, szWorkFile ) ;
    } else {

       /*************************************************************************/
       /*  Convert ASCII to UTF-8 for processing.                               */
       /*************************************************************************/
       bReturn = ConvertImport(szSourceFile, szWorkFile, EQF_ASCII2UTF8);
       strcpy( szSourceFile, szWorkFile ) ;
    } 

    strcpy( szSourceFile, szWorkFile ) ;

    /*************************************************************************/
    /*  Process file.                                                        */
    /*************************************************************************/
    if ( bReturn ) {
//     if ( strstr( szMarkupTable, "JDK2" ) )   Disable for OpenTM2
//        bCheckMessageFormat = TRUE ; 

       bLRB = chk_LRB_File( longfilename, pSource, &sType, pTagTable ) ;

       if ( bLRB ) {

           parseJavaResource( szSourceFile, sType, bCheckMessageFormat, hSlider);

       } else {

          bReturn = ParseFile( szSourceFile, pTempSource, sType, bCheckMessageFormat, hSlider ) ;
          CopyFile(pTempSource, szSourceFile, FALSE); 

       }
       EQFSETSLIDER( hSlider, 100 ) ;
    }


    *pfNoSegment = TRUE ;                  /* Do not perform text analysis. */
    if ( bReturn ) {
       bReturn = ConvertImport( szSourceFile, pTempSource, EQF_UTF82UTF16 ) ; /* Convert from UTF-8 to UTF-16 */
    }
    remove( szWorkFile ) ;
    if ( ( bLRB ) &&
         ( bReturn ) ) {
       resequence_TM2(pTempSource);
    }

    return(bReturn);

}  /* EQFPRESEG2 */



__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTSEGW(
                       PSZ pTagTable,
                       PSZ      pEdit,
                       PSZ      pProgPath,
                       PSZ      pSegSource,
                       PSZ      pSegTarget,
                       PTATAG_W pTATAGW,
                       HWND     hSlider,
                       PEQF_BOOL  pfKill )

{

    return(TRUE);

}


/*************************************************************************/
/*  Pre-unsegmentation, including access to progress window.             */
/*************************************************************************/
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPREUNSEGW(
                       PSZ       pTagTable,
                       PSZ       pEdit,
                       PSZ       pProgPath,
                       PSZ       pSegTarget,
                       PSZ       pTemp,
                       PTATAG_W  pTATagW,
                       PEQF_BOOL pfNoUnseg,
                       PEQF_BOOL pfKill )
{
    UCHAR  tmpfilename[1024];
    UCHAR  tmpfilename2[1024];
    char  *szAltTempExt3 = ".$$3";
    char  *szAltTempExt4 = ".$$4";
    short  sType ;
    BOOL   bReturn = TRUE ;
    BOOL   bDBCS ;


    PrepDocLanguageInfo( pSegTarget ) ;   /* Set language unqiue processing, like DBCS */
    bDBCSSrc = SetDBCSLang( szDocSourceLanguage ) ;
    bDBCSTgt = SetDBCSLang( szDocTargetLanguage ) ;
    bDBCS = IsDBCS( '\x90' ) ;

    *pfNoUnseg = FALSE ;                    /* Unsegmentation is required.   */

    TM2ConvertShortToLong(  pSegTarget, tmpfilename );

    if (tmpfilename[0] == 0) {
        strcpy(tmpfilename, pProgPath);
    }

    // Check if LRB or Property file and process as required.

    CreateTempFileName2( pTemp, pSegTarget, szAltTempExt3, TEMPNAME_STARGET ) ;
    bReturn = ConvertImport( pSegTarget, pTemp, EQF_UTF162UTF8 ) ;    /* UTF-16  ->  UTF-8 */


    if ( bReturn ) {
       CreateTempFileName2( tmpfilename2, pSegTarget, szAltTempExt4, TEMPNAME_STARGET ) ;

       if ( ! chk_LRB_File( tmpfilename, pTemp, &sType, pTagTable ) ) {

          bReturn = ! ExportFile( pTemp, tmpfilename2 );

          if ( bReturn ) {
             bReturn = ConvertImport( tmpfilename2, pTemp, EQF_UTF82UTF16 ) ; /* UTF-8 to UTF-16 */
          }

       } else {

          bReturn = ExportLRBFile( pTemp, tmpfilename2, pTemp ) ;   

          if ( bReturn ) {
             bReturn = ConvertImport( tmpfilename2, pTemp, EQF_UTF82UTF16 ) ; /* UTF-8 to UTF-16 */
          }

       }
       remove( tmpfilename2 ) ;
    }


    return( bReturn ) ;

}  /* EQFPREUNSEG2 */






/*************************************************************************/
/*  Post-unsegmentation, including access to progress window.             */
/*************************************************************************/
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTUNSEGW(
                       PSZ       pTagTable,
                       PSZ       pEdit,
                       PSZ       pProgPath,
                       PSZ       pTarget,
                       PTATAG    pTATag,
                       PEQF_BOOL pfKill )
{
    char   szTempFile[256] ;
    char   szMarkupTable[80] ;
    UCHAR  tmpfilename[1024];
    char  *szAltTempExt5 = ".$$5";
    short  sType ;
    USHORT usCPType = EXPORT_CP_NONE ;
    BOOL   bReturn = TRUE ;


    PrepDocLanguageInfo( pTarget ) ;   /* Set language unqiue processing, like DBCS */
    bDBCSSrc = SetDBCSLang( szDocSourceLanguage ) ;
    bDBCSTgt = SetDBCSLang( szDocTargetLanguage ) ;

    TM2ConvertShortToLong(  pTarget, tmpfilename );

    if (tmpfilename[0] == 0) 
        strcpy(tmpfilename, pProgPath);

    /*************************************************************************/
    /*  Convert ASCII file into UTF-8 for export.                            */
    /*************************************************************************/
    strcpy( szMarkupTable, pTagTable ) ;
    strupr( szMarkupTable ) ;
    if ( ! strncmp( szMarkupTable, "OTMU", 4 ) ) {
       usCPType = EXPORT_CP_UTF8 ;
       bReturn = ConvertExport( pTarget, EQF_UTF162UTF8 ) ;    /* UTF-16  ->  UTF-8 */
    } else {
       if ( ! strncmp( szMarkupTable, "OTMA", 4 ) ) {       /* ANSI markup table */
          usCPType = EXPORT_CP_DBCS_ASCII ;
          bReturn = ConvertExport( pTarget, EQF_UTF162ANSI ) ; /* UTF-16  ->  ANSI */
          if ( ! strcmp( szDocTargetLanguage, "Kazakh" ) )     /* Kazakh */
             usCPType = EXPORT_CP_UTF8 ;
       } else {
          if ( ! strncmp( szMarkupTable, "OTMNJDK", 7 ) ) {
             bReturn = ConvertToEscapedUnicode( pTarget ) ;       /* UTF-16  ->  Escaped Unicode */
          } else {
             usCPType = EXPORT_CP_DBCS_ASCII ;
             bReturn = ConvertExport( pTarget, EQF_UTF162ASCII ) ;/* UTF-16  ->  ASCII */
             if ( ! strcmp( szDocTargetLanguage, "Kazakh" ) )     /* Kazakh */
                usCPType = EXPORT_CP_UTF8 ;
          }
       }
    }


    // Check if Property file, then process as required.

    if ( ! chk_LRB_File(tmpfilename, pTarget, &sType, pTagTable ) ) {

       CreateTempFileName2( szTempFile, pTarget, szAltTempExt5, TEMPNAME_TARGET ) ;

       ExportFile2( pTarget, szTempFile, usCPType );


       remove( szTempFile ) ;

    } 

    return( bReturn );
}


/*******************************************************************/
/*                                                                 */
/*    Check if file is a ListResourceBundle Type file              */
/*                                                                 */
/*******************************************************************/


BOOL chk_LRB_File(char *sourceName, char *shortName, short *sType,
                  char* MarkupName )
{
    FILE *sourceFile_ptr = NULL;
    char line[MAX_LEN];
    char upperCaseBuff[MAX_LEN+1];
    PSZ  pName, pDot ;
    BOOL bLRB = FALSE ;

    *sType = LRB_NONE ;
    memset(upperCaseBuff, 0, sizeof(upperCaseBuff));

    strcpy(upperCaseBuff, sourceName);
    StrUpr(upperCaseBuff);

    pName = strrchr( upperCaseBuff, '\\' );   /* Find file name                */
    pDot  = strrchr( upperCaseBuff, '.' );    /* Find file extension           */

    if (pName == NULL) {
        pName = upperCaseBuff;
    }

    if ( ( pDot > pName ) &&                       /* 10-25-01 */
         ( ( ! strcmp( pDot, ".JAV"  ) ) ||
           ( ! strcmp( pDot, ".JAVA" ) ) ) ) {
        bLRB = TRUE;
    }

    sourceFile_ptr = fopen (shortName,"rb");
    while (fgets(line, MAX_LEN, sourceFile_ptr)) {
        memset(upperCaseBuff, 0, sizeof(upperCaseBuff));
        strcpy(upperCaseBuff, line);
        StrUpr(upperCaseBuff);
        if ( ( strstr( upperCaseBuff, "IMPORT JAVA." ) != NULL ) &&
             ( strchr( upperCaseBuff, ';'            ) != NULL ) ) { /* 4-3-08 */
           bLRB = TRUE;
        }
        if ( strstr( upperCaseBuff, "EXTENDS" ) != NULL ) {
           if ( strstr( upperCaseBuff, "RESOURCEBUNDLE" ) != NULL ) {
              bLRB = TRUE;
           } 
           break ;
        }
    }
    fclose(sourceFile_ptr);


    if ( *sType == LRB_NONE ) {
       if ( bLRB ) {
          *sType = LRB_STANDARD ;
       } else {
          *sType = PRB_STANDARD ;
       }
    }
    return( bLRB ) ;
}



/****************************************************************************/
/*                                                                          */
/* TM2ConvertShortToLong                                                    */
/*                                                                          */
/* Convert the TM/2 short file name to the long file name.                  */
/*                                                                          */
/*    szPath       - Complete short name path, including TM/2 folder, file  */
/*    szFileName   - pointer to Empty string                                */
/*                                                                          */
/*                                                                          */
/****************************************************************************/
USHORT TM2ConvertShortToLong( UCHAR *szPath, UCHAR *szLong )
{
    UCHAR  szTempPath[MAX_LEN];
    UCHAR  szShortName[MAX_LEN];
    UCHAR  *path_ptr ;
    UCHAR  *short_ptr ;
    UCHAR  *temp_ptr ;
    USHORT rc ;

    strcpy( szTempPath, szPath ) ;
    strcpy( szShortName, szPath ) ;

    /*  Pull folder path out of the full path name name FOLDER_DRIVE:\EQF\FOLDER_NAME.F00    */

    path_ptr = (strstr( szTempPath, szFOLDER_EXT ));
    if ( path_ptr ) {
        *(path_ptr+2)= 0 ;
    } else {
        rc=10;  // Return fail no folder path
        return rc;
    }


/*  Pull short name out of the full path name name NAME.000    */

    short_ptr = strrchr(szShortName, '\\');

    if (short_ptr ) {
        short_ptr += 1 ;
    } else {
        rc = 11;    // Return fail no short filename
        return rc;
    }
    temp_ptr = strrchr( short_ptr, '.' ) ;          
    if ( ( temp_ptr ) &&                                                    
         ( strlen(temp_ptr) == 5 ) ) {
       *(temp_ptr+4) = 0 ;  // Remove trailing letter from name NAME.000A 
    }
    szLong[0] = 0 ;   // Clear the long filename buff TM/2 requires to know which conversion to do


//kml : P007994 do conversion to appropriate filename
    rc = EQFCONVERTFILENAMES( szTempPath, szLong, short_ptr ) ;
    if ( (rc == 0) && (szLong[0] != 0) ){
        strcpy( short_ptr, szLong ) ;
    } else {
        strcpy( szLong, short_ptr ) ;
    }

    return rc ;
}



/****************************************************************************/
/*                                                                          */
/* ConvertFromEscapedUnicode                                                */
/*                                                                          */
/* Convert the Java escaped Unicode data into UTF-16.                       */
/*                                                                          */
/*    szInFile     - Input file (escaped Unicode).                          */
/*    szOutFile    - Output file (UTF-16).                                  */
/*                                                                          */
/****************************************************************************/
USHORT ConvertFromEscapedUnicode( UCHAR *szInFile, UCHAR *szOutFile )
{
    FILE   *fInput ;
    FILE   *fOutput ;
    CHAR   szInChars[10] ;
    CHAR   cChar ;
    LONG   lFilePos ;
    USHORT usChar ;
    USHORT usChar1 ;
    USHORT usChar2 ;
    USHORT i ;
    USHORT rc = 1 ;
    BOOL   bSkipRead = FALSE ;
    BOOL   bLoop = TRUE ;


    fInput = fopen( szInFile, "rb" ) ;
    fOutput = fopen( szOutFile, "wb" ) ;
    fprintf( fOutput, "\xFF\xFE" ) ;     /* Write BOM x'FFFE" for UTF-16 file */

    /*------------------------------------------------------------------------*/
    /*  Process all characters in this file.                                  */
    /*------------------------------------------------------------------------*/
    while( bLoop==TRUE && rc==1 ) {
       if ( ! bSkipRead ) {
          usChar1 = fgetc( fInput ) ;
          if ( feof(fInput) )
             break ;
       }
       bSkipRead = FALSE ;

       if ( usChar1 == '\\' ) {                    /* Escaped character       */
          usChar2 = fgetc( fInput ) ;              /* Get next character      */
          if ( usChar2 != 'u' ) {                  /* If not Unicode,         */
             fprintf( fOutput, "%c", usChar1 ) ;   /*  then write out '\'     */
             fputc( '\x00', fOutput ) ;
             if ( usChar2 == '\\' ) {              /* If escaped backslash,   */
                fprintf( fOutput, "%c", usChar2 ) ; 
                fputc( '\x00', fOutput ) ;
             } else {
                usChar1 = usChar2 ;
                bSkipRead = TRUE ;                 /* Next char already read  */
             }
             continue ;
          }
          lFilePos = ftell( fInput ) ;
          for( i=0 ; i<4 ; ++i ) {                 /* Escaped Unicode \u____  */
             cChar = (UCHAR)fgetc(fInput) ;        /* Get next character      */
             if ( isxdigit(cChar) ) {              /* If valid hex digit      */
                if ( isdigit(cChar) ) {            /* Set hex digit value     */
                   usChar = cChar - '0' ;          /* 0 - 9                   */
                } else 
                if (cChar <= 'F' ) {
                   usChar = cChar - 'A' + 10 ;     /* A - F                   */
                } else {
                   usChar = cChar - 'a' + 10 ;     /* a - f                   */
                }
                szInChars[i] = usChar ;            /* Save value              */
             } else {
                rc = 0 ;
                fseek( fInput, lFilePos, 0 ) ;     /* Reset file reading pos  */
                fputc( '\\', fOutput ) ;            
                fputc( '\x00', fOutput ) ;
                fputc( 'u', fOutput ) ;
                fputc( '\x00', fOutput ) ;
                break ;
             }
          }

          if ( rc != 0 ) {                         /* Write UTF-16 value      */
             if ( ( szInChars[2] == 5  ) &&
                  ( szInChars[3] == 12 ) &&
                  ( szInChars[0] == 0  ) &&
                  ( szInChars[1] == 0  ) ) { 
                fwprintf( fOutput, L"\\u005%c",cChar ) ;  
             } else
             if ( ( ( szInChars[3] == 0  ) ||       
                    ( szInChars[3] == 10 ) ||       
                    ( szInChars[3] == 13 ) ) &&
                  ( szInChars[0] == 0  ) &&
                  ( szInChars[1] == 0  ) &&
                  ( szInChars[2] == 0  ) ) { 
                fwprintf( fOutput, L"\\u000%c",cChar ) ;  
             } else
             if ( ( szInChars[0] == 15 ) &&         
                  ( szInChars[1] == 15 ) &&
                  ( szInChars[2] == 15 ) &&
                  ( ( szInChars[3] == 12 ) ||
                    ( szInChars[3] == 13 ) ) ) { 
                fwprintf( fOutput, L"\\uFFF%c",cChar ) ;  
             } else {
                fprintf( fOutput, "%c", (UCHAR)(szInChars[2]*16+szInChars[3]) ) ;
                fprintf( fOutput, "%c", (UCHAR)(szInChars[0]*16+szInChars[1]) ) ;
             }
          } else {
             rc = 1 ;                               
          }
       } else {
          fprintf( fOutput, "%c", usChar1 ) ;      /* Write non-escaped char  */
          fputc( '\x00', fOutput ) ;
       }
    }
    fclose( fInput ) ;
    fclose( fOutput ) ;

    return rc ;
}



/****************************************************************************/
/*                                                                          */
/* ConvertToEscapedUnicode                                                  */
/*                                                                          */
/* Convert the UTF-16 data into Java escaped Unicode.                       */
/*                                                                          */
/*    szFile       - Input file (UTF-16).                                   */
/*                                                                          */
/****************************************************************************/
USHORT ConvertToEscapedUnicode( UCHAR *szFile )
{
    FILE   *fInput ;
    FILE   *fOutput ;
    UCHAR  szTempFile[256];
    char  *szAltTempExt6 = ".$$6";
    CHAR   szOutChars[10] ;
    USHORT usChar1 ;
    USHORT usChar2 ;
    USHORT rc = 1 ;
    BOOL   bLoop = TRUE ;

    CreateTempFileName2( szTempFile, szFile, szAltTempExt6, TEMPNAME_TARGET ) ;

    fInput = fopen( szFile, "rb" ) ;
    fOutput = fopen( szTempFile, "wb" ) ;
    usChar1 = fgetc( fInput ) ;           /* Skip BOM x'FFFE' for UTF-16 file */
    usChar2 = fgetc( fInput ) ;

    /*----------------------------------------------------------------------*/
    /*  Process all characters in this file.                                */
    /*----------------------------------------------------------------------*/
    while( bLoop == TRUE ){
       usChar1 = fgetc( fInput ) ;                 /* Get next UTF-16 char  */
       usChar2 = fgetc( fInput ) ;
       if ( feof(fInput) )
          break ;

       if ( ( usChar1 < 128 ) &&                   /* If non-escaped char,  */
            ( usChar2 ==  0 ) ) {
          szOutChars[0] = usChar1 ;                /* Write out 1 character */
          szOutChars[1] = 0 ;
       } else {                                    /* Write escaped Unicode */
          sprintf(szOutChars, "\\u%2.2x%2.2x", usChar2, usChar1 ) ;
       }
       fputs( szOutChars,fOutput ) ;

    }
    fclose( fInput ) ;
    fclose( fOutput ) ;

    CopyFile( szTempFile, szFile, FALSE);
    remove( szTempFile ) ;

    return rc ;
}



/****************************************************************************/
/*                                                                          */
/* GetCharLength                                                            */
/*                                                                          */
/* Determine the length of this character.                                  */  
/*                                                                          */
/* Check for differences between ANSI/ASCII and UTF-8.                      */
/*        Unicode        UTF-8                                              */
/*     Min      Max      Byte Sequence in Binary                            */
/*     ---------------   -----------------------------------                */
/*     0000     007F     0vvvvvvv                                           */
/*     0080     07FF     110vvvvv 10vvvvvv                                  */
/*     0800     FFFF     1110vvvv 10vvvvvv 10vvvvvv                         */
/*     10000    10FFFF   11110vvv 10vvvvvv 10vvvvvv 10vvvvvv                */
/*                                                                          */
/* Input:      szInput   - Input string to check.                           */
/* Return:               - If UTF-8 char, then return the length of char.   */
/*                       - If note UTF-8, return 1.                         */
/*                                                                          */
/****************************************************************************/

SHORT GetCharLength( UCHAR * szCheck )
{
   SHORT    sLength = 1 ;

   if ( szCheck[0] >= (UCHAR)'\x80' ) {               /* Possible UTF-8     */
      if ( ( szCheck[1] >= (UCHAR)'\x80' ) &&
           ( szCheck[1] <= (UCHAR)'\xBF' ) ) {
         sLength = 2 ;                                /* Valid 2-byte UTF8  */
         if ( szCheck[0] >= (UCHAR)'\xE0' ) {
            if ( ( szCheck[2] >= (UCHAR)'\x80' ) &&
                 ( szCheck[2] <= (UCHAR)'\xBF' ) ) {
               sLength = 3 ;                          /* Valid 3-byte UTF8  */
               if ( szCheck[0] >= (UCHAR)'\xF0' ) {
                  if ( ( szCheck[3] >= (UCHAR)'\x80' ) &&
                       ( szCheck[3] <= (UCHAR)'\xBF' ) ) {
                     sLength = 4 ;                    /* Valid 4-byte UTF8  */
                     if ( szCheck[0] >= (UCHAR)'\xF8' ) {
                        sLength = 1 ;           /* Not UTF-8 character      */
                     }
                  } else {
                     sLength = 1 ;              /* Not UTF-8 character      */
                  }
               }
            } else {
               sLength = 1 ;                    /* Not UTF-8 character      */
            }
         }
      } else {
         sLength = 1 ;                          /* Not UTF-8 character      */
      }
   }
//printf("CC: %d  ##%s##\n",sLength,szCheck);

   return( sLength ) ;
}

/****************************************************************************/
/*                                                                          */
/* SetDBCSLang                                                              */
/*                                                                          */
/* Determine if language is DBCS.                                           */  
/*                                                                          */
/* Input:      szLangauge  - Language to chekc.                             */
/* Return:               - TRUE.  Language is DBCS.                         */
/*                       - FALSE. Language is not DBCS.                     */
/*                                                                          */
/****************************************************************************/

BOOL SetDBCSLang( UCHAR * szLanguage )
{
   if ( ( ! stricmp( szLanguage, "Japanese"        ) ) ||
        ( ! stricmp( szLanguage, "Korean"          ) ) ||
        ( ! stricmp( szLanguage, "Chinese(simpl.)" ) ) ||
        ( ! stricmp( szLanguage, "Chinese(trad.)"  ) ) ) 
      return( TRUE ) ;

   return( FALSE ) ;
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
USHORT __cdecl /*APIENTRY*/ EQFQUERYEXITINFO(  PSZ pszTagTable, // name of the markup table, e.g. "OTMHTM32"
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
