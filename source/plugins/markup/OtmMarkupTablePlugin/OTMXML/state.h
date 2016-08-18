/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/****************************************************************************/
/*                                                                          */
/* STATE.H                                                                  */
/*                                                                          */
/* I do this to hide the complexity of                                      */
/* Changing States.                                                         */
/* Not Sure if there is an easier way to do this ...                        */
/*                                                                          */
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#ifndef _STATE_H_INCLUDE_
#define _STATE_H_INCLUDE_

#define INCL_BASE
#define INCL_DOSFILEMGR                 /* File Manager values               */

#include "unicode.h"


using namespace mku ;


#define XML_TAG_LENGTH           80
#define XML_ATTR_LENGTH          8192
#define XML_NAMES_LENGTH         1024
#define XML_ATTR_VALUE_LENGTH    8192
#define XML_COMPLETE_TAG_LENGTH  8192    /* 8-23-10 */

#define STATE_XML                1       // Use state table for normal XML
#define STATE_JS                 2       // Use state table for JavaScript
#define STATE_VBS                3       // Use state table for VisualBasic Script

#define TRANS_STATE_NONE         0
#define TRANS_STATE_ON           1
#define TRANS_STATE_OFF          2
#define TRANS_STATE_ATTR_CHECK   3

#define SCRIPT_STATE_NONE        0       // Not in a SCRIPT section
#define SCRIPT_STATE_TAG_UNKNOWN 1       // Within SCRIPT tag, but no type yet
#define SCRIPT_STATE_TAG_JS      2       // Within SCRIPT tag, found type=JavaScript
#define SCRIPT_STATE_TAG_VBS     3       // Within SCRIPT tag, found type=VisualBasic
#define SCRIPT_STATE_OTHER       4       // Within SCRIPT section, type is unknown
#define SCRIPT_STATE_JAVASCRIPT  5       // Within SCRIPT section, type is JavaScript
#define SCRIPT_STATE_VBSCRIPT    6       // Within SCRIPT section, type is VisualBasic

#define LINES_STATE_OFF          0       // Within sentence segmentation
#define LINES_STATE_ON           1       // Within line-by-line segmentation section

#define COPYTEXT_STATE_OFF          0       // Not found a copy text tag yet
#define COPYTEXT_STATE_FROM_START1  1       // Found 'from start' tag, look for tag end
#define COPYTEXT_STATE_FROM_START2  2       // Found 'from start' tag end, look for 'start from end' tag
#define COPYTEXT_STATE_FROM_END     3       // Found 'from' text, look for 'to start' tag
#define COPYTEXT_STATE_TO_START1    4       // Found 'to start' tag, look for tag end
#define COPYTEXT_STATE_TO_START2    5       // Found 'to start' tag end, copy from text
#define COPYTEXT_STATE_TO_START3    6       // Found 'to start' tag, look for tag end
#define COPYTEXT_STATE_IGNORE       7       // Found tag inside of trans-unit to ignore 

#define INSERTATTR_OFF              0       // Not found an insert attribute tag yet
#define INSERTATTR_TAG_FOUND        1       // Found tag to insert an attribute into
#define INSERTATTR_COND_FOUND       2       // Found conditional attribute, look for value
#define INSERTATTR_SRC_FOUND        3       // Found attribute to copy from
#define INSERTATTR_TGT_FOUND        4       // Not used
#define INSERTATTR_COPIED           5       // Inserted new attribute

#define IGNORECOMMENT_STATE_OFF     0        // Not in section for ignoring comment delimiters
#define IGNORECOMMENT_STATE_ON      1        // In section for ignoring comment delimiters
#define IGNORECOMMENT_STATE_START   2        // Looking for end of comment delimiters to be ignored

typedef struct                               // Structure to define translatable attributes
{
   wchar_t   Tag[XML_TAG_LENGTH] ;           // (1) Tag name for these attributes
   wchar_t   NotTag[XML_TAG_LENGTH] ;        // (1) When Tag='*', then tag name to ignore
   wchar_t   Attr[XML_NAMES_LENGTH] ;        // (1+) Attributes which have translatable text
   wchar_t   CondAttr[XML_ATTR_LENGTH] ;     // (1) Conditional attr. as to whether attr. is translatable
   wchar_t   CondAttrValue[XML_NAMES_LENGTH] ; // (1+) Conditional attr. values which make attr translatable
   wchar_t   SkipAttrValue[XML_NAMES_LENGTH] ; // (1+) Attribute values to skip
   VOID      *ptrNext ;                      // Pointer to next item in linked list
} ATTRTAG ;

typedef struct                               // Structure to define copy text tags
{
   wchar_t   FromTag[XML_TAG_LENGTH] ;       // (1) Tag name to copy content text from
   wchar_t   ToTag[XML_TAG_LENGTH] ;         // (1) Tag name to copy content text to 
   wchar_t   ParentTag[XML_TAG_LENGTH] ;     // (1) Tag name to copy content text to  
   wchar_t   *TextBefore ;                   // Text to add before inserted text 
   wchar_t   *TextAfter ;                    // Text to add after  inserted text  
   VOID      *ptrNext ;                      // Pointer to next item in linked list
} COPYTEXTTAG ;

typedef struct
{
	wchar_t  Tag[XML_TAG_LENGTH];            // (1) Tag name to insert attribute
	wchar_t  CopyAttr[XML_NAMES_LENGTH];     // Attribute text to be copied
	wchar_t  CondAttr[XML_NAMES_LENGTH];     // (1+) Copy attribute only if tag contains this attribute 
    wchar_t  CondAttrValue[XML_NAMES_LENGTH] ; // (1+) Conditional attr. values for when to insert attribute
    BOOL     Remove ;                        // TRUE=Remove attr. before line is written
    VOID     *ptrNext ;                      // Pointer to next item in linked list
}
INSERTATTRTAG;

typedef struct                               // Structure to define global tags
{
   wchar_t   Tag[XML_TAG_LENGTH] ;              
   BOOL      Generic ;                       // TRUE=Partial tag name is defined
   VOID      *ptrNext ;                      // Pointer to next item in linked list 
} GLOBALTAG ;

typedef struct                               // Structure to define line rather than sentence segmentation
{
   wchar_t   Tag[XML_TAG_LENGTH] ;           // (1) Tag name which starts this section
   wchar_t   EndTag[XML_TAG_LENGTH] ;        // (1) Tag name which ends this section
   wchar_t   CondAttr[XML_ATTR_LENGTH] ;     // (1) Attribute which starts lines state change
   wchar_t   CondAttrValue[XML_NAMES_LENGTH] ;// (1) Attribute value which starts lines state change
   VOID      *ptrNext ;                      // Pointer to next item in linked list 
} LINESTAG ;

typedef struct                               // Structure to define neutral/imbedded tags
{
   wchar_t   Tag[XML_TAG_LENGTH] ;           // (1) Tag name which is a neutral tag
   BOOL      Break ;                         // TRUE=Break segmentation if after end of sentence
   BOOL      Generic ;                       // TRUE=Partial tag name is defined
   VOID      *ptrNext ;                      // Pointer to next item in linked list 
} NEUTTAG ;

typedef struct                               // Structure to define changing translation state by attribute value
{
   wchar_t   CondAttr[XML_ATTR_LENGTH] ;      // (1) Attribute which starts translation state change
   wchar_t   CondAttrValue[XML_NAMES_LENGTH] ; // (1+) Attribute value which starts translation state change
   short     Initial ;                       // Translation state for text following this tag
   VOID      *ptrNext ;                      // Pointer to next item in linked list 
} STATEATTR ;

typedef struct                               // Structure to define tag nesting translation state changes
{
   wchar_t   EndTag[XML_TAG_LENGTH] ;        // (1) Tag which ends translation state change
   short     State ;                         // State to return to after end found     
   BOOL      Neutral ;                       // Tag type to return to after end found     
   VOID      *ptrNext ;                      // Pointer to next item in linked list 
} STATEATTRLVL ;

typedef struct                               // Structure to define changing translation state
{
   wchar_t   Tag[XML_TAG_LENGTH] ;           // (1) Tag name which starts this state change
   wchar_t   EndTag[XML_TAG_LENGTH] ;        // (1) Tag name which ends this state change
   wchar_t   CondAttr[XML_NAMES_LENGTH] ;    // (1+) Conditional attr. as to whether this tag starts state change
   wchar_t   CondAttrValue[XML_NAMES_LENGTH] ; // (1+) Conditional attr. value which starts state change
   wchar_t   PrevTag[XML_TAG_LENGTH] ;       // (1) Previous tag name for state change to be valid.
   BOOL      TransAttrOnly ;                 // State change only to allow translatable attribute values
   short     Initial ;                       // Initial translation state when starting new state
   short     CondState ;                     // 1=Only in TRANS state, 2=Only in non-trans state, 0=Any state
   BOOL      Generic ;                       // TRUE=Partial tag name is defined
   VOID      *ptrNext ;                      // Pointer to next item in linked list 
} STATETAG ;




const int XML_numstates = 26 ;       //  number of states + 1

const int SCRIPT_numstates = 15 ;    //  Maximum number of SCRIPT states + 1
const int JS_numstates = 15 ;        //  Maximum number of JAVASCRIPT states + 1 
const int VBS_numstates = 10 ;       //  Maximum number of VISUAL BASIC states + 1 


class XmlState {
public :
    XmlState() { initializeStates();};
    ~XmlState() {} ;
    enum {
        comment,           // Within a comment area
        tag,               // Within an XML tag
        doctag,            // Within a <!DOCTYPE> XML statement
        entity,            // Within a <!ENTITY> XML statement
        cdata,             // Within a <!CDATA> XML statement
        squote,            // Within a single quoted area (translatable text)
        dquote,            // Within a double quoted area (translatable text)
        hsquote,           // Within a single quoted area (non-translatable text, hidden)
        hdquote,           // Within a double quoted area (non-translatable text, hidden)
        system,            // For ENTITY state, found SYSTEM keyword
        hcomment,          // For ENTITY state, within a comment area
        xmldoc,            // Within a <?> XML statement
        xmlname,           // For XMLDOC state, found XML name
        encoding,          // For XMLDOC state, found ENCODING attribute
        xmllang,           // For TAG state, found XMLLANG attribute
        neuttag,           // For TAG state, tag is considered neutral/imbedded
        attrtag,           // For TAG state, tag may contain a translatable attribute
        condattr,          // For ATTRTAG state, found conditional attribute allowing TRANSATTR state to be set
        transattr,         // For ATTRTAG state, found a tag attribute whose value is translatable
        dtdname,           // For DOCTAG state, found DTD reference
        nontranstag,       // Within a tag which starts a non-translatable block section of XML
        nontrans,          // Within a non-translatable block section of XML
        globaltag,         // Within JavaScript/VBScript global tag                     
        nontransblock      // Within start/end nontranslatable block
    } ;

    enum {
        JS_lcomment,       // JavaScript. Within a line comment area
        JS_bcomment,       // JavaScript. Within a block comment area
        JS_squote,         // JavaScript. Within a single quoted area (translatable text)
        JS_dquote,         // JavaScript. Within a double quoted area (translatable text)
        JS_hsquote,        // JavaScript. Within a single quoted area (non-translatable text, hidden)
        JS_hdquote,        // JavaScript. Within a double quoted area (non-translatable text, hidden)
        JS_paren,          // JavaScript. Within a parenthesis area
        JS_assign,         // JavaScript. Within an assignment statement
        JS_assign1,        // JavaScript. Within an assignment statement, start of variable
        JS_assign2,        // JavaScript. Within an assignment statement, end of variable
        JS_globaltag,      // JavaScript. Within a global tag (handle as if not in SCRIPT section)
        JS_array,          // JavaScript. Within an array assignment statement
        JS_nontransblock,  // JavaScript. Within start/end nontranslatable block
    } ;

    enum {
        VBS_comment,       // Visual Basic. Within a line comment area
        VBS_dquote,        // Visual Basic. Within a double quoted area (translatable text)
        VBS_hdquote,       // Visual Basic. Within a double quoted area (non-translatable text, hidden)
        VBS_paren,         // Visual Basic. Within a parenthesis area
        VBS_assign,        // Visual Basic. Within an assignment statement
        VBS_assign1,       // Visual Basic. Within an assignment statement, start of variable
        VBS_assign2,       // Visual Basic. Within an assignment statement, end of variable
        VBS_globaltag,     // Visual Basic. Within a global tag (handle as if not in SCRIPT section)
        VBS_nontransblock  // Visual Basic. Within start/end nontranslatable block
    } ;

                           // peek to see if we can turn state off
    BOOL off(short state) ;

                           // peek to see if we can turn state on
    BOOL on(short state) ;

                           // see if state is off
    BOOL IsOff(short state) ;

                           // see if state is on
    BOOL IsOn(short state) ;

                           // test to see if all states are false but the ones listed
    BOOL AllFalseBut(short count, ...) ;

                           // test to see if all listed states are true/false (positive=true, negative=false)
    BOOL CheckStates(short count, ...) ;

                           // change the state values
    void setoff(short state) ;
    void seton(short state) ;

                           // peek when changing state for JavaScript
    BOOL JS_off(short state, short* rval) ;
    BOOL JS_on(short state, short* rval) ;
                           // peek when changing state for VisualBasic Script
    BOOL VBS_off(short state, short* rval) ;
    BOOL VBS_on(short state, short* rval) ;

                           // we define these static states, used as offsets to the array
    BOOL XML_tbl[XML_numstates] ;
    BOOL SCRIPT_tbl[SCRIPT_numstates] ;

    int numstates ;
    int statetype ;
    BOOL *tbl ;

    void XmlState::initializeStates(void) {

       initializeState(STATE_JS);
       initializeState(STATE_VBS);
       initializeState(STATE_XML);  /* Must be done last */
    }

    void XmlState::initializeState( int state) {
       int i ;

       switch( state ) {
       case STATE_XML:
          statetype = 0 ;              /* Force state table change */
          SetStateTable( STATE_XML ) ;
          for ( i = 0 ; i < XML_numstates ; i++ )
             XML_tbl[i] = FALSE;
          break ;
       case STATE_JS:
          SetStateTable( STATE_JS ) ;
          for ( i = 0 ; i < JS_numstates ; i++ )
             SCRIPT_tbl[i] = FALSE;
          break ;
       case STATE_VBS:
          SetStateTable( STATE_VBS ) ;
          for ( i = 0 ; i < VBS_numstates ; i++ )
             SCRIPT_tbl[i] = FALSE;
          break ;
       default :
           break ;
       }
    };

    void XmlState::SetStateTable(int state) {
       if ( state != statetype ) {
          switch( state ) {
          case STATE_XML:
              numstates = XML_numstates ;
              statetype = STATE_XML ;
              tbl = XML_tbl ;
              break ;
          case STATE_JS:
              numstates = JS_numstates ;
              statetype = STATE_JS ;
              tbl = SCRIPT_tbl ;
              break ;
          case STATE_VBS:
              numstates = VBS_numstates ;
              statetype = STATE_VBS ;
              tbl = SCRIPT_tbl ;
              break ;
          default :
              break ;
          }
       }
    };

private :
    // we define the area of states, will be set to true or false via on() & off()
} ;

enum {
    sys_tok,               // 0   Found SYSTEM keyword (SYSTEM)
    rdoc_tok,              // 1   Found end of DOCTYPE statement delimiter (]>)
    ldoc_tok,              // 2   Found start of DOCTYPE statement delimiter (<!DOCTYPE)
    entity_tok,            // 3   Found start of ENTITY statement delimiter (<!ENTITY)
    l_cdata_tok,           // 4   Found start of CDATA statement delimiter (<![CDATA)
    r_cdata_tok,           // 5   Found end of CDATA statement delimiter (]]>)
    l_comment_tok,         // 6   Found start of comment area delimiter (<!-)
    r_comment_tok,         // 7   Found end of comment area delimiter (-->)
    dquote_tok,            // 8   Found double quote (")
    squote_tok,            // 9   Found single quote (')
    ltag_tok,              // 10  Found start of tag delimiter (<)
    rtag_tok,              // 11  Found end of tag delimiter (>)
    rtagempty_tok,         // 12  Found end of tag delimiter for an empty tag (/>)
    eof_tok,               // 13  Found end-of-file
    eol_tok,               // 14  Found end-of-line
    lxml_tok,              // 15  Found start of XML statement delimiter (<?)
    rxml_tok,              // 16  Found end of XML statement delimiter (?>)
    lxmlneut_tok,          // 17  Found start of processing instruction(<?xx?>)
    xml_tok,               // 18  Found XML keyword (XML)
    encode_tok,            // 19  Found ENCODING keyword (ENCODING)
    xmllang_tok,           // 20  Found XML:LANG keyword (XML:LANG)
    neuttag_tok,           // 21  Found tag which is considered neutral/imbedded
    attrtag_tok,           // 22  Found tag which may contain a translatable attribute
    neutattrtag_tok,       // 23  Found tag which is considered neutral and has a translatable attribute
    condattr_tok,          // 24  Found conditional attribute which allows a translatable attribute
    transattr_tok,         // 25  Found translatable attribute keyword within this tag
    transstatetag_tok,     // 26  Found XML tag which reverses the current translation state
    transstateattrtag_tok, // 27  Found XML tag which reverses the current trans state and has trans. attr.
    linestag_tok,          // 28  Found XML tag which starts line rather than sentence segmentation
    dtd_tok,               // 29  Found reference to DTD name used in this XML file
    stateattr_tok,         // 30  Found state attribute keyword within this tag
    startnontrans_tok,     // 31  Found start of non-traslatable section          
    endnontrans_tok,       // 32  Found end of non-translatable section
    period_tok,            // 33  Found period which maybe should not be sentence delimiter in attribute value

    JS_equal_tok,          // 37  JavaScript. Found assignment delimiter (=)
    JS_plus_tok,           // 38  JavaScript. Found assignment concatenation delimiter (+)
    JS_lparen_tok,         // 39  JavaScript. Found left parenthesis (()
    JS_rparen_tok,         // 40  JavaScript. Found right parenthesis ())
    JS_lb_comment_tok,     // 41  JavaScript. Found left  block comment delimiter (/*)
    JS_rb_comment_tok,     // 42  JavaScript. Found right block comment delimiter (*/)
    JS_l_comment_tok,      // 43  JavaScript. Found line comment delimiter (//)
    JS_assignvar_tok,      // 44  JavaScript. Found JavaScript variable char (A-Z 0-9 _ .)
    JS_globaltag_tok,      // 45  JavaScript. Found global tag imbedded in this section.       
    JS_lbracket_tok,       // 46  JavaScript. Found left bracket for array assignment.
    JS_rbracket_tok,       // 47  JavaScript. Found right bracket for array.
    JS_comma_tok,          // 48  JavaScript. Found comma within an array.

    VBS_equal_tok,         // 53  Visual Basic. Found assignment delimiter (=)
    VBS_concat_tok,        // 54  Visual Basic. Found assignment concatenation delimiter (& +)
    VBS_lparen_tok,        // 55  Visual Basic. Found left parenthesis (()
    VBS_rparen_tok,        // 56  Visual Basic. Found right parenthesis ())
    VBS_comment_tok,       // 57  Visual Basic. Found line comment delimiter (')
    VBS_assignvar_tok,     // 58  Visual Basic. Found JavaScript variable char (A-Z 0-9 _ .)
    VBS_globaltag_tok,     // 59  Visual Basic. Found global tag imbedded in this section.       
    
	HTML_attribute_equal_tok, // 60 XHTML. Equal sign after attribute in  <input type=...>, its value is not surrounded by quotes
    starttrans_tok,        // 61  Found start of traslatable section          
    endtrans_tok           // 62  Found end of translatable section

} ;

static wchar_t* delims = L"<>\"\'-]";



class Token {
public :
    Token() { initializeToken(); } ;
    short GetNextToken(void) ;
    void SetTrans(void) ;
    void SetString(IString & ) ;
    void SetDTDInfo(IString &, BOOL ) ;
    void FreeDTDInfo(void) ;
    void SaveCurrentTagName(void) ;
    void SetCurrentTagName( wchar_t * );
    wchar_t* GetCurrentTagName(void) ;
    void SaveCurrentAttrName(void) ;
    void ResetCurrentAttrName(void) ;
    wchar_t* GetCurrentAttrName(void) ;
    void AddTagTransOff(void) ;
    void AddTagTransOn(void) ;
    void ForceTransOn(void) ;
    void AddNeutAttrOff(int) ;
    void AddNeutAttrOn(void) ;
    void AddTagTextOff(void) ;
    void AddTagTextOn(void) ;
    void AddNeutTextOff(void) ;
    void AddNeutTextOn(void) ;
    void ManageLevelTransStateAttr( BOOL *, BOOL ) ;
    BOOL CheckCondAttr(void) ;
    void CheckNonTrans2K( int, int * ) ;
    BOOL CheckTransStateAttr(void) ;
    BOOL CheckSkipAttrValue(void) ;
    BOOL CheckWebSphereAttrValue(void) ;
    BOOL CheckMozillaRDFAttrValue(void) ;
    BOOL CheckWebSphereModelerValue( short ) ;
    BOOL CheckIConsECMText( int ) ;
    void SetWelcomeTransAttr( wchar_t *, wchar_t * ) ;
    BOOL SetCHKPIIXliffTargetLang(void) ;
    void HandleBidiAttr(void) ;
    IString& GetString(void) ;
    IString& DebugString(void) ;
    BOOL IsNeutralTag( wchar_t * ) ;
    BOOL IsTransAttrHasCondAttr(wchar_t *szText);
    BOOL IsAttributeTag( wchar_t * ) ;
    BOOL IsAttribute( wchar_t *, int *, wchar_t, wchar_t * ) ;
    BOOL IsTransStateTag( wchar_t * ) ;
    BOOL IsGlobalTag( wchar_t * ) ;
    BOOL IsIgnoreCommentTag( wchar_t * ) ;
    BOOL IsLinesTag( wchar_t * ) ;
    BOOL IsCopyTextTag( wchar_t * ) ;
    void HandleCopyTextText( wchar_t *, wchar_t ) ;
    int  HandleInsertAttribute(wchar_t *);
    void RemoveInsertAttribute(void) ;
    int  GetTransState(void) ;
    int  GetTransStateAttr(void) ;
    int  GetScriptState(void) ;
    wchar_t GetCurrentChar(void) ;
    void SetTransState( short ) ;
    void SetTransStateAttr( short ) ;
    void SetScriptState( short ) ;
    void AddLineBreak(void) ;
    void AdjustLastToken( short ) ;
    void SetNeutralBreakOn(void) ;
    void SetNeutralBreakOff(void) ;
    BOOL GetTagAttr( wchar_t *, wchar_t *, wchar_t * ) ; 
    BOOL HandleXmlLang( wchar_t *, short, IString &, int, int ) ; 
    void FindCondAttribute( wchar_t *, wchar_t, wchar_t * ) ; 
    void FindTransStateAttribute( wchar_t * ) ; 
    void GetCompleteTag(wchar_t *, long, long );
    void ReplacePeriod(void) ;
    BOOL IsDBCSSentenceEnd( char *, short ) ;
    BOOL DBCSCheckChar( char *, short ) ;

    void Token::initializeToken(void) {
       index = 1 ;
       Length = 0 ;
       lasttoken = 0 ;
       sCopyTextState = COPYTEXT_STATE_OFF ;
	   bCopyXmlLang   = TRUE;
	   bCopySourceLang   = FALSE ;
	   bStateTransAttrOnly = FALSE ; 
	   bNonTransAttrValue = FALSE ; 
	   nInsertAttrTag = 0;
       sTransState = TRANS_STATE_ON ;
       sTransStateAttr = TRANS_STATE_NONE ;
       sSaveTransState = TRANS_STATE_NONE ;    
       sPrevTransStateAttr = TRANS_STATE_NONE ;
       bNeutralBreak = FALSE ;
       bPrevNeutralBreak = FALSE ;
       bScriptSkipHTMLComments = FALSE ;
       trsoff = FALSE ;
       trsoffidx = 0 ;
       cCurrentChar = 0 ;
       bCurrentTagEnd = FALSE ;
       memset( szCurrentTagName, 0, sizeof(szCurrentTagName) ) ;
       memset( szCurrentAttrName, 0, sizeof(szCurrentAttrName) ) ;
       memset( szLinesEndTag, 0, sizeof(szLinesEndTag) ) ;
       memset( szIgnoreCommentEndTag, 0, sizeof(szIgnoreCommentEndTag) ) ;
       memset( szTransStateEndTag, 0, sizeof(szTransStateEndTag) ) ;
       memset( szAttrNames, 0, sizeof(szAttrNames) ) ;
       memset( szCondAttrNames, 0, sizeof(szCondAttrNames) ) ;
       memset( szCondAttrValues, 0, sizeof(szCondAttrValues) ) ;
       memset( szSkipAttrValues, 0, sizeof(szSkipAttrValues) ) ;
       memset( szStateAttrNames, 0, sizeof(szStateAttrNames) ) ;
       memset( szStateAttrValues, 0, sizeof(szStateAttrValues) ) ;
       memset( szInsertCopyAttr, 0, sizeof(szInsertCopyAttr) ) ;
       memset( szInsertCondAttr, 0, sizeof(szInsertCondAttr) ) ;
       memset( szInsertCondAttrValues, 0, sizeof(szInsertCondAttrValues) ) ;
       ptrAttributeTagHead = NULL ;
       ptrCopyTextTagHead = NULL ;
       ptrCopyTextTagHeadSave = NULL ;
	   ptrInsertAttrTagHead = NULL ;
       ptrCopyTextTagCur = NULL ;
       ptrGlobalTagHead = NULL ;
       ptrIgnoreCommentTagHead = NULL ;
       ptrLinesTagHead = NULL ;
       ptrNeutralTagHead = NULL ;
       ptrStateAttrHead = NULL ;
       ptrStateAttrLevelHead = NULL ;
       ptrStateTagHead = NULL ;
       ptrStateTagHeadSave = NULL ;
       ptrSaveCopyText = NULL ;
       ptrSaveCopyText2 = NULL ;
    };
    BOOL     trsoff ;
    long     trsoffidx ;
    int      nInsertAttrTag;
    BOOL     bCurrentTagEnd;

private :
    BOOL     GetLine(void) ;

    std::ofstream       InFile ;
    std::ifstream       Infile ;
    IString        str ;
    BOOL           bNeutralBreak ;
    BOOL           bPrevNeutralBreak ;
    BOOL           bCopyXmlLang;
    BOOL           bCopySourceLang;        
    BOOL           bStateTransAttrOnly;
    BOOL           bNonTransAttrValue ;    
    BOOL           bScriptSkipHTMLComments ; 
    int            index ;
    int            Length ;
    int            lasttoken ;
    int            sCopyTextState ;
    int            sTransState ;
    int            sTransStateAttr ;
    int            sSaveTransState ;      
    int            sPrevTransStateAttr ;
    long           lCopyTextIndex ;
    int            lCopyTextIndex2 ;
    long           lCopyTextSrcIdx ;
    ATTRTAG        *ptrAttributeTagHead ;
    COPYTEXTTAG    *ptrCopyTextTagHead ;
    COPYTEXTTAG    *ptrCopyTextTagHeadSave ;
    COPYTEXTTAG    *ptrCopyTextTagCur ;
	INSERTATTRTAG  *ptrInsertAttrTagHead ;
	INSERTATTRTAG  *ptrInsertAttrTagCur ;
    GLOBALTAG      *ptrGlobalTagHead ;
    LINESTAG       *ptrLinesTagHead ;
    LINESTAG       *ptrIgnoreCommentTagHead ;
    NEUTTAG        *ptrNeutralTagHead ;
    STATEATTR      *ptrStateAttrHead ;
    STATEATTRLVL   *ptrStateAttrLevelHead ;
    STATETAG       *ptrStateTagHead ;
    STATETAG       *ptrStateTagHeadSave ;
    wchar_t        cCurrentChar ;
    wchar_t        *ptrSaveCopyText ;
    wchar_t        *ptrSaveCopyText2 ;
    wchar_t        szContext[XML_NAMES_LENGTH] ;            
    wchar_t        szCurrentTagName[XML_TAG_LENGTH] ;       
    wchar_t        szCurrentAttrName[XML_ATTR_LENGTH] ;   
    wchar_t        szLinesEndTag[XML_TAG_LENGTH] ;         
    wchar_t        szIgnoreCommentEndTag[XML_TAG_LENGTH] ;         
    wchar_t        szTransStateEndTag[XML_TAG_LENGTH] ;
    wchar_t        szAttrNames[XML_NAMES_LENGTH] ;
    wchar_t        szCondAttrNames[XML_NAMES_LENGTH] ;
    wchar_t        szCondAttrValues[XML_NAMES_LENGTH] ;
    wchar_t        szSkipAttrValues[XML_NAMES_LENGTH] ;
    wchar_t        szStateAttrNames[XML_NAMES_LENGTH] ;
    wchar_t        szStateAttrValues[XML_NAMES_LENGTH] ;
	wchar_t        szInsertCopyAttr[XML_NAMES_LENGTH] ;
	wchar_t        szInsertCondAttr[XML_NAMES_LENGTH] ;
	wchar_t        szInsertCondAttrValues[XML_NAMES_LENGTH] ;
} ;               
#endif
