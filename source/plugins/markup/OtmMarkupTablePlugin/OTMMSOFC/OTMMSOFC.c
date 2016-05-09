/*
*
*  Copyright (C) 1998-2016, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/

/*-------------------------------------------------------------------*/
/* otmmsofc.c Source file for a C DLL                                */
/*-------------------------------------------------------------------*/

/****************************************************************************/
/*                                                                          */
/* otmmsofc.c                                                               */
/*                                                                          */
/****************************************************************************/
/*                                                                          */
/*    Description:                                                          */
/*         This function will examine a Microsoft Office file to determine  */
/*         if any additional processing will be needed for segmentation to  */
/*         occur properly in OpenTM2.  This DLL will have 4 entry points.   */
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
/* 2/26/2016: IBM : Original Source                                         */
/*==========================================================================*/

#define MAXPATHLEN 2024

#define INCL_BASE
#define INCL_DOSFILEMGR


#include "unicode.h"
#include "otmmsofc.h"
#define DosCopy(a, b , value) CopyFileA(a, b, FALSE);

using namespace mku ;                       

#include <ole2.h>
#include <oleauto.h>
#include <direct.h>
#include <sys/stat.h>
#include <tlhelp32.h>


#ifndef CCHMAXPATHCOMP
  #define CCHMAXPATHCOMP MAXPATHLEN
#endif


#define MAX_RCD_LENGTH         12288
#define MAX_TEXT_LENGTH        30000


#define  ZIP_FILE_SEPARATOR              "<!-- TWB %s -->\n"
#define  ZIP_FILE_SEPARATOR_COMMENT      "<!--"
#define  ZIP_FILE_SEPARATOR_COMMENT_END  "-->"
#define  ZIP_FILE_SEPARATOR_START        "<!-- TWB "
#define  ZIP_FILE_SEPARATOR_END          " -->\n"

char        szErrMsg[4096] ;
char        szErrTitle[80] ;
short       sDefaultDoNotTranslate = 0 ;

extern   char    szDocTargetLanguage[80];   /* From USRCALLS.C  */
extern   char    szDocSourceLanguage[80];   /* From USRCALLS.C  */
extern   short   sTPVersion ;               /* From USRCALLS.C  */
         char    szProgPath[256] ;


#define  MSG_MSOFC_MISSING_SOURCE_FILE          "Source file could not be located."
#define  MSG_MSOFC_MISSING_TARGET_FILE          "Target file could not be located."
#define  MSG_MSOFC_PARSING_ERROR                "File could not be parsed.\n\nProcessing is terminated."
#define  MSG_MSOFC_ZIP_XML_MISSING_UNZIP        "UNZIP.EXE could not be found to extract the XML files from the Office file.\n\nInstall UNZIP.EXE in a directory which is in your PATH environment variable." 
#define  MSG_MSOFC_ZIP_XML_UNZIP_FAILED         "XML files could not be extracted from the Office file.\n\nUNZIP errors:\n"
#define  MSG_MSOFC_ZIP_XML_CONCAT_FAILED        "XML files could not be concatenated together.\n\nProcessing is terminated.\n"
#define  MSG_MSOFC_ZIP_XML_DIR_FAILED           "Could not determine the XML files contained in the Office file.\n\nProcessing is terminated.\n"
#define  MSG_MSOFC_ZIP_XML_SOURCE_PATH          "Source file path is not correct: %s"
#define  MSG_MSOFC_XML_ZIP_MISSING_ZIP          "ZIP.EXE could not be found to update the XML files in the Office file.\n\nInstall ZIP.EXE in a directory which is in your PATH environment variable."
#define  MSG_MSOFC_XML_ZIP_TARGET_PATH          "Target file path is not correct: %s"
#define  MSG_MSOFC_XML_ZIP_CREATE_XML_FILE      "XML file could not be recreated: %s"
#define  MSG_MSOFC_XML_ZIP_CREATE_XML_FORMAT    "XML file cannot be split into its individual components."
#define  MSG_MSOFC_XML_ZIP_REPLACE_FAILED       "XML files could not be replaced into the Office file.\n\nZIP errors:\n" 
#define  MSG_MSOFC_PARAGRAPH_MISMATCH           "Target paragraph ID=%ld does not match source paragraph ID=%ld.\n\nProcessing is terminated."
#define  MSG_MSOFC_TEXT_UNITS_MISMATCH          "Target paragraph %ld contains %ld text units while source contains %ld.\n\nProcessing is terminated."
#define  MSG_MSOFC_INCORRECT_AMPERSAND          "The ampersand character (&) must be defined as ""&amp;"".\n\nOK - Return to segment to correct the text."
#define  MSG_MSOFC_FILE_CONVERSION              "Code page conversion failed.\n\nProcessing is terminated."

#define  TITLE_MSOFC_WORD_XML_ERROR             "Word-to-XML Conversion Error"
#define  TITLE_MSOFC_XML_WORD_ERROR             "XML-to-Word Conversion Error"
#define  TITLE_MSOFC_MISSING_FILE               "Missing File"
#define  TITLE_MSOFC_PARSING_ERROR              "Parsing Error"
#define  TITLE_MSOFC_PARAGRAPH_MISMATCH         "Paragraph Mismatch"
#define  TITLE_MSOFC_TEXT_UNITS_MISMATCH        "Text Units Mismatch"
#define  TITLE_MSOFC_FILE_CONVERSION            "File Conversion Error"
         
         

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
EQF_BOOL __cdecl EQFPRESEG2(
                                PSZ        pTagTable,
                                PSZ        pEdit,
                                PSZ        pProgPath,
                                PSZ        pSource,
                                PSZ        pTempSource,
                                PEQF_BOOL  pfNoSegment,
                                HWND       hSlider,
                                PEQF_BOOL  pfKill )

{
    CHAR *Source ;
    CHAR *ptrChar ;

    USHORT  usFileFormat ;
    BOOL    bReturn = TRUE;
    char    xxxFile[512] ;
    char    TempFile[512], TempFile2[512], TempFile3[512] ;
    char    SaveXmlName[512] ;
    char    *szAltTempExt1 = ".$1$";
    char    *szAltTempExt2 = ".$2$";
    int     rc;

    PrepDocLanguageInfo( pSource ) ; /* Set language unqiue processing, like DBCS */
    strcpy( szProgPath, pProgPath ) ;
    if (!pEdit) //For ITM processing
        strcpy(szDocTargetLanguage,szDocSourceLanguage);

    *pfNoSegment = FALSE;
     
    Source = strdup(pSource) ;

    CreateTempFileName2( TempFile2, pSource, szAltTempExt1, TEMPNAME_SSOURCE ) ;
    CreateTempFileName2( TempFile3, pSource, szAltTempExt2, TEMPNAME_SSOURCE ) ;
    strcpy( TempFile, TempFile2 ) ;
    strcat( TempFile, ".XML" ) ;
    strcpy( pTempSource,TempFile );
    
    if ( ! ConvertWordToXml( Source, TempFile, TempFile3, szErrMsg, &usFileFormat ) ) {
       MessageBoxA(HWND_DESKTOP, szErrMsg, TITLE_MSOFC_WORD_XML_ERROR, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
       bReturn = FALSE ;
    }

    if ( bReturn ) {
       rc = ConvertImport(TempFile, TempFile2, EQF_UTF82UTF16);
       if ( !rc ) {
          MessageBoxA(HWND_DESKTOP, MSG_MSOFC_FILE_CONVERSION, TITLE_MSOFC_FILE_CONVERSION, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
          bReturn = FALSE ;
       }
       rc=remove(TempFile);
    }
                     
    if ( bReturn ) {
       strcpy( SaveXmlName, Source ) ;
       ptrChar = strstr( SaveXmlName, "\\SOURCE\\" ) ;
       if ( ptrChar ) {
          strcpy( ptrChar, "\\MISC\\" ) ;
          _mkdir( SaveXmlName ) ;
       }

       strcpy( SaveXmlName, Source ) ;
       ptrChar = strstr( SaveXmlName, "\\SOURCE\\" ) ;
       if ( ptrChar ) {
          memmove( ptrChar+6, ptrChar+8, strlen(ptrChar+8)+1 ) ;
          strncpy( ptrChar, "\\MISC\\", 6 ) ;
          CopyFileA(TempFile2, SaveXmlName, FALSE);
       } else {
          MessageBoxA(HWND_DESKTOP, MSG_MSOFC_MISSING_SOURCE_FILE, TITLE_MSOFC_MISSING_FILE, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
          bReturn = FALSE ;
       }
    }

    sDefaultDoNotTranslate = 2 ;   /* Translatable */

    if ( bReturn ) {
       bReturn = PreParse(TempFile2, pTempSource, pTagTable, hSlider, usFileFormat);
    }

    if ( ! bReturn )
       remove(TempFile);
    remove(TempFile2);
    remove(TempFile3);     /* Original temp file */

    return(bReturn);

}  /* EQFPRESEG2 */



/*******************************************************************************
*
*       function:       EQFPOSTSEGW
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
*******************************************************************************/

__declspec(dllexport)
EQF_BOOL __cdecl EQFPOSTSEGW(
                               PSZ    pTagTable,
                               PSZ    pEdit,
                               PSZ    pProgPath,
                               PSZ    pSegSource,
                               PSZ    pSegTarget,
                               PTATAG_W      pTATag,
                               HWND        hSlider,
                               PEQF_BOOL     pfKill )
{
   BOOL    bReturn = TRUE;


  PrepDocLanguageInfo( pSegSource ) ; /* Set language unqiue processing, like DBCS */
  strcpy( szProgPath, pProgPath ) ;

   bReturn = PostParse( pSegSource, pSegTarget, hSlider ) ;
   if ( bReturn ) {
      CopyFileA( pSegTarget, pSegSource, FALSE ) ;
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
EQF_BOOL __cdecl EQFPREUNSEGW(
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


/*******************************************************************************
*
*       function:       EQFPOSTUNSEGW
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
EQF_BOOL __cdecl EQFPOSTUNSEGW(
                                 PSZ    pTagTable,
                                 PSZ    pEdit,
                                 PSZ    pProgPath,
                                 PSZ    pTarget,
                                 PTATAG        pTATag,
                                 PEQF_BOOL     pfKill )
{
    struct _stat findbuf ;
    BOOL     bReturn = TRUE;
    USHORT   usFileFormat ;
    char     szMarkup[80] ;
    char     *szAltTempExt3 = ".$3$";
    char     *szAltTempExt4 = ".$4$";
    char     SourceFile[512];
    char     TempFile[512], TempFile2[512], TempFile3[512] ;
    char     *ptrChar ;
    int      rc;


    PrepDocLanguageInfo( pTarget ) ; /* Set language unqiue processing, like DBCS */
    strcpy( szProgPath, pProgPath ) ;
    
    CreateTempFileName2( TempFile2, pTarget, szAltTempExt3, TEMPNAME_TARGET ) ;
    CreateTempFileName2( TempFile3, pTarget, szAltTempExt4, TEMPNAME_TARGET ) ;
    strcpy( TempFile, TempFile2 ) ;
    strcat( TempFile, ".DOC" ) ;

    CopyFileA(pTarget, TempFile, FALSE);
    strcpy( SourceFile, pTarget ) ; 
    ptrChar = strstr( SourceFile, "\\TARGET\\" ) ;
    if ( ptrChar ) {
       memmove( ptrChar+6, ptrChar+8, strlen(ptrChar+8)+1 ) ;
       strncpy( ptrChar, "\\MISC\\", 6 ) ;
       if ( _stat( SourceFile, &findbuf ) ) { 
          MessageBoxA(HWND_DESKTOP, MSG_MSOFC_MISSING_TARGET_FILE, TITLE_MSOFC_MISSING_FILE, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
          bReturn = FALSE ;
       }
    } else {
       MessageBoxA(HWND_DESKTOP, MSG_MSOFC_MISSING_TARGET_FILE, TITLE_MSOFC_MISSING_FILE, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
       bReturn = FALSE ;
    }

    if ( bReturn ) {
       bReturn = PostExport( TempFile, pTarget, SourceFile, &usFileFormat ) ;
       rc=remove(TempFile);
    }
    
    if ( bReturn ) {
       rc = ConvertExport(pTarget, EQF_UTF162UTF8);
       if ( !rc ) {
          MessageBoxA(HWND_DESKTOP, MSG_MSOFC_FILE_CONVERSION, TITLE_MSOFC_FILE_CONVERSION, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
          bReturn = FALSE ;
       }
    }


    if ( bReturn ) {
       if ( ! ConvertXmlToWord( pTarget, TempFile, szErrMsg, usFileFormat ) ) {
          MessageBoxA(HWND_DESKTOP, szErrMsg, TITLE_MSOFC_XML_WORD_ERROR, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL);
          return FALSE;
       }
    }

    remove( TempFile ) ;
    remove( TempFile2 ) ;
    remove( TempFile3 );     /* Original temp file */

    return(bReturn);

}  /* EQFPOSTUNSEG2 */





__declspec(dllexport)
USHORT __cdecl EQFQUERYEXITINFO(
                                   PSZ pszTagTable,  // name of the markup table, e.g. "OTMHTM32"
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
*       function:       EQFCHECKSEGEXW
*
* -----------------------------------------------------------------------------
*       Description:
*               Allow to validate of the current segment.
*
*       Arguments:   PSZ    ... pointer to previous source segment
*                    PSZ    ... pointer to current source segment
*                    PSZ    ... pointer to current target segment
*                    PBOOL  ... ptr to whether segement was changed
                     LONG   ... handle to use with EQFGETPREVSEG
                     ULONG  ... current segment number 
*                    BOOL   ... whether message display is requested
*       Return:
 *              TRUE  - Text successfully checked and no validation problem. 
*               FALSE - Text failed the check, so text should not be saved. 
*******************************************************************************/

__declspec(dllexport)
EQF_BOOL __cdecl EQFCHECKSEGEXW(
                                     PSZ_W     pszPrevSrc,
                                     PSZ_W     pszSrc,
                                     PSZ_W     pszTgt,
                                     PEQF_BOOL pfChanged,
                                     LONG      lInfo,          
                                     ULONG     ulSegNum,       
                                     EQF_BOOL  fMsg)
{
   BOOL   bReturn = TRUE ; 

   PrepDocLanguageInfo( NULL ) ;   /* Set language unqiue processing, like DBCS */

   bReturn = CheckXmlText( pszPrevSrc, pszSrc, pszTgt, pfChanged, lInfo, ulSegNum, fMsg ) ;

   return( bReturn ) ;
}





/*****************************************************************************/
/*  ConvertWordToXml                                                         */
/*                                                                           */
/*  Function called by EQFPRESEG2                                            */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be processed.                            */
/*****************************************************************************/

BOOL ConvertWordToXml( char *in, char *out, char *TempFile, char *ErrText, USHORT *usFileFormat )
{
    FILE *fInput ;
    char szIn[MAX_RCD_LENGTH] ;
    WCHAR   swTemp[512];
    BOOL bReturn = TRUE;
    BOOL bReturn2 = TRUE;
    BOOL bWordActive = FALSE ;


    HRESULT    hr;
    CLSID      clsid;
    IDispatch  *pDisp;
    IUnknown   *pUnk = NULL;
    IDispatch  *pDispRoot = NULL;
    IDispatch  *pDispApp  = NULL;
    IDispatch  *pDispDocs = NULL;
    IDispatch  *pDispDoc = NULL;
    IDispatch  *pDispProperties = NULL;

    DISPID     dispID;

    HANDLE     hProcess = NULL; 
    ULONG      ulStartWordIDs[50] ;
    ULONG      ulEndWordIDs[50] ;
    USHORT     i, j ;


    ErrText[0] = 0 ;

    /*************************************************************************/
    /*  Determine if file is XML or Word document.                           */
    /*************************************************************************/
    *usFileFormat = FILE_FORMAT_UNKNOWN ;
    fInput = fopen( in, "rb" ) ;
    if ( ( fInput ) &&
         ( fgets( szIn, MAX_RCD_LENGTH, fInput ) != NULL ) ) {
       if ( strstr( szIn, "<?xml" ) ) 
          *usFileFormat = FILE_FORMAT_XML ;
       if ( ! strncmp( szIn, "PK", 5 ) ) 
          *usFileFormat = FILE_FORMAT_ZIP ;
       if ( ! strncmp( szIn, "ÐÏà¡±", 6 ) ) 
          *usFileFormat = FILE_FORMAT_WORD ;
       if ( ! strncmp( szIn, "{\\rtf1\\", 7 ) ) {
          strcat( ErrText, "\nWord files using RTF cannot be handled by this markup table.\nUse EQFMSWRD instead." ) ;
          bReturn = FALSE ;
       }
    }
    if ( fInput ) 
       fclose( fInput ) ;
    if ( ( bReturn ) &&
         ( *usFileFormat == FILE_FORMAT_UNKNOWN ) ) {
       strcat( ErrText, "\nSource file could not be read." ) ;
       bReturn = FALSE ;
    }


    if ( bReturn ) {

       /**********************************************************************/
       /*  XML file needs no conversion.                                     */
       /**********************************************************************/
       if ( *usFileFormat == FILE_FORMAT_XML ) {
          CopyFileA(in, out, FALSE);
       } else 

       /**********************************************************************/
       /*  Convert MS Word 2003 document to XML.                             */
       /**********************************************************************/
       if ( *usFileFormat == FILE_FORMAT_WORD ) {

          OleInitialize(NULL);

          // Get CLSID for our server...
          hr=CLSIDFromProgID(L"Word.Application",&clsid);
          if ( FAILED(hr) ) {
             bReturn = FALSE ;
             strcat( ErrText, "\nGetting CLSID for MS Word failed." ) ;
          } 


          GetWordProcessList( &ulStartWordIDs[0] ) ;

          if ( bReturn ) {
             // Start the server...
             hr=CoCreateInstance(clsid,NULL,CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**) &pUnk);
             if ( FAILED(hr) ) {
                 bReturn = FALSE;
                 strcat( ErrText, "\nStarting MS Word server failed." ) ;
             }
          } 

          GetWordProcessList( &ulEndWordIDs[0] ) ;


          if ( bReturn ) {
             // Query for IDispatch
             hr = pUnk->QueryInterface(IID_IDispatch, (void**)&pDispRoot);
             if ( FAILED(hr) ) {
                 pUnk->Release();
                 strcat( ErrText, "\nQuery for IDispatch failed." ) ;
                 bReturn = FALSE;
             } 
          }

          if ( bReturn ) {
             // GET App IDispatch
             VARIANT Result;
             VariantInit(&Result);
             bReturn = AutomationWrapper(DISPATCH_PROPERTYGET,&Result,pDispRoot,"Application",ErrText,0);
             pDispApp = Result.pdispVal;
             if ( ! bReturn ) {
                 strcat( ErrText, "\nGet Application IDispatch." ) ;
             } else {
                bWordActive = TRUE ;
             }
          }

          if ( bReturn ) {
             // Get Documents collection...         [WordApp~Documents]
             VARIANT result;
             VariantInit(&result);
             bReturn = AutomationWrapper(DISPATCH_PROPERTYGET, &result, pDispApp, "Documents", ErrText, 0);
             pDispDocs = result.pdispVal;
             if ( ! bReturn ) {
                strcat( ErrText, "\nGet document collection failed." ) ;
             } 
          }


          if ( bReturn ) {
             // Call Documents::Add()...            [WordApp~Add]
             VARIANT result;
             VariantInit(&result);
             VARIANT parm ;
             parm.vt = VT_BSTR;
             mbstowcs( swTemp, in, 256 ) ;
             parm.bstrVal = SysAllocString(swTemp) ;
             bReturn = AutomationWrapper(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &result, pDispDocs, "Add", ErrText, 1,parm);
             pDispDoc = result.pdispVal;
             if ( ! bReturn ) {
                strcat( ErrText, "\nOpening MS Word document failed." ) ;
             } 
          }

          if ( bReturn ) {
             // Call Documents::Add()...            [WordApp~Selection]
             VARIANT result;
             VariantInit(&result);
             bReturn = AutomationWrapper(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &result, pDispApp, "Selection", ErrText, 0);
             if ( ! bReturn ) {
                 strcat( ErrText, "\nOpening MS Word \"File\" menu item for \"SaveAs\" operation failed." ) ;
             } 
          }

          if ( bReturn ) {
             // SaveAs XML                           [WordApp~SaveAs]
             /*  WdSaveFormat
                    wdFormatDocument  0 
                    wdFormatDOSText  4 
                    wdFormatDOSTextLineBreaks  5 
                    wdFormatEncodedText  7 
                    wdFormatFilteredHTML  10 
                    wdFormatHTML  8 
                    wdFormatRTF  6 
                    wdFormatTemplate  1 
                    wdFormatText  2 
                    wdFormatTextLineBreaks  3 
                    wdFormatUnicodeText  7 
                    wdFormatWebArchive  9 
                    wdFormatXML  11                              */

             int num = 11;                            /* SaveAs XML format  wdFormatXML */ 
             VARIANT result;
             VariantInit(&result);
             VARIANT parm;
             parm.vt = VT_BSTR;
             mbstowcs( swTemp, out, 256 ) ;
             parm.bstrVal = SysAllocString(swTemp) ;
             VARIANT parm1;
             parm1.vt = VT_I4;
             parm1.lVal = num;
             bReturn = AutomationWrapper(DISPATCH_METHOD, &result, pDispDoc, "SaveAs", ErrText, 2, parm1, parm);
             if ( ! bReturn ) {
                 strcat( ErrText, "\nSaveAs XML failed." ) ;
                 strcat( ErrText, "\nMS Word document could not be saved as an XML document." ) ;
                 remove( out ) ;

                 parm1.lVal = 0 ;   /* Close Word doc without saving to avoid pop-up: do you want to save changes? */ 
                 bReturn2 = AutomationWrapper(DISPATCH_METHOD, &result, pDispDoc, "Close", szIn, 1, parm1);
             } 
             SysFreeString(parm.bstrVal);
             
          } 


          if ( bWordActive ) {
              //Quit                               [WordApp~Quit]
              VARIANT result;
              VariantInit(&result);
              bReturn2 = AutomationWrapper(DISPATCH_METHOD, 0 , pDispApp, "Quit", ErrText, 0);
              if ( ! bReturn2 ) {
                  bReturn = FALSE ;
                  strcat( ErrText, "\nMS Word \"Quit\" operation failed." ) ;
              } 
          }



          // Clean up...
          //pDispSelection->Release();
          if ( pDispDoc ) pDispDoc->Release();
          if ( pDispDocs ) pDispDocs->Release();
          if ( pDispApp ) pDispApp->Release();
          if ( pDispRoot ) pDispRoot->Release();
          if ( pUnk ) pUnk->Release();

          // Uninitialize OLE Libraries...
          OleUninitialize();

          for( i=0 ; i<50 && ulEndWordIDs[i] ; ++i ) {
             for( j=0 ; j<50 && ulStartWordIDs[j] ; ++j ) {
                if ( ulEndWordIDs[i] == ulStartWordIDs[j] ) 
                   break;
             }
             if ( ulStartWordIDs[j] == 0 ) {
                hProcess = OpenProcess( PROCESS_ALL_ACCESS,FALSE,ulEndWordIDs[i] ) ;
                if ( hProcess != NULL )  {
                   TerminateProcess(hProcess, 0xffffffff);
                   CloseHandle( hProcess);
                }
             }
          }

       } else 

       /**********************************************************************/
       /*  Convert MS Office 2007 document to XML.                           */
       /**********************************************************************/
       if ( *usFileFormat == FILE_FORMAT_ZIP ) {
          if ( ! ExtractXmlFromZip( in, out, TempFile, szErrMsg ) ) {
             bReturn = FALSE ;
          }
       }
    }


    return(bReturn);
} 


/*****************************************************************************/
/*  ConvertXmlToWord                                                         */
/*                                                                           */
/*  Function called by EQFPOSTUNSEGW.                                        */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be processed.                            */
/*****************************************************************************/

BOOL ConvertXmlToWord( char *in, char *temp, char *ErrText, USHORT usFileFormat )
{
    WCHAR   swTemp[512];
    char    szTemp[512];
    BOOL bReturn = TRUE;
    BOOL bReturn2 = TRUE;
    BOOL bWordActive = FALSE ;


    HRESULT    hr;
    CLSID      clsid;
    IDispatch  *pDisp;
    IUnknown   *pUnk = NULL;
    IDispatch  *pDispRoot = NULL;
    IDispatch  *pDispApp  = NULL;
    IDispatch  *pDispDocs = NULL;
    IDispatch  *pDispDoc = NULL;
    IDispatch  *pDispProperties = NULL;

    HANDLE     hProcess = NULL; 
    ULONG      ulStartWordIDs[50] ;
    ULONG      ulEndWordIDs[50] ;
    USHORT     i, j ;

    DISPID     dispID;


    ErrText[0] = 0 ;

    /**********************************************************************/
    /*  XML file needs no conversion.                                     */
    /**********************************************************************/
    if ( usFileFormat == FILE_FORMAT_XML ) {
       
    } else 

    /**********************************************************************/
    /*  Convert XML to MS Word 2003 document.                             */
    /**********************************************************************/
    if ( usFileFormat == FILE_FORMAT_WORD ) {

       OleInitialize(NULL);
     
     
       // Get CLSID for our server...
       hr=CLSIDFromProgID(L"Word.Application",&clsid);
       if ( FAILED(hr) ) {
          bReturn = FALSE ;
          strcat( ErrText, "\nGetting CLSID for MS Word failed." ) ;
       } 
     
     
       GetWordProcessList( &ulStartWordIDs[0] ) ;
     
       if ( bReturn ) {
          // Start the server...
          hr=CoCreateInstance(clsid,NULL,CLSCTX_LOCAL_SERVER, IID_IUnknown, (void**) &pUnk);
          if ( FAILED(hr) ) {
              bReturn = FALSE;
              strcat( ErrText, "\nStarting MS Word server failed." ) ;
          }
       } 
     
       GetWordProcessList( &ulEndWordIDs[0] ) ;
     
       if ( bReturn ) {
          // Query for IDispatch
          hr = pUnk->QueryInterface(IID_IDispatch, (void**)&pDispRoot);
          if ( FAILED(hr) ) {
              pUnk->Release();
              strcat( ErrText, "\nQuery for IDispatch failed." ) ;
              bReturn = FALSE;
          } 
       }
     
       if ( bReturn ) {
          // GET App IDispatch
          VARIANT Result;
          VariantInit(&Result);
          bReturn = AutomationWrapper(DISPATCH_PROPERTYGET,&Result,pDispRoot,"Application",ErrText,0);
          pDispApp = Result.pdispVal;
          if ( ! bReturn ) {
              strcat( ErrText, "\nGet Application IDispatch." ) ;
          } else {
             bWordActive = TRUE ;
          } 
       }
     
       if ( bReturn ) {
          // Get Documents collection...         [WordApp~Documents]
          VARIANT result;
          VariantInit(&result);
          bReturn = AutomationWrapper(DISPATCH_PROPERTYGET, &result, pDispApp, "Documents", ErrText, 0);
          pDispDocs = result.pdispVal;
          if ( ! bReturn ) {
             strcat( ErrText, "\nGet document collection failed." ) ;
          } 
       }
     
     
       if ( bReturn ) {
          // Call Documents::Add()...            [WordApp~Add]
          VARIANT result;
          VariantInit(&result);
          VARIANT parm ;
          parm.vt = VT_BSTR;
          mbstowcs( swTemp, in, 256 ) ;
          parm.bstrVal = SysAllocString(swTemp) ;
          bReturn = AutomationWrapper(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &result, pDispDocs, "Add", ErrText, 1,parm);
          pDispDoc = result.pdispVal;
          if ( ! bReturn ) {
              strcat( ErrText, "\nInternal translated XML file could not be opened in MS Word." ) ;
          } 
       }

       if ( bReturn ) {
          // Call Documents::Add()...            [WordApp~Selection]
          VARIANT result;
          VariantInit(&result);
          bReturn = AutomationWrapper(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &result, pDispApp, "Selection", ErrText, 0);
          if ( ! bReturn ) {
              strcat( ErrText, "\nOpening MS Word \"File\" menu item for \"SaveAs\" operation failed." ) ;
          } 
       }
      
       if ( bReturn ) {
          // SaveAs Word document                 [WordApp~SaveAs]
          int num = 0 ;                       /* SaveAs Word document format */ 
          VARIANT result;
          VariantInit(&result);
          VARIANT parm;
          parm.vt = VT_BSTR;
          mbstowcs( swTemp, temp, 256 ) ;
          parm.bstrVal = SysAllocString(swTemp) ;
          VARIANT parm1;
          parm1.vt = VT_I4;
          parm1.lVal = num;
          ErrText[0] = NULL ;
          bReturn = AutomationWrapper(DISPATCH_METHOD, &result, pDispDoc, "SaveAs", ErrText, 2, parm1, parm);
          SysFreeString(parm.bstrVal);
          if ( ! bReturn ) {
              strcat( ErrText, "\nInternal translated XML file could not be saved as an MS Word document." ) ;
      
              parm1.lVal = 0 ;   /* Close Word doc without saving to avoid pop-up: do you want to save changes? */ 
              bReturn2 = AutomationWrapper(DISPATCH_METHOD, &result, pDispDoc, "Close", szTemp, 1, parm1);
          } 
          
       } 
      
      
       if ( bWordActive ) {
          // Call Documents::Add()...            [WordApp~Selection]
          VARIANT result;
          VariantInit(&result);
          bReturn2 = AutomationWrapper(DISPATCH_METHOD|DISPATCH_PROPERTYGET, &result, pDispApp, "Selection", ErrText, 0);
          if ( ! bReturn2 ) {
              bReturn = FALSE ;
              strcat( ErrText, "\nOpening MS Word \"File\" menu item for \"Quit\" operation failed." ) ;
          } 
       }
      
      
       if ( bWordActive ) {
           //Quit                               [WordApp~Quit]
           VARIANT result;
           VariantInit(&result);
           bReturn2 = AutomationWrapper(DISPATCH_METHOD, 0 , pDispApp, "Quit", ErrText, 0);
           if ( ! bReturn2 ) {
               bReturn = FALSE ;
               strcat( ErrText, "\nMS Word \"Quit\" operation failed." ) ;
           } 
      
           CoDisconnectObject( pUnk, NULL ) ;
       }
      
      
      
       // Clean up...
       if ( pDispDoc ) pDispDoc->Release();
       if ( pDispDocs ) pDispDocs->Release();
       if ( pDispApp ) pDispApp->Release();
       if ( pDispRoot ) pDispRoot->Release();
       if ( pUnk ) pUnk->Release();
      
       // Uninitialize OLE Libraries...
       OleUninitialize();
      
       for( i=0 ; i<50 && ulEndWordIDs[i] ; ++i ) {
          for( j=0 ; j<50 && ulStartWordIDs[j] ; ++j ) {
             if ( ulEndWordIDs[i] == ulStartWordIDs[j] ) 
                break;
          }
          if ( ulStartWordIDs[j] == 0 ) {
             hProcess = OpenProcess( PROCESS_ALL_ACCESS,FALSE,ulEndWordIDs[i] ) ;
             if ( hProcess != NULL )  {
                TerminateProcess(hProcess, 0xffffffff);
                CloseHandle( hProcess);
             }
          }
       }
    } else 

    /**********************************************************************/
    /*  Convert MS Office 2007 document to XML.                           */
    /**********************************************************************/
    if ( usFileFormat == FILE_FORMAT_ZIP ) {
       if ( ! ReplaceXmlInZip( in, temp, szErrMsg ) ) {
          bReturn = FALSE ;
       }
    }


    return(bReturn);
} 
 


/*****************************************************************************/
/*  GetWordProcessList                                                       */
/*                                                                           */
/*  Get a list of the process IDs which are running MS Word.                 */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be processed.                            */
/*****************************************************************************/

BOOL GetWordProcessList( ULONG *ulList )
{
    HANDLE         hSnapshot = NULL; 
    PROCESSENTRY32 pe32      = {0}; 
    USHORT         i = 0 ;
 
    //  Take a snapshot of all processes currently in the system. 
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0); 
    if (hSnapshot == (HANDLE)-1) 
        return (FALSE); 
 
    //  Fill in the size of the structure before using it. 
    pe32.dwSize = sizeof(PROCESSENTRY32); 
 
    //  Walk the snapshot of the processes.
    if (Process32First(hSnapshot, &pe32)) { 
        do { 
           if ( ! wcscmp( pe32.szExeFile, L"WINWORD.EXE" ) ) {
              ulList[i++] = pe32.th32ProcessID ;
           }
        } 
        while (Process32Next(hSnapshot, &pe32)); 
    } 
    CloseHandle (hSnapshot); 
    ulList[i] = 0 ;

    return( TRUE ) ;
}




//==============================================================================
//  AutomationWrapper                                                          |
//=============================================================================|
//  Wrapper function for making generic 16-bit Automation calls...             |
//                                                                             |
//    from HOWTO: Do 16-bit Automation in C++ Using VC 1.52                    |
//                Article ID: Q194656                                          |
//                                                                             |
//                                                                             |
//  Prereqs:                                                                   |
//   None.                                                                     |
//=============================================================================|
//  SideEffects:                                                               |
//   None.                                                                     |
//==============================================================================

BOOL AutomationWrapper (
                    int autoType,
                    VARIANT *pvResult,
                    IDispatch *pDisp,
                    char *ptName,
                    char *ErrText,
                    int cArgs...
                 )
{
    va_list marker;
    BOOL    fOk = TRUE;
    // Variables used...
    DISPPARAMS dp = { NULL, NULL, 0, 0};
    DISPID dispidNamed = DISPID_PROPERTYPUT;
    DISPID dispID;
    HRESULT hr;

    CHAR    szTemp[512];
    WCHAR   swTemp[512];
    PSZ     pszMsgTable[2];
                   
    // Allocate memory for arguments...
    VARIANT *pArgs = new VARIANT[cArgs+1];

    va_start(marker, cArgs);

    if ( !pDisp ) {
       if ( ErrText[0]  )
          strcat( ErrText, "\n\n" ) ;
       strcat( ErrText, "No Dispatch Interface passed.  " ) ;
       fOk = FALSE;
    } 

    if ( fOk ) {

       LPOLESTR   lpUniBuffer;
       mbstowcs( swTemp, ptName, 256 ) ;
       lpUniBuffer = swTemp ;

       // Get DISPID for name passed...
       hr = pDisp->GetIDsOfNames(IID_NULL,
                                 &lpUniBuffer,
                                 1,
                                 LOCALE_USER_DEFAULT,
                                 &dispID);
       if ( hr != S_OK ) {
          if ( ErrText[0]  )
             strcat( ErrText, "\n\n" ) ;
          sprintf( szTemp, "Get Name IDs failed: %lx.  ",hr ) ;
          strcat( ErrText, szTemp ) ;
          fOk = FALSE;
       } 

    } 

    if ( fOk ) {
        // Extract arguments...
        for ( int i=0; i<cArgs; i++ ) {
            pArgs[i] = va_arg(marker, VARIANT);
        }

        // Build DISPPARAMS
        dp.cArgs = cArgs;
        dp.rgvarg = pArgs;

        // Handle special-case for property-puts!
        if ( autoType & DISPATCH_PROPERTYPUT ) {
            dp.cNamedArgs = 1;
            dp.rgdispidNamedArgs = &dispidNamed;
        } 

        // Make the call!
        hr = pDisp->Invoke(dispID, IID_NULL, LOCALE_SYSTEM_DEFAULT,
                           autoType, &dp, pvResult, NULL, NULL);
//      if ( FAILED(hr) ) {
        if ( hr != S_OK ) {
           if ( ErrText[0]  )
              strcat( ErrText, "\n\n" ) ;
           sprintf( szTemp, "Invoke failed: %lx.  ",hr ) ;
           strcat( ErrText, szTemp ) ;
           fOk = FALSE;

        } 

    } 

    // End variable-argument section...
    va_end(marker);

    delete [] pArgs;



    return fOk;

} 





/*****************************************************************************/
/*  ExtractXmlFromZip                                                        */
/*                                                                           */
/*  Extract the XML files from the ZIP file so that the translatable text    */
/*  can be identified.  All of the XML files will be concatenated together   */
/*  into one output XML file.                                                */
/*                                                                           */
/* Input:      ZipFile       - Input ZIP file to extract from.               */
/*             XmlFile       - Output combined XML file.                     */
/*             TempFile      - Temporary work file to use.                   */
/*             ErrText       - Any error text to be shown to user.           */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be processed.                            */
/*****************************************************************************/

BOOL ExtractXmlFromZip( char *ZipFile, char *XmlFile, char *TempFile, char *ErrText )
{
    FILE      *fTemp ;
    char      szIn[MAX_RCD_LENGTH] ;
    char      szCommand[1024] ;
    char      szTempDir1[256] ;
    char      szTempDir2[256] ;
    char      szTempFile[256] ;
    char      szUnzipExe[256] ;
    char      szTemp[4096] ;
    char      *ptrChar ;
    USHORT    rc ;
    DWORD     dwCode;
    BOOL      bFormatXml = FALSE ;
    BOOL      bReturn = TRUE;



    /*-----------------------------------------------------------------------*/
    /*  Determine input file format, whether ZIP file or CONTENT.XML         */
    /*-----------------------------------------------------------------------*/
    GetOTMDllPath( szProgPath, szUnzipExe ) ;
    strcat( szUnzipExe, "UNZIP.EXE" ) ;
    if ( ! szUnzipExe[0] ) {
       strcpy( ErrText, MSG_MSOFC_ZIP_XML_MISSING_UNZIP ) ;
       bReturn = FALSE ;
    }
    remove( XmlFile ) ;
    if ( bReturn ) {
       /*--------------------------------------------------------------------*/
       /*  XML files must be extracted from the document ZIP file.           */
       /*    1.  Create temporary directories.                               */
       /*    2.  Unzip all XML into temporary directory.                     */
       /*    3.  Create 1 XML from all XML files.                            */
       /*    4.  Delete temporary directories.                               */
       /*--------------------------------------------------------------------*/
       strcpy( szTempDir1, ZipFile ) ;
       ptrChar = strstr( szTempDir1, "\\SOURCE\\" ) ;
       if ( ptrChar ) {
          /*-----------------------------------------------------------------*/
          /*  Create temporary directories.                                  */
          /*-----------------------------------------------------------------*/
          strcpy( ptrChar, "\\MISC\\" ) ;
          _mkdir( szTempDir1 ) ;             /* Create \EQF\...\MISC\        */
          strcpy( szTempDir2, szTempDir1 ) ;
          ptrChar = strrchr( ZipFile, '\\' ) ;
          strcat( szTempDir2, ++ptrChar ) ;
          strcat( szTempDir2, "$\\" ) ;
          _mkdir( szTempDir2 ) ;             /* Create \EQF\...\MISC\...\    */

          /*-----------------------------------------------------------------*/
          /*  Unzip all XML files from the zip file.                         */
          /*-----------------------------------------------------------------*/
          sprintf( szCommand, "\"%s\" -q -o %s *.xml -d %s 2> %s", 
                              szUnzipExe, ZipFile, szTempDir2, TempFile ) ;
          strcpy( ErrText, MSG_MSOFC_ZIP_XML_UNZIP_FAILED ) ;
          if ( ExecuteCommand( szCommand, TempFile, ErrText ) ) {

             /*--------------------------------------------------------------*/
             /*  Concatenate all of the XML files together into 1 file.      */
             /*--------------------------------------------------------------*/
             remove( TempFile ) ;
             sprintf( szCommand, "cmd /C dir /s /b %s*.xml > %s", szTempDir2, TempFile ) ;
             ExecuteCommand( szCommand, TempFile, ErrText ) ;
             ReorderFiles( TempFile ) ;           /* Reorder slides for PPTX */
             fTemp = fopen( TempFile, "r" ) ;
             if ( fTemp ) {
                bReturn = TRUE ;
                while ( fgets( szIn, MAX_RCD_LENGTH, fTemp ) != NULL ) {
                   strtok( szIn, "\n" ) ;
                   strcpy( szTempFile, &szIn[strlen(szTempDir2)] ) ;
                   if ( bReturn ) {
                      bReturn = ConcatFiles( XmlFile, szIn, szTempFile ) ;
                      if ( ! bReturn ) {
                         strcpy( ErrText, MSG_MSOFC_ZIP_XML_CONCAT_FAILED ) ;
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
                strcpy( ErrText, MSG_MSOFC_ZIP_XML_DIR_FAILED ) ;
             }
          } else {
             bReturn = FALSE ;
          }

          _rmdir( szTempDir2 ) ;
          _rmdir( szTempDir1 ) ;
       } else {
          strcpy( szTemp, MSG_MSOFC_ZIP_XML_SOURCE_PATH ) ;
          sprintf( ErrText, szTemp, ZipFile ) ;
          bReturn = FALSE ;
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
    char      szSourceFile[256] ;
    char      szZipExe[256] ;
    char      szCommand[1024] ;
    char      szTempDir1[256] ;
    char      szTempDir2[256] ;
    char      szTempZip[256] ;
    char      szSaveDir[256] ;
    char      szTemp[4096] ;
    char      *ptrChar ;
    USHORT    rc ;
    DWORD     dwCode;
    BOOL      bReturn = TRUE;


    /*-----------------------------------------------------------------------*/
    /*  Determine input file format, whether ZIP file or CONTENT.XML         */
    /*-----------------------------------------------------------------------*/
    strcpy( szSourceFile, TargetFile ) ;
    ptrChar = strstr( szSourceFile, "\\TARGET\\" ) ;
    if ( ptrChar ) {
       strncpy( ptrChar, "\\SOURCE\\", 8 ) ;
       GetOTMDllPath( szProgPath, szZipExe ) ;
       strcat( szZipExe, "ZIP.EXE" ) ;
       if ( ! szZipExe[0] ) {
          strcpy( ErrText, MSG_MSOFC_XML_ZIP_MISSING_ZIP ) ;
          bReturn = FALSE ;
       }
    } else {
       strcpy( szTemp, MSG_MSOFC_XML_ZIP_TARGET_PATH ) ;
       sprintf( ErrText, szTemp, TargetFile ) ;
       bReturn = FALSE ;
    }

    if ( bReturn ) {
       /*--------------------------------------------------------------------*/
       /*  XML files must be replaced into the original document ZIP file.   */
       /*    1.  Create temporary directories.                               */
       /*    2.  Split XML file into individual files in temporary directory.*/
       /*    3.  Copy source ZIP to same name in temporary directory.        */
       /*    4.  Zip updated files in temp dir into temp ZIP file.           */
       /*    5.  Copy temp ZIP file to output file.                          */
       /*    6.  Delete temporary directories.                               */
       /*--------------------------------------------------------------------*/

       /*-----------------------------------------------------------------*/
       /*  Create temporary directories.                                  */
       /*-----------------------------------------------------------------*/
       strcpy( szTempDir1, TargetFile ) ;
       ptrChar = strstr( szTempDir1, "\\TARGET\\" ) ;
       strcpy( ptrChar, "\\MISC\\" ) ;
       _mkdir( szTempDir1 ) ;             /* Create \EQF\...\MISC\        */
       strcpy( szTempDir2, szTempDir1 ) ;
       ptrChar = strrchr( TargetFile, '\\' ) ;
       strcat( szTempDir2, ++ptrChar ) ;
       strcat( szTempDir2, "$\\" ) ;
       _mkdir( szTempDir2 ) ;             /* Create \EQF\...\MISC\...\    */

       strcpy( szTempZip, szTempDir1 ) ;
       strcat( szTempZip, ptrChar ) ;
       strcat( szTempZip, ".ZIP" ) ;
       DosCopy( szSourceFile, szTempZip, DCPY_EXISTING ) ;

       /*-----------------------------------------------------------------*/
       /*  Split the one XML file into its individual files.              */
       /*-----------------------------------------------------------------*/
       bReturn = SplitFiles( TargetFile, szTempDir2, ErrText ) ;

       /*-----------------------------------------------------------------*/
       /*  Update the files in the ZIP file.                              */
       /*-----------------------------------------------------------------*/
       if ( bReturn ) {
          _getcwd( szSaveDir, sizeof(szSaveDir) ) ;
          chdir( szTempDir2 ) ;
          sprintf( szCommand, "\"%s\"  -r -f %s *.xml > %s", 
                   szZipExe, szTempZip, TempFile ) ;
          strcpy( ErrText, MSG_MSOFC_XML_ZIP_REPLACE_FAILED ) ;
          bReturn = ExecuteCommand( szCommand, TempFile, ErrText ) ;
          if ( bReturn ) {
             DosCopy( szTempZip, TargetFile, DCPY_EXISTING ) ;
          }
          chdir( szSaveDir ) ;
       }

       /*-----------------------------------------------------------------*/
       /*  Clean-up.                                                      */
       /*-----------------------------------------------------------------*/
       remove( szTempZip ) ;
       sprintf( szCommand, "cmd /C rd /s /q %s > %s", szTempDir2, TempFile ) ;
       ExecuteCommand( szCommand, TempFile, NULL ) ;
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
/*             FromFileName  - File name contained in ZIP file.              */
/*                                                                           */
/*  Return:  TRUE  - Action was successful.                                  */
/*           FALSE - Action failed.                                          */
/*****************************************************************************/

BOOL ConcatFiles( char *BaseFile, char *FromFile, char *FromFileName )
{
    FILE       *fConcat, *fFrom ;
    CHAR       szIn[MAX_RCD_LENGTH*2] ;
    BOOL       bReturn = FALSE ;

    fConcat = fopen( BaseFile, "a" ) ;
    fFrom = fopen( FromFile, "r" ) ;
    if ( ( fConcat ) &&
         ( fFrom   ) ) {
       fprintf( fConcat, ZIP_FILE_SEPARATOR, FromFileName ) ;
       while( fgets( szIn, MAX_RCD_LENGTH, fFrom ) != NULL ) {
          fputs( szIn, fConcat ) ;
       }
       bReturn = TRUE ;
    }

    fclose( fConcat ) ;
    fclose( fFrom ) ;

    return( bReturn ) ;
}



/*****************************************************************************/
/*  SplitFiles                                                               */
/*                                                                           */
/*  Split the combined XML file into its individual files.                   */
/*                                                                           */
/* Input:      XmlFile       - XML file to split into its components.        */
/*             OutputDir     - Output directory to save split files into.    */
/* Output:     ErrText       - Error message if failure.                     */
/*                                                                           */
/*  Return:  TRUE  - Action was successful.                                  */
/*           FALSE - Action failed.                                          */
/*****************************************************************************/

BOOL SplitFiles( char *XmlFile, char *OutputDir, char *ErrText )
{
    FILE       *fSource, *fOut ;
    CHAR       szIn[MAX_RCD_LENGTH*2] ;
    CHAR       szOutputFile[256] ;
    CHAR       szOutputName[256] ;
    CHAR       szTemp[4096] ;
    CHAR       *ptrChar, *ptrChar2 ;
    CHAR       *ptrText, *ptrFile ; 
    USHORT     rc ;
    BOOL       bReturn = TRUE ;

  
    fSource = fopen( XmlFile, "r" ) ;
    if ( ( fSource ) &&
         ( fgets( szIn, MAX_RCD_LENGTH, fSource ) != NULL ) &&
         ( ! strncmp( szIn, ZIP_FILE_SEPARATOR_START, strlen(ZIP_FILE_SEPARATOR_START) ) ) ) {
       while( ( bReturn ) &&
              ( ! strncmp( szIn, ZIP_FILE_SEPARATOR_START, strlen(ZIP_FILE_SEPARATOR_START) ) ) ) {

          /*-----------------------------------------------------------------*/
          /*  Split the next records into a separate XML file.               */
          /*-----------------------------------------------------------------*/
          ptrChar = szIn + strlen( ZIP_FILE_SEPARATOR_START ) ;
          ptrChar2 = strstr( ptrChar, ZIP_FILE_SEPARATOR_END ) ;
          if ( ! ptrChar2 ) {        /* If only part of separator line in buffer 11-6-14 */
             fgets( &szIn[strlen(szIn)], MAX_RCD_LENGTH, fSource ) ;
             ptrChar2 = strstr( ptrChar, ZIP_FILE_SEPARATOR_END ) ;
          }
          if ( ptrChar2 ) {
             *ptrChar2 = 0 ;
             ptrText = ptrChar2 + strlen( ZIP_FILE_SEPARATOR_END ) ; 
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
                   ptrFile = strstr( ptrText, ZIP_FILE_SEPARATOR_COMMENT ) ;  /* 1-5-11 */
                   if ( ( ptrFile ) &&
                        ( ! strstr( ptrFile, ZIP_FILE_SEPARATOR_COMMENT_END ) ) ) {
                      *ptrFile = 0 ;
                      fputs( ptrText, fOut ) ;
                      *ptrFile = '<' ;
                      memmove( szIn, ptrFile, strlen(ptrFile)+1 ) ;
                      ptrText = szIn ;
                      fgets( &szIn[strlen(szIn)], MAX_RCD_LENGTH, fSource ) ;
                   }
                   ptrFile = strstr( ptrText, ZIP_FILE_SEPARATOR_START ) ; 
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
                strcpy( szTemp, MSG_MSOFC_XML_ZIP_CREATE_XML_FILE ) ;
                sprintf( ErrText, szTemp, szOutputFile ) ;
                bReturn = FALSE ;
             }

          } else {
             strcpy( ErrText, MSG_MSOFC_XML_ZIP_CREATE_XML_FORMAT ) ;
             bReturn = FALSE ;
          }
       }
    } else {
       strcpy( ErrText, MSG_MSOFC_XML_ZIP_CREATE_XML_FORMAT ) ;
       bReturn = FALSE ;
    }

    fclose( fSource ) ;
    fclose( fOut ) ;

    return( bReturn ) ;
}



/*****************************************************************************/
/*  ReorderFiles                                                             */
/*                                                                           */
/*  Reorder the PPTX files so that all of the slide information is           */
/*  together in the concatenated file.  This it done so that the             */
/*  speaker notes for a slide come immediately after that slide.             */
/*                                                                           */
/* Input:      ListFile      - List of XML files in this PPTX file.          */
/*                                                                           */
/*  Return:  None.                                                    3-6-15 */
/*****************************************************************************/

VOID ReorderFiles( char *ListFile )
{

typedef struct                           
{
   char      szFile[1024] ;                  // Complete file name
   USHORT    usSlide ;                       // Slide number
   USHORT    usType ;                        // Slide type
   VOID      *ptrNext ;                      // Pointer to next item in linked list 
} SLIDE_INFO ;

#define  SLIDE_TYPE_NONE           0
#define  SLIDE_TYPE_SLIDE          1
#define  SLIDE_TYPE_NOTES          2
#define  SLIDE_TYPE_LAYOUT         3
#define  SLIDE_TYPE_OTHER          4

    SLIDE_INFO *ptrSlideHead ;
    SLIDE_INFO *ptrSlideCur ;
    SLIDE_INFO *ptrSlidePrev ;
    SLIDE_INFO *ptrSlideNew ;

    FILE       *fList ;
    CHAR       szIn[MAX_RCD_LENGTH] ;
    CHAR       szNum[8] ;
    CHAR       szTemp[4096] ;
    CHAR       *ptrChar, *ptrChar2 ;
    USHORT     usSlide ;
    USHORT     usType ;
    BOOL       bPPT = FALSE ;

  
    fList = fopen( ListFile, "r" ) ;
    if ( fList ) {
       
       /*-----------------------------------------------------------*/
       /*  See if this is a PowerPoint file or not.                 */
       /*-----------------------------------------------------------*/
       while( fgets( szIn, MAX_RCD_LENGTH, fList ) != NULL ) {
          if ( strstr( szIn, "notesSlide" ) ) {
             bPPT = TRUE ; 
             break;
          }
       }

       /*-----------------------------------------------------------*/
       /*  Identify which slide each file belongs to.               */
       /*-----------------------------------------------------------*/
       if ( bPPT ) {
          fseek( fList, 0, SEEK_SET ) ;   /* Reset to start of file */

          ptrSlideHead = 0 ;
          while( fgets( szIn, MAX_RCD_LENGTH, fList ) != NULL ) {
             ptrSlideNew = (SLIDE_INFO*)calloc( sizeof(SLIDE_INFO), 1 ) ;
             strcpy( ptrSlideNew->szFile, szIn ) ;
             usSlide = 0 ;
             usType = 0 ;

             ptrChar = strrchr( szIn, '\\' ) ;
             if ( ptrChar ) {
                ++ptrChar ;
                if ( ! strncmp( ptrChar, "notesSlide", 10 ) ) {
                   usType = SLIDE_TYPE_NOTES ; 
                   ptrChar += 10 ;
                } else
                if ( ! strncmp( ptrChar, "slideLayout", 11 ) ) {
                   usType = SLIDE_TYPE_LAYOUT ; 
                   ptrChar += 11 ;
                } else
                if ( ( ! strncmp( ptrChar, "slide", 5 ) ) &&
                     ( isdigit( *(ptrChar+5) ) ) ) {
                   usType = SLIDE_TYPE_SLIDE ; 
                   ptrChar += 5 ;
                } else {
                   usType = SLIDE_TYPE_NONE ;
                }
                ptrSlideNew->usType = usType ;

                if ( usType != SLIDE_TYPE_NONE ) {
                   for( ptrChar2=ptrChar ; *ptrChar2 && isdigit(*ptrChar2) ; ++ptrChar2 ) ;
                   *ptrChar2 = 0 ;
                   usSlide = atoi( ptrChar ) ;
                   ptrSlideNew->usSlide = usSlide ;
                }
             }

             /*-----------------------------------------------------------*/
             /*  Add this node to the list in the right location.         */
             /*-----------------------------------------------------------*/
             ptrSlidePrev = 0 ;
             for( ptrSlideCur=ptrSlideHead ; 
                  ( ( ptrSlideCur ) &&
                    ( ( usSlide > ptrSlideCur->usSlide ) ||
                      ( ( usSlide == ptrSlideCur->usSlide ) &&
                        ( usType  >= ptrSlideCur->usType  ) ) ) ) ; 
                  ptrSlidePrev=ptrSlideCur,
                  ptrSlideCur=(SLIDE_INFO*)ptrSlideCur->ptrNext ) ;
             if ( ptrSlideHead ) {
                if ( ptrSlideCur == ptrSlideHead ) {
                   ptrSlideNew->ptrNext = ptrSlideHead ;
                   ptrSlideHead = ptrSlideNew ;
                } else {
                   ptrSlideNew->ptrNext = ptrSlideCur ;
                   ptrSlidePrev->ptrNext = ptrSlideNew ;
                }
             } else {
                ptrSlideHead = ptrSlideNew ;
             }
          }

          /*-----------------------------------------------------------*/
          /*  Rewrite the files in the proper order.                   */
          /*-----------------------------------------------------------*/
          if ( ptrSlideHead ) {
             fclose( fList ) ;
             fList = fopen( ListFile, "w" ) ;
             if ( fList ) {
                for( ptrSlideCur=ptrSlideHead ; ptrSlideCur ; ptrSlideCur=ptrSlidePrev ) {
                   fputs( ptrSlideCur->szFile, fList ) ;
                   ptrSlidePrev = (SLIDE_INFO*)ptrSlideCur->ptrNext ;
                   free( ptrSlideCur ) ;
                }
             }
          }
       }
    }
    if ( fList )
       fclose( fList ) ;

    return ;
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
       if ( ErrText ) {
          strcat( ErrText, Command ) ;
          strcat( ErrText, "\n" ) ;
          sprintf( szText, "RC=%d   %ld\n", rc, dwCode);
          strcat( ErrText, szText ) ;
          fTemp = fopen( ErrFile, "r" ) ; 
          if ( fTemp ) {
             while( fgets( szText, sizeof(szText), fTemp ) != NULL ) {
                strcat( ErrText, szText ) ;
             }
             fclose( fTemp ) ;
          }
       }
    }

    return( bReturn ) ;
}

