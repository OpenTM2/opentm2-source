/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/******************************************************************************
*
* parse.c
*
* Functions called to parse OpenDocument XML files
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


    BOOL GetAttributeValue   ( WCHAR*, WCHAR*, WCHAR* ) ;  
            

    CHAR     szODC_P[8]                     = "<text:p" ;
    CHAR     szODC_ODCNL[9]                 = "<ODCnl/>" ;
    CHAR     szODC_ODCNL_NL[10]             = "<ODCnl/>\n" ;
    CHAR     szODC_TEXT_CHANGE[12]          = "text:change" ;
    CHAR     szODC_CHANGE[13]               = "<text:change" ;
    CHAR     szODC_CHANGE_END[17]           = "<text:change-end" ;
    CHAR     szODC_CHANGE_START[19]         = "<text:change-start" ;
    CHAR     szODC_TRACKEDCHANGES_START[22] = "<text:tracked-changes" ;
    CHAR     szODC_TRACKEDCHANGES_END[23]   = "</text:tracked-changes" ;
    CHAR     szODC_BODY[13]                 = "<office:body" ;
    CHAR     szODC_DOCUMENTCONTENT[25]      = "<office:document-content" ;
    CHAR     szODC_TWBSTYLE[13]             = "<TWB:style/>" ;

    WCHAR    szODC_TWB_END[7]               = L"--twb>" ;
    WCHAR    szODC_TWB_START[7]             = L"<twb--" ;
    WCHAR    szODC_TAG_END[6]               = L"-end " ;
    WCHAR    szODC_TAG_START[8]             = L"-start " ;
    WCHAR    szODC_STRING_VALUE[18]         = L"text:string-value" ;
            

typedef struct
{
   WCHAR   Tag[ODC_TAG_SIZE] ;
} TAGLIST ;

#define NUM_REF_TAGS   5

static TAGLIST REF_TAGS[ NUM_REF_TAGS ] =
{
 L"text:bookmark", 
 L"text:reference",
 L"text:toc-mark", 
 L"text:user-index",
 L"text:alphabetical",
} ;



/****************************************************************************/
/*                                                                          */
/* PreParse1                                                                */
/*                                                                          */
/* Function is called by EQFPRESEG2 -- before -- OTMXML processing.         */
/*                                                                          */
/* Input:      InFile        - Input file name.                             */
/*             OutFile       - Output file name.                            */
/*             StyleFile     - Output file for saved style information.     */
/*             hwndSlider    - Handle of status slider.                     */
/* Output:     TRUE          - Porcessing successful.                       */
/*             FALSE         - Processing failed.                           */
/****************************************************************************/
BOOL PreParse1(PSZ InFile, PSZ OutFile, PSZ StyleFile, HWND hwndSlider)
{
    FILE     *fIn, *fOut, *fStyle ;
    CHAR     szIn[MAX_RCD_LENGTH*3] ;
    CHAR     *ptrChar, *ptrChar2 ;
    CHAR     *ptrTrackedChangesStart ;
    LONG     lFileSize, lFileBytes ;
    USHORT   i, j ;
    USHORT   usTrackedChangesState = 0 ;
    USHORT   usStyleState = ODC_STYLE_STATE_BEFORE ;
    BOOL     bParagraphFound = FALSE ;
    BOOL     bReturn = TRUE;


    fIn = fopen( InFile, "r" ) ;
    fOut = fopen( OutFile, "w" ) ;
    if ( StyleFile[0] ) 
       fStyle = fopen( StyleFile, "w" ) ;
    else
       usStyleState = ODC_STYLE_STATE_AFTER ;
    if ( !fIn || !fOut || ( !fStyle && usStyleState==ODC_STYLE_STATE_BEFORE ) ) {
       if ( fIn ) 
          fclose( fIn ) ;
       if ( fOut ) 
          fclose( fOut ) ;
       if ( fStyle ) 
          fclose( fStyle ) ;
       return( FALSE ) ;
    }

    /*------------------------------------------------------------------------*/
    /*  Get number of characters in entire file.  Used to determine percent   */
    /*  completed for updating slider information.                            */
    /*------------------------------------------------------------------------*/
    fseek( fIn, 0, SEEK_END ) ;              /* Seek to the end of the file   */
    lFileSize = ftell( fIn ) ;               /* Get size of file              */
    fseek( fIn, 0, SEEK_SET ) ;              /* Seek to the beginning of file */
    lFileBytes = 0 ;

    if ( hwndSlider )
       EQFSETSLIDER( hwndSlider, 0 ) ;       /* Initialize slider to 0.       */
    /*------------------------------------------------------------------------*/
    /*  Process each line.                                                    */
    /*------------------------------------------------------------------------*/
    while ( GetRcd( szIn, MAX_RCD_LENGTH, fIn, TRUE ) != NULL ) {

       lFileBytes += strlen( szIn ) ; 
       if ( hwndSlider )
          EQFSETSLIDER( hwndSlider, (USHORT)((lFileBytes * 100)/lFileSize) ) ;


       /*---------------------------------------------------------------------*/
       /*  Find each <text:p> tag and make sure it starts on a new line.      */
       /*---------------------------------------------------------------------*/
       for( i=0 ; szIn[i] && isspace(szIn[i]) ; ++i ) ;
       if ( szIn[i] ) {
          for( ptrChar=strstr(&szIn[++i],szODC_P) ; 
               ptrChar ;
               ptrChar=strstr(ptrChar+1,szODC_P) ) {
             if ( ( isspace(*(ptrChar+strlen(szODC_P)) ) ) || 
                  ( strchr( "/>", *(ptrChar+strlen(szODC_P)) ) ) ) {            
                bParagraphFound = TRUE ;
                if ( *(ptrChar+strlen(szODC_P)) != '/' ) {            
                   memmove( ptrChar+9, ptrChar, strlen(ptrChar)+1 ) ;
                   strncpy( ptrChar, szODC_ODCNL_NL, 9 ) ;
                   ptrChar += 10 ;
                }
             }
          }
       }

       /*---------------------------------------------------------------------*/
       /*  Remove all tracked-changes tags and content.                       */
       /*---------------------------------------------------------------------*/
       if ( usTrackedChangesState == 0 ) {
          ptrChar = strstr( szIn, szODC_TRACKEDCHANGES_START ) ; 
          if ( ptrChar ) {
             ptrChar2 = strchr( ptrChar, '>' ) ;
             if ( ptrChar2 ) {
                if ( *(ptrChar2-1) == '/' ) {
                   usTrackedChangesState = 0 ;     /* No tracks to be removed  */
                } else {
                   ptrTrackedChangesStart = ptrChar ;
                   usTrackedChangesState = 1 ;
                }
             }
          }
       }
       if ( usTrackedChangesState == 1 ) {
          ptrChar = strstr( szIn, szODC_TRACKEDCHANGES_END ) ; 
          if ( ptrChar ) {
             ptrChar2 = strchr( ptrChar, '>' ) ;
             if ( ptrChar2 ) {
                memmove( ptrTrackedChangesStart, ptrChar2+1, strlen(ptrChar2+1)+1 ) ;
                usTrackedChangesState = 0 ;
             }
          } else {
             *ptrTrackedChangesStart = 0 ;
             ptrTrackedChangesStart = szIn ;
          }
       }

       /*---------------------------------------------------------------------*/
       /*  Remove all change flagging tags from the translatable text.        */
       /*---------------------------------------------------------------------*/
       if ( usTrackedChangesState != 1 ) {
          for( ptrChar=strstr(szIn,szODC_TEXT_CHANGE) ; 
               ptrChar ;    
               ptrChar=strstr(++ptrChar,szODC_TEXT_CHANGE) ) {
             if ( ptrChar > szIn ) {
                ptrChar2 = strchr( ptrChar, '>' ) ;
                if ( ( ptrChar2 ) &&
                     ( ( ! strncmp( ptrChar-1, szODC_CHANGE,       12 ) ) ||
                       ( ! strncmp( ptrChar-1, szODC_CHANGE_END,   16 ) ) ||
                       ( ! strncmp( ptrChar-1, szODC_CHANGE_START, 18 ) ) ) ) {
                   memmove( --ptrChar, ++ptrChar2, strlen(ptrChar2)+1 ) ;
                   if ( ptrChar > szIn ) 
                      --ptrChar ;
                }
             }
          }
       }

       /*---------------------------------------------------------------------*/
       /*  Save all style information to a separate file in order to reduce   */
       /*  the size of the segmented source files, and to reduce the time     */
       /*  to open the file in the editors.                                   */
       /*---------------------------------------------------------------------*/
       if ( usStyleState != ODC_STYLE_STATE_AFTER ) {
          if ( usStyleState == ODC_STYLE_STATE_BEFORE ) {
             ptrChar = strstr( szIn, szODC_DOCUMENTCONTENT ) ;
             if ( ptrChar ) {
                usStyleState = ODC_STYLE_STATE_IN ;
                *ptrChar = 0 ;
                fputs( szIn, fOut ) ;
                fputs( szODC_TWBSTYLE, fOut ) ;
                *ptrChar = '<' ; 
                memmove( szIn, ptrChar, strlen(ptrChar)+1 ) ;
             } else { 
                if ( bParagraphFound ) 
                   usStyleState = ODC_STYLE_STATE_AFTER ;
             }
          }
          if ( usStyleState == ODC_STYLE_STATE_IN ) {
             ptrChar = strstr( szIn, szODC_BODY ) ;
             if ( ptrChar ) {
                usStyleState = ODC_STYLE_STATE_AFTER ;
                *ptrChar = 0 ;
                fputs( szIn, fStyle ) ;
                *ptrChar = '<' ; 
                memmove( szIn, ptrChar, strlen(ptrChar)+1 ) ;
             } else {
                if ( bParagraphFound ) {
                   usStyleState = ODC_STYLE_STATE_AFTER ;
                   for( ptrChar=strstr(szIn,szODC_P) ; 
                        ptrChar ;
                        ptrChar=strstr(ptrChar+1,szODC_P) ) {
                      if ( ( isspace(*(ptrChar+strlen(szODC_P)) ) ) || 
                           ( strchr( "/>", *(ptrChar+strlen(szODC_P)) ) ) ) {            
                         break ;
                      }
                   }
                   if ( ptrChar ) 
                      *ptrChar = 0 ;
                   fputs( szIn, fStyle ) ;
                   *ptrChar = szODC_P[0] ; 
                   memmove( szIn, ptrChar, strlen(ptrChar)+1 ) ;
                } else {
                   fputs( szIn, fStyle ) ;
                   continue ;
                }
             }
          }
       }


       fputs( szIn, fOut ) ;
    }
    fclose( fIn ) ;
    fclose( fOut ) ;
    fclose( fStyle ) ;
    if ( usStyleState == ODC_STYLE_STATE_BEFORE ) /* No style info saved */
       remove( StyleFile ) ;

    EQFSETSLIDER(hwndSlider, 100) ;
    return(bReturn);

} /* PreParse1 */


/****************************************************************************/
/*                                                                          */
/* PreParse2                                                                */
/*                                                                          */
/* Function is called by EQFPRESEG2 -- after -- OTMXML processing.          */
/*                                                                          */
/* Input:      InFile        - Input file name.                             */
/*             OutFile       - Output file name.                            */
/*             hwndSlider    - Handle of status slider.                     */
/* Output:     TRUE          - Porcessing successful.                       */
/*             FALSE         - Processing failed.                           */
/****************************************************************************/
BOOL PreParse2(PSZ in, PSZ out, HWND hwndSlider)
{
    FILE     *fIn, *fOut ;
    WCHAR    szIn[MAX_RCD_LENGTH*2] ;
    WCHAR    szSave[MAX_RCD_LENGTH*2] ;
    WCHAR    szTag[ODC_TAG_SIZE] ;
    WCHAR    szTag1[ODC_TAG_SIZE] ;
    WCHAR    szTag2[ODC_TAG_SIZE] ;
    WCHAR    szAttribute1[512] ;
    WCHAR    szAttribute2[512] ;
    WCHAR    szSurroundTags[20][ODC_TAG_SIZE] ;
    WCHAR    *ptrSurround1[20] ;
    WCHAR    *ptrSurround2[20] ;
    WCHAR    *ptrTwbEnd, *ptrTwbStart ;
    WCHAR    *ptrChar, *ptrChar2, *ptrChar3 ;
    WCHAR    *ptrChar1s, *ptrChar1e, *ptrChar2s, *ptrChar2e ;
    LONG     lFileSize, lFileBytes ;
    USHORT   usTagIndex ;
    USHORT   i, j ;
    BOOL     bTagsRemoved ; 
    BOOL     bMoveTags ; 
    BOOL     bBeginEndTags ;
    BOOL     bReturn = TRUE;

    fIn = fopen( in, "rb" ) ;
    fOut = fopen( out, "wb" ) ;
    if ( !fIn || !fOut ) {
       if ( fIn ) 
          fclose( fIn ) ;
       if ( fOut ) 
          fclose( fOut ) ;
       return( FALSE ) ;
    }

    /*------------------------------------------------------------------------*/
    /*  Get number of characters in entire file.  Used to determine percent   */
    /*  completed for updating slider information.                            */
    /*------------------------------------------------------------------------*/
    fseek( fIn, 0, SEEK_END ) ;              /* Seek to the end of the file   */
    lFileSize = ftell( fIn ) ;               /* Get size of file              */
    fseek( fIn, 0, SEEK_SET ) ;              /* Seek to the beginning of file */
    lFileBytes = 0 ;

    if ( hwndSlider )
       EQFSETSLIDER( hwndSlider, 0 ) ;       /* Initialize slider to 0.       */

    /*------------------------------------------------------------------------*/
    /*  Process each line.                                                    */
    /*------------------------------------------------------------------------*/
    while ( fgetws( szIn, MAX_RCD_LENGTH, fIn ) != NULL ) {
       lFileBytes += wcslen(szIn) * 2 ; 
       if ( hwndSlider )
          EQFSETSLIDER( hwndSlider, (USHORT)((lFileBytes * 100)/lFileSize) ) ;

       ptrTwbEnd = wcsstr( szIn, szODC_TWB_END ) ; 
       if ( ptrTwbEnd ) 
          ptrTwbStart = wcsstr( ptrTwbEnd, szODC_TWB_START ) ; 
       else
          ptrTwbStart = 0 ;

       /*------------------------------------------------------------------------*/
       /*  Repeat checking for conditions where inline tags can be moved         */
       /*  from the translatable section of the text to the non-translatable     */
       /*  section.                                                              */
       /*------------------------------------------------------------------------*/
       bTagsRemoved = TRUE ;
       while( bTagsRemoved ) {
          bTagsRemoved = FALSE ;

          /*---------------------------------------------------------------------*/
          /*  Handle begin/end tag pairs before the start of the translatable    */
          /*  text.                                                              */
          /*                                                                     */
          /*     <text:p>--twb><text:bookmark/>text                              */
          /*     <text:p>--twb><text:bookmark-start/><text:bookmark-end/>text    */
          /*  Becomes:                                                           */
          /*     <text:p><text:bookmark/>--twb>text                              */
          /*     <text:p><text:bookmark-start/><text:bookmark-end/>--twb>text    */
          /*---------------------------------------------------------------------*/
          if ( ( ptrTwbEnd ) &&
               ( *(ptrTwbEnd+wcslen(szODC_TWB_END)) == L'<' ) ) {
             ptrChar1s = ptrTwbEnd + wcslen(szODC_TWB_END) ;
             for( i=0 ; i<NUM_REF_TAGS ; ++i ) {
                if ( ! wcsncmp( ptrChar1s+1, REF_TAGS[i].Tag, wcslen(REF_TAGS[i].Tag) ) ) {
                   break ;
                }
             }
             if ( i < NUM_REF_TAGS ) {
                wcscpy( szTag, REF_TAGS[i].Tag ) ;
                ptrChar = ptrChar1s + wcslen(szTag) + 1 ;
                bMoveTags = FALSE ;
                /*---------------------------------------------------------------*/
                /*  Handle <text:bookmark /> tag before text.                    */
                /*---------------------------------------------------------------*/
                if ( iswspace( *ptrChar ) ) {
                   ptrChar1e = wcschr( ptrChar, L'>' ) ;
                   ptrChar2 = wcsstr( ptrChar, szODC_STRING_VALUE ) ;
                   if ( ( ptrChar1e ) &&
                        ( *(ptrChar1e-1) == L'/' ) &&
                        ( ( ! ptrChar2 ) ||
                          ( ptrChar2 > ptrChar1e ) ) ) {
                      bMoveTags = TRUE ;
                   }
                } else
                /*---------------------------------------------------------------*/
                /*  Handle <text:bookmark-end /> tag before text.                */
                /*---------------------------------------------------------------*/
                if ( ! wcsncmp( ptrChar, szODC_TAG_END, wcslen(szODC_TAG_END) ) ) {
                   ptrChar1e = wcschr( ptrChar, L'>' ) ;
                   if ( ( ptrChar1e ) &&
                        ( *(ptrChar1e-1) == L'/' ) ) {
                      bMoveTags = TRUE ;
                   }
                } else
                /*---------------------------------------------------------------*/
                /*  Handle <text:bookmark-start /> tag before text.              */
                /*---------------------------------------------------------------*/
                if ( ! wcsncmp( ptrChar, szODC_TAG_START, wcslen(szODC_TAG_START) ) ) {
                   ptrChar1e = wcschr( ptrChar, L'>' ) ;
                   if ( ( ptrChar1e ) &&
                        ( *(ptrChar1e-1) == L'/' ) ) {
                      bBeginEndTags = FALSE ;
                      /*---------------------------------------------------------*/
                      /*  Handle <text:bookmark-start /><text:bookmark-end />    */
                      /*  tags before text which have the same text:name         */
                      /*  attribute values.                                      */
                      /*---------------------------------------------------------*/
                      if ( ! wcsncmp( (ptrChar1e+2), szTag, wcslen(szTag) ) ) { 
                         ptrChar2s = ptrChar1e + 1 ;
                         ptrChar = ptrChar2s + wcslen(szTag) + 1 ;
                         if ( ! wcsncmp( ptrChar, szODC_TAG_END, wcslen(szODC_TAG_END) ) ) {
                            ptrChar2e = wcschr( (ptrChar2s), L'>' ) ;
                            if ( ( ptrChar2e ) &&
                                 ( *(ptrChar2e-1) == L'/' ) ) {
                               if ( ( GetAttributeValue( ptrChar1s, L"text:name", szAttribute1 ) ) &&
                                    ( GetAttributeValue( ptrChar2s, L"text:name", szAttribute2 ) ) &&
                                    ( ! wcscmp( szAttribute1, szAttribute2 ) ) ) {
                                  bMoveTags = TRUE ;
                                  bBeginEndTags = TRUE ;
                                  ptrChar1e = ptrChar2e ;
                               } else
                               if ( ( GetAttributeValue( ptrChar1s, L"text:id", szAttribute1 ) ) &&
                                    ( GetAttributeValue( ptrChar2s, L"text:id", szAttribute2 ) ) &&
                                    ( ! wcscmp( szAttribute1, szAttribute2 ) ) ) {
                                  bMoveTags = TRUE ;
                                  bBeginEndTags = TRUE ;
                                  ptrChar1e = ptrChar2e ;
                               } 
                            }
                         }
                      }
                      /*---------------------------------------------------------*/
                      /*  Handle <text:bookmark-start /> tag before text and     */
                      /*  no other "text:bookmark" tag after that.               */
                      /*---------------------------------------------------------*/
                      if ( ( ! bBeginEndTags ) &&
                           ( ! wcsstr( ptrChar1e, szTag ) ) ) { 
                         bMoveTags = TRUE ;
                      }
                   }
                }
                if ( bMoveTags ) {
                   bTagsRemoved = TRUE ;
                   wmemmove( ptrTwbEnd, ptrChar1s, ptrChar1e-ptrChar1s+1 ) ;
                   ptrTwbEnd = ptrChar1e - wcslen(szODC_TWB_END) + 1 ;
                   wcsncpy( ptrTwbEnd, szODC_TWB_END, wcslen(szODC_TWB_END) ) ;
                }
             }
          }



          /*------------------------------------------------------------------------*/
          /*  Handle begin/end tag pairs after the end of the translatable text.    */
          /*                                                                        */
          /*     <text:p>--twb>text<text:bookmark/><twb--                           */
          /*     <text:p>--twb>text<text:bookmark-start/><text:bookmark-end/><twb-- */
          /*  Becomes:                                                              */
          /*     <text:p>--twb>text<twb--<text:bookmark/>                           */
          /*     <text:p>--twb>text<twb--<text:bookmark-start/><text:bookmark-end/> */
          /*------------------------------------------------------------------------*/
          if ( ( ptrTwbEnd   ) &&
               ( ptrTwbStart ) &&
               ( *(ptrTwbStart-1) == L'>' ) ) {
             ptrChar1e = ptrTwbStart - 1 ;
             for( ptrChar1s=ptrChar1e-1 ; 
                  ptrChar1s>ptrTwbEnd && *ptrChar1s!=L'<' ;
                  --ptrChar1s ) ;
             if ( *ptrChar1s == L'<' ) {
                for( i=0 ; i<NUM_REF_TAGS ; ++i ) {
                   if ( ! wcsncmp( ptrChar1s+1, REF_TAGS[i].Tag, wcslen(REF_TAGS[i].Tag) ) ) {
                      break ;
                   }
                }
                if ( i < NUM_REF_TAGS ) {
                   wcscpy( szTag, REF_TAGS[i].Tag ) ;
                   ptrChar = ptrChar1s + wcslen(szTag) + 1 ;
                   bMoveTags = FALSE ;
                   /*---------------------------------------------------------------*/
                   /*  Handle <text:bookmark /> tag after text.                     */
                   /*---------------------------------------------------------------*/
                   if ( iswspace( *ptrChar ) ) {
                      ptrChar2 = wcsstr( ptrChar, szODC_STRING_VALUE ) ;
                      if ( ( *(ptrChar1e-1) == L'/' ) &&
                           ( ( ! ptrChar2 ) ||
                             ( ptrChar2 > ptrChar1e ) ) ) {
                         bMoveTags = TRUE ;
                      }
                   } else
                   /*---------------------------------------------------------------*/
                   /*  Handle <text:bookmark-start /> tag after text.               */
                   /*---------------------------------------------------------------*/
                   if ( ! wcsncmp( ptrChar, szODC_TAG_START, wcslen(szODC_TAG_START) ) ) {
                      bMoveTags = TRUE ;
                   } else
                   /*---------------------------------------------------------------*/
                   /*  Handle <text:bookmark-end /> tag after text.                 */
                   /*---------------------------------------------------------------*/
                   if ( ! wcsncmp( ptrChar, szODC_TAG_END, wcslen(szODC_TAG_END) ) ) {
                      if ( *(ptrChar1e-1) == L'/' ) {

                         /*---------------------------------------------------------*/
                         /*  Handle <text:bookmark-start /><text:bookmark-end />    */
                         /*  tags after text which have the same text:name          */
                         /*  attribute values.                                      */
                         /*---------------------------------------------------------*/
                         bBeginEndTags = FALSE ; 
                         if ( *(ptrChar1s-1) == L'>') {
                            ptrChar2e = ptrChar1s - 1 ;
                            for( ptrChar2s=ptrChar2e-1 ; 
                                 ptrChar2s>ptrTwbEnd && *ptrChar2s!=L'<' ;
                                 --ptrChar2s ) ;
                            if ( ( *ptrChar2s == L'<' ) &&
                                 ( ! wcsncmp( (ptrChar1s+1), szTag, wcslen(szTag) ) ) ) { 
                               ptrChar = ptrChar2s + wcslen(szTag) + 1 ;
                               if ( ! wcsncmp( ptrChar, szODC_TAG_START, wcslen(szODC_TAG_START) ) ) {
                                  if ( *(ptrChar2e-1) == L'/' ) {
                                     if ( ( GetAttributeValue( ptrChar1s, L"text:name", szAttribute1 ) ) &&
                                          ( GetAttributeValue( ptrChar2s, L"text:name", szAttribute2 ) ) &&
                                          ( ! wcscmp( szAttribute1, szAttribute2 ) ) ) {
                                        bMoveTags = TRUE ;
                                        ptrChar1s = ptrChar2s ;
                                     } else
                                     if ( ( GetAttributeValue( ptrChar1s, L"text:id", szAttribute1 ) ) &&
                                          ( GetAttributeValue( ptrChar2s, L"text:id", szAttribute2 ) ) &&
                                          ( ! wcscmp( szAttribute1, szAttribute2 ) ) ) {
                                        bMoveTags = TRUE ;
                                        ptrChar1s = ptrChar2s ;
                                     } 
                                  }
                               }
                            }
                         } 
                         /*---------------------------------------------------------*/
                         /*  Handle <text:bookmark-end /> tag after text and        */
                         /*  no other "text:bookmark" tag before that.              */
                         /*---------------------------------------------------------*/
                         ptrChar = wcsstr( ptrTwbEnd, szTag ) ;
                         if ( ( ! bBeginEndTags ) &&
                              ( ptrChar ) &&
                              ( ptrChar > ptrChar1s ) ) { 
                            bMoveTags = TRUE ;
                         }
                      }
                   }
                   if ( bMoveTags ) {
                      bTagsRemoved = TRUE ;
                      wmemmove( ptrChar1s+wcslen(szODC_TWB_START), ptrChar1s, ptrChar1e-ptrChar1s+1 ) ;
                      ptrTwbStart = ptrChar1s ;
                      wcsncpy( ptrTwbStart, szODC_TWB_START, wcslen(szODC_TWB_START) ) ;
                   }
                }
             }
          }



          /*---------------------------------------------------------------------*/
          /*  Handle matching begin/end tags which surround the entire piece of  */
          /*  translatable text with no imbedded tags of the same type between   */
          /*  them.                                                              */
          /*                                                                     */
          /*     --twb><text:a><text:span>text</text:span></text:a><twb--        */
          /*  Becomes:                                                           */
          /*     <text:a><text:span>--twb>text<twb--</text:span></text:a>        */
          /*---------------------------------------------------------------------*/
          if ( ( ptrTwbEnd ) &&
               ( ptrTwbEnd < ptrTwbStart ) &&
               ( *(ptrTwbEnd+wcslen(szODC_TWB_END)) == L'<' ) &&
               ( *(ptrTwbStart-1)== L'>' ) ) {
             wcscpy( szSave, szIn ) ;
             usTagIndex = 0 ;
             while( ptrTwbEnd < ptrTwbStart ) {
                /*---------------------------------------------------------------*/
                /*  See if "--twb>" is followed by a tag and "<twb--" is         */
                /*  preceded by a tag.                                           */
                /*---------------------------------------------------------------*/
                ptrChar1s = ptrTwbEnd + wcslen(szODC_TWB_END) ;
                ptrChar2e = ptrTwbStart - 1 ;
                if ( ( *ptrChar1s == L'<' ) &&
                     ( *ptrChar2e == L'>' ) ) { 
                   /*------------------------------------------------------------*/
                   /*  See if the leading tag is a begin tag and if the trailing */
                   /*  tag is an end tag.                                        */
                   /*------------------------------------------------------------*/
                   ptrChar1e = wcschr( ptrChar1s, L'>' ) ;
                   for( ptrChar2s=ptrChar2e ;
                        ptrChar2s>ptrChar1s && *ptrChar2s!=L'<' ;
                        --ptrChar2s ) ;
                   if ( ( ptrChar1e ) &&
                        ( ptrChar2s > ptrChar1e ) ) {
                      if ( ( ( *(ptrChar1e-1) != L'/' ) &&   /* Not empty tags   */ 
                             ( *(ptrChar2s+1) == L'/' ) ) ||
                           ( ( *(ptrChar1e-1) == L'/' ) &&   /* Both are empty tags */
                             ( *(ptrChar2e-1) == L'/' ) ) ) {
                         wcsncpy( szTag1, ptrChar1s+1, ODC_TAG_SIZE ) ;
                         szTag1[ODC_TAG_SIZE-1] = NULL ;
                         wcstok( szTag1, L" />\n\t\r" ) ;
                         wcsncpy( szTag2, ptrChar2s+2, ODC_TAG_SIZE ) ;
                         szTag1[ODC_TAG_SIZE-1] = NULL ;
                         wcstok( szTag2, L" />\n\t\r" ) ;
                         /*---------------------------------------------------------*/
                         /*  See if the name of the leading and trailing tags are   */
                         /*  the same.                                              */
                         /*---------------------------------------------------------*/
                         if ( ( *(ptrChar1e-1) != L'/' ) &&
                              ( ! wcscmp( szTag1, szTag2 ) ) ) {
                            /*---------------------------------------------------*/
                            /*  Matched leading and trailing tag.  Make them     */
                            /*  part of the non-translatable text surrounding    */
                            /*  this translatable text.                          */
                            /*---------------------------------------------------*/
                            if ( usTagIndex < 19 ) {
                               wcscpy( szSurroundTags[usTagIndex], szTag1 ) ;
                               ptrSurround1[usTagIndex] = ptrTwbEnd ;
                               ptrSurround2[usTagIndex] = ptrTwbStart + wcslen(szODC_TWB_START) - 1 ;
                               ++usTagIndex ;
                            }
                            bTagsRemoved = TRUE ;
                            wmemmove( ptrTwbEnd, ptrChar1s, ptrChar1e-ptrChar1s+2 ) ;
                            ptrTwbEnd = ptrChar1e - wcslen(szODC_TWB_END) + 1 ;
                            wcsncpy( ptrTwbEnd, szODC_TWB_END, wcslen(szODC_TWB_END) ) ;
                            wmemmove( ptrChar2s+wcslen(szODC_TWB_START), ptrChar2s, ptrChar2e-ptrChar2s+2 ) ;
                            ptrTwbStart = ptrChar2s ;
                            wcsncpy( ptrTwbStart, szODC_TWB_START, wcslen(szODC_TWB_START) ) ;
                            continue ;

                         } else
                         /*---------------------------------------------------------*/
                         /*  See if the name of the leading and trailing tags are   */
                         /*  matching begin/end tags.  For example:                 */
                         /*      bookmark-start  and  bookmark-end                  */
                         /*---------------------------------------------------------*/
                         if ( *(ptrChar1e-1) == L'/' ) {
                            for( i=0 ; i<NUM_REF_TAGS ; ++i ) {
                               if ( ! wcsncmp( szTag1, REF_TAGS[i].Tag, wcslen(REF_TAGS[i].Tag) ) ) {
                                  break ;
                               }
                            }
                            if ( i < NUM_REF_TAGS ) {
                               wcscpy( szTag, REF_TAGS[i].Tag ) ;
                               ptrChar  = ptrChar1s + wcslen(szTag) + 1 ;
                               ptrChar2 = ptrChar2s + wcslen(szTag) + 1 ;
                               if ( ( ! wcsncmp( ptrChar,  szODC_TAG_START, wcslen(szODC_TAG_START) ) ) &&
                                    ( ! wcsncmp( ptrChar2, szODC_TAG_END,   wcslen(szODC_TAG_END)   ) ) ) {
                                  bMoveTags = FALSE ;
                                  if ( ( GetAttributeValue( ptrChar1s, L"text:name", szAttribute1 ) ) &&
                                       ( GetAttributeValue( ptrChar2s, L"text:name", szAttribute2 ) ) &&
                                       ( ! wcscmp( szAttribute1, szAttribute2 ) ) ) {
                                     bMoveTags = TRUE ;
                                  } else
                                  if ( ( GetAttributeValue( ptrChar1s, L"text:id", szAttribute1 ) ) &&
                                       ( GetAttributeValue( ptrChar2s, L"text:id", szAttribute2 ) ) &&
                                       ( ! wcscmp( szAttribute1, szAttribute2 ) ) ) {
                                     bMoveTags = TRUE ;
                                  }
                                  if ( bMoveTags ) {
                                     bTagsRemoved = TRUE ;
                                     wmemmove( ptrTwbEnd, ptrChar1s, ptrChar1e-ptrChar1s+2 ) ;
                                     ptrTwbEnd = ptrChar1e - wcslen(szODC_TWB_END) + 1 ;
                                     wcsncpy( ptrTwbEnd, szODC_TWB_END, wcslen(szODC_TWB_END) ) ;
                                     wmemmove( ptrChar2s+wcslen(szODC_TWB_START), ptrChar2s, ptrChar2e-ptrChar2s+2 ) ;
                                     ptrTwbStart = ptrChar2s ;
                                     wcsncpy( ptrTwbStart, szODC_TWB_START, wcslen(szODC_TWB_START) ) ;
                                  } 
                               }
                            }
                         }
                      }
                   }
                }
                break ;
             }


             /*---------------------------------------------------------------------*/
             /*  If one of the surrounding tags is also imbedded in the middle of   */
             /*  the translatable text, then those surrounding tags must be part of */
             /*  the translatable text.                                             */
             /*---------------------------------------------------------------------*/
             if ( usTagIndex > 0 ) {
                for( i=0 ; i<usTagIndex ; ++i ) {
                   ptrChar = wcsstr( ptrTwbEnd, szSurroundTags[i] ) ;
                   if ( ( ptrChar ) &&
                        ( ptrChar < ptrTwbStart ) &&
                        ( wcschr( L" />\n\t\r", *(ptrChar+wcslen(szSurroundTags[i])) ) ) ) {
                      bTagsRemoved = FALSE ; 
                      ptrChar1s = ptrSurround1[i] ;
                      wmemmove( ptrChar1s+wcslen(szODC_TWB_END), ptrChar1s, ptrTwbEnd-ptrChar1s ) ;
                      ptrTwbEnd = ptrChar1s ;
                      wcsncpy( ptrTwbEnd, szODC_TWB_END, wcslen(szODC_TWB_END) ) ;
                      ptrChar2e = ptrSurround2[i] ;
                      ptrChar2s = ptrTwbStart + wcslen(szODC_TWB_START) ;
                      wmemmove( ptrTwbStart, ptrChar2s, ptrChar2e-ptrChar2s+1 ) ;
                      ptrTwbStart = ptrChar2e - wcslen(szODC_TWB_START ) + 1 ;
                      wcsncpy( ptrTwbStart, szODC_TWB_START, wcslen(szODC_TWB_START) ) ;
                      break ;
                   }
                }
             }
          }



          /*---------------------------------------------------------------------*/
          /*  Handle consecutive <text:span> elements which do not change the    */
          /*  style.                                                             */
          /*                                                                     */
          /*     <text:span text:style-name="T6">text                            */
          /*     </text:span><text:span text:style-name=T6">text</text:span>     */
          /*  Becomes:                                                           */
          /*     <text:span text:style-name="T6">text text</text:span>           */
          /*---------------------------------------------------------------------*/
          if ( ( ptrTwbEnd ) &&
               ( ( ptrTwbEnd < ptrTwbStart ) ||
                 ( ! ptrTwbStart ) ) &&
               ( wcsstr( ptrTwbEnd, L"<text:span" ) ) ) {
             if ( ptrTwbStart ) 
                ptrChar3 = ptrTwbStart ;
             else
                ptrChar3 = ptrTwbEnd + wcslen(ptrTwbEnd) ;
             for( ptrChar=wcsstr(ptrTwbEnd,L"<text:span") ;    
                  ptrChar && ptrChar<ptrChar3 ;
                  ptrChar=wcsstr(ptrChar+1,L"<text:span") ) {
                ptrChar1s = ptrChar ;
                ptrChar1e = wcschr( ptrChar1s, L'>' ) ;
                if ( ptrChar1e ) {
                   for( ptrChar2s=wcschr(ptrChar1e,L'<') ;
                        ( ptrChar2s < ptrChar3 ) && 
                        ( ( ! wcsncmp( ptrChar2s, L"<text:s/>",              9 ) ) ||
                          ( ! wcsncmp( ptrChar2s, L"<text:tab/>",           11 ) ) || 
                          ( ! wcsncmp( ptrChar2s, L"<text:line-break/>",    18 ) ) || 
                        ( ! wcsncmp( ptrChar2s, L"<text:soft-page-break/>", 23 ) ) ) ;
                        ptrChar2s=wcschr(ptrChar2s+1,L'<') ) ;
                   if ( ( ptrChar2s ) &&
                        ( ptrChar2s < ptrChar3 ) &&
                        ( ! wcsncmp( ptrChar2s, L"</text:span>", 12 ) ) ) { 
                      i = ptrChar1e - ptrChar1s + 1 ;
                      ptrChar2 = ptrChar2s + 12 ; 
                      if ( ! wcsncmp( ptrChar2, ptrChar1s, i ) ) {
                         wmemmove( ptrChar2s, ptrChar2+i, wcslen(ptrChar2+i)+1 ) ;
                         if ( ptrTwbStart ) 
                            ptrTwbStart -= i + 12 ;
                         ptrChar3 -= i + 12 ;
                         --ptrChar ;
                         bTagsRemoved = TRUE ;
                      }
                   }
                }
             }
          }
       }


       fputws( szIn, fOut ) ;
    }
    fclose( fIn ) ;
    fclose( fOut ) ;


    EQFSETSLIDER(hwndSlider, 100) ;
    return(bReturn);

} /* PreParse2 */



/* Function called by EQFPOSTSEGW
*/

/****************************************************************************/
/*                                                                          */
/* PostParse                                                                */
/*                                                                          */
/* Function is called by EQFPOSTSEQW                                        */
/*                                                                          */
/* Input:      InFile        - Input file name.                             */
/*             OutFile       - Output file name.                            */
/*             hwndSlider    - Handle of status slider.                     */
/* Output:     TRUE          - Porcessing successful.                       */
/*             FALSE         - Processing failed.                           */
/****************************************************************************/
BOOL PostParse(PSZ in, PSZ out, HWND hwndSlider)
{
    BOOL     bReturn = TRUE;

    return( bReturn );

} /* PostParse */




/****************************************************************************/
/*                                                                          */
/* GetAttributeValue                                                        */
/*                                                                          */
/* Get the specific attribute value.                                        */
/*                                                                          */
/* Input:      szTag       - The entire tag to process.                     */
/*             szAttr      - The attribute keyword to find.                 */
/*             szAttrValue - The attribute's value to return.               */
/* Returns:    TRUE        - Attribute was found.                           */
/*             FALSE       - Attribute was not found.                       */
/*                                                                          */
/****************************************************************************/

BOOL GetAttributeValue( WCHAR  *szTag, WCHAR  *szAttr, WCHAR  *szAttrValue )
{
   WCHAR     szScanAttr[256] ;   
   WCHAR     AttrQuote ;
   WCHAR     *TagEnd ;
   WCHAR     *StartAttr ;
   WCHAR     *ptrAttr ;
   WCHAR     *ptr ;
   BOOL      bAttrFound = FALSE ;

   szAttrValue[0] = NULL ;
   szScanAttr[0] = L' ' ;
   wcscpy( &szScanAttr[1], szAttr ) ;
   TagEnd = wcschr( szTag, L'>' ) ;

   for( ptrAttr=wcsstr(szTag,szScanAttr ) ;       
        ptrAttr && ( !TagEnd || ptrAttr<TagEnd ) ; 
        ptrAttr=wcsstr(ptrAttr+1,szScanAttr ) ) {
      ptr = ptrAttr + wcslen(szScanAttr) ; 
      for( ; *ptr && iswspace(*ptr) ; ++ptr ) ;
      if ( *ptr != L'=' ) 
         continue;
      for( ++ptr ; *ptr && iswspace(*ptr) ; ++ptr ) ;
      if ( ( *ptr == L'\"' ) ||
           ( *ptr == L'\'' ) ) {
         AttrQuote = *ptr ;
         StartAttr = ++ptr ;
         for( ; *ptr && *ptr!=AttrQuote ; ++ptr ) {
            if ( *ptr == L'\\' ) 
               ++ptr ;
         } 
         if ( *ptr == AttrQuote ) {
            wcsncpy( szAttrValue, StartAttr, ptr-StartAttr ) ;
            szAttrValue[ptr-StartAttr] = NULL ;
            bAttrFound = TRUE ;
            break ;
         }
      }
   }
   return( bAttrFound ) ;
}




/****************************************************************************/
/*                                                                          */
/* GetRcd                                                                   */
/*                                                                          */
/* Get the next complete record from the input file.                        */
/*                                                                          */
/* Input:      szIn      - Output record.                                   */
/*             MaxLen    - Max record length.                               */
/*             fIn       - Input file.                                      */
/*             FullTag   - TRUE=Get complete tag.                           */
/*                                                                          */
/* Returns:    TRUE        - Read was successful.                           */
/*             FALSE       - Read failed.                                   */
/*                                                                          */
/****************************************************************************/

BOOL GetRcd( char *szIn, long MaxLen, FILE *fIn, BOOL FullTag ) 
{
   char      *ptrChar, *ptrChar2 ;
   long      i ;
   BOOL      bReturn = FALSE ;

   if ( fgets( szIn, MaxLen, fIn ) != NULL ) {
      bReturn = TRUE ;

      /*---------------------------------------------------------------------*/
      /*  Each line must end with a complete tag.                            */
      /*---------------------------------------------------------------------*/
      if ( FullTag ) {
         if ( strlen(szIn) == MaxLen-1 ) {
            ptrChar  = strrchr( szIn, '<' ) ;
            ptrChar2 = strrchr( szIn, '>' ) ;
            if ( ( ptrChar ) &&
                 ( ( ! ptrChar2 ) ||
                   ( ptrChar2 < ptrChar ) ) ) {
               i = strlen(szIn) ;
               while( ( szIn[i]=fgetc(fIn) ) != EOF ) {
                  if ( ( szIn[i] == '>' ) ||
                       ( i > MaxLen*2 ) ) 
                     break;
                  ++i ;
               }
               szIn[i+1] = 0 ;
            }
         } 
      }
   }
   return( bReturn ) ;
}
