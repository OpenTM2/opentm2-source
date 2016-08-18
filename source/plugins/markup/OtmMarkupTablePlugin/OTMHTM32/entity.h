/* entity.h file */

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

#ifndef _ENTITY_H_INCLUDE_
#define _ENTITY_H_INCLUDE_

BOOL EntityTag(char *sourceName, char *tempName);
BOOL PostEntityTag(char *sourceName, char *tempName, BOOL *Resequence);
BOOL chk_system(wchar_t *sub_str);
void chk_entity(wchar_t *char_str, FILE *sourceFile_ptr, FILE *tempFile_ptr);
void chk_font(wchar_t *char_str, FILE *sourceFile_ptr, short *FontState, short *NobrState, short *TagState, short *PrevTagState, short *InputState, short *InputCount, BOOL ValueAttr );
void chk_valueattr(wchar_t *tag, wchar_t *char_str, FILE *sourceFile_ptr, short *InputState, short TagState, short *ValueCount, BOOL ValueAttr ) ;
void chk_scriptquote(wchar_t *Text, FILE *InFile, short *sQuoteState, short *sQuoteNum, short *sQuoteCtl, wchar_t *cQuoteChar );
void chk_Seg2k(wchar_t *char_str, wchar_t *char_str2, short *SegLength, BOOL *Resequence );
void chk_nontrans(wchar_t *sub_str, BOOL *State, BOOL *AttrState );
void chk_bidihtml(wchar_t *tag_str);
void chk_bidiattr(wchar_t *char_str, FILE *InFile);
#endif

