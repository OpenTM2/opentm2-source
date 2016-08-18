/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/*
*
* OTMXMODC
*
* Define TranslationManager/2 analysis program for OpenDocument XML files.
*
* FUNCTIONS:
*    EQFPRESEG
*      +-->EQFPRESEG2
*         (+-->PreParse)
*         (+-->OTMXML)
*    EQFPOSTSEG
*      +-->EQFPOSTSEG2
*    EQFPREUNSEG
*      +-->EQFPREUNSEG2
*         (+-->PostParse)
*         (+-->OTMXML)
*    EQFPOSTUNSEG
*      +-->EQFPOSTUNSEG2
*    EQFSETSLIDER
*
*/
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#include "otmxmodc.h"

#include <direct.h>


#define  ODC_EXTRACT_ERROR_TITLE             "Extract XML from OpenDocument Error"
#define  ODC_EXTRACT_FAILED                  "XML file %s could not be extracted from the OpenDocument file.\n\nUNZIP errors:\n"
#define  ODC_EXTRACT_MISSING_UNZIP           "UNZIP.EXE could not be found to extract the XML file from the OpenDocument file.\n\nInstall UNZIP.EXE in a directory which is in your PATH environment variable." 
#define  ODC_EXTRACT_SOURCE_PATH             "Source file path is not correct: %s"
#define  ODC_EXTRACT_CONCAT                  "XML file %s could not be concatenated to %s"

#define  ODC_REPLACE_ERROR_TITLE             "Replace XML into OpenDocument Error"
#define  ODC_REPLACE_FAILED                  "XML file %s could not be replaced into the OpenDocument file.\n\nZIP errors:\n" 
#define  ODC_REPLACE_MISSING_ZIP             "ZIP.EXE could not be found to update the XML file in the OpenDocument file.\n\nInstall ZIP.EXE in a directory which is in your PATH environment variable."
#define  ODC_REPLACE_TARGET_PATH             "Target file path is not correct: %s"
#define  ODC_REPLACE_OPEN                    "XML file could not be opened: %s"
#define  ODC_REPLACE_XML_FILE                "XML file could not be recreated: %s"
#define  ODC_REPLACE_SPLIT                   "XML file cannot be split into its individual components."

#define  ODC_BAD_FORMAT                      "Source file is not in a valid OpenDocument format." 


#define  ODC_CONTENT_XML                     "content.xml"
#define  ODC_META_XML                        "meta.xml"
#define  ODC_STYLES_XML                      "styles.xml"
#define  ODC_TEMP_XML                        "otmxmodc.tmp"

#define  ODC_SEPARATOR1_META_XML             "<!-- TWB meta.xml -->\n"
#define  ODC_SEPARATOR1_STYLES_XML           "<!-- TWB styles.xml -->\n"
#define  ODC_SEPARATOR1_FILE                 "<!-- TWB %s -->\n"

#define  ODC_SEPARATOR                       "<!-- TWB %s -->\n"
#define  ODC_SEPARATOR_COMMENT               "<!--"
#define  ODC_SEPARATOR_COMMENT_END           "-->"
#define  ODC_SEPARATOR_START                 "<!-- TWB "
#define  ODC_SEPARATOR_END                   " -->\n"

#define  ODC_STARTTAG_META_XML               "<office:meta>"
#define  ODC_STARTTAG_META_XML2              "<office:meta "                    /* 2-4-11 */
#define  ODC_STARTTAG_STYLES_XML             "<office:master-styles>"


char        szErrMsg[4096] ;
char        szErrTitle[80] ;
char        szZipExe[256] ;
char        szUnzipExe[256] ;


extern   char    szDocTargetLanguage[80];   /* From USRCALLS.C  */
extern   char    szDocSourceLanguage[80];   /* From USRCALLS.C  */
extern   short   sTPVersion ;               /* From USRCALLS.C  */


    BOOL       ExtractXmlFromZip(char*, char*, char*, char* );
    BOOL       ReplaceXmlInZip(char*, char*, char* );
    BOOL       ExecuteCommand( char*, char*, char* );
    BOOL       ConcatFiles( char*, char*, char* );
    BOOL       SplitFiles_1( char*, char*, char*, char*, char* );
    BOOL       SplitFiles_2( char*, char*, char* ) ;
    BOOL       FindFileType(char*, BOOL* );
    BOOL       FindSTargetType(char* );




/*******************************************************************************
*
*       function:       EQFPRESEG/EQFPRESEG2
*
* -----------------------------------------------------------------------------
*       Description:
*               called before text segmentation is invoked. Prepares the source
*               file for segmentation and stores the prepared file temporarily.
*               Text Analysis will remove the temporary file.
*               EQFPRESEG2 will pass a slider handle which could be used
*               together with the EQFSETSLIDER call to set the slider in cases
*               of user exits
*
*       Arguments:   PSZ    ... pointer to markup table name (full path)
*
*                    PSZ    ... pointer to editor name
*
*                    PSZ    ... pointer to program path
*
*                    PSZ    ... pointer to source file name  (full path)
*
*                    PSZ    ... pointer to the buffer for the name of the
*                               temp output file, the buffer is allocated by Text
*                               Analysis, with a size of 144 byte.
*
*                    BOOL * ... output flag which indicates whether text
*                               segmentation shall be performed,
*                               TRUE: do not perform Text Analysis,
*                                     i.e. no segmentation of the text
*                                     provided temp. file will be copied
*                                     as segmented source and segmented target
*                               FALSE: do perform text analysis
*
*                   HWND    ... handle of the slider window
*
*                   PBOOL   ... ptr to 'kill' flag
*                               if this flag changes to TRUE the userexit
*                               should return ASAP
*
*       Return:     BOOL    ... TRUE: processing was OK
*                               FALSE: an error occured during processing,
*                                      e.g. a file could not be written, so
*                                      that no further processing should be done
*
*******************************************************************************/
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPRESEG2(
                   PSZ       pTagTable,
                   PSZ       pEdit,
                   PSZ       pProgPath,
                   PSZ       pSource,
                   PSZ       pTempSource,
                   PEQF_BOOL pfNoSegment,
                   HWND      hSlider,
                   PEQF_BOOL pfKill           )
{

   BOOL bReturn = TRUE;
   char szxxx[4096] ;
   char szXmlFile[512];
   char szStyleFile[512];
   char szTempFile[512];
   char szTempFile2[512];
   char szTempFile3[512];
   char szDLLFile[512];
   char *szAltTempExt1 = ".$1$";
   char *szAltTempExt2 = ".$2$";
   char *szAltTempExt3 = ".$3$";
   char *szAltTempExt4 = ".$4$";
   char *ptrChar ;

   int     num, rc;
   PSZ     ModuleName = "OTMXML";
   PSZ     ProcName = "EQFPRESEG2";
   HINSTANCE  hInst_DLL;
   ULONG   ModuleType     = 0;

   typedef EQF_BOOL (* __stdcall XMLPRESEG_DCL)( PSZ, PSZ, PSZ, PSZ, PSZ, PEQF_BOOL, HWND, PEQF_BOOL ) ;
    XMLPRESEG_DCL  XMLPRESEG_addr;



   PrepDocLanguageInfo( pSource ) ;   /* Set language unqiue processing, like DBCS */

   szStyleFile[0] = 0 ;
   CreateTempFileName2( szXmlFile, pSource, szAltTempExt1, TEMPNAME_SOURCE ) ;
   CreateTempFileName2( szTempFile, pSource, szAltTempExt2, TEMPNAME_SOURCE ) ;
   CreateTempFileName2( szTempFile2, pSource, szAltTempExt3, TEMPNAME_SOURCE ) ;
   CreateTempFileName2( szTempFile3, pSource, szAltTempExt4, TEMPNAME_SOURCE ) ;

   if ( ! ExtractXmlFromZip( pSource, szXmlFile, szTempFile, szErrMsg ) ) {
      MessageBoxA(HWND_DESKTOP, szErrMsg, ODC_EXTRACT_ERROR_TITLE, MB_OK);
      bReturn = FALSE ;
   }


   if ( bReturn ) {
      strcpy( szStyleFile, pSource ) ;
      ptrChar = strstr( szStyleFile, "\\SOURCE\\" ) ;
      if ( ptrChar ) {
         strcpy( ptrChar, "\\RTF\\" ) ;
         _mkdir( szStyleFile ) ;                /* Create \EQF\...\RTF\         */
         ptrChar = strrchr( pSource, '\\' ) ;
         strcat( szStyleFile, ++ptrChar ) ;
      }
      bReturn = PreParse1( szXmlFile, szTempFile, szStyleFile, hSlider);

      if ( bReturn ) {
         remove( szTempFile2 ) ;
         rename( pSource, szTempFile2 ) ;
         rename( szTempFile, pSource ) ;

         *pfNoSegment = TRUE;

         /* Explicitly call an OTMXML.DLL function */
         rc = 0 ;

         GetOTMDllPath( pProgPath, szDLLFile ) ;
         strcat( szDLLFile, ModuleName ) ;
         hInst_DLL = LoadLibraryA( szDLLFile );
         if ( hInst_DLL != NULL ) {
            XMLPRESEG_addr = (XMLPRESEG_DCL)GetProcAddress((HMODULE)hInst_DLL, ProcName);
            if (XMLPRESEG_addr != NULL ) {
              rc = (*XMLPRESEG_addr)(pTagTable,pEdit,pProgPath,pSource,pTempSource,pfNoSegment,hSlider,pfKill);
            } 
            FreeLibrary( hInst_DLL ) ;
         } 
         remove( pSource ) ;
         rename( szTempFile2, pSource ) ;

         if ( rc == 0 ) {
            bReturn = FALSE ;
         } else {
            bReturn = PreParse2( pTempSource, szTempFile3, hSlider);
            if ( bReturn ) {
               DosCopy( szTempFile3, pTempSource, DCPY_EXISTING ) ;
            }
         }


      }
   }

   remove( szXmlFile ) ;
   remove( szTempFile ) ;
   remove( szTempFile2 ) ;
   remove( szTempFile3 ) ;
   if ( ( ! bReturn ) &&
        ( szStyleFile[0] ) ) 
      remove( szStyleFile ) ;

  return(bReturn);
}  /* EQFPRESEG2 */


/*******************************************************************************
*
*       function:       EQFPOSTSEG
*
* -----------------------------------------------------------------------------
*       Description:
*               called after text segmentation is invoked, to change the
*               segmented source and target file before translation takes place.
*               EQFPOSTSEG2 will pass a slider handle which could be used
*               together with the EQFSETSLIDER call to set the slider in cases
*               of user exits
*
*       Arguments:   PSZ    ... pointer to markup table name
*
*                    PSZ    ... pointer to editor name
*
*                    PSZ    ... pointer to program path
*
*                    PSZ    ... pointer to segmented source file name
*
*                    PSZ    ... pointer to the segmented target file name
*
*                    PTATAG ... pointer to tags inserted by text segmentation,
*                               ( see layout of tag structure )
*
*                   HWND    ... handle of the slider window
*
*                   PBOOL   ... ptr to 'kill' flag
*                               if this flag changes to TRUE the userexit
*                               should return ASAP
*
*       Return:     BOOL    ... TRUE: processing was OK
*                               FALSE: an error occured during processing
*
*       Note: It is vital that the name of the segmented source and target file
*              is not changed!
*       Post-segmentation, including access to progress window.               
*******************************************************************************/

__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTSEGW(
                    PSZ    pTagTable,
                    PSZ    pEdit,
                    PSZ    pProgPath,
                    PSZ    pSegSource,
                    PSZ    pSegTarget,
                    PTATAG_W      pTATag,
                    HWND        hSlider,
                    PEQF_BOOL     pfKill )
{
    BOOL bReturn = TRUE;
    char szTempFile[512];      /* Required to avoid abend */
    char szDLLFile[512];

    HINSTANCE  hInst_DLL;

    PSZ     ModuleName = "OTMXML.DLL" ;
    PSZ     ProcName = "EQFPOSTSEGW";
	typedef EQF_BOOL (* __cdecl /*APIENTRY*/ XMLPOSTSEG_DCL)( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG_W, HWND, PEQF_BOOL ) ;
	XMLPOSTSEG_DCL  XMLPOSTSEG_addr;
    int     rc ;

    /* processing steps go here if needed */

  // ============================================================
  /* Explicitly load OTMXML.DLL */

    if ( bReturn ) {
       rc = 0 ;
       GetOTMDllPath( pProgPath, szDLLFile ) ;
       strcat( szDLLFile, ModuleName ) ;
       hInst_DLL = LoadLibraryA( szDLLFile );
       if ( hInst_DLL != NULL ) {
          XMLPOSTSEG_addr = (XMLPOSTSEG_DCL)GetProcAddress((HMODULE)hInst_DLL, ProcName);
          if (XMLPOSTSEG_addr != NULL ) {
            rc = (*XMLPOSTSEG_addr)(pTagTable,pEdit,pProgPath,pSegSource,pSegTarget,pTATag,hSlider,pfKill);
          } 
          FreeLibrary( hInst_DLL ) ;
       } 
       if ( rc == 0 ) 
          bReturn = FALSE ;
    }

    return(bReturn);
}  /* EQFPOSTSEG2 */




/*******************************************************************************
*
*       function:       EQFPREUNSEG / EQFPREUNSEG2
*
* -----------------------------------------------------------------------------
*       Description:
*               called before unsegmentation is invoked, to change the
*               segmented target file and store the prepared file temporarily
*               before unsegmentation takes place.
*
*
*       Arguments:   PSZ    ... pointer to markup table name (full path)
*
*                    PSZ    ... pointer to editor name
*
*                    PSZ    ... pointer to program path
*
*                    PSZ    ... pointer to the segmented target file name (full path)
*
*                    PSZ    ... pointer to the buffer for the name of the
*                               temp output file, the buffer is allocated by Text
*                               Analysis, with a size of 144 bytes.
*
*                    PTATAG ... pointer to tags inserted by text segmentation
*
*                    BOOL * ... output flag whether unsegmentation is
*                               required
*                               TRUE: unsegmentation not required
*                               FALSE: unsegmentation required
*                               If unsegmentation is not required, only a file
*                               copy from the temp file to the target path
*                               will be done.
*
*                   PBOOL   ... ptr to 'kill' flag
*                               if this flag changes to TRUE the userexit
*                               should return ASAP
*
*       Return:     BOOL    ... TRUE: processing was OK
*                               FALSE: an error occured during processing
*
*******************************************************************************/
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPREUNSEGW(
                     PSZ         pTagTable,
                     PSZ         pEdit,
                     PSZ         pProgPath,
                     PSZ         pSegTarget,
                     PSZ         pTemp,
                     PTATAG_W      pTATag,
                     PEQF_BOOL   pfNoUnseg,
                     PEQF_BOOL   pfKill )
{
   BOOL bReturn = TRUE;
   char szTempFile[512];      /* Required to avoid abend */
   char szDLLFile[512];


   HINSTANCE  hInst_DLL;

   PSZ     ModuleName = "OTMXML" ;
   PSZ     ProcName = "EQFPREUNSEGW";
   typedef EQF_BOOL (* __cdecl /*APIENTRY*/ XMLPREUNSEG_DCL)( PSZ, PSZ, PSZ, PSZ, PSZ, PTATAG_W, PEQF_BOOL, PEQF_BOOL ) ;
   XMLPREUNSEG_DCL  XMLPREUNSEG_addr;
   USHORT  rc ;

   /* processing steps go here if needed */

// ============================================================
 /* Explicitly load OTMXML.DLL */

   rc = 0 ;
   GetOTMDllPath( pProgPath, szDLLFile ) ;
   strcat( szDLLFile, ModuleName ) ;
   hInst_DLL = LoadLibraryA( szDLLFile );
   if ( hInst_DLL != NULL ) {
      XMLPREUNSEG_addr = (XMLPREUNSEG_DCL)GetProcAddress((HMODULE)hInst_DLL, ProcName);
      if (XMLPREUNSEG_addr != NULL ) {
        rc = (*XMLPREUNSEG_addr)(pTagTable,pEdit,pProgPath,pSegTarget,pTemp,pTATag,pfNoUnseg,pfKill);
      } 
      FreeLibrary( hInst_DLL ) ;
   } 
   if ( rc == 0 ) 
      bReturn = FALSE ;

   return(bReturn);
}  /* EQFPREUNSEG2 */


/*******************************************************************************
*
*       function:       EQFPOSTUNSEG
*
* -----------------------------------------------------------------------------
*       Description:
*               called after unsegmentation is invoked, to change the
*               target file to its final form.
*
*
*       Arguments:   PSZ    ... pointer to markup table name (full path)
*
*                    PSZ    ... pointer to editor name
*
*                    PSZ    ... pointer to program path (full path)
*
*                    PSZ          ... pointer to target file name (full path)
*
*                    PTATAG       ... pointer to tags inserted by text segmentation
*
*                   PBOOL   ... ptr to 'kill' flag
*                               if this flag changes to TRUE the userexit
*                               should return ASAP
*
*
*       Return:     BOOL          ... TRUE: processing was OK
*                                     FALSE: an error occured during processing
*
*******************************************************************************/
__declspec(dllexport)
EQF_BOOL __cdecl /*APIENTRY*/ EQFPOSTUNSEGW(
                      PSZ    pTagTable,
                      PSZ    pEdit,
                      PSZ    pProgPath,
                      PSZ    pTarget,
                      PTATAG      pTATag,
                      PEQF_BOOL     pfKill )
{
// ============================================================
    BOOL bReturn = TRUE;
    char szStyleFile[512];
    char szTempFile[512];      /* Required to avoid abend */
    char szTempFile2[512];  
    char szDLLFile[512];
    char *ptrChar ;

    char *szAltTempExt1 = ".$6$";

    PSZ     ModuleName = "OTMXML" ;
    PSZ     ProcName = "EQFPOSTUNSEGW";
    HINSTANCE  hInst_DLL;
    USHORT     rc ;

	typedef EQF_BOOL (* __cdecl /*APIENTRY*/ XMLPOSTUNSEG_DCL)( PSZ, PSZ, PSZ, PSZ, PTATAG, PEQF_BOOL ) ;
	XMLPOSTUNSEG_DCL  XMLPOSTUNSEG_addr;
// ============================================================


    PrepDocLanguageInfo( pTarget ) ;   /* Set language unqiue processing, like DBCS */

    CreateTempFileName2( szTempFile2, pTarget, szAltTempExt1, TEMPNAME_TARGET ) ;


  /* Explicitly load OTMXML.DLL */
    rc = 0 ;
    GetOTMDllPath( pProgPath, szDLLFile ) ;
    strcat( szDLLFile, ModuleName ) ;
    hInst_DLL = LoadLibraryA( szDLLFile );
    if ( hInst_DLL != NULL ) {
       XMLPOSTUNSEG_addr = (XMLPOSTUNSEG_DCL)GetProcAddress((HMODULE)hInst_DLL, ProcName);
       if (XMLPOSTUNSEG_addr != NULL ) {
          rc = (*XMLPOSTUNSEG_addr)(pTagTable,pEdit,pProgPath,pTarget,pTATag,pfKill);
       } 
     FreeLibrary( hInst_DLL ) ;
    } 
    if ( rc == 0 ) 
       bReturn = FALSE ;


    if ( bReturn ) {
       strcpy( szStyleFile, pTarget ) ;
       ptrChar = strstr( szStyleFile, "\\TARGET\\" ) ;
       if ( ptrChar ) {
          strcpy( ptrChar, "\\RTF\\" ) ;
          ptrChar = strrchr( pTarget, '\\' ) ;
          strcat( szStyleFile, ++ptrChar ) ;
       } else {
          szStyleFile[0] = 0 ;
       }
       bReturn = PostUnseg( pTarget, szStyleFile );
    }
    if ( bReturn ) {
       if ( ! ReplaceXmlInZip( pTarget, szTempFile2, szErrMsg ) ) {
          MessageBoxA(HWND_DESKTOP, szErrMsg, ODC_REPLACE_ERROR_TITLE, MB_OK);
          bReturn = FALSE ;
       }
    }

    remove( szTempFile2 ) ;

    return(bReturn);
}  /* EQFPOSTUNSEGW */





/*****************************************************************************/
/*  ExtractXmlFromZip                                                        */
/*                                                                           */
/*  Extract the XML files from the ZIP file so that the translatable text    */
/*  can be identified.                                                       */
/*                                                                           */
/* Input:      InFile        - Input ZIP file to extract from.               */
/*             OutFile       - Output XML file.                              */
/*             TempFile      - Temporary work file to use.                   */
/*             ErrText       - Any error text to be shown to user.           */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be processed.                            */
/*****************************************************************************/

BOOL ExtractXmlFromZip( char *InFile, char *OutFile, char *TempFile, char *ErrText )
{
    FILE      *fTemp ;
    struct stat  FileInfo ;
    char      szIn[MAX_RCD_LENGTH] ;
    char      szCommand[1024] ;
    char      szTempDir1[256] ;
    char      szTempDir2[256] ;
    char      szTempFile[256] ;
    char      *ptrChar ;
    USHORT    rc ;
    DWORD     dwCode;
    BOOL      bFormatXml = FALSE ;
    BOOL      bReturn = TRUE;



    /*-----------------------------------------------------------------------*/
    /*  Determine input file format, whether ZIP file or CONTENT.XML         */
    /*-----------------------------------------------------------------------*/
    if ( FindFileType( InFile, &bFormatXml ) ) {
       if ( ! bFormatXml ) {
          GetOTMDllPath( InFile, szUnzipExe) ;
          strcat( szUnzipExe, "UNZIP.EXE" ) ;
          rc = stat( szUnzipExe, &FileInfo ) ;
          if ( rc != 0 ) {
             strcpy( ErrText, ODC_EXTRACT_MISSING_UNZIP ) ;
             bReturn = FALSE ;
          }
       }
    } else {
       strcpy( ErrText, ODC_BAD_FORMAT ) ;
       bReturn = FALSE ;
    }

    if ( bReturn ) {

       /*-----------------------------------------------------------------------*/
       /*  File is already in XML format.                                       */
       /*-----------------------------------------------------------------------*/
       if ( bFormatXml ) {
          DosCopy( InFile, OutFile, DCPY_EXISTING ) ;
       } else {
          /*--------------------------------------------------------------------*/
          /*  XML files must be extracted from the document ZIP file.           */
          /*    1.  Create temporary directories.                               */
          /*    2.  Unzip CONTENT.XML into temporary directory.                 */
          /*    3.  Save copy of CONTENT.XML for segmentation.                  */
          /*    4.  Unzip META.XML into temporary directory.                    */
          /*    5.  Concatenate META.XML to end of CONTENT.XML.                 */
          /*    6.  Unzip STYLE.XML into temporary directory.                   */
          /*    7.  Concatenate STYLE.XML to end of CONTENT.XML.                */
          /*    8.  Delete temporary directories.                               */
          /*--------------------------------------------------------------------*/
          strcpy( szTempDir1, InFile ) ;
          ptrChar = strstr( szTempDir1, "\\SOURCE\\" ) ;
          if ( ptrChar ) {
             /*-----------------------------------------------------------------*/
             /*  Create temporary directories.                                  */
             /*-----------------------------------------------------------------*/
             strcpy( ptrChar, "\\MISC\\" ) ;
             _mkdir( szTempDir1 ) ;             /* Create \EQF\...\MISC\        */
             strcpy( szTempDir2, szTempDir1 ) ;
             ptrChar = strrchr( InFile, '\\' ) ;
             strcat( szTempDir2, ++ptrChar ) ;
             strcat( szTempDir2, "\\" ) ;
             _mkdir( szTempDir2 ) ;             /* Create \EQF\...\MISC\...\    */

             /*-----------------------------------------------------------------*/
             /*  Unzip all of the CONTENT.XML, META.XML, STYLES.XML files from  */
             /*  the ZIP file and save them.                            2-29-12 */
             /*-----------------------------------------------------------------*/
             sprintf( szCommand, "\"%s\" -q -o %s *%s *%s -d %s 2> %s", 
                                 szUnzipExe, InFile, 
                                 ODC_CONTENT_XML, ODC_META_XML,
                                 szTempDir2, TempFile ) ;
             sprintf( ErrText, ODC_EXTRACT_FAILED, ODC_CONTENT_XML ) ;
             if ( ExecuteCommand( szCommand, TempFile, ErrText ) ) {
                sprintf( szCommand, "\"%s\" -q -o %s *%s  -d %s 2> %s",  /* STYLES.XML on for non-ODF files */
                                    szUnzipExe, InFile, ODC_STYLES_XML,
                                    szTempDir2, TempFile ) ;
                ExecuteCommand( szCommand, TempFile, ErrText );

                /*--------------------------------------------------------------*/
                /*  Concatenate all of the XML files together into 1 file.      */
                /*--------------------------------------------------------------*/
                remove( TempFile ) ;
                sprintf( szCommand, "cmd /C dir /s /b %s*.xml > %s", szTempDir2, TempFile ) ;
                ExecuteCommand( szCommand, TempFile, ErrText ) ;
                fTemp = fopen( TempFile, "r" ) ;
                if ( fTemp ) {
                   bReturn = TRUE ;
                   remove( OutFile ) ;
                   while ( fgets( szIn, MAX_RCD_LENGTH, fTemp ) != NULL ) {
                      strtok( szIn, "\n" ) ;
                      strcpy( szTempFile, &szIn[strlen(szTempDir2)] ) ;
                      if ( bReturn ) {
                         bReturn = ConcatFiles( OutFile, szIn, szTempFile ) ;
                         if ( ! bReturn ) {
                            sprintf( ErrText, ODC_EXTRACT_CONCAT, szIn, OutFile ) ;
                         }
                      }
                      remove( szIn ) ;
                      while( ( ptrChar=strrchr( szIn, '\\' ) ) &&
                             ( ptrChar-szIn > strlen(szTempDir2) ) ) {
                         *ptrChar = 0 ;
                         _rmdir( szIn ) ;
                      }
                   }
                   fclose( fTemp ) ;
                } else {
                   bReturn = FALSE ;
                   strcpy( ErrText, ODC_EXTRACT_FAILED ) ;
                }
             } else {
                bReturn = FALSE ;
             }


       ///   remove( szTempFile ) ;
             _rmdir( szTempDir2 ) ;
             _rmdir( szTempDir1 ) ;
          } else {
             sprintf( ErrText, ODC_EXTRACT_SOURCE_PATH, szTempDir1 ) ;
             bReturn = FALSE ;
          }
       }
    }

   return( bReturn ) ;
}



/*****************************************************************************/
/*  ReplaceXmlInZip                                                          */
/*                                                                           */
/*  Replace the XML files into the ZIP file after translation.               */
/*                                                                           */
/* Input:      TargetFile    - Input CONTENT.XML. Output updated ZIP file.   */
/*             TempFile      - Temporary work file to use.                   */
/*             ErrText       - Any error text to be shown to user.           */
/*                                                                           */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be processed.                            */
/*****************************************************************************/

BOOL ReplaceXmlInZip( char *TargetFile, char *TempFile, char *ErrText )
{
    STARTUPINFOA           StartupInfo ;
    PROCESS_INFORMATION    piProcessInfo ; 
    FILE      *fTemp ;
    struct stat  FileInfo ;
    char      szSourceFile[256] ;
    char      szCommand[1024] ;
    char      szTempDir1[256] ;
    char      szTempDir2[256] ;
    char      szTempFile[256] ;
    char      szTempZip[256] ;
    char      szSaveDir[256] ;
    char      *ptrChar ;
    USHORT    rc ;
    DWORD     dwCode;
    BOOL      bFormatXml = FALSE ;
    BOOL      bReturn = TRUE;


    /*-----------------------------------------------------------------------*/
    /*  Determine input file format, whether ZIP file or CONTENT.XML         */
    /*-----------------------------------------------------------------------*/
    strcpy( szSourceFile, TargetFile ) ;
    ptrChar = strstr( szSourceFile, "\\TARGET\\" ) ;
    if ( ptrChar ) {
       strncpy( ptrChar, "\\SOURCE\\", 8 ) ;
       if ( FindFileType( szSourceFile, &bFormatXml ) ) {
          if ( ! bFormatXml ) {
             GetOTMDllPath( TargetFile, szZipExe) ;
             strcat( szZipExe, "ZIP.EXE" ) ;
             rc = stat( szZipExe, &FileInfo ) ;
             if ( rc != 0 ) {
                strcpy( ErrText, ODC_REPLACE_MISSING_ZIP ) ;
                bReturn = FALSE ;
             }
             GetOTMDllPath( TargetFile, szUnzipExe) ;
             strcat( szUnzipExe, "UNZIP.EXE" ) ;
             rc = stat( szUnzipExe, &FileInfo ) ;
             if ( rc != 0 ) {
                strcpy( ErrText, ODC_EXTRACT_MISSING_UNZIP ) ;
                bReturn = FALSE ;
             }
          }
       } else {
          strcpy( ErrText, ODC_BAD_FORMAT ) ;
          bReturn = FALSE ;
       }
    } else {
       sprintf( ErrText, ODC_REPLACE_TARGET_PATH, TargetFile ) ;
       bReturn = FALSE ;
    }

    if ( bReturn ) {
       /*-----------------------------------------------------------------------*/
       /*  File is already in XML format.  No action necessary.                 */
       /*-----------------------------------------------------------------------*/
       if ( bFormatXml ) {
       } else {
          /*--------------------------------------------------------------------*/
          /*  XML files must be replaced into the original document ZIP file.   */
          /*    1.  Create temporary directories.                               */
          /*    2.  Copy target file to CONTENT.XML in temporary directory.     */
          /*    3.  Copy target ZIP to same name in temporary directory.        */
          /*    4.  Zip temp dir CONTENT.XML into temp dir ZIP file.            */
          /*    5.  Copy temp dir ZIP file to output file.                      */
          /*    6.  Delete temporary directories.                               */
          /*--------------------------------------------------------------------*/
          strcpy( szTempDir1, TargetFile ) ;
          ptrChar = strstr( szTempDir1, "\\TARGET\\" ) ;
          if ( ptrChar ) {
             /*-----------------------------------------------------------------*/
             /*  Create temporary directories.                                  */
             /*-----------------------------------------------------------------*/
             strcpy( ptrChar, "\\MISC\\" ) ;
             _mkdir( szTempDir1 ) ;             /* Create \EQF\...\MISC\        */
             strcpy( szTempDir2, szTempDir1 ) ;
             ptrChar = strrchr( TargetFile, '\\' ) ;
             strcat( szTempDir2, ++ptrChar ) ;
             strcat( szTempDir2, "\\" ) ;
             _mkdir( szTempDir2 ) ;             /* Create \EQF\...\MISC\...\    */

             strcpy( szTempFile, szTempDir2 ) ;
             strcat( szTempFile, ODC_TEMP_XML ) ;
             DosCopy( TargetFile, szTempFile, DCPY_EXISTING ) ;
             strcpy( szTempZip, szTempDir2 ) ;
             strcat( szTempZip, ptrChar ) ;
             DosCopy( szSourceFile, szTempZip, DCPY_EXISTING ) ;

             /*-----------------------------------------------------------------*/
             /*  Split the one XML file into its individual files.              */
             /*-----------------------------------------------------------------*/
             if ( FindSTargetType( TargetFile ) ) {
                bReturn = SplitFiles_1( TargetFile, szTempDir2, szSourceFile, TempFile, ErrText ) ;
                if ( bReturn ) {
                  /*-----------------------------------------------------------*/
                  /*  Update the files in the ZIP file.                        */
                  /*-----------------------------------------------------------*/
                   sprintf( szCommand, "\"%s\"  -j %s %s*.xml > %s", 
                            szZipExe, szTempZip, szTempDir2, TempFile ) ;
                   sprintf( ErrText, ODC_REPLACE_FAILED, ODC_CONTENT_XML ) ;
                   bReturn = ExecuteCommand( szCommand, TempFile, ErrText ) ;
                   if ( bReturn ) {
                      DosCopy( szTempZip, TargetFile, DCPY_EXISTING ) ;
                   }
                }
             } else {
                bReturn = SplitFiles_2( TargetFile, szTempDir2, ErrText ) ;
                if ( bReturn ) {
                   /*-----------------------------------------------------------*/
                   /*  Update the files in the ZIP file.                        */
                   /*-----------------------------------------------------------*/
                   _getcwd( szSaveDir, sizeof(szSaveDir) ) ;
                   chdir( szTempDir2 ) ;
                   sprintf( szCommand, "\"%s\"  -r -f %s *.xml > %s", 
                            szZipExe, szTempZip, TempFile ) ;
                   strcpy( ErrText, ODC_REPLACE_FAILED ) ;
                   bReturn = ExecuteCommand( szCommand, TempFile, ErrText ) ;
                   if ( bReturn ) {
                      DosCopy( szTempZip, TargetFile, DCPY_EXISTING ) ;
                   }
                   chdir( szSaveDir ) ;
                }
             }

             /*-----------------------------------------------------------------*/
             /*  Clean-up.                                                      */
             /*-----------------------------------------------------------------*/
             //      remove( szTempFile ) ;
             //      remove( szTempZip ) ;
             //      strcpy( szTempFile, szTempDir2 ) ;
             //      strcat( szTempFile, ODC_CONTENT_XML ) ;
             //      remove( szTempFile ) ;
             //      strcpy( szTempFile, szTempDir2 ) ;
             //      strcat( szTempFile, ODC_META_XML ) ;
             //      remove( szTempFile ) ;
             //      strcpy( szTempFile, szTempDir2 ) ;
             //      strcat( szTempFile, ODC_STYLES_XML ) ;
             //      remove( szTempFile ) ;
             //
             //      _rmdir( szTempDir2 ) ;
             //      _rmdir( szTempDir1 ) ;

             remove( szTempZip ) ;
             sprintf( szCommand, "cmd /C rd /s /q %s > %s", szTempDir2, TempFile ) ;
             ExecuteCommand( szCommand, TempFile, NULL ) ;
          } else {
             sprintf( ErrText, ODC_REPLACE_TARGET_PATH, szTempDir1 ) ;
             bReturn = FALSE ;
          }
       }
    }

   return( bReturn ) ;
}



/*****************************************************************************/
/*  ExecuteCommand                                                           */
/*                                                                           */
/*  Execute the ZIP or UNZIP command.                                        */
/*                                                                           */
/* Input:      Command       - Command string to execute.                    */
/*             ErrFile       - Temporary file to capture error messages.     */
/* Output:     ErrText       - Error message if failure.                     */
/*                                                                           */
/*  Return:  TRUE  - Action was successful.                                  */
/*           FALSE - Action failed.                                          */
/*****************************************************************************/

BOOL ExecuteCommand( char *Command, char *ErrFile, char *ErrText )
{
    STARTUPINFOA           StartupInfo ;
    PROCESS_INFORMATION    piProcessInfo ; 
    FILE                   *fTemp ;
    CHAR                   szText[256] ;
    USHORT                 rc ;
    DWORD                  dwCode;
    BOOL                   bReturn = FALSE ;

    /*-----------------------------------------------------------------------*/
    /*  Execute command so that DOS window does not pop up.                  */
    /*-----------------------------------------------------------------------*/
    GetStartupInfoA( &StartupInfo ) ;
    StartupInfo.dwFlags = STARTF_USESHOWWINDOW ;
    StartupInfo.wShowWindow = SW_HIDE ;
    bReturn = CreateProcessA( NULL, Command, NULL, NULL, FALSE, (DWORD)0, NULL, NULL,
                   &StartupInfo, &piProcessInfo ) ; 
    WaitForSingleObject( piProcessInfo.hProcess, INFINITE ) ;
    rc = GetExitCodeProcess(piProcessInfo.hProcess, &dwCode);

    if ( dwCode == 0 ) {
       bReturn = TRUE ;
    } else {
       bReturn = FALSE ;
       strcat( ErrText, Command ) ;
       strcat( ErrText, "\n" ) ;
       fTemp = fopen( ErrFile, "r" ) ; 
       if ( fTemp ) {
          while( fgets( szText, sizeof(szText), fTemp ) != NULL ) {
             strcat( ErrText, szText ) ;
          }
          fclose( fTemp ) ;
       }
    }

    return( bReturn ) ;
}



/*****************************************************************************/
/*  ConcatFiles                                                              */
/*                                                                           */
/*  Concatenate the contents of one file to the end of another file.         */
/*                                                                           */
/* Input:      BaseFile      - File to append to.                            */
/*             FromFile      - File to copy from.                            */
/*             bCopyAll      - TRUE=Copy all records.                        */
/*                                                                           */
/*  Return:  TRUE  - Action was successful.                                  */
/*           FALSE - Action failed.                                          */
/*****************************************************************************/

BOOL ConcatFiles( char *BaseFile, char *FromFile, char *FromFileName )
{
    FILE       *fConcat, *fFrom ;
    CHAR       szIn[MAX_RCD_LENGTH*2] ;
    CHAR       szStartTag[80] ;
    CHAR       szStartTag2[80] ;
    CHAR       szSeparator[80] ;
    CHAR       *ptrChar, *ptrChar2 ;
    USHORT     i ;
    BOOL       bCopyRcd = FALSE ;
    BOOL       bReturn = FALSE ;


    fConcat = fopen( BaseFile, "a" ) ;
    fFrom = fopen( FromFile, "r" ) ;
    if ( ( fConcat ) &&
         ( fFrom   ) ) {
  //   if ( strstr( FromFileName, ODC_META_XML ) ) {
  //      strcpy( szStartTag, ODC_STARTTAG_META_XML ) ;
  //      strcpy( szStartTag2, ODC_STARTTAG_META_XML2 ) ;          /* 2-4-11 */
  ////    strcpy( szSeparator, ODC_SEPARATOR_META_XML ) ;
  //   } else 
  //   if ( strstr( FromFileName, ODC_STYLES_XML ) ) {
  //      strcpy( szStartTag, ODC_STARTTAG_STYLES_XML ) ;
  //      szStartTag2[0] = 0 ;
  ////    strcpy( szSeparator, ODC_SEPARATOR_STYLES_XML ) ;
  //   } else {
  //      bCopyRcd = TRUE ;
  //      bReturn = TRUE ;
  //   }
       sprintf( szSeparator, ODC_SEPARATOR, FromFileName ) ;

       while( GetRcd( szIn, MAX_RCD_LENGTH, fFrom, TRUE ) != NULL ) {
          if ( ! bCopyRcd ) {
  //         ptrChar = strstr( szIn, szStartTag ) ;
  //         if ( ( ! ptrChar ) &&                                 /* 2-4-11 */
  //              ( szStartTag2[0] ) ) 
  //            ptrChar = strstr( szIn, szStartTag2 ) ;
  //         if ( ptrChar ) {
  //            if ( ptrChar != szIn ) 
  //               memmove( szIn, ptrChar, strlen(ptrChar)+1 ) ;
                bCopyRcd = TRUE ;
                bReturn = TRUE ;
                fputs( szSeparator, fConcat ) ;
  //           }
          }
          if ( bCopyRcd )
             fputs( szIn, fConcat ) ;
       }
    }

    fclose( fConcat ) ;
    fclose( fFrom ) ;

    return( bReturn ) ;
}



/*****************************************************************************/
/*  SplitFiles_1                                                             */
/*                                                                           */
/*  Split the CONTENT.XML file into these parts:                             */
/*       CONTENT.XML                                                         */
/*       META.XML                                                            */
/*       STYLES.XML                                                          */
/*                                                                           */
/* Input:      ContentFile   - File to split into its components.            */
/*             OutputDir     - Output directory to save split files into.    */
/*             SourceZip     - Source ZIP file to get start of split files.  */
/* Output:     ErrText       - Error message if failure.                     */
/*                                                                           */
/*  Return:  TRUE  - Action was successful.                                  */
/*           FALSE - Action failed.                                          */
/*****************************************************************************/

BOOL SplitFiles_1( char *ContentFile, char *OutputDir, char *SourceZip,
                   char *TempFile, char *ErrText )
{
    FILE       *fSplit, *fContent, *fIn, *fOut ;
    CHAR       szIn[MAX_RCD_LENGTH*2] ;
    CHAR       szIn2[MAX_RCD_LENGTH*2] ;
    CHAR       szOutputFile[256] ;
    CHAR       szTempFile[256] ;
    char       szCommand[1024] ;
    CHAR       *ptrChar ;
    USHORT     usFileType ;
    USHORT     rc ;
    BOOL       bReturn = TRUE ;


    fSplit = fopen( ContentFile, "r" ) ;
    strcpy( szOutputFile, OutputDir ) ;
    strcat( szOutputFile, ODC_CONTENT_XML ) ;
    fOut = fopen( szOutputFile, "w" ) ;
    if ( ( ! fSplit ) ||
         ( ! fOut   ) ) {
       if ( fSplit ) 
          fclose( fSplit ) ;
       if ( fOut ) 
          fclose( fOut ) ;
       sprintf( ErrText, ODC_REPLACE_OPEN, ODC_CONTENT_XML ) ;
       return( FALSE ) ;  
    }
    usFileType = 1 ;

    /*-----------------------------------------------------------------*/
    /*  Copy the records for the CONTENT.XML file.                     */
    /*-----------------------------------------------------------------*/
    while( GetRcd( szIn, MAX_RCD_LENGTH, fSplit, TRUE ) != NULL ) {
       ptrChar = strstr( szIn, ODC_SEPARATOR1_META_XML ) ;
       if ( ptrChar ) {
          *ptrChar = 0 ;
          fputs( szIn, fOut ) ;
          ptrChar += strlen(ODC_SEPARATOR1_META_XML) ;
          if ( *ptrChar ) 
             memmove( szIn, ptrChar, strlen(ptrChar)+1 ) ;
          else 
             szIn[0] = 0 ;
          usFileType = 2 ;
          break ;
       }
       ptrChar = strstr( szIn, ODC_SEPARATOR1_STYLES_XML ) ;
       if ( ptrChar ) {
          *ptrChar = 0 ;
          fputs( szIn, fOut ) ;
          ptrChar += strlen(ODC_SEPARATOR1_STYLES_XML) ;
          if ( *ptrChar ) 
             memmove( szIn, ptrChar, strlen(ptrChar)+1 ) ;
          else 
             szIn[0] = 0 ;
          usFileType = 3 ;
          break;
       }
       fputs( szIn, fOut ) ;
    }
    fclose( fOut ) ;

    /*-----------------------------------------------------------------*/
    /*  Copy the records for the META.XML file.                        */
    /*-----------------------------------------------------------------*/
    if ( usFileType == 2 ) {
       strcpy( szOutputFile, OutputDir ) ;
       strcat( szOutputFile, ODC_META_XML ) ;

       /*--------------------------------------------------------------*/
       /*  Unzip source META.XML.                                      */
       /*--------------------------------------------------------------*/
       sprintf( szCommand, "\"%s\" -q -o %s %s -d %s > %s", 
                           szUnzipExe, SourceZip, ODC_META_XML, 
                           OutputDir, TempFile ) ;
       sprintf( ErrText, ODC_EXTRACT_FAILED, ODC_META_XML ) ;
       bReturn = ExecuteCommand( szCommand, TempFile, ErrText ) ;
       if ( bReturn ) {
          DosCopy( szOutputFile, TempFile, DCPY_EXISTING ) ;

             /*-----------------------------------------------------------*/
             /*  Copy 1st unchanged part of META.XML file.                */
             /*-----------------------------------------------------------*/
          fOut = fopen( szOutputFile, "w" ) ;
          fIn = fopen( TempFile, "r" ) ;
          if ( ( fIn ) &&
               ( fOut ) ) {
             while( GetRcd( szIn2, MAX_RCD_LENGTH, fIn, TRUE ) != NULL ) {
                ptrChar = strstr( szIn2, ODC_STARTTAG_META_XML ) ;
                if ( ! ptrChar )                                /* 2-4-11 */
                   ptrChar = strstr( szIn2, ODC_STARTTAG_META_XML2 ) ;
                if ( ptrChar ) {
                   *ptrChar = 0 ;
                   fputs( szIn2, fOut ) ;
                   break ;
                }
                fputs( szIn2, fOut ) ;
             }
             fclose( fIn ) ;
          } else {
             sprintf( ErrText, ODC_REPLACE_OPEN, ODC_META_XML ) ;
             bReturn = FALSE ;
          }
          remove( TempFile ) ;

          /*-----------------------------------------------------------*/
          /*  Copy 2nd translated part of META.XML from CONTENT.XML.   */
          /*-----------------------------------------------------------*/
          if ( bReturn ) {
             fputs( szIn, fOut ) ;
             while( GetRcd( szIn, MAX_RCD_LENGTH, fSplit, TRUE ) != NULL ) {
                ptrChar = strstr( szIn, ODC_SEPARATOR1_STYLES_XML ) ;
                if ( ptrChar ) {
                   *ptrChar = 0 ;
                   fputs( szIn, fOut ) ;
                   ptrChar += strlen(ODC_SEPARATOR1_STYLES_XML) ;
                   if ( *ptrChar ) 
                      memmove( szIn, ptrChar, strlen(ptrChar)+1 ) ;
                   else 
                      szIn[0] = 0 ;
                   usFileType = 3 ;
                   break;
                }
                fputs( szIn, fOut ) ;
             }
          }
          fclose( fOut ) ;
       } else {
          sprintf( ErrText, ODC_REPLACE_OPEN, ODC_META_XML ) ;
          bReturn = FALSE ;
       }
    }


    /*-----------------------------------------------------------------*/
    /*  Copy the records for the STYLES.XML file.                      */
    /*-----------------------------------------------------------------*/
    if ( ( usFileType == 3 ) &&
         ( bReturn ) ) {
       strcpy( szOutputFile, OutputDir ) ;
       strcat( szOutputFile, ODC_STYLES_XML ) ;

       /*--------------------------------------------------------------*/
       /*  Unzip source STYLES.XML.                                    */
       /*--------------------------------------------------------------*/
       sprintf( szCommand, "\"%s\" -q -o %s %s -d %s 2> %s", 
                           szUnzipExe, SourceZip, ODC_STYLES_XML, 
                           OutputDir, TempFile ) ;
       sprintf( ErrText, ODC_EXTRACT_FAILED, ODC_STYLES_XML ) ;
       bReturn = ExecuteCommand( szCommand, TempFile, ErrText ) ;
       if ( bReturn ) {
          strcpy( szTempFile, OutputDir ) ;
          strcat( szTempFile, ODC_STYLES_XML ) ;
          DosCopy( szTempFile, TempFile, DCPY_EXISTING ) ;

          /*-----------------------------------------------------------*/
          /*  Copy 1st unchanged part of STYLES.XML file.              */
          /*-----------------------------------------------------------*/
          fOut = fopen( szOutputFile, "w" ) ;
          fIn = fopen( TempFile, "r" ) ;
          if ( fIn ) {
             while( GetRcd( szIn2, MAX_RCD_LENGTH, fIn, TRUE ) != NULL ) {
                ptrChar = strstr( szIn2, ODC_STARTTAG_STYLES_XML ) ;
                if ( ptrChar ) {
                   *ptrChar = 0 ;
                   fputs( szIn2, fOut ) ;
                   break ;
                }
                fputs( szIn2, fOut ) ;
             }
             fclose( fIn ) ;
          } else {
             sprintf( ErrText, ODC_REPLACE_OPEN, ODC_STYLES_XML ) ;
             bReturn = FALSE ;
          }
          remove( TempFile ) ;

          /*-----------------------------------------------------------*/
          /*  Copy 2nd translated part of STYLES.XML from CONTENT.XML. */
          /*-----------------------------------------------------------*/
          if ( bReturn ) {
             fputs( szIn, fOut ) ;
             while( GetRcd( szIn, MAX_RCD_LENGTH, fSplit, TRUE ) != NULL ) {
                fputs( szIn, fOut ) ;
             }
          }
          fclose( fOut ) ;
       } else {
          sprintf( ErrText, ODC_REPLACE_OPEN, ODC_STYLES_XML ) ;
          bReturn = FALSE ;
       }
    }

    fclose( fSplit ) ;

    return( bReturn ) ;
}



/*****************************************************************************/
/*  SplitFiles_2                                                             */
/*                                                                           */
/*  Split the CONTENT.XML file into these parts, including subdirectories:   */
/*       CONTENT.XML                                                         */
/*       META.XML                                                            */
/*       STYLES.XML                                                          */
/*                                                                           */
/* Input:      XmlFile       - File to split into its components.            */
/*             OutputDir     - Output directory to save split files into.    */
/* Output:     ErrText       - Error message if failure.                     */
/*                                                                           */
/*  Return:  TRUE  - Action was successful.                                  */
/*           FALSE - Action failed.                                          */
/*****************************************************************************/
BOOL SplitFiles_2( char *XmlFile, char *OutputDir, char *ErrText )
{
    FILE       *fSource, *fOut ;
    CHAR       szIn[MAX_RCD_LENGTH*2] ;
    CHAR       szOutputFile[256] ;
    CHAR       szOutputName[256] ;
    CHAR       *ptrChar, *ptrChar2 ;
    CHAR       *ptrText, *ptrFile ; 
    USHORT     rc ;
    BOOL       bReturn = TRUE ;

  
    fSource = fopen( XmlFile, "r" ) ;
    if ( ( fSource ) &&
         ( fgets( szIn, MAX_RCD_LENGTH, fSource ) != NULL ) &&
         ( ! strncmp( szIn, ODC_SEPARATOR_START, strlen(ODC_SEPARATOR_START) ) ) ) {
       while( ( bReturn ) &&
              ( ! strncmp( szIn, ODC_SEPARATOR_START, strlen(ODC_SEPARATOR_START) ) ) ) {

          /*-----------------------------------------------------------------*/
          /*  Split the next records into a separate XML file.               */
          /*-----------------------------------------------------------------*/
          ptrChar = szIn + strlen( ODC_SEPARATOR_START ) ;
          ptrChar2 = strstr( ptrChar, ODC_SEPARATOR_END ) ;
          if ( ptrChar2 ) {
             *ptrChar2 = 0 ;
             ptrText = ptrChar2 + strlen( ODC_SEPARATOR_END ) ; 
             strcpy( szOutputFile, ptrChar ) ;
             /*--------------------------------------------------------------*/
             /*  Create all of the directories needed for this file.         */
             /*--------------------------------------------------------------*/
             for( ptrChar=strchr(szOutputFile,'\\') ;
                  ptrChar ; 
                  ptrChar=strchr(ptrChar+1,'\\') ) {
                *ptrChar = 0 ;
                strcpy( szOutputName, OutputDir ) ;
                strcat( szOutputName, szOutputFile ) ;
                _mkdir( szOutputName ) ;
                *ptrChar = '\\' ;
             }
             strcpy( szOutputName, OutputDir ) ;
             strcat( szOutputName, szOutputFile ) ;
             fOut = fopen( szOutputName, "w" ) ;
             if ( fOut ) {
                /*-----------------------------------------------------------*/
                /*  Copy all records until the start of the next file.       */
                /*-----------------------------------------------------------*/
                while ( bReturn ) {
                   ptrFile = strstr( ptrText, ODC_SEPARATOR_COMMENT ) ;  /* 1-5-11 */
                   if ( ( ptrFile ) &&
                        ( ! strstr( ptrFile, ODC_SEPARATOR_COMMENT_END ) ) ) {
                      *ptrFile = 0 ;
                      fputs( ptrText, fOut ) ;
                      *ptrFile = '<' ;
                      memmove( szIn, ptrFile, strlen(ptrFile)+1 ) ;
                      ptrText = szIn ;
                      fgets( &szIn[strlen(szIn)], MAX_RCD_LENGTH, fSource ) ;
                   }
                   ptrFile = strstr( ptrText, ODC_SEPARATOR_START ) ; 
                   if ( ptrFile ) {
                      *ptrFile = 0 ;
                      fputs( ptrText, fOut ) ;
                      *ptrFile = '<' ;
                      memmove( szIn, ptrFile, strlen(ptrFile)+1 ) ;
                      break ;
                   } else {
                      fputs( ptrText, fOut ) ;
                      if ( fgets( szIn, MAX_RCD_LENGTH, fSource ) == NULL ) {
                         szIn[0] = 0 ;
                         break ;
                      }
                      ptrText = szIn ; 
                   }
                }
                fclose( fOut ) ;
             } else {
                sprintf( ErrText, ODC_REPLACE_XML_FILE, szOutputFile ) ;
                bReturn = FALSE ;
             }

          } else {
             sprintf( ErrText, ODC_REPLACE_SPLIT ) ;
             bReturn = FALSE ;
          }
       }
    } else {
       sprintf( ErrText, ODC_REPLACE_SPLIT ) ;
       bReturn = FALSE ;
    }

    fclose( fSource ) ;
    fclose( fOut ) ;

    return( bReturn ) ;
}



/*****************************************************************************/
/*  FindFileType                                                             */
/*                                                                           */
/*  Determine the type of the input file, either OpenDocument file or        */
/*  XML file.                                                                */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be processed.                            */
/*****************************************************************************/

BOOL FindFileType( char *InFile, BOOL *bFormatXml ) 
{
    FILE      *fInput ;
    char      szIn[20] ;
    BOOL      bReturn = FALSE ;


    fInput = fopen( InFile, "rb" ) ;
    if ( ( fInput ) &&
         ( fgets( szIn, 10, fInput ) != NULL ) ) {
       if ( ! strncmp( szIn, "<?xml ", 6 ) ) {
          *bFormatXml = TRUE ;
          bReturn = TRUE ;
       } else
       if ( ! strcmp( szIn, "PK\x03\x04\x14" ) ) {
          *bFormatXml = FALSE ;
          bReturn = TRUE ;
       }
    }
    if ( fInput ) 
       fclose( fInput ) ;

    return( bReturn ) ;
}



/*****************************************************************************/
/*  FindSTargetType                                                          */
/*                                                                           */
/*  Determine the type of the concatenated file:                             */
/*     TRUE = Old format with only CONTENT.XML, META.XML, STYLES.XML         */
/*     FALSE= New format with only files from subdirectories.                */
/*                                                                           */
/*****************************************************************************/

BOOL FindSTargetType( char *InFile ) 
{
    FILE      *fInput ;
    char      szIn[80] ;
    BOOL      bReturn = TRUE ;


    fInput = fopen( InFile, "rb" ) ;
    if ( ( fInput ) &&
         ( fgets( szIn, 80, fInput ) != NULL ) ) {
       if ( ! strncmp( szIn, "<!-- TWB ", 9 ) ) {
          bReturn = FALSE ;
       }
    }
    if ( fInput ) 
       fclose( fInput ) ;

    return( bReturn ) ;
}


/*******************************************************************************
*
*       function:       EQFQUERYEXITINFO
*
* -----------------------------------------------------------------------------
*       Description:
*               Determine the files which are required for this markup table.
*       Parameters:
*               PSZ           // name of the markup table, e.g. "IBMHTM32"
*               USHORT        // type of information being queried
*               PSZ           // buffer area receiving the information returned by the exit
*               USHORT        // length of buffer area
*       Return:
*               EQF_BOOL      // 0=Successful
*******************************************************************************/

__declspec(dllexport)
USHORT __cdecl /*APIENTRY*/ EQFQUERYEXITINFO(PSZ pszTagTable, // name of the markup table, e.g. "IBMHTM32"
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
