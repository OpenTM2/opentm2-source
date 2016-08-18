/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/*********************************************************************
 *
 * FUNCTION:
 *
 * Segmentation routine to segment JDK 1.1 Property files
 * for Translation in TM/2
 *
 ********************************************************************/
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#include "otmjdk11.h"

void   HandleChoiceVariables( char *, BOOL ) ;
void   HandleHtmlTags( char *, BOOL ) ;

extern  short    sTPVersion ;
extern   char    szDocTargetLanguage[80];
extern   char    szDocSourceLanguage[80];
extern   BOOL    bDBCSSrc ;
extern   BOOL    bDBCSTgt ;
extern   BOOL    bDBCSInfo ;
extern   BOOL    bJapanese ;
extern   BOOL    bKorean ;
extern   BOOL    bSChinese ;
extern   BOOL    bTChinese ;
extern   BOOL    bNoDBCS ;

UCHAR       szErrMsg[256] ;
UCHAR       szErrTitle[80] ;
UCHAR       szNoSplitTag[14] = "<TWB_NOSPLIT>" ;
UCHAR       szMF_StartQuoteBlock[7] = "<TWBQ>" ;
UCHAR       szMF_EndQuoteBlock[8] = "</TWBQ>" ;
UCHAR       szMF_SingleQuote[7] = "&TWBQ;" ;
UCHAR       szMF_EscapedSingleQuote[8] = "&TWBEQ;" ;
UCHAR       szCONTEXT_START[9] = "<TWBCTX>" ;
UCHAR       szCONTEXT_END[10] = "</TWBCTX>" ;
ULONG       usSegNum ;


#define     MF_NONE      0
#define     MF_ALL       1
#define     MF_VAR       2

/*************************************************************************/
/*  Parse the JDK 1.1 property file.                                     */
/*************************************************************************/
USHORT  ParseFile(
                 PSZ    pszSource,
                 PSZ    pszTarget,
                 SHORT  sType,
                 BOOL   bCheckMessageFormat, 
                 HWND   hwndSlider )
{

    FILE     *fSource, *fTarget ;
    struct stat buf;

    UCHAR    szStringId[256] ;
    UCHAR    szTempFile[256] ;
    UCHAR    szNewLine[4]="";
    UCHAR    *szIn = NULL ;
    UCHAR    *szToken = NULL ;
    UCHAR    *szTemp = NULL ;
    UCHAR    *szSavePartialText = NULL ;
    UCHAR    *ptrToken ;
    UCHAR    *ptrChar ;

    ULONG    ulFileSize, ulFileBytes ;
    ULONG    ulSegLen ;
    USHORT   usSegmentType ;
    USHORT   usHtmlTagType ;
    SHORT    cnt2k ;    
    SHORT    i, j, k, u ;
    BOOL     bMessageStart ;
    BOOL     bFindSeparator ;
    BOOL     bLineContinue ;
    BOOL     bFileSplitMessage = FALSE ; 
    BOOL     bPartialMessage ;
    BOOL     bInNonTransSection=FALSE ;
    BOOL     bNewLine ;
    BOOL     bMaxRead ;
    BOOL     bStartNewTrans = TRUE ; 
    BOOL     bEscapeLeadBlank ;
    BOOL     bForever = TRUE ;


    SHORT    sMF_Type ;
    SHORT    sMF_SkipNextMessage ;
    BOOL     bMF_StringVars = FALSE ; 
    BOOL     bMF_AddComment = FALSE ; 
    BOOL     bMF_InBlock ; 



    fSource = fopen ( pszSource, "rb" ) ;     /* Open input source file.       */
    fTarget = fopen ( pszTarget, "wb+" ) ;    /* Open output segmented file.   */

    szToken = malloc( MAX_RCD_LENGTH ) ;
    szIn = malloc( MAX_RCD_LENGTH ) ;
    szTemp = malloc( MAX_RCD_LENGTH ) ;

    if ( ( ! fSource ) ||
         ( ! fTarget ) ||
         ( ! szIn    ) ||
         ( ! szToken ) ||
         ( ! szTemp  ) ) {
       if ( fSource ) fclose(fSource) ;
       if ( fTarget ) fclose(fTarget) ;
       if ( szIn ) free(szIn) ;
       if ( szToken ) free(szToken) ;
       if ( szTemp ) free(szTemp) ;
       return 0 ; 
    }


    if ( bCheckMessageFormat ) {              /* Managing single quotes?      */
       sMF_Type = MF_VAR ;                    /* Assume modify squotes        */
    } else {
       sMF_Type = MF_NONE ;                   /* Assume leave squotes alone   */
    }
    bMF_AddComment = TRUE ;                   /* Add if not found at beginning*/
    sMF_SkipNextMessage = 0 ;

    /*------------------------------------------------------------------------*/
    /*  Get number of characters in entire file.  Used to determine percent   */
    /*  completed for updating slider information.                            */
    /*------------------------------------------------------------------------*/

    stat(pszSource, &buf);
    ulFileSize = buf.st_size;
    ulFileBytes = 0 ;

    if ( hwndSlider)
        EQFSETSLIDER( hwndSlider, 0 ) ;       /* Initialize slider to 0.      */

    usSegmentType = SEGMENT_NONE ;           /* Default undefined segment.    */
    usSegNum = 0 ;                           /* 1st segment number will be 1. */


    /*------------------------------------------------------------------------*/
    /*  Process all records within this file.                                 */
    /*------------------------------------------------------------------------*/
    while ( (INT)fgets( szIn, MAX_RCD_LENGTH, fSource ) > 0 ) {
        if ( strlen(szIn) == MAX_RCD_LENGTH-1 ) {                 
           bMaxRead = TRUE ; 
        } else {
           bMaxRead = FALSE ; 
           if ( ! szNewLine[0] ) {
              strcpy( szTemp, szIn ) ;
              StripNewline( szTemp, szNewLine ) ;
           }
        }
        i = 0 ;
        ulFileBytes = ulFileBytes + strlen(szIn) ;

        /* Each record starts non-trans. */
        StartJDKSegment( fTarget, SEGMENT_NONTRANS, &usSegmentType ) ;


        if ( bMF_StringVars ) {               /* If text contains variables,  */
           bMF_StringVars = FALSE ;           /*  then mark end of squote text*/
           fprintf( fTarget, szMF_EndQuoteBlock ) ;
        }

        if ( ( usSegNum == 1 ) &&                          /* Strip UTF-8 BOM  */
             ( ! strncmp( szIn, "\xEF\xBB\xBF", 3 ) ) ) {         
           memmove( szIn, &szIn[3], strlen(&szIn[3])+1 ) ;
        }


        /*--------------------------------------------------------------------*/
        /*  Find first non-blank character in this line.                      */
        /*--------------------------------------------------------------------*/
        j = strlen( szIn ) ;
        for ( i=0; isspace( szIn[i] ) && ( i < j ) ; ++i ) {
           if ( i == 2000 ) {                    /* Break 2K segments */
              strcpy( szTemp, &szIn[i+1] ) ;
              szIn[i+1] = '\0' ; 
              fputs( szIn, fTarget ) ;
              strcpy( szIn, szTemp ) ;
              i = -1 ;
              j = strlen( szIn ) ;
              StartJDKSegment( fTarget, SEGMENT_NONTRANS, &usSegmentType ) ;
           }
        }
        if ( i >= j ) {                       /* If blank record,             */
            fputs( szIn, fTarget ) ;          /*   write it out and continue  */
            continue ;
        }

        /*--------------------------------------------------------------------*/
        /*  If comment line, then skip it.                                    */
        /*--------------------------------------------------------------------*/
        if ( ( szIn[i] == '#' ) ||
             ( szIn[i] == '!' ) ||
             ( szIn[i] == ':' ) ||                                
             ( szIn[i] == ';' ) ||                                
             ( ( szIn[i]   == '/' ) &&                            
               ( szIn[i+1] == '/' ) ) ) {
            for( ptrChar=szIn ; *ptrChar ; ++ptrChar ) {
               if ( ( *ptrChar == '\\'     ) &&                   
                    ( ( ( *(ptrChar+1) == '\n' ) &&
                        ( *(ptrChar+2) == 0    ) ) ||
                      ( ( *(ptrChar+1) == '\r' ) &&               
                        ( *(ptrChar+2) == '\n' ) &&
                        ( *(ptrChar+3) == 0    ) ) ) ) {
                  memmove( ptrChar+2, ptrChar+1, strlen(ptrChar+1)+1 ) ;
                  *(ptrChar+1) = ' ' ;
                  break ;
               }
               u = GetCharLength( ptrChar ) ;                       
               if ( u > 1 ) {                              /* Multi-byte char */
                  ptrChar += u - 1 ;
               }
            }
            while( strlen(szIn) > 2000 ) {                         
               strcpy( szTemp, &szIn[1990] ) ;
               szIn[1990] = 0 ;
               fputs( szIn, fTarget ) ;          
               StartJDKSegment( fTarget, SEGMENT_NONTRANS, &usSegmentType ) ;
               strcpy( szIn,szTemp ) ;
            }
            fputs( szIn, fTarget ) ;          /* Write out comment record.    */
            strupr( szIn ) ;

            /*----------------------------------------------------------------*/
            /*  If special comment which defines a block of source which      */
            /*  should not be translated, then set up processing.             */
            /*----------------------------------------------------------------*/
            if ( strstr( szIn, "NON-TRANSLATABLE" ) ) { 
               if ( strstr( szIn, "START" ) ) 
                  bInNonTransSection = TRUE ;
               if ( strstr( szIn, "END" ) ) 
                  bInNonTransSection = FALSE ;
            }

            /*----------------------------------------------------------------*/
            /*  If special comment which defines how single quotes should be  */
            /*  handled because of MessageFormat processing, then set up      */
            /*  processing.                                                   */
            /*----------------------------------------------------------------*/
            if ( strstr( szIn, "NLS_MESSAGEFORMAT_" ) ) {
               if ( strstr( szIn, "NLS_MESSAGEFORMAT_ALL" ) ) {
                  sMF_Type = MF_ALL ;         /* All quotes are doubled       */
                  bMF_AddComment = FALSE ; 
               } else {
                  if ( strstr( szIn, "NLS_MESSAGEFORMAT_NONE" ) ) {
                     sMF_Type = MF_NONE ;     /* No special processing        */  
                     bMF_AddComment = FALSE ; 
                  } else {
                     if ( strstr( szIn, "NLS_MESSAGEFORMAT_VAR" ) ) {
                        sMF_Type = MF_VAR ;   /* Based on replacement vars.   */
                        bMF_AddComment = FALSE ; 
                     } else 
                        if ( strstr( szIn, "NLS_MESSAGEFORMAT_SKIP" ) ) {
                           sMF_SkipNextMessage = 1 ;
                        } 
                  }
               }
            }

            continue ;                        /* Continue with next record.   */
        }

        /*--------------------------------------------------------------------*/
        /*  If in non-translatable section, then ignore this line.            */
        /*--------------------------------------------------------------------*/
        if ( bInNonTransSection ) {
           while( strlen(szIn) > 2000 ) {                    
              strcpy( szTemp, &szIn[1990] ) ;
              szIn[1990] = 0 ;
              fputs( szIn, fTarget ) ;          
              StartJDKSegment( fTarget, SEGMENT_NONTRANS, &usSegmentType ) ;
              strcpy( szIn,szTemp ) ;
           }
           fputs( szIn, fTarget ) ;          /* Write out non-trans record.   */
           continue ;                        /* Continue with next record.    */
        }

        strcpy( szToken, szIn ) ;                            
        ptrToken = &szToken[i] ;
        for( ptrChar=ptrToken ;
             *ptrChar && !strchr("\n\r\t=: ", *ptrChar) ;    
             ++ptrChar ) {
           if ( *ptrChar == '\\' )
              ++ptrChar ;
        }
        *ptrChar = 0 ;
        if ( ptrToken == ptrChar ) {
           fputs( szIn, fTarget ) ;           /* Write out empty record.      */
           continue ;                         /* Continue with next record.   */
        }

        /*--------------------------------------------------------------------*/
        /*  Found a new message identifier (string not in comment nor part of */
        /*                                  the existing message text).       */
        /*--------------------------------------------------------------------*/
        strncpy( szStringId, ptrToken, sizeof(szStringId) ) ;
        szStringId[sizeof(szStringId)-1] = 0 ;

        i = ptrToken - szToken + strlen( ptrToken ) - 1 ;
        bMessageStart  = FALSE ;              /* Start of msg text not found  */
        bFindSeparator = TRUE ;               /* Find message separator       */
        bLineContinue  = FALSE ;              /* Text not continue across line*/
        bNewLine = FALSE ;                    /* No newline char found in text*/

        if ( bMF_AddComment ) {               /* If no definition comment yet,*/
           bMF_AddComment = FALSE ;           /*  then add default comment    */
           if ( szNewLine[0] ) {              /* No comment if only 1 line    */
              if ( bCheckMessageFormat ) 
                 fputs( "# NLS_MESSAGEFORMAT_VAR", fTarget ) ;   /* Default= VAR */
              else
                 fputs( "# NLS_MESSAGEFORMAT_NONE", fTarget ) ;   /* Default=NONE*/
              if ( ! szNewLine[0] )                          
                 strcpy( szNewLine, "\n" ) ;
              fputs( szNewLine, fTarget ) ; 
           }
        }
        if ( sMF_SkipNextMessage == 1 ) 
           sMF_SkipNextMessage = 2 ;
        else
           sMF_SkipNextMessage = 0 ;

        /*--------------------------------------------------------------------*/
        /*  Look for the message text associated with this message ID.        */
        /*--------------------------------------------------------------------*/
        bPartialMessage = FALSE ;
        cnt2k = 0 ;
        while ( bForever ) {
            ++i ;
            ++cnt2k ;

            /*----------------------------------------------------------------*/
            /*  If at the end of this record, then message is done.           */
            /*     Message text must start on same line as message ID.        */
            /*----------------------------------------------------------------*/
            if ( ! szIn[i] ) {

               /*-------------------------------------------------------------*/
               /*  If there are trailing blanks at the end of the message text*/
               /*  then add a special tag so that the blanks are preserved by */
               /*  TM during translation.                                     */
               /*-------------------------------------------------------------*/
                if ( ( ! bPartialMessage ) &&
                     ( strlen(szIn) >= 2 ) &&
                     ( ( ( szIn[strlen(szIn)-1] == '\n'  ) &&
                         ( szIn[strlen(szIn)-2] != '\r'  ) &&
                         ( isspace(szIn[strlen(szIn)-2]) ) ) ||
                       ( ( szIn[strlen(szIn)-1] == '\n'  ) &&    
                         ( szIn[strlen(szIn)-2] == '\r'  ) &&
                         ( isspace(szIn[strlen(szIn)-3]) ) ) ) ) {
                    if ( szIn[strlen(szIn)-2] == '\r' ) {
                       strcpy( szTemp, &szIn[strlen(szIn)-2] ) ;
                       szIn[strlen(szIn)-2] = 0 ;
                    } else {
                       strcpy( szTemp, &szIn[strlen(szIn)-1] ) ;
                       szIn[strlen(szIn)-1] = 0 ;
                    }
                    fputs( szIn, fTarget ) ;   /* Write out current text      */
                    StartJDKSegment( fTarget, SEGMENT_NONTRANS, &usSegmentType ) ;
                    fputs( "<TWBEOL>", fTarget ) ; /*Write special tag for EOL*/
                    fputs( szTemp, fTarget ) ; 
                } else {
                   fputs( szIn, fTarget ) ;    /* Write out current text      */
                }
                if ( ! bPartialMessage ) 
                   break;                      /* Reset for next msg ID       */
                bMessageStart = FALSE ;
            }

            /*----------------------------------------------------------------*/
            /*  Found first character of message text.                        */
            /*----------------------------------------------------------------*/
            if ( ! bMessageStart ) {           /* If no message text found yet*/
                if ( isspace( szIn[i] ) )      /* If blank, then continue.    */
                    continue;
                if ( bFindSeparator ) {         /* Still looking for separator*/
                    if ( ( szIn[i] == '=' ) ||  /* If found "=", look for text*/
                         ( szIn[i] == ':' ) ) { /* If found ":", look for text*/
                        bFindSeparator = FALSE ;

                        /* Insert <TWBCTX> and </TWBCTX> to encapsulate messageID as the context info */
                        /* Look for the start of message ID, after ":eqfn.:qfn n=xx." in szIn ":eqfn.:qfn n=8.InstallUA =" */
                        fprintf( fTarget, szCONTEXT_START ) ;
                        
                        for( j=i-1; szIn[j] && isspace(szIn[j]) ; --j );
                        j++;
		                strcpy( szTemp, szIn ) ;
		                szTemp[j] = '\0' ;
		                fputs( szTemp, fTarget ) ;  /* Write out data before msg text*/
		                fprintf( fTarget, szCONTEXT_END ) ;
		                strcpy( szTemp, &szIn[j] ) ;
		                strcpy(szIn, szTemp);
		                i = i-j;
                        continue ;
                    }
                }
                bMessageStart = TRUE ;         /* Text for message is found.  */
                if ( bPartialMessage ) {         
                   strcpy( szTemp, szSavePartialText ) ;
                   szSavePartialText[0] = 0 ;
                   if ( strlen(szTemp) == MAX_RCD_LENGTH-1 ) 
                      bMaxRead = TRUE ; 
                   else
                      bMaxRead = FALSE ; 
                   bPartialMessage = FALSE ;
                   bStartNewTrans = FALSE ;
                } else {
                   j = i ;
                   strcpy( szTemp, szIn ) ;
                   szTemp[j] = '\0' ;
                   fputs( szTemp, fTarget ) ;  /* Write out data before msg text*/
                   strcpy( szTemp, &szIn[j] ) ;

                   cnt2k = 0 ;                 /* 11-30-06 */
                   ulSegLen = 0 ;
     
                   if ( hwndSlider)
                       EQFSETSLIDER( hwndSlider, (USHORT)((ulFileBytes * 100)/ulFileSize) ) ;
                }

                /*------------------------------------------------------------*/
                /*  Accumulate all text for this message into a single string.*/
                /*  If text contains any line continuation characters, remove */
                /*  them.  Keep track if find any newline or line-continuation*/
                /*  characters for later segmentation.                        */
                /*------------------------------------------------------------*/
                szIn[0] = '\0' ;             /* Start with null message text. */
                i = -1 ;
                while ( bForever ) {         /* Accumulate text for this msg. */
                    ++i ;
                    if ( ! szTemp[i] ) {     /* If end of input string,       */
                        if ( ! szIn[0] ) {                   
                           strcpy( szIn, szTemp ) ;  
                           if ( bMaxRead ) {    /* If partial record read     */
                              fgets( szTemp, MAX_RCD_LENGTH, fSource ) ;
                              if ( strlen(szTemp) == MAX_RCD_LENGTH-1 ) 
                                 bMaxRead = TRUE ; 
                              else
                                 bMaxRead = FALSE ; 
                              i = -1 ;
                              continue ;
                           } 
                           break ;               /* End of message text found.*/
                        }
                        if ( strlen(szIn)+strlen(szTemp) >= MAX_RCD_LENGTH-256 ) { 
                           if ( ! szSavePartialText )
                              szSavePartialText = malloc( MAX_RCD_LENGTH ) ;
                           strcpy( szSavePartialText, szTemp ) ;
                           bPartialMessage = TRUE ; 
                           break;
                        }
                        strcat( szIn, szTemp ) ;  /*   then end of message text    */
                        break ;                   /*   was found.  End message ID. */
                    }

                    /*--------------------------------------------------------*/
                    /*  If DBCS character, skip next character.               */
                    /*--------------------------------------------------------*/
                    u = GetCharLength( &szTemp[i] ) ;                  
                    if ( u > 1 ) {                             /* Multi-byte char */
                       i += u - 1 ;
                       continue;
                    }

                    /*------------------------------------------------------------*/
                    /*  If message text line continuation character, then remove  */
                    /*  it and get next record to concatenate its text with       */
                    /*  current message text.                                     */
                    /*------------------------------------------------------------*/
                    if ( szTemp[i] == '\\' ) {   /* Possible line continuation    */
                        if ( szTemp[i+1] == '\\' ) {
                            ++i ;                /* If not end-of-line backslash  */
                            continue ;           /*   then actual part of msg text*/
                        }
                        /*---------------------------------------------------------*/
                        /*  If rest of line is blank, then valid continuation char.*/
                        /*---------------------------------------------------------*/
                        for ( j=i+1;
                            isspace(szTemp[j]) && szTemp[j]!=' ' && ( j<strlen(szTemp) ) ; 
                            ++j ) ;                                    
                        if ( j >= strlen(szTemp) ) { /* Rest of line is blank      */
                            bLineContinue = TRUE ;    /* Remember line continuation */
                            bFileSplitMessage = TRUE ; /* OK to split message text  */
                            szTemp[i] = '\0' ;
                            if ( strlen(szIn)+strlen(szTemp) >= MAX_RCD_LENGTH-256 ) {
                               if ( ! szSavePartialText )
                                  szSavePartialText = malloc( MAX_RCD_LENGTH ) ;
                               strcpy( szSavePartialText, szTemp ) ;
                               strcat( szSavePartialText, "\\" ) ;    /* Reset line continuation */
                               bPartialMessage = TRUE ; 
                               break;
                            }

                            strcat( szIn, szTemp ) ;  /* Concatenate message text   */
                                                      /* Get next input record.     */

                            if ( (INT)fgets( szToken, MAX_RCD_LENGTH, fSource ) == 0 )
                                break ;
                            if ( strlen(szToken) == MAX_RCD_LENGTH-1 ) 
                               bMaxRead = TRUE ; 
                            else
                               bMaxRead = FALSE ; 
                            for ( i=0; szToken[i] && isspace( szToken[i] ) ; ++i ) ;
                            if ( szToken[i] ) {
                               strcpy( szTemp, &szToken[i] ) ;  
                            } else {
                               strcpy( szTemp, szToken ) ;      
                            }
                            i = -1 ;
                            ulFileBytes = ulFileBytes + strlen(szToken) ;
                            continue ;
                        }
                        if ( szTemp[i+1] == 'n' ) /* If newline character,    */
                            bNewLine = TRUE ;      /*   Remember this fact.   */
                    }
                }                              /* Message text is accumulated */
                i = 0 ;


                /*----------------------------------------------------------------*/
                /*  Handle single quotes if MessageFormat is used to format this  */
                /*  message text.  Special comments can be supplied to override   */
                /*  this processing:                                              */
                /*     # NLS_MESSAGEFORMAT_NONE                                   */
                /*     # NLS_MESSAGEFORMAT_ALL                                    */
                /*     # NLS_MESSAGEFORMAT_VAR                                    */
                /*                                                                */
                /*  NONE:  No special single quote processing.                    */
                /*  ALL:   All message text is handled by MessageFormat API.      */
                /*         All double single quotes are changed to 1 single quote.*/
                /*  VAR:   Only text which contains replacement variables is      */
                /*         handled by MessageFormat API ('' changed to ').        */
                /*----------------------------------------------------------------*/
                bMF_StringVars = FALSE ;            /* Assume no replacement vars */
                if ( ( sMF_Type != MF_NONE ) &&     /* If special processing      */
                     ( ! sMF_SkipNextMessage ) ) {
                   if ( sMF_Type == MF_ALL ) {
                      bMF_StringVars = TRUE ;       /* Force special processing   */
                   } else {
                      if ( TextContainsVariable( szIn ) ) /* Contains variables   */
                         bMF_StringVars = TRUE ; 
                   }
                   if ( bMF_StringVars ) {          /* Normalize single quotes    */
                      HandleSingleQuotes( szIn, TRUE, &bMF_InBlock ) ;
                      fprintf( fTarget, szMF_StartQuoteBlock ) ;
                   }
                }


                /* Segment change to translatable*/
                if ( bStartNewTrans ) 
                   StartJDKSegment( fTarget, SEGMENT_TRANS, &usSegmentType ) ;
                bStartNewTrans = TRUE ;

                for( j=strlen(szIn)-1 ; j>=0 && isspace(szIn[j]) ; --j ) ; 
                if ( ( ( j == 0 ) &&                      /* Single letter    */
                       ( isalpha( szIn[0] ) ) ) ||
                     ( ( j == 3 ) &&                      /* Accel key VK_a   */
                       ( ! strncmp( szIn, "VK_", 3 ) ) &&
                       ( isalpha( szIn[3] ) ) ) ) {
                   fprintf( fTarget, "<TWB %s>",szStringId ) ;
                }

                /* Modify choice variables to identify trans. text.           */
                if ( ( strchr( szIn, '{' )      ) &&
                     ( strstr( szIn, "choice" ) ) ) {
                   HandleChoiceVariables( szIn, TRUE ) ;
                }

                /* Identify HTML tags versus text after less than.            */
                if ( ( strchr( szIn, '<' ) ) &&
                     ( strchr( szIn, '>' ) ) ) {
                   HandleHtmlTags( szIn, TRUE ) ;
                }
            }

            /*----------------------------------------------------------------*/
            /*  If end of sentence is found (period, question, exclamation)   */
            /*  and next character is blank (blank, newline, tab), then end   */
            /*  the previous segment and start a new one.                     */
            /*----------------------------------------------------------------*/
            usHtmlTagType = 0 ;
            if ( ( ( ( szIn[i] == '.' ) ||
                     ( szIn[i] == '?' ) ||
                     ( szIn[i] == '!' ) ) &&
                   ( ( isspace( szIn[i+1] ) ) ||
                     ( ! strncmp( &szIn[i+1], "\\n", 2 ) ) ||
                     ( ! strncmp( &szIn[i+1], "\\t", 2 ) ) ||
                     ( IsHTMLTag( &szIn[i+1], &usHtmlTagType ) ) ) ) ||
                 ( IsDBCSSentenceEnd( szIn, i ) ) ) {
                if ( ( usHtmlTagType == 1 ) &&                   
                     ( szIn[i+2] == '/'   ) ) {
                   for ( j=i+3; isalpha(szIn[j]) && j<strlen(szIn) ; ++j ) ;
                   if ( szIn[j] == '>' )
                      i = j ;
                }
                /* Find next non-blank character */
                for ( j=i+1; isspace( szIn[j] ) && ( j<strlen(szIn) ) ; ++j ) ;
                /* Find next logical non-blank   */
                for ( k=j; k<strlen(szIn) &&
                    ( ( isspace( szIn[k] ) ) ||
                      ( !strncmp( &szIn[k], "\\n", 2 ) ) ||
                      ( !strncmp( &szIn[k], "\\t", 2 ) ) ) ; ++k ) {
                    if ( ! isspace( szIn[k] ) )
                        ++k ;                /* Skip 2nd part: tab or newline */
                }
                ulSegLen = 0 ;

                /*------------------------------------------------------------*/
                /*  If there is more message text after this sentence, then   */
                /*  start a new segment after this sentence.                  */
                /*------------------------------------------------------------*/
                if ( ( j < strlen(szIn) ) &&
                     ( k < strlen(szIn) ) ) {
                    if ( j > i+3 )           /* if >2 trailing blanks, include*/
                        j = i + 2 ;          /*  blanks with next sentence.   */
                    k = j ;
                    if ( !strncmp( &szIn[j], "\\n", 2 ) )  /* If newline char,*/
                        k = k + 2 ;          /* Include with this sentence.   */
                    strcpy( szTemp, szIn ) ;
                    szTemp[k] = '\0' ;
                    fputs( szTemp, fTarget ) ;   /* Write out this sentence.  */

                    /*--------------------------------------------------------*/
                    /*  If no newline chars in message text,                  */
                    /*     then add non-translatable continuation and newline,*/
                    /*     and start sentence on next line.                   */
                    /*  If sentence ends with newline character,              */
                    /*     then newline and start sentence on next line.      */
                    /*  Otherwise, message text will exist on a single line.  */
                    /*--------------------------------------------------------*/
                    bEscapeLeadBlank = FALSE ;
                    if ( bLineContinue && !bNewLine ) {
                        StartJDKSegment( fTarget, SEGMENT_NONTRANS, &usSegmentType ) ;
//                      fputs( "\\\n", fTarget ) ;
                        fprintf( fTarget, "\\%s", szNewLine ) ;
                        if ( szIn[k] == ' ' )                   
                           bEscapeLeadBlank = TRUE ;
                    } else {
                       if ( !strncmp( &szIn[j], "\\n", 2 ) ) {
//                        fputs( "\n", fTarget ) ;
                          fputs( szNewLine, fTarget ) ;
                          if ( szIn[k] == ' ' )                 
                             bEscapeLeadBlank = TRUE ;
                       }
                    }
                    /* Segment end and start trans.  */
                    StartJDKSegment( fTarget, SEGMENT_TRANS, &usSegmentType ) ;
                    if ( bEscapeLeadBlank )                     
                       fputs( "\\", fTarget ) ;
                    strcpy( szTemp, &szIn[k] ) ;
                    strcpy( szIn, szTemp ) ;
                    i = -1 ;
                    cnt2k = 0 ;
                    continue ;
                }
            }

            /*----------------------------------------------------------------*/
            /*  If control character (backslash followed by something).       */
            /*----------------------------------------------------------------*/
            if ( szIn[i] == '\\' ) {
                /*------------------------------------------------------------*/
                /*  If double backslash, skip chars.                          */
                /*------------------------------------------------------------*/
                if ( szIn[i+1] == '\\' ) {
                    ++i ;
                    continue ;
                }
                /*------------------------------------------------------------*/
                /*  If newline and there is more message text for this ID,    */
                /*  then, start a new output line.                            */
                /*------------------------------------------------------------*/
                if ( szIn[i+1] == 'n' ) {
                    for ( j=i+2; isspace( szIn[j] ) && ( j<strlen(szIn) ) ; ++j ) ;
                    if ( j < strlen(szIn) ) {
                        strcpy( szTemp, szIn ) ;
                        szTemp[i+2] = '\0' ; /* Include \n with prev. sentence*/
                        ulSegLen = ulSegLen + strlen(szTemp) ;
                        fputs( szTemp, fTarget ) ;
//                      fputs( "\n", fTarget ) ;  /* Start a new output line.      */
                        fputs( szNewLine, fTarget ) ;  /* Start a new output line. */
                        strcpy( szTemp, &szIn[i+2] ) ; /* Save rest of message text*/
                        strcpy( szIn, szTemp ) ;
                        if ( szIn[0] == ' ' ) 
                           fputs( "\\", fTarget ) ;             
                        i = -1 ;
                        cnt2k = 0 ;
                        if ( ulSegLen > 1000 ) {
                            ulSegLen = 0 ;
                            StartJDKSegment( fTarget, SEGMENT_TRANS, &usSegmentType ) ;
                        }
                        continue ;
                    }
                }
            }

            /*----------------------------------------------------------------*/
            /*  If exceeding 2k segment limit, then force a new segment.      */
            /*----------------------------------------------------------------*/
            if ( ( ( cnt2k > 1600     ) &&                        
                   ( isspace(szIn[i]) ) ) ||
                 ( cnt2k > 2000 ) ) {       /* Force break if no blank        */
               strcpy( szTemp, szIn ) ;
               szTemp[i+1] = '\0' ; 
               fputs( szTemp, fTarget ) ;
               memmove( szIn, &szIn[i+1], strlen(&szIn[i+1])+1 ) ; /* Save rest of message text*/
               i = -1 ;
               cnt2k = 0 ;
               ulSegLen = 0 ;
               StartJDKSegment( fTarget, SEGMENT_TRANS, &usSegmentType ) ;
               continue ;
            }
        }
    }

    if ( bMF_StringVars ) {               /* If text contains variables,      */
       bMF_StringVars = FALSE ;           /*  then mark end of squote text    */
       StartJDKSegment( fTarget, SEGMENT_NONTRANS, &usSegmentType ) ;
       fprintf( fTarget, szMF_EndQuoteBlock ) ;
    }


    EndJDKSegment( fTarget, usSegmentType ) ; /* End final segment.           */

    fclose ( fSource ) ;                     /* Close input source file.      */
    fclose ( fTarget ) ;                     /* Close segmented target file   */

    /*------------------------------------------------------------------------*/
    /*  If not supposed to split lines in this file, then prepare for         */
    /*  later processing.                                                     */
    /*------------------------------------------------------------------------*/
    if ( ! bFileSplitMessage ) {                                  
       strcpy( szTempFile, pszTarget) ;
       strcat( szTempFile, "$" ) ;
       fSource = fopen ( pszTarget, "rb" ) ;   
       fTarget = fopen ( szTempFile, "wb+" ) ; 
       bMessageStart = TRUE ; 
       while ( (INT)fgets( szIn, MAX_RCD_LENGTH, fSource ) > 0 ) {
          if ( bMessageStart ) {
             bMessageStart = FALSE ;
             ptrChar = strchr( szIn, '.' ) ;
             if ( ptrChar ) {
                ++ptrChar ;
                memmove( ptrChar+strlen(szNoSplitTag), ptrChar, strlen(ptrChar)+1 ) ;
                strncpy( ptrChar, szNoSplitTag, strlen(szNoSplitTag) ) ;
             }
          }
          fputs( szIn, fTarget ) ;           
       }
       fclose ( fSource ) ;  
       fclose ( fTarget ) ;  
       CopyFile( szTempFile, pszTarget, FALSE ) ;
       remove( szTempFile ) ;
    }

    if ( hwndSlider)
        EQFSETSLIDER( hwndSlider, 100 ) ;

    if ( szIn )
        free( szIn ) ;
    if ( szTemp )
        free( szTemp ) ;
    if ( szToken )
        free( szToken ) ;
    if ( szSavePartialText ) 
        free( szSavePartialText ) ;

    return 1 ;
}



/*************************************************************************/
/*  Export the JDK 1.1 property file.                                    */
/*************************************************************************/
USHORT  ExportFile(
                  PSZ    pszSource,
                  PSZ    pszTarget  )
{

    FILE     *fSource, *fTarget ;

    UCHAR    *szIn = NULL ;
    UCHAR    *szNext = NULL ;
    UCHAR    *szTemp = NULL ;
    UCHAR    *ptrPeriod, *ptrJoin, *ptrNext ;
    UCHAR    *ptrChar, *ptrChar2 ;
    UCHAR    szTag[40] ;
    UCHAR    szLastQfTag[4] ;
    UCHAR    szNewLine[4]="";

    LONG     lCurrentPos ;
    BOOL     bMF_InBlock = FALSE ; 
    SHORT    i, j ;
    USHORT   usSegmentType ;
    USHORT   usSegmentLength ;
    USHORT   usLastSeqNum ;
    BOOL     bTextContinued ;
    BOOL     bJoinLast = FALSE ;
    BOOL     bInNonTransSection=FALSE ;
    BOOL     bTrailingBackslash=FALSE ;
    BOOL     bHandleChoiceVariables=FALSE ;
    BOOL     bCommentLine ;
    BOOL     bPartialRecord ;
    BOOL     bPartialRecordNext ;
    BOOL     bLoop = FALSE ;


    fSource = fopen ( pszSource, "rb" ) ;     /* Open input segmented file.    */
    if ( ! fSource ) return 0 ;
    fTarget = fopen ( pszTarget, "wb+" ) ;    /* Open output target file.      */
    if ( ! fTarget ) return 0 ;

    szIn = malloc( MAX_RCD_LENGTH +256 ) ;   
    if ( ! szIn ) return 0 ;
    szNext = malloc( MAX_RCD_LENGTH +256) ;
    if ( ! szNext ) return 0 ;
    szTemp = malloc( MAX_RCD_LENGTH +256) ;
    if ( ! szTemp ) return 0 ;

    usSegmentType = SEGMENT_NONTRANS ;       /* Start as undefined segment.   */

    fgets( szIn, MAX_RCD_LENGTH, fSource ) ; /* Get 1st record for look-ahead.*/
    if ( strlen(szIn) == MAX_RCD_LENGTH-1 ) 
       bPartialRecord = TRUE ;
    else
       bPartialRecord = FALSE ;
    ptrChar = strstr( szIn, "<TWB " ) ;     /* Remove special TWB tags        */
    if ( ptrChar ) {
       ptrChar2 = strchr( ptrChar, '>' ) ;
       if ( ptrChar2 )
          memmove( ptrChar, ptrChar2+1, strlen(ptrChar2+1)+1 ) ;
    }
    szNext[0] = 0 ;                                             

    strcpy( szTemp, szIn ) ;
    StripNewline( szTemp, szNewLine ) ;

    /* Modify choice variables to identify translatable text.  */
    if ( ( strchr( szIn, '{' )      ) &&
         ( strstr( szIn, "choice" ) ) ) {
       bHandleChoiceVariables = TRUE ;                        
       HandleChoiceVariables( szIn, FALSE ) ;
    }

    /* Identify HTML tags versus text after less than.  */
    if ( ( strchr( szIn, '<' ) ) &&
         ( strchr( szIn, '>' ) ) ) {
       HandleHtmlTags( szIn, FALSE ) ;
    }


    /*------------------------------------------------------------------------*/
    /*  Read all records within this file.                                    */
    /*------------------------------------------------------------------------*/
    while ( (INT)fgets( szNext, MAX_RCD_LENGTH, fSource ) > 0 ) {
       if ( strlen(szNext) == MAX_RCD_LENGTH-1 ) 
          bPartialRecordNext = TRUE ;
       else
          bPartialRecordNext = FALSE ;

        /*---------------------------------------------------------------------*/
        /*  Remove special <TWB xxx> tag which was added to make single letter */
        /*  text and accelerator key values unique within TM.                  */
        /*---------------------------------------------------------------------*/
        ptrChar = strstr( szIn, "<TWB " ) ;
        if ( ptrChar ) {
           ptrChar2 = strchr( ptrChar, '>' ) ;
           if ( ptrChar2 )
              memmove( ptrChar, ptrChar2+1, strlen(ptrChar2+1)+1 ) ;
        }

        /*---------------------------------------------------------------------*/
        /*  Remove special <TWBEOL> tag which was added to preserve trailing   */
        /*  blanks.                                                            */
        /*---------------------------------------------------------------------*/
        ptrChar = strstr( szIn, "<TWBEOL>" ) ;
        if ( ptrChar ) {
           memmove( ptrChar, ptrChar+8, strlen(ptrChar+8)+1 ) ;
        }


        /*---------------------------------------------------------------------*/
        /*  If special comment which defines a block of source which           */
        /*  should not be translated, then set up processing.                  */
        /*---------------------------------------------------------------------*/
        bCommentLine = FALSE ;
        if ( ( ( ! strncmp( szIn, ":eqf", 4 ) ) ||
               ( ! strncmp( szIn, ":EQF", 4 ) ) ) &&
             ( ( ! strncmp( &szIn[5], ".:qfn", 5 ) ) ||
               ( ! strncmp( &szIn[5], ".:QFN", 5 ) ) ) ) {
           ptrChar = strchr( &szIn[9], '.' ) ;
           if ( ptrChar ) {  
              for ( ++ptrChar ; *ptrChar && isspace(*ptrChar) ; ++ptrChar ) ;
              if ( ( *ptrChar == '#' ) ||
                   ( *ptrChar == '!' ) ||
                   ( *ptrChar == ':' ) ||      
                   ( *ptrChar == ';' ) ||      
                   ( ( *ptrChar    == '/' ) &&  
                     ( *(ptrChar+1)== '/' ) ) ) {
                 bCommentLine = TRUE ;
                 if ( strstr( szIn, "NON-TRANSLATABLE" ) ) { 
                    if ( strstr( szIn, "START" ) ) 
                       bInNonTransSection = TRUE ;
                    if ( strstr( szIn, "END" ) ) 
                       bInNonTransSection = FALSE ;
                 }
              }
           }
        }

        /*---------------------------------------------------------------------*/
        /*  Remove special <TWBCTX>, </TWBCTX>  tag which are added to surround*/
        /*  context info.                                                      */
        /*---------------------------------------------------------------------*/
        RemoveContextInfo(szIn);

        /*---------------------------------------------------------------------*/
        /* Modify choice variables to identify translatable text. */
        /*---------------------------------------------------------------------*/
        if ( ( ( strchr( szIn, '{' )      ) &&
               ( strstr( szIn, "choice" ) ) ) ||
             ( bHandleChoiceVariables       ) ) {          
           bHandleChoiceVariables = TRUE ;
           HandleChoiceVariables( szIn, FALSE ) ;
        }

        /*---------------------------------------------------------------------*/
        /* Identify HTML tags versus text after less than. */
        /*---------------------------------------------------------------------*/
        if ( ( strchr( szIn, '<' ) ) &&
             ( strchr( szIn, '>' ) ) ) {
           HandleHtmlTags( szIn, FALSE ) ;
        }

        /*---------------------------------------------------------------------*/
        /* Double single quotes if text goes through MessageFormat.  */
        /*---------------------------------------------------------------------*/
        HandleSingleQuotes( szIn, FALSE, &bMF_InBlock ) ;

        /*---------------------------------------------------------------------*/
        /*  Scan backward to find the last :qf_ tag to determine whether the   */
        /*  line ends in translatable or nontranslatable text.                 */
        /*---------------------------------------------------------------------*/
        for ( i = strlen(szIn) ;
            i>=0 && ( strncmp( &szIn[i], ":qf", 3 ) &&
                      strncmp( &szIn[i], ":QF", 3 ) ) ;
            --i ) ;
        if ( i > 0 ) {                        /* If found tag, set line type.  */
            if ( ( ! strncmp( &szIn[i], ":qfn", 4 ) ) ||
                 ( ! strncmp( &szIn[i], ":QFN", 4 ) ) ) {
               usSegmentType = SEGMENT_NONTRANS ;
            } else {
               usSegmentType = SEGMENT_TRANS ;
               strncpy( szLastQfTag, &szIn[i+1], 3 ) ;
               szLastQfTag[3] = 0 ;
               if ( isupper(szLastQfTag[0]) ) 
                  ptrChar = strstr( &szIn[i], "N=" ) ;
               else
                  ptrChar = strstr( &szIn[i], "n=" ) ;
               if ( ptrChar ) {
                  strcpy( szTemp, ptrChar+2 ) ;
                  strtok( szTemp, " \t." ) ;
                  usLastSeqNum = atoi( szTemp ) ;
               }
               ptrPeriod = strchr( &szIn[i], '.' ) ;
               if ( ptrPeriod ) 
                  usSegmentLength = strlen( ptrPeriod + 1 ) ;
               else
                  usSegmentLength = 0 ;
            }
        } else {
           if ( usSegmentType == SEGMENT_TRANS ) {
              usSegmentLength += strlen( szIn ) ;
           }
        }

        /*---------------------------------------------------------------------*/
        /*  If end of this line is part of translatable text and the start of  */
        /*  the next line is a continuation of the translatable text,          */
        /*  then add a continuation character to the end of the line.          */
        /*---------------------------------------------------------------------*/
        if ( usSegmentType == SEGMENT_TRANS ) {                      

           bTextContinued = TRUE ;
           if ( ( ( ! strncmp(szNext,":eqf",4) ) &&
                  ( ( szNext[6] != ':' ) ||
                    ( ! strncmp(&szNext[5],".:qfn",5) ) ) ) ||
                ( ( ! strncmp(szNext,":EQF",4) ) &&
                  ( ( szNext[6] != ':' ) ||
                    ( ! strncmp(&szNext[5],".:QFN",5) ) ) ) ) {
              for( i=9 ; szNext[i] && szNext[i]!='.' ; ++i ) ;
              if ( ( szNext[i] ) &&        /* If next rcd has only cont. char, */
                   ( szNext[i+1] == '\\' ) ) { /* then continued text.         */
                 for( j=i+2 ; szNext[j] && isspace(szNext[j]) ; ++j ) ;
                 if ( szNext[j] ) 
                    bTextContinued = FALSE ;
              } else {
                 bTextContinued = FALSE ;
              }
           }

           /*------------------------------------------------------------------*/
           /*  If next lines starts with "JOIN=2", then this line has already  */
           /*  been joined with the previous line and will not be written out. */
           /*  Just skip this line.                                            */
           /*      :QFX N=3 JOIN=1 S=1.translatable text.  More text.          */
           /*      :EQFX.:QFF N=4 JOIN=2 S=1.More text.                        */
           /*------------------------------------------------------------------*/
           if ( bTextContinued ) {
              ptrPeriod = strchr( &szNext[6], '.' ) ;
              ptrJoin = strstr( &szNext[6], " JOIN=2" ) ;
              if ( ptrPeriod && ptrJoin && (ptrJoin<=ptrPeriod) ) {
                 bTextContinued = FALSE ;
                 lCurrentPos = ftell( fSource ) ; /* Save current file position   */
                 strcpy( szTemp, ptrPeriod+1 ) ;
                 bLoop = TRUE ;
                 do {
                    for( ptrNext=szTemp ; ptrNext ; ) {
                       ptrJoin = strstr(ptrNext,":eqf") ;
                       if ( ! ptrJoin )
                          ptrJoin = strstr(ptrNext,":EQF") ;
                       if ( ptrJoin ) {
                          if ( ( ( ! strncmp(ptrJoin+5,".:qff",5) ) ||
                                 ( ! strncmp(ptrJoin+5,".:QFF",5) ) ) &&
                               ( strstr( ptrJoin+5, " JOIN=2" ) ) ) {
                             ptrNext = strchr( ptrJoin+5, '.' ) ;
                             continue ;
                          }
                          if ( ( strncmp(ptrJoin+5,".:qfn",5) ) &&
                               ( strncmp(ptrJoin+5,".:QFN",5) ) &&
                               ( *(ptrJoin+6) ) ) {
                             bTextContinued = TRUE ;
                          }
                          bLoop = FALSE ;
                          break ;
                       }
                       break ;
                    }
                    if ( ! bLoop ) 
                       break ;
                 } while ( (INT)fgets( szTemp, MAX_RCD_LENGTH, fSource ) > 0 ) ;
                 fseek( fSource, lCurrentPos, 0 ) ;  /* Reset file reading pos */
              }
           }
           if ( bTextContinued ) {
              if ( ( strlen(szIn) > 0 ) &&
                   ( szIn[strlen(szIn)-1] == '\n' ) )
                 szIn[strlen(szIn)-1] = '\0' ;

             if ( ( strlen(szIn) > 0 ) &&
                  ( szIn[strlen(szIn)-1] == '\r' ) )
                 szIn[strlen(szIn)-1] = '\0' ;

              /*------------------------------------------------------------------*/
              /*  If line ends with text which is not DBCS nor \n, then add blank.*/
              /*------------------------------------------------------------------*/
              bTrailingBackslash = FALSE ;                           
              i = strlen( szIn ) ;
              if ( ( i > 0 ) &&                /* Check if already backslash continuation */
                   ( szIn[i-1] == '\\' ) ) {
                 for( j=0 ; j < i ; ++j ) {
                    if ( szIn[j] == '\\' ) 
                       ++j ; 
                 }
                 if ( j > i ) 
                    bTrailingBackslash = TRUE ; 
              }
              if ( ! bTrailingBackslash ) {
                 if ( ( ( i == 1 ) ||                              /* 1 char, or    */
                        ( ( i > 1 ) &&                             /* Not newline   */
                          ( strncmp( &szIn[i-2], "\\n", 2 ) ) &&   /*   and         */
                          ( ( i == 2 ) ||                          /* Not DBCS char */
                            ( !IsUTF8DBCS(szIn,(SHORT)(i-1),bDBCSTgt) ) ) ) ) &&                               
                      ( ! isspace( szIn[i-1] ) ) &&                /* Not whitespace*/
                      ( ! bPartialRecord ) ) { 
                    strcat( szIn, " " ) ;
                 }
   //            strcat( szIn, "\\\n" ) ;
                 strcat( szIn, "\\" ) ;
                                /* If the translated segment is already close to the max 2047 */
                                /* characters, then adding backslash continuation may cause   */
                                /* the segment length of 2047.  If this happens, the export   */
                                /* fails. By adding a dummy duplicate segment, the export     */
                                /* completes successfully.                           10-20-09 */
                 if ( usSegmentLength > 1800 ) {                 
                    if ( isupper(szLastQfTag[0]) ) 
                       sprintf( szTemp, ":E%s.:%s N=%d.", szLastQfTag, szLastQfTag, usLastSeqNum ) ;
                    else
                       sprintf( szTemp, ":e%s.:%s n=%d.", szLastQfTag, szLastQfTag, usLastSeqNum ) ;
                    strcat( szIn, szTemp ) ;
                    usSegmentLength = 0 ;
                 }
              }
              strcat( szIn, szNewLine ) ;
           }
        } else {

           /*---------------------------------------------------------------------*/
           /*  If line contains a line comment not in column 1, then reflow it    */
           /*  back to a separate line.                                           */
           /*---------------------------------------------------------------------*/
           ptrChar = strstr( szIn, ".#" ) ;
           if ( ! ptrChar )
              ptrChar = strstr( szIn, ".!" ) ;
           if ( ptrChar >= szIn+14 ) {
              for( ptrChar-=14, i=0 ;
                   i<4 && ptrChar>szIn && *ptrChar!=':' ;
                   --ptrChar, ++i ) ;
              if ( ( ptrChar > szIn  ) &&
                   ( *ptrChar == ':' ) ) {
                 strncpy( szTag, ptrChar, sizeof(szTag) ) ;
                 szTag[sizeof(szTag)-1] = 0 ;
                 strupr( szTag ) ;
                 if ( ( ! strncmp( szTag, ":EQF", 4 )         ) &&
                      ( ! strncmp( &szTag[5], ".:QFN N=", 8 ) ) ) {
                    if ( strlen(szNewLine) == 1 ) {
                       memmove( ptrChar+1, ptrChar, strlen(ptrChar)+1 ) ;
                       *ptrChar = '\n' ;
                    } else {
                       memmove( ptrChar+2, ptrChar, strlen(ptrChar)+1 ) ;
                       strncpy( ptrChar, szNewLine, 2 ) ;
                    }
                 }
              }
           }
        }


        fputs( szIn, fTarget ) ;             /* Write out this output record. */
        strcpy( szIn, szNext ) ;             /* Set current from look-ahead.  */
        bPartialRecord = bPartialRecordNext ;
        szNext[0] = '\0' ;
    }

    if ( szIn[0] ) {                         /* If still look-ahead,          */
       HandleSingleQuotes( szIn, FALSE, &bMF_InBlock ) ;
       RemoveContextInfo( szIn ) ;           /* Remove context tags           */
       fputs( szIn, fTarget )  ;             /*   Write out this final text.  */
    }
    if ( szNext[0] ) {                       /* If still look-ahead text,     */
       HandleSingleQuotes( szNext, FALSE, &bMF_InBlock ) ;
       RemoveContextInfo( szNext ) ;         /* Remove context tags           */
       fputs( szNext, fTarget ) ;            /*   Write out this final text.  */
    }

    fclose ( fSource ) ;                     /* Close input segmented file.   */
    fclose ( fTarget ) ;                     /* Close output target file.     */


    if ( szIn )
        free( szIn ) ;
    if ( szNext )
        free( szNext ) ;
    if ( szTemp )
        free( szTemp ) ;

    return 0 ;
}



/*************************************************************************/
/*  Post export the JDK 1.1 property file.                               */
/*************************************************************************/
USHORT  ExportFile2(
                  PSZ    pszTarget,
                  PSZ    pszTemp,
                  USHORT usCPType )
{

    FILE     *fTarget, *fTemp ;

    UCHAR    *szIn = NULL ;
    UCHAR    *ptrChar, *ptrPrevChar ;
    SHORT    i, u ;
    INT      iRC ;
    BOOL     bInNonTransSection = FALSE ;
    BOOL     bLineContinue = FALSE ;


    fTarget = fopen ( pszTarget, "rb" ) ;     /* Open input target file.    */
    if ( ! fTarget ) return 0 ;

    szIn = malloc( MAX_RCD_LENGTH ) ;
    if ( ! szIn ) return 0 ;

    /*---------------------------------------------------------------------*/
    /*  If first record contains <TWB_NOSPLIT>, then the file should       */
    /*  not contain any continued lines.  Reflow text onto a single line   */
    /*  for each string.                                                   */
    /*---------------------------------------------------------------------*/
    iRC = (INT)fgets( szIn, MAX_RCD_LENGTH, fTarget ) ; 
    ptrChar = strstr( szIn, szNoSplitTag ) ; 
    if ( ptrChar ) {
       memmove( ptrChar, ptrChar+strlen(szNoSplitTag), strlen(ptrChar+strlen(szNoSplitTag))+1 ) ;
       fTemp = fopen ( pszTemp, "wb+" ) ;        /* Open output temp file.    */
       if ( ! fTemp ) return 0 ;

       while ( iRC > 0 ) {

          /*----------------------------------------------------------------*/
          /*  If special comment which defines a block of source which      */
          /*  should not be translated, then set up processing.             */
          /*----------------------------------------------------------------*/
          if ( strstr( szIn, "NON-TRANSLATABLE" ) ) { 
             for ( i=0; isspace( szIn[i] ) ; ++i ) ;
             if ( ( szIn[i] == '#' ) ||
                  ( szIn[i] == '!' ) ||
                  ( szIn[i] == ':' ) ||                                 
                  ( szIn[i] == ';' ) ||                                 
                  ( ( szIn[i]   == '/' ) &&                             
                    ( szIn[i+1] == '/' ) ) ) {
                if ( strstr( szIn, "START" ) ) 
                   bInNonTransSection = TRUE ;
                if ( strstr( szIn, "END" ) ) 
                   bInNonTransSection = FALSE ;
             }
          }

          if ( ! bInNonTransSection ) {                      
              
             /* Remove extra backslash for blanks */
             ptrChar = strstr( szIn, "\\ " ) ;  
             if ( ( bLineContinue ) &&          
                  ( ptrChar ) && 
                  ( ptrChar == szIn ) ) {
                memmove( ptrChar, ptrChar+1, strlen(ptrChar+1)+1 ) ;
             }
             bLineContinue = FALSE ;                        
             for( ptrChar=szIn ; *ptrChar ; ++ptrChar ) {
                if ( *ptrChar == '\\' ) {
                   if ( *(ptrChar+1) == '\\' ) {    
                      ++ptrChar ;
                   } else {
                      if (  ( ( ( *(ptrChar+1) == '\n' ) &&  
                              ( *(ptrChar+2) == 0    ) ) ||
                            ( ( ( *(ptrChar+1) == '\r' ) &&  
                                ( *(ptrChar+2) == '\n' ) &&
                                ( *(ptrChar+3) == 0    ) ) ) ) ) {
                        *ptrChar = 0 ;
                         bLineContinue = TRUE ;
                        break ;
                     }
                   }
                }
                if ( ( usCPType == EXPORT_CP_DBCS_ASCII ) &&
                     ( IsDBCS(*ptrChar) ) ) {
                   ++ptrChar ;
                }
                if (  usCPType == EXPORT_CP_UTF8 ) {
                   u = GetCharLength( ptrChar ) ;                   
                   if ( u > 1 ) {                             /* Multi-byte char */
                      ptrChar += u - 1 ;
                   }
                }
             }
          }
          fputs( szIn, fTemp ) ;           

          iRC = (INT)fgets( szIn, MAX_RCD_LENGTH, fTarget ) ;
       }
       fclose ( fTarget ) ;                     /* Close output target file. */
       fclose ( fTemp ) ;                 

       CopyFile( pszTemp, pszTarget, FALSE ) ;
       remove( pszTemp ) ;
    } else {
       fclose ( fTarget ) ;                     /* Close output target file. */
    }

    if ( szIn )
       free( szIn ) ;

    return 0 ;
}



/*************************************************************************/
/*  Start a new TM/2 segment.                                            */
/*************************************************************************/
VOID  StartJDKSegment(
                     FILE   *fTarget,
                     USHORT usSegmentType,
                     USHORT *usCurSegmentType )
{

    /*------------------------------------------------------------------------*/
    /*  If current segment is either translatable or non-translatable, then   */
    /*  end the current segment before starting a new segment.                */
    /*------------------------------------------------------------------------*/
    if ( *usCurSegmentType != SEGMENT_NONE )
        EndJDKSegment( fTarget, *usCurSegmentType ) ;

    ++usSegNum ;                             /* Increment segment number.     */

    /*------------------------------------------------------------------------*/
    /*  Write out start of new segment tagging.                               */
    /*------------------------------------------------------------------------*/
    if ( usSegmentType == SEGMENT_TRANS )
        fprintf( fTarget, ":qff n=%d.", usSegNum ) ;
    else
        fprintf( fTarget, ":qfn n=%d.", usSegNum ) ;

    *usCurSegmentType = usSegmentType ;      /* Save type of current segment. */
    return ;
}


/*************************************************************************/
/*  End an existing TM/2 segment.                                        */
/*************************************************************************/
VOID  EndJDKSegment(
                   FILE   *fTarget,
                   USHORT usSegmentType )
{

    /*---------------------------------------------------------------------*/
    /*  If current segment is either translatable or non-translatable,     */
    /*  then end the current segment.                                      */
    /*---------------------------------------------------------------------*/
    if ( usSegmentType != SEGMENT_NONE ) {
        if ( usSegmentType == SEGMENT_TRANS )
            fprintf( fTarget, ":eqff." ) ;
        else
            fprintf( fTarget, ":eqfn." ) ;
    }

    return ;
}


/*************************************************************************/
/*  Determine if the current DBCS character is a sentence ending         */
/*  character or not.  Index is intentionally the 2nd byte of the DBCS   */
/*  character.  Return TRUE if is DBCS sentence ending char.             */
/*************************************************************************/
BOOL  IsDBCSSentenceEnd(
                       char  *szInput,
                       SHORT sIndex )
{

    char     szDBCSChar[4] ;
    ULONG    CpList[8] ;
    ULONG    CpSize ;
    USHORT   usCodePage ;

    /*------------------------------------------------------------------------*/
    /*  If need to determine whether this is for DBCS, then get the code page */
    /*  information for this session.                                         */
    /*------------------------------------------------------------------------*/
    if ( !bDBCSInfo ) {
        bDBCSInfo = TRUE ;
        if ( ! stricmp( szDocSourceLanguage, "Japanese" ) ) 
           bJapanese = TRUE ;
        else
           if ( ! stricmp( szDocSourceLanguage, "Korean" ) ) 
              bKorean = TRUE ;
        else
           if ( ! stricmp( szDocSourceLanguage, "Chinese(simpl.)" ) )
              bSChinese = TRUE ;
        else
           if ( ! stricmp( szDocSourceLanguage, "Chinese(trad.)" ) )
              bTChinese = TRUE ;
        else
            bNoDBCS = TRUE ;
    }

    /*------------------------------------------------------------------------*/
    /*  Determine if the current DBCS char (index is to 2nd byte of char) is  */
    /*  a sentence-ending character.                                          */
    /*------------------------------------------------------------------------*/
    if ( bNoDBCS ||
         sIndex<2 )
        return FALSE ;

    strncpy( szDBCSChar, &szInput[sIndex-2], 3 ) ;
    szDBCSChar[3] = 0 ;


    if ( ( ( bJapanese ) ||                      /* Japanese                      */
           ( bKorean   ) ||                      /* Korean                        */
           ( bSChinese ) ||                      /* Simplified Chinese            */
           ( bTChinese ) ) &&                    /* Traditional Chinese           */
         ( ( !strcmp( szDBCSChar, "。" ) ) ||   /* x'E38082', J-period(full stop)*/
           ( ( !strcmp( szDBCSChar, "．" ) ) && /* x'EFBC8E', period             */
             ( ! bSChinese ) ) ||                /*  not Simplified Chinese       */
           ( !strcmp( szDBCSChar, "？" ) ) ||   /* x'EFBC9F', question mark      */
           ( !strcmp( szDBCSChar, "！" ) ) ) )   /* x'EFBC81', exclamation mark   */
        return TRUE ;

    return FALSE ;
}


/*************************************************************************/
/*  Determine if the current DBCS character is a UTF-8 DBCS character    */
/*  or not.  Index must be set to the last byte of a multi-byte UTF-8    */
/*  character to return TRUE.                                            */
/*************************************************************************/
BOOL  IsUTF8DBCS(
                       char  *szInput,
                       SHORT sIndex,
                       BOOL  bDBCS  )
{
    if ( ( ! bDBCS ) ||
         ( sIndex < 2 ) ||
         ( (UCHAR)(szInput[sIndex-2]) < (UCHAR)'\xE3' ) ) {     
       return FALSE ;                          /* Not DBCS char */
    }

    return TRUE ;                              /* Is DBCS char */
}


/*************************************************************************/
/*  Determine if the string begins with an HTML tag.                     */
/*************************************************************************/
BOOL  IsHTMLTag( UCHAR  *szInput, USHORT *usType ) 
{
typedef struct
{
   UCHAR   szTag[15] ;
   USHORT  usType ;       /* 0=Break tag, 1=Inline tag */
} TAG ;

#define NUM_HTML_TAGS         89
static TAG  HTML_Tags[ NUM_HTML_TAGS ] =
{
   "A",              1,
   "ABBR",           1,
   "ACRONYM",        1,
   "ADDRESS",        0,
   "APPLET",         0,
   "AREA",           0,
   "B",              1,
   "BASE",           0,
   "BIG",            1,
   "BLOCKQUOTE",     0,
   "BODY",           0,
   "BR",             0,
   "BUTTON",         0,
   "CAPTION",        0,
   "CENTER",         0,
   "CITE",           1,
   "CODE",           1,
   "COL",            0,
   "COLGROUP",       0,
   "COMMENT",        0,
   "CONTENT",        0,
   "DD",             0,
   "DEL",            0,
   "DFN",            1,
   "DIR",            0,
   "DIV",            0,
   "DL",             0,
   "DT",             0,
   "EM",             1,
   "FONT",           1,
   "FORM",           0,
   "FRAME",          0,
   "FRAMESET",       0,
   "H1",             0,
   "H2",             0,
   "H3",             0,
   "H4",             0,
   "H5",             0,
   "H6",             0,
   "HEAD",           0,
   "HR",             0,
   "HTML",           0,
   "I",              1,
   "IMG",            1,
   "IMPLIED",        0,
   "INPUT",          0,
   "INS",            0,
   "ISINDEX",        0,
   "KBD",            1,
   "LABEL",          0,
   "LEGEND",         0,
   "LI",             0,
   "LINK",           1,
   "MAP",            0,
   "MENU",           0,
   "META",           0,
   "NOFRAMES",       0,
   "OBJECT",         0,
   "OL",             0,
   "OPTGROUP",       0,
   "OPTION",         0,
   "P",              0,
   "PARAM",          0,
   "PRE",            0,
   "Q",              1,
   "S",              1,
   "SAMP",           1,
   "SCRIPT",         0,
   "SELECT",         0,
   "SMALL",          1,
   "SPAN",           1,
   "STRIKE",         1,
   "STRONG",         1,
   "STYLE",          0,
   "SUB",            1,
   "SUP",            1,
   "TABLE",          0,
   "TBODY",          0,
   "TD",             0,
   "TEXTAREA",       0,
   "TFOOT",          0,
   "TH",             0,
   "THEAD",          0,
   "TITLE",          0,
   "TR",             0,
   "TT",             1,
   "U",              1,
   "UL",             0,
   "VAR",            1,
} ;

    UCHAR    szTag[20] ;
    short    i ;
    BOOL     bHTMLTag = FALSE ;

    *usType = 0 ;
    if ( szInput[0] == '<' ) {
       if ( szInput[1] == '/' ) 
          strncpy( szTag, &szInput[2], sizeof(szTag) ) ;
       else
          strncpy( szTag, &szInput[1], sizeof(szTag) ) ;
       szTag[sizeof(szTag)-1] = 0 ;
       strtok( szTag, " >/\n\t\r" ) ;
       strupr( szTag ) ; 
       for( i=0 ; i<NUM_HTML_TAGS ; ++i ) {
          if ( ! strcmp( szTag, HTML_Tags[i].szTag ) ) {
             bHTMLTag = TRUE ; 
             *usType = HTML_Tags[i].usType ;
             break ;
          }
       }
    }

    return( bHTMLTag ) ;
}


/******************************************************************************/
/*  Modify "choice" variables to properly identify translatable text.         */
/*  Example:                                                                  */
/*  {0,choice,0#0 items|1#{0,number,integer} item|1<{0,number,integer} items} */
/*  {1,choice,-1#|0# at location {1}}                                         */
/******************************************************************************/
void  HandleChoiceVariables(
                       char  *szInput, 
                       BOOL   bAnalysis ) 
{
   CHAR     *ptrCharS, *ptrChar;
   SHORT    u ;
   BOOL     bVar ;


    if ( bAnalysis ) {
       /*------------------------------------------------------------------------*/
       /*  Modify choice variable to identify translatable text.                 */
       /*------------------------------------------------------------------------*/
       for( ptrCharS=strchr(szInput,'{') ;  ptrCharS ; ptrCharS=strchr(ptrCharS+1,'{') ) {
          for( ptrChar=ptrCharS+1 ; *ptrChar && isspace(*ptrChar) ; ++ptrChar ) ;
          if ( isdigit(*ptrChar) ) {
             for( ; *ptrChar && isdigit(*ptrChar) ; ++ptrChar ) ;
             for( ; *ptrChar && ( isspace(*ptrChar) || *ptrChar==',' ) ; ++ptrChar ) ;
             if ( ! strncmp( ptrChar, "choice", 6 ) ) {
                for( ptrChar+=6 ; 
                     *ptrChar && ( isspace(*ptrChar) || *ptrChar==',' ) ; ++ptrChar ) ;
                for( ; *ptrChar && *ptrChar!='}' ; ++ptrChar ) {
                   for( ; *ptrChar && (isdigit(*ptrChar)||*ptrChar=='-') ; ++ptrChar ) ;
                   if ( ( *ptrChar ) &&
                        ( strchr("#<>", *ptrChar) ) ) 
                      ++ptrChar ;
                   memmove( ptrChar+5, ptrChar, strlen(ptrChar)+1 ) ;
                   strncpy( ptrChar, "$TWB}", 5 ) ;
                   ptrChar += 5 ;
                   for( bVar=FALSE ; *ptrChar ; ++ptrChar ) {
//                    if ( IsDBCS( *ptrChar ) ) {
//                       ++ptrChar ;
//                       continue ;
//                    }
                      u = GetCharLength( ptrChar ) ;                      
                      if ( u > 1 ) {                              /* Multi-byte char */
                         ptrChar += u - 1 ;
                         continue ;
                      }
                      if ( bVar ) {
                         if ( *ptrChar == '}' )
                            bVar = FALSE ;
                         continue ;
                      }

                      if ( strchr( "|}", *ptrChar ) ) 
                         break ;
                      if ( *ptrChar == '{' ) 
                         bVar = TRUE ; 
                   }
                   if ( *ptrChar ) {
                      if ( *ptrChar == '|' ) {
                          memmove( ptrChar+5, ptrChar, strlen(ptrChar)+1 ) ;
                          strncpy( ptrChar, "{TWB$", 5 ) ;
                          ptrChar += 5 ;
                      } else {
                         --ptrChar ;
                      }
                   }
                }
                if ( ( *ptrChar ) &&
                     ( *ptrChar == '}' ) ) {
                   memmove( ptrChar+5, ptrChar, strlen(ptrChar)+1 ) ;
                   strncpy( ptrChar, "{TWB$", 5 ) ;
                   ptrChar += 4 ;
                }
             }
          }
       }
    } else {
       /*------------------------------------------------------------------------*/
       /*  Remove extra TWB tags to restore choice variable format.      2-26-03 */
       /*------------------------------------------------------------------------*/
       for( ptrCharS=strstr(szInput,"{TWB$") ;  ptrCharS ; ptrCharS=strstr(ptrCharS,"{TWB$") ) {
          memmove( ptrCharS, ptrCharS+5, strlen(ptrCharS+5)+1 ) ;
       }
       for( ptrCharS=strstr(szInput,"$TWB}") ;  ptrCharS ; ptrCharS=strstr(ptrCharS,"$TWB}") ) {
          memmove( ptrCharS, ptrCharS+5, strlen(ptrCharS+5)+1 ) ;
       }
    }

    return ;
}


/******************************************************************************/
/*  Distinguish between real HTML tags and translatable text enclosed within  */
/*  less than and greater than.                                               */
/*       <select name="xx">                                                   */
/*       <Select range>                                                       */
/******************************************************************************/
void  HandleHtmlTags(
                       char  *szInput, 
                       BOOL   bAnalysis ) 
{
   CHAR     *ptrCharS, *ptrCharQ, *ptrCharE;
   UCHAR    szTag[20] ;


   if ( bAnalysis ) {
      for( ptrCharS=strchr(szInput,'<') ;  ptrCharS ; ptrCharS=strchr(ptrCharS+1,'<') ) {
         if ( ( *(ptrCharS+1) == 'S' ) ||
              ( *(ptrCharS+1) == 's' ) ) {
            strncpy( szTag, ptrCharS+1, sizeof(szTag) ) ;
            szTag[sizeof(szTag)-1] = 0 ;
            strtok( szTag, " >/\n\t\r" ) ;
            strupr( szTag ) ; 
            if ( ( ! strcmp( szTag, "SELECT" ) ) &&
                 ( isspace( *(ptrCharS+7) )    ) ) {
               ptrCharQ = strchr( ptrCharS, '=' ) ;
               ptrCharE = strchr( ptrCharS, '>' ) ;
               if ( ( ptrCharQ > ptrCharS ) &&
                    ( ptrCharE > ptrCharQ ) ) {
                  *(ptrCharS+6) = 'Q' ;
               }
            }
         }
      }
   } else {
      /*------------------------------------------------------------------------*/
      /*  Restore original HTML tag names.                                      */
      /*------------------------------------------------------------------------*/
      for( ptrCharS=strchr(szInput,'<') ;  ptrCharS ; ptrCharS=strchr(ptrCharS+1,'<') ) {
         if ( ( *(ptrCharS+1) == 'S' ) ||
              ( *(ptrCharS+1) == 's' ) ) {
            strncpy( szTag, ptrCharS+1, sizeof(szTag) ) ;
            szTag[sizeof(szTag)-1] = 0 ;
            strtok( szTag, " >/\n\t\r" ) ;
            strupr( szTag ) ; 
            if ( ! strcmp( szTag, "SELECQ" ) ) {
               if ( *(ptrCharS+5) == 'c' ) 
                  *(ptrCharS+6) = 't' ;
               else
                  *(ptrCharS+6) = 'T' ;
            }
         }
      }
   }

   return ;
}


/******************************************************************************/
/*  Determine if the text contains a replacement variable.                    */
/*  Example:                                                                  */
/*      {number}                                                              */
/*      {number,options}                                                      */
/******************************************************************************/
BOOL  TextContainsVariable(
                       char  *szInput )
{
   CHAR     *ptrCharS, *ptrChar;
   BOOL     bFoundVar = FALSE ; 


    for( ptrCharS=strchr(szInput,'{') ;  ptrCharS ; ptrCharS=strchr(ptrCharS+1,'{') ) {
       for( ptrChar=ptrCharS+1 ; *ptrChar && isspace(*ptrChar) ; ++ptrChar ) ;
       if ( isdigit(*ptrChar) ) {
          for( ; *ptrChar && isdigit(*ptrChar) ; ++ptrChar ) ;
          for( ; *ptrChar && isspace(*ptrChar) ; ++ptrChar ) ;
          if ( ( *ptrChar == '}' ) || 
               ( *ptrChar == ',' ) ) {
             bFoundVar = TRUE ;
             break ;
          }
///    } else {              Special code for France, TPC22
///       if ( *ptrChar == '$' ) {
///          for( ; *ptrChar && !isspace(*ptrChar) && *ptrChar!='}' ; ++ptrChar ) ;
///          if ( *ptrChar == '}' ) {
///             bFoundVar = TRUE ;
///             break ;
///          }
///       }
       }
    }

    return( bFoundVar ) ;
}


/******************************************************************************/
/*  Handle single quotes in the text which will be processed by the Java      */
/*  MessageFormat API.                                                        */
/*    bRemove=TRUE   Change double single quote to single single quote.       */
/*    bRemove=FALSE  Change single single quote to double single quote.       */
/******************************************************************************/
void  HandleSingleQuotes(
                       char  *szInput, 
                       BOOL  bRemove,
                       BOOL  *bMF_InBlock  )
{
   CHAR     *ptrChar;
   SHORT    u ;


    if ( bRemove ) {
       for( ptrChar=szInput ;  *ptrChar ; ++ptrChar ) {
//        if ( IsDBCS( *ptrChar ) ) {
//           ++ptrChar ;
//           continue ;
//        }
          u = GetCharLength( ptrChar ) ;                      
          if ( u > 1 ) {                              /* Multi-byte char */
             ptrChar += u - 1 ;
             continue ;
          }
          if ( *ptrChar == '\'' ) {
             if ( *(ptrChar+1) == '\'' ) {
                memmove( ptrChar+1, ptrChar+2, strlen(ptrChar+2)+1 ) ;
             } else 
             if ( ( *(ptrChar+1) == '\\' ) &&
                  ( *(ptrChar+2) == '\'' ) ) {
                memmove( ptrChar+1, ptrChar+3, strlen(ptrChar+3)+1 ) ;
             } else {
                memmove( ptrChar+strlen(szMF_SingleQuote), ptrChar+1, strlen(ptrChar+1)+1 ) ;
                strncpy( ptrChar, szMF_SingleQuote, strlen(szMF_SingleQuote) ) ;
             }
          } else {
             if ( ( *ptrChar     == '\\' ) &&
                  ( *(ptrChar+1) == '\'' ) ) {
                if ( *(ptrChar+2) == '\'' ) {
                   memmove( ptrChar+2, ptrChar+3, strlen(ptrChar+3)+1 ) ;
                   ++ptrChar ;
                } else
                if ( ( *(ptrChar+2) == '\\' ) &&
                     ( *(ptrChar+3) == '\'' ) ) {
                   memmove( ptrChar+2, ptrChar+4, strlen(ptrChar+4)+1 ) ;
                   ++ptrChar ;
                } else {
                   memmove( ptrChar+strlen(szMF_EscapedSingleQuote), ptrChar+2, strlen(ptrChar+2)+1 ) ;
                   strncpy( ptrChar, szMF_EscapedSingleQuote, strlen(szMF_EscapedSingleQuote) ) ;
                }
             }
          }
       }
    } else {
       for( ptrChar=szInput ;  *ptrChar ; ++ptrChar ) {
//        if ( IsDBCS( *ptrChar ) ) {
//           ++ptrChar ;
//           continue ;
//        }
          u = GetCharLength( ptrChar ) ;                      
          if ( u > 1 ) {                              /* Multi-byte char */
             ptrChar += u - 1 ;
             continue ;
          }
          if ( *bMF_InBlock ) {
             if ( *ptrChar == '\'' ) {
                memmove( ptrChar+1, ptrChar, strlen(ptrChar)+1 ) ;
                ++ptrChar ;
                continue ;
             } 
             if ( ( *ptrChar     == '\\' ) &&
                  ( *(ptrChar+1) == '\'' ) ) {
                memmove( ptrChar+2, ptrChar, strlen(ptrChar)+1 ) ;
                ptrChar += 3 ;
                continue ;
             }
             if ( ! strncmp( ptrChar, szMF_SingleQuote, strlen(szMF_SingleQuote) ) ) {
                memmove( ptrChar+1, ptrChar+strlen(szMF_SingleQuote), strlen(ptrChar+strlen(szMF_SingleQuote))+1 ) ;
                *ptrChar = '\'' ;
                continue ;
             } 
             if ( ! strncmp( ptrChar, szMF_EscapedSingleQuote, strlen(szMF_EscapedSingleQuote) ) ) {
                memmove( ptrChar+2, ptrChar+strlen(szMF_EscapedSingleQuote), strlen(ptrChar+strlen(szMF_EscapedSingleQuote))+1 ) ;
                *(ptrChar++) = '\\' ;
                *ptrChar = '\'' ;
                continue ;
             } 
          }
          if ( !strncmp( ptrChar, szMF_StartQuoteBlock, strlen(szMF_StartQuoteBlock) ) ) {
             *bMF_InBlock = TRUE ;
             memmove( ptrChar, ptrChar+strlen(szMF_StartQuoteBlock),strlen(ptrChar+strlen(szMF_StartQuoteBlock))+1 ) ;
             --ptrChar ; 
             continue ;
          }
          if ( !strncmp( ptrChar, szMF_EndQuoteBlock, strlen(szMF_EndQuoteBlock) ) ) {
             *bMF_InBlock = FALSE ;
             memmove( ptrChar, ptrChar+strlen(szMF_EndQuoteBlock),strlen(ptrChar+strlen(szMF_EndQuoteBlock))+1 ) ;
             --ptrChar ; 
             continue ;
          }
       }
    }

    return ;
}

/******************************************************************************/
/*  Remove special <TWBCTX>, </TWBCTX> tags which were added to surround      */
/*  the context information.                                                  */
/******************************************************************************/
/*----------------------------------------------------------------------*/
void RemoveContextInfo(
                       char  *szText)
{
    char   *ptrChar;
				
    for( ptrChar=strstr(szText,szCONTEXT_START) ;  ptrChar ; ptrChar=strstr(ptrChar,szCONTEXT_START) ) {
       memmove( ptrChar, ptrChar+8, strlen(ptrChar+8)+1 ) ;
    }
    for( ptrChar=strstr(szText,szCONTEXT_END) ;  ptrChar ; ptrChar=strstr(ptrChar,szCONTEXT_END) ) {
       memmove( ptrChar, ptrChar+9, strlen(ptrChar+9)+1 ) ;
    }
}

