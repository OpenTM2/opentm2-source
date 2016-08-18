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
//   THIS FILE CONTAINS GENERAL PURPOSE ROUTINES
//
//**********************************************************************************************************



#define STRING_SIZE 4096    

#include "usrcalls.h"


   short   sTPVersion = TP_UNKNOWN ;
   char    DBCSRanges[12]={0,0,0,0,0,0,0,0,0,0,0,0};
   char    szDocTargetLanguage[80];
   char    szDocSourceLanguage[80];



extern void __cdecl openFileToPosition(char *file,long filePos,FILE **fp)
{

   *fp = fopen (file,"rb");

   fseek(*fp,filePos,SEEK_SET);
}

/****************************************************************************/
/*                                                                          */
/* StrUpr                                                                   */
/*                                                                          */
/* Uppercases a string                                                      */
/*                                                                          */
/****************************************************************************/

extern _TCHAR * __cdecl StrUpr( _TCHAR * Str )
{
_TCHAR *cptr;

   for (cptr = Str;cptr[0];cptr++)
      {
      cptr[0] = _totupper(cptr[0]);
      }

   return (Str);
}

/****************************************************************************/
/*                                                                          */
/* StrnUpr                                                                  */
/*                                                                          */
/* Uppercases a portion of a string                                         */
/*                                                                          */
/****************************************************************************/

extern _TCHAR * __cdecl StrnUpr( _TCHAR * Str, short starting, short numBytes )
{
_TCHAR *cptr;

   for (cptr = &Str[starting];
       (cptr[0]) && ((cptr - &Str[starting]) < numBytes)
       ;cptr++)
      {
      cptr[0] = _totupper(cptr[0]);
      }

   return (Str);
}

/****************************************************************************/
/*                                                                          */
/* cleanString                                                              */
/*                                                                          */
/* Remove dummy tags from String                                            */
/*                                                                          */
/****************************************************************************/

extern _TCHAR * __cdecl cleanString( _TCHAR * tempString )
{
  int htTranPos;
  int htTextPos;
  _TCHAR htTran[STRING_SIZE];

  if (strloc(tempString,_T("&httext.") ) >= 0)
     {
       _tcscpy(tempString,deleteSubString(tempString, _T("<HTFWM") ));
       htTranPos = strloc(tempString, _T("></HTFWM>") );
       _tcscpy(tempString,deleteSubString(tempString, _T("></HTFWM>") ));
       _tcscpy(htTran,strsub(tempString,htTranPos,(strloclast(tempString, _T("\n") ) - htTranPos)));
       _tcscpy(tempString,strdel(tempString,htTranPos,(strloclast(tempString, _T("\n") ) - htTranPos)));
       htTextPos = strloc(tempString, _T("&httext.") );
       _tcscpy(tempString,strdel(tempString,htTextPos,8));
       _tcscpy(tempString,strins(tempString,htTran,htTextPos));
     }
  _tcscpy(tempString,deleteSubString(tempString, _T("<HTMAC") ));
  _tcscpy(tempString,deleteSubString(tempString, _T("></HTMAC>") ));
  _tcscpy(tempString,deleteSubString(tempString, _T("<HTFWM") ));
  _tcscpy(tempString,deleteSubString(tempString, _T("></HTFWM>") ));


  return tempString;
}



/****************************************************************************/
/*                                                                          */
/* deleteSubString                                                          */
/*                                                                          */
/* Delete all occurences of a substring from a string                       */
/*                                                                          */
/****************************************************************************/

extern _TCHAR * __cdecl deleteSubString( _TCHAR * string1, _TCHAR * string2 )
{
   int i,j;
   int lengthSubString=0;
   int lengthMainString=0;
   int afterStringMatch=0;
   int startStringMatch=0;
   _TCHAR stringMatch;
   _TCHAR foundString;


   while (_tcsstr(string1,string2) != NULL)
      {
        foundString = 'N';
        lengthSubString = _tcslen(string2);
        lengthMainString = _tcslen(string1);
        for (i=0; i<=lengthMainString; i++)
           {

             j = 0;
             if ( string1[i] == string2[j] )
                {
                  stringMatch = 'Y';
                  for ( j=0; j<=(lengthSubString - 1); j++)
                     {
                       if (string1[i+j] == string2[j])
                          {

                          }
                       else
                          {
                            stringMatch = 'N';
                          }
                     }
                  if (stringMatch == 'Y')
                     {
                       startStringMatch = i;
                       afterStringMatch = i + j;
                       foundString = 'Y';
                     }
                }
           }

        if (foundString == 'Y')
           {
             for ( i=0; (i+afterStringMatch)<=lengthMainString; i++)
                {
                  string1[startStringMatch + i] = string1[afterStringMatch + i];
                }

           }

      }
   return string1;
}


/****************************************************************************/
/*                                                                          */
/* strloc                                                                   */
/*                                                                          */
/* Locate first occurence of substring in string and return substring index */
/*                                                                          */
/****************************************************************************/

extern int __cdecl strloc( _TCHAR * string1, _TCHAR * string2 )
{
  int length1, length2;
  int i, j;
  _TCHAR match;
  _TCHAR lstring1[STRING_SIZE];
  _TCHAR lstring2[STRING_SIZE];
  _TCHAR * lw1;
  _TCHAR * lw2;

  length1 = _tcslen(string1);
  length2 = _tcslen(string2);

  _tcscpy(lstring1,string1);
  _tcscpy(lstring2,string2);

  lw1 = _tcslwr(lstring1);
  lw2 = _tcslwr(lstring2);

  if (length2 > length1)
     {
       return -1;
     }
  else
     {
       for (i=0; i <= (length1-length2); i++)
          {
            match = 'Y';
            for (j=0; j< (length2); j++)
               {
                 if (lstring1[j+i] ==  lstring2[j])
                    {
                    }
                 else
                    {
                      match = 'N';
                    }
               }
            if (match == 'Y')
               {
                 return i;
               }
          }

     }


  return -1;
}

/****************************************************************************/
/*                                                                          */
/* strlocnext                                                               */
/*                                                                          */
/* Locate occurence of substring in string starting at specified position   */
/* and return substring index.                                              */
/*                                                                          */
/****************************************************************************/

extern int __cdecl strlocnext( _TCHAR * string1, _TCHAR * string2, int next )
{
  int length1, length2;
  int i, j;
  _TCHAR match;
  _TCHAR lstring1[STRING_SIZE];
  _TCHAR lstring2[STRING_SIZE];
  _TCHAR * lw1;
  _TCHAR * lw2;

  length1 = _tcslen(string1);
  length2 = _tcslen(string2);

  _tcscpy(lstring1,string1);
  _tcscpy(lstring2,string2);

  lw1 = _tcslwr(lstring1);
  lw2 = _tcslwr(lstring2);

  if (length2 > length1)
     {
       return -1;
     }
  else
     {
       for (i=next; i <= (length1-length2); i++)
          {
            match = 'Y';
            for (j=0; j< (length2); j++)
               {
                 if (lstring1[j+i] ==  lstring2[j])
                    {
                    }
                 else
                    {
                      match = 'N';
                    }
               }
            if (match == 'Y')
               {
                 return i;
               }
          }

     }


  return -1;
}

/****************************************************************************/
/*                                                                          */
/* strnextnonblank                                                          */
/*                                                                          */
/* Locate occurence of next non-blank character in string starting at       */
/* specified position and return substring index.                           */
/*                                                                          */
/****************************************************************************/

extern int __cdecl strnextnonblank( _TCHAR * string1, int next )
{
  int length1;
  int i;
  _TCHAR lstring1[STRING_SIZE];
  _TCHAR * lw1;

  length1 = _tcslen(string1);

  _tcscpy(lstring1,string1);

  lw1 = _tcslwr(lstring1);

  if (next > (length1-1))
     {
       return -1;
     }
  else
     {
       i = next;
       while (lstring1[i] == ' ')
          {
           i++;
           if (i>(length1-1))
              {
                return -1;
              }
          }
       return i;
     }


//  return -1;
}
/****************************************************************************/
/*                                                                          */
/* strloclast                                                               */
/*                                                                          */
/* Locate last occurence of substring in string and return substring index  */
/*                                                                          */
/****************************************************************************/

extern int __cdecl strloclast( _TCHAR * string1, _TCHAR * string2 )
{
  int length1, length2;
  int i, j;
  _TCHAR match;
  _TCHAR lstring1[STRING_SIZE];
  _TCHAR lstring2[STRING_SIZE];
  _TCHAR * lw1;
  _TCHAR * lw2;

  length1 = _tcslen(string1);
  length2 = _tcslen(string2);

  _tcscpy(lstring1,string1);
  _tcscpy(lstring2,string2);

  lw1 = _tcslwr(lstring1);
  lw2 = _tcslwr(lstring2);

  if (length2 > length1)
     {
       return -1;
     }
  else
     {
       for (i=(length1-length2); i >= 0; i--)
          {
            match = 'Y';
            for (j=0; j< (length2); j++)
               {
                 if (lstring1[j+i] ==  lstring2[j])
                    {
                    }
                 else
                    {
                      match = 'N';
                    }
               }
            if (match == 'Y')
               {
                 return i;
               }
          }

     }


  return -1;
}


/****************************************************************************/
/*                                                                          */
/* strsub                                                                   */
/*                                                                          */
/* Return a substring of the specified length from another string           */
/* starting at the specified index                                          */
/*                                                                          */
/****************************************************************************/

extern _TCHAR * __cdecl strsub( _TCHAR * string1, int subStart, int subLength )
{
    _TCHAR subString[STRING_SIZE];
    int length1;
    int i;
    _TCHAR szTemp[STRING_SIZE];

    length1 = _tcslen(string1);

    if (length1 >= (subStart + subLength)) {

        for (i=subStart; i<(subStart+subLength);i++) {
            subString[i - subStart] = string1[i];
            _stprintf(szTemp, _T("\ni=%d\n"),i);
        }
        subString[i - subStart] = '\0';
        return subString;
    }
    else {
        return NULL;
    }
}


/****************************************************************************/
/*                                                                          */
/* strdel                                                                   */
/*                                                                          */
/* Delete a specified number of characters from a string starting at the    */
/* specified index                                                          */
/*                                                                          */
/****************************************************************************/

extern _TCHAR * __cdecl strdel(_TCHAR * string1, int delStart, int delLength )
{
  _TCHAR delString[STRING_SIZE];
  int length1;
  int i,j;

  length1 = _tcslen(string1);

  if (length1 >= (delStart + delLength))
     {
       for (i=0,j=0; i<length1; i++)
          {
            if ((i<delStart)||(i>=(delStart+delLength)))
               {
                 delString[j] = string1[i];
                 j++;
               }
          }
       delString[j] = '\0';

       memmove (string1,delString, (_tcslen(delString) + 1)*sizeof(_TCHAR));

       return string1;
     }
  else
     {
       return NULL;
     }
}


/****************************************************************************/
/*                                                                          */
/* strins                                                                   */
/*                                                                          */
/* Insert a substring into another string before the specified index        */
/*                                                                          */
/****************************************************************************/

extern _TCHAR * __cdecl strins( _TCHAR * string1, _TCHAR * string2, int insIndex )
{
  _TCHAR insString[STRING_SIZE];
  int length1;
  int length2;
  int i,j,k;

  length1 = _tcslen(string1);
  length2 = _tcslen(string2);

  for (i=0,j=0; i<length1; i++)
     {
       if (i == insIndex)
          {
            for (k=0; k<(length2); k++)
               {
                 insString[j] = string2[k];
                 j++;
               }
          }
       insString[j] = string1[i];
       j++;
     }
  insString[j] = '\0';

  memmove (string1,insString, (_tcslen(insString) + 1)*sizeof(_TCHAR));

  return string1;
}


/****************************************************************************/
/*                                                                          */
/* strCOPY                                                                  */
/*                                                                          */
/* Copy string 2 to string 1.                                               */
/*                                                                          */
/****************************************************************************/

extern void __cdecl strCOPY( _TCHAR * string1, _TCHAR * string2 )
{
  int i;
  int length2;

  length2 = _tcslen(string2);

  for (i=0; i<=length2; i++)
     {
       string1[i] = string2[i];
     }
}

/****************************************************************************/
/*                                                                          */
/* searchAndReplaceString                                                   */
/*                                                                          */
/* This will search(case insensitive) and replace all instances of a string */
/* within a string.                                                         */
/*                                                                          */
/****************************************************************************/

extern short __cdecl searchAndReplaceString(_TCHAR *buffer,_TCHAR *search_str,_TCHAR *replace_str,short maxLen){
_TCHAR *tmpBuf = NULL;
_TCHAR *tmpSearchStr = NULL;
_TCHAR *cptr;
short offs,bufOffs = 0;

 tmpSearchStr = (_TCHAR *) calloc (1, (_tcslen(search_str) + 1)*sizeof(_TCHAR));
 tmpBuf = (_TCHAR *) calloc (1,maxLen*sizeof(_TCHAR));
 if (tmpBuf != NULL)
    {
    memcpy (tmpBuf,buffer, (_tcslen(buffer))*sizeof(_TCHAR));
    StrUpr(tmpBuf);
    memcpy (tmpSearchStr,search_str, (_tcslen(search_str))*sizeof(_TCHAR));
    StrUpr(tmpSearchStr);
    while ((cptr = _tcsstr (tmpBuf,tmpSearchStr)) != NULL)
       {
       offs = cptr - tmpBuf;
       strdel(&buffer[bufOffs+offs],0,_tcslen(tmpSearchStr));
       strins(&buffer[bufOffs+offs],replace_str,0);
       bufOffs = bufOffs + offs + _tcslen(replace_str); /* needed in case of multiple replacements */
       memmove(tmpBuf,&tmpBuf[offs+_tcslen(tmpSearchStr)], (_tcslen(&tmpBuf[offs+_tcslen(tmpSearchStr)]) + 1)*sizeof(_TCHAR));
       }
    free (tmpBuf);
    free (tmpSearchStr);
    }

 return (1);
}

/****************************************************************************/
/*                                                                          */
/* copyPartialFile                                                          */
/*                                                                          */
/* Copy srcFile to tmpFile.                                                 */
/*                                                                          */
/****************************************************************************/

extern long __cdecl copyPartialFile (char *srcFile, FILE **sfp,
                      char *tmpFile, FILE **tfp,
                      long startPos, long numBytes,
                      short newFile)
{
int c;
long fPos = -1;

   if ((*sfp) == NULL)
      {
      *sfp = fopen(srcFile,"rb");
      }

   if ((*tfp) == NULL)
      {
      if (newFile)
         {
         *tfp = fopen(tmpFile,"wb");
         }
      else
         {
         *tfp = fopen(tmpFile,"ab");
         }
      }

   if (((*tfp) != NULL) && (*sfp != NULL))
      {
      fseek (*sfp, startPos, SEEK_SET);
      while ((ftell(*sfp) < (startPos+numBytes)) && (!feof(*sfp)) && (!ferror(*sfp)))
         {
         if ((c = _fgettc(*sfp)) != _TEOF)
            {
            _fputtc(c,*tfp);
            }
         }
      fPos = ftell(*sfp);
      }

   return (fPos);
}

/****************************************************************************/
/*                                                                          */
/* StrSearchFile                                                            */
/*                                                                          */
/* Searches a file for a particular string.                                 */
/*                                                                          */
/* Returns:    -1 = Error;                                                  */
/*           -100 = EOF (not found string);                                 */
/*           -100 = EndPos (not found string);                              */
/*           -101 = EOL (not found string)                                  */
/*                                                                          */
/****************************************************************************/

extern long  __cdecl StrSearchFile (FILE  **fp,         /* open file pointer                          */
                                    _TCHAR  *str,         /* string to look for                         */
                                    long  startPos,     /* The starting position > endPos if backward */
                                    long  endPos,       /* The ending position < startPos if backward */
                                    _TCHAR  *delims,      /* a set of chars to indicated end of search  */
                                    long  *foundPos)    /* position the string was found in the file  */
{
    _TCHAR *readBuf;
    long  rc = -100; /* -100 = NOT FOUND, -1 = Error  */
    int offs = 0,done = 0,c;
    struct stat statBuf;
    long  ePos,sPos;

    fstat(fileno(*fp),&statBuf);
    ePos = endPos;
    sPos = startPos;

    if ((ePos < 0) && (sPos < 0)) {
        sPos = 0;
        ePos = statBuf.st_size;            /* set to the end of file */
    }

    if (ePos < 0) {     /* implied forward search to the end */
        if (sPos >= 0) {
            ePos = statBuf.st_size;         /* set to the end of file */
        }
    }

    if (sPos < 0) {
        if (ePos >= 0) {  /* implied backward search from the end */
            sPos = statBuf.st_size - 1*sizeof(_TCHAR);     /* set to the end of file */
        }
    }

    offs = 1;   /* forward search */
    if (sPos > ePos) {
        offs = -1;  /* backward search */
    }

    *foundPos = -1;
    readBuf = (_TCHAR *) calloc (_tcslen(str) + 2,1*sizeof(_TCHAR));

    if (readBuf != NULL) {
        fseek (*fp,sPos,SEEK_SET);
        for (c = _fgettc(*fp); ((!feof(*fp)) && (!ferror(*fp)) && (!done));) {
            rc = 0;
            if (offs > 0) {  /* forward search */
                readBuf[_tcslen(readBuf)] = _totupper(c);
                if (_tcslen(readBuf) > _tcslen(str)) {
                    memmove(&readBuf[0],&readBuf[1], (_tcslen(str)+1)*sizeof(_TCHAR));
                }
            }
            else {  /* backward search */
                if (_tcslen(str) > 1*sizeof(_TCHAR))   /* always move unless 1 char string */
                    memmove(&readBuf[1],&readBuf[0], (_tcslen(str) - 1)*sizeof(_TCHAR));
                readBuf[0] = _totupper(c);
            }
            if (!_tcscmp(readBuf,str)) {
                done = 1;
                if (offs > 0) {  /* forward search */
                    *foundPos = ftell(*fp) - (_tcslen(str))*sizeof(_TCHAR);
                }
                else {  /* backward search */
                    *foundPos = ftell(*fp) - 1*sizeof(_TCHAR);
                }
            }
            if (!done) {
                sPos+=offs*sizeof(_TCHAR);
                if (((sPos < ePos) && (offs == -1)) ||
                    ((sPos > ePos) && (offs == 1))) {
                    rc = -100;
                    done = 1;
                }
            }
            if (!done) {
                if (offs < 0) {  /* file pointer needs to be set every time on a backward search */
                    if (fseek (*fp,sPos,SEEK_SET)) {
                        rc = -100;
                        done = 1;
                    }
                }
            }
            if (!done) {
                c = _fgettc(*fp);
                if (c != _TEOF) {
                    if (_tcschr(delims,c)) {
                        done = 1;
                        rc = -101;             /* found end delimiter */
                    }
                }
                else {
                    if (feof(*fp))
                        rc = -100;
                    else
                        rc = -1;
                }
            }
        }

        free(readBuf);
    }

    return(rc);
}



/*****************************************************************************/
/*                                                                           */
/* CreateTempFileName                                                        */
/*                                                                           */
/* Get a temporary file name in a temp directory.                            */
/*                                                                           */
/* Parms:     pName         - Name of temporary file.                        */
/*            pAltName      - Name of alternate file name.                   */
/*                                                                           */
/* Returns:   None.                                                          */
/*                                                                           */
/*****************************************************************************/
extern void __cdecl CreateTempFileName( PSZ pName, PSZ pAltName ) 
{
   FILE   *fTemp ;
   struct stat statBuf;
   char   szTemp[256] ;  
   char   *ptrChar, *ptrChar2 ;
   int    iNum ; 

   ptrChar2 = strrchr( pAltName, '\\' ) ;
   if ( ptrChar2 ) 
      ++ptrChar2 ; 
   else
      ptrChar2 = pAltName ;

   ptrChar = tempnam( NULL, ptrChar2 ) ;     
   if ( ptrChar == NULL) {
      strcpy( pName, pAltName ) ;
      ptrChar = strrchr( pName, '.' ) ;
      if ( ptrChar ) 
         ptrChar = 0 ;
      srand( time(NULL) ) ;                       /* Get seed value from system clock */
      iNum = rand() ;
      sprintf( szTemp, "%s.%d", pName, iNum ) ;
      strcpy( pName, szTemp ) ; 
   } else {
      sprintf( pName, "%s%lx", ptrChar, pName ) ; 
      free( ptrChar ) ;
   }

   strcpy( szTemp, pName ) ;                 
   for( iNum=0 ; iNum < 20 ; ++iNum ) {        /* Get unique file name which does not exist */
      if ( iNum > 0 ) 
         sprintf( pName, "%s%d", szTemp, iNum ) ;
      if ( stat(pName,&statBuf) ) 
         break;
   }

   fTemp = fopen( pName, "w" ) ;               /* Avoid duplicate temp names */
   fprintf(fTemp, " " ) ;
   fclose( fTemp ) ;

   return ;
} 


/*****************************************************************************/
/*                                                                           */
/* CreateTempFileName2                                                       */
/*                                                                           */
/* Get a temporary file name in TM directory.                                */
/*                                                                           */
/* Parms:     pName         - Name of temporary file.                        */
/*            pBase         - Base path/name to base temporary name on.      */
/*            pAltExt       - Extension to append to the name.               */
/*            Folder        - 1= SOURCE                                      */
/*                            2= SSOURCE                                     */
/*                            3= STARGET                                     */
/*                            4= TARGET                                      */
/* Returns:   None.                                                          */
/*                                                                           */
/*****************************************************************************/
extern void __cdecl CreateTempFileName2( PSZ pName, PSZ pBase, PSZ pAltExt, 
                                         USHORT Folder ) 
{
   FILE   *fTemp ;
   char   szTemp[256] ;  
   char   *ptrChar, *ptrChar2 ;

   strcpy( pName, pBase ) ;
   if ( pBase ) {
      strcpy( szTemp, pBase ) ;
      ptrChar = strstr( szTemp, ".F" ) ; 
      if ( ptrChar ) {
         ptrChar += 5 ;
         ptrChar2 = strchr( ptrChar, '\\' ) ;
         if ( *ptrChar2 ) {
            *ptrChar = 0 ;
            strcpy( pName, szTemp ) ;
            if ( Folder == TEMPNAME_SOURCE ) 
               strcat( pName, "SOURCE" ) ;
            else
            if ( Folder == TEMPNAME_SSOURCE ) 
               strcat( pName, "SSOURCE" ) ;
            else
            if ( Folder == TEMPNAME_STARGET ) 
               strcat( pName, "STARGET" ) ;
            else
            if ( Folder == TEMPNAME_TARGET ) 
               strcat( pName, "TARGET" ) ;
            else
               strcat( pName, "SSOURCE" ) ;
            strcat( pName, ptrChar2 ) ;
         } 
      }
   }
   strcat( pName, pAltExt ) ;

   fTemp = fopen( pName, "w" ) ;               /* Avoid duplicate temp names */
   fprintf(fTemp, " " ) ;
   fclose( fTemp ) ;

   return ;
} 

/*****************************************************************************/
/*                                                                           */
/* ConvertImport                                                             */
/*                                                                           */
/* During file analysis, convert file from file's original code page to      */
/* TM's internal code page.                                                  */
/*                                                                           */
/* Parms:     pInput        - Name of input file.                            */
/*            pOutput       - Name of converted output file.                 */
/*            usConvertType - Type of EQF conversion to perform.             */
/*                                                                           */
/* Returns:   1  = File converted successfully.                              */
/*            0  = Convert failed.                                           */
/*                                                                           */
/*****************************************************************************/
extern int __cdecl ConvertImport( PSZ pInput, PSZ  pOutput, USHORT usConvertType  )
{
   USHORT  usRC ;
   int     iReturn = 0 ;                    /* Default for failed conversion */
   char    szTemp[256];
   char    szFromCP[10] ;  
   char    szToCP[10] ;  

   /**************************************************************************/
   /*  Convert using EQFFILECONVERSIONEX API                                 */
   /**************************************************************************/
   if ( ! *pOutput ) {
      strcpy( pOutput, pInput ) ;                                  /* 10-7-05 */
      strcat( pOutput, ".I$$" ) ;
   }

   usRC = EQFFILECONVERSIONEX( pInput, pOutput, szDocSourceLanguage, usConvertType ) ;
   if ( usRC == EQFRC_OK ) {
      iReturn = 1 ;
   } else {
      if ( ( usConvertType == EQF_ASCII2ANSI  ) ||
           ( usConvertType == EQF_ASCII2UTF8  ) ||
           ( usConvertType == EQF_ASCII2UTF16 ) ) 
         strcpy( szFromCP, "ASCII" ) ;
      else
         if ( ( usConvertType == EQF_ANSI2ASCII ) ||
              ( usConvertType == EQF_ANSI2UTF8  ) ||
              ( usConvertType == EQF_ANSI2UTF16 ) ) 
            strcpy( szFromCP, "ANSI" ) ;
         else
            if ( ( usConvertType == EQF_UTF82ASCII ) ||
                 ( usConvertType == EQF_UTF82ANSI  ) ||
                 ( usConvertType == EQF_UTF82UTF16 ) ) 
               strcpy( szFromCP, "UTF-8" ) ;
            else
               if ( ( usConvertType == EQF_UTF162ASCII ) ||
                    ( usConvertType == EQF_UTF162ANSI  ) ||
                    ( usConvertType == EQF_UTF162UTF8  ) ) 
                  strcpy( szFromCP, "UTF-16" ) ;
               else
                  strcpy( szFromCP, "ASCII" ) ;
      if ( ( usConvertType == EQF_ANSI2ASCII  ) ||
           ( usConvertType == EQF_UTF82ASCII  ) ||
           ( usConvertType == EQF_UTF162ASCII ) ) 
         strcpy( szToCP, "ASCII" ) ;
      else
         if ( ( usConvertType == EQF_ASCII2ANSI ) ||
              ( usConvertType == EQF_UTF82ANSI  ) ||
              ( usConvertType == EQF_UTF162ANSI ) ) 
            strcpy( szToCP, "ANSI" ) ;
         else
            if ( ( usConvertType == EQF_ASCII2UTF8 ) ||
                 ( usConvertType == EQF_ANSI2UTF8  ) ||
                 ( usConvertType == EQF_UTF162UTF8 ) ) 
               strcpy( szToCP, "UTF-8" ) ;
            else
               if ( ( usConvertType == EQF_ASCII2UTF16 ) ||
                    ( usConvertType == EQF_ANSI2UTF16  ) ||
                    ( usConvertType == EQF_UTF82UTF16  ) ) 
                  strcpy( szToCP, "UTF-16" ) ;
               else
                  strcpy( szToCP, "ASCII" ) ;
      sprintf(szTemp, "Code Page Conversion Error:  Could not convert from %s to %s",
              szFromCP, szToCP);
      MessageBoxA(HWND_DESKTOP,szTemp, "Conversion Error",MB_ICONHAND | MB_OK);
   }


   return ( iReturn ) ;
} 


/*****************************************************************************/
/*                                                                           */
/* ConvertExport                                                             */
/*                                                                           */
/* During file export, convert file from TM's internal code page to the      */
/* correct code page for the exported file.                                  */
/*                                                                           */
/* Parms:     pReplace      - Name of file to be converted in place.         */
/*            usConvertType - Type of EQF conversion to perform.             */
/*                                                                           */
/* Returns:   1  = File converted successfully.                              */
/*            0  = Convert failed.                                           */
/*                                                                           */
/*****************************************************************************/
extern int __cdecl ConvertExport( PSZ pReplace,  USHORT usConvertType  )
{

   char    szTempFile[256];
// char    szAltTempName[15] = "\\cvtexport.$$$";
   char    szTemp[256];
   char    szFromCP[10] ;  
   char    szToCP[10] ;  
   USHORT  usRC ;
   int     iReturn = 0 ;


   /**************************************************************************/
   /*  Convert using EQFFILECONVERSIONEX API.                                */
   /**************************************************************************/
// CreateTempFileName( szTempFile, szAltTempName ) ;
   strcpy( szTempFile, pReplace ) ;                               /* 10-7-05 */
   strcat( szTempFile, ".X$$" ) ;


   usRC = EQFFILECONVERSIONEX( pReplace, szTempFile, szDocTargetLanguage, usConvertType ) ;
   if ( usRC == EQFRC_OK ) {
      CopyFileA( szTempFile,pReplace, FALSE ) ;
      remove( szTempFile ) ; 
      iReturn = 1 ;
   } else {
      if ( ( usConvertType == EQF_ASCII2ANSI  ) ||
           ( usConvertType == EQF_ASCII2UTF8  ) ||
           ( usConvertType == EQF_ASCII2UTF16 ) ) 
         strcpy( szFromCP, "ASCII" ) ;
      else
         if ( ( usConvertType == EQF_ANSI2ASCII ) ||
              ( usConvertType == EQF_ANSI2UTF8  ) ||
              ( usConvertType == EQF_ANSI2UTF16 ) ) 
            strcpy( szFromCP, "ANSI" ) ;
         else
            if ( ( usConvertType == EQF_UTF82ASCII ) ||
                 ( usConvertType == EQF_UTF82ANSI  ) ||
                 ( usConvertType == EQF_UTF82UTF16 ) ) 
               strcpy( szFromCP, "UTF-8" ) ;
            else
               if ( ( usConvertType == EQF_UTF162ASCII ) ||
                    ( usConvertType == EQF_UTF162ANSI  ) ||
                    ( usConvertType == EQF_UTF162UTF8  ) ) 
                  strcpy( szFromCP, "UTF-16" ) ;
               else
                  strcpy( szFromCP, "ASCII" ) ;
      if ( ( usConvertType == EQF_ANSI2ASCII  ) ||
           ( usConvertType == EQF_UTF82ASCII  ) ||
           ( usConvertType == EQF_UTF162ASCII ) ) 
         strcpy( szToCP, "ASCII" ) ;
      else
         if ( ( usConvertType == EQF_ASCII2ANSI ) ||
              ( usConvertType == EQF_UTF82ANSI  ) ||
              ( usConvertType == EQF_UTF162ANSI ) ) 
            strcpy( szToCP, "ANSI" ) ;
         else
            if ( ( usConvertType == EQF_ASCII2UTF8 ) ||
                 ( usConvertType == EQF_ANSI2UTF8  ) ||
                 ( usConvertType == EQF_UTF162UTF8 ) ) 
               strcpy( szToCP, "UTF-8" ) ;
            else
               if ( ( usConvertType == EQF_ASCII2UTF16 ) ||
                    ( usConvertType == EQF_ANSI2UTF16  ) ||
                    ( usConvertType == EQF_UTF82UTF16  ) ) 
                  strcpy( szToCP, "UTF-16" ) ;
               else
                  strcpy( szToCP, "ASCII" ) ;
      sprintf(szTemp, "Code Page Conversion Error:  Could not convert from %s to %s",
              szFromCP, szToCP);
      MessageBoxA(HWND_DESKTOP,szTemp, "Conversion Error",MB_ICONHAND | MB_OK);
   }
   
   return ( iReturn ) ;
}

/*****************************************************************************/
/*                                                                           */
/* PrepDocLanguageInfo                                                       */
/*                                                                           */
/* Prepare the segmentation environment based on the source/target           */
/* language of this document.                                                */
/*   1.  Determine if TP 6.0 and later, or TP 5.5.2.x and before.            */
/*   2.  Get source and target languages of this document.                   */
/*   3.  Prepare the DBCS processing environment.                            */
/*                                                                           */
/* TP 6.0 & later:       DBCS lead bytes hardcoded based on language.        */
/* TP 5.5.2.x & before:  Use standard Windows IsDBCSLeadByte() routine.      */
/*                                                                           */
/* Inputs:   FilePath   - Full TM file name and path.                        */
/*                                                                           */
/* Returns:  None.                                                           */
/*                                                                           */
/*****************************************************************************/
extern void __cdecl PrepDocLanguageInfo( PSZ FilePath )
{

   char         szTempPath[512];
   char         *ptrChar, *ptrChar2, *ptrChar3 ;

   short        sTPVersionSave ;
   ULONG       rc;


   DBCSRanges[0] = 0 ;                      /* Assume not a DBCS language    */

   /**************************************************************************/
   /*  Determine if EQFFILECONVERSION API exists or not.                     */
   /*    If API exists, then this is TP 6.0 and later.                       */
   /*    If API does not exist, then this is TP 5.5.2.x and before.          */
   /**************************************************************************/
   if ( sTPVersion == TP_UNKNOWN ) {
      sTPVersion = TP_602 ; 
   }

   /**************************************************************************/
   /*  Get source and target languages from TM folder information.           */
   /**************************************************************************/
   if ( sTPVersion >= TP_60 ) {
      sTPVersionSave = sTPVersion ;
      sTPVersion = TP_602 ;                 /* Default if not language found 11-11-05 */
      szDocSourceLanguage[0] = 0 ;
      szDocTargetLanguage[0] = 0 ;
      if ( FilePath ) {
         strcpy( szTempPath, FilePath ) ;
         ptrChar = strstr( szTempPath, ".F00" ) ;
         if ( ptrChar ) {
            ptrChar2 = strchr( ptrChar+5, '\\' ) ;
            if (*ptrChar2 ) {
               *(ptrChar+4)= 0 ;
               ptrChar3 = strrchr( ptrChar2+1, '.' ) ;   /* If temp file named xxx.000B */
               if ( ( ptrChar3 ) &&                      /*   then remove "B".          */
                    ( strlen( ptrChar3+1 ) > 3 ) ) 
                  *(ptrChar3+4) = 0 ;
               szDocSourceLanguage[0] = 0 ;
               szDocTargetLanguage[0] = 0 ;
               rc = EQFGETSOURCELANG(szTempPath, ptrChar2+1, szDocSourceLanguage);
               rc = EQFGETTARGETLANG(szTempPath, ptrChar2+1, szDocTargetLanguage);
               sTPVersion = sTPVersionSave ;
            }
         }
      }
      if ( ( ! stricmp( szDocTargetLanguage, "Japanese" ) ) ||
           ( ! stricmp( szDocSourceLanguage, "Japanese" ) ) ) {
         DBCSRanges[0] = '\x81' ;
         DBCSRanges[1] = '\x9F' ;
         DBCSRanges[2] = '\xE0' ;
         DBCSRanges[3] = '\xFC' ;
         DBCSRanges[4] = 0    ;
         DBCSRanges[5] = 0    ;
      } else
      if ( ( ! stricmp( szDocTargetLanguage, "Korean" ) ) ||
           ( ! stricmp( szDocSourceLanguage, "Korean" ) ) ) {
         DBCSRanges[0] = '\x8F' ;
         DBCSRanges[1] = '\xFE' ;
         DBCSRanges[2] = 0    ;
         DBCSRanges[3] = 0    ;
      } else
      if ( ( ! stricmp( szDocTargetLanguage, "Chinese(simpl.)" ) ) ||
           ( ! stricmp( szDocSourceLanguage, "Chinese(simpl.)" ) ) ) {
         DBCSRanges[0] = '\x8C' ;
         DBCSRanges[1] = '\xFE' ;
         DBCSRanges[2] = 0    ;
         DBCSRanges[3] = 0    ;
      } else
      if ( ( ! stricmp( szDocTargetLanguage, "Chinese(trad.)" ) ) ||
           ( ! stricmp( szDocSourceLanguage, "Chinese(trad.)" ) ) ) {
         DBCSRanges[0] = '\x81' ;
         DBCSRanges[1] = '\xFE' ;
         DBCSRanges[2] = 0    ;
         DBCSRanges[3] = 0    ;
      } 
   }

}

/*****************************************************************************/
/*                                                                           */
/*   DESCRIPTIVE NAME   : Is DBCS or not                                     */
/*                                                                           */
/*   FUNCTION           : test for DBCS first byte                           */
/*                                                                           */
/*   This function tests the single-byte character c if it is in the         */
/*   code point range of the first byte of a DBCS double-byte character or   */
/*   not by checking the DBCS EV.  A non-zero value is returned when it is,  */
/*   otherwise a zero value is returned.                                     */
/*                                                                           */
/*   INPUT              :  unsigned char c                                   */
/*                                                                           */
/*   EXIT-NORMAL        :  1: is a 1st byte of DBCS                          */
/*                         0: is not a 1st byte of DBCS                      */
/*                                                                           */
/*****************************************************************************/

extern int __cdecl IsDBCS( UCHAR c )
{
  int i;
  BOOL bDBCSLeadByte = FALSE ;

  if ( sTPVersion == TP_55 ) {
     bDBCSLeadByte = IsDBCSLeadByte( c ) ; 
  } else {

     for( i=0 ; DBCSRanges[i] && !bDBCSLeadByte ; i+=2 ) {
       if ( ( c >= (UCHAR)DBCSRanges[i]   ) &&    /* If char falls within range,       */
            ( c <= (UCHAR)DBCSRanges[i+1] ) ) {   /*  then it is a DBCS lead byte.     */                          
          bDBCSLeadByte = TRUE ;                      
       }
     }
  }
  return( bDBCSLeadByte ) ; 
                                    
}


/*****************************************************************************/
/*                                                                           */
/* QueryExportFiles                                                          */
/*                                                                           */
/* For a given markup table, identify all of the files which are required    */
/* for a complete markup table.  If a markup table is not in this table,     */
/* then it is assumed that the xxx.TBL and xxx.DLL are the only files.       */
/*                                                                           */
/* This routine is recursively called.                                       */
/*                                                                           */
/* Inputs:   szMarkup     - Name of the markup table to get info for.        */
/*           usBufLen     - Size of szExportList variable.                   */
/* Outputs:  szExportList - List of related markup files.                    */
/*                                                                           */
/* Returns:  None.                                                           */
/*                                                                           */
/*****************************************************************************/

extern void __cdecl QueryExportFiles(char *szMarkup, char *szExportList, USHORT usBufLen, BOOL bRecursive ) {

//       Markup       Prereq markups           Unique Files (default \OTM\TABLE\)

    char MarkupList[][256] = {
        "OTMAJDK2    :OTMJDK21                :",                           
        "OTMHTM32    :                        :BIN\\OTMHTM32.DLL", 
        "OTMJDK21    :                        :BIN\\OTMJDK11.DLL",                    
        "OTMNJDK2    :OTMJDK21                :",                                     
        "OTMUHTM3    :OTMHTM32                :",
        "OTMUJDK2    :OTMJDK21                :",                                     
        "OTMXAHTM    :OTMXHTML                :OTMXAHTM.XML",                         
        "OTMXAXLF    :OTMXMXLF                :",                                     
        "OTMXAXML    :OTMXML                  :",                                     
        "OTMXHTML    :OTMXML                  :OTMXHTML.XML",
        "OTMXML      :                        :BIN\\OTMXML.DLL,OTMXML.CTL,OTMXML.LCL,",
        "OTMXMOCM    :OTMXML                  :OTMOCIM.XML",                          
        "OTMXMXLF    :OTMXML                  :OTMXLIFF.XML",                         
        "OTMXUHTM    :OTMXHTML                :OTMXUHTM.XML",                         
        "OTMXUXLF    :OTMXMXLF                :",                                     
        "OTMXUXML    :OTMXML                  :",                                     
        "NULL"
    };
    char delim[] = " ,";
    char *token, *ptr;
    char szTmp[512];
    char szBuffer[1024],  szMarkupName[80], szReqMarkup[1024];
    char szMarkupList[10][80];
    int count,idx, i;

    if ( !strcmp(szMarkup,"") ) {
        return;               
    }
    if ( ! bRecursive ) {
       szExportList[0] = 0 ;
    }

    idx = 0;
    strcpy(szTmp, szMarkup);
    token = strtok(szTmp, delim);
    while ( token != NULL ) {
        strcpy(szMarkupList[idx], token);
        idx++;
        token = strtok(NULL, delim);
    }
    count = 0;
    szReqMarkup[0] = 0 ;
    while ( strcmp(MarkupList[count], "NULL") ) {
        strcpy(szBuffer, MarkupList[count] );
        count++;
        if ( !strncmp(szBuffer, "////", 2) ) //comment line
            continue;

        //strcpy(szTemp, szBuffer);

        token = strtok(szBuffer, ":");
        if ( token != NULL ) {
            strcpy(szMarkupName, token);  //markup name
            ptr = strchr(szMarkupName, ' ');
            *ptr = 0;
            for ( i=0; i < idx; i++ ) {
                if ( !stricmp(szMarkupName, szMarkupList[i] ) ) {
                    if ( i == 0 )
                        szReqMarkup[0] = 0;
                    token = strtok(NULL, ":");
                    if ( token != NULL ) {
                        if ( strcmp(szReqMarkup, "") )
                            strcat(szReqMarkup, ",");
                        strcat(szReqMarkup, token);  //dependency part                        
                        ptr = strchr(szReqMarkup, ' ');
                        *ptr = 0;
                    }
                    token = strtok(NULL, "\n");

                    if (token != NULL ) {
                        if ( ! strstr( szExportList, token ) ) {
                           if ( strcmp(szExportList, "") )
                               strcat(szExportList, ",");
                           strcat(szExportList, token); // required file list
                        }
                    }
                }
            }
        }
    }

    QueryExportFiles(szReqMarkup, szExportList, usBufLen, TRUE);        
}


/*****************************************************************************/
/*                                                                           */
/* StripNewline                                                              */
/*                                                                           */
/* Inputs:   szText       - String to update.                                */
/* Outputs:  szNewLine    - Newline character(s).                            */
/*                                                                           */
/* Returns:  TRUE         - Newline removed.                                 */
/*           FALSE        - String not changed.                              */
/*                                                                           */
/*****************************************************************************/

extern int __cdecl StripNewline( char *szText, char *szNewLine ) {

   int i ;
   int iRC = FALSE ; 


   i = strlen( szText ) ;
   if ( i > 0 ) {
      if ( szText[i-1] == '\n' ) {
         if ( ( i > 1 ) &&
              ( szText[i-2] == '\r' ) ) {
            szText[i-2] = 0 ;
            if ( ! szNewLine[0] ) 
               strcpy( szNewLine, "\r\n" );
         } else {
            szText[i-1] = 0 ;
            if ( ! szNewLine[0] ) 
               strcpy( szNewLine, "\n" );
         }
         iRC = TRUE ;
      }
   }

   return( iRC ) ;
}


/*****************************************************************************/
/*                                                                           */
/* GetOTMTablePath                                                           */
/*                                                                           */
/* Inputs:   szBase       - Base directory.                                  */
/* Outputs:  szOTM        - OTM directory.                                   */
/*                                                                           */
/*****************************************************************************/

extern void __cdecl GetOTMTablePath( char * szBase, char * szOTM ) {

   char  *ptr ;

   strcpy( szOTM, szBase ) ;
   ptr = strchr( szOTM, '\\' ) ;              /* Find  c:\                   */ 
   if ( ptr ) {                               
      ptr = strchr( ptr+1, '\\' ) ;           /* Find  c:\eqf\               */
      if ( ptr ) {
         *(ptr+1) = 0 ;
         strcat( ptr, OTM_MARKUPPLUGIN_TABLE_DIR ) ;
         strcat( ptr, "\\" ) ;
      }
   }
   return ;
}

/*****************************************************************************/
/*                                                                           */
/* GetOTMDllPath                                                             */
/*                                                                           */
/* Inputs:   szBase       - Base directory.                                  */
/* Outputs:  szOTM        - OTM directory.                                   */
/*                                                                           */
/*****************************************************************************/

extern void __cdecl GetOTMDllPath( char * szBase, char * szOTM ) {

   char  *ptr ;

   strcpy( szOTM, szBase ) ;
   ptr = strchr( szOTM, '\\' ) ;              /* Find  c:\                   */ 
   if ( ptr ) {                               
      ptr = strchr( ptr+1, '\\' ) ;           /* Find  c:\otm\               */
      if ( ptr ) {
         *(ptr+1) = 0 ;
         strcat( ptr, OTM_MARKUPPLUGIN_DLL_DIR ) ;
         strcat( ptr, "\\" ) ;
      }
   }

   return ;
}
