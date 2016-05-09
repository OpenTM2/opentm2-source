//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |

//+----------------------------------------------------------------------------+
//|  Description:     Utilities for the filter functions                       |
//+----------------------------------------------------------------------------+
//|  Entry Points:                                                             |
//|                                                                            |
//|  FiltLoadFilterHeader   Load a filter header into memory                   |
//|  FiltRead               Read a filter into memory                          |
//|  FiltWrite              Write a filter to disk                             |
//|  FiltAddToBuffer        Add a string to a buffer                           |
//|  FiltStripBlanks        Remove leading and trailing blanks                 |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|  Externals:                                                                |
//+----------------------------------------------------------------------------+
//|  Internals:                                                                |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|  Include files:                                                            |
//+----------------------------------------------------------------------------+
//|  To be done / known limitations / caveats:                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| PVCS Section                                                               |
//
// $CMVC
// 
// $Revision: 1.1 $ ----------- 14 Dec 2009
//  -- New Release TM6.2.0!!
// 
// 
// $Revision: 1.1 $ ----------- 1 Oct 2009
//  -- New Release TM6.1.8!!
// 
// 
// $Revision: 1.1 $ ----------- 2 Jun 2009
//  -- New Release TM6.1.7!!
// 
// 
// $Revision: 1.1 $ ----------- 8 Dec 2008
//  -- New Release TM6.1.6!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Sep 2008
//  -- New Release TM6.1.5!!
// 
// 
// $Revision: 1.1 $ ----------- 23 Apr 2008
//  -- New Release TM6.1.4!!
// 
// 
// $Revision: 1.1 $ ----------- 13 Dec 2007
//  -- New Release TM6.1.3!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Aug 2007
//  -- New Release TM6.1.2!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Apr 2007
//  -- New Release TM6.1.1!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2006
//  -- New Release TM6.1.0!!
// 
// 
// $Revision: 1.1 $ ----------- 9 May 2006
//  -- New Release TM6.0.11!!
// 
// 
// $Revision: 1.1 $ ----------- 20 Dec 2005
//  -- New Release TM6.0.10!!
// 
// 
// $Revision: 1.1 $ ----------- 16 Sep 2005
//  -- New Release TM6.0.9!!
// 
// 
// $Revision: 1.1 $ ----------- 18 May 2005
//  -- New Release TM6.0.8!!
// 
// 
// $Revision: 1.1 $ ----------- 29 Nov 2004
//  -- New Release TM6.0.7!!
// 
// 
// $Revision: 1.1 $ ----------- 30 Aug 2004
//  -- New Release TM6.0.6!!
// 
// 
// $Revision: 1.1 $ ----------- 3 May 2004
//  -- New Release TM6.0.5!!
// 
// 
// $Revision: 1.1 $ ----------- 15 Dec 2003
//  -- New Release TM6.0.4!!
// 
// 
// $Revision: 1.1 $ ----------- 6 Oct 2003
//  -- New Release TM6.0.3!!
// 
// 
// $Revision: 1.1 $ ----------- 27 Jun 2003
//  -- New Release TM6.0.2!!
// 
// 
// $Revision: 1.3 $ ----------- 26 Feb 2003
// --RJ: removed compiler defines not needed any more and rework code to avoid warnings
// 
//
// $Revision: 1.2 $ ----------- 24 Feb 2003
// --RJ: delete obsolete code and remove (if possible)compiler warnings
//
//
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 3 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFFILTU.CV_   1.0   09 Jan 1996 09:06:28   BUILD  $
 *
 * $Log:   J:\DATA\EQFFILTU.CV_  $
 *
 *    Rev 1.0   09 Jan 1996 09:06:28   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_FILT             // dictionary filter functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqffilti.h"             // Filter private include file

#define ALLOC_AND_READ( length, ptr, size, used ) \
  {                                                                 \
    if ( length )                                                   \
    {                                                               \
      if ( size < length )                                            \
      {                                                               \
        UtlAlloc( (PVOID *) &ptr, (LONG)size, (LONG) max( MIN_ALLOC, length ), ERROR_STORAGE );  \
        size = max( MIN_ALLOC, length );                              \
      } /* endif */                                                   \
      if ( ptr )                                                      \
      {                                                               \
        used = length;                                                \
        usRC = UtlReadL( hFilter, ptr, length, &ulBytesRead, TRUE );   \
      }                                                               \
      else                                                            \
      {                                                               \
        usRC = ERROR_NOT_ENOUGH_MEMORY;                               \
      } /* endif */                                                   \
    }                                                                 \
    else                                                              \
    {                                                                 \
      ulBytesRead = 0;                                                \
      used = 0;                                                       \
      size = 0;                                                       \
      ptr = NULL;                                                     \
    } /* endif */                                                     \
  }

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltLoadFilterHeader   Load a filter header into memory  |
//+----------------------------------------------------------------------------+
//|Function call:     FiltLoadFilterHeader( PSZ pszName, PFILTPROP &pProp );   |
//+----------------------------------------------------------------------------+
//|Description:       Loads the header part of a filter property file into     |
//|                   memory.                                                  |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ          pszName  name of filter (w/o path and ext.) |
//|                   PFILTPROP    pProp    address of buffer for header data  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR           function completed successfully       |
//|                   other              return codes of Dos calls             |
//+----------------------------------------------------------------------------+
//|Prerequesits:      _                                                        |
//+----------------------------------------------------------------------------+
//|Side effects:      Area pointed to by pProp is filled with the header data  |
//|                   of the filter.                                           |
//+----------------------------------------------------------------------------+
//|Samples:           usRC = FiltLoadFilterHeader( "TCGFILT", &Props );        |
//+----------------------------------------------------------------------------+
//|Function flow:     build fully qualified filter file name                   |
//|                   open filter file                                         |
//|                   if ok read filter header                                 |
//|                   close filter file                                        |
//|                   return return code                                       |
//+----------------------------------------------------------------------------+
USHORT FiltLoadFilterHeader
(
  PSZ                pszName,          // name of filter (w/o path and ext.)
  PFILTPROP          pProp             // address of buffer for header data
)
{
  USHORT    usRC = NO_ERROR;           // function return code
  CHAR      szFilterPath[MAX_EQF_PATH];// buffer for filter path name
  HFILE     hFilter = NULLHANDLE;      // file handle for filter file
  USHORT    usAction;                  // action performed by DosOpen
  ULONG     ulBytesRead;               // number of bytes read from filter

  /********************************************************************/
  /* build fully qualified filter name                                */
  /********************************************************************/
  UtlMakeEQFPath( szFilterPath, NULC, PROPERTY_PATH, NULL );
  strcat( szFilterPath, BACKSLASH_STR );
  strcat( szFilterPath, pszName );
  strcat( szFilterPath, EXT_OF_FILTPROP );

  /********************************************************************/
  /* open the filter file                                             */
  /********************************************************************/
  usRC = UtlOpen( szFilterPath, &hFilter , &usAction,
                  0L, FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                  OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, 0L, FALSE );

  /********************************************************************/
  /* read filter header part                                          */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    usRC = UtlReadL( hFilter, pProp, sizeof(FILTPROP), &ulBytesRead, FALSE );
    if ( !usRC && (ulBytesRead != sizeof(FILTPROP)) )
    {
      usRC = ERROR_READ_FAULT;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Close filter file                                                */
  /********************************************************************/
  if ( hFilter )      UtlClose( hFilter, FALSE );

  /********************************************************************/
  /* return return code to calling function                           */
  /********************************************************************/
  return( usRC );

} /* end of function FiltLoadFilterHeader */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltRead               Read a filter into memory         |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = FiltRead( PSZ pszName, PFILTER pFilter );         |
//+----------------------------------------------------------------------------+
//|Description:       Physically reads a filter into memory.                   |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszName     fully qualified filter name         |
//|                   PFILTER  pFilter     address of filter structure         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR             if function completed OK            |
//|                   other                return codes from Dos... calls      |
//+----------------------------------------------------------------------------+
//|Function flow:     open the filter file                                     |
//|                   read filter header                                       |
//|                   allocate and read all names array                        |
//|                   allocate and read selected names array                   |
//|                   allocate and read where MLE data                         |
//|                   allocate and read polish stack                           |
//|                   allocate and read string buffer                          |
//|                   close filter file                                        |
//+----------------------------------------------------------------------------+
USHORT FiltRead
(
  PSZ      pszName,                    // fully qualified filter name
  PFILTER  pFilter                     // address of filter structure
)
{
  USHORT usRC = NO_ERROR;              // function return code
  HFILE  hFilter = NULLHANDLE;         // file handle of filter file
  USHORT usAction;                     // action performed by DosOpen
  ULONG  ulBytesRead;                  // number of bytes read from file

  /********************************************************************/
  /* Check existence of filter file                                   */
  /********************************************************************/
  if ( !UtlFileExist( pszName ) )
  {
    PSZ    pszFilter;
    CHAR   szFilter[MAX_FILESPEC];

    Utlstrccpy( szFilter, UtlGetFnameFromPath( pszName ), DOT );
    pszFilter = szFilter;
    UtlError( ERROR_FILT_DOESNOTEXIST, MB_CANCEL, 1, &pszFilter, EQF_ERROR );
    usRC = ERROR_FILE_NOT_FOUND;
  } /* endif */

  /********************************************************************/
  /* Open the filter file                                             */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    usRC = UtlOpen( pszName, &hFilter, &usAction,
                    0L, FILE_NORMAL, OPEN_ACTION_OPEN_IF_EXISTS,
                    OPEN_ACCESS_READONLY | OPEN_SHARE_DENYWRITE, 0L, TRUE );
  } /* endif */

  /********************************************************************/
  /* Read filter header                                               */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    usRC = UtlReadL( hFilter, &(pFilter->Prop), sizeof(FILTPROP),
                    &ulBytesRead, TRUE );
  } /* endif */

  /********************************************************************/
  /* Allocate and read filter data areas                              */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    ALLOC_AND_READ( pFilter->Prop.FiltAllNames.usLen,
                    pFilter->pusAllNames, pFilter->usAllNameSize,
                    pFilter->usAllNameUsed );
  } /* endif */
  if ( usRC == NO_ERROR )
  {
    ALLOC_AND_READ( pFilter->Prop.FiltSelectNames.usLen,
                    pFilter->pusSelNames, pFilter->usSelNameSize,
                    pFilter->usSelNameUsed );
  } /* endif */
  if ( usRC == NO_ERROR )
  {
    ALLOC_AND_READ( pFilter->Prop.FilterMle.usLen,
                    pFilter->pucWhereMLE, pFilter->usWhereMLESize,
                    pFilter->usWhereMLEUsed );
  } /* endif */
  if ( usRC == NO_ERROR )
  {
    ALLOC_AND_READ( pFilter->Prop.FilterStack.usLen,
                    pFilter->pStack, pFilter->usStackSize,
                    pFilter->usStackUsed );
  } /* endif */
  if ( usRC == NO_ERROR )
  {
    ALLOC_AND_READ( pFilter->Prop.FilterBuffer.usLen,
                    pFilter->pucBuffer, pFilter->usBufferSize,
                    pFilter->usBufferUsed );
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( hFilter )
  {
    UtlClose( hFilter, FALSE );
  } /* endif */

  /********************************************************************/
  /* Return to calling function                                       */
  /********************************************************************/
  return( usRC );

} /* end of function FiltRead */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltFree               Free all filter memory            |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = FiltFree( PFILTER pFilter );                      |
//+----------------------------------------------------------------------------+
//|Description:       Frees all memory allocated for a filter.                 |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszName     fully qualified filter name         |
//|                   PFILTER  pFilter     address of filter structure         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR             if function completed OK            |
//+----------------------------------------------------------------------------+
//|Function flow:     free all names array                                     |
//|                   free selected names array                                |
//|                   free where MLE data                                      |
//|                   free polish stack                                        |
//|                   free string buffer                                       |
//|                   free stack array                                         |
//|                   free selected names array                                |
//+----------------------------------------------------------------------------+
USHORT FiltFree
(
  PFILTER  pFilter                     // address of filter structure
)
{
  USHORT usRC = NO_ERROR;              // function return code
  USHORT usI;                          // loop index

  if ( pFilter->pusAllNames ) UtlAlloc( (PVOID *) &pFilter->pusAllNames, 0L, 0L, NOMSG );
  if ( pFilter->pusSelNames ) UtlAlloc( (PVOID *) &pFilter->pusSelNames, 0L, 0L, NOMSG );
  if ( pFilter->pucSelMLE )   UtlAlloc( (PVOID *) &pFilter->pucSelMLE,   0L, 0L, NOMSG );
  if ( pFilter->pucWhereMLE ) UtlAlloc( (PVOID *) &pFilter->pucWhereMLE, 0L, 0L, NOMSG );
  if ( pFilter->pStack )      UtlAlloc( (PVOID *) &pFilter->pStack,      0L, 0L, NOMSG );
  if ( pFilter->pucBuffer )   UtlAlloc( (PVOID *) &pFilter->pucBuffer,   0L, 0L, NOMSG );
  if ( pFilter->pusSelNames ) UtlAlloc( (PVOID *) &pFilter->pusSelNames, 0L, 0L, NOMSG );
  for ( usI = 0; usI < pFilter->usDicts; usI++ )
  {
    if ( pFilter->pOpStack[usI] )
    {
      UtlAlloc( (PVOID *) &(pFilter->pOpStack[usI]), 0L, 0L, NOMSG );
    } /* endif */
    if ( pFilter->pusSelected[usI] )
    {
      UtlAlloc( (PVOID *) &(pFilter->pusSelected[usI]), 0L, 0L, NOMSG );
    } /* endif */
  } /* endfor */

  return( usRC );

} /* end of function FiltFree */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltWrite              Write a filter to disk            |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = FiltWrite( PSZ pszName, PFILTER pFilter );        |
//+----------------------------------------------------------------------------+
//|Description:       Physically writes a filter to disk.                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ      pszName     fully qualified filter name         |
//|                   PFILTER  pFilter     address of filter structure         |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR             if function completed OK            |
//|                   other                return codes from Dos... calls      |
//+----------------------------------------------------------------------------+
//|Function flow:     open the filter file                                     |
//|                   setup filter property header                             |
//|                   write filter header                                      |
//|                   write all names array                                    |
//|                   write selected names array                               |
//|                   write where MLE buffer                                   |
//|                   write polish stack                                       |
//|                   write string buffer                                      |
//|                   close filter file                                        |
//+----------------------------------------------------------------------------+
USHORT FiltWrite
(
  PSZ      pszName,                    // fully qualified filter name
  PFILTER  pFilter                     // address of filter structure
)
{
  USHORT usRC = NO_ERROR;              // function return code
  HFILE  hFilter = NULLHANDLE;         // file handle of filter file
  USHORT usAction;                     // action performed by DosOpen
  USHORT  usBytesWritten;               // number of bytes read from file

  /********************************************************************/
  /* Open the filter file                                             */
  /********************************************************************/
  usRC = UtlOpen( pszName, &hFilter, &usAction,
                  0L, FILE_NORMAL, FILE_TRUNCATE | FILE_CREATE,
                  OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE, 0L, TRUE );

  /********************************************************************/
  /* Setup filter property header                                     */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    pFilter->Prop.FiltAllNames.usStart = sizeof(FILTPROP);
    pFilter->Prop.FiltAllNames.usLen   = pFilter->usAllNameUsed;

    pFilter->Prop.FiltSelectNames.usStart = pFilter->Prop.FiltAllNames.usStart +
                                            pFilter->Prop.FiltAllNames.usLen;
    pFilter->Prop.FiltSelectNames.usLen   = pFilter->usSelNameUsed;

    pFilter->Prop.FilterMle.usStart = pFilter->Prop.FiltSelectNames.usStart +
                                      pFilter->Prop.FiltSelectNames.usLen;
    pFilter->Prop.FilterMle.usLen   = pFilter->usWhereMLEUsed;

    pFilter->Prop.FilterStack.usStart = pFilter->Prop.FilterMle.usStart +
                                        pFilter->Prop.FilterMle.usLen;
    pFilter->Prop.FilterStack.usLen   = pFilter->usStackUsed;

    pFilter->Prop.FilterBuffer.usStart = pFilter->Prop.FilterStack.usStart +
                                         pFilter->Prop.FilterStack.usLen;
    pFilter->Prop.FilterBuffer.usLen   = pFilter->usBufferUsed;
  } /* endif */

  /********************************************************************/
  /* Write filter header                                              */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    usRC = UtlWrite( hFilter, &(pFilter->Prop), sizeof(FILTPROP),
                     &usBytesWritten, TRUE );
    if ( !usRC && (usBytesWritten != sizeof(FILTPROP)) )
    {
      usRC = ERROR_DISK_FULL;
      UtlError( usRC, MB_CANCEL, 1, &pszName, DOS_ERROR );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Write filter data areas                                          */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    usRC = UtlWrite( hFilter, pFilter->pusAllNames, pFilter->usAllNameUsed,
                     &usBytesWritten, TRUE );
    if ( !usRC && (usBytesWritten != pFilter->usAllNameUsed) )
    {
      usRC = ERROR_DISK_FULL;
      UtlError( usRC, MB_CANCEL, 1, &pszName, DOS_ERROR );
    } /* endif */
  } /* endif */

  if ( usRC == NO_ERROR )
  {
    usRC = UtlWrite( hFilter, pFilter->pusSelNames, pFilter->usSelNameUsed,
                     &usBytesWritten, TRUE );
    if ( !usRC && (usBytesWritten != pFilter->usSelNameUsed) )
    {
      usRC = ERROR_DISK_FULL;
    } /* endif */
  } /* endif */

  if ( usRC == NO_ERROR )
  {
    usRC = UtlWrite( hFilter, pFilter->pucWhereMLE, pFilter->usWhereMLEUsed,
                     &usBytesWritten, TRUE );
    if ( !usRC && (usBytesWritten != pFilter->usWhereMLEUsed) )
    {
      usRC = ERROR_DISK_FULL;
      UtlError( usRC, MB_CANCEL, 1, &pszName, DOS_ERROR );
    } /* endif */
  } /* endif */

  if ( usRC == NO_ERROR )
  {
    usRC = UtlWrite( hFilter, pFilter->pStack, pFilter->usStackUsed,
                     &usBytesWritten, TRUE );
    if ( !usRC && (usBytesWritten != pFilter->usStackUsed) )
    {
      usRC = ERROR_DISK_FULL;
      UtlError( usRC, MB_CANCEL, 1, &pszName, DOS_ERROR );
    } /* endif */
  } /* endif */

  if ( usRC == NO_ERROR )
  {
    usRC = UtlWrite( hFilter, pFilter->pucBuffer, pFilter->usBufferUsed,
                     &usBytesWritten, TRUE );
    if ( !usRC && (usBytesWritten != pFilter->usBufferUsed) )
    {
      usRC = ERROR_DISK_FULL;
      UtlError( usRC, MB_CANCEL, 1, &pszName, DOS_ERROR );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Cleanup                                                          */
  /********************************************************************/
  if ( hFilter )
  {
    UtlClose( hFilter, FALSE );
    if ( usRC != NO_ERROR )
    {
      UtlDelete( pszName, 0L, FALSE );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Return to calling function                                       */
  /********************************************************************/
  return( usRC );

} /* end of function FiltWrite */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FiltAddToBuffer     Add a string to a buffer             |
//+----------------------------------------------------------------------------+
//|Function call:     FiltAddToBuffer( PSZ *ppBuffer, PUSHORT pusUsed,         |
//|                                    PUSHORT  pusSize, PSZ pszString );      |
//+----------------------------------------------------------------------------+
//|Description:       Add the given string to the buffer and enlarge the       |
//|                   buffer if necessary.                                     |
//+----------------------------------------------------------------------------+
//|Input parameter:   PSZ     *ppBuffer        address of buffer pointer       |
//|                   PUSHORT pusUsed          address of used bytes variable  |
//|                   PUSHORT pusSize          address of size variable        |
//|                   PSZ     pszString        pointer to string being added.  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       NO_ERROR                 string has been added to buffer |
//|                   ERROR_NOT_ENOUGH_MEMORY  memory allocation failed        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      Buffer pointer, used variable and size variable must     |
//|                   have been initialized to zero before the first call      |
//|                   to FiltAddToBuffer.                                      |
//+----------------------------------------------------------------------------+
//|Side effects:      The buffer is enlarged if necessary                      |
//+----------------------------------------------------------------------------+
//|Function flow:     get string length                                        |
//|                   enlarge buffer if necessary                              |
//|                   add string to buffer                                     |
//+----------------------------------------------------------------------------+
USHORT FiltAddToBuffer
(
  PSZ     *ppBuffer,                   // address of buffer pointer
  PULONG  pulUsed,                     // address of used bytes variable
  PULONG  pulSize,                     // address of size variable
  PSZ     pszString                    // pointer to string being added.
)
{
  USHORT    usRC = NO_ERROR;           // function return code
  ULONG     ulLen;                     // length of string being added

  /********************************************************************/
  /* Get length of string being added                                 */
  /********************************************************************/
  ulLen = strlen( pszString ) + 1;

  /********************************************************************/
  /* Enlarge buffer if necessary                                      */
  /********************************************************************/
  if ( (*pulUsed + ulLen) > *pulSize )
  {
    if ( UtlAlloc( (PVOID *) ppBuffer, *pulSize,
                   (*pulSize+BUFFER_SIZE), ERROR_STORAGE ) )
    {
      *pulSize += BUFFER_SIZE;
    }
    else
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Add string to buffer                                             */
  /********************************************************************/
  if ( usRC == NO_ERROR )
  {
    memcpy( *ppBuffer + *pulUsed, pszString, ulLen );
    *pulUsed += ulLen;
  } /* endif */

  /********************************************************************/
  /* Return function return code                                      */
  /********************************************************************/
  return( usRC );
} /* end of function FiltAddToBuffer */

