/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/****************************************************************************/
/*                                                                          */
/* PARSE.CPP                                                                */
/*                                                                          */
/*FUNCTIONS:                                                                */
/* PreParse                                                                 */
/* PostParse                                                                */
/*                                                                          */
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#include "state.h"
#include "parse.h"
#include "usrcalls.h"
#include "unicode.h"
#include "reseq.h"

using namespace mku ;                       

// lets define some useful shortcuts.
#define ATTRTAG       (XmlState::attrtag)
#define CDATA         (XmlState::cdata)
#define COMENT        (XmlState::comment)
#define CONDATTR      (XmlState::condattr)
#define DOCTAG        (XmlState::doctag)
#define DQUOTE        (XmlState::dquote)
#define DTDNAME       (XmlState::dtdname)
#define ENCODING      (XmlState::encoding)
#define ENTITY        (XmlState::entity)
#define HCOMMENT      (XmlState::hcomment)
#define HDQUOTE       (XmlState::hdquote)
#define HSQUOTE       (XmlState::hsquote)
#define NEUTTAG       (XmlState::neuttag)
#define NEUTATTRTAG   (XmlState::neutattrtag)
#define NONTRANS      (XmlState::nontrans)
#define NONTRANSTAG   (XmlState::nontranstag)
#define SQUOTE        (XmlState::squote)
#define SYSTEM        (XmlState::system)
#define TAG           (XmlState::tag)
#define TRANSATTR     (XmlState::transattr)
#define XMLDOC        (XmlState::xmldoc)
#define XMLLANG       (XmlState::xmllang)
#define XMLNAME       (XmlState::xmlname)
#define GLOBALTAG     (XmlState::globaltag)
#define NONTRANSBLOCK (XmlState::nontransblock)

#define JS_ASSIGN        (XmlState::JS_assign)
#define JS_ASSIGN1       (XmlState::JS_assign1)
#define JS_ASSIGN2       (XmlState::JS_assign2)
#define JS_BCOMMENT      (XmlState::JS_bcomment)
#define JS_DQUOTE        (XmlState::JS_dquote)
#define JS_HDQUOTE       (XmlState::JS_hdquote)
#define JS_HSQUOTE       (XmlState::JS_hsquote)
#define JS_LCOMMENT      (XmlState::JS_lcomment)
#define JS_PAREN         (XmlState::JS_paren)
#define JS_SQUOTE        (XmlState::JS_squote)
#define JS_GLOBALTAG     (XmlState::JS_globaltag)
#define JS_ARRAY         (XmlState::JS_array)
#define JS_NONTRANSBLOCK (XmlState::JS_nontransblock)

#define VBS_ASSIGN        (XmlState::VBS_assign)
#define VBS_ASSIGN1       (XmlState::VBS_assign1)
#define VBS_ASSIGN2       (XmlState::VBS_assign2)
#define VBS_DQUOTE        (XmlState::VBS_dquote)
#define VBS_HDQUOTE       (XmlState::VBS_hdquote)
#define VBS_COMMENT       (XmlState::VBS_comment)
#define VBS_PAREN         (XmlState::VBS_paren)
#define VBS_GLOBALTAG     (XmlState::VBS_globaltag)
#define VBS_NONTRANSBLOCK (XmlState::VBS_nontransblock)


#define XHTML_TAG_NONE       0
#define XHTML_TAG_TEXT       1
#define XHTML_TAG_INTAG      10
#define XHTML_TAG_NORMAL     11
#define XHTML_TAG_NEUTRAL    12
#define XHTML_TAG_SCRIPT     13
#define XHTML_TAG_STYLE      14
#define XHTML_TAG_INPUT      15

#define XHTML_FONT_NONE       0
#define XHTML_FONT_NEUTRAL    1



//intended. MUST NOT be 4096 since it will make buffer overflow
// while reading from file streams into a buffer of pre-defined size 4096.
#define BINARY_DATA_LENGTH  6144 

XmlState  XTbl ;
short     sSCRIPT_ParenCount ;
int       sCopyTextState ;
int       sIgnoreCommentState ;
int       sLinesState ;
int       sMaxLength ;            
int       sPrevScriptState ;
int       sPrevTransState ;
int       sScriptState ;
int       sXHTML_FontState  ;
int       sXHTML_TagState  ;
int       sXHTML_PrevTagState  ;
int       sLongSegLen ;          
BOOL      bCDATA_XHTMLTag ;      
BOOL      bEmptyEntityValue ;   

wifstream  *InputFile ;
extern BOOL bCodePageANSI ;
extern BOOL bCodePageHTML ;
extern BOOL bCodePageUTF8 ;
extern BOOL bCodePageUTF8_BOM ;    
extern BOOL bCodePageUTF16 ;
       BOOL bXLIFF  ;
       BOOL bSDLXLF ;
extern BOOL bXHTML  ;
       BOOL bFirstTag ;
       BOOL bInFirstTag ;
       BOOL bBidi  ;
       BOOL bUTF16 ;
       BOOL bNeutralTag ;         
       BOOL bXHTML_InCommentCheck ;
	   BOOL bXHTMLCondAttr;                      
       BOOL bTransAttrHasNoCondAttr;   
       BOOL bTruncatedRead ;                     
       BOOL bPreserveEOL ;                       
       BOOL bInsertEOL ;                         
       BOOL bLongSegTrans ;                      
       BOOL bEscapeDquote ;      
       BOOL bStartTrans ;         


inline void ParseTags(IString&, Token&) ;
inline BOOL HandleJavaScriptTokens( short *, Token& ) ;
inline BOOL HandleVBScriptTokens( short *, Token& ) ;
inline BOOL HandleXHTMLTags(wchar_t * );
inline BOOL ConvertEOL(PSZ, PSZ, wchar_t * ) ;
inline void SplitLongSegments(IString&) ;
inline BOOL FindAltSplit( wchar_t **, wchar_t *, wchar_t *, wchar_t *, short ) ;

#ifdef _UNICODE
wchar_t ENDL[4] = L"\r\n";
#else
char ENDL[4] = "\r\n";
#endif


extern   short   sTPVersion ;               /* From USRCALLS.C  */
extern   char    szDocTargetLanguage[80];   /* From USRCALLS.C  */
extern   char    szDocSourceLanguage[80];   /* From USRCALLS.C  */

/*
*   This introduces the following tags into the file
*   <twb--     start comment
*   --twb>    an end comment
*/


/*****************************************************************************/
/*  ShowError                                                                */
/*                                                                           */
/*  Show an error to the user using the platform's message box function.     */
/*                                                                           */
/*  Return:  TRUE  - User pressed OK.                                        */
/*           FALSE - User pressed cancel.                                    */
/*****************************************************************************/

BOOL ShowError( wchar_t *Title, wchar_t *Message, BOOL bOKCancel )
{
    short  rc ;
    BOOL   bReturn = TRUE;


    if ( bOKCancel ) {

        rc = MessageBox( HWND_DESKTOP, Message, Title,
                         MB_OKCANCEL | MB_DEFBUTTON2 | MB_ICONSTOP | MB_SYSTEMMODAL ) ;
        if ( rc != IDOK )
            bReturn = FALSE ;

    } else {

        rc = MessageBox( HWND_DESKTOP, Message, Title,
                         MB_OK | MB_DEFBUTTON1 | MB_ICONSTOP | MB_SYSTEMMODAL ) ;

    }

    return bReturn ;
}




/*****************************************************************************/
/*  PreParse                                                                 */
/*                                                                           */
/*  Function called by EQFPRESEG2.                                           */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be analyzed.                             */
/*****************************************************************************/

BOOL PreParse(PSZ in, PSZ out, PSZ markup, PSZ sourcefile, PSZ tempfile, HWND hSlider)
{
    BOOL bReturn = TRUE;
    Token    XTok ;
    XTbl.initializeStates();
    XTok.initializeToken();
    wofstream OutFile ( out ) ;

    InputFile = new wifstream( in ) ;

    IString s ;
    IString t ;
    wchar_t szErrTitle[128] ;
    wchar_t szEOLType[10] ;
    wchar_t *szErrText ;
    wchar_t *ptrChar, *ptrChar2 ;
    wchar_t szChar1[10] ;
    int FileSize      = 1;
    int ONE           = 1;
    int ZERO          = 0;
    int iTemp ;
    long save_pos;
    long lWrite ;
    long lIndex ;
    BOOL    b1stRead = TRUE ;
    BOOL    bTranslate ;

    struct stat statBuf;


    // int FileSize ;
    int TotalByteRead = 0 ;
    EQFSETSLIDER(hSlider, 0) ;


    szErrText = (wchar_t*)malloc( MAX_BUFFER_SIZE*sizeof(wchar_t)*3 ) ;

#ifdef _UNICODE
    mbstowcs(szErrText, markup, strlen(markup)+1);  
#else
    strcpy( szErrText, markup ) ;            /* Find file type from markup as */
#endif

    bEscapeDquote = FALSE ;
    bXLIFF = FALSE ;
    bSDLXLF= FALSE ;
    bXHTML = FALSE ;
    sXHTML_FontState   = XHTML_FONT_NONE ;
    sXHTML_TagState     = XHTML_TAG_NONE ;
    sXHTML_PrevTagState = XHTML_TAG_NONE ;
    bXHTML_InCommentCheck = FALSE ;
    bXHTMLCondAttr = FALSE ;        
    bTransAttrHasNoCondAttr = FALSE; 
    bCDATA_XHTMLTag = FALSE ;
    bEmptyEntityValue = FALSE ;
    if ( ( ! stricmp( szDocTargetLanguage, "ARABIC" ) ) ||
         ( ! stricmp( szDocTargetLanguage, "HEBREW" ) ) ||
         ( ! stricmp( szDocSourceLanguage, "ARABIC" ) ) ||
         ( ! stricmp( szDocSourceLanguage, "HEBREW" ) ) )
       bBidi = TRUE ;
    else
       bBidi = FALSE ;

    bPreserveEOL = FALSE ;
    bInsertEOL = FALSE ;
    wcscat( szErrText, L".DTD\"" ) ;         /*   default if no !DOCTYPE tag. */
    s=IString(szErrText) ;
    XTok.SetDTDInfo( s, TRUE ) ;

    bUTF16 = (*InputFile).IsUTF16();

    if ( bPreserveEOL ) {
       (*InputFile).freefile() ;
       (*InputFile).~wifstream() ;
       if ( ConvertEOL( in, tempfile, szEOLType ) ) {
          InputFile = new wifstream( tempfile ) ;
       } else {
          InputFile = new wifstream( in ) ;
       }
    }

    /* first find file size*/
    (*InputFile).fseekt(0, std::ios::end) ;
    FileSize = (*InputFile).ftellt() ;
    (*InputFile).fseekt(0, std::ios::beg) ;


    sCopyTextState = COPYTEXT_STATE_OFF ;
    sIgnoreCommentState = IGNORECOMMENT_STATE_OFF ;
    sLinesState = LINES_STATE_OFF ;
    sPrevScriptState = SCRIPT_STATE_NONE ;
    sPrevTransState = TRANS_STATE_NONE ; 
    sScriptState = SCRIPT_STATE_NONE ;
    bFirstTag = TRUE ;
    bInFirstTag = FALSE ;
    sSCRIPT_ParenCount=0 ;
    bNeutralTag = FALSE ;
    bLongSegTrans = TRUE ;
    sLongSegLen = 0 ;
    sMaxLength = 0 ;                

    OutFile.InsertBOM();


    if ( bPreserveEOL ) {
       OutFile << L"<TWB--EOL=" << szEOLType << L"--TWB>" << ENDL ;
    }

    
    /* look into the file before we do a read */
    while ( (*InputFile).getline(szErrText, MAX_BUFFER_SIZE, bUTF16) != NULL ) {

        if ( ( wcslen(szErrText) == MAX_BUFFER_SIZE-1 ) ||
             ( ( b1stRead ) &&         
               ( bUTF16   ) &&
               ( wcslen(szErrText) == MAX_BUFFER_SIZE-2 ) ) ) {
           bTruncatedRead = TRUE ;
           ptrChar  = wcsrchr( szErrText, L'<' ) ;
           ptrChar2 = wcsrchr( szErrText, L'>' ) ;
           if ( ( ptrChar ) &&
                ( ( ptrChar2 == NULL   ) ||
                  ( ptrChar2 < ptrChar ) ) ) {
              lIndex = wcslen(szErrText) ;
              while( (*InputFile).getline(szChar1,(int)2,bUTF16)!=NULL ) {
                 szErrText[lIndex++] = szChar1[0] ;
                 szErrText[lIndex] = NULL ;
                 if ( ( szChar1[0] == L'>' ) ||
                      ( lIndex > MAX_BUFFER_SIZE*2 ) ) 
                    break;
              }
           }
        } else {
           bTruncatedRead = FALSE ;
        }
        s = szErrText;
        b1stRead = FALSE ;


        XTok.SetString(s) ;
        TotalByteRead += s.length() + 1 ;
        EQFSETSLIDER(hSlider, (TotalByteRead*2*100)/FileSize) ;
        
        if ( sLinesState == LINES_STATE_ON ) {
           if ( ( XTok.GetTransState()     == TRANS_STATE_OFF ) ||    
                ( XTok.GetTransStateAttr() == TRANS_STATE_OFF ) ||
                ( XTbl.off(COMENT) ) ) {                              
           } else {
            XTok.AddTagTransOff() ;
            XTok.AddTagTransOn() ;
        }
        }

        if( wcsstr(szErrText,L" ") || (wcslen(szErrText) != MAX_BUFFER_SIZE-1 ) || wcsstr(szErrText,L"<")) { //if not binary data
            ParseTags(s, XTok) ;
        } else {
           s = XTok.GetString() ;        /* Force 2K split for non-trans block  */
        }


        for( lIndex=1, lWrite=s.length() ; lWrite>0 ; lWrite-=MAX_BUFFER_SIZE,lIndex+=MAX_BUFFER_SIZE ) { 
           OutFile << s.subString(lIndex,MAX_BUFFER_SIZE) ;
        }
        if ( ! bTruncatedRead ) {       //end of line   
           OutFile << ENDL ;
        }    
    }

    if (FileSize == ZERO) {
       wcscpy( szErrText, L"File does not contain any information, 0 bytes in size.  Analysis terminated." ) ;
       wcscpy( szErrTitle, L"Invalid Source" ) ;
       ShowError( szErrTitle, szErrText, FALSE ) ;
       bReturn = FALSE;
    }

    if ( bReturn ) {
       if ( bCodePageANSI ) {
          OutFile << L"<TWB--ANSI--TWB>" << ENDL ;
       } else
       if ( bCodePageHTML ) {
          OutFile << L"<TWB--HTML--TWB>" << ENDL ;
       } else
       if ( bCodePageUTF8 ) {
          if ( bCodePageUTF8_BOM )       
             OutFile << L"<TWB--UTF8BOM--TWB>" << ENDL ;
          else
          OutFile << L"<TWB--UTF8--TWB>" << ENDL ;
       } else
       if ( bCodePageUTF16 ) {
          OutFile << L"<TWB--UTF16--TWB>" << ENDL ;
       } else {
          OutFile << L"<TWB--ASCII--TWB>" << ENDL ;
       }
    }

    EQFSETSLIDER(hSlider, 100) ;
    XTok.FreeDTDInfo() ;

    (*InputFile).freefile() ;
    (*InputFile).~wifstream() ;

    if ( szErrText ) free(szErrText) ;

    return(bReturn);

} /* PreParse */




/*****************************************************************************/
/*  PostParse                                                                */
/*                                                                           */
/*  Function called by EQFPOSTSEG2.                                          */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be processed.                            */
/*****************************************************************************/

BOOL PostParse(PSZ in, PSZ out, HWND hSlider)
{
    BOOL bReturn = TRUE;
    wifstream InFile ( in ) ;
    wofstream OutFile ( out ) ;
    IString s ;
    int TotalByteRead = 0 ;
    wchar_t szErrTitle[128] ;
    wchar_t *szErrText ;
    int FileSize = 1;
    int ONE      = 1;
    int ZERO     = 0;
    int SegLen   = 0 ;
    long lFilePos ; 
    int NumChars ;
    BOOL bUTF16 = FALSE;
    BOOL b1stRead = TRUE;
    BOOL bPartialRecord;
    BOOL bSegTrans = FALSE ;
    BOOL bResequence = FALSE ;
    BOOL bInlineNonTrans = FALSE ;
    BOOL bStartInlineNonTrans ;
    BOOL bRecheckInlineNonTrans ;
    BOOL bTemp ;

    wchar_t TWBLT[7] = L"<twb--" ;
    wchar_t TWBGT[6] = L"-twb>" ;
    wchar_t szTagEQF[5] = L":EQF" ;
    wchar_t szTagQF[4] = L":QF" ;
    wchar_t szTagEQFF_QFF[16] = L":EQFF.:QFF N=0." ;
    wchar_t *ptr1 ;   
    wchar_t *ptrChar;
    wchar_t *ptrAmpersand ;
    wchar_t *ptrColon ;
    wchar_t *ptrLessthan ;
    wchar_t *ptrMinus ;
    wchar_t *ptrPeriod ;
    wchar_t *ptrEndAmpersand ;
    wchar_t *ptrEndLessthan ;
    wchar_t *ptrEndMinus ;
    wchar_t *ptrEndQFTag ;
    wchar_t *ptrEndText;
    wchar_t *ptrSegStart ;
    int i,j,k ;


    szErrText = (wchar_t*)malloc( MAX_BUFFER_SIZE*sizeof(wchar_t)*3 ) ;
    EQFSETSLIDER(hSlider, 0) ;

    /* first find file size*/
    InFile.fseekt(0, std::ios::end) ;
    FileSize = InFile.ftellt() ;
    InFile.fseekt(0, std::ios::beg) ;

    bUTF16 = InFile.IsUTF16();
    OutFile.InsertBOM();

    while ( InFile.getline(szErrText, MAX_BUFFER_SIZE, bUTF16) != NULL ) {
       if ( b1stRead ) {
          ptrColon = wcschr( szErrText, L':' ) ;
          if ( ( ptrColon ) &&
               ( iswlower( *(ptrColon+1) ) ) ) {
             wcslwr( szTagEQF ) ;
             wcslwr( szTagQF ) ;
             wcslwr( szTagEQFF_QFF ) ;
          }
       }

        bPartialRecord = FALSE ;
        if ( ( wcslen(szErrText) == MAX_BUFFER_SIZE-1 ) ||
             ( ( b1stRead ) &&                             
               ( bUTF16   ) &&
               ( wcslen(szErrText) == MAX_BUFFER_SIZE-2 ) ) ) {
            bPartialRecord = TRUE ;

            /* --------------------------------------------------------------*/
            /* Read extra characters if a partial record splits an important */
            /* tag so that the tag is complete.                              */
            /* --------------------------------------------------------------*/
            ptrEndText      = &szErrText[wcslen(szErrText)] ;
            ptrMinus        = wcsrchr( szErrText, L'-' ) ;  //Check truncated "--twb>"
            ptrLessthan     = wcsrchr( szErrText, L'<' ) ;  //Check truncated "<twb--"
            ptrAmpersand    = wcsrchr( szErrText, L'&' ) ;  //Check truncated "&TWBxxx;"
            ptrColon        = wcsrchr( szErrText, L':' ) ;  //Check truncated ":eqf_.:qf_ n=_."
            ptrPeriod       = wcsrchr( szErrText, L'.' ) ;  
            ptrEndMinus     = ptrMinus + 5 ;
            ptrEndLessthan  = ptrLessthan + 5 ;
            ptrEndAmpersand = ptrAmpersand + 7 ;
            if ( ( ptrEndMinus     >= ptrEndText ) ||
                 ( ptrEndLessthan  >= ptrEndText ) ||
                 ( ptrEndAmpersand >= ptrEndText ) ||
                 ( ( ptrColon ) &&
                   ( ( ! ptrPeriod ) ||
                     ( ptrPeriod < ptrColon ) ||
                     ( ! wcsncmp( ptrColon, szTagEQF, 4 ) ) ) ) ) {
               lFilePos = InFile.ftellt() ;
               InFile.getline( ptrEndText, 10, bUTF16 ) ;
               InFile.fseekt( lFilePos, 0 ) ;
               NumChars = 0 ;
               if ( ptrMinus ) {                            //Check truncated "--twb>"
                  if ( ( ! wcsncmp( ptrMinus, L"-twb>", 5 ) ) &&
                       ( *(ptrMinus-1) == L'-'              ) &&
                       ( ptrEndMinus-1 >= ptrEndText        ) ) {
                     i = ptrEndMinus-1 - ptrEndText + 1 ; 
                     if ( i > NumChars ) 
                        NumChars = i ;
                  } else {
                     if ( ( ! wcsncmp( ptrMinus, L"--twb>", 6 ) ) &&
                          ( ptrEndMinus >= ptrEndText           ) ) {
                        i = ptrEndMinus - ptrEndText + 1 ; 
                        if ( i > NumChars ) 
                           NumChars = i ;
                     }
                  }
               }
               if ( ( ptrLessthan ) &&                      //Check truncated "<twb--"
                    ( ! wcsncmp( ptrLessthan, L"<twb--", 6 ) ) &&
                    ( ptrEndLessthan >= ptrEndText           ) ) {
                  i =  ptrEndLessthan - ptrEndText + 1 ; 
                  if ( i > NumChars ) 
                     NumChars = i ;
               }                                            //Check truncated "&TWBxxx;"
               if ( ( ptrAmpersand ) &&
                    ( ( ! wcsncmp( ptrAmpersand, L"&TWBVAR;", 7 ) ) ||
                      ( ! wcsncmp( ptrAmpersand, L"&TWBPER;", 7 ) ) ) &&
                    ( ptrEndAmpersand >= ptrEndText                 ) ) {
                  i =  ptrEndAmpersand - ptrEndText + 1 ; 
                  if ( i > NumChars ) 
                     NumChars = i ;
               }
               if ( ptrColon ) {                            //Check truncated ":eqf_."
                  if ( ! wcsncmp( ptrColon, szTagEQF, 4 ) ) {
                     ptrChar = wcsrchr( ptrColon+1, L':' ) ; 
                     if ( ptrChar )  
                        ptrColon = ptrChar ;
                  }
                  if ( ! wcsncmp( ptrColon, szTagQF, 3 ) ) {   //Check truncated ":qf_ n=____."
                     ptrPeriod = wcschr( ptrColon, L'.' ) ; 
                     if ( ( ! ptrPeriod ) ||
                          ( ptrPeriod < ptrColon ) ||
                          ( ptrPeriod >= ptrEndText ) ) {
                        InFile.getline( ptrEndText, 20, bUTF16 ) ;
                        InFile.fseekt( lFilePos, 0 ) ;
                        ptrPeriod = wcschr( ptrColon, L'.' ) ; 
                        if ( ptrPeriod ) {
                           i =  ptrPeriod - ptrEndText + 1 ; 
                           if ( i > NumChars ) 
                              NumChars = i ;
                        }
                     }
                  }
               }
               if ( NumChars ) {
                  InFile.getline( ptrEndText, NumChars+1, bUTF16 ) ;
               } else {
                  *ptrEndText = NULL ;
               }
            }
        }


        /* --------------------------------------------------------------*/
        /* Determine if a 2K segment is split in the middle of an XML    */
        /* tag.                                                          */
        /* --------------------------------------------------------------*/
        ptrSegStart = szErrText ;
        bStartInlineNonTrans = bInlineNonTrans ;
        bRecheckInlineNonTrans = FALSE ;

        for( ptr1=szErrText ; *ptr1 ; ++ptr1 ) {

           if ( *ptr1 == L':' ) {
              ptrColon = ptr1 ;
              if ( ! wcsncmp( ptr1, szTagEQF, 4 ) ) {
                 if ( bInlineNonTrans ) 
                    bRecheckInlineNonTrans = TRUE ;  /* May need to add tags */
              SegLen += ptrColon - ptrSegStart ; 
              if ( ( SegLen > 1700 ) &&
                   ( bSegTrans ) &&
                   ( ! wcschr( L"nN", *(ptrColon+9) ) ) ) {    /* Ignore :eqff.:qfn */
                 if ( ptrColon != szErrText ) {
                       /* --------------------------------------------------------------*/
                       /* If break is within an XML tag, then move the break to just    */
                       /* before the tag.                                               */
                       /*    Before:  xxx<tag attr1="xx":eqff.qff n=nn. attr2="xx">     */
                       /*    After:   xxx:eqff.qff n=nn.<tag attr1="xx" attr2="xx">     */
                       /* --------------------------------------------------------------*/
                    ptrChar = ptrColon ;
                    if ( FindAltSplit( &ptrChar, ptrSegStart, szTagQF, szTagEQF, -1 ) ) {
                          if ( *ptrChar == L'<' ) {         /* Start of prev split XML tag */
                             ptrEndQFTag = wcschr( ptrColon, L'.' ) ;  /* End of :eqff tag */
                          if ( ptrEndQFTag )
                                ptrEndQFTag = wcschr( ptrEndQFTag+1, L'.' ) ; /* End of :qff tag */
                          if ( ptrEndQFTag ) {
                                NumChars = ptrEndQFTag - ptrColon + 1 ;  /* :eqff.:qff___. */
                                wcsncpy( szErrTitle, ptrColon, NumChars ) ;   /* Save tags */
                                wmemmove( ptrChar+NumChars, ptrChar, ptrColon-ptrChar ) ;
                                wcsncpy( ptrChar, szErrTitle, NumChars ) ;   /* Move tags before '<' */
                             ptrColon = ptrChar + 5 ;
                                ptr1 = ptrColon ;
                             SegLen = 0 ;
                             ptrChar += NumChars + 1 ;
                                /* --------------------------------------------------------------*/
                                /* If end of this XML tag is found, then insert a break after    */
                                /* the tag so that the next segment is not too long.             */
                                /*    Before:  xx:eqff.qff n=nn.<tag attr="xx">xx                */
                                /*    After:   xx:eqff.qff n=nn.<tag attr="xx">:eqff.:qff n=x.xx */
                                /* --------------------------------------------------------------*/
                             if ( FindAltSplit( &ptrChar, NULL, szTagQF, szTagEQF, 1 ) ) {
                                if ( ( *ptrChar == L'>' ) &&
                                     ( wcsncmp( ptrChar+1, szTagEQF, wcslen(szTagEQF) ) ) ) {
                                   ++ptrChar ;
                                   bResequence = TRUE ;
                                   NumChars = wcslen( szTagEQFF_QFF ) ;
                                   wmemmove( ptrChar+NumChars, ptrChar, wcslen(ptrChar)+1  ) ;
                                   wcsncpy( ptrChar, szTagEQFF_QFF, NumChars ) ; 
                                   ptrColon = ptrChar + 5 ;
                                      ptr1 = ptrColon ;
                                   SegLen = 0 ;
                                }
                             }
                             continue ;
                          }
                       } else 
                       if ( ptrChar > ptrSegStart ) 
                          continue ;
                    }
                 }
                 ptrEndQFTag = wcschr( ptrColon, L'.' ) ;
                 if ( ptrEndQFTag )
                    ptrEndQFTag = wcschr( ptrEndQFTag+1, L'.' ) ;
                 if ( ptrEndQFTag ) {
                       /* --------------------------------------------------------------*/
                       /* If end of this XML tag is found, then move the break after    */
                       /* the tag.                                                      */
                       /*    Before:  :eqff.qff n=nn.attr="xx">xx                       */
                       /*    After:   attr="xx">:eqff.qff n=nn.xx                       */
                       /* --------------------------------------------------------------*/
                    ptrChar = ptrEndQFTag ;
                    if ( FindAltSplit( &ptrChar, NULL, szTagQF, szTagEQF, 1 ) ) {
                       if ( ( *ptrChar == L'>' ) &&
                            ( SegLen+(ptrChar-szErrTitle)+1 < 2048 ) ) {
                          ++ptrChar ;
                          NumChars = ptrEndQFTag - ptrColon + 1 ;
                          wcsncpy( szErrTitle, ptrColon, NumChars ) ; 
                          wmemmove( ptrColon, ptrColon+NumChars, ptrChar-ptrColon-NumChars ) ;
                          wcsncpy( ptrChar-NumChars, szErrTitle, NumChars ) ; 
                       }
                    }
                 }
              }
           } else 
           if ( !wcsncmp( ptrColon, szTagQF, 3 ) ) {
              ptrPeriod = wcschr( ptrColon, L'.' ) ;
              if ( ptrPeriod ) {
                 ptrSegStart = ptrPeriod + 1 ;
                 SegLen = 0 ;
                 if ( wcschr( L"nN", *(ptrColon+3) ) )
                    bSegTrans = FALSE ;
                 else
                    bSegTrans = TRUE ;
              }
           }
        }
//         }
           else
           if ( ! wcsncmp( ptr1, L"<twb++", 6 ) ) {
              bInlineNonTrans = TRUE ;
           } else
           if ( ! wcsncmp( ptr1, L"++twb>", 6 ) ) {
              bInlineNonTrans = FALSE ;
           }
        }
        SegLen += wcslen( ptrSegStart ) ;

        /* --------------------------------------------------------------*/
        /* If segment ends in middle of inline non-translatable text.    */
        /* then need to add "++twb>" to end inline segment and "<twb++"  */
        /* to restart inline text.                                       */
        /*    Before:  xx<twb++xx:eqff.qff n=nn.xx++twbxx                */
        /*    After:   xx<twb++xx++twb>:eqff.qff n=nn.<twb++xx++twbxx    */
        /* --------------------------------------------------------------*/
        if ( bRecheckInlineNonTrans ) {                     
           bInlineNonTrans = bStartInlineNonTrans ;
           for( ptr1=szErrText ; *ptr1 ; ++ptr1 ) {
              if ( ( ! wcsncmp( ptr1, szTagEQF, 4 ) ) &&
                   ( ! wcschr( L"nN", *(ptr1+4) ) ) &&   /* Ignore :eqfn */
                   ( bInlineNonTrans ) ) {
                 ptrEndQFTag = wcschr( ptr1, L'.' ) ;  
                 if ( ptrEndQFTag )
                    ptrEndQFTag = wcschr( ptrEndQFTag+1, L'.' ) ; 
                 if ( ptrEndQFTag ) {
                    ++ptrEndQFTag ;
                    wmemmove( ptrEndQFTag+6, ptrEndQFTag, wcslen(ptrEndQFTag)+1 ) ;
                    wcsncpy( ptrEndQFTag, L"<twb++", 6 ) ;
                    wmemmove( ptr1+6, ptr1, wcslen(ptr1)+1 ) ;
                    wcsncpy( ptr1, L"++twb>", 6 ) ;
                    bInlineNonTrans = FALSE ;
                 }
              }
              else
              if ( ! wcsncmp( ptr1, L"<twb++", 6 ) ) {
                 bInlineNonTrans = TRUE ;
              } else
              if ( ! wcsncmp( ptr1, L"++twb>", 6 ) ) {
                 bInlineNonTrans = FALSE ;
              }
           }

        }
//DebugFile1 << "ENDRCD:  "<<SegLen<<"\n";
//DebugFile1.close();






        s = szErrText;
        TotalByteRead += s.length() ;
        EQFSETSLIDER(hSlider, (TotalByteRead*2*100)/FileSize) ;

        s.change(L"<twb--", L"") ;
        s.change(L"--twb>", L"") ;

        s.change(L"&TWBVAR;", L"") ;   
        s.change(L"&TWBPER;", L".") ;  
        s.change(L"<twb##?##twb>", L"?") ;      

        if ( bPartialRecord )        
           OutFile << s;             // Block of long line, not yet end of line
        else
           OutFile << s << ENDL ;    //  End of line

        b1stRead = FALSE ;
    } /* while */

    if (FileSize == ZERO) {  //
        wcscpy( szErrText, L"File does not contain any information, 0 bytes in size.  Analysis terminated." ) ;
        wcscpy( szErrTitle, L"Invalid Source" ) ;
        ShowError( szErrTitle, szErrText, FALSE ) ;
        bReturn = FALSE;
    }

    if ( bResequence ) 
       resequence_TM2( out ) ;

    EQFSETSLIDER(hSlider, 100) ;
//  InFile.~wifstream();

    if ( szErrText ) free(szErrText) ;

    return(bReturn);

} /* Parse */


/*****************************************************************************/
/*  PostExport                                                               */
/*                                                                           */
/*  Function called by EQFPOSTUNSEG2.                                        */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be processed.                            */
/*****************************************************************************/

BOOL PostExport(PSZ in, PSZ out, PSZ markup )
{
    wchar_t szChar[2] ;
    wchar_t szEOL[10] ;
    wchar_t *szIn ;
    wchar_t *ptrChar, *ptrChar2 ;
    wifstream InFile ( in ) ;
    wofstream OutFile ( out ) ;
    IString s, s1 ;
    unsigned idx, idx2, idx3 ;
    BOOL    bSkipEOL = FALSE ;
    BOOL    b1stRead = TRUE;

    szIn = (wchar_t*)malloc( MAX_BUFFER_SIZE*sizeof(wchar_t)*3 ) ;

    BOOL bUTF16 = InFile.IsUTF16();
    OutFile.InsertBOM();
    bPreserveEOL = FALSE ;
    wcscpy( szEOL, ENDL ) ;

    while ( InFile.getline(szIn, MAX_BUFFER_SIZE, bUTF16) != NULL ) {

       if ( ( wcslen(szIn) == MAX_BUFFER_SIZE-1 ) ||
            ( ( b1stRead ) &&        
              ( bUTF16   ) &&
              ( wcslen(szIn) == MAX_BUFFER_SIZE-2 ) ) ) {
           ptrChar  = wcsrchr( szIn, L'<' ) ;
           ptrChar2 = wcsrchr( szIn, L'>' ) ;
           if ( ( ptrChar!=NULL ) &&
                ( ( ptrChar2 == NULL ) ||
                  ( ptrChar2 < ptrChar ) ) ) { /* End with open tag */
              for( szChar[0]=NULL ;            /* Find end of tag   */
                   szChar[0]!= L'>' &&
                   wcslen(szIn)<MAX_BUFFER_SIZE*2 &&  
                   (InFile.getline(szChar,2,bUTF16)!=NULL ) ;
                   wcscat(szIn,szChar) ) ;
           }
        }
       b1stRead = FALSE ;


        if ( ! wcsncmp( szIn, L"<TWB--ANSI--TWB>", 16 ) ) {  /* ANSI encoding  */
            bCodePageANSI = TRUE ; 
            continue ;
        }
        if ( ! wcsncmp( szIn, L"<TWB--HTML--TWB>", 16 ) ) {  /* HTML encoding  */
            bCodePageHTML = TRUE ; 
            continue ;
        }
        if ( ! wcsncmp( szIn, L"<TWB--UTF8--TWB>", 16 ) ) {  /* UTF-8 encoding */
            bCodePageUTF8 = TRUE ; 
            continue ;
        }
        if ( ! wcsncmp( szIn, L"<TWB--UTF16--TWB>", 17 ) ) {  /* UTF-16 encoding */
            bCodePageUTF16= TRUE ; 
            continue ;
        }
        if ( ! wcsncmp( szIn, L"<TWB--ASCII--TWB>", 17 ) ) {  /* ASCII encoding */
            continue ;
        }
        if ( ! wcsncmp( szIn, L"<TWB--EOL=", 10 ) ) {        /* Preserve source EOL    */
            bPreserveEOL = TRUE ; 
            szEOL[0] = NULL ;
            for( ptrChar=&szIn[10] ; *ptrChar!=L'-11' ; ++ptrChar ) {
               if ( *ptrChar == L'A' ) 
                  wcscat( szEOL, L"\n" ) ;
               else
                 if ( *ptrChar == L'D' ) 
                    wcscat( szEOL, L"\r" ) ;
            }
            continue ;
        }


        if ( bSkipEOL ) {
           bSkipEOL = FALSE ;
           if ( szIn[0] == L'>' ) 
              s = &szIn[1];
           else
              s = szIn;
        } else {
           s = szIn;
        }
        s.change(L"<twb##", L"") ;     /* Remove translatable attr. value tags */
        s.change(L"##twb>", L"") ;
        s.change(L"<twb++", L"") ;     /* Remove non-trans neutral text tags   */
        s.change(L"++twb>", L"") ;
        s.change(L"&TWBGT;", L">") ;
        if ( s.indexOf(L"<twbnl") ) {
           bSkipEOL = TRUE ;
           s.change(L"<twbnl", L"") ;
        }
        s.change(L"<twbnl\r\n>", L">") ;
        if ( bXHTML ) {
           s.change(L"fontQ", L"font") ;      /* XHTML */
           s.change(L"FONTQ", L"FONT") ;      /* XHTML */
        }

        /*********************************************************************/
        /* Handle DITA pseudo tags by restoring the original tag name from   */
        /* the added TWB attribute value.                                    */
        /*********************************************************************/
        idx = 0 ;
        while ( (idx=s.indexOf(L" TWB=\"",idx+1)) > 0 ) {  /* Real tag name?   */
            idx2 = s.lastIndexOf( '<', idx ) ;    /* Find tag start           */
            if ( s.subString(idx2+1,1) == '/' )   /* Handle end tag           */
                ++idx2 ;
            idx += 6 ;
            idx3 = s.indexOf( L"\"", idx ) ;       /* Find end of TWB attribute*/
            if ( (idx2>0) && (idx3>0) ) {
                s1 = s.subString( idx, idx3-idx, ' ' ) ;
                s.remove( idx2+1, idx3-idx2 ) ;
                s.insert( s1, idx2, ' ' ) ;
                idx = idx2 ;
            }
        }

        if ( ( wcslen(szIn) < MAX_BUFFER_SIZE-1 ) &&  //end of line
             ( ! bSkipEOL ) ) 
           OutFile << s << szEOL ;
        else
            OutFile << s;  //block of long line, not yet end of line
    } 

    if ( szIn ) free( szIn ) ;

    return(TRUE);
}/* PostExport */



/*****************************************************************************/
/*  ParseTags                                                                */
/*                                                                           */
/*  This function will process a line at a time from the input file and      */
/*  parse it into a sequence of tokens.  It will figure out the appropriate  */ 
/*  pattern,  i.e. translatable pattern or non-translatable.                 */
/*                                                                           */
/*  Return:  TRUE  - File successfully processed.                            */
/*           FALSE - File could not be processed.                            */
/*****************************************************************************/
inline void ParseTags(IString& s, Token& XTok )
{

    wchar_t     szErrTitle[128] ;
    wchar_t     szSaveCurrentTagName[XML_TAG_LENGTH] ;  
    wchar_t     *swInput ;  
    short    sTokenType ;
    BOOL     bPrevNeutralTag = FALSE ;
    BOOL     bEmptyTag = FALSE ;


    swInput = (wchar_t*)malloc( MAX_BUFFER_SIZE*sizeof(wchar_t)*3 ) ;

    if ( bXHTML ) {
       wcscpy( swInput, s ) ;
       if ( HandleXHTMLTags( swInput ) ) {
          s = IString(swInput) ;
          XTok.SetString(s) ;
       }
    }


    if ( XTbl.XML_tbl[NONTRANS] ) {
       if ( XTok.GetScriptState() == SCRIPT_STATE_JAVASCRIPT ) {
          XTbl.SetStateTable( STATE_JS ) ;
          XTbl.setoff( JS_LCOMMENT ) ;
       } else {
          if ( XTok.GetScriptState() == SCRIPT_STATE_VBSCRIPT ) {
             XTbl.SetStateTable( STATE_VBS ) ;
             XTbl.setoff( VBS_COMMENT ) ;
          }
       }
    }

    while ( (sTokenType = XTok.GetNextToken()) != eol_tok  ) {
       if ( ( XTok.GetScriptState() == SCRIPT_STATE_NONE ) ||
            ( ( ( XTok.GetScriptState() != SCRIPT_STATE_JAVASCRIPT ) ||
                ( ! HandleJavaScriptTokens( &sTokenType, XTok )    ) ) &&
              ( ( XTok.GetScriptState() != SCRIPT_STATE_VBSCRIPT   ) ||
                ( ! HandleVBScriptTokens( &sTokenType, XTok )      ) ) ) ) {

       /**********************************************************************/
       /*                                                                    */
       /*  Handle normal XML tokens.                                         */
       /*                                                                    */
       /**********************************************************************/
          bEmptyTag = FALSE ;

          switch (sTokenType) {
          /* -----------------------------------------------------*/
          case sys_tok :
              if ( XTbl.on(SYSTEM) )
                  XTbl.seton(SYSTEM) ;
              break ;
          /* -----------------------------------------------------*/
          case rdoc_tok :
              if ( XTbl.off(DOCTAG) ) {
                  XTbl.setoff(DOCTAG) ;
                  XTok.AddTagTransOn() ;
              }
              break ;
          /* -----------------------------------------------------*/
          case ldoc_tok :
              if ( XTbl.on(DOCTAG) ) {
                  XTbl.seton(DOCTAG) ;
                  XTok.AddTagTransOff() ;
              }
              break ;
          /* -----------------------------------------------------*/
          case lxml_tok :
              if ( XTbl.on(XMLDOC) ) {
                  XTbl.seton(XMLDOC) ;
                  XTok.AddTagTransOff() ;
              }
              break ;
          case lxmlneut_tok :                          
			if ( XTbl.on(TAG) ) {            /* Not in comment  */
              }
              break ;
          /* -----------------------------------------------------*/
          case xml_tok :
              if ( XTbl.on(XMLNAME) )
                  XTbl.seton(XMLNAME) ;
              break ;
          /* -----------------------------------------------------*/
          case encode_tok :
              if ( XTbl.on(ENCODING) )
                  XTbl.seton(ENCODING) ;
              break ;
          /* -----------------------------------------------------*/
          case rxml_tok :
              if ( XTbl.off(ENCODING) ) {
                  XTbl.setoff(ENCODING) ;
              }
              if( XTbl.off(XMLNAME) ) {
                  XTbl.setoff(XMLNAME) ;
              }
			  if ( XTbl.off(XMLDOC) ) {
			     XTbl.setoff(XMLDOC) ;
			     XTok.AddTagTransOn() ;
			  }

              break ;
          /* -----------------------------------------------------*/
          case entity_tok :
              if ( XTbl.on(ENTITY) ) {
                  XTok.SaveCurrentTagName() ;
                  if ( XTbl.on(DOCTAG) ) {
                      XTok.AddTagTransOff() ;
                  }
                  XTbl.seton(ENTITY) ;
              }
              break ;
          /* -----------------------------------------------------*/
          case l_cdata_tok :
              if ( ( XTbl.on(CDATA) ) &&
                   ( XTok.GetTransStateAttr() != TRANS_STATE_OFF ) ) { 
                 XTbl.seton(CDATA) ;
                 XTok.AddTagTransOff() ;
                 XTok.AdjustLastToken( +1 ) ; 
                 XTok.AddTagTransOn() ;
              }
              break ;
          /* -----------------------------------------------------*/
          case r_cdata_tok :
              if ( XTbl.off(CDATA) ) {
                  XTbl.setoff(CDATA);
                  XTok.AddTagTransOff() ;
                  XTok.AddTagTransOn();
              }
              break ;
          /* -----------------------------------------------------*/
          case starttrans_tok :                        
              if ( XTbl.on(COMENT) ) {
                 //XTok.SetTransState( TRANS_STATE_ON ) ;
                 bStartTrans = TRUE ;
                 //XTok.AddTagTransOn();
              break ;
              } 
              goto skip_endnontrans ;
          /* -----------------------------------------------------*/
          /* Intentionally left out break to fall into comment.   */
          /* -----------------------------------------------------*/
          case endtrans_tok :                          
              XTok.SetTransState( TRANS_STATE_OFF ) ;
              bStartTrans = FALSE ;
              //XTok.AddTagTransOff();
              goto skip_endnontrans ;
          /* -----------------------------------------------------*/
          /* Intentionally left out break to fall into comment.   */
          /* -----------------------------------------------------*/
          /* -----------------------------------------------------*/
          case startnontrans_tok :
              if ( XTbl.on(COMENT) &&
                   XTbl.on(NONTRANSBLOCK) ) {
                 XTok.AddTagTransOff();
                 XTbl.seton(NONTRANSBLOCK) ;
                 break ;
              } 
              goto skip_endnontrans ;
          /* -----------------------------------------------------*/
          /* Intentionally left out break to fall into comment.   */
          /* -----------------------------------------------------*/
          case endnontrans_tok :
              if ( XTbl.off(NONTRANSBLOCK) ) {
                 XTbl.setoff(NONTRANSBLOCK) ;
              } 
          skip_endnontrans:
          /* -----------------------------------------------------*/
          /* Intentionally left out break to fall into comment.   */
          /* -----------------------------------------------------*/
          case l_comment_tok :
              if ( XTbl.on(COMENT) ) {
                 if ( XTok.GetTransStateAttr() != TRANS_STATE_OFF )  
                 XTok.AddTagTransOff();
                 if ( sIgnoreCommentState == IGNORECOMMENT_STATE_ON ) {
                    XTok.AddTagTransOn();
                    sIgnoreCommentState = IGNORECOMMENT_STATE_START ;
                 } else {
                    XTbl.seton(COMENT);
                 }
              } else if ( XTbl.on(HCOMMENT) )
                 XTbl.seton(HCOMMENT) ;
              break ;
          /* -----------------------------------------------------*/
          case r_comment_tok:
              if ( XTbl.off(COMENT) ) {
                 if ( sIgnoreCommentState == IGNORECOMMENT_STATE_START ) {
                    XTok.AddTagTransOff();
                    sIgnoreCommentState = IGNORECOMMENT_STATE_ON ;
                 }
                 if ( XTok.GetTransStateAttr() != TRANS_STATE_OFF ) 
                 XTok.AddTagTransOn();
                 XTbl.setoff(COMENT);
              } else if ( XTbl.off(HCOMMENT) )
                 XTbl.setoff(HCOMMENT) ;
              break ;
          /* -----------------------------------------------------*/
          case dquote_tok :
              if ( XTbl.on(DQUOTE) ) {
                 if ( ! XTok.CheckSkipAttrValue() ) {
                    if ( XTbl.off(NEUTTAG) )
                       XTok.AddNeutAttrOn();
                    else
                       XTok.AddTagTransOn();
                    XTbl.seton(DQUOTE);
                    if ( ( bBidi  ) &&
                         ( bXHTML ) ) {
                       XTok.HandleBidiAttr() ;
                    }
                    /* After handling the \" ,switch back to the JAVASCRIPT state */
                    if ( ( bEscapeDquote ) &&
                         ( XTok.GetScriptState() == SCRIPT_STATE_NONE ) ) { 
                       bEscapeDquote = FALSE ;
                       XTok.SetScriptState ( SCRIPT_STATE_JAVASCRIPT ) ;   
                    }
                    break;
                 } else {  /* Ignore the ending quote */
                    if ( bEmptyEntityValue ) {
                       XTok.AdjustLastToken(1) ;
                       bEmptyEntityValue = FALSE ;
                    }
                 }
                 XTbl.setoff(TRANSATTR) ;
              } 

              if ( XTbl.off(DQUOTE) ) {
                  if ( XTbl.off(NEUTTAG) )
                     XTok.AddNeutAttrOff(0);
                  else
                     XTok.AddTagTransOff();
                  XTbl.setoff(DQUOTE);
                  XTbl.setoff(ENCODING) ;
                  XTbl.setoff(XMLLANG) ;
                  XTbl.setoff(TRANSATTR) ;
                  if( bTransAttrHasNoCondAttr){       
                     bTransAttrHasNoCondAttr = FALSE; 
                  }
              } else {
                 if ( ( XTbl.on(CONDATTR) ) &&           
                      ( XTok.CheckCondAttr() ) ) {
                    XTbl.seton(CONDATTR) ;
                 }
                 if ( XTok.GetTransStateAttr() == TRANS_STATE_ATTR_CHECK ) {
                    XTok.CheckTransStateAttr() ;
                 }
                 if ( XTbl.on(HDQUOTE) ) {
                     XTbl.seton(HDQUOTE) ;
                 } else
                 if ( XTbl.off(HDQUOTE) ) {
                     XTbl.setoff(HDQUOTE) ;
                     if( bTransAttrHasNoCondAttr){   /* 7-1-10 */
                         bTransAttrHasNoCondAttr = FALSE; 
                     }
                 }
                 if ( ( bBidi  ) &&
                      ( bXHTML ) ) {
                    XTok.HandleBidiAttr() ;
                 }
              }
              /* After handling the \" ,switch back to the JAVASCRIPT state */
              if ( ( bEscapeDquote ) &&
                   ( XTok.GetScriptState() == SCRIPT_STATE_NONE ) ) {  
                 bEscapeDquote = FALSE ;
                 XTok.SetScriptState ( SCRIPT_STATE_JAVASCRIPT ) ;   
              }
              break ;
          /* -----------------------------------------------------*/
          case squote_tok :
              if ( XTbl.on(SQUOTE) ) {
                 if ( ! XTok.CheckSkipAttrValue() ) {
                    if ( XTbl.off(NEUTTAG) )
                       XTok.AddNeutAttrOn();
                    else                                 
                       XTok.AddTagTransOn();
                    XTbl.seton(SQUOTE);
                    if ( ( bBidi  ) &&
                         ( bXHTML ) ) {
                       XTok.HandleBidiAttr() ;
                    }
                    break;
                 }
                 XTbl.setoff(TRANSATTR) ;
              } 

              if ( XTbl.off(SQUOTE) ) {                  
                 if ( XTbl.off(NEUTTAG) )
                    XTok.AddNeutAttrOff(0);
                 else                                    
                    XTok.AddTagTransOff();
                 XTbl.setoff(SQUOTE);
                 XTbl.setoff(ENCODING) ;
                 XTbl.setoff(XMLLANG) ;
                 XTbl.setoff(TRANSATTR) ;
              } else {
                 if ( ( XTbl.on(CONDATTR) ) &&
                      ( XTok.CheckCondAttr() ) ) {
                    XTbl.seton(CONDATTR) ;
                 }
                 if ( XTok.GetTransStateAttr() == TRANS_STATE_ATTR_CHECK ) {
                    XTok.CheckTransStateAttr() ;
                 }
                 if ( XTbl.on(HSQUOTE)) {
                     XTbl.seton(HSQUOTE);
                 } else
                 if ( XTbl.off(HSQUOTE)) {
                     XTbl.setoff(HSQUOTE);
                 }
                 if ( ( bBidi  ) &&
                      ( bXHTML ) ) {
                    XTok.HandleBidiAttr() ;
                 }
              }
              break ;
          /* -----------------------------------------------------*/
          case transstatetag_tok :
          case transstateattrtag_tok :
              if ( XTbl.on(COMENT) ) {                 
                 if ( XTok.GetTransState() == TRANS_STATE_ON ) {
                    if ( XTbl.on(TAG) ) {
                       XTbl.seton(TAG) ;
                       XTbl.setoff(NEUTTAG) ;
                       XTbl.setoff(ATTRTAG) ;
                       if ( sTokenType == transstateattrtag_tok ) {
                          XTbl.seton(ATTRTAG) ;
                          XTbl.seton(CONDATTR) ;
                       }
                       XTok.ManageLevelTransStateAttr( &bNeutralTag, bPrevNeutralTag ) ;
                       XTok.AddTagTransOff() ;
                       XTbl.seton(NONTRANSTAG) ;
                       break ;
                    }
                    XTok.AddTagTransOff() ;
                    XTbl.seton(NONTRANSTAG) ;
                 } else {
                    XTok.SetTransState( TRANS_STATE_ON ) ;
                    if ( XTok.GetScriptState() >= SCRIPT_STATE_OTHER ) {
                       XTok.SetScriptState ( SCRIPT_STATE_NONE ) ;
                    }
                    XTbl.SetStateTable( STATE_XML ) ;
                    XTbl.setoff(NONTRANS) ;
                    if ( XTbl.on(TAG) ) {
                       XTbl.seton(TAG) ;
                       XTbl.setoff(NEUTTAG) ;
                       XTbl.setoff(ATTRTAG) ;
                       if ( sTokenType == transstateattrtag_tok ) {
                          XTbl.seton(ATTRTAG) ;
                          XTbl.seton(CONDATTR) ;
                       }
                    }
                 }
              }
          /* -----------------------------------------------------*/
          /* Intentionally left out break to fall into "ltag_tok". */
          /* -----------------------------------------------------*/
          case ltag_tok :
          case attrtag_tok :
          case linestag_tok :
          case neuttag_tok :
          case neutattrtag_tok :
              if ( XTbl.on(TAG) ) {
	              if ( bStartTrans ) {                
	                  XTok.SetTransState(TRANS_STATE_ON) ;
	              }
                  XTbl.seton(TAG) ;
                  XTbl.setoff(NEUTTAG) ;
                  XTbl.setoff(ATTRTAG) ;
                  XTok.SaveCurrentTagName() ;
                  if ( XTok.GetTransStateAttr() == TRANS_STATE_OFF ) {
                     if ( bNeutralTag )          /* End non-trans text before tag */
                        XTok.AddNeutTextOn() ;  
                     else
                        XTok.AddTagTextOn() ;
                  }

                  XTok.FindTransStateAttribute( NULL ) ; /* Find state attr in complete tag */ 

                  if ( sTokenType == neuttag_tok ) {
                     XTbl.seton(NEUTTAG) ;
                     XTok.AddTagTransOn() ;
                  } else {
                     if ( sTokenType == neutattrtag_tok ) {
                        XTbl.seton(NEUTTAG) ;
                        XTbl.seton(ATTRTAG) ;
                        XTbl.seton(CONDATTR) ;
                        XTok.AddTagTransOn() ;
                     } else {
                        XTok.AddTagTransOff() ;
                        if ( sTokenType == attrtag_tok ) {
                           XTbl.seton(ATTRTAG) ;
                           XTbl.seton(CONDATTR) ;
                        }
                     }
                  }
              }
              break ;
          /* -----------------------------------------------------*/
          case transattr_tok :
              if ( XTbl.on(TRANSATTR) ) {
                  XTbl.seton(TRANSATTR) ;
                  if ( ( bBidi  ) &&
                       ( bXHTML ) ) 
                     XTok.SaveCurrentAttrName() ;
              }
              break ;
          /* -----------------------------------------------------*/
          case condattr_tok :
              if ( ( bXHTML ) &&   
                   ( ! XTbl.off(COMENT) ) ){ 
                 bXHTMLCondAttr = TRUE;
                 if ( XTbl.off(CONDATTR) ) {
                     XTbl.setoff(CONDATTR) ;
                 }
              } else {
                 if ( XTbl.off(CONDATTR) ) {
                    XTbl.setoff(CONDATTR) ;
                 }
              }
              break ;
          /* -----------------------------------------------------*/
          case stateattr_tok :
              XTok.SetTransStateAttr( TRANS_STATE_ATTR_CHECK ) ;
              XTok. SaveCurrentAttrName() ;
              break ;
          /* -----------------------------------------------------*/
          case xmllang_tok :
              if ( XTbl.on(XMLLANG) ) {
                  XTbl.seton(XMLLANG) ;
              }
              break ;
          /* -----------------------------------------------------*/
          case rtagempty_tok :
              bEmptyTag = TRUE ;
          /* -----------------------------------------------------*/
          /* Intentionally left out break to fall into "rtag_tok". */
          /* -----------------------------------------------------*/
          case rtag_tok :
              if (XTbl.off(DOCTAG)) {
                  // it is possible that this is a <!DocType tag.
                  // Verify it is not ending one of
                  // those tags first!
                  XTbl.setoff(DOCTAG) ;
                  XTok.AddTagTransOn() ;
              } else {
                  if ( XTbl.off(TAG) ) {
                     bPrevNeutralTag = bNeutralTag ;
                     if ( XTbl.off(NEUTTAG) ) 
                        bNeutralTag = TRUE ;
                     else
                        bNeutralTag = FALSE ;
                      XTbl.setoff(TRANSATTR) ;
                      XTbl.setoff(CONDATTR) ;
                      XTbl.setoff(ATTRTAG) ;
                      XTbl.setoff(NEUTTAG) ;
                      XTbl.setoff(TAG) ;
                      XTok.nInsertAttrTag = INSERTATTR_OFF ; 
                      if ( bCDATA_XHTMLTag ) {               
                         bCDATA_XHTMLTag = FALSE ;
                         XTbl.seton(CDATA) ;
                      }
                      if ( ( XTok.GetScriptState() == SCRIPT_STATE_TAG_UNKNOWN ) ||
                           ( XTok.GetScriptState() == SCRIPT_STATE_TAG_JS      ) ||
                           ( XTok.GetScriptState() == SCRIPT_STATE_TAG_VBS     ) ) {
                         XTok.SetTransState( TRANS_STATE_OFF ) ;
                         XTbl.seton(NONTRANS) ;
                         if ( XTok.GetScriptState() == SCRIPT_STATE_TAG_JS ) {
                            XTok.SetScriptState( SCRIPT_STATE_JAVASCRIPT ) ;
                            XTbl.SetStateTable(STATE_JS) ;
                            XTbl.initializeState(STATE_JS) ;
                         } else {
                            if ( XTok.GetScriptState() == SCRIPT_STATE_TAG_VBS ) {
                               XTok.SetScriptState( SCRIPT_STATE_VBSCRIPT ) ;
                               XTbl.SetStateTable(STATE_VBS) ;
                               XTbl.initializeState(STATE_VBS) ;
                            } else {
                               XTok.SetScriptState( SCRIPT_STATE_OTHER ) ;
                            }
                         }
                      } else {
                         if ( ( XTbl.statetype == STATE_XML ) &&
                              ( XTbl.off(GLOBALTAG) ) ) {
                            XTbl.setoff(GLOBALTAG);
                            XTbl.seton(NONTRANS) ;
                            sScriptState = sPrevScriptState ;
                            sPrevScriptState = SCRIPT_STATE_NONE ;
                            XTok.SetTransState( sPrevTransState ) ;
                            sPrevTransState = TRANS_STATE_NONE ;
                            if ( sScriptState == SCRIPT_STATE_JAVASCRIPT ) 
                               XTbl.SetStateTable( STATE_JS ) ;
                            else
                               XTbl.SetStateTable( STATE_VBS ) ;
                            XTbl.setoff(JS_GLOBALTAG);
                         } else {
                             if ( XTbl.off(NONTRANSTAG) ) {
                                if ( ( XTok.GetTransState() == TRANS_STATE_ON      ) ||
                                     ( XTok.GetTransStateAttr() == TRANS_STATE_OFF ) ) { 
                                   XTok.AddTagTransOff() ;
                                   XTbl.seton(NONTRANS) ;
                                   XTok.SetTransState( TRANS_STATE_OFF ) ;
                                } else {
                                   XTok.AddTagTransOn() ;
                                }
                                XTbl.setoff(NONTRANSTAG) ;
                             } else {
                                XTok.AddTagTransOn() ;
                             }
                         }
                      }
                      XTok.ManageLevelTransStateAttr( &bNeutralTag, bPrevNeutralTag ) ;
                      if ( bEmptyTag ) {                       
                         wcscpy( szSaveCurrentTagName, XTok.GetCurrentTagName() ) ;
                         XTok.SetCurrentTagName( L"/" ) ;
                         XTok.ManageLevelTransStateAttr( &bNeutralTag, bPrevNeutralTag ) ;
                         XTok.SetCurrentTagName( szSaveCurrentTagName ) ;
                      }
                      if ( XTok.GetTransStateAttr() == TRANS_STATE_OFF ) {
                         if ( bNeutralTag ) {       /* Start non-trans text after tag */
                            XTok.AddNeutTextOff() ;
                         } else {
                            XTok.AddTagTextOff() ;
                         }
                      } 
                      if ( ! bNeutralTag ) {
                         XTok.SetNeutralBreakOn() ;
                         if ( ( bInsertEOL ) &&
                              ( XTok.bCurrentTagEnd ) )
                            XTok.AddLineBreak() ;
                      }
                  } else if ( XTbl.off(ENTITY) ) {
                      XTbl.setoff(ENTITY) ;
                      XTbl.setoff(SYSTEM) ;
                      if (XTbl.on(DOCTAG))
                          XTok.AddTagTransOn() ;
                  }
              }
              break ;
          /* -----------------------------------------------------*/
          case dtd_tok :
              if ( XTbl.on(DTDNAME) ) {
                 XTok.SetDTDInfo( s, FALSE ) ;
              }
              break ;
          /* -----------------------------------------------------*/
          case period_tok:                             
              if ( ( ( XTbl.off(DQUOTE) ) ||
                     ( XTbl.off(SQUOTE) ) ) &&
                   ( XTbl.off(NEUTTAG) ) ) {
                 XTok.ReplacePeriod();
              }
              break ;


          /* -----------------------------------------------------*/
          case HTML_attribute_equal_tok:    
			  if ( bXHTMLCondAttr &&
				   ( !wcsicmp(XTok.GetCurrentTagName(), L"input" ) ) ){   // currently <input> has only one conditional attribute "type"
				 if ( XTok.CheckCondAttr() ) {
					XTbl.seton(CONDATTR);
				 }
			  }
			  bXHTMLCondAttr = FALSE;
			  break;
          /* -----------------------------------------------------*/
          default :
             break ;                         // unknown state
          } // switch

       }


    } // while, each token in a line

    XTok.RemoveInsertAttribute() ;

    s = XTok.GetString() ;

    if ( swInput ) free( swInput ) ;

}/* ParseTags */




/*****************************************************************************/
/*  HandleJavaScriptTokens                                                   */
/*                                                                           */
/*  This function will handle the JavaScript unique tokens.                  */
/*                                                                           */
/*  Return:  TRUE  - Token has been handled.                                 */
/*           FALSE - Token has not yet been handled.                         */
/*****************************************************************************/
inline BOOL HandleJavaScriptTokens( short *sTokenType, Token& XTok )
{
    wchar_t  tchar ;
    BOOL     bHandled ;

    if ( ( XTbl.statetype == STATE_XML ) &&  
         ( XTbl.off(GLOBALTAG) ) ) {
       if ( *sTokenType != rtag_tok ) {
          if ( *sTokenType == JS_globaltag_tok ) {
             *sTokenType = ltag_tok ;
          }
       } else {
          if ( ( XTbl.on(DQUOTE) ) &&      
               ( XTbl.on(SQUOTE) ) &&
               ( XTbl.on(HDQUOTE) ) &&
               ( XTbl.on(HSQUOTE) ) ) {
             XTbl.SetStateTable( STATE_JS ) ;
          }
       }
       return( FALSE ) ; 
    }

    if ( XTbl.XML_tbl[NONTRANS] ) {

       XTbl.SetStateTable( STATE_JS ) ;
       bHandled = TRUE ;

       switch (*sTokenType) {
       /* --------------------------------------------------*/
       case JS_equal_tok :
           if ( XTbl.on(JS_ASSIGN) ) {
              XTbl.seton(JS_ASSIGN) ;
           }
           break ;
       /* --------------------------------------------------*/
       case JS_plus_tok :
           if ( XTbl.off(JS_ASSIGN) ) {
              XTbl.setoff( JS_ASSIGN1 ) ;
              XTbl.setoff( JS_ASSIGN2 ) ;
           }
           break ;
       /* --------------------------------------------------*/
       case JS_lparen_tok :
           if ( XTbl.on(JS_PAREN ) ) {
              XTbl.seton( JS_PAREN ) ;
              ++sSCRIPT_ParenCount ;
           }
           break ;
       /* --------------------------------------------------*/
       case JS_rparen_tok :
           if ( XTbl.off(JS_PAREN) ) {
              --sSCRIPT_ParenCount ;
              if ( sSCRIPT_ParenCount == 0 ) {
                 XTbl.setoff( JS_PAREN ) ;
              }
           }
           break ;
       /* --------------------------------------------------*/
       case JS_lbracket_tok :
           if ( XTbl.on(JS_ARRAY ) ) {
              XTbl.seton( JS_ARRAY ) ;
           }
           break ;
       /* --------------------------------------------------*/
       case JS_rbracket_tok :
           if ( XTbl.off(JS_ARRAY ) ) {
              XTbl.setoff( JS_ARRAY ) ;
           }
           break ;
       /* --------------------------------------------------*/
       case JS_comma_tok :
           if ( XTbl.off(JS_ARRAY) ) {
              XTbl.setoff( JS_ASSIGN1 ) ;
              XTbl.setoff( JS_ASSIGN2 ) ;
           }
           break ;
       /* --------------------------------------------------*/
       case JS_lb_comment_tok :
           if ( XTbl.on(JS_BCOMMENT ) ) {
              XTbl.seton( JS_BCOMMENT ) ;
              if ( XTbl.on(JS_ASSIGN2 ) ) {
                 XTbl.seton( JS_ASSIGN2 ) ;
              }
           }
           break ;
       /* --------------------------------------------------*/
       case JS_rb_comment_tok :
           if ( XTbl.off(JS_BCOMMENT) ) {
              XTbl.setoff( JS_BCOMMENT ) ;
           }
           break ;
       /* --------------------------------------------------*/
       case startnontrans_tok :
           if ( XTbl.on(JS_LCOMMENT) &&
                XTbl.on(JS_NONTRANSBLOCK) ) {
              XTbl.seton(JS_NONTRANSBLOCK) ;
              XTbl.seton( JS_LCOMMENT ) ;
              break ;
           } 
           goto skip_endnontrans ;
       /* -----------------------------------------------------*/
       /* Intentionally left out break to fall into comment.   */
       /* -----------------------------------------------------*/
       case endnontrans_tok :
           if ( XTbl.off(JS_NONTRANSBLOCK) ) {
              XTbl.setoff(JS_NONTRANSBLOCK) ;
              XTbl.seton( JS_LCOMMENT ) ;
              break;
           } 
       skip_endnontrans:

       /* --------------------------------------------------*/
       case JS_l_comment_tok :
           if ( XTbl.on(JS_LCOMMENT) ) {
              XTbl.seton( JS_LCOMMENT ) ;
              if ( XTbl.on(JS_ASSIGN2 ) ) {
                 XTbl.seton( JS_ASSIGN2 ) ;
              }
           }
           break ;
       /* --------------------------------------------------*/
       case dquote_tok :
           if ( XTbl.on(JS_DQUOTE) ) {
               XTbl.seton(JS_DQUOTE);
               XTbl.SetStateTable( STATE_XML ) ;
               XTbl.setoff(NONTRANS) ;
               XTok.SetTransState( TRANS_STATE_ON ) ;
               XTok.AddTagTransOn();
           } else if ( XTbl.on(JS_HDQUOTE) ) {
               XTbl.seton(JS_HDQUOTE) ;
           } else if ( XTbl.off(JS_HDQUOTE) ) {
               XTbl.setoff(JS_HDQUOTE) ;
           }
           break ;
       /* --------------------------------------------------*/
       case squote_tok :
           if ( ( sScriptState == SCRIPT_STATE_JAVASCRIPT ) &&
                ( XTbl.SCRIPT_tbl[JS_SQUOTE] ) &&
                ( XTbl.IsOn(TAG) ) &&
                ( XTok.GetTransState() == TRANS_STATE_ON ) ) {
              XTok.AddTagTransOn();
           }
           if ( XTbl.on(JS_SQUOTE) ) {
               XTbl.seton(JS_SQUOTE);
               XTbl.SetStateTable( STATE_XML ) ;
               XTbl.setoff(NONTRANS) ;
               XTok.SetTransState( TRANS_STATE_ON ) ;
               XTok.AddTagTransOn();
           } else if ( XTbl.on(JS_HSQUOTE) ) {
               XTbl.seton(JS_HSQUOTE) ;
           } else if ( XTbl.off(JS_HSQUOTE) ) {
               XTbl.setoff(JS_HSQUOTE) ;
           }
           break ;
       /* --------------------------------------------------*/
       case JS_globaltag_tok :
           if ( XTbl.on(JS_GLOBALTAG) ) {
               XTbl.seton(JS_GLOBALTAG);
               sPrevScriptState = sScriptState ;
               sScriptState = SCRIPT_STATE_NONE ;
               XTbl.SetStateTable( STATE_XML ) ;
               XTbl.seton(GLOBALTAG);
               XTbl.setoff(NONTRANS) ;
               XTok.SetTransState( TRANS_STATE_ON ) ;
               XTok.AdjustLastToken( -1 ) ; 
               XTok.AddTagTransOn();
           }
           break ;
       /* --------------------------------------------------*/
       default :
           break ;                         // unknown state
       }

       if ( XTbl.XML_tbl[NONTRANS] ) {
          tchar = XTok.GetCurrentChar() ;
          if ( XTbl.off(JS_ASSIGN) ) {
             if ( XTbl.on(JS_ASSIGN1) ) { /* Look for start of varaiable */
                if ( iswalpha(tchar) ||
                     ( tchar == '_' ) )
                   XTbl.seton(JS_ASSIGN1) ;
                else
                   if ( ( ! iswspace(tchar)     ) &&
                        ( ! wcschr(L"+=",tchar) ) ) {
                      XTbl.setoff(JS_ASSIGN);
                   }
             } else {
                if ( XTbl.on(JS_ASSIGN2) ) { /* Look for end of variable */
                   if ( iswspace(tchar) ||
                        ( wcschr(L"+",tchar) ) )
                      XTbl.seton(JS_ASSIGN2) ;
                   else
                      if ( !iswalnum(tchar) &&   
                           !wcschr(L"_.)[]",tchar) ) {
                         XTbl.setoff(JS_ASSIGN1) ;
                         XTbl.setoff(JS_ASSIGN) ;
                      }
                } else {
                   if ( XTbl.off(JS_ASSIGN2) &&  /* Already found end of variable */
                        ! XTbl.off(JS_BCOMMENT) &&
                        ! XTbl.off(JS_LCOMMENT) &&
                        ! XTbl.off(JS_PAREN)    &&
                        ! iswspace(tchar) &&
                        ! wcschr(L"+",tchar) ) {
                      XTbl.setoff(JS_ASSIGN2) ;
                      XTbl.setoff(JS_ASSIGN1) ;
                      XTbl.setoff(JS_ASSIGN) ;
                   }
                }
             }
          } 
       }
    } else {

       bHandled = FALSE ;

       switch (*sTokenType) {
          case dquote_tok :
              if ( XTbl.on(DQUOTE) &&
                   XTbl.SCRIPT_tbl[JS_DQUOTE] ) {
                 bHandled = TRUE ;
                 if ( XTbl.IsOn(TAG) ) {     
                    XTok.ForceTransOn();
                 }
                 XTbl.seton(NONTRANS) ;
                 XTbl.SetStateTable( STATE_JS ) ;
                 XTbl.setoff(JS_DQUOTE);
                 XTbl.seton(JS_ASSIGN);
                 XTbl.seton(JS_ASSIGN1);
                 XTbl.seton(JS_ASSIGN2);
                 XTok.SetTransState( TRANS_STATE_OFF ) ;
                 XTok.AddTagTransOff();

                 XTbl.SetStateTable( STATE_XML ) ;
                 XTbl.setoff(ATTRTAG);
                 XTbl.setoff(CONDATTR);
                 XTbl.setoff(DQUOTE);
                 XTbl.setoff(HDQUOTE);
                 XTbl.setoff(HSQUOTE);
                 XTbl.setoff(NEUTTAG);
                 XTbl.setoff(NONTRANSTAG);
                 XTbl.setoff(SQUOTE);
                 XTbl.setoff(TAG);
                 XTbl.setoff(TRANSATTR);
                 XTbl.SetStateTable( STATE_JS ) ;
              }
              break ;
          /* -----------------------------------------------------*/
          case squote_tok :
              if ( XTbl.on(SQUOTE) &&
                   XTbl.SCRIPT_tbl[JS_SQUOTE] ) {
                 bHandled = TRUE ;
                 if ( XTbl.IsOn(TAG) ) {      
                    XTok.ForceTransOn();
                 }
                 XTbl.seton(NONTRANS) ;
                 XTbl.SetStateTable( STATE_JS ) ;
                 XTbl.setoff(JS_SQUOTE);
                 XTbl.seton(JS_ASSIGN);
                 XTbl.seton(JS_ASSIGN1);
                 XTbl.seton(JS_ASSIGN2);
                 XTok.SetTransState( TRANS_STATE_OFF ) ;
                 XTok.AddTagTransOff();

                 XTbl.SetStateTable( STATE_XML ) ;
                 XTbl.setoff(ATTRTAG);
                 XTbl.setoff(CONDATTR);
                 XTbl.setoff(DQUOTE);
                 XTbl.setoff(HDQUOTE);
                 XTbl.setoff(HSQUOTE);
                 XTbl.setoff(NEUTTAG);
                 XTbl.setoff(NONTRANSTAG);
                 XTbl.setoff(SQUOTE);
                 XTbl.setoff(TAG);
                 XTbl.setoff(TRANSATTR);
                 XTbl.SetStateTable( STATE_JS ) ;
              }
              break ;
          /* -----------------------------------------------------*/
          case rtag_tok :
              if ( XTbl.on(TAG) &&
                   XTbl.SCRIPT_tbl[JS_GLOBALTAG] ) {
                 bHandled = TRUE ;
                 XTbl.seton(NONTRANS) ;
                 XTbl.SetStateTable( STATE_JS ) ;
                 XTbl.setoff(JS_GLOBALTAG);
                 XTok.SetTransState( TRANS_STATE_OFF ) ;
                 XTok.AddTagTransOff();
              }
              break ;
       /* --------------------------------------------------*/
       default :
           break ;                         // unknown state
       }
    }

    return( bHandled ) ;
}




/*****************************************************************************/
/*  HandleVBScriptTokens                                                     */
/*                                                                           */
/*  This function will handle the VisualBasic unique tokens.                 */
/*                                                                           */
/*  Return:  TRUE  - Token has been handled.                                 */
/*           FALSE - Token has not yet been handled.                         */
/*****************************************************************************/
inline BOOL HandleVBScriptTokens( short *sTokenType, Token& XTok )
{
    wchar_t  tchar ;
    BOOL     bHandled ;

    if ( ( XTbl.statetype == STATE_XML ) &&    
         ( XTbl.off(GLOBALTAG) ) ) {
       if ( *sTokenType != rtag_tok ) {
          if ( *sTokenType == VBS_globaltag_tok ) {
             *sTokenType = ltag_tok ;
          }
       } else {
          if ( ( XTbl.on(DQUOTE) ) &&
               ( XTbl.on(SQUOTE) ) &&
               ( XTbl.on(HDQUOTE) ) &&
               ( XTbl.on(HSQUOTE) ) ) {
             XTbl.SetStateTable( STATE_VBS ) ;
          }
       }
       return( FALSE ) ; 
    }

    if ( XTbl.XML_tbl[NONTRANS] ) {

       XTbl.SetStateTable( STATE_VBS ) ;
       bHandled = TRUE ;


       switch (*sTokenType) {
       /* --------------------------------------------------*/
       case VBS_equal_tok :
           if ( XTbl.on(VBS_ASSIGN) ) {
              XTbl.seton(VBS_ASSIGN) ;
           }
           break ;
       /* --------------------------------------------------*/
       case VBS_concat_tok :
           if ( XTbl.off(VBS_ASSIGN) ) {
              XTbl.setoff( VBS_ASSIGN1 ) ;
              XTbl.setoff( VBS_ASSIGN2 ) ;
           }
           break ;
       /* --------------------------------------------------*/
       case VBS_lparen_tok :
           if ( XTbl.on(VBS_PAREN ) ) {
              XTbl.seton( VBS_PAREN ) ;
              ++sSCRIPT_ParenCount ;
           }
           break ;
       /* --------------------------------------------------*/
       case VBS_rparen_tok :
           if ( XTbl.off(VBS_PAREN) ) {
              --sSCRIPT_ParenCount ;
              if ( sSCRIPT_ParenCount == 0 ) {
                 XTbl.setoff( VBS_PAREN ) ;
              }
           }
           break ;
       /* --------------------------------------------------*/
       case startnontrans_tok :
           if ( XTbl.on(VBS_COMMENT) &&
                XTbl.on(VBS_NONTRANSBLOCK) ) {
              XTbl.seton(VBS_NONTRANSBLOCK) ;
              XTbl.seton( VBS_COMMENT ) ;
              break ;
           } 
           goto skip_endnontrans ;
       /* -----------------------------------------------------*/
       /* Intentionally left out break to fall into comment.   */
       /* -----------------------------------------------------*/
       case endnontrans_tok :
           if ( XTbl.off(VBS_NONTRANSBLOCK) ) {
              XTbl.setoff(VBS_NONTRANSBLOCK) ;
              XTbl.seton( VBS_COMMENT ) ;
              break;
           } 
       skip_endnontrans:
       /* --------------------------------------------------*/
       case VBS_comment_tok :
           if ( XTbl.on(VBS_COMMENT) ) {
              XTbl.seton( VBS_COMMENT ) ;
              if ( XTbl.on(VBS_ASSIGN2 ) ) {
                 XTbl.seton( VBS_ASSIGN2 ) ;
              }
           }
           break ;
       /* --------------------------------------------------*/
       case dquote_tok :
           if ( XTbl.on(VBS_DQUOTE) ) {
               XTbl.seton(VBS_DQUOTE);
               XTbl.SetStateTable( STATE_XML ) ;
               XTbl.setoff(NONTRANS) ;
               XTok.SetTransState( TRANS_STATE_ON ) ;
               XTok.AddTagTransOn();
           } else if ( XTbl.on(VBS_HDQUOTE) ) {
               XTbl.seton(VBS_HDQUOTE) ;
           } else if ( XTbl.off(VBS_HDQUOTE) ) {
               XTbl.setoff(VBS_HDQUOTE) ;
           }
           break ;
       /* --------------------------------------------------*/
       case JS_globaltag_tok :
           if ( XTbl.on(VBS_GLOBALTAG) ) {
               XTbl.seton(VBS_GLOBALTAG);
               sPrevScriptState = sScriptState ;
               sScriptState = SCRIPT_STATE_NONE ;
               XTbl.SetStateTable( STATE_XML ) ;
               XTbl.seton(GLOBALTAG);
               XTbl.setoff(NONTRANS) ;
               XTok.SetTransState( TRANS_STATE_ON ) ;
               XTok.AdjustLastToken( -1 ) ; 
               XTok.AddTagTransOn();
           }
           break ;
       /* --------------------------------------------------*/
       default :
           break ;                         // unknown state
       }

       if ( XTbl.XML_tbl[NONTRANS] ) {
          tchar = XTok.GetCurrentChar() ;
          if ( XTbl.off(VBS_ASSIGN) ) {
             if ( XTbl.on(VBS_ASSIGN1) ) { /* Look for start of variable */
                if ( iswalpha(tchar) ||
                     ( tchar == '_' ) )
                   XTbl.seton(VBS_ASSIGN1) ;
                else
                   if ( ( ! iswspace(tchar)      ) &&
                        ( ! wcschr(L"&+=",tchar) ) ) {
                      XTbl.setoff(VBS_ASSIGN);
                   }
             } else {
                if ( XTbl.on(VBS_ASSIGN2) ) { /* Look for end of variable */
                   if ( iswspace(tchar) ||
                        ( wcschr(L"&+",tchar) ) )
                      XTbl.seton(VBS_ASSIGN2) ;
                   else
                      if ( !iswalpha(tchar) &&
                           !wcschr(L"_.)",tchar) ) {
                         XTbl.setoff(VBS_ASSIGN1) ;
                         XTbl.setoff(VBS_ASSIGN) ;
                      }
                } else {
                   if ( XTbl.off(VBS_ASSIGN2) &&  /* Already found end of variable */
                        ! XTbl.off(VBS_COMMENT) &&
                        ! XTbl.off(VBS_PAREN) &&
                        ! iswspace(tchar) &&
                        ! wcschr(L"&+",tchar) ) {
                      XTbl.setoff(VBS_ASSIGN2) ;
                      XTbl.setoff(VBS_ASSIGN1) ;
                      XTbl.setoff(VBS_ASSIGN) ;
                   }
                }
             }
          }
       }
    } else {

       bHandled = FALSE ;

       switch (*sTokenType) {
          case dquote_tok :
              if ( XTbl.on(DQUOTE) &&
                   XTbl.SCRIPT_tbl[VBS_DQUOTE] ) {
                 bHandled = TRUE ;
                 XTbl.seton(NONTRANS) ;
                 XTbl.SetStateTable( STATE_VBS ) ;
                 XTbl.setoff(VBS_DQUOTE);
                 XTbl.seton(VBS_ASSIGN);
                 XTbl.seton(VBS_ASSIGN1);
                 XTbl.seton(VBS_ASSIGN2);
                 XTok.SetTransState( TRANS_STATE_OFF ) ;
                 XTok.AddTagTransOff();
              }
              break ;
          /* -----------------------------------------------------*/
          case rtag_tok :
              if ( XTbl.on(TAG) &&
                   XTbl.SCRIPT_tbl[VBS_GLOBALTAG] ) {
                 bHandled = TRUE ;
                 XTbl.seton(NONTRANS) ;
                 XTbl.SetStateTable( STATE_VBS ) ;
                 XTbl.setoff(VBS_GLOBALTAG);
                 XTok.SetTransState( TRANS_STATE_OFF ) ;
                 XTok.AddTagTransOff();
              }
              break ;
          /* --------------------------------------------------*/
       default :
           break ;                         // unknown state
       }
    }

    return( bHandled ) ;
}


/*****************************************************************************/
/*  HandleXHTMLTags                                                          */
/*                                                                           */
/*  Determine how the XHTML <FONT> tag is used.                              */
/*                                                                           */
/*  Return:  TRUE  - String has been modified.                               */
/*           FALSE - String has not been modified. .                         */
/*****************************************************************************/
inline BOOL HandleXHTMLTags(wchar_t *Text )
{

    static wchar_t  TAGLIST_NEUTRAL[150] =
    L" A ABBR ACRONYM B BDO BIG BLINK BUTTON CITE CODE DFN EM I IMG KBD LINK Q S SAMP SMALL SPAN STRIKE STRONG SUB SUP TT U VAR " ;


#define LEAD_FONT_NONE     0
#define LEAD_FONT_TAG      1
#define LEAD_FONT_TEXT     2
#define LEAD_FONT_SKIPTAG  3
#define LEAD_FONT_ENDTAG   4

    wchar_t   *TokenDelimit = L" =\n\r\t";   

    wchar_t   *szTemp ;
    wchar_t   szTagName[40] ;
    wchar_t   szTempTag[42] ;
    wchar_t   *ptrChar, *ptrChar2, *ptrChar3, *ptrTag ;
    ULONG     ulFilePos ;
    short     sLeadFontState ;
    short     sFindState ;
    BOOL      bChanged = FALSE ; 


    szTemp = (wchar_t*)malloc( MAX_BUFFER_SIZE*sizeof(wchar_t)*2 ) ;

    for( ptrChar=Text ; *ptrChar ; ++ptrChar ) {
       if ( iswspace(*ptrChar) )
          continue ;

       if ( sXHTML_TagState > XHTML_TAG_INTAG ) {
          if ( ( sXHTML_TagState != XHTML_TAG_SCRIPT ) &&
               ( sXHTML_TagState != XHTML_TAG_STYLE  ) ) { 
             if ( *ptrChar == '>' ) {
                if ( ( sXHTML_PrevTagState == XHTML_TAG_NORMAL ) &&
                     ( sXHTML_TagState == XHTML_TAG_NEUTRAL    ) )
                   sXHTML_PrevTagState = XHTML_TAG_NORMAL ;
                else
                   sXHTML_PrevTagState = sXHTML_TagState ;
                sXHTML_TagState = XHTML_TAG_NONE ;
             }
             continue ;
          }
       }

       if ( ( *ptrChar == '<' ) &&
            ( ( iswalpha( *(ptrChar+1) ) ) ||
              ( ( *(ptrChar+1) == '/'     ) &&
                ( iswalpha( *(ptrChar+2) ) ) ) ||
                ( *(ptrChar+1) == '!'       ) ) ) {  /* Comment */
          ptrTag = ptrChar ;
          wcsncpy( szTagName, ptrChar+1, sizeof(szTagName)/sizeof(wchar_t) ) ;
          szTagName[sizeof(szTagName)/sizeof(wchar_t)-1] = NULL /*0*/ ;
          wcstok( szTagName, L" >\n\t\r" ) ;
          wcsupr( szTagName ) ;
          if ( ! wcscmp( szTagName, L"SCRIPT" ) ) {
             sXHTML_TagState = XHTML_TAG_SCRIPT ;
          } else
          if ( ! wcscmp( szTagName, L"STYLE" ) ) {
             sXHTML_TagState = XHTML_TAG_STYLE ;
          } else
          if ( ( ! wcscmp( szTagName, L"/SCRIPT" ) ) &&
               ( sXHTML_TagState == XHTML_TAG_SCRIPT ) ) {
             sXHTML_TagState  = XHTML_TAG_NORMAL ;
             sXHTML_FontState = XHTML_FONT_NONE ;
          } else
          if ( ( ! wcscmp( szTagName, L"/STYLE" ) ) &&
               ( sXHTML_TagState == XHTML_TAG_STYLE ) ) {
             sXHTML_TagState  = XHTML_TAG_NORMAL ;
             sXHTML_FontState = XHTML_FONT_NONE ;
          } else 
          if ( ( sXHTML_TagState == XHTML_TAG_SCRIPT ) ||
               ( sXHTML_TagState == XHTML_TAG_STYLE  ) ) {
             continue ;
          } else
          if ( ( ! wcscmp( szTagName, L"FONT" ) ) ||
               ( ( ! wcscmp( szTagName, L"/FONT" ) ) &&  /* Unmatched </FONT> */
                 ( sXHTML_FontState == XHTML_FONT_NONE    ) ) ) {
             sXHTML_TagState = XHTML_TAG_NORMAL ;
             /*--------------------------------------------------------------*/
             /*  A <FONT> tag can be both an inline tag or a tag which       */
             /*  breaks segmentation.  Which type is dependent on its        */
             /*  location and the location of its matching </FONT> tag.      */
             /*  - If the <FONT> is just after a breaking tag and the        */
             /*    </FONT> is just before a breaking tag, then both tags are */
             /*    handled as breaking tags.                                 */
             /*        <p><font>text</font></p>                              */
             /*  - Otherwise, they are handled as inline tags.               */
             /*        <p><font>text</font>text</p>                          */
             /*        <p>text<font>text</font></p>                          */
             /*        <p>text<font>text</font>text</p>                      */
             /*--------------------------------------------------------------*/
             if ( ( sXHTML_PrevTagState == XHTML_TAG_NEUTRAL ) ||   /* Need to find trailing state */
                  ( sXHTML_PrevTagState == XHTML_TAG_TEXT    ) ||
                  ( ( ! wcscmp( szTagName, L"FONT" ) ) &&           
                    ( sXHTML_PrevTagState == XHTML_TAG_NORMAL ) ) ) {
                ulFilePos = (*InputFile).ftellt() ;        /* Save file position   */
                if ( sXHTML_PrevTagState == XHTML_TAG_NORMAL ) 
                   sLeadFontState = LEAD_FONT_SKIPTAG ;
                else
                   sLeadFontState = LEAD_FONT_NONE ;
                ptrChar2 = ptrChar + 1 ;
                while( ptrChar2 ) {
                   /*----------------------------------------------------------*/
                   /*  Find the end of the current tag.                        */
                   /*----------------------------------------------------------*/
                   if ( sLeadFontState == LEAD_FONT_SKIPTAG ) {     
                      for( ; *ptrChar2 && *ptrChar2!='>' ; ++ptrChar2 ) ;
                      if ( *ptrChar2 ) 
                         sLeadFontState = LEAD_FONT_TEXT ;
                   }
                   /*----------------------------------------------------------*/
                   /*  In text, so find start of the next tag.                 */
                   /*----------------------------------------------------------*/
                   if ( sLeadFontState == LEAD_FONT_TEXT ) {        
                      for( ; *ptrChar2 && *ptrChar2!='<' ; ++ptrChar2 ) ;
                      ptrChar3 = ptrChar2 ;
                   } else {
                      /*-------------------------------------------------------*/
                      /*  Find the end of the current tag and then find        */
                      /*  next non-blank character.                            */
                      /*-------------------------------------------------------*/
                      for( ; *ptrChar2 && *ptrChar2!='>' ; ++ptrChar2 ) ;
                      if ( *ptrChar2 )
                         for( ptrChar3=ptrChar2+1; *ptrChar3 && iswspace(*ptrChar3) ; ++ptrChar3 ) ;
                      else
                         ptrChar3 = ptrChar2 ;
                   }
                   /*----------------------------------------------------------*/
                   /*  Get next record to continue process.                    */
                   /*----------------------------------------------------------*/
                   if ( ! *ptrChar3 ) {                    /* Find char after this tag */
                      if ( *ptrChar2 ) 
                         sFindState = 2 ;                  /* Find next non-blank */
                      else
                         sFindState = 1 ;                  /* Find end of tag  */
                      while ( (*InputFile).getline(szTemp, MAX_BUFFER_SIZE, bUTF16) != NULL ) {
                         if ( sFindState == 1 ) 
                            ptrChar2 = szTemp ;
                         /*----------------------------------------------------------*/
                         /*  Find the end of the current tag.                        */
                         /*----------------------------------------------------------*/
                         if ( sLeadFontState == LEAD_FONT_SKIPTAG ) {
                            for(  ; *ptrChar2 && *ptrChar2!='>' ; ++ptrChar2 ) ;
                            if ( ! *ptrChar2 ) 
                               continue ;
                            sLeadFontState = LEAD_FONT_TEXT ;
                         }
                         /*----------------------------------------------------------*/
                         /*  In text, so find start of the next tag.                 */
                         /*----------------------------------------------------------*/
                         if ( sLeadFontState == LEAD_FONT_TEXT ) {
                            for( ; *ptrChar2 && *ptrChar2!='<' ; ++ptrChar2 ) ;
                            if ( ! *ptrChar2 ) 
                               continue ;
                            ptrChar3 = ptrChar2 ;
                            break ;
                         } else {
                            /*-------------------------------------------------------*/
                            /*  Find the end of the current tag and then find        */
                            /*  next non-blank character.                            */
                            /*-------------------------------------------------------*/
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
                   /*----------------------------------------------------------*/
                   /*  If followed by another tag or comment, then see if it   */
                   /*  a neutral tag or not.                                   */
                   /*----------------------------------------------------------*/
                   if ( ( ptrChar3 ) &&                   
                        ( *ptrChar3 == '<' ) &&           
                        ( ( iswalpha( *(ptrChar3+1) ) ) ||
                          ( ( *(ptrChar3+1) == '/'     ) &&
                            ( iswalpha( *(ptrChar3+2) ) ) ) ||
                          ( *(ptrChar3+1) == '!'         ) ) ) {  /* Comment */
                      wcsncpy( szTagName, ptrChar3+1, sizeof(szTagName)/sizeof(wchar_t) ) ;
                      szTagName[sizeof(szTagName)/sizeof(wchar_t)-1] = NULL /*0*/ ;
                      wcstok( szTagName, L" >\n\t\r" ) ;
                      wcsupr( szTagName ) ;
                      if ( szTagName[0] == '/' )
                         swprintf( szTempTag, L" %s ", &szTagName[1] ) ;
                      else
                          swprintf( szTempTag, L" %s ", szTagName ) ;
                      if ( ( wcsstr( TAGLIST_NEUTRAL , szTempTag ) ) || /* Next tag is neutral */
                           ( ! wcscmp( szTempTag, L" FONT "  ) ) ) {    
                         ptrChar2 = ptrChar3 + 1 ; /* Look for next tag */
                         if (  sLeadFontState >= LEAD_FONT_TAG ) {   
                            if ( ! wcscmp( szTagName, L"/FONT"  ) ) 
                               sLeadFontState = LEAD_FONT_ENDTAG ;
                            else
                               if ( sLeadFontState == LEAD_FONT_TEXT ) 
                                  sLeadFontState = LEAD_FONT_SKIPTAG ; 
                               else 
                                  if ( sLeadFontState != LEAD_FONT_ENDTAG )
                                     sLeadFontState = LEAD_FONT_TEXT ;
                         }
                      } else {
                         break ;         /* Non-neutral tag found */
                      }
                   } else {
                      /*----------------------------------------------------------*/
                      /*  If followed by text, then identify if to be neutral.    */
                      /*----------------------------------------------------------*/
                      if ( wcschr( L"\"\'", *ptrChar2 ) )
                         sXHTML_TagState = XHTML_TAG_NONE ;
                      else
                         if ( ( sLeadFontState == LEAD_FONT_NONE   ) ||    
                              ( sLeadFontState == LEAD_FONT_ENDTAG ) )  {
                            sXHTML_TagState = XHTML_TAG_NEUTRAL ; /* Translatable text */
                         }
                      break ;
                   }
                }
               (*InputFile).fseekt( ulFilePos, SEEK_SET ) ;  /* Reset for next read */
          
                if ( sXHTML_TagState == XHTML_TAG_NEUTRAL ) {
                   if ( *(ptrChar+1) == '/' ) {
                      memmove( ptrChar+7, ptrChar+6, (wcslen(ptrChar+6)+1)*sizeof(wchar_t) ) ;
                      *(ptrChar+6) = 'Q' ;
                   } else {
                      memmove( ptrChar+6, ptrChar+5, (wcslen(ptrChar+5)+1)*sizeof(wchar_t) ) ;
                      *(ptrChar+5) = 'Q' ;
                   }
                   sXHTML_FontState = XHTML_FONT_NEUTRAL ;
                   bChanged = TRUE ;
                }
             }
          } else
          if ( ! wcscmp( szTagName, L"/FONT" ) ) {
             if ( sXHTML_FontState == XHTML_FONT_NEUTRAL ) {
                memmove( ptrChar+7, ptrChar+6, (wcslen(ptrChar+6)+1)*sizeof(wchar_t) ) ;
                *(ptrChar+6) = 'Q' ;
                sXHTML_TagState  = XHTML_TAG_NEUTRAL ;
                sXHTML_FontState = XHTML_FONT_NONE ;
                bChanged = TRUE ;
             } else {
                 sXHTML_TagState = XHTML_TAG_NORMAL ;
             }
          } else {
             if ( szTagName[0] == '/' )
                swprintf( szTempTag, L" %s ", &szTagName[1] ) ;
             else
                swprintf( szTempTag, L" %s ", szTagName ) ;
             if ( ( wcsstr( TAGLIST_NEUTRAL , szTempTag ) ) || /* Next tag is neutral */
                  ( ! wcscmp( szTempTag, L" FONT "  ) ) ) {     
                sXHTML_TagState = XHTML_TAG_NEUTRAL ;
             } else {
                sXHTML_TagState  = XHTML_TAG_NORMAL ;
                sXHTML_FontState = XHTML_FONT_NONE ;
             }
          }
          continue ;
       }

       if ( ! wcsncmp( ptrChar, L"<!--", 4 ) ) {
          bXHTML_InCommentCheck = TRUE ; 
       }
       if ( ! wcsncmp( ptrChar, L"-->", 3 ) ) {
          sXHTML_TagState = XHTML_TAG_NONE ;
          sXHTML_PrevTagState = XHTML_TAG_NORMAL ;
          if ( bXHTML_InCommentCheck ) {         /* Change '--->' to '- -->' */
             bXHTML_InCommentCheck = FALSE ;                    
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
             sXHTML_PrevTagState = XHTML_TAG_TEXT ;  /* Translatable text */
          }
       }
    }

    if ( szTemp ) free( szTemp ) ;
    return( bChanged ) ;
}



/*****************************************************************************/
/*  ConvertEOL                                                               */
/*                                                                           */
/*  Convert file to Windows end-of-line conventions.                         */
/*                                                                           */
/*  Return:  TRUE  - String has been modified.                               */
/*           FALSE - String has not been modified. .                         */
/*****************************************************************************/
inline BOOL ConvertEOL( PSZ in, PSZ out, wchar_t *Type )
{
    wifstream InFile ( in ) ;
    wofstream OutFile ( out ) ;

    wchar_t   *szInput ;
    wchar_t   *ptrRcd ;
    wchar_t   *ptrChar, *ptrChar2 ;
    wchar_t   szEOL[10] ;
    IString   s ;

    BOOL      b0D = FALSE ; 
    BOOL      b0A = FALSE ; 
    BOOL      b0D0A = FALSE ; 

    BOOL      bReturn = FALSE ; 

    Type[0] = NULL ;
    szInput = (wchar_t*)malloc( MAX_BUFFER_SIZE*sizeof(wchar_t)*2 ) ;
    ptrRcd = InFile.get(szInput, MAX_BUFFER_SIZE, bUTF16, TRUE) ;
    if ( ptrRcd != NULL ) {
       ptrChar = wcschr( szInput, L'\r');      
       ptrChar2 = wcschr(szInput, L'\n'); 
       if ( ( ptrChar == NULL ) ||
            ( ( ptrChar2 != NULL ) &&
              ( ptrChar2 < ptrChar ) ) ) {
          ptrChar = ptrChar2 ;
       }
       if ( ptrChar != NULL ) {
          for( ; *ptrChar && wcschr(L"\r\n",*ptrChar) ; ++ptrChar ) {
             if ( *ptrChar == L'\r' ) {
                if ( ! wcschr( Type, L'\r' ) )
                   wcscat( Type, L"D" ) ;
                else 
                   break ;
             } else {
                if ( *ptrChar == L'\n' ) {
                   if ( ! wcschr( Type, L'\n' ) )
                      wcscat( Type, L"A" ) ;
                   else 
                      break ;
                }
             }
          }
       }
    }
    if ( ! wcscmp( Type, L"A" ) ) {
       b0A = TRUE ;
    } else {
       if ( ! wcscmp( Type, L"D" ) ) {
          b0D = TRUE ;
       } else {
          b0D0A = TRUE ;
       }
    }


    if ( ! b0D0A ) {
       bReturn = TRUE ; 
       while ( ptrRcd != NULL ) {
             szEOL[0] = NULL ;
          s = szInput ;
          if ( b0A ) {
             s.change(L"\n", L"\r\n") ;
          } else {
             s.change(L"\r", L"\r\n") ;
          } 
          OutFile << s << szEOL ;
          ptrRcd = InFile.get(szInput, MAX_BUFFER_SIZE, bUTF16, TRUE) ;
       }
    }


    OutFile.close();
    InFile.close();

    return( bReturn ) ;
}



/*****************************************************************************/
/*  SplitLongSegments                                                        */
/*                                                                           */
/*  This function will split long segments into smaller pieces.              */
/*                                                                           */
/*  Return:  None.                                                           */
/*****************************************************************************/
inline void SplitLongSegments( IString& s )
{
   INT        i ;
   SHORT      sSegStart = 1 ;
   SHORT      sSegEnd = 0 ; 
   BOOL       bForever = TRUE ;



//  This splitting has to be done after the file has been segmented,
//  because sentences in the 1600-character string will not be known 
//  during pre-seg.

   for( i=1 ; s[i]!=NULL ; ++i ) {
      ++sLongSegLen ; 
      if ( bLongSegTrans ) {
         if ( ( s[i] == L'<' ) &&
              ( ! wcsncmp( s.subString(i,6), L"<twb--", 6 ) ) ) {
            bLongSegTrans = FALSE ; 
            sSegStart = i ;
            sSegEnd = 0 ;
            sLongSegLen = 1 ;
            continue ;
         }
      } else {
         if ( ( s[i] == L'-' ) &&
              ( ! wcsncmp( s.subString(i,6), L"--twb>", 6 ) ) ) {
            bLongSegTrans = TRUE ; 
            i += 6 ;
            sSegStart = i ;
            sSegEnd = 0 ;
            sLongSegLen = 1 ;
            continue ;
         }
      }

      if ( sLongSegLen > 1600 ) {
         if ( ( s[i] == L'<'  ) || 
              ( s[i] == L'>'  ) ) {
            if ( s[i] == L'<' ) 
               --i ;
            if ( bLongSegTrans ) 
               s.insert( L"<twb----twb>", i ) ; 
            else 
               s.insert( L"--twb><twb--", i ) ; 
            i += 12 ;
            sSegStart = i ;
            sSegEnd = 0 ;
            sLongSegLen = 1 ;
         }
      }
   }

   return; 
}



/*****************************************************************************/
/*  FindAltSplit                                                             */
/*                                                                           */
/*  Find alternate location to split a translatable segment.                 */
/*                                                                           */
/*  Return:  TRUE  - Alternate location found.                               */
/*           FALSE - No alternate location found.                            */
/*****************************************************************************/
inline BOOL FindAltSplit( wchar_t **Start, wchar_t *End, 
                          wchar_t *TagQF, wchar_t *TagEQF, short incr ) 
{
   wchar_t   *ptrChar ;
   BOOL      bBackward ;
   BOOL      bFound = FALSE ; 

   if ( incr > 0 ) 
      bBackward = FALSE ;
   else
      bBackward = TRUE ; 

   for( ptrChar=*Start+incr ;
        ( *ptrChar!=NULL ) &&
        ( ( ! bBackward ) ||
          ( ptrChar>End ) ) ;
        ptrChar+=incr ) {
      if ( ( *ptrChar == L'<' ) &&
           ( wcsncmp( ptrChar, L"<twb--", 6 ) ) &&
           ( wcsncmp( ptrChar, L"<twb##", 6 ) ) ) {
         bFound = TRUE ;
         break ;
      }
      if ( ( *ptrChar == L'>' ) &&
           ( ptrChar-4 >= *Start ) &&
           ( wcsncmp( ptrChar-4, L"-twb>", 5 ) ) &&    
           ( wcsncmp( ptrChar-4, L"#twb>", 5 ) ) ) {
         bFound = TRUE ;
         break ;
      }
      if ( ( *ptrChar == L':' ) &&
           ( ( ( bBackward ) &&
               ( ! wcsncmp( ptrChar, TagQF, 3 ) ) ) ||
             ( ( ! bBackward ) &&
               ( ! wcsncmp( ptrChar, TagEQF, 4 ) ) ) ) ) {
         bFound = TRUE ;
         break;
      }
   }

   *Start = ptrChar ;

   return( bFound ) ;
}
