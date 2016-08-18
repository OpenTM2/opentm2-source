/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/*
*
*      OTMXML.C
*
*      FUNCTIONS:
*        EQFPRESEG
*        EQFPRESEG2
*        EQFPOSTSEG
*        EQFPOSTSEG2
*        EQFPOSTTM
*        EQFPOSTTM2
*        EQFSETSLIDER
*        EQFPREUNSEG
*        EQFPREUNSEG2
*        EQFPOSTUNSEG
*        EQFPOSTUNSEG2
*
*/
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#define MAXPATHLEN 2024

#define INCL_BASE
#define INCL_DOSFILEMGR

  #include "unicode.h"
  #define DosCopy(a, b , value) CopyFileA(a, b, FALSE);


int GetSourceFileFormat(char *source, PSZ markup);
int AddUTF8BOM(char *target, char *temp);
BOOL IsFileUTF16(char *source);

#include "parse.h"
#include "usrcalls.h"


#ifndef CCHMAXPATHCOMP
  #define CCHMAXPATHCOMP MAXPATHLEN
#endif



#define MAX_RCD_LENGTH         8192
#define MAX_TEXT_LENGTH        30000

#define  CODEPAGE_ERROR                  "File Conversion Error"
#define  ICONV_CODEPAGE_ERROR            "Error occured during ICONV code page conversionr"
#define  UNICODE_CODEPAGE_ERROR          "Error occured during code page conversion"
#define  CONTROLFILE_TITLE               "Analysis Failed"
#define  CONTROLFILE_ERROR               "Internal control file is missing.  TM or markup table is not properly installed."


#define SEGMENT_NONE           0
#define SEGMENT_NONTRANS       1
#define SEGMENT_TRANS          2

UCHAR       szErrMsg[256] ;
UCHAR       szErrTitle[80] ;
char        szProgPath[256] ;
ULONG       usSegNum ;
BOOL        bCodePageANSI ;                /* ANSI               */
BOOL        bCodePageHTML ;                /* HTML default       */
BOOL        bCodePageUTF8 ;                /* UTF-8              */
BOOL        bCodePageUTF8_BOM ;            /* UTF-8 with BOM     */
BOOL        bCodePageUTF16 ;               /* UTF-16             */
BOOL        bXHTML ;

extern   char    szDocTargetLanguage[80];   /* From USRCALLS.C  */
extern   char    szDocSourceLanguage[80];   /* From USRCALLS.C  */
extern   short   sTPVersion ;               /* From USRCALLS.C  */



/*
  Pre-segmentation, including access to progress window.
*/
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
    CHAR *Source, *Temp ;
    USHORT nFileFormat=0;

    BOOL bReturn = TRUE;
    char TempFile[512], TempFile2[512], TempFile3[512]  ;
    char *szAltTempExt1 = ".$$A";
    char *szAltTempExt2 = ".$$B";
    char *szAltTempExt3 = ".$$C";
    char  szMarkup[80] ;
    int rc;
    HINSTANCE hInstance, hInst_DLL;


    struct stat statBuf;



    PrepDocLanguageInfo( pSource ) ;   /* Set language unqiue processing, like DBCS */
    strcpy( szProgPath, pProgPath ) ;

    if (!pEdit) //For ITM processing
        strcpy(szDocTargetLanguage,szDocSourceLanguage);

    *pfNoSegment = FALSE;

    Source = strdup(pSource) ;

    
    CreateTempFileName2( TempFile, pSource, szAltTempExt1, TEMPNAME_SSOURCE ) ;
    strcpy( pTempSource,TempFile );
    CreateTempFileName2( TempFile2, pSource, szAltTempExt2, TEMPNAME_SSOURCE ) ;
    CreateTempFileName2( TempFile3, pSource, szAltTempExt3, TEMPNAME_SSOURCE ) ;
    
    bCodePageANSI = FALSE;
    bCodePageHTML = FALSE;
    bCodePageUTF8 = FALSE;
    bCodePageUTF16 = IsFileUTF16( pSource ) ;
    bXHTML = FALSE ;

    strcpy( szMarkup, pTagTable ) ;
    strupr( szMarkup ) ;

    if ( bCodePageUTF16 ) {
       DosCopy (pSource,TempFile3,DCPY_EXISTING);
    } else
    if ( ( ! strcmp( szMarkup, "OTMXUXML" ) ) ||     /* XML          - UTF-8 */
         ( ! strcmp( szMarkup, "OTMXUHTM" ) ) ||     /* XHTML        - UTF-8 */
         ( ! strcmp( szMarkup, "OTMXUSDL" ) ) ||     /* SDLXLIFF     - UTF-8 */
         ( ! strcmp( szMarkup, "OTMXUXLF" ) ) ) {    /* XLIFF        - UTF-8 */   
        //UTF-8
        bCodePageUTF8 = TRUE ;
        rc = ConvertImport(pSource, TempFile3, EQF_UTF82UTF16);
        if ( !rc ) {
            MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
            return FALSE;
        }
    } else 
    if ( ( ! strcmp( szMarkup, "OTMXAXML" ) ) ||     /* XML     - ANSI */
         ( ! strcmp( szMarkup, "OTMXAXLF" ) ) ||     /* XLIFF   - ANSI */   
         ( ! strcmp( szMarkup, "OTMXAHTM" ) ) ) {    /* XHTML   - ANSI */   
       bCodePageANSI = TRUE ;
       rc = ConvertImport(pSource, TempFile3, EQF_ANSI2UTF16);
       if ( !rc ) {
          MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
          return FALSE;
       }
    } else {
       nFileFormat = GetSourceFileFormat(pSource, pTagTable);
       if ( nFileFormat == 0) { //ascii
          rc = ConvertImport(pSource, TempFile3, EQF_ASCII2UTF16);
          if ( !rc ) {
             MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
             return FALSE;
          }
       } else 
       if ( nFileFormat == 1) { //ansi
          bCodePageANSI = TRUE ;
          rc = ConvertImport(pSource, TempFile3, EQF_ANSI2UTF16);
          if ( !rc ) {
             MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
             return FALSE;
          } 
       } else 
       if ( nFileFormat == 2) { //utf-8
          bCodePageUTF8 = TRUE ;
          rc = ConvertImport(pSource, TempFile3, EQF_UTF82UTF16);
          if ( !rc ) {
             MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
             return FALSE;
          }
       } else 
       if ( nFileFormat == 3) { //utf-16
          bCodePageUTF16 = TRUE ;
          DosCopy (pSource,TempFile3,DCPY_EXISTING);
       } else {
          if ( ( ! strcmp( szMarkup, "OTMXML"     ) ) ||
               ( ! strcmp( szMarkup, "OTMXMXLF"   ) ) ) {
             rc = ConvertImport(pSource, TempFile3, EQF_ASCII2UTF16);
             if ( !rc ) {
                MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
                return FALSE;
             }
          } else {
             MessageBoxA(HWND_DESKTOP, CONTROLFILE_ERROR, CONTROLFILE_TITLE, MB_OK);
             return FALSE;
          }
       }
    }


    PreParse(TempFile3, pTempSource, pTagTable, pSource, TempFile2, hSlider);

    rc=remove(TempFile2);
    rc=remove(TempFile3);

    return(bReturn);

}  /* EQFPRESEG2 */



/*
  Post-segmentation, including access to progress window.
*/
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTSEGW(
                    PSZ    pTagTable,
                    PSZ    pEdit,
                    PSZ    pProgPath,
                    PSZ    pSegSource,
                    PSZ    pTarget,
                    PTATAG_W      pTATag,
                    HWND        hSlider,
                    PEQF_BOOL     pfKill )
{

    BOOL  bReturn = TRUE;
    char  *szAltTempExt4 = ".$$D";
    char  OutFile[512];


    PrepDocLanguageInfo( pSegSource ) ;   /* Set language unique processing, like DBCS */
    strcpy( szProgPath, pProgPath ) ;

    CreateTempFileName2( OutFile, pSegSource, szAltTempExt4, TEMPNAME_STARGET ) ;

    bReturn = PostParse(pSegSource, OutFile, hSlider );
    DosCopy (OutFile,pSegSource,DCPY_EXISTING);
    DosCopy (OutFile,pTarget,DCPY_EXISTING);
    remove(OutFile);

    return(bReturn);
}  /* EQFPOSTSEG2 */


/*
   Pre-unsegmentation, including access to progress window.
*/
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPREUNSEGW(
                     PSZ    pTagTable,
                     PSZ    pEdit,
                     PSZ    pProgPath,
                     PSZ    pTarget,
                     PSZ    pTemp,
                     PTATAG_W      pTATag,
                     PEQF_BOOL     pfNoUnseg,
                     PEQF_BOOL     pfKill )
{

  return TRUE;
}  /* EQFPREUNSEG2 */


/*
   Post-unsegmentation, including access to progress window.
*/
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTUNSEGW(
                      PSZ    pTagTable,
                      PSZ    pEdit,
                      PSZ    pProgPath,
                      PSZ    pTarget,
                      PTATAG      pTATag,
                      PEQF_BOOL     pfKill )
{
    BOOL  bReturn = TRUE;
    char  szMarkup[80] ;
    char  szSourceFile[256] ;
    char  *szAltTempExt5 = ".$$E";
    char  TempFile[512];
    char  *ptr ;
    int rc;
    HINSTANCE hInst_DLL;


    PrepDocLanguageInfo( pTarget ) ;   /* Set language unqiue processing, like DBCS */
    strcpy( szProgPath, pProgPath ) ;
    strcpy( szMarkup, pTagTable ) ;
    strupr( szMarkup ) ;

    bXHTML = FALSE ;
    if ( ( ! strcmp( szMarkup, "OTMXHTML" ) ) ||     /* XHTML     - ASCII */
         ( ! strcmp( szMarkup, "OTMXAHTM" ) ) ||     /* XHTML     - ANSI  */
         ( ! strcmp( szMarkup, "OTMXUHTM" ) ) ) {    /* XHTML     - UTF-8 */   
       bXHTML = TRUE ;
    }

    
    CreateTempFileName2( TempFile, pTarget, szAltTempExt5, TEMPNAME_TARGET ) ;
    strcpy( szSourceFile, pTarget ) ;
    strupr( szSourceFile ) ;
    ptr = strstr( szSourceFile, "\\TARGET\\" ) ;
    if ( ptr ) 
       strncpy( ptr, "\\SOURCE\\", 8 ) ;
    else
       szSourceFile[0] = 0 ;

    bCodePageANSI = FALSE ; 
    bCodePageHTML = FALSE ; 
    bCodePageUTF8 = FALSE ;
    bCodePageUTF16 = IsFileUTF16( szSourceFile ) ;
    CopyFileA(pTarget, TempFile, FALSE);
    bReturn = PostExport( TempFile, pTarget, szMarkup ) ;
    
    /*   Override export conversion based on specific markup table names */
    if ( ! bCodePageUTF16 ) {
       if ( ( ! strcmp( szMarkup, "OTMXUXML" ) ) ||     /* XML       - UTF-8 */
            ( ! strcmp( szMarkup, "OTMXUHTM" ) ) ||     /* XHTML     - UTF-8 */
            ( ! strcmp( szMarkup, "OTMXUSDL" ) ) ||     /* SDLXLIFF  - UTF-8 */
            ( ! strcmp( szMarkup, "OTMXUXLF" ) ) ) {    /* XLIFF     - UTF-8 */   
          bCodePageUTF8 = TRUE ;
          bCodePageANSI = FALSE ; 
          bCodePageUTF16 = FALSE ;
       }
       if ( ( ! strcmp( szMarkup, "OTMXAXML" ) ) ||     /* XML     - ANSI */
            ( ! strcmp( szMarkup, "OTMXAXLF" ) ) ||     /* XLIFF   - ANSI */
            ( ! strcmp( szMarkup, "OTMXAHTM" ) ) ) {    /* XHTML   - ANSI */   
          bCodePageANSI = TRUE ;
          bCodePageUTF8 = FALSE ; 
          bCodePageUTF16 = FALSE ;
       }
    }

    if ( bCodePageANSI ) { //ANSI
        rc = ConvertExport(pTarget, EQF_UTF162ANSI);
        if( !rc ) {
            MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
            return FALSE;
        }
    }
    if ( bCodePageUTF8 ) { //UTF-8
        rc = ConvertExport(pTarget, EQF_UTF162UTF8);
        if( !rc ) {
            MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
            return FALSE;
        }
    }
    if ( bCodePageUTF16 ) { //UTF-16
    }
    if( !(bCodePageANSI || bCodePageHTML || bCodePageUTF8 || bCodePageUTF16) ) { //ASCII
        rc = ConvertExport(pTarget, EQF_UTF162ASCII);
        if( !rc ) {
            MessageBoxA(HWND_DESKTOP, UNICODE_CODEPAGE_ERROR, CODEPAGE_ERROR, MB_OK);
            return FALSE;
        }
    }

    remove( TempFile ) ;
    return(bReturn);

}  /* EQFPOSTUNSEG2 */



__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFQUERYEXITINFO(PSZ pszTagTable,  // name of the markup table, e.g. "OTMHTM32"
                        USHORT usMode,   // type of information being queried
                        PSZ pszBuffer,      // buffer area receiving the information returned by the exit
                        USHORT usBufLen // length of buffer area
)
{
    
    if( usMode == QUERYEXIT_ADDFILES) {
        QueryExportFiles(pszTagTable, pszBuffer, usBufLen, FALSE);
    }

    return 0;
}




/*******************************************************************************
*
*       function:       IsFileUTF16           
*
*       Description:
*               Determine if the file is encoded in UTF-16.      
*
*******************************************************************************/
BOOL IsFileUTF16(char *source) {
   FILE       *sourceFile_ptr;
   char       buff[MAX_TEXT_LENGTH+1];
   BOOL       bUTF16 = FALSE ;

   sourceFile_ptr = fopen(source, "r");
   if ( sourceFile_ptr ) {
      if ( fgets(buff, MAX_TEXT_LENGTH, sourceFile_ptr) != NULL) {
         if ( ( buff[0] == (CHAR)'\xFF' ) &&     /* Little endian        */
              ( buff[1] == (CHAR)'\xFE' ) ) {
            bUTF16 = TRUE ;
         }
      }
      fclose( sourceFile_ptr ) ;
   }

   return( bUTF16 ) ;
}




/*******************************************************************************
*
*       function:       GetSourceFileFormat
*
*       Description:
*               Determine the XML format for this file.          
*
*******************************************************************************/
int GetSourceFileFormat(char *source, PSZ markup) {
    FILE       *sourceFile_ptr;
    FILE       *fControl ;
    char       buff[MAX_TEXT_LENGTH+1];
    char       szDTD[512] ;       
    char       *dtd, *dtdext ;
    char       szRecordMB[512] ; //to hold multibyte
    char       szControlPath[256] ;
    char       *ptrCharMB, *ptrChar2MB ; //to hold multi-byte

    char       szRecord[512] ;
    char       *ptrChar, *ptrChar2 ;
    char       *ptrTag, *ptrText ;

    ULONG      ulRcdNum = 0 ;

    BOOL bIsInDocType;
    BOOL bFound;

    int nFileFormat=0;

    sourceFile_ptr = fopen(source, "r");
    strcpy(szDTD, markup);
    strcat(szDTD, ".DTD\"");
    dtd = szDTD;

    dtdext = strstr( szDTD, ".DTD\"" ) ;
    if ( ! dtdext )
       dtdext = strstr( szDTD, ".DTD\'" ) ;
    if ( dtdext ) {
       bFound = FALSE ;
       *(dtdext+4) = 0 ;
       for( dtd=dtdext ; dtd>szDTD && !strchr( "/\\\"\'", *dtd) ; --dtd ) ;
       if ( dtd > szDTD )
          ++dtd ;
       if ( strstr( dtd, "XHTML" ) ) {
          strcpy( dtd, "XHTML.DTD" ) ;
          dtdext = strstr( dtd, ".DTD" ) ;
       }
    }
    bIsInDocType = FALSE;
    while ( fgets(buff, MAX_TEXT_LENGTH, sourceFile_ptr) != NULL) {
       ++ulRcdNum ;

       if ( ( ulRcdNum == 1 ) &&
            ( buff[0] == (CHAR)'\xFF' ) &&     /* Little endian        */
            ( buff[1] == (CHAR)'\xFE' ) ) {
          nFileFormat = 3 ;
       }

       ptrChar =  strstr(buff,"<!DOCTYPE") ;
        if ( ptrChar ) {
           bIsInDocType = TRUE;
           if ( ptrChar != buff ) 
              memmove( buff, ptrChar, strlen(ptrChar)+1 ) ; 
        }
        if ( bIsInDocType && strstr(buff,"]>") ) {
            bIsInDocType = FALSE;
        }
        strupr(buff);

        if ( bIsInDocType && ( (strstr(buff,".DTD\"")) || (strstr(buff,".DTD\'")) ||
                               (strstr(buff,".dtd\"")) || (strstr(buff,".dtd\'")) ) ) {
            strncpy(szDTD, buff, sizeof(szDTD) );    
            szDTD[sizeof(szDTD)-1] = NULL ;
            strupr(szDTD);
            dtdext = strstr( szDTD, ".DTD\"" ) ;
            if ( ! dtdext )
                dtdext = strstr( szDTD, ".DTD\'" ) ;
            if ( dtdext ) {
                bFound = FALSE ;
                *(dtdext+4) = NULL ;
                for ( dtd=dtdext ; dtd>szDTD && !strchr( "/\\\"\'", *dtd) ; --dtd ) ;
                if ( dtd > szDTD )
                    ++dtd ;
                if ( strstr( dtd, "XHTML" ) ) {
                    strcpy( dtd, "XHTML.DTD" ) ;
                    dtdext = strstr( dtd, ".DTD" ) ;
                }
            }
            break;            
        }

        if ( strncmp(buff,"<XLIFF ",7) == 0 ) {
           strcpy(szDTD, "XLIFF.DTD") ;
           dtd = szDTD;
           dtdext = strstr( dtd, ".DTD" );
        }
    }
    fclose(sourceFile_ptr);
//##############################################################


    if ( dtdext ) {
        //Search for OTMXML.CTL file. This file contains the dtd name and the corresponding
        //markup table internal  control file.
//     _searchenv( "OTMXML.CTL", "PATH", szControlPath ); 
//     if ( ! szControlPath[0] ) {
//        strcpy( szControlPath, szProgPath ) ;
//        ptrChar = strrchr( szControlPath, '\\' ) ;
//        if ( ptrChar ) 
//           strcpy( ptrChar, "\\TABLE\\OTMXML.CTL" ) ;
//        else
//           szControlPath[0] = 0 ;
//     }
        GetOTMTablePath( szProgPath, szControlPath ) ;
        strcat( szControlPath, "OTMXML.CTL" ) ;
        
        //Finding the internal control file.
        if ( szControlPath[0] ) {
			
            fControl = fopen( szControlPath, "r" ) ;
            if ( fControl ) {
                while ( fgets( szRecordMB, sizeof(szRecordMB), fControl ) ) {
                    ptrCharMB = strtok( szRecordMB, " \n\t\r" ) ;
                    ptrChar2MB = strtok( NULL, " \n\t\r" ) ;
                    if ( ptrCharMB && ptrChar2MB && ( ! stricmp( ptrCharMB, dtd ) ) ) {
                        strcpy( szDTD, ptrChar2MB ) ;
                        dtd = szDTD ;
                        bFound = TRUE ;
                        break ;
                    }
                }
                fclose( fControl ) ;
            } else {
               nFileFormat = 9 ;            /* Terminating error */
            }
        } else {
           nFileFormat = 9 ;            /* Terminating error */
        }

        if ( ! bFound ) { 			               
           strcpy(szDTD, markup);
           strcat(szDTD, ".DTD");
           dtd = szDTD;
           //Search for OTMXML.CTL file. This file contains the dtd name and the corresponding
           //markup table internal  control file.
//         _searchenv( "OTMXML.CTL", "PATH", szControlPath ); 
//         if ( ! szControlPath[0] ) {
//            strcpy( szControlPath, szProgPath ) ;
//            ptrChar = strrchr( szControlPath, '\\' ) ;
//            if ( ptrChar ) 
//               strcpy( ptrChar, "\\TABLE\\OTMXML.CTL" ) ;
//            else
//               szControlPath[0] = 0 ;
//         }
           GetOTMTablePath( szProgPath, szControlPath ) ;
           strcat( szControlPath, "OTMXML.CTL" ) ;
         
           //Finding the internal control file.
           if ( szControlPath[0] ) {
         
              fControl = fopen( szControlPath, "r" ) ;
              if ( fControl ) {
                 while ( fgets( szRecordMB, sizeof(szRecordMB), fControl ) ) {
                    ptrCharMB = strtok( szRecordMB, " \n\t\r" ) ;
           	        ptrChar2MB = strtok( NULL, " \n\t\r" ) ;
           	        if ( ptrCharMB && ptrChar2MB && ( ! stricmp( ptrCharMB, dtd ) ) ) {
                       strcpy( szDTD, ptrChar2MB ) ;
                       dtd = szDTD ;
                       bFound = TRUE ;
                       break;
                    }
                 }
                 fclose( fControl ) ;
              } else {
                 nFileFormat = 9 ;            /* Terminating error */
              }
           } else {
              nFileFormat = 9 ;            /* Terminating error */
           }
        }



        GetOTMTablePath( szProgPath, szControlPath ) ;
        strcat( szControlPath, dtd ) ;
        if ( ! szControlPath[0] ) {
           strcpy( szControlPath, szProgPath ) ;
           ptrChar = strrchr( szControlPath, '\\' ) ;
           if ( ptrChar ) {
              strcpy( ptrChar, "\\TABLE\\" ) ;
              strcat( ptrChar, dtd ) ;
           } else
              szControlPath[0] = 0 ;
        }

        if ( szControlPath[0] ) {
           fControl = fopen( szControlPath, "r" ) ;          
           if ( fControl ) {
              strupr( szControlPath );
              while ( fgets( szRecord, sizeof(szRecord), fControl ) ) {
                 ptrChar = strchr( szRecord, '<' ) ;
                 ptrChar2 = strchr( szRecord, '>' ) ;
                 if ( ( ptrChar ) &&
                      ( ptrChar2 > ptrChar+1 ) &&
                      ( *(ptrChar+1) != '!' ) ) {
                    ptrTag = strtok( ptrChar+1, " <>\n\t\r" ) ;
                    ptrText = strtok( ptrChar2+1, "<\n\t\r" ) ;
                    if ( ptrTag ) {
                       if ( ptrText ) {
                           for ( ; ptrText && isspace(*ptrText) ; ++ptrText ) ;
                           for ( ptrChar=ptrText+strlen(ptrText)-1 ;
                               ptrChar>ptrText && isspace(*ptrChar) ;
                               *ptrChar=0, --ptrChar ) ;
                       }
                       /*--------------------------------------------------------*/
                       /*   If an <ANSI> tag, then import and export files are   */
                       /*   encoded in ANSI rather than ASCII.                   */
                       /*      <ANSI>  </ANSI>                                   */
                       /*--------------------------------------------------------*/
                       if ( !stricmp( ptrTag, "ANSI" ) ) {
                          if ( nFileFormat != 3 )               /* Not UTF-16    */
                             nFileFormat = 1; 
                       }
                       /*--------------------------------------------------------*/
                       /*   If an <UTF8> tag, then import and export files are   */
                       /*   encoded in UTF-8 rather than ASCII.                  */
                       /*      <UTF8>  </UTF8>                                   */
                       /*--------------------------------------------------------*/
                       else if ( !stricmp( ptrTag, "UTF8" ) ) {
                          if ( nFileFormat != 3 )               /* Not UTF-16    */
                             nFileFormat = 2;
                       }
                    }
                 }
              }
              fclose(fControl);
           } else {
              nFileFormat = 9 ;        /* Terminating error */
           }
        } else {
           nFileFormat = 9 ;            /* Terminating error */
        }
    }
    return nFileFormat;
}



/*******************************************************************************
*
*       function:       AddUTF8BOM         
*
*       Description:
*               Add the UTF-8 Byte Order Mark (BOM) to the file. 
*
*******************************************************************************/
int AddUTF8BOM(char *target, char *temp ) 
{
    FILE       *InFile ;
    FILE       *OutFile ;
    char       buff[MAX_TEXT_LENGTH+1];

    CopyFileA( target, temp, FALSE);

    InFile = fopen( temp, "rb" ) ;
    OutFile = fopen( target, "wb" ) ;

    fputs( "\xEF\xBB\xBF", OutFile ) ;

    while ( fgets(buff, MAX_TEXT_LENGTH, InFile) != NULL ) {
       fputs( buff, OutFile ) ;
    }

    fclose( InFile ) ;
    fclose( OutFile ) ;

    return( TRUE ) ;

}
