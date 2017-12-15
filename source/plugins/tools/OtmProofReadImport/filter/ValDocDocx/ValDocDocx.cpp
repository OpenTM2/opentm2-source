/*! \file
	Filter code for the conversion of Validation DOCX file into the internal proof read import XML format
	
	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved

 
    CHANGES:
       When     Why    Who  What                                                
     -------- -------  ---  -------------------------                           
      4/11/17           DAW  Created from IBMXMWRD                              
      8/16/17 P403835   DAW  Improve performance. Read file only once and       
                             save text in linked list rather than read twice.   
      9/08/17 P403868   DAW  Abend. 1st file has no entries, fails on 2nd file. 
      9/18/17 P403874   DAW  Handle completely deleted changed text.                 
*/

#include "EQF.H"
#include "EQFFOL.H"
#include "..\..\OtmProofReadFilter.h"

#include <sys/stat.h>
#include <string.h>
#include <ole2.h>
#include <oleauto.h>
#include <direct.h>
#include <sys/stat.h>
#include <tlhelp32.h>
#include "ValDocDocx.h"
#include "ValDocDocxUtil.h"




#define  DOC_STATE_NONE                    1
#define  DOC_STATE_HDR_DOCUMENT            2
#define  DOC_STATE_HDR_FOLDER              3
#define  DOC_STATE_HDR_DATECREATED         4
#define  DOC_STATE_HDR_PROJECT             5
#define  DOC_STATE_HDR_PROJECTMANAGER      6
#define  DOC_STATE_HDR_TRANSLATOR          7
#define  DOC_STATE_HDR_VALIDATOR           8
#define  DOC_STATE_HDR_COMMENTS            9
#define  DOC_STATE_HDR_OTHER              10
#define  DOC_STATE_BODY                   11


#define  RC_OK                    0
#define  RC_ERROR                 1
#define  RC_WARNING               2

#define  UTF82UTF16               1
#define  UTF162UTF8               2

#define  ZIP_FILE_SEPARATOR              "<!-- TWB %s -->\n"

extern   char    szDocTargetLanguage[80];   /* From USRCALLS.C  */
extern   char    szDocSourceLanguage[80];   /* From USRCALLS.C  */
extern   short   sTPVersion ;               /* From USRCALLS.C  */
         char    szProgPath[256] ;

typedef struct {
  ULONG   ulSegNum ;                    /* Segment number                    */
  BOOL    bTrans ;                      /* TRUE=translatable                 */
  USHORT  usMatch ;                     /* Type of match found               */
  char    *SegStart ;                   /* Ptr to start of segment in string */
  char    *SegEnd ;                     /* Ptr to end of segment in string   */
  char    *szText ;                     /* Segment's text                    */
  VOID    *ptrNext ;                    /* pointer to next entry             */
  VOID    *ptrPrev ;                    /* pointer to previous entry         */
} SEGDATA ;

   char     *szERR_Title             = "Preparing Validation Failed" ;
   char     *szERR_EndProcess        = "\n\nProcessing has been terminated." ;

   char     *szERR_MissingInput      = "Input file does not exist:  %s" ;
   char     *szERR_CreatingOutput    = "Output file cannot be created:  %s" ;
   char     *szERR_MissingOtmFolder  = "Original OpenTM2 folder cannot be found:  %s" ;
   char     *szERR_MissingOtmDoc     = "Original OpenTM2 document (%s) in folder (%s) cannot be found." ;
   char     *szERR_BadOtmDocInfo     = "Information cannot be found for original OpenTM2 document (%s) in folder (%s)." ;
   char     *szERR_OpenOtmDoc        = "Original OpenTM2 document (%s) in folder (%s) cannot be opened.  It may be in use by another process." ;
   char     *szERR_SegOpenFail       = "OpenTM2 document cannot be opened:  %s" ;
   char     *szERR_UTF8Conversion    = "OpenTM2 text cannot be converted to UTF-8." ;
   char     *szERR_BadFormat         = "Input Office Word document is not an OpenTM2 validation document." ;
   char     *szERR_Unknown           = "Any unexpected error has occurred.  RC=%ld" ;


   char     *szWARN_Title            = "Preparing Validation Warning" ;
   char     *szWARN_ContinueProcess  = "\nSelect \"OK\" to continue processing.\nSelect \"Cancel\" to terminate this process." ;

   char     *szWARN_MismatchSrc      = "Source validation text does not match source segment text." ;
   char     *szWARN_MismatchTgt      = "Target validation text does not match target segment text." ;
   char     *szWARN_MissingContext   = "Validation context ID cannot be found in OpenTM2 document." ;
   char     *szWARN_BadSegmentData   = "OpenTM2 segment data could not be found for this string." ;
   char     *szWARN_MismatchChg      = "Changed text for segment cannot be found." ;
   char     *szWARN_ExtraSrc         = "Source text has no matching source segment." ;
   char     *szWARN_ExtraTgt         = "Target text has no matching target segment." ;
   char     *szWARN_ExtraChg         = "Changed text has no matching target segment." ;
   char     *szWARN_MissingSrcTag    = "Validation entry does not have a <source> element." ;
   char     *szWARN_MissingTgtTag    = "Validation entry does not have a <target> element." ;

// char     *szWARN_ADD_Context      = "   Context=[%s]" ;
// char     *szWARN_ADD_Text         = "   Text=   [%s]" ;
// char     *szWARN_ADD_Segment      = "   Segment=[%s]" ;
// char     *szWARN_ADD_String       = "   String= [%s]" ;
// char     *szWARN_ADD_SegNum       = "   Seg#   =%ld"  ;



   char     *szMSG_ADD_Context       = "   Context= [%s]" ;
   char     *szMSG_ADD_ValSrc        = "   Val Src= [%s]" ;
   char     *szMSG_ADD_ValTgt        = "   Val Tgt= [%s]" ;
   char     *szMSG_ADD_SegSrc        = "   Seg Src= [%s]" ;
   char     *szMSG_ADD_SegTgt        = "   Seg Tgt= [%s]" ;
   char     *szMSG_ADD_SegNum        = "   Seg#   = %ld"  ;

   char     *szLOG_SegmentNum        = "   Seg#=%ld\n\n" ;

// char     *szWARN_MissingSrc       = "Source text for segment %ld cannot be found.\n\n   Segment=[%s]\n" ;
// char     *szWARN_MissingTgt       = "Target text for segment %ld cannot be found.\n\n   Segment=[%s]\n" ;
// char     *szWARN_ExtraChg = "Changed text has no matching target segment.\n\n   Segment=[%s]\n" ;



   char     *szXML_Declare                = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" ;
   char     *szXML_Root                   = "<ProofReadingResults>\n" ;
   char     *szXML_Header                 = " <Header>\n" ;
   char     *szXML_Header_Folder          = "  <Folder>%s</Folder>\n" ;
   char     *szXML_Header_Creation        = "  <CreationDate>%s</CreationDate>\n" ;
   char     *szXML_Header_Proofread       = "  <ProofReadDate></ProofReadDate>\n" ;
   char     *szXML_Header_Translator      = "  <Translator></Translator>\n" ;
   char     *szXML_Header_Proofreader     = "  <ProofReader></ProofReader>\n" ;
   char     *szXML_Header_LogFile         = "  <LogFile>%s</LogFile>\n" ;
   char     *szXML_Header_End             = " </Header>\n" ;
   char     *szXML_DocumentList           = " <DocumentList>\n" ;
   char     *szXML_Document               = "  <Document Name=\"%s\" SourceLang=\"%s\" TargetLang=\"%s\" Markup=\"%s\">\n" ;
   char     *szXML_SegmentList            = "   <SegmentList>\n" ;
   char     *szXML_Segment                = "    <Segment Number=\"%d\" Selected=\"no\" Processed=\"no\">\n" ;
   char     *szXML_Segment_2              = "    <Segment Number=\"%d\" Length=\"%d\" Selected=\"no\" Processed=\"no\">\n" ;
   char     *szXML_Segment_Source         = "     <Source>%s</Source>\n" ;
   char     *szXML_Segment_OrgTarget      = "     <OrgTarget>%s</OrgTarget>\n" ;
   char     *szXML_Segment_ModTarget      = "     <ModTarget>%s</ModTarget>\n" ;
   char     *szXML_Segment_ModTarget_Del  = "     <ModTarget type=\"deleted\"></ModTarget>\n" ;
   char     *szXML_Segment_NewTarget      = "     <NewTarget>%s</NewTarget>\n" ;
   char     *szXML_Segment_Comment        = "     <Comment>%s</Comment>\n" ;
   char     *szXML_Segment_Problem        = "     <Problem>%s</Problem>\n" ;
   char     *szXML_Segment_End            = "    </Segment>\n" ;
   char     *szXML_SegmentList_End        = "   </SegmentList>\n" ;
   char     *szXML_Document_End           = "  </Document>\n" ;
   char     *szXML_DocumentList_End       = " </DocumentList>\n" ;
   char     *szXML_Root_End               = "</ProofReadingResults>\n" ;
                                         
   char     *szLOGOUT_Separator           = "----------------------------------------------------------------------------------------------------\n" ;
   char     *szLOGOUT_SeparatorLine       = "Validation\n--Line----------------------------------------------------------------------------------------------\n" ;
   char     *szLOGOUT_H1                  = "Extraction of Validation Changes from Validation DOC/DOCX file                 %02d/%02d/%02d %02d:%02d \n" ;
   char     *szLOGOUT_H2                  = "    OpenTM2 Folder:      %s\n" ;
   char     *szLOGOUT_H3                  = "    OpenTM2 Document:    %s\n" ;
   char     *szLOGOUT_H4                  = "    Validation File:     %s\n" ;
   char     *szLOGOUT_H5                  = "    Source Language:     %s\n" ;
   char     *szLOGOUT_H6                  = "    Target Language:     %s\n" ;
                                         
   char     *szLOGOUT_S1                  = "    Strings:\n" ;
   char     *szLOGOUT_S2                  = "        Changed:       %d\n" ;
   char     *szLOGOUT_S3                  = "        Unchanged:     %d\n" ;
   char     *szLOGOUT_S4                  = "        Invalid:       %d\n" ;
   char     *szLOGOUT_S5                  = "        Total:         %d\n" ;
   char     *szLOGOUT_S6                  = "    Segments (changed strings only):\n" ;
   char     *szLOGOUT_S7                  = "        Changed:       %d\n" ;
   char     *szLOGOUT_S8                  = "        Unchanged:     %d\n" ;
   char     *szLOGOUT_S9                  = "        Invalid:       %d\n" ;
   char     *szLOGOUT_S10                 = "        Total:         %d\n" ;
   char     *szLOGOUT_S11                 = "                                                                               %02d/%02d/%02d %02d:%02d \n" ;
                                         
   char     *szLOGOUT_ERROR               = "\n  *** NOTE:  Processing was terminated by the user.\n\n" ;
   char     *szLOGOUT_NewFile             = "    New OpenTM2 document:    %s\n" ;

   char     cMATCH_CHAR = '\x1F' ;


   CHAR   *TITLE_FILE_CONVERSION                ="File Conversion Error";
   CHAR   *MSG_FILE_CONVERSION                  ="Code page conversion failed.\n\nProcessing is terminated.";

   CHAR   *TITLE_XMWRD_WORD_XML_ERROR           ="Word-to-XML Conversion Error";

   CHAR  *MSG_XMWRD_ZIP_XML_MISSING_UNZIP       ="UNZIP.EXE could not be found to extract the XML files from the Office Word file.\n\nInstall UNZIP.EXE in a directory which is in your PATH environment variable.";
   CHAR  *MSG_XMWRD_ZIP_XML_UNZIP_FAILED        ="XML files could not be extracted from the Office Word file.\n\nUNZIP errors:\n";
   CHAR  *MSG_XMWRD_ZIP_XML_CONCAT_FAILED       ="XML files could not be concatenated together.\n\nProcessing is terminated.\n";
   CHAR  *MSG_XMWRD_ZIP_XML_DIR_FAILED          ="Could not determine the XML files contained in the Office Word file.\n\nProcessing is terminated.\n";
   CHAR  *MSG_XMWRD_ZIP_XML_SOURCE_PATH         ="Source file path is not correct: %s";



   FILE     *fLog ;

   char     szErrMsg[MAX_RCD_LENGTH] ;   
   char     szProblemMsg[256] ;

   ULONG    ulStringsTotal ;
   ULONG    ulStringsChanged ;
   ULONG    ulStringsInvalid ;  
   ULONG    ulStringsUnchanged ;

   ULONG    ulValLineNumber ;
   ULONG    ulValIdLineNumber ;
   ULONG    ulValSrcLineNumber ;
   ULONG    ulValTgtLineNumber ;
   ULONG    ulValChgLineNumber ;

   USHORT   usWarnAction ;

USHORT  StripText( char *, char *, USHORT, BOOL * );
BOOL    WriteXmlSegment( FILE *, ULONG, USHORT, char *, char *, char *, char *, char *, BOOL ) ;
BOOL    ConvertFile( char *, char *, USHORT ) ;
void    BuildMsg( char *, char *, int, char *, ULONG ) ;
void    AddLogError( char * ) ;
void    AddLogWarning( char * ) ;
void    ShowError( char * ) ;
USHORT  ShowWarning( char * ) ;
BOOL    ConvertDocToDocx(char*, char*, char*, char* );
BOOL    GetWordProcessList( ULONG * ); 
BOOL    AutomationWrapper(int,VARIANT*,IDispatch*,char*,char*,int ...);



BOOL   bMainDebug = FALSE ;
BOOL   bTimeDebug = FALSE ;
FILE   *fTimeDebug ;
char   szTimeDebug[256] ;


/*! \brief Convert a file returned from the proof reading process into the internal proof read XML document format
  \param pszProofReadInFile fully qualified input file selected by the user
  \param pszProofReadXMLOut name of the XML output file to be created
  \returns 0 if successful or an error code
*/
int convertToProofReadFormat( const char *pszProofReadInFile, const char *pszProofReadXMLOut )
{

    FILE     *fIn, *fOut ;

    SYSTEMTIME  TimeStamp ;


    char     szIn[MAX_RCD_LENGTH] ;

    ULONG    ulSegmentID = 0 ;
    char     szSegmentSource[MAX_RCD_LENGTH] ;
    char     szSegmentTarget[MAX_RCD_LENGTH] ;
    char     szSegmentChanged[MAX_RCD_LENGTH] ;

    char     szDocument[256] ; 
    char     szFolder[256] ;
    char     szDateCreated[80] ; 
    char     szProject[256] ;
    char     szProjectManager[80] ; 
    char     szTranslator[80] ; 
    char     szValidator[80] ;
    char     szComments[256] ;
    char     szSourceLang[80] ;
    char     szTargetLang[80] ;
    char     szMarkup[80] ;
    char     szLogFile[256] ;

    char     szFolderPath[512] ;
    char     szDocShortName[20] ;
    char     szDocFullPath[512] ;
    char     szTemp[MAX_RCD_LENGTH] ;
    char     szTemp2[MAX_RCD_LENGTH] ;
    char     *ptrCur, *ptrEnd ;
    char     *ptrText ;
    char     *ptr1, *ptr2 ;


    ULONG    ulFilePos ;

    USHORT   usDocState ;
    USHORT   usBodyColumn ;
    USHORT   usColSeg = 0 ;
    USHORT   usColSrc = 0 ;
    USHORT   usColTgt = 0 ;

    USHORT   usWriteHeader = 1 ;
    USHORT   usReturn = RC_OK ;
    USHORT   usReturn2 = RC_OK ;

    char    TempFile1[512], TempFile2[512], TempFile3[512] ;
    int     rc;
    BOOL    bSegTextDeleted = FALSE ;
    BOOL    bReturn = TRUE ;


    /*------------------------------------------------------------------------*/
    /*  Initialize processing.                                                */
    /*------------------------------------------------------------------------*/
    for( ptr1=(char*)pszProofReadXMLOut, ptr2=TempFile1 ; *ptr1 ; ++ptr1 ) {
       if ( !isspace( *ptr1 ) )             /* Remove all blanks */
          *(ptr2++) = *ptr1 ;
    }
    *ptr2 = 0 ; 
    strcpy( TempFile2, TempFile1 ) ;
    strcpy( TempFile3, TempFile1 ) ;
    strcpy( szTimeDebug, TempFile1 ) ;
    strcat( TempFile1, ".$TEMP1$" ) ;
    strcat( TempFile2, ".$TEMP2$" ) ;
    strcat( TempFile3, ".$TEMP3$" ) ;

    if ( bTimeDebug ) {
       strcat( szTimeDebug,".$DEBUG$" ) ;
       fTimeDebug = fopen( szTimeDebug, "wb");
       GetLocalTime( (LPSYSTEMTIME) &TimeStamp ) ;
       fprintf( fTimeDebug, "-----------       %02d/%02d/%02d %02d:%02d:%02d\n",TimeStamp.wYear-2000, TimeStamp.wMonth, TimeStamp.wDay,TimeStamp.wHour, TimeStamp.wMinute, TimeStamp.wSecond ) ;                  
       fprintf( fTimeDebug, "Input:    %s\n",pszProofReadInFile);
       fprintf( fTimeDebug, "Output:   %s\n",pszProofReadXMLOut);
       fprintf( fTimeDebug, "Temp1:    %s\n",TempFile1);
       fprintf( fTimeDebug, "Temp2:    %s\n",TempFile2);
       fprintf( fTimeDebug, "Temp3:    %s\n",TempFile3);
    }


    /*------------------------------------------------------------------------*/
    /*  For DOC files, convert the file to a DOCX file using Word.            */
    /*------------------------------------------------------------------------*/
    ptr1 = strrchr( (char*)pszProofReadInFile, '.' ) ;
    if ( ptr1 ) {
       strcpy( szTemp, ptr1+1 ) ;
       strupr( szTemp ) ;
       if ( ! strcmp( szTemp, "DOC" ) ) {
          if ( bTimeDebug ) {
             GetLocalTime( (LPSYSTEMTIME) &TimeStamp ) ;
             fprintf( fTimeDebug, "-----------       %02d/%02d/%02d %02d:%02d:%02d   DOC->DOCX\n",TimeStamp.wYear-2000, TimeStamp.wMonth, TimeStamp.wDay,TimeStamp.wHour, TimeStamp.wMinute, TimeStamp.wSecond ) ;                  
          }
          if ( ConvertDocToDocx( (char*)pszProofReadInFile, TempFile1, TempFile2, szErrMsg ) ) {
             if ( bTimeDebug ) {
                GetLocalTime( (LPSYSTEMTIME) &TimeStamp ) ;
                fprintf( fTimeDebug, "-----------       %02d/%02d/%02d %02d:%02d:%02d   EXTRACT XML FROM ZIP\n",TimeStamp.wYear-2000, TimeStamp.wMonth, TimeStamp.wDay,TimeStamp.wHour, TimeStamp.wMinute, TimeStamp.wSecond ) ;                  
             }
             if ( ! ExtractXmlFromZip( TempFile1, TempFile2, TempFile3, szErrMsg ) ) {
                ShowIBMMessage( TITLE_XMWRD_WORD_XML_ERROR, szErrMsg, FALSE, FALSE ) ;
                usReturn = RC_ERROR ;
                remove( TempFile2 ) ;
             }
          } else {
             ShowIBMMessage( TITLE_XMWRD_WORD_XML_ERROR, szErrMsg, FALSE, FALSE ) ;
             usReturn = RC_ERROR ;
          }
          remove( TempFile1 ) ;
          remove( TempFile3 ) ;
       } else {
          if ( bTimeDebug ) {
             GetLocalTime( (LPSYSTEMTIME) &TimeStamp ) ;
             fprintf( fTimeDebug, "-----------       %02d/%02d/%02d %02d:%02d:%02d   EXTRACT XML FROM ZIP\n",TimeStamp.wYear-2000, TimeStamp.wMonth, TimeStamp.wDay,TimeStamp.wHour, TimeStamp.wMinute, TimeStamp.wSecond ) ;                  
          }
          if ( ! ExtractXmlFromZip( (char*)pszProofReadInFile, TempFile2, TempFile3, szErrMsg ) ) {
             ShowIBMMessage( TITLE_XMWRD_WORD_XML_ERROR, szErrMsg, FALSE, FALSE ) ;
             usReturn = RC_ERROR ;
             remove( TempFile3 ) ;
          }
       }
    } else {
       ShowError( MSG_XMWRD_ZIP_XML_DIR_FAILED ) ;
    }

    if ( usReturn == RC_OK ) {
       if ( bTimeDebug ) {
          GetLocalTime( (LPSYSTEMTIME) &TimeStamp ) ;
          fprintf( fTimeDebug, "-----------       %02d/%02d/%02d %02d:%02d:%02d   PARSE XML\n",TimeStamp.wYear-2000, TimeStamp.wMonth, TimeStamp.wDay,TimeStamp.wHour, TimeStamp.wMinute, TimeStamp.wSecond ) ;                  
       }
       bReturn = Parse( TempFile2, TempFile1 ) ;
       if ( bReturn ) {
          if ( bTimeDebug ) {
             GetLocalTime( (LPSYSTEMTIME) &TimeStamp ) ;
             fprintf( fTimeDebug, "-----------       %02d/%02d/%02d %02d:%02d:%02d   UTF8->UTF16\n",TimeStamp.wYear-2000, TimeStamp.wMonth, TimeStamp.wDay,TimeStamp.wHour, TimeStamp.wMinute, TimeStamp.wSecond ) ;                  
          }
          rc = ConvertFile( TempFile1, TempFile2, UTF162UTF8 ) ;
          if ( !rc ) {
              ShowIBMMessage( TITLE_FILE_CONVERSION, MSG_FILE_CONVERSION, FALSE, FALSE ) ;
              usReturn = RC_ERROR ;
          }
       } else {
          usReturn = RC_ERROR ;
       }
    }
    remove( TempFile1 ) ;
    if ( usReturn != RC_OK ) {
       remove( TempFile2 ) ;
       remove( TempFile3 ) ;
       return( usReturn ) ;
    }


    /*------------------------------------------------------------------------*/
    /*  Initialize processing.                                                */
    /*------------------------------------------------------------------------*/
    fLog = NULL ;
    if ( bTimeDebug ) {
       GetLocalTime( (LPSYSTEMTIME) &TimeStamp ) ;
       fprintf( fTimeDebug, "-----------       %02d/%02d/%02d %02d:%02d:%02d   CREATE XML\n",TimeStamp.wYear-2000, TimeStamp.wMonth, TimeStamp.wDay,TimeStamp.wHour, TimeStamp.wMinute, TimeStamp.wSecond ) ;                  
    }

    szDocument[0] = 0 ; 
    szFolder[0] = 0 ;
    szDateCreated[0] = 0 ; 
    szProject[0] = 0 ;
    szProjectManager[0] = 0 ; 
    szTranslator[0] = 0 ; 
    szValidator[0] = 0 ;
    szComments[0] = 0 ;
    szSourceLang[0] = 0 ;
    szTargetLang[0] = 0 ;
    szMarkup[0] = 0 ;

    ulStringsTotal = 0 ;
    ulStringsChanged = 0 ;
    ulStringsInvalid = 0 ;
    ulStringsUnchanged = 0 ;
    ulValLineNumber = 0 ;

    fIn = fopen( TempFile2, "rb" ) ;
    if ( ! fIn ) {
       usReturn = RC_ERROR ;
       sprintf( szErrMsg, szERR_MissingInput, TempFile2 ) ;
       ShowError( szErrMsg ) ;
    }

    fOut = NULL ;
    if ( usReturn == RC_OK ) {
       fOut = fopen( pszProofReadXMLOut, "wb" ) ;
       if ( ! fOut ) {
          usReturn = RC_ERROR ;
          fclose( fIn ) ;
          sprintf( szErrMsg, szERR_CreatingOutput, pszProofReadXMLOut ) ;
          ShowError( szErrMsg ) ;
       }
    }

    /*------------------------------------------------------------------------*/
    /*  Set up log file.                                                      */
    /*------------------------------------------------------------------------*/
    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
 // strcpy( szLogFile, "C:\\otm\\win" ) ;
 // if ( ( isalpha( szLogFile[0] ) ) &&
 //      ( szLogFile[1] == ':'     ) &&
 //      ( szLogFile[2] == '\\'    ) ) {
 //    ptr1 = strchr( &szLogFile[3], '\\' ) ;
 //    if ( ptr1 ) { 
 //       *(ptr1+1) = 0 ;
 //    } else {
 //       szLogFile[3] = 0 ;
 //       strcat( szLogFile, "OTM\\" ) ;
 //    }
 //    strcat( szLogFile, "LOGS\\" ) ;
 // } else {
 //    strcpy( szLogFile, "c:\\OTM\\LOGS\\" ) ;
 // }


    /*------------------------------------------------------------------------*/
    /*  Process each line.                                                    */
    /*------------------------------------------------------------------------*/
    usDocState = DOC_STATE_NONE ; 
    usBodyColumn = 0 ;

    while ( ( usReturn != RC_ERROR ) &&
            ( fgets( szIn, MAX_RCD_LENGTH, fIn ) != NULL ) ) {
       ++ulValLineNumber ;

       /*------------------------------------------------------------------------*/
       /*  Look at each character of the line.                                   */
       /*------------------------------------------------------------------------*/
       for( ptrCur=szIn ; *ptrCur ; ++ptrCur ) {

          /*---------------------------------------------------------------------*/
          /*  Start of a new paragraph.                                          */
          /*---------------------------------------------------------------------*/
          if ( ! strncmp( ptrCur, "<w:p ", 5 ) ) {
             ptrEnd = strstr( ptrCur, "</w:p>" ) ;
             while ( ( !ptrEnd ) && 
                     ( fgets( szTemp, MAX_RCD_LENGTH, fIn ) != NULL ) ) {
                strcat( szIn, szTemp ) ;
                ptrEnd = strstr( ptrCur, "</w:p>" ) ;
             }
             if ( ! ptrEnd ) {
                //  ERROR
                usReturn = usReturn2 = RC_ERROR ;
                break;
             }
             if ( usDocState == DOC_STATE_BODY ) {
                strcpy( szTemp, ptrCur ) ;
                ptr1 = strchr( szTemp, '>' ) ;
                if ( ptr1 ) {
                   *ptr1 = 0 ;
                   if ( strstr( szTemp, " tr=\"1\"" ) ) {
                      usBodyColumn = 0 ;
                      szSegmentSource[0] = 0 ;
                      szSegmentTarget[0] = 0 ;
                      szSegmentChanged[0] = 0 ;
                   }
                }
             }
             ptrText = strchr( ptrCur, '>' ) ;
             ++ptrText ;
             *ptrEnd = 0 ;

             /*---------------------------------------------------------------------*/
             /*  Check if the start of another document section.                    */
             /*---------------------------------------------------------------------*/
             if ( ( usDocState == DOC_STATE_BODY ) &&
                  ( usBodyColumn == 0 ) &&
                  ( strstr( ptrText, "Document" ) ) ) {
                StripText( ptrText, szTemp, (USHORT)1, NULL ) ;
                if ( ! strcmp( szTemp, "Document:" ) ) {
                   ulFilePos = ftell( fIn ) ;
                   if ( ( fgets( szTemp, MAX_RCD_LENGTH, fIn ) != NULL ) &&
                        ( fgets( szTemp, MAX_RCD_LENGTH, fIn ) != NULL ) &&
                        ( ! strncmp( szTemp, "<w:p ", 5 ) ) ) {
                      ptr1 = strstr( szTemp, "</w:p>" ) ;
                      if ( ptr1 )
                         *ptr1 = 0 ;
                      ptr1 = strchr( szTemp, '>' ) ;
                      if ( ptr1 ) {
                         ++ptr1 ;
                         StripText( ptr1, szTemp2, (USHORT)1, NULL ) ;
                         if ( ! strcmp( szTemp2, "Folder:" ) ) {
                            usDocState = DOC_STATE_NONE ;
                            if ( usWriteHeader == 0 )         /* 9-8-17 */
                               usWriteHeader = 2 ;
                         }
                      }
                   }
                }
                fseek( fIn, ulFilePos, 0 ) ;
             }

             /*---------------------------------------------------------------------*/
             /*  Handle header part of document.                                    */
             /*---------------------------------------------------------------------*/
             if ( usDocState != DOC_STATE_BODY ) {
                StripText( ptrText, szTemp, (USHORT)1, NULL ) ;
                if ( ! strcmp( szTemp, "Document:" ) ) {
                   usDocState = DOC_STATE_HDR_DOCUMENT ; 
                } else
                if ( ! strcmp( szTemp, "Folder:" ) ) {
                   usDocState = DOC_STATE_HDR_FOLDER ; 
                } else
                if ( ! strcmp( szTemp, "Date created:" ) ) {
                   usDocState = DOC_STATE_HDR_DATECREATED ;
                } else
                if ( ! strcmp( szTemp, "Project:" ) ) {
                   usDocState = DOC_STATE_HDR_PROJECT ; 
                } else
                if ( ! strcmp( szTemp, "Project manager:" ) ) {
                   usDocState = DOC_STATE_HDR_PROJECTMANAGER ;
                } else
                if ( ! strcmp( szTemp, "Translator:" ) ) {
                   usDocState = DOC_STATE_HDR_TRANSLATOR ;
                } else
                if ( ! strcmp( szTemp, "Validator:" ) ) {
                   usDocState = DOC_STATE_HDR_VALIDATOR ;
                } else
                if ( ! strcmp( szTemp, "Comments:" ) ) {
                   usDocState = DOC_STATE_HDR_COMMENTS ; 
                } else

                if ( ! strcmp( szTemp, "Translation" ) ) {
                   usDocState = DOC_STATE_BODY ;
                } else {
                   if ( ! strncmp( szTemp+strlen(szTemp)-2, "\xC2\xA0", 2 )  ) 
                      *(szTemp+strlen(szTemp)-2) = 0 ;
                   if ( usDocState == DOC_STATE_HDR_DOCUMENT ) {
                      strcpy( szDocument, szTemp ) ;
                      usDocState = DOC_STATE_HDR_OTHER ;
                   } else
                   if ( usDocState == DOC_STATE_HDR_FOLDER ) {
                      strcpy( szFolder, szTemp ) ;
                      usDocState = DOC_STATE_HDR_OTHER ;
                   } else
                   if ( usDocState == DOC_STATE_HDR_DATECREATED ) {
                      strcpy( szDateCreated, szTemp ) ;
                      usDocState = DOC_STATE_HDR_OTHER ;
                   } else
                   if ( usDocState == DOC_STATE_HDR_PROJECT ) {
                      strcpy( szProject, szTemp ) ;
                      usDocState = DOC_STATE_HDR_OTHER ;
                   } else
                   if ( usDocState == DOC_STATE_HDR_PROJECTMANAGER ) {
                      strcpy( szProjectManager, szTemp ) ;
                      usDocState = DOC_STATE_HDR_OTHER ;
                   } else
                   if ( usDocState == DOC_STATE_HDR_TRANSLATOR ) {
                      strcpy( szTranslator, szTemp ) ;
                      usDocState = DOC_STATE_HDR_OTHER ;
                   } else
                   if ( usDocState == DOC_STATE_HDR_VALIDATOR ) {
                      strcpy( szValidator, szTemp ) ;
                      usDocState = DOC_STATE_HDR_OTHER ;
                   } else
                   if ( usDocState == DOC_STATE_HDR_COMMENTS ) {
                      strcpy( szComments, szTemp ) ;
                      usDocState = DOC_STATE_HDR_OTHER ;
                   }
                }

                if ( usColSeg == 0 ) {
                   if ( ( strstr( ptrText, ">Seg<" ) ) &&
                        ( strstr( ptrText, ">#<" ) ) ) { 
                      usBodyColumn = 1 ;
                      usColSeg = 1 ;
                   }
                } else 
                if ( ( usColSrc == 0 ) ||
                     ( usColTgt == 0 ) ) {
                   ++usBodyColumn ;
                   if ( ! strcmp( ptrText, "Source" ) ) {
                      usColSrc = usBodyColumn ;
                   } else
                   if ( ! strcmp( ptrText, "Translation" ) ) {
                      usColTgt = usBodyColumn ;
                      usBodyColumn = 0 ;
                   } 
                }
                continue ;
             }

             /*---------------------------------------------------------------------*/
             /*  Write output XML and log headers.                                  */
             /*---------------------------------------------------------------------*/
             if ( ( usDocState == DOC_STATE_BODY ) &&
                  ( usWriteHeader ) ) {

                /*------------------------------------------------------------------*/
                /*  Write output XML header.                                        */
                /*------------------------------------------------------------------*/
                if ( usWriteHeader == 1 ) {
                   BOOL fIsNew ;
                   if ( SubFolNameToObjectName( szFolder, szFolderPath ) ) {
                      if ( ! FolLongToShortDocName( szFolderPath, szDocument, szDocShortName, &fIsNew ) ) {
                         strcpy( szTemp, szFolderPath ) ;
                         strcat( szTemp, "\\" ) ;
                         strcat( szTemp, szDocShortName ) ;
                         if ( DocQueryInfo2( szTemp, NULL, szMarkup, szSourceLang, szTargetLang, NULL, NULL, NULL, FALSE ) ) {
                            usReturn = RC_ERROR ;
                            sprintf( szErrMsg, szERR_BadOtmDocInfo, szDocument, szFolder ) ;
                            ShowError( szErrMsg ) ;
                         }
                      } else {
                         usReturn = RC_ERROR ;
                         sprintf( szErrMsg, szERR_MissingOtmDoc, szDocument, szFolder ) ;
                         ShowError( szErrMsg ) ;
                      }
                   } else {
                      usReturn = RC_ERROR ;
                      sprintf( szErrMsg, szERR_MissingOtmFolder, szFolder ) ;
                      ShowError( szErrMsg ) ;
                   }

                   fputs( szXML_Declare, fOut ) ;
                   fputs( szXML_Root, fOut ) ;
                   fputs( szXML_Header, fOut ) ;
                   fprintf( fOut, szXML_Header_Folder, szFolder ) ;
                   fprintf( fOut, szXML_Header_Creation, szDateCreated ) ;
                   fprintf( fOut, szXML_Header_Proofread, "" ) ;
                   fprintf( fOut, szXML_Header_Translator, szTranslator ) ;
                   fprintf( fOut, szXML_Header_Proofreader, szValidator ) ;
                   strcat(szLogFile, "\\v_" ) ;
                   for( ptr1=szDocument,ptr2=&(szLogFile[strlen(szLogFile)]); *ptr1 ; ++ptr1 ) {
                      if ( isalnum( *ptr1 ) ) 
                         (*ptr2++) = *ptr1 ; 
                   }
                   *ptr2 = 0 ;
                   strcat(szLogFile, ".LOG" ) ;
                   fprintf( fOut, szXML_Header_LogFile, szLogFile ) ;
                   fputs( szXML_Header_End, fOut ) ;
                   fputs( szXML_DocumentList, fOut ) ;
                } else
                if ( usWriteHeader == 2 ) {
                   fputs( szXML_SegmentList_End, fOut ) ;
                   fputs( szXML_Document_End, fOut ) ;
                }

                fprintf( fOut, szXML_Document, szDocument, szSourceLang, szTargetLang, szMarkup ) ;
                fputs( szXML_SegmentList, fOut ) ;
                fflush( fOut ) ;


                /*------------------------------------------------------------------*/
                /*  Write output log header.                                        */
                /*------------------------------------------------------------------*/
                if ( usWriteHeader == 1 ) {
                   fLog = fopen( szLogFile, "wb" ) ;
                   GetLocalTime( (LPSYSTEMTIME) &TimeStamp ) ;
                   fprintf( fLog, szLOGOUT_H1,  TimeStamp.wYear-2000, TimeStamp.wMonth, TimeStamp.wDay, 
                                                TimeStamp.wHour, TimeStamp.wMinute ) ;
                   fputs( szLOGOUT_Separator, fLog ) ;
                   fprintf( fLog, szLOGOUT_H2, szFolder ) ;
                   fprintf( fLog, szLOGOUT_H3, szDocument ) ;
                   fprintf( fLog, szLOGOUT_H4, pszProofReadInFile ) ;
                   fprintf( fLog, szLOGOUT_H5, szSourceLang ) ;
                   fprintf( fLog, szLOGOUT_H6, szTargetLang ) ;
                   fputs( szLOGOUT_SeparatorLine, fLog ) ;
                } else
                if ( usWriteHeader == 2 ) {
                   fputs( szLOGOUT_Separator, fLog ) ;
                   fprintf( fLog, szLOGOUT_NewFile, szDocument ) ;
                   fputs( szLOGOUT_Separator, fLog ) ;
                }
                fflush( fLog ) ;

                usWriteHeader = 0 ;
             }

             /*---------------------------------------------------------------------*/
             /*  Handle body part of document.                                      */
             /*---------------------------------------------------------------------*/
             if ( usDocState == DOC_STATE_BODY ) {
                ++usBodyColumn ; 
                if ( usBodyColumn == usColSeg ) {              /* Segment number    */ 
                   ulSegmentID = atol( ptrText ) ;
                   if ( ! ulSegmentID ) {
                      //  Error
                   }
                   szSegmentSource[0] = 0 ;
                   szSegmentTarget[0] = 0 ;
                   szSegmentChanged[0] = 0 ;
                   ++ulStringsTotal ;
                   ++ulStringsUnchanged ;
                } else
                if ( usBodyColumn == usColSrc ) {              /* Source text       */
                   StripText( ptrText, szSegmentSource, (USHORT)1, NULL ) ;
                } else
                if ( usBodyColumn == usColTgt ) {              /* Target text       */ 
                   usBodyColumn = 0 ;
                   StripText( ptrText, szSegmentTarget, (USHORT)2, NULL ) ;
                   StripText( ptrText, szSegmentChanged, (USHORT)3, &bSegTextDeleted ) ;

//                 if ( ( szSegmentSource[0]  ) &&     /* If different changed text */
//                      ( szSegmentTarget[0]  ) &&
// Write only changed   ( szSegmentChanged[0] ) &&
// segments.            ( strcmp( szSegmentChanged, szSegmentTarget ) ) ) {
//                    ++ulStringsChanged ;
//                    --ulStringsUnchanged ;
//                    WriteXmlSegment( fOut,   
//                                     ulSegmentID, 0,
//                                     szSegmentSource,
//                                     szSegmentTarget,
//                                     szSegmentChanged,
//                                     "", "" ) ;
//                    if ( usReturn2 != RC_OK ) {
//                       if ( usReturn2 == RC_WARNING ) 
//                          usReturn = RC_WARNING ; 
//                       else
//                          usReturn = RC_ERROR ; 
//                    }
//                 } else {                              /* Found change text */
//                    if ( ( ! szSegmentSource[0]  ) ||   /*   but no source   */
//                         ( ! szSegmentTarget[0]  ) ) {  /*   or  no target   */
//                       if ( ! szSegmentSource[0]  ) 
//                          strcpy( szProblemMsg, szWARN_MissingSrcTag ) ;
//                       else
//                          strcpy( szProblemMsg, szWARN_MissingTgtTag ) ;
//                 //    BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValIdLineNumber ) ;
//                 //    BuildMsg( szErrMsg, szMSG_ADD_Context, 2, szUniqueId, NULL ) ;
//                       if ( ShowWarning( szErrMsg ) == IDOK ) 
//                          usReturn = usReturn2 = RC_WARNING ;
//                       else
//                          usReturn = usReturn2 = RC_ERROR ;
//                       WriteXmlSegment( fOut,      /* Error */
//                                        0, 0,
//                                        szSegmentSource,
//                                        szSegmentTarget,
//                                        szSegmentChanged, 
//                                        NULL, szProblemMsg ) ;
//                       break ;
//                    } else {
//                       /* translated text was unchanged. */
//                    }
//                 }

                   /* Write all segments */
                   if ( ( ! szSegmentSource[0]  ) ||   /* No source text.   */
                        ( ! szSegmentTarget[0]  ) ) {  /* No target text.   */
                      if ( ! szSegmentSource[0]  ) 
                         strcpy( szProblemMsg, szWARN_MissingSrcTag ) ;
                      else
                         strcpy( szProblemMsg, szWARN_MissingTgtTag ) ;
                      if ( ShowWarning( szErrMsg ) == IDOK ) 
                         usReturn = usReturn2 = RC_WARNING ;
                      else
                         usReturn = usReturn2 = RC_ERROR ;
                      WriteXmlSegment( fOut,      /* Error */
                                       0, 0,
                                       szSegmentSource,
                                       szSegmentTarget,
                                       szSegmentChanged, 
                                       NULL, szProblemMsg, FALSE ) ;
                      break ;
                   } else {
                      if ( ( ( szSegmentChanged[0] ) ||
                             ( bSegTextDeleted     ) ) &&
                           ( strcmp( szSegmentChanged, szSegmentTarget ) ) ) {
                         ++ulStringsChanged ;
                         --ulStringsUnchanged ;
                      } else {
                         szSegmentChanged[0] = 0 ;
                      }
                      WriteXmlSegment( fOut,   
                                       ulSegmentID, 0,
                                       szSegmentSource,
                                       szSegmentTarget,
                                       szSegmentChanged,
                                       "", "", bSegTextDeleted ) ;
                      if ( usReturn2 != RC_OK ) {
                         if ( usReturn2 == RC_WARNING ) 
                            usReturn = RC_WARNING ; 
                         else
                            usReturn = RC_ERROR ; 
                      }
                   }
                }
             }
          }
       }
    }

    /*------------------------------------------------------------------------*/
    /*  Processing is completed.                                              */
    /*------------------------------------------------------------------------*/
    if ( fOut ) {
       fputs( szXML_SegmentList_End, fOut ) ;
       fputs( szXML_Document_End, fOut ) ;
       fputs( szXML_DocumentList_End, fOut ) ;
       fputs( szXML_Root_End, fOut ) ;
    }
    if ( usDocState != DOC_STATE_BODY ) { 
       usReturn = RC_ERROR ;
       strcpy( szErrMsg, szERR_BadFormat ) ;
       ShowError( szErrMsg ) ;
    }
    
    if ( fIn ) 
       fclose( fIn ) ;
    if ( fOut ) 
       fclose( fOut ) ;
    if ( fLog ) {
       if ( usReturn == RC_ERROR ) 
          fputs( szLOGOUT_ERROR, fLog ) ;
       fputs( szLOGOUT_Separator, fLog ) ;
       fputs( szLOGOUT_S1, fLog ) ;
       fprintf( fLog, szLOGOUT_S2, ulStringsChanged ) ;
       fprintf( fLog, szLOGOUT_S3, ulStringsUnchanged ) ;
       fprintf( fLog, szLOGOUT_S4, ulStringsInvalid ) ;
       fprintf( fLog, szLOGOUT_S5, ulStringsTotal ) ;
       GetLocalTime( (LPSYSTEMTIME) &TimeStamp ) ;
       fprintf( fLog, szLOGOUT_S11, TimeStamp.wYear-2000, TimeStamp.wMonth, TimeStamp.wDay, 
                                    TimeStamp.wHour, TimeStamp.wMinute ) ;
       fputs( szLOGOUT_Separator, fLog ) ;

       fclose( fLog ) ;
    }
    if ( ! bMainDebug ) {
       remove( TempFile1 ) ;
       remove( TempFile2 ) ;
       remove( TempFile3 ) ;
    }
    if ( bTimeDebug ) {
       GetLocalTime( (LPSYSTEMTIME) &TimeStamp ) ;
       fprintf( fTimeDebug, "-----------       %02d/%02d/%02d %02d:%02d:%02d   DONE\n",TimeStamp.wYear-2000, TimeStamp.wMonth, TimeStamp.wDay,TimeStamp.wHour, TimeStamp.wMinute, TimeStamp.wSecond ) ;                  
       fclose(fTimeDebug);
    }

    if ( usReturn == RC_ERROR ) {      /* If error, delete output XML file */
       remove( pszProofReadXMLOut ) ;
       return( usReturn ) ;
    }

    return( 0 ) ;
}



/****************************************************************************/
/*                                                                          */
/* StripText                                                                */
/*                                                                          */
/* Get the contents of a single or double quoted string.                    */
/* Then determine if the string is a key or the value of a key/value pair.  */
/*                                                                          */
/* Input:      szIn          - Input string to strip inline tags from.      */
/*             usType        - Type of output text.                         */
/*                                1 = Source text.                          */
/*                                2 = Previous translated text.             */
/*                                3 = New translated text.                  */
/* Output:     szOut         - Output string with inline tags removed.      */
/*             bTextDeleted  - TRUE=Changed text was deleted (Type=3 only). */
/* Return:     RC_OK         - String value is returned.                    */
/*             RC_ERROR      - String is incorrectly formatted.             */
/*                                                                          */
/****************************************************************************/

USHORT  StripText( char *szIn, char *szOut, USHORT usType, BOOL *bSegDeleted ) 
{
   char     *ptrIn, *ptrOut ;
   BOOL     bInsert = FALSE ; 
   BOOL     bDelete = FALSE ; 
   BOOL     bTextDeleted = FALSE ; 
   USHORT   usReturn = RC_OK ;

   if ( bSegDeleted ) 
      *bSegDeleted = FALSE ;

   for ( ptrIn=szIn, ptrOut=szOut ;  *ptrIn ; ++ptrIn ) {

      /*--------------------------------------------------------------------*/
      /*  Remove internal tags from text.                                   */
      /*--------------------------------------------------------------------*/
       if ( *ptrIn == '<' ) {
          if ( ( ! strncmp( ptrIn, "<~+",  3 ) ) ||
               ( ! strncmp( ptrIn, "</~+", 4 ) ) ) {
             ptrIn = strchr( ptrIn, '>' ) ;
             continue ;
          }
          if ( ! strncmp( ptrIn, "<Br>", 4 ) ) {
             ptrIn += 3 ;
             continue ;
          }
          if ( ! strncmp( ptrIn, "<InS>", 5 ) ) {
             bInsert = TRUE ; 
             ptrIn += 4 ;
             continue ;
          }
          if ( ! strncmp( ptrIn, "</InS>", 6 ) ) {
             bInsert = FALSE ; 
             ptrIn += 5 ;
             continue ;
          }
          if ( ! strncmp( ptrIn, "<DeL>", 5 ) ) {
             bDelete = TRUE ; 
             bTextDeleted = TRUE ;
             ptrIn += 4 ;
             continue ;
          }
          if ( ! strncmp( ptrIn, "</DeL>", 6 ) ) {
             bDelete = FALSE ; 
             ptrIn += 5 ;
             continue ;
          }
       }

       /*--------------------------------------------------------------------*/
       /*  Save output character based on conditions.                        */
       /*--------------------------------------------------------------------*/
       if ( ( ( usType == 1 ) &&
              ( ! bDelete   ) ) ||
            ( ( usType == 2 ) &&
              ( ! bInsert   ) ) ||
            ( ( usType == 3 ) &&
              ( ! bDelete   ) ) ) {
          *ptrOut++ = *ptrIn ;
       }
   }

   /*--------------------------------------------------------------------*/
   /*  Handle entire segment's text was deleted.                         */
   /*--------------------------------------------------------------------*/
   if ( ( usType == 3      ) &&
        ( bTextDeleted     ) &&
        ( bSegDeleted      ) &&
        ( szOut[0] == NULL ) ) {
      *bSegDeleted = TRUE ;
   }

   *ptrOut = 0 ;

   return( usReturn ) ;
}




/****************************************************************************/
/*                                                                          */
/* WriteXmlSegment                                                          */
/*                                                                          */
/* Add a segment to the XML output file.                                    */
/*                                                                          */
/* Input:      fOut          - Output XML file.                             */
/*             ulSegNum      - Segment number.                              */
/*             ulSegNum      - >1. Include number of segments in text.      */
/*             szSource      - Source text.                                 */
/*             szTarget      - Target text,                                 */
/*             szChanged     - Changed text.                                */
/*             szComment     - Comment text.                                */
/*             szProblem     - !NULL. Include error message text.           */
/*             bChangeDeleted- TRUE=All changed text was deleted.            */
/* Output:     None.                                                        */
/* Return:     TRUE          - Segment written successfully.                */
/*             FALSE         - Segment could not be written.                */
/*                                                                          */
/****************************************************************************/

BOOL  WriteXmlSegment( FILE *fOut, ULONG ulSegNum, USHORT usSegCount, 
                       char *szSource, char *szTarget, char *szChanged, 
                       char *szComment, char *szProblem,
                       BOOL bChangeDeleted ) 
{
   if ( usSegCount > 1 ) {
      fprintf( fOut, szXML_Segment_2, ulSegNum, usSegCount ) ;
   } else {
      fprintf( fOut, szXML_Segment, ulSegNum ) ;
   }
   fprintf( fOut, szXML_Segment_Source, szSource ) ;
   fprintf( fOut, szXML_Segment_OrgTarget, szTarget ) ;
   if ( bChangeDeleted ) 
      fprintf( fOut, szXML_Segment_ModTarget_Del ) ;
   else
      fprintf( fOut, szXML_Segment_ModTarget, szChanged ) ;
   fprintf( fOut, szXML_Segment_NewTarget, "" ) ;
   fprintf( fOut, szXML_Segment_Comment, szComment ) ;
   if ( szProblem[0] ) 
      fprintf( fOut, szXML_Segment_Problem, szProblem ) ;
   fputs( szXML_Segment_End, fOut ) ;
   fflush( fOut ) ;

   return( TRUE ) ;
}


/****************************************************************************/
/*                                                                          */
/* ConvertFile                                                              */
/*                                                                          */
/* Convert file between UTF-8 and UTF-16.                                   */
/*                                                                          */
/****************************************************************************/

BOOL ConvertFile( char *szInFile, char *szOutFile, USHORT usType )
{
  FILE     *fIn, *fOut ;
  char     szUTF8[MAX_RCD_LENGTH*2] ;
  WCHAR    szUTF16[MAX_RCD_LENGTH*2] ;
  ULONG    ulIndex ;
  ULONG    ulOutput = 0;
  USHORT   i ;
  DWORD    dwFlags = 0;
  LONG     lRc = 0;
  BOOL     bReturn = TRUE ; 

  fIn = fopen( szInFile, "rb" ) ;
  fOut = fopen( szOutFile, "wb" ) ;
  if ( fIn && fOut ) {
     if ( usType == UTF82UTF16 ) {
        fputws( L"\xFEFF", fOut ) ;
        while ( fgets( szUTF8, MAX_RCD_LENGTH, fIn ) != NULL ) {
           if ( strlen(szUTF8) == MAX_RCD_LENGTH-1 ) { /* Handle long records    */
              ulIndex = MAX_RCD_LENGTH-2 ;           /* Read entire UTF-8 char */
              while( szUTF8[ulIndex] > '\x7F' ) {
                 szUTF8[++ulIndex] = (CHAR)fgetc(fIn) ;
                 if ( szUTF8[ulIndex] <= '\x7F' ) {
                    szUTF8[++ulIndex] = 0 ;
                    break ;
                 }
                 if ( szUTF8[ulIndex] >= '\xF0' )       /* 4 byte UTF-8 char */
                    i = 3 ;
                 else
                 if ( szUTF8[ulIndex] >= '\xE0' )       /* 3 byte UTF-8 char */
                    i = 2 ;
                 else
                 if ( szUTF8[ulIndex] >= '\xC0' )       /* 2 byte UTF-8 char */
                    i = 1 ;
                 else
                    i = 0 ;                      /* Not 1st byte of UTF-8 char     */
                 if ( i > 0 ) {                  /* Read rest of UTF-8 char        */
                    for( ; i ; --i ) {
                       szUTF8[++ulIndex] = (CHAR)fgetc(fIn) ;
                    }
                    szUTF8[++ulIndex] = 0 ;
                    break ;
                 }
              }
           }
           szUTF16[0] = NULL ;
           ulOutput = MultiByteToWideChar( CP_UTF8, dwFlags, szUTF8, -1, szUTF16, MAX_RCD_LENGTH*2 );
           if ( ! ulOutput ) {
              lRc = GetLastError();
              bReturn = FALSE ; 
              break;
           }
           fputws( szUTF16, fOut ) ;
        }
     } else {
        while ( fgetws( szUTF16, MAX_RCD_LENGTH, fIn ) != NULL ) {
           szUTF8[0] = 0 ;
           ulOutput = WideCharToMultiByte( CP_UTF8, 0, szUTF16, -1, szUTF8, MAX_RCD_LENGTH*2, NULL, NULL );
           if ( ! ulOutput ) {
              lRc = GetLastError();
              bReturn = FALSE ; 
              break;
           }
           fputs( szUTF8, fOut ) ;
        }
     }
  } else {
     bReturn = FALSE ;
  }

  if ( fIn ) 
     fclose( fIn ) ;
  if ( fOut ) 
     fclose( fOut ) ;
  if ( ! bReturn ) {
     remove( szOutFile ) ;
  }

  return( bReturn ) ;
}


/****************************************************************************/
/*                                                                          */
/* BuildMsg                                                                 */
/*                                                                          */
/* Show a terminating error message to the user.                            */
/*                                                                          */
/* Input:      szText        - Message box content text.                    */
/* Output:     None.                                                        */
/* Return:     None.                                                        */
/*                                                                          */
/****************************************************************************/

void  BuildMsg( char *szMessage, char* szText, int iType, char* szString, ULONG ulNum ) 
{
   char      szConcat[MAX_RCD_LENGTH] ;

   if ( iType == 1 ) {
      strcpy( szMessage, szText ) ;
      if ( fLog ) {
         fprintf( fLog, "\n% 5ld   ", ulNum ) ; 
         fputs( szText, fLog ) ;
      }
      strcat( szMessage, "\n\n" ) ;
   } else
   if ( iType == 2 ) {
      if ( szString != NULL ) {
         sprintf( szConcat, szText, szString ) ;
      } else
      if ( ulNum ) {
         sprintf( szConcat, szText, ulNum ) ;
      } else {
         strcpy( szConcat, szText ) ;
      }
      if ( fLog ) {
         fprintf( fLog, "\n        " ) ; 
         fputs( szConcat, fLog ) ;
      }
      strcat( szConcat, "\n\n" ) ;
      strcat( szMessage, szConcat ) ;
   } 

   return ;
}


/****************************************************************************/
/*                                                                          */
/* ShowError                                                                */
/*                                                                          */
/* Show a terminating error message to the user.                            */
/*                                                                          */
/* Input:      szText        - Message box content text.                    */
/* Output:     None.                                                        */
/* Return:     None.                                                        */
/*                                                                          */
/****************************************************************************/

void  ShowError( char *szText ) 
{
   AddLogError( szText ) ;
   strcat( szText, szERR_EndProcess ) ;
   MessageBoxA( NULL, szText, szERR_Title, MB_OK| MB_ICONERROR ) ;

   return ;
}


/****************************************************************************/
/*                                                                          */
/* ShowWarning                                                              */
/*                                                                          */
/* Show a warning message to the user.                                      */
/*                                                                          */
/* Input:      szText        - Message box content text.                    */
/* Output:     None.                                                        */
/* Return:     USHORT        - Action user selected                         */
/*                             MB_OK       Continue processing.             */
/*                             MB_CANCEL   Terminate processing.            */
/*                                                                          */
/****************************************************************************/

USHORT  ShowWarning( char *szText ) 
{
   USHORT      usRC ;

   if ( fLog ) {
      fputs( "\n\n", fLog ) ;
   }

   strcat( szText, szWARN_ContinueProcess ) ;
   usRC = (USHORT) MessageBoxA( NULL, szText, szWARN_Title, MB_OKCANCEL| MB_ICONWARNING | MB_DEFBUTTON1 ) ;

   return( usRC ) ;
}


/****************************************************************************/
/*                                                                          */
/* AddLogError                                                              */
/*                                                                          */
/* Record the error condition in the log file.                              */
/*                                                                          */
/* Input:      szText        - Txt to record.                               */
/* Output:     None.                                                        */
/* Return:     None.                                                        */
/*                                                                          */
/****************************************************************************/

void  AddLogError( char *szText ) 
{
   if ( fLog ) {
      fputs( "\n\n**ERROR**\n", fLog ) ;
      fputs( szText, fLog ) ;
      fputs( szERR_EndProcess, fLog ) ;
      fputs( "\n\n", fLog ) ;
   }
   return ;
}


/****************************************************************************/
/*                                                                          */
/* AddLogWarning                                                            */
/*                                                                          */
/* Record the warning condition in the log file.                            */
/*                                                                          */
/* Input:      szText        - Txt to record.                               */
/* Output:     None.                                                        */
/* Return:     None.                                                        */
/*                                                                          */
/****************************************************************************/

void  AddLogWarning( char *szText ) 
{
   if ( fLog ) {
      fputs( "\n\n**WARNING**\n", fLog ) ;
      fputs( szText, fLog ) ;
      fputs( szERR_EndProcess, fLog ) ;
      fputs( "\n\n", fLog ) ;
   }
   return ;
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
    char      szCommand[1024] ;
    char      szTempDir1[256] ;
    char      szTempFile1[256] ;
    char      szUnzipExe[256] ;
    char      *ptrChar ;
    BOOL      bRc ;
    BOOL      bReturn = TRUE;

    ErrText;

    /*-----------------------------------------------------------------------*/
    /*  Determine input file format, whether ZIP file or CONTENT.XML         */
    /*-----------------------------------------------------------------------*/
    //GetIBMDllPath( szProgPath, szUnzipExe ) ;
    //strcpy( szUnzipExe, "c:\\otm\\plugins\\otmproofreadimport\\filter\\" ) ;
    UtlMakeEQFPath( szUnzipExe, NULC, PLUGIN_PATH, NULL );
    strcat( szUnzipExe, "\\OtmProofReadImport\\filter\\" ); // TODO: use actual path of filter DLL
    strcat( szUnzipExe, "UNZIP.EXE" ) ;
    if ( ! szUnzipExe[0] ) {
       strcpy( szErrMsg, MSG_XMWRD_ZIP_XML_MISSING_UNZIP ) ;
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
       ptrChar = strrchr( szTempDir1, '\\' ) ;
       if ( ptrChar ) 
          *(ptrChar+1) = 0 ;
       strcat( szTempDir1, "OTMTEMP\\" ) ;
       _rmdir( szTempDir1 ) ;
       _mkdir( szTempDir1 ) ;
       strcpy( szTempFile1, szTempDir1 ) ;
       strcat( szTempFile1, "document.xml" ) ;
       remove( szTempFile1 ) ;

       if ( ptrChar ) {
  //      /*-----------------------------------------------------------------*/
  //      /*  Create temporary directories.                                  */
  //      /*-----------------------------------------------------------------*/
  //      strcpy( ptrChar, "\\MISC\\" ) ;
  //      _mkdir( szTempDir1 ) ;             /* Create \EQF\...\MISC\        */
  //      strcpy( szTempDir2, szTempDir1 ) ;
  //      ptrChar = strrchr( ZipFile, '\\' ) ;
  //      strcat( szTempDir2, ++ptrChar ) ;
  //      strcat( szTempDir2, "$\\" ) ;
  //      _mkdir( szTempDir2 ) ;             /* Create \EQF\...\MISC\...\    */

          /*-----------------------------------------------------------------*/
          /*  Unzip all XML files from the zip file.                         */
          /*-----------------------------------------------------------------*/
          sprintf( szCommand, "\"%s\" -q -j -o \"%s\" word/document.xml -d %s 2> %s", 
                              szUnzipExe, ZipFile, szTempDir1, TempFile ) ;
          strcpy( szErrMsg, MSG_XMWRD_ZIP_XML_UNZIP_FAILED ) ;
          if ( ExecuteCommand( szCommand, TempFile, szErrMsg ) ) {

             /*--------------------------------------------------------------*/
             /*  Concatenate all of the XML files together into 1 file.      */
             /*--------------------------------------------------------------*/
             remove( TempFile ) ;
             bRc = ConvertFile( szTempFile1, XmlFile, UTF82UTF16 ) ;
             if ( ! bRc ) {
                strcpy( szErrMsg, MSG_FILE_CONVERSION ) ;
                bReturn = FALSE ;
             }
          } else {
             bReturn = FALSE ;
          }
          if ( ! bReturn ) {
             remove( TempFile ) ;
          }
          remove( szTempFile1 ) ;
          _rmdir( szTempDir1 ) ;
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
    ULONG                  rc ;
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
/*  ConvertDocToDocx                                                         */
/*                                                                           */
/*  Function called by EQFPRESEG2                                            */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be processed.                            */
/*****************************************************************************/

BOOL ConvertDocToDocx( char *in, char *out, char *TempFile, char *ErrText )
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



    /**********************************************************************/
    /*  Convert MS Word DOC document to DOCX document.                    */
    /**********************************************************************/

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
              wdFormatDocument                 0	Microsoft Office Word format.
              wdFormatDOSText                  4	Microsoft DOS text format.
              wdFormatDOSTextLineBreaks        5	Microsoft DOS text with line breaks preserved.
              wdFormatEncodedText	             7	Encoded text format.
              wdFormatFilteredHTML	        10	Filtered HTML format.
              wdFormatHTML	                 8	Standard HTML format.
              wdFormatRTF	                     6	Rich text format (RTF).
              wdFormatTemplate	             1	Word template format.
              wdFormatText	                 2	Microsoft Windows text format.
              wdFormatTextLineBreaks	         3	Windows text format with line breaks preserved.
              wdFormatUnicodeText	             7	Unicode text format.
              wdFormatWebArchive	             9	Web archive format.
              wdFormatXML	                    11	Extensible Markup Language (XML) format.
              wdFormatDocument97	             0	Microsoft Word 97 document format.
              wdFormatDocumentDefault	        16	Word default document file format. For Microsoft Office Word 2007, this is the DOCX format.
              wdFormatPDF	                    17	PDF format.
              wdFormatTemplate97               1	Word 97 template format.
              wdFormatXMLDocument	            12	XML document format.
              wdFormatXMLDocumentMacroEnabled	13	XML document format with macros enabled.
              wdFormatXMLTemplate	            14	XML template format.
              wdFormatXMLTemplateMacroEnabled	15	XML template format with macros enabled.
              wdFormatXPS	                    18	XPS format.

              wdFormatDocument97	0 
              wdFormatDocumentDefault         16  Word default document file format. For Microsoft Office Word 2007, this is the DOCX format.
              wdFormatPDF                     17  PDF format.
              wdFormatTemplate97               1  Word 97 template format.
              wdFormatXMLDocument             12  XML document format.
              wdFormatXMLDocumentMacroEnabled 13  XML document format with macros enabled.
              wdFormatXMLTemplate             14  XML template format.
              wdFormatXMLTemplateMacroEnabled 15  XML template format with macros enabled.
              wdFormatXPS                     18
              */

       int num = 16;                            /* SaveAs XML format  wdFormatDocumentDefaault (DOCX) */ 
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
           strcat( ErrText, "\nSaveAs DOCX failed." ) ;
           strcat( ErrText, "\nMS Word document could not be saved as an DOCX document." ) ;
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
///    if ( FAILED(hr) ) {
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








/*! \brief Return information about this filter
  \param pszFilterName buffer for the filter name to be displayed to the user, NULL if no filter name is to be returned
  \param iFilterNameBufSize size of the pszFilterName buffer
  \param pszFileExtension buffer for the file type extension processed by this filter, NULL if no file extension info is to be returned
  \param iFileExtensionBufSize size of the pszFileExtension buffer
  \param pszVersion buffer for the filter version, NULL if no version info is to be returned
  \param iVersionBufSize size of the pszVersion buffer
  \returns 0 if successful or an error code
*/
int getFilterInfo( PSZ pszFilterName, int iFilterNameBufSize, PSZ pszFileExtension, int iFileExtensionBufSize, PSZ pszVersion, int iVersionBufSize )
{
  if ( pszFilterName != NULL )
  {
    strncpy( pszFilterName, "OpenTM2 Validation Document DOC/DOCX", iFilterNameBufSize - 1);
    pszFilterName[iFilterNameBufSize - 1] = 0;
  }
  if ( pszFileExtension != NULL )
  {
    strncpy( pszFileExtension, "*.DOCX;*.DOC", iFileExtensionBufSize - 1 );
    pszFileExtension[iFileExtensionBufSize - 1] = 0;
  }
  if ( pszVersion != NULL )
  {
    strncpy( pszVersion, "0.1", iVersionBufSize - 1 );
    pszVersion[iVersionBufSize - 1] = 0;
  }

  return( 0 );
}
