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
//*********************************************************************************************************
//
//   THIS FILE CONTAINS GENERAL PURPOSE UNICODE ROUTINES
//
//**********************************************************************************************************
#include "unicode.h"

BOOL bUnicodeEnabled;

extern  char    szDocTargetLanguage[80];
extern  char    szDocSourceLanguage[80];
extern  short   sTPVersion;


using namespace mku ;

//////////Member functions of wistream/////////////////////////
::wifstream::wifstream(char *fileName) {    
    filePtr = fopen(fileName, "rb");
    std::ifstream((char *)fileName);
}

::wifstream::wifstream( ) {                    
}

int ::wifstream::open(char *fileName) {        
    filePtr = fopen(fileName, "rb");
    std::ifstream((char *)fileName);
    return 0 ;
}

void ::wifstream::freefile(void) {             
   fclose(filePtr);
   close();
}

wchar_t* ::wifstream::getline(wchar_t *pwch, int nCount, BOOL isUTF16) {
    wchar_t *ptr;

    if ( isUTF16 ) {
        if ( (fgetws(pwch, nCount, this->filePtr) != NULL) ) {
            
			if ( feof(this->filePtr) ) {
                if ( ( wcslen(pwch) == 0 ) ||      
                     ( pwch[0] == '\x1A' ) ) 
                   return NULL ;
                if ( pwch[wcslen(pwch)-1] == '\x1A' ) 
                   pwch[wcslen(pwch)-1] = NULL ;
            }
            ptr = wcsrchr(pwch, '\r');         
            if (ptr == NULL)
                ptr = wcsrchr(pwch, '\n');     

            if (ptr) {
                *(ptr) = NULL;
            }

            RemoveBOM(pwch);
            return pwch;
        }
        else {
            return NULL;
        }
    }
    else {
       char    szTemp[MAX_BUFFER_SIZE*3];
        if ( (fgets(szTemp, nCount, this->filePtr) != NULL) ) {

            MultiByteToWideChar(CP_UTF8, 0, szTemp, sizeof(szTemp), pwch, nCount);                                    

            ptr = wcsrchr(pwch, '\r');         
            if (ptr == NULL )
                ptr = wcsrchr(pwch, '\n');     

            if (ptr)
                *(ptr) = NULL;

            RemoveBOM(pwch);
            return pwch;
        }
        else {
            return NULL;
        }
    }
}

char* ::wifstream::getline(char *pch, int nCount, BOOL isUTF16) {
    char *ptr;

    if( (fgets(pch, nCount, this->filePtr) != NULL) ) {
        
		if ( feof(this->filePtr) ) {
            if ( ( strlen(pch) == 0 ) ||       
                 ( pch[0] == '\x1A' ) ) 
               return NULL ;
            if ( pch[strlen(pch)-1] == '\x1A' ) 
               pch[strlen(pch)-1] = NULL ;
		}
		
		ptr = strrchr(pch, '\r');              
        if(ptr == NULL )
            ptr = strrchr(pch, '\n');          
        
        if(ptr)
           *(ptr) = 0;
        return pch;
    }
    else {
        return NULL;
    }
}

wchar_t* ::wifstream::get(wchar_t *pwch, int nCount, BOOL isUTF16, BOOL includeBOM) {
    wchar_t *ptr;

    if ( isUTF16 ) {
        if ( fgetws(pwch, nCount, this->filePtr) != NULL ) {
            
			if ( feof(this->filePtr) ) {
                if ( wcslen(pwch) == 0 ) 
                   return NULL ;
            }

            if ( ! includeBOM ) 
               RemoveBOM(pwch);
            return pwch;
        } else {
            return NULL;
        }
    } else {
       char    szTemp[MAX_BUFFER_SIZE*3];
        if ( fgets(szTemp, nCount, this->filePtr) != NULL ) {

            MultiByteToWideChar(CP_UTF8, 0, szTemp, sizeof(szTemp), pwch, nCount);                                    

            if ( ! includeBOM ) 
               RemoveBOM(pwch);
            return pwch;
        } else {
            return NULL;
        }
    }
}

char* ::wifstream::get(char *pch, int nCount, BOOL isUTF16) {
    char *ptr;

    if( fgets(pch, nCount, this->filePtr) != NULL ) {
        
		if ( feof(this->filePtr) ) {
            if ( strlen(pch) == 0 ) 
               return NULL ;
		}
        return pch;
    } else {
        return NULL;
    }
}


int ::wifstream::feoft() {    
    return feof(filePtr);
}

long ::wifstream::ftellt() {
    return ftell(this->filePtr);
    
}

long ::wifstream::tellg() {
    return ftell(this->filePtr);
    
}

int ::wifstream::fseekt(long offset, int origin ) {
    return fseek(this->filePtr, offset, origin);
}

int ::wifstream::seekg(long offset, int origin ) {
    return fseek(this->filePtr, offset, origin);
}
//////////////////////////////////////////////////////


///////////wofstream//////////////////////////////////

::wofstream::wofstream(char *fileName) {    
    filePtr = fopen(fileName,"ab") ;
    std::ofstream(fileName, std::ios::out|std::ios::binary);
    this->setf(std::ios::unitbuf);
}

::wofstream& ::wofstream::operator <<(wint_t nNumber) {
    fwprintf( filePtr, L"%d", nNumber ) ;
    fflush( filePtr ) ;
    return (wofstream&)*this;
} 

::wofstream& ::wofstream::operator <<( wchar_t *szBuffer) {
   fputws( szBuffer, filePtr ) ;
   fflush( filePtr ) ;
   return (wofstream&)*this;
}

::wofstream& ::wofstream::operator <<(IString is) {
    _TCHAR  szBuffer[MAX_BUFFER_SIZE*3];
   _tcscpy(szBuffer, is.GetBuffer(is.GetLength()));
   is.ReleaseBuffer(is.GetLength());
   fputws( szBuffer, filePtr ) ;
   fflush( filePtr ) ;
   return (wofstream&)*this;
}


//////////////////////////////////////////////////////


/////////////Member functions of IString ///////////////////////
IString& IString::change( _TCHAR* pInputString, _TCHAR* pOutputString, int startPos, unsigned numChanges ) {    
///    this->Replace(pInputString, pOutputString);

    _TINT  count=0;
    _TINT  start=0;
    int  found=0;
    
    if ( startPos > 0 )
       start=startPos-1 ;
    for( ; count<numChanges ; ++count ) {
       found=this->Find(pInputString,start);
       if ( found < 0 ) break ;
       this->Delete(found,wcslen(pInputString));
       this->Insert(found,pOutputString);
       start=found+wcslen(pOutputString);
    }
    return *this;
}


IString& IString::copy( _TINT numCopies){
    //numCopies is always 1 in the code. So basically this functions is used as in operator=() function.
    return *this;
}

IString& IString::remove(int startPos, int numChars){
    CString str;

    this->Delete(startPos-1, numChars);
    return *this;
}

bool IString::includes(_TCHAR * pString) const {
   CString str,str2;

    str = *this;
    str.MakeUpper();
    str2 = pString;
    str2.MakeUpper();
    
    if ( str.Find(str2) >= 0 )
        return TRUE ;
    else
       return FALSE ;

}

int IString::indexOf(LPCTSTR pString, int startPos) const {
    int ret;
    
    ret = this->Find(pString, startPos-1);
    if( ret == -1 )
        return 0;
    return ret+1;
}


int IString::indexOf(_TCHAR * pString) const {
    int ret;

    ret = this->Find(pString);
    if( ret == -1 )
        return 0;

    return ret+1;
}

int IString::indexOfAnyOf(LPCTSTR pString, int startPos) const {
    int start=0;
    int length=this->GetLength();

    if ( startPos > 0 )
       start=startPos-1 ;

    for( ; start<length ; ++start ) {
       if ( wcschr( pString, this->GetAt(start) ) )
          return (start+1) ;
    }
    return 0 ;
}

int IString::indexOfAnyBut(LPCTSTR pString, int startPos) const {
    int start=0;
    int length=this->GetLength();

    if ( startPos > 0 )
       start=startPos-1 ;

    for( ; start<length ; ++start ) {
       if ( ! wcschr( pString, this->GetAt(start) ) )
          return (start+1) ;
    }
    return 0 ;
}

int IString::lastIndexOf(_TCHAR * pString, int idx) const {
	int ret,found;
	CString s1;

	s1 = this->Left(idx);
	if( s1.IsEmpty() )
		return 0;

   for(ret=-1,found=1 ; found>=0 ; ) {
      found=s1.Find(pString,ret+1);
      if ( found>=0 ) 
         ret=found;
   }
	
	return ret+1;
}

int IString::lastIndexOf(_TCHAR * pString) const {
	int ret,found;

   for(ret=-1,found=1 ; found>=0 ; ) {
      found=this->Find(pString,ret+1);
      if ( found>=0 ) 
         ret=found;
   }
	
	return ret+1;
}

int IString::lastIndexOf(_TCHAR ch, int idx) const {
	int ret;
	CString s1;

	s1 = this->Left(idx);
	if( s1.IsEmpty() )
		return 0;
	else
		ret = s1.ReverseFind(ch);
	
	return ret+1;
}

int IString::lastIndexOf(_TCHAR ch) const {
	int ret;

   ret = this->ReverseFind(ch);
	
	return ret+1;
}


int IString::length() const {

    return this->GetLength();
}

IString IString::subString(_TINT startPos) const {
    CString str;
    
    str = this->Mid(startPos-1);
    return (IString&)str;
}

IString IString::subString(_TINT startPos, _TINT length, TCHAR padCharacter ) const {
    CString str;
    
    str = this->Mid(startPos-1, length);
    return (IString&)str;
}

IString& IString::overlayWith(const _TCHAR* pOverlayString, int index ) {    
    int  start=0;
    int  length1=this->GetLength();
    int  length2=wcslen(pOverlayString);
   
    if ( index > 0 )
       start=index-1 ;
    for( ; length1<start ; *this+=L" ", ++length1) ;
    this->Delete(start,length2);
    this->Insert(start,pOverlayString);
    return *this;
}

bool IString::isWhiteSpace() const {
    CString str;

    str = *this;
    str.TrimRight();
    return str.IsEmpty();
}

IString& IString::insert(const _TCHAR* pString, _TINT index, _TCHAR padChar ) {
    this->Insert(index/*intended,no decrement needed*/, pString);
    return *this;
}

IString& IString::lowercase() {
    this->MakeLower();
    return *this;
}

IString& IString::uppercase() {
    this->MakeUpper();
    return *this;
}

IString& IString::upperCase() {
    this->MakeUpper();
    return *this;
}

IString& IString::lowerCase() {
    this->MakeLower();
    return *this;
}

IString& IString::stripLeading() {
    this->TrimLeft();
    return *this;
}

IString& IString::stripTrailing() {
    this->TrimRight();
    return *this;
}

IStringEnum::CharType IString::charType(int index) const {
    if( isleadbyte(index) )
        return IStringEnum::dbcs1;
    else 
        return IStringEnum::sbcs;
}

wchar_t IString::operator [] (int nIndex) {
    return this->GetAt(nIndex-1);
}


IString IString::lineFrom(::wifstream& astream, _TCHAR delim) {  
    _TCHAR szBuffer[MAX_BUFFER_SIZE];
    IString str;

    astream.getline(szBuffer, _tcslen(szBuffer), delim);
    str = szBuffer;
    return str;
}
//////////////////////////////////////////////////////////////////////


int ::wifstream::IsUTF16() {
    FILE *stream;
    BOOL ret = FALSE;
    char list[4];
    int  i, numread, numwritten;

    numread = fread( list, sizeof( char ), 3, filePtr );
    if( (list[0] == 0xFFFFFFFF) && (list[1] == 0xFFFFFFFE) ) {
        ret = TRUE;
    }
    else {
        ret = FALSE;
    }
    fseek(filePtr,0,SEEK_SET);
    return ret;
}

BOOL IsUTF16(char *fileName) {
    FILE *stream;
    BOOL ret = FALSE;
    char list[4];
    int  i, numread, numwritten;

    if( (stream = fopen( fileName, "rb" )) != NULL ) {
       numread = fread( list, sizeof( char ), 3, stream );
       if( (list[0] == 0xFFFFFFFF) && (list[1] == 0xFFFFFFFE) ) {
           ret = TRUE;
       }
       else {
           ret = FALSE;
       }
       fclose( stream );
    }
    return ret;
}


int ::wofstream::InsertBOM() {
#ifdef _UNICODE

    fprintf(filePtr, "\xFF\xFE");
    fflush( filePtr ) ;
#endif
    return TRUE;
}

int ::wifstream::RemoveBOM(wchar_t *szBuffer) {
#ifdef _UNICODE
    wchar_t *ptrBuf;

    ptrBuf = szBuffer;
    if( !ptrBuf )
        return TRUE;

    if ( ptrBuf  && (*ptrBuf == BYTE_ORDER_MARK) ) {
        wmemmove( ptrBuf, ptrBuf+1, wcslen(ptrBuf+1)+1 ) ;
        return TRUE;
    }
#endif
    return FALSE;
}


void mbox(char *str) {
    MessageBoxA(NULL, str, str, MB_OK);
}


void mbox(wchar_t *str) {
    MessageBoxW(NULL, str, str, MB_OK);
}


BOOL RemoveBOM(wchar_t *szBuffer) {

#ifdef _UNICODE
    wchar_t *ptrBuf;

    ptrBuf = szBuffer;
    if( !ptrBuf )
        return TRUE;

    if ( ptrBuf  && (*ptrBuf == BYTE_ORDER_MARK) ) {
       wmemmove( ptrBuf, ptrBuf+1, wcslen(ptrBuf+1)+1 ) ;
        return TRUE;
    }
#endif
    return FALSE;
}

