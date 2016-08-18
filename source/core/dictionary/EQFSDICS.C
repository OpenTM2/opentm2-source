// *************************** Prolog *********************************
//
//               Copyright (C) 1990-2012, International Business Machines      
//               Corporation and others. All rights reserved         
//
//  Short description: Remote dictionary server code
//
//  Author:
//
//  Description:       This file includes the procedures that run
//                     on the server.
//
// *********************** End Prolog *********************************

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#include <eqf.h>                  // General Translation Manager include file

#include "eqfqdami.h"             // QDAM private header file
#include <eqfsetup.h>

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     GetDictList                                              |
//+----------------------------------------------------------------------------+
//|Function call:     GetDictList( pDictListOut )                              |
//+----------------------------------------------------------------------------+
//|Description:       Returns list of dictionaries situated on selected server |
//+----------------------------------------------------------------------------+
//|Input parameter:   PFILE_LIST_OUT )                                         |
//|Parameters:                                                                 |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     Get server system drive                                  |
//|                   Build up remote dictionary property file                 |
//|                   Loop through all remote dictionary property files        |
//|                   and add dictionary name to output structure              |
//+----------------------------------------------------------------------------+

__declspec(dllexport)
USHORT GetFilePart( PGETDICTPART_IN  pGetPartIn,
                    PGETPART_OUT  pGetPartOut )
{
   USHORT   usRc = NO_ERROR;                //return code from DosRead
   ULONG    ulNewPointer;                   //new pointer location
   PBTREEGLOB   pBTreeGlob;
   PBTREE   pBTreeStruct;

   pBTreeStruct = (PBTREE)pGetPartIn->pBTree;
   pBTreeGlob = pBTreeStruct->pBTree;

   //--- set the file pointer to the value passed in pGetPartIn->ulFilePos
   usRc = UtlChgFilePtr( pBTreeGlob->fp, pGetPartIn->ulFilePos,
                         FILE_BEGIN, &ulNewPointer, FALSE );

   //--- if requested bytes to read are higher than the defined maximum
   if ( pGetPartIn->ulBytesToRead > GETPART_BUFFER_SIZE )
   {
      //--- set bytes to read to defined maximum
      pGetPartIn->ulBytesToRead = GETPART_BUFFER_SIZE;
   }/*endif*/

   if ( !usRc )                             //no error until now
   {
      usRc = UtlReadL( pBTreeGlob->fp, pGetPartOut->aucOutBuffer,
                      pGetPartIn->ulBytesToRead,
                      &pGetPartOut->ulBytesRead, FALSE );
   }/*endif*/

   if ( !usRc )          //no error until now
   {
      //--- set new file position
      pGetPartOut->ulNextFilePos = ulNewPointer + pGetPartOut->ulBytesRead;
   }/*endif*/

   //--- set length of output structure
   pGetPartOut->prefout.usLenOut = sizeof( GETPART_OUT );

   return ( usRc );
}/* end GetFilePart */
