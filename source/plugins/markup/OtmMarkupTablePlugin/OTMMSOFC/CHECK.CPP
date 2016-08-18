/*
*
*  Copyright (C) 1998-2016, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/****************************************************************************/
/*                                                                          */
/* CHECK.CPP                                                                */
/*                                                                          */
/*FUNCTIONS:                                                                */
/* CheckXmlText                                                             */
/*                                                                          */
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 2/26/2016: IBM : Original Source                                         */
/*==========================================================================*/

#include "unicode.h"
#include "otmmsofc.h"

   BOOL         bCheckCommonProperty ;


#define  TITLE_MSOFC_TAG_STRUCTURE              L"Invalid Tag Structure"
#define  TITLE_MSOFC_AMPERSAND                  L"Incorrect Ampersand (&)"
#define  TITLE_MSOFC_INVALID_TEXT_LENGTH        L"Invalid Text Length"
#define  TITLE_MSOFC_INLINE_TAGGING             L"Incorrect Inline Tagging"
                                                
#define  MSG_MSOFC_RETURN_TO_SEG                L"\n\nOK - Return to segment to correct the text."
#define  MSG_MSOFC_AMPERSAND                    L"The ampersand character (&) must be defined as \"&amp;\".\n\nOK - Return to segment to correct the text." 
#define  MSG_MSOFC_TAG_MUST_NEST                L"Tag %s must be nested within:\n"
#define  MSG_MSOFC_TAG_MUST_NOT_NEST            L"Tag %s cannot be nested inside of another tag."
#define  MSG_MSOFC_TAG_MISMATCH                 L"Tags in source and target segments do not match.\n"
#define  MSG_MSOFC_TAG_MISSING                  L"Missing tags from target segment:\n"
#define  MSG_MSOFC_TAG_EXTRA                    L"\nExtra tags in target segment:\n"
#define  MSG_MSOFC_TAG_ORDER                    L"\nEnd tag %s cannot occur before begin tag %s.\n" 
#define  MSG_MSOFC_MAX_TEXT_LENGTH              L"Text length is longer than maximum:\n\n   Current length:   %d\n   Maximum length:  %d\n\nOK - Return to segment to shorten the text."
#define  MSG_MSOFC_INLINE_TAGGING               L"</~+:__> must be defined at the end of the segment after all translatable text.\n\nOK - Return to segment to correct the text."
#define  MSG_MSOFC_PICT_TRAILBLANKS             L"Remove the trailing blanks after the last <~+:n_PICT> tag in this segment."
#define  MSG_MSOFC_JOINED                       L"Paragraph sections cannot be joined.  This will corrupt the file format.\n\nPosition cursor before </w:p> and select menu item:  Translate -> Split Joined Segments." 
#define  MSG_MSOFC_FIRST_TAG                    L"<~+:1> must be defined at the beginning of the segment before any text."
#define  MSG_MSOFC_TAG_SEQUENCE                 L"Invalid tag sequence number: %s.\nFormat must be: <%snn>"
#define  MSG_MSOFC_TAG_PSEUDO                   L"Invalid pseudo tag name after '_': %s."
#define  MSG_MSOFC_TAG_END                      L"Tag is not properly ended: %s.\nFormat must be: <%snn>"
#define  MSG_MSOFC_TAG_INVALID                  L"Invalid tag found: %s.\nFormat must be: <%snn>"
#define  MSG_MSOFC_TAG_ORDER2                   L"Tags are in the wrong order.\n\nExpected end tag for:  <%s%d>.\nInstead found end tag for:  <%s%d>."
#define  MSG_MSOFC_INVALID_CHAR                 L"Invalid character found in segment at offset +%d.\n\n   Char= [%c]\n   Hex=  [%04lx]\n   Text= [%-40.40s]\n\nReplace this character with a supported character."

/*****************************************************************************/
/*                                                                           */
/*  CheckXmlText                                                             */
/*                                                                           */
/*  Check the current XML text after translation.                            */
/*                                                                           */
/*  Return:  TRUE  - Text successfully checked and no validation problem.    */
/*           FALSE - Text failed the check, so text should not be saved.     */
/*                                                                           */
/*****************************************************************************/

BOOL CheckXmlText( WCHAR * PrevSourceSegment, WCHAR * CurSourceSegment,
                   WCHAR * CurTargetSegment, EQF_BOOL * bSegmentChanged,
                   LONG lInfo, ULONG ulSegNum, EQF_BOOL bMessageDisplay )
{


   CHK_INFO     SrcTags[200] ;
   CHK_INFO     TgtTags[200] ;
   WCHAR        swSeg[2048] ;
   WCHAR        swErrMsg[1000];
   WCHAR        swErrMsg2[1000];
   WCHAR        swErrTitle[128];
   WCHAR        swNum[20];
   WCHAR        *ptrChar, *ptrChar2 ;
   short        sSrcNestSeq[200] ;
   short        sTgtNestSeq[200] ;
   short        iSrc, iTgt ;
   short        sNestLevel ;
   short        sMaxNum ;
   short        i, j, k, m ;
   USHORT       usRC, usNum ;
   ULONG        ulTempSegNum ;
   BOOL         bReturn ;
   BOOL         bFlag, bFlag2 ;


    *bSegmentChanged = FALSE ;
    bCheckCommonProperty = FALSE ; 


    /*---------------------------------------------------------------------*/
    /* Find previous <w:p> tag to determine if there is text defined       */
    /* using common properties, in which case the surrounding <~+:>        */
    /* tags are not required.                                              */
    /*---------------------------------------------------------------------*/
    usRC = EQFRC_OK ;
    ulTempSegNum = ulSegNum ;
    ptrChar = NULL ;
    while( ( usRC == EQFRC_OK ) &&
           ( ! ptrChar ) ) {
       usNum = sizeof(swSeg)/sizeof(WCHAR) ;
       usRC = EQFGETPREVSEGW( lInfo, &ulTempSegNum, swSeg, &usNum ) ;
       if ( usRC == EQFRC_OK ) 
          ptrChar = wcsstr( swSeg, L"<w:p " ) ;
       else
          ptrChar = NULL ;
    }
    if ( ( usRC == EQFRC_OK ) &&
         ( ptrChar ) ) {
       for(  ; ptrChar ; ptrChar2=ptrChar, ptrChar=wcsstr(ptrChar+1,L"<w:p ") ) ;
       ptrChar = wcschr( ptrChar2, L'>' ) ;
       if ( ptrChar ) {
          *ptrChar = NULL ;
         if ( wcsstr( ptrChar2, L" cp=\"" ) ) {
            bCheckCommonProperty = TRUE ; 
         }
       }
    }


   /*------------------------------------------------------------------------*/
   /* Verify neutral tags are properly defined and nested.                   */
   /*------------------------------------------------------------------------*/
   bReturn = CheckParseSegment( CurSourceSegment, &SrcTags[0], &iSrc, swErrMsg, FALSE ) ;
   if ( bReturn )  
      bReturn = CheckParseSegment( CurTargetSegment, &TgtTags[0], &iTgt, swErrMsg, TRUE ) ;
   if ( ( ! bReturn ) &&
        ( bMessageDisplay ) ) {
      wcscat( swErrMsg, MSG_MSOFC_RETURN_TO_SEG ) ;
      MessageBox(HWND_DESKTOP, swErrMsg, TITLE_MSOFC_TAG_STRUCTURE, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL ) ;
   }

   if ( ( bReturn ) &&
        ( ( iSrc > -1 ) ||
          ( iTgt > -1 ) ) ) {

     /*---------------------------------------------------------------------*/
     /* Match inline tags and identify nesting level.                       */
     /*---------------------------------------------------------------------*/
      swErrMsg[0] = NULL ;
      for( i=0 ; i<=iSrc && bReturn ; ++i ) {
         sNestLevel = SrcTags[i].NestLevel ;
         if ( sNestLevel > 0 )  
            sSrcNestSeq[sNestLevel] = SrcTags[i].SeqNum ; 
         for ( j=0 ; j<=iTgt && bReturn ; ++j ) {
            if ( sNestLevel > 0 )
               sTgtNestSeq[TgtTags[j].NestLevel] = TgtTags[j].SeqNum ; 
            if ( ( ! wcscmp( SrcTags[i].Tag, TgtTags[j].Tag )    ) ) {
               for( k=1 ; 
                    k<=sNestLevel && sSrcNestSeq[k]==sTgtNestSeq[k] ; 
                    ++k ) ;
               if ( k > sNestLevel ) {
                  SrcTags[i].Match = 1 ;
                  TgtTags[j].Match = 1 ;
               } else {
                  wcscpy( swErrMsg2, MSG_MSOFC_TAG_MUST_NEST ) ;
                  swprintf( swErrMsg, swErrMsg2, TgtTags[j].Tag ) ;
                  for( k=0, m=1 ; k<i ; ++k ) {
                     if ( sSrcNestSeq[m] == SrcTags[k].SeqNum ) {
                        ++m ; 
                        wcscat( swErrMsg, L"   " ) ;
                        wcscat( swErrMsg, SrcTags[k].Tag ) ;
                        wcscat( swErrMsg, L"\n" ) ;
                     }
                  }
                  if ( m == 1 ) {
                     wcscpy( swErrMsg, MSG_MSOFC_TAG_MUST_NOT_NEST ) ;
                     swprintf( swErrMsg, swErrMsg2, TgtTags[j].Tag ) ;
                  }
                  bReturn = FALSE ;
               }
               break;
            }
         }
      }
  
      /*---------------------------------------------------------------------*/
      /* Determine if there are any extra/missing tags.                      */
      /*---------------------------------------------------------------------*/
      if ( bReturn ) {
         bFlag2 = FALSE ;
         for( i=0 ; i<=iSrc ; ++i ) {
            if ( SrcTags[i].Match == 0 ) {
               if ( swErrMsg[0] == NULL ) {
                  if ( ! bFlag2 ) {
                     bFlag2 = TRUE ;
                     wcscpy( swErrMsg, MSG_MSOFC_TAG_MISMATCH ) ;
                  }
                  wcscat( swErrMsg, MSG_MSOFC_TAG_MISSING ) ;
               }
               wcscat( swErrMsg, L"   " ) ;
               wcscat( swErrMsg, SrcTags[i].Tag ) ;
               wcscat( swErrMsg, L"\n" ) ;
               bReturn = FALSE ;
            }
         }
         for( j=0, bFlag=FALSE ; j<=iTgt ; ++j ) {
            if ( TgtTags[j].Match == 0 ) {
               if ( ! bFlag ) {
                  bFlag = TRUE ;
                  if ( ! bFlag2 ) {
                     bFlag2 = TRUE ;
                     wcscpy( swErrMsg, MSG_MSOFC_TAG_MISMATCH ) ;
                  }
                  wcscat( swErrMsg, MSG_MSOFC_TAG_EXTRA ) ;
               }
               wcscat( swErrMsg, L"   " ) ;
               wcscat( swErrMsg, TgtTags[j].Tag ) ;
               wcscat( swErrMsg, L"\n" ) ;
               bReturn = FALSE ;
            }
         }
      }

      /*---------------------------------------------------------------------*/
      /* Determine if tags in proper order in target segment.                */
      /*---------------------------------------------------------------------*/
      if ( bReturn ) {
         for( i=0 ; i<=iTgt ; TgtTags[i++].Match=0 ) ;
         for( i=0 ; i<=iTgt && bReturn ; i++ ) {
            if ( TgtTags[i].Match == 0 ) {
               if ( TgtTags[i].TagType == TAG_TYPE_EMPTY )
                  continue ;
               if ( TgtTags[i].TagType == TAG_TYPE_BEGIN ) {
                  for ( j=i+1 ; j<=iTgt ; ++j ) {
                     if ( ( TgtTags[j].SeqNum == TgtTags[i].SeqNum ) &&
                          ( TgtTags[j].TagType == TAG_TYPE_END ) ) {
                        TgtTags[j].Match = 1 ;
                        break;
                     }
                  }
                  continue ;
               }
               if ( TgtTags[i].TagType == TAG_TYPE_END ) {
                  for ( j=i+1 ; j<=iTgt ; ++j ) {
                     if ( ( TgtTags[j].SeqNum == TgtTags[i].SeqNum ) &&
                          ( TgtTags[j].TagType == TAG_TYPE_BEGIN ) ) {
                        wcscpy( swErrMsg2, swErrMsg ) ;
                        wcscat( swErrMsg2, MSG_MSOFC_TAG_ORDER ) ;
                        swprintf( swErrMsg, swErrMsg2, TgtTags[i].Tag, TgtTags[j].Tag ) ;
                        bReturn = FALSE ;
                        break;
                     }
                  }
                  continue ;
               }
            }
         }
      }


      if ( ( ! bReturn ) &&
           ( bMessageDisplay ) ) {
         wcscat( swErrMsg, MSG_MSOFC_RETURN_TO_SEG ) ;
         MessageBox(HWND_DESKTOP, swErrMsg, TITLE_MSOFC_TAG_STRUCTURE, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL ) ;
      }


   }

   /*------------------------------------------------------------------------*/
   /* Verify maximum text length (if required).                              */
   /*------------------------------------------------------------------------*/
   if ( ( bReturn ) &&
        ( wcsstr( PrevSourceSegment, L" max=\"" ) ) ) {
      ptrChar = wcsstr( PrevSourceSegment, L" max=\"" ) ;
      ptrChar += 6 ;
      ptrChar2 = wcschr( ptrChar, L'\"' ) ;
      if ( ptrChar2 ) {
         wcsncpy( swNum, ptrChar, sizeof(swNum)/sizeof(WCHAR) ) ;
         swNum[ptrChar2-ptrChar] = NULL ;
         sMaxNum = _wtoi( swNum ) ;
         for( i=0, j=0 ; CurTargetSegment[i] ; ++i ) {
            ++j ;
            if ( CurTargetSegment[i] == L'&' ) {
               if ( ( ! wcsncmp( &CurTargetSegment[i], L"&lt;", 4 ) ) ||
                    ( ! wcsncmp( &CurTargetSegment[i], L"&gt;", 4 ) ) ) {
                  i += 3 ;
               } else
               if ( ! wcsncmp( &CurTargetSegment[i], L"&amp;", 5 ) ) {
                  i += 4 ;
               } else
               if ( ( ! wcsncmp( &CurTargetSegment[i], L"&apos;", 6 ) ) ||
                    ( ! wcsncmp( &CurTargetSegment[i], L"&quot;", 6 ) ) ) {
                  i += 5 ;
               } 
            }
         }
         if ( sMaxNum < j ) {
            bReturn = FALSE ;
            if ( bMessageDisplay ) {
               wcscpy( swErrMsg2, MSG_MSOFC_MAX_TEXT_LENGTH ) ;
               swprintf( swErrMsg, swErrMsg2, j, sMaxNum ) ;
               MessageBox(HWND_DESKTOP, swErrMsg, TITLE_MSOFC_INVALID_TEXT_LENGTH, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL ) ;
            }
         }
      }
   }

   /*------------------------------------------------------------------------*/
   /* Verify '&' is correctly defined.                                       */
   /*------------------------------------------------------------------------*/
   if ( ( bReturn ) &&
        ( wcschr( CurTargetSegment, L'&' ) ) ) {
      for( ptrChar=wcschr( CurTargetSegment, L'&' ) ;
           ptrChar ; 
           ptrChar=wcschr( ptrChar+1, L'&' ) ) {
         if ( ( wcsncmp( ptrChar, L"&lt;",   4 ) ) &&
              ( wcsncmp( ptrChar, L"&gt;",   4 ) ) &&
              ( wcsncmp( ptrChar, L"&amp;",  5 ) ) &&
              ( wcsncmp( ptrChar, L"&apos;", 6 ) ) &&
              ( wcsncmp( ptrChar, L"&quot;", 6 ) ) ) {
            bReturn = FALSE ;                                   
            if ( bMessageDisplay ) 
               MessageBox(HWND_DESKTOP, MSG_MSOFC_AMPERSAND, TITLE_MSOFC_AMPERSAND, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL ) ;
         }
      }
   }

   /*------------------------------------------------------------------------*/
   /* Verify no text after final inline tag.                                 */
   /*------------------------------------------------------------------------*/
   if ( bReturn ) {
      usNum = sizeof(swErrMsg)/sizeof(WCHAR) ;
      usRC = EQFGETNEXTSEGW( lInfo, &ulSegNum, swErrMsg, &usNum ) ;
      if ( ( usRC == EQFRC_OK ) &&
           ( ! wcsncmp( swErrMsg, L"</w:p>", 6 ) ) &&
           ( wcsstr( CurTargetSegment, L"~+:" ) ) ) {
         if ( ! bCheckCommonProperty ) {
            if ( ( CurTargetSegment[wcslen(CurTargetSegment)-1] != L'>' ) &&
                 ( ( CurTargetSegment[wcslen(CurTargetSegment)-1] != L'\x0007' ) || /* Ignore soft line feed at end.*/
                   ( CurTargetSegment[wcslen(CurTargetSegment)-2] != L'>'      ) ) ) {
               bReturn = FALSE ;                                     
               if ( bMessageDisplay ) 
                  MessageBox(HWND_DESKTOP, MSG_MSOFC_INLINE_TAGGING, TITLE_MSOFC_INLINE_TAGGING, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL ) ;
            }
         } else {
            /*------------------------------------------------------------------*/
            /* Check for text before leading or after trailing PICT tag when    */
            /* common property defined.                                         */
            /*------------------------------------------------------------------*/
            ptrChar = wcsstr( CurTargetSegment, L"_PICT>" ) ;
            if ( ptrChar ) {
               for( ptrChar2 = ptrChar-1 ; 
                    ( ptrChar2>CurTargetSegment ) && (*ptrChar2!=L'<') ; 
                    --ptrChar2 ) ;
               if ( *ptrChar2 == L'<' ) {
                  for( ptrChar=wcsstr( CurTargetSegment, L"_PICT>" ) ;
                       ptrChar ; ptrChar2=ptrChar, ptrChar=wcsstr( ptrChar+6, L"_PICT>" ) ) ;
                  for( ptrChar2+=6 ; *ptrChar2== L'<' ; ) {
                     if ( ( ! wcsncmp( ptrChar2+1, L"~+:",  3 ) ) ||
                          ( ! wcsncmp( ptrChar2+1, L"/~+:", 4 ) ) ) {
                        for( ; *ptrChar2 && *ptrChar2!=L'>' ; ++ptrChar2 ) ;
                        if ( *ptrChar2 != L'>' ) 
                           break ;
                        ++ptrChar2 ; 
                     }
                  }
                  if ( *ptrChar2 ) {
                     for( ; ( *ptrChar2 ) &&
                          ( ( iswspace(*ptrChar2) ) ||
                            ( *ptrChar2 == L'\x0007' ) ) ;
                          ++ptrChar2 ) ;
                     if ( *ptrChar2 == NULL ) {
                        bReturn = FALSE ;
                        MessageBox(HWND_DESKTOP, MSG_MSOFC_PICT_TRAILBLANKS, TITLE_MSOFC_TAG_STRUCTURE, MB_OK | MB_DEFBUTTON1 | MB_ICONWARNING | MB_SYSTEMMODAL ) ;
                     }
                  }
               }
            }
         }
      }
   }


   return bReturn ;                         /* Text will be saved as is      */
}


/*****************************************************************************/
/*                                                                           */
/*  CheckParseSegment                                                        */
/*                                                                           */
/*  Parse the neutral tags out of the segment text.                          */
/*                                                                           */
/*  Return:  TRUE  - Text successfully checked and no validation problem.    */
/*           FALSE - Text failed the check, so text should not be saved.     */
/*                                                                           */
/*****************************************************************************/

BOOL CheckParseSegment( WCHAR * Segment, CHK_INFO *ChkInfo, 
                        short *MaxTag, WCHAR * ErrMsg, BOOL bTarget ) 
{

   WCHAR        NEUTRAL_TAG[4] = L"~+:" ;
   WCHAR        FLDCHAR_BEGIN[3] = L"FD" ;
   WCHAR        FLDCHAR_END[4] = L"FDE" ;

   WCHAR        swTag[20] ;
   WCHAR        swPseudoName[20] ;
   WCHAR        swTemp[500] ;
   WCHAR        *ptrChar, *ptrChar2 ;
   short        i, j ;
   short        Nesting[50] ;
   short        sNestLevel = 0 ;
   short        sNestLevelAdjust ;
   BOOL         bReturn = TRUE ;


// ShowError( L"test", Segment, FALSE ) ;
   for( ptrChar=Segment,*MaxTag=-1 ; 
        bReturn && *ptrChar!=NULL ; 
        ++ptrChar ) {
      if ( *ptrChar == L'<' ) {
         if ( ( ! wcsncmp( ptrChar, L"</w:p>", 6 ) ) ||
              ( ! wcsncmp( ptrChar, L"<w:p ",  5 ) ) ) {
            wcscpy( ErrMsg, MSG_MSOFC_JOINED ) ;
            bReturn = FALSE ;
            break ;
         }
         if ( ( ( ! wcsncmp( ptrChar, L"<~+:1>",  6 ) ) ||          /* 10-12-12 */
                ( ! wcsncmp( ptrChar, L"<~+:1/>", 7 ) ) ) &&
              ( ptrChar != Segment ) &&
              ( ! bCheckCommonProperty ) ) {     /* Allow text before tag when common properties */
            if ( ( *Segment != L'\x0007'  ) ||   /* Ignore soft line feed in pos 1. 1-8-13 */
                 ( ptrChar != &Segment[1] ) ) {       
               wcscpy( ErrMsg, MSG_MSOFC_FIRST_TAG ) ;
               bReturn = FALSE ;
               break ;
            }
         }
         if ( ! wcsncmp( ptrChar, L"<Br>", 4 ) ) {          /* 8-19-15 */
            ptrChar += 3 ;
            continue ;
         }

         ChkInfo[++(*MaxTag)].TagType = TAG_TYPE_BEGIN ;
         ChkInfo[*MaxTag].SeqNum = -1 ;
         ChkInfo[*MaxTag].NestLevel = -1 ;
         ChkInfo[*MaxTag].Match = 0 ;
         ChkInfo[*MaxTag].Tag[0] = NULL ;
         wcsncpy( swTag, ptrChar, sizeof(swTag)/sizeof(WCHAR) ) ;
         swTag[(sizeof(swTag)/sizeof(WCHAR))-1] = NULL ;
         ptrChar2 = wcschr( swTag, L'>' ) ;
         if  ( ptrChar2 ) 
            *(ptrChar2+1) = NULL ;
         wcscpy( ChkInfo[*MaxTag].Tag, swTag ) ;

         sNestLevelAdjust = 0 ;
         if ( *(++ptrChar) == L'/' ) {
            ++ptrChar ;
            sNestLevelAdjust = -1 ;
            ChkInfo[*MaxTag].TagType = TAG_TYPE_END ;
         } else {
            ++sNestLevel ;
         }
         if ( ! wcsncmp( ptrChar, NEUTRAL_TAG, 3 ) ) {
            ptrChar += 3 ;
            for( i=0 ; *ptrChar!=NULL && iswdigit(*ptrChar) ; swTemp[i++]=*(ptrChar++) ) ;
            if ( i > 0 ) {
               swTemp[i] = NULL ; 
               ChkInfo[*MaxTag].SeqNum = wcstol( swTemp, NULL, 10 ) ;
            } else {
               wcscpy( swTemp, MSG_MSOFC_TAG_SEQUENCE ) ;
               swprintf( ErrMsg, swTemp, swTag, NEUTRAL_TAG ) ;
               bReturn = FALSE ;
            }

            swPseudoName[0] = NULL ;
            if ( ( bReturn ) &&
                 ( *ptrChar == L'_' ) ) {
               ++ptrChar ;
               for( i=0 ; 
                    *ptrChar!=NULL && iswalpha(*ptrChar) ; 
                    swPseudoName[i++]=*(ptrChar++) ) ; /* 4-22-09 */
               if ( i == 0 ) { 
                  wcscpy( swTemp, MSG_MSOFC_TAG_PSEUDO ) ;
                  swprintf( ErrMsg, swTemp, swTag ) ;
                  bReturn = FALSE ;
               } else {
                  swPseudoName[i] = NULL ;
               }
            }

            if ( bReturn ) {
               if ( *ptrChar == L'/' ) {
                  ++ptrChar ;
                  sNestLevelAdjust = -1 ; ;
                  ChkInfo[*MaxTag].TagType = TAG_TYPE_EMPTY ;
               } else 
               if ( ( !wcscmp( swPseudoName, FLDCHAR_BEGIN ) ) || /* 4-22-09 */
                    ( !wcscmp( swPseudoName, FLDCHAR_END   ) ) ) {
                  sNestLevelAdjust = -1 ; ;
                  ChkInfo[*MaxTag].TagType = TAG_TYPE_EMPTY ;
               }

               if ( *ptrChar == L'>' ) {
               } else {
                  wcscpy( swTemp, MSG_MSOFC_TAG_PSEUDO ) ;
                  swprintf( ErrMsg, swTemp, swTag, NEUTRAL_TAG ) ;
                  bReturn = FALSE ;
               }
            }

         } else {
            ChkInfo[*MaxTag].TagType = TAG_TYPE_NONE ;
            wcscpy( swTemp, MSG_MSOFC_TAG_INVALID ) ;
            swprintf( ErrMsg, swTemp, swTag, NEUTRAL_TAG ) ;
            bReturn = FALSE ;
         }
         ChkInfo[*MaxTag].NestLevel = sNestLevel ;
         sNestLevel += sNestLevelAdjust ;
      } else 
      if ( ( bTarget ) &&         /* Check for invalid XML control character 5-19-15 */
//         ( *ptrChar == L'\x001F' ) ) {
           ( *ptrChar <  L'\x0020' ) &&
           ( *ptrChar != L'\x0007' ) &&          /* Internal OTM soft line feed */
           ( ! wcschr( L"\n\r\t", *ptrChar ) ) ) {
         wcscpy( swTemp, MSG_MSOFC_INVALID_CHAR ) ;
         swprintf( ErrMsg, swTemp, ptrChar-Segment, *ptrChar, *ptrChar, ptrChar ) ;
         bReturn = FALSE ;
      }
   }

   if ( bReturn ) {
      sNestLevelAdjust = 1 ;
      for( i=0, j=-1 ; i<*MaxTag ; ++i ) {
         if ( ChkInfo[i].TagType == TAG_TYPE_BEGIN ) {
            Nesting[++j] = ChkInfo[i].SeqNum ;
         } else {
            if ( ( ChkInfo[i].TagType == TAG_TYPE_END ) &&
                 ( j >= 0 ) ) {
               if ( Nesting[j] != ChkInfo[i].SeqNum ) {
                  wcscpy( swTemp, MSG_MSOFC_TAG_ORDER2 ) ;
                  swprintf( ErrMsg, swTemp, NEUTRAL_TAG, Nesting[j],NEUTRAL_TAG, ChkInfo[i].SeqNum);
                  bReturn = FALSE ;
                  break ;
               } else {
                  --j ;
               }
            }
         }
      }
   }


   return( bReturn ) ;
}
