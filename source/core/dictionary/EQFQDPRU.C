//+----------------------------------------------------------------------------+
//|EQFQDPRU.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:                                                                     |
//|  Marc Hoffmann                                                             |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  Internal utilities for QDPR                                               |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|    QDPRAddToProcessBuffer                                                  |
//|    QDPRAllocateBufferExt                                                   |
//|    QDPRClearAndDeallocateBuffer                                            |
//|    QDPRCloseDictionary                                                     |
//|    QDPRCompareNodes                                                        |
//|    QDPRCopyCurrentTemplates                                                |
//|    QDPRDeallocateIDAStorage                                                |
//|    QDPRDeallocateInOutStruct                                               |
//|    QDPRLineNumbers                                                         |
//|    QDPRMakeTagFromTagID                                                    |
//|    QDPROpenDictionary                                                      |
//|    QDPRPrintDestClose                                                      |
//|    QDPRPrintDestOpen                                                       |
//|    QDPRPrintDestWrite                                                      |
//|    QDPRQueryFileSize                                                       |
//|    QDPRResetFormatBuffer                                                   |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//|  Line 1486 - if UtlBufOpen can handle file append make replacement         |
//+----------------------------------------------------------------------------+

#define INCL_EQF_PRINT            // general print functions
#define INCL_EQF_DICTPRINT        // dictionary print functions
#define INCL_EQF_DLGUTILS         // dialog utilities
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_LDB              // dictionary data encoding functions
#define INCL_EQF_FILT             // dictionary filter functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include <eqf.h>                  // General Translation Manager include file
#include "OtmDictionaryIF.H"
#include <eqfldbi.h>
#include "EQFQDPRI.H"                  // internal header file for dictionary print
#include "EQFQDPR.ID"                  // IDs for dictionary print

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRAddToProcessBuffer                                       |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRAddToProcessBuffer( ppsctCurBufExt, ppchrBuffer, pszString )   |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function adds a string to the buffer area in the current process     |
//|  buffer structure. The string is added to the location to which            |
//|  ppchrBuffer is referencing.                                               |
//|                                                                            |
//|  If the buffer runs full, the function will copy that part of the string   |
//|  that fits in the buffer, allocate a new process buffer structure          |
//|  which is attached to the current one and copy the rest of the string      |
//|  to the buffer of the new process buffer structure.                        |
//|  The new process buffer structure becomes the current one.                 |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_PROCESS_BUFFER *ppsctCurBufExt; // pointer to the current process   |
//|                                        // buffer structure                 |
//|                                        // (or its extension)               |
//|  PCHAR                *ppchrBuffer;    // pointer to the current work      |
//|                                        // position of the buffer in        |
//|                                        // the process buffer structure     |
//|  PSZ                  pszString;       // string to copy to the buffer     |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - updates the value of ppchrBuffer depending on the number of             |
//|    characters added to the buffer                                          |
//|  - if the buffer in the process buffer structure runs full, the function   |
//|    will allocate a new process buffer extension and split the string       |
//|    to copy, so that the old buffer is filled up and the rest of the        |
//|    string is in the new buffer                                             |
//|    in this case ppsctCurBufExt is updated                                  |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRC = QDPRAddToProcessBuffer( &psctCurBuf, &pchrBuffer, "Test" );        |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF string fits completely in the buffer                                   |
//|    Copy the string to the buffer                                           |
//|    Increase the buffer pointer by the number of characters copied          |
//|  ELSE                                                                      |
//|    Allocate a new process buffer structure                                 |
//|    Copy the first part of the string to the current buffer                 |
//|    Copy the rest of the string to the new buffer                           |
//+----------------------------------------------------------------------------+

USHORT QDPRAddToProcessBuffer
(
  PQDPR_PROCESS_BUFFER *ppsctCurBufExt, // pointer to the current process
                                        // buffer structure
                                        // (or its extension)
  PCHAR                *ppchrBuffer,    // pointer to the current work
                                        // position of the buffer in
                                        // the process buffer structure
  PSZ                  pszString        // string to copy to the buffer
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode
  USHORT                usBufFree;                // chars still free
                                                  // in the buffer

  /********************************************************************/
  /*   check if the adding the string to the buffer would overflow    */
  /*                            the buffer                            */
  /********************************************************************/
  usBufFree = (USHORT)(QDPR_MAX_PROCESS_BUFFER -
              ( *ppchrBuffer - (*ppsctCurBufExt)->achrBuffer ) - 1);

  if ( usBufFree > strlen( pszString ) )
  {
    /******************************************************************/
    /*   enough space left, so just copy the string on the current    */
    /*                      work buffer location                      */
    /*   and increase the work buffer by the amount of chars copied   */
    /******************************************************************/
    strcpy( *ppchrBuffer, pszString );
    *ppchrBuffer += strlen( pszString );
  }
  else
  {
    /******************************************************************/
    /*  not enough space left, so copy that part of the string that   */
    /*    still fits in the work buffer then allocate a new buffer    */
    /*                  extension and copy the rest                   */
    /******************************************************************/
    strncpy( *ppchrBuffer, pszString, usBufFree );

    usRC = QDPRAllocateBufferExt( ppsctCurBufExt );
    if ( usRC == QDPR_NO_ERROR )
    {
      *ppchrBuffer = (*ppsctCurBufExt)->achrBuffer;

      strncpy( *ppchrBuffer, &pszString[usBufFree],
               strlen( pszString ) - usBufFree );

      *ppchrBuffer += ( strlen( pszString ) - usBufFree );
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRAddToProcessBuffer */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRAllocateBufferExt                                        |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRAllocateBufferExt( ppsctCurBufExt )                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function allocates another extension of a process buffer.            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_PROCESS_BUFFER  *ppsctCurBufExt; // pointer to current process      |
//|                                         // buffer extension to which       |
//|                                         // new extension shall be          |
//|                                         // attached                        |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Allocate new extension and set new current buffer extension pointer       |
//+----------------------------------------------------------------------------+

USHORT QDPRAllocateBufferExt
(
  PQDPR_PROCESS_BUFFER  *ppsctCurBufExt  // pointer to current process
                                         // buffer extension to which
                                         // new extension shall be
                                         // attached
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode



  if ( UtlAlloc( (PVOID *) &((*ppsctCurBufExt)->psctBufferExtension),
                 0L, (LONG)sizeof( QDPR_PROCESS_BUFFER ), NOMSG ) )
  {
    *ppsctCurBufExt = (*ppsctCurBufExt)->psctBufferExtension;
  }
  else
  {
    usRC = QDPR_NO_MEMORY;
  } /* endif */

  return( usRC );

} /* end of function QDPRAllocateBufferExt */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRClearAndDeallocateBuffer                                 |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRClearAndDeallocateBuffer( psctBuffer )                         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function clears the psctBuffer->achrBuffer to NULC and deallocates   |
//|  any extensions to psctBuffer->psctBufferExtension.                        |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_PROCESS_BUFFER  psctBuffer; // process buffer area to be            |
//|                                    // cleared and of which the             |
//|                                    // extensions will be deallocated       |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRC = QDPRClearAndDeallocateBuffer( psctCurBuffer );                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Clear buffer area in psctBuffer->achrBuffer                               |
//|  WHILE there are extensions to psctBuffer                                  |
//|    Deallocate the extensions                                               |
//+----------------------------------------------------------------------------+

USHORT QDPRClearAndDeallocateBuffer
(
  PQDPR_PROCESS_BUFFER  psctBuffer  // process buffer area to be
                                    // cleared and of which the
                                    // extensions will be deallocated
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode
  PQDPR_PROCESS_BUFFER  psctTemp;                 // temp buffer pointer



  /********************************************************************/
  /*               clear the buffer area in psctBuffer                */
  /********************************************************************/
  if ( psctBuffer != NULL )
  {
    memset( psctBuffer->achrBuffer, NULC, QDPR_MAX_PROCESS_BUFFER );

    /******************************************************************/
    /*          now deallocate any extensions of the buffer           */
    /******************************************************************/
    if ( psctBuffer->psctBufferExtension != NULL )
    {
      psctTemp = psctBuffer->psctBufferExtension;
      while ( psctTemp != NULL )
      {
        psctBuffer->psctBufferExtension = psctTemp->psctBufferExtension;
        UtlAlloc( (PVOID *) &psctTemp, 0L, 0L, NOMSG );
        psctTemp = psctBuffer->psctBufferExtension;
      } /* endwhile */
      psctBuffer->psctBufferExtension = NULL;
    } /* endif */
  } /* endif */

  return ( usRC );

} /* end of function QDPRClearAndDeallocateBuffer */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRCloseDictionary                                          |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  VOID QDPRCloseDictionary( hUCB, hDCB )                                    |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function closes dictionaries opened with QDPROpenDictionary.         |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  HUCB         hUCB;      // UCB handle returned by QDPRDictionaryOpen      |
//|  HDCB         hDCB;      // DCB handle returned by QDPRDictionaryOpen      |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: none                                                       |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - closes dictionary                                                       |
//|  - ends ASD processing                                                     |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Close dictionary                                                          |
//|  End ASD processing                                                        |
//+----------------------------------------------------------------------------+

VOID QDPRCloseDictionary                /* 1@KIT1084C */
(
  HUCB         hUCB,      // UCB handle returned by QDPRDictionaryOpen
  HDCB         hDCB,      // DCB handle returned by QDPRDictionaryOpen
  BOOL         fUnLock    // unlock dictionary flag             /* 1@KIT1084A */
)
{
                                                                /* 3@KIT1084A */
  CHAR    szDictName[MAX_FNAME];       // buffer for dictionary name
  CHAR    szPropName[MAX_EQF_PATH];    // buffer for dictionary property name

  if ( hUCB != NULL )
  {
    if ( hDCB != NULL )
    {
                                                               /* 17@KIT1084A */
      if ( fUnLock )
      {
        /****************************************************************/
        /* Get dictionary name                                          */
        /****************************************************************/
        AsdQueryDictShortName( hDCB, szDictName );

        /****************************************************/
        /* Build dictionary property name                   */
        /****************************************************/
        UtlMakeEQFPath( szPropName, NULC, SYSTEM_PATH, NULL );
        strcat( szPropName, BACKSLASH_STR );
        Utlstrccpy( szPropName + strlen(szPropName ), szDictName, DOT );
        strcat( szPropName, EXT_OF_DICTPROP );
      } /* endif */

      /****************************************************************/
      /*                       close dictionary                       */
      /****************************************************************/
      AsdClose( hUCB, hDCB );

      /****************************************************************/
      /* Unlock dictionary                                            */
      /****************************************************************/
      if ( fUnLock )
      {
        REMOVESYMBOL( szPropName );
      } /* endif */
    } /* endif */

    /******************************************************************/
    /*                       end ASD processing                       */
    /******************************************************************/
    AsdEnd( hUCB );
  } /* endif */

} /* end of function QDPRCloseDictionary */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRCompareNodes                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  BOOL QDPRCompareNodes( pNode1, pNode2, usNoOfFields )                     |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function compares two nodes and returns TRUE if the data             |
//|  in all fields of the nodes is equal.                                      |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_NODE      pNode1;        // pointer to node 1                       |
//|  PQLDB_NODE      pNode2;        // pointer to node 2                       |
//|  USHORT          usNoOfFields;  // No of fields in the two nodes           |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: BOOL                                                       |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  TRUE         - all fields in the two nodes are equal                      |
//|  FALSE        - fields are not equal                                       |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  FOR all fields in the nodes DO                                            |
//|    Check if the data in the field is equal                                 |
//+----------------------------------------------------------------------------+

BOOL QDPRCompareNodes(

  PQLDB_NODE      pNode1,        // pointer to node 1
  PQLDB_NODE      pNode2,        // pointer to node 2
  USHORT          usNoOfFields ) // No of fields in the two nodes

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  BOOL            fEqual = TRUE;          // function returncode
  USHORT          i;                      // a counter



  /********************************************************************/
  /*                loop over the fields in the nodes                 */
  /********************************************************************/
  for ( i = 0; ( i < usNoOfFields ) && fEqual; i++ )
  {
    /******************************************************************/
    /*                 compare the data in the nodes                  */
    /******************************************************************/
    if ( UTF16strcmp( pNode1->aFields[i].pszData,
                 pNode2->aFields[i].pszData ) != 0 )
    {
      fEqual = FALSE;
    } /* endif */
  } /* endfor */

  return( fEqual );

} /* end of function QDPRCompareNodes */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRCopyCurrentTemplates                                     |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRCopyCurrentTemplates( pSourceTemplate, pTargetTemplate )       |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  The function copies the current template of pSourceTemplate to the        |
//|  current template of pTargetTemplate.                                      |
//|                                                                            |
//|  If data areas are allocated outside the node they will be copied too.     |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE   pSourceTemplate; // pointer to tree handle of               |
//|                                 // template from which to copy             |
//|  PQLDB_HTREE   pTargetTemplate; // pointer to tree handle of               |
//|                                 // template into which to copy             |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory                                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - the storage for the target template has to be already allocated         |
//|    using QLDBCreateTree                                                    |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRC = QDPRCopyCurrentTemplates( pSourceTemplate, pTargetTemplate );      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  FOR level = 1 to QLDB_MAX_LEVELS                                          |
//|     Check if in the target node are data areas allocated out of the node   |
//|     Move the data from the source node to the target node                  |
//|     FOR field = 1 to usNoOfFields                                          |
//|        IF data in source node is allocated out of the node                 |
//|          Allocate storage for the data outside the node                    |
//|          Copy the data from the source node field to the target node field |
//+----------------------------------------------------------------------------+

USHORT QDPRCopyCurrentTemplates(

       PVOID         pvSource,        // pointer to tree handle of
                                      // template from which to copy
       PVOID         pvTarget )       // pointer to tree handle of
                                      // template into which to copy

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  PQLDB_HTREE pSourceTemplate = (PQLDB_HTREE)pvSource;
  PQLDB_HTREE pTargetTemplate = (PQLDB_HTREE)pvTarget;
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode
  USHORT                i;                        // level index
  USHORT                j;                        // node fields index


  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QDPR_NO_ERROR );
        i++ )
  {
    /******************************************************************/
    /*  move over the fields in the target node to check whether the  */
    /*    data they are pointing to is out of the node and so has     */
    /*                    to be deallocated before                    */
    /******************************************************************/
    for ( j = 0; j < pTargetTemplate->ausNoOfFields[i]; j++ )
    {
      if ( !( pTargetTemplate->apCurLevelNode[i]->aFields[j].fDataInNode ) )
      {
        if ( pTargetTemplate->apCurLevelNode[i]->aFields[j].pszData
             != NULL )
        {
          UtlAlloc( (PVOID *) &(pTargetTemplate->apCurLevelNode[i]->aFields[j].pszData),
                    0L, 0L, NOMSG );
        } /* endif */
      } /* endif */
    } /* endfor */

    /******************************************************************/
    /*    move the content of the source node over the target node    */
    /******************************************************************/
    memcpy( pTargetTemplate->apCurLevelNode[i],
            pSourceTemplate->apCurLevelNode[i],
            (USHORT)pSourceTemplate->aulStorageOnLevel[i] );

    /******************************************************************/
    /*                      adjust node pointers                      */
    /******************************************************************/
    if ( i == 0 )
    {
      pTargetTemplate->apCurLevelNode[i]->pParent = NULL;
    }
    else
    {
      pTargetTemplate->apCurLevelNode[i]->pParent =
             pTargetTemplate->apCurLevelNode[i-1];
    } /* endif */

    pTargetTemplate->apCurLevelNode[i]->pLeft = NULL;
    pTargetTemplate->apCurLevelNode[i]->pRight = NULL;

    if ( i != QLDB_MAX_LEVELS - 1 )
    {
      pTargetTemplate->apCurLevelNode[i]->pChild =
             pTargetTemplate->apCurLevelNode[i+1];
    }
    else
    {
      pTargetTemplate->apCurLevelNode[i]->pChild = NULL;
    } /* endif */

    /******************************************************************/
    /*   now loop over the fields to check if the data is in or out   */
    /*  of the node, if the latter is the case allocate storage for   */
    /*           the data and copy it from the source node            */
    /******************************************************************/
    for ( j = 0; ( j < pSourceTemplate->ausNoOfFields[i] ) &&
                 ( usRC == QDPR_NO_ERROR ); j++ )
    {
      if ( !( pSourceTemplate->apCurLevelNode[i]->aFields[j].fDataInNode ) )
      {
        if ( UtlAlloc( (PVOID *)
                &(pTargetTemplate->apCurLevelNode[i]->aFields[j].pszData),
                0L,
                (LONG)( UTF16strlenBYTE( pSourceTemplate->apCurLevelNode[i]->
                                aFields[j].pszData ) + sizeof(CHAR_W)/*1*/ ),
                NOMSG ) )
        {
          UTF16strcpy( pTargetTemplate->apCurLevelNode[i]->aFields[j].pszData,
                  pSourceTemplate->apCurLevelNode[i]->aFields[j].pszData );
        }
        else
        {
          usRC = QDPR_NO_ERROR;
        } /* endif */
      } /* endif */
    } /* endfor */
  } /* endfor */

  return( usRC );

} /* end of function QDPRCopyCurrentTemplates */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRDeallocateIDAStorage                                     |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  This function deallocates all previously allocated structures that are    |
//|  referenced in the thread IDA (expect the input/output structure).         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  USHORT QDPRDeallocateIDAStorage( psctIDA )                                |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_THREAD    psctIDA;    // pointer to thread IDA                      |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - deallocates all structure referenced in the thread IDA                  |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRC = QDPRDeallocateIDAStorage( psctIDA );                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Deallocate the field cross-reference tables                               |
//|  Deallocate the buffers                                                    |
//|  Deallocate the templates                                                  |
//|  Deallocate the end-tag and repeat stack                                   |
//|  Deallocate format buffer areas                                            |
//|  Deallocate entry buffer area                                              |
//|  Deallocate thread stack area                                              |
//+----------------------------------------------------------------------------+

USHORT QDPRDeallocateIDAStorage
(
  PQDPR_THREAD    psctIDA     // pointer to thread IDA
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT                usRC = QDPR_NO_ERROR;     // function returncode
  PQDPR_FCRT            psctFCRTTemp;             // temp fcrt pointer
  PQDPR_REPEAT_STACK    psctRepeatStackTemp;      // temp repeat stack ptr



  if ( psctIDA != NULL )
  {
    /******************************************************************/
    /*              deallocate only the FCRT extensions,              */
    /*    because the base FCRT will be deallocated together with     */
    /*                    the corresponding buffer                    */
    /******************************************************************/
    if ( psctIDA->psctHeaderFCRT != NULL )
    {
      psctIDA->psctHeaderFCRT =
        psctIDA->psctHeaderFCRT->psctFCRTExtension;
      while ( psctIDA->psctHeaderFCRT != NULL )
      {
        psctFCRTTemp = psctIDA->psctHeaderFCRT;
        psctIDA->psctHeaderFCRT = psctFCRTTemp->psctFCRTExtension;
        UtlAlloc( (PVOID *) &psctFCRTTemp, 0L, 0L, NOMSG );
      } /* endwhile */
    } /* endif */

    if ( psctIDA->psctPageheadFCRT != NULL )
    {
      psctIDA->psctPageheadFCRT =
        psctIDA->psctPageheadFCRT->psctFCRTExtension;
      while ( psctIDA->psctPageheadFCRT != NULL )
      {
        psctFCRTTemp = psctIDA->psctPageheadFCRT;
        psctIDA->psctPageheadFCRT = psctFCRTTemp->psctFCRTExtension;
        UtlAlloc( (PVOID *) &psctFCRTTemp, 0L, 0L, NOMSG );
      } /* endwhile */
    } /* endif */

    if ( psctIDA->psctEntryFCRT != NULL )
    {
      psctIDA->psctEntryFCRT =
        psctIDA->psctEntryFCRT->psctFCRTExtension;
      while ( psctIDA->psctEntryFCRT != NULL )
      {
        psctFCRTTemp = psctIDA->psctEntryFCRT;
        psctIDA->psctEntryFCRT = psctFCRTTemp->psctFCRTExtension;
        UtlAlloc( (PVOID *) &psctFCRTTemp, 0L, 0L, NOMSG );
      } /* endwhile */
    } /* endif */

    if ( psctIDA->psctRepeatFCRT != NULL )
    {
      psctIDA->psctRepeatFCRT =
        psctIDA->psctRepeatFCRT->psctFCRTExtension;
      while ( psctIDA->psctRepeatFCRT != NULL )
      {
        psctFCRTTemp = psctIDA->psctRepeatFCRT;
        psctIDA->psctRepeatFCRT = psctFCRTTemp->psctFCRTExtension;
        UtlAlloc( (PVOID *) &psctFCRTTemp, 0L, 0L, NOMSG );
      } /* endwhile */
    } /* endif */

    if ( psctIDA->psctPagefootFCRT != NULL )
    {
      psctIDA->psctPagefootFCRT =
        psctIDA->psctPagefootFCRT->psctFCRTExtension;
      while ( psctIDA->psctPagefootFCRT != NULL )
      {
        psctFCRTTemp = psctIDA->psctPagefootFCRT;
        psctIDA->psctPagefootFCRT = psctFCRTTemp->psctFCRTExtension;
        UtlAlloc( (PVOID *) &psctFCRTTemp, 0L, 0L, NOMSG );
      } /* endwhile */
    } /* endif */

    if ( psctIDA->psctTrailerFCRT != NULL )
    {
      psctIDA->psctTrailerFCRT =
        psctIDA->psctTrailerFCRT->psctFCRTExtension;
      while ( psctIDA->psctTrailerFCRT != NULL )
      {
        psctFCRTTemp = psctIDA->psctTrailerFCRT;
        psctIDA->psctTrailerFCRT = psctFCRTTemp->psctFCRTExtension;
        UtlAlloc( (PVOID *) &psctFCRTTemp, 0L, 0L, NOMSG );
      } /* endwhile */
    } /* endif */

    /******************************************************************/
    /*                     deallocate the buffers                     */
    /******************************************************************/
    QDPRClearAndDeallocateBuffer( psctIDA->psctHeaderBuffer );
    UtlAlloc( (PVOID *) &( psctIDA->psctHeaderBuffer ), 0L, 0L, NOMSG );

    QDPRClearAndDeallocateBuffer( psctIDA->psctPageheadBuffer );
    UtlAlloc( (PVOID *) &( psctIDA->psctPageheadBuffer ), 0L, 0L, NOMSG );

    QDPRClearAndDeallocateBuffer( psctIDA->psctEntryBuffer );
    UtlAlloc( (PVOID *) &( psctIDA->psctEntryBuffer ), 0L, 0L, NOMSG );

    QDPRClearAndDeallocateBuffer( psctIDA->psctPagefootBuffer );
    UtlAlloc( (PVOID *) &( psctIDA->psctPagefootBuffer ), 0L, 0L, NOMSG );

    QDPRClearAndDeallocateBuffer( psctIDA->psctTrailerBuffer );
    UtlAlloc( (PVOID *) &( psctIDA->psctTrailerBuffer ), 0L, 0L, NOMSG );

    /******************************************************************/
    /*                     destroy the templates                      */
    /******************************************************************/
    if ( psctIDA->psctEntry != NULL )
    {
      QLDBDestroyTree( &(psctIDA->psctEntry ) );
    } /* endif */

    if ( psctIDA->psctCurrentTemplate != NULL )
    {
      QLDBDestroyTree( &(psctIDA->psctCurrentTemplate ) );
    } /* endif */

    if ( psctIDA->psctFirstPageTemplate != NULL )
    {
      QLDBDestroyTree( &(psctIDA->psctFirstPageTemplate ) );
    } /* endif */

    if ( psctIDA->psctLastPageTemplate != NULL )
    {
      QLDBDestroyTree( &(psctIDA->psctLastPageTemplate ) );
    } /* endif */

    /******************************************************************/
    /*                    destroy the repeat stack                    */
    /******************************************************************/
    while ( psctIDA->psctRepeatStack != NULL )
    {
      psctRepeatStackTemp = psctIDA->psctRepeatStack;
      psctIDA->psctRepeatStack = psctRepeatStackTemp->psctPrev;
      UtlAlloc( (PVOID *) &psctRepeatStackTemp, 0L, 0L, NOMSG );
    } /* endwhile */

    /******************************************************************/
    /*                   deallocate the format IDA                    */
    /******************************************************************/
    if ( psctIDA->psctFormatIDA != NULL )
    {
      if ( psctIDA->psctFormatIDA->psctEntry != NULL )
      {
        if ( psctIDA->psctFormatIDA->psctEntry->psctBuffer != NULL )
        {
          QDPRClearAndDeallocateBuffer(
                      psctIDA->psctFormatIDA->psctEntry->psctBuffer );
        } /* endif */
      } /* endif */

      if ( psctIDA->psctFormatIDA->psctOther != NULL )
      {
        if ( psctIDA->psctFormatIDA->psctOther->psctBuffer != NULL )
        {
          QDPRClearAndDeallocateBuffer(
                      psctIDA->psctFormatIDA->psctOther->psctBuffer );
        } /* endif */
      } /* endif */

      UtlAlloc( (PVOID *) &(psctIDA->psctFormatIDA), 0L, 0L, NOMSG );
    } /* endif */

    /******************************************************************/
    /*                  deallocate entry buffer area                  */
    /******************************************************************/
    if ( psctIDA->pucEntry != NULL )
    {
      UtlAlloc( (PVOID *) &(psctIDA->pucEntry), 0L, 0L, NOMSG );
    } /* endif */

    /******************************************************************/
    /*                  deallocate thread stack area                  */
    /******************************************************************/
    if ( psctIDA->pThreadStack != NULL )
    {
      UtlAlloc( (PVOID *) &( psctIDA->pThreadStack ), 0L, 0L, NOMSG );
    } /* endif */

    /******************************************************************/
    /*               deallocate input/output structure                */
    /******************************************************************/
    QDPRDeallocateInOutStruct( &(psctIDA->psctInOutput) );

    /******************************************************************/
    /*                  deallocate page eject string                  */
    /******************************************************************/
    if ( psctIDA->pszPageEject != NULL )
    {
      UtlAlloc( (PVOID *) &(psctIDA->pszPageEject), 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRDeallocateIDAStorage */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRDeallocateInOutStruct                                    |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  VOID QDPRDeallocateInOutStruct( ppsctInOut )                              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function deallocates the QDPR input/output structure and             |
//|  its various contents.                                                     |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_IN_OUTPUT    *ppsctInOut;         // pointer to input/output        |
//|                                          // structure to be deallocated    |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: none                                                       |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - input/output structure is deallocated                                   |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Deallocate format file buffer area                                        |
//|  Deallocate tokenized format file buffer area                              |
//|  Deallocate input/output structure itself                                  |
//+----------------------------------------------------------------------------+

VOID QDPRDeallocateInOutStruct
(
  PQDPR_IN_OUTPUT    *ppsctInOut          // pointer to input/output
                                          // structure to be deallocated
)
{

  if ( *ppsctInOut != NULL )
  {
    /******************************************************************/
    /*               deallocate format file buffer area               */
    /******************************************************************/
    if ( (*ppsctInOut)->pszFormatFile != NULL )
    {
      UtlAlloc( (PVOID *) &((*ppsctInOut)->pszFormatFile), 0L, 0L, NOMSG );
    } /* endif */

    /******************************************************************/
    /*          deallocate tokenized format file buffer area          */
    /******************************************************************/
    if ( (*ppsctInOut)->pTokFormatFile  != NULL )
    {
      UtlAlloc( (PVOID *) &((*ppsctInOut)->pTokFormatFile), 0L, 0L, NOMSG );
    } /* endif */

    /******************************************************************/
    /*            deallocate input/output structure itself            */
    /******************************************************************/
    UtlAlloc( (PVOID *) ppsctInOut, 0L, 0L, NOMSG );

    *ppsctInOut = NULL;
  } /* endif */

} /* end of function QDPRDeallocateInOutStruct */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRLineNumbers                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRLineNumbers( pszStart, pszEnd, usLength )                      |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function computes the number of lines in a search area.              |
//|                                                                            |
//|  The search area may be specified in three ways:                           |
//|  1. set pszEnd to an address where searching should stop (excluding        |
//|     the character which is referenced by *pszEnd)                          |
//|  2. set usLength to the size of the search area (e.g. usLength = 5         |
//|     would look in the 5 characters pszStart, pszStart+1, pszStart+2,       |
//|     pszStart+3, pszStart+4 for LFs)                                        |
//|     for this setting pszEnd MUST be set to NULL                            |
//|  3. set pszEnd = NULL, usLength = 0, then searching will be done           |
//|     until the first '\0' characters is found                               |
//|                                                                            |
//|  The function returns the number of LFs (i.e. the number of lines) found.  |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PSZ    pszStart;   // pointer to start of search area                     |
//|  PSZ    pszEnd;     // pointer to end of search area or NULL               |
//|                     // if until a '\0' is to be read                       |
//|  USHORT usLength;   // length of search area (pszEnd must be NULL) or 0    |
//|                     // if until a '\0' is to be read                       |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  - no of lines found in the search area                                    |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usLines = QDPRLineNumbers( pszStart, pszEnd, 0 );                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF pszStart is not NULL                                                   |
//|    Set pszRun to pszStart                                                  |
//|    WHILE loop shall not stop                                               |
//|      IF pszEnd is not NULL                                                 |
//|        IF pszRun is the same as pszEnd                                     |
//|          Indicate loop to stop                                             |
//|      ELSE                                                                  |
//|        IF usLength is not 0                                                |
//|          IF the length has been processed                                  |
//|            Indicate loop to stop                                           |
//|        ELSE                                                                |
//|          IF current character is NULC                                      |
//|            Indicate loop to stop                                           |
//|      IF search should go on                                                |
//|        IF current charcater is a LF                                        |
//|          Increase number of lines found                                    |
//|        Let pszRun go on one character                                      |
//+----------------------------------------------------------------------------+

USHORT QDPRLineNumbers(

  PSZ    pszStart,   // pointer to start of search area
  PSZ    pszEnd,     // pointer to end of search area or NULL
                     // if until a '\0' is to be read
  USHORT usLength )  // length of search area (pszEnd must be NULL) or 0
                     // if until a '\0' is to be read

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT    usLines = 0;             // no of lines found in search area
  USHORT    i = 1;                   // a counter
  PSZ       pszRun;                  // run pointer
  BOOL      fStopSearch = FALSE;     // stop searching



  if ( pszStart != NULL )
  {
    pszRun = pszStart;

    /******************************************************************/
    /*                 now loop until loop shall stop                 */
    /******************************************************************/
    while ( ( pszRun != NULL ) && ( *pszRun != NULC ) && !fStopSearch )
    {
      if ( pszEnd != NULL )
      {
        /**************************************************************/
        /*    check if pszEnd is reached or a '\0' (although there    */
        /*                   should not be a '\0')                    */
        /**************************************************************/
        if ( ( pszRun == pszEnd ) || ( *pszRun == NULC ) )
        {
          fStopSearch = TRUE;
        } /* endif */
      }
      else
      {
        if ( usLength != 0 )
        {
          /************************************************************/
          /*   check if the length of the search area is processed    */
          /*  (or a '\0' is found - although there shouldn't be one)  */
          /************************************************************/
          if ( ( i >= usLength ) || ( *pszRun == NULC ) )
          {
            fStopSearch = TRUE;
          } /* endif */
        }
        else
        {
          if ( *pszRun == NULC )
          {
            fStopSearch = TRUE;
          } /* endif */
        } /* endif */
      } /* endif */

      if ( !fStopSearch )
      {
        /**************************************************************/
        /*    if search shall continue check if LF has been found     */
        /*                   which means a new line                   */
        /**************************************************************/
        if ( *pszRun == QDPR_LF )
        {
          usLines++;
        } /* endif */

        pszRun++;
        i++;
      } /* endif */
    } /* endwhile */
  } /* endif */

  return( usLines );

} /* end of function QDPRLineNumbers */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRMakeTagFromTagID                                         |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRMakeTagFromTagID( pszTagBuffer, usBufLength, usTagID, fMsg )   |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function returns the tag string correspoding to the tag ID.          |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  USHORT    usBufLength;                  // length of pszTagBuffer         |
//|                                          // including '\0'                 |
//|  USHORT    usTagID;                      // tag ID                         |
//|  BOOL      fMsg;                         // show messages flag             |
//|                                          // TRUE show messages             |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PSZ       pszTagBuffer;                 // tag buffer                     |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  QDPR_NO_MEMORY        - not enough memory left                            |
//|  QDPR_PATH_NOT_FOUND   - path to tag table could not be found              |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  QDPRMakeTagFromTagID( szTag, sizeof( szTag ), QDPR_DESCRIPTION_TOKEN );   |
//|                                                                            |
//|  This would return "<description>" in szTag.                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Load tag table for QDPR                                                   |
//|  SWITCH on tag ID                                                          |
//|    Copy appropriate tag string in pszTagBuffer                             |
//+----------------------------------------------------------------------------+

USHORT QDPRMakeTagFromTagID(

  PSZ       pszTagBuffer,                 // tag buffer
  USHORT    usBufLength,                  // length of pszTagBuffer
                                          // including '\0'
  USHORT    usTagID,                      // tag ID
  BOOL      fMsg )                        // show messages flag
                                          // TRUE show messages

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  USHORT       usRetCode;                         // returncode from
                                                  // called functions
  USHORT       usStorageError;                    // storage error msg no.
  PLOADEDTABLE pLoadedTagTable = NULL;            // pointer to loaded
                                                  // tag table
  PTAGTABLE    pTagTable = NULL;                  // pointer to tag table
  PTAG         pTag;                              // pointer to structure of
                                                  // active tag
  PSZ          pszPathFileName = NULL;            // full file name
  PSZ          pszWorkBuffer = NULL;              // work buffer
  PSZ          pszAttr;                           // pointer to attribute
                                                  // string
  PSZ          pszTag;                            // pointer to tag names
  PSZ          pTagNames;                         // pointer to start of
                                                  // tag names
  PBYTE        pByte;                             // help pointer
  PATTRIBUTE   pAttr;                             // ptr to start of attributes
                                                  // in tagtable



  /********************************************************************/
  /*                           clear buffer                           */
  /********************************************************************/
  memset( pszTagBuffer, NULC, usBufLength );

  /********************************************************************/
  /*                    allocate temporary storage                    */
  /********************************************************************/
  if ( fMsg )
  {
    usStorageError = ERROR_STORAGE;
  }
  else
  {
    usStorageError = NOMSG;
  } /* endif */

  if ( UtlAlloc( (PVOID *) &pszPathFileName, 0L, (LONG)( 2 * QDPR_MAX_STRING ),
                 usStorageError ) )
  {
    pszWorkBuffer = (PSZ)( pszPathFileName + QDPR_MAX_STRING );
  }
  else
  {
    usRC = QDPR_NO_MEMORY;
  } /* endif */

  /******************************************************************/
  /*                 now try to load the tag table                  */
  /******************************************************************/
  if ( usRC == QDPR_NO_ERROR )
  {
    usRetCode = TALoadTagTable( QDPR_TAGTABLE_NAME, &pLoadedTagTable, TRUE, fMsg );

    if ( usRetCode != NO_ERROR )
    {
      usRC = QDPR_PATH_NOT_FOUND;
    } /* endif */
  } /* endif */

  if ( usRC == QDPR_NO_ERROR )
  {
    /******************************************************************/
    /*                    set pointer to tag table                    */
    /******************************************************************/
    pTagTable = pLoadedTagTable->pTagTable;
  } /* endif */

  /********************************************************************/
  /*           now get the tag name according to the tag ID           */
  /********************************************************************/
  if ( usRC == QDPR_NO_ERROR )
  {
    /******************************************************************/
    /*     set pointers to get to the token name in the tag table     */
    /******************************************************************/
    pByte = (PBYTE)pTagTable;
    pTag = (PTAG)(pByte + pTagTable->stFixTag.uOffset);
    pTagNames = (PSZ)(pByte +  pTagTable-> uTagNames);

    switch ( usTagID )
    {
      case QDPR_COMMENT_ETOKEN :
      case QDPR_DESCRIPTION_ETOKEN :
      case QDPR_DICTBACK_ETOKEN :
      case QDPR_DICTFRONT_ETOKEN :
      case QDPR_ENTRY_ETOKEN :
      case QDPR_PAGEFOOT_ETOKEN :
      case QDPR_PAGEHEAD_ETOKEN :
      case QDPR_REPEAT_ETOKEN :
      case QDPR_COMMENT_TOKEN :
      case QDPR_DESCRIPTION_TOKEN :
      case QDPR_DICTBACK_TOKEN :
      case QDPR_DICTFRONT_TOKEN :
      case QDPR_ENTRY_TOKEN :
      case QDPR_PAGEFOOT_TOKEN :
      case QDPR_PAGEHEAD_TOKEN :
      case QDPR_REPEAT_TOKEN :
      case QDPR_SET_TOKEN :
      case QDPR_VAR_TOKEN :
        {
          /************************************************************/
          /*                  get to the tag string                   */
          /************************************************************/
          pszTag = pTag[usTagID].uTagnameOffs + pTagNames;

          /************************************************************/
          /*        now copy the tag name into the tag buffer         */
          /************************************************************/
          strncpy( pszTagBuffer, pszTag, usBufLength - 1 );

          /************************************************************/
          /*          copy the end-tag character to the tag           */
          /************************************************************/
          if ( strlen( pszTagBuffer ) < (USHORT)(usBufLength - 1) )
          {
            strcat( pszTagBuffer, QDPR_END_TAG_STR );
          } /* endif */
        }
      break;
      case QDPR_FIRST_ON_PAGE_ATTR :
      case QDPR_LAST_ON_PAGE_ATTR :
      case QDPR_LEFT_ATTR :
      case QDPR_NO_DISPLAY_ATTR :
      case QDPR_RIGHT_ATTR :
      case QDPR_SAME_ENTRY_AGAIN_ATTR :
      case QDPR_LEVEL_ENTRY_ATTR :
      case QDPR_LEVEL_HOM_ATTR :
      case QDPR_LEVEL_SENSE_ATTR :
      case QDPR_LEVEL_TARGET_ATTR :
      case QDPR_SYSNAME_DATE_ATTR :
      case QDPR_SYSNAME_DICTNAME_ATTR :
      case QDPR_SYSNAME_FILENAME_ATTR :
      case QDPR_SYSNAME_LINE_LENGTH_ATTR :
      case QDPR_SYSNAME_PAGE_EJECT_ATTR :
      case QDPR_SYSNAME_PAGE_LENGTH_ATTR :
      case QDPR_SYSNAME_PAGE_NO_ATTR :
      case QDPR_SYSNAME_TIME_ATTR :
      case QDPR_FORMAT_ATTR :
      case QDPR_NAME_ATTR :
      case QDPR_VALUE_ATTR :
      case QDPR_MAX_ATTR :
      case QDPR_MIN_ATTR :
        {
          /************************************************************/
          /*         now set pointers to get to the attribute         */
          /************************************************************/
          pAttr = (PATTRIBUTE)(pByte + pTagTable->stAttribute.uOffset);
          pszAttr = pTagNames +
                    pAttr[usTagID - pTagTable->uNumTags].uStringOffs;

          /************************************************************/
          /*        now copy the attribute into the tag buffer        */
          /************************************************************/
          strncpy( pszTagBuffer, pszAttr, usBufLength - 1 );
        }
      break;
    } /* endswitch */
  } /* endif */

  /********************************************************************/
  /*                   deallocate temporary storage                   */
  /********************************************************************/
  if ( pLoadedTagTable )
  {
    TAFreeTagTable( pLoadedTagTable );
  } /* endif */
  if ( pszPathFileName != NULL )
  {
    UtlAlloc( (PVOID *) &pszPathFileName, 0L, 0L, NOMSG );
  } /* endif */

  return( usRC );

} /* end of function QDPRMakeTagFromTagID */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPROpenDictionary                                           |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPROpenDictionary( pszDictName, phUCB, phDCB )                    |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function opens the named dictionary.                                 |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PSZ       pszDictName;   // dictionary name                               |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PHUCB     phUCB;         // UCB handle                                    |
//|  PHDCB     phDCB;         // DCB handle                                    |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//|  any of the LX_.... returncodes                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - ASD is started                                                          |
//|  - Dictionary is opened                                                    |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Start ASD processing                                                      |
//|  Open the dictionary                                                       |
//+----------------------------------------------------------------------------+

USHORT QDPROpenDictionary
(
  PSZ       pszDictName,   // dictionary name
  PHUCB     phUCB,         // UCB handle
  PHDCB     phDCB          // DCB handle
)

{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC;                              // function returncode
  USHORT       usErrDict;                         // for AsdOpen
  PSZ          pszProfName;                       // for AsdOpen
  CHAR         szWorkBuffer[QDPR_MAX_PATH_FILENAME]; // work buffer



  /********************************************************************/
  /*                       start ASD processing                       */
  /********************************************************************/
  usRC = AsdBegin( 1, phUCB );

  if ( usRC == LX_RC_OK_ASD )
  {
    BOOL fIsNew = FALSE;               // is-new flag
    CHAR  szShortName[MAX_FNAME];
    ObjLongToShortName( pszDictName, szShortName, DICT_OBJECT, &fIsNew );
    usRC = QDPR_NO_ERROR;

    /******************************************************************/
    /*              build name to dictionary properties               */
    /******************************************************************/
    UtlMakeEQFPath( szWorkBuffer, NULC, PROPERTY_PATH, (PSZ) NULP );
    strcat( szWorkBuffer, BACKSLASH_STR );
    strcat( szWorkBuffer, szShortName );
    strcat( szWorkBuffer, EXT_OF_DICTPROP );
    pszProfName = szWorkBuffer;

    /******************************************************************/
    /*                        open dictionary                         */
    /******************************************************************/
    usRC = AsdOpen( *phUCB, ASD_GUARDED | ASD_LOCKED, 1, &pszProfName,
                    phDCB, &usErrDict );

    if ( usRC == LX_RC_OK_ASD )
    {
      usRC = QDPR_NO_ERROR;
    }
    else
    {
       /***************************************************************/
       /* Use dictionary language as message parameter if activation  */
       /* of dictionary language failed                               */
       /***************************************************************/
       if ( usRC == LX_BAD_LANG_CODE )
       {
         HPROP           hProp;                       // properties handle
         PPROPDICTIONARY pDictProp;                   // ptr to dict props
         EQFINFO         ErrorInfo;                   // buffer for error info

         UtlMakeEQFPath( szWorkBuffer, NULC, SYSTEM_PATH, NULL );
         strcat( szWorkBuffer, BACKSLASH_STR );
         strcat( szWorkBuffer, pszDictName );
         strcat( szWorkBuffer, EXT_OF_DICTPROP );
         hProp = OpenProperties( szWorkBuffer, NULL,
                                 PROP_ACCESS_READ, &ErrorInfo );
         if ( hProp )
         {
            pDictProp =(PPROPDICTIONARY) MakePropPtrFromHnd( hProp );
            strcpy( szWorkBuffer, pDictProp->szSourceLang );
            pszDictName = szWorkBuffer;
            CloseProperties( hProp, PROP_QUIT, &ErrorInfo );
         } /* endif */
       } /* endif */

       UtlError ( usRC, MB_CANCEL, 1, &pszDictName, QDAM_ERROR );
    } /* endif */
  }
  else
  {
     UtlError ( usRC, MB_CANCEL, 0, NULL, QDAM_ERROR );
  } /* endif */

  return( usRC );

} /* end of function QDPROpenDictionary */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintDestClose                                           |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRPrintDestClose( ppPrintDest )                                  |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function closes the print destination.                               |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_PRINT_DEST   *ppPrintDest; // pointer to print destination          |
//|                                   // structure                             |
//|  PUSHORT            pusDosRC      // pointer to buffer for Dos return code |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR                 - everything was OK                         |
//|  QDPR_ERROR_WRITE_TO_DEST_FILE - error while writing to the print          |
//|                                  destination                               |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - ppPrintDest must have been created using QDPRPrintDestOpen              |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF printer destination is a printer                                       |
//|    CALL UtlPrintClose                                                      |
//|  ELSE                                                                      |
//|    CALL UtlBufClose                                                        |
//|  Deallocate print destination structure                                    |
//+----------------------------------------------------------------------------+

USHORT QDPRPrintDestClose
(
  PQDPR_PRINT_DEST   *ppPrintDest, // pointer to print destination
                                   // structure
  PUSHORT            pusDosRC      // pointer to buffer for Dos return code
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode


  *pusDosRC = NO_ERROR;

  if ( *ppPrintDest != NULL )
  {
    /******************************************************************/
    /*                   close the printer or file                    */
    /******************************************************************/
    if ( (*ppPrintDest)->fPrinter )
    {
       if ( (*ppPrintDest)->hPrinter )  UtlPrintClose( (*ppPrintDest)->hPrinter );
    }
    else
    {
      *pusDosRC = UtlBufClose( (*ppPrintDest)->pFile, FALSE );
      if ( *pusDosRC != NO_ERROR )
      {
        usRC = QDPR_ERROR_WRITE_TO_DEST_FILE;
        UtlDelete( (*ppPrintDest)->szPrintDest, 0L, FALSE );
      } /* endif */
    } /* endif */

    /******************************************************************/
    /*             deallocate print destination structure             */
    /******************************************************************/
    UtlAlloc( (PVOID *) ppPrintDest, 0L, 0L, NOMSG );

    *ppPrintDest = NULL;
  } /* endif */

  return( usRC );

} /* end of function QDPRPrintDestClose */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintDestOpen                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRPrintDestOpen( ppPrintDest, pszDestName, fPrinter,             |
//|                            fFileReplace )                                  |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function opens the print destination, either a printer or a file.    |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PSZ                pszDestName;  // destination name if a file is         |
//|                                   // the print destination                 |
//|                                   // if a printer is the print destination |
//|                                   // put in here the dictionary to be      |
//|                                   // printed                               |
//|  BOOL               fPrinter;     // TRUE if a printer is the print        |
//|                                   // destination                           |
//|  BOOL               fFileReplace; // TRUE if file is to be replaced        |
//|                                   // only valid if fPrinter is FALSE       |
//|  PUSHORT            pusDosRC      // pointer to buffer for Dos return code |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|PQDPR_PRINT_DEST   *ppPrintDest; // pointer to print destination            |
//|                                 // structure                               |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR               - everything was OK                           |
//|  QDPR_NO_MEMORY              - not enough memory left                      |
//|  QDPR_ERROR_OPEN_DEST_FILE   - the print destination file could not        |
//|                                be opened                                   |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Allocate print destination storage                                        |
//|  IF a printer is to be opened                                              |
//|    CALL UtlPrintOpen                                                       |
//|  ELSE                                                                      |
//|    CALL UtlBufOpen                                                         |
//+----------------------------------------------------------------------------+

USHORT QDPRPrintDestOpen
(
  PQDPR_PRINT_DEST   *ppPrintDest, // pointer to print destination
                                   // structure
  PSZ                pszDestName,  // destination name if a file is
                                   // the print destination
                                   // if a printer is the print destination
                                   // put in here the dictionary to be
                                   // printed
  BOOL               fPrinter,     // TRUE if a printer is the print
                                   // destination
  BOOL               fFileReplace, // TRUE if file is to be replaced
                                   // only valid if fPrinter is FALSE
  PUSHORT            pusDosRC      // pointer to buffer for Dos return code
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  PQDPR_PRINT_DEST   pPrintDest = NULL;           // pointer to print destination
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  USHORT       usOpenMode;                        // file open mode


  *pusDosRC = NO_ERROR;


  *ppPrintDest = NULL;

  /********************************************************************/
  /*       allocate storage for the print destination structure       */
  /********************************************************************/
  if ( UtlAlloc( (PVOID *) ppPrintDest, 0L, (LONG)sizeof( QDPR_PRINT_DEST ),
                 NOMSG ) )
  {
    pPrintDest = *ppPrintDest;
    pPrintDest->fPrinter = fPrinter;
    strcpy( pPrintDest->szPrintDest, pszDestName );
  }
  else
  {
    usRC = QDPR_NO_MEMORY;
  } /* endif */

  if ( usRC == QDPR_NO_ERROR )
  {
    /******************************************************************/
    /*      check if a printer is used or a file is to be opened      */
    /******************************************************************/
    if ( fPrinter )
    {
	  HMODULE hResMod;
	  hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      /****************************************************************/
      /* Setup print job name (using pPrintDest->szPrintDest as buffer)*/
      /****************************************************************/
      LOADSTRING( NULLHANDLE, hResMod, SID_QDPR_PRTLIST_TITLE,
                  pPrintDest->szPrintDest );
      ANSITOOEM( pPrintDest->szPrintDest );
      strcat( pPrintDest->szPrintDest, pszDestName );

      /****************************************************************/
      /*                        open a printer                        */
      /****************************************************************/
      if ( !UtlPrintOpen( &(pPrintDest->hPrinter), pPrintDest->szPrintDest, NULLHANDLE ) )
      {
        usRC = QDPR_ERROR_OPEN_DEST_FILE;
      } /* endif */
      strcpy( pPrintDest->szPrintDest, pszDestName );
    }
    else
    {
      /****************************************************************/
      /*                         open a file                          */
      /****************************************************************/
      if ( fFileReplace )
      {
        usOpenMode = FILE_CREATE;
      }
      else
      {
        usOpenMode = FILE_APPEND;
      } /* endif */

      *pusDosRC = UtlBufOpen( &(pPrintDest->pFile),
                              pszDestName, QDPR_MAX_PROCESS_BUFFER,
                              usOpenMode, FALSE );
      if ( *pusDosRC != NO_ERROR )
      {
        usRC = QDPR_ERROR_OPEN_DEST_FILE;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*   if an error occurred deallocate print destination structure    */
  /********************************************************************/
  if ( usRC != QDPR_NO_ERROR )
  {
    if ( *ppPrintDest != NULL )
    {
      UtlAlloc( (PVOID *) ppPrintDest, 0L, 0L, NOMSG );
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRPrintDestOpen */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRPrintDestWrite                                           |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRPrintDestWrite( pPrintDest, pszString )                        |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function writes a string to the print destination.                   |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_PRINT_DEST      pPrintDest; // pointer to print destination         |
//|                                    // structure                            |
//|  PSZ                   pszString;  // string to be written                 |
//|  PUSHORT            pusDosRC      // pointer to buffer for Dos return code |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR                 - everything was OK                         |
//|  QDPR_NO_MEMORY                - not enough memory left                    |
//|  QDPR_ERROR_WRITE_TO_DEST_FILE - error while writing to the print          |
//|                                  destination                               |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - pPrintDest must have been created using QDPRPrintDestOpen               |
//|  - pszString must end with a NULC character                                |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF a printer is the print destination                                     |
//|    Divide the string up into seperate lines                                |
//|    Print each line (UtlPrintLine)                                          |
//|  ELSE                                                                      |
//|    CALL UtlBufWrite                                                        |
//+----------------------------------------------------------------------------+

USHORT QDPRPrintDestWrite
(
  PQDPR_PRINT_DEST      pPrintDest, // pointer to print destination
                                    // structure
  PSZ                   pszString,  // string to be written
  PUSHORT            pusDosRC      // pointer to buffer for Dos return code
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;   // function returncode
  PSZ          pszRun;                 // run pointer
  PSZ          pszStart;               // start pointer
  CHAR         chrReplaced;            // replace character

  *pusDosRC = NO_ERROR;

  if ( pPrintDest != NULL )
  {
    /******************************************************************/
    /*            check if a printer is print destination             */
    /******************************************************************/
    if ( pPrintDest->fPrinter )
    {
      /****************************************************************/
      /*      as UtlPrintLine supports only printing of one line      */
      /*                     loop over the lines                      */
      /****************************************************************/
      pszRun = pszString;
      pszStart = pszString;

      while ( ( *pszRun != NULC ) && ( usRC == QDPR_NO_ERROR ) )
      {
        /**************************************************************/
        /*         loop until a NULC or a CRLF has been found         */
        /**************************************************************/
        while ( ( *pszRun != NULC ) && ( *pszRun != QDPR_LF ) )
        {
          pszRun++;
        } /* endwhile */

        /**************************************************************/
        /*                  if a CRLF has been found                  */
        /**************************************************************/
        if ( *pszRun == QDPR_LF )
        {
          pszRun++;

          /************************************************************/
          /*   replace the current character with NULC but remember   */
          /*                  the character replaced                  */
          /************************************************************/
          chrReplaced = *pszRun;
          *pszRun = NULC;

          /************************************************************/
          /*                      print the line                      */
          /************************************************************/
          if ( !UtlPrintLine( pPrintDest->hPrinter, pszStart ) )
          {
            usRC = QDPR_ERROR_WRITE_TO_DEST_FILE;
          } /* endif */

          /************************************************************/
          /*                 re-replace the character                 */
          /************************************************************/
          *pszRun = chrReplaced;

          pszStart = pszRun;
        } /* endif */
      } /* endwhile */

      /****************************************************************/
      /* print the rest of the string (from the last CRLF to the NULC)*/
      /****************************************************************/
      if ( pszRun != pszStart )
      {
        if ( !UtlPrintLine( pPrintDest->hPrinter, pszStart ) )
        {
            usRC = QDPR_ERROR_WRITE_TO_DEST_FILE;
        } /* endif */
      } /* endif */
    }
    else
    {
      *pusDosRC = UtlBufWrite( pPrintDest->pFile, pszString,
                               (USHORT)(strlen( pszString )), FALSE );
      if ( *pusDosRC != NO_ERROR )
      {
        usRC = QDPR_ERROR_WRITE_TO_DEST_FILE;
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRPrintDestWrite */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRQueryFileSize                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRQueryFileSize( pszFilename, pulFileSize )                      |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function queries the filesize of an existing, non-opened file.       |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PSZ          pszFilename;            // filename                          |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PULONG       pulFileSize;            // returned file size                |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR               - everything was OK                           |
//|  QDPR_ERROR_OPEN_DEST_FILE   - the file could not be opened or an error    |
//|                                during the file size retrieval occurred     |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - the file must be in a close status                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|   usRC = QDPRQueryFileSize( "C:\TEST.DAT", &ulFileSize );                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Open file                                                                 |
//|  Retrieve file size                                                        |
//|  Close file                                                                |
//+----------------------------------------------------------------------------+

USHORT QDPRQueryFileSize
(
  PSZ          pszFilename,            // filename
  PULONG       pulFileSize             // returned file size
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode
  HFILE        hFile;                             // file handle
  USHORT       usAction;                          // action taken

  *pulFileSize = 0L;

  /********************************************************************/
  /*                            open file                             */
  /********************************************************************/
  if ( UtlOpen( pszFilename, &hFile, &usAction, 0L, FILE_NORMAL,
                FILE_OPEN, OPEN_ACCESS_READONLY | OPEN_SHARE_DENYNONE,
                0L, FALSE ) == NO_ERROR )
  {
    /******************************************************************/
    /*                         get file size                          */
    /******************************************************************/
    if ( UtlGetFileSize( hFile, pulFileSize, FALSE ) != NO_ERROR )
    {
      usRC = QDPR_ERROR_OPEN_DEST_FILE;
    } /* endif */

    /******************************************************************/
    /*                      close the file again                      */
    /******************************************************************/
    UtlClose( hFile, FALSE );
  }
  else
  {
    usRC = QDPR_ERROR_OPEN_DEST_FILE;
  } /* endif */

  return( usRC );

} /* end of function QDPRQueryFileSize */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QDPRResetFormatBuffer                                        |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QDPRResetFormatBuffer( psctFormatBuffer, fResetVar )               |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function clears the buffer area of psctBuffer, deallocates any       |
//|  extensions of the buffer area and resets the pointers and variables       |
//|  in the format buffer.                                                     |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQDPR_FORMAT_BUFFERS  psctFormatBuffer; // format buffer to be reset      |
//|  BOOL                  fResetVar;        // TRUE = reset variables         |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QDPR_NO_ERROR         - everything was OK                                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - format buffer is cleared and reset                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Clear buffer and deallocate extensions (QDPRClearAndDeallocateBuffer)     |
//|  Reset variables in the format buffer                                      |
//+----------------------------------------------------------------------------+

USHORT QDPRResetFormatBuffer
(
  PQDPR_FORMAT_BUFFERS  psctFormatBuffer,  // format buffer to be reset
  BOOL                  fResetVar         // TRUE = reset variables
)
{
  /********************************************************************/
  /*                         local variables                          */
  /********************************************************************/
  USHORT       usRC = QDPR_NO_ERROR;              // function returncode



  if ( psctFormatBuffer != NULL )
  {
     usRC = QDPRClearAndDeallocateBuffer( psctFormatBuffer->psctBuffer );

     psctFormatBuffer->pchrLastWritten =
                       psctFormatBuffer->psctBuffer->achrBuffer;
     psctFormatBuffer->psctCurBufExt =
                       psctFormatBuffer->psctBuffer;

    if ( fResetVar )
    {
      psctFormatBuffer->fNewLineStarts = TRUE;
      psctFormatBuffer->fLineSplit = FALSE;
      psctFormatBuffer->usCurIndent = 0;
      psctFormatBuffer->usCharsUsed = 0;
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QDPRResetFormatBuffer */
