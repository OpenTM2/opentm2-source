/*! \file
	Filter code for the conversion of PWB Validation JSON file into the internal proof read import XML format
	
	Copyright Notice:

	Copyright (C) 1990-2017, International Business Machines
	Corporation and others. All rights reserved

 
    CHANGES:
        5/10/17   DAW   Inital version
*/

#include "EQF.H"
#include "EQFFOL.H"
#include "..\..\OtmProofReadFilter.h"

#include <sys/stat.h>


#define  MAX_RCD_LENGTH           4096


#define  PWB_STATE_NONE           1
#define  PWB_STATE_ID             2
#define  PWB_STATE_TEXT           3


#define  RC_OK                    1
#define  RC_ERROR                 2
#define  RC_WARNING               3


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
   char     *szERR_OpenOtmDoc        = "Original OpenTM2 document (%s) in folder (%s) cannot be opened.  It may be in use by another process." ;
   char     *szERR_MissingWtoMeta    = "OpenTM2 meta information is missing.  Make sure that a validation JSON file is being imported." ;
   char     *szERR_BadWtoMeta        = "OpenTM2 meta information is not in the correct format.  Make sure that a validation JSON file is being imported." ;
   char     *szERR_MismatchWTOMeta   = "Meta data for validation JSON file does not match the meta data in the OpenTM2 document." ;
   char     *szERR_SegOpenFail       = "OpenTM2 document cannot be opened:  %s" ;
   char     *szERR_UTF8Conversion    = "OpenTM2 text cannot be converted to UTF-8." ;
   char     *szERR_BadString         = "Syntax error in validation JSON file for quoted text.  Line=%ld" ;
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



   char     *szXML_Declare             = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n" ;
   char     *szXML_Root                = "<ProofReadingResults>\n" ;
   char     *szXML_Header              = " <Header>\n" ;
   char     *szXML_Header_Folder       = "  <Folder>%s</Folder>\n" ;
   char     *szXML_Header_Creation     = "  <CreationDate>%s</CreationDate>\n" ;
   char     *szXML_Header_Proofread    = "  <ProofReadDate></ProofReadDate>\n" ;
   char     *szXML_Header_Translator   = "  <Translator></Translator>\n" ;
   char     *szXML_Header_Proofreader  = "  <ProofReader></ProofReader>\n" ;
   char     *szXML_Header_LogFile      = "  <LogFile>%s</LogFile>\n" ;
   char     *szXML_Header_End          = " </Header>\n" ;
   char     *szXML_DocumentList        = " <DocumentList>\n" ;
   char     *szXML_Document            = "  <Document Name=\"%s\" SourceLang=\"%s\" TargetLang=\"%s\" Markup=\"%s\">\n" ;
   char     *szXML_SegmentList         = "   <SegmentList>\n" ;
   char     *szXML_Segment             = "    <Segment Number=\"%d\" Selected=\"no\" Processed=\"no\">\n" ;
   char     *szXML_Segment_2           = "    <Segment Number=\"%d\" Length=\"%d\" Selected=\"no\" Processed=\"no\">\n" ;
   char     *szXML_Segment_Source      = "     <Source>%s</Source>\n" ;
   char     *szXML_Segment_OrgTarget   = "     <OrgTarget>%s</OrgTarget>\n" ;
   char     *szXML_Segment_ModTarget   = "     <ModTarget>%s</ModTarget>\n" ;
   char     *szXML_Segment_NewTarget   = "     <NewTarget>%s</NewTarget>\n" ;
   char     *szXML_Segment_Comment     = "     <Comment>%s</Comment>\n" ;
   char     *szXML_Segment_Problem     = "     <Problem>%s</Problem>\n" ;
   char     *szXML_Segment_End         = "    </Segment>\n" ;
   char     *szXML_SegmentList_End     = "   </SegmentList>\n" ;
   char     *szXML_Document_End        = "  </Document>\n" ;
   char     *szXML_DocumentList_End    = " </DocumentList>\n" ;
   char     *szXML_Root_End            = "</ProofReadingResults>\n" ;

   char     *szLOGOUT_Separator        = "----------------------------------------------------------------------------------------------------\n" ;
   char     *szLOGOUT_SeparatorLine    = "Validation\n--Line----------------------------------------------------------------------------------------------\n" ;
   char     *szLOGOUT_H1               = "Extraction of Validation Changes from Validation PWB JSON file                 %02d/%02d/%02d %02d:%02d \n" ;
   char     *szLOGOUT_H2               = "    OpenTM2 Folder:      %s\n" ;
   char     *szLOGOUT_H3               = "    OpenTM2 Document:    %s\n" ;
   char     *szLOGOUT_H4               = "    Validation File:     %s\n" ;
   char     *szLOGOUT_H5               = "    Source Language:     %s\n" ;
   char     *szLOGOUT_H6               = "    Target Language:     %s\n" ;
            
   char     *szLOGOUT_S1               = "    Strings:\n" ;
   char     *szLOGOUT_S2               = "        Changed:       %d\n" ;
   char     *szLOGOUT_S3               = "        Unchanged:     %d\n" ;
   char     *szLOGOUT_S4               = "        Invalid:       %d\n" ;
   char     *szLOGOUT_S5               = "        Total:         %d\n" ;
   char     *szLOGOUT_S6               = "    Segments:\n" ;
   char     *szLOGOUT_S7               = "        Changed:       %d\n" ;
   char     *szLOGOUT_S8               = "        Unchanged:     %d\n" ;
   char     *szLOGOUT_S9               = "        Invalid:       %d\n" ;
   char     *szLOGOUT_S10              = "        Total:         %d\n" ;

   char     *szLOGOUT_ERROR            = "\n  *** NOTE:  Processing was terminated by the user.\n\n" ;

   char     cMATCH_CHAR = '\x1F' ;



   FILE     *fLog ;

   char     szObjectWtoMeta[MAX_RCD_LENGTH] ;
   char     szErrMsg[MAX_RCD_LENGTH] ;   
   char     szProblemMsg[256] ;

   ULONG    ulStringsTotal ;
   ULONG    ulStringsChanged ;
   ULONG    ulStringsInvalid ;  
   ULONG    ulStringsUnchanged ;
   ULONG    ulSegmentsTotal ;
   ULONG    ulSegmentsChanged ;
   ULONG    ulSegmentsInvalid ;
   ULONG    ulSegmentsUnchanged ;

   ULONG    ulValLineNumber ;
   ULONG    ulValIdLineNumber ;
   ULONG    ulValSrcLineNumber ;
   ULONG    ulValTgtLineNumber ;
   ULONG    ulValChgLineNumber ;

   USHORT   usWarnAction ;


   USHORT  GetString( char *, char **, char *, USHORT * );
   USHORT  GetWTOMeta( char *, char *, char *, char *, char *, char *, char *, char *, char *, USHORT ) ;
   USHORT  CheckOtmFolder( char *, char *, char *, char *, char *, char *, char *, char *, char *, char * ) ;
   USHORT  ProcessOtmData( char *, char *, char *, char *, FILE *, FILE *, FILE * ) ;
   USHORT  FindOtmContext( FILE *, char *, char *, ULONG *, char * );
   BOOL    GetOtmNextSegment( char *, char *, ULONG * );
   USHORT  GetOtmSegments( FILE *, char *, USHORT *, SEGDATA **, SEGDATA ** ) ;
   void    SetObjectText( char *, char * ) ;
   BOOL    WriteXmlSegment( FILE *, ULONG, USHORT, char *, char *, char *, char *, char * ) ;
   ULONG   UTF82Unicode( char *, WCHAR *, LONG, BOOL, LONG * ) ;
   ULONG   Unicode2UTF8( WCHAR *, char *, LONG, BOOL, PLONG ) ; 
   void    BuildMsg( char *, char *, int, char *, ULONG ) ;
   void    AddLogError( char * ) ;
   void    AddLogWarning( char * ) ;
   void    ShowError( char * ) ;
   USHORT  ShowWarning( char * ) ;



    BOOL    bDebug = FALSE ;










/*! \brief Convert a file returned from the proof reading process into the internal proof read XML document format
  \param pszProofReadInFile fully qualified input file selected by the user
  \param pszProofReadXMLOut name of the XML output file to be created
  \returns 0 if successful or an error code
*/
int convertToProofReadFormat( const char *pszProofReadInFile, const char *pszProofReadXMLOut )
{

  FILE     *fIn, *fOut ;
  FILE     *fSegSource, *fSegTarget ;

  SYSTEMTIME  TimeStamp ;

  char     *ptr1 ;

  char     szIn[MAX_RCD_LENGTH] ;
  char     szText[MAX_RCD_LENGTH] ;
  char     szUniqueId[MAX_RCD_LENGTH] ;

  char     szObjectId[MAX_RCD_LENGTH] ;
  char     szObjectSource[MAX_RCD_LENGTH] ;
  char     szObjectTarget[MAX_RCD_LENGTH] ;
  char     szObjectChanged[MAX_RCD_LENGTH] ;

  char     szDocumentLong[256] ; 
  char     szDocumentShort[256] ; 
  char     szFolderLong[256] ;
  char     szFolderShort[256] ; 
  char     szDocumentMarkup[80] ;
  char     szDocumentSrcLang[80] ; 
  char     szDocumentTgtLang[80] ; 
  char     szDocumentDate[80] ;
  char     szSegSourceDoc[256] ;
  char     szSegTargetDoc[256] ;
  char     szLogFile[256] ;

  char     szTemp[MAX_RCD_LENGTH] ;
  char     szObjectIdList[20][100] ;
  char     *ptrCur, *ptrEnd ;
  char     cQuoteChar ;
  char     *pProgPath= "c:\\otm\\logs\\" ;

  USHORT   usIdCount = 0 ;
  USHORT   usNestLevel ;
  USHORT   usState ;
  USHORT   usType ;

  ULONG    i ;

  BOOL     bFileWTOMetaNext = FALSE ;
  BOOL     bFileWTOMetaFound = FALSE ;
  USHORT   usReturn = RC_OK ;
  USHORT   usReturn2 = RC_OK ;


  /*------------------------------------------------------------------------*/
  /*  Initialize processing.                                                */
  /*------------------------------------------------------------------------*/
  szObjectWtoMeta[0] = 0 ;
  fSegSource = NULL ;
  fSegTarget = NULL ;
  fLog = NULL ;
  fOut = NULL ;

  ulStringsTotal = 0 ;
  ulStringsChanged = 0 ;
  ulStringsInvalid = 0 ;
  ulStringsUnchanged = 0 ;
  ulSegmentsTotal = 0 ;
  ulSegmentsChanged = 0 ;
  ulSegmentsInvalid = 0 ;
  ulSegmentsUnchanged = 0 ;
  ulValLineNumber = 0 ;

  remove( pszProofReadXMLOut );

  fIn = fopen( pszProofReadInFile, "rb" ) ;
  if ( ! fIn ) {
     usReturn = RC_ERROR ;
     sprintf( szErrMsg, szERR_MissingInput, pszProofReadInFile ) ;
     ShowError( szErrMsg ) ;
  }

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
//strcpy( szLogFile, pProgPath ) ;
//if ( ( isalpha( szLogFile[0] ) ) &&
//     ( szLogFile[1] == ':'     ) &&
//     ( szLogFile[2] == '\\'    ) ) {
//   ptr1 = strchr( &szLogFile[3], '\\' ) ;
//   if ( ptr1 ) { 
//      *(ptr1+1) = 0 ;
//   } else {
//      szLogFile[3] = 0 ;
//      strcat( szLogFile, "OTM\\" ) ;
//   }
//   strcat( szLogFile, "LOGS\\" ) ;
//} else {
//   strcpy( szLogFile, "c:\\OTM\\LOGS\\" ) ;
//}


  /*------------------------------------------------------------------------*/
  /*  Process each line.                                                    */
  /*------------------------------------------------------------------------*/
  usNestLevel = 0 ;
  usState = PWB_STATE_NONE ; 

  while ( ( usReturn != RC_ERROR ) &&
          ( fgets( szIn, MAX_RCD_LENGTH, fIn ) != NULL ) ) {
     ++ulValLineNumber ;

     /*------------------------------------------------------------------------*/
     /*  Look at each character of the line.                                   */
     /*------------------------------------------------------------------------*/
     for( ptrCur=szIn ; *ptrCur ; ++ptrCur ) {

        /*---------------------------------------------------------------------*/
        /*  Determine the state of this character.                             */
        /*---------------------------------------------------------------------*/
        if ( usState == PWB_STATE_NONE ) {
           if ( isspace( *ptrCur ) ) {
              continue ;
           } 
           if ( ! strncmp( ptrCur, "//", 2 ) ) {     /* Line comment           */
              ptrCur += strlen(ptrCur) - 1 ;
              continue ;
           }
           if ( *ptrCur == '{' ) {                   /* Start of nesting level */
              ++usNestLevel ;
           } else
           if ( *ptrCur == '}' ) {                   /* End of nesting level   */
              if ( usNestLevel ) 
                 --usNestLevel ;
           } else
           if ( *ptrCur == '[' ) {                   /* Start of array         */
           } else
           if ( *ptrCur == ']' ) {                   /* End of array           */ 
           } else
           if ( ( *ptrCur == '\'' ) ||               /* Start of quoted string */
                ( *ptrCur == '\"' ) ) {
              cQuoteChar = *ptrCur ;
              usReturn2 = GetString( ptrCur, &ptrEnd, szText, &usType ) ;
              if ( usReturn2 == RC_OK ) {
                 if ( usType == PWB_STATE_ID ) {
                    ++usIdCount ;
                    strcpy( szObjectId, szText ) ;
                    if ( ( strcmp( szObjectId, "source" ) ) &&
                         ( strcmp( szObjectId, "target" ) ) &&
                         ( strcmp( szObjectId, "changed" ) ) ) {
                       szObjectSource[0] = 0 ;
                       szObjectTarget[0] = 0 ;
                       szObjectChanged[0] = 0 ;
                       ulValIdLineNumber = ulValLineNumber ;
                    }
                    strcpy( szObjectIdList[usNestLevel], szObjectId ) ;

                    szUniqueId[0] = 0 ;
                    for( i=1 ; i<=usNestLevel ; ++i ) {     /* Build complete ID  */
                       if ( szUniqueId[0] )
                          strcat( szUniqueId, "." );
                       strcat( szUniqueId, szObjectIdList[i] ) ;
                    }

                    bFileWTOMetaNext = FALSE ;
                    if ( ! bFileWTOMetaFound ) {
                       if ( ( ( usIdCount == 1 ) &&
                              ( strcmp( szObjectId, "_id" ) ) ) ||
                            ( ( usIdCount == 2 ) &&
                              ( strcmp( szObjectId, "file-name" ) ) ) ||
                            ( ( usIdCount == 3 ) &&
                              ( strcmp( szObjectId, "WTO-meta" ) ) ) ) {
                          usReturn = usReturn2 = RC_ERROR ;
                          strcpy( szErrMsg, szERR_MissingWtoMeta ) ;
                          ShowError( szErrMsg ) ;
                          break ;
                       } else 
                       if ( ! strcmp( szObjectId, "WTO-meta" ) ) {
                          bFileWTOMetaNext = TRUE ;
                       }
                    }
                 } else
                 if ( usType == PWB_STATE_TEXT ) {

                    /*---------------------------------------------------------------------*/
                    /*  Handle <WTO-meta> tag to get folder and document information.      */
                    /*---------------------------------------------------------------------*/
                    if ( bFileWTOMetaNext ) {
                       bFileWTOMetaFound = TRUE ;
                       strcpy( szObjectWtoMeta, szText ) ;
                       usReturn2 = GetWTOMeta( szText, szDocumentLong, szDocumentShort, szFolderLong, szFolderShort, szDocumentMarkup,
                                               szDocumentSrcLang, szDocumentTgtLang, szDocumentDate, 1 ) ;
                       if ( usReturn2 == RC_OK ) {
                          usReturn2 = CheckOtmFolder( szDocumentLong, szDocumentShort, szFolderLong, szFolderShort, szDocumentMarkup,
                                                      szDocumentSrcLang, szDocumentTgtLang, pProgPath, szSegSourceDoc, szSegTargetDoc ) ;
                          if ( usReturn2 == RC_OK ) {
                             fputs( szXML_Declare, fOut ) ;
                             fputs( szXML_Root, fOut ) ;
                             fputs( szXML_Header, fOut ) ;
                             fprintf( fOut, szXML_Header_Folder, szFolderLong ) ;
                             strcpy( szTemp, szDocumentDate ) ;
                             szTemp[8] = 0 ;
                             fprintf( fOut, szXML_Header_Creation, szTemp ) ;
                             fputs( szXML_Header_Proofread, fOut ) ;
                             fputs( szXML_Header_Translator, fOut ) ;
                             fputs( szXML_Header_Proofreader, fOut ) ;
                             strcat(szLogFile, "\\v_" ) ;
                             ptr1 = strrchr( szDocumentLong, '\\' ) ;
                             if ( ptr1 )
                                strcat( szLogFile, ptr1+1 ) ;
                             else 
                                strcat(szLogFile, szDocumentLong ) ;
                             strcat(szLogFile, ".LOG" ) ;
                             fprintf( fOut, szXML_Header_LogFile, szLogFile ) ;
                             fputs( szXML_Header_End, fOut ) ;
                             fputs( szXML_DocumentList, fOut ) ;
                             fprintf( fOut, szXML_Document,szDocumentLong, szDocumentSrcLang, szDocumentTgtLang, szDocumentMarkup ) ;
                             fputs( szXML_SegmentList, fOut ) ;
                             fflush( fOut ) ;

                             fSegSource = fopen( szSegSourceDoc, "rb" ) ;
                             if ( ! fSegSource ) {
                                usReturn = usReturn2 = RC_ERROR ;
                                sprintf( szErrMsg, szERR_SegOpenFail, szSegSourceDoc ) ;
                                ShowError( szErrMsg ) ;
                                break ;
                             } else {
                                fSegTarget = fopen( szSegTargetDoc, "rb" ) ;
                                if ( ! fSegTarget ) {
                                   usReturn = usReturn2 = RC_ERROR ;
                                   sprintf( szErrMsg, szERR_SegOpenFail, szSegTargetDoc ) ;
                                   ShowError( szErrMsg ) ;
                                   break ;
                                }
                             }
                             if ( usReturn2 == RC_OK ) {
                                fLog = fopen( szLogFile, "wb" ) ;
                                GetLocalTime( (LPSYSTEMTIME) &TimeStamp ) ;
                                fprintf( fLog, szLOGOUT_H1,  TimeStamp.wYear-2000, TimeStamp.wMonth, TimeStamp.wDay, 
                                                             TimeStamp.wHour, TimeStamp.wMinute ) ;
                                fputs( szLOGOUT_Separator, fLog ) ;
                                fprintf( fLog, szLOGOUT_H2, szFolderLong ) ;
                                fprintf( fLog, szLOGOUT_H3, szDocumentLong ) ;
                                fprintf( fLog, szLOGOUT_H4, pszProofReadInFile ) ;
                                fprintf( fLog, szLOGOUT_H5, szDocumentSrcLang ) ;
                                fprintf( fLog, szLOGOUT_H6, szDocumentTgtLang ) ;
                                fputs( szLOGOUT_SeparatorLine, fLog ) ;
                             }
                          } else {
                             //  Errors reported in subroutine.
                             usReturn = usReturn2 = RC_ERROR ;
                             break ;
                          }
                       } else {
                          usReturn = usReturn2 = RC_ERROR ;
                          strcpy( szErrMsg, szERR_BadWtoMeta ) ;
                          ShowError( szErrMsg ) ;
                          break ;
                       }
                    } else

                    /*---------------------------------------------------------------------*/
                    /*  Handle <source> validation text.                                   */
                    /*---------------------------------------------------------------------*/
                    if ( ! strcmp( szObjectId, "source" ) ) {
                       ulValSrcLineNumber = ulValLineNumber ;
                       if ( ! bFileWTOMetaFound ) {
                          usReturn = usReturn2 = RC_ERROR ;
                          strcpy( szErrMsg, szERR_MissingWtoMeta ) ;
                          ShowError( szErrMsg ) ;
                          break ;
                       }
                       SetObjectText( szObjectSource, szText ) ;
                       ++ulStringsTotal ;
                       ++ulStringsUnchanged ;
                    } else

                    /*---------------------------------------------------------------------*/
                    /*  Handle <target> validation text.                                   */
                    /*---------------------------------------------------------------------*/
                    if ( ! strcmp( szObjectId, "target" ) ) {
                       ulValTgtLineNumber = ulValLineNumber ;
                       SetObjectText( szObjectTarget, szText ) ;
                    } else

                    /*---------------------------------------------------------------------*/
                    /*  Handle <changed> validation text.                                  */
                    /*---------------------------------------------------------------------*/
                    if ( ! strcmp( szObjectId, "changed" ) ) {
                       ulValChgLineNumber = ulValLineNumber ;
                       SetObjectText( szObjectChanged, szText ) ;

//                     if ( ( szObjectSource[0]  ) &&     /* If different changed text */
//                          ( szObjectTarget[0]  ) &&
// Write only changed       ( szObjectChanged[0] ) &&
//  segments.               ( strcmp( szObjectChanged, szObjectTarget ) ) ) {
//                        ++ulStringsChanged ;
//                        --ulStringsUnchanged ;
//                        ptr1 = strrchr( szUniqueId, '.' ) ;
//                        if ( ptr1 ) 
//                           *ptr1 = 0 ;
//                        usReturn2 = ProcessOtmData( szUniqueId, szObjectSource, szObjectTarget, szObjectChanged,
//                                                  fSegSource, fSegTarget, fOut ) ;
//                        if ( usReturn2 != RC_OK ) {
//                           if ( usReturn2 == RC_WARNING ) 
//                              usReturn = RC_WARNING ; 
//                           else
//                              usReturn = RC_ERROR ; 
//                        }
//                     } else {                              /* Found change text */
//                        if ( ( ! szObjectSource[0]  ) ||   /*   but no source   */
//                             ( ! szObjectTarget[0]  ) ) {  /*   or  no target   */
//                           if ( ! szObjectSource[0]  ) 
//                              strcpy( szProblemMsg, szWARN_MissingSrcTag ) ;
//                           else
//                              strcpy( szProblemMsg, szWARN_MissingTgtTag ) ;
//                           BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValIdLineNumber ) ;
//                           BuildMsg( szErrMsg, szMSG_ADD_Context, 2, szUniqueId, NULL ) ;
//                           if ( ShowWarning( szErrMsg ) == IDOK ) 
//                              usReturn = usReturn2 = RC_WARNING ;
//                           else
//                              usReturn = usReturn2 = RC_ERROR ;
//                           WriteXmlSegment( fOut,      /* Error */
//                                            0, 0,
//                                            szObjectSource,
//                                            szObjectTarget,
//                                            szObjectChanged, 
//                                            szUniqueId, szProblemMsg ) ;
//                           break ;
//                        }
//                     }

                       /* Write all segments */
                       if ( ( ! szObjectSource[0]  ) ||   /* No source text.   */
                            ( ! szObjectTarget[0]  ) ) {  /* No target text.   */
                          if ( ! szObjectSource[0]  ) 
                             strcpy( szProblemMsg, szWARN_MissingSrcTag ) ;
                          else
                             strcpy( szProblemMsg, szWARN_MissingTgtTag ) ;
                          BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValIdLineNumber ) ;
                          BuildMsg( szErrMsg, szMSG_ADD_Context, 2, szUniqueId, NULL ) ;
                          if ( ShowWarning( szErrMsg ) == IDOK ) 
                             usReturn = usReturn2 = RC_WARNING ;
                          else
                             usReturn = usReturn2 = RC_ERROR ;
                          WriteXmlSegment( fOut,      /* Error */
                                           0, 0,
                                           szObjectSource,
                                           szObjectTarget,
                                           szObjectChanged, 
                                           szUniqueId, szProblemMsg ) ;
                          break ;
                       } else {
                          if ( ( szObjectChanged[0] ) &&
                               ( strcmp( szObjectChanged, szObjectTarget ) ) ) {
                             ++ulStringsChanged ;
                             --ulStringsUnchanged ;
                          } else {
                             szObjectChanged[0] = 0 ;
                          }
                          ptr1 = strrchr( szUniqueId, '.' ) ;
                          if ( ptr1 ) 
                             *ptr1 = 0 ;
                          usReturn2 = ProcessOtmData( szUniqueId, szObjectSource, szObjectTarget, szObjectChanged,
                                                    fSegSource, fSegTarget, fOut ) ;
                          if ( usReturn2 != RC_OK ) {
                             if ( usReturn2 == RC_WARNING ) 
                                usReturn = RC_WARNING ; 
                             else
                                usReturn = RC_ERROR ; 
                          }
                       }
                    } 
                 } 
                 ptrCur = ptrEnd ;
              } else {    /* Quoted string is incorrectly formatted */
                 usReturn = usReturn2 = RC_ERROR ;
                 sprintf( szErrMsg, szERR_BadString, ulValLineNumber ) ;
                 ShowError( szErrMsg ) ;
                 break ;
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

  if ( fIn ) 
     fclose( fIn ) ;
  if ( fOut ) 
     fclose( fOut ) ;
  if ( fSegSource ) 
     fclose( fSegSource ) ;
  if ( fSegTarget ) 
     fclose( fSegTarget ) ;
  if ( fLog ) {
     if ( usReturn == RC_ERROR ) 
        fputs( szLOGOUT_ERROR, fLog ) ;
     fputs( szLOGOUT_Separator, fLog ) ;
     fputs( szLOGOUT_S1, fLog ) ;
     fprintf( fLog, szLOGOUT_S2, ulStringsChanged ) ;
     fprintf( fLog, szLOGOUT_S3, ulStringsUnchanged ) ;
     fprintf( fLog, szLOGOUT_S4, ulStringsInvalid ) ;
     fprintf( fLog, szLOGOUT_S5, ulStringsTotal ) ;
     fputs( szLOGOUT_S6, fLog ) ;
     fprintf( fLog, szLOGOUT_S7, ulSegmentsChanged ) ;
     fprintf( fLog, szLOGOUT_S8, ulSegmentsUnchanged ) ;
     fprintf( fLog, szLOGOUT_S9, ulSegmentsInvalid ) ;
     fprintf( fLog, szLOGOUT_S10, ulSegmentsChanged+ulSegmentsUnchanged+ulSegmentsInvalid ) ;
     fputs( szLOGOUT_Separator, fLog ) ;

     fclose( fLog ) ;
  }

  if ( usReturn == RC_ERROR ) {      /* If error, delete output XML file */
     if ( ! bDebug ) 
        remove( pszProofReadXMLOut ) ;
     return( usReturn ) ;
  }

  return( 0 ) ;
}



/****************************************************************************/
/*                                                                          */
/* GetString                                                                */
/*                                                                          */
/* Get the contents of a single or double quoted string.                    */
/* Then determine if the string is a key or the value of a key/value pair.  */
/*                                                                          */
/* Input:      ptrStart      - Ptr to beginning quote character of string   */
/* Output:     ptrEnd        - Ptr to ending quote character of the string  */
/*             szString      - String vale without the quotes.              */
/*             usType        - Whether the string is a key or value.        */
/* Return:     RC_OK         - String value is returned.                    */
/*             RC_ERROR      - String is incorrectly formatted.             */
/*                                                                          */
/****************************************************************************/

USHORT  GetString( char *ptrStart, char **ptrEnd, char * szString, USHORT *usType ) 
{
   char     cQuote ;
   char     *ptr1 ;
   USHORT   usReturn = RC_ERROR ;

   *usType = PWB_STATE_NONE ;
   
   cQuote = *ptrStart ; 
   for( ptr1=ptrStart+1 ; *ptr1 ; ++ptr1 ) {
      if ( *ptr1 == cQuote ) {                 /* Text's ending quote    */
         break ;
      } else
      if ( *ptr1 == '\\' ) {                   /* Escaped character      */
         ++ptr1 ;
      } 
   }
   *ptrEnd = ptr1 ;

   if ( *ptr1 ) {
      usReturn = RC_OK ;
      strcpy( szString, ptrStart+1 ) ;
      szString[ptr1-ptrStart-1] = 0 ;
      for( ptr1=ptr1+1 ; *ptr1 && isspace(*ptr1) ; ++ptr1 ) ;
      if ( *ptr1 == ':' ) 
         *usType = PWB_STATE_ID ; 
      else
         *usType = PWB_STATE_TEXT ; 
   }

   return( usReturn );
}


/****************************************************************************/
/*                                                                          */
/* GetWTOMeta                                                               */
/*                                                                          */
/* Special key/value pairs contain meta information used in this validation */
/* process.  The key is always "WTO-meta".  This routine will parse that    */
/* string into its individual parts.                                        */
/*                                                                          */
/* Input:      szString      - String containing the meta information.      */
/*             usType        - 1 = File level meta data.                    */
/*                             2 = String level meta data.                  */
/* Output:     szDocumentLong   - OTM document long name.                   */
/*             szDocumentShort  - OTM document short name.                  */
/*             szFolderLong     - OTM folder long name.                     */
/*             szFolderShort    - OTM folder short name.                    */
/*             szMarkup         - OTM markup table used for this file.      */
/*             szSourceLang     - OTM source language.                      */
/*             szTargetLang     - OTM target language.                      */
/*             szDate           - Date/time original JSON was analyzed.     */
/* Return:     RC_OK         - Meta data was correctly formatted            */
/*             RC_ERROR      - Meta data was in wrong format.               */
/*                                                                          */
/****************************************************************************/

USHORT  GetWTOMeta( char *szString, char *szDocumentLong, char *szDocumentShort, 
                    char *szFolderLong, char *szFolderShort, char *szMarkup, 
                    char *szSourceLang, char *szTargetLang, char *szDate, USHORT usType ) 
{
   char     *ptr1, *ptr2 ;
   USHORT   usReturn = RC_ERROR ;

   if ( usType == 1  ) {
      for( ptr1=szString ; *ptr1 ; ++ptr1 ) {
         if ( ! strncmp( ptr1, "doc=<", 5 ) ) {
            ptr1 += 5 ;
            ptr2 = strchr( ptr1, '>' ) ;
            if ( ptr2 ) {
               strncpy( szDocumentLong, ptr1, 256 ) ;
               szDocumentLong[ptr2-ptr1] = 0 ;
               ptr1 = ptr2 + 1 ;
               if ( *ptr1 == '<' ) {
                  ptr2 = strchr( ++ptr1, '>' ) ;
                  if ( ptr2 ) {
                     strncpy( szDocumentShort, ptr1, 256 ) ;
                     szDocumentShort[ptr2-ptr1] = 0 ;
                     ptr1 = ptr2 ;
                  }
               }
            }
         } else
         if ( ! strncmp( ptr1, "fld=<", 5 ) ) {
            ptr1 += 5 ;
            ptr2 = strchr( ptr1, '>' ) ;
            if ( ptr2 ) {
               strncpy( szFolderLong, ptr1, 256 ) ;
               szFolderLong[ptr2-ptr1] = 0 ;
               ptr1 = ptr2 + 1 ;
               if ( *ptr1 == '<' ) {
                  ptr2 = strchr( ++ptr1, '>' ) ;
                  if ( ptr2 ) {
                     strncpy( szFolderShort, ptr1, 256 ) ;
                     szFolderShort[ptr2-ptr1] = 0 ;
                     ptr1 = ptr2 ;
                  }
               }
            }
         } else
         if ( ! strncmp( ptr1, "mk=<", 4 ) ) {
            ptr1 += 4 ;
            ptr2 = strchr( ptr1, '>' ) ;
            if ( ptr2 ) {
               strncpy( szMarkup, ptr1, 80 ) ;
               szMarkup[ptr2-ptr1] = 0 ;
               ptr1 = ptr2 ;
            }
         } else
         if ( ! strncmp( ptr1, "sl=<", 4 ) ) {
            ptr1 += 4 ;
            ptr2 = strchr( ptr1, '>' ) ;
            if ( ptr2 ) {
               strncpy( szSourceLang, ptr1, 80 ) ;
               szSourceLang[ptr2-ptr1] = 0 ;
               ptr1 = ptr2 ;
            }
         } else
         if ( ! strncmp( ptr1, "tl=<", 4 ) ) {
            ptr1 += 4 ;
            ptr2 = strchr( ptr1, '>' ) ;
            if ( ptr2 ) {
               strncpy( szTargetLang, ptr1, 80 ) ;
               szTargetLang[ptr2-ptr1] = 0 ;
               ptr1 = ptr2 ;
            }
         } else
         if ( ! strncmp( ptr1, "dt=<", 4 ) ) {
            ptr1 += 4 ;
            ptr2 = strchr( ptr1, '>' ) ;
            if ( ptr2 ) {
               strncpy( szDate, ptr1, 80 ) ;
               szDate[ptr2-ptr1] = 0 ;
               ptr1 = ptr2 ;
            }
         } else {
            // Unknown meta information
         }
      }
      if ( ( szDocumentLong[0]  ) &&        /* Verify all WTO fields are set */
           ( szDocumentShort[0] ) &&
           ( szFolderLong[0]    ) &&
           ( szFolderShort[0]   ) &&
           ( szMarkup[0]        ) &&
           ( szSourceLang[0]    ) &&
           ( szTargetLang[0]    ) &&
           ( szDate[0]          ) ) {
         usReturn = RC_OK ;
      }
   }

   return( usReturn );

}


/****************************************************************************/
/*                                                                          */
/* CheckOtmFolder                                                           */
/*                                                                          */
/* Using the WTO meta data, locate the corresponding OpenTM2 folder and     */
/* document which contains the original translation for this validation     */
/* file.  If the document does exist, verify that the file's WTO meta data  */
/* is exactly the same.  If it is different, then the validation file and   */
/* the OTM document do not match.                                           */
/*                                                                          */
/* Input:      szDocumentLong   - OTM document long name.                   */
/*             szDocumentShort  - OTM document short name.                  */
/*             szFolderLong     - OTM folder long name.                     */
/*             szFolderShort    - OTM folder short name.                    */
/*             szMarkup         - OTM markup table used for this file.      */
/*             szSourceLang     - OTM source language.                      */
/*             szTargetLang     - OTM target language.                      */
/*             szPath           - Path to OTM directory.                    */
/* Output:     szSegSourceDoc   - Full path to OTM segmented source file.   */
/*             szSegTargetDoc   - Full path to OTM segmented target file.   */
/* Return:     RC_OK         - The OTM document exists and is correct.      */
/*             RC_ERROR      - The OTM document does not exist or the files */
/*                             do not match.                                */
/*                                                                          */
/****************************************************************************/

USHORT  CheckOtmFolder( char *szDocumentLong, char *szDocumentShort, 
                        char *szFolderLong, char *szFolderShort, char *szMarkup, 
                        char *szSourceLang, char *szTargetLang, char *szPath,
                        char *szSegSourceDoc, char *szSegTargetDoc ) 
{
   struct stat  FileInfo ;
   FILE     *fSource ;

   WCHAR    szIn[MAX_RCD_LENGTH] ;
   WCHAR    szWTO[MAX_RCD_LENGTH] ;
   WCHAR    *ptrW1 ;

   char     szOtmPath[256] ;
   char     szFolderPath[256] ;
   char     szTemp[MAX_RCD_LENGTH] ;
   char     szTemp2[MAX_RCD_LENGTH] ;

   ULONG    ulLineNum = 0 ;
   LONG     lRc ; 
   USHORT   usReturn = RC_ERROR ;

   szSourceLang; szTargetLang; szMarkup; szPath, 


   SubFolNameToObjectName( szFolderLong, szFolderPath ) ;
   sprintf( szSegSourceDoc, "%s\\SSOURCE\\%s", szFolderPath, szDocumentShort ) ;
   sprintf( szSegTargetDoc, "%s\\STARGET\\%s", szFolderPath, szDocumentShort ) ;

    if ( szFolderPath[0] ) {
      if ( ( ! stat( szSegSourceDoc, &FileInfo ) ) &&
           ( ! stat( szSegTargetDoc, &FileInfo ) ) ) {
         usReturn = RC_OK ;
         mbstowcs( szWTO, szObjectWtoMeta, sizeof(szObjectWtoMeta) ) ;
         fSource = fopen( szSegSourceDoc, "rb" ) ;
         if ( fSource ) {
            while ( fgetws( szIn, MAX_RCD_LENGTH, fSource ) != NULL ) {
               ++ulLineNum ;
               ptrW1 = wcschr( szIn, L'\"' ) ;
               if ( ( ptrW1 ) &&
                    ( ! wcsncmp( ptrW1+1, L"WTO-meta\"", 9 ) ) ) {
                  ptrW1 = wcschr( ptrW1+10, L'\"' ) ;
                  if ( ( ! ptrW1 ) ||
                       ( wcsncmp( ptrW1+1, szWTO, wcslen(szWTO) ) ) ) {
                     usReturn = RC_ERROR ;
                     strcpy( szProblemMsg, szERR_MismatchWTOMeta ) ;
                     Unicode2UTF8( szWTO, szTemp, MAX_RCD_LENGTH, TRUE, &lRc ) ;
                     Unicode2UTF8( ptrW1+1, szTemp2, MAX_RCD_LENGTH, TRUE, &lRc ) ;
                     BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulLineNum ) ;
                     BuildMsg( szErrMsg, szMSG_ADD_ValSrc,  2, szTemp, NULL ) ;
                     BuildMsg( szErrMsg, szMSG_ADD_SegSrc,  2, szTemp2, NULL ) ;
                     ShowError( szErrMsg ) ;
                  }
                  break ;
               }
            }
            fclose( fSource ) ;
         } else {
            usReturn = RC_ERROR ;
            sprintf( szErrMsg, szERR_OpenOtmDoc, szDocumentLong, szFolderLong ) ;
            ShowError( szErrMsg ) ;
         }
      } else {
         usReturn = RC_ERROR ;
         sprintf( szTemp, "%s%s.F00", szOtmPath, szFolderShort ) ;
         if ( stat( szTemp, &FileInfo ) )  
            sprintf( szErrMsg, szERR_MissingOtmFolder, szFolderLong ) ;
         else
            sprintf( szErrMsg, szERR_MissingOtmDoc, szDocumentLong, szFolderLong ) ;
         ShowError( szErrMsg ) ;
      }
   } else {
      usReturn = RC_ERROR ;
      sprintf( szErrMsg, szERR_MissingOtmFolder, szFolderLong ) ;
      ShowError( szErrMsg ) ;
   }

   return( usReturn );
}


/****************************************************************************/
/*                                                                          */
/* ProcessOtmData                                                           */
/*                                                                          */
/* Find the validation string in the OpenTM2 document.  If found, then      */
/* determine which segments have changed and generate the XML output for    */
/* those changed segments.                                                  */
/*                                                                          */
/* Input:      szUniqueId       - Context ID used to find string in file.   */
/*             szSrcString      - Validation "source" string.               */
/*             szTgtString      - Validation "target" string.               */
/*             szChgString      - Validation "changed" string.              */
/*             fSegSource       - OTM segmented source file                 */
/*             fSegTarget       - OTM segmented target file                 */
/* Output:     None.                                                        */
/* Return:     RC_OK         - String successfully processed.               */
/*             RC_ERROR      - Errors while processing string.              */
/*                                                                          */
/****************************************************************************/

USHORT  ProcessOtmData( char *szUniqueId, char *szSrcString, char *szTgtString,
                        char *szChgString, FILE *fSegSource, FILE *fSegTarget,
                        FILE *fOut ) 
{
   SEGDATA  *sdSrcHead, *sdSrcTail ;
   SEGDATA  *sdSrcCur, *sdSrcTemp ;
   SEGDATA  *sdTgtHead, *sdTgtTail ;
   SEGDATA  *sdTgtCur, *sdTgtTemp, *sdTgtNext ;

   char     szSrcText[MAX_RCD_LENGTH] ;
   char     szTgtText[MAX_RCD_LENGTH] ;
   char     szEndChars[10] ;
   char     szTemp[MAX_RCD_LENGTH] ;
   char     szTemp2[MAX_RCD_LENGTH] ;
   char     *ptrSrcString ;
   char     *ptrTgtString ;
   char     *ptrChgString ;
   char     *ptr1;

   ULONG    ulSegNum ;

   int      i, j, s ;
   int      iSegCount ;
   BOOL     bStringInvalid = FALSE ;
   USHORT   usSrcSegCount ;
   USHORT   usTgtSegCount ;
   USHORT   usReturn = RC_ERROR ;
   USHORT   usRet1 = RC_ERROR ;
   USHORT   usRet2 = RC_ERROR ;

   szSrcText[0] = 0 ;
   szTgtText[0] = 0 ;

   sdSrcHead = NULL ;
   sdSrcTail = NULL ;
   sdSrcCur  = NULL ;
   sdTgtHead = NULL ;
   sdTgtTail = NULL ;
   sdTgtCur  = NULL ;

   if ( bDebug ) {
      fprintf( fLog, "Context:   [%s]\n",szUniqueId ) ;
      fputs("  Source : [", fLog ) ;
      fputs( szSrcString, fLog ) ;
      fputs( "]\n", fLog ) ;
      fputs("  Target : [", fLog ) ;
      fputs( szTgtString, fLog ) ;
      fputs( "]\n", fLog ) ;
      fputs("  Changed: [", fLog ) ;
      fputs( szChgString, fLog ) ;
      fputs( "]\n", fLog ) ;
   }
   if ( szChgString[0] == 0 ) 
      strcpy( szChgString, szTgtString ) ;

   usRet1 = FindOtmContext( fSegSource, szUniqueId, szSrcString, &ulSegNum, szSrcText ) ;
   if ( usRet1 == RC_OK ) 
      usRet2 = FindOtmContext( fSegTarget, szUniqueId, NULL, &ulSegNum, szTgtText ) ;
   if ( ( usRet1 == RC_OK ) &&                 /* Found source text in OTM */
        ( usRet2 == RC_OK ) ) {                /* Found target text in OTM */
      usRet1 = GetOtmSegments( fSegSource, szSrcText, &usSrcSegCount, &sdSrcHead, &sdSrcTail ) ;
      if ( usRet1 == RC_OK ) 
         usRet2 = GetOtmSegments( fSegTarget, szTgtText, &usTgtSegCount, &sdTgtHead, &sdTgtTail ) ;
      if ( ( usRet1 == RC_OK ) &&              /* Found all source segments */
           ( usRet2 == RC_OK ) ) {             /* Found all target segments */

         /*---------------------------------------------------------------------*/
         /*  Have all source/target segments for this context ID.               */
         /*  See which segments have changed translated text.                   */
         /*---------------------------------------------------------------------*/

         /*---------------------------------------------------------------------*/
         /*  Handle source text which has only 1 source segment.                */
         /*---------------------------------------------------------------------*/
         if ( sdSrcHead->ptrNext == NULL ) {
            if ( ! strcmp( szSrcString, sdSrcHead->szText ) ) {
               if ( sdTgtHead->ptrNext == NULL ) {
                  if ( ! strcmp( szTgtString, sdTgtHead->szText ) ) {
                     usReturn = RC_OK ;
                     if ( ! strcmp( szTgtString, szChgString ) ) { /* Unchanged */
                        ++ulSegmentsUnchanged ;
                        szChgString[0] = 0 ;
                        if ( bDebug ) {
                           fprintf( fLog, "CHG match:      [" ) ;
                           fputs( szTgtString, fLog ) ;
                           fputs( "]\n", fLog ) ;
                        }
                     } else {
                        ++ulSegmentsChanged ;
                        if ( bDebug ) {
                           fprintf( fLog, "CHG NO match:      [" ) ;
                           fputs( szTgtString, fLog ) ;
                           fputs( "]\n", fLog ) ;
                        }
                     }
                     WriteXmlSegment( fOut,      /* OK */
                                      sdSrcHead->ulSegNum, usSrcSegCount,   
                                      szSrcString,
                                      szTgtString,
                                      szChgString, 
                                      szUniqueId, "" ) ;
                  } else { 
                     /* ERROR: Target segment text does not match <target> string. */
                     ++ulSegmentsInvalid ;
                     bStringInvalid = TRUE ;
                     strcpy( szProblemMsg, szWARN_MismatchTgt ) ;
                     BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValTgtLineNumber ) ;
                     BuildMsg( szErrMsg, szMSG_ADD_SegNum,  2, NULL, sdTgtHead->ulSegNum ) ;
                     BuildMsg( szErrMsg, szMSG_ADD_ValTgt,  2, szTgtString, NULL ) ;
                     BuildMsg( szErrMsg, szMSG_ADD_SegTgt,  2, sdTgtHead->szText, NULL ) ;
                     if ( ShowWarning( szErrMsg ) == IDOK ) 
                        usReturn = RC_WARNING ;
                     else
                        usReturn = RC_ERROR ;
                     WriteXmlSegment( fOut,      /* Error */
                                      sdSrcHead->ulSegNum, usSrcSegCount,   
                                      szSrcString,
                                      szTgtString,
                                      szChgString,
                                      szUniqueId, szProblemMsg ) ;
                  }
               } else {   
                  /* ERROR: More target segments than there are source segments. */
                  bStringInvalid = TRUE ;
                  ++ulSegmentsInvalid ;
                  usReturn = RC_ERROR ;
                  sprintf( szErrMsg, szERR_Unknown, 10 ) ;
                  ShowError( szErrMsg ) ;
               }
            } else {    
               /* ERROR: Source segment text does not match <source> string. */
               ++ulSegmentsInvalid ;
               bStringInvalid = TRUE ;
               strcpy( szProblemMsg, szWARN_MismatchSrc ) ;
               BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValSrcLineNumber ) ;
               BuildMsg( szErrMsg, szMSG_ADD_SegNum,  2, NULL, sdSrcHead->ulSegNum ) ;
               BuildMsg( szErrMsg, szMSG_ADD_ValSrc,  2, szSrcString, NULL ) ;
               BuildMsg( szErrMsg, szMSG_ADD_SegSrc,  2, sdSrcHead->szText, NULL ) ;
               if ( ShowWarning( szErrMsg ) == IDOK ) 
                  usReturn = RC_WARNING ;
               else
                  usReturn = RC_ERROR ;
               WriteXmlSegment( fOut,      /* Error */
                                sdSrcHead->ulSegNum, usSrcSegCount,   
                                szSrcString,
                                szTgtString,
                                szChgString,
                                szUniqueId, szProblemMsg ) ;
            }
         } else { 

            /*---------------------------------------------------------------------*/
            /*  Handle source text which has 2 or more source segments.            */
            /*---------------------------------------------------------------------*/

            /*---------------------------------------------------------------------*/
            /*  Find first source segment which matches the beginning of the       */
            /*  source string.  This is to handle JSON arrays.                     */
            /*---------------------------------------------------------------------*/

            for( sdSrcCur=sdSrcHead ; sdSrcCur ; sdSrcCur=(SEGDATA*)sdSrcCur->ptrNext ) {
               if ( ! strncmp( sdSrcCur->szText, szSrcString, strlen(sdSrcCur->szText) ) )
                   break ;
            }
            if ( sdSrcCur ) {
               for( sdTgtCur=sdTgtHead ; sdTgtCur ; sdTgtCur=(SEGDATA*)sdTgtCur->ptrNext ) {
                  if ( sdTgtCur->ulSegNum == sdSrcCur->ulSegNum ) 
                      break ;
               }
            }

            /*---------------------------------------------------------------------*/
            /*  Verify all source and target segments exactly match all of the     */
            /*  validation source and target segments.                             */
            /*---------------------------------------------------------------------*/
            if ( ( sdSrcCur ) &&
                 ( sdTgtCur ) ) {
               sdSrcTemp = sdSrcCur ;
               sdTgtTemp = sdTgtCur ;
               ptrSrcString = szSrcString ;
               ptrTgtString = szTgtString ;
               for( iSegCount=0 ; *ptrSrcString && *ptrTgtString ; ++iSegCount ) {
                  if ( ( sdSrcTemp ) &&
                       ( ! strncmp( sdSrcTemp->szText, ptrSrcString, strlen(sdSrcTemp->szText) ) ) &&
                       ( sdTgtTemp ) &&
                       ( ! strncmp( sdTgtTemp->szText, ptrTgtString, strlen(sdTgtTemp->szText) ) ) ) {
                     ptrSrcString += strlen(sdSrcTemp->szText) ;
                     ptrTgtString += strlen(sdTgtTemp->szText) ;
                     sdSrcTemp = (SEGDATA*)sdSrcTemp->ptrNext ;
                     sdTgtTemp = (SEGDATA*)sdTgtTemp->ptrNext ;
                  } else {
                     /*---------------------------------------------------------------------*/
                     /*  ERROR: Source/target text does not match validation source/target. */
                     /*---------------------------------------------------------------------*/
                     if ( ! sdSrcTemp ) {    /* Some source text does not exist in OTM */
                        strcpy( szProblemMsg, szWARN_ExtraSrc ) ;
                        BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValSrcLineNumber ) ;
                        BuildMsg( szErrMsg, szMSG_ADD_ValSrc, 2, ptrSrcString, NULL ) ;
                     } else                  /* Some source text does not match OTM */
                     if ( strncmp( sdSrcTemp->szText, ptrSrcString, strlen(sdSrcTemp->szText) ) ) { 
                        strcpy( szTemp, ptrSrcString ) ;
                        szTemp[strlen(sdSrcTemp->szText)] = 0 ;
                        strcpy( szProblemMsg, szWARN_MismatchSrc ) ;
                        BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValSrcLineNumber ) ;
                        BuildMsg( szErrMsg, szMSG_ADD_SegNum,  2, NULL, sdSrcTemp->ulSegNum ) ;
                        BuildMsg( szErrMsg, szMSG_ADD_ValSrc,  2, szTemp, NULL ) ;
                        BuildMsg( szErrMsg, szMSG_ADD_SegSrc,  2, sdSrcTemp->szText, NULL ) ;
                     } else
                     if ( ! sdTgtTemp ) {   /* Some target text does not exist in OTM */
                        strcpy( szProblemMsg, szWARN_ExtraTgt ) ;
                        BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValTgtLineNumber ) ;
                        BuildMsg( szErrMsg, szMSG_ADD_ValTgt,  2, ptrTgtString, NULL ) ;
                     } else                 /* Some Target text does not match OTM */ 
                     if ( strncmp( sdTgtTemp->szText, ptrTgtString, strlen(sdTgtTemp->szText) ) ){
                        strcpy( szTemp, ptrTgtString ) ;
                        szTemp[strlen(sdTgtTemp->szText)] = 0 ;
                        sprintf( szErrMsg, szWARN_MismatchTgt, sdTgtTemp->ulSegNum, ptrTgtString, sdTgtTemp->szText ) ;
                        strcpy( szProblemMsg, szWARN_MismatchTgt ) ;
                        BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValTgtLineNumber ) ;
                        BuildMsg( szErrMsg, szMSG_ADD_SegNum,  2, NULL, sdTgtHead->ulSegNum ) ;
                        BuildMsg( szErrMsg, szMSG_ADD_ValTgt,  2, szTemp, NULL ) ;
                        BuildMsg( szErrMsg, szMSG_ADD_SegTgt,  2, sdTgtHead->szText, NULL ) ;
                     }
                     ++ulSegmentsInvalid ;
                     bStringInvalid = TRUE ;
                     if ( ShowWarning( szErrMsg ) == IDOK ) 
                        usReturn = RC_WARNING ;
                     else
                        usReturn = RC_ERROR ;
                     WriteXmlSegment( fOut,      /* Error */
                                      sdSrcHead->ulSegNum, usSrcSegCount,   
                                      szSrcString,
                                      szTgtString,
                                      szChgString,
                                      szUniqueId, szProblemMsg ) ;
                     break ;
                  }
               }

               /*---------------------------------------------------------------------*/
               /*  All of Source and Target text has matching segments.               */
               /*  This means that further analysis can be done.                      */
               /*---------------------------------------------------------------------*/
               if ( ( ! *ptrSrcString ) &&          /* Source and Target text matches */
                    ( ! *ptrTgtString ) &&
                    ( ! bStringInvalid ) ) {
                  if ( iSegCount == 1 ) {           /* If match has only 1 segment    */
                     if ( ! strcmp( szTgtString, szChgString ) ) {
                        ++ulSegmentsUnchanged ;
                        szChgString[0] = 0 ;
                     } else {
                        ++ulSegmentsChanged ;
                     }
                     usReturn = RC_OK ;
                     WriteXmlSegment( fOut,      /* OK */
                                      sdSrcCur->ulSegNum, usSrcSegCount,   
                                      szSrcString,
                                      szTgtString,
                                      szChgString, 
                                      szUniqueId, "" ) ;
                  } else {

                     /*---------------------------------------------------------------------*/
                     /*  Determine which Target segments are unchanged in the changed text. */
                     /*  This will help to delimit segment boundaries for unchanged text.   */
                     /*---------------------------------------------------------------------*/
                     strcpy( szTemp, szChgString ) ;
                     sdTgtTemp = sdTgtCur ;
                     for( s=1 ; s<=iSegCount ; ++s ) {  
                        ptrChgString = strstr( szTemp, sdTgtTemp->szText ) ;
                        if ( ptrChgString ) {
                           memset( ptrChgString, cMATCH_CHAR, strlen(sdTgtTemp->szText) ) ;
                           sdTgtTemp->usMatch = 1 ;          /* Mark unchanged segment text */
                           sdTgtTemp->SegStart = ptrChgString ;
                           sdTgtTemp->SegEnd = ptrChgString + strlen(sdTgtTemp->szText) - 1 ;
                           if ( bDebug ) {
                              fprintf( fLog, "CHG match:      [" ) ;
                              fputs( sdTgtTemp->szText, fLog ) ;
                              fputs( "]\n", fLog ) ;
                           }
                        } else {
                           if ( bDebug ) {
                              fprintf( fLog, "CHG NO match:   [" ) ;
                              fputs( sdTgtTemp->szText, fLog ) ;
                              fputs( "]\n", fLog ) ;
                           }
                        }
                        sdTgtTemp = (SEGDATA*)sdTgtTemp->ptrNext ;
                     }
                     sdTgtTemp = sdTgtCur ;
                     ptrChgString = szTemp ;
                     if ( bDebug ) {
                        fprintf( fLog, "CHG Text:   [" ) ;
                        fputs( szTemp, fLog ) ;
                        fputs( "]\n", fLog ) ;
                     }

                     /*---------------------------------------------------------------------*/
                     /*  For each changed target segment, identify corresponding            */
                     /*  validation target text which goes with this segment.               */
                     /*---------------------------------------------------------------------*/
                     for( s=1 ; s<=iSegCount ; ++s ) {
                        if ( ! sdTgtTemp->usMatch ) {                   /* Segment has changed */
                           for( ; *ptrChgString==cMATCH_CHAR ; ++ptrChgString ) ;  /* Skip unchanged text */
                           if ( *ptrChgString ) {
                              sdTgtTemp->SegStart = ptrChgString ;
                              sdTgtNext = (SEGDATA*)sdTgtTemp->ptrNext ;

                              if ( ! sdTgtNext ) {                  /* If last segment of text */
                                 sdTgtTemp->SegEnd = ptrChgString + strlen(ptrChgString) - 1 ; /* Rest of text */
                                 sdTgtTemp->usMatch = 2 ;          /* Mark changed last segment text */
                                 memset( ptrChgString, cMATCH_CHAR, sdTgtTemp->SegEnd - sdTgtTemp->SegStart + 1 ) ;
                                 ptrChgString += sdTgtTemp->SegEnd - sdTgtTemp->SegStart + 1 ;
                              } else

                              if ( sdTgtNext->usMatch ) {           /* Next segment is matched */
                                 ptr1 = strchr( ptrChgString, cMATCH_CHAR ) ;
                                 if ( ptr1 ) {
                                    sdTgtTemp->SegEnd = ptr1 - 1 ;
                                 } else {
                                    sdTgtTemp->SegEnd = ptrChgString + strlen(ptrChgString) - 1 ; /* Rest of text */
                                 }
                                 sdTgtTemp->usMatch = 2 ;        /* Mark changed segment between 2 matched segments */
                                 memset( ptrChgString, cMATCH_CHAR, sdTgtTemp->SegEnd - sdTgtTemp->SegStart + 1 ) ;
                                 ptrChgString += sdTgtTemp->SegEnd - sdTgtTemp->SegStart + 1 ;
                              } else {

                                 // 2 or more changed segments in a row.  Try to find the end of this segment.
                                 strcpy( szTemp2, ptrChgString ) ;
                                 ptr1 = strchr( szTemp2, cMATCH_CHAR ) ;
                                 if ( ptr1 ) {
                                    *ptr1 = 0 ;
                                 }
                                 i = min( strlen(sdTgtTemp->szText)-1, 5 ) ;
                                 strcpy( szEndChars, (sdTgtTemp->szText)+strlen(sdTgtTemp->szText)-i ) ;
                                 sdTgtNext = (SEGDATA*)sdTgtTemp->ptrNext ;
                                 for( ; i>2 ; --i ) {     /* Locate ending characters of segment */
                                    ptr1 = strstr( szTemp2, szEndChars ) ;
                                    if ( ptr1 ) {
                                       j = min( strlen(sdTgtNext->szText)-1, 5 ) ;
                                       for( ; j>2 ; --j ) {
                                          if ( ! strncmp( ptr1+i, sdTgtNext->szText, j ) ) {
                                             sdTgtTemp->SegEnd = ptrChgString + (ptr1+i-szTemp2) - 1 ;
                                             sdTgtTemp->usMatch = 2 ;        /* Mark changed segment */
                                             memset( ptrChgString, cMATCH_CHAR, sdTgtTemp->SegEnd - sdTgtTemp->SegStart + 1 ) ;
                                             ptrChgString += sdTgtTemp->SegEnd - sdTgtTemp->SegStart + 1 ;
                                             break ;
                                          }
                                       }
                                       if ( sdTgtTemp->usMatch == 2 ) 
                                          break ;
                                    } else {
                                       memmove( szEndChars, szEndChars+1, i ) ;
                                    }
                                 }

                                 // 2 or more changed segments in a row.  Try to find the beginning of the next segment.
                                 if ( sdTgtTemp->usMatch != 2 ) {
                                    strcpy( szTemp2, sdTgtNext->szText ) ;
                                    szTemp2[10] = 0 ;
                                    ptr1 = strstr( ptrChgString, szTemp2 ) ;
                                    if ( ptr1 ) {
                                       sdTgtTemp->SegEnd = ptr1 - 1 ;
                                       sdTgtTemp->usMatch = 2 ;        /* Mark changed segment */
                                       memset( ptrChgString, cMATCH_CHAR, sdTgtTemp->SegEnd - sdTgtTemp->SegStart + 1 ) ;
                                       ptrChgString += sdTgtTemp->SegEnd - sdTgtTemp->SegStart + 1 ;
                                       break ;
                                    }
                                 }
                                 if ( sdTgtTemp->usMatch != 2 ) {
                                    /* ERROR: Could not find the end of this segment in the changed text. */
                                    ++ulSegmentsInvalid ;
                                    bStringInvalid = TRUE ;
                                    strcpy( szProblemMsg, szWARN_MismatchChg ) ;
                                    BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValChgLineNumber ) ;
                                    BuildMsg( szErrMsg, szMSG_ADD_SegNum,  2, NULL, sdTgtTemp->ulSegNum ) ;
                                    BuildMsg( szErrMsg, szMSG_ADD_SegTgt,  2, sdTgtTemp->szText, NULL ) ;
                                    if ( ShowWarning( szErrMsg ) == IDOK ) 
                                       usReturn = RC_WARNING ;
                                    else
                                       usReturn = RC_ERROR ;
                                    WriteXmlSegment( fOut,      /* Error */
                                                     sdSrcHead->ulSegNum, usSrcSegCount,   
                                                     szSrcString,
                                                     szTgtString,
                                                     szChgString, 
                                                 //  "",
                                                 //  sdTgtTemp->szText,
                                                 //  ptrChgString,
                                                     szUniqueId, szProblemMsg ) ;
                                 }




                                 // other options...


                              }
                           } else {
                              /* ERROR: End of changed text, but still unmatched target segments. */
                              ++ulSegmentsInvalid ;
                              bStringInvalid = TRUE ;
                              strcpy( szProblemMsg, szWARN_MismatchChg ) ;
                              BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValChgLineNumber ) ;
                              BuildMsg( szErrMsg, szMSG_ADD_SegNum,  2, NULL, sdTgtTemp->ulSegNum ) ;
                              BuildMsg( szErrMsg, szMSG_ADD_SegTgt,  2, sdTgtTemp->szText, NULL ) ;
                              if ( ShowWarning( szErrMsg ) == IDOK ) 
                                 usReturn = RC_WARNING ;
                              else
                                 usReturn = RC_ERROR ;
                              WriteXmlSegment( fOut,      /* Error */
                                               sdSrcHead->ulSegNum, usSrcSegCount,   
                                               szSrcString,
                                               szTgtString,
                                               szChgString, 
                                           //  "",
                                           //  sdTgtTemp->szText,
                                           //  ptrChgString,
                                               szUniqueId, szProblemMsg ) ;
                           }

                           if ( ! sdTgtTemp->ptrPrev ) {
                              //  Add more options.    ???
                           }
                        }
                        sdTgtTemp = (SEGDATA*)sdTgtTemp->ptrNext ;
                     }

                     /*---------------------------------------------------------------------*/
                     /*  Find extra changed text which does not match any segment.          */
                     /*---------------------------------------------------------------------*/
                     for( ptrChgString=szTemp; *ptrChgString ; ++ptrChgString ) {  
                        if ( *ptrChgString != cMATCH_CHAR ) {
                           ptr1 = strchr( ptrChgString, cMATCH_CHAR ) ;
                           if ( ! *ptr1 ) {
                              ptr1 = ptrChgString + strlen(ptrChgString) ;
                              *(ptr1+1) = 0 ;
                           }
                           if ( bDebug ) {
                              fprintf( fLog, "Extra CHG:      [" ) ;
                              fputs( ptrChgString, fLog ) ;
                              fputs( "]\n", fLog ) ;
                           }
                           if ( ptrChgString == szTemp ) {    /* Prepend to first segment */
                              if ( sdTgtHead->SegStart == ptr1 ) {
                                 sdTgtHead->SegStart = ptrChgString ;
                                 if ( sdTgtHead->usMatch == 1 )      
                                    sdTgtHead->usMatch = 2 ;
                              }
                           } else {
                              for( sdTgtCur=sdTgtHead ; sdTgtCur ; /* Concat to prev segment */
                                   sdTgtCur=(SEGDATA*)sdTgtCur->ptrNext ) {
                                 if ( sdTgtCur->SegEnd == ptrChgString - 1 ) {
                                    sdTgtCur->SegEnd = ptr1 - 1 ;
                                    if ( sdTgtCur->usMatch == 1 )      
                                       sdTgtCur->usMatch = 2 ;
                                 }
                              }
                           }
                           ptrChgString = ptr1 ;
                        }
                     }

                     /*---------------------------------------------------------------------*/
                     /*  Save segments where translated text has changed.                   */
                     /*---------------------------------------------------------------------*/
                     strcpy( szTemp, szChgString ) ;
                     for( sdSrcCur=sdSrcHead, sdTgtCur=sdTgtHead ; 
                          sdSrcCur && sdTgtCur ;
                          sdSrcCur=(SEGDATA*)sdSrcCur->ptrNext,
                          sdTgtCur=(SEGDATA*)sdTgtCur->ptrNext ) {
                        if ( sdTgtCur->usMatch == 1 ) {      /* Unchanged segment text      */
                           ++ulSegmentsUnchanged ;
                           usReturn = RC_OK ;
                           strcpy( szTemp2, sdTgtCur->SegStart ) ;
                           szTemp2[sdTgtCur->SegEnd-sdTgtCur->SegStart+1] = 0 ;;
                           WriteXmlSegment( fOut,      /* OK */
                                            sdSrcCur->ulSegNum, 1,
                                            sdSrcCur->szText,
                                            sdTgtCur->szText,
                                            "",
                                            szUniqueId, "" ) ;
                        } else 
                        if ( sdTgtCur->usMatch == 0 ) {      /* Invalid segment             */
                           ++ulSegmentsInvalid ;
                        } else
                        if ( sdTgtCur->usMatch == 2 ) {      /* Changed target text         */
                           ++ulSegmentsChanged ;
                           usReturn = RC_OK ;
                           strcpy( szTemp2, sdTgtCur->SegStart ) ;
                           szTemp2[sdTgtCur->SegEnd-sdTgtCur->SegStart+1] = 0 ;;
                           WriteXmlSegment( fOut,      /* OK */
                                            sdSrcCur->ulSegNum, 1,
                                            sdSrcCur->szText,
                                            sdTgtCur->szText,
                                            szTemp2, 
                                            szUniqueId, "" ) ;
                        } 
                     }
                  }
               } else {
                  /* ERROR: Extra source or target text. */
                  if ( ! bStringInvalid ) {
                     bStringInvalid = TRUE ;
                     if ( *ptrSrcString ) {
                        strcpy( szProblemMsg, szWARN_ExtraSrc ) ;
                        BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValSrcLineNumber ) ;
                        BuildMsg( szErrMsg, szMSG_ADD_ValSrc, 2, ptrSrcString, NULL ) ;
                     } else
                     if ( *ptrTgtString ) {
                        strcpy( szProblemMsg, szWARN_ExtraTgt ) ;
                        BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValTgtLineNumber ) ;
                        BuildMsg( szErrMsg, szMSG_ADD_ValTgt, 2, ptrTgtString, NULL ) ;
                     }
                     if ( ShowWarning( szErrMsg ) == IDOK ) 
                        usReturn = RC_WARNING ;
                     else
                        usReturn = RC_ERROR ;
                     WriteXmlSegment( fOut,      /* ERROR */
                                      sdSrcHead->ulSegNum, usSrcSegCount,   
                                      ptrSrcString,
                                      ptrTgtString,
                                      szChgString,
                                      szUniqueId, szProblemMsg ) ;
                  }
               }
            } else {
               /* ERROR: Extra source or target text. */
               if ( ! bStringInvalid ) {
                  bStringInvalid = TRUE ;
                  if ( ! sdSrcCur ) {
                     strcpy( szProblemMsg, szWARN_ExtraSrc ) ;
                     BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValSrcLineNumber ) ;
                     BuildMsg( szErrMsg, szMSG_ADD_ValSrc, 2, szSrcString, NULL ) ;
                  } else 
                  if ( ! sdTgtCur ) {
                     strcpy( szProblemMsg, szWARN_ExtraTgt ) ;
                     BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValTgtLineNumber ) ;
                     BuildMsg( szErrMsg, szMSG_ADD_ValTgt, 2, szTgtString, NULL ) ;
                  }
                  if ( ShowWarning( szErrMsg ) == IDOK ) 
                     usReturn = RC_WARNING ;
                  else
                     usReturn = RC_ERROR ;
                  WriteXmlSegment( fOut,      /* ERROR */
                                   sdSrcHead->ulSegNum, usSrcSegCount,   
                                   szSrcString,
                                   szTgtString,
                                   szChgString,
                                   szUniqueId, szProblemMsg ) ;
               }
            }
         }
      } else {
         /* ERROR: Could not get source/target segments for this context ID */
         ++ulSegmentsInvalid ;
         bStringInvalid = TRUE ;
         strcpy( szProblemMsg, szWARN_BadSegmentData ) ;
         if ( usRet1 != RC_OK ) {
            BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValSrcLineNumber ) ;
            BuildMsg( szErrMsg, szMSG_ADD_ValSrc, 2, szSrcText, NULL ) ;
         } else {
            BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValTgtLineNumber ) ;
            BuildMsg( szErrMsg, szMSG_ADD_ValTgt, 2, szTgtText, NULL ) ;
         }
         if ( ShowWarning( szErrMsg ) == IDOK ) 
            usReturn = RC_WARNING ;
         else
            usReturn = RC_ERROR ;
         if ( usSrcSegCount ) {
            WriteXmlSegment( fOut,      /* ERROR */
                             sdSrcHead->ulSegNum, usSrcSegCount,   
                             szSrcString,
                             szTgtString,
                             szChgString,
                             szUniqueId, szProblemMsg ) ;
         } else {
            WriteXmlSegment( fOut,      /* ERROR */
                             0, 0,
                             szSrcString,
                             szTgtString,
                             szChgString,
                             szUniqueId, szProblemMsg ) ;
         }
      }

      if ( bDebug ) {
         fprintf( fLog, "FREE Src....\n" );
         for( sdSrcCur=sdSrcHead ; sdSrcCur ; sdSrcCur=(SEGDATA*)sdSrcCur->ptrNext ) {
            fprintf( fLog, "    %ld   %d   [",sdSrcCur->ulSegNum, sdSrcCur->bTrans ) ;
            fputs( sdSrcCur->szText,fLog ) ;
            fputs( "]\n", fLog ) ;
         }
         fprintf( fLog, "FREE Tgt....\n" );
         for( sdTgtCur=sdTgtHead ; sdTgtCur ; sdTgtCur=(SEGDATA*)sdTgtCur->ptrNext ) {
            fprintf( fLog, "    %ld   %d   [",sdTgtCur->ulSegNum, sdTgtCur->bTrans ) ;
            fputs( sdTgtCur->szText,fLog ) ;
            fputs( "]\n", fLog ) ;
         }
      }

      for( ; sdSrcHead ; sdSrcHead=sdSrcCur ) {
         free( sdSrcHead->szText ) ;
         sdSrcCur = (SEGDATA*)sdSrcHead->ptrNext ;
         free( sdSrcHead ) ;
      }
      for( ; sdTgtHead ; sdTgtHead=sdTgtCur ) {
         free( sdTgtHead->szText ) ;
         sdTgtCur = (SEGDATA*)sdTgtHead->ptrNext ;
         free( sdTgtHead ) ;
      }
      
   } else {
      /* ERROR: Could not find source/target text for this context ID */
      bStringInvalid = TRUE ;
      strcpy( szProblemMsg, szWARN_MissingContext ) ;
      BuildMsg( szErrMsg, szProblemMsg, 1, NULL, ulValIdLineNumber ) ;
      BuildMsg( szErrMsg, szMSG_ADD_Context,    2, szUniqueId, NULL ) ;
      BuildMsg( szErrMsg, szMSG_ADD_ValSrc,     2, szSrcString, NULL ) ;
      if ( ShowWarning( szErrMsg ) == IDOK ) 
         usReturn = RC_WARNING ;
      else
         usReturn = RC_ERROR ;
      WriteXmlSegment( fOut,      /* ERROR */
                       0, 0,
                       szSrcString,
                       szTgtString,
                       szChgString,
                       szUniqueId, szProblemMsg ) ;
   }

   if ( bStringInvalid ) 
      ++ulStringsInvalid ; 

   return( usReturn );
}


/****************************************************************************/
/*                                                                          */
/* FindOtmContext                                                           */
/*                                                                          */
/* Find the <TWBCTX> value which matches the string we are looking for.     */
/*                                                                          */
/* Input:      fFile         - Segmented OTM file to search.                */
/*             szContext     - Context string to find.                      */
/*             szMatch       - Text to match after context.                 */
/*             ulMatchSegNum - Segment number to match (if szMatch is NULL).*/
/* Output:     szString      - Rest of text after context string.           */
/* Return:     RC_OK         - Context string found.                        */
/*             RC_ERROR      - Context string does not exist in this file.  */
/*                                                                          */
/****************************************************************************/

USHORT  FindOtmContext( FILE *fFile, char *szContext, char *szMatch,
                        ULONG *ulMatchSegNum, char *szString ) 
{
   WCHAR    szSrcIn[MAX_RCD_LENGTH] ;

   char     szIn[MAX_RCD_LENGTH] ;
   char     szSegment[MAX_RCD_LENGTH] ;
   char     *ptr1 ;
   ULONG    ulSegNum ;
   ULONG    ulFilePos ;
   ULONG    ulFilePosCtx ;
   ULONG    ulLastContextPos = 0 ;
   LONG     lRc ;

   int      iResetFile ;
   BOOL     bContextMatch = FALSE ;
   BOOL     bFound = FALSE ;
   USHORT   usReturn = RC_OK ;


   szString[0] = 0 ;
   ulFilePos = ftell( fFile ) ;

   for( iResetFile=0 ; ! bFound && iResetFile<2 ; ++iResetFile ) {
      if ( iResetFile == 1 ) {
         fseek( fFile, 0, SEEK_SET ) ;        /* Reset to beginning of file */
      }

      while ( usReturn == RC_OK ) {
         ulFilePosCtx = ftell( fFile ) ;
         if ( fgetws( szSrcIn, MAX_RCD_LENGTH, fFile ) == NULL ) 
            break;

        if ( ! Unicode2UTF8( szSrcIn, szIn, sizeof(szIn), TRUE, &lRc ) ) {
           usReturn = RC_ERROR ;
           sprintf( szErrMsg, szERR_UTF8Conversion ) ;
           ShowError( szErrMsg ) ;
           break ;
        }

        ptr1 = strstr( szIn, "<TWBCTX " ) ;
        if ( ptr1 ) {
           bContextMatch = FALSE ;
           ulLastContextPos = 0 ;
           ptr1 += 8 ;
           if ( ( ! strncmp( ptr1, szContext, strlen(szContext) ) ) &&
                ( *(ptr1+strlen(szContext)) == '>' ) ) {
              bContextMatch = TRUE ;
              ulLastContextPos = ulFilePosCtx ;
              strcpy( szString, (ptr1+strlen(szContext)+1) ) ;
              if ( ( ( GetOtmNextSegment( szString, szSegment, &ulSegNum ) ) &&
                     ( ( ( szMatch ) &&
                         ( ! strncmp( szMatch, szSegment, strlen(szSegment) ) ) ) ||
                       ( ( ! szMatch ) &&
                         ( *ulMatchSegNum == ulSegNum ) ) ) ) ||
                   ( ! strcmp( szContext, "value.product.meta-keywords" ) ) ) {
                 if ( szMatch )
                    *ulMatchSegNum = ulSegNum ; 
                 bFound = TRUE ;
                 break ; 
              }
           }
        } else 
        if ( bContextMatch ) {
           strcpy( szString, szIn ) ;
           if ( ( ( GetOtmNextSegment( szString, szSegment, &ulSegNum ) ) &&
                  ( ( ( szMatch ) &&
                      ( ! strncmp( szMatch, szSegment, strlen(szSegment) ) ) ) ||
                    ( ( ! szMatch ) &&
                      ( *ulMatchSegNum == ulSegNum ) ) ) ) ||
                ( ! strcmp( szContext, "value.product.meta-keywords" ) ) ) {
              if ( szMatch )
                 *ulMatchSegNum = ulSegNum ; 
              if ( ulLastContextPos )
                 fseek( fFile, ulLastContextPos, SEEK_SET ) ;
              bFound = TRUE ;
              break ; 
           }
        }

        if ( ( iResetFile ) &&                /* If read entire file, quit  */
             ( ulFilePos < (ULONG)ftell( fFile ) ) ) {
           break;
        }
      }
   }
   if ( ! bFound ) 
      usReturn = RC_ERROR ;

   return( usReturn );
}


/****************************************************************************/
/*                                                                          */
/* GetOtmNextSegment                                                        */
/*                                                                          */
/* Get the next segment from the current string.                            */
/*                                                                          */
/* Input:      fFile         - Segmented OTM file to read.                  */
/*             szString      - String to find segment info for.             */
/* Output:     ptrHead       - Pointer to start of segment data.            */
/*             ptrTail       - Pointer to end of segment data.              */
/* Return:     RC_OK         - Segment data was found.                      */
/*             RC_ERROR      - Segment data could not be found.             */
/*                                                                          */
/****************************************************************************/

BOOL  GetOtmNextSegment( char *szString, char *szSegment, ULONG *ulSegNum ) 
{

   char     szNum[10] ;
   char     *ptr1, *ptr2 ;

   BOOL     bReturn = FALSE ;


   if ( bDebug ) {
      fputs( "Get Segment:  \n",fLog ) ;
   }


   for( ptr1=szString ; *ptr1 ; ++ptr1 ) {
      if ( ( *ptr1 == ':') &&
           ( strchr( "eEqQ", *(ptr1+1) ) ) ) {
         ++ptr1 ;
         if ( ( ( ! strncmp( ptr1, "eqf", 3 ) ) || 
                ( ! strncmp( ptr1, "EQF", 3 ) ) ) &&
              ( isalpha( *(ptr1+3) ) ) &&
              ( *(ptr1+4) == '.' ) ) {
         } else
         if ( ( ( ! strncmp( ptr1, "qf", 2 ) ) || 
                ( ! strncmp( ptr1, "QF", 2 ) ) ) &&
              ( isalpha( *(ptr1+2) ) ) &&
              ( *(ptr1+3) == ' ' ) ) {
            *ulSegNum = 0 ;
            for( ptr1+=4 ; *ptr1 && *ptr1!='.' ; ++ptr1 ) {
               if ( ( ! strncmp( ptr1, "n=", 2 ) ) ||
                    ( ! strncmp( ptr1, "N=", 2 ) ) ) {
                  strncpy( szNum, ptr1+2, sizeof(szNum) ) ;
                  strtok( szNum, " ." ) ;
                  *ulSegNum = atoi(szNum ) ;
               }
            }
            if ( *ptr1 ) {
               strcpy( szSegment, ptr1+1 ) ;
               ptr2 = strstr( szSegment, ":eqf" ) ;
               if ( ! ptr2 ) 
                  ptr2 = strstr( szSegment, ":EQF" ) ;
               if ( ptr2 ) {
                  *ptr2 = 0 ;
                  bReturn = TRUE ;
                  if ( bDebug ) {
                     fprintf( fLog, "   NextSeg:  %d  [", *ulSegNum ) ;
                     fputs( szSegment, fLog ) ;
                     fputs("]\n", fLog ) ;
                  }
                  break ;
               }
            }
         } 
      }
   }

   return( bReturn );
}


/****************************************************************************/
/*                                                                          */
/* GetOtmSegments                                                           */
/*                                                                          */
/* Get the segments which correspond to this string we are looking for.     */
/*                                                                          */
/* Input:      fFile         - Segmented OTM file to read.                  */
/*             szString      - String to find segment info for.             */
/* Output:     usSegCount    - Number of segments in string.                */
/*             ptrHead       - Pointer to start of segment data.            */
/*             ptrTail       - Pointer to end of segment data.              */
/* Return:     RC_OK         - Segment data was found.                      */
/*             RC_ERROR      - Segment data could not be found.             */
/*                                                                          */
/****************************************************************************/

USHORT  GetOtmSegments( FILE *fFile, char *szString, USHORT *usSegCount, 
                        SEGDATA **ptrHead, SEGDATA **ptrTail ) 
{
   SEGDATA  *ptrCur ;
   WCHAR    szSrcIn[MAX_RCD_LENGTH] ;

   char     szSegment[MAX_RCD_LENGTH] ;
   char     szNum[10] ;
   char     *ptr1, *ptr2 ;
   LONG     lRc ;
   ULONG    ulSegNum ;
   ULONG    ulFilePos = 0 ;

   BOOL     bTrans = FALSE ;
   BOOL     bFound = FALSE ;
   USHORT   usReturn = RC_OK ;


   if ( bDebug ) {
      fputs( "Get Segments:  \n",fLog ) ;
   }
   *usSegCount = 0 ;

   for( ptr1=szString ; usReturn==RC_OK ; ) {
      if ( bDebug ) {
         fputs( "   String: [",fLog ) ;
         fputs( szString, fLog ) ;
         fputs("]\n", fLog );
      }
      if ( ! *ptr1 ) {
         ulFilePos = ftell( fFile ) ;
         if ( fgetws( szSrcIn, MAX_RCD_LENGTH, fFile ) != NULL ) {
            if ( ! Unicode2UTF8( szSrcIn, szString, MAX_RCD_LENGTH, TRUE, &lRc ) ) {
               usReturn = RC_ERROR ;
               sprintf( szErrMsg, szERR_UTF8Conversion ) ;
               ShowError( szErrMsg ) ;
               break ;
            }
            if ( bDebug ) {
               fputs( "   Line:   [", fLog ) ;
               fputs( szString, fLog ) ;
               fputs("]\n", fLog ) ;
            }
         } else {
            break;
         }
      }

      for( ptr1=szString ; *ptr1 && usReturn==RC_OK ; ++ptr1 ) {
         if ( ( *ptr1 == ':') &&
              ( strchr( "eEqQ", *(ptr1+1) ) ) ) {
            ++ptr1 ;
            if ( ( ( ! strncmp( ptr1, "eqf", 3 ) ) || 
                   ( ! strncmp( ptr1, "EQF", 3 ) ) ) &&
                 ( isalpha( *(ptr1+3) ) ) &&
                 ( *(ptr1+4) == '.' ) ) {
            } else
            if ( ( ( ! strncmp( ptr1, "qf", 2 ) ) || 
                   ( ! strncmp( ptr1, "QF", 2 ) ) ) &&
                 ( isalpha( *(ptr1+2) ) ) &&
                 ( *(ptr1+3) == ' ' ) ) {

               if ( ! strchr( "nN", *(ptr1+2) ) )
                 bTrans = TRUE ;
               ulSegNum = 0 ;
               for( ptr1+=4 ; *ptr1 && *ptr1!='.' ; ++ptr1 ) {
                  if ( ( ! strncmp( ptr1, "n=", 2 ) ) ||
                       ( ! strncmp( ptr1, "N=", 2 ) ) ) {
                     strncpy( szNum, ptr1+2, sizeof(szNum) ) ;
                     strtok( szNum, " ." ) ;
                     ulSegNum = atoi(szNum ) ;
                  }
               }
               if ( *ptr1 ) {
                  strcpy( szSegment, ptr1+1 ) ;
                  ptr2 = strstr( szSegment, ":eqf" ) ;
                  if ( ! ptr2 ) 
                     ptr2 = strstr( szSegment, ":EQF" ) ;
                  if ( ptr2 ) {
                     *ptr2 = 0 ;
                     if ( bDebug ) {
                        fprintf( fLog, "   AddSeg:  %d  [", ulSegNum ) ;
                        fputs( szSegment, fLog ) ;
                        fputs("]\n", fLog ) ;
                     }
                     ptrCur = (SEGDATA*)calloc( sizeof(SEGDATA), 1 ) ;
                     if ( ptrCur ) {
                        ptrCur->bTrans = bTrans ; 
                        ptrCur->ulSegNum = ulSegNum ;
                        ptrCur->szText = (char*)malloc( strlen(szSegment)+1 ) ;
                        strcpy( ptrCur->szText, szSegment ) ;
                        if ( ! (*ptrHead) ) 
                           (*ptrHead) = ptrCur ;
                        if ( *ptrTail ) 
                           (*ptrTail)->ptrNext = ptrCur ;
                        ptrCur->ptrPrev = (*ptrTail) ;
                        ptrCur->ptrNext = NULL ;
                        (*ptrTail) = ptrCur ; 
                        ptr1 += ptr2 - szSegment + 6 ;
                        memmove( szString, ptr1, strlen(ptr1)+1 ) ;
                        ptr1 = szString ;
                        bFound = TRUE ;
                        ++(*usSegCount) ;
                     }
                  }
               }
            } 
         } else
         if ( !strncmp( ptr1, "<TWBCTX ", 8 ) ) {
            if ( ulFilePos ) {
               fseek( fFile, ulFilePos, 0 ) ;
            }
            break ;
         }
      }
      if ( !strncmp( ptr1, "<TWBCTX ", 8 ) ) {
         break ;
      }
   }
   if ( ! bFound ) {
      usReturn = RC_ERROR ;
      *usSegCount = 0 ;
   }

   return( usReturn );
}


/****************************************************************************/
/*                                                                          */
/* SetObjectText                                                            */
/*                                                                          */
/* Get the segments which correspond to this string we are looking for.     */
/*                                                                          */
/* Input:      szIn          - Input object text to adjust.                 */
/* Output:     szOut         - Output adjusted object text.                 */
/*                                                                          */
/****************************************************************************/

void   SetObjectText( char *szOut, char *szIn ) 
{
   char     *ptr ;
   int      i ;


   strcpy( szOut, szIn ) ;
   for( ptr=strchr(szOut,'&') ; ptr ; ptr=strchr(ptr+1,'&') ) {
      for( i=0 ; i<50 && iswalnum(*(ptr+i)) ; ++i ) ;
      if ( *(ptr+i) != ';' ) {
         memmove( ptr+8, ptr+1, strlen(ptr+1)+1 ) ;
         strncpy( ptr+1, "TWBAMP;", 7 ) ;
      }
   }

   return ;
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
/* Output:     None.                                                        */
/* Return:     TRUE          - Segment written successfully.                */
/*             FALSE         - Segment could not be written.                */
/*                                                                          */
/****************************************************************************/

BOOL  WriteXmlSegment( FILE *fOut, ULONG ulSegNum, USHORT usSegCount, 
                       char *szSource, char *szTarget, char *szChanged, 
                       char *szComment, char *szProblem ) 
{
   if ( usSegCount > 1 ) {
      fprintf( fOut, szXML_Segment_2, ulSegNum, usSegCount ) ;
   } else {
      fprintf( fOut, szXML_Segment, ulSegNum ) ;
   }
   fprintf( fOut, szXML_Segment_Source, szSource ) ;
   fprintf( fOut, szXML_Segment_OrgTarget, szTarget ) ;
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
/* UTF82Unicode                                                             */
/*                                                                          */
/* Convert UTF-8 text to UTF-16 text.                                       */
/*                                                                          */
/****************************************************************************/

ULONG UTF82Unicode( char *szUTF8, WCHAR *szUni, LONG lUniLen, BOOL fMsg, PLONG plRc ) 
{
  ULONG    ulOutput = 0;
  DWORD    dwFlags = 0;
  LONG     lRc = 0;

  fMsg;

  szUni[0] = NULL ;
  dwFlags = MB_ERR_INVALID_CHARS;
  ulOutput = MultiByteToWideChar( CP_UTF8, dwFlags, szUTF8, -1, szUni, lUniLen );

  if ( ! ulOutput ) {
     lRc = GetLastError();
  } else {
     lRc = 0 ;
  }

  *plRc = lRc ;
  return( ulOutput ) ;
}


/****************************************************************************/
/*                                                                          */
/* Unicode2UTF8                                                             */
/*                                                                          */
/* Convert UTF-16 text to UTF-8 text.                                       */
/*                                                                          */
/****************************************************************************/

ULONG Unicode2UTF8( WCHAR *szUni, char *szUTF8, LONG lUTF8Len, BOOL fMsg, PLONG plRc ) 
{
  ULONG    ulOutput = 0;
  LONG     lRc = 0;

  fMsg;

  szUTF8[0] = 0 ;
  ulOutput = WideCharToMultiByte( CP_UTF8, 0, szUni, -1, szUTF8, lUTF8Len, NULL, NULL );

  if ( ! ulOutput ) {
     lRc = GetLastError();
  } else {
     lRc = 0 ;
  }

  *plRc = lRc ;
  return( ulOutput ) ;
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
   usRC = (USHORT)MessageBoxA( NULL, szText, szWARN_Title, MB_OKCANCEL| MB_ICONWARNING | MB_DEFBUTTON1 ) ;

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
    strncpy( pszFilterName, "PWB Validation JSON file", iFilterNameBufSize - 1);
    pszFilterName[iFilterNameBufSize - 1] = 0;
  }
  if ( pszFileExtension != NULL )
  {
    strncpy( pszFileExtension, ".JSON", iFileExtensionBufSize - 1 );
    pszFileExtension[iFileExtensionBufSize - 1] = 0;
  }
  if ( pszVersion != NULL )
  {
    strncpy( pszVersion, "0.1", iVersionBufSize - 1 );
    pszVersion[iVersionBufSize - 1] = 0;
  }

  return( 0 );
}
