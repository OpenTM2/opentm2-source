/*!
 UtlString.c - string functions
*/
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2014, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+

#include <time.h>
#include <tchar.h>

#include "eqf.h"                  // General Translation Manager include file
#include "eqfshape.h"
#include "eqfutchr.h"
#include "Utility.h"

//------------------------------------------------------------------------------
// Function Name:     UtlParseX15            Parse X15 delimited strings        
//------------------------------------------------------------------------------
// Description:       Parses a string for char \15 and returns a PSZ to the     
//                    n-th string separated by a \15.                           
//------------------------------------------------------------------------------
// Parameters:        1. PSZ - string to parse                                  
//------------------------------------------------------------------------------
// Returncode type:   PSZ                                                       
//------------------------------------------------------------------------------
// Note on usage:     This proc modifies the input string in such a way that    
//                    the trailing \15 of the requested string is replaced by   
//                    \0 to make it a C string.                                 
//                    \0 and \15 are skipped until n-th string is located       
PSZ UtlParseX15 (PSZ pszX15String, SHORT sStringId)
{
   PSZ   pszTemp = pszX15String;    // set temp. pointer to beginning of string

   while (sStringId--)
   {
      // search end of this part of the X15 string
      while ((*pszTemp != X15) && (*pszTemp != EOS))
      {
         pszTemp++;
      } /* endwhile */

      // position temp. pointer to beginning of next string part (now it points
      // to the delimiter character, i.e. either 0 or 0x15)
      pszTemp++;
   } /* endwhile */

   // remember the beginning of the result string
   pszX15String = pszTemp;

   // change the delimiter character to 0, so that this is a valid C string
   // search end of the result string
   while ((*pszTemp != X15) && (*pszTemp != EOS))
   {
      pszTemp++;
   } /* endwhile */
   // set end of string character
   *pszTemp = EOS;

   // return the pointer to the result string
   return (pszX15String);
} /* end of UtlParseX15 */

PSZ_W UtlParseX15W (PSZ_W pszX15String, SHORT sStringId)
{
   PSZ_W   pszTemp = pszX15String;    // set temp. pointer to beginning of string

   while (sStringId--)
   {
      // search end of this part of the X15 string
      while ((*pszTemp != X15) && (*pszTemp != EOS))
      {
         pszTemp++;
      } /* endwhile */

      // position temp. pointer to beginning of next string part (now it points
      // to the delimiter character, i.e. either 0 or 0x15)
      pszTemp++;
   } /* endwhile */

   // remember the beginning of the result string
   pszX15String = pszTemp;

   // change the delimiter character to 0, so that this is a valid C string
   // search end of the result string
   while ((*pszTemp != X15) && (*pszTemp != EOS))
   {
      pszTemp++;
   } /* endwhile */
   // set end of string character
   *pszTemp = EOS;

   // return the pointer to the result string
   return (pszX15String);
} /* end of UtlParseX15 */

// parse string using given character
PSZ_W UtlParseCharW (PSZ_W pszX15String, SHORT sStringId, CHAR_W chParse )
{
   PSZ_W   pszTemp = pszX15String;    // set temp. pointer to beginning of string

   while (sStringId--)
   {
      // search end of this part of the string
      while ((*pszTemp != chParse) && (*pszTemp != EOS))
      {
         pszTemp++;
      } /* endwhile */

      // position temp. pointer to beginning of next string part (now it points
      // to the delimiter character, i.e. either 0 or chParse)
      pszTemp++;
   } /* endwhile */

   // remember the beginning of the result string
   pszX15String = pszTemp;

   // change the delimiter character to 0, so that this is a valid C string
   // search end of the result string
   while ((*pszTemp != chParse) && (*pszTemp != EOS))
   {
      pszTemp++;
   } /* endwhile */
   // set end of string character
   *pszTemp = EOS;

   // return the pointer to the result string
   return (pszX15String);
} /* end of UtlParseCharW */

// extracts an specific column of an X15 seperated string
USHORT UtlExtractX15( PSZ pszTarget, PSZ pszX15String, SHORT sStringId )
{
  // position to start of requested string
  while ( (*pszX15String != EOS) && (sStringId != 0) )
  {
    if ( *pszX15String == X15 )
    {
      sStringId--;
    } /* endif */
    pszX15String++;
  } /* endwhile */

  // copy found string to target location
  while ( (*pszX15String != EOS) && (*pszX15String != X15) )
  {
    *pszTarget++ = *pszX15String++;
  } /* endwhile */
  *pszTarget = EOS;

  return( NO_ERROR );
} /* end of function UtlExtractX15 */

//------------------------------------------------------------------------------
// Function Name:     Utlstrccpy             strcpy up to a given character     
//------------------------------------------------------------------------------
// Description:       Copies a string from source to target like the C          
//                    function strcpy, but stops if a given character           
//                    is encountered.                                           
//------------------------------------------------------------------------------
// Function Call:     Utlstrccpy( pszTarget, pszSource, chChar )                
//------------------------------------------------------------------------------
// Parameters:        'pszTarget' points to the buffer for the target string    
//                    'pszSource' points to the source string                   
//                    'chChar' is the character which will stop the copy        
//------------------------------------------------------------------------------
// Returncode type:   PSZ                                                       
//------------------------------------------------------------------------------
// Returncodes:       ptr to target path                                        
PSZ Utlstrccpy( PSZ pszTarget, PSZ pszSource, CHAR chStop )
{
   PSZ pTemp;

   pTemp = pszTarget;
   while ( *pszSource && (*pszSource != chStop) )
   {
      *pszTarget++ = *pszSource++;
   } /* endwhile */
   *pszTarget = NULC;
   return( pTemp );
}

//------------------------------------------------------------------------------
// Function Name:     Utlstrnccpy            strcpy up to a char or a count     
//------------------------------------------------------------------------------
// Description:       Copies a string from source to target like the C          
//                    function strcpy, but stops if a given character           
//                    is encountered or the given character count is exceeded.  
//------------------------------------------------------------------------------
// Function Call:     Utlstrnccpy( pszTarget, pszSource, usCount, chChar )      
//------------------------------------------------------------------------------
// Parameters:        'pszTarget' points to the buffer for the target string    
//                    'pszSource' points to the source string                   
//                    'usCount' is the maximum of characters to be copied       
//                    'chChar' is the character which will stop the copy        
//------------------------------------------------------------------------------
// Returncode type:   PSZ                                                       
//------------------------------------------------------------------------------
// Returncodes:       ptr to target path                                        
PSZ Utlstrnccpy( PSZ pszTarget, PSZ pszSource, USHORT usCount, CHAR chStop)
{
   PSZ pTemp;

   pTemp = pszTarget;
   while ( *pszSource && (*pszSource != chStop) && usCount )
   {
      *pszTarget++ = *pszSource++;
      usCount--;
   } /* endwhile */
   *pszTarget = NULC;
   return( pTemp );
}


//------------------------------------------------------------------------------
// Function Name:     UtlLongToTimeString    Convert a long time to a string    
//------------------------------------------------------------------------------
// Description:       Converts a LONG time value to a time string. The format   
//                    of the time string is obtained from OS2.INI.              
//------------------------------------------------------------------------------
// Function Call:     UtlLongToTimeString( LONG lTime,                          
//                                         PSZ pszBuffer,                       
//                                         USHORT usBufferLength );             
//------------------------------------------------------------------------------
// Parameters:        'lTime' is the time in LONG format (e.g. obtained         
//                      using the C function time() )                           
//                    'pszBuffer' points to the buffer where the date           
//                      string will be stored.                                  
//                    'usBufferLength' is the size of the buffer                
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       length of time string                                     

ULONG UtlLongToTimeStringW
(
   LONG     lTime,                       // date in long format
   PSZ_W    pszBufferW,                   // ptr to buffer for date string
   ULONG    ulBufferLength               // size of buffer in char_w's
)
{

  CHAR   szTime[50];       // date converted to a character string
  ULONG  ulLen = 0;

   ulLen = UtlLongToTimeString( lTime, &(szTime[0]), sizeof(szTime) );

   ASCII2UnicodeBuf( szTime, pszBufferW, ulBufferLength, 0L);

   return( UTF16strlenCHAR( pszBufferW ) );
}


ULONG UtlLongToTimeString
(
   LONG   lTime,                       // time in long format
   PSZ    pszBuffer,                   // ptr to buffer for time string
   ULONG  ulBufferLength               // size of buffer in bytes
)
{
   struct tm   *pTimeDate;    // time/date structure
   CHAR        szTime[50];    // time converted to a character string

   *pszBuffer = NULC;                  // clear buffer

   if ( lTime != 0L ) lTime += 10800L; // correction: + 3 hours

   pTimeDate = localtime( (time_t*) &lTime );
   if ( (lTime != 0L) && pTimeDate )   // if localtime was successful ...
   {
      if ( UtiVar[UtlGetTask()].usTimeFormat == S_24_HOURS_TIME_FORMAT )
      {
         sprintf( szTime,
                  TIMEFORMATSTRING,
                  pTimeDate->tm_hour,
                  UtiVar[UtlGetTask()].chTimeSeperator,
                  pTimeDate->tm_min,
                  UtiVar[UtlGetTask()].chTimeSeperator,
                  pTimeDate->tm_sec );
      }
      else
      {
         sprintf( szTime,
                  TIMEFORMATSTRING,
                  pTimeDate->tm_hour % 12,
                  UtiVar[UtlGetTask()].chTimeSeperator,
                  pTimeDate->tm_min,
                  UtiVar[UtlGetTask()].chTimeSeperator,
                  pTimeDate->tm_sec );
         strcat( szTime, " " );
         if ( pTimeDate->tm_hour < 12 )
         {
            strcat( szTime, UtiVar[UtlGetTask()].szTime1159 );
         }
         else
         {
            strcat( szTime, UtiVar[UtlGetTask()].szTime2359 );
         } /* endif */
      } /* endif */
      strncpy( pszBuffer, szTime, ulBufferLength );
      *(pszBuffer + ulBufferLength - 1) = NULC;
   } /* endif */

   return( strlen( pszBuffer ) );
}

//------------------------------------------------------------------------------
// Function Name:     UtlLongToDateString    Convert a long date to a string    
//------------------------------------------------------------------------------
// Description:       Converts a LONG date value to a date string. The format   
//                    of the date string is obtained from OS2.INI.              
//------------------------------------------------------------------------------
// Function Call:     UtlLongToDateString( LONG lDate,                          
//                                         PSZ pszBuffer,                       
//                                         USHORT usBufferLength );             
//                    where 'lDate' is the date in LONG format (e.g. obtained   
//                      using the C function time() )                           
//                    where 'pszBuffer' points to the buffer where the date     
//                      string will be stored.                                  
//                    where 'usBufferLength' is the size of the buffer          
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       length of date string                                     
ULONG UtlLongToDateStringW
(
   LONG     lTime,                       // date in long format
   PSZ_W    pszBufferW,                   // ptr to buffer for date string
   ULONG    ulBufferLength               // size of buffer in char_w's
)
{

  CHAR   szDate[50];       // date converted to a character string
  ULONG  ulLen = 0;

   ulLen = UtlLongToDateString( lTime, &(szDate[0]), sizeof(szDate) );

   ASCII2UnicodeBuf( szDate, pszBufferW, ulBufferLength, 0L);

   return( UTF16strlenCHAR( pszBufferW ) );
}


ULONG UtlLongToDateString
(
   LONG   lTime,                       // date in long format
   PSZ    pszBuffer,                   // ptr to buffer for date string
   ULONG  ulBufferLength               // size of buffer in bytes
)
{
   struct tm   *pTimeDate;    // time/date structure
   CHAR        szDate[50];    // date converted to a character string
   USHORT      usId = UtlGetTask();

   *pszBuffer = NULC;                  // clear buffer

   if ( lTime != 0L ) lTime += 10800L; // correction: + 3 hours

   pTimeDate = localtime( (time_t*) &lTime );
   if ( (lTime != 0L) && pTimeDate )   // if localtime was successful ...
   {
      LONG lYear = pTimeDate->tm_year + 1900;

      // setup date string
      switch ( UtiVar[usId].usDateFormat )
      {
         case MDY_DATE_FORMAT:
            sprintf( szDate,
                     DATEFORMATSTRING_MDY,
                     pTimeDate->tm_mon + 1,
                     UtiVar[usId].chDateSeperator,
                     pTimeDate->tm_mday,
                     UtiVar[usId].chDateSeperator,
                     lYear );
            break;
         case DMY_DATE_FORMAT:
            sprintf( szDate,
                     DATEFORMATSTRING_DMY,
                     pTimeDate->tm_mday,
                     UtiVar[usId].chDateSeperator,
                     pTimeDate->tm_mon + 1,
                     UtiVar[usId].chDateSeperator,
                     lYear );
            break;
         case YMD_DATE_FORMAT:
            sprintf( szDate,
                     DATEFORMATSTRING_YMD,
                     lYear,
                     UtiVar[usId].chDateSeperator,
                     pTimeDate->tm_mon + 1,
                     UtiVar[usId].chDateSeperator,
                     pTimeDate->tm_mday );
            break;
      } /* endswitch */

      strncpy( pszBuffer, szDate, ulBufferLength );
      *(pszBuffer + ulBufferLength - 1) = NULC;
   } /* endif */

   return( strlen( pszBuffer ) );
}

//------------------------------------------------------------------------------
// Function Name:     UtlFTimeToTimeString   Convert a FTIME time to a string   
//------------------------------------------------------------------------------
// Description:       Converts a FTIME time value to a time string. The format  
//                    of the time string is obtained from OS2.INI.              
//------------------------------------------------------------------------------
// Function Call:     UtlFTimeToTimeString( FTIME *pFTime,                      
//                                          PSZ pszBuffer,                      
//                                          USHORT usBufferLength );            
//------------------------------------------------------------------------------
// Parameters:        'pFTime' points to FTIME structure containing the time    
//                      in FTIME format (e.g. from DosFindFirst ResultBuffer)   
//                    'pszBuffer' points to the buffer where the time           
//                      string will be stored.                                  
//                    'usBufferLength' is the size of the buffer                
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       length of time string                                     
ULONG UtlFTimeToTimeString
(
   FTIME  *pFTime,                     // time in FTIME format
   PSZ    pszBuffer,                   // ptr to buffer for time string
   ULONG  ulBufferLength               // size of buffer in bytes
)
{
   CHAR        szTime[50];    // time converted to a character string
   USHORT      usId = UtlGetTask();

   *pszBuffer = NULC;                  // clear buffer

   if ( UtiVar[usId].usTimeFormat == S_24_HOURS_TIME_FORMAT )
   {
      sprintf( szTime,
               TIMEFORMATSTRING,
               pFTime->hours,
               UtiVar[usId].chTimeSeperator,
               pFTime->minutes,
               UtiVar[usId].chTimeSeperator,
               pFTime->twosecs << 1 );
   }
   else
   {
      sprintf( szTime,
               TIMEFORMATSTRING,
               pFTime->hours % 12,
               UtiVar[usId].chTimeSeperator,
               pFTime->minutes,
               UtiVar[usId].chTimeSeperator,
               pFTime->twosecs << 1 );
      strcat( szTime, " " );
      if ( pFTime->hours < 12 )
      {
         strcat( szTime, UtiVar[usId].szTime1159 );
      }
      else
      {
         strcat( szTime, UtiVar[usId].szTime2359 );
      } /* endif */
   } /* endif */
   strncpy( pszBuffer, szTime, ulBufferLength );
   *(pszBuffer + ulBufferLength - 1) = NULC;

   return(strlen( pszBuffer ) );
}

//------------------------------------------------------------------------------
// Function Name:     UtlFDateToDateString    Convert a FDATE date to a string  
//------------------------------------------------------------------------------
// Description:       Converts a FDATE date value to a date string. The format  
//                    of the date string is obtained from OS2.INI.              
//------------------------------------------------------------------------------
// Function Call:     UtlFDateToDateString( FDATE *pFDATE,                      
//                                          PSZ pszBuffer,                      
//                                          USHORT ulBufferLength );            
//------------------------------------------------------------------------------
// Parameters:        'pFDate' points to FDATE structure containing the date    
//                      in FDATE format (e.g. from DosFindFirst ResultBuffer)   
//                    'pszBuffer' points to the buffer where the date           
//                      string will be stored.                                  
//                    'ulBufferLength' is the size of the buffer                
//------------------------------------------------------------------------------
// Returncode type:   USHORT                                                    
//------------------------------------------------------------------------------
// Returncodes:       length of date string                                     
ULONG UtlFDateToDateString
(
   FDATE  *pFDate,                     // date in FDATE format
   PSZ    pszBuffer,                   // ptr to buffer for date string
   ULONG  ulBufferLength               // size of buffer in bytes
)
{
   CHAR        szDate[50];    // date converted to a character string
   USHORT      usId = UtlGetTask();
   SHORT       sYear;

   /*******************************************************************/
   /* correct the year ( because it based on 01.01.1980 )             */
   /*******************************************************************/
   sYear = pFDate->year + 1980;

   switch ( UtiVar[usId].usDateFormat )
   {
      case MDY_DATE_FORMAT:
         sprintf( szDate,
                  DATEFORMATSTRING_MDY,
                  pFDate->month,
                  UtiVar[usId].chDateSeperator,
                  pFDate->day,
                  UtiVar[usId].chDateSeperator,
                  sYear );
         break;
      case DMY_DATE_FORMAT:
         sprintf( szDate,
                  DATEFORMATSTRING_DMY,
                  pFDate->day,
                  UtiVar[usId].chDateSeperator,
                  pFDate->month,
                  UtiVar[usId].chDateSeperator,
                  sYear );
         break;
      case YMD_DATE_FORMAT:
         sprintf( szDate,
                  DATEFORMATSTRING_YMD,
                  sYear,
                  UtiVar[usId].chDateSeperator,
                  pFDate->month,
                  UtiVar[usId].chDateSeperator,
                  pFDate->day );
         break;
   } /* endswitch */

   strncpy( pszBuffer, szDate, ulBufferLength );
   *(pszBuffer + ulBufferLength - 1) = NULC;

   return( strlen( pszBuffer ) );
}

// convert a string to a FDATE value
BOOL UtlDateStringToFDate
(
   PSZ    pszDate,                     // ptr to date string
   FDATE  *pFDate                      // buffer for date in FDATE format
)
{
  USHORT           usId = UtlGetTask();// current task (thread) ID
  USHORT           usDate[3];          // buffer for date values
  USHORT           usDigits;           // number of digits processed in date
  CHAR             chSeper;            // seperator character of date data
  USHORT           usGroup;            // current group in date processing
  PSZ              pszString;          // ptr for string processing
  BOOL             fOK = TRUE;         // data is O.K. flag


  memset( pFDate, 0, sizeof(FDATE) );  // clear caller's buffer

  /******************************************************************/
  /* check for general date format which is:                        */
  /* digit [digit] seperator digit [digit] seperator digit [digit]  */
  /******************************************************************/
  usGroup  = 0;
  usDate[0] = 0;
  usDate[1] = 0;
  usDate[2] = 0;
  usDigits = 0;
  chSeper  = EOS;
  pszString = pszDate;
  while ( *pszString && fOK )
  {
    if ( (*pszString >= '0') && (*pszString <= '9') )
    {
      if ( usDigits < 4 )
      {
        usDate[usGroup] = (usDate[usGroup] * 10) + (*pszString - '0');
        usDigits++;
      }
      else
      {
        fOK = FALSE;                // to much digits for date formats
      } /* endif */
    }
    else
    {
      if ( !usDigits || (usGroup > 1) )
      {
        fOK = FALSE;               // no digits or more than 2 seperators
      }
      else if ( chSeper && (chSeper != *pszString) )
      {
        fOK   = FALSE;               // different seperators used
      }
      else
      {
        chSeper = *pszString;        // remember seperator character
        usGroup++;                   // skip to next date group
        usDigits = 0;                // no digits for this group yet
      } /* endif */
    } /* endif */
    pszString++;
  } /* endwhile */

  /******************************************************************/
  /* Check if processed date follows the date format described in   */
  /* the OS/2 configuration file OS2.INI                            */
  /******************************************************************/
  if ( fOK )
  {
    if ( chSeper != UtiVar[usId].chDateSeperator )
    {
      fOK = FALSE;
    }
    else
    {
      switch ( UtiVar[usId].usDateFormat )
      {
        case YMD_DATE_FORMAT :
          if ( (usDate[1] == 0) || (usDate[1] > 12) ||
               (usDate[2] == 0) || (usDate[2] > 31) )
          {
            fOK = FALSE;
          }
          else
          {
            if ( usDate[0] < 90 )
            {
              usDate[0] += 2000;
            }
            else if ( usDate[0] < 100 )
            {
              usDate[0] += 1900;
            } /* endif */

            pFDate->day   = usDate[2];
            pFDate->month = usDate[1];
            pFDate->year  = usDate[0] - 1980;
          } /* endif */
          break;

        case MDY_DATE_FORMAT :
          if ( (usDate[0] == 0) || (usDate[0] > 12) ||
               (usDate[1] == 0) || (usDate[1] > 31) )
          {
            fOK = FALSE;
          }
          else
          {
            if ( usDate[2] < 90 )
            {
              usDate[2] += 2000;
            }
            else if ( usDate[2] < 100 )
            {
              usDate[2] += 1900;
            } /* endif */
            pFDate->day   = usDate[1];
            pFDate->month = usDate[0];
            pFDate->year  = usDate[2] - 1980;
          } /* endif */
          break;

        case DMY_DATE_FORMAT :
        default:
          if ( (usDate[1] == 0) || (usDate[1] > 12) ||
               (usDate[0] == 0) || (usDate[0] > 31) )
          {
            fOK = FALSE;
          }
          else
          {
            if ( usDate[2] < 90 )
            {
              usDate[2] += 2000;
            }
            else if ( usDate[2] < 100 )
            {
              usDate[2] += 1900;
            } /* endif */
            pFDate->day   = usDate[0];
            pFDate->month = usDate[1];
            pFDate->year  = usDate[2] - 1980;
          } /* endif */
          break;
      } /* endswitch */
    } /* endif */
  } /* endif */
  return( fOK );
} /* end of function UtlDateStringToFDate*/

// convert a string to a FTIME value
BOOL UtlTimeStringToFTime
(
   PSZ    pszTime,                     // ptr to date string
   FTIME  *pFTime                      // buffer for time in FTIME format
)
{
  USHORT           usId = UtlGetTask();// current task (thread) ID
  USHORT           usTime[3];          // buffer for date values
  USHORT           usDigits;           // number of digits processed in date
  CHAR             chSeper;            // seperator character of time data
  USHORT           usGroup;            // current group in date processing
  PSZ              pszString;          // ptr for string processing
  BOOL             fOK = TRUE;         // data is O.K. flag
  USHORT           usOffset = 0;       // hour offset

  memset( pFTime, 0, sizeof(FTIME) );  // clear caller's buffer

  /******************************************************************/
  /* check for general time format which is:                        */
  /* digit [digit] seperator digit [digit] seperator digit [digit] [am/pm] */
  /******************************************************************/
  usGroup  = 0;
  usTime[0] = 0;
  usTime[1] = 0;
  usTime[2] = 0;
  usDigits = 0;
  chSeper  = EOS;


  pszString = pszTime;
  while ( *pszString == ' ' ) pszString++; // skip leading blanks

  while ( *pszString && fOK )
  {
    if ( (*pszString >= '0') && (*pszString <= '9') )
    {
      if ( usDigits < 4 )
      {
        usTime[usGroup] = (usTime[usGroup] * 10) + (*pszString - '0');
        usDigits++;
      }
      else
      {
        fOK = FALSE;                // to much digits for time formats
      } /* endif */
    }
    else if (*pszString == ' ')
    {
      if ( usGroup < 2 )
      {
        // expected some more digits
        fOK = FALSE;
      }
      else
      {
        // check for am/pm string following
        if ( _stricmp( pszString+1, UtiVar[usId].szTime1159 ) == 0 )
        {
          usOffset = 0;
        }
        else if ( _stricmp( pszString+1, UtiVar[usId].szTime2359 ) == 0 )
        {
          usOffset = 12;
        }
        else
        {
          // unknown data following
          fOK = FALSE;
        }
      } /* endif */
    }
    else
    {
      if ( !usDigits || (usGroup > 1) )
      {
        fOK = FALSE;               // no digits or more than 2 seperators
      }
      else if ( chSeper && (chSeper != *pszString) )
      {
        fOK   = FALSE;               // different seperators used
      }
      else
      {
        chSeper = *pszString;        // remember seperator character
        usGroup++;                   // skip to next date group
        usDigits = 0;                // no digits for this group yet
      } /* endif */
    } /* endif */
    pszString++;
  } /* endwhile */

  if ( fOK )
  {
    if ( chSeper != UtiVar[usId].chTimeSeperator )
    {
      fOK = FALSE;
    }
    else
    {
      pFTime->hours   = usTime[0] + usOffset;
      pFTime->minutes = usTime[1];
      pFTime->twosecs = usTime[2] / 2;
    } /* endif */
  } /* endif */
  return( fOK );
} /* end of function UtlTimeStringToFDate*/


//------------------------------------------------------------------------------
// Function name:     UtlStripBlanks         Remove leading and trailing blanks 
//------------------------------------------------------------------------------
// Function call:     UtlStripBlanks( PSZ pszString );                          
//------------------------------------------------------------------------------
// Description:       Removes leading and trailing blanks from the supplied     
//                    string. The passed string is overwritten with the         
//                    stripped string.                                          
//------------------------------------------------------------------------------
// Input parameter:   PSZ      pszString      string to be procecessed          
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Side effects:      pszString is overwritten with the resulting string        
//------------------------------------------------------------------------------
VOID UtlStripBlanks
(
  PSZ    pszString                     // pointer to inpit/output string
)
{
  PSZ    pszSource;                    // source position pointer
  PSZ    pszTarget;                    // target position pointer
  PSZ    pszLastNonBlank;              // last non-blank position

  /********************************************************************/
  /* Skip leading blanks                                              */
  /********************************************************************/
  pszSource = pszTarget = pszString;
  pszLastNonBlank = pszString - 1;
  while ( *pszSource == BLANK )
  {
    pszSource++;
  } /* endwhile */

  /********************************************************************/
  /* Copy remaining characters                                        */
  /********************************************************************/
  while ( *pszSource )
  {
    if ( *pszSource != BLANK )
    {
      pszLastNonBlank = pszTarget;
    } /* endif */
    *pszTarget++ = *pszSource++;
  } /* endwhile */

  /********************************************************************/
  /* Truncate string at last non-blank position                       */
  /********************************************************************/
  pszLastNonBlank++;
  *pszLastNonBlank = EOS;

} /* end of function UtlStripBlanks */

VOID UtlStripBlanksW
(
  PSZ_W  pszString                     // pointer to inpit/output string
)
{
  PSZ_W  pszSource;                    // source position pointer
  PSZ_W  pszTarget;                    // target position pointer
  PSZ_W  pszLastNonBlank;              // last non-blank position

  /********************************************************************/
  /* Skip leading blanks                                              */
  /********************************************************************/
  pszSource = pszTarget = pszString;
  pszLastNonBlank = pszString - 1;
  while ( *pszSource == BLANK )
  {
    pszSource++;
  } /* endwhile */

  /********************************************************************/
  /* Copy remaining characters                                        */
  /********************************************************************/
  while ( *pszSource )
  {
    if ( *pszSource != BLANK )
    {
      pszLastNonBlank = pszTarget;
    } /* endif */
    *pszTarget++ = *pszSource++;
  } /* endwhile */

  /********************************************************************/
  /* Truncate string at last non-blank position                       */
  /********************************************************************/
  pszLastNonBlank++;
  *pszLastNonBlank = EOS;

} /* end of function UtlStripBlanks */


//------------------------------------------------------------------------------
// Function name:     UtlLowUpInit                                              
//------------------------------------------------------------------------------
// Function call:     UtlLowUpInit( usCountry, usCodePage );                    
//------------------------------------------------------------------------------
// Description:       This function will build up 2 256 character arrays        
//                    containing the upper and lower character tables           
//------------------------------------------------------------------------------
// Parameters:        USHORT usCountry          country code                    
//                    USHORT usCodePage         code page                       
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Side effects:      if EQFLowUpInit is called with one or both parameters     
//                    set to 0, the currently active country and code page is   
//                    taken.                                                    
//------------------------------------------------------------------------------
VOID
UtlLowUpInit()
{
  USHORT  i;                           // index

  /********************************************************************/
  /* init tables with default                                         */
  /********************************************************************/
  for ( i=0; i<256; i++ )
  {
    chEQFLower[ i ] = chEQFUpper[ i ] = (CHAR) i;
  } /* endfor */
  /********************************************************************/
  /* add a string end character -- just in case we have to use it     */
  /*   with string functions                                          */
  /********************************************************************/
  chEQFLower[ 256 ] = chEQFUpper[ 256 ] = EOS;

  // use the ANSILOW function
  EQFOemToAnsi( (PSZ)&(chEQFUpper[1]), (PSZ)&(chEQFUpper[1]) );
  AnsiUpper( (PSZ)&(chEQFUpper[1]) );
  EQFAnsiToOem( (PSZ)&(chEQFUpper[1]), (PSZ)&(chEQFUpper[1]) );

  EQFOemToAnsi( (PSZ)&(chEQFLower[1]), (PSZ)&(chEQFLower[1]) );
  AnsiLower( (PSZ)&(chEQFLower[1]) );
  EQFAnsiToOem( (PSZ)&(chEQFLower[1]), (PSZ)&(chEQFLower[1]) );

  return;
} /* end of function UtlLowUpInit */


//------------------------------------------------------------------------------
// Function name:     UtlLower                                                  
//------------------------------------------------------------------------------
// Function call:     UtlLower( pData );                                        
//------------------------------------------------------------------------------
// Description:       this function will lowercase the passed string            
//------------------------------------------------------------------------------
// Parameters:        PSZ  pData          pointer to data                       
//------------------------------------------------------------------------------
// Returncode type:   PSZ                                                       
//------------------------------------------------------------------------------
// Returncodes:       pointer to the converted string                           
//------------------------------------------------------------------------------
// Prerequesits:      UtlLowUpInit have to be run previously                    
//------------------------------------------------------------------------------
PSZ
UtlLower
(
  PSZ  pData                           // pointer to data
)
{
  PSZ  pTempData = pData;              // get pointer to data
  BYTE c;                              // active byte

  while ( (c = *pTempData) != NULC )
  {
    *pTempData++ = chEQFLower[ c ];
  } /* endwhile */

  return pData;
} /* end of function UtlLower */


PSZ_W
UtlLowerW
(
  PSZ_W  pData                           // pointer to data
)
{
    CharLowerW(pData);

  return pData;
} /* end of function UtlLower */

//------------------------------------------------------------------------------
// Function name:     UtlUpper                                                  
//------------------------------------------------------------------------------
// Function call:     UtlUpper( pData );                                        
//------------------------------------------------------------------------------
// Description:       this function will uppercase the passed string            
//------------------------------------------------------------------------------
// Parameters:        PSZ  pData          pointer to data                       
//------------------------------------------------------------------------------
// Returncode type:   PSZ                                                       
//------------------------------------------------------------------------------
// Returncodes:       pointer to the converted string                           
//------------------------------------------------------------------------------
// Prerequesits:      UtlLowUpInit have to be run previously                    
//------------------------------------------------------------------------------
PSZ
UtlUpper
(
  PSZ  pData                           // pointer to data
)
{
  PSZ  pTempData = pData;              // get pointer to data
  BYTE c;                              // active byte

  while ( (c = *pTempData) != NULC )
  {
    *pTempData++ = chEQFUpper[ c ];
  } /* endwhile */

  return pData;
} /* end of function UtlUpper */

PSZ_W
UtlUpperW
(
  PSZ_W  pData                           // pointer to data
)
{
  CharUpperW(pData);
  return pData;
} /* end of function UtlUpper */


BYTE
UtlToUpper
(
  BYTE d                               // input character
)
{
  return chEQFUpper[d];
} /* end of function UtlUpper */

CHAR_W
UtlToUpperW
(
  CHAR_W d                               // input character
)
{
   DWORD  dw = MAKELONG( d, 0 );
   return(LOWORD(CharUpperW( (PSZ_W)dw )));
 } /* end of function UtlUpper */


//------------------------------------------------------------------------------
//iCompare                                                                   
//   local function to compare characters, used by qsort                     
//   if element1 less    than element2 => return value less than 0           
//   if element1 equal   to   element2 => return value 0                     
//   if element1 greater than element2 => return value greater than 0        
//------------------------------------------------------------------------------
static int iCompare( const void *arg1, const void *arg2 )
{
   char *pchArg1 = (char *)arg1;
   char *pchArg2 = (char *)arg2;

   int intRc;

   if ( *pchArg1 < *pchArg2 )
   {
     intRc = -1;
   }
   else if ( *pchArg1 == *pchArg2 )
   {
     intRc = 0;
   }
   else
   {
     intRc = 1;
   }/*endif*/
   return( intRc );
}/* end iCompare */

//------------------------------------------------------------------------------
//SortString                                                                    
//   sorts the passed string                                                    
//------------------------------------------------------------------------------
VOID UtlSortString( PSZ pszString )
{
   qsort( (void *)pszString, (size_t)strlen( pszString ),
          (size_t)sizeof( CHAR ), iCompare );
}/* end UtlSortString*/


/**********************************************************************/
/* Do pattern matching for a string                                   */
/**********************************************************************/
BOOL UtlMatchStrings
(
  PSZ          pszString,              // string being compared
  PSZ          pPattern,               // pattern for pattern matching
  PBOOL        pfMatch                 // pointer to result of compare
)
{
  BOOL       fOK = TRUE;               // function return code
  PSZ        pszSource, pszTarget;     // ptr for pattern string pre-processing
  PSZ        pszMatch;                 // ptr to string which is tested
  PSZ        pszPattern;               // ptr to string being used as pattern
  PSZ        pszMatch2, pszMatch3;     // ptr for multiple substitution checks
  PSZ        pszOp1, pszOp2;           // ptr to copies of operand strings
  PSZ        pszBuffer = NULL;         // ptr to buffer for copies of operands
  BOOL       fFound;                   // string found flag

  /********************************************************************/
  /* Initialize compare result                                        */
  /********************************************************************/
  *pfMatch = TRUE;
  pszOp1 = NULL;
  pszOp2 = NULL;

  /********************************************************************/
  /* Get copy of strings and convert to uppercase                     */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &pszBuffer, 0L, (LONG)
                    max( MIN_ALLOC, (strlen(pszString) +
                                     strlen(pPattern) + 2 ) ), NOMSG );
    if ( fOK )
    {
      pszOp1 = pszBuffer;
      pszOp2 = pszBuffer + (strlen(pszString) + 1);
      strcpy( pszOp1, pszString );
      strcpy( pszOp2, pPattern );
      UtlUpper( pszOp1 );
      UtlUpper( pszOp2 );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* convert substrings like ??*?? to ????*                           */
  /* and compress multiple * to single *                              */
  /********************************************************************/
  if ( fOK )
  {
    pszSource = pszTarget = pszOp2;
    while ( *pszSource )
    {
      while ( *pszSource && (*pszSource != MULTIPLE_SUBSTITUTION) )
      {
        *pszTarget++ = *pszSource++;
      } /* endwhile */

      if ( *pszSource )
      {
        /****************************************************************/
        /* combine multiple substitution characters                     */
        /****************************************************************/
        while ( *(pszSource+1) == MULTIPLE_SUBSTITUTION )
        {
          pszSource++;
        } /* endif */

        /****************************************************************/
        /* Shift single substitution characters before multiple         */
        /* substitution characters                                      */
        /****************************************************************/
        if ( *(pszSource+1) == SINGLE_SUBSTITUTION )
        {
          /**************************************************************/
          /* replace * with ?                                           */
          /**************************************************************/
          pszSource++;
          *pszTarget++ = SINGLE_SUBSTITUTION;

          /**************************************************************/
          /* replace last ? with *                                      */
          /**************************************************************/
          while ( *(pszSource+1) == SINGLE_SUBSTITUTION )
          {
            *pszTarget++ = *pszSource++;
          } /* endwhile */
          pszSource++;
          *pszTarget++ = MULTIPLE_SUBSTITUTION;

          if ( *(pszSource+1) != MULTIPLE_SUBSTITUTION )
          {
            *pszTarget++ = *pszSource++;
          } /* endif */
        }
        else
        {
          *pszTarget++ = *pszSource++;
        } /* endif */
      } /* endif */
    } /* endwhile */
    *pszTarget = EOS;
  } /* endif */

  /********************************************************************/
  /* do actual pattern matching                                       */
  /********************************************************************/
  if ( fOK )
  {
    pszMatch   = pszOp1;       // address match string
    pszPattern = pszOp2;       // address pattern string

    if ( !*pszMatch && !*pszPattern )
    {
      /****************************************************************/
      /* Empty strings do always match ...                            */
      /****************************************************************/
    }
    else
    {
      while ( *pfMatch && *pszPattern )
      {
        switch( *pszPattern )
        {
          case SINGLE_SUBSTITUTION :
             if ( !*pszMatch )
             {
               *pfMatch = FALSE; // no match as match string ends
             }
             else
             {
               pszMatch++;             // test next character
               pszPattern++;
             } /* endif */
             break;

          case MULTIPLE_SUBSTITUTION :
             ++pszPattern;
             if ( *pszPattern )
             {
               /*******************************************************/
               /* compare strings until next * or                     */
               /* end of string found in second string                */
               /* fFound is true if match found                       */
               /*******************************************************/
               fFound = FALSE;
               while (!fFound && *pfMatch )
               {
                        /* find character in first string matching
                           first character after replacement symbol
                           in second symbol                            */
                 pszMatch = strchr(pszMatch, *pszPattern);
                 if (pszMatch == (PSZ)NULL)
                   *pfMatch = FALSE; // strings do not match
                 else
                 {
                   /* check for matching of characters until
                      next replacement character or end of
                      string found in second string             */
                   pszMatch3 = pszPattern;
                   pszMatch2 = pszMatch;
                   while (!fFound && *pfMatch )
                   {
                     ++pszMatch2;                   /* first characters */
                     ++pszMatch3;                   /* already checked  */
                     if (*pszMatch3 == EOS && *pszMatch2 == EOS ||
                                *pszMatch3 == MULTIPLE_SUBSTITUTION)
                     {
                                              /* substring has matched */
                       fFound = TRUE;
                       pszMatch = pszMatch2;        /* set pointers after */
                       pszPattern = pszMatch3;       /* last matching chars*/
                     }
                     else if (*pszMatch2 == EOS)  /* end of string      */
                       *pfMatch = FALSE; // strings do not match
                     else if (*pszMatch3 != SINGLE_SUBSTITUTION &&
                              *pszMatch2 != *pszMatch3)
                     {
                       ++pszMatch;     /*retry next char in first string*/
                       break;
                     }
                   } /* endwhile */
                 }
               } /* endwhile */
             } /* endif */
             break;

          default :
            if (*pszMatch != *pszPattern) /* characters not equal */
            {
              *pfMatch = FALSE; // strings do not match
            }
            else
            {
              pszMatch++;             // test next character
              pszPattern++;
            } /* endif */
        } /* end switch */
      } /* end while (!fError && !fcomplete) */

      if ( *pfMatch &&
           (*pszMatch != EOS ) &&
           (pszPattern[-1] != MULTIPLE_SUBSTITUTION) )
      {
        *pfMatch = FALSE; // no match as match string is not at end yet
      } /* endif */

    } /* endif */
  } /* endif */

  /********************************************************************/
  /* free allocated buffer                                            */
  /********************************************************************/
  if ( pszBuffer )
  {
    UtlAlloc( (PVOID *) &pszBuffer, 0L, 0L, NOMSG );
  } /* endif */

  return( fOK );

} /* end of function UtlMatchStrings */


/**********************************************************************/
/* Do pattern matching for a string                                   */
/**********************************************************************/
BOOL UtlMatchStringsW
(
  PSZ_W        pszString,              // string being compared
  PSZ_W        pPattern,               // pattern for pattern matching
  PBOOL        pfMatch                 // pointer to result of compare
)
{
  BOOL       fOK = TRUE;               // function return code
  PSZ_W      pszSource, pszTarget;     // ptr for pattern string pre-processing
  PSZ_W      pszMatch;                 // ptr to string which is tested
  PSZ_W      pszPattern;               // ptr to string being used as pattern
  PSZ_W      pszMatch2, pszMatch3;     // ptr for multiple substitution checks
  PSZ_W      pszOp1, pszOp2;           // ptr to copies of operand strings
  PSZ_W      pszBuffer = NULL;         // ptr to buffer for copies of operands
  BOOL       fFound;                   // string found flag

  /********************************************************************/
  /* Initialize compare result                                        */
  /********************************************************************/
  *pfMatch = TRUE;
  pszOp1 = NULL;
  pszOp2 = NULL;

  /********************************************************************/
  /* Get copy of strings and convert to uppercase                     */
  /********************************************************************/
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *) &pszBuffer, 0L, (LONG)
                    max( MIN_ALLOC, (UTF16strlenCHAR(pszString) +
                                     UTF16strlenCHAR(pPattern) + 2 )*sizeof(CHAR_W) ), NOMSG );
    if ( fOK )
    {
      pszOp1 = pszBuffer;
      pszOp2 = pszBuffer + (UTF16strlenCHAR(pszString) + 1);
      UTF16strcpy( pszOp1, pszString );
      UTF16strcpy( pszOp2, pPattern );
      UtlUpperW( pszOp1 );
      UtlUpperW( pszOp2 );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* convert substrings like ??*?? to ????*                           */
  /* and compress multiple * to single *                              */
  /********************************************************************/
  if ( fOK )
  {
    pszSource = pszTarget = pszOp2;
    while ( *pszSource )
    {
      while ( *pszSource && (*pszSource != MULTIPLE_SUBSTITUTION) )
      {
        *pszTarget++ = *pszSource++;
      } /* endwhile */

      if ( *pszSource )
      {
        /****************************************************************/
        /* combine multiple substitution characters                     */
        /****************************************************************/
        while ( *(pszSource+1) == MULTIPLE_SUBSTITUTION )
        {
          pszSource++;
        } /* endif */

        /****************************************************************/
        /* Shift single substitution characters before multiple         */
        /* substitution characters                                      */
        /****************************************************************/
        if ( *(pszSource+1) == SINGLE_SUBSTITUTION )
        {
          /**************************************************************/
          /* replace * with ?                                           */
          /**************************************************************/
          pszSource++;
          *pszTarget++ = SINGLE_SUBSTITUTION;

          /**************************************************************/
          /* replace last ? with *                                      */
          /**************************************************************/
          while ( *(pszSource+1) == SINGLE_SUBSTITUTION )
          {
            *pszTarget++ = *pszSource++;
          } /* endwhile */
          pszSource++;
          *pszTarget++ = MULTIPLE_SUBSTITUTION;

          if ( *(pszSource+1) != MULTIPLE_SUBSTITUTION )
          {
            *pszTarget++ = *pszSource++;
          } /* endif */
        }
        else
        {
          *pszTarget++ = *pszSource++;
        } /* endif */
      } /* endif */
    } /* endwhile */
    *pszTarget = EOS;
  } /* endif */

  /********************************************************************/
  /* do actual pattern matching                                       */
  /********************************************************************/
  if ( fOK )
  {
    pszMatch   = pszOp1;       // address match string
    pszPattern = pszOp2;       // address pattern string

    if ( !*pszMatch && !*pszPattern )
    {
      /****************************************************************/
      /* Empty strings do always match ...                            */
      /****************************************************************/
    }
    else
    {
      while ( *pfMatch && *pszPattern )
      {
        switch( *pszPattern )
        {
          case SINGLE_SUBSTITUTION :
             if ( !*pszMatch )
             {
               *pfMatch = FALSE; // no match as match string ends
             }
             else
             {
               pszMatch++;             // test next character
               pszPattern++;
             } /* endif */
             break;

          case MULTIPLE_SUBSTITUTION :
             ++pszPattern;
             if ( *pszPattern )
             {
               /*******************************************************/
               /* compare strings until next * or                     */
               /* end of string found in second string                */
               /* fFound is true if match found                       */
               /*******************************************************/
               fFound = FALSE;
               while (!fFound && *pfMatch )
               {
                        /* find character in first string matching
                           first character after replacement symbol
                           in second symbol                            */
                 pszMatch = UTF16strchr(pszMatch, *pszPattern);
                 if (pszMatch == (PSZ_W)NULL)
                   *pfMatch = FALSE; // strings do not match
                 else
                 {
                   /* check for matching of characters until
                      next replacement character or end of
                      string found in second string             */
                   pszMatch3 = pszPattern;
                   pszMatch2 = pszMatch;
                   while (!fFound && *pfMatch )
                   {
                     ++pszMatch2;                   /* first characters */
                     ++pszMatch3;                   /* already checked  */
                     if (*pszMatch3 == EOS && *pszMatch2 == EOS ||
                                *pszMatch3 == MULTIPLE_SUBSTITUTION)
                     {
                                              /* substring has matched */
                       fFound = TRUE;
                       pszMatch = pszMatch2;        /* set pointers after */
                       pszPattern = pszMatch3;       /* last matching chars*/
                     }
                     else if (*pszMatch2 == EOS)  /* end of string      */
                       *pfMatch = FALSE; // strings do not match
                     else if (*pszMatch3 != SINGLE_SUBSTITUTION &&
                              *pszMatch2 != *pszMatch3)
                     {
                       ++pszMatch;     /*retry next char in first string*/
                       break;
                     }
                   } /* endwhile */
                 }
               } /* endwhile */
             } /* endif */
             break;

          default :
            if (*pszMatch != *pszPattern) /* characters not equal */
            {
              *pfMatch = FALSE; // strings do not match
            }
            else
            {
              pszMatch++;             // test next character
              pszPattern++;
            } /* endif */
        } /* end switch */
      } /* end while (!fError && !fcomplete) */

      if ( *pfMatch &&
           (*pszMatch != EOS ) &&
           (pszPattern[-1] != MULTIPLE_SUBSTITUTION) )
      {
        *pfMatch = FALSE; // no match as match string is not at end yet
      } /* endif */

    } /* endif */
  } /* endif */

  /********************************************************************/
  /* free allocated buffer                                            */
  /********************************************************************/
  if ( pszBuffer )
  {
    UtlAlloc( (PVOID *) &pszBuffer, 0L, 0L, NOMSG );
  } /* endif */

  return( fOK );

} /* end of function UtlMatchStrings */



///////////////////////////////////////////////////////////////////////////////
///                                                                         ///
///  UNICODE enabled string functions (UTF16 only)                          ///
///                                                                         ///
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///  UTF16strcpy   copy an Unicode UTF16 string to another location         ///
///////////////////////////////////////////////////////////////////////////////

PSZ_W UTF16strcpy( PSZ_W pszTarget, PSZ_W pszSource )
{
  PUSHORT pusTarget = (PUSHORT)pszTarget;
  PUSHORT pusSource = (PUSHORT)pszSource;
  while ( *pusSource != 0 )
  {
    *pusTarget++ = *pusSource++;
  } /* endwhile */
  *pusTarget = 0;
  return( pszTarget );
}



PSZ_W UTF16strccpy( PSZ_W pszTarget, PSZ_W pszSource, CHAR_W chStop )
{
  PSZ_W pusTarget = pszTarget;
  PSZ_W pusSource = pszSource;

  while ( *pusSource  && (*pusSource != chStop) )
  {
    *pusTarget++ = *pusSource++;
  } /* endwhile */
  *pusTarget = 0;
  return( pszTarget );
}


///////////////////////////////////////////////////////////////////////////////
///  UTF16strcat   concatenate an Unicode UTF16 string to then end of string///
///////////////////////////////////////////////////////////////////////////////

PSZ_W UTF16strcat( PSZ_W pszTarget, PSZ_W pszSource )
{
  int iTargetLen = UTF16strlenCHAR( pszTarget );
  UTF16strcpy( pszTarget + iTargetLen, pszSource );
  return( pszTarget );
}


///////////////////////////////////////////////////////////////////////////////
///  UTF16strcmp   compare two Unicode UTF16 strings                        ///
///////////////////////////////////////////////////////////////////////////////

int UTF16strcmp( PSZ_W pszString1, PSZ_W pszString2 )
{
  int iResult = 0;
  PUSHORT pusString1 = (PUSHORT)pszString1;
  PUSHORT pusString2 = (PUSHORT)pszString2;

  while ( iResult == 0 )
  {
    if ( *pusString1 == 0)
    {
      if ( *pusString2 == 0 )
      {
        break;                         // strings are identical
      }
      else
      {
        iResult = -1;                  // string1 ends while string2 continues
      } /* endif */
    }
    else if ( *pusString2 == 0 )
    {
      iResult = 1;                     // string2 ends while string1 continues
    }
    else
    {
      iResult =  *pusString1++ - *pusString2++;
    } /* endif */
  } /* endwhile */
  return( iResult);
}


///////////////////////////////////////////////////////////////////////////////
///  UTF16strncmp   compare two Unicode UTF16 strings up to usLen chars     ///
///////////////////////////////////////////////////////////////////////////////
int UTF16strncmp( PSZ_W pszString1, PSZ_W pszString2, USHORT usLen )
{
	LONG lLen = usLen;

	return (UTF16strncmpL(pszString1, pszString2, lLen));
}

int UTF16strncmpL( PSZ_W pszString1, PSZ_W pszString2, LONG lLen )
{
  int iResult = 0;
  PUSHORT pusString1 = (PUSHORT)pszString1;
  PUSHORT pusString2 = (PUSHORT)pszString2;

  while ( (iResult == 0) && (lLen > 0))
  {
  lLen--;

    if ( *pusString1 == 0)
    {
      if ( *pusString2 == 0 )
      {
        break;                         // strings are identical
      }
      else
      {
        iResult = -1;                  // string1 ends while string2 continues
      } /* endif */
    }
    else if ( *pusString2 == 0 )
    {
      iResult = 1;                     // string2 ends while string1 continues
    }
    else
    {
      iResult =  *pusString1++ - *pusString2++;
    } /* endif */
  } /* endwhile */
  return( iResult);
}

///////////////////////////////////////////////////////////////////////////////
///  UTF16strncmp   compare two Unicode UTF16 strings up to usLen chars     ///
///   without respect of upper/lowercase                                    ///
///////////////////////////////////////////////////////////////////////////////

int UTF16strnicmp( PSZ_W pszString1, PSZ_W pszString2, USHORT usLen )
{
  return( _wcsnicmp( pszString1, pszString2, usLen));
}

int UTF16strnicmpL( PSZ_W pszString1, PSZ_W pszString2, LONG lLen )
{
  return( _wcsnicmp( pszString1, pszString2, lLen));
}

///////////////////////////////////////////////////////////////////////////////
///  UTF16strlen   get the length in bytes of an Unicode UTF16 string       ///
///////////////////////////////////////////////////////////////////////////////

int UTF16strlenCHAR( PSZ_W pszString )
{
  int iLen = 0;

  while ( pszString && *pszString != 0 )
  {
    iLen ++;
    pszString++;
  } /* endwhile */
  return( iLen );
}


int UTF16strlenBYTE( PSZ_W pszString )
{
  int iLen = 0;

  while ( pszString && *pszString != 0 )
  {
    iLen += 2;
    pszString++;
  } /* endwhile */
  return( iLen );
}



///////////////////////////////////////////////////////////////////////////////
///  UTF16strchr   find character in an Unicode UTF16 string                ///
///////////////////////////////////////////////////////////////////////////////

PSZ_W UTF16strchr( PSZ_W pszString, CHAR_W ch  )
{
  CHAR_W c;
  while ( ((c = *pszString) != 0) && (c != ch) )
  {
    pszString++;
  } /* endwhile */
  return( (c == ch) ? pszString : NULL );
}


///////////////////////////////////////////////////////////////////////////////
///  UTF16strrev    reverse a Unicode string                                ///
///////////////////////////////////////////////////////////////////////////////

PSZ_W UTF16strrev( PSZ_W pszString )
{
  ULONG ulLen = UTF16strlenCHAR( pszString );
  USHORT usPos = 0;
  ULONG  ulMid = ulLen/2;
  CHAR_W ch;

  for ( usPos=0; usPos < ulMid; usPos++ )
  {
  ch = pszString[usPos];
  pszString[usPos] = pszString[ulLen-usPos-1];
  pszString[ulLen-usPos-1] = ch;
  }

  return( pszString);
}


///////////////////////////////////////////////////////////////////////////////
///  UTF16strstr   find the specified string in the source string           ///
///////////////////////////////////////////////////////////////////////////////

PSZ_W UTF16strstr( PSZ_W pszSource, PSZ_W pszFindString )
{
  int iPos = 0;
  int iTargetLen = UTF16strlenCHAR( pszSource );
  ULONG ulFindLen   = UTF16strlenCHAR( pszFindString );
  CHAR_W cFindStart = pszFindString[0];
  PSZ_W pReturn = NULL;

  for ( iPos = 0; iPos<iTargetLen; iPos++ )
  {
    if ( pszSource[iPos] == cFindStart )
    {
      if (UTF16strncmp( &pszSource[iPos], pszFindString, (USHORT)ulFindLen ) == 0)
      {
        pReturn = &pszSource[iPos];
        break;
      }
    }
  }

  return( pReturn );
}



///////////////////////////////////////////////////////////////////////////////
///  UTF16strcspn   find the first matching character of String in Source   ///
///////////////////////////////////////////////////////////////////////////////

int UTF16strcspn( PSZ_W pszSource, PSZ_W pszString )
{
  CHAR_W c;
  int iPos = 0;
  int iTargetLen = UTF16strlenCHAR( pszSource );

  for ( iPos = 0; iPos<iTargetLen; iPos++ )
  {
  c = pszSource[iPos];
  if ( UTF16strchr(  pszString, c ) )
  {
    break;
  }
  }

  return( iPos );
}





///////////////////////////////////////////////////////////////////////////////
///  Unicode2ASCII  convert unicode string to Ansi                          ///
///////////////////////////////////////////////////////////////////////////////

PSZ Unicode2ASCII( PSZ_W pszUni, PSZ pszASCII, ULONG ulCP )
{
  // it is assumed, that pszASCII is large enough to hold the converted string
  PSZ_W    pTemp = NULL;
  USHORT   usCP = (USHORT) ulCP;
  if ( pszUni && pszASCII )
  {
	if (*pszUni == EOS)
	{
		*pszASCII = EOS;
    }
    else
    {
		if (!usCP)
		{
		  usCP = (USHORT)GetLangOEMCP(NULL);
		}

    // always use 932 when 943 is specified
    if ( usCP == 943 ) usCP = 932;

		// do special handling for Arabic to get rid of shaping to allow for conversion
		// back to 864
		switch ( usCP )
		{
		  case 864:
			UtlAlloc( (PVOID *) &pTemp, 0L, MAX_SEGMENT_SIZE * sizeof(CHAR_W), ERROR_STORAGE );

			if (pTemp)
			{
			  CHAR_W c;
			  PSZ_W  p = pTemp;

			  UTF16strcpy( pTemp, pszUni );
			  BidiConvert06ToFE(pTemp, UTF16strlenCHAR(pTemp)+1);
			  //shapeUnicode(pTemp,UTF16strlenCHAR(pTemp)+1);
			  // convert the LTR&RTL markers to codepoints in the standard 864 page,
			  // so that our WideCharToMultiByte can handle it...

			  while ((c = *p++) != NULC)
			  {
				switch (c)
				{
				  case 0x200E:      // Left-to-Right marker
					*(p-1) = 0x009B;// Unicode character converted from CP864 (0x9B)
					break;
				  case 0x200F:      // Right-to-Left marker
					*(p-1) = 0x009C;// Unicode character converted from CP864 (0x9C)
					break;
				}
			  }
			  WideCharToMultiByte( usCP, 0, (LPCWSTR)pTemp, -1,
								 pszASCII, MAX_SEGMENT_SIZE, NULL, NULL );
			  UtlAlloc( (PVOID *) &pTemp, 0L, 0L, NOMSG );
			}
			break;
		  case 867:
		  case 862:
			// convert the LRM&RLM markers to codepoints in the standard 862 page,
			// so that our WideCharToMultiByte can handle it...
			UtlAlloc( (PVOID *) &pTemp, 0L, MAX_SEGMENT_SIZE * sizeof(CHAR_W), ERROR_STORAGE );

			if (pTemp)
			{
			  CHAR_W c;
			  PSZ_W  p = pTemp;
			  UTF16strcpy(p, pszUni);
			  while ((c = *p++) != NULC)
			  {
				switch (c)
				{
				  case 0x200E:      // Left-to-Right marker
					*(p-1) = 0x00E1;// Unicode character converted from CP862 (0xA0)
					break;
				  case 0x200F:      // Right-to-Left marker
					*(p-1) = 0x00ED;// Unicode character converted from CP862 (0xA1)
					break;
				}
			  }

			  usCP = 862;     // we have to use CP862 since Windows only supports this
			  WideCharToMultiByte( usCP, 0, (LPCWSTR)pTemp, -1,
								   pszASCII, MAX_SEGMENT_SIZE, NULL, NULL );
			  UtlAlloc( (PVOID *) &pTemp, 0L, 0L, NOMSG );
			}
			break;
          case 850:
            // Change the replacement character from '?' to '%'
            // '?' causes false breaks.
            WideCharToMultiByte( usCP, 0, (LPCWSTR)pszUni, -1,
                                 pszASCII, MAX_SEGMENT_SIZE, "%", NULL );
            break;
		  default:
			WideCharToMultiByte( usCP, 0, (LPCWSTR)pszUni, -1,
								 pszASCII, MAX_SEGMENT_SIZE, NULL, NULL );
			break;
		}
    } /* endif */
  }
  else if (pszASCII)
  {
  *pszASCII = EOS;
  }
  return( pszASCII );
}

ULONG Unicode2ASCIIBuf( PSZ_W pszUni, PSZ pszASCII, ULONG ulLen, LONG lBufLen, ULONG ulCP)
{
	LONG lRc = 0;
	ULONG ulOutPut = 0;

	ulOutPut = Unicode2ASCIIBufEx(pszUni, pszASCII, ulLen, lBufLen, ulCP, &lRc);
	return(ulOutPut);
}

// ulLen = # of char-W's in pszUni!
// internal function, used only in this source-file
ULONG Unicode2ASCIIBufEx( PSZ_W pszUni, PSZ pszASCII, ULONG ulLen, LONG lBufLen,
                          ULONG ulCP, PLONG plRc)
{
	static CHAR_W szUniTemp[ MAX_SEGMENT_SIZE ];
	ULONG ulOutPut = 0;
	USHORT usCP = (USHORT) ulCP;
	LONG lRc = 0;

	if (!usCP)
	{
		ulCP = (USHORT)GetLangOEMCP(NULL);
		usCP = (USHORT)ulCP;
	}

	if ( pszUni && pszASCII )
	{
		PSZ_W pTemp = NULL;
		USHORT usCP = (USHORT) ulCP;

		// always use 932 when 943 is specified
		if ( usCP == 943 ) usCP = 932;

		// do special handling for Arabic to get rid of shaping to allow for conversion
		// back to 864
		*pszASCII = 0;
		if (ulLen)
		{
			switch ( usCP )
			{
				case 864:
					UtlAlloc( (PVOID *) &pTemp, 0L, (ulLen+1) * sizeof(CHAR_W), ERROR_STORAGE );
					if (pTemp)
					{
						CHAR_W c;
						PSZ_W  p = pTemp;
						UTF16strncpy( pTemp, pszUni, ulLen );
						BidiConvert06ToFE(pTemp, ulLen);
						//shapeUnicode( pTemp,ulLen);
						// convert the LTR&RTL markers to codepoints in the standard 864 page,
						// so that our WideCharToMultiByte can handle it...

						while ((c = *p++) != NULC)
						{
							switch (c)
							{
								case 0x200E:      // Left-to-Right marker
									*(p-1) = 0x009B;// Unicode character converted from CP864 (0x9B)
									break;
								case 0x200F:      // Right-to-Left marker
									*(p-1) = 0x009C;// Unicode character converted from CP864 (0x9C)
									break;
							}
						}

						ulOutPut = WideCharToMultiByte( usCP, 0, (LPCWSTR)pTemp, ulLen,
														pszASCII, lBufLen, NULL, NULL );
						if (!ulOutPut )
						{
							lRc = GetLastError();
						}

						UtlAlloc( (PVOID *) &pTemp, 0L, 0L, NOMSG );
					}
					break;
				case 867:
				case 862:
					// convert the LRM&RLM markers to codepoints in the standard 862 page,
					// so that our WideCharToMultiByte can handle it...
					UtlAlloc( (PVOID *) &pTemp, 0L, (ulLen+1) * sizeof(CHAR_W), ERROR_STORAGE );

					if (pTemp)
					{
						CHAR_W c;
						PSZ_W  p = pTemp;
						UTF16strncpy(p, pszUni, ulLen);

						while ((c = *p++)!= NULC)
						{
							switch (c)
							{
								case 0x200E:      // Left-to-Right marker
									*(p-1) = 0x00E1;// Unicode character converted from CP862 (0xA0)
									break;
								case 0x200F:      // Right-to-Left marker
									*(p-1) = 0x00ED;// Unicode character converted from CP862 (0xA1)
									break;
							}
						}
						usCP = 862;     // we have to use CP862 since Windows only supports this

						ulOutPut = WideCharToMultiByte( usCP, 0, (LPCWSTR)pTemp, ulLen,
														pszASCII, lBufLen, NULL, NULL );
						if (!ulOutPut )
						{
							lRc = GetLastError();
						}

						UtlAlloc( (PVOID *) &pTemp, 0L, 0L, NOMSG );
					}
					break;
				case 850:
		            // Change the replacement character from '?' to '%'
		            // '?' causes false breaks.
				    ulOutPut = WideCharToMultiByte( usCP, 0, (LPCWSTR)pszUni, ulLen,
		                                            pszASCII, lBufLen, "%", NULL );
		            break;
				default:
					ulOutPut = WideCharToMultiByte( usCP, 0, (LPCWSTR)pszUni, ulLen,
													pszASCII, lBufLen, NULL, NULL );

					if (!ulOutPut )
					{
						lRc = GetLastError();
					}
					break;
			} /* endswitch */
		} /* endif ulLen */
	}
	else if (pszASCII)
	{
		*pszASCII = EOS;
	}

	*plRc = lRc;

  return( ulOutPut );
}

PSZ Unicode2Ansi( PSZ_W pszUni, PSZ pszAnsi, ULONG ulOemCP )
{
  PUCHAR  pConvTable = NULL;
  ULONG   ulLen = 0;
  // it is assumed that pszANsi is large enough to hold the converted string

  if (!ulOemCP)
  {
      ulOemCP =  GetLangOEMCP(NULL);
  }
  Unicode2ASCII( (PSZ_W)pszUni, pszAnsi, ulOemCP );
  UtlQueryCharTableEx( ASCII_TO_ANSI_TABLE, &pConvTable, (USHORT)ulOemCP);
  ulLen = strlen(pszAnsi);
  if ( pConvTable && pszAnsi )
  {
    ULONG i;
    unsigned char *pData = (PUCHAR)pszAnsi;
    for ( i = 0; i < ulLen; i++ )
    {
      *pData = pConvTable[*pData];
      pData++;
    } /* endfor */
  } /* endif */

  return( pszAnsi );
}

ULONG Unicode2AnsiBuf( PSZ_W pszUni, PSZ pszAnsi, ULONG ulLen, LONG lBufLen,
                         ULONG ulOemCP)
{
  PUCHAR  pConvTable = NULL;
  ULONG  ulOutPut = 0;

  if (!ulOemCP)
  {
      ulOemCP =  GetLangOEMCP(NULL);
  }
  ulOutPut = Unicode2ASCIIBuf( (PSZ_W)pszUni, pszAnsi, ulLen, lBufLen, ulOemCP );
  UtlQueryCharTableEx( ASCII_TO_ANSI_TABLE, &pConvTable, (USHORT)ulOemCP);
  if ( pConvTable && pszAnsi )
  {
    ULONG i;
    unsigned char *pData = (PUCHAR)pszAnsi;
    for ( i = 0; i < ulLen; i++ )
    {
      *pData = pConvTable[*pData];
      pData++;
    } /* endfor */
  } /* endif */

  return( ulOutPut );
}


///////////////////////////////////////////////////////////////////////////////
///  ASCII2Unicode convert ASCII string to Unicode                          ///
///////////////////////////////////////////////////////////////////////////////

PSZ_W ASCII2Unicode( PSZ pszASCII, PSZ_W pszUni, ULONG  ulCP )
{
  int iRC = 0;
  USHORT  usCP = (USHORT)ulCP;

  if ( pszASCII && pszUni )
  {
	  if (*pszASCII == EOS )
	  {
		  *pszUni = EOS;
      }
      else
      {
		if (!usCP)
		{
		  usCP = (USHORT)GetLangOEMCP(NULL);
		}

    // always use 932 when 943 is specified
    if ( usCP == 943 ) usCP = 932;

		iRC = MultiByteToWideChar( usCP, 0, pszASCII, -1, pszUni, MAX_SEGMENT_SIZE );
    if ( iRC == 0 ) iRC = GetLastError();
    if ( iRC == ERROR_INVALID_PARAMETER )
    {
      // codepage not supported by OS, use default code page for conversion
		  MultiByteToWideChar( CP_OEMCP, 0, pszASCII, -1, pszUni, MAX_SEGMENT_SIZE );
    } /* endif */

		switch (usCP)
		{
		  case 864:
		  {
			  PSZ_W p = pszUni;
			  CHAR_W c;

			  while ((c = *p++) != NULC)
			  {
				switch (c)
				{
				  case 0x009B:      // Left-to-Right marker
					*(p-1) = 0x200E;// Unicode character converted from CP864 (0x9B)
					break;
				  case 0x009C:      // Right-to-Left marker
					*(p-1) = 0x200F;// Unicode character converted from CP864 (0x9C)
					break;
				}
			  }
		      // do special handling for Arabic to allow shaping in even for 864 stuff
		      BidiConvertFETo06(pszUni, UTF16strlenCHAR(pszUni));
		    }
			break;
		  case 862:
		  case 867:
			{
			  // Convert RLM&LRM characters correctly to UNICODE
			  PSZ_W p = pszUni;
			  CHAR_W c;
			  while ((c = *p++)!= NULC)
			  {
				switch (c)
				{
				  case 0x00E1:    //0xA0:      // Left-to-Right marker
					*(p-1) = 0x200E;
					break;
				  case 0x00ED:    //0xA1:      // Right-to-Left marker
					*(p-1) = 0x200F;
					break;
				}
			  }
			}
			break;
		} /* endswitch */

     }/* endif */
  }
  else if (pszUni)
  {
      *pszUni = 0;
    }
  return( pszUni );
}

ULONG ASCII2UnicodeBuf( PSZ pszASCII, PSZ_W pszUni, ULONG ulLen, ULONG ulCP )
{
	LONG  lBytesLeft = 0;
	ULONG ulOutPut = 0;

	ulOutPut = ASCII2UnicodeBufEx( pszASCII, pszUni, ulLen, ulCP, FALSE, NULL, &lBytesLeft );
	// For consistent behaviour with old ASCII2UnicodeBuf function where dwFlags was 0:
	// Within TM : FileRead expects to get EOS at position ulOutPut
    if (lBytesLeft && (ulOutPut < ulLen))
    {
       *(pszUni+ulOutPut) = EOS;
       ulOutPut++;
    }
	return(ulOutPut);
}

// ulLen in number of CHAR_W's which can be in pszUni!
ULONG ASCII2UnicodeBufEx( PSZ pszASCII, PSZ_W pszUni, ULONG ulLen, ULONG ulCP,
                          BOOL fMsg, PLONG plRc, PLONG plBytesLeft )
{
  ULONG  ulOutPut = 0;
  ULONG  ulTempCP = ulCP;
  LONG   lRc = 0;

  if ( pszASCII && pszUni )
  {
    if (!ulTempCP)
    {
       ulTempCP = GetLangOEMCP(NULL);
    }

    // always use 932 when 943 is specified
    if ( ulTempCP == 943 ) ulTempCP = 932;


    *pszUni = EOS;
    if (ulLen)
    {
      ulOutPut = MultiByteToWideChar( ulTempCP, MB_ERR_INVALID_CHARS,
                                    pszASCII, ulLen,
                                    pszUni, ulLen );
      if (!ulOutPut)
      {
        lRc = GetLastError();
        if ((lRc == ERROR_NO_UNICODE_TRANSLATION) && IsDBCS_CP(ulTempCP) )
        {// the last byte in the buffer may be half of a DBCS char...
         // so try again with one byte less..
            ulOutPut = MultiByteToWideChar( ulTempCP, MB_ERR_INVALID_CHARS,
                                            pszASCII, ulLen-1, pszUni, ulLen-1 );
            if (!ulOutPut)
            { // the last byte was not the reason, so just go on
				lRc = GetLastError();
				ulOutPut = MultiByteToWideChar( ulTempCP, 0L, pszASCII,
				                                 ulLen, pszUni, ulLen );
				lRc = GetLastError();
		    }
		    else
		    {
				lRc = 0;
				if (plBytesLeft)
				{
					*plBytesLeft = 1;
			    }
		    }
	    }
      }
      else
      {
        switch ( ulTempCP )
        {
		    case 864:
			// do special handling for Arabic to allow shaping in even for 864 stuff
			{
			  PSZ_W p = pszUni;
			  CHAR_W c;

			  while ((c = *p++) != NULC)
			  {
				switch (c)
				{
				  case 0x009B:      // Left-to-Right marker
					*(p-1) = 0x200E;// Unicode character converted from CP864 (0x9B)
					break;
				  case 0x009C:      // Right-to-Left marker
					*(p-1) = 0x200F;// Unicode character converted from CP864 (0x9C)
					break;
				}
			  }
			  BidiConvertFETo06(pszUni, ulLen);
			}
			break;
		  case 862:
		  case 867:
			{
			  // Convert RLM&LRM characters correctly to UNICODE
			  PSZ_W p = pszUni;
			  CHAR_W c;
			  while ((c = *p++)!= NULC)
			  {
				switch (c)
				{
				  case 0x00E1:    //0xA0:      // Left-to-Right marker
					*(p-1) = 0x200E;
					break;
				  case 0x00ED:    //0xA1:      // Right-to-Left marker
					*(p-1) = 0x200F;
					break;
				}
			  }
			}
			break;
          } /* endswitch */
	    }
     }
  }
  else if (pszUni)
  {
    *pszUni = 0;
  }
  if (plRc)
  {
     *(plRc) = lRc;
  }
  
  // this function is always called with fMsg==FALSE, so the following code-block
  // is eliminated
  fMsg;
//  if (fMsg && lRc)
//  { CHAR szTemp[10];
//	PSZ pszTemp = szTemp;
//	sprintf(szTemp, "%d", lRc);
//    UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
//  }

  return( ulOutPut );
}


PSZ_W Ansi2Unicode( PSZ pszAnsi, PSZ_W pszUni, ULONG ulOemCP )
{
  PUCHAR  pConvTable = NULL;
  ULONG  ulLen = 0;

  if (!ulOemCP)
  {
      ulOemCP = GetLangOEMCP(NULL);
  }
  ulLen = strlen(pszAnsi);
  UtlQueryCharTableEx( ANSI_TO_ASCII_TABLE, &pConvTable, (USHORT)ulOemCP);
  if ( pszAnsi && pszUni )
  { // for Thai and DBCS countries, ANSI == ASCII and pConvTable is NULL
    if (pConvTable)
    {
      ULONG i;
      unsigned char * pData = (PUCHAR)pszAnsi;
      for ( i = 0; i < ulLen; i++ )
      {
        *pData = pConvTable[*pData];
        pData++;
      } /* endfor */
    }
    ASCII2Unicode( pszAnsi, (PSZ_W)pszUni, ulOemCP );
  }
  else
  {
      *pszUni = EOS;
  } /* endif */

  return( pszUni );
}


ULONG Ansi2UnicodeBuf( PSZ pszAnsi, PSZ_W pszUni, ULONG ulLen, ULONG ulOemCP)
{
  PUCHAR  pConvTable = NULL;
  ULONG   ulOutPut = 0;

  if (!ulOemCP)
  {
    ulOemCP = GetLangOEMCP(NULL);
  }
  UtlQueryCharTableEx( ANSI_TO_ASCII_TABLE, &pConvTable, (USHORT)ulOemCP);
  if ( pszAnsi && pszUni )
  { // for Thai and DBCS countries, ANSI == ASCII and pConvTable is NULL
    if (pConvTable)
    {
      ULONG i;
      unsigned char *pData = (PUCHAR) pszAnsi;
      for ( i = 0; i < ulLen; i++ )
      {
        *pData = pConvTable[*pData];
        pData++;
      } /* endfor */
    }
    ulOutPut = ASCII2UnicodeBuf( pszAnsi, (PSZ_W)pszUni, ulLen, ulOemCP );
  }
  else
  {
    *pszUni = EOS;
  } /* endif */
  return(ulOutPut);
}

ULONG UTF82UnicodeBuf( PSZ pszUTF8, PSZ_W pszUni, LONG Len )
{
	return(UTF82UnicodeBufEx( pszUTF8, pszUni, Len, FALSE, NULL, NULL ));
}

// ulLen in number of CHAR_W's which can be in pszUni!
ULONG UTF82UnicodeBufEx( PSZ pszUTF8, PSZ_W pszUni, LONG Len, BOOL fMsg, PLONG plRc,
                          PLONG plBytesLeft )
{
  ULONG ulOutPut = 0;
  DWORD dwFlags = 0;
  USHORT   usI = 0;
  LONG     lRc = 0;

  if ( pszUTF8 && pszUni )
  {
	 *pszUni = EOS;
	 if (Len)
	 {// set dwFlags only if on WinXP, otherwise MultiByteToWideChar fails!
	    if ( UtlGetOperatingSystemInfo()== OP_WINXP)
	    {
	        dwFlags = MB_ERR_INVALID_CHARS;
	        ulOutPut = MultiByteToWideChar( CP_UTF8, dwFlags, pszUTF8, Len,
			                                    pszUni, Len );

			if ( !ulOutPut)
			{
			  lRc = GetLastError();
			  while (lRc == ERROR_NO_UNICODE_TRANSLATION && !(usI == 4)) /* chg 3->4 11-4-14 */
			  {// the last byte in the buffer may be part of a Multibyte char...
				 // so try again with one byte less..
				usI++;
				ulOutPut = MultiByteToWideChar( CP_UTF8, dwFlags,
											  pszUTF8,Len-usI, pszUni, Len-usI );

				if (!ulOutPut)
				{ // the last byte was not the reason, so just go on
					lRc = GetLastError();
				}
				else
				{
				   lRc = 0;
				}
			  } /* endwhile */
		    }
	   }
	   else
	   {
		   ULONG ulNewOutPut = 0;
		   ulOutPut = MultiByteToWideChar( CP_UTF8, dwFlags, pszUTF8, Len,
			                                    pszUni, Len );
		   ulNewOutPut = ulOutPut;
		   while (ulNewOutPut && (usI < 4) && (Len > usI) && (ulNewOutPut == ulOutPut))
		   {
			 usI++;
		     ulNewOutPut = MultiByteToWideChar( CP_UTF8, dwFlags, pszUTF8, Len-usI,
			                                    pszUni, 0 );
           }
           if (ulNewOutPut != ulOutPut)
           {
			   usI--;
	       }
	       if (!ulOutPut)
	       {
			   lRc = GetLastError();
	       }
	       else
	       {
			   lRc = 0;
	       }
       }

	   if (plRc)
	   {
		   *plRc = lRc;
	   }
	   if (plBytesLeft && !lRc)
	   {
		   *plBytesLeft = usI;
       }
		// this function is always called with fMsg==FALSE, so the following code-block
		// is eliminated
		fMsg;
//		if (fMsg && lRc)
//		{  CHAR szTemp[10];
//		   PSZ pszTemp = szTemp;
//		   sprintf(szTemp, "%d", lRc);
//		   UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
//		}
     }
  }
  else if (pszUni)
  {
    *pszUni = 0;
  }
  return( ulOutPut );
}

ULONG Unicode2UTF8Buf(PSZ_W pszUni, PSZ pszUTF8, LONG Len)
{
  return(Unicode2UTF8BufEx(pszUni, pszUTF8, Len, Len, FALSE, NULL));
}

// lBufLen = # of bytes which can be in buffer
// Len = # of char_w's in pszUni input buffer
ULONG Unicode2UTF8BufEx( PSZ_W pszUni, PSZ pszUTF8, LONG Len, LONG lBufLen, BOOL fMsg, PLONG plRc )
{
  ULONG ulOutPut = 0;
  LONG  lRc = 0;
  // it is assumed, that pszUTF8 is large enough to hold the converted string
  if ( pszUni && pszUTF8 )
  {
	  *pszUTF8 = EOS;
	  if (Len)
	  {
		ulOutPut = WideCharToMultiByte( CP_UTF8, 0, (LPCWSTR)pszUni, Len,
										pszUTF8, lBufLen, NULL, NULL );
		if (!ulOutPut)
		{
		  lRc = GetLastError();
			// this function is always called with fMsg==FALSE, so the following code-block
			// is eliminated
			fMsg;
//		  if (fMsg && lRc)
//		  {CHAR szTemp[10];
//		   PSZ pszTemp = szTemp;
//		   sprintf(szTemp, "%d", lRc);
//		   UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
//		  }
		}
		if (plRc )
		{
		  *(plRc) = lRc;
		}
	  }
  }
  else if (pszUTF8)
  {
    *pszUTF8 = EOS;
  }
  return( ulOutPut );       //# of bytes written
}


// Function to check if specified character is a DBCS character or not
BOOL EQFIsDBCSChar( CHAR_W c, ULONG ulCP)
{
	CHAR_W chW[2];
	CHAR ch[5];
	LONG lRc = 0;

	chW[0] = c; chW[1] = EOS;
	memset (ch, 0, sizeof(ch));

	return (Unicode2ASCIIBufEx(chW, ch, 1, sizeof(ch), ulCP, &lRc) == 2);

}




///////////////////////////////////////////////////////////////////////////////
///  UTF16strncpy   copy up to ulLen wide characters                        ///
///////////////////////////////////////////////////////////////////////////////
PSZ_W UTF16strncpy(PSZ_W pusTarget, PSZ_W pusSource, LONG lLen)
{
  // usLen is number of CHAR_W's!!
  PSZ_W pszTarget = pusTarget;
  while ( *pusSource != 0 && (lLen > 0))
  {
    *pusTarget++ = *pusSource++;
    lLen--;
  } /* endwhile */
  if ( lLen > 0 )
  {
    *pusTarget = EOS;
  }
  return( pszTarget );

}

///////////////////////////////////////////////////////////////////////////////
///  UTF16stricmp   copy up to usLen wide characters                        ///
///////////////////////////////////////////////////////////////////////////////
int UTF16stricmp(PSZ_W pusTarget, PSZ_W pusSource)
{
  return _wcsicmp( pusTarget, pusSource );
}

///////////////////////////////////////////////////////////////////////////////
///  UTF16memset   initializes usLen wide characters with c                 ///
///////////////////////////////////////////////////////////////////////////////
PSZ_W UTF16memset( PSZ_W pusString, CHAR_W c, USHORT usNum )
{
  PSZ_W p = pusString;
  while ( usNum -- )
    *pusString++ = c;
  return p;
}


// Convert FE to 06 in Arabic Strings
// Input: CHAR_W *   ptr to string
//        bufferlen  Lengt of string

void BidiConvertFETo06
(
  CHAR_W * lpWideCharStr,
  ULONG Length
) //lpWideCharStr is a Zero terminated buffer.
{
  ULONG i = 0;
  CHAR_W  c;
  for ( i=0; i<Length; i++)
  {
    c = lpWideCharStr[i];

    if ( (c >= 0xFE80) && (c <= 0xFEF3 ) )
    {
        lpWideCharStr[i] =  (CHAR_W)(TabFEto06 [ (c - 0xFE80) ] );

    // Substitute character only if input char has not already been NULL
    // function is used for converting contents of buffers too, and there
    // may be NULL's in the buffers!

       if (lpWideCharStr[i] == 0x0000)
              lpWideCharStr[i] = 0x001A;
    }
  }
}

// Reverse direction (from Unicode to 864)
void BidiConvert06ToFE(LPWSTR lpWideCharStr, int Length) //lpWideCharStr is a Zero terminated buffer.
{
  int i = 0;
  for ( i=0; i<Length; i++)
  {
    if ( (lpWideCharStr[i] >= 0x0621) && (lpWideCharStr[i] <= 0x064A ) )
        lpWideCharStr[i] = (CHAR_W)(Tab06ToFE [ (lpWideCharStr[i] - 0x0621) ] );

  }
}

//ulLen in number of CHAR_W's which are in pszUni!
ULONG UtlDirectUnicode2AnsiBufInternal( PSZ_W pszUni, PSZ pszAnsi, ULONG ulLen, LONG lBufLen,
                             ULONG ulAnsiCP, PLONG plRc, PSZ pszTemp )
{
  static CHAR_W szUniTemp[ MAX_SEGMENT_SIZE ];
  ULONG ulOutPut = 0;
  USHORT usAnsiCP = (USHORT) ulAnsiCP;
  LONG   lRc = 0;

  if (plRc)
  {
	  *plRc = 0;
  }
  if (!usAnsiCP)
  {
     usAnsiCP = (USHORT)GetLangAnsiCP(NULL);
  }
  if ( pszUni && pszAnsi )
  {
	// do special handling for Arabic to get rid of shaping to allow for conversion
	// back to 864/1256 ?? nec or not??

	*pszAnsi = EOS;
	if (ulLen)
	{
    // always use 932 when 943 is specified
    if ( usAnsiCP == 943 ) usAnsiCP = 932;

		ulOutPut = WideCharToMultiByte( usAnsiCP, 0, (LPCWSTR)pszUni, ulLen,
											pszAnsi, lBufLen, NULL, NULL );
		if (plRc && !ulOutPut)
		{
			lRc = GetLastError();
		  *(plRc) = lRc;
		}
		if (pszTemp && lRc)
		{
		   sprintf(pszTemp, "%d", lRc);
// UtlError-call moved to UtlString2.c
//		   UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );

		}
    }
  }
  else if (pszAnsi)
  {
    *pszAnsi = EOS;
  }
  return( ulOutPut );     // # of bytes written
}

// ulLen in number of CHAR_W's which can be in pszUni!
// rc ulOutPut2 in # of char-w's
ULONG UtlDirectAnsi2UnicodeBufInternal( PSZ pszAnsi, PSZ_W pszUni, ULONG ulLen,
                              ULONG ulAnsiCP, PLONG plRc, PLONG plBytesLeft, PSZ pszTemp)
{
  ULONG  ulOutPut = 0;
  ULONG  ulTempCP = ulAnsiCP;
  LONG   lRc = 0;

  if (plRc)
  {
  	  *plRc = 0;
  }
  if ( pszAnsi && pszUni )
  {
    if (!ulTempCP)
    {
       ulTempCP = GetLangAnsiCP(NULL);
    }

    // always use 932 when 943 is specified
    if ( ulTempCP == 943 ) ulTempCP = 932;


    if (ulLen)
    {
		*pszUni = EOS;
		ulOutPut = MultiByteToWideChar( ulTempCP, MB_ERR_INVALID_CHARS,
                                    pszAnsi, ulLen,
                                    pszUni, ulLen );
        if (!ulOutPut)
        {
          lRc = GetLastError();
          if ((lRc == ERROR_NO_UNICODE_TRANSLATION) && IsDBCS_CP(ulTempCP))
          {// the last byte in the buffer may be half of a DBCS char...
           // so try again with one byte less..
              ulOutPut = MultiByteToWideChar( ulTempCP, MB_ERR_INVALID_CHARS,
                                            pszAnsi, ulLen-1, pszUni, ulLen-1 );
              if (!ulOutPut)
              { // the last byte was not the reason, so just go on
  				lRc = GetLastError();
				ulOutPut = MultiByteToWideChar( ulTempCP, 0L, pszAnsi,
				                                 ulLen, pszUni, ulLen );
				lRc = GetLastError();
              }
              else
              {
				lRc = 0;
				if (plBytesLeft)
				{
					*plBytesLeft = 1;
			    }
              }
           }
        }

		if (plRc && !ulOutPut)
		{
		  *(plRc) = lRc;
		}
		if (pszTemp && lRc)
		{
		   sprintf(pszTemp, "%d", lRc);
// UtlError-call moved to UtlString2.c
//			UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
		}
     }
  }
  else if (pszUni)
  {
    *pszUni = 0;
  }
  return( ulOutPut );   // # of bytes written
}


PSZ_W UtlDirectAnsi2Unicode( PSZ pszAnsi, PSZ_W pszUni, ULONG ulAnsiCP )
{
  int iRC = 0;
  USHORT  usCP = (USHORT)ulAnsiCP;

  if ( pszAnsi && pszUni )
  {
	  if (*pszAnsi == EOS )
	  {
		  *pszUni = EOS;
      }
      else
      {
		if (!usCP)
		{
		  usCP = (USHORT)GetLangAnsiCP(NULL);
		}

    // always use 932 when 943 is specified
    if ( usCP == 943 ) usCP = 932;


		iRC = MultiByteToWideChar( usCP, 0, pszAnsi, -1, pszUni, MAX_SEGMENT_SIZE );
		if ( iRC == 0 ) iRC = GetLastError();
		if ( iRC == ERROR_INVALID_PARAMETER )
		{
		  // codepage not supported by OS, use default code page for conversion
			  MultiByteToWideChar( CP_ACP, 0, pszAnsi, -1, pszUni, MAX_SEGMENT_SIZE );
		} /* endif */
     }/* endif */
  }
  else if (pszUni)
  {
      *pszUni = 0;
  }
  return( pszUni );
}

///////////////////////////////////////////////////////////////////////////////
///  UtlDirectUnicode2Ansi  convert unicode string to Ansi                          ///
///////////////////////////////////////////////////////////////////////////////

PSZ UtlDirectUnicode2Ansi( PSZ_W pszUni, PSZ pszAnsi, ULONG ulAnsiCP )
{
  // it is assumed, that pszAnsi is large enough to hold the converted string
  USHORT   usCP = (USHORT) ulAnsiCP;
  int      iRC = 0;
  if ( pszUni && pszAnsi )
  {
	if (*pszUni == EOS)
	{
		*pszAnsi = EOS;
    }
    else
    {
		if (!usCP)
		{
		  usCP = (USHORT)GetLangAnsiCP(NULL);
		}

    // always use 932 when 943 is specified
    if ( usCP == 943 ) usCP = 932;

		iRC = WideCharToMultiByte( usCP, 0, (LPCWSTR)pszUni, -1,
								 pszAnsi, MAX_SEGMENT_SIZE, NULL, NULL );
		if ( iRC == 0) iRC = GetLastError();
		// possible: ERROR_INSUFFICIENT_BUFFER /ERROR_INVALID_FLAGS / ERROR_INVALID_PARAMETER
		if ( iRC == ERROR_INVALID_PARAMETER )
		{
		  // codepage not supported by OS, use default code page for conversion
		  MultiByteToWideChar( CP_ACP, 0, pszAnsi, -1, pszUni, MAX_SEGMENT_SIZE );
		} /* endif */

    } /* endif */
  }
  else if (pszAnsi)
  {
    *pszAnsi = EOS;
  }
  return( pszAnsi );
}


//+----------------------------------------------------------------------------+
// External function                                                            
//+----------------------------------------------------------------------------+
// Function name:     UtlQueryCharTable   Return ptr to character table         
//+----------------------------------------------------------------------------+
// Description:       Returns a pointer to the requested character conversion   
//                    or identification table.                                  
//+----------------------------------------------------------------------------+
// Function call:     usRC = UtlQueryCharTable( CHARTABLEID TableID,            
//                                              PUCHAR      *ppTable );         
//+----------------------------------------------------------------------------+
// Input parameter:   CHARTABLEID  TableID    ID of requested table             
//+----------------------------------------------------------------------------+
// Output parameter:  PUCHAR       *ppTable   address of caller's table pointer 
//+----------------------------------------------------------------------------+
// Returncode type:   USHORT                                                    
//+----------------------------------------------------------------------------+
// Returncodes:       NO_ERROR             function completed successfully      
//                    ERROR_INVALID_DATA   no table with the given ID available 
//+----------------------------------------------------------------------------+
// Prerequesits:      none                                                      
//+----------------------------------------------------------------------------+
// Side effects:      none                                                      
//+----------------------------------------------------------------------------+
// Function flow:     set caller's table pointer to requested table or          
//                      set return code for unknown tables                      
//                    return function return code                               
//+----------------------------------------------------------------------------+
USHORT UtlQueryCharTable
(
  CHARTABLEID TableID,                 // ID of requested table
  PUCHAR      *ppTable                 // address of caller's table pointer
)
{
  return( UtlQueryCharTableEx( TableID, ppTable, 0 ) );
} /* end of function UtlQueryCharTable */

USHORT UtlQueryCharTableLang
(
  CHARTABLEID TableID,
  PUCHAR      *ppTable,
  PSZ         pszLanguage
)
{
  ULONG    ulCP = 0L;

  ulCP = GetLangOEMCP(pszLanguage);
  return(UtlQueryCharTableEx(TableID, ppTable, (USHORT)ulCP));

} /* end of function UtlQueryCharTableLang */


USHORT UtlQueryCharTableEx
(
  CHARTABLEID TableID,                 // ID of requested table
  PUCHAR      *ppTable,                // address of caller's table pointer
  USHORT      usCodePage               // code page to be used or 0
)
{
  USHORT      usRC = NO_ERROR;

  switch ( TableID )
  {
    case IS_TEXT_TABLE :
      *ppTable = chIsText;
      break;

    case ANSI_TO_ASCII_TABLE :
      {
        DOSVALUE usCP;
        PUCHAR   pTable;

        if ( usCodePage != 0 )
        {
          usCP = usCodePage;
        }
        else
        {
           TCHAR        cp [6];
           GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTCODEPAGE, cp, sizeof(cp));
           usCP = (USHORT)_ttol (cp);
        } /* endif */
        switch ( usCP )
        {
          case 852  : pTable = chAnsiToPC852; break;
          case 855  : pTable = chAnsiToPC855; break;
          case 866  : pTable = chAnsiToPC866; break;
          case 869  : pTable = chAnsiToPC869; break;
          case 915  :
          case 28595: pTable = chAnsiToPC915; break;
          case 857  : pTable = chAnsiToPC857; break;
          case 862  : pTable = chAnsiToPC862; break;
          case 813  : pTable = chAnsiToPC813; break;
          case 737  :
            if ( (GetOEMCP() == 869) && (GetKBCodePage() == 869))
            { // fix for sev1 Greek: Win NT problem (01/09/23)
                  usCP = 869;
                  pTable = chAnsiToPC869;
            }
            else
            {
                pTable = chAnsiToPC737;
            } /* endif */
            break;
          case 775  : pTable = chAnsiToPC775; break;
          case 864  : pTable = chAnsiToPC864; break;  // Arabic OS/2
          case 720  : pTable = chAnsiToPC864; break;  // Arabic Windows
          case 874  : pTable = NULL;          break; //Thai
          case 932 : pTable  = NULL;         break; //Jap
          case 936 : pTable = NULL;          break; //Chin-s
          case 950 : pTable = NULL;          break; // Chin-t
          case 943 : pTable  = NULL;         break; //Jap2
          case 949 : pTable = NULL;          break; //Korean
          case 1251: pTable = NULL;          break; // Belarusian + Ukrainian
          case 28603:                               //ISO 8859-13 is identical to 921
          case 921  : pTable = chAnsiToPC921; break; //Lithuanian, Latvian, Estonian
          case 850  : pTable = chAnsiToPC850; break;
          default   :
             pTable = chAnsiToPC850;
             usRC = ERROR_NOT_READY;
             break;

        } /* endswitch */

        *ppTable = pTable;
      }
      break;

    case ASCII_TO_ANSI_TABLE :
      {
        DOSVALUE usCP;
        PUCHAR   pTable;
        PUCHAR   pInvTable;
        SHORT i;
        if ( usCodePage != 0 )
        {
          usCP = usCodePage;
        }
        else
        {
          TCHAR        cp [6];
          GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTCODEPAGE, cp, sizeof(cp));
          usCP = (USHORT)_ttol (cp);
        } /* endif */
        switch ( usCP )
        {
          case 852  : pTable = chAnsiToPC852;  pInvTable = chPC852ToAnsi; break;
          case 855  : pTable = chAnsiToPC855;  pInvTable = chPC855ToAnsi; break;
          case 866  : pTable = chAnsiToPC866;  pInvTable = chPC866ToAnsi; break;
          case 869  : pTable = chAnsiToPC869;  pInvTable = chPC869ToAnsi; break;
          case 28595:
          case 915  : pTable = chAnsiToPC915;  pInvTable = chPC915ToAnsi; break;
          case 857  : pTable = chAnsiToPC857;  pInvTable = chPC857ToAnsi; break;
          case 862  : pTable = chAnsiToPC862;  pInvTable = chPC862ToAnsi; break;
          case 813  : pTable = chAnsiToPC813;  pInvTable = chPC813ToAnsi; break;
          case 737  :
            if ( (GetOEMCP() == 869) && (GetKBCodePage() == 869) )
            { // fix for sev1 Greek: Win NT problem (01/09/23)
              usCP = 869;
              pTable = chAnsiToPC869;
              pInvTable = chPC869ToAnsi;
            }
            else
            {
              pTable = chAnsiToPC737;
              pInvTable = chPC737ToAnsi;
            } /* endif */
            break;
          case 775  : pTable = chAnsiToPC775;  pInvTable = chPC775ToAnsi; break;
          case 864  : pTable = chAnsiToPC864;  pInvTable = chPC864ToAnsi; break;  // Arabic OS/2
          case 720  : pTable = chAnsiToPC864;  pInvTable = chPC864ToAnsi; break;    // Arabic Windows
          case 874  : pTable = NULL;           pInvTable = NULL;          break;  // Thai
          case 932  : pTable = NULL;           pInvTable = NULL;          break;  // Jap
          case 936  : pTable = NULL;           pInvTable = NULL;          break;  // Chin
          case 950  : pTable = NULL;           pInvTable = NULL;          break;  // Chin
          case 949  : pTable = NULL;           pInvTable = NULL;          break;  // Korean
          case 1251 : pTable = NULL;           pInvTable = NULL;          break;  // Belarusian + Ukrainian
          case 28603:
          case 921  : pTable = chAnsiToPC921;  pInvTable = chPC921ToAnsi; break;  // Baltic 921-1257
          case 850  : pTable = chAnsiToPC850;  pInvTable = chPC850ToAnsi; break;
          default   :
             pTable = chAnsiToPC850;
             pInvTable = chPC850ToAnsi;
             usRC = ERROR_NOT_READY;
             break;
        } /* endswitch */

        // chPC864ToAnsi already filled up -- therefore it need not te be created
        if ( pInvTable && pTable && (pInvTable != &chPC864ToAnsi[0]))
        {
          memset( pInvTable, 0, 256 );
          for ( i = 0; i < 256; i++ )
          {
            pInvTable[pTable[i]] = (CHAR)i;
          } /* endfor */
          // any empty slots should be filled up with blanks
          for (i = 1; i < 256; i++)
          {
                 if (pInvTable[i] == 0)
             pInvTable[i] = ' '; //i;
          }
        } /* endif */
        *ppTable = pInvTable;
      }
      break;

    default :
      usRC = ERROR_INVALID_DATA;
      break;
  } /* endswitch */

  return( usRC );
} /* end of UtlQueryCharTableEx */

//+----------------------------------------------------------------------------+
// External function                                                            
//+----------------------------------------------------------------------------+
// Function name:     UtlGetOperatingSystemInfo                                 
//+----------------------------------------------------------------------------+
// Function call:     SHORT UtlGetOperatingSystemInfo(VOID)                     
//+----------------------------------------------------------------------------+
// Description:  Gets information about the running operating system            
//               returns                                                        
//               OP_WIN31X        Win32s on Windows 3.11                        
//               OP_WINDOWS       Windows 95/98                                 
//               OP_WINDOWSNT     Windows NT                                    
//               OP_WINDOWS2K     Windows 2000                                  
//               OP_WINXP         Windows XP                                    
//               OP_NO_WINDOWS    other                                         
//+----------------------------------------------------------------------------+
// Returncode type:  VOID                                                       
//+----------------------------------------------------------------------------+
// Function flow:                                                               
//                                                                              
//+----------------------------------------------------------------------------+

SHORT UtlGetOperatingSystemInfo( VOID )
{
    OSVERSIONINFOEX osvi;
    BOOL            fOsVersionInfoEx;

    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    fOsVersionInfoEx = GetVersionEx((OSVERSIONINFO *) &osvi);
    if (!fOsVersionInfoEx)
    {
		osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
		if (!GetVersionEx((OSVERSIONINFO *) &osvi))
		  return FALSE;
    }


    switch (osvi.dwPlatformId)
    {
    case VER_PLATFORM_WIN32s:
       return OP_WIN31X;
       break;
    case VER_PLATFORM_WIN32_WINDOWS:
       return OP_WINDOWS;
       break;
    case VER_PLATFORM_WIN32_NT:
       if ( osvi.dwMajorVersion <= 4 )
          return OP_WINDOWSNT;
       if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0)
          return OP_WINDOWS2K;
       if (osvi.dwMajorVersion == 5 && osvi.dwMinorVersion >= 1)
          return OP_WINXP;
          // dwMinorVersion == 2 is MS WIndows Server 2003 family
       return OP_WINXP;
       break;
    default:
       return (OP_NO_WINDOWS);
    }
} // end of UtlGetOperatingSystemInfo

//+----------------------------------------------------------------------------+
// Internal function                                                            
//+----------------------------------------------------------------------------+
// Function Name:     UtlReplString          Replace a string value             
//+----------------------------------------------------------------------------+
// Description:       Replaces a MAT string value.                              
//                    This procedure is NOT a public procedure!                 
//+----------------------------------------------------------------------------+
// Function Call:     VOID UtlReplString( SHORT sID, PSZ pszString );           
//+----------------------------------------------------------------------------+
// Parameters:        sID is the string ID (QST_...)                            
//                    pszString is the new value for the string                 
//+----------------------------------------------------------------------------+
// Returncode type:   VOID                                                      
//+----------------------------------------------------------------------------+
// Function flow:     if ID is in valid range then                              
//                      free old value in internal buffers                      
//                      store copy of string in internal buffers                
//                    endif                                                     
//+----------------------------------------------------------------------------+
VOID UtlReplString( SHORT sID, PSZ pszString )
{
  PSZ  pszBuffer;                      // buffer for new string value

   if ( (sID > QST_FIRST) && (sID < QST_LAST) )
   {
      if ( UtlAlloc( (PVOID *)&pszBuffer, 0L,
                     (LONG)  max( MIN_ALLOC, strlen(pszString)+1 ),
                     ERROR_STORAGE ) )
      {
        USHORT      usTask = UtlGetTask();
        strcpy( pszBuffer, pszString );

        if ( UtiVar[usTask].pszQueryArea[sID] )
        {
           UtlAlloc( (PVOID *)&(UtiVar[usTask].pszQueryArea[sID]), 0L, 0L, NOMSG );
        } /* endif */

        UtiVar[usTask].pszQueryArea[sID] = pszBuffer;
      } /* endif */
   } /* endif */
}

//ulLen in number of CHAR_W's which are in pszUni!
ULONG UtlDirectUnicode2AnsiBuf( PSZ_W pszUni, PSZ pszAnsi, ULONG ulLen, LONG lBufLen,
                             ULONG ulAnsiCP, BOOL fMsg, PLONG plRc )
{
	ULONG ulOutPut = 0;
	CHAR szTemp[10];
	PSZ pszTemp = szTemp;

	ulOutPut = UtlDirectUnicode2AnsiBufInternal( pszUni, pszAnsi, ulLen, lBufLen,
												ulAnsiCP, plRc, pszTemp);
	if (fMsg && *plRc)
	{  
		UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
	}
	return( ulOutPut );     // # of bytes written
}

// ulLen in number of CHAR_W's which can be in pszUni!
// rc ulOutPut2 in # of char-w's
ULONG UtlDirectAnsi2UnicodeBuf( PSZ pszAnsi, PSZ_W pszUni, ULONG ulLen,
                              ULONG ulAnsiCP, BOOL fMsg, PLONG plRc, PLONG plBytesLeft)
{
	ULONG  ulOutPut = 0;
	CHAR szTemp[10];
	PSZ pszTemp = szTemp;

	ulOutPut = UtlDirectAnsi2UnicodeBufInternal( pszAnsi, pszUni, ulLen,
												ulAnsiCP, plRc, plBytesLeft, 
												pszTemp);

	if (fMsg && *plRc)
	{  
		UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
	}
	return( ulOutPut );   // # of bytes written
}

//+----------------------------------------------------------------------------+
// External function                                                            
//+----------------------------------------------------------------------------+
// Function name:     UtlCompIgnWhiteSpace                                      
//+----------------------------------------------------------------------------+
// Function call:     UtlCompIgnWhiteSpace(PSZ, PZS, USHORT)                    
//+----------------------------------------------------------------------------+
// Description:       this function will return 0 if both strings differ only   
//                    in blanks or linefeeds. ( one blank/LF / CRLF is seen as  
//                    equal to one CRLF / LF / more blanks)                     
//                    BUT if string1 has one blank between two other characters 
//                    and string2 has the two characters without any kind of    
//                    whitespace between, the strings are not equal             
//                    Example:                                                  
//     string1:       <a href=a.htm#Transfer calls">                            
//     string2:       <a href=a.htm#Transfercalls">        NOT EQUAL            
//     string1:       <a href=a.htm#Transfer calls">                            
//     string2:       <a href=a.htm#Transfer    calls">        EQUAL            
//     string1:       This is \r\nOk                                            
//     string2:       This    is \nOk                          EQUAL            
// P016340: string1:  \x0AThis is Ok                                            
//     string2:       This    is \x0AOk                        EQUAL            
//+----------------------------------------------------------------------------+
// Parameters:        PSZ      pString1     string to be compared               
//                    PSZ      pString2     string to be compared               
//                    USHORT   usLen        length of longer string             
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT                                                     
//+----------------------------------------------------------------------------+
// Returncodes:       0               strings are the same                      
//                    number !=0      strings are different                     
//+----------------------------------------------------------------------------+
// Function flow:     _                                                         
//+----------------------------------------------------------------------------+
LONG UtlCompIgnWhiteSpaceW( PSZ_W pD1, PSZ_W pD2, ULONG ulLen )
{
  LONG   lRc = 0;
  ULONG  ulI = 0;
  CHAR_W c, d;
  BOOL fNullTerminated = FALSE;
  BOOL   ulIncreaseI = FALSE;

  if (ulLen == 0 )
  {
    ULONG   ulLenTmp;

    fNullTerminated = TRUE;
    ulLenTmp = UTF16strlenCHAR(pD2);
    ulLen    = UTF16strlenCHAR(pD1);
    if (ulLenTmp > ulLen )
    {
      ulLen = ulLenTmp;
    } /* endif */
  } /* endif */

  // RJ: skip leading whitespaces in both strings
  if ( ulLen )
  {
    while ( UtlIsWhiteSpaceW( *pD1 ) && (ulI < ulLen) )
    {
      pD1++;
      ulI++;
    }
    while ( UtlIsWhiteSpaceW( *pD2 ) ) pD2++;
  }  /* endif */

  while ( ulI++ < ulLen )
  {
    c = *pD1; d = *pD2;
    if ( UtlIsWhiteSpaceW(c) && UtlIsWhiteSpaceW(d) )
    {
      pD1++; pD2++;
      /****************************************************************/
      /* skip any consecutive white spaces                            */
      /****************************************************************/
      ulIncreaseI = FALSE;
      while ( UtlIsWhiteSpaceW( *pD1 ) && (ulI < ulLen) )
      {
        pD1++;
        ulI++;
        ulIncreaseI = TRUE;
      } /* endwhile */
      while ( UtlIsWhiteSpaceW( *pD2 ) )
      {
        pD2++;
      } /* endwhile */
      // if we reached the end of our string do a sRc setting
      // IV000162: i.e.RTF Tag "\line ": add ulIncreaseI
      // otherwise tags ending both with one blank are recognized as different!
      if (ulIncreaseI && (ulI >= ulLen ))
      {
        lRc = (*pD1 - *pD2);
      }
    }
    else if ( c == d )
    {
      pD1++; pD2++;
    }
    else
    {
      ulI = ulLen;   // stop loop;
      lRc = ( c-d );
    } /* endif */
  } /* endwhile */

  // GQ: check if both strings have been processed completely (but ignore
  // any trailing whitespace characters
  if ( !lRc && fNullTerminated )
  {
    while ( *pD1 && UtlIsWhiteSpaceW( *pD1 ) ) pD1++;
    while ( *pD2 && UtlIsWhiteSpaceW( *pD2 ) ) pD2++;

    if ( *pD1 )
    {
      lRc = 1;
    }
    else if ( *pD2 )
    {
      lRc = -1;
    } /* endif */
  } /* endif */

  return lRc;
}

SHORT UtlCompIgnWhiteSpace( PSZ pD1, PSZ pD2, USHORT usLen )
{
  SHORT sRc = 0;
  USHORT usI = 0;
  BYTE   c, d;

  if (usLen == 0 )
  {
    USHORT  usLenTmp;

    usLenTmp = (USHORT)strlen(pD2);
    usLen    = (USHORT)strlen(pD1);
    if (usLenTmp > usLen )
    {
      usLen = usLenTmp;
    } /* endif */
  } /* endif */

  while ( usI++ < usLen )
  {
    c = *pD1; d = *pD2;
    if ( isspace(c) && isspace(d) )
    {
      pD1++; pD2++;
      /****************************************************************/
      /* skip any consecutive white spaces                            */
      /****************************************************************/
      while ( isspace( *pD1 ) )
      {
        pD1++;
        usI++;
      } /* endwhile */
      while ( isspace( *pD2 ) )
      {
        pD2++;
      } /* endwhile */
    }
    else if ( c == d )
    {
      pD1++; pD2++;
    }
    else
    {
      usI = usLen;   // stop loop;
      sRc = ( c-d );
    } /* endif */
  } /* endwhile */
  return sRc;
}

LONG UtlCompIgnSpaceW( PSZ_W pD1, PSZ_W pD2, ULONG ulLen )
{
  LONG  lRc = 0;
  ULONG ulI = 0;
  CHAR_W c, d;

  if (ulLen == 0 )
  {
    ULONG  ulLenTmp;

    ulLenTmp = UTF16strlenCHAR(pD2);
    ulLen    = UTF16strlenCHAR(pD1);
    if (ulLenTmp > ulLen )
    {
      ulLen = ulLenTmp;
    } /* endif */
  } /* endif */

  while ( ulI++ < ulLen )
  {
    c = *pD1; d = *pD2;
    if ( (c == ' ') && (d == ' ') )
    {
      pD1++; pD2++;
      /****************************************************************/
      /* skip any consecutive white spaces                            */
      /****************************************************************/
      while ( *pD1 == ' ' )
      {
        pD1++;
        ulI++;
      } /* endwhile */
      while ( *pD2 == ' ' )
      {
        pD2++;
      } /* endwhile */
    }
    else if ( c == d )
    {
      pD1++; pD2++;
    }
    else
    {
      ulI = ulLen;   // stop loop;
      lRc = ( c-d );
    } /* endif */
  } /* endwhile */
  return lRc;
}

BOOL UtlIsWhiteSpaceW( CHAR_W c )
{
  BOOL isWhiteSpace = ( (c == L' ') || (c == L'\r') || (c == L'\n') || (c == L'\t') );
  return ( isWhiteSpace );
}

/*!
    Name:         usConvertCRLF                                               
    Purpose:      Converts an array of characters to a specified output       
                  format with either only LF or both CRLF as the line change  
                  characters.                                                 
    Parameters:   1.PCHAR   - pointer to input character array                
                  2.USHORT  - length of input area                            
                  3.PCHAR   - pointer to result character array               
                  4.BOOL    - flag indicating whether CRLF or LF is desired   
                              as the output format; TRUE means only LF is     
                              used. FALSE uses CRLF as the line end code      
    Returns:      USHORT    - number of characters that have been written to  
                              the output area                                 
    Comments:     This function converts a string with either CRLF or LF into 
                  a fixed form where only CRLF or LF is used. The output form 
                  is controlled with a flag indicating the conversion mode.   
                  If a normal \0 ended string is used as input, the input     
                  length must include the \0 character otherwise it will not  
                  be copied to the output string (with unpredictable results) 
                                                                              
    Samples:      usRc = usConvertCRLF (pszInput, strlen (pszInput) + 1,      
                                        pszOutput, TRUE);                     
                  usOutputLength = usConvertCRLF (pcInputArea, usInputLength, 
                                        pcOutputArea, FALSE);                 
    Examples:     "123\r\n45\n67\r\n" will be converted to :                  
                   fMode = TRUE:  "123\n45\n67\n"                             
                   fMode = FALSE: "123\r\n45\r\n67\r\n"                       
*/
USHORT usConvertCRLF (PCHAR  pcInput,     // pointer to character input area
                      USHORT usInputLen,  // length of input area
                      PCHAR  pcOutput,    // pointer to output area
                      BOOL   fMode)       // conversion mode :
                                          //   TRUE -> convert to LF
                                          //   FALSE -> convert to CRLF

{
   USHORT   usOutputLen = 0,              // length of converted output
            usIndex;                      // index of input string

   for (usIndex = 0; usIndex < usInputLen; usIndex++)
   {
      if ((*pcInput != '\r') && (*pcInput != '\n'))
      {
         // no special handling required -> copy character and increase pointer
         *(pcOutput++) = *(pcInput++);
         usOutputLen++;
      }
      else
      {
         if (fMode)
         {
            // fMode is TRUE so convert CRLF to single LF
            // if input area contains CRLF, skip CR and copy next char
            // if input area contains only CR without LF, copy CR literally
            if ((*pcInput == '\r') && (*(pcInput + 1) == '\n'))
            {
               pcInput++;
               continue;  // do next loop
            } /* endif */

            // if input area doesn't contain CRLF (either skipped or original)
            // the character can be copied
            *(pcOutput++) = *(pcInput++);
            usOutputLen++;
         }
         else
         {
            // fMode is FALSE so convert LF to CRLF
            // do special handling only if current character is a LF and
            // previous character is not a CR
            if ((*pcInput == '\n') && (*(pcInput - 1) != '\r'))
            {
               *(pcOutput++) = '\r';
               usOutputLen++;
            } /* endif */

            // now copy the original character of the input string
            *(pcOutput++) = *(pcInput++);
            usOutputLen++;
         } /* endif */
      } /* endif */
   } /* endfor */

   return (usOutputLen);
} /* end of usConvertCRLF */

USHORT usConvertCRLFW(PCHAR_W pcInput,     // pointer to character input area
                      USHORT  usInputLen,  // length of input area
                      PCHAR_W pcOutput,    // pointer to output area
                      BOOL    fMode)       // conversion mode :
                                           //   TRUE -> convert to LF
                                           //   FALSE -> convert to CRLF

{
   USHORT   usOutputLen = 0,              // length of converted output
            usIndex;                      // index of input string

   for (usIndex = 0; usIndex < usInputLen; usIndex++)
   {
      if ((*pcInput != '\r') && (*pcInput != '\n'))
      {
         // no special handling required -> copy character and increase pointer
         *(pcOutput++) = *(pcInput++);
         usOutputLen++;
      }
      else
      {
         if (fMode)
         {
            // fMode is TRUE so convert CRLF to single LF
            // if input area contains CRLF, skip CR and copy next char
            // if input area contains only CR without LF, copy CR literally
            if ((*pcInput == '\r') && (*(pcInput + 1) == '\n'))
            {
               pcInput++;
               continue;  // do next loop
            } /* endif */

            // if input area doesn't contain CRLF (either skipped or original)
            // the character can be copied
            *(pcOutput++) = *(pcInput++);
            usOutputLen++;
         }
         else
         {
            // fMode is FALSE so convert LF to CRLF
            // do special handling only if current character is a LF and
            // previous character is not a CR
            if ((*pcInput == '\n') && (*(pcInput - 1) != '\r'))
            {
               *(pcOutput++) = '\r';
               usOutputLen++;
            } /* endif */

            // now copy the original character of the input string
            *(pcOutput++) = *(pcInput++);
            usOutputLen++;
         } /* endif */
      } /* endif */
   } /* endfor */

   return (usOutputLen);
} /* end of usConvertCRLF */

