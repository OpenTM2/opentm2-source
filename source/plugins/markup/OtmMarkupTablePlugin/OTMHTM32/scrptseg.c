/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

   #ifndef CCHMAXPATHCOMP
     #define CCHMAXPATHCOMP MAXPATHLEN
   #endif

   #include <windows.h>
   #include <wtypes.h>
   #define DosCopy(File1, File2, BOOL)  CopyFileA(File1, File2, FALSE);


#define MAXPATHLEN 2024


#include <sys\stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scrptseg.h"
#include "otmhtm32.h"
#include "usrcalls.h"

   wchar_t script2k2[18] = L"{TWBSCR}:EQFN.:QF" ;   /*  {TWBSCR}:EQFN.:QFN N=x.<XSCRCONT:  */
   wchar_t comment2k[12] = L"-->:EQFN.:Q" ;         /*  -->:EQFN.:QFN N=x.<XCONT:          */

#define NUM_JS_HTML_BREAK         24
static wchar_t  *JS_HTML_BREAK[ NUM_JS_HTML_BREAK ] =
{
   L"<BR>",
   L"<BR/>",
   L"<H1>",
   L"<H2>",
   L"<LI>",
   L"<OL>",
   L"<P>",
   L"<TD>",
   L"<TD >",
   L"<UL>",
   L"</H1>",
   L"</H2>",
   L"</LI>",
   L"</OL>",
   L"</P>",
   L"</TD>",
   L"</UL>",
   L"<\\/H1>",
   L"<\\/H2>",
   L"<\\/LI>",
   L"<\\/OL>",
   L"<\\/P>", 
   L"<\\/TD>",
   L"<\\/UL>",
} ;

BOOL preSegJSON(char *sourceName, char *tempName, long *startPos, long *endPos,short sJS_FileType)
{

    sJS_FileType = JS_TYPE_JSON_PRESEG ;
    return( postSegScriptTag(sourceName, tempName, startPos, endPos, sJS_FileType) ) ;
}



BOOL postSegScriptTag(char *sourceName, char *tempName, long *startPos, long *endPos,short sJS_FileType)
{

    FILE *sourceFile_ptr = NULL;
    FILE *tempFile_ptr = NULL;
    wchar_t *QFX = L":EQFN.:QFF N=.";                      /* Uppercased */
    wchar_t *EQFX = L":EQFF.:QFN N=.";                     /* Uppercased */
    wchar_t *TWBSQ = L"<TWBSQ>";                           /* Handle ' in squoted text */
    wchar_t *TWBSQE = L"</TWBSQ>";                        /* Handle ' in squoted text */
    wchar_t *JSON_START = L"<twbj--" ;
    wchar_t *JSON_END = L"--twbj>" ;
    wchar_t *eqChar = L"=";
    wchar_t *doubleQuote = L"\"";
    wchar_t *singleQuote = L"\'";
    wchar_t *escChar = L"\\";
    wchar_t *fwdslash = L"/";
    wchar_t *lineComment = L"/";
    wchar_t *blockComment = L"*";
    wchar_t *not = L"!";
    wchar_t *plusChar  = L"+";
    wchar_t *leftBracketChar  = L"[";
    wchar_t *leftBraceChar  = L"{";
    wchar_t *commaChar  = L",";
    wchar_t *ampersandChar  = L"&";
    wchar_t *greaterChar  = L">";
    wchar_t *rightBracketChar  = L"]";
    wchar_t *semicolonChar  = L";";
    wchar_t *colonChar  = L":";
    wchar_t *lessChar  = L"<";
    wchar_t *exclamationChar  = L"!";
    wchar_t *newLine  = L"\n";
    wchar_t *leftParen = L"(";
    wchar_t *rightParen = L")";
////char *pigdogChar = "µ"; //kml - P007503: fix arbitrary insertion of script continuation string.
    int     quoteChar = 0;
    int     commentChar = 0;
    wchar_t newChar;
    wchar_t maybeChar;
    wchar_t prevChar;
    int     doneEqual = 0;
    int     doneQuote = 0;
    int     doneComment = 0;
    short   i, j ;
    short   newFile = 0;
    long    fileposLastQFX ;
    int     rc;
    int     char_index;
    wchar_t tagBuff[1024];
    wchar_t upperCaseBuff[1024];
    wchar_t szChar8[9] ;
    wchar_t *strpos;
    BOOL bInTransScript;
    BOOL bInJavaScript;
    BOOL bInVBScript;
    BOOL bInEqualStmt = FALSE ;
    BOOL bInArrayStmt = FALSE ;
    BOOL bInColonStmt = FALSE ;
    BOOL bInColonArray = FALSE ;
    BOOL bFindPlus = FALSE ;
    BOOL set_not = FALSE;
    BOOL not_char = FALSE;
    BOOL skip_colon = FALSE;
    BOOL bInNonTransSection = FALSE;
    BOOL bInFunction = FALSE ;  
    BOOL bSkipEndWrite = FALSE;
    BOOL bSkipLeadingHTMLComments = TRUE ;
    BOOL bInHTMLComment = FALSE ;
    BOOL bInSQuotedText = FALSE ;
    BOOL bJSONPreSeg = FALSE ;
    BOOL bLoop = TRUE ;
    int quoteFlag;
    int doneWhiteSpace = 0;
    int plusSet = 0;
    int commentIndex ;
    long filepos ;
    int depthColonArray = 0;      
    int numParenNest = 0;         
    int numParenFunction = 0;     

    if ( sJS_FileType != JS_TYPE_NONE ) {
       if ( sJS_FileType == JS_TYPE_JSON_PRESEG ) 
          bJSONPreSeg = TRUE ;
       else
       if ( sJS_FileType == JS_TYPE_JSON )    
          return( TRUE ) ;
    }

       memset(tagBuff, 0, sizeof(tagBuff));
       memset(upperCaseBuff, 0, sizeof(upperCaseBuff));


    //
    // OPENS TWO FILES A SOURCE AND A TEMP.
    // COPIES THE TEXT FROM SOURCE TO TEMP UPTO THE <SCRIPT> TAG.
    //


       copyPartialFile (sourceName,&sourceFile_ptr,
                        tempName, &tempFile_ptr,
                        0, *startPos,
                        (newFile = TRUE));

       fseek(sourceFile_ptr,(*startPos),SEEK_SET); /* set to start tag */
       char_index = 0;
       while((((newChar = fgetwc(sourceFile_ptr)) != '>') && (ftell(sourceFile_ptr) <= *endPos))){ // get to the end of the script open tag
          fputwc(newChar, tempFile_ptr);
          tagBuff[char_index] = newChar;
          char_index++;
          if(char_index > sizeof(tagBuff)/sizeof(wchar_t)){
             fclose(sourceFile_ptr);
             fclose(tempFile_ptr);
             remove(tempName);
             return TRUE;
          }
       }
       tagBuff[char_index] = newChar;
       tagBuff[char_index+1] = '\0';
       wcscpy(upperCaseBuff, tagBuff);
       StrUpr(upperCaseBuff);
       bInTransScript = FALSE;
       bInJavaScript = FALSE;
       bInVBScript = FALSE;
       
       if(ftell(sourceFile_ptr) < *endPos){
          if(((strpos = wcsstr(upperCaseBuff, L"LANGUAGE")) != NULL)){
                strpos = strpos+8;
          }
          else if(((strpos = wcsstr(upperCaseBuff, L"TYPE")) != NULL)){
                strpos = strpos+4;
          }
          if(strpos != NULL){
             while( (*strpos) && iswspace(*strpos) ) {
                strpos++;
             }
             if(*strpos == '='){
                strpos++;
                while( (*strpos) && iswspace(*strpos) ) {
                   strpos++;
                }
                if((*strpos == '"') || (*strpos == '\'')){
                   strpos++;
                   while( (*strpos) && iswspace(*strpos) ) {
                   strpos++;
                   }
                }

        // First we must verify we are in a javascript or VBscript section

                if( (wcsstr(strpos, L"JAVASCRIPT") != NULL) ||
                    (wcsstr(strpos, L"JSCRIPT")    != NULL) ||  //kml:P000008 check for either identifier
                    (wcsstr(strpos, L"VBSCRIPT")   != NULL) )  {//daw:P007889 Add support for VBScript
                   if ( wcsstr( strpos, L"VBSCRIPT") != NULL )
                      bInVBScript = TRUE ;
                   else
                      bInJavaScript = TRUE ;
                   bInTransScript = TRUE;
                   fputwc(newChar, tempFile_ptr);  // write ">" to file
                   quoteFlag = 0;

                   if ( bJSONPreSeg ) {                                /* 4-3-12 */
                      fputws(L"{TWBSCR}", tempFile_ptr);      /* End <SCRIPT> segment */
                      fputws(JSON_START, tempFile_ptr);
                   }

                //We loop until we get to the script end tag

                   while((((newChar = fgetwc(sourceFile_ptr)) != _TEOF) && (ftell(sourceFile_ptr) <= *endPos))){

                      //  Skip all leading HTML comments, only up to the end of the current line. 
                      if ( bSkipLeadingHTMLComments ) {
                         if ( bInHTMLComment ) {
                            fputwc(newChar, tempFile_ptr); 
                            if ( newChar == *newLine )  
                               bInHTMLComment = FALSE ;
                            continue ;
                         }
                         if ( iswspace( newChar ) ) {
                            fputwc(newChar, tempFile_ptr); 
                            continue ;
                         }
                         if ( newChar == *lessChar ) {
                            maybeChar = fgetwc(sourceFile_ptr); 
                            if ( maybeChar == *exclamationChar ) {
                               fputwc(newChar, tempFile_ptr); 
                               fputwc(maybeChar, tempFile_ptr); 
                               bInHTMLComment = TRUE ;
                               continue ;
                            }
                            fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);
                         }
                         bSkipLeadingHTMLComments = FALSE ;
                      }

                      /* if double byte char skip next char  an look next for non DB char*/
                      if(IsDBCS(newChar) != 0){     /* check for double byte char */
                         while(((IsDBCS(newChar) != 0) && (newChar != EOF))){ /* loop on double byte char */
                            fputwc(newChar, tempFile_ptr); /* write first byte of double byte to file */
                            newChar = fgetwc(sourceFile_ptr); /* get second byte */
                            fputwc(newChar, tempFile_ptr); /* write second byte to file */
                            newChar = fgetwc(sourceFile_ptr); /* get next char */
                         }
                      }

                      // LOOK FOR A "/" CHAR to signal possible start of comment
                      if( ( bInJavaScript && (newChar==*fwdslash)    ) ||
                          ( bInVBScript   && (newChar==*singleQuote) ) ) {
                         fputwc(newChar, tempFile_ptr); /* write char to file */
                         if( ( bInVBScript ) ||
                             ( ( (newChar=fgetwc(sourceFile_ptr)) != WEOF ) && 
                               ( ftell(sourceFile_ptr) <= *endPos ) ) ) { // Get next char
                            if( ( newChar == *lineComment ) || // check char second char of a line comment
                                ( bInVBScript ) ) {
                               commentChar = '\n'; // set the end comment char to wait for
                               doneComment = 0;
                               commentIndex = 0 ;
                               if ( ! bInVBScript )
                                  fputwc(newChar, tempFile_ptr); // write Comment to file
                               while((((newChar = fgetwc(sourceFile_ptr)) != WEOF) && (ftell(sourceFile_ptr) <= *endPos) && (doneComment != 1))){
                    /* if double byte char skip next char  an look next for non DB char*/
                                  if(IsDBCS(newChar) != 0){     /* check for double byte char */
                                     while(((IsDBCS(newChar) != 0) && (newChar != WEOF))){
                                        fputwc(newChar, tempFile_ptr); /* write first byte of double byte to file */
                                        newChar = fgetwc(sourceFile_ptr); /* get second byte */
                                        fputwc(newChar, tempFile_ptr); /* write second byte to file */
                                        newChar = fgetwc(sourceFile_ptr); /* get next char */
                                     }
                                  }
                                  if(newChar == commentChar){    // LOOK FOR the end comment char CHAR
                                     doneComment = 1;
                                  }
                                  fputwc(newChar, tempFile_ptr); // write char to file
                                  if ( ( ! iswspace(newChar) ) &&    // Save non-blank comment text
                                       ( newChar != L'\xA0'  ) )    
                                     tagBuff[commentIndex++] = newChar ;
                               }
                               fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);
                               /********************************************************/
                               /*  Handle user-defined block of lines which will not   */
                               /*  contain any translatable text.                      */
                               /********************************************************/
                               if ( commentIndex > 18 ) {
                                  tagBuff[commentIndex] = 0 ;
                                  wcsupr( tagBuff ) ;
                                  if ( ( ! bInNonTransSection ) &&
                                       ( ( ! wcsncmp( tagBuff, L"STARTNON-TRANSLATABLE", 21 ) ) || 
                                         ( ! wcsncmp( tagBuff, L"STARTQUOTETRANSLATABLE", 22 ) ) ) )
                                     bInNonTransSection = TRUE ;
                                  else
                                     if ( ( bInNonTransSection ) &&
                                          ( ( ! wcsncmp( tagBuff, L"ENDNON-TRANSLATABLE", 19) ) ||  
                                            ( ! wcsncmp( tagBuff, L"ENDQUOTETRANSLATABLE", 20 ) ) ) )
                                        bInNonTransSection = FALSE ; 
                               }
                               continue ;                                   
                            }
                            else if((newChar == *blockComment)){    // check char for second char of a block comment
                               commentChar = '*';   // set the end comment char to wait for
                               doneComment = 0;
                               fputwc(newChar, tempFile_ptr); // write Comment to file
                               while((((newChar = fgetwc(sourceFile_ptr)) != WEOF) && (ftell(sourceFile_ptr) <= *endPos) && (doneComment != 1))){
                    /* if double byte char skip next char  an look next for non DB char*/
                                  if(IsDBCS(newChar) != 0){     /* check for double byte char */
                                     while(((IsDBCS(newChar) != 0) && (newChar != WEOF))){
                                        fputwc(newChar, tempFile_ptr); /* write first byte of double byte to file */
                                        newChar = fgetwc(sourceFile_ptr); /* get second byte */
                                        fputwc(newChar, tempFile_ptr); /* write second byte to file */
                                        newChar = fgetwc(sourceFile_ptr); /* get next char */
                                     }
                                  }
                                  if(newChar == commentChar){    // LOOK FOR the first char of the ending block comment '*'
                                     fputwc(newChar, tempFile_ptr); // write * to file
                                     while( ( (newChar=fgetwc(sourceFile_ptr)) != WEOF ) && // Get next char
                                            ( ftell(sourceFile_ptr) <= *endPos ) &&        
                                            ( newChar == commentChar ) ) {  // skip string of asterisks 
                                        fputwc(newChar, tempFile_ptr); 
                                     }
                                     if(newChar == '/'){    // LOOK FOR the second char of the ending block comment '/'
                                        doneComment = 1;
                                     } else 
                                     if ( ( newChar == script2k2[0] ) &&  
                                          ( chkWhiteSpace(sourceFile_ptr, tempFile_ptr, &newChar, *endPos, TRUE ) ) &&
                                          ( newChar == ':' ) ) { 
                                        newChar = fgetwc( sourceFile_ptr ) ;
                                        if ( newChar == '/' )
                                           doneComment = 1;
                                     }
                                  }
                                  fputwc(newChar, tempFile_ptr); // write "=" to file
                               }
                               fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);
                            }
                            else{
                               fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);
                            }
                         }
                      }

                 // If we find a single or double quote not after an = char we loop until we find the matching end quote
                 // all char inside the quotes are non translatable and no other processing is done

                      else if( ( bInJavaScript && ((newChar==*doubleQuote)||(newChar==*singleQuote)) && (prevChar != *escChar) ) || /* Ignore escaped quote */
                               ( bInVBScript   && (newChar==*doubleQuote) ) ) {
                         quoteChar = newChar;
                         doneQuote = 0;
                         fputwc(newChar, tempFile_ptr); // write quote to file
                         while((((newChar = fgetwc(sourceFile_ptr)) != WEOF) && (ftell(sourceFile_ptr) <= *endPos) && (doneQuote != 1))){
                    /* if double byte char skip next char  an look next for non DB char*/
                            if(IsDBCS(newChar) != 0){     /* check for double byte char */
                               while(((IsDBCS(newChar) != 0) && (newChar != WEOF))){
                                  fputwc(newChar, tempFile_ptr); /* write first byte of double byte to file */
                                  newChar = fgetwc(sourceFile_ptr); /* get second byte */
                                  fputwc(newChar, tempFile_ptr); /* write second byte to file */
                                  newChar = fgetwc(sourceFile_ptr); /* get next char */
                               }
                            }
                            if(newChar == *escChar){          /* Ignore escaped quote */
                               fputwc(newChar, tempFile_ptr);
                               newChar = fgetwc(sourceFile_ptr);
                               fputwc(newChar, tempFile_ptr);
                               continue;
                            }

                            if(newChar == quoteChar){    // LOOK FOR AN '"' CHAR
                               doneQuote = 1;
                            }
                            fputwc(newChar, tempFile_ptr); // write char to file
                         }
                         fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);
                      }
                // If we find a single or double quote after an = char then the quoted text is translatable.
                // If a + char follows the quoted string processing for translatable text should continue
                // example = "translatable" + nontranslatable + "translatable2" text
                // processing for translatible text should end at the first non white space char after translatable2

                      else if( ( !bInNonTransSection ) && // In translatable section
                               ( ( newChar == *eqChar ) ||    // LOOK FOR AN "=" CHAR
                                 ( newChar == *colonChar ) ||   // LOOK FOR AN ":" CHAR, JSON 
                                 ( bInEqualStmt &&
                                   ( ( newChar == *plusChar ) ||
                                     ( bInJavaScript &&                   
                                       ( ( ( newChar == *leftBracketChar ) &&
                                           ( prevChar == *eqChar ) ) ||
                                         ( ( bInArrayStmt ) &&
                                           ( newChar == *commaChar ) ) ) ) ||
                                     ( bInVBScript && (newChar==*ampersandChar) ))))) {
                         fputwc(newChar, tempFile_ptr); // write "=" to file
                         prevChar = newChar ;
                         skip_colon = 0 ;
                         if ( newChar == *colonChar ) {        /* Ignore TM tags */
                            filepos = ftell( sourceFile_ptr ) ;
                            szChar8[0] = fgetwc(sourceFile_ptr) ;
                            szChar8[1] = fgetwc(sourceFile_ptr) ;
                            szChar8[2] = fgetwc(sourceFile_ptr) ;
                            szChar8[3] = NULL ;
                            fseek(sourceFile_ptr, filepos, SEEK_SET);
                            wcsupr( szChar8 ) ;
                            if ( ( ! wcsncmp( szChar8, L"EQF", 3 ) ) ||
                                 ( ! wcsncmp( szChar8, L"QF",  2 ) ) ) {
                               while( ( (newChar = fgetwc(sourceFile_ptr)) != WEOF ) && 
                                      ( newChar != L'.' ) &&
                                      ( ftell(sourceFile_ptr) <= *endPos ) ) {
                                  fputwc(newChar, tempFile_ptr); 
                                  prevChar = newChar ;
                               }
                               if ( ftell(sourceFile_ptr) <= *endPos ) {
                                  fputwc(newChar, tempFile_ptr); 
                                  prevChar = newChar ;
                               }
                               skip_colon = 1;
                            }
                         }
                         if ( ( ! not_char   ) &&
                              ( ! skip_colon ) ) {
                            doneEqual = 0;
                            bInEqualStmt = TRUE ;                   
                            bFindPlus = FALSE ;
                            if ( newChar == *leftBracketChar )      
                               bInArrayStmt = TRUE ; 
                            if ( newChar == *colonChar ) {          
                               bInColonStmt = TRUE ; 
                               bInColonArray = FALSE ;
                               depthColonArray = 0; 
                            }

                            while((((newChar = fgetwc(sourceFile_ptr)) != WEOF) && (doneEqual != 1) && (ftell(sourceFile_ptr) <= *endPos))){  // get next char
                     /* if double byte char skip next char  an look next for non DB char*/
                               if(IsDBCS(newChar) != 0){     /* check for double byte char */
                                  while(((IsDBCS(newChar) != 0) && (newChar != EOF))){
                                     fputwc(newChar, tempFile_ptr); /* write first byte of double byte to file */
                                     newChar = fgetwc(sourceFile_ptr); /* get second byte */
                                     fputwc(newChar, tempFile_ptr); /* write second byte to file */
                                     newChar = fgetwc(sourceFile_ptr); /* get next char */
                                  }
                               }
                               fputwc(newChar, tempFile_ptr);                                     // write char to file

                               /********************************************************/
                               /*  Handle line comments which may define the start/end */
                               /*  of a non-translatable block.                        */
                               /********************************************************/
                               if( bInJavaScript && (newChar==*fwdslash) ) {
                                  if( ( (newChar=fgetwc(sourceFile_ptr)) != WEOF ) && 
                                      ( ftell(sourceFile_ptr) <= *endPos ) ) {      // Get next char
                                     if( newChar == *lineComment ) { // check char second char of a line comment
                                        commentChar = '\n'; // set the end comment char to wait for
                                        doneComment = 0;
                                        commentIndex = 0 ;
                                        fputwc(newChar, tempFile_ptr); // write Comment to file
                                        while((((newChar = fgetwc(sourceFile_ptr)) != WEOF) && (ftell(sourceFile_ptr) <= *endPos) && (doneComment != 1))){
                                           /* if double byte char skip next char  an look next for non DB char*/
                                           if(IsDBCS(newChar) != 0){     /* check for double byte char */
                                              while(((IsDBCS(newChar) != 0) && (newChar != WEOF))){
                                                 fputwc(newChar, tempFile_ptr); /* write first byte of double byte to file */
                                                 newChar = fgetwc(sourceFile_ptr); /* get second byte */
                                                 fputwc(newChar, tempFile_ptr); /* write second byte to file */
                                                 newChar = fgetwc(sourceFile_ptr); /* get next char */
                                              }
                                           }
                                           if(newChar == commentChar){    // LOOK FOR the end comment char CHAR
                                              doneComment = 1;
                                           }
                                           fputwc(newChar, tempFile_ptr); // write char to file
                                           if ( ( ! iswspace(newChar) ) &&  // Save non-blank comment text
                                                ( newChar != L'\xA0'  ) ) 
                                              tagBuff[commentIndex++] = newChar ;
                                        }
                                        fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);
                                        /********************************************************/
                                        /*  Handle user-defined block of lines which will not   */
                                        /*  contain any translatable text.                      */
                                        /********************************************************/
                                        if ( commentIndex > 18 ) {
                                           tagBuff[commentIndex] = 0 ;
                                           wcsupr( tagBuff ) ;
                                           if ( ( ! bInNonTransSection ) &&
                                                ( ( ! wcscmp( tagBuff, L"STARTNON-TRANSLATABLE" ) ) ||
                                                  ( ! wcsncmp( tagBuff, L"STARTQUOTETRANSLATABLE", 22 ) ) ) )
                                              bInNonTransSection = TRUE ;
                                           else
                                              if ( ( bInNonTransSection ) &&
                                                   ( ( ! wcsicmp( tagBuff, L"ENDNON-TRANSLATABLE" ) ) ||
                                                     ( ! wcsncmp( tagBuff, L"ENDQUOTETRANSLATABLE", 20 ) ) ) ) 
                                                 bInNonTransSection = FALSE ; 
                                        }
                                     } else {
                                        fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);
                                     }
                                     continue ;                     
                                  }
                               }

                               if ( bInNonTransSection ) 
                                  continue;

                               if ( bInEqualStmt && bInJavaScript && ( newChar == *leftParen ) ) {     
                                  numParenNest++;
                                  filepos = ftell( sourceFile_ptr ) ;
                                  fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-2*sizeof(wchar_t)), SEEK_SET);
                                  prevChar = fgetwc(sourceFile_ptr);
                                  while ( chkWhiteSpace(sourceFile_ptr, tempFile_ptr, &prevChar, *endPos, FALSE ) ) {
                                        fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-2*sizeof(wchar_t)), SEEK_SET);
                                        prevChar = fgetwc(sourceFile_ptr);
                                  }
                                  if ( ( iswalpha( prevChar ) ) ||
                                     ( prevChar == L'_'   ) ) {
                                     bInFunction = TRUE ; 
                                     numParenFunction = numParenNest ;
                                  }
                                  fseek(sourceFile_ptr, filepos, SEEK_SET);
                               } else if ( bInFunction && bInJavaScript && ( newChar == *rightParen ) ) {
                                  if ( numParenNest ) 
                                     numParenNest--;
                                  if ( numParenNest < numParenFunction ) {
                                     bInFunction = FALSE ;
                                  }
                               }
                               if ( bInFunction )
                                  continue; 
                               if ( ( bInJavaScript && ((newChar==*doubleQuote)||(newChar==*singleQuote)) ) ||   // if its a quote start translatable segment
                                    ( bInVBScript  && (newChar==*doubleQuote) ) ) {
                                  quoteChar = newChar;
                                  if ( ( bInJavaScript ) &&         /* Special ' handling in squoted text */
                                       ( newChar==*singleQuote ) ) {
                                     fputws( TWBSQ, tempFile_ptr ) ;
                                     bInSQuotedText = TRUE ;
                                  }
                                  fileposLastQFX = ftell( tempFile_ptr ) ;
                                  if ( bJSONPreSeg )
                                     fputws(JSON_END, tempFile_ptr);
                                  else 
                                  fputws(QFX, tempFile_ptr);
                                  doneQuote = 0;
                                  plusSet = 0;
                                  while((((newChar = fgetwc(sourceFile_ptr)) != WEOF) && (doneQuote != 1) && (ftell(sourceFile_ptr) <= *endPos))){ // look for end quote

                                  /* if double byte char skip next char  an look next for non DB char*/
                                     if(IsDBCS(newChar) != 0){     /* check for double byte char */
                                        while(((IsDBCS(newChar) != 0) && (newChar != WEOF))){
                                           fputwc(newChar, tempFile_ptr); /* write first byte of double byte to file */
                                           newChar = fgetwc(sourceFile_ptr); /* get second byte */
                                           fputwc(newChar, tempFile_ptr); /* write second byte to file */
                                           newChar = fgetwc(sourceFile_ptr); /* get next char */
                                        }
                                        fileposLastQFX = NULL ;
                                     }
                                     if(newChar == *escChar){
                                        maybeChar = fgetwc(sourceFile_ptr);
                                        if ( ( ! bInSQuotedText ) ||             /* Handle escaped single quotes */
                                             ( maybeChar != *singleQuote ) ) 
                                        fputwc(newChar, tempFile_ptr);
                                        fputwc(maybeChar, tempFile_ptr);
                                        fileposLastQFX = NULL ;
///                                     newChar = fgetc(sourceFile_ptr);
                                        continue ;              
                                     }

                                     //Improper segmentation within javascript
                                     //Here is where we need to check for several potential false terminators which are
                                     //double-double-quotes {""}
                                     //double-single-quotes {''}
                                     //backslashed-double-quotes {\"}
                                     //backslashed-single-quotes {\"}
                                     //backslashed-anycharacter {\?}
                                     if(newChar == quoteChar){    // if quote char end translatable segment
                                         fileposLastQFX = NULL ;
                                         maybeChar = fgetwc(sourceFile_ptr); /* get next char */
                                         if (maybeChar == quoteChar) {
                                             fputwc(newChar, tempFile_ptr); // write escaping quote
                                             fputwc(maybeChar, tempFile_ptr); // write literal quote
                                         } else {
                                             fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);
                                             if ( ! bSkipEndWrite ) {
                                                if ( bJSONPreSeg ) 
                                                   fputws(JSON_START, tempFile_ptr);
                                                else
                                                fputws(EQFX, tempFile_ptr);
                                             } else {
                                                bSkipEndWrite = FALSE ;
                                             }
                                             if ( bInSQuotedText ) {     /* Special ' handling in squoted text */
                                                fputws( TWBSQE, tempFile_ptr ) ;
                                                bInSQuotedText = FALSE ;
                                             }
                                             fputwc(newChar, tempFile_ptr); // write ending quote
                                             while((((newChar = fgetwc(sourceFile_ptr)) != WEOF) && (doneQuote != 1) && (ftell(sourceFile_ptr) <= *endPos))){

                                                /* if double byte char skip next char and look next for non DB char*/
                                                if(IsDBCS(newChar) != 0){     /* check for double byte char */
                                                   while(((IsDBCS(newChar) != 0) && (newChar != WEOF))){
                                                      fputwc(newChar, tempFile_ptr); /* write first byte of double byte to file */
                                                      newChar = fgetwc(sourceFile_ptr); /* get second byte */
                                                      fputwc(newChar, tempFile_ptr); /* write second byte to file */
                                                      newChar = fgetwc(sourceFile_ptr); /* get next char */
                                                   }
                                                }

                                                //kml - P007099: for every fget there must be a matching fput in the loop
                                                fputwc(newChar, tempFile_ptr);


                                                if( ( newChar == *plusChar ) ||
                                                    ( bInJavaScript &&          
                                                      ( ( bInArrayStmt  && (newChar==*commaChar) ) || 
                                                        ( bInColonArray && (newChar==*commaChar) ) ) ) || 
                                                    ( bInVBScript && (newChar==*ampersandChar) ) ) {
                                                   doneQuote = 1;
                                                   plusSet = 1;
                                                   if( bInJavaScript &&  
                                                       ( bInColonArray && (newChar==*commaChar) )  ){ 
                                                   	plusSet = 0;
                                                   } 
                                                   doneWhiteSpace = 0;
                                                   while((((newChar = fgetwc(sourceFile_ptr)) != WEOF) && (doneWhiteSpace != 1) && (ftell(sourceFile_ptr) <= *endPos))){
                                                      if(!chkWhiteSpace(sourceFile_ptr, tempFile_ptr, &newChar, *endPos, TRUE )){
                                                         doneWhiteSpace = 1;
                                                      }
                                                      else{
                                                         if ( newChar != ':' )         
                                                           fputwc(newChar, tempFile_ptr);
                                                      }
                                                   }
                                                   fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-2*sizeof(wchar_t)), SEEK_SET);
                                                }
                                                else if(!chkWhiteSpace(sourceFile_ptr, tempFile_ptr, &newChar, *endPos, FALSE )){
                                                   doneEqual = 1;
                                                   doneQuote = 1;
                                                   //P006886-KML Need to put one character back because we have gone one to far.
                                                   if (newChar == *fwdslash) {

                                                       fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);
                                                       fseek(tempFile_ptr, (ftell(tempFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);
                                                   }
                                                   if ( newChar == *semicolonChar ) { 
                                                      bInEqualStmt = FALSE ;
                                                      bInArrayStmt = FALSE ;          
                                                      bInColonStmt = FALSE ;          
                                                   }
                                                   if ( ( newChar == *commaChar ) &&    
                                                        ( bInColonStmt   ) &&
                                                        ( ( ! bInColonArray ) ||        
                                                          ( newChar == *rightBracketChar ) ) ) {
                                                      bInEqualStmt = FALSE ;
                                                      bInArrayStmt = FALSE ; 
                                                      bInColonStmt = FALSE ;      
                                                      bInColonArray = FALSE ;      
                                                   }
                                                   if( ( newChar == *rightBracketChar ) &&    
                                                   	   ( bInColonStmt   ) &&
                                                   	   ( bInColonArray ) ) {
                                                   	   	
                                                      depthColonArray --; 
                                                      if(depthColonArray <1 ){                
                                                          bInColonArray = FALSE ; 
                                                          depthColonArray = 0; 
                                                        }else
                                                          doneEqual = 0;
                                                   } 
                                                }
                                                else{
                                                }
                                             }
                                             fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);

                                         }
                                         continue ;
                                     }

                                     /********************************************************/
                                     /*  If :QFN or :EQFN tag, then change tags to indicate  */
                                     /*  translatable text (:QFX, :EQFX).                    */
                                     /********************************************************/
                                     if ( newChar == *colonChar ) {
                                        filepos = ftell( sourceFile_ptr ) ;
                                        szChar8[0] = fgetwc(sourceFile_ptr) ;
                                        szChar8[1] = fgetwc(sourceFile_ptr) ;
                                        szChar8[2] = fgetwc(sourceFile_ptr) ;
                                        szChar8[3] = fgetwc(sourceFile_ptr) ;
                                        szChar8[4] = NULL ;
                                        fseek(sourceFile_ptr, filepos, SEEK_SET);
                                        wcsupr( szChar8 ) ;
                                        if ( ( ! wcsncmp( szChar8, L"QFN",  3 ) ) ||
                                             ( ! wcsncmp( szChar8, L"EQFN", 4 ) ) ) {
                                           if ( szChar8[0] == L'Q' ) 
                                              i = 3 ;
                                           else
                                              i = 4 ;
                                           for( ; i>0 ; --i ) {
                                              fputwc(newChar, tempFile_ptr); 
                                              newChar = fgetwc(sourceFile_ptr); 
                                           }
                                           if ( newChar == L'N' ) 
                                              fputwc(L'F', tempFile_ptr); 
                                           else
                                              fputwc(L'f', tempFile_ptr); 
                                           continue ;
                                        }
                                     }

                                     /********************************************************/
                                     /*  Force a segment break if there is a common HTML     */
                                     /*  break tag imbedded in the text.                     */
                                     /********************************************************/
                                     if ( newChar == *lessChar ) {
                                        filepos = ftell( sourceFile_ptr ) ;
                                        szChar8[0] = newChar ;
                                        szChar8[1] = fgetwc(sourceFile_ptr) ;
                                        szChar8[2] = fgetwc(sourceFile_ptr) ;
                                        szChar8[3] = fgetwc(sourceFile_ptr) ;
                                        szChar8[4] = fgetwc(sourceFile_ptr) ;
                                        szChar8[5] = fgetwc(sourceFile_ptr) ;   
                                        szChar8[6] = NULL ;
                                        fseek(sourceFile_ptr, filepos, SEEK_SET);
                                        wcsupr( szChar8 ) ;

                                        if ( ( ! wcschr( L"/!\\", szChar8[1] ) ) &&     /* Handle lone < in trans text */
                                             ( ! isalpha( szChar8[1] ) ) ) {
                                           fputws( L"&TWBLT", tempFile_ptr ); 
                                           newChar = L';' ;
                                        }

                                        for( i=0 ; i<NUM_JS_HTML_BREAK ; i++ ) { /* Check if break tag */
                                           if ( ! wcsncmp( szChar8, JS_HTML_BREAK[i], wcslen(JS_HTML_BREAK[i]) ) ) 
                                              break;
                                        }
                                        if ( i < NUM_JS_HTML_BREAK ) {
                                           j = wcslen( JS_HTML_BREAK[i] ) ;
                                           if ( fileposLastQFX ) { /* Break at beginning of quoted text */
                                              fseek(tempFile_ptr, fileposLastQFX, SEEK_SET);
                                              fileposLastQFX = NULL ; 
                                           } else {
                                              if ( bJSONPreSeg ) 
                                                 fputws(JSON_START, tempFile_ptr);
                                              else
                                              fputws(EQFX, tempFile_ptr);
                                           }
                                           fputwc(newChar, tempFile_ptr);
                                           for( --j ; j>0 ; --j ) {
                                              newChar = fgetwc(sourceFile_ptr); 
                                              fputwc(newChar, tempFile_ptr); 
                                           }
                                           while( bLoop ) {  /* Include following break tags */
                                              filepos = ftell( sourceFile_ptr ) ;
                                              szChar8[0] = fgetwc(sourceFile_ptr) ;
                                              if ( szChar8[0] == *lessChar ) {
                                                 szChar8[1] = fgetwc(sourceFile_ptr) ;
                                                 szChar8[2] = fgetwc(sourceFile_ptr) ;
                                                 szChar8[3] = fgetwc(sourceFile_ptr) ;
                                                 szChar8[4] = fgetwc(sourceFile_ptr) ;
                                                 szChar8[5] = fgetwc(sourceFile_ptr) ; 
                                                 szChar8[6] = NULL ;
                                                 fseek(sourceFile_ptr, filepos, SEEK_SET);
                                                 wcsupr( szChar8 ) ;
                                                 for( i=0 ; i<NUM_JS_HTML_BREAK ; i++ ) {
                                                    if ( ! wcsncmp( szChar8, JS_HTML_BREAK[i], wcslen(JS_HTML_BREAK[i]) ) ) 
                                                       break;
                                                 }
                                                 if ( i < NUM_JS_HTML_BREAK ) {
                                                    j = wcslen( JS_HTML_BREAK[i] ) ;
                                                    for( ; j>0 ; --j ) {
                                                       newChar = fgetwc(sourceFile_ptr); 
                                                       fputwc(newChar, tempFile_ptr); 
                                                    }
                                                 } else
                                                    break ;
                                              } else {
                                                 fseek(sourceFile_ptr, filepos, SEEK_SET);
                                                 break ;
                                              }
                                           }
                                           filepos = ftell( sourceFile_ptr ) ;
                                           szChar8[0] = fgetwc(sourceFile_ptr) ;
                                           szChar8[1] = fgetwc(sourceFile_ptr) ;
                                           fseek(sourceFile_ptr, filepos, SEEK_SET);
                                           if( ( szChar8[0] == quoteChar ) &&
                                               ( szChar8[1] != quoteChar ) ) {
                                              bSkipEndWrite = TRUE ;
                                           } else {
                                              if ( bJSONPreSeg ) 
                                                 fputws(JSON_END, tempFile_ptr);
                                              else
                                              fputws(QFX, tempFile_ptr);
                                           }
                                           continue ;
                                        }
                                     }

                                     fputwc(newChar, tempFile_ptr);   // if not ending quote write out char
                                     fileposLastQFX = NULL ;
                                  }
                                  fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);
                               }
                               else{
                                  if(!chkWhiteSpace(sourceFile_ptr, tempFile_ptr, &newChar, *endPos, FALSE )){
                                     if(!(plusSet == 1)){
                                        if ( ( bInJavaScript && 
                                               ( ( ( ! bInArrayStmt ) &&
                                                   ( newChar != *leftBracketChar ) &&
                                                   ( newChar != *plusChar ) ) ||
                                                 ( ( bInArrayStmt ) &&
                                                   ( newChar != *commaChar ) ) ||
                                                 ( ( bInColonStmt ) &&
                                                   ( newChar == *leftBracketChar ) ) ) ) ||
                                             ( bInVBScript && 
                                               ( ( newChar != *plusChar ) &&
                                                 ( newChar != *ampersandChar) ) ) ) {
                                           if ( newChar == *eqChar ) { 
                                              if ( prevChar == *plusChar )
                                                 prevChar = 0 ;
                                              if ( prevChar == *eqChar ) {
                                                 bInEqualStmt = FALSE ;
                                                 bInArrayStmt = FALSE ;
                                                 doneEqual = 1 ;
                                                 plusSet = 0 ;
                                              }
                                               
                                           } else
                                           	if(bInColonArray &&                 
                                           	   (newChar == *commaChar)) {
                                           	   plusSet = 0;  /* what's the difference of 1, and 0? */
                                           } else
                                           if ( ( bInColonArray ) &&     
                                                ( newChar == *leftBracketChar ) ) {
                                              bInColonArray = TRUE ;
                                              depthColonArray ++; 
                                              doneEqual = 0;
                                              plusSet = 1; 
                                           } else {
                                              doneEqual = 1;
                                              plusSet = 0;
                                           }
                                        }
                                        else{
                                           plusSet = 1;
                                           doneWhiteSpace = 0;
                                           if ( ( prevChar == *eqChar ) &&
                                                ( newChar  == *leftBracketChar ) )
                                              bInArrayStmt = TRUE ;
                                           while((((newChar = fgetwc(sourceFile_ptr)) != WEOF) && (doneWhiteSpace != 1) && (ftell(sourceFile_ptr) <= *endPos))){
                                              if(!chkWhiteSpace(sourceFile_ptr, tempFile_ptr, &newChar, *endPos, TRUE )){    
                                                 if ( ( newChar == *eqChar ) &&      
                                                      ( prevChar == *plusChar ) ) {
                                                    prevChar = 0 ;
                                                 } else {
                                                    doneWhiteSpace = 1;
                                                 }
                                              }
                                              else{
                                                 if ( newChar != ':' )        
                                                    fputwc(newChar, tempFile_ptr);
                                              }
                                           }
                                           fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-2*sizeof(wchar_t)), SEEK_SET);
                                        }
                                     } else {
                                        plusSet = 0;         
                                     }
                                  }
                                  else{
                                     plusSet = 0;
                                  }
                               }
                               if ( ( bInColonStmt ) &&            
                                    ( newChar == *leftBraceChar ) ) {   
                                  bInEqualStmt = FALSE ;
                                  bInArrayStmt = FALSE ;            
                                  bInColonStmt = FALSE ;           
                                  doneEqual = 1;
                               }
//                               if ( ( bInColonStmt ) &&
//                                    ( newChar == *leftBracketChar ) ) {
//                                  bInColonArray = TRUE ;
//                                  doneEqual = 0;
//                               }
                               if ( ( bInColonStmt ) &&  
                                    ( !bInColonArray ) &&
                                    ( newChar == *leftBracketChar ) ) {
                                  bInColonArray = TRUE ;
                                  depthColonArray ++; 
                                  doneEqual = 0;
                               }
                            }
                            fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-1*sizeof(wchar_t)), SEEK_SET);
                         } else {
                            bInEqualStmt = FALSE ;
                            bInArrayStmt = FALSE ;
                         }
                      }
                      else if((newChar == *not) ||    // LOOK FOR A "!" used to test for !=
                              (newChar == *greaterChar) ||
                              (newChar == *lessChar) ) {
                            set_not = TRUE;
                            fputwc(newChar, tempFile_ptr); // write "!" to file
                      }
                      else if(newChar == *newLine) {    // LOOK FOR A newline
                            if ( bInEqualStmt ) {
                               bFindPlus = TRUE ;
                            }
                            fputwc(newChar, tempFile_ptr); // write newline to file
                      }
                      else if ( bInEqualStmt && bInJavaScript && ( newChar == *leftParen ) ) { 
                              fputwc(newChar, tempFile_ptr);
                              numParenNest++;
                              filepos = ftell( sourceFile_ptr ) ;
                              fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-2*sizeof(wchar_t)), SEEK_SET);
                              prevChar = fgetwc(sourceFile_ptr);
                              while ( chkWhiteSpace(sourceFile_ptr, tempFile_ptr, &prevChar, *endPos, FALSE ) ) {
                                    fseek(sourceFile_ptr, (ftell(sourceFile_ptr)-2*sizeof(wchar_t)), SEEK_SET);
                                    prevChar = fgetwc(sourceFile_ptr);
                              }
                              if ( ( iswalpha( prevChar ) ) ||
                                   ( prevChar == L'_'   ) ) {
                                 bInFunction = TRUE ; 
                                 numParenFunction = numParenNest ;
                              }
                              fseek(sourceFile_ptr, filepos, SEEK_SET);
                      } 
                      else if ( bInFunction && bInJavaScript && ( newChar == *rightParen ) ) { 
                         fputwc(newChar, tempFile_ptr);
                         if ( numParenNest ) 
                            numParenNest--;
                         if ( numParenNest < numParenFunction ) {
                            bInFunction = FALSE ;
                         }
                      }
                      else{
                         fputwc(newChar, tempFile_ptr);
                         chkWhiteSpace(sourceFile_ptr, tempFile_ptr, &newChar, *endPos, FALSE ) ;
                         if ( ( bInEqualStmt ) &&        
                              ( bFindPlus    ) &&
                              ( ! iswspace(newChar) ) ) {
                            bInEqualStmt = FALSE ;
                            bInArrayStmt = FALSE ;
                            bFindPlus = FALSE ;
                         }

                      }

                      if(set_not){        // This sets or clears the not_char latch used to determine if we
                         set_not = FALSE; // have just "=" or "!=" in the file.
                         not_char = TRUE;
                      }
                      else{
                         not_char = FALSE;
                      }
                      if ( newChar == *semicolonChar ) { 
                         bInEqualStmt = FALSE ;
                         bInArrayStmt = FALSE ;          
                         bInColonStmt = FALSE ;          
                      }
                      if ( ( bInColonStmt ) &&           
                           ( newChar == *leftBraceChar ) ) {   
                         bInEqualStmt = FALSE ;
                         bInArrayStmt = FALSE ;            
                         bInColonStmt = FALSE ;           
                      }
                      if ( newChar == *escChar ) {  
                         prevChar = newChar ;                     
                      }
                   }

                   if ( bJSONPreSeg )                 
                      fputws(JSON_END, tempFile_ptr);
                }
             }

          }
       }
       if(!bInTransScript){
          fclose(sourceFile_ptr);
          fclose(tempFile_ptr);
          remove(tempName);
          return TRUE;
       }
    //
    // COPIES THE TEXT FROM SOURCE TO TEMP FROM THE END OF </SCRIPT> TAG
    // TO THE END OF FILE.
    //

       fseek(sourceFile_ptr, 0, SEEK_END);
       copyPartialFile (sourceName,&sourceFile_ptr,
                        tempName, &tempFile_ptr,
                        *endPos, ftell(sourceFile_ptr),
                        (newFile = FALSE));
       fclose(sourceFile_ptr);
       fclose(tempFile_ptr);
       rc = DosCopy(tempName, sourceName, DCPY_EXISTING); // COPIES MODIFIED TEMP BACK TO SOURCE
       remove(tempName);
       return TRUE;
}


BOOL chkWhiteSpace(FILE *sourceFile_ptr, FILE *tempFile_ptr, wchar_t *lastChar, long endPos, BOOL bCurCharWrite )
{

   int chrcnt,matchcnt;
   int done2k,not2k;
   long srcpos,tgtpos;
   int CheckLen ;
   wchar_t newChar;
   wchar_t *ptrCheck ; 

      newChar = *lastChar;

      if(!iswspace(newChar)) {                // if char after end quote is not a + then check for white space chars 
         if( ( newChar == script2k2[0]) ||
             ( newChar == comment2k[0]) ) {
            srcpos = ftell(sourceFile_ptr);
            tgtpos = ftell(tempFile_ptr);
            if ( newChar == script2k2[0]) {         /* New <script> 2k comment */
               CheckLen = wcslen( script2k2 ) ;
               ptrCheck = script2k2 ; 
            } else {
               CheckLen = wcslen( comment2k ) ;     /* Break comment text for 2k limit */
               ptrCheck = comment2k ; 
            }
            
            if ( bCurCharWrite )                     
               fputwc(newChar, tempFile_ptr);
            done2k = FALSE;
            not2k = FALSE;
            chrcnt = 1;
            while((chrcnt<CheckLen) && (!done2k)&& (ftell(sourceFile_ptr)<=endPos)){    /* Verify that 2k comment is complete */
               chrcnt++;
               ptrCheck++;
               newChar = fgetwc(sourceFile_ptr);
               fputwc(newChar, tempFile_ptr);
               if(towupper(newChar) != (wchar_t)*ptrCheck){ 
                  done2k = TRUE;                           /* Not 2k comment */
                  not2k = TRUE;
               }
            }
            if(!done2k){
               done2k = TRUE;
               while((((newChar=fgetwc(sourceFile_ptr))!=':') && (chrcnt<40) && (ftell(sourceFile_ptr)<=endPos))){
                  fputwc(newChar, tempFile_ptr);
                  chrcnt++;
               }
               if (newChar == ':'){
                  fputwc(newChar, tempFile_ptr);  /* End of 2k comment found */
               } else {
                  not2k = TRUE;               /* End of 2k comment not found */
               }
            }
            if(not2k){
               fseek(sourceFile_ptr,(srcpos),SEEK_SET);
               fseek(tempFile_ptr,(tgtpos),SEEK_SET);
               return FALSE;            /* Non-blank char and not 2k comment */
            }
         } else {
            return FALSE;               /* Non-blank char and not 2k comment */ 
         }

      }
      *lastChar = newChar;
      return TRUE;
}
