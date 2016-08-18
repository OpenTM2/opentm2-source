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
/*These classes and APIs were implemented for supporting unicode input files.
Since TM552, TM has been supporting internal files in unicode(UTF-16) format.

The markup table code was originally written using IBM VisualAge C++.
Since unicode support was not provided by IBM at the time of this coversion,
We needed to support IBM C++ classe's functionality using Microsoft C++ base classes.
So, the String classes needed to be derived from the base class to 
support functionality provided by IBM visual C++.
   
Microsoft iostream classes do not support the unicode data at the time of this conversion.
Since the original markup code used many iostream functions, iostream classes needed to be
improved to support unicode data. So, both input and output clases were derived from the 
base class.*/

#ifndef _UNICODE_H_INCLUDE_
#define _UNICODE_H_INCLUDE_


#include <afxwin.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <tchar.h>
#include "usrcalls.h"

#define BYTE_ORDER_MARK 0xFEFF
#define MAX_BUFFER_SIZE 8192     

#define  EQF_ASCII2ANSI  1       
#define  EQF_ANSI2ASCII  2       

void mbox(char*);
void mbox(wchar_t*);


BOOL IsUTF16(char *fileName);
BOOL RemoveBOM(wchar_t *szBuffer);

namespace mku {
   class wifstream : public std::ifstream {
   private:
       FILE* filePtr;
   public:
       wifstream(char *fileName); 
       wifstream(); 
       ~wifstream() {fclose(filePtr);close();}
   
       int open(char *fileName); 
       void freefile(void);
   
       int IsUTF16();
   
       wchar_t* getline(wchar_t *pwch, int nCount, BOOL isUTF16);   
       char* getline( char* pch, int nCount, BOOL isUTF16 );
       wchar_t* get(wchar_t *pwch, int nCount, BOOL isUTF16, BOOL includeBOM);   
       char* get( char* pch, int nCount, BOOL isUTF16 );
   
       int RemoveBOM(wchar_t *szBuffer);
   
       int feoft();
       long ftellt();
       int fseekt(long offset, int origin );
       long tellg();
       int seekg(long offset, int origin );
   };
   
   class IStringEnum {
   public:
   /*------------------------- StripMode ----------------------------------------*/
   typedef enum {
     leading,
     trailing,
     both
     } StripMode;
   
   /*------------------------- Character Type -----------------------------------*/
   typedef enum {
     sbcs,
     dbcs1 = 1,
     mbcs1 = 1,
     dbcs2 = 2,
     mbcs2 = 2,
     mbcs3 = 3,
     mbcs4 = 4
     } CharType;
   };
   
   class IString : public CString {
   public:
       IString() : CString() {}
       IString(const _TCHAR* psz) : CString(psz){};
   
       ~IString() {};    
   
       IString& change( _TCHAR* pInputString, _TCHAR* pOutputString, int startPos=0, unsigned numChanges=(_TINT)UINT_MAX);
       IString& copy( _TINT numCopies);
       IString& remove(int startPos, int numChars);
   	 bool includes(_TCHAR* pString ) const;
       int indexOf(const _TCHAR* pString, int startPos) const;
   	 int indexOf(_TCHAR* pString) const;
       int indexOfAnyBut(const _TCHAR* pString, int startPos) const;
       int indexOfAnyOf(const _TCHAR* pString, int startPos) const;
   	 int IString::lastIndexOf(_TCHAR* pString, int idx) const;
   	 int IString::lastIndexOf(_TCHAR* pString ) const;
   	 int IString::lastIndexOf(_TCHAR ch, int idx) const;
   	 int IString::lastIndexOf(_TCHAR ch ) const;
       int length() const;
       IString subString(_TINT startPos) const;
       IString subString(_TINT startPos, _TINT length, TCHAR padCharacter=' ' ) const;
       IString& overlayWith(const _TCHAR* pOverlayString, int index);
       bool isWhiteSpace() const;
       IString& insert(const _TCHAR* pString, _TINT index, _TCHAR padChar=' ');
       IString& lowercase();
       IString& uppercase();
       IString& upperCase();
       IString& lowerCase();
       IString& stripLeading();
       IString& stripTrailing();
       IStringEnum::CharType charType(int index) const;
       wchar_t operator [] (int nIndex);
   
       static IString lineFrom(wifstream& astream, _TCHAR delim = '\n');
   };
   

   
   class wofstream : public std::ofstream {
   private:
       FILE* filePtr;
   public:
       wofstream(char *fileName );
       ~wofstream() {fclose(filePtr);close();}
   
       wofstream& operator <<( wchar_t *szBuffer);
       wofstream& operator <<( IString szBuffer);
   
       BOOL InsertBOM();
   
       wofstream& operator <<( unsigned short us   );
       
       inline wofstream& operator << ( char ch ) {
             return (wofstream&)std::ostream::operator << ( ch );
       }
   
       inline wofstream& operator << ( const char* psz  ) {
             return (wofstream&)std::ostream::operator << ( psz );
       }
   
       inline wofstream& operator << ( unsigned char uch   ) {
             return (wofstream&)std::ostream::operator << ( uch );
       }
   
       inline wofstream& operator << ( signed char sch   ) {
             return (wofstream&)std::ostream::operator << ( sch );
       }
   
       inline wofstream& operator << ( const unsigned char* pusz   ) {
             return (wofstream&)std::ostream::operator << ( pusz );
       }
   
       inline wofstream& operator << ( const signed char* pssz   ) {
             return (wofstream&)std::ostream::operator << ( pssz );
       }
   
       inline wofstream& operator << (  short s   ) {
             return (wofstream&)std::ostream::operator << ( s );
       }
   
       inline wofstream& operator << ( int n   ) {
             return (wofstream&)std::ostream::operator << ( n );
       }
   
       inline wofstream& operator << ( unsigned int un   ) {
             return (wofstream&)std::ostream::operator << ( un );
       }
   
       inline wofstream& operator << ( long l   ) {
             return (wofstream&)std::ostream::operator << ( l );
       }
   
       inline wofstream& operator << ( unsigned long ul    ) {
             return (wofstream&)std::ostream::operator << ( ul );
       }
   
       inline wofstream& operator << ( float f    ) {
             return (wofstream&)std::ostream::operator << ( f );
       }
   
       inline wofstream& operator << ( double d    ) {
             return (wofstream&)std::ostream::operator << ( d );
       }
   
       inline wofstream& operator << ( long double ld    ) {
             return (wofstream&)std::ostream::operator << ( ld );
       }
   
       inline wofstream& operator << ( const void* pv    ) {
             return (wofstream&)std::ostream::operator << ( pv );
       }
   
       inline wofstream& operator << ( std::streambuf* psb    ) {
             return (wofstream&)std::ostream::operator << ( psb );
       }
   
       inline wofstream& operator << ( std::ostream& (*fcn)(std::ostream&)   ) {
             return (wofstream&)std::ostream::operator << ( fcn );
       }
   
       inline wofstream& operator << ( std::ios& (*fcn)(std::ios&)    ) {
             return (wofstream&)std::ostream::operator << ( fcn );
       }
   };

}

#endif


