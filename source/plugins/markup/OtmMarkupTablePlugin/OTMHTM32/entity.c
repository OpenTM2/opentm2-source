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

#define MAXENTITY_READ   1700     
#define MAXENTITY_ALLOC  1800     

#define END_ENTITY2 L"{TWBENT}"

#include <ctype.h>
#include <sys\stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "scrptseg.h"
#include "otmhtm32.h"
#include "entity.h"
#include "usrcalls.h"



#define TAG_NONE       0
#define TAG_TEXT       1
#define TAG_INTAG      10
#define TAG_NORMAL     11
#define TAG_NEUTRAL    12
#define TAG_SCRIPT     13
#define TAG_STYLE      14
#define TAG_INPUT      15
#define TAG_OPTION     16
#define TAG_PARAM      17
#define TAG_JSP        18
#define TAG_LI         19

#define FONT_NONE      0
#define FONT_NEUTRAL   1


extern   char    szDocTargetLanguage[80];   /* From USRCALLS.C  */
extern   char    szDocSourceLanguage[80];   /* From USRCALLS.C  */

    BOOL   bIn2K ;
    BOOL   bInCommentCheck ;
    wchar_t   szQuote[4] ;
    BOOL   bBidiProcessing ;

// This function checks for any entity tag inside a doctype tag and causes
// TM/2 to correctly mark the translatable segments in the entity tag.
// This is done by defining a special tag <ENTITY with a  as the end delimiter
// all text between the <ENTITY and the  will be marked non translatable. In preseg
// we insert this tag to mark non translatable text in the entity tag then in post seg
// we remove them


BOOL EntityTag(char *sourceName, char *tempName )
{

    FILE *sourceFile_ptr = NULL;
    FILE *tempFile_ptr = NULL;
    wchar_t char_str[MAXENTITY_ALLOC];
    wchar_t new_str[MAXENTITY_ALLOC];
    wchar_t  cQuoteChar = 0 ;
    wchar_t * sub_str;
    int  rc;
    short sFontState = 0 ;
    short sNobrState = 0 ;
    short sValueAttrState = 0 ;
    short sValueAttrCount = 0 ;            
    short sTagState = 0 ;
    short sPrevTagState = 0 ;
    short sQuoteTransState = 0 ;
    short sQuoteNum = 0 ;
    short sQuoteTransCtl[50] ;             
    BOOL  bNonTransState = FALSE ;
    BOOL  bNonTransAttrValue = FALSE ;

    tempFile_ptr = fopen(tempName, "wb");
    sourceFile_ptr = fopen(sourceName, "rb");
    bIn2K = FALSE ;
    bInCommentCheck = FALSE ;
    if ( ( ! stricmp( szDocTargetLanguage, "ARABIC" ) ) ||
         ( ! stricmp( szDocTargetLanguage, "HEBREW" ) ) ||
         ( ! stricmp( szDocSourceLanguage, "ARABIC" ) ) ||
         ( ! stricmp( szDocSourceLanguage, "HEBREW" ) ) )
       bBidiProcessing = TRUE ;
    else
       bBidiProcessing = FALSE ;

    while (fgetws(char_str,MAXENTITY_READ,sourceFile_ptr) != NULL) {
       chk_nontrans( char_str, &bNonTransState, &bNonTransAttrValue ) ; /* Check for non-trans block */
       if ( bNonTransState ) {
          fputws(char_str,tempFile_ptr);
       } else {
          chk_font( char_str, sourceFile_ptr, &sFontState, &sNobrState, &sTagState, &sPrevTagState, &sValueAttrState, &sValueAttrCount, bNonTransAttrValue ) ;
          if ( sTagState == TAG_SCRIPT ) 
             chk_scriptquote( char_str, sourceFile_ptr, &sQuoteTransState, &sQuoteNum, 
                              &sQuoteTransCtl[0], &cQuoteChar ) ;

          if ((sub_str = wcsstr(char_str,L"<!DOCTYPE")) != NULL) {   //  Check if in a doctype tag
              if ((sub_str = wcsstr(char_str,L"[")) != NULL) {
                  chk_entity(char_str,sourceFile_ptr,tempFile_ptr);  // check for an entity tag in the same line as doctype
                  fputws(char_str,tempFile_ptr);
                  while (fgetws(char_str,MAXENTITY_READ,sourceFile_ptr) != NULL) {  // check for an entity tag in all other lines
                      chk_font( char_str, sourceFile_ptr, &sFontState, &sNobrState, &sTagState, &sPrevTagState, &sValueAttrState, &sValueAttrCount, bNonTransAttrValue ) ;
                      chk_entity(char_str,sourceFile_ptr,tempFile_ptr);
                      fputws(char_str,tempFile_ptr);
                  }
              } else {
                  fputws(char_str,tempFile_ptr);
              }
          } else {
              fputws(char_str,tempFile_ptr);
          }
       }
    }
    fclose(sourceFile_ptr);
    fclose(tempFile_ptr);

    rc = DosCopy(tempName, sourceName, DCPY_EXISTING); // COPIES MODIFIED TEMP BACK TO SOURCE
    remove(tempName);

    return TRUE;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void chk_entity(wchar_t *char_str, FILE *sourceFile_ptr, FILE *tempFile_ptr)
{
    wchar_t *sub_str;
    wchar_t *sub_str1;
    wchar_t *sub_str2;
    BOOL done = FALSE;
    long last_line;
    wchar_t new_str[MAXENTITY_ALLOC];
    BOOL system_attr = FALSE;


    if ((sub_str = wcsstr(char_str,L"<!ENTITY")) != NULL) {       // Look for an Entity tag
        memset (new_str,'\0',sizeof(new_str));
        wcsncpy(new_str,char_str,(wcslen(char_str) - wcslen(sub_str))); // if found copy the string up to the entity tag
        wcscat(new_str,L"<ENTITY_COMMENT>");                             // and insert the Entity_Comment tag so TM/2 can process as a comment
        system_attr = chk_system(sub_str);              // Check if system Follows the entity tag if so nothing is translatable
        if ((sub_str1 = wcsstr(sub_str,L"\"")) != NULL) {               // if not check for double quoted text because its translatable
            wcsncat(new_str,char_str,((wcslen(char_str) - wcslen(sub_str1))+1));
            if (!system_attr) {
                wcscat(new_str,END_ENTITY2);                     // End the comment at the begining of translatable text
            }
            if ((sub_str2 = wcsstr(sub_str1+1,L"\"")) != NULL) {           // look for the end quote of the translatable text if not on this line the look on the next line
                wcsncat(new_str,sub_str1+1,((wcslen(sub_str1) - wcslen(sub_str2))-1));
                if (!system_attr) {
                    wcscat(new_str,L"<ENTITY_COMMENT>");
                }
                wcscat(new_str,sub_str2);
                wcscat(new_str,END_ENTITY2);    
                wcscpy(char_str,new_str);
            } else {
                wcscat(new_str,sub_str1+1);
                fputws(new_str,tempFile_ptr);

                done = FALSE;
                while (((fgetws(char_str,MAXENTITY_READ,sourceFile_ptr) != NULL)) & (!done)) {   // loop utill we find the end quote
                    last_line = ftell(sourceFile_ptr);
                    memset (new_str,'\0',sizeof(new_str));
                    if ((sub_str1 = wcsstr(char_str,L"\"")) != NULL) {
                        wcsncat(new_str,char_str,(wcslen(char_str) - wcslen(sub_str1)));
                        if (!system_attr) {
                            wcscat(new_str,L"<ENTITY_COMMENT>");
                        }
                        wcscat(new_str,sub_str1);
                        wcscat(new_str,END_ENTITY2);    
                        done=TRUE;
                    } else {
                        fputws(char_str,tempFile_ptr);
                    }
                }
                wcscpy(char_str,new_str);
                fseek(sourceFile_ptr,last_line,SEEK_SET);     // set file pointer back to the previous line
            }
        }
    }

    return;
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOL chk_system(wchar_t *sub_str)
{
    wchar_t *sub_str1;
    wchar_t sub_temp[MAXENTITY_ALLOC];
    wchar_t temp_str[MAXENTITY_ALLOC];

    memset (temp_str,'\0',sizeof(temp_str));
    wcscpy(sub_temp,sub_str);
    if ((sub_str1 = wcsstr(wcsupr(sub_temp),L" SYSTEM ")) != NULL) {              // Check if system Follows the entity tag if so nothing is translatable
        wcsncat(temp_str, sub_temp, (wcslen(sub_temp) - wcslen(sub_str1)));
        if (wcsstr(temp_str, L"\"") == NULL) {                      // Check if SYSTEM is inside a quoted string
            return(TRUE);                                         // If not Then we have a valid system attribute all quoted text is non translatable
        } else {                                                   // Else system is just a text string process as normal
            return(FALSE);
        }
    }
    return(FALSE);                                          // System was not found

}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/***********************************************************************/
/*  Determine if a block of HTML should be non-translatable.           */
/***********************************************************************/
void chk_nontrans( wchar_t *sub_str, BOOL *bInNonTrans, BOOL *bInNonTransAttrValue )
{
    wchar_t   *str1, *str2 ;
    wchar_t   temp_str[80] ;
    int    i ;
    BOOL   bSkipStmt = FALSE ;

    str1 = wcsstr( sub_str, L"<!--" ) ; 
    if ( str1 ) {
       str1 += 4 ;
       str2 = wcsstr( str1, L"-->" ) ; 
       if ( str2 ) {
          for( i=0 ; str1<str2 && i<79 ; ++str1 ) {
             if ( ( ! iswspace(*str1) ) &&
                  ( *str1 != L'\xA0'   ) )       
                temp_str[i++] = *str1 ; 
          }
          temp_str[i] = 0 ;
          if ( ( ! (*bInNonTrans) ) &&
               ( ! wcsicmp( temp_str, L"STARTNON-TRANSLATABLE" ) ) ) {
             *bInNonTrans = TRUE ;
             bSkipStmt = TRUE ; 
             wcscat( sub_str, L"<!--{TWB}" ) ;
          } else {
             if ( ( *bInNonTrans ) &&
                  ( ! wcsicmp( temp_str, L"ENDNON-TRANSLATABLE" ) ) ) {
                *bInNonTrans = FALSE ; 
                bSkipStmt = TRUE ; 
                memmove( sub_str+8, sub_str, (wcslen(sub_str)+1)*sizeof(wchar_t) ) ;
                wcsncpy( sub_str, L"{TWB}-->", 8 ) ;
             }
          }
          if ( ! wcsicmp( temp_str, L"STARTNON-TRANSLATABLEVALUE" ) ) { 
             *bInNonTransAttrValue = TRUE ;
          } else {
             if ( ! wcsicmp( temp_str, L"ENDNON-TRANSLATABLEVALUE" ) ) {
                *bInNonTransAttrValue = FALSE ; 
             }
          }
       }
    }

    if ( ( *bInNonTrans ) &&
         ( ! bSkipStmt  ) ) {
       for( str1=sub_str ; *str1 ; ++str1 ) {
          if ( IsDBCS( *str1 ) ) {     
             ++str1 ; 
             continue ; 
          }
          if ( ! wcsncmp( str1, L"<!--", 4 ) ) {
             memmove( str1+8, str1, (wcslen(str1)+1)*sizeof(wchar_t) ) ;
             wcsncpy( str1, L"{TWB}-->", 8 ) ;
             str1 += 8 ;
          } else 
          if ( ! wcsncmp( str1, L"-->", 3 ) ) {
             str1 += 3 ;
             memmove( str1+9, str1, (wcslen(str1)+1)*sizeof(wchar_t) ) ;
             wcsncpy( str1, L"<!--{TWB}", 9 ) ;
             str1 += 9 ;
          }
       }
    }

    return ;
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/***********************************************************************/
/*  Determine if <FONT> tag is used properly.                          */
/*  Handle <INPUT> tag to determine if VALUE attribute is translatable.*/
/*  Break segments bigger than 2K limit.                               */
/***********************************************************************/
void chk_font(wchar_t *Text, FILE *InFile, short *FontState, short *NobrState, 
              short *TagState, short *PrevTagState, short *ValueAttrState,
              short *ValueAttrCount, BOOL bNonTransAttrValue )
{

//#define TAG_NONE       0
//#define TAG_TEXT       1
//#define TAG_INTAG      10
//#define TAG_NORMAL     11
//#define TAG_NEUTRAL    12
//#define TAG_SCRIPT     13
//#define TAG_STYLE      14
//#define TAG_INPUT      15
//
//#define FONT_NONE      0
//#define FONT_NEUTRAL   1

#define LEAD_FONT_NONE     0
#define LEAD_FONT_TAG      1
#define LEAD_FONT_TEXT     2
#define LEAD_FONT_SKIPTAG  3
#define LEAD_FONT_ENDTAG   4


    static wchar_t  TAGLIST_NEUTRAL[160] =
    L" A ABBR ACRONYM B BDI BDO BIG BLINK CITE CODE DFN EM I ILAYER IMG KBD MARK METER PROGRESS Q RP S SAMP SMALL SPAN STRIKE STRONG SUB SUP TIME TT U VAR WBR " ;


    wchar_t   *TokenDelimit = L" =\n\r\t";   

    wchar_t   szTemp[MAXENTITY_ALLOC];
    wchar_t   szTagName[20] ;
    wchar_t   szTagName2[20] ;
    wchar_t   szTempTag[22] ;
    wchar_t   *ptrChar, *ptrChar2, *ptrChar3, *ptrTag ;
    wchar_t   *ptrInputStart ;
    ULONG  ulFilePos ;
    short  sFontState ;
    short  sNobrState ;
    short  sValueAttrState ;
    short  sValueAttrCount ;
    short  sTagState ;
    short  sPrevTagState ;
    short  sAdjustment ;
    short  sLeadFontState ;
    short  sFindState ;
    BOOL   bFontTag ;


    sFontState = *FontState ;
    sNobrState = *NobrState ;
    sTagState = *TagState ;
    sPrevTagState = *PrevTagState ;
    sValueAttrState = *ValueAttrState ;
    sValueAttrCount = *ValueAttrCount ;

    if ( ( sValueAttrState == TAG_INPUT  ) ||
         ( sValueAttrState == TAG_OPTION ) ||
         ( sValueAttrState == TAG_PARAM  ) ||
         ( sValueAttrState == TAG_LI     ) ) {       
       wcscpy( szTemp, Text ) ;
       wcsupr( szTemp );
       sAdjustment = 0 ;                             
       for( ptrChar2=wcstok(szTemp,TokenDelimit) ;
            ptrChar2 ; 
            ptrChar2=wcstok(NULL,TokenDelimit) ) {   // RENAME VALUE ATTR TO mod+VALUE
          if ( ! wcscmp(ptrChar2,L"VALUE") ) {         // AND DEFINE THAT ATTR IN MARKUP TABLE
             ptrChar3 = Text + sAdjustment + (ptrChar2-szTemp) ;
             memmove( ptrChar3+4, ptrChar3, (wcslen(ptrChar3)+1)*sizeof(wchar_t) ) ;
             wcsncpy( ptrChar3, L"mod+", 4 ) ;
             sAdjustment += 4 ;
             --(sValueAttrCount) ;                 
             if ( sValueAttrCount <= 0 ) {
                sValueAttrState = TAG_NONE ;
                break ;
             }
          }
       }
    }

    for( ptrChar=Text ; *ptrChar ; ++ptrChar ) {
       if ( iswspace(*ptrChar) )
          continue ;

       if ( sTagState > TAG_INTAG ) {
          if ( ( sTagState != TAG_SCRIPT ) &&
               ( sTagState != TAG_STYLE  ) &&  
               ( sTagState != TAG_JSP    ) ) {
             if ( *ptrChar == '>' ) {
                if ( ( sPrevTagState == TAG_NORMAL ) &&
                     ( sTagState == TAG_NEUTRAL    ) )
                   sPrevTagState = TAG_NORMAL ;
                else
                   sPrevTagState = sTagState ;
                sTagState = TAG_NONE ;
             }
             continue ;
          }
       }

       if ( ( *ptrChar == '<' ) &&
            ( ( iswalpha( *(ptrChar+1) ) ) ||
              ( ( *(ptrChar+1) == '/'     ) &&
                ( iswalpha( *(ptrChar+2) ) ) ) ||
              ( ( *(ptrChar+1) == '\\'   ) &&          /* <\/xxxx> */
                ( *(ptrChar+2) == '/'    ) &&
                ( iswalpha( *(ptrChar+3) ) ) ) ||
              ( *(ptrChar+1) == '!'       ) ) ) {      /* Comment */
          ptrTag = ptrChar ;
          if ( *(ptrChar+1) == '\\' )                
             wcsncpy( szTagName, ptrChar+2, sizeof(szTagName)/sizeof(wchar_t) ) ;
          else
          wcsncpy( szTagName, ptrChar+1, sizeof(szTagName)/sizeof(wchar_t) ) ;
          szTagName[sizeof(szTagName)/sizeof(wchar_t)-1] = 0 ;
          wcstok( szTagName, L" >\n\t\r" ) ;
          wcsupr( szTagName ) ;
          if ( ! wcscmp( szTagName, L"SCRIPT" ) ) {
             sTagState = TAG_SCRIPT ;
          } else
          if ( ! wcscmp( szTagName, L"STYLE" ) ) {
             sTagState = TAG_STYLE ;
          } else
          if ( ( ! wcscmp( szTagName, L"/SCRIPT" ) ) &&
               ( sTagState == TAG_SCRIPT ) ) {
             sTagState = TAG_NORMAL ;
             sFontState = FONT_NONE ;
             sNobrState = FONT_NONE ;
          } else
          if ( ( ! wcscmp( szTagName, L"/STYLE" ) ) &&
               ( sTagState == TAG_STYLE ) ) {
             sTagState = TAG_NORMAL ;
             sFontState = FONT_NONE ;
             sNobrState = FONT_NONE ;
          } else 
          if ( ( ! wcscmp( szTagName, L"INPUT"  ) ) ||       
               ( ! wcscmp( szTagName, L"OPTION" ) ) ||
               ( ! wcscmp( szTagName, L"PARAM"  ) ) ||
               ( ! wcscmp( szTagName, L"LI"  ) ) ) {         
             chk_valueattr( szTagName, ptrTag, InFile, &sValueAttrState, sTagState, &sValueAttrCount, bNonTransAttrValue ) ;
             sAdjustment = 0 ;                              
             if ( sValueAttrState != TAG_NONE ) {
                wcscpy( szTemp, ptrTag ) ;
                wcsupr( szTemp );
                for( ptrChar2 = wcstok(szTemp,TokenDelimit) ;
                     ptrChar2 ; ptrChar2=wcstok(NULL,TokenDelimit) ) {   
                   if ( ! wcscmp(ptrChar2,L"VALUE") ) {        
                      ptrChar3 = ptrTag + sAdjustment + (ptrChar2-szTemp) ;
                      memmove( ptrChar3+4, ptrChar3, (wcslen(ptrChar3)+1)*sizeof(wchar_t) ) ;
                      wcsncpy( ptrChar3, L"mod+", 4 ) ;
                      ptrChar += 4 ;
                      sAdjustment += 4 ;           
                      --(sValueAttrCount) ;
                      if ( sValueAttrCount <= 0 ) {
                         sValueAttrState = TAG_NONE ;
                         break ;
                      }
                   }
                }
             }
             if ( ( sTagState != TAG_SCRIPT ) &&
                  ( sTagState != TAG_STYLE  ) ) {
                sTagState = TAG_NORMAL ;
                sFontState = FONT_NONE ;
                sNobrState = FONT_NONE ;
             }
          } else
          if ( ( sTagState == TAG_SCRIPT ) ||
               ( sTagState == TAG_STYLE  ) ) {
             continue ;
          } else
          if ( ( bBidiProcessing ) &&              
               ( ! wcscmp( szTagName, L"HTML"  ) ) ) {
             chk_bidihtml( ptrTag ) ;
          } else
          if ( ( ! wcscmp( szTagName, L"FONT"  ) ) ||
               ( ( ! wcscmp( szTagName, L"/FONT" ) ) &&  /* Unmatched </FONT> */
                 ( sFontState == FONT_NONE    ) ) ||
               ( ! wcscmp( szTagName, L"NOBR"  ) ) ||              
                 ( ( ! wcscmp( szTagName, L"/NOBR" ) ) && 
                   ( sNobrState == FONT_NONE ) ) ) { 
             if ( ( ! wcscmp( szTagName, L"NOBR"  ) ) ||
                  ( ! wcscmp( szTagName, L"/NOBR" ) ) ) 
                bFontTag = FALSE ;
             else
                bFontTag = TRUE ; 
             sTagState = TAG_NORMAL ;
             if ( ( sPrevTagState == TAG_NEUTRAL ) ||   /* Need to find trailing state */
                  ( sPrevTagState == TAG_TEXT    ) ||
                  ( ( bFontTag ) &&                                
                    ( ! wcscmp( szTagName, L"FONT" ) ) &&
                    ( sPrevTagState == TAG_NORMAL ) ) ) {
                ulFilePos = ftell ( InFile ) ;
                if ( sPrevTagState == TAG_NORMAL ) 
                   sLeadFontState = LEAD_FONT_SKIPTAG ;
                else
                   sLeadFontState = LEAD_FONT_NONE ;
                ptrChar2 = ptrChar + 1 ;
                while( ptrChar2 ) {
                   if ( sLeadFontState == LEAD_FONT_SKIPTAG ) {    
                      for( ; *ptrChar2 && *ptrChar2!='>' ; ++ptrChar2 ) ;
                      if ( *ptrChar2 ) 
                         sLeadFontState = LEAD_FONT_TEXT ;
                   }
                   if ( sLeadFontState == LEAD_FONT_TEXT ) {       
                      for( ; *ptrChar2 && *ptrChar2!='<' ; ++ptrChar2 ) ;
                      ptrChar3 = ptrChar2 ;
                   } else {
                      for( ; *ptrChar2 && *ptrChar2!='>' ; ++ptrChar2 ) ;
                      if ( *ptrChar2 )
                         for( ptrChar3=ptrChar2+1; *ptrChar3 && iswspace(*ptrChar3) ; ++ptrChar3 ) ;
                      else
                         ptrChar3 = ptrChar2 ;
                   }
                   if ( ! *ptrChar3 ) {                    /* Find char after this tag */
                      if ( *ptrChar2 ) 
                         sFindState = 2 ;
                      else
                         sFindState = 1 ;
                      while (fgetws(szTemp,MAXENTITY_READ,InFile) != NULL) {
                         if ( sFindState == 1 ) 
                            ptrChar2 = szTemp ;
                         if ( sLeadFontState == LEAD_FONT_SKIPTAG ) {
                            for(  ; *ptrChar2 && *ptrChar2!='>' ; ++ptrChar2 ) ;
                            if ( ! *ptrChar2 ) 
                               continue ;
                            sLeadFontState = LEAD_FONT_TEXT ;
                         }
                         if ( sLeadFontState == LEAD_FONT_TEXT ) {
                            for( ; *ptrChar2 && *ptrChar2!='<' ; ++ptrChar2 ) ;
                            if ( ! *ptrChar2 ) 
                               continue ;
                            ptrChar3 = ptrChar2 ;
                            break ;
                         } else {
                            if ( sFindState == 1 ) {
                               for( ; *ptrChar2 && *ptrChar2!='>' ; ++ptrChar2 ) ;
                               if ( ! *ptrChar2 )
                                  continue ;
                               sFindState = 2 ;
                               ptrChar3 = ptrChar2 + 1 ;
                            } else {
                               ptrChar3 = szTemp ;
                            }
                            for( ; *ptrChar3 && iswspace(*ptrChar3) ; ++ptrChar3 ) ;
                         }
                         if ( *ptrChar3 ) 
                            break ;
                      }
                   }
                   if ( ( ptrChar3 ) &&                    /* If followed by another tag,   */
                        ( *ptrChar3 == '<' ) &&            /*  then check if neutral or not */
                        ( ( iswalpha( *(ptrChar3+1) ) ) ||
                          ( ( *(ptrChar3+1) == '/'     ) &&
                            ( iswalpha( *(ptrChar3+2) ) ) ) ||
                          ( ( *(ptrChar+1) == '\\'   ) &&          /* <\/xxxx> */
                            ( *(ptrChar+2) == '/'    ) &&
                            ( iswalpha( *(ptrChar+3) ) ) ) ||
                          ( *(ptrChar3+1) == '!'         ) ) ) {  /* Comment */
                      wcsncpy( szTagName, ptrChar3+1, sizeof(szTagName)/sizeof(wchar_t) ) ;
                      szTagName[sizeof(szTagName)/sizeof(wchar_t)-1] = 0 ;
                      wcstok( szTagName, L" >\n\t\r" ) ;
                      wcsupr( szTagName ) ;
                      if ( szTagName[0] == '/' )
                         swprintf( szTempTag, L" %s ", &szTagName[1] ) ;
                      else
                      if ( szTagName2[0] == '\\' )                       
                         swprintf( szTempTag, L" %s ", &szTagName2[2] ) ;
                      else
                          swprintf( szTempTag, L" %s ", szTagName ) ;
                      if ( ( wcsstr( TAGLIST_NEUTRAL , szTempTag ) ) || /* Next tag is neutral */
                           ( ! wcscmp( szTempTag, L" FONT "  ) ) ) { 
////                     sTagState = TAG_NEUTRAL ;
                         ptrChar2 = ptrChar3 + 1 ; /* Look for next tag */
                         if (  sLeadFontState >= LEAD_FONT_TAG ) {   
                            if ( ! wcscmp( szTagName, L"/FONT"  ) ) 
                               sLeadFontState = LEAD_FONT_ENDTAG ;
                            else
                               if ( sLeadFontState == LEAD_FONT_TEXT ) 
                                  sLeadFontState = LEAD_FONT_SKIPTAG ; 
                               else 
                                  sLeadFontState = LEAD_FONT_TEXT ;
                         }
                      } else {
                         break ;         /* Non-neutral tag found */
                      }
                   } else {
                      if ( wcschr( L"\"\'", *ptrChar2 ) ) 
                         sTagState = TAG_NONE ;
                      else 
                         if ( ( sLeadFontState == LEAD_FONT_NONE   ) ||      
                              ( sLeadFontState == LEAD_FONT_ENDTAG ) )  {
                            sTagState = TAG_NEUTRAL ; /* Translatable text */
                         }
                      break ;
                   }
                }
                fseek( InFile, ulFilePos, 0 ) ;

                if ( sTagState == TAG_NEUTRAL ) {
                   if ( *(ptrChar+1) == '/' ) {
                      memmove( ptrChar+7, ptrChar+6, (wcslen(ptrChar+6)+1)*sizeof(wchar_t) ) ;
                      *(ptrChar+6) = 'Q' ;
                   } else {
                      memmove( ptrChar+6, ptrChar+5, (wcslen(ptrChar+5)+1)*sizeof(wchar_t) ) ;
                      *(ptrChar+5) = 'Q' ;
                   }
                   if ( bFontTag ) 
                      sFontState = FONT_NEUTRAL ;
                   else
                      sNobrState = FONT_NEUTRAL ;
                }
             }
          } else
          if ( ! wcscmp( szTagName, L"/FONT" ) ) {
             if ( sFontState == FONT_NEUTRAL ) {
                memmove( ptrChar+7, ptrChar+6, (wcslen(ptrChar+6)+1)*sizeof(wchar_t) ) ;
                *(ptrChar+6) = 'Q' ;
                sTagState = TAG_NEUTRAL ;
                sFontState = FONT_NONE ;
             } else {
                 sTagState = TAG_NORMAL ;
             }
          } else
          if ( ! wcscmp( szTagName, L"/NOBR" ) ) {     
             if ( sNobrState == FONT_NEUTRAL ) {
                memmove( ptrChar+7, ptrChar+6, (wcslen(ptrChar+6)+1)*sizeof(wchar_t) ) ;
                *(ptrChar+6) = 'Q' ;
                sTagState = TAG_NEUTRAL ;
                sNobrState = FONT_NONE ;
             } else {
                 sTagState = TAG_NORMAL ;
             }
          } else {
             if ( szTagName[0] == '/' )
                swprintf( szTempTag, L" %s ", &szTagName[1] ) ;
             else
             if ( szTagName2[0] == '\\' )                        
                swprintf( szTempTag, L" %s ", &szTagName2[2] ) ;
             else
                swprintf( szTempTag, L" %s ", szTagName ) ;
             if ( ( wcsstr( TAGLIST_NEUTRAL , szTempTag ) ) || /* Next tag is neutral */
                  ( ! wcscmp( szTempTag, L" FONT "  ) ) ) {   
                sTagState = TAG_NEUTRAL ;
             } else {
                sTagState = TAG_NORMAL ;
                sFontState = FONT_NONE ;
                sNobrState = FONT_NONE ;
             }
          }
          continue ;
       }

       if ( ! wcsncmp( ptrChar, L"<!--", 4 ) ) {
          bInCommentCheck = TRUE ; 
       }
       if ( ! wcsncmp( ptrChar, L"-->", 3 ) ) {
          sTagState = TAG_NONE ;
          sPrevTagState = TAG_NORMAL ;
          if ( bInCommentCheck ) {         /* Change '--->' to '- -->' */
             bInCommentCheck = FALSE ;                     
             if ( ( ptrChar > Text ) &&
                  ( *(ptrChar-1) == '-' ) ) {
                memmove( ptrChar+1, ptrChar, (wcslen(ptrChar)+1)*sizeof(wchar_t) ) ;
                *ptrChar = ' ' ; 
             }
          }
          ptrChar += 2 ;
       } else {
          if ( ! wcsncmp( ptrChar, L"&nbsp;", 6 ) ) {
             ptrChar += 5 ;
          } else {
             if ( ! wcsncmp( ptrChar, L"[TWB", 4 ) ) { /* JSP internal tags */
                ptrChar2 = wcschr( ptrChar, ']' ) ;
                if ( ptrChar2 ) {
                   if ( sTagState == TAG_SCRIPT ) {
                      if ( ! wcsncmp( ptrChar, L"[TWBSTART] [TWBLT]%", 19 ) ) { 
                            memmove( ptrChar+7, ptrChar-1, (wcslen(ptrChar-1)+1)*sizeof(wchar_t) ) ;
                            wcsncpy( ptrChar-1, L"{TWBSCR}", 8 ) ; 
                            ptrChar += 8 ;
                            sTagState = TAG_JSP ;
                            sPrevTagState = TAG_SCRIPT ;
                      }
                   } else {
                      if ( sTagState == TAG_JSP ) {
                         if ( ! wcsncmp( ptrChar, L"[TWBGT] [TWBSTOP] ", 18 ) ) { 
                               memmove( ptrChar+18+10, ptrChar+18, (wcslen(ptrChar+18)+1)*sizeof(wchar_t) ) ;
                               wcsncpy( ptrChar+18, L"<XSCRCONT:", 10 ) ; 
                               sTagState = TAG_SCRIPT ;
                               sPrevTagState = TAG_TEXT ;
                         }
                      } else {
                         ptrChar = ptrChar2 ;
                         sTagState = TAG_NONE ;
                         sPrevTagState = TAG_NORMAL ;
                      }
                   }
///                      } else
///                      if ( !wcsncmp( ptrChar, L"[TWBSTOP]", 9 ) ) {
///                         memmove( ptrChar+9+10, ptrChar+9, (wcslen(ptrChar+9)+1)*sizeof(wchar_t) ) ;
///                         wcsncpy( ptrChar+9, L"<XSCRCONT:", 10 ) ; 
///                      }

                } else {
                   sPrevTagState = TAG_TEXT ;  /* Translatable text */
                }
             } else {
                sPrevTagState = TAG_TEXT ;  /* Translatable text */
             }
          }
       }
    }
    *FontState = sFontState ;
    *NobrState = sNobrState ;
    *TagState  = sTagState ;
    *PrevTagState = sPrevTagState ;
    *ValueAttrState = sValueAttrState ;
    *ValueAttrCount = sValueAttrCount ;

    if ( bBidiProcessing )                 /* Make Bidi attributes translatable */
       chk_bidiattr( Text, InFile ) ;

    /*************************************************************************/
    /*                                                                       */
    /*  If maximum number of characters was read, then add special 2K        */
    /*  tag to break this line into smaller pieces for TM analysis.          */
    /*                                                                       */
    /*************************************************************************/
    if ( ( ( wcslen(Text) > MAXENTITY_READ-2 ) ||    // (1024-2 )           
           ( bIn2K ) ) &&
         ( !wcschr( Text, '\n' ) ) ) {
       if ( !wcschr( Text, '>'  ) ) {
          ptrChar = wcschr( Text, '<' ) ;
          if ( ptrChar )
             ptrChar2 = wcschr( ptrChar+1, '<' ) ;
          else
             ptrChar2 = 0 ;
          if ( ( bIn2K ) ||
               ( ( ptrChar ) &&
                 ( !ptrChar2 ) &&
                 ( ( iswalpha( *(ptrChar+1) ) ) ||
                   ( ( *(ptrChar+1) == '/'    ) &&
                     ( iswalpha( *(ptrChar+2) ) ) ) ||
                   ( ( *(ptrChar+1) == '\\'   ) &&          /* <\/xxxx> */
                     ( *(ptrChar+2) == '/'    ) &&
                     ( iswalpha( *(ptrChar+3) ) ) ) ) ) ) {
             if ( ! bIn2K )
                wcscpy( szQuote, L"   " ) ;
             bIn2K = TRUE ;
             for( ptrChar=Text ; *ptrChar ; ++ptrChar ) {
                if ( ( *(ptrChar) == '\\' ) &&    /* Skip escaped quotes  */
                     ( ( *(ptrChar+1) == '\"' ) ||
                       ( *(ptrChar+1) == '\'' ) ) ) {
                   ++ptrChar ;
                   continue ;
                }
                if ( ( *ptrChar == '\"' ) ||      /* Process single/double quote */
                     ( *ptrChar == '\'' ) ) {
                   if ( *(ptrChar+1) == *ptrChar ) { /* Skip consecutive quotes  */
                      ++ptrChar ;
                      continue ;
                   }
                   if ( szQuote[1] == *ptrChar ) {/* If 1st quote is a match,  */
                      if ( szQuote[2] == ' '  )   /* If no 2nd quote char,     */
                         szQuote[1] = ' ' ;       /* then end of 1st quote     */
                   } else {
                      if ( szQuote[2] == *ptrChar ) /* If 2nd quote is a match,*/
                         szQuote[2] = ' ' ;       /* then end of 2nd quote     */
                      else
                        if ( szQuote[1] == ' ' )  /* If no 1st quote yet,      */
                            szQuote[1] = *ptrChar ;/* Set 1st quote char        */
                        else
                           szQuote[2] = *ptrChar ;/* Set 2nd quote char        */
                   }
                }
             }
             wcscat( Text, L"{TWB2K}" ) ;
             memset( szTemp, 0, 4*sizeof(wchar_t) ) ;
             if ( szQuote[2] != ' ' ) {
                szTemp[2] = szQuote[2] ;
                wcscat( Text, &szTemp[2] ) ;
             }
             if ( szQuote[1] != ' ' ) {
                szTemp[0] = szQuote[1] ;
                wcscat( Text, szTemp ) ;
             }
             wcscat( Text, L">\n<TWB2K" ) ;
             if ( szTemp[0] )
                wcscat( Text, szTemp ) ;
             if ( szTemp[2] )
                wcscat( Text, &szTemp[2] ) ;
             wcscat( Text, L"{TWB2K}" ) ;
          }
       } else {
          bIn2K = FALSE ;                                          
          ptrChar = wcsrchr( Text, '<' ) ;
          if ( ( ptrChar ) &&                       /* If possible partial tag, */
               ( wcslen(ptrChar) < 10 ) ) {         /*  then find previous tag */
             ptrChar2 = ptrChar ;
             *ptrChar2 = ' ' ;
             ptrChar = wcsrchr( Text, '<' ) ;
             *ptrChar2 = '<' ;
          }                                         /* If neutral tag, then force newline */
          if ( ( ptrChar > Text ) &&
               ( ( iswalpha( *(ptrChar+1) ) ) ||          /* <xxxx> */
                 ( ( *(ptrChar+1) == '/'    ) &&          /* </xxxx> */
                   ( iswalpha( *(ptrChar+2) ) ) ) ||
                 ( ( *(ptrChar+1) == '\\'   ) &&          /* <\/xxxx> */
                   ( *(ptrChar+2) == '/'    ) &&
                   ( iswalpha( *(ptrChar+3) ) ) ) ) ) {
             wcsncpy( szTagName2, ptrChar+1, sizeof(szTagName2)/sizeof(wchar_t) ) ;
             szTagName2[sizeof(szTagName2)/sizeof(wchar_t)-1] = 0 ;
             wcstok( szTagName2, L" >\n\t\r" ) ;
             wcsupr( szTagName2 ) ;
             if ( szTagName2[0] == '/' )
                swprintf( szTempTag, L" %s ", &szTagName2[1] ) ;
             else
             if ( szTagName2[0] == '\\' )                        
                swprintf( szTempTag, L" %s ", &szTagName2[2] ) ;
             else
                 swprintf( szTempTag, L" %s ", szTagName2 ) ;
             if ( wcsstr( TAGLIST_NEUTRAL , szTempTag ) ) {
                memmove( ptrChar+15, ptrChar, (wcslen(ptrChar)+1)*sizeof(wchar_t) ) ;
                wcsncpy( ptrChar, L"<TWB2K>{TWB2K}\n", 15 ) ;
             }
          }
       }
    } else {
       bIn2K = FALSE ;
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/***************************************************************************/
/*  Handle <INPUT>, <OPTION>, <PARAM> tags to determine if VALUE attribute */
/*  contains translatable text or not.                                     */
/***************************************************************************/
void chk_valueattr(wchar_t *Tag, wchar_t *Text, FILE *InFile, short *ValueAttrState, short TagState,
               short *ValueAttrCount, BOOL bNonTransAttrValue )
{

    wchar_t   *TokenDelimit = L" =\n\r\t";   
    wchar_t   *TokenAttrDelimit = L" =\n\r\t\'\">";  
    wchar_t   szTemp[MAXENTITY_ALLOC];
    wchar_t   szTemp2[MAXENTITY_ALLOC];
    wchar_t   cQuoteChar = 0 ;
    wchar_t   *ptrChar, *ptrChar2, *ptrChar3 ;
    ULONG     ulFilePos ;
    ULONG     i ;  
    BOOL      bNonTrans ;


    *ValueAttrState = TAG_NONE ; 
    *ValueAttrCount = 0 ;
    wcscpy( szTemp, Text ) ;
    wcsupr( szTemp ) ;
    cQuoteChar = NULL ;
    ulFilePos = ftell( InFile ) ;
    for(  ptrChar=szTemp ; *ptrChar!='>'||cQuoteChar!=0 ; ptrChar++ ) {
       if ( *ptrChar == 0 ) {
          if ( ( wcslen(szTemp) > MAXENTITY_ALLOC-2 ) ||
               ( TagState == TAG_SCRIPT ) ||
               ( TagState == TAG_STYLE  ) ) {
             break ;
          }

          szTemp2[0] = 0 ;
          if ( fgetws( szTemp2, MAXENTITY_READ,InFile ) == 0 ) 
             break ;

          wcsupr( szTemp2 ) ;
          if ( (wcslen(szTemp)+wcslen(szTemp2)) < MAXENTITY_ALLOC-1 ) {
             wcscat( szTemp, szTemp2 ) ;
          } else {
             wcsncpy( ptrChar, szTemp2, MAXENTITY_ALLOC-wcslen(szTemp)-1 ) ; 
             szTemp[MAXENTITY_ALLOC-1] = 0 ;
          }
          --ptrChar ; 
          continue ;
       }
       if ( IsDBCS( *ptrChar ) ) {     
          ++ptrChar ; 
          continue ; 
       }

       if ( ( *ptrChar == L'\\' ) &&           /* Skip escaped quote within quoted string */
            ( cQuoteChar != 0   ) &&
            ( ( *(ptrChar+1) == L'\"' ) ||
              ( *(ptrChar+1) == L'\'' ) ) ) {
          ++ptrChar ; 
          continue ; 
       }                                       /* Ignored quoted text when looking for end */
       if ( ( *ptrChar == L'\"' ) ||
            ( *ptrChar == L'\'' ) ) {
          if ( cQuoteChar == 0 ) {
             cQuoteChar = *ptrChar ;
          } else {
             if ( cQuoteChar == *ptrChar ) {
                cQuoteChar = 0 ;
             }
          }
       }

       if ( ( *ptrChar == L'%' ) &&        /* Ignore end of JSP statement */
            ( *(ptrChar+1) == L'>' ) ) {
          ++ptrChar ; 
          continue ; 
       }
    }
    *ptrChar = 0 ;
    fseek( InFile, ulFilePos, SEEK_SET ) ;

    if ( bNonTransAttrValue )                   
       bNonTrans = TRUE ;
    else
       bNonTrans = FALSE ;
    ptrChar = wcstok( szTemp, TokenDelimit ) ;
    while( ptrChar ) {
       if ( ( ! wcscmp( Tag, L"INPUT" )     ) &&
            ( ! wcscmp( ptrChar, L"TYPE" ) ) ) {
          ptrChar = wcstok( NULL, TokenDelimit ) ;
          if ( ( ! wcsstr( ptrChar, L"BUTTON" ) ) &&
               ( ! wcsstr( ptrChar, L"RESET"  ) ) &&
               ( ! wcsstr( ptrChar, L"SEARCH" ) ) &&  
               ( ! wcsstr( ptrChar, L"SUBMIT" ) ) &&
               ( ! wcsstr( ptrChar, L"TEL" ) ) &&     
               ( ! wcsstr( ptrChar, L"TEXT"   ) ) ) {
             bNonTrans = TRUE ; 
          }
       }
       if ( ! wcscmp( Tag, L"LI" ) ) 
          bNonTrans = TRUE ; 
       if ( ( bNonTrans ) &&                      
            ( ! wcscmp( ptrChar, L"VALUE" ) ) ) { /* Non-translatable */
          if ( ! wcscmp( Tag, L"INPUT" ) ) 
             *ValueAttrState = TAG_INPUT ; 
          else
          if ( ! wcscmp( Tag, L"OPTION" ) ) 
             *ValueAttrState = TAG_OPTION ; 
          else
          if ( ! wcscmp( Tag, L"PARAM" ) ) 
             *ValueAttrState = TAG_PARAM ; 
          else
          if ( ! wcscmp( Tag, L"LI" ) )            
             *ValueAttrState = TAG_LI ; 
          ++(*ValueAttrCount) ;  /* Find all VALUE words */
       }
       if ( ! wcscmp( Tag, L"OPTION" ) ) {
           if ( ! wcscmp( ptrChar, L"VALUE" ) ) {
               ptrChar = wcstok( NULL, TokenAttrDelimit ) ;
               if ( NULL != ptrChar ) {
                   if ( ( ! wcsncmp( ptrChar, L"HTTP://",  7 ) ) ||
                        ( ! wcsncmp( ptrChar, L"HTTPS://", 8 ) ) ||
                        ( ! wcsncmp( ptrChar, L"//WWW.",   6 ) ) ||
                        ( ! wcsncmp( ptrChar, L"WWW.",     4 ) ) ) {
                    *ValueAttrState = TAG_OPTION ; 
                   }
               }
           }
       }
       ptrChar = wcstok( NULL, TokenDelimit ) ;
    }
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/***********************************************************************/
/*  Handle quoted text when special quote comment is used to define    */
/*  which of the strings on a line are to be translatable.             */
/***********************************************************************/
void chk_scriptquote(wchar_t *Text, FILE *InFile, short *sQuoteState,
                     short *sQuoteNum, short sQuoteCtl[], wchar_t *QuoteChar )
{

#define QUOTE_NONE     0
#define QUOTE_LINE_1   1
#define QUOTE_LINE_2   2
#define QUOTE_BLOCK    3

    wchar_t   szTemp[MAXENTITY_ALLOC];
    wchar_t   *ptrChar, *ptrChar2, *ptrChar3 ;
    wchar_t   *ptr, *ptr1, *ptr2, *ptr3 ;
    wchar_t   sBLANK_DELIMIT[5] = L" \n\t\r" ;
    wchar_t   sQUOTE[6] = L"QUOTE" ;
    wchar_t   sSTART[6] = L"START" ;
    wchar_t   sTRANSLATABLE[13] = L"TRANSLATABLE" ;
    short  i ;
    BOOL   bSkipChar ;


    if ( *QuoteChar == 0 ) {
       if ( *sQuoteState == QUOTE_LINE_2 ) 
          *sQuoteState = QUOTE_NONE ;
       if ( *sQuoteState == QUOTE_LINE_1 )
          *sQuoteState = QUOTE_LINE_2 ;
       *sQuoteNum = 0 ; 
    }

    for( ptrChar=Text ; *ptrChar ; ++ptrChar ) {
       if ( iswspace(*ptrChar) )
          continue ;
       if ( IsDBCS(*ptrChar) ) {
          ++ptrChar ;
          continue ;
       }

       /********************************************************************/
       /*  Handle user-defined block of lines which will contain certain   */
       /*  quoted translatable text.                                       */
       /********************************************************************/
       if ( ( *ptrChar     == '/' ) &&     
            ( *(ptrChar+1) == '/' ) ) {
          wcscpy( szTemp, ptrChar+2 ) ;
          ptr  = wcstok( szTemp, sBLANK_DELIMIT ) ;
          ptr1 = wcstok( NULL, sBLANK_DELIMIT ) ;
          ptr2 = wcstok( NULL, sBLANK_DELIMIT ) ;

          if ( ( ptr2 ) &&
               ( ( ( ! wcscmp( ptr, sQUOTE ) ) &&
                   ( ! wcscmp( ptr1, sTRANSLATABLE ) ) ) ||
                 ( ( ! wcscmp( ptr1, sQUOTE ) ) &&
                   ( ! wcscmp( ptr2, sTRANSLATABLE ) ) ) ) ) {
             *QuoteChar = 0 ; 
             for( i=0 ; i<50 ; sQuoteCtl[i++]=0 ) ;
             if ( ! wcscmp( ptr, sQUOTE ) ) {      /* Single line override */
                *sQuoteState = QUOTE_LINE_1 ;
                for( ptr=ptr2 ; ptr ; ptr=wcstok(NULL,sBLANK_DELIMIT) ) { 
                   i = _wtoi(ptr);
                   if ( (i>0) && (i<50) ) 
                      sQuoteCtl[i] = 1 ;
                }
             } else {                                    /* Block override */
                if ( ! wcscmp( ptr, sSTART ) ) {
                   *sQuoteState = QUOTE_BLOCK ;
                   ptr = wcstok(NULL,sBLANK_DELIMIT) ; 
                   if ( ! ptr ) {          /* All quoted text translatable */
                      for( i=0 ; i<50 ; sQuoteCtl[i++]=1 ) ;    
                   } else {
                      for( ; ptr ; ptr=wcstok(NULL,sBLANK_DELIMIT) ) { 
                         i = _wtoi(ptr);
                         if ( (i>0) && (i<50) ) 
                            sQuoteCtl[i] = 1 ;
                      }
                   }
                } else {
                   *sQuoteState = QUOTE_NONE ;
                }
             }
          }
          if ( *QuoteChar == 0 )   /* If not in the middle of a quoted string */
             break ;
       }

       if ( *sQuoteState > QUOTE_NONE ) {

          /********************************************************************/
          /*  Ignore text in block comment.                                   */
          /********************************************************************/
          if ( *sQuoteState > 10 ) {
             if ( ( *ptrChar     == '*' ) &&
                  ( *(ptrChar+1) == '/' ) ) {
                *sQuoteState -= 10 ;
                ++ptrChar ;
             }
             continue ;
          }
          if ( ( *ptrChar == '/' ) &&
               ( *(ptrChar+1) == '*' ) ) {
             *sQuoteState += 10 ;
             ++ptrChar ; 
             continue ;
          }

          /*******************************************************************/
          /*  Look for single/double quoted text to see if it is             */
          /*  translatable.                                                  */
          /*******************************************************************/
          if ( ( *ptrChar == '\"' ) ||
               ( *ptrChar == '\'' ) ||
               ( *QuoteChar       ) ) {
             if ( *QuoteChar == 0 ) {
                ++(*sQuoteNum) ; 
                if ( *(ptrChar+1) == *ptrChar ) {   /* Null text             */
                   ++ptrChar ;
                   continue ;
                }
                *QuoteChar = *ptrChar ;
                if ( ( *sQuoteNum < 50 ) &&
                     ( sQuoteCtl[*sQuoteNum] > 0 ) ) {
                   memmove( ptrChar+9, ptrChar+1, (wcslen(ptrChar))*sizeof(wchar_t) ) ;
                   wcsncpy( ptrChar+1, L"{TWBSCR}", 8 ) ; 
                   ptrChar += 8 ;
                }
                ++ptrChar ;
             }
                                                   /* Find ending quote */
             for( bSkipChar=FALSE; 
                  ( *ptrChar ) && 
                  ( (*ptrChar != *QuoteChar ) || 
                    ( (*(ptrChar-1)=='\\')&&(!bSkipChar) )) ; 
                  ++ptrChar ) {
                if ( ( *ptrChar == '\\' ) &&
                     ( *(ptrChar+1) == '\\' ) ) { 
                   bSkipChar = TRUE ; 
                   ++ptrChar ; 
                } else {
                   bSkipChar = FALSE ; 
                   if ( IsDBCS(*ptrChar) ) { 
                      ++ptrChar ;
                      bSkipChar = TRUE ; 
                   }
                }
             }
             if ( *ptrChar == *QuoteChar ) {
                *QuoteChar = 0 ;
                if ( ( *sQuoteNum < 50 ) &&
                     ( sQuoteCtl[*sQuoteNum] > 0 ) ) {
                   memmove( ptrChar+10, ptrChar, (wcslen(ptrChar)+1)*sizeof(wchar_t) ) ;
                   wcsncpy( ptrChar, L"<XSCRCONT:", 10 ) ; 
                   ptrChar += 10 ;
                }
             }
          }
       }
    }

    ptr = wcsstr( Text, L"<TWB2K" ) ;           
    if ( ( ptr ) &&
         ( *QuoteChar ) ) {
       memmove( ptr+18, ptr, (wcslen(ptr)+1)*sizeof(wchar_t) ) ;
       wcsncpy( ptr, L"<XSCRCONT:{TWBSCR}", 18 ) ; 
    }
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/***********************************************************************/
/*  Handle bidi processing for <HTML> tag.  Add a DIR="RTL" attribute. */
/***********************************************************************/
void chk_bidihtml(wchar_t *Tag )
{

    wchar_t   *TokenDelimit = L" =\n\r\t";   
    wchar_t   szTemp[MAXENTITY_ALLOC];
    wchar_t   *ptrChar, *ptrChar2, *ptrChar3 ;


    wcscpy( szTemp, Tag ) ;
    wcsupr( szTemp ) ;
    ptrChar = wcschr( szTemp, L'>' ) ;                 /* Find end of <HTML> tag        */
    if ( ptrChar ) 
       *ptrChar = NULL ;

    ptrChar = wcstok( szTemp, TokenDelimit ) ;
    while( ptrChar ) {
       if ( ! wcscmp( ptrChar, L"DIR" ) ) {            /* Find DIR attribute            */
          ptrChar2 = wcstok( NULL, TokenDelimit ) ;
          if ( ptrChar2 ) {
             ptrChar3 = wcsstr( ptrChar2, L"LTR" ) ;   /* Change attr. value LTR to RTL */
             if ( ptrChar3 ) {
                wcsncpy( ptrChar3, L"RTL", 3 ) ;
                wcsncpy( &Tag[(ptrChar3-szTemp)], L"RTL", 3 ) ;
             }
             ptrChar3 = wcsstr( ptrChar2, L"RTL" ) ;   /* Add quotes around RTL if needed */
             if ( ( ptrChar3 ) &&
                  ( *(ptrChar3-1) != L'\"' ) &&
                  ( *(ptrChar3-1) != L'\'' ) ) {
                wmemmove( &Tag[ptrChar3-szTemp+1], &Tag[ptrChar3-szTemp], wcslen(&Tag[ptrChar3-szTemp])+1 ) ;
                Tag[ptrChar3-szTemp] = L'\"' ;
                ++ptrChar3 ;
                wmemmove( &Tag[ptrChar3-szTemp+4], &Tag[ptrChar3-szTemp+3], wcslen(&Tag[ptrChar3-szTemp+3])+1 ) ;
                Tag[ptrChar3-szTemp+3] = L'\"' ;
             }
          }
          wmemmove( &Tag[ptrChar-szTemp+4], &Tag[ptrChar-szTemp], wcslen(&Tag[ptrChar-szTemp])+1 ) ;
          wcsncpy( &Tag[(ptrChar-szTemp)], L"mod+", 4 ) ;  /* Make DIR value translatable */
          break;
       }
       ptrChar = wcstok( NULL, TokenDelimit ) ;
    }
    if ( ptrChar == NULL ) {                           /* No DIR attribute, add it */
       wmemmove( &Tag[19], &Tag[5], wcslen(&Tag[5])+1 ) ;
       wcsncpy( &Tag[5], L" mod+dir=\"RTL\"", 14 ) ;
    }
}

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/***********************************************************************/
/*  Handle making certain Bidi attribute values translatable.          */
/*     DIR attribute value should always be translatable.              */
/*     ALIGN attribute value is translatable when "LEFT" or "RIGHT".   */
/***********************************************************************/
void chk_bidiattr(wchar_t *Text, FILE *InFile )
{

    wchar_t   *ATTR_ALIGN = L"ALIGN" ; 
    wchar_t   *ATTR_DIR = L"DIR" ; 
    wchar_t   szTemp[MAXENTITY_ALLOC];
    wchar_t   szTemp2[MAXENTITY_ALLOC];
    wchar_t   *ptrTextEnd ;
    wchar_t   *ptrAttrEnd ;
    wchar_t   *ptrChar, *ptrChar2, *ptrChar3 ;
    ULONG     ulFilePos ;
    ULONG     i ;  
    BOOL      bAlignAttr ;
    BOOL      bGetNextLine = TRUE ;


    wcscpy( szTemp, Text ) ;
    wcsupr( szTemp ) ;
    ptrChar = wcsstr( szTemp, ATTR_ALIGN ) ;
    if ( ! ptrChar ) 
       ptrChar = wcsstr( szTemp, ATTR_DIR ) ;
    if ( ptrChar ) {
       ptrTextEnd = szTemp + wcslen(szTemp);
       for( ptrChar=szTemp ; ptrChar<ptrTextEnd ; ptrChar++ ) {
          if ( ( ( ! wcsncmp( ptrChar, ATTR_ALIGN, 5 ) ) ||
                 ( ! wcsncmp( ptrChar, ATTR_DIR,   3 ) ) ) &&
               ( ( ptrChar == szTemp ) ||
                 ( iswspace( *(ptrChar-1) ) ) ) ) {
             if ( ! wcsncmp( ptrChar, ATTR_ALIGN, 5 ) ) {
                bAlignAttr = TRUE ;
                ptrChar2 = ptrChar + 5 ;
             } else {
                bAlignAttr = FALSE ;
                ptrChar2 = ptrChar + 3 ;
             }
             for( ; ptrChar2<ptrTextEnd && iswspace(*ptrChar2) ; ptrChar2++ ) ;
             if ( ( ptrChar2>=ptrTextEnd ) ||
                  ( *ptrChar2 == L'=' ) ){
                if ( bGetNextLine ) {
                   bGetNextLine = FALSE ;
                   ulFilePos = ftell( InFile ) ;
                   szTemp2[0] = 0 ;
                   if ( fgetws( szTemp2, MAXENTITY_READ,InFile ) != 0 ) {
                      fseek( InFile, ulFilePos, SEEK_SET ) ;

                      wcsupr( szTemp2 ) ;
                      if ( (wcslen(szTemp)+wcslen(szTemp2)) < MAXENTITY_ALLOC-1 ) {
                         wcscat( szTemp, szTemp2 ) ;
                      } else {
                         wcsncpy( ptrTextEnd, szTemp2, MAXENTITY_ALLOC-wcslen(szTemp)-1 ) ; 
                         szTemp[MAXENTITY_ALLOC-1] = 0 ;
                      }
                      for( ; *ptrChar2 && iswspace(*ptrChar2) ; ptrChar2++ ) ;
                   }
                }
                if ( *ptrChar2 == L'=' ) {
                   for( ++ptrChar2 ; *ptrChar2 && iswspace(*ptrChar2) ; ptrChar2++ ) ;
                   if ( ptrChar2 < ptrTextEnd ) {
                      if ( ( *ptrChar2 == L'\"' ) ||
                           ( *ptrChar2 == L'\'' ) ) 
                         ++ptrChar2 ;
                      if ( ( ( bAlignAttr ) &&
                             ( ( ! wcsncmp( ptrChar2, L"LEFT",  4  ) ) ||
                               ( ! wcsncmp( ptrChar2, L"RIGHT", 5 ) ) ) ) ||
                           ( ( ! bAlignAttr ) &&
                             ( ( ! wcsncmp( ptrChar2, L"RTL", 3 ) ) ||
                               ( ! wcsncmp( ptrChar2, L"LTR", 3 ) ) ) ) ) {
                         if ( bAlignAttr ) {
                            if ( ! wcsncmp( ptrChar2, L"LEFT",  4  ) ) {
                               wmemmove( &Text[ptrChar2-szTemp+5], &Text[ptrChar2-szTemp+4], wcslen(&Text[ptrChar2-szTemp+4])+1 ) ;
                               wcsncpy( &Text[(ptrChar2-szTemp)], L"RIGHT", 5 ) ;  /* Change LEFT to RIGHT */
                               wmemmove( ptrChar2+5, ptrChar2+4, wcslen(ptrChar2+4)+1 ) ;
                               wcsncpy( ptrChar, L"RIGHT", 5 ) ;  
                               ptrTextEnd += 1 ;
                               ptrAttrEnd = ptrChar2 + 5 ;
                            } else {
                               wmemmove( &Text[ptrChar2-szTemp+4], &Text[ptrChar2-szTemp+5], wcslen(&Text[ptrChar2-szTemp+5])+1 ) ;
                               wcsncpy( &Text[(ptrChar2-szTemp)], L"LEFT", 4 ) ;  /* Change RIGHT to LEFT */
                               wmemmove( ptrChar2+4, ptrChar2+5, wcslen(ptrChar2+5)+1 ) ;
                               wcsncpy( ptrChar, L"LEFT", 4 ) ;  
                               ptrTextEnd -= 1 ;
                               ptrAttrEnd = ptrChar2 + 4 ;
                            }

                         } else {
                            ptrAttrEnd = ptrChar2 + 3 ;
                         }

                         wmemmove( &Text[ptrChar-szTemp+4], &Text[ptrChar-szTemp], wcslen(&Text[ptrChar-szTemp])+1 ) ;
                         wcsncpy( &Text[(ptrChar-szTemp)], L"mod+", 4 ) ;  /* Make attr. value translatable */
                         wmemmove( ptrChar+4, ptrChar, wcslen(ptrChar)+1 ) ;
                         wcsncpy( ptrChar, L"mod+", 4 ) ; 
                         ptrChar += 8 ;
                         ptrTextEnd += 4 ;
                         if ( ( *(ptrChar2+3) != '\"' ) &&     /* Attribute value must be in quotes */
                              ( *(ptrChar2+3) != '\'' ) ) {
                            ptrChar2 += 4 ;                    /* Adjust for added "mod+" */
                            ptrAttrEnd += 4 ;                  /* Adjust for added "mod+" */
                            wmemmove( &Text[ptrAttrEnd-szTemp+1], &Text[ptrAttrEnd-szTemp], wcslen(&Text[ptrAttrEnd-szTemp])+1 ) ;
                            Text[ptrAttrEnd-szTemp] = '\"' ;
                            wmemmove( &Text[ptrChar2-szTemp+1], &Text[ptrChar2-szTemp], wcslen(&Text[ptrChar2-szTemp])+1 ) ;
                            Text[ptrChar2-szTemp] = '\"' ;
                            wmemmove( ptrChar+2, ptrChar, wcslen(ptrChar)+1 ) ;
                            wcsncpy( ptrChar, L"xx", 2 ) ; 
                            ptrChar +=2 ; 
                         }
                      }
                   }
                }
             }
          }
       }
    }
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

BOOL PostEntityTag(char *sourceName, char *tempName, BOOL *Resequence)
{

    FILE *sourceFile_ptr = NULL;
    FILE *tempFile_ptr = NULL;
    wchar_t char_str[MAXENTITY_ALLOC*2 /*+1024*/];                      
    wchar_t new_str[MAXENTITY_ALLOC*2 /*+1024*/];                       
    wchar_t szPrevSegTag[10] = {0} ;
    wchar_t * sub_str;
    wchar_t *ptrChar, *ptrChar2, *ptrChar3 ;
    short sSegLength = 0 ;
    int i, j, rc;
    BOOL  bEntityFound ; 
    BOOL  bEntity1Prev = FALSE ;

    wchar_t ENTITY_COMMENT[17] = L"<ENTITY_COMMENT>" ;
    wchar_t TWB2K_TAG[8] = L"<TWB2K>" ;
    wchar_t TWB2K_BRACE[8] = L"{TWB2K}" ;


    sourceFile_ptr = fopen(sourceName, "rb");
    tempFile_ptr = fopen(tempName, "wb");
    *Resequence = FALSE ; 

          // Get a line up to a newline character or 1024 bytes as input to parse
    while ((fgetws(char_str,MAXENTITY_READ*2/*+1024*/,sourceFile_ptr)) != NULL) { 

       if ( wcslen(char_str) == MAXENTITY_READ*2-1/*+1023*/ ) {                   
          ptrChar = wcsrchr( char_str, '<' ) ;
          if ( ptrChar ) {
             for( i=0, j=wcslen(ENTITY_COMMENT) ; 
                  *ptrChar && i<j && *ptrChar==ENTITY_COMMENT[i] ; 
                  ++ptrChar, ++i ) ;
             if ( ( ! *ptrChar ) &&
                  ( i < j ) ) 
                fgetws( &char_str[wcslen(char_str)], j-i+1, sourceFile_ptr ) ;
          }
          ptrChar = wcsrchr( char_str, '<' ) ;
          if ( ptrChar ) {
             for( i=0, j=wcslen(TWB2K_TAG), ptrChar2=ptrChar ; 
                  *ptrChar2 && i<j && *ptrChar2==TWB2K_TAG[i] ; 
                  ++ptrChar2, ++i ) ;
             if ( ( ! *ptrChar2 ) &&
                  ( i < j ) ) 
                fgetws( &char_str[wcslen(char_str)], j-i+1, sourceFile_ptr ) ;
             if ( ( ! wcsncmp( ptrChar, TWB2K_TAG, wcslen(TWB2K_TAG) ) ) &&
                  ( ! wcschr( ptrChar, '{' ) ) ) 
                fgetws( &char_str[wcslen(char_str)], 25, sourceFile_ptr ) ;

          }
          ptrChar = wcsrchr( char_str, '{' ) ;
          if ( ptrChar ) {
             for( i=0, j=wcslen(TWB2K_BRACE), ptrChar2=ptrChar ; 
                  *ptrChar2 && i<j && *ptrChar2==TWB2K_BRACE[i] ; 
                  ++ptrChar2, ++i ) ;
             if ( ( ! *ptrChar2 ) &&
                  ( i < j ) ) 
                fgetws( &char_str[wcslen(char_str)], j-i+1, sourceFile_ptr ) ;
          }
       }


        memset (new_str,0,sizeof(new_str));
        bEntityFound = FALSE ;                                
        while ((sub_str = wcsstr(char_str,L"<ENTITY_COMMENT>")) != NULL) {
            bEntityFound = TRUE ;
            memset (new_str,0,sizeof(new_str));
            wcsncat(new_str,char_str,((wcslen(char_str) - wcslen(sub_str))));
            wcscat(new_str,sub_str + wcslen(L"<ENTITY_COMMENT>"));
            wcscpy(char_str,new_str);
        }
        while ( ( ( bEntityFound ) ||         
                  ( bEntity1Prev ) ) && 
                ( (sub_str = wcsstr(char_str,END_ENTITY2)) != NULL ) ) { 
            memset (new_str,0,sizeof(new_str));
            wcsncat(new_str,char_str,((wcslen(char_str) - wcslen(sub_str))));
            wcscat(new_str,sub_str + wcslen(END_ENTITY2));
            wcscpy(char_str,new_str);
        }
        if ( bEntityFound )                    
           bEntity1Prev = TRUE ;
        else 
           bEntity1Prev = FALSE ; 
        while( ( sub_str = wcsstr( char_str, L"{TWB2K}" ) ) != NULL ) { 
           *Resequence = TRUE ;                                        
           ptrChar = wcsstr( char_str, L"<TWB2K" ) ;
           if ( ! ptrChar  ) {
              ptrChar = wcschr( sub_str, ':' ) ;
              if ( ptrChar ) {   /* Remove {TWB2K} up to next tag start */
                 memmove( sub_str, ptrChar, (wcslen(ptrChar) + 1)*sizeof(wchar_t) ) ;
              } else {
                 *sub_str = 0 ;  /* Remove {TWB2K} and rest of line */
              }
           } else {
              if ( ptrChar < sub_str ) {
                 if ( *(ptrChar+6) != '>' ) { /* Remove tag continuation {TWB2K} */
                    memmove( ptrChar, sub_str+7, (wcslen(sub_str+7)+1)*sizeof(wchar_t) ) ;
                 } else {       /* Remove continued segment {TWB2K} */
                    for( ptrChar2=ptrChar ; ptrChar2>char_str && *ptrChar2!=':' ; --ptrChar2 ) ;
                    for( ptrChar3=sub_str ; ptrChar3>ptrChar && *ptrChar3!=':' ; --ptrChar3 ) ;
                    if ( ( *ptrChar2 == ':' ) && 
                         ( *ptrChar3 == ':' ) ) {
                       memmove( sub_str, sub_str+7, (wcslen(sub_str+7)+1)*sizeof(wchar_t) ) ;
                       memmove( ptrChar2, ptrChar3, (wcslen(ptrChar3)+1)*sizeof(wchar_t) ) ;
                       for( ptrChar=ptrChar2-7 ;
                            ptrChar>=char_str && wcsncmp(ptrChar,L":QF",3 ) &&
                                                 wcsncmp(ptrChar,L":qf",3 ) ;  
                            --ptrChar ) ;
                       if ( ( ptrChar>=char_str             ) &&
                            ( ptrChar2-ptrChar < 500        ) &&
                            ( *(ptrChar2+3) == *(ptrChar+3) ) ) {
                            ptrChar = wcschr( ptrChar2, '.' ) ;
                            memmove( ptrChar2-6, ptrChar+1, wcslen(ptrChar)*sizeof(wchar_t) );
                       }
                    } else {                                     
                       ptrChar2 = wcschr( sub_str, ':' ) ;
                       if ( ptrChar2 ) {   /* Remove {TWB2K} up to next tag start */
                          memmove( sub_str, ptrChar2, (wcslen(ptrChar2) + 1)*sizeof(wchar_t) ) ;
                    } else {                                    
                          *sub_str = 0 ;  /* Remove {TWB2K} and rest of line */
                       }
                       if ( ptrChar+7 == sub_str ) {               
                          memmove( ptrChar, sub_str, (wcslen(sub_str) + 1)*sizeof(wchar_t) ) ;
                       }
                    }
                 }
              }
           }
        }
        // Remove special 2K <STYLE> tags.
        if ( (sub_str = wcsstr( char_str, L"<STYLE ID=TWB>" )) != NULL ) {
           memmove( sub_str, sub_str+14, (wcslen(sub_str+14)+1)*sizeof(wchar_t) ) ; 
           if ( (sub_str = wcsstr( char_str, L"</STYLE>" )) != NULL ) 
              memmove( sub_str, sub_str+8, (wcslen(sub_str+8)+1)*sizeof(wchar_t) ) ; 
        }

        // Remove special non-translatable block tags.
        while( (sub_str=wcsstr(char_str,L"<!--{TWB}") ) != NULL ) { 
           memmove( sub_str, sub_str+9, (wcslen(sub_str+9)+1)*sizeof(wchar_t) ) ;
        }
        while( (sub_str=wcsstr(char_str,L"{TWB}-->") ) != NULL ) {  
           memmove( sub_str, sub_str+8, (wcslen(sub_str+8)+1)*sizeof(wchar_t) ) ;
        }


        chk_Seg2k( char_str, szPrevSegTag, &sSegLength, Resequence ) ; 

        fputws(char_str,tempFile_ptr);

    }
    fclose(sourceFile_ptr);
    fclose(tempFile_ptr);

    rc = DosCopy(tempName, sourceName, DCPY_EXISTING); // COPIES MODIFIED TEMP BACK TO SOURCE
    remove(tempName);
    return(TRUE);
}


//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

/***********************************************************************/
/*  Determine if there is any segment >2K which must be split.         */
/***********************************************************************/
void chk_Seg2k( wchar_t *Text, wchar_t *PrevTag, short *SegLength, BOOL *Resequence )
{

    wchar_t   szTemp[128] ; 
    wchar_t   szTagName[20] ;
    wchar_t   *ptrChar, *ptrChar2 ;
    short  sCurSegLen ; 
    short  i ;

    sCurSegLen = wcslen( Text ) ; 
    for( ptrChar=wcschr(Text,':') ; ptrChar ; ptrChar=wcschr(ptrChar+1,':') ) {
       wcsncpy( szTagName, ptrChar+1, sizeof(szTagName)/sizeof(wchar_t) ) ;
       szTagName[4] = 0 ; 
       wcsupr( szTagName ) ;
       if ( ( ( ! wcsncmp( szTagName, L"QFF", 3 ) ) ||
              ( ! wcsncmp( szTagName, L"QFX", 3 ) ) ||
              ( ! wcsncmp( szTagName, L"QFN", 3 ) ) ||
              ( ! wcsncmp( szTagName, L"QFA", 3 ) ) ) &&
            ( iswspace(szTagName[3]) ) ) {
          wcsncpy( PrevTag, ptrChar+1, 3 ) ; 
          PrevTag[3] = 0 ; 
          ptrChar2 = wcschr( ptrChar, '.' ) ;
          *SegLength = 0 ; 
          if ( ptrChar2 ) 
             sCurSegLen = wcslen( ptrChar2+1 ) ; 
          else
             sCurSegLen = 0 ; 
       }
    }

    if ( *SegLength + sCurSegLen > 1600 ) {    
       *Resequence = TRUE ; 
       if ( iswupper( PrevTag[0] ) ) 
          wcscpy( szTagName, L"EN" ) ; 
       else
          wcscpy( szTagName, L"en" ) ; 
       ptrChar = wcschr( Text, '<' ) ;
       if ( ( ! ptrChar ) ||
            ( ptrChar - Text > 200 ) ) {
          ptrChar = Text ;
       }
       *SegLength = wcslen(ptrChar) ;
       memmove( ptrChar+15, ptrChar, (wcslen(ptrChar)+1)*sizeof(wchar_t) ) ; 
       swprintf( szTemp, L":%c%s.:%s %c=0.", szTagName[0], PrevTag, PrevTag, szTagName[1] ) ; 
       wcsncpy( ptrChar, szTemp, 15 ) ; 
    } else {
       *SegLength += sCurSegLen ; 
    }
}

