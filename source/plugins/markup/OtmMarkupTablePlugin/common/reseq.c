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


#include <windows.h>
#include <wtypes.h>


#define DosCopy(FILE1, FILE2, BOOL) CopyFile(FILE1, FILE2, FALSE)


#include <sys\stat.h>
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "reseq.h"
#include "usrcalls.h"

#define  MAXLEN   4096
#define  QFLEN    41       /* Maximum length of tmpString:40 characters, ":QFX N=123456 S=1 X=2000000500060005565B." */

extern BOOL __cdecl resequence_TM2(char *fileName)
{

   FILE  *sourceFile_ptr = NULL;
   FILE  *tempFile_ptr = NULL;
   WCHAR *tmp_ptr;
   WCHAR tmpBuff[MAXLEN+512+QFLEN];             
   WCHAR upperCaseBuff[MAXLEN+512+QFLEN];       
   WCHAR newBuff[MAXLEN+512+QFLEN];             

   BOOL bPossibleQfxCutOff = FALSE;
   WCHAR tmpString[QFLEN];
   WCHAR *pLastColon = NULL;
   int tmpLength;

   char  tempName[256] ;
   WCHAR *delimiters = L" ";
   WCHAR *token;
   WCHAR *start_ptr;
   WCHAR *end_ptr;
   long  sequenceNum;
   int   offset;
   WCHAR nSequence[256];

   char *szAltTempExt1 = ".$$Q";


   memset(tmpBuff, 0, sizeof(tmpBuff));
   memset(newBuff, 0, sizeof(newBuff));
   memset(upperCaseBuff, 0, sizeof(upperCaseBuff));
   memset(nSequence, 0, sizeof(nSequence));
     memset(tmpString,0,sizeof(tmpString)); 

   CreateTempFileName2( tempName, fileName, szAltTempExt1, TEMPNAME_SSOURCE ) ;

   sourceFile_ptr = fopen(fileName, "rb");
   tempFile_ptr = fopen(tempName, "wb");
   sequenceNum = 0;
   while ((tmp_ptr = fgetws(tmpBuff, MAXLEN, sourceFile_ptr)) != NULL){

       tmpLength = wcslen(tmpBuff);
       if(bPossibleQfxCutOff){            // append tmpBuff to tmpString ":***" from the last line or reading
     	  if(tmpString[0] != (WCHAR)NULL){
     		  wcscpy(newBuff, tmpString ) ;
     		  wcscat(newBuff,tmpBuff);
     		  wcscpy(tmpBuff,newBuff);
     		  memset(tmpString,0,sizeof(tmpString));  
     		  memset(newBuff,0,sizeof(newBuff));  
     	  }
     	  bPossibleQfxCutOff = FALSE;	  
       }
       if(tmpLength == MAXLEN -1){
     	  if((pLastColon=wcsrchr(tmpBuff, L':')) !=NULL){ 
     		  if(*(pLastColon+1) == (WCHAR)NULL ) 
     			   bPossibleQfxCutOff = TRUE;
     		   else 
                if(*(pLastColon+1) == L'Q' || *(pLastColon+1) == L'q'){    
     			   if(*(pLastColon+2) == (WCHAR)NULL ) 
     				   bPossibleQfxCutOff = TRUE;
     			   else 
     			   if(*(pLastColon+2) == L'F'|| *(pLastColon+2) == L'f'){    /* xx:QF */
     				   if ( ! wcschr(pLastColon, L'.') ) /* Incomplete :QF tag */
     						bPossibleQfxCutOff = TRUE;	
     			   }
     		   }

                if(bPossibleQfxCutOff){
                  wcscpy(tmpString, pLastColon);      
                  *pLastColon = (WCHAR)NULL;       
                }
     	  }
      }
      wcscpy(upperCaseBuff, tmpBuff);
      wcsupr(upperCaseBuff);
      token = wcstok(upperCaseBuff, delimiters);
      offset = 0;
      while(token != NULL){
         if(wcsstr(token, L":QF") != NULL){
            if((token = wcstok(NULL, delimiters)) != NULL){
               if((start_ptr = wcsstr(token, L"N=")) != NULL){
                  if((end_ptr = wcsstr(start_ptr,L".")) != NULL){
                     sequenceNum = sequenceNum +1;
                     swprintf(nSequence, 20, (WCHAR*)L"n=%ld",(long)sequenceNum);
                     wcsncpy(newBuff, tmpBuff, ((start_ptr - upperCaseBuff) + offset));
                     wcscat(newBuff, nSequence);
                     wcscat(newBuff, &tmpBuff[((end_ptr - upperCaseBuff) + offset)]);
                     offset = wcslen(nSequence) - (end_ptr - start_ptr) + offset;
                     memset(tmpBuff, 0, sizeof(tmpBuff));
                     wcscpy(tmpBuff, newBuff);
                     memset(newBuff, 0, sizeof(newBuff));
                     memset(nSequence, 0, sizeof(nSequence));
                  }
               }
            }
         }
         else{
            token = wcstok(NULL, delimiters);
         }
      }
      fputws(tmpBuff, tempFile_ptr);
   }
   if(bPossibleQfxCutOff){  /* Rail strings not being handled yet */
       if(tmpString != NULL)
     	fputws(tmpString, tempFile_ptr);
       bPossibleQfxCutOff = FALSE;
   }

   fclose(sourceFile_ptr);
   fclose(tempFile_ptr);
   CopyFileA(tempName, fileName, FALSE);
   remove(tempName);
   return TRUE;
}



