/*
*
*  Copyright (C) 1998-2013, International Business Machines          
*         Corporation and others. All rights reserved 
*
*/
/****************************************************************************/
/*                                                                          */
/* STATE.CPP                                                                */
/*                                                                          */
/* Define a state machine for xml markup table. This lets you define where  */
/* you can have quotes, comments, tags, cdata sections etc ..               */
/*                                                                          */
/* Functions:                                                               */
/*   AllFalseBut(short count, ...) checks that all states are false but ... */
/*   on(short state)  peeks to see if we can turn on a state                */
/*   off(short state) peeks to see if we can turn off a state               */
/*                                                                          */
/*==========================================================================*/
/*  Date    : Who : Description                                             */
/*==========================================================================*/
/* 7/08/2010: IBM : Original Source                                         */
/*==========================================================================*/

#include "state.h"
#include <stdlib.h>
#include "usrcalls.h"

#define CDATA         (XmlState::cdata)
#define COMENT        (XmlState::comment)
#define CONDATTR      (XmlState::condattr)
#define DOCTAG        (XmlState::doctag)
#define DQUOTE        (XmlState::dquote)
#define ENTITY        (XmlState::entity)
#define HDQUOTE       (XmlState::hdquote)
#define HSQUOTE       (XmlState::hsquote)
#define JS_LCOMMENT   (XmlState::JS_lcomment)
#define JS_ASSIGN     (XmlState::JS_assign)
#define JS_ASSIGN1    (XmlState::JS_assign1)
#define JS_ASSIGN2    (XmlState::JS_assign2)
#define JS_HDQUOTE    (XmlState::JS_hdquote)
#define JS_HSQUOTE    (XmlState::JS_hsquote)
#define SQUOTE        (XmlState::squote)
#define TAG           (XmlState::tag)
#define NONTRANSBLOCK (XmlState::nontransblock)
#define NONTRANS      (XmlState::nontrans)

extern wifstream *InputFile ;
extern XmlState  XTbl ;
extern char szProgPath[256] ;
extern int  sCopytextState ;
extern int  sCopytextState ;
extern int  sIgnoreCommentState ;
extern int  sLinesState ;
extern int  sMaxLength ;             
extern int  sScriptState ;
extern BOOL bCodePageANSI ;
extern BOOL bCodePageHTML ;
extern BOOL bCodePageUTF8 ;
extern BOOL bCodePageUTF16 ;
extern BOOL bXLIFF ;
extern BOOL bSDLXLF;
extern BOOL bXHTML ;
extern BOOL bFirstTag ;
extern BOOL bInFirstTag ;
extern BOOL bBidi  ;
extern BOOL bUTF16 ;
extern BOOL bTruncatedRead ;
extern BOOL bPreserveEOL ;   
extern BOOL bInsertEOL ;   
extern BOOL bCDATA_XHTMLTag ;
extern BOOL bEmptyEntityValue ;
extern BOOL bTransAttrHasNoCondAttr ; 
extern BOOL bEscapeDquote ;

extern BOOL bStartTrans ;             


#ifdef _UNICODE
extern wchar_t ENDL[4] ;
#else
extern char ENDL[4] ;
#endif

extern   short   sTPVersion ;               /* From USRCALLS.C  */
extern   char    szDocTargetLanguage[80];   /* From USRCALLS.C  */
extern   char    szDocSourceLanguage[80];   /* From USRCALLS.C  */



/****************************************************************************/
/*                                                                          */
/* XmlState::AllFalseBut                                                    */
/*                                                                          */
/* Check that all states are FALSE except for the ones in the argument      */
/* list.                                                                    */
/*                                                                          */
/****************************************************************************/
BOOL XmlState::AllFalseBut(short count, ...)
{
    va_list ap ;
    int i = 0 ;
    BOOL parlist[XML_numstates] ;

    // initialize the variable param list
    va_start(ap, count) ;

    // find out what states need to be false and what states need
    // to be true ...
    for (i = 0; i < numstates; i++ )
        parlist[i] = FALSE ;
    for (i = 0; i < count; i++ )
        parlist[va_arg(ap, int)] = TRUE ;

    for ( int j = 0 ; j < numstates ; j++ ) {
        if (  tbl[j] != parlist[j] ) {
            return FALSE ;
        } /* if */
    } // for
    return TRUE ;
}


/****************************************************************************/
/*                                                                          */
/* XmlState::CheckStates                                                    */
/*                                                                          */
/* Check that all passed states are set as follows (ignore all other states)*/
/*     positive value  = TRUE                                               */
/*     negative value  = FALSE                                              */
/*                                                                          */
/****************************************************************************/
BOOL XmlState::CheckStates(short count, ...)
{
    va_list ap ;
    int i ;
    short val ;
    BOOL bTest ;

    // initialize the variable param list
    va_start(ap, count) ;

    for (i = 0; i < count; i++ ) {
       val = va_arg(ap, int) ;
       if ( val >= 0 ) {
          bTest = TRUE ;
       } else {
          val = -val ;
          bTest = FALSE ;
       }
       if ( tbl[val] != bTest )
          return FALSE ;
    }
    return TRUE ;
}


/****************************************************************************/
/*                                                                          */
/* XmlState::off                                                            */
/*                                                                          */
/* Check to see if it is OK to turn the state off.                          */
/*                                                                          */
/****************************************************************************/
BOOL XmlState::off(short state)
{
    short rval = FALSE ;


    if ( ( sScriptState == SCRIPT_STATE_NONE ) ||
         ( ( ( sScriptState != SCRIPT_STATE_JAVASCRIPT ) ||
             ( ! JS_off( state, &rval )                ) ) &&
           ( ( sScriptState != SCRIPT_STATE_VBSCRIPT   ) ||
             ( ! VBS_off( state, &rval )               ) ) ) ) {

       switch ( state ) {
       case doctag :
       case cdata :
       case xmldoc:
           rval =    AllFalseBut(1,state);
           break;
       case comment:
           rval =    AllFalseBut(1,state)
                  || AllFalseBut(2,state,nontrans);
           break;
       case nontransblock:
           rval =    AllFalseBut(1,state)
                  || AllFalseBut(2,state,nontrans);
           break;
       case tag:
           rval =    AllFalseBut(1,state)
                  || AllFalseBut(2,state,globaltag)                                                  
                  || CheckStates(6,state,attrtag,-dquote,-squote,-hdquote,-hsquote)             
                  || CheckStates(7,state,attrtag,globaltag,-dquote,-squote,-hdquote,-hsquote)   
                  || CheckStates(6,state,neuttag,-dquote,-squote,-hdquote,-hsquote)             
                  || CheckStates(6,state,nontrans,-dquote,-squote,-hdquote,-hsquote)            
                  || CheckStates(7,state,nontrans,globaltag,-dquote,-squote,-hdquote,-hsquote)  
                  || CheckStates(6,state,nontranstag,-dquote,-squote,-hdquote,-hsquote) ;       
           break;
       case squote :
       case dquote :
           rval =    AllFalseBut(3,state,entity,doctag)
                  || AllFalseBut(2,state,entity)
                  || AllFalseBut(4,state,xmldoc,xmlname,encoding)
                  || CheckStates(2,state,tag) 
                  || CheckStates(3,state,tag,globaltag) ;   
           break ;
       case hsquote:
       case hdquote :
           rval =    AllFalseBut(1,state)
                  || AllFalseBut(2,state,doctag)
                  || AllFalseBut(4,state,doctag,entity,system)
                  || AllFalseBut(3,state,entity,system)
                  || CheckStates(2,state,tag) ;
           break ;
       case hcomment:
           rval =    AllFalseBut(2,state,doctag);
           break ;
       case encoding:
           rval =    AllFalseBut(3,state,xmldoc,xmlname);
           break ;
       case xmlname:
           rval =    AllFalseBut(2,state,xmldoc);
           break ;
       case xmllang:
           rval =    CheckStates(2,state,tag);
           break ;
       case system:
           rval =    CheckStates(2,state,entity);
           break ;
       case entity:
           rval =    AllFalseBut(2,state,doctag)
                  || AllFalseBut(3,state,doctag,system)
                  || AllFalseBut(1,state)
                  || AllFalseBut(2,state,system);
           break ;
       case attrtag:
       case condattr:
       case transattr:
       case neuttag :
           rval =    CheckStates(2,state,tag)  
                  || CheckStates(3,state,tag,globaltag) ;  
           break ;
       case nontranstag:
           rval =    CheckStates(1,state) ;
           break ;
       case globaltag :                                         
           rval =    CheckStates(1,state) ;
           break ;
       default :
           // unknown state
           rval =    FALSE;
           break ;
       } // switch
    }

    return( (BOOL)rval ) ;
}


/****************************************************************************/
/*                                                                          */
/* XmlState::on                                                             */
/*                                                                          */
/* Check to see if it is OK to turn the state on.                           */
/*                                                                          */
/****************************************************************************/
BOOL XmlState::on(short state)
{
    short rval = FALSE ;


    if ( ( sScriptState == SCRIPT_STATE_NONE ) ||
         ( ( ( sScriptState != SCRIPT_STATE_JAVASCRIPT ) ||
             ( ! JS_on( state, &rval )                 ) ) &&
           ( ( sScriptState != SCRIPT_STATE_VBSCRIPT   ) ||
             ( ! VBS_on( state, &rval )                ) ) ) ) {

       switch ( state ) {
       case doctag:
       case cdata:
       case xmldoc:
           rval =    AllFalseBut(0) 
                  || AllFalseBut(1,globaltag) ;   
           break;
       case tag:
           rval =    AllFalseBut(0) 
                  || AllFalseBut(1,globaltag)    
                  || ( AllFalseBut(1,cdata) && bCDATA_XHTMLTag ) ;
           break;
       case comment:
           rval =    AllFalseBut(0)
                  || AllFalseBut(1,nontrans);
           break;
       case nontransblock:
           rval =    AllFalseBut(0)
                  || AllFalseBut(1,nontrans);
           break;
       case system :
           rval =    AllFalseBut(2,entity,doctag)
                  || AllFalseBut(1,entity) ;
           break;
       case squote:
       case dquote:
           rval =    AllFalseBut(2,entity,doctag)
                  || AllFalseBut(1,entity)
                  || AllFalseBut(3,xmldoc,xmlname,encoding)
                  || CheckStates(5,tag,xmllang,-squote,-dquote,-system)
                  || AllFalseBut(4,tag,attrtag,condattr,transattr)
                  || AllFalseBut(5,tag,attrtag,condattr,transattr,globaltag)
                  || AllFalseBut(5,tag,attrtag,neuttag,condattr,transattr) 
                  || AllFalseBut(5,tag,attrtag,condattr,transattr,nontranstag)           
                  || AllFalseBut(6,tag,attrtag,condattr,transattr,globaltag,nontranstag) 
                  || AllFalseBut(6,tag,attrtag,neuttag,condattr,transattr,nontranstag);  
           break ;
       case hsquote:
       case hdquote :
           rval =    AllFalseBut(1,tag)
                  || AllFalseBut(2,tag,globaltag)                                        
                  || AllFalseBut(1,doctag)                                               
                  || AllFalseBut(3,doctag,entity,system)                                 
                  || AllFalseBut(2,entity,system)                                        
                  || AllFalseBut(2,tag,attrtag)                                          
                  || AllFalseBut(3,tag,attrtag,globaltag)                                
                  || AllFalseBut(3,tag,attrtag,condattr)                                 
                  || AllFalseBut(4,tag,attrtag,condattr,globaltag)                       
                  || AllFalseBut(4,tag,attrtag,neuttag,condattr)                         
                  || CheckStates(4,tag,xmllang,-hsquote,-hdquote)
                  || AllFalseBut(3,tag,attrtag,nontranstag)           
                  || AllFalseBut(4,tag,attrtag,globaltag,nontranstag) 
                  || AllFalseBut(4,tag,attrtag,condattr,nontranstag)  
                  || AllFalseBut(5,tag,attrtag,condattr,globaltag,nontranstag) 
                  || AllFalseBut(5,tag,attrtag,neuttag,condattr,nontranstag)   
                  || AllFalseBut(2,tag,neuttag)                     
                  || AllFalseBut(3,tag,neuttag,condattr)            
                  || AllFalseBut(4,tag,neuttag,condattr,nontranstag); 
           break ;
       case hcomment:
           rval =    AllFalseBut(1,doctag) ;
           break ;
       case xmlname:
           rval =    AllFalseBut(1,xmldoc);
           break ;
       case encoding:
           rval =    AllFalseBut(2,xmldoc,xmlname);
           break ;
       case xmllang:
           rval =    CheckStates(2,tag,-nontrans);
           break ;
       case entity:
           rval =    AllFalseBut(1,doctag)
                  || AllFalseBut(0) ;
           break ;
       case attrtag:
           rval =    AllFalseBut(1,tag)
                  || AllFalseBut(2,tag,globaltag)    
                  || AllFalseBut(2,tag,neuttag) ;
           break ;
       case neuttag:
           rval =    AllFalseBut(1,tag)
                  || AllFalseBut(2,tag,globaltag)    
                  || AllFalseBut(2,tag,attrtag);
           break ;
       case condattr:
           rval =    AllFalseBut(2,tag,attrtag)
                  || AllFalseBut(3,tag,attrtag,globaltag)                                           
                  || AllFalseBut(3,tag,attrtag,neuttag)                                             
                  || AllFalseBut(3,tag,attrtag,nontranstag);                                        
           break ;                                                                                  
       case transattr:                                                                              
           rval =    AllFalseBut(2,tag,attrtag)                                                     
                  || AllFalseBut(3,tag,attrtag,globaltag)                                           
                  || AllFalseBut(3,tag,attrtag,condattr)                                            
                  || AllFalseBut(4,tag,attrtag,condattr,globaltag)                                  
                  || AllFalseBut(4,tag,attrtag,neuttag,condattr)                                    
                  || AllFalseBut(3,tag,attrtag,nontranstag)                                         
                  || AllFalseBut(4,tag,attrtag,condattr,nontranstag);                               
           break ;
       case dtdname:
           rval =    AllFalseBut(1,doctag)
                  || AllFalseBut(2,doctag,dquote)
                  || AllFalseBut(2,doctag,squote)
                  || AllFalseBut(2,doctag,hdquote)
                  || AllFalseBut(2,doctag,hsquote)
                  || AllFalseBut(2,doctag,system)
                  || AllFalseBut(3,doctag,system,dquote)
                  || AllFalseBut(3,doctag,system,squote)
                  || AllFalseBut(3,doctag,system,hdquote)
                  || AllFalseBut(3,doctag,system,hsquote) ;
           break ;
       case globaltag :
           rval =    CheckStates(1,-state) ;
           break ;
       default :
           // unknown state
           rval =    FALSE;
           break ;
       } // switch
    }

    return( (BOOL)rval ) ;
}


/****************************************************************************/
/*                                                                          */
/* XmlState::JS_off                                                         */
/*                                                                          */
/* Check to see if it is OK to turn the JavaScript unique state off.        */
/*                                                                          */
/****************************************************************************/
BOOL XmlState::JS_off(short state, short* pval)
{
    short    rval ;
    BOOL     bHandled ;

    rval = *pval ;

    if ( XML_tbl[nontrans] ) {

       bHandled = TRUE ;

       switch ( state ) {
       case JS_lcomment:
       case JS_bcomment:
           rval =    CheckStates(5,state,-JS_squote,-JS_dquote,-JS_hsquote,-JS_hdquote);
           break;
       case JS_squote:
       case JS_dquote:
           rval =    AllFalseBut(1,state);
           break;
       case JS_hsquote:
       case JS_hdquote:
           rval =    AllFalseBut(1,state)
                  || AllFalseBut(2,state,JS_paren)
                  || AllFalseBut(3,state,JS_assign,JS_paren)          
                  || AllFalseBut(4,state,JS_assign,JS_assign1,JS_paren)
                  || AllFalseBut(5,state,JS_assign,JS_assign1,JS_paren,JS_array);
           break;
       case JS_paren:
           rval =    AllFalseBut(1,state)
                  || AllFalseBut(2,state,JS_assign)          
                  || AllFalseBut(3,state,JS_assign,JS_assign1)
                  || AllFalseBut(3,state,JS_assign,JS_array)            
                  || AllFalseBut(4,state,JS_assign,JS_assign1,JS_array);
           break;
       case JS_array:
           rval =    AllFalseBut(1,state)                    
                  || CheckStates(2,state,JS_assign)
                  || AllFalseBut(4,state,JS_assign,JS_assign1,JS_assign2);
           break;
       case JS_assign:
       case JS_assign1:
       case JS_assign2:
           rval =    CheckStates(5,state,-JS_squote,-JS_dquote,-JS_hsquote,-JS_hdquote); 
           break;
       case JS_globaltag:
           rval =    CheckStates(1,state);
           break;
       case JS_nontransblock:
           rval =    CheckStates(1,state) ;
           break;
       default :
           rval =    FALSE;          // unknown state
           break ;
       }
    } else {

       bHandled = FALSE ;

       switch ( state ) {
       case dquote:
           if ( SCRIPT_tbl[JS_dquote] ) {
              rval = TRUE ;
              bHandled = TRUE ;
           }
           break ;
       case squote:
           if ( SCRIPT_tbl[JS_squote] ) {
              rval = TRUE ;
              bHandled = TRUE ;
           }
           break ;                       
       case tag:
           if ( SCRIPT_tbl[VBS_globaltag] ) { 
              rval = TRUE ;
              bHandled = TRUE ;
           }
           break ;
       default :
           break ;
       }
    }

    *pval = rval ;
    return( bHandled ) ;
}


/****************************************************************************/
/*                                                                          */
/* XmlState::JS_on                                                          */
/*                                                                          */
/* Check to see if it is OK to turn the JavScript unique state on.          */
/*                                                                          */
/****************************************************************************/
BOOL XmlState::JS_on(short state, short* pval)
{
    short    rval ;
    BOOL     bHandled ;

    rval = *pval ;

    if ( XML_tbl[nontrans] ) {

       bHandled = TRUE ;

       switch ( state ) {
       case JS_lcomment:
       case JS_bcomment:
           rval =    AllFalseBut(0)
                  || AllFalseBut(1,JS_assign)
                  || AllFalseBut(2,JS_assign,JS_assign1)
                  || AllFalseBut(3,JS_assign,JS_assign1,JS_assign2)
                  || AllFalseBut(2,JS_assign,JS_array)
                  || AllFalseBut(3,JS_assign,JS_assign1,JS_array)
                  || AllFalseBut(4,JS_assign,JS_assign1,JS_assign2,JS_array)
                  || AllFalseBut(1,JS_paren)              
                  || AllFalseBut(2,JS_assign,JS_paren)
                  || AllFalseBut(3,JS_assign,JS_assign1,JS_paren)
                  || AllFalseBut(4,JS_assign,JS_assign1,JS_assign2,JS_paren)
                  || AllFalseBut(3,JS_assign,JS_array,JS_paren)
                  || AllFalseBut(4,JS_assign,JS_assign1,JS_array,JS_paren)
                  || AllFalseBut(5,JS_assign,JS_assign1,JS_assign2,JS_array,JS_paren);
           break;
       case JS_squote:
       case JS_dquote:
           rval =    AllFalseBut(1,JS_assign)
                  || AllFalseBut(2,JS_assign,JS_array)
                  || AllFalseBut(2,JS_assign,JS_assign1)  
                  || AllFalseBut(3,JS_assign,JS_assign1,JS_array);  
           break;
       case JS_hsquote:
       case JS_hdquote:
           rval =    AllFalseBut(0)
                  || AllFalseBut(1,JS_paren)
                  || AllFalseBut(2,JS_assign,JS_paren)              
                  || AllFalseBut(3,JS_assign,JS_assign1,JS_paren)
                  || AllFalseBut(4,JS_assign,JS_assign1,JS_paren,JS_array);
           break;
       case JS_paren:
           rval =    AllFalseBut(0)
                  || AllFalseBut(1,state)
                  || AllFalseBut(1,JS_assign)
                  || AllFalseBut(2,JS_assign,state)
                  || AllFalseBut(2,JS_assign,JS_assign1)
                  || AllFalseBut(3,JS_assign,JS_assign1,state);
           break;
       case JS_array:
           rval =    AllFalseBut(1,JS_assign)
                  || AllFalseBut(2,JS_assign,state);
           break;
       case JS_assign:
           rval =    AllFalseBut(0);
           break;
       case JS_assign1:
           rval =    AllFalseBut(1,JS_assign);
           break;
       case JS_assign2:
           rval =    AllFalseBut(2,JS_assign,JS_assign1);
           break;
       case JS_globaltag:
           rval =    AllFalseBut(0);
           break;
       case JS_nontransblock:
           rval =    AllFalseBut(0) ;
           break;
       default :
           // unknown state
           rval =    FALSE;
           break ;
       }
    } else {

       bHandled = FALSE ;

       switch ( state ) {
       case dquote:
           if ( SCRIPT_tbl[JS_dquote] ) {
              rval = TRUE ;
              bHandled = TRUE ;
           }
           break ;
       case squote:
           if ( SCRIPT_tbl[JS_squote] ) {
              rval = TRUE ;
              bHandled = TRUE ;
           }
           break ;
       case tag:
           if ( SCRIPT_tbl[JS_globaltag] ) {
              rval = TRUE ;
              bHandled = TRUE ;
           }
           break ;
       default :
           break ;
       }
    }

    *pval = rval ;
    return( bHandled ) ;

}


/****************************************************************************/
/*                                                                          */
/* XmlState::VBS_off                                                        */
/*                                                                          */
/* Check to see if it is OK to turn the VisualBasic unique state off.       */
/*                                                                          */
/****************************************************************************/
BOOL XmlState::VBS_off(short state, short* pval)
{
    short    rval ;
    BOOL     bHandled ;

    rval = *pval ;

    if ( XML_tbl[nontrans] ) {

       bHandled = TRUE ;

       switch ( state ) {
       case VBS_comment:
           rval =    CheckStates(1,state);
           break;
       case VBS_dquote:
           rval =    AllFalseBut(1,state);
           break;
       case VBS_hdquote:
           rval =    AllFalseBut(1,state)
                  || AllFalseBut(2,state,VBS_paren)
                  || AllFalseBut(4,state,VBS_assign,VBS_assign1,VBS_paren);
           break;
       case VBS_paren:
           rval =    AllFalseBut(1,state)
                  || AllFalseBut(2,state,VBS_assign)
                  || AllFalseBut(3,state,VBS_assign,VBS_assign1);
           break;
       case VBS_assign:
       case VBS_assign1:
       case VBS_assign2:
           rval =    CheckStates(3,state,-VBS_dquote,-VBS_hdquote); 
           break;
       case VBS_globaltag:
           rval =    CheckStates(1,state);
           break;
       case VBS_nontransblock:
           rval =    CheckStates(1,state) ;
           break;
       default :
           rval =    FALSE;          // unknown state
           break ;
       }
    } else {

       bHandled = FALSE ;

       switch ( state ) {
       case dquote:
           if ( SCRIPT_tbl[VBS_dquote] ) {
              rval = TRUE ;
              bHandled = TRUE ;
           }
           break ;
       case tag:
           if ( SCRIPT_tbl[VBS_globaltag] ) {
              rval = TRUE ;
              bHandled = TRUE ;
           }
           break ;
       default :
           break ;
       }
    }

    *pval = rval ;
    return( bHandled ) ;
}


/****************************************************************************/
/*                                                                          */
/* XmlState::VBS_on                                                         */
/*                                                                          */
/* Check to see if it is OK to turn the VisualBasic unique state on.        */
/*                                                                          */
/****************************************************************************/
BOOL XmlState::VBS_on(short state, short* pval)
{
    short    rval ;
    BOOL     bHandled ;

    rval = *pval ;

    if ( XML_tbl[nontrans] ) {

       bHandled = TRUE ;

       switch ( state ) {
       case VBS_comment:
           rval =    AllFalseBut(0)
                  || AllFalseBut(1,VBS_assign)
                  || AllFalseBut(2,VBS_assign,VBS_assign1)
                  || AllFalseBut(3,VBS_assign,VBS_assign1,VBS_assign2);
           break;
       case VBS_dquote:
           rval =    AllFalseBut(1,VBS_assign)
                  || AllFalseBut(2,VBS_assign,VBS_assign1);  
           break;
       case VBS_hdquote:
           rval =    AllFalseBut(0)
                  || AllFalseBut(1,VBS_paren)
                  || AllFalseBut(3,VBS_assign,VBS_assign1,VBS_paren);
           break;
       case VBS_paren:
           rval =    AllFalseBut(0)
                  || AllFalseBut(1,state)
                  || AllFalseBut(1,VBS_assign)
                  || AllFalseBut(2,VBS_assign,state)
                  || AllFalseBut(2,VBS_assign,VBS_assign1)
                  || AllFalseBut(3,VBS_assign,VBS_assign1,state);
           break;
       case VBS_assign:
           rval =    AllFalseBut(0);
           break;
       case VBS_assign1:
           rval =    AllFalseBut(1,VBS_assign);
           break;
       case VBS_assign2:
           rval =    AllFalseBut(2,VBS_assign,VBS_assign1);
           break;
       case VBS_globaltag:
           rval =    AllFalseBut(0);
           break;
       case VBS_nontransblock:
           rval =    AllFalseBut(0) ;
           break;
       default :
           // unknown state
           rval =    FALSE;
           break ;
       }
    } else {

       bHandled = FALSE ;

       switch ( state ) {
       case dquote:
           if ( SCRIPT_tbl[VBS_dquote] ) {
              rval = TRUE ;
              bHandled = TRUE ;
           }
           break ;
       default :
           break ;
       }
    }

    *pval = rval ;
    return( bHandled ) ;

}


/****************************************************************************/
/*                                                                          */
/* XmlState::setoff                                                         */
/*                                                                          */
/* Set the state to off.                                                    */
/*                                                                          */
/****************************************************************************/
void XmlState::setoff(short state)
{
    if ( state >= numstates )
        return ;
    tbl[state] = FALSE ;
}


/****************************************************************************/
/*                                                                          */
/* XmlState::seton                                                          */
/*                                                                          */
/* Set the state to on.                                                     */
/*                                                                          */
/****************************************************************************/
void XmlState::seton(short state)
{
    tbl[state] = TRUE ;
}


/****************************************************************************/
/*                                                                          */
/* XmlState::IsOn                                                           */
/*                                                                          */
/* Check is state is on.                                                    */
/*                                                                          */
/****************************************************************************/
BOOL XmlState::IsOn(short state)
{
    return( tbl[state] ) ;
}


/****************************************************************************/
/*                                                                          */
/* XmlState::IsOff                                                          */
/*                                                                          */
/* Check is state is off                                                    */
/*                                                                          */
/****************************************************************************/
BOOL XmlState::IsOff(short state)
{
    return( ! tbl[state] ) ;
}




/****************************************************************************/
/*                                                                          */
/* Token::GetNextToken                                                      */
/*                                                                          */
/* Get the value of the next XML token in the string.  All types of         */
/* string matching is done here.                                            */
/*                                                                          */
/****************************************************************************/
short Token::GetNextToken(void)
{
    IString s ;
    int len ;
    int type ;
    int i = (index += lasttoken) ;
    BOOL bTemp ;
    wchar_t c ;
    wchar_t cPrev ;
    wchar_t buff[2048] ;                                     
    wchar_t *ptr, *ptr2, *ptr3 ;
    wchar_t quote_char ;
    wchar_t szTemp[1024] ;
    char szNum[1024] ;
    int j, k, l ;


    if (  i > Length ) {
       if ( ( i == 1 ) &&                    /* Save blank line. */
            ( sCopyTextState ) ) {
          cPrev = L' ' ;
          HandleCopyTextText((wchar_t*)(LPCTSTR)str.subString(0), cPrev ) ;  
       }
       return eol_tok ;
    }

    wcsncpy( buff, (wchar_t*)(LPCTSTR)str.subString(i), sizeof(buff)/sizeof(wchar_t)) ;
    buff[(sizeof(buff)/sizeof(wchar_t))-1] = NULL;
    wcsupr(buff) ;
    c = *buff;
    cCurrentChar = c ;
    if ( i > 1 ) {
        cPrev = str.GetAt(i-2);
    } else {
       cPrev = L' ' ;
       if ( sCopyTextState ) 
          HandleCopyTextText((wchar_t*)(LPCTSTR)str.subString(0), cPrev ) ;  
    }

    if ( ( GetTransState() != TRANS_STATE_OFF ) &&
         ( ! XTbl.off(NONTRANSBLOCK) ) &&               /* Not in non-trans block  */
         ( ! XTbl.off(COMENT) ) &&                       /* Not in comment section */
         ( ! XTbl.off(HDQUOTE) ) &&                      /* Not in quoted attr.    */
         ( ! XTbl.off(HSQUOTE) ) &&                      /* Not in quoted attr.    */
         ( wcscmp( GetCurrentTagName(), L"!ENTITY" ) ) ) /* Not in entity section  */
       HandleInsertAttribute(buff);

    // check if this is an end of sentence character.
    if ( ! iswspace(c) ) {
       
       if ( ( wcschr( L".?!", c ) ) )
          SetNeutralBreakOn() ;
       else
          SetNeutralBreakOff() ;
    }


    // check if this is a dbcs char
    lasttoken = 2 ;

    /*************************************************************************/
    /* Handle JavaScript unique tokens.                                      */
    /*************************************************************************/
    if ( sScriptState == SCRIPT_STATE_JAVASCRIPT ) {
       lasttoken = 1 ;
       if ( bScriptSkipHTMLComments ) { /* Skip HTML comments to line end */
          if ( ( c == '<'         ) &&
               ( *(buff+1) == '!' ) ) 
             return JS_l_comment_tok;
          if ( ( ! iswspace( c ) ) &&
               ( ! XTbl.off(JS_LCOMMENT) ) )  /* Not in line comment */
             bScriptSkipHTMLComments = FALSE ;
       }
       if ( ( c == '=' ) &&
            ( ! wcschr( L"<>=!-", cPrev   ) ) &&
            ( ( ! buff[1] ) ||
              ( ! wcschr( L"<>=!-", buff[1] ) ) ) ) return JS_equal_tok;
       if ( c == '+' ) return JS_plus_tok;
       if ( c == '[' ) {

          if ( wcsncmp(buff,L"[TWB",4) ==0 ) { /* Ignore other TWB tags */
          } else {
             return JS_lbracket_tok;                    
          }
       }
       if ( c == ',' ) return JS_comma_tok;             
       if ( c == ']' ) return JS_rbracket_tok;          
       if ( c == '(' ) return JS_lparen_tok;
       if ( c == ')' ) return JS_rparen_tok;
       if ( ( c == '<' ) &&
            ( ptrGlobalTagHead ) &&
            ( IsGlobalTag(buff) ) ) return JS_globaltag_tok ;

       lasttoken = 2 ;
       if ( wcsncmp(buff,L"/*",2)==0 ) return JS_lb_comment_tok;
       if ( wcsncmp(buff,L"*/",2)==0 ) return JS_rb_comment_tok;
       if ( ( wcsncmp(buff,L"//",2)==0 ) && 
            ( cPrev != ':' ) ) {          /* could be http:// */
          ptr = wcsstr(buff,L"NON-TRANSLATABLE") ; 
          if ( ptr ) {
             for( --ptr ; ptr>buff && iswspace(*ptr) ; --ptr ) ;
             for(       ; ptr>buff && iswalpha(*ptr) ; --ptr ) ;
             ++ptr ;
             if ( ! wcsncmp(ptr,L"START ",6) ) 
                return startnontrans_tok ;
             if ( ! wcsncmp(ptr,L"END ",4) ) 
                return endnontrans_tok ;
          }
          return JS_l_comment_tok;
       }
    }

    /*************************************************************************/
    /* Handle Visual Basic unique tokens.                                    */
    /*************************************************************************/
    if ( sScriptState == SCRIPT_STATE_VBSCRIPT ) {
       lasttoken = 1 ;
       if ( ( c == '=' ) &&
            ( ! wcschr( L"<>", cPrev ) ) ) return VBS_equal_tok;
       if ( c == '&' ) return VBS_concat_tok;
       if ( c == '+' ) return VBS_concat_tok;
       if ( c == '(' ) return VBS_lparen_tok;
       if ( c == ')' ) return VBS_rparen_tok;
       if ( c == '\'' ) {
          ptr = wcsstr(buff,L"NON-TRANSLATABLE") ; 
          if ( ptr ) {
             for( --ptr ; ptr>buff && iswspace(*ptr) ; --ptr ) ;
             for(       ; ptr>buff && iswalpha(*ptr) ; --ptr ) ;
             ++ptr ;
             if ( ! wcsncmp(ptr,L"START ",6) ) 
                return startnontrans_tok ;
             if ( ! wcsncmp(ptr,L"END ",4) ) 
                return endnontrans_tok ;
          }
          return VBS_comment_tok;
       }
       if ( ( wcsncmp(buff,L"IF",2) ==0 ) &&   /* Treat IF statement as a     */
            ( iswspace(cPrev) ) &&             /*  line comment since equal   */
            ( buff[2] ) &&                     /*  is not an assignment       */
            ( iswspace(buff[2]) ) ) return VBS_comment_tok;
       if ( ( c == '<' ) &&
            ( ptrGlobalTagHead ) &&
            ( IsGlobalTag(buff) ) ) return VBS_globaltag_tok ;
    }
    
    /*************************************************************************/
    /* Handle HTML unique tokens.                                            */
    /* Attribute value is not surrounded by quotes                           */
	/*************************************************************************/
		if(bXHTML){
			if( (c == L'=') &&
				 (XTbl.off(TAG) ) ){
				for( ptr=buff+1; (*ptr!=NULL) && iswspace(*ptr) ; ++ptr ) ;
				if( (*ptr && 
					(*ptr != L'\'') && 
					(*ptr != L'\"') ) ||
					(*ptr == NULL) ){
					lasttoken = 1 ;
					return HTML_attribute_equal_tok;					
				}		
			}
		}

    /*************************************************************************/
    /* Handle normal XML tokens.                                             */
    /*************************************************************************/
    lasttoken = 1 ;
    if ( c == '\'' ) return squote_tok ;
    if ( c == '\"' ) return dquote_tok ;


    /*************************************************************************/
    /* End of current tag,                                                   */
    /*************************************************************************/
    if ( c == '>' ) { 

		if ( nInsertAttrTag == INSERTATTR_COPIED ) {
           nInsertAttrTag = INSERTATTR_OFF ;
		   return -1;
		}

       if ( ( GetTransState() == TRANS_STATE_ON ) &&             
            ( ! XTbl.on(TAG)            ) &&     /* If tag found in protected area, */
            ( ! XTbl.off(TAG)           ) &&   
            ( ! XTbl.off(DOCTAG)        ) &&
            ( ! XTbl.off(ENTITY)        ) &&  
            ( ! XTbl.off(COMENT)        ) &&
            ( ! XTbl.off(NONTRANSBLOCK) ) &&
            ( ! XTbl.off(CDATA)         ) ) {
          str.remove( i, 1 ) ;
          str.insert( L"&TWBGT;", i-1 );
          Length += 6 ;
          lasttoken = 6 ;
          return -1 ;
       } 
       if ( ( GetTransState() == TRANS_STATE_ON ) &&             
            ( bStateTransAttrOnly ) ) {
          bStateTransAttrOnly = FALSE ;
          szTransStateEndTag[0] = NULL ;
          SetTransState( TRANS_STATE_OFF ) ;
       }

       if ( ( sCopyTextState ) &&
            ( sCopyTextState != COPYTEXT_STATE_FROM_START2 ) && /* Neutral tag */
            ( sCopyTextState != COPYTEXT_STATE_TO_START2 ) &&   /* Skip <target> text */
            ( ! XTbl.off(COMENT) ) ) {                       
          HandleCopyTextText((wchar_t*)(LPCTSTR)str.subString(i), cPrev ) ;  
       }

       if ( ( bXLIFF ) &&
            ( ! XTbl.off(COMENT) ) ) {   /* Not in comment section */
          if ( ( ! wcscmp(GetCurrentTagName(),L"GROUP" ) ) || /* If new XLIFF unit, reset */
               ( ! wcscmp(GetCurrentTagName(),L"/GROUP") ) ) { 
             if ( GetTransStateAttr() == TRANS_STATE_NONE ) 
                SetTransStateAttr( TRANS_STATE_ON ) ;
             SetTransStateAttr( GetTransStateAttr() ) ; /* Set prev state to current state */
             SetTransState( TRANS_STATE_NONE ) ;
             XTbl.setoff(NONTRANS) ;
             bTemp = FALSE ;
             ManageLevelTransStateAttr( &bTemp, FALSE ) ;
             XTbl.seton(NONTRANS) ;
             SetTransState( TRANS_STATE_OFF ) ;
          }
       }


       bInFirstTag = FALSE ;

       if ( cPrev == '/' ) {                /* Empty tag <xxxx /> */
          return rtagempty_tok ;
       }

       return rtag_tok ;
    }
    if ( iswspace(c) )  return - 1 ;

    lasttoken = 9 ;
    if ( wcsncmp(buff,L"<!DOCTYPE",9)==0 ) return ldoc_tok ;

    lasttoken = 8 ;
    if ( wcsncmp(buff,L"<!ENTITY",8)==0 ) return entity_tok ;
    if ( wcsncmp(buff,L"<![CDATA",8)==0 ) return l_cdata_tok ;
    if ( wcsncmp(buff,L"ENCODING",8)==0 ) {
       if ( ( ! wcsstr( buff, L"\"UTF-8\""  ) ) && 
            ( ! wcsstr( buff, L"\'UTF-8\'"  ) ) && 
            ( ! wcsstr( buff, L"\"UTF-16\"" ) ) && 
            ( ! wcsstr( buff, L"\'UTF-16\'" ) ) )  
          return encode_tok ;
    }
    if ( wcsncmp(buff,L"XML:LANG",8)==0 ) {
       IsCopyTextTag(buff);
       if ( bInFirstTag ) {
          if ( ! HandleXmlLang( buff, sizeof(buff)/sizeof(wchar_t), str, i, 8 ) ) 
             return xmllang_tok ;  /* Translatable if not auto replaced */
       } else {
          return xmllang_tok ;    /* Translatable when not on 1st tag.  */
       }
    }

    lasttoken = 6 ;
    if ( ( wcsncmp(buff,L"SYSTEM",6)==0 ) &&
         ( iswspace(cPrev) ) &&
         ( iswspace(*(buff+6)) ) )
       return sys_tok ;

    lasttoken = 4 ;
    if ( wcsncmp(buff,L"<!--",4)==0 ) {
       ptr = wcsstr(buff,L"NON-TRANSLATABLE") ; 
       if ( ptr ) {
          ptr2 = wcsstr( ptr, L"-->" ) ;
          ptr3 = wcsstr( ptr, L" VALUE" ) ;
          for( --ptr ; ptr>buff && iswspace(*ptr) ; --ptr ) ;
          for(       ; ptr>buff && iswalpha(*ptr) ; --ptr ) ;
          ++ptr ;
          if ( ! wcsncmp(ptr,L"START ",6) ) {
             if ( ( ptr3 ) &&                    
                  ( ( ! ptr2 ) ||
                    ( ptr3 < ptr2 ) ) ) 
                bNonTransAttrValue = TRUE ;
             else
                return startnontrans_tok ;
          } else
          if ( ! wcsncmp(ptr,L"END ",4) ) {
             if ( ( ptr3 ) &&                    
                  ( ( ! ptr2 ) ||
                    ( ptr3 < ptr2 ) ) ) 
                bNonTransAttrValue = FALSE ;
             else
                return endnontrans_tok ;
          }
       }
       return l_comment_tok ;
    }

    lasttoken = 3 ;
    if ( wcsncmp(buff,L"]]>",3)==0 ) return r_cdata_tok ;
    if ( wcsncmp(buff,L"-->",3)==0 ) return r_comment_tok ;
    if ( wcsncmp(buff,L"XML",3)==0 ) return xml_tok ;

    lasttoken = 2 ;
    if ( ( wcscmp( GetCurrentTagName(), L"!ENTITY" ) ) &&  
         ( GetScriptState() != SCRIPT_STATE_NONE     ) ) { 
       if ( wcsncmp(buff,L"\\\"",2) ==0 ) return -1  ;
       if ( wcsncmp(buff,L"\\\'",2) ==0 ) return -1  ;
    }

    if ( ( szCondAttrNames[0] ) &&
         ( IsAttribute(buff,&lasttoken,cPrev,szCondAttrNames ) ) ) {
       return condattr_tok ;
    }
    if ( ( ptrStateAttrHead ) &&
         ( IsAttribute(buff,&lasttoken,cPrev,szStateAttrNames ) ) &&
         ( ( XTbl.IsOn(TAG) ) ||      /* In tag */
           ( bXLIFF         ) ) &&
         ( ! XTbl.off(COMENT) ) )     /* Not in comment section */
       return stateattr_tok ;
    if ( ( szAttrNames[0] ) &&
         ( IsAttribute(buff,&lasttoken,cPrev,szAttrNames ) ) ) {
       if ( szCondAttrNames[0] &&                         
            ( ( ( XTbl.on(CONDATTR) ) || 
                ( ( GetTransState() == TRANS_STATE_ON ) &&       
                  ( GetTransStateAttr() == TRANS_STATE_ON ) &&       
                  ( XTbl.off(CONDATTR) ) ) ) ) )
          FindCondAttribute(buff,cPrev,szCondAttrNames ) ;
       if ( ( bXHTML ) &&                                 
            ( ! IsTransAttrHasCondAttr(buff) ) ) {        
           bTransAttrHasNoCondAttr = TRUE;
       }
       if ( ( GetTransState() != TRANS_STATE_OFF ) &&       
            ( GetTransStateAttr() != TRANS_STATE_OFF ) ) 
          return transattr_tok ;
    }
    if ( ( ! wcsncmp(buff,L".DTD\"",5) ) ||
         ( ! wcsncmp(buff,L".DTD\'",5) ) ) 
       return dtd_tok ;
    if ( wcsncmp(buff,L"\\\\",2) ==0 ) return -1  ;
    if ( wcsncmp(buff,L"]>",2) ==0 ) return rdoc_tok ;
    if ( wcsncmp(buff,L"?>",2) ==0 ) return rxml_tok ;
    if ( wcsncmp(buff,L"<?",2) ==0 ) {
       return lxml_tok ;
    }

    /*************************************************************************/
    /* Start of new tag.                                                     */
    /*************************************************************************/
    lasttoken = 1 ;
    if ( c == '<' ) {
       if ( buff[1] == '/' ) 
          bCurrentTagEnd = TRUE ;
       else
          bCurrentTagEnd = FALSE ;
       if ( bFirstTag ) {
          bInFirstTag = TRUE ;
          if ( wcsncmp(buff,L"<XLIFF ",7) == 0 ) {
             s=IString(L"xliff.dtd\"") ;
             SetDTDInfo( s, FALSE ) ;
          }
       }


       if ( bXLIFF ) {
          if ( ! wcsncmp(buff,L"<TRANS-UNIT",11) ) {  /* If new XLIFF unit, reset */
             SetTransStateAttr( TRANS_STATE_ATTR_CHECK ) ;
             if ( ptrStateAttrLevelHead ) 
                SetTransStateAttr( ptrStateAttrLevelHead->State ) ;
             else
                SetTransStateAttr( TRANS_STATE_NONE ) ;
             SetTransState( TRANS_STATE_OFF ) ;
             XTbl.seton(NONTRANS) ;
             SaveCurrentTagName() ;
          } else
          if ( ( ! wcsncmp(buff,L"<GROUP", 6) ) || 
               ( ! wcsncmp(buff,L"</GROUP",7) ) ) { 
             SetTransStateAttr( TRANS_STATE_ATTR_CHECK ) ;
             if ( ptrStateAttrLevelHead ) 
                SetTransStateAttr( ptrStateAttrLevelHead->State ) ;
             else
                SetTransStateAttr( TRANS_STATE_NONE ) ;
             SaveCurrentTagName() ;
          } else
          if ( ( ! wcsncmp(buff,L"<BIN-UNIT",  9) ) ||     
               ( ! wcsncmp(buff,L"</BIN_UNIT",10) ) ) { 
             SetTransStateAttr( TRANS_STATE_ATTR_CHECK ) ;
             if ( ptrStateAttrLevelHead ) 
                SetTransStateAttr( ptrStateAttrLevelHead->State ) ;
             else
                SetTransStateAttr( TRANS_STATE_NONE ) ;
             SaveCurrentTagName() ;
          } else
          if ( ! wcsncmp(buff,L"</TARGET", 8) ) {
             SaveCurrentTagName() ;
          } else
          if ( ! wcsncmp(buff,L"</ALT-TRANS", 11) ) {
             ptrStateTagHead = ptrStateTagHeadSave ;
             ptrStateTagHeadSave = NULL ;
             ptrCopyTextTagHead = ptrCopyTextTagHeadSave ;
             ptrCopyTextTagHeadSave = NULL ;
          } 
       }


       if ( wcsncmp(buff,L"<!",2) )       /* If not comment, 1st tag found */
          bFirstTag = FALSE ;
       if ( ( ! XTbl.off(COMENT) ) &&                    
            ( IsCopyTextTag(buff) ) ) {   /* Action performed in routine */
          if ( ( bXLIFF ) &&                             
               ( ! wcsncmp(buff,L"<ALT-TRANS", 10) ) ) {
             ptrStateTagHeadSave = ptrStateTagHead ;
             ptrStateTagHead = NULL ;
             ptrCopyTextTagHeadSave = ptrCopyTextTagHead ;
             ptrCopyTextTagHead = NULL ;
          }
          return -1 ;
       } 
       if ( ( bXLIFF ) &&                                
            ( ! wcsncmp(buff,L"<ALT-TRANS", 10) ) ) {
          ptrStateTagHeadSave = ptrStateTagHead ;
          ptrStateTagHead = NULL ;
          ptrCopyTextTagHeadSave = ptrCopyTextTagHead ;
          ptrCopyTextTagHead = NULL ;
       }
       if ( IsIgnoreCommentTag(buff) ) ;  /* Action performed in routine   */

       if ( ( XTbl.IsOn(CDATA) ) &&         /* Protect XHTML tags in CDATA section */
            ( sScriptState == SCRIPT_STATE_NONE ) &&     
            ( ( ! wcsncmp( buff, L"<P>"     , 3 ) ) ||
              ( ! wcsncmp( buff, L"</P>"    , 4 ) ) ||
              ( ! wcsncmp( buff, L"<BR>"    , 4 ) ) ||
              ( ! wcsncmp( buff, L"<BR/>"   , 5 ) ) ||
              ( ! wcsncmp( buff, L"<BR />"  , 6 ) ) ||
              ( ! wcsncmp( buff, L"<UL>"    , 4 ) ) ||   
              ( ! wcsncmp( buff, L"</UL>"   , 5 ) ) ||
              ( ! wcsncmp( buff, L"<LI>"    , 4 ) ) ||
              ( ! wcsncmp( buff, L"</LI>"   , 5 ) ) ||
              ( ! wcsncmp( buff, L"<HTML>"  , 6 ) ) ||
              ( ! wcsncmp( buff, L"</HTML>" , 7 ) ) ||
              ( ! wcsncmp( buff, L"<OL>"    , 4 ) ) ||   
              ( ! wcsncmp( buff, L"</OL>"   , 5 ) ) ||
              ( ! wcsncmp( buff, L"<DIV>"   , 5 ) ) ||
              ( ! wcsncmp( buff, L"</DIV>"  , 6 ) ) ) ) {
          bCDATA_XHTMLTag = TRUE ;
          XTbl.setoff(CDATA) ;
       }

       if ( ( GetTransState() == TRANS_STATE_ON ) &&          
            ( ! XTbl.on(TAG) ) )        /* If tag found in protected area, */
          return ltag_tok ;             /*  then do not reset any info.    */

       if ( ( ptrStateTagHead    ) &&
              ( ( ( ! XTbl.off(COMENT) ) &&          /* Not in comment section */
                  ( ! XTbl.off(NONTRANSBLOCK) ) ) || /* Not in non-trans block  */
              ( sScriptState != SCRIPT_STATE_NONE ) ) &&       
            ( IsTransStateTag(buff) ) ) {
          if ( IsAttributeTag(buff) ) return transstateattrtag_tok ;
          return transstatetag_tok ;
       }
       if ( IsLinesTag(buff) ) return linestag_tok ;
       if ( IsAttributeTag(buff) ) {
          if ( IsNeutralTag(buff) ) return neutattrtag_tok ;
          return attrtag_tok ;
       }
       if ( IsNeutralTag(buff) ) return neuttag_tok ;
       
       return ltag_tok ;             //this must be searched for last
    }
    if ( c == '.' ) return period_tok ;


    return -1 ;
}


/****************************************************************************/
/*                                                                          */
/* Token::SetString                                                         */
/*                                                                          */
/* Initialize the fields for this new string.                               */
/*                                                                          */
/****************************************************************************/
void Token::SetString(IString& s)
{
    index = 1 ;
    str = s ;
    Length = str.length() ;
    lasttoken = 0 ;
    trsoffidx = 0 ;              
}


/****************************************************************************/
/*                                                                          */
/* Token::GetString                                                         */
/*                                                                          */
/* Return the value of the current string.                                  */
/*                                                                          */
/****************************************************************************/
IString& Token::GetString(void)
{
    if ( trsoff ) {
       if ( ( Length <= 6 ) ||
            ( wcscmp( str.subString(Length-5), L"<twb--" ) ) ) {
          str +=  L"--twb><twb--" ;
          CheckNonTrans2K( Length+1, &index ) ;
       }
    }
    return str ;
}


/****************************************************************************/
/*                                                                          */
/* Token::DebugString                                                       */
/*                                                                          */
/* Return the value of the current string.                                  */
/*                                                                          */
/****************************************************************************/
IString& Token::DebugString(void)
{
    return str ;
}


/****************************************************************************/
/*                                                                          */
/* Token::GetCurrentChar                                                    */
/*                                                                          */
/* Return the current character being processed.                            */
/*                                                                          */
/****************************************************************************/
wchar_t Token::GetCurrentChar(void)
{
    return cCurrentChar ;
}


/****************************************************************************/
/*                                                                          */
/* Token::AddTagTransOn                                                     */
/*                                                                          */
/* Set the translation state to ON.  This will add the internal tags to     */
/* the source file, updates the string and all states in the class.         */
/*                                                                          */
/****************************************************************************/
void Token::AddTagTransOn(void)
{
    if ( trsoff ) {
       if ( GetTransState() == TRANS_STATE_ON ) {
          if ( ( index+lasttoken <= 6 ) ||                        
               ( wcsncmp( str.subString(index+lasttoken-6), L"--twb>", 6 ) ) ) {
             str.insert(L"--twb>",index+lasttoken-1) ;
             CheckNonTrans2K( index+lasttoken-1, &lasttoken ) ;
             Length += 6 ;
             lasttoken += 6 ;
          }
          trsoff = FALSE ;
       }
    } else {
       if ( GetTransState() == TRANS_STATE_OFF ) {
          AddTagTransOff();
       }
    }
}


/****************************************************************************/
/*                                                                          */
/* Token::AddTagTransOff                                                    */
/*                                                                          */
/* Set the translation state to OFF.  This will add the internal tags to    */
/* the source file, updates the string and all states in the class.         */
/*                                                                          */
/****************************************************************************/
void Token::AddTagTransOff(void)
{
    short  i = 0 ;

    if ( ! trsoff ) {
       if ( wcsncmp( str.subString(index), L"<twb--", 6 ) ) {    
          str.insert(L"<twb--",index-1+i) ;
          trsoffidx = index-1+6+i ;
          lasttoken += 6 ;
          Length += 6 ;
       }
       trsoff = TRUE ;
    }
}


/****************************************************************************/
/*                                                                          */
/* Token::ForceTransOn                                                      */
/*                                                                          */
/* Add ending TWB tag.                                                      */
/*                                                                          */
/****************************************************************************/
void Token::ForceTransOn(void)
{
   if ( index-1 >= 0 ) {
      str.insert(L"--twb>",index-1) ;
      CheckNonTrans2K( index-1, &index ) ;
      Length += 6 ;
      index += 6 ;
   }
}


/****************************************************************************/
/*                                                                          */
/* Token::AddNeutAttrOn                                                     */
/*                                                                          */
/* Have a translatable attribute within a neutral tag.  Must terminate      */
/* previous tag with special string (which is removed during file export).  */
/*                                                                          */
/****************************************************************************/
void Token::AddNeutAttrOn(void)
{
    str.insert(L"##twb>",index+lasttoken-1) ;
    Length += 6 ;
    lasttoken += 6 ;
}


/****************************************************************************/
/*                                                                          */
/* Token::AddNeutAttrOff                                                    */
/*                                                                          */
/* Have a translatable attribute within a neutral tag.  Must start a new    */
/* tag with a special string (which is removed during file export).         */
/*                                                                          */
/****************************************************************************/
void Token::AddNeutAttrOff(int offset)
{
    index += offset ;
    str.insert(L"<twb##",index+lasttoken-2) ;
    Length += 6 ;
    lasttoken += 6 ;
    index -= offset ;
}


/****************************************************************************/
/*                                                                          */
/* Token::AddTagTextOn                                                      */
/*                                                                          */
/* Have end of non-translatable text after a tag.                           */
/* Set the translation state to ON.  This will add the internal tags to     */
/* the source file, updates the string and all states in the class.         */
/*                                                                          */
/****************************************************************************/
void Token::AddTagTextOn( void )
{
    if ( ( index < 6 ) ||
         ( wcsncmp( str.subString(index-6), L"--twb>", 6 ) != 0 ) ) {
       str.insert(L"--twb>",index-1) ;
       CheckNonTrans2K( index-1, &index ) ;
       Length += 6 ;
       index += 6 ;
    }
    trsoff = FALSE ;                                      
}


/****************************************************************************/
/*                                                                          */
/* Token::AddTagTextOff                                                     */
/*                                                                          */
/* Have non-translatable text after a tag.                                  */
/* Set the translation state to OFF.  This will add the internal tags to    */
/* the source file, updates the string and all states in the class.         */
/*                                                                          */
/****************************************************************************/
void Token::AddTagTextOff(void)
{
    str.insert(L"<twb--",index+lasttoken-1) ;
    trsoffidx = index+lasttoken-1+6 ;
    Length += 6 ;
    lasttoken += 6 ;
    trsoff = TRUE ;                                       
}


/****************************************************************************/
/*                                                                          */
/* Token::AddNeutTextOn                                                     */
/*                                                                          */
/* Have end of non-translatable text after a neutral tag.  Must terminate   */
/* previous tag with special string (which is removed during file export).  */
/*                                                                          */
/****************************************************************************/
void Token::AddNeutTextOn( void )
{
    if ( ( index > 6 ) &&                                 
         ( ! wcsncmp( str.subString(index-6,6), L"<twb++", 6 ) ) ) {
       str.remove(index-6,6) ;
       Length -= 6 ;
       index -= 6 ;
    } else {
       str.insert(L"++twb>",index-1) ;
       Length += 6 ;
       index += 6 ;
    }
}


/****************************************************************************/
/*                                                                          */
/* Token::AddNeutTextOff                                                    */
/*                                                                          */
/* Have non-translatable text after a neutral tag.  Must start a new tag    */
/* with a special string (which is removed during file export).             */
/*                                                                          */
/****************************************************************************/
void Token::AddNeutTextOff(void)
{
    str.insert(L"<twb++",index+lasttoken-1) ;
    Length += 6 ;
    lasttoken += 6 ;
}



/****************************************************************************/
/*                                                                          */
/* Token::IsNeutralTag                                                      */
/*                                                                          */
/* Search table to determine if string is a neutral XML string.             */
/*                                                                          */
/****************************************************************************/
BOOL Token::IsNeutralTag( wchar_t *szText )
{
   wchar_t     szTag[XML_TAG_LENGTH] ;
   wchar_t     szTag2[XML_TAG_LENGTH] ;
   wchar_t     szTagLong[XML_COMPLETE_TAG_LENGTH] ;
   wchar_t     *ptrChar ;
   NEUTTAG     *ptrTemp ;
   BOOL        bFound = FALSE ;

   if ( ( ptrNeutralTagHead ) &&             /* If neutral tags for this XML,*/
        ( ( iswalpha(szText[1]) ) ||         /* and first char is alphabetic */
          ( szText[1] == '/'   ) ) ) {       /*     or valid end tag.        */
      wcsncpy( szTag, &szText[1], XML_TAG_LENGTH-1 ) ;
      szTag[XML_TAG_LENGTH-1] = NULL ;
      if ( ! wcschr( szTag, L'>' ) ) {              /* Neutral tag cannot be */
         wcsncpy( szTagLong, &szText[1], 2000 ) ;   /*   longer than a TM    */
         szTagLong[2000-1] = NULL ;                 /*   segment.            */
         if ( ( ! wcschr( szTagLong, L'>' ) ) &&
              ( wcslen(szTagLong) > 1998    ) ) {
            return( FALSE ) ;
         }
      }
      if ( szTag[0] == '/' ) 
         wcstok( szTag+1, L" />\n\t\r" ) ;        /* Get tag name for search */
      else
         wcstok( szTag, L" />\n\t\r" ) ;          /* Get tag name for search */

      for( ptrTemp=ptrNeutralTagHead ; ptrTemp ; 
           ptrTemp=(NEUTTAG*)(ptrTemp->ptrNext) ) {
         wcscpy( szTag2, ptrTemp->Tag ) ;
         wcsupr( szTag2 ) ; 
         if ( ( ( ! ptrTemp->Generic ) &&         /* Tag is an exact match  */
                ( ! wcscmp(szTag2, szTag ) ) ) ||
              ( ( ptrTemp->Generic ) &&           /* Tag matches generic tag */
                ( ( ! wcsncmp(szTag2, szTag, wcslen(szTag2) ) ) ||
                  ( ( szTag2[0] == L'*' ) &&     
                    ( wcslen(&szTag2[1]) < wcslen(szTag) ) &&
                    ( ! wcsncmp(&szTag2[1], &szTag[wcslen(szTag)-wcslen(&szTag2[1])], wcslen(&szTag2[1]) ) ) ) ) ) ) {
            break ;
         }

      }
      if ( ( ptrTemp ) &&                   /* If tag found, then valid     */
           ( ( ! ptrTemp->Break     ) ||    /* If not possible break tag, or*/
             ( ! bPrevNeutralBreak  ) ) )   /* no previous end of sentence  */
         bFound = TRUE ;                    /*  neutral tag.                */
   }

   return ( bFound ) ;
}


/****************************************************************************/
/*                                                                          */
/* Token::IsTransAttrHasCondAttr                                            */
/*                                                                          */
/* Search table to determine if a transtable attribute has                  */
/* corresponding conditional attribute.                                     */
/*                                                                          */
/****************************************************************************/
BOOL Token::IsTransAttrHasCondAttr( wchar_t *szText)
{
    ATTRTAG     *ptrTemp ;
    wchar_t     szTag[XML_TAG_LENGTH];
    wchar_t     szAttr[XML_ATTR_LENGTH] ;
    wchar_t     szAttrNames[XML_NAMES_LENGTH] ;
    BOOL        bFound = false ;
    wchar_t    *ptrChar;


    if ( ( ptrAttributeTagHead ) &&          /* If attribute tags for XML,   */
         ( iswalpha(szText[1])  ) ) {         /* and first char is alphabetic */

       wcscpy(szTag, GetCurrentTagName());

       wcsncpy( szAttr, &szText[0], XML_ATTR_LENGTH-1 ) ;
       szAttr[XML_ATTR_LENGTH-1] = NULL ;
       wcstok( szAttr, L" =/>\n\t\r" ) ;        /* Get attr name for search.     */

       for( ptrTemp=ptrAttributeTagHead ;
            ptrTemp ;
            ptrTemp=(ATTRTAG*)ptrTemp->ptrNext ) {
          if ( ( ! wcsicmp( ptrTemp->Tag, szTag ) ) || /* Specific tag match */
               ( ( ! wcscmp( ptrTemp->Tag, L"*" ) ) &&  /* Attr for any tag   */
                 ( ( ! ptrTemp->NotTag[0] ) ||         /* but not exception  */
                   ( wcsicmp( ptrTemp->NotTag, szTag ) ) ) ) ) {  
          
             wcscpy(szAttrNames, ptrTemp->Attr);
             //wcsupr(szTemp);
             ptrChar = wcstok(szAttrNames, L" \n\t\r");
             for( ; ptrChar; ptrChar=wcstok(NULL, L" \n\t\r")){
             	if ( !wcsicmp(ptrChar, szAttr) ) {
                   if( ptrTemp->CondAttr[0] ){  /* If conditional attr*/
                      bFound = TRUE ;
                      break;
                   }
                }
             }
             if(bFound)
                break;
          }
       } /* END FOR */
    }
    return bFound;
}

/****************************************************************************/
/*                                                                          */
/* Token::IsAttributeTag                                                    */
/*                                                                          */
/* Search table to determine if string is an XML tag which may have a       */
/* translatable attribute.                                                  */
/*                                                                          */
/****************************************************************************/
BOOL Token::IsAttributeTag( wchar_t *szText )
{
   wchar_t     szTag[XML_TAG_LENGTH] ;
   wchar_t     *ptrChar ;
   ATTRTAG     *ptrTemp ;
   BOOL        bFound = FALSE ;


   szAttrNames[0] = NULL ;
   szTag[0] = NULL ;
   if ( wcsncmp(&szText[1],L"SCRIPT",6) ) {
      szCondAttrNames[0] = NULL ;
      szCondAttrValues[0] = NULL ;
      szSkipAttrValues[0] = NULL ;
   }
   if ( ( ptrAttributeTagHead ) &&          /* If attribute tags for XML,   */
        ( iswalpha(szText[1])  ) ) {         /* and first char is alphabetic */
      wcsncpy( szTag, &szText[1], XML_TAG_LENGTH-1 ) ;
      szTag[XML_TAG_LENGTH-1] = NULL ;
      wcstok( szTag, L" />\n\t\r" ) ;        /* Get tag name for search.     */

      /*--------------------------------------------------------------------*/
      /*                                                                    */
      /* Find all attribute names for this particular tag (if there are any)*/
      /*                                                                    */
      /*--------------------------------------------------------------------*/
      for( ptrTemp=ptrAttributeTagHead ;
           ptrTemp ;
           ptrTemp=(ATTRTAG*)ptrTemp->ptrNext ) {
         if ( ( ! wcsicmp( ptrTemp->Tag, szTag ) ) || /* Specific tag match */
              ( ( ! wcscmp( ptrTemp->Tag, L"*" ) ) &&  /* Attr for any tag   */
                ( ( ! ptrTemp->NotTag[0] ) ||         /* but not exception  */
                  ( wcsicmp( ptrTemp->NotTag, szTag ) ) ) ) ) { 
            bFound = TRUE ;
            if ( ptrTemp->Attr[0] ) {                 /* If trans. attr.,   */
               wcscat( szAttrNames, L" " ) ;           /*  then add to list  */
               wcscat( szAttrNames, ptrTemp->Attr ) ;
               wcscat( szAttrNames, L" " ) ;
            }
            if ( ( ptrTemp->CondAttr[0] ) &&          /* If conditional attr*/
                 ( wcsncmp(szTag,L"SCRIPT",6) ) ) {
               wcscat( szCondAttrNames, L" " ) ;       /*  then add to list  */
               wcscat( szCondAttrNames, ptrTemp->CondAttr ) ;
               wcscat( szCondAttrNames, L" " ) ;
            }
            if ( ( ptrTemp->CondAttrValue[0] ) &&     /* If cond. attr value*/
                 ( wcsncmp(szTag,L"SCRIPT",6) ) ) {
               wcscat( szCondAttrValues, L" " ) ;      /*  then add to list  */
               wcscat( szCondAttrValues, ptrTemp->CondAttrValue ) ;
               wcscat( szCondAttrValues, L" " ) ;
            }
            if ( ( ptrTemp->SkipAttrValue[0] ) &&     /* If skip attr value */
                 ( wcsncmp(szTag,L"SCRIPT",6) ) ) {
               wcscat( szSkipAttrValues, L" " ) ;      /*  then add to list  */
               wcscat( szSkipAttrValues, ptrTemp->SkipAttrValue ) ;
               wcscat( szSkipAttrValues, L" " ) ;
            }
         }
      }
   }
   wcsupr( szAttrNames ) ;
   wcsupr( szCondAttrNames ) ;
   wcsupr( szCondAttrValues ) ;
   wcsupr( szSkipAttrValues ) ;

   if ( ( bNonTransAttrValue ) &&                
        ( ( ! wcscmp( szTag, L"INPUT"  ) ) ||
          ( ! wcscmp( szTag, L"OPTION" ) ) ||
          ( ! wcscmp( szTag, L"PARAM"  ) ) ) ) {
      ptrChar = wcsstr( szAttrNames, L" VALUE " ) ;
      if ( ptrChar ) {
         wmemmove( ptrChar, ptrChar+6, wcslen(ptrChar+6)+1 ) ;
      }
   }

   return ( bFound ) ;
}


/****************************************************************************/
/*                                                                          */
/* Token::IsAttribute                                                       */
/*                                                                          */
/* Search table to determine if string is an attribute keyword which has    */
/* special processing.                                                      */
/*                                                                          */
/****************************************************************************/
BOOL Token::IsAttribute( wchar_t *szText, int *AttrLen, wchar_t cPrev, wchar_t *szTable  )
{
   wchar_t     szAttr[XML_ATTR_LENGTH] ;
   BOOL        bFound = FALSE ;

   if ( ( szTable[0]  ) &&
        ( iswspace(cPrev) ) &&
        ( iswalpha(szText[0]) ) ) {
      szAttr[0] = ' ' ;
      wcsncpy( &szAttr[1], szText, XML_ATTR_LENGTH-1 ) ;
      szAttr[XML_ATTR_LENGTH-1] = NULL ;
      wcstok( &szAttr[1], L" >=\n\t\r" ) ;
      wcscat( szAttr, L" " ) ;
      if ( wcsstr( szTable, szAttr ) ) {
         bFound = TRUE ;
         *AttrLen = wcslen(szAttr) - 2 ;      /* Adjust for lead/trail blank */
      }
   }

   return ( bFound ) ;
}


/****************************************************************************/
/*                                                                          */
/* Token::CheckCondAttr                                                     */
/*                                                                          */
/* Get the value of the next XML token in the string, and verify whether    */
/* it is a value which allows translatable attributes.                      */
/*                                                                          */
/****************************************************************************/
BOOL Token::CheckCondAttr(void)
{
    wchar_t    szValue[XML_ATTR_LENGTH] ;
    wchar_t*   tbuff ;
    wchar_t    szTemp[XML_COMPLETE_TAG_LENGTH] ;
    int        i = (index+lasttoken) ;
    BOOL       bFound = FALSE ;

	wchar_t     szTagText[XML_TAG_LENGTH] ; 
    
    wchar_t cPrev;  
	wchar_t *ptrChar;

	if ( ( szCondAttrValues[0] ) &&
         ( i <= Length+1         ) ) {             
       wcsncpy(szTemp, (wchar_t*)(LPCTSTR)str.subString(i), XML_ATTR_VALUE_LENGTH-1 );
       szTemp[XML_ATTR_VALUE_LENGTH-1] = NULL ;
     
       if ( i > 1 ) {
        cPrev = str.GetAt(i-2);
       } else
       	cPrev = L' ' ;
       
       if ( ( i > 0 ) &&
            ( szTemp[0] == *((wchar_t*)(LPCTSTR(str.subString(i-1,1)))) ) ) {
          szValue[0] = NULL ;
       } else {
	        if( bXHTML &&  cPrev == L'='){    
             /* trim leading blanks in szTemp */  
             GetCompleteTag( szTemp, sizeof(szTemp), 0 ) ; 
             tbuff = wcstok(szTemp, L" \n\t\r" );
          } else 
             tbuff = wcstok(szTemp, L" \"\'\n\t\r" );
          	
          wcsupr(tbuff);
          if ( tbuff ) {
             if ( sScriptState == SCRIPT_STATE_TAG_UNKNOWN ) { 
                tbuff = wcstok(tbuff, L"0123456789." ) ;  /* Handle "JavaScript1.2" */
             }
             szValue[0] = ' ' ;
             wcsncpy( &szValue[1], tbuff, XML_ATTR_LENGTH-3 ) ;
             szValue[XML_ATTR_LENGTH-2] = NULL ;
             wcscat( szValue, L" " ) ;
             if ( wcsstr( szCondAttrValues, szValue ) ) {
                bFound = TRUE ;
                if ( sScriptState == SCRIPT_STATE_TAG_UNKNOWN ) {
                   if ( ( ! wcsicmp( szValue, L" JAVASCRIPT "      ) ) ||
                        ( ! wcsicmp( szValue, L" JSCRIPT "         ) ) ||
                        ( ! wcsicmp( szValue, L" TEXT/JAVASCRIPT " ) ) ||                 
                        ( ! wcsicmp( szValue, L" TEXT/JSCRIPT "    ) ) ) {                
                      sScriptState = SCRIPT_STATE_TAG_JS ;                                
                      bScriptSkipHTMLComments = TRUE ;                                    
                   } else {                                                               
                      if ( ( ! wcsicmp( szValue, L" VBSCRIPT "      ) ) ||                
                           ( ! wcsicmp( szValue, L" TEXT/VBSCRIPT " ) ) )                 
                         sScriptState = SCRIPT_STATE_TAG_VBS ;
                   }
                   szCondAttrNames[0] = NULL ;
                   szCondAttrValues[0] = NULL ;
                }
             }
          }
       }
    }
    return bFound ;
}


/****************************************************************************/
/*                                                                          */
/* Token::CheckSkipAttrValue                                                */
/*                                                                          */
/* Get the value of the next XML token in the string, and verify whether    */
/* it's value is supposed to be translatable or not.                        */
/*                                                                          */
/****************************************************************************/
BOOL Token::CheckSkipAttrValue(void)
{
    wchar_t    szValue[XML_ATTR_VALUE_LENGTH] ;
    wchar_t    szTemp[XML_ATTR_VALUE_LENGTH] ;
    wchar_t    szTemp2[XML_ATTR_VALUE_LENGTH] ;
    wchar_t*   tbuff ;
    wchar_t    *ptrChar, *ptrChar2 ;
    int        i = (index+lasttoken) ;
    int        j, k ;
    BOOL       bFound = FALSE ;

    if ( ( szSkipAttrValues[0] ) &&
         ( i <= Length         ) ) {
       wcsncpy(szTemp, (wchar_t*)(LPCTSTR)str.subString(i),XML_ATTR_VALUE_LENGTH-1 );
       szTemp[XML_ATTR_VALUE_LENGTH-1] = NULL ;
       tbuff = wcstok(szTemp, L" \"\'\n\t\r" );
       if ( tbuff ) {
          wcsupr(tbuff);
          szValue[0] = ' ' ;
          wcsncpy( &szValue[1], tbuff, XML_ATTR_VALUE_LENGTH-1 ) ;
          szValue[XML_ATTR_VALUE_LENGTH-1] = NULL ;
          wcscat( szValue, L" " ) ;
          if ( wcsstr( szSkipAttrValues, szValue ) ) {
             bFound = TRUE ;
          } else {
             if ( wcschr( szSkipAttrValues, L'*' ) ) {           
                wcscpy( szTemp, szSkipAttrValues ) ;
                for( tbuff=wcstok(szTemp, L" " ) ;
                     tbuff && !bFound ; 
                     tbuff=wcstok(NULL, L" " ) ) {
                   if ( wcschr( tbuff, L'*' ) ) {
                      for( j=1, ptrChar=tbuff  ; 
                           *ptrChar!=L'*' && *ptrChar==szValue[j] ;
                           ++j, ++ptrChar ) ;
                      if ( *ptrChar == L'*' ) {
                         bFound = TRUE ;
                      }
                   }
                }
             }
          }
       }
    }
    if ( ( ! bFound ) &&                    /* Skip empty attributes  */
         ( ! wcsncmp( (wchar_t*)(LPCTSTR)str.subString(i-1), (wchar_t*)(LPCTSTR)str.subString(i), 1 ) ) ) {
       bFound = TRUE ;
       if ( ! wcscmp( GetCurrentTagName(), L"!ENTITY" ) )  /* Ignore the ending quote  */
          bEmptyEntityValue = TRUE ;                                  
    }
    return bFound ;
}


/****************************************************************************/
/*                                                                          */
/* Token::CheckTransStateAttr                                               */
/*                                                                          */
/* Get the value of the next XML token in the string, and verify whether    */
/* it is a value which allows translatable sections.                        */
/*                                                                          */
/****************************************************************************/
BOOL Token::CheckTransStateAttr(void)
{
    STATEATTR  *ptrTemp ;
    wchar_t    szValue[XML_ATTR_VALUE_LENGTH] ;
    wchar_t    szTemp[XML_ATTR_VALUE_LENGTH] ;
    wchar_t*   tbuff ;
    int        i = (index+lasttoken) ;
    BOOL       bFound = FALSE ;

    if ( ( szStateAttrValues[0] ) &&
         ( i <= Length         ) ) {

       wcsncpy(szTemp, (wchar_t*)(LPCTSTR)str.subString(i), XML_ATTR_VALUE_LENGTH-1);
       szTemp[XML_ATTR_VALUE_LENGTH-1] = NULL ;
       tbuff = wcstok(szTemp, L" \"\'\n\t\r" );
       if ( tbuff ) {
          wcsupr(tbuff);                    
          szValue[0] = ' ' ;
          wcsncpy( &szValue[1], tbuff, XML_ATTR_VALUE_LENGTH-1 ) ;
          szValue[XML_ATTR_VALUE_LENGTH-1] = NULL ;
          wcscat( szValue, L" " ) ;
          if ( wcsstr( szStateAttrValues, szValue ) ) {
             for( ptrTemp=ptrStateAttrHead ;
                  ptrTemp ;
                  ptrTemp=(STATEATTR*)ptrTemp->ptrNext ) {
                if ( ! wcscmp( ptrTemp->CondAttrValue, tbuff ) ) {
                   SetTransStateAttr( ptrTemp->Initial ) ;
                }
             }
             bFound = TRUE ;
          }
       }
    }
    return bFound ;
}


/****************************************************************************/
/*                                                                          */
/* Token::IsTransStateTag                                                   */
/*                                                                          */
/* Search table to determine if string is a tag which causes a change in    */
/* the basic translation state.                                             */
/*                                                                          */
/****************************************************************************/
BOOL Token::IsTransStateTag( wchar_t *szText )
{
   wchar_t     szTag[XML_TAG_LENGTH] ;
   wchar_t     szTag2[XML_TAG_LENGTH] ;
   wchar_t     szCompleteTag[XML_COMPLETE_TAG_LENGTH] ; 
   wchar_t     *ptrChar ;
   STATETAG    *ptrTemp ;
   BOOL        bFound = FALSE ;

   if ( ( ptrStateTagHead ) &&
        ( ( iswalpha(szText[1]) ) ||
          ( szText[1] == '/'    ) ) &&
        ( GetTransStateAttr() != TRANS_STATE_OFF ) ) {
      wcsncpy( szTag, &szText[1], XML_TAG_LENGTH-1 ) ;
      szTag[XML_TAG_LENGTH-1] = NULL ;
      if ( szTag[0] == L'/' ) 
         wcstok( szTag+1, L" />\n\t\r" ) ;        /* Get tag name for search */
      else
         wcstok( szTag, L" />\n\t\r" ) ;          /* Get tag name for search */

      if ( ( iswalpha(szTag[0]) ) &&
           ( ! szTransStateEndTag[0] ) ) {

         for( ptrTemp=ptrStateTagHead ; ptrTemp ;     
              ptrTemp=(STATETAG*)(ptrTemp->ptrNext) ) {
            wcscpy( szTag2, ptrTemp->Tag ) ;
            wcsupr( szTag2 ) ; 
            if ( ( ( ( ! ptrTemp->Generic ) &&         /* Tag is an exact match  */
                     ( ! wcscmp(szTag2, szTag ) ) ) ||
                   ( ( ptrTemp->Generic ) &&           /* Tag matches generic tag */
                     ( ( ! wcsncmp(szTag2, szTag, wcslen(szTag2) ) ) ||
                       ( ( szTag2[0] == L'*' ) &&     
                         ( wcslen(&szTag2[1]) < wcslen(szTag) ) &&
                         ( ! wcsncmp(&szTag2[1], &szTag[wcslen(szTag)-wcslen(&szTag2[1])], wcslen(&szTag2[1]) ) ) ) ) ) ) &&
                 ( ( ptrTemp->CondState == TRANS_STATE_NONE ) ||
                   ( ( ptrTemp->CondState == TRANS_STATE_ON ) &&
                     ( GetTransState() == TRANS_STATE_ON    ) ) ||
                   ( ( ptrTemp->CondState == TRANS_STATE_OFF ) &&
                     ( GetTransState() == TRANS_STATE_OFF    ) ) ) &&
                 ( ( ptrTemp->PrevTag[0] == NULL ) ||
                   ( ! wcsicmp( ptrTemp->PrevTag, GetCurrentTagName() ) ) ) ) {
               break ;
            }

         }
         if ( ptrTemp ) {
            ptrChar = wcschr( szText,'>' );            
            if ( ( ! ptrChar ) ||
                 ( *(ptrChar-1) != '/' ) || 
                 ( ptrTemp->TransAttrOnly ) ) {
               if ( ptrTemp->Initial == TRANS_STATE_ON )
                  SetTransState( TRANS_STATE_ON ) ;
               if ( ptrTemp->Initial == TRANS_STATE_OFF )
                  SetTransState( TRANS_STATE_OFF ) ;
               if ( ptrTemp->TransAttrOnly )           
                  bStateTransAttrOnly = TRUE ;
               if ( ptrTemp->EndTag[0] ) {
                  if ( ptrTemp->Initial == TRANS_STATE_NONE )
                     bFound = TRUE ;
                  if ( ptrTemp->Generic ) {
                     szTransStateEndTag[0] = L'/' ;
                     wcscpy( &szTransStateEndTag[1], szTag ) ;
                  } else {
                     wcscpy( szTransStateEndTag, ptrTemp->EndTag ) ;
                  }
                  if ( ! wcsicmp( szTag, L"SCRIPT") ) {
                     sScriptState = SCRIPT_STATE_TAG_UNKNOWN ;
                     wcscpy( szCondAttrNames, L" " ) ;
                     wcscat( szCondAttrNames, ptrTemp->CondAttr ) ;
                     wcscat( szCondAttrNames, L" " ) ;
                     wcsupr( szCondAttrNames ) ;
                     wcscpy( szCondAttrValues, L" " ) ;
                     wcscat( szCondAttrValues, ptrTemp->CondAttrValue ) ;
                     wcscat( szCondAttrValues, L" " ) ;
                     wcsupr( szCondAttrValues ) ;
                  }
                  if ( ptrTemp->CondAttr[0] ) {
                     wcscpy( szCondAttrNames, L" " ) ;
                     wcscat( szCondAttrNames, ptrTemp->CondAttr ) ;
                     wcscat( szCondAttrNames, L" " ) ;
                     wcsupr( szCondAttrNames ) ;
                     wcscpy( szCondAttrValues, L" " ) ;
                     wcscat( szCondAttrValues, ptrTemp->CondAttrValue ) ;
                     wcscat( szCondAttrValues, L" " ) ;
                     wcsupr( szCondAttrValues ) ;
                  }
               }
            } 
         }
      } else {
         if ( ! wcsicmp( szTransStateEndTag, szTag ) ) {
            bFound = TRUE ;
            szTransStateEndTag[0] = NULL ;
            sScriptState = SCRIPT_STATE_NONE ;
            XTbl.setoff( JS_LCOMMENT ) ;              
            XTbl.setoff( JS_ASSIGN2 ) ;               
            XTbl.setoff( JS_ASSIGN1 ) ;               
            XTbl.setoff( JS_ASSIGN ) ;                
         }
      }
   }

   return ( bFound ) ;
}


/****************************************************************************/
/*                                                                          */
/* Token::IsGlobalTag                                                       */
/*                                                                          */
/* Search table to determine if string is a global tag which is used in a   */
/* non-translatable section.                                                */
/*                                                                          */
/****************************************************************************/
BOOL Token::IsGlobalTag( wchar_t *szText )
{
   wchar_t      szTag[XML_TAG_LENGTH] ;
   wchar_t      szTag2[XML_TAG_LENGTH] ;
   GLOBALTAG *ptrTemp ;
   short     i ;
   BOOL      bFound = FALSE ;
   BOOL      bGeneric = FALSE ;

   if ( ( ptrGlobalTagHead ) &&             /* If global tags for XML,      */
        ( ( iswalpha(szText[1]) ) ||         /* and first char is alphabetic */
          ( szText[1] == '/' ) ) ) {        /* or start of end tag          */
      if ( szText[1] == '/' ) 
         i = 2 ; 
      else
         i = 1 ; 
      wcsncpy( szTag, &szText[i], XML_TAG_LENGTH-1 ) ;
      szTag[XML_TAG_LENGTH-1] = NULL ;
      wcstok( szTag, L" />\n\t\r" ) ;        /* Get tag name for search.     */

      for( ptrTemp=ptrGlobalTagHead ; ptrTemp ; 
           ptrTemp=(GLOBALTAG*)(ptrTemp->ptrNext) ) {
         wcscpy( szTag2, ptrTemp->Tag ) ;
         wcsupr( szTag2 ) ; 
         if ( ( ( ! ptrTemp->Generic ) &&
                ( ! wcscmp(szTag2, szTag ) ) ) ||
              ( ( ptrTemp->Generic ) &&
                ( ! wcsncmp(szTag2, szTag, wcslen(szTag2) ) ) ) ) 
            break ;
      }
      if ( ptrTemp )                        /* If tag found, then valid     */
         bFound = TRUE ;                    /*  global tag.                 */
   }

   return ( bFound ) ;
}



/****************************************************************************/
/*                                                                          */
/* Token::IsLinesTag                                                        */
/*                                                                          */
/* Search table to determine if string is a tag which starts line-by-line   */
/* segmentation rather than sentence segmentation.                          */
/*                                                                          */
/****************************************************************************/
BOOL Token::IsLinesTag( wchar_t *szText )
{
   wchar_t     szTag[XML_TAG_LENGTH] ;
   wchar_t     *ptrChar, *ptrChar2 ;
   LINESTAG    *ptrTemp ;
   BOOL        bFound = FALSE ;


   if ( ( ptrLinesTagHead ) &&              /* If lines tags for this XML,  */
        ( ( iswalpha(szText[1]) ) ||        /* and first char is alphabetic */
          ( szText[1] == '/'  ) ) ) {       /*     or valid end tag.        */
      wcsncpy( szTag, &szText[1], XML_TAG_LENGTH-1 ) ;
      szTag[XML_TAG_LENGTH-1] = NULL ;
      wcstok( szTag, L" >\n\t\r" ) ;         /* Get tag name for search.     */

      if ( ( iswalpha(szTag[0]) ) &&
           ( ! szLinesEndTag[0] ) ) {
         for( ptrTemp=ptrLinesTagHead ;
              ptrTemp && wcsicmp(ptrTemp->Tag, szTag) ;
              ptrTemp=(LINESTAG*)(ptrTemp->ptrNext) ) ;
         if ( ptrTemp ) {                      /* If tag found, then valid     */
            if ( ptrTemp->CondAttr[0] ) {      /* Check conditional          */
               wchar_t  szTemp[256] ;
               wcsncpy( szTemp, szText, 255 ) ;
               szTemp[255] = NULL ;
               wcsupr( szTemp ) ;
               ptrChar = wcsstr( szTemp, ptrTemp->CondAttr ) ;
               if ( ptrChar ) 
                  ptrChar = wcsstr( ptrChar, ptrTemp->CondAttrValue ) ;
               if ( ptrChar ) {
                  ptrChar2 = wcschr( szTemp, L'>' ) ;
                  if ( ptrChar2 <= ptrChar ) 
                     ptrTemp = NULL ;
               } else {
                  ptrTemp = NULL ;
               }
            }
            if ( ptrTemp ) {   
               if ( ( GetTransState() != TRANS_STATE_OFF ) &&  /* If translatable */
                    ( GetTransStateAttr() != TRANS_STATE_OFF ) ) {
                  sLinesState = LINES_STATE_ON ;
                  wcscpy( szLinesEndTag, ptrTemp->EndTag ) ;
               }
               bFound = TRUE ;                 
            }
         }
      } else {
         if ( ! wcsicmp( szLinesEndTag, szTag ) ) {
            sLinesState = LINES_STATE_OFF ;
            szLinesEndTag[0] = NULL ;
            bFound = TRUE ;                 
         }
      }
   }

   return ( bFound ) ;
}



/****************************************************************************/
/*                                                                          */
/* Token::IsIgnoreCommentTag                                                */
/*                                                                          */
/* Search table to determine if string is a tag which starts section where  */
/* comment delimiters should be ignored.                                    */
/*                                                                          */
/****************************************************************************/
BOOL Token::IsIgnoreCommentTag( wchar_t *szText )
{
   wchar_t     szTag[XML_TAG_LENGTH] ;
   LINESTAG *ptrTemp ;
   BOOL     bFound = FALSE ;


   if ( ( ptrIgnoreCommentTagHead ) &&      /* If lines tags for this XML,  */
        ( ( iswalpha(szText[1]) ) ||        /* and first char is alphabetic */
          ( szText[1] == '/' ) ) ) {        /*     or valid end tag.        */
      wcsncpy( szTag, &szText[1], XML_TAG_LENGTH-1 ) ;
      szTag[XML_TAG_LENGTH-1] = NULL ;
      wcstok( szTag, L" >\n\t\r" ) ;         /* Get tag name for search.     */

      if ( ( iswalpha(szTag[0]) ) &&
           ( ! szIgnoreCommentEndTag[0] ) ) {
         for( ptrTemp=ptrIgnoreCommentTagHead ;
              ptrTemp && wcsicmp(ptrTemp->Tag, szTag) ;
              ptrTemp=(LINESTAG*)(ptrTemp->ptrNext) ) ;
         if ( ptrTemp ) {                      /* If tag found, then valid     */
            sIgnoreCommentState = IGNORECOMMENT_STATE_ON ;
            wcscpy( szIgnoreCommentEndTag, ptrTemp->EndTag ) ;
            bFound = TRUE ;                 
         }
      } else {
         if ( ! wcsicmp( szIgnoreCommentEndTag, szTag ) ) {
            sIgnoreCommentState = IGNORECOMMENT_STATE_OFF ;
            szIgnoreCommentEndTag[0] = NULL ;
            bFound = TRUE ;                 
         }
      }
   }

   return ( bFound ) ;
}

/****************************************************************************/
/*                                                                          */
/* Token::IsCopyTextTag                                                     */
/*                                                                          */
/* Search table to determine if string is a tag where the tag's content     */
/* should be copied to the content of another tag.                          */
/*                                                                          */
/****************************************************************************/
BOOL Token::IsCopyTextTag( wchar_t *szText )
{
    wchar_t         szTag[XML_TAG_LENGTH] ;
    wchar_t         szTag2[XML_TAG_LENGTH] ;
    wchar_t         szTemp[XML_COMPLETE_TAG_LENGTH] ;
    wchar_t         *ptrChar ;
    COPYTEXTTAG     *ptrTemp ;
    short           i, j ;
    BOOL            bFound = FALSE ;


    if ( ( ptrCopyTextTagHead ) &&           /* If copy text tags for XML,   */
         ( ( iswalpha(szText[1]) ) ||         /* and first char is alphabetic */
           ( szText[1] == '/' ) ) ) {        /* or start of end tag          */
        if ( szText[1] == '/' )
            i = 2 ;
        else
            i = 1 ; 
        wcsncpy( szTag, &szText[i], XML_TAG_LENGTH-1 ) ;
        szTag[XML_TAG_LENGTH-1] = NULL ;
        wcstok( szTag, L" />\n\t\r" ) ;        /* Get tag name for search.     */

        if( wcsstr(szText, L"XML:LANG") && (sCopyTextState == COPYTEXT_STATE_FROM_START1) ) {
           bCopyXmlLang = TRUE;
           return FALSE;	     
        }
        if ( ( bXLIFF ) &&                               
             ( ! wcscmp( szTag, L"FILE" ) ) &&
             ( sCopyTextState == COPYTEXT_STATE_OFF ) ) { 
           wcscpy( szTemp, szText ) ;
           GetCompleteTag( szTemp, sizeof(szTemp), 0 ) ;
           wcsupr( szTemp ) ;
           if( wcsstr(szTemp, L"SOURCE-LANGUAGE" ) ) {
               bCopySourceLang = TRUE;
               bCopyXmlLang = FALSE ;    /* Don't add if src lang is defined, unless <source> has it */
           }
        }
      
        for ( ptrTemp=ptrCopyTextTagHead ; ptrTemp ; 
            ptrTemp=(COPYTEXTTAG*)(ptrTemp->ptrNext) ) {

            if ( ( sCopyTextState == COPYTEXT_STATE_OFF      ) ||
                 ( sCopyTextState == COPYTEXT_STATE_FROM_END ) ) {
                wcscpy( szTag2, ptrTemp->FromTag ) ;
                wcsupr( szTag2 ) ; 
                if (bSDLXLF && !wcscmp(szTag, L"SEG-SOURCE")) { 
                    sCopyTextState = COPYTEXT_STATE_IGNORE; 
                } 
                if ( ! wcscmp( szTag2, szTag ) ) {
                    sCopyTextState = COPYTEXT_STATE_FROM_START1 ;
                    ptrCopyTextTagCur = ptrTemp ;
                    lCopyTextSrcIdx = index ;
                    bFound = FALSE ;              

                    /* Start saving the text after this tag  */

                }
            }
            if ( sCopyTextState == COPYTEXT_STATE_FROM_START2 ) {
                if ( ptrCopyTextTagCur ) {
                    wcscpy( szTag2, ptrCopyTextTagCur->FromTag ) ;
                    wcsupr( szTag2 ) ; 
                    if ( ( ! wcscmp( szTag2, szTag ) ) &&
                         ( szText[1] == '/' ) ) {
                        sCopyTextState = COPYTEXT_STATE_FROM_END ;

                        wcsncpy( szTag, (wchar_t*)(LPCTSTR)str.subString(index,XML_TAG_LENGTH-1), XML_TAG_LENGTH-1 ) ;
                        szTag[XML_TAG_LENGTH-1] = NULL ;
                        wcstok( szTag, L" >\n\t\r" ) ;        /* Get tag name for search.     */
                        ptrChar = wcsstr( ptrSaveCopyText, szTag ) ;
                        if ( ptrChar )
                            *ptrChar = NULL ;

                        bFound = FALSE ;           

                        /* End saving the text before this tag  */

                    }
                }
            } else if (sCopyTextState == COPYTEXT_STATE_IGNORE) { 
                wcscpy( szTag2, L"SEG-SOURCE") ; 
                wcsupr( szTag2 ) ;  
                if ( ( ! wcscmp( szTag2, szTag ) ) && 
                     ( szText[1] == '/' ) ) { 
                    sCopyTextState = COPYTEXT_STATE_FROM_END ; 
                } 
            } else if ( sCopyTextState == COPYTEXT_STATE_FROM_END ) {
                if (bSDLXLF && GetTransStateAttr() == TRANS_STATE_OFF) { 
                    sCopyTextState = COPYTEXT_STATE_TO_START3; 
                } else { 
               ///  if( !wcsnicmp(szText,L"<target", 7) && wcsstr(szText, L"/>") ) {
                    if( !wcsnicmp(szText,L"<target", 7) ) {
                       ptrChar = wcschr( szText, L'>' ) ;
                       if ( ( ptrChar ) &&
                            ( *(ptrChar-1) == L'/' ) ) {

                          wcsncpy( szTemp, (wchar_t*)(LPCTSTR)str.subString(index), sizeof(szTemp)/sizeof(wchar_t) ) ; 
                          szTemp[sizeof(szTemp)/sizeof(wchar_t)-1] = NULL ;    
                          ptrChar = wcsstr( szTemp, L"/>" ) ;
                          *(ptrChar+2) = NULL ; 
                          str.remove(index, wcslen(szTemp));         
       
                          *ptrChar = NULL ; 
                          wcscat(szTemp, L">");
                          wcscat(szTemp,L"</target>");
                          str.insert( szTemp, index-1 ) ;            
                          Length = str.length() ;
       
                          wcsncpy( szText, (wchar_t*)(LPCTSTR)str.subString(index), 512 ) ;
                          szText[512-1] = NULL;
                          wcsupr(szText) ;
                       }

                    }
                    
                    if ( ptrCopyTextTagCur ) {
                        wcscpy( szTag2, ptrCopyTextTagCur->ToTag ) ;
                        wcsupr( szTag2 ) ; 
                        if ( ! wcscmp( szTag2, szTag ) ) {
                            sCopyTextState = COPYTEXT_STATE_TO_START1 ;
                            bFound = FALSE ;              
                            /* Insert text afer this tag */

                        } else  {  //no parent check needed. target tag is added after source tag.
                            wchar_t szTemp[XML_TAG_LENGTH];
                            sCopyTextState = COPYTEXT_STATE_TO_START1;
                            //Copy xml:lang="en" attribute to the ToTag.
                            if (bSDLXLF) {
                                if ( bCopyXmlLang ) {
                                    swprintf(szTemp, L"<%s %s></%s>", ptrCopyTextTagCur->ToTag, L"xml:lang=\"en\"", ptrCopyTextTagCur->ToTag);
                                } else {
                                    swprintf(szTemp, L"<%s></%s>", ptrCopyTextTagCur->ToTag,  ptrCopyTextTagCur->ToTag);
                                }
                            } else {
                                if ( bCopyXmlLang ) {
                                    swprintf(szTemp, L"<%s %s></%s>\r\n", ptrCopyTextTagCur->ToTag, L"xml:lang=\"en\"", ptrCopyTextTagCur->ToTag);
                                } else {
                                    swprintf(szTemp, L"<%s></%s>\r\n", ptrCopyTextTagCur->ToTag,  ptrCopyTextTagCur->ToTag);
                                }
                                j = str.length() - wcslen(szText) ;
                                for (int i=0; i < j ; i++ );
                                   wcscat(szTemp, L" ");
                            }

                            str.insert( szTemp, index-1 ) ;
                            if ( lCopyTextSrcIdx > index ) {
                               szTemp[0] = NULL ;
                               for( int i=lCopyTextSrcIdx-index ; i>0 ; --i ) {
                                  wcscat( szTemp, L" " ) ;
                               }
                               str.insert( szTemp, index-1 ) ;
                               index += lCopyTextSrcIdx - index ; 
                            }
                            Length = str.length() ;
                            wcsncpy( szText, (wchar_t*)(LPCTSTR)str.subString(index), 512 ) ;
                            szText[512-1] = NULL;
                            wcsupr(szText) ;
                        }
                        if ( bCopySourceLang ) { 
                           bCopyXmlLang = FALSE ;
                        }
                    }
                }
            } else if( sCopyTextState == COPYTEXT_STATE_TO_START2) {
                wchar_t szTmp[8192];
                int nDiff;
                BOOL bTargetText;                
                if ( ptrCopyTextTagCur ) {

                    wcscpy( szTag2, ptrCopyTextTagCur->ToTag ) ;
                    wcsupr( szTag2 ) ; 
                    if ( (! wcscmp( szTag2, szTag )) && ( szText[1] == '/' ) ) {
                        bFound = FALSE ;
                        sCopyTextState = COPYTEXT_STATE_TO_START3;                        
                        bTargetText = TRUE;
                        nDiff = lCopyTextIndex2- (Length-index);
                        szTmp[0] = NULL;
                        
                        wcscpy( szTmp, L"</" ) ;  
                        wcscat( szTmp, ptrCopyTextTagCur->ToTag ) ;
                        ptrChar = wcsstr( ptrSaveCopyText2, szTmp ) ;
                        if ( ptrChar )
                            *(ptrChar) = NULL ;

                        for(int j=0; j < wcslen(ptrSaveCopyText2); j++ ) {
                            if( (!iswspace(ptrSaveCopyText2[j]))  )
                                bTargetText = FALSE;
                        } 
                        if( bTargetText && ptrSaveCopyText && wcscmp(ptrSaveCopyText,L"") ) {
                           --index ;
                           i = index ;
                           if ( ptrCopyTextTagCur->TextBefore ) {
                              str.insert( ptrCopyTextTagCur->TextBefore, i-6, ' ' ) ;
                              i += wcslen(ptrCopyTextTagCur->TextBefore) ;
                           }
                           str.insert( ptrSaveCopyText, i, ' ' ) ;
                           if ( ptrCopyTextTagCur->TextAfter ) {
                              i += wcslen(ptrSaveCopyText) ;
                              str.insert( ptrCopyTextTagCur->TextAfter, i, ' ' ) ;
                           }
                           Length = str.length() ;
                           bFound = TRUE;

                           if ( ptrSaveCopyText ) 
                              free( ptrSaveCopyText ) ;
                           ptrSaveCopyText = NULL ;
                           if ( ptrSaveCopyText2 ) 
                              free( ptrSaveCopyText2 ) ;
                           ptrSaveCopyText2 = NULL ;
                        }
                    }
                }               
            }
        }
    }

    return( bFound ) ;
}



/****************************************************************************/
/*                                                                          */
/* Token::HandleCopyTextText                                                */
/*                                                                          */
/* When copying text from one tag to another, handle saving the text and    */
/* copying the text.                                                        */
/*                                                                          */
/****************************************************************************/
void Token::HandleCopyTextText( wchar_t* szText, wchar_t cPrev )
{
   IString   s1 ;
   wchar_t      *ptrTemp ;
   long      i ;


   if ( sCopyTextState == COPYTEXT_STATE_FROM_START1 ) { /* End of 'from' start tag */
      sCopyTextState = COPYTEXT_STATE_FROM_START2 ;
      if ( ptrSaveCopyText ) 
         free( ptrSaveCopyText ) ; 
      ptrSaveCopyText = NULL ;
      lCopyTextIndex = -(index) ;
      if ( cPrev == L'/' ) {                         /* Handle <source/> */
         sCopyTextState = COPYTEXT_STATE_FROM_END ;
         if ( ptrSaveCopyText2 ) {
            free( ptrSaveCopyText2 ) ;
            ptrSaveCopyText2 = NULL;
         }
      } else {
         i = wcslen(szText) ;
         if ( i > 1 ) {
            ptrSaveCopyText = (wchar_t*)malloc( (i+4+12)*sizeof(wchar_t) ) ;
            wcscpy( ptrSaveCopyText, &szText[1] ) ;
         }
      }
   } else 
   if ( sCopyTextState == COPYTEXT_STATE_FROM_START2 ) { /* Next record of text */
      ptrTemp = ptrSaveCopyText ;
      i = wcslen(szText) ;
      if ( ptrTemp )
         i += wcslen(ptrTemp) ;
      ptrSaveCopyText = (wchar_t*)malloc( (i+20)*sizeof(wchar_t)) ;
      if ( ptrTemp ) {
         wcscpy( ptrSaveCopyText, ptrTemp ) ;
      } else {
         *ptrSaveCopyText = NULL ;
      }
      lCopyTextIndex = wcslen(ptrSaveCopyText) ;
      wcscat( ptrSaveCopyText, ENDL ) ;      
      if ( ( ptrTemp ) &&
           ( sLinesState == LINES_STATE_ON ) ) {
         wcscat( ptrSaveCopyText, L"<twb----twb>" ) ;
      }
      wcscat( ptrSaveCopyText, szText ) ;
  
      if ( ptrTemp != NULL) { 
         free( ptrTemp ) ;
         ptrTemp = NULL;
      }
  
      if ( ptrSaveCopyText2 ) {
         free( ptrSaveCopyText2 ) ;
         ptrSaveCopyText2 = NULL;
      }
  
   } else 
   if ( (sCopyTextState == COPYTEXT_STATE_TO_START1) ) { /* Start of 'to' start tag */
       sCopyTextState = COPYTEXT_STATE_TO_START2 ;
       
       ptrSaveCopyText2 = NULL ;
       i = wcslen(szText) ;
       if ( i > 1 ) {
          ptrSaveCopyText2 = (wchar_t*)malloc( (i+3)*sizeof(wchar_t) ) ;
          wcscpy( ptrSaveCopyText2, &szText[1] ) ;
       } else {
          ptrSaveCopyText2 = (wchar_t*)malloc( (1+1)*sizeof(wchar_t) ) ;
          wcscpy( ptrSaveCopyText2, L"" ) ;
       }
       lCopyTextIndex2 = -(index) ;
   } else 
   if (sCopyTextState == COPYTEXT_STATE_TO_START2) {  /* End of 'to' start tag */
       ptrTemp = ptrSaveCopyText2 ;
       i = wcslen(szText) ;
       if ( ptrTemp )
          i += wcslen(ptrTemp) ;
       if ( i > 1 ) {
          ptrSaveCopyText2 = (wchar_t*)malloc( (i + 2)*sizeof(wchar_t) ) ;
          if ( ptrTemp )
             wcscpy( ptrSaveCopyText2, ptrTemp ) ;
          else 
             *ptrSaveCopyText2 = NULL ;
          lCopyTextIndex2 = wcslen(ptrSaveCopyText2) ;
          wcscat( ptrSaveCopyText2, szText ) ;
       } else {
          ptrSaveCopyText2 = NULL ;
       }
       if ( ptrTemp != NULL) { 
          free( ptrTemp ) ;
          ptrTemp = NULL;
       }
   } else 
   if ( sCopyTextState == COPYTEXT_STATE_TO_START3 ) { /* End of 'to' end tag */
      sCopyTextState = COPYTEXT_STATE_OFF ;

      if( ptrSaveCopyText != NULL ) {
          free( ptrSaveCopyText ) ;
          ptrSaveCopyText = NULL ;
      }
      if( ptrSaveCopyText2 != NULL ) {
          free( ptrSaveCopyText2 ) ;
          ptrSaveCopyText2 = NULL ;
      }
            
   }

   return ;
}




/****************************************************************************/
/*                                                                          */
/* Token::HandleInsertAttribute                                             */
/*                                                                          */
/* Insert specified attribute text into specific tags.                      */
/*                                                                          */
/****************************************************************************/
int Token::HandleInsertAttribute(wchar_t *szText) {
   wchar_t    szTag[XML_COMPLETE_TAG_LENGTH] ;                   
   wchar_t    szTemp[2048];
   wchar_t    szAttrValue[256];
   wchar_t    *ptrChar, *ptrChar2, *ptrChar3 ;
   int i;
   int rc=0;
   INSERTATTRTAG  *ptrTemp, *ptrTemp2;


   if( nInsertAttrTag == INSERTATTR_COND_FOUND ) {   /* Conditional attribute value */
      szAttrValue[0] = L' ' ;
      if ( szText[0] == L'\"' ) 
         wcsncpy( &szAttrValue[1], &szText[1], 254 ) ;
      else
         wcsncpy( &szAttrValue[1], szText, 254 ) ;
      szAttrValue[254] = NULL ;
      ptrChar = wcschr( szAttrValue, L'\"' ) ;
      if ( ( ptrChar ) &&
           ( ptrChar > &szAttrValue[1] ) ) {
         *(ptrChar++) = L' ' ;
         *ptrChar = NULL ;
      } else {
         szAttrValue[0] = NULL ;
      }
   }


   if( ptrInsertAttrTagHead && (nInsertAttrTag == INSERTATTR_OFF)) {
      szInsertCopyAttr[0] = NULL;
      szInsertCondAttr[0] = NULL;
      szInsertCondAttrValues[0] = NULL;

      if( ( szText[0] == '<' ) &&
          ( szText[1] != '!' ) ) {                 /* Not comment */
         for( ptrTemp=ptrInsertAttrTagHead ; ptrTemp ; 
            ptrTemp=(INSERTATTRTAG*)(ptrTemp->ptrNext) ) {
            i = wcslen(ptrTemp->Tag) ;
            if( ( ( wcsnicmp(&szText[1], ptrTemp->Tag, i) == 0 ) &&
                  ( ( iswspace(szText[i+1]) ) ||
                    ( wcschr(L"/>", szText[i+1]) ) ) ) ||
                ( wcscmp( ptrTemp->Tag, L"*" ) == 0  ) ) {        
               nInsertAttrTag = INSERTATTR_TAG_FOUND;
               if ( ( wcscmp( ptrTemp->Tag, L"*" ) != 0  ) || 
                    ( szInsertCopyAttr[0] == NULL ) ) {
                  wcscpy(szInsertCopyAttr, ptrTemp->CopyAttr);
                  wcscpy(szInsertCondAttr, ptrTemp->CondAttr);
                  wcscpy(szInsertCondAttrValues, ptrTemp->CondAttrValue);
                  if ( wcscmp( szInsertCondAttr, L"*" ) == 0 ) 
                     nInsertAttrTag = INSERTATTR_SRC_FOUND;
               }
            }
         }
      }
   } else 
   if( (nInsertAttrTag > 0) && 
       (wcsnicmp(szText, szInsertCondAttr, wcslen(szInsertCondAttr)) == 0) &&
       ( ( iswspace(szText[wcslen(szInsertCondAttr)]) ) ||
         ( wcschr(L"=", szText[wcslen(szInsertCondAttr)]) ) ) ) {
      if ( iswspace(szText[wcslen(szInsertCondAttr)]) ) {   
         for( ptrChar2=szText+wcslen(szInsertCondAttr) ; *ptrChar2 && iswspace(*ptrChar2) ; ++ptrChar2 ) ;
         if ( *ptrChar2 == L'=' ) { 
            if ( szInsertCondAttrValues[0] == NULL ) 
               nInsertAttrTag = INSERTATTR_SRC_FOUND;
            else
               nInsertAttrTag = INSERTATTR_COND_FOUND;
         }
      } else {
         if ( szInsertCondAttrValues[0] == NULL ) 
            nInsertAttrTag = INSERTATTR_SRC_FOUND;
         else 
            nInsertAttrTag = INSERTATTR_COND_FOUND;
      }
   } else 
   if( (nInsertAttrTag == INSERTATTR_COND_FOUND) &&    /* Conditional attribute value */
       ( szAttrValue[0] ) &&
       ( wcsstr( szInsertCondAttrValues, szAttrValue) ) ) { 
      nInsertAttrTag = INSERTATTR_SRC_FOUND;
   } else
   if( (nInsertAttrTag > 0) && 
       ( (wcsncmp(szText, L">",  1) == 0) ||
         (wcsncmp(szText, L"/>", 2) == 0) ) ) {     
      //insert target attributes.		
      if( nInsertAttrTag == INSERTATTR_SRC_FOUND  ) {
         swprintf(szTemp, L" %s", szInsertCopyAttr);
         str.insert( szTemp, index-1 ) ;

         Length = str.length();
         lasttoken = 0;
         index -= 1;
         nInsertAttrTag = INSERTATTR_COPIED;
         rc = 1;
      }
   } else
   if( nInsertAttrTag > 0 ) {
      wchar_t szTemp1[1024], szTemp2[1024];
      wchar_t *ptr1, *ptr2;
      wcscpy(szTemp1, szText);
      wcscpy(szTemp2, szInsertCopyAttr);
      ptr1 = wcstok(szTemp1, L" =");
      ptr2 = wcstok(szTemp2, L" =");
      if ( (ptr1 != NULL ) && (ptr2 != NULL ) && !wcsicmp(ptr1, ptr2) ) {
          nInsertAttrTag = INSERTATTR_OFF;
      }
   }

   return rc;
}




/****************************************************************************/
/*                                                                          */
/* Token::RemoveInsertAttribute                                             */
/*                                                                          */
/* Remove inserted attribute text from specific tags.                       */
/*                                                                          */
/****************************************************************************/
void Token::RemoveInsertAttribute( void ) {
   wchar_t szTemp[XML_NAMES_LENGTH];
   wchar_t szTemp2[XML_NAMES_LENGTH];
   INSERTATTRTAG  *ptrTemp;
 
   wcscpy( szTemp2, L"" ) ;
   for( ptrTemp=ptrInsertAttrTagHead ; ptrTemp ; 
      ptrTemp=(INSERTATTRTAG*)(ptrTemp->ptrNext) ) {
      if ( ptrTemp->Remove ) {
         wcscpy( szTemp, L" " ) ;
         wcscat( szTemp, ptrTemp->CopyAttr ) ;
         if ( wcscmp( szTemp, szTemp2 ) ) {
            str.change( szTemp, L"" ) ;
            wcscpy( szTemp2, szTemp ) ;
         }
      }
   }
 
   return;
}


/****************************************************************************/
/*                                                                          */
/* Token::FindCondAttribute                                                 */
/*                                                                          */
/* Look for conditional attribute which may be coded after the current      */
/* attribute being processed.                                               */
/*                                                                          */
/****************************************************************************/
void Token::FindCondAttribute( wchar_t *Record, wchar_t PrevChar, wchar_t *AttrNames ) 
{
    wchar_t    szTag[XML_COMPLETE_TAG_LENGTH] ;                
    wchar_t    szTag2[XML_COMPLETE_TAG_LENGTH] ;               
    wchar_t    szValue[XML_ATTR_VALUE_LENGTH] ;
    wchar_t    szAttr[XML_ATTR_LENGTH] ;
    wchar_t    cPrevChar ;
    wchar_t    *ptrChar, *ptrChar2 ;
    int        i ;     
    int        State ;


    szTag[0] = L' ' ;
    if ( Record ) {
       wcsncpy( &szTag[1], Record, (sizeof(szTag)/sizeof(wchar_t))-2 ) ;
       szTag[0] = PrevChar ;
    } else {                                      
       wcsncpy( &szTag[1], (wchar_t*)(LPCTSTR)str.subString(index+lasttoken), (sizeof(szTag)/sizeof(wchar_t))-2 ) ;
       if ( index+lasttoken > 0 ) 
          szTag[0] = (wchar_t)(LPCTSTR)str.subString(index+lasttoken-1) ;
       else 
          szTag[0] = L' ' ;
    }
    szTag[sizeof(szTag)/sizeof(wchar_t)-1] = NULL;
    GetCompleteTag( szTag, sizeof(szTag), 0 ) ;                 
    wcsupr( szTag ) ;

    /**********************************************************************/
    /*  Determine if the conditional attribute is defined in the rest     */
    /*  of the tag.                                                       */
    /**********************************************************************/

    for( ptrChar=szTag,State=0 ; *ptrChar ; ++ptrChar ) {
       if ( iswspace( *ptrChar ) )
          continue;
       if ( ( State == 0 ) &&
            ( IsAttribute(ptrChar,&i,*(ptrChar-1),AttrNames ) ) ) {
          wcsncpy( szAttr, ptrChar, XML_ATTR_LENGTH-1 ) ;
          szAttr[XML_ATTR_LENGTH-1] = NULL;
          wcstok( szAttr, L" =/>\n\t\r" ) ;       /* Get attribute name */
          State = 1 ;
          ptrChar += i - 1 ;
          XTbl.setoff(CONDATTR) ;
          continue ;
       }
       if ( ( State == 1 ) &&
            ( *ptrChar == L'=' ) ) {
          State = 2 ;
          continue ;
       }
       if ( ( State == 2 ) &&
            ( ( *ptrChar == L'\"' ) ||
              ( *ptrChar == L'\'' ) ) ) {
          State = 3 ;
          if ( *(ptrChar+1) == *ptrChar ) {
             szValue[0] = NULL ;
          } else {
             wcscpy( szTag2, ptrChar+1 ) ;
             ptrChar2 = wcstok( szTag2, L" \"\'\n\t\r" );
             if ( ptrChar2 ) {
                szValue[0] = L' ' ;
                wcsncpy( &szValue[1], ptrChar2, XML_ATTR_VALUE_LENGTH-1 ) ;
                szValue[XML_ATTR_VALUE_LENGTH-1] = NULL ;
                wcscat( szValue, L" " ) ;
                if ( wcsstr( szCondAttrValues, szValue ) ) {
                   if ( XTbl.on(CONDATTR) )         
                      XTbl.seton(CONDATTR) ;
                   break ;
                }
             } 
          }
          State = 0 ;
          continue ;
       }
       
       /************************************************************************/
       /* Handle "type" attribute values not in quotes in XHTML <input> tag    */
       /************************************************************************/
       if ( ( State == 2 ) && 
            ( bXHTML ) && 
            ( !wcsicmp(szAttr, L"type") ) ){
           State = 3 ;          
           wcscpy( szTag2, ptrChar ) ;
           ptrChar2 = wcstok( szTag2, L" \n\t\r" );
           if ( ptrChar2 ) {
              szValue[0] = L' ' ;
              wcsncpy( &szValue[1], ptrChar2, XML_ATTR_VALUE_LENGTH-1 ) ;
              szValue[XML_ATTR_VALUE_LENGTH-1] = NULL ;
              wcscat( szValue, L" " ) ;
              if ( wcsstr( szCondAttrValues, szValue ) ) {
                 if ( XTbl.on(CONDATTR) ){          
                    XTbl.seton(CONDATTR) ;
           		 }
              }
           }
           State = 0 ;
           continue ;                     
       }
       
       State = 0 ;
    }

    return ;
}


/****************************************************************************/
/*                                                                          */
/* Token::FindTransStateAttribute                                           */
/*                                                                          */
/* Look for translation state attr which may be coded after the current     */
/* attribute being processed.                                               */
/*                                                                          */
/****************************************************************************/
void Token::FindTransStateAttribute( wchar_t *Record ) 
{
    wchar_t    szTag[XML_COMPLETE_TAG_LENGTH] ;     
    wchar_t    szTag2[XML_COMPLETE_TAG_LENGTH] ;    
    wchar_t    szValue[XML_ATTR_VALUE_LENGTH] ;
    wchar_t    szAttr[XML_ATTR_LENGTH] ;
    wchar_t    *ptrChar, *ptrChar2 ;
    int        i ;     
    int        State ;
    BOOL       bFound = FALSE ;
    STATEATTR  *ptrTemp;


    if ( ! ptrStateAttrHead ) 
       return ;

    szTag[0] = L' ' ;
    if ( Record ) {
       wcsncpy( &szTag[0], Record, (sizeof(szTag)/sizeof(wchar_t))-1 ) ;
    } else {                                         
       wcsncpy( &szTag[0], (wchar_t*)(LPCTSTR)str.subString(index+lasttoken), (sizeof(szTag)/sizeof(wchar_t))-1 ) ;
    }
    szTag[sizeof(szTag)/sizeof(wchar_t)-1] = NULL;
    GetCompleteTag( szTag, sizeof(szTag), 0 ) ;                  
    wcsupr( szTag ) ;


    /**********************************************************************/
    /*  Determine if the state attribute is defined in the rest           */
    /*  of the tag.                                                       */
    /**********************************************************************/

    for( ptrChar=szTag,State=0 ; *ptrChar && bFound==0 ; ++ptrChar ) {
       if ( iswspace( *ptrChar ) )
          continue;
       if ( ( State == 0 ) &&
            ( IsAttribute(ptrChar,&i,*(ptrChar-1),szStateAttrNames ) ) ) {
          wcsncpy( szAttr, ptrChar, XML_ATTR_LENGTH-1 ) ;
          szAttr[XML_ATTR_LENGTH-1] = NULL;
          wcstok( szAttr, L" =/>\n\t\r" ) ;       /* Get attribute name */
          State = 1 ;
          ptrChar += i - 1 ;
          continue ;
       }
       if ( ( State == 1 ) &&
            ( *ptrChar == L'=' ) ) {
          State = 2 ;
          continue ;
       }
       if ( ( State == 2 ) &&
            ( ( *ptrChar == L'\"' ) ||
              ( *ptrChar == L'\'' ) ) ) {
          State = 3 ;
          if ( *(ptrChar+1) == *ptrChar ) {
             szValue[0] = NULL ;
          } else {
             wcscpy( szTag2, ptrChar+1 ) ;
             ptrChar2 = wcstok( szTag2, L" \"\'\n\t\r" );
             if ( ptrChar2 ) {
                szValue[0] = L' ' ;
                wcsncpy( &szValue[1], ptrChar2, XML_ATTR_VALUE_LENGTH-1 ) ;
                szValue[XML_ATTR_VALUE_LENGTH-1] = NULL ;
                wcscat( szValue, L" " ) ;
                if ( wcsstr( szStateAttrValues, szValue ) ) {
                   ptrChar = wcstok( szValue, L" " ) ;
                   for( ptrTemp=ptrStateAttrHead ;
                        ptrTemp ;
                        ptrTemp=(STATEATTR*)ptrTemp->ptrNext ) {
                      if ( ( ! wcscmp( ptrTemp->CondAttr, szAttr       ) ) &&
                           ( ! wcscmp( ptrTemp->CondAttrValue, ptrChar ) ) ) {
                         SetTransStateAttr( ptrTemp->Initial ) ;
                      }
                   }
                   bFound = TRUE ;
                }
             } 
          }
          State = 0 ;
          continue ;
       }
       State = 0 ;
    }

    return ;
}


/****************************************************************************/
/*                                                                          */
/* Token::HandleBidiAttr                                                    */
/*                                                                          */
/* Handle changing Bidi attribute values.                                   */
/*                                                                          */
/****************************************************************************/
void Token::HandleBidiAttr(void)
{
    wchar_t    *tbuff, *tEndQuote, *tFound ;
    short      i = (index+lasttoken-1) ;


    if ( ! wcscmp( GetCurrentAttrName(),L"ALIGN" ) ) {
       if ( ! wcsicmp( str.subString(i+1,4), L"LEFT" ) ) {
          str.remove( i+1, 4 );
          str.insert(L"right", i ) ;
          ++Length  ;
       } else
       if ( ! wcsicmp( str.subString(i+1,5), L"RIGHT" ) ) {
          str.remove( i+1, 5 );
          str.insert( L"left", i ) ;
          --Length  ;
       } 
    } else
    if ( ! wcscmp( GetCurrentAttrName(),L"DIR" ) ) {
       if ( ( ! wcscmp( GetCurrentTagName(),L"HTML" ) ) &&
            ( ! wcsicmp( str.subString(i+1,3), L"LTR" ) ) ) {
          str.remove( i+1, 3 );
          str.insert( L"rtl", i ) ;
       }
    }

    return ;
}


/****************************************************************************/
/*                                                                          */
/* GetTagAttr                                                               */
/*                                                                          */
/* Determine if the specified attribute exists in this tag, and if so,      */
/* return the attribute's value.                                            */
/*                                                                          */
/* Input:      szTag       - The entire tag to process.                     */
/*             szAttr      - The attribute keyword to find.                 */
/*             szAttrValue - The attribute's value to return.               */
/* Returns:    TRUE        - Attribute was found.                           */
/*             FALSE       - Attribute was not found.                       */
/*                                                                          */
/****************************************************************************/
BOOL Token::GetTagAttr( wchar_t  *szTag, wchar_t  *szAttr, wchar_t  *szAttrValue )
{
   wchar_t   AttrQuote ;
   wchar_t   *StartAttr, *EndAttr ;
   wchar_t   *ptr, *ptr2 ;
   BOOL      bAttrFound = FALSE ;

   szAttrValue[0] = NULL ;
   for( ptr=wcsstr(szTag,szAttr ) ;             
        ptr && !bAttrFound ; 
        ptr=wcsstr(ptr+1,szAttr ) ) {
      if ( iswspace( *(ptr-1) ) ) {
         for( ptr+=wcslen(szAttr) ; *ptr && iswspace(*ptr) ; ++ptr ) ;
         if ( *ptr == L'=' ) {
            ++ptr ;
            for( ; *ptr && iswspace(*ptr) ; ++ptr ) ;
            if ( ( *ptr == L'\'' ) ||
                 ( *ptr == L'\"' ) ) {
               StartAttr = ptr ;
               AttrQuote = *ptr ;
               for( ptr2=ptr+1 ; *ptr2 && (*ptr2!=AttrQuote) ; ++ptr2 ) {
                  if ( *ptr2 == L'\\' ) 
                     ++ptr2 ;
               }
               if ( *ptr2 == AttrQuote ) {
                  EndAttr = ptr2 ;
                  wcsncpy( szAttrValue, StartAttr+1, EndAttr-StartAttr-1 ) ;
                  szAttrValue[EndAttr-StartAttr-1] = NULL ;
                  bAttrFound = TRUE ;
               }
            }
         } 
      }
   }
   return( bAttrFound ) ;
}

/****************************************************************************/
/*                                                                          */
/* Token::HandleXmlLang                                                     */
/*                                                                          */
/* Handle changing the XML:LANG attribute value when it is defined on the   */
/* root tag of an XML file.                                                 */
/*                                                                          */
/****************************************************************************/
BOOL Token::HandleXmlLang( wchar_t *Buffer, short sBufferLen, 
                           IString &Record, int iIndex, int iXmlLength )
{
    FILE       *fTemp ;
    char       szFilePath[256] ;   
    char       szRcd[100] ;
    char       *p1, *p2 ;
    wchar_t    szText[2048] ;
    wchar_t    szLocale[20] ;
    wchar_t    *ptr, *ptr2;
    int        i ;             
    long       save_pos ;
    BOOL       bReturn= FALSE ;


    _searchenv( "OTMXML.LCL", "PATH", szFilePath );
    if ( ! szFilePath[0] ) {
       strcpy( szFilePath, szProgPath ) ;
       p1 = strrchr( szFilePath, '\\' ) ;
       if ( p1 ) 
          strcpy( p1, "\\TABLE\\OTMXML.LCL" ) ;
       else
          szFilePath[0] = 0 ;
    }
    if ( szFilePath[0] ) {
       fTemp = fopen( szFilePath, "r" ) ;
       if ( fTemp ) {
          while( fgets( szRcd, sizeof(szRcd), fTemp ) ) {
             p1 = strtok( szRcd, " =\n\t\r" ) ;
             p2 = strtok( NULL, " =\n\t\r" ) ;
             if ( p1 && p2 &&
                  ( ! stricmp( p1, szDocTargetLanguage ) ) ) {
                mbstowcs( szLocale, p2, sizeof(szLocale)/sizeof(wchar_t) ) ;

                wcsncpy( szText, (wchar_t*)(LPCTSTR)Record.subString(iIndex), sizeof(szText)/sizeof(wchar_t) ) ;
                szText[sizeof(szText)/sizeof(wchar_t)-1] = NULL;
                ptr = wcschr( szText, '>' ) ;

                if ( ptr ) {
                   *(ptr+1) = NULL ;
                }

                wcsupr( szText ) ;

                /**********************************************************************/
                /*  In the XML:LANG attribute, find the value.                        */
                /**********************************************************************/
                for( ptr=szText+iXmlLength ; *ptr!=NULL && iswspace(*ptr) ; ++ptr ) ;
                if ( *ptr == '=' ) {
                   for( ++ptr ; *ptr && iswspace(*ptr) ; ++ptr ) ;
                   if ( wcschr( L"\"\'", *ptr ) ) {
                      ptr2 = wcschr( ptr+1, *ptr ) ;
                      if ( ( ptr2 ) &&
                           ( *ptr2 == *ptr ) ) {
                         ++ptr ;
                         i = iIndex + (ptr-szText) ;
                         if ( ( ptr2 == ptr + 2 ) &&       /* 2-char XML:LANG */
                              ( stricmp( szDocTargetLanguage, "Chinese(simpl.)" ) ) &&
                              ( stricmp( szDocTargetLanguage, "Chinese(trad.)" ) ) ) 
                            szLocale[2] = NULL ;
                         str.remove( i, ptr2-ptr ) ;
                         str.insert( szLocale, i-1, ' ' ) ;
                         Length = Record.length() ;
                         bReturn = TRUE ;
                      }
                   }
                }
                break ;
             }
          }
          fclose( fTemp ) ;
       }
    }






    
    return( bReturn ) ;
}


/****************************************************************************/
/*                                                                          */
/* Token::SaveCurrentTagName                                                */
/*                                                                          */
/* Save the name of the current tag for future use.                         */
/*                                                                          */
/****************************************************************************/
void Token::SaveCurrentTagName( )
{

   wcsncpy( szCurrentTagName, (wchar_t*)(LPCTSTR)str.subString(index+1), XML_TAG_LENGTH ) ;
   szCurrentTagName[XML_TAG_LENGTH-1] = NULL;
   wcsupr(szCurrentTagName) ;
   if ( szCurrentTagName[0] == '/' )
      wcstok( szCurrentTagName+1, L" />\n\t\r" ) ;        /* Get tag name    */
   else
      wcstok( szCurrentTagName, L" />\n\t\r" ) ;          /* Get tag name    */

   return ;
}


/****************************************************************************/
/*                                                                          */
/* Token::SetCurrentTagName                                                 */
/*                                                                          */
/* Set the name of the current tag for future use.                          */
/*                                                                          */
/****************************************************************************/
void Token::SetCurrentTagName( wchar_t *TagName )
{
   wcscpy( szCurrentTagName, TagName ) ;
   return ;
}


/****************************************************************************/
/*                                                                          */
/* Token::GetCurrentTagName                                                 */
/*                                                                          */
/* Return the name of the current tag.                                      */
/*                                                                          */
/****************************************************************************/
wchar_t* Token::GetCurrentTagName( )
{
   return (szCurrentTagName) ;
}


/****************************************************************************/
/*                                                                          */
/* Token::SaveCurrentAttrName                                               */
/*                                                                          */
/* Save the name of the current attribute for future use.                   */
/*                                                                          */
/****************************************************************************/
void Token::SaveCurrentAttrName( )
{

   wcsncpy( szCurrentAttrName, (wchar_t*)(LPCTSTR)str.subString(index), XML_ATTR_LENGTH-1 ) ;
   szCurrentAttrName[XML_ATTR_LENGTH-1] = NULL;
   wcsupr(szCurrentAttrName) ;
   wcstok( szCurrentAttrName, L" =/>\n\t\r" ) ;       /* Get attribute name */

   return ;
}


/****************************************************************************/
/*                                                                          */
/* Token::ResetCurrentAttrName                                              */
/*                                                                          */
/* Not in an attribute, so delete the name.                                 */
/*                                                                          */
/****************************************************************************/
void Token::ResetCurrentAttrName( )
{

   szCurrentAttrName[0] = NULL ;

   return ;
}


/****************************************************************************/
/*                                                                          */
/* Token::GetCurrentAttrName                                                */
/*                                                                          */
/* Return the name of the current attribute.                                */
/*                                                                          */
/****************************************************************************/
wchar_t* Token::GetCurrentAttrName( )
{
   return (szCurrentAttrName) ;
}


/****************************************************************************/
/*                                                                          */
/* Token::ManageLevelTransStateAttr                                         */
/*                                                                          */
/* Add a new tag to the nesting level for this translation state change     */
/* based on an attribute value.                                             */
/*                                                                          */
/****************************************************************************/
void Token::ManageLevelTransStateAttr( BOOL *NeutralTag, BOOL PrevNeutralTag )
{
   STATEATTRLVL   *ptrSALTemp ;

   if ( ( GetTransStateAttr() > TRANS_STATE_NONE ) &&
        ( GetTransState() <= TRANS_STATE_ON ) ) {
      if ( szCurrentTagName[0] == '/' ) {
        if ( ptrStateAttrLevelHead ) {
           ptrSALTemp = ptrStateAttrLevelHead ;
           ptrStateAttrLevelHead = (STATEATTRLVL*)ptrSALTemp->ptrNext ;
           if ( ptrStateAttrLevelHead ) {
              SetTransStateAttr( ptrStateAttrLevelHead->State ) ;
              sPrevTransStateAttr = ptrStateAttrLevelHead->State ;
           } else {
              SetTransStateAttr( TRANS_STATE_NONE ) ;
              sPrevTransStateAttr = TRANS_STATE_NONE ;
           }
           *NeutralTag = ptrSALTemp->Neutral ;
           free( ptrSALTemp ) ; 
        }
      } else {
        ptrSALTemp = (STATEATTRLVL*)malloc( sizeof(STATEATTRLVL)*sizeof(wchar_t) ) ;
        swprintf( ptrSALTemp->EndTag, L"/%s", szCurrentTagName ) ;
           ptrSALTemp->State = sTransStateAttr ;
        ptrSALTemp->Neutral = PrevNeutralTag ;
        ptrSALTemp->ptrNext = ptrStateAttrLevelHead ;
        ptrStateAttrLevelHead = ptrSALTemp ;

        sPrevTransStateAttr = sTransStateAttr ;   
      }
   }

   return ;
}


/****************************************************************************/
/*                                                                          */
/* Token::GetTransState                                                     */
/*                                                                          */
/* Return translation state.                                                */
/*                                                                          */
/****************************************************************************/
int Token::GetTransState(void)
{
    return( sTransState ) ;
}


/****************************************************************************/
/*                                                                          */
/* Token::SetTransState                                                     */
/*                                                                          */
/* Set translation state.                                                   */
/*                                                                          */
/****************************************************************************/
void Token::SetTransState( short sNewState )
{
    sTransState = sNewState ;
    return ;
}


/****************************************************************************/
/*                                                                          */
/* Token::GetTransStateAttr                                                 */
/*                                                                          */
/* Return translation state attribute.                                      */
/*                                                                          */
/****************************************************************************/
int Token::GetTransStateAttr(void)
{
    return( sTransStateAttr ) ;
}


/****************************************************************************/
/*                                                                          */
/* Token::SetTransStateAttr                                                 */
/*                                                                          */
/* Set translation state attribute.                                         */
/*                                                                          */
/****************************************************************************/
void Token::SetTransStateAttr( short sNewState )
{
    if ( sTransStateAttr != TRANS_STATE_ATTR_CHECK )
       sPrevTransStateAttr = sTransStateAttr ;
    sTransStateAttr = sNewState ;
    return ;
}


/****************************************************************************/
/*                                                                          */
/* Token::GetScriptState                                                    */
/*                                                                          */
/* Return <SCRIPT> state.                                                   */
/*                                                                          */
/****************************************************************************/
int Token::GetScriptState(void)
{
    return( sScriptState ) ;
}


/****************************************************************************/
/*                                                                          */
/* Token::SetScriptState                                                    */
/*                                                                          */
/* Set <SCRIPT> state.                                                      */
/*                                                                          */
/****************************************************************************/
void Token::SetScriptState( short sNewState )
{
    sScriptState = sNewState ;
    return ;
}


/****************************************************************************/
/*                                                                          */
/* Token::AdjustLastToken                                                   */
/*                                                                          */
/* Adjust last token starting position.                                     */
/*                                                                          */
/****************************************************************************/
void Token::AdjustLastToken( short sAdjust )
{
    lasttoken += sAdjust ;
    return ;
}

/****************************************************************************/
/*                                                                          */
/* Token::SetNeutralBreakOn                                                 */
/*                                                                          */
/* Set flag that indicates if the next token is a neutral tag, then this    */
/* tag should break the segmentation.                                       */
/*                                                                          */
/****************************************************************************/
void Token::SetNeutralBreakOn( void )
{
    bPrevNeutralBreak = bNeutralBreak ;
    bNeutralBreak = TRUE ;    
    return ;
}

/****************************************************************************/
/*                                                                          */
/* Token::SetNeutralBreakOff                                                */
/*                                                                          */
/* Set flag that indicates if the next token is a neutral tag, then this    */
/* tag should not break the segmentation.                                   */
/*                                                                          */
/****************************************************************************/
void Token::SetNeutralBreakOff( void )
{
    bPrevNeutralBreak = bNeutralBreak ;
    bNeutralBreak = FALSE ;    
    return ;
}


/****************************************************************************/
/*                                                                          */
/* Token::AddLineBreak                                                      */
/*                                                                          */
/* Force a temporary line break.                                            */
/*                                                                          */
/****************************************************************************/
void Token::AddLineBreak(void)
{
    str.insert(L"<twbnl\r\n>",index+lasttoken-1) ;
    Length += 9 ;
    lasttoken += 9 ;
}


/****************************************************************************/
/*                                                                          */
/* Token::ReplacePeriod                                                     */
/*                                                                          */
/* Replace sentence-ending period in a neutral translatable attribute       */
/* so that sentence is not broken.                                          */
/*                                                                          */
/****************************************************************************/
void Token::ReplacePeriod(void)
{
    short      i = (index+lasttoken-1) ;

    str.remove( i, 1 ) ;
    str.insert(L"&TWBPER;",i-1) ;
    Length += 7 ;
    lasttoken += 7 ;
}


/****************************************************************************/
/*                                                                          */
/* Token::GetCompleteTag                                                    */
/*                                                                          */
/* Get the complete tag if it exists on multiple lines.                     */
/*                                                                          */
/****************************************************************************/
void Token::GetCompleteTag( wchar_t *szTag, long lTagSize, long lSpecial )
{
   wchar_t         szTemp[3000] ;                       
   wchar_t         *ptrChar, *ptrChar2, *ptrChar3 ;
   wchar_t         cQuote = NULL ;
   long            save_pos ;
   short           i ;
   BOOL            bIgnoreQuotedText = FALSE ;
   BOOL            bPrevCharEqual = FALSE ;

   if ( szTag[0] == L'<' ) 
      bIgnoreQuotedText = TRUE ;

   for( ptrChar=szTag ; *ptrChar ; ++ptrChar ) {
      if ( ( ( bIgnoreQuotedText ) ||
             ( bPrevCharEqual    ) ||
             ( cQuote != NULL    ) ) &&
           ( ( *ptrChar == L'\"' ) ||
             ( *ptrChar == L'\'' ) ) ) {
         bPrevCharEqual = FALSE ;
         if ( cQuote == NULL ) 
            cQuote = *ptrChar ;
         else
            if ( cQuote == *ptrChar ) 
               cQuote = NULL ; 
      }
      if ( ( cQuote == NULL   ) &&
           ( *ptrChar == L'=' ) ) {
         bPrevCharEqual = TRUE ; 
      } else {
         if ( ( bPrevCharEqual     ) &&
              ( ! iswspace(*ptrChar) ) ) 
            bPrevCharEqual = FALSE ;
      }
      if ( ( cQuote == NULL   ) &&
           ( *ptrChar == L'>' ) )
         break ;
   }
   if ( ( *ptrChar == L'>' ) &&
        ( lSpecial == 0    ) ) {
      *(ptrChar+1) = NULL ;
   } else {
      /******************************************************************/
      /*  Read ahead to find end of this tag                            */
      /******************************************************************/
      save_pos = (*InputFile).ftellt() ;        /* Save file position   */
      for( i=0 ; i<15 ; ++i ) {
         if ( ( wcslen(szTag) < lTagSize/sizeof(wchar_t) ) &&
              ( (*InputFile).getline(szTemp,(INT)sizeof(szTemp)/sizeof(wchar_t),bUTF16) != NULL ) ) {
            szTemp[sizeof(szTemp)/sizeof(wchar_t)-wcslen(szTag)-2] = NULL ;
            if ( ! bTruncatedRead )    
               wcscat( szTag, L" " ) ;
            wcscat( szTag, szTemp ) ;

            cQuote = NULL ;
            bPrevCharEqual = FALSE ;
            for( ptrChar=szTag ; *ptrChar ; ++ptrChar ) {
               if ( ( ( bIgnoreQuotedText ) ||
                      ( bPrevCharEqual    ) ||
                      ( cQuote != NULL    ) ) &&
                    ( ( *ptrChar == L'\"' ) ||
                      ( *ptrChar == L'\'' ) ) ) {
                  bPrevCharEqual = FALSE ;
                  if ( cQuote == NULL ) 
                     cQuote = *ptrChar ;
                  else
                     cQuote = NULL ; 
               }
               if ( cQuote == NULL ) {
                  if ( *ptrChar == L'=' ) {
                     bPrevCharEqual = TRUE ; 
                  } else {
                     if ( ( bPrevCharEqual     ) &&
                          ( iswspace(*ptrChar) ) ) 
                        bPrevCharEqual = FALSE ;
                  }
                  if ( *ptrChar == L'>' ) 
                     break ;
               }
            }
            if ( *ptrChar == L'>' ) {
               if ( lSpecial == 1 ) {
                  if ( szTag[1] != L'/' ) {        /* Not an end tag */
                     for( ptrChar2=ptrChar+1 ;
                          *ptrChar2 && iswspace(*ptrChar2) ; 
                          wmemmove( ptrChar2, ptrChar2+1, wcslen(ptrChar2+1)+1 ) ) ;
                     if ( *ptrChar2 == L'<' ) {
                        if ( *(ptrChar2+1) == L'?' ) {   /* Skip <?Pub ... ?> */
                           ptrChar3 = wcsstr( ptrChar2, L"?>" ) ; 
                           if ( ( ptrChar3 ) &&
                                ( *(ptrChar3+2) == L'<' ) ) {
                              ptrChar2 = ptrChar3 + 1 ;
                           }
                        }
                        if ( *(ptrChar2+1) == L'/' ) {  /* If end tag */
                           ptrChar2 = wcschr( ptrChar2, L'>' ) ;
                           if ( ptrChar2 ) 
                              ptrChar = ptrChar2 ; 
                        }
                     }
                  }
               }
               *(ptrChar+1) = NULL ;
               break ;
            }
         } else {
            break ;
         }
      }
      (*InputFile).fseekt( save_pos, SEEK_SET ) ;  /* Reset for next read */
   }

   return;
}


/****************************************************************************/
/*                                                                          */
/* Token::CheckNonTrans2K                                                   */
/*                                                                          */
/* Break a long non-translatable block based on 2K segment limit.           */
/*                                                                          */
/****************************************************************************/
void Token::CheckNonTrans2K( int iIndex, int *iUpdateIdx )
{
   long            i ;

   if ( iIndex > trsoffidx+1800 ) {
      for( i=trsoffidx+1800 ; i<iIndex ; i+=1800 ) {
         str.insert(L"--twb><twb--",i) ;
         Length += 12 ;
         *iUpdateIdx += 12 ;
      }
   }

   return;
}

/****************************************************************************/
/*                                                                          */
/* Token::IsDBCSSentenceEnd                                                 */
/*                                                                          */
/*  Determine if the current DBCS character is a sentence ending character  */
/*  or not.  Index is intentionally the 2nd byte of the DBCS character.     */
/*  Return TRUE if is DBCS sentence ending char.                            */
/****************************************************************************/
BOOL  Token::IsDBCSSentenceEnd( char *szInput, short sIndex )
{
 static BOOL     bDBCSInfo  = FALSE ;
 static BOOL     bJapanese  = FALSE ;
 static BOOL     bKorean    = FALSE ;
 static BOOL     bSChinese  = FALSE ;
 static BOOL     bTChinese  = FALSE ;
 static BOOL     bNoDBCS    = FALSE ;

   char     szDBCSChar[3] ;
   ULONG    CpList[8] ;
   ULONG    CpSize ;
   USHORT   usCodePage ;

   /*------------------------------------------------------------------------*/
   /*  If need to determine whether this is for DBCS, then get the code page */
   /*  information for this session.                                         */
   /*------------------------------------------------------------------------*/
   if ( ! bDBCSInfo ) {
      bDBCSInfo = TRUE ;
      if ( sTPVersion >= TP_60 ) { 
         if ( ( ! stricmp( szDocTargetLanguage, "Japanese" ) ) ||
              ( ! stricmp( szDocSourceLanguage, "Japanese" ) ) ) {
            bJapanese = TRUE ;
         } else
         if ( ( ! stricmp( szDocTargetLanguage, "Korean" ) ) ||
              ( ! stricmp( szDocSourceLanguage, "Korean" ) ) ) {
            bKorean = TRUE ;
         } else
         if ( ( ! stricmp( szDocTargetLanguage, "Chinese(simpl.)" ) ) ||
              ( ! stricmp( szDocSourceLanguage, "Chinese(simpl.)" ) ) ) {
            bSChinese = TRUE ;
         } else
         if ( ( ! stricmp( szDocTargetLanguage, "Chinese(trad.)" ) ) ||
              ( ! stricmp( szDocSourceLanguage, "Chinese(trad.)" ) ) ) {
            bTChinese = TRUE ;
         } else
            bNoDBCS = TRUE ;
      } else {
         usCodePage = GetKBCodePage() ;
         if ( ( usCodePage == 932 ) ||
              ( usCodePage == 942 ) ||
              ( usCodePage == 943 ) )
            bJapanese = TRUE ;
         else
            if ( ( usCodePage == 949 ) ||
                 ( usCodePage == 934 ) ||
                 ( usCodePage == 944 ) )
               bKorean = TRUE ;
            else
               if ( ( usCodePage == 1381 ) ||
                    ( usCodePage == 936  ) ||
                    ( usCodePage == 946  ) )
                  bSChinese = TRUE ;
               else
                  if ( usCodePage == 950 )
                     bTChinese = TRUE ;
                  else
                     bNoDBCS = TRUE ;
      }
   }

   /*------------------------------------------------------------------------*/
   /*  Determine if the current DBCS char (index is to 2nd byte of char) is  */
   /*  a sentence-ending character.                                          */
   /*------------------------------------------------------------------------*/
   if ( bNoDBCS ||
        sIndex==0 )
      return FALSE ;

   strncpy( szDBCSChar, &szInput[sIndex-1], 2 ) ;
   szDBCSChar[2] = NULL ;
   if ( bJapanese &&                          /* Japanese                    */
        ( !strcmp( szDBCSChar, "。" ) ||      /* x'8142', J-period           */
          !strcmp( szDBCSChar, "．" ) ||      /* x'8144', period             */
          !strcmp( szDBCSChar, "？" ) ||      /* x'8148', question mark      */
          !strcmp( szDBCSChar, "！" ) ) &&    /* x'8149', exclamation mark   */
        DBCSCheckChar( szInput, sIndex-1 ) )
      return TRUE ;
   else
   if ( bKorean &&                            /* Korean                      */
        ( !strcmp( szDBCSChar, "｡｣" ) ||      /* x'A1A3', full stop          */
          !strcmp( szDBCSChar, "｣ｮ" ) ||      /* x'A3AE', period             */
          !strcmp( szDBCSChar, "｣ｿ" ) ||      /* x'A3BF', question mark      */
          !strcmp( szDBCSChar, "｣｡" ) ) &&    /* x'A3A1', exclamation mark   */
        DBCSCheckChar( szInput, sIndex-1 ) )
      return TRUE ;
   else
   if ( bSChinese &&                          /* Simplified Chinese          */
        ( !strcmp( szDBCSChar, "｡｣" ) ||      /* x'A1A3', full stop          */
          !strcmp( szDBCSChar, "｣ｿ" ) ||      /* x'A3BF', question mark      */
          !strcmp( szDBCSChar, "｣｡" ) ) &&    /* x'A3A1', exclamation mark   */
        DBCSCheckChar( szInput, sIndex-1 ) )
      return TRUE ;
   else
   if ( bTChinese &&                          /* Traditional Chinese         */
        ( !strcmp( szDBCSChar, "｡C" ) ||      /* x'A143', full stop          */
          !strcmp( szDBCSChar, "｡D" ) ||      /* x'A144', period             */
          !strcmp( szDBCSChar, "｡H" ) ||      /* x'A148', question mark      */
          !strcmp( szDBCSChar, "｡I" ) ) &&    /* x'A149', exclamation mark   */
        DBCSCheckChar( szInput, sIndex-1 ) )
      return TRUE ;

   return FALSE ;
}

/*************************************************************************/
/*  Determine if the current character is the start of a DBCS character  */
/*  Return TRUE if it is the start of a DBCS character.                  */
/*************************************************************************/
BOOL  Token::DBCSCheckChar( char *szInput, short sIndex )
{
   short    i ;

   for( i=0 ; i<sIndex ; ++i ) {
      if ( IsDBCS( szInput[i] ) )
         ++i ;
   }

   if ( i == sIndex )
      return TRUE ;
   else
      return FALSE ;
}




/****************************************************************************/
/*                                                                          */
/* Token::FreeDTDInfo                                                       */
/*                                                                          */
/* Release all storage associated with the DTD information.                 */
/*                                                                          */
/****************************************************************************/
void Token::FreeDTDInfo(void)
{
    ATTRTAG        *ptrASave ;
    COPYTEXTTAG    *ptrCSave ;
    INSERTATTRTAG  *ptrISave ; 
    GLOBALTAG      *ptrGSave ;
    LINESTAG       *ptrLSave ;
    LINESTAG       *ptrICSave ;
    NEUTTAG        *ptrNSave ;
    STATEATTR      *ptrSASave ;
    STATEATTRLVL   *ptrSALSave ;
    STATETAG       *ptrSSave ;
                  
    while ( ptrNeutralTagHead ) {
       ptrNSave = (NEUTTAG*)ptrNeutralTagHead->ptrNext ;
       free( ptrNeutralTagHead ) ;
       ptrNeutralTagHead = ptrNSave ;
    }
    ptrNeutralTagHead = NULL ;

    while ( ptrAttributeTagHead ) {
       ptrASave = (ATTRTAG*)ptrAttributeTagHead->ptrNext ;
       free( ptrAttributeTagHead ) ;
       ptrAttributeTagHead = ptrASave ;
    }
    ptrAttributeTagHead = NULL ;

    while ( ptrStateTagHead ) {
       ptrSSave = (STATETAG*)ptrStateTagHead->ptrNext ;
       free( ptrStateTagHead ) ;
       ptrStateTagHead = ptrSSave ;
    }
    ptrStateTagHead = NULL ;

    while ( ptrStateAttrHead ) {
       ptrSASave = (STATEATTR*)ptrStateAttrHead->ptrNext ;
       free( ptrStateAttrHead ) ;
       ptrStateAttrHead = ptrSASave ;
    }
    ptrStateAttrHead = NULL ;

    while ( ptrStateAttrLevelHead ) {
       ptrSALSave = (STATEATTRLVL*)ptrStateAttrLevelHead->ptrNext ;
       free( ptrStateAttrLevelHead ) ;
       ptrStateAttrLevelHead = ptrSALSave ;
    }
    ptrStateAttrLevelHead = NULL ;

    while ( ptrGlobalTagHead ) {
       ptrGSave = (GLOBALTAG*)ptrGlobalTagHead->ptrNext ;
       free( ptrGlobalTagHead ) ;
       ptrGlobalTagHead = ptrGSave ;
    }
    ptrGlobalTagHead = NULL ;

    while ( ptrLinesTagHead ) {
       ptrLSave = (LINESTAG*)ptrLinesTagHead->ptrNext ;
       free( ptrLinesTagHead ) ;
       ptrLinesTagHead = ptrLSave ;
    }
    ptrLinesTagHead = NULL ;

    while ( ptrCopyTextTagHead ) {
       ptrCSave = (COPYTEXTTAG*)ptrCopyTextTagHead->ptrNext ;
       if ( ptrCopyTextTagHead->TextBefore ) 
          free( ptrCopyTextTagHead->TextBefore ) ;
       if ( ptrCopyTextTagHead->TextAfter ) 
          free( ptrCopyTextTagHead->TextAfter ) ;
       free( ptrCopyTextTagHead ) ;
       ptrCopyTextTagHead = ptrCSave ;
    }
    ptrCopyTextTagHead = NULL ;

    while ( ptrInsertAttrTagHead ) {
       ptrISave = (INSERTATTRTAG*)ptrInsertAttrTagHead->ptrNext ;
       free( ptrInsertAttrTagHead ) ;
       ptrInsertAttrTagHead = ptrISave ;
    }
    ptrInsertAttrTagHead = NULL ;

    while ( ptrIgnoreCommentTagHead ) {        
       ptrICSave = (LINESTAG*)ptrIgnoreCommentTagHead->ptrNext ;
       free( ptrIgnoreCommentTagHead ) ;
       ptrIgnoreCommentTagHead = ptrICSave ;
    }
    ptrIgnoreCommentTagHead = NULL ;
}


/****************************************************************************/
/*                                                                          */
/* Token::SetDTDInfo                                                        */
/*                                                                          */
/* Set the unique information for this particular DTD (if it is available). */
/* This information includes:                                               */
/*    1)  Which XML tags are considered neutral tags.                       */
/*    1)  Which XML tags define line-by-line segmentation sections.         */
/*    2)  Which XML tags may have attributes which define translatable text.*/
/*    3)  Which XML tags change the translatability state within the file.  */
/*                                                                          */
/****************************************************************************/
void Token::SetDTDInfo(IString& s, BOOL bFree )
{
    FILE        *fControl ;
    char        szControlPath[256] ;   
    char        szDTD[512] ;    
    char        *dtd, *dtdext ;
    char        szRecordMB[512] ; //to hold multibyte
    char        *ptrCharMB, *ptrChar2MB ; //to hold multi-byte
    char        *ptr;
    char        szUName[512];
            
    wchar_t     szRecord[512] ;
    wchar_t     *buff ;
    wchar_t     *ptrChar, *ptrChar2 ;
    wchar_t     *ptrTag, *ptrText ;
    wchar_t     szTemp[512];

    NEUTTAG     *ptrNNew=NULL, *ptrNLast=NULL ;
    ATTRTAG     *ptrANew=NULL, *ptrALast=NULL ;
    STATETAG    *ptrSNew=NULL, *ptrSLast=NULL ;
    STATEATTR   *ptrSANew=NULL, *ptrSALast=NULL ;
    GLOBALTAG   *ptrGNew=NULL, *ptrGLast=NULL ;
    LINESTAG    *ptrLNew=NULL, *ptrLLast=NULL ;
    LINESTAG    *ptrICNew=NULL, *ptrICLast=NULL ;
    COPYTEXTTAG *ptrCNew=NULL, *ptrCLast=NULL ;
    INSERTATTRTAG  *ptrINew=NULL, *ptrILast=NULL;
    short       rc ;
    BOOL        bAttribute = FALSE ;
    BOOL        bNeutral = FALSE ;
    BOOL        bState = FALSE ;
    BOOL        bStateAttr = FALSE ;
    BOOL        bGlobal = FALSE ;
    BOOL        bLines = FALSE ;
    BOOL        bCopyText = FALSE ;
    BOOL        bInsertAttr = FALSE ;
    BOOL        bIgnoreComment = FALSE ;
    BOOL        bFound ;
               

    if ( bFree ) 
       FreeDTDInfo() ;
    szControlPath[0] = NULL ;

    wcsncpy(szTemp, (wchar_t*)(LPCTSTR)s.subString(0),sizeof(szTemp)/sizeof(wchar_t)); 
    szTemp[sizeof(szTemp)/sizeof(wchar_t)-1] = NULL ;     
    wcstombs(szDTD, szTemp, sizeof(szDTD)); 
	strupr(szDTD);
    
    dtdext = strstr( szDTD, ".DTD\"" ) ;
    if ( ! dtdext )
       dtdext = strstr( szDTD, ".DTD\'" ) ;
    if ( dtdext ) {
       bFound = FALSE ;
       *(dtdext+4) = NULL ;
       for( dtd=dtdext ; dtd>szDTD && !strchr( "/\\\"\'", *dtd) ; --dtd ) ;
       if ( dtd > szDTD )
          ++dtd ;
       if ( strstr( dtd, "XHTML" ) ) {
          strcpy( dtd, "XHTML.DTD" ) ;
          dtdext = strstr( dtd, ".DTD" ) ;
       }
       szControlPath[0] = NULL ;

//     _searchenv( "OTMXML.CTL", "PATH", szControlPath );
//     if ( ! szControlPath[0] ) {
//        strcpy( szControlPath, szProgPath ) ;
//        ptr = strrchr( szControlPath, '\\' ) ;
//        if ( ptr ) 
//           strcpy( ptr, "\\TABLE\\OTMXML.CTL" ) ;
//        else
//           szControlPath[0] = 0 ;
//     }
       GetOTMTablePath( szProgPath, szControlPath ) ;
       strcat( szControlPath, "OTMXML.CTL" ) ;

       if ( szControlPath[0] ) {
          fControl = fopen( szControlPath, "r" ) ;
          if ( fControl ) {
             while( fgets( szRecordMB, sizeof(szRecordMB), fControl ) ) {
                ptrCharMB = strtok( szRecordMB, " \n\t\r" ) ;
                ptrChar2MB = strtok( NULL, " \n\t\r" ) ;
                if ( ptrCharMB && ptrChar2MB &&
                     ( ! stricmp( ptrCharMB, dtd ) ) ) {
                   strcpy( szDTD, ptrChar2MB ) ;
                   dtd = szDTD ;
                   bFound = TRUE ;
                   break ;
                }
             }
             fclose( fControl ) ;
          }
       }
       if ( ! bFound ) {                /* Use DTD name as control file name */
          *(dtdext+1) = NULL ;
          strcat( dtdext, "XML" ) ;
       }
       szControlPath[0] = NULL ;

//     _searchenv( dtd, "PATH", szControlPath );
//     if ( ! szControlPath[0] ) {
//        strcpy( szControlPath, szProgPath ) ;
//        ptr = strrchr( szControlPath, '\\' ) ;
//        if ( ptr ) {
//           strcpy( ptr, "\\TABLE\\" ) ;
//           strcat( ptr, dtd ) ;
//        } else
//           szControlPath[0] = 0 ;
//     }
       GetOTMTablePath( szProgPath, szControlPath ) ;
       strcat( szControlPath, dtd ) ;

       if ( szControlPath[0] ) {
          fControl = fopen( szControlPath, "r" ) ;
          if ( ! fControl )
             return ;
          FreeDTDInfo() ;
          szStateAttrNames[0] = NULL ;
          szStateAttrValues[0] = NULL ;
          szTransStateEndTag[0] = NULL ;                    
          szLinesEndTag[0] = NULL ;                         
          strupr( szControlPath ) ;
          if ( strstr(szControlPath, "OTMXLIFF.XML" ) ) {
             bXLIFF = TRUE ;
          } else if ( strstr(szControlPath, "OTMSDLXLF.XML" ) ) {
             bXLIFF = TRUE;
             bSDLXLF = TRUE;
          } else { 
             bXLIFF = FALSE ;
          }
          if ( ( strstr(szControlPath, "OTMXHTML.XML" ) ) || 
               ( strstr(szControlPath, "OTMXAHTM.XML" ) ) || 
               ( strstr(szControlPath, "OTMXUHTM.XML" ) ) )  
             bXHTML = TRUE ;
          else 
             bXHTML = FALSE ;

          while( fgets( szRecordMB, sizeof(szRecordMB), fControl ) ) {
             mbstowcs(szRecord, szRecordMB, sizeof(szRecordMB));
             
             ptrChar = wcschr( szRecord, L'<' ) ;
             ptrChar2 = wcschr( szRecord, L'>' ) ;
             if ( ( ptrChar ) &&
                  ( ptrChar2 > ptrChar+1 ) &&
                  ( *(ptrChar+1) != L'!' ) ) {
                ptrTag = wcstok( ptrChar+1, L" <>\n\t\r" ) ;
                ptrText = wcstok( ptrChar2+1, L"<\n\t\r" ) ;
                if ( ptrTag ) {
                   if ( ptrText ) {
                      for ( ; ptrText && iswspace(*ptrText) ; ++ptrText ) ;
                      for ( ptrChar=ptrText+wcslen(ptrText)-1 ;
                            ptrChar>ptrText && iswspace(*ptrChar) ;
                            *ptrChar=0, --ptrChar ) ;
                   }

                   /*--------------------------------------------------------*/
                   /*   Start define of neutral tags (tags which can be      */
                   /*   defined inside of a segment, like bold).             */
                   /*                                                        */
                   /*   This section will only contain <TAG> items.          */
                   /*                                                        */
                   /*      <NEUTRAL>                                         */
                   /*          <TAG> tag_name (1) </TAG>                     */
                   /*          <BREAK> </BREAK>                              */
                   /*      </NEUTRAL>                                        */
                   /*                                                        */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"NEUTRAL" ) ) {
                      bNeutral = TRUE ;
                      ptrNNew = (NEUTTAG*)malloc( sizeof(NEUTTAG)/**sizeof(wchar_t)*/ ) ;
                      ptrNNew->Tag[0] = NULL ;
                      ptrNNew->Break = FALSE ;
                      ptrNNew->Generic = FALSE ;
                      ptrNNew->ptrNext = NULL ;
                      if ( ptrNLast )
                         ptrNLast->ptrNext = ptrNNew ;
                      else
                         ptrNeutralTagHead = ptrNNew ;
                      ptrNLast = ptrNNew ;
                   } else

                   if ( ! wcsicmp( ptrTag, L"/NEUTRAL" ) ) {
                      bNeutral = FALSE ;
                   } else

                   /*--------------------------------------------------------*/
                   /*   Start define of tags with attribute values which     */
                   /*   are considered translatable.                         */
                   /*                                                        */
                   /*   This section will only contain <TAG>, <TRANSATTR>,   */
                   /*   <CONDATTR>, <CONDATTRVALUE>, <NOTTAG>, and           */
                   /*   <SKIPATTRVALUE> items.                               */
                   /*                                                        */
                   /*      <ATTRIBUTE>                                       */
                   /*         <TAG> tag_name, *name, name* (1) </TAG>        */
                   /*         <TRANSATTR> attr_name (1+) </TRANSATTR>        */
                   /* (opt)   <NOTTAG> tag_name (1) </NOTTAG>                */
                   /* (opt)   <CONDATTR> attr_name (1) </CONDATTR>           */
                   /* (opt)   <CONDATTRVALUE> attr_value (1+)</CONDATTRVALUE>*/
                   /* (opt)   <SKIPATTRVALUE> attr_value (1+)</SKIPATTRVALUE>*/
                   /*      </ATTRIBUTE>                                      */
                   /*                                                        */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"ATTRIBUTE" ) ) {
                      bAttribute = TRUE ;
                      ptrANew = (ATTRTAG*)malloc( sizeof(ATTRTAG)/**sizeof(wchar_t)*/ ) ;
                      ptrANew->Tag[0] = NULL ;
                      ptrANew->NotTag[0] = NULL ;         
                      ptrANew->Attr[0] = NULL ;
                      ptrANew->CondAttr[0] = NULL ;
                      ptrANew->CondAttrValue[0] = NULL ;
                      ptrANew->SkipAttrValue[0] = NULL ;  
                      ptrANew->ptrNext = NULL ;
                      if ( ptrALast )
                         ptrALast->ptrNext = ptrANew ;
                      else
                         ptrAttributeTagHead = ptrANew ;
                      ptrALast = ptrANew ;
                   } else

                   if ( ! wcsicmp( ptrTag, L"/ATTRIBUTE" ) ) {
                      bAttribute = FALSE ;
                   } else

                   /*--------------------------------------------------------*/
                   /*   Start define of tags which change translation        */
                   /*   states.  A state is used when everything in a        */
                   /*   section is non-translatable, like a <SCRIPT> section.*/
                   /*                                                        */
                   /*   This section will only contain <TAG>, <ENDTAG>,      */
                   /*   <INITIAL>, <CONDATTR>, and <CONDATTRVALUE> items.    */
                   /*                                                        */
                   /*      <STATE>                                           */
                   /*         <TAG> tag_name, *name, name* (1) </TAG>        */
                   /*         <ENDTAG> tag_name (1) </ENDTAG>                */
                   /*         <INITIAL>  ON || OFF  </INITIAL>               */
                   /*         ##### <INITIAL> cannot be used with <ENDTAG>   */
                   /* (opt)   <CONDATTR> attr_name (1) </CONDATTR>           */
                   /* (opt)   <CONDATTRVALUE> attr_value (1+)</CONDATTRVALUE>*/
                   /* (opt)   <CONDSTATE> TRANS || NONTRANS </CONDSTATE>     */
                   /* (opt)   <PREVTAG> tag_name </PREVTAG>                  */
                   /* (opt)   <TRANSATTRONLY> YES </TRANSATTRONLY>           */
                   /*      </STATE>                                          */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"STATE" ) ) {
                      bState = TRUE ;
                      ptrSNew = (STATETAG*)malloc( sizeof(STATETAG)/**sizeof(wchar_t)*/ ) ;
                      ptrSNew->Tag[0] = NULL ;
                      ptrSNew->EndTag[0] = NULL ;
                      ptrSNew->Initial = TRANS_STATE_NONE ;
                      ptrSNew->CondAttr[0] = NULL ;
                      ptrSNew->CondAttrValue[0] = NULL ;
                      ptrSNew->PrevTag[0] = NULL ;
                      ptrSNew->TransAttrOnly = FALSE ;
                      ptrSNew->Generic = FALSE ;
                      ptrSNew->CondState = TRANS_STATE_NONE ;
                      ptrSNew->ptrNext = NULL ;
                      if ( ptrSLast )
                         ptrSLast->ptrNext = ptrSNew ;
                      else
                         ptrStateTagHead = ptrSNew ;
                      ptrSLast = ptrSNew ;
                   } else

                   if ( ! wcsicmp( ptrTag, L"/STATE" ) ) {
                      bState = FALSE ;
                   } else

                   /*--------------------------------------------------------*/
                   /*   Start define of attributes which change translation  */
                   /*   states.  An attribute state is used when text        */
                   /*   between this start tag and its end tag is either     */
                   /*   translatable or not.                                 */
                   /*                                                        */
                   /*   This section will only contain <CONDATTR>,           */
                   /*   <CONDATTRVALUE> and <INITIAL> items.                 */
                   /*                                                        */
                   /*      <STATEATTR>                                       */
                   /*         <CONDATTR> attr_name (1) </CONDATTR>           */
                   /*         <CONDATTRVALUE> attr_value (1+)</CONDATTRVALUE>*/
                   /*         <INITIAL>  ON || OFF  </INITIAL>               */
                   /*      </STATEATTR>                                      */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"STATEATTR" ) ) {
                      bStateAttr = TRUE ;
                      ptrSANew = (STATEATTR*)malloc( sizeof(STATEATTR)/**sizeof(wchar_t)*/ ) ;
                      ptrSANew->CondAttr[0] = NULL ;
                      ptrSANew->CondAttrValue[0] = NULL ;
                      ptrSANew->Initial = TRANS_STATE_NONE ;
                      ptrSANew->ptrNext = NULL ;
                      if ( ptrSALast ) {
                         ptrSALast->ptrNext = ptrSANew ;
                      } else {
                         ptrStateAttrHead = ptrSANew ;
                      }
                      ptrSALast = ptrSANew ;
                   } else

                   if ( ! wcsicmp( ptrTag, L"/STATEATTR" ) ) {
                      bStateAttr = FALSE ;
                   } else

                   /*--------------------------------------------------------*/
                   /*   Start define of tags which define global tag which   */
                   /*   can occur anywhere in the XML source.  (XSL)         */
                   /*                                                        */
                   /*   This section will only contain <TAG> items.          */
                   /*                                                        */
                   /*      <GLOBAL>                                          */
                   /*          <TAG> tag_name (1) </TAG>                     */
                   /*      </GLOBAL>                                         */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"GLOBAL" ) ) {
                      bGlobal = TRUE ;
                      ptrGNew = (GLOBALTAG*)malloc( sizeof(GLOBALTAG)/**sizeof(wchar_t)*/ ) ;
                      ptrGNew->Tag[0] = NULL ;
                      ptrGNew->Generic = FALSE ; 
                      ptrGNew->ptrNext = NULL ;
                      if ( ptrGLast )
                         ptrGLast->ptrNext = ptrGNew ;
                      else
                         ptrGlobalTagHead = ptrGNew ;
                      ptrGLast = ptrGNew ;
                   } else

                   if ( ! wcsicmp( ptrTag, L"/GLOBAL" ) ) {
                      bGlobal = FALSE ;
                   } else

                   /*--------------------------------------------------------*/
                   /*   Start define of tags which should be segmented on a  */
                   /*   line basis rather than a sentence basis.             */
                   /*                                                        */
                   /*   This section will only contain <TAG>, <ENDTAG>,      */
                   /*   <CONDATTR> and <CONDATTRVALUE> items.                */
                   /*                                                        */
                   /*      <LINES>                                           */
                   /*          <TAG> tag_name (1) </TAG>                     */
                   /*          <ENDTAG> tag_name (1) </ENDTAG>               */
                   /*          <CONDATTR> attr_name (1) </CONDATTR>          */
                   /*          <CONDATTRVALUE> attr_value (1)</CONDATTRVALUE>*/
                   /*      </LINES>                                          */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"LINES" ) ) {
                      bLines = TRUE ;
                      ptrLNew = (LINESTAG*)malloc( sizeof(LINESTAG)/**sizeof(wchar_t)*/ ) ;
                      ptrLNew->Tag[0] = NULL ;
                      ptrLNew->EndTag[0] = NULL ;
                      ptrLNew->CondAttr[0] = NULL ;
                      ptrLNew->CondAttrValue[0] = NULL ;
                      ptrLNew->ptrNext = NULL ;
                      if ( ptrLLast )
                         ptrLLast->ptrNext = ptrLNew ;
                      else
                         ptrLinesTagHead = ptrLNew ;
                      ptrLLast = ptrLNew ;
                   } else

                   if ( ! wcsicmp( ptrTag, L"/LINES" ) ) {
                      bLines = FALSE ;
                   } else

                   /*--------------------------------------------------------*/
                   /*   Start define of tags which define translatable text  */
                   /*   which should be copied from one tag to another.      */
                   /*                                                        */
                   /*   This section will only contain <TAG>, <ENDTAG> items.*/
                   /*                                                        */
                   /*      <COPYTEXT>                                        */
                   /*         <TAG> from_tag name (1) </TAG>                 */
                   /*         <ENDTAG> to_tag name (1) </ENDTAG>             */
                   /* (opt)   <BEFORE> text_to add_before </BEFORE>          */
                   /* (opt)   <AFTER> text_to_add_after </AFTER>             */
                   /* (no support) <PARENT> xxxxxxxxxxxxxxxxxx </PARENT>     */
                   /*      </COPYTEXT>                                       */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"COPYTEXT" ) ) {
                      bCopyText = TRUE ;
                      ptrCNew = (COPYTEXTTAG*)malloc( sizeof(COPYTEXTTAG)/**sizeof(wchar_t)*/ ) ;
                      ptrCNew->FromTag[0] = NULL ;
                      ptrCNew->ToTag[0] = NULL ;
                      ptrCNew->ParentTag[0] = NULL ;
                      ptrCNew->TextBefore = NULL ;
                      ptrCNew->TextAfter = NULL ;
                      ptrCNew->ptrNext = NULL ;
                      if ( ptrCLast ) {
                         ptrCLast->ptrNext = ptrCNew ;
                      } else {
                         ptrCopyTextTagHead = ptrCNew ;
                      }
                      ptrCLast = ptrCNew ;
                   } else

                   if ( ! wcsicmp( ptrTag, L"/COPYTEXT" ) ) {
                      bCopyText = FALSE ;
                   } else

                   /*--------------------------------------------------------*/
                   /*   Start define of tags which define attribute text     */
                   /*   to be inserted in a tag on condition.                */
                   /*                                                        */
                   /*                                                        */
                   /*      <INSERTATTR>                                      */
                   /*         <TAG> tag name (1) </TAG>                      */
                   /*         <COPYATTR> attribute to be copied </COPYATTR>  */
                   /*         <CONDATTR> tag should contain this text		 */
                   /*                      to add copyattr </CONDATTR>       */
                   /*                    Can be "*" for all tags.            */
                   /*         <CONDATTRVALUE> attr_value*(1+)</CONDATTRVALUE>*/
                   /*         <REMOVE> remove attr before write </REMOVE>    */
                   /*      </INSERTATTR>                                     */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"INSERTATTR" ) ) {
                      bInsertAttr = TRUE ;
                      ptrINew = (INSERTATTRTAG*)malloc( sizeof(INSERTATTRTAG)/**sizeof(wchar_t)*/ ) ;
                      ptrINew->Tag[0] = NULL ;
                      ptrINew->CopyAttr[0] = NULL ;
                      ptrINew->CondAttr[0] = NULL ;
                      ptrINew->CondAttrValue[0] = NULL ;
                      ptrINew->Remove = FALSE ;
                      ptrINew->ptrNext = NULL ;
                      if ( ptrILast ) {
                    	 ptrILast->ptrNext = ptrINew ;
                      } else {
                    	 ptrInsertAttrTagHead = ptrINew ;
                      }
                      ptrILast = ptrINew ;
                   } else
                   
                   if ( ! wcsicmp( ptrTag, L"/INSERTATTR" ) ) {
                      bInsertAttr = FALSE ;
                   } else
                   
                   /*--------------------------------------------------------*/
                   /*   Start define of tags which define section where      */
                   /*   comment delimiters should be ignored.                */
                   /*                                                        */
                   /*                                                        */
                   /*      <IGNORECOMMENT>                                   */
                   /*         <TAG> tag name (1) </TAG>                      */
                   /*         <ENDTAG> tag name (1) </ENDTAG>                */
                   /*      </IGNORECOMMENT>                          7/13/04 */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"IGNORECOMMENT" ) ) {
                      bIgnoreComment = TRUE ;
                      ptrICNew = (LINESTAG*)malloc( sizeof(LINESTAG)/**sizeof(wchar_t)*/ ) ;
                      ptrICNew->Tag[0] = NULL ;
                      ptrICNew->EndTag[0] = NULL ;
                      ptrICNew->ptrNext = NULL ;
                      if ( ptrICLast ) {
                         ptrICLast->ptrNext = ptrICNew ;
                      } else {
                         ptrIgnoreCommentTagHead = ptrICNew ;
                      }
                      ptrICLast = ptrICNew ;
                   } else

                   if ( ! wcsicmp( ptrTag, L"/IGNORECOMMENT" ) ) {
                      bIgnoreComment = FALSE ;
                   } else

                   /*--------------------------------------------------------*/
                   /*   If an <ANSI> tag, then import and export files are   */
                   /*   encoded in ANSI rather than ASCII.                   */
                   /*                                                        */
                   /*      <ANSI>  </ANSI>                                   */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"ANSI" ) ) {
                      if ( ! bCodePageUTF16 ) {
                         bCodePageANSI = TRUE ; 
                         bCodePageHTML = FALSE ;
                         bCodePageUTF8 = FALSE ;
                      }
                   } else

                   /*--------------------------------------------------------*/
                   /*   If an <HTML> tag, then import and export files are   */
                   /*   encoded in HTML set of code pages rather than ASCII. */
                   /*                                                        */
                   /*      <HTML>  </HTML>                                   */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"HTML" ) ) {
                      if ( ! bCodePageUTF16 ) {
                         bCodePageHTML = TRUE ; 
                         bCodePageANSI = FALSE ;
                         bCodePageUTF8 = FALSE ;
                      }
                   } else

                   /*--------------------------------------------------------*/
                   /*   If an <PRESERVEEOL> tag, then end-of-line character  */
                   /*   should be preserved in the exported file.            */
                   /*                                                        */
                   /*      <PRESERVEEOL>  </PRESERVEEOL>                     */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"PRESERVEEOL" ) ) {
                      bPreserveEOL = TRUE ; 
                   } else

                   /*--------------------------------------------------------*/
                   /*   If an <INSERTEOL> tag, then end-of-line characters   */
                   /*   should be added after each non-neutral end tag.      */
                   /*                                                        */
                   /*      <INSERTEOL>  </INSERTEOL>                         */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"INSERTEOL" ) ) {
                      bInsertEOL = TRUE ; 
                   } else

                   /*--------------------------------------------------------*/
                   /*   If an <UTF8> tag, then import and export files are   */
                   /*   encoded in UTF-8 rather than ASCII.                  */
                   /*                                                        */
                   /*      <UTF8>  </UTF8>                                   */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"UTF8" ) ) {
                      if ( ! bCodePageUTF16 ) {
                         bCodePageUTF8 = TRUE ; 
                         bCodePageANSI = FALSE ;
                         bCodePageHTML = FALSE ;
                      }
                   } else

                   /*--------------------------------------------------------*/
                   /*   Within <ATTRIBUTE>, <GLOBAL>, <NEUTRAL>, and <STATE> */
                   /*   sections, this defines an XML tag which has special  */
                   /*   processing based on the parent section.              */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"TAG" ) ) &&
                        ( ptrText ) ) {
                      if ( bAttribute ) {
                         wcscpy( ptrALast->Tag, ptrText ) ;
                      } else
                      if ( bNeutral ) {
                         wcscpy( ptrNLast->Tag, ptrText ) ;
                         if ( ptrText[wcslen(ptrText)-1] == '*' ) {
                            ptrNLast->Generic = TRUE ;   /* Generic tag name */
                            *(ptrNLast->Tag+wcslen(ptrText)-1) = NULL ; 
                         } else {
                            if ( ptrText[0] == '*' ) {   /* Match end of tag */
                               ptrNLast->Generic = TRUE ;/* Generic tag name */
                            }
                         }
                      } else
                      if ( bState ) {
                         wcscpy( ptrSLast->Tag, ptrText ) ;
                         if ( ptrText[wcslen(ptrText)-1] == '*' ) {
                            ptrSLast->Generic = TRUE ;   /* Generic tag name */
                            *(ptrSLast->Tag+wcslen(ptrText)-1) = NULL ;
                         } else {
                            if ( ptrText[0] == '*' ) {   /* Match end of tag */
                               ptrSLast->Generic = TRUE ;/* Generic tag name */
                            }
                         }
                      } else
                      if ( bGlobal ) {
                         wcscpy( ptrGLast->Tag, ptrText ) ;
                         if ( ptrText[wcslen(ptrText)-1] == '*' ) {
                            ptrGLast->Generic = TRUE ;
                            *(ptrGLast->Tag+wcslen(ptrText)-1) = NULL ; 
                         }
                      } else
                      if ( bLines ) {
                         wcscpy( ptrLLast->Tag, ptrText ) ;
                      } else
                      if ( bCopyText ) {
                         wcscpy( ptrCLast->FromTag, ptrText ) ;
                      } else 
                      if ( bIgnoreComment ) {
                         wcscpy( ptrICLast->Tag, ptrText ) ;
                      }

                      if ( bInsertAttr ) {
                         wcscpy( ptrILast->Tag, ptrText ) ;
                      } 
                   } else

                   /*--------------------------------------------------------*/
                   /*   Within an <ATTRIBUTE> section, this defines an       */
                   /*   attribute whose value is used as a condition as to   */
                   /*   whether the <ATTRIBUTE> value is translatable.       */
                   /*                                                        */
                   /*   Within a <STATE> section, this defines an            */
                   /*   attribute whose value is used as a condition as to   */
                   /*   the type of state (used for <SCRIPT> sections).      */
                   /*                                                        */
                   /*   Within a <STATEATTR> section, this defines an        */
                   /*   attribute whose value is used as a condition as to   */
                   /*   the type of translation state of the text following  */
                   /*   the start tag.                                       */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"CONDATTR" ) ) &&
                        ( ptrText ) ) {
                      if ( bAttribute ) {
                         wcscpy( ptrALast->CondAttr, ptrText ) ;
                      } else
                      if ( bState ) {
                         wcscpy( ptrSLast->CondAttr, ptrText ) ;
                      } else
                      if ( bStateAttr ) {
                         wcscpy( ptrSALast->CondAttr, ptrText ) ;
                         wcsupr( ptrSALast->CondAttr ) ;
                         if ( ! wcsstr( szStateAttrNames, ptrText) ) {
                            wcscat( szStateAttrNames, L" " ) ;      
                            wcscat( szStateAttrNames, ptrSALast->CondAttr ) ;
                            wcscat( szStateAttrNames, L" " ) ;
                         }                      
                      }					  
                      if ( bInsertAttr ) {
                    	 wcscpy( ptrILast->CondAttr, ptrText) ;
                    	 wcsupr( ptrILast->CondAttr ) ;
                      } 
                      if ( bLines ) {
                    	 wcscpy( ptrLLast->CondAttr, ptrText) ;
                    	 wcsupr( ptrLLast->CondAttr ) ;
                      }
                   } else

                   /*--------------------------------------------------------*/
                   /*   Within an <ATTRIBUTE> section, this defines an       */
                   /*   attribute value which is used as a condition as to   */
                   /*   whether the <ATTRIBUTE> value is translatable.       */
                   /*                                                        */
                   /*   Within a <STATE> section, this defines an            */
                   /*   attribute value which is used as a condition as to   */
                   /*   the type of state (used of <SCRIPT> sections).       */
                   /*                                                        */
                   /*   Within a <STATEATTR> section, this defines an        */
                   /*   attribute value which is used as a condition as to   */
                   /*   the type of translation state of the text following  */
                   /*   the start tag.                                       */
                   /*   Allow '*' in value to define a string which is to be */
                   /*   found somewhere in the attribute value.              */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"CONDATTRVALUE" ) ) &&
                        ( ptrText ) ) {
                      if ( bAttribute ) {
                         wcscpy( ptrALast->CondAttrValue, ptrText ) ;
                         wcsupr( ptrALast->CondAttrValue ) ;
                      } else
                      if ( bState ) {
                         wcscpy( ptrSLast->CondAttrValue, ptrText ) ;
                         wcsupr( ptrSLast->CondAttrValue ) ;
                      } else
                      if ( bStateAttr ) {
                         wcscpy( ptrSALast->CondAttrValue, ptrText ) ;
                         wcsupr( ptrSALast->CondAttrValue ) ;
                         if ( ! wcsstr( szStateAttrValues, ptrText) ) {
                            wcscat( szStateAttrValues, L" " ) ;      
                            wcscat( szStateAttrValues, ptrSALast->CondAttrValue ) ;
                            wcscat( szStateAttrValues, L" " ) ;
                         }
                      } else
                      if ( bInsertAttr ) {
                         wcscpy( ptrILast->CondAttrValue, L" " ) ;
                         wcscat( ptrILast->CondAttrValue, ptrText ) ;
                         wcscat( ptrILast->CondAttrValue, L" " ) ;
                         wcsupr( ptrILast->CondAttrValue ) ;
                      } 
                      if ( bLines ) {
                    	 wcscpy( ptrLLast->CondAttrValue, ptrText) ;
                    	 wcsupr( ptrLLast->CondAttrValue ) ;
                      }
                   } else

                   /*--------------------------------------------------------*/
                   /*   Within an <ATTRIBUTE> section, this defines an       */
                   /*   attribute which contains translatable text.          */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"TRANSATTR" ) ) &&
                        ( ptrText ) ) {
                      if ( bAttribute ) {
                         wcscpy( ptrALast->Attr, ptrText ) ;
                      }
                   } else

                   /*--------------------------------------------------------*/
                   /*   Within an <ATTRIBUTE> section, this defines an       */
                   /*   attribute value which should not be translatable.    */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"SKIPATTRVALUE" ) ) &&
                        ( ptrText ) ) {
                      if ( bAttribute ) {
                         wcscpy( ptrALast->SkipAttrValue, ptrText ) ;
                         wcsupr( ptrALast->SkipAttrValue ) ;
                      } 
                   } else

                   /*--------------------------------------------------------*/
                   /*   Within an <STATE> section, this defines the initial  */
                   /*   translatability state for the section starting with  */
                   /*   this tag.  No <ENDTAG> should be defined with this   */
                   /*   <STATE> section.                                     */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"INITIAL" ) ) &&
                        ( ptrText ) ) {
                      if ( bState ) {
                         if ( ! wcsicmp( ptrText, L"OFF" ) )
                            ptrSLast->Initial = TRANS_STATE_OFF ;
                         else
                            if ( ! wcsicmp( ptrText, L"ON" ) )
                               ptrSLast->Initial = TRANS_STATE_ON ;
                      } else
                      if ( bStateAttr ) {
                         if ( ! wcsicmp( ptrText, L"OFF" ) )
                            ptrSALast->Initial = TRANS_STATE_OFF ;
                         else
                            if ( ! wcsicmp( ptrText, L"ON" ) )
                               ptrSALast->Initial = TRANS_STATE_ON ;
                      }
                   } else

                   /*--------------------------------------------------------*/
                   /*   Within an <STATE> section, this defines the end tag  */
                   /*   which corresponds with the previously defined <TAG>  */
                   /*   which resets the translation state to its default    */
                   /*   value.                                               */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"ENDTAG" ) ) &&
                        ( ptrText ) ) {
                      if ( bState ) {
                         wcscpy( ptrSLast->EndTag, ptrText ) ;
                      } else
                      if ( bLines ) {
                         wcscpy( ptrLLast->EndTag, ptrText ) ;
                      } else 
                      if ( bCopyText ) {
                         wcscpy( ptrCLast->ToTag, ptrText ) ;
                      } else 
                      if ( bIgnoreComment ) {
                         wcscpy( ptrICLast->EndTag, ptrText ) ;
                      } 
                   } else

                   /*--------------------------------------------------------*/
                   /*   Defines an XML tag which is an exception to the      */
                   /*   <TAG> information.                                   */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"NOTTAG" ) ) &&
                        ( ptrText ) ) {
                      if ( bAttribute ) {
                         wcscpy( ptrALast->NotTag, ptrText ) ;
                         wcsupr( ptrALast->NotTag ) ;
                      } 
                   } else

                   /*--------------------------------------------------------*/
                   /* To copy the ENDTAG tag which is defined within COPYTEXT */
                   /*  section, PARENT tag is used to validate copy action   */
                   /*                                                        */
                   /*--------------------------------------------------------*/
            ///    if ( ( ! wcsicmp( ptrTag, L"PARENT" ) ) &&
            ///     		( ptrText ) ) {
            ///     	  if ( bCopyText ) {
            ///     		 wcscpy( ptrCLast->ParentTag, ptrText) ;
            ///     		 wcsupr( ptrCLast->ParentTag ) ;
            ///     	  } 
            ///    } else
                   
                   /*--------------------------------------------------------*/
                   /*  Within a <STATETAG> section, this defines the         */
                   /*  previous tag in order for the state change to occur.  */
                   /*                                                        */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"PREVTAG" ) ) &&
                    		( ptrText ) ) {
                    	  if ( bState ) {
                    		 wcscpy( ptrSLast->PrevTag, ptrText) ;
                    		 wcsupr( ptrSLast->PrevTag ) ;
                    	  } 
                   } else
                   
                   /*--------------------------------------------------------*/
                   /*  Within a <INSERTATTR> section, this defines           */
                   /*  the attribute to be copied within a tag.              */
                   /*                                                        */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"COPYATTR" ) ) &&
                    		( ptrText ) ) {
                    	  if ( bInsertAttr ) {
                    		 wcscpy( ptrILast->CopyAttr, ptrText) ;
                    		 //strupr( ptrILast->CopyAttr ) ;
                    	  } 
                   } else
                   /*--------------------------------------------------------*/
                   /*   Within a <NEUTRAL> section, this defines that a      */
                   /*   neutral tag should be considered to break the        */
                   /*   segmentation when it occurs just after the end of    */
                   /*   a sentence.                                          */
                   /*--------------------------------------------------------*/
                   if ( ! wcsicmp( ptrTag, L"BREAK" ) ) {
                      if ( bNeutral ) 
                         ptrNLast->Break = TRUE ;
                   } else

                   /*--------------------------------------------------------*/
                   /*   Within an <COPYTEXT> section, this defines common    */
                   /*   text which is added before the inserted text.        */
                   /*   Any '<' in text must be coded as "&lt;".             */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"BEFORE" ) ) &&
                        ( ptrText ) ) {
                      if ( bCopyText ) {
                         ptrCLast->TextBefore = (wchar_t*)malloc((wcslen(ptrText)+1)*sizeof(wchar_t)) ;
                         wcscpy( ptrCLast->TextBefore, ptrText ) ;
                         for( ptrChar=wcsstr(ptrCLast->TextBefore, L"&lt;") ;
                              ptrChar ; 
                              ptrChar=wcsstr(ptrChar, L"&lt;") ) {
                            memmove( ptrChar+1, ptrChar+4, (wcslen(ptrChar+4)+1)*sizeof(wchar_t) ) ;
                            *ptrChar = '<' ;
                         }
                      } 
                   } else

                   /*--------------------------------------------------------*/
                   /*   Within an <COPYTEXT> section, this defines common    */
                   /*   text which is added after the inserted text.         */
                   /*   Any '<' in text must be coded as "&lt;".             */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"AFTER" ) ) &&
                        ( ptrText ) ) {
                      if ( bCopyText ) {
                         ptrCLast->TextAfter = (wchar_t*)malloc( (wcslen(ptrText)+1)*sizeof(wchar_t) ) ;
                         wcscpy( ptrCLast->TextAfter, ptrText ) ;
                         for( ptrChar=wcsstr(ptrCLast->TextAfter, L"&lt;") ;
                              ptrChar ; 
                              ptrChar=wcsstr(ptrChar, L"&lt;") ) {
                            memmove( ptrChar+1, ptrChar+4, (wcslen(ptrChar+4)+1)*sizeof(wchar_t) ) ;
                            *ptrChar = '<' ;
                         }
                      } 
                   } else

                   /*--------------------------------------------------------*/
                   /*  Within a <INSERTATTR> section, this defines           */
                   /*  whether an inserted attribute should be removed       */
                   /*  before the text is saved.                             */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"REMOVE" ) ) &&
                            ( ptrText ) ) {
                      if ( bInsertAttr ) {
                         if ( ! wcsicmp( ptrText, L"YES" ) )
                            ptrILast->Remove = TRUE ;      
                      } 
                   } else

                   /*--------------------------------------------------------*/
                   /*  Within a <STATE> section, this defines                */
                   /*  whether attribute values are considered translatable  */
                   /*  even when in a non-translatable section.              */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"TRANSATTRONLY" ) ) &&
                            ( ptrText ) ) {
                      if ( bState ) {
                         if ( ! wcsicmp( ptrText, L"YES" ) )
                            ptrSLast->TransAttrOnly = TRUE ;
                      }
                   } else

                   /*--------------------------------------------------------*/
                   /*  Within a <STATE> section, this defines whether the    */
                   /*  state change can occur only in a translatable state,  */
                   /*  only in a non-translatable state, or in any state     */
                   /*  (which is the default)                                */
                   /*--------------------------------------------------------*/
                   if ( ( ! wcsicmp( ptrTag, L"CONDSTATE" ) ) &&
                            ( ptrText ) ) {
                      if ( bState ) {
                         if ( ! wcsicmp( ptrText, L"TRANS" ) )
                            ptrSLast->CondState = TRANS_STATE_ON ;
                         else
                         if ( ! wcsicmp( ptrText, L"NONTRANS" ) )
                            ptrSLast->CondState = TRANS_STATE_OFF ;
                      }
                   } else

                   {}
                }
             }
          }
          fclose( fControl ) ;


          /*-----------------------------------------------------------------*/
          /*   If processing XHTML for Bidi language, then perform special   */
          /*   processing so that ALIGN and DIR attributes are translatable. */
          /*-----------------------------------------------------------------*/
          if ( ( bBidi  ) &&
               ( bXHTML ) ) {
             /*--------------------------------------------------------------*/
             /*   Add ALIGN attribute as translatable.                       */
             /*--------------------------------------------------------------*/
             ptrANew = (ATTRTAG*)calloc( 1, sizeof(ATTRTAG) ) ;
             wcscpy( ptrANew->Tag, L"*" ) ;
             wcscpy( ptrANew->Attr, L"ALIGN" ) ;
             wcscpy( ptrANew->SkipAttrValue, L"CENTER JUSTIFY CHAR" ) ;
             ptrANew->ptrNext = NULL ;
             if ( ptrALast )
                ptrALast->ptrNext = ptrANew ;
             else
                ptrAttributeTagHead = ptrANew ;
             ptrALast = ptrANew ;

             /*--------------------------------------------------------------*/
             /*   Add DIR attribute as translatable.                         */
             /*-------------------------------------------------------------*/
             ptrANew = (ATTRTAG*)calloc( 1, sizeof(ATTRTAG) ) ;
             wcscpy( ptrANew->Tag, L"*" ) ;
             wcscpy( ptrANew->Attr, L"DIR" ) ;
             ptrALast->ptrNext = ptrANew ;
             ptrALast = ptrANew ;

             /*--------------------------------------------------------------*/
             /*   Add DIR attribute to <HTML> tag.                           */
             /*--------------------------------------------------------------*/
             ptrINew = (INSERTATTRTAG*)calloc( 1, sizeof(INSERTATTRTAG) ) ;
             wcscpy( ptrINew->Tag, L"HTML" ) ;
             wcscpy( ptrINew->CopyAttr, L"dir=\"rtl\"" ) ;        
             wcscpy( ptrINew->CondAttr, L"*" ) ;
             if ( ptrILast ) 
                ptrILast->ptrNext = ptrINew ;
             else 
                ptrInsertAttrTagHead = ptrINew ;
             ptrILast = ptrINew ;

          } 
       }
    }
}
