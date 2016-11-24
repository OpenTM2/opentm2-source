//+----------------------------------------------------------------------------+
//|EQFNTMDB.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:   G.Jornitz                                                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:  TranslationMemory Layer for QDAM functions                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//| -- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
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
// $Revision: 1.3 $ ----------- 13 Jan 2005
// GQ: - removed compiler warnings
// 
// 
// $Revision: 1.2 $ ----------- 11 Jan 2005
// GQ: - added function EQFNTMOrganizeIndex to compact the index part of a memory
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
// $Revision: 1.3 $ ----------- 31 Oct 2001
//
//
// $Revision: 1.2 $ ----------- 22 Oct 2001
// -- RJ: get rid of compiler warnings
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 4 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFNTMDB.CV_   1.4   09 Nov 1998 09:39:38   BUILD  $
 *
 * $Log:   K:\DATA\EQFNTMDB.CV_  $
 *
 *    Rev 1.4   09 Nov 1998 09:39:38   BUILD
 * - continued work on new TM approach
 *
 *    Rev 1.3   29 Sep 1998 07:31:34   BUILD
 * - adapted to ULONG length functions in QDAM
 *
 *    Rev 1.2   20 Dec 1996 09:57:14   BUILD
 * -- KAT0254: correct plausibility check for key (use '>' instead of '>=')
 *
 *    Rev 1.1   30 Oct 1996 19:38:14   BUILD
 * - do not write event log for BTREE_NOT_FOUND condition in EQFNTMGet
 *
 *    Rev 1.0   09 Jan 1996 09:16:48   BUILD
 * Initial revision.
*/
// ----------------------------------------------------------------------------+

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_ASD
#define INCL_EQF_DAM
#include "EQF.H"
#include "EQFQDAMI.H"
#include "eqfevent.h"                  // event logging facility

UCHAR  ucbEncodeTbl[30]
        =  { 00,  06,
             97,  32, 101,  65, 116, 110, 105, 115, 114,  99, 111,
             14, 100, 108, 117, 104,  98, 103,  71, 102, 109, 112,  10,
             02,  03,  04,  05 };

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMCreate                                             |
//+----------------------------------------------------------------------------+
//|Function call:     EQFNTMCreate( pTMName, pUserData, usLen, ulStart, &pNTM);|
//+----------------------------------------------------------------------------+
//|Description:       This function will create the appropriate Transl.Memory  |
//|                   file.                                                    |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ  pTMName,         name of file to be created         |
//|                   PCHAR  pUserData,     user data                          |
//|                   USHORT usLen,         length of user data                |
//|                   ULONG  ulStartKey,    first key to start automatic insert|
//|                   PBTREE * ppBTIda      pointer to structure               |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                 no error happened                      |
//|                   BTREE_NO_ROOM     memory shortage                        |
//|                   BTREE_USERDATA    user data too long                     |
//|                   BTREE_OPEN_ERROR  dictionary already exists              |
//|                   BTREE_READ_ERROR  read error from disk                   |
//|                   BTREE_DISK_FULL   disk full condition encountered        |
//|                   BTREE_WRITE_ERROR write error to disk                    |
//+----------------------------------------------------------------------------+
//|Function flow:     use standard DictCreateLocal with some modified parts..  |
// ----------------------------------------------------------------------------+
SHORT
EQFNTMCreate
(
   PSZ  pTMName,                       // name of file to be created
   PCHAR  pUserData,                   // user data
   USHORT usLen,                       // length of user data
   ULONG  ulStartKey,                  // first key to start automatic insert...
   PVOID  * ppBTIda                    // pointer to structure
)
{
   SHORT sRc = 0;                      // return code
   PBTREE  pBTIda;
   NTMVITALINFO NtmVitalInfo;          // structure to contain vital info for TM

   NtmVitalInfo.ulStartKey = NtmVitalInfo.ulNextKey = ulStartKey;


  UtlAlloc( (PVOID *)&pBTIda, 0L, (LONG) sizeof(BTREE), NOMSG );
  if ( !pBTIda )
  {
    sRc = BTREE_NO_ROOM;
  }
  else
  {
    *ppBTIda = pBTIda;                     // set pointer to base structure

    /******************************************************************/
    /* move @@@@ line one below and you will get compression active   */
    /******************************************************************/
    sRc = QDAMDictCreateLocal( pTMName, 20, pUserData, usLen,
//@@@@                        ucbEncodeTbl,
                              NULL,
                              NULL, NULL, (PPBTREE) ppBTIda,
                              &NtmVitalInfo );
    if ( sRc == BTREE_OPEN_ERROR )
    {
       UtlAlloc( (PVOID *)&(pBTIda->pBTree), 0L, 0L, NOMSG );
       UtlAlloc( (PVOID *)ppBTIda, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  return( sRc );
} /* end of function EQFNTMCreate */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMOpen     Open a Translation Memory file            |
//+----------------------------------------------------------------------------+
//|Function call:     EQFNTMOpen( PSZ, USHORT, PPBTREE );                      |
//+----------------------------------------------------------------------------+
//|Description:       Open a file locally for processing                       |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ              name of the index file                  |
//|                   USHORT           open flags                              |
//|                   PPBTREE          pointer to btree structure              |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                 no error happened                      |
//|                   BTREE_NO_ROOM     memory shortage                        |
//|                   BTREE_OPEN_ERROR  dictionary already exists              |
//|                   BTREE_READ_ERROR  read error from disk                   |
//|                   BTREE_DISK_FULL   disk full condition encountered        |
//|                   BTREE_WRITE_ERROR write error to disk                    |
//|                   BTREE_ILLEGAL_FILE not a valid dictionary                |
//|                   BTREE_CORRUPTED   dictionary is corrupted                |
//+----------------------------------------------------------------------------+
//|Function flow:     Open the file read/write or readonly                     |
//|                   if ok                                                    |
//|                     read in header                                         |
//|                     if ok                                                  |
//|                       check for validity of the header                     |
//|                       if ok                                                |
//|                         fill tree structure                                |
//|                         call UtlQFileInfo to get the next free record      |
//|                         if ok                                              |
//|                           allocate buffer space                            |
//|                           if alloc fails                                   |
//|                             set Rc = BTREE_NO_ROOM                         |
//|                           endif                                            |
//|                         endif                                              |
//|                       endif                                                |
//|                     else                                                   |
//|                       set Rc = BTREE_READ_ERROR                            |
//|                     endif                                                  |
//|                     if error                                               |
//|                       close dictionary                                     |
//|                     endif                                                  |
//|                     if read/write open was done                            |
//|                       set open flag and write it into file                 |
//|                     endif                                                  |
//|                     get corruption flag and set rc if nec.                 |
//|                   else                                                     |
//|                    set Rc = BTREE_OPEN_ERROR                               |
//|                   endif                                                    |
//|                   return Rc                                                |
// ----------------------------------------------------------------------------+

SHORT  EQFNTMOpen
(
  PSZ   pName,                        // name of the file
  USHORT usOpenFlags,                 // Read Only or Read/Write
  PVOID  * ppBTIda                    // pointer to BTREE structure
)
{
   PBTREE    pBTIda;                   // pointer to BTRee structure
   SHORT     sRc = 0;                  // return code

   DEBUGEVENT( EQFNTMOPEN_LOC, FUNCENTRY_EVENT, 0 );

   if ( ! UtlAlloc( (PVOID *)&pBTIda, 0L , (LONG) sizeof( BTREE ), NOMSG )  )
   {
      sRc = BTREE_NO_ROOM;
   }
   else
   {
     /*****************************************************************/
     /* check if same dictionary is already open else return index    */
     /* of next free slot...                                          */
     /*****************************************************************/
     sRc = QDAMCheckDict( pName, pBTIda );
     if ( !sRc )
     {
       /***************************************************************/
       /* Set shared flag for shared TMs (only if TM is not opened    */
       /* exclusively)                                                */
       /***************************************************************/
       if ( !(usOpenFlags & ASD_LOCKED) )
       {
         PSZ   pszExt;

         pszExt = strrchr( pName, DOT );
         if ( (pszExt != NULL) &&
              ( (strcmp( pszExt, EXT_OF_SHARED_MEM ) == 0) ||
                (strcmp( pszExt, EXT_OF_SHARED_MEMINDEX ) == 0) ) )
         {
           usOpenFlags |= ASD_SHARED;
         } /* endif */
       } /* endif */

       /***************************************************************/
       /* check if dictionary is locked                               */
       /***************************************************************/
       if ( ! pBTIda->usDictNum )
       {
         SHORT  RetryCount;                  // retry counter for in-use condition
         RetryCount = MAX_RETRY_COUNT;
         do
         {
           sRc = QDAMDictOpenLocal( pName, 20, usOpenFlags, &pBTIda );
           if ( sRc == BTREE_IN_USE )
           {
             RetryCount--;
             UtlWait( MAX_WAIT_TIME );
             if ( RetryCount > 0 )
             {
               /*******************************************************/
               /* re-allocate PBTIDA (has been de-allocated within    */
               /* QDAMDictOpenLocal due to error code)                */
               /*******************************************************/
               if ( ! UtlAlloc( (PVOID *)&pBTIda, 0L , (LONG) sizeof( BTREE ), NOMSG )  )
               {
                  sRc = BTREE_NO_ROOM;
               } /* endif */
             } /* endif */
           } /* endif */
         } while ( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
                   (RetryCount > 0) ); /* enddo */
       }
       else
       {
          sRc = ( usOpenFlags & ASD_LOCKED ) ? BTREE_DICT_LOCKED : sRc;
       } /* endif */
     } /* endif */
   } /* endif */

   *ppBTIda = pBTIda;                               // set base pointer

   if ( sRc != NO_ERROR )
   {
     ERREVENT( EQFNTMOPEN_LOC, ERROR_EVENT, sRc );
   } /* endif */

   DEBUGEVENT( EQFNTMOPEN_LOC, FUNCEXIT_EVENT, 0 );

   return ( sRc );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMClose  close the TM file                           |
//+----------------------------------------------------------------------------+
//|Function call:     EQFNTMClose( PPBTREE );                                  |
//+----------------------------------------------------------------------------+
//|Description:       Close the file                                           |
//+----------------------------------------------------------------------------+
//|Parameters:        PPBTREE                pointer to btree structure        |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                 no error happened                      |
//|                   BTREE_INVALID     incorrect pointer                      |
//|                   BTREE_DISK_FULL   disk full condition encountered        |
//|                   BTREE_WRITE_ERROR write error to disk                    |
//|                   BTREE_CORRUPTED   dictionary is corrupted                |
//|                   BTREE_CLOSE_ERROR error closing dictionary               |
//+----------------------------------------------------------------------------+
//|Function flow:     call QDAMDictCloseLocal routine ...                      |
// ----------------------------------------------------------------------------+

SHORT EQFNTMClose
(
  PVOID * ppBTIda
)
{
  SHORT sRc;

  DEBUGEVENT( EQFNTMCLOSE_LOC, FUNCENTRY_EVENT, 0 );

  if ( *ppBTIda )
  {
    sRc = QDAMDictCloseLocal( (PBTREE) *ppBTIda );
  }
  else
  {
    sRc = BTREE_INVALID;
  } /* endif */

  if ( !sRc )
  {
    UtlAlloc( ppBTIda, 0L, 0L, NOMSG );
  } /* endif */

  if ( sRc != NO_ERROR )
  {
    ERREVENT( EQFNTMCLOSE_LOC, ERROR_EVENT, sRc );
  } /* endif */

  DEBUGEVENT( EQFNTMCLOSE_LOC, FUNCEXIT_EVENT, 0 );

  return sRc;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMUpdSign  Write User Data                           |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMDictUpdSignLocal( PBTREE, PCHAR, USHORT );           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       Writes the second part of the first record (user data)   |
//|                   This is done using the original QDAMDictUpdSignLocal     |
//|                   function                                                 |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to btree structure        |
//|                   PCHAR                  pointer to user data              |
//|                   USHORT                 length of user data               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                 no error happened                      |
//|                   BTREE_DISK_FULL   disk full condition encountered        |
//|                   BTREE_WRITE_ERROR write error to disk                    |
//|                   BTREE_INVALID     pointer invalid                        |
//|                   BTREE_USERDATA    user data too long                     |
//|                   BTREE_CORRUPTED   dictionary is corrupted                |
//+----------------------------------------------------------------------------+
//|NOTE:              This function could be implemented as MACRO too, but     |
//|                   for consistency reasons, the little overhead was used... |
//+----------------------------------------------------------------------------+
//|Function flow:     call QDAMDictUpdSignLocal ...                            |
// ----------------------------------------------------------------------------+

SHORT EQFNTMUpdSign
(
   PVOID  pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   USHORT usLen                        // length of user data
)
{
  return( QDAMDictUpdSignLocal( (PBTREE) pBTIda, pUserData, usLen ) );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMSign      Read signature record                    |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMDictSignLocal( PBTREE, PCHAR, PUSHORT );             |
//+----------------------------------------------------------------------------+
//|Description:       Gets the second part of the first record ( user data )   |
//|                   This is done using the original QDAMDictSignLocal func.  |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to btree structure        |
//|                   PCHAR                  pointer to user data              |
//|                   PUSHORT                length of user data area (input)  |
//|                                          filled length (output)            |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                 no error happened                      |
//|                   BTREE_INVALID     pointer invalid                        |
//|                   BTREE_USERDATA    user data too long                     |
//|                   BTREE_NO_BUFFER   no buffer free                         |
//|                   BTREE_READ_ERROR  read error from disk                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:      return signature record even if dictionary is corrupted  |
//+----------------------------------------------------------------------------+
//|NOTE:              This function could be implemented as MACRO too, but     |
//|                   for consistency reasons, the little overhead was used... |
//+----------------------------------------------------------------------------+
//|Function flow:     call QDAMDictSignLocal ....                              |
// ----------------------------------------------------------------------------+

SHORT EQFNTMSign
(
   PVOID  pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   PUSHORT pusLen                      // length of user data
)
{
  SHORT sRc;                           // function return code
  SHORT RetryCount;                    // retry counter for in-use condition

  RetryCount = MAX_RETRY_COUNT;
  do
  {
    sRc = QDAMDictSignLocal( (PBTREE) pBTIda, pUserData, pusLen );
    if ( sRc == BTREE_IN_USE )
    {
      RetryCount--;
      UtlWait( MAX_WAIT_TIME );
    } /* endif */
  } while ( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
            (RetryCount > 0) ); /* enddo */

  if ( sRc != NO_ERROR )
  {
    ERREVENT( EQFNTMSIGN_LOC, ERROR_EVENT, sRc );
  } /* endif */


  return( sRc );

}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMInsert                                             |
//+----------------------------------------------------------------------------+
//|Function call:     sRc = EQFNTMInsert( pBTIda, &ulKey, pData, usLen );      |
//+----------------------------------------------------------------------------+
//|Description:       insert a new key (ULONG) with data                       |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE  pBTIda,      pointer to binary tree struct       |
//|                   PULONG  pulKey,      pointer to key                      |
//|                   PBYTE   pData,       pointer to user data                |
//|                   USHORT  usLen        length of user data                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       BTREE_NUMBER_RANGE   requested key not in allowed range  |
//|                   BTREE_READONLY       file is opened read only - no write |
//|                   BTREE_CORRUPTED      file is corrupted                   |
//|                   errors returned by QDAMDictInsertLocal                   |
//|                   0                    success indicator                   |
//+----------------------------------------------------------------------------+
//|Function flow:     check for corruption and open in correct mode            |
//|                   if okay                                                  |
//|                     either check passed key or assign new key              |
//|                                    and update the header                   |
//|                   endif                                                    |
//|                   if okay                                                  |
//|                     call QDAMDictInsertLocal                               |
//|                   endif                                                    |
//|                   return success indicator..                               |
// ----------------------------------------------------------------------------+
SHORT
EQFNTMInsert
(
  PVOID   pBTIda,           // pointer to binary tree struct
  PULONG  pulKey,           // pointer to key
  PBYTE   pData,            // pointer to user data
  ULONG   ulLen             // length of user data
)
{
   SHORT         sRc = 0;   // return code
   PBTREEGLOB    pBT = NULL;

   DEBUGEVENT( EQFNTMINSERT_LOC, FUNCENTRY_EVENT, 0 );

   /*******************************************************************/
   /* validate passed pointer ...                                     */
   /*******************************************************************/
   if ( !pBTIda )
   {
     sRc = BTREE_INVALID;
   }
   else
   {
     pBT = ((PBTREE)pBTIda)->pBTree;
   } /* endif */

  /********************************************************************/
  /* do initial security checking...                                  */
  /********************************************************************/
   if ( !sRc && pBT->fCorrupted )
   {
      sRc = BTREE_CORRUPTED;
   } /* endif */
   if ( !sRc && !pBT->fOpen )
   {
     sRc = BTREE_READONLY;
   } /* endif */
  /********************************************************************/
  /* if user wants that we find an appropriate key, we have to do so..*/
  /********************************************************************/
  if ( !sRc )
  {
    if ( *pulKey == NTMREQUESTNEWKEY )
    {
      /******************************************************************/
      /* find next free key and anchor new value in file ...            */
      /******************************************************************/
      ULONG  ulKey;
      ulKey = *pulKey = (NTMNEXTKEY( pBT ))++;
      if ( ulKey > 0xFFFFFF )
      {
        sRc = BTREE_NUMBER_RANGE;
      }
      else
      {
        /**************************************************************/
        /* force update of header (only from time to time to avoid    */
        /* too much performance degration)...                         */
        /**************************************************************/
        if ( (ulKey & 0x020) || (pBT->usOpenFlags & ASD_SHARED) )
        {
          sRc = QDAMWriteHeader( (PBTREE)pBTIda );
          pBT->fUpdated = TRUE;
        } /* endif */
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* check if key is in valid range ...                           */
      /****************************************************************/
      if ( *pulKey > NTMSTARTKEY( pBT ) )
      {
        sRc = BTREE_NUMBER_RANGE;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* call QDAMDictInsert to do the dirty work of inserting entry...   */
  /********************************************************************/
  if ( !sRc )
  {
    SHORT  RetryCount;                  // retry counter for in-use condition
    RetryCount = MAX_RETRY_COUNT;
    do
    {
      sRc = QDAMDictInsertLocal( (PBTREE) pBTIda, (PSZ_W) pulKey, pData, ulLen );
      if ( sRc == BTREE_IN_USE )
      {
        RetryCount--;
        UtlWait( MAX_WAIT_TIME );
      } /* endif */
    } while ( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (RetryCount > 0) ); /* enddo */
  } /* endif */

  if ( sRc )
  {
    ERREVENT( EQFNTMINSERT_LOC, INTFUNCFAILED_EVENT, sRc );
  } /* endif */

  return sRc;
} /* end of function EQFNTMInsert */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMUpdate                                             |
//+----------------------------------------------------------------------------+
//|Function call:     sRc = EQFNTMUpdate( pBTIda,  ulKey, pData, usLen );      |
//+----------------------------------------------------------------------------+
//|Description:       update the data of an already inserted key               |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE  pBTIda,      pointer to binary tree struct       |
//|                   ULONG   ulKey,      key value                            |
//|                   PBYTE   pData,       pointer to user data                |
//|                   USHORT  usLen        length of user data                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       BTREE_NUMBER_RANGE   requested key not in allowed range  |
//|                   BTREE_READONLY       file is opened read only - no write |
//|                   BTREE_CORRUPTED      file is corrupted                   |
//|                   errors returned by QDAMDictInsertLocal                   |
//|                   0                    success indicator                   |
//+----------------------------------------------------------------------------+
//|Function flow:     call QDAMDictUpdateLocal                                 |
//|                   return success indicator..                               |
// ----------------------------------------------------------------------------+
SHORT
EQFNTMUpdate
(
  PVOID   pBTIda,           // pointer to binary tree struct
  ULONG   ulKey,            // key value
  PBYTE   pData,            // pointer to user data
  ULONG   ulLen             // length of user data
)
{
  SHORT  sRc = 0;           // success indicator

  DEBUGEVENT( EQFNTMUPDATE_LOC, FUNCENTRY_EVENT, 0 );

  if ( pBTIda )
  {
    SHORT  RetryCount;                  // retry counter for in-use condition
    RetryCount = MAX_RETRY_COUNT;
    do
    {
      sRc = QDAMDictUpdateLocal( (PBTREE)pBTIda, (PSZ_W) &ulKey, pData, ulLen );
      if ( sRc == BTREE_IN_USE )
      {
        RetryCount--;
        UtlWait( MAX_WAIT_TIME );
      } /* endif */
    } while ( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (RetryCount > 0) ); /* enddo */
  }
  else
  {
    sRc = BTREE_INVALID;
  } /* endif */

  if ( sRc )
  {
    ERREVENT( EQFNTMUPDATE_LOC, INTFUNCFAILED_EVENT, sRc );
  } /* endif */


  return sRc;
} /* end of function EQFNTMUpdate */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMGet                                                |
//+----------------------------------------------------------------------------+
//|Function call:     sRc = EQFNTMGet( pBTIda, ulKey, chData, &usLen );        |
//+----------------------------------------------------------------------------+
//|Description:       get the data string for the passed key                   |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE pBTIda,       pointer to btree struct             |
//|                   ULONG  ulKey,        key to be searched for              |
//|                   PCHAR  pchBuffer,    space for user data                 |
//|                   PUSHORT pusLength    in/out length of returned user data |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       same as for QDAMDictExactLocal...                        |
//+----------------------------------------------------------------------------+
//|Function flow:     call QDAMDictExactLocal to retrieve the data             |
// ----------------------------------------------------------------------------+
SHORT
EQFNTMGet
(
   PVOID  pBTIda,                      // pointer to btree struct
   ULONG  ulKey,                       // key to be searched for
   PCHAR  pchBuffer,                   // space for user data
   PULONG pulLength                    // in/out length of returned user data
)
{
  SHORT sRc;                           // return code
  ULONG ulLength;

  DEBUGEVENT( EQFNTMGET_LOC, FUNCENTRY_EVENT, 0 );

  CHECKPBTREE( ((PBTREE)pBTIda), sRc );
  if ( !sRc )
  {
    SHORT  RetryCount;                  // retry counter for in-use condition
    RetryCount = MAX_RETRY_COUNT;
    do
    {
      /******************************************************************/
      /* disable corruption flag to allow get of data in case memory    */
      /* is corrupted                                                   */
      /******************************************************************/
      PBTREEGLOB    pBT = ((PBTREE)pBTIda)->pBTree;
      BOOL          fCorrupted = pBT->fCorrupted;

      ulLength = *pulLength;
      pBT->fCorrupted = FALSE;
      sRc = QDAMDictExactLocal( (PBTREE) pBTIda,(PSZ_W) &ulKey, (PBYTE)pchBuffer, &ulLength, FEXACT );
      pBT->fCorrupted = fCorrupted;

      if ( sRc == BTREE_IN_USE )
      {
        RetryCount--;
        UtlWait( MAX_WAIT_TIME );
      } /* endif */
    } while ( ((sRc == BTREE_IN_USE) || (sRc == BTREE_INVALIDATED)) &&
              (RetryCount > 0) ); /* enddo */
    *pulLength = ulLength;
  } /* endif */

  if ( sRc && (sRc != BTREE_NOT_FOUND) )
  {
    ERREVENT( EQFNTMGET_LOC, INTFUNCFAILED_EVENT, sRc );
  } /* endif */


  return sRc;
} /* end of function EQFNTMGet */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMGetMaxNumber                                       |
//+----------------------------------------------------------------------------+
//|Function call:     sRc = EQFNTMGetNextNumber( pBTIda, &ulKey, &ulNextFree );|
//+----------------------------------------------------------------------------+
//|Description:       get the start key and the next free key ...              |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE pBTIda,       pointer to btree struct             |
//|                   PULONG pulStartKey   first key                           |
//|                   PULONG pulNextKey    next key to be assigned             |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0     always                                             |
//+----------------------------------------------------------------------------+
//|Function flow:     access data from internal structure                      |
// ----------------------------------------------------------------------------+
SHORT
EQFNTMGetNextNumber
(
   PVOID  pBTIda,                      // pointer to btree struct
   PULONG pulStartKey,                 // return start key number
   PULONG pulNextKey                   // return next key data
)
{
  NTMVITALINFO ntmVitalInfo;

  memcpy( &ntmVitalInfo, ((PBTREE)pBTIda)->pBTree->chCollate,
          sizeof(NTMVITALINFO));
  *pulStartKey = ntmVitalInfo.ulStartKey;
  *pulNextKey  = ntmVitalInfo.ulNextKey;

  return 0;
} /* end of function EQFNTMGetNextNumber */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMPhysLock                                           |
//+----------------------------------------------------------------------------+
//|Function call:     sRc = EQFNTMPhysLock( pBTIda );                          |
//+----------------------------------------------------------------------------+
//|Description:       Physicall lock or unlock database.                       |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE             The database to be locked             |
//|                   BOOL               TRUE = LOCK, FALSE = Unlock           |
//|                   PBOOL              ptr to locked flag (set to TRUE if    |
//|                                      locking was successful                |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
// ----------------------------------------------------------------------------+
SHORT EQFNTMPhysLock
(
   PVOID          pBTIda,
   BOOL           fLock,
   PBOOL          pfLocked
)
{
  SHORT       sRc = 0;                 // function return code

  DEBUGEVENT( EQFNTMPHYSLOCK_LOC, FUNCENTRY_EVENT, 0 );

  if ( pBTIda )
  {
    sRc = QDAMPhysLock( (PBTREE)pBTIda, fLock, pfLocked );
  }
  else
  {
    sRc = BTREE_INVALID;
  } /* endif */

  if ( sRc )
  {
    ERREVENT( EQFNTMPHYSLOCK_LOC, INTFUNCFAILED_EVENT, sRc );
  } /* endif */


  return sRc;
} /* end of function EQFNTMPhysLock */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDAMIncrUpdCounter      Inrement database update counter |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMIncrUpdCounter( PBTREE, SHORT sIndex )               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       Update one of the update counter field in the dummy      |
//|                   /locked terms file                                       |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to btree structure        |
//|                   SHORT                  index of counter field            |
//|                                                                      PLONG                                                                  ptr to buffer for new counte value|
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                 no error happened                      |
//|                   BTREE_DISK_FULL   disk full condition encountered        |
//|                   BTREE_WRITE_ERROR write error to disk                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:     read update counter from dummy file                      |
//|                   increment update counter                                 |
//|                   position ptr to begin of file                            |
//|                   write update counter to disk                             |
// ----------------------------------------------------------------------------+
SHORT EQFNTMIncrUpdCounter
(
   PVOID      pBTIda,                  // pointer to btree structure
   SHORT      sIndex,                  // index of update counter
   PLONG                         plNewValue                                               // ptr to buffer for new counte value|
)
{
  SHORT       sRc = 0;                 // function return code

  if ( pBTIda )
  {
    sRc = QDAMIncrUpdCounter( (PBTREE)pBTIda, sIndex, plNewValue );
  }
  else
  {
    sRc = BTREE_INVALID;
  } /* endif */

  return sRc;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     EQFNTMGetUpdCounter       Get database update counter    |
//+----------------------------------------------------------------------------+
//|Function call:     EQFNTMGetUpdCounter( PBTREE, PLONG, SHORT, SHORT );        |
//+----------------------------------------------------------------------------+
//|Description:       Get one or more of the the database update counters      |
//|                   from the dummy/locked terms file                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to btree structure        |
//|                   PLONG                  ptr to buffer for update counter  |
//|                   SHORT                  index of requested update counter |
//|                   SHORT                  number of counters requested      |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
// ----------------------------------------------------------------------------+
SHORT EQFNTMGetUpdCounter
(
   PVOID      pBTIda,                   // pointer to btree structure
   PLONG      plUpdCount,               // ptr to buffer for update counter
   SHORT      sIndex,                   // index of requested update counter
   SHORT      sNumCounters              // number of counters requested
)
{
  SHORT       sRc = 0;                 // function return code

  if ( pBTIda )
  {
    sRc = QDAMGetUpdCounter( (PBTREE)pBTIda, plUpdCount, sIndex, sNumCounters );
  }
  else
  {
    sRc = BTREE_INVALID;
  } /* endif */

  return sRc; 
} /* end of function EQFNTGetUpdCounter */


// function EQFNTMOrganizeIndex
//
// re-organizes the index part of a memory by seuentially writing the
// index records into a new file
//
USHORT EQFNTMOrganizeIndex
(
   PVOID          *ppBTIda,            // ptr to BTREE being organized
   USHORT         usOpenFlags,         // open flags to be used for index file
   ULONG          ulStartKey           // first key to start automatic insert...
)
{
  SHORT          sRc = 0;              // function return code
  PBTREE         pbTree = (PBTREE)(*ppBTIda);
  PCHAR_W        pchKeyBuffer = NULL;  // buffer for record keys
  ULONG          ulKeyBufSize = 0;     // current size of key buffer (number of characters)
  PBYTE          pbData = NULL;        // buffer for record data
  ULONG          ulDataBufSize = 0;    // current size of record data buffer (number of bytes)
  BOOL           fNewIndexCreated = FALSE; // new-index-has-been-created flag
  CHAR           szNewIndex[MAX_LONGPATH]; // buffer for new index name
  PBTREE         pBtreeOut = NULL;     // structure for output BTREE
  USHORT         usSigLen = 0;         // length of signature record
  ULONG          ulKey;

  // allocate buffer areas
  ulKeyBufSize = 256;
  if ( !UtlAlloc( (PVOID *)&pchKeyBuffer, 0, ulKeyBufSize*sizeof(CHAR_W) , ERROR_STORAGE ) ) 
  {
    sRc = ERROR_NOT_ENOUGH_MEMORY;
  } /* endif */

  if ( !sRc )
  {
    ulDataBufSize = MAX_INDEX_LEN * sizeof(LONG) * 4;    
    if ( !UtlAlloc( (PVOID *)&pbData, 0, ulDataBufSize, ERROR_STORAGE ) ) 
    {
      sRc = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  if ( !sRc )
  {
    if ( !UtlAlloc( (PVOID *)&pBtreeOut, 0, sizeof(BTREE), ERROR_STORAGE ) ) 
    {
      sRc = ERROR_NOT_ENOUGH_MEMORY;
    } /* endif */
  } /* endif */

  // get user data (signature record)
  if ( !sRc )
  {
    usSigLen = (USHORT)ulDataBufSize;
    sRc = QDAMDictSignLocal( pbTree, (PCHAR)pbData, &usSigLen );
  } /* endif */

  // check if index is empty
  if ( !sRc )
  {
    ULONG ulDataLen = ulDataBufSize; 
    ULONG ulKeyLen  = sizeof(ULONG) + 1; // ulKeyBufSize;

    sRc = QDAMDictFirstLocal( pbTree, (PCHAR_W)&ulKey, &ulKeyLen, pbData, &ulDataLen );
  } /* endif */

  // setup name for new index file
  if ( !sRc )
  {
    PSZ pszExt;

    strcpy( szNewIndex, pbTree->szFileName );
    pszExt = strrchr( szNewIndex, DOT );
    if ( pszExt )
    {
      strcpy( pszExt, ".XMI" );
    }
    else
    {
      strcat( pszExt, ".XMI" );
    } /* endif */
  } /* endif */

  // create new index file
  if ( !sRc )
  {
    NTMVITALINFO NtmVitalInfo;          // structure to contain vital info for TM

    memset( &NtmVitalInfo, 0, sizeof(NtmVitalInfo) );

    NtmVitalInfo.ulStartKey = NtmVitalInfo.ulNextKey = ulStartKey;
    sRc = QDAMDictCreateLocal( szNewIndex, 20, (PCHAR)pbData, usSigLen,
                                NULL, NULL, NULL, &pBtreeOut, &NtmVitalInfo );
    if ( !sRc )
    {
      fNewIndexCreated = TRUE;
    } /* endif */
  } /* endif */

  // write all data from existing index to new index file
  if ( !sRc )
  {
    ULONG ulDataLen = ulDataBufSize; 
    ULONG ulKeyLen  = sizeof(ULONG) + 1; // ulKeyBufSize;

    sRc = QDAMDictFirstLocal( pbTree, (PCHAR_W)&ulKey, &ulKeyLen, pbData, &ulDataLen );

    while ( !sRc )
    {
      sRc = QDAMDictInsertLocal( pBtreeOut, (PCHAR_W)&ulKey, pbData, ulDataLen );

      if ( !sRc )
      {
        ulDataLen = ulDataBufSize; 
        ulKeyLen  = sizeof(ULONG) + 1; // ulKeyBufSize;

        sRc = QDAMDictNextLocal( pbTree, (PCHAR_W)&ulKey, &ulKeyLen, pbData, &ulDataLen );
      } /* endif */
    } /*endwhile */

    if ( sRc == BTREE_EOF_REACHED )
    {
      sRc = 0;
    } /* endif */
  } /* endif */

  // close dictionaries
  if ( !sRc ) QDAMDictCloseLocal( pbTree );
  if ( fNewIndexCreated )QDAMDictCloseLocal( pBtreeOut );

  // replace index file with newly created one or discard new index file in case of errors
  if ( !sRc )
  {
    CHAR szOldIndex[MAX_LONGPATH];

    // replace old index file
    strcpy( szOldIndex, pbTree->szFileName );
    UtlDelete( szOldIndex, 0L, NOMSG );
    UtlMove( szNewIndex, szOldIndex, 0L, NOMSG );

    // re-open new index file 
    UtlAlloc( ppBTIda, 0, 0, NOMSG );
    sRc = EQFNTMOpen( szOldIndex, usOpenFlags, ppBTIda );
  }
  else if ( fNewIndexCreated )
  {
    // discard new index file (if created)
    UtlDelete( szNewIndex, 0L, NOMSG );
  } /* endif */

  // cleanup
  if ( pchKeyBuffer ) UtlAlloc( (PVOID *)&pchKeyBuffer, 0, 0, NOMSG );
  if ( pbData ) UtlAlloc( (PVOID *)&pbData, 0, 0, NOMSG );
  if ( pBtreeOut ) UtlAlloc( (PVOID *)pBtreeOut, 0, 0, NOMSG );

  // re-map some return codes..
  switch ( sRc )
  {
    case BTREE_EMPTY:
      sRc = 0;
      break;
    default:
      break;
  } /*endswitch */

  return( (USHORT)sRc );
} /* end of function EQFNTMOrganizeIndex */
