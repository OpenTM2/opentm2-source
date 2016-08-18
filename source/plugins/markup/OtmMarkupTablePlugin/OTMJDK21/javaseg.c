/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/*********************************************************************

 * FUNCTION:
 *
 * Segmentation routine to segment JDK 1.1 ListResourceBundle (LRB) files
 * for Translation in TM/2
 *
 ********************************************************************/
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/


#define INCL_DOSNLS
#define INCL_DOSFILEMGR
#define INCL_DOSERRORS
#include "otmjdk11.h"

void   HandleLRBChoiceVariables( FILE *, FILE * ) ;
void   MFAdjustQuotes( char *, char * ) ;

    #include <windows.h>
    #include <wtypes.h>


/* DEFINE STATE VALUES */

#define  BIT_ESC                0x00000001
#define  BIT_OPEN_BRAC          0x00000002
#define  BIT_COMMA              0x00000004
#define  BIT_DOUBLE_QUOTE       0x00000008
#define  BIT_SINGLE_QUOTE       0x00000010
#define  BIT_XLATE              0x00000020
#define  BIT_START_XLATE        0x00000040
#define  BIT_END_XLATE          0x00000080
#define  BIT_FWD_SLASH          0x00000100
#define  BIT_ASTRISK            0x00000200
#define  BIT_COMMENT_BLOCK      0x00000400
#define  BIT_COMMENT_LINE       0x00000800
#define  BIT_NEW_SEG            0x00001000
#define  BIT_SEG_CHAR           0x00002000
#define  BIT_EQUAL              0x00004000
#define  BIT_SEMICOLON          0x00008000

#define  MASK_COMMA             0X00000C02
#define  MASK_ESC               0x00000C01
#define  MASK_DOUBLE_QUOTE      0x00000C16
#define  MASK_SINGLE_QUOTE      0x00000C0E
#define  MASK_XLATE             0x00000020
#define  MASK_START_XLATE       0x00000040
#define  MASK_END_XLATE         0x00000080
#define  MASK_OPEN_BRAC         0x00000C20
#define  MASK_CLOSE_BRAC        0x00000C20
#define  MASK_NEW_SEG           0x00001000
#define  MASK_SPACE             0x00002000
#define  MASK_DQUOTE_EQUAL      0x00004C10
#define  MASK_EQUAL             0x00004000
#define  MASK_COMMENT           0x00000C00
#define  MASK_SEMICOLON         0x00008000

#define  RESULT_COMMA           0x00000002
#define  RESULT_COMMENT_BLOCK   0x00000400
#define  RESULT_COMMENT_LINE    0x00000800
#define  RESULT_ESC             0x00000001
#define  RESULT_DOUBLE_QUOTE    0x00000006
#define  RESULT_SINGLE_QUOTE    0x00000006
#define  RESULT_DQUOTE_EQUAL    0x00004000

#define  CLR_ESC                0xFFFFFFFE
#define  CLR_XLATE              0xFFFFFFDF
#define  CLR_START_XLATE        0xFFFFFFBF
#define  CLR_END_XLATE          0xFFFFFF7F
#define  CLR_DOUBLE_QUOTE       0xFFFFFFF7
#define  CLR_SINGLE_QUOTE       0xFFFFFFEF
#define  CLR_FWD_SLASH          0xFFFFFEFF
#define  CLR_ASTRISK            0xFFFFFDFF
#define  CLR_COMMENT_LINE       0xFFFFF7FF
#define  CLR_COMMENT_BLOCK      0xFFFFFBFF
#define  CLR_NEW_SEG            0xFFFFEFFF
#define  CLR_SEG_CHAR           0xFFFFDFFF
#define  CLR_EQUAL              0xFFFFBFFF
#define  CLR_WORD               0x00000000
#define  CLR_SEMICOLON          0xFFFF7FFF

extern UCHAR       szMF_StartQuoteBlock[7] ;
extern UCHAR       szMF_EndQuoteBlock[8] ;
extern UCHAR       szMF_SingleQuote[7] ;
extern UCHAR       szMF_EscapedSingleQuote[8] ;
extern UCHAR       szCONTEXT_START[9] ;
extern UCHAR       szCONTEXT_END[10] ;
extern BOOL        bDBCSSrc ;
extern BOOL        bDBCSTgt ;

       UCHAR       szMF_CheckQuoteBlock[7] = "<TWBC>";
       UCHAR       szMF_CheckQuoteBlockStart[8] = "<TWBCS>";
       UCHAR       szMF_CheckQuoteBlockEnd[8] = "<TWBCE>";

#define     MF_NONE      0
#define     MF_ALL       1
#define     MF_VAR       2


int parseJavaResource(
                     char* sourceName, 
                     short sType, 
                     BOOL   bCheckMessageFormat, 
                     HWND hwndSlider)
{

    FILE  *sourceFile_ptr = NULL;
    FILE  *tempFile_ptr = NULL;
    FILE  *prevNewLine_ptr = NULL;
    char  dbChar[6];
    char  *tempName = "javares.mjc";
    char  szPrevText[80];
    char  szTempText[1024] ;
    char  szLRBName[256] ;
    char  *pszBkSrcFile;
    char  *pszWorkFile;
    short newChar;
    short tempChar, tempChar2 ;
    char *QFX = ":eqfn.:qff n=.";
    char *EQFX = ":eqff.:qfn n=.";
    char *newSeg = ":eqff.:qff n=.";
    char *commentBreak = ":eqfn.:qfn n=.";
    char *newString = "newString[]{";
    char *ptrChar ;
    long state_word = 0;
    long prev_state_word = 0;
    int  cnt2k = 0;
    int rc = 0;
    int brace_depth =0;
    float k_factor;
    long file_length;
    long slider_cnt;
    long file_percent;
    long file_save, file_save2;
    long outfile_save;
    long file_PrevNewLine = 0;
    USHORT usHtmlTagType ;
    int wait_double_quote=0;
    int wait_double_quote_delay=0;
    int newStringIndex=0;
    int sInNonTransSection = 0 ;
    int u, u2 ;
    BOOL found_LRB_keyword = FALSE;
    BOOL no_break = FALSE;
    BOOL bLRB_Standard = TRUE;
    BOOL bFoundCarriageReturn = FALSE;
    BOOL bCheckUTF8Bom = TRUE ;


    SHORT    sMF_Type ;
    SHORT    sMF_SkipNextMessage ;
    BOOL     bMF_StringVars = FALSE ; 
    BOOL     bMF_AddComment = FALSE ; 
    BOOL     bMF_AddCommentEOL = FALSE ; 
    BOOL     bMF_AdjustQuotes = FALSE ; 
    BOOL     bMF_Find1stQuote = TRUE ; 

    sourceFile_ptr = fopen (sourceName,"rb");
    tempFile_ptr = fopen(tempName,"wb");
    EQFSETSLIDER(hwndSlider, 0); // initialize slider to zero

    // Get file length and calculate mult factor for TM/2 slider
    fseek(sourceFile_ptr,0L,SEEK_END);
    file_length = ftell(sourceFile_ptr);
    fseek(sourceFile_ptr,0L,SEEK_SET);
    k_factor= 100.0/ file_length;
    file_percent = file_length/10;

    rc = 0;
    cnt2k = 0;
    slider_cnt = 0;
    state_word = (state_word & CLR_WORD);
    prev_state_word = (state_word & CLR_WORD);

    if ( bCheckMessageFormat )                /* Managing single quotes?      */
       sMF_Type = MF_VAR ;                    /* Assume modify squotes        */
    else
       sMF_Type = MF_NONE ;                   /* Assume leave squotes alone   */
    bMF_AddComment = TRUE ;                   /* Add if not found at beginning*/
    sMF_SkipNextMessage = 0 ;
    szLRBName[0] = 0 ;


    fputs(":qfn n=1.", tempFile_ptr);       /* Turn translation off at begining of file */
    while (((newChar = fgetc(sourceFile_ptr)) != EOF)) {
        // Update slider every 10% of the file size
        if (slider_cnt > file_percent) {
            EQFSETSLIDER(hwndSlider, (USHORT)(k_factor * ftell(sourceFile_ptr))); /* Adjust slider by file pos * k_factor to get percent done */
            slider_cnt = 0;
        }
        slider_cnt++;

        if ( bCheckUTF8Bom ) {                     
           bCheckUTF8Bom = FALSE ;
           if ( (char)newChar == (char)'\xEF' ) {
              file_save = ftell( sourceFile_ptr ) ; 
              tempChar = fgetc(sourceFile_ptr) ;
              if ( (char)tempChar == (char)'\xBB' ) {
                 tempChar = fgetc(sourceFile_ptr) ;
                 if ( (char)tempChar == (char)'\xBF' ) {
                    slider_cnt += 2 ;
                    continue ;
                 }
              }
              fseek( sourceFile_ptr, file_save, 0 ) ; 
           }
        }


        if ( ( bDBCSSrc          ) &&        /* DBCS chars are 3 byte UTF-8 */
             ( (UCHAR)newChar >= (UCHAR)'\xE3' ) &&        
             ( newChar != EOF    ) ) {
                 dbChar[3]= newChar;
           if ((state_word & MASK_NEW_SEG) > 0) {       //kml - P006808 do segementation break if indicating
               fputs(newSeg, tempFile_ptr);             /* output proper EQFX  QFX to start a new seg */
               cnt2k = 0;                               /* reset the 2k segment count */
               state_word = (state_word & CLR_NEW_SEG);  /* clear the new seg bit */
               state_word = (state_word & CLR_SEG_CHAR); /* DAW: clear the new seg char bit */
           }
           dbChar[0]= newChar;
           fputc(newChar, tempFile_ptr);        /* Write UTF-8 byte 1 */
           if ( (UCHAR)newChar >= (UCHAR)'\xE0' ) 
              u = 3 ; 
           else
              if ( (UCHAR)newChar >= (UCHAR)'\xF0' ) 
                 u = 4 ; 
              else
                 u = 2 ;
           for( u2=1 ; u2<u ; ++u2 ) {
              newChar = fgetc (sourceFile_ptr); 
              fputc(newChar, tempFile_ptr);     /* Write UTF-8 byte */
              dbChar[u2]= newChar;
              ++cnt2k ;
           }
           dbChar[u2] = 0 ;

           if (IsDBCSSentenceEnd(dbChar, 2)) {
              state_word = (state_word | BIT_SEG_CHAR);   /* set new segment char bit */
              state_word = (state_word | BIT_NEW_SEG);    /* DAW: set new segment bit */
           }
           continue;
        }

        cnt2k++;
        if ((cnt2k > 1600) &&                     /* make sure no seg > 2k */
            ((UCHAR)newChar != (UCHAR)'\x0A')) {  /* Do not split x'0D0A'  */
            if ((state_word & BIT_XLATE) > 0)     /* If in a translatable segment */
               fputs(newSeg, tempFile_ptr);       /* Break trans segment          */  
            else
               fputs(commentBreak, tempFile_ptr); /* Break non-trans segment      */
            cnt2k = 0;
        }
        if ( newChar == '"' ) 
           outfile_save = ftell( tempFile_ptr ) ; 


        fputc(newChar, tempFile_ptr);


        wait_double_quote_delay=0;  // clr quote_delay    The quote delay is used to prevent the end variable quote from being processed.
        no_break = FALSE;
        if (((state_word & MASK_ESC) == RESULT_ESC) && (newChar != 'n' || newChar != 'r')) {    // IF ESC SEQ SKIP THE NEXT CHAR
            state_word = (state_word & CLR_ESC);

        } else {      // if its \n or \r we need to still process
            state_word = (state_word & CLR_ESC);
            if ((state_word & MASK_SPACE) > 0) {              /* if Previous char was a new seg char " */
                state_word = (state_word | BIT_NEW_SEG);   /* set new segment bit */
                no_break = TRUE;
            }

            if ((newChar == '"') &&  // check for a double quote
                ((state_word & MASK_COMMENT) == 0) ) {     /* if not in a comment  */
                if (wait_double_quote == 1) {   // If wait_double_quote is set (means your in a doublequoted variable name)
                    wait_double_quote_delay=1;   // Then this is the end quote and should not be processed.
                                                 /* Identify string ID for context  5-25-11 */
                    fseek( tempFile_ptr, outfile_save, 0 ) ;  /* remove " from output */
                    fputs( szCONTEXT_END,tempFile_ptr ) ;     /* Write "<TWBCTX>"     */
                    fputc( newChar, tempFile_ptr ) ;          /* Rewrite "            */
                } else {
                }
                wait_double_quote=0;  // clear the variable if set or not.
            }
            if ( ( wait_double_quote != 1 ) &&         // If not in a non-translatable double quote, and
                 ( wait_double_quote_delay != 1 ) ) {  // If not in a double quote delay
                // then dont process the char

                if ( ( (state_word & MASK_DOUBLE_QUOTE) == RESULT_DOUBLE_QUOTE ) && 
                     ( newStringIndex <= 20 ) &&
                     ( ! isspace( newChar ) ) ) {
                   szPrevText[newStringIndex++] = newChar ;
                }

                switch (newChar) {                                             // else process the char

                case '{':                                  /* found { */
                    if ((state_word & MASK_OPEN_BRAC) == 0) {        /* if your in a xlate segment or comment do nothing */
                        if ((state_word & BIT_EQUAL) != 0) {
                            state_word = (state_word & CLR_WORD);      /* else reset state word and start all over*/
                            state_word = (state_word | BIT_COMMA);
                            state_word = (state_word | BIT_OPEN_BRAC);
                        } else {
                            state_word = (state_word & CLR_WORD);      /* else reset state word and start all over*/
                            state_word = (state_word | BIT_OPEN_BRAC);

                            if ( newStringIndex == strlen(newString) ) { 
                               szPrevText[newStringIndex] = 0 ;
                               if ( ! strcmp( szPrevText, newString ) )
                                  state_word = (state_word | BIT_COMMA);
                            }
                        }
                        brace_depth++; 
                        newStringIndex = 0 ; 
                        bMF_Find1stQuote = TRUE ;
                        if ( bMF_AddComment ) {
                           bMF_AddComment = FALSE ;
                           bMF_AddCommentEOL = TRUE ;
                        }
                    } else {
                       if ((state_word & BIT_XLATE) > 0) {       /* if in a translatable segment */
                          HandleLRBChoiceVariables(sourceFile_ptr, tempFile_ptr) ;
                       }
                    }
                    break;

                case '}':                                   /* found } */
                    if ((state_word & MASK_CLOSE_BRAC) == 0) {      /* if your in a xlate segment or comment do nothing */
                        state_word = (state_word & CLR_WORD); /* else reset state word and start all over*/
                        brace_depth--;
                        newStringIndex = 0 ;
                        if ( ( sMF_Type == MF_VAR ) &&      /* If special variable processing  */
                             ( sMF_SkipNextMessage == 0 ) &&
                             ( ! bMF_Find1stQuote ) ) 
                           fprintf( tempFile_ptr, szMF_CheckQuoteBlockEnd ) ;
                        sMF_SkipNextMessage = 0 ;
                    }
                    break;

                case ',':                                     /* found , */
                    if ((state_word & MASK_COMMA) == RESULT_COMMA) {    /* if your in an open brac and not a comment */
                        state_word = (state_word | BIT_COMMA);        /* set valid comma */
                    }
                    newStringIndex = 0 ;
                    break;
                    /* found " */
                case '"':
                    if (((state_word & MASK_DOUBLE_QUOTE) == RESULT_DOUBLE_QUOTE)) {
                                                               /* if your in a '{' and ',' and not in a single quote or a comment and */
                        if ((state_word & MASK_XLATE) > 0) {              /* if your in xlate segment then must be the end " */
                                                                          /* end of translatable segment */
                            file_save = ftell(sourceFile_ptr);      
                            while( ( (tempChar = fgetc(sourceFile_ptr)) != EOF ) ) {
                               if ( ! isspace(tempChar) ) {
                                  if ( ( tempChar == '/' ) &&       /* Ignore '//' comment */
                                       ( tempChar = fgetc(sourceFile_ptr)) &&
                                       ( ( tempChar == '/' ) ||
                                         ( tempChar == '*' ) ) ) {  /* Ignore '/*' comment */
                                     if ( tempChar == '/' ) {       /* Ignore '//' comment */
                                        while( ( (tempChar = fgetc(sourceFile_ptr)) != EOF ) ) {
                                           if ( tempChar == '\n' ) 
                                              break ;
                                        }
                                     } else {                       /* Ignore '/*' comment */
                                        while( ( (tempChar = fgetc(sourceFile_ptr)) != EOF ) ) {
                                           if ( ( tempChar == '*' ) &&
                                                ( tempChar = fgetc(sourceFile_ptr)) &&
                                                ( tempChar == '/' ) )
                                              break ;
                                        }
                                     }
                                     continue ;
                                  }
                                  if ( tempChar != '+' ) 
                                     break ;
                               }
                            }
                            if ( tempChar == '"' ) {
                               fseek( tempFile_ptr, outfile_save, 0 ) ; 
                               continue ;
                            }
                            fseek( sourceFile_ptr, file_save, 0 ) ;  /* Reset file reading pos */

                            state_word = (state_word | BIT_END_XLATE);   
                            state_word = (state_word & CLR_XLATE);
                            state_word = (state_word & CLR_DOUBLE_QUOTE);
                        } else {                                           /* else its the start of a translatable segment */
                            if ((state_word & MASK_COMMENT) == 0) {        /* if not in a comment */

                               file_save = ftell(sourceFile_ptr);       /* Determine if no text */
                               tempChar = fgetc(sourceFile_ptr);
                               if ( tempChar == '"' ) {                 /* Found 2 double quotes */
                                  file_save2 = ftell(sourceFile_ptr); 
                                  tempChar2 = fgetc(sourceFile_ptr);
                                  if ( tempChar2 != '"' ) {             /* If not 3 double quotes, then null text */
                                     fputs(commentBreak, tempFile_ptr); /* Create dummy segment so */
                                     fputs(commentBreak, tempFile_ptr); /*  segment numbers match. */
                                     fputc(tempChar, tempFile_ptr);     /* Write ending quote */
                                     fseek( sourceFile_ptr, file_save2, 0 ) ;  /* Reset file reading pos */
                                     continue;
                                  }
                               }
                               fseek( sourceFile_ptr, file_save, 0 ) ;  /* Reset file reading pos */

                               state_word = (state_word | BIT_START_XLATE);
                               state_word = (state_word | BIT_XLATE);
                               state_word = (state_word | BIT_DOUBLE_QUOTE);
                            }
                        }

                    }
                    else if (((state_word & MASK_DQUOTE_EQUAL) == RESULT_DQUOTE_EQUAL) &&
                             (brace_depth < 2)) { 

                        /* if you are after an "="  AND not in a single quote OR a comment AND you are within first level of braces */
                        if ((state_word & MASK_XLATE) > 0) {              /* if your in xlate segment then must be the end " */

                            file_save = ftell(sourceFile_ptr); 
                            while( ( (tempChar = fgetc(sourceFile_ptr)) != EOF ) ) {
                               if ( ! isspace(tempChar) ) {
                                  if ( ( tempChar == '/' ) &&       /* Ignore '//' comment */
                                       ( tempChar = fgetc(sourceFile_ptr)) &&
                                       ( ( tempChar == '/' ) ||
                                         ( tempChar == '*' ) ) ) {  /* Ignore '/*' comment */
                                     if ( tempChar == '/' ) {       /* Ignore '//' comment */
                                        while( ( (tempChar = fgetc(sourceFile_ptr)) != EOF ) ) {
                                           if ( tempChar == '\n' ) 
                                              break ;
                                        }
                                     } else {                       /* Ignore '/*' comment */
                                        while( ( (tempChar = fgetc(sourceFile_ptr)) != EOF ) ) {
                                           if ( ( tempChar == '*' ) &&
                                                ( tempChar = fgetc(sourceFile_ptr)) &&
                                                ( tempChar == '/' ) )
                                              break ;
                                        }
                                     }
                                     continue ;
                                  }
                                  if ( tempChar != '+' ) 
                                     break ;
                               }
                            }
                            if ( tempChar == '"' ) {
                               fseek( tempFile_ptr, outfile_save, 0 ) ; 
                               continue ;
                            }
                            fseek( sourceFile_ptr, file_save, 0 ) ;  /* Reset file reading pos */

                            state_word = (state_word | BIT_END_XLATE);
                            state_word = (state_word & CLR_XLATE);
                            state_word = (state_word & CLR_DOUBLE_QUOTE);
                        } else {                                          /* else its the start of a translatable segment */
                            if ((state_word & MASK_COMMENT) == 0) {        /* if not in a comment  */

                               file_save = ftell(sourceFile_ptr);       /* Determine if no text */
                               tempChar = fgetc(sourceFile_ptr);
                               if ( tempChar == '"' ) {                 /* Found 2 double quotes */
                                  file_save2 = ftell(sourceFile_ptr); 
                                  tempChar2 = fgetc(sourceFile_ptr);
                                  if ( tempChar2 != '"' ) {             /* If not 3 double quotes, then null text */
                                     fputs(commentBreak, tempFile_ptr); /* Create dummy segment so */
                                     fputs(commentBreak, tempFile_ptr); /*  segment numbers match. */
                                     fputc(tempChar, tempFile_ptr);     /* Write ending quote */
                                     fseek( sourceFile_ptr, file_save2, 0 ) ;  /* Reset file reading pos */
                                     continue;
                                  }
                               }
                               fseek( sourceFile_ptr, file_save, 0 ) ;  /* Reset file reading pos */

                               state_word = (state_word | BIT_START_XLATE);
                               state_word = (state_word | BIT_XLATE);
                               state_word = (state_word | BIT_DOUBLE_QUOTE);
                            }
                        }

                    } else {
                        if ((state_word & MASK_COMMENT) == 0) {
                            wait_double_quote=1;
                            fputs( szCONTEXT_START,tempFile_ptr ) ;            /* 5-25-11 */
                        }
                    }
                    break;

                case '\'':                                        /* found ' */
                    if ((state_word & MASK_SINGLE_QUOTE) == RESULT_SINGLE_QUOTE) {  /* if your in a '{' and ',' and not in a double quote or a comment */
                        if ((state_word & MASK_XLATE) > 0) {                     /* if your in xlate segment then must be the end " */

                            file_save = ftell(sourceFile_ptr);            
                            while( ( (tempChar = fgetc(sourceFile_ptr)) != EOF ) ) {
/////                          if ( ( ! isspace(tempChar) ) &&
/////                               ( tempChar != '+'   ) ) 
/////                             break ;
                               if ( ! isspace(tempChar) ) {
                                  if ( ( tempChar == '/' ) &&       /* Ignore '//' comment  */
                                       ( tempChar = fgetc(sourceFile_ptr)) &&
                                       ( ( tempChar == '/' ) ||
                                         ( tempChar == '*' ) ) ) {  /* Ignore '/*' comment  */
                                     if ( tempChar == '/' ) {       /* Ignore '//' comment */
                                        while( ( (tempChar = fgetc(sourceFile_ptr)) != EOF ) ) {
                                           if ( tempChar == '\n' ) 
                                              break ;
                                        }
                                     } else {                       /* Ignore '/*' comment */
                                        while( ( (tempChar = fgetc(sourceFile_ptr)) != EOF ) ) {
                                           if ( ( tempChar == '*' ) &&
                                                ( tempChar = fgetc(sourceFile_ptr)) &&
                                                ( tempChar == '/' ) )
                                              break ;
                                        }
                                     }
                                     continue ;
                                  }
                                  if ( tempChar != '+' ) 
                                     break ;
                               }
                            }
                            if ( tempChar == '\'' ) {
                               fseek( tempFile_ptr, outfile_save, 0 ) ; 
                               continue ;
                            }
                            fseek( sourceFile_ptr, file_save, 0 ) ;  /* Reset file reading pos */

                            state_word = (state_word | BIT_END_XLATE);          /* else its the start of a translatable segment */
                            state_word = (state_word & CLR_XLATE);
                            state_word = (state_word & CLR_SINGLE_QUOTE);
                        } else {                                                /* end of translatable segment */
                            if ((state_word & MASK_COMMENT) == 0) {        /* if not in a comment  */
                               state_word = (state_word | BIT_START_XLATE);
                               state_word = (state_word | BIT_XLATE);
                               state_word = (state_word | BIT_SINGLE_QUOTE);
                            }
                        }
                    }
                    break;

                case '\\':                 /* found \ */
                    if ((state_word & BIT_XLATE) > 0) {       /* if in a translatable segment */
                        state_word = (state_word | BIT_ESC);   /* esc seq is valid */
                    }
                    break;

                case '*':                 /* found '*' */
                    if (((state_word & BIT_FWD_SLASH) > 0) &&      /* if next char after forward slash */
                        (((state_word & BIT_COMMENT_LINE) == 0) || //kml - P007108 need to make sure we are not already in a line comment also
                         ( sInNonTransSection == 5 ))) {               /* 8-23-07 */
                        state_word = (state_word | BIT_COMMENT_BLOCK); /* start of block comment */

                        file_save = ftell(sourceFile_ptr);
///    6-28-07          if ( (INT)fgets( szTempText, sizeof(szTempText), sourceFile_ptr ) > 0 ) {
                        while ( (INT)fgets( szTempText, sizeof(szTempText), sourceFile_ptr ) > 0 ) {
                           strupr( szTempText ) ;

                           /*----------------------------------------------------------------*/
                           /*  If special comment which defines a block of source which      */
                           /*  should not be translated, then set up processing.             */
                           /*----------------------------------------------------------------*/
                           if ( strstr( szTempText, "NON-TRANSLATABLE" ) ) {
                              if ( strstr( szTempText, "START" ) )
                                 sInNonTransSection = 3 ;    
                              if ( strstr( szTempText, "END" ) ) 
                                 sInNonTransSection = 4 ; 
                           }

                           /*----------------------------------------------------------------*/
                           /*  If special comment which defines how single quotes should be  */
                           /*  handled because of MessageFormat processing, then set up      */
                           /*  processing.                                                   */
                           /*----------------------------------------------------------------*/
                           if ( strstr( szTempText, "NLS_MESSAGEFORMAT_" ) ) {
                              if ( strstr( szTempText, "NLS_MESSAGEFORMAT_ALL" ) ) {
                                 sMF_Type = MF_ALL ;         /* All quotes are doubled       */
                                 bMF_AddComment = FALSE ; 
                              } else {
                                 if ( strstr( szTempText, "NLS_MESSAGEFORMAT_NONE" ) ) {
                                    sMF_Type = MF_NONE ;     /* No special processing        */  
                                    bMF_AddComment = FALSE ; 
                                 } else {
                                    if ( strstr( szTempText, "NLS_MESSAGEFORMAT_VAR" ) ) {
                                       sMF_Type = MF_VAR ;   /* Based on replacement vars.   */
                                       bMF_AddComment = FALSE ; 
                                    } else 
                                       if ( strstr( szTempText, "NLS_MESSAGEFORMAT_SKIP" ) ) {
                                          sMF_SkipNextMessage = 1 ;
                                       } 
                                 }
                              }
                           }

                           /*----------------------------------------------------------------*/
                           /*  If end of comment, quit looking.                              */
                           /*----------------------------------------------------------------*/
                           if ( strstr( szTempText, "*/" ) ) {
                              break ;
                           }
                        }
                        fseek( sourceFile_ptr, file_save, 0 ) ;   /* Reset file reading pos */
                    } else {
                        state_word = (state_word | BIT_ASTRISK);  /* else just an astrisk */
                        prev_state_word = (prev_state_word & CLR_ASTRISK); /* prevent cycle reset because we have another astrisk */
                    }
                    break;

                case '/':                 /* found '/' */
                    if ((state_word & BIT_XLATE) == 0) {       /* if not in a translatable segment process for comment */
                        if (((state_word & BIT_FWD_SLASH) > 0) ||       /* if set in previous cycle, or         */
                            (((state_word & BIT_COMMENT_LINE) > 0) &&   /* if already in line comment */
                              (((state_word & BIT_ASTRISK) == 0      ) ||
                               ((state_word & BIT_COMMENT_BLOCK) == 0)))) {
                            state_word = (state_word | BIT_COMMENT_LINE);   /* it is a line comment */
                            if ( sInNonTransSection == 5 ) 
                               state_word = (state_word | BIT_FWD_SLASH);                                     /* else set slash */

                            file_save = ftell(sourceFile_ptr);
                            if ( (INT)fgets( szTempText, sizeof(szTempText), sourceFile_ptr ) > 0 ) {
                               fseek( sourceFile_ptr, file_save, 0 ) ;   /* Reset file reading pos */
                               strupr( szTempText ) ;

                               /*----------------------------------------------------------------*/
                               /*  If special comment which defines a block of source which      */
                               /*  should not be translated, then set up processing.             */
                               /*----------------------------------------------------------------*/
                               if ( strstr( szTempText, "NON-TRANSLATABLE" ) ) {
                                  if ( strstr( szTempText, "START" ) ) {
                                     sInNonTransSection = 1 ;          /* Start line comment     */
                                  }
                                  if ( strstr( szTempText, "END" ) ) 
                                     sInNonTransSection = 2 ;          /* End line comment       */
                               }

                               /*----------------------------------------------------------------*/
                               /*  If special comment which defines how single quotes should be  */
                               /*  handled because of MessageFormat processing, then set up      */
                               /*  processing.                                           9-24-04 */
                               /*----------------------------------------------------------------*/
                               if ( strstr( szTempText, "NLS_MESSAGEFORMAT_" ) ) {
                                  if ( strstr( szTempText, "NLS_MESSAGEFORMAT_ALL" ) ) {
                                     sMF_Type = MF_ALL ;         /* All quotes are doubled       */
                                     bMF_AddComment = FALSE ; 
                                  } else
                                  if ( strstr( szTempText, "NLS_MESSAGEFORMAT_NONE" ) ) {
                                     sMF_Type = MF_NONE ;        /* No special processing        */  
                                     bMF_AddComment = FALSE ; 
                                  } else 
                                  if ( strstr( szTempText, "NLS_MESSAGEFORMAT_SKIP" ) ) {
                                     sMF_SkipNextMessage = 1 ;
                                  } else {
                                     sMF_Type = MF_VAR ;         /* Based on replacement vars.   */
                                     bMF_AddComment = FALSE ; 
                                  }
                               }
                            }
                        } else if (((state_word & BIT_ASTRISK) > 0) && ((state_word & BIT_COMMENT_BLOCK) > 0)) {  /* if in a block comment and astrisk is previous char */
                            state_word = (state_word & CLR_COMMENT_BLOCK);                                     /* must be end of block comment */
                            if ( sInNonTransSection == 3 ) {                   /* If end of start block comment */
                               sInNonTransSection = 5 ;                        /*  now in non-trans section     */
                               state_word = (state_word | BIT_COMMENT_LINE);   /* set as a line comment */
                            } else {
                               if ( sInNonTransSection == 4 )                  /* If end of end block comment   */
                                  sInNonTransSection = 0 ;                     /*  then end start/end block     */
                            }
                        } else {
                            state_word = (state_word | BIT_FWD_SLASH);                                     /* else set slash */
                        }
                    }
                    break;

                case '\n':                 /* found '\n' */
                    file_PrevNewLine = ftell(sourceFile_ptr);         /* Save position of newline char    */
                    if ( sInNonTransSection == 1 )                    /* If newline in start line comment */
                       sInNonTransSection = 5 ;                       /*  now in non-trans section        */
                    else
                       if ( sInNonTransSection == 2 )                 /* If newline in end line comment   */  
                          sInNonTransSection = 0 ;                    /*  now in normal trans section     */
                    if ( sInNonTransSection == 0 )                    /* If in normal translatable section*/ 
                       state_word = (state_word & CLR_COMMENT_LINE);  /*  then end line comment           */

                    if ((state_word & MASK_EQUAL) > 0) {              /* if Previous char was an equal char " */
                        prev_state_word = (prev_state_word & CLR_EQUAL); /* prevent equal cycle reset because white space is ok */
                    }
                    if ((state_word & MASK_XLATE) > 0) {              /* if your in xlate segment then " */
//                     bFileSplitMessage = TRUE ;                     /* Text on multiple lines          */
                        if ((state_word & MASK_SPACE) > 0) {          /* if Previous char was a new seg char " */
                            state_word = (state_word | BIT_NEW_SEG);  /* set new segment bit */
                        }
                    }
                    if ( bMF_AddCommentEOL ) {
                       bMF_AddCommentEOL = FALSE ; 
                       if ( bCheckMessageFormat ) 
                          fputs( "/* NLS_MESSAGEFORMAT_VAR */", tempFile_ptr ) ; /* Default= VAR    */
                       else
                          fputs( "/* NLS_MESSAGEFORMAT_NONE */", tempFile_ptr ) ; /* Default= NONE   */
                       if ( bFoundCarriageReturn) 
                          fputs( "\r", tempFile_ptr ) ; 
                       fputs( "\n", tempFile_ptr ) ; 
                    }
                    break;

                case '\r':                 /* found '\r' */
                    bFoundCarriageReturn = TRUE ;
                    if ( sInNonTransSection == 1 )                    /* If newline in start line comment */
                       sInNonTransSection = 5 ;                       /*  now in non-trans section        */
                    else
                       if ( sInNonTransSection == 2 )                 /* If newline in end line comment   */  
                          sInNonTransSection = 0 ;                    /*  now in normal trans section     */
                    if ( sInNonTransSection == 0 )                    /* If in normal translatable section*/ 
                       state_word = (state_word & CLR_COMMENT_LINE);  /*  then end line comment           */

                    if ((state_word & MASK_EQUAL) > 0) {              /* if Previous char was an equal char " */
                        prev_state_word = (prev_state_word & CLR_EQUAL); /* prevent equal cycle reset because white space is ok */
                    }
                    if ((state_word & MASK_XLATE) > 0) {              /* if your in xlate segment then " */
                        if ((state_word & MASK_SPACE) > 0) {              /* if Previous char was a new seg char " */
                            state_word = (state_word | BIT_NEW_SEG);   /* set new segment bit */
                        }
                    }
                    break;

                case '<':                 /* Possible HTML tag */
                    if ( ( (state_word & MASK_XLATE) > 0 ) &&    /* if your in xlate segment then " */
                         ( (state_word & MASK_SPACE) > 0 ) ) {   /* if Previous char was a new seg char " */
                       file_save = ftell(sourceFile_ptr);
                       szTempText[0] = newChar ;
                       if ( (INT)fgets( &szTempText[1], sizeof(szTempText), sourceFile_ptr ) > 0 ) {
                          fseek( sourceFile_ptr, file_save, 0 ) ;   /* Reset file reading pos */
                          if ( IsHTMLTag( szTempText, &usHtmlTagType ) ) {        
                             fseek(tempFile_ptr, (ftell(tempFile_ptr)-1), SEEK_SET); /* backup file point 1 the char has already been output */
                             fputs(newSeg, tempFile_ptr);    /* output proper EQFX  QFX to start a new seg */
                             fputc(newChar, tempFile_ptr);   /* replace the char now */
                             cnt2k = 0;                      /* reset the 2k segment count */
                          }
                       }
                    }
                    break;

                case '.':                 /* found '.' */
                    if ((state_word & MASK_XLATE) > 0) {              /* if your in xlate segment then " */
                        state_word = (state_word | BIT_SEG_CHAR);   /* set new segment bit */
                    }
                    break;

                case '?':                 /* found '?' */
                    if ((state_word & MASK_XLATE) > 0) {              /* if your in xlate segment then " */
                        state_word = (state_word | BIT_SEG_CHAR);   /* set new segment bit */
                    }
                    break;

                case '!':                 /* found '!' */
                    if ((state_word & MASK_XLATE) > 0) {              /* if your in xlate segment then " */
                        state_word = (state_word | BIT_SEG_CHAR);   /* set new segment bit */
                    }
                    break;

                case ' ':                 /* found ' ' */
                    if ((state_word & MASK_SPACE) > 0) {              /* if Previous char was a new seg char " */
                        state_word = (state_word | BIT_NEW_SEG);   /* set new segment bit */
                    }
                    if ((state_word & MASK_EQUAL) > 0) {              /* if Previous char was an equal char " */
                        prev_state_word = (prev_state_word & CLR_EQUAL); /* prevent equal cycle reset because white space is ok */
                    }
                    break;

                case '=':                 /* found '=' */
                    if (((state_word & MASK_COMMENT) != RESULT_COMMENT_LINE) && /* KML- P006808 if not in a line/block comment then set bit */
                        ((state_word & MASK_COMMENT) != RESULT_COMMENT_BLOCK)) {
                        state_word = (state_word | BIT_EQUAL);   /* set equal bit */
                    }
                    break;

                case ';':                 /* found ';' */
                    if (((state_word & MASK_XLATE)   == 0) &&           /* if your not  in an xlate segment */
                        ((state_word & MASK_COMMENT) == 0)) {           /* if not in a comment           */
                        if ((state_word & MASK_EQUAL) > 0) {            /* if your after an = then " */
                            state_word = (state_word | BIT_SEMICOLON);  /* the semicolon must be the end of the = segment */

                            if ( ( sMF_Type == MF_VAR ) &&      /* If special variable processing          */
                                 ( sMF_SkipNextMessage == 0 ) &&
                                 ( ! bMF_Find1stQuote ) ) 
                               fprintf( tempFile_ptr, szMF_CheckQuoteBlockEnd ) ;
                        }
                        sMF_SkipNextMessage = 0 ;
                    }
                    break;

                case '_':                 /* found '_' */             
                    if ( (state_word & MASK_OPEN_BRAC) == 0 ) {       /* if in a xlate segment or comment, do nothing */
                       if ( ( brace_depth == 0    ) &&
                            ( ! found_LRB_keyword ) ) {
                          file_save = ftell(sourceFile_ptr);
                          if ( (INT)fgets( szTempText, sizeof(szTempText), sourceFile_ptr ) > 0 ) {
                             if ( ( strncmp( szTempText, "en", 2 ) ) ||
                                  ( ( ! isspace( szTempText[2] ) ) &&
                                    ( szTempText[2] != '_' ) ) ) {
                                fseek( sourceFile_ptr, file_save, 0 ) ;  /* Reset file reading pos */
                                break ;
                             }
                          }
                          do {
                             if ( strstr( szTempText, "ListResourceBundle" ) ) {
                                found_LRB_keyword = TRUE ;
                                break ;
                             }
                             if ( ( strchr( szTempText, '{' ) ) ||
                                  ( strchr( szTempText, ';' ) ) ) {
                                break ;
                             }
                          } while( (INT)fgets( szTempText, sizeof(szTempText), sourceFile_ptr ) > 0 ) ;
                        
                          fseek( sourceFile_ptr, file_save, 0 ) ;  /* Reset file reading pos */
                          if ( found_LRB_keyword ) {
                             fputs(QFX, tempFile_ptr);                  /* Start translatable */
                             while( (newChar=fgetc(sourceFile_ptr)) != EOF ) {
                                if ( isspace( newChar ) ) {
                                   fputs(EQFX, tempFile_ptr);           /* End translatable */
                                   fputc(newChar, tempFile_ptr);
                                   break ;
                                }
                                fputc(newChar, tempFile_ptr);
                             }
                        
                             file_save = ftell(sourceFile_ptr);
                             fseek( sourceFile_ptr, file_PrevNewLine, 0 ) ;  /* Read entire line */
                             if ( (INT)fgets( szTempText, sizeof(szTempText), sourceFile_ptr ) > 0 ) {
                                ptrChar = strstr( szTempText, "_en" ) ;
                                if ( ptrChar ) {
                                   *(ptrChar+3) = 0 ;
                                   for( ; ptrChar>szTempText && !isspace(*(ptrChar-1)) ; --ptrChar ) ;
                                   strcpy( szLRBName, ptrChar ) ;
                                }
                             }
                             fseek( sourceFile_ptr, file_save, 0 ) ;  /* Reset file reading pos */
                          }
                       }
                       if ( ( brace_depth == 1 ) &&
                            ( szLRBName[0]     ) ) {
                          file_save = ftell(sourceFile_ptr);
                          if ( (INT)fgets( szTempText, sizeof(szTempText), sourceFile_ptr ) > 0 ) {
                             if ( ( strncmp( szTempText, "en", 2 ) ) ||
                                  ( ( ! isspace( szTempText[2] ) ) &&
                                    ( szTempText[2] != '_'       ) &&
                                    ( szTempText[2] != '('       ) ) ) {
                                fseek( sourceFile_ptr, file_save, 0 ) ;  /* Reset file reading pos */
                                break ;
                             }
                          }
                          fseek( sourceFile_ptr, file_PrevNewLine, 0 ) ;  /* Read entire line */
                          if ( (INT)fgets( szTempText, sizeof(szTempText), sourceFile_ptr ) > 0 ) {
                             ptrChar = strstr( szTempText, szLRBName ) ;
                             fseek( sourceFile_ptr, file_save, 0 ) ;  /* Reset file reading pos */
                             fputs(QFX, tempFile_ptr);                  /* Start translatable */
                             while( (newChar=fgetc(sourceFile_ptr)) != EOF ) {
                                if ( ( isspace( newChar ) ) ||
                                     ( newChar == '('     ) ) {
                                   fputs(EQFX, tempFile_ptr);           /* End translatable */
                                   fputc(newChar, tempFile_ptr);
                                   break ;
                                }
                                fputc(newChar, tempFile_ptr);
                             }
                          } else {
                             fseek( sourceFile_ptr, file_save, 0 ) ;  /* Reset file reading pos */
                          }
                       }
                    }
                    break;

                default:
                    break;

                }

                state_word =((state_word & ((prev_state_word ^ state_word) | CLR_FWD_SLASH)));  /* reset BIT_FWD_SLASH after one cycle  */
                state_word =((state_word & ((prev_state_word ^ state_word) | CLR_ASTRISK)));  /* reset BIT_ASTRISK after one cycle  */
                state_word =((state_word & ((prev_state_word ^ state_word) | CLR_SEG_CHAR)));  /* reset BIT_SEG_CHAR after one cycle  */
                if ((state_word & MASK_SEMICOLON) > 0) {
                    state_word =((state_word & ((prev_state_word ^ state_word) | CLR_EQUAL)));  /* reset BIT_EQUAL if not in a xlate segment */
                    state_word = (state_word &  CLR_SEMICOLON);
                }
                /*************************************************/
                /*                                               */
                /*  Check if there is a segmentation change      */
                /*                                               */
                /*************************************************/

                if ((state_word & MASK_START_XLATE) > 0) {      /* if this is the start of an xlateable segment then */


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
                        ( sMF_SkipNextMessage == 0 ) ) {
                      if ( sMF_Type == MF_ALL ) {
                         bMF_StringVars = TRUE ;       /* Force special processing   */
                         bMF_AdjustQuotes = TRUE ; 
                         fprintf( tempFile_ptr, szMF_StartQuoteBlock ) ;
                      } else {
                         bMF_StringVars = TRUE ;       /* Possible special process   */
                         bMF_AdjustQuotes = TRUE ; 
                         if ( bMF_Find1stQuote ) {                    
                            fprintf( tempFile_ptr, szMF_CheckQuoteBlockStart ) ;
                            bMF_Find1stQuote = FALSE ;
                         }
                         fprintf( tempFile_ptr, szMF_CheckQuoteBlock ) ;
                      }
                   }


                    fputs(QFX, tempFile_ptr);                  /* enter the proper QFX tags */
                    cnt2k = 0;                                 /* reset the 2k segment count */
                    state_word = (state_word & CLR_START_XLATE);
                } else if ((state_word & MASK_END_XLATE) > 0) {   /* if this is the end of an xlateable segment then */
                    fseek(tempFile_ptr, (ftell(tempFile_ptr)-1), SEEK_SET); /* backup file point 1 the quote has already been output */
                    fputs(EQFX, tempFile_ptr);                              /* and we need to insert EQFX before it */

                    if ( bMF_StringVars ) {               /* If text contains variables,  */
                       bMF_StringVars = FALSE ;           /*  then mark end of squote text*/
                       fprintf( tempFile_ptr, szMF_EndQuoteBlock ) ;
                    }

                    fputc(newChar, tempFile_ptr);                           /* replace the quote now */
                    cnt2k = 0;                                              /* reset the 2k segment count */
                    state_word = (state_word & CLR_END_XLATE);
                    state_word = (state_word & CLR_NEW_SEG);
                } else if ((state_word & MASK_NEW_SEG) > 0) {   /* if there is a new segment break ('.' '?') */
                    if (chk_good_break_spot(newChar)) {
                        if (no_break) {
                            no_break = FALSE;
                            state_word = (state_word & CLR_NEW_SEG);                   /* clear the new seg bit */
                        } else {
                            fseek(tempFile_ptr, (ftell(tempFile_ptr)-1), SEEK_SET); /* backup file point 1 the char has already been output */
                            fputs(newSeg, tempFile_ptr);             /* output proper EQFX  QFX to start a new seg */
                            fputc(newChar, tempFile_ptr);                           /* replace the char now */
                            cnt2k = 0;                                              /* reset the 2k segment count */
                            state_word = (state_word & CLR_NEW_SEG);                   /* clear the new seg bit */
                        }
                    }
                }

                prev_state_word = state_word;
            } else {
                if (newChar == '\\') { // check for esc seq while in double quoted variables so we can ignore the next char.
                    state_word = (state_word | BIT_ESC);   /* esc seq is valid */
                }
            }

        }
        if ( (UCHAR)newChar >= (UCHAR)'\xC0' ) {
           if ( (UCHAR)newChar >= (UCHAR)'\xE0' ) 
              u = 3 ; 
           else
              if ( (UCHAR)newChar >= (UCHAR)'\xF0' ) 
                 u = 4 ; 
              else
                 u = 2 ;
           for( u2=1 ; u2<u ; ++u2 ) {
              newChar = fgetc (sourceFile_ptr); 
              fputc(newChar, tempFile_ptr);     /* Write UTF-8 byte */
              ++cnt2k ;
           }
           continue ;
        }
    }
    fclose(sourceFile_ptr);
    fclose(tempFile_ptr);
    tempFile_ptr = fopen(tempName,"a");
    fputs(":eqfn.", tempFile_ptr);          /* add the ending EQFN tag */
    fclose(tempFile_ptr);

    if ( bMF_AdjustQuotes ) {
       MFAdjustQuotes( tempName, sourceName ); 
    } else {
       rc = CopyFile(tempName, sourceName, FALSE); // COPIES MODIFIED TEMP BACK TO SOURCE
    }

    remove(tempName);
    EQFSETSLIDER(hwndSlider,100);
    return TRUE;
}



/*************************************************************************/
/*  Export the JDK 1.1 ListResourceBundle file.                          */
/*************************************************************************/
USHORT  ExportLRBFile(
                  PSZ    pszSource,
                  PSZ    pszTarget  )
{

    FILE     *fSource, *fTarget ;
    UCHAR    *szIn = NULL ;
    UCHAR    szTag[10] ;
    UCHAR    *ptrChar, *ptrTemp ;
    short    NextChar;
    SHORT    i ;
    USHORT   usSegmentType ;
    USHORT   usMaxLineLength = 1800 ;
    USHORT   usLineLength ;
    LONG     lFilePos ;
    BOOL     bMF_InBlock = FALSE ; 



    fSource = fopen ( pszSource, "rb" ) ;     /* Open input segmented file.    */
    if ( ! fSource ) return 0 ;
    fTarget = fopen ( pszTarget, "wb+" ) ;    /* Open output target file.      */
    if ( ! fTarget ) return 0 ;

    szIn = malloc( MAX_RCD_LENGTH+300 ) ;                      
    if ( ! szIn ) return 0 ;

    usSegmentType = SEGMENT_NONTRANS ;       /* Start as undefined segment.   */

    /*------------------------------------------------------------------------*/
    /*  Read all records within this file.                                    */
    /*------------------------------------------------------------------------*/
    while ( (INT)fgets( szIn, MAX_RCD_LENGTH, fSource ) > 0 ) {

        /*--------------------------------------------------------------------*/
        /*  Determine whether in translatable or non-translatable segment.    */
        /*--------------------------------------------------------------------*/
        usLineLength = 0 ;
        for( i=0 ; szIn[i] ; ++i ) {                           
           ++usLineLength ;
           if ( szIn[i] == ':' ) {
              strncpy( szTag, &szIn[i+1], 3 ) ;
              szTag[3] = 0 ; 
              strupr( szTag ) ;
              if ( ! strncmp( szTag, "QF", 2 ) ) {
                 if ( ! strncmp( szTag, "QFN", 3 ) ) 
                    usSegmentType = SEGMENT_NONTRANS ;
                 else
                    usSegmentType = SEGMENT_TRANS ;
              }
           }
   
           /*-----------------------------------------------------------------*/
           /*  If found newline and in translatable segment, then split line  */
           /*  to create a concatenated line to avoid too long records.       */
           /*-----------------------------------------------------------------*/
           if ( ( usSegmentType == SEGMENT_TRANS     ) &&
                ( usLineLength > usMaxLineLength     ) &&
                ( ! strncmp( &szIn[i],   "\\n", 2  ) ) &&
                (   strncmp( &szIn[i+2], "\\n", 2  ) ) ) {
              if ( ( ! strncmp( &szIn[i+2], ":EQF", 4 ) ) ||
                   ( ! strncmp( &szIn[i+2], ":eqf", 4 ) ) ) {
                 ptrChar = strchr( &szIn[i+8], '.' ) ;
                 if ( ( ptrChar ) &&
                      ( *(ptrChar+1) == '\"' ) ) 
                    continue ;
              }
              memmove( &szIn[i+7], &szIn[i+2], strlen(&szIn[i+2])+1 ) ;
              strncpy( &szIn[i+2], "\" +\n\"", 5 ) ;
              i += 6 ;
              usLineLength = 0 ;
           }
        }

        /*------------------------------------------------------------------------*/
        /*  Remove extra TWB tags to restore choice variable format.              */
        /*------------------------------------------------------------------------*/
        for( ptrChar=strstr(szIn,"{TWB$") ;  ptrChar ; ptrChar=strstr(ptrChar,"{TWB$") ) {
           memmove( ptrChar, ptrChar+5, strlen(ptrChar+5)+1 ) ;
        }
        for( ptrChar=strstr(szIn,"$TWB}") ;  ptrChar ; ptrChar=strstr(ptrChar,"$TWB}") ) {
           memmove( ptrChar, ptrChar+5, strlen(ptrChar+5)+1 ) ;
        }

        /*------------------------------------------------------------------------*/
        /*  Handle single quotes in text going thru MessageFormat API.            */
        /*------------------------------------------------------------------------*/
        HandleSingleQuotes( szIn, FALSE, &bMF_InBlock ) ;

        /*------------------------------------------------------------------------*/
        /*  Remove context tags.                                                  */
        /*------------------------------------------------------------------------*/
        RemoveContextInfo( szIn ) ;

        /*------------------------------------------------------------------------*/
        /*  Reflow text onto single line if this is required.                     */
        /*------------------------------------------------------------------------*/
        if ( usSegmentType == SEGMENT_TRANS ) {
           lFilePos = ftell( fSource ) ;
           if ( ((NextChar = fgetc(fSource)) != EOF ) &&
                ( NextChar == (INT)':' ) ) {             /* :EQF tag, end of text */
              for( i=strlen(szIn)-1 ; i>0 ; --i ) {      /* Remove newline chars  */
                 if ( strchr( "\n\r", szIn[i] ) ) 
                    szIn[i] = 0 ;
                 else
                    break ;
              }
           }
           fseek( fSource, lFilePos, 0 ) ;              /* Reset file reading pos */
        }

        fputs( szIn, fTarget ) ;              /* Write out this output record. */
    }

    fclose ( fSource ) ;                     /* Close input segmented file.   */
    fclose ( fTarget ) ;                     /* Close output target file.     */

    if ( szIn )
        free( szIn ) ;

    return 1 ;
}



int chk_good_break_spot(short newChar)
{
    int found_spot;

    /* Make sure you have a valid char to start a new segment on */

    switch (newChar) {

    case '.':
        found_spot = FALSE;   /* not valid */
        break;

    case ' ':
        found_spot = FALSE;  /* not valid */
        break;

    case '\n':
        found_spot = FALSE;  /* not valid */
        break;

    case '\r':
        found_spot = FALSE;  /* not valid */
        break;

    case '\t':                           
        found_spot = FALSE;  /* not valid */
        break;

    case '?':
        found_spot = FALSE;   /* not valid */
        break;

    case '!':
        found_spot = FALSE;  /* not valid */
        break;

    case '\\':
        found_spot = FALSE;  /* not valid */
        break;

    default:
        found_spot = TRUE;  /* valid ok to start a new segment */
        break;

    }

    return found_spot;
}



/*************************************************************************/
/*  Modify "choice" variables to properly identify translatable text.    */
/*  Example:                                                             */
/*  {0,choice,0#0 items|1#{0,number,integer} item|1<{0,number,integer} items} */
/*  {1,choice,-1#|0# at location {1}}                                    */
/*************************************************************************/
void  HandleLRBChoiceVariables(
                       FILE  *fInput, 
                       FILE  *fOutput ) 
{
    CHAR     szTemp[512] ;
    CHAR     *ptrChar;
    long     infile_save ;
    short    sTWBLength = 0 ;
    short    u ; 
    BOOL     bChoiceFound = FALSE ;
    BOOL     bVar ;


    infile_save = ftell( fInput ) ; 
    if ( (INT)fgets( szTemp, sizeof(szTemp), fInput ) > 0 ) {

       /*------------------------------------------------------------------------*/
       /*  Modify choice variable to identify translatable text.                 */
       /*------------------------------------------------------------------------*/
       for( ptrChar=szTemp ; *ptrChar && isspace(*ptrChar) ; ++ptrChar ) ;
       if ( isdigit(*ptrChar) ) {
          for( ; *ptrChar && isdigit(*ptrChar) ; ++ptrChar ) ;
          for( ; *ptrChar && ( isspace(*ptrChar) || *ptrChar==',' ) ; ++ptrChar ) ;
          if ( ! strncmp( ptrChar, "choice", 6 ) ) {
             bChoiceFound = TRUE ;
             for( ptrChar+=6 ; 
                  *ptrChar && ( isspace(*ptrChar) || *ptrChar==',' ) ; ++ptrChar ) ;
             for( ; *ptrChar && *ptrChar!='}' ; ++ptrChar ) {
                for( ; *ptrChar && (isdigit(*ptrChar)||*ptrChar=='-') ; ++ptrChar ) ;
                if ( ( *ptrChar ) &&
                     ( strchr("#<>", *ptrChar) ) ) 
                   ++ptrChar ;
                memmove( ptrChar+5, ptrChar, strlen(ptrChar)+1 ) ;
                strncpy( ptrChar, "$TWB}", 5 ) ;
                sTWBLength += 5 ;
                ptrChar += 5 ;
                for( bVar=FALSE ; *ptrChar ; ++ptrChar ) {
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
                       sTWBLength += 5 ;
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
                sTWBLength += 5 ;
                ptrChar += 6 ;
                *ptrChar = 0 ;
             }
          }
       }
    }

    if ( bChoiceFound ) {
       fputs( szTemp, fOutput ) ;
       infile_save += strlen(szTemp) - sTWBLength ;
    }
    fseek( fInput, infile_save, 0 ) ; 

    return ;
}



/*************************************************************************/
/*  Adjust how single quotes are defined in this file, based on whether  */
/*  the text will be processed by MessageFormat or not.                  */
/*************************************************************************/
void  MFAdjustQuotes(
                       char  *InFile, 
                       char  *OutFile ) 
{
    FILE     *fInput, *fOutput ;
    char     *szIn = NULL ;
    char     *szTemp = NULL ;

    char     *ptrChar;

    long     lFilePosNext ;
    short    sState = 0 ;
    short    u ;
    BOOL     bVars ;
    BOOL     bBlockVars ;


    /*------------------------------------------------------------------------*/
    /*  Setup                                                                 */
    /*------------------------------------------------------------------------*/
    fInput  = fopen ( InFile, "rb" ) ;    
    fOutput = fopen ( OutFile, "wb+" ) ;  

    szIn = malloc( MAX_RCD_LENGTH ) ;
    szTemp = malloc( MAX_RCD_LENGTH ) ;

    if ( ( ! fInput  ) ||
         ( ! fOutput ) ||
         ( ! szIn    ) ||
         ( ! szTemp  ) ) {
       if ( fInput ) fclose(fInput) ;
       if ( fOutput ) fclose(fOutput) ;
       if ( szIn ) free(szIn) ;
       if ( szTemp ) free(szTemp) ;
       return ; 
    }
    bBlockVars = FALSE ;



    /*------------------------------------------------------------------------*/
    /*  Process each record in the file.                                      */
    /*------------------------------------------------------------------------*/
    while ( (INT)fgets( szIn, MAX_RCD_LENGTH, fInput ) > 0 ) {
       lFilePosNext = ftell( fInput ) ;               

       for( ptrChar=szIn ; *ptrChar ; ++ptrChar ) {
          u = GetCharLength( ptrChar ) ;                
          if ( u > 1 ) {                              /* Multi-byte char */
             ptrChar += u - 1 ;
             continue ;
          }


          if ( *ptrChar == '<'  ) {
             if ( ! strncmp( ptrChar, szMF_StartQuoteBlock, strlen(szMF_StartQuoteBlock) ) ) {
                sState = 1 ; 
             }

             if ( ! strncmp( ptrChar, szMF_EndQuoteBlock, strlen(szMF_EndQuoteBlock) ) ) {
                if ( sState == 2 ) {
                   memmove( ptrChar, ptrChar+strlen(szMF_EndQuoteBlock), strlen(ptrChar+strlen(szMF_EndQuoteBlock))+1 ) ;
                   --ptrChar ; 
                }
                sState = 0 ; 
             }

             if ( ! strncmp( ptrChar, szMF_CheckQuoteBlock, strlen(szMF_CheckQuoteBlock) ) ) {
                bVars = FALSE ;
                strcpy( szTemp, ptrChar ) ;
                if ( bBlockVars ) {                     
                   bVars = TRUE ;
                } else {
                   bVars = TextContainsVariable( szTemp ) ;
                   while( ( ! bVars ) &&
                          ( ! strstr( szTemp, szMF_EndQuoteBlock ) ) &&
                          ( (INT)fgets( szTemp, MAX_RCD_LENGTH, fInput ) > 0 ) ) {
                      bVars = TextContainsVariable( szTemp ) ;
                   }
                   fseek( fInput, lFilePosNext, 0 ) ;  /* Reset file reading pos */
                }
                if ( bVars ) {
                   *(ptrChar+4) = 'Q' ;
                   sState = 1 ;
                } else {
                   sState = 2 ;
                   memmove( ptrChar, ptrChar+strlen(szMF_CheckQuoteBlock), strlen(ptrChar+strlen(szMF_CheckQuoteBlock))+1 ) ;
                   --ptrChar ; 
                }
             }

             if ( ! strncmp( ptrChar, szMF_CheckQuoteBlockStart, strlen(szMF_CheckQuoteBlockStart) ) ) {
                bVars = FALSE ;
                strcpy( szTemp, ptrChar ) ;
                bVars = TextContainsVariable( szTemp ) ;
                while( ( ! bVars ) &&
                       ( ! strstr( szTemp, szMF_CheckQuoteBlockEnd ) ) &&
                       ( (INT)fgets( szTemp, MAX_RCD_LENGTH, fInput ) > 0 ) ) {
                   bVars = TextContainsVariable( szTemp ) ;
                }
                fseek( fInput, lFilePosNext, 0 ) ;  /* Reset file reading pos */
                memmove( ptrChar, ptrChar+strlen(szMF_CheckQuoteBlockStart), strlen(ptrChar+strlen(szMF_CheckQuoteBlockStart))+1 ) ;
                --ptrChar ; 
                if ( bVars ) {
                   bBlockVars = TRUE ;
                } else {
                   bBlockVars = FALSE ;
                }
             }

             if ( ! strncmp( ptrChar, szMF_CheckQuoteBlockEnd, strlen(szMF_CheckQuoteBlockEnd) ) ) { 
                bVars = FALSE ;
                memmove( ptrChar, ptrChar+strlen(szMF_CheckQuoteBlockEnd), strlen(ptrChar+strlen(szMF_CheckQuoteBlockEnd))+1 ) ;
                --ptrChar ; 
                bBlockVars = FALSE ;
             }

          }

          if ( sState == 1 ) {
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

       }

       fputs( szIn, fOutput ) ;           
    }


    fclose ( fInput ) ;  
    fclose ( fOutput ) ;  

    if ( szIn )
        free( szIn ) ;
    if ( szTemp )
        free( szTemp ) ;

    return ;
}


