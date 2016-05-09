//+----------------------------------------------------------------------------+
//|EQFQLDBI.C                                                                  |
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
//|  This file contains the internal functions of QLDB processing.             |
//|  The external functions can be found in the file EQFQLDB.C.                |
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
//|    QLDBCompare2Nodes                                                       |
//|    QLDBComputeFirstFieldOffset                                             |
//|    QLDBComputeSubtreeToDestroy                                             |
//|    QLDBConvertRecord                                                       |
//|    QLDBCopyNode                                                            |
//|    QLDBCopyNodeToRecord                                                    |
//|    QLDBCreateNode                                                          |
//|    QLDBCreateTreeHandle                                                    |
//|    QLDBDestroyIncompleteSubtree                                            |
//|    QLDBDestroyNode                                                         |
//|    QLDBFillData                                                            |
//|    QLDBFilterEscChar                                                       |
//|    QLDBJoinSameSubtree                                                     |
//|    QLDBNextRecCtrlChar                                                     |
//|    QLDBNextTreeTemplate                                                    |
//|    QLDBNodeStorage                                                         |
//|    QLDBNodeToRecStorage                                                    |
//|    QLDBPrevRecCtrlChar                                                     |
//|    QLDBPrevTreeTemplate                                                    |
//|    QLDBRemoveChildSubtree                                                  |
//|    QLDBUpdateFieldData                                                     |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Changed by Gerhard:                                                         |
//|  - removed temporary save of old data in QLDBUpdateFieldData (Performance) |
//|    In case of memory shortage no further processing will be performed on   |
//|    the tree. It is not required to restore any old data.                   |
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
// $Revision: 1.2 $ ----------- 19 Oct 2001
// --RJ: unicode enabling
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
 * $Header:   J:\DATA\EQFQLDBI.CV_   1.0   09 Jan 1996 09:14:06   BUILD  $
 *
 * $Log:   J:\DATA\EQFQLDBI.CV_  $
 *
 *    Rev 1.0   09 Jan 1996 09:14:06   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
#define INCL_EQF_LDB              // dictionary data encoding functions
#include <eqf.h>                  // General Translation Manager include file

#include <eqfldbi.h>              // LDB private include file



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBCompare2Nodes                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBCompare2Nodes( phTree, pNode1, pNode2, pfNodesAreSame )        |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function compares two nodes and checks if the data in the fields     |
//|  is the same.                                                              |
//|                                                                            |
//|  If the nodes are the same the output flag is set to TRUE otherwise        |
//|  to FALSE.                                                                 |
//|                                                                            |
//|  If an error occurred the output flag is set to FALSE.                     |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//|  PQLDB_NODE   pNode1;                           // first compare node      |
//|  PQLDB_NODE   pNode2;                           // second compare node     |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  BOOL         *pfNodesAreSame;                  // TRUE = nodes are same   |
//|                                                 // FALSE = they are not    |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_ERROR_IN_TREE          - an unrecoverable error is in the tree       |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle                                                     |
//|  - 2 valid nodes                                                           |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//| usRetCode = QLDBCompare2Nodes( phTree, pNode1, pNode2, &fSame );           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  FOR 1 to usNoOfFields DO                                                  |
//|    Check if data in field of each node is the same                         |
//+----------------------------------------------------------------------------+

USHORT QLDBCompare2Nodes
(
  PQLDB_HTREE  phTree,                           // handle to tree
  PQLDB_NODE   pNode1,                           // first compare node
  PQLDB_NODE   pNode2,                           // second compare node
  BOOL         *pfNodesAreSame                   // TRUE = nodes are same
                                                 // FALSE = they are not
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pfNodesAreSame = FALSE;

  /********************************************************************/
  /*               check if the nodes are the same node               */
  /********************************************************************/
  if ( pNode1 == pNode2 )
  {
    usRC = QLDB_ERROR_IN_TREE;
  } /* endif */

  /********************************************************************/
  /*          check if the levels of the nodes are the same           */
  /********************************************************************/
  if ( pNode1->usLevel != pNode2->usLevel )
  {
    usRC = QLDB_ERROR_IN_TREE;
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*     now loop over the fields of the nodes and compare them     */
    /******************************************************************/
    *pfNodesAreSame = TRUE;

    for ( i = 0; ( i < phTree->ausNoOfFields[pNode1->usLevel-1] ) &&
                 *pfNodesAreSame; i++ )
    {
      if ( UTF16strcmp( pNode1->aFields[i].pszData,
                   pNode2->aFields[i].pszData ) != 0 )
      {
        *pfNodesAreSame = FALSE;
      } /* endif */
    } /* endfor */
  } /* endif */

  return( usRC );

} /* end of function QLDBCompare2Nodes */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBComputeFirstFieldOffset                                  |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBComputeFirstFieldOffset( phTree, usLevel, pusOffset )          |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function computes the offset of the first preallocated data          |
//|  field in the node from the beginning of the node on (in bytes)            |
//|                                                                            |
//|  If the node contains no fields at all the offset is set to 0.             |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//|  USHORT       usLevel;                          // level on which node     |
//|                                                 // is located              |
//|                                                 // for which offset is     |
//|                                                 // computed                |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  USHORT       *pusOffset;                       // pointer to computed     |
//|                                                 // offset variable         |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects: none                                                          |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBComputeFirstFieldOffset( phTree, 2, &usOffset );          |
//|                                                                            |
//|  This computes the offset of the first field in a second level node.       |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Compute the node offset                                                   |
//+----------------------------------------------------------------------------+

USHORT QLDBComputeFirstFieldOffset(

  PQLDB_HTREE  phTree,                           // handle to tree
  USHORT       usLevel,                          // level on which node
                                                 // is located
                                                 // for which offset is
                                                 // computed
  USHORT       *pusOffset )                      // pointer to computed
                                                 // offset variable

{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code



  if ( phTree->ausNoOfFields[usLevel-1] != 0 )
  {
    *pusOffset = sizeof( QLDB_NODE) +
                 ( phTree->ausNoOfFields[usLevel-1] - 1 ) *
                 sizeof( QLDB_FIELD );
  }
  else
  {
    *pusOffset = 0;
  } /* endif */

  return( usRC );

} /* end of function QLDBComputeFirstFieldOffset */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBComputeSubtreeToDestroy                                  |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBComputeSubtreeToDestroy( pNode, ppValidSubtreeNode )           |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This is a recursive function which computes the node of which the         |
//|  subtree forms a valid subtree to destroy.                                 |
//|                                                                            |
//|  If the input node has no sisters a destruction of the node would lead to  |
//|  an invalid template because the parent node of the node destroyed has no  |
//|  more children. Therefore a parent node of this node has to be found       |
//|  which has sister nodes. The so found node now forms together with its     |
//|  children nodes a valid subtree to destroy, because the parent node now    |
//|  has at least one more child (the sister node of the node found).          |
//|                                                                            |
//|  If the function finds a valid node this node is returned.                 |
//|                                                                            |
//|  The function will stop if a node without parent node is found.            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_NODE   pNode;                            // pointer to node         |
//|                                                 // which is to be checked  |
//|                                                 // if it forms a valid     |
//|                                                 // subtree to destroy      |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PQLDB_NODE   *ppValidSubtreeNode;              // pointer to node of      |
//|                                                 // which the subtree       |
//|                                                 // forms a valid subtree   |
//|                                                 // to destroy              |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid node pointer                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBComputeSubtreeToDestroy( phTree->pCurNode, &pTempNode );  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF input pointer != NULL pointer                                          |
//|    IF pRight of input pointer != NULL pointer                              |
//|      Set output pointer to input pointer                                   |
//|    ELSE                                                                    |
//|      IF pLeft of returned node pointer != NULL pointer                     |
//|        Set output pointer to input pointer                                 |
//|      ELSE                                                                  |
//|        IF pParent of input pointer != NULL pointer                         |
//|          CALL QLDBComputeSubtreeToDestroy with pParent of                  |
//|            input pointer                                                   |
//|          Set output pointer to returned pointer                            |
//|        ELSE                                                                |
//|          Set output pointer to input pointer                               |
//+----------------------------------------------------------------------------+

USHORT QLDBComputeSubtreeToDestroy
(
  PQLDB_NODE   pNode,                            // pointer to node
                                                 // which is to be checked
                                                 // if it forms a valid
                                                 // subtree to destroy
  PQLDB_NODE   *ppValidSubtreeNode               // pointer to node of
                                                 // which the subtree
                                                 // forms a valid subtree
                                                 // to destroy
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code



  if ( pNode != NULL )
  {
    /******************************************************************/
    /*   if a node to the right exists then the input pointer forms   */
    /*                   a valid subtree to destroy                   */
    /******************************************************************/
    if ( pNode->pRight != NULL )
    {
      *ppValidSubtreeNode = pNode;
    }
    else
    {
      /****************************************************************/
      /*  if a node to the right exists then the input pointer forms  */
      /*                  a valid subtree to destroy                  */
      /****************************************************************/
      if ( pNode->pLeft != NULL )
      {
        *ppValidSubtreeNode = pNode;
      }
      else
      {
        /**************************************************************/
        /*   now we have to check if the parent node forms a valid    */
        /*                     subtree to destroy                     */
        /**************************************************************/
        if ( pNode->pParent != NULL )
        {
          usRC = QLDBComputeSubtreeToDestroy( pNode->pParent,
                                              ppValidSubtreeNode );
        }
        else
        {
          *ppValidSubtreeNode = pNode;
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBComputeSubtreeToDestroy */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBConvertRecord                                            |
//+----------------------------------------------------------------------------+
//|Function call: QLDBConvertRecord( pchrLDBRecord, usLDBRecLength,            |
//|                                  ausNoOfFields, &pchrQLDBRecord,           |
//|                                  &usQLDBRecLength ):                       |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function converts an old LDB record to a new QLDB record format.     |
//+----------------------------------------------------------------------------+
//|Input parameter: PCHAR    pchrLDBRecord       pointer to old LDB record     |
//|                 USHORT   usLDBRecLength      length of old LDB record      |
//|                 USHORT   ausNoOfFields[]     number of fields per level    |
//+----------------------------------------------------------------------------+
//|Output parameter: PCHAR   *ppchrQLDBRecord    pointer to converted record   |
//|                  USHORT  *pusQLDBRecLength   length of converted record    |
//+----------------------------------------------------------------------------+
//|Returncode type:  USHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:      QLDB_NO_ERROR         record converted successfully       |
//|                  QLDB_NO_VALID_DATA    old LDB record is corrupted         |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|  Get total number of fields                                                |
//|  check data of LDB record in old format and compute size of new record     |
//|  allocate space for converted LDB record                                   |
//|  store control information in new record                                   |
//|  LOOP over all nodes of old record                                         |
//|    IF node is a data node                                                  |
//|      copy field data to new record and add escape characters if necessary  |
//|    ELSE                                                                    |
//|      add level identifier to new record if not on entry level              |
//|  add record end character                                                  |
//+----------------------------------------------------------------------------+
USHORT QLDBConvertRecord
(
  PCHAR        pchrLDBRecord,          // pointer to old LDB record
  ULONG        ulLDBRecLength,         // length of old LDB record
  USHORT       ausNoOfFields[],        // number of fields per level array
  PCHAR        *ppchrQLDBRecord,       // pointer to converted LDB record
  ULONG        *pulQLDBRecLength       // length of converted LDB record
)
{
  USHORT     usNodeCount;              // number of nodes in dictionary record
  PUCHAR     pucField;                 // ptr to current field
  PUCHAR     pucFieldData;             // ptr to start of field data
  PUCHAR     pucOutPos;                // ptr for processing of output record
  PUSHORT    pusNode;                  // ptr to current node
  USHORT     usNodeIndex;              // LDB code name converted to index
  USHORT     usI;                      // general loop index
  USHORT     usNoOfFields = 0;         // number of fields in dictionary
  USHORT     usNewLen= 0;              // length of converted LDB record
  USHORT     usRC     = QLDB_NO_ERROR; // function return code
  USHORT     usLevel  = 0;             // current tree level
  USHORT     usFieldsExpected = 0;     // number of fields expected for level
  USHORT     usFieldLen;               // length of a field
  PUSHORT    pusEscFields;             // pointer to number of escape fields
  BOOL       fFirstEscapeChar;         // TRUE = no esc character for field yet

  *pulQLDBRecLength = ulLDBRecLength;

  /********************************************************************/
  /* Get total number of fields                                       */
  /********************************************************************/
  usNoOfFields = 0;
  for ( usI = 0; usI < QLDB_MAX_LEVELS; usI++ )
  {
    usNoOfFields = usNoOfFields + ausNoOfFields[usI];
  } /* endfor */

  /********************************************************************/
  /* check data of LDB record in old format and compute size of       */
  /* converted LDB record                                             */
  /********************************************************************/
  pusNode      = (PUSHORT)pchrLDBRecord;    // get start of nodes
  usNodeCount  = pusNode[1];                // get node count = first offset
  pucFieldData = (PUCHAR)(pchrLDBRecord +            // get start of field data
                 (usNodeCount * sizeof(USHORT) * 2));
  if ( pucFieldData >=  (PUCHAR)(pchrLDBRecord + ulLDBRecLength ))
  {
    /******************************************************************/
    /* Record data is invalid                                         */
    /******************************************************************/
    usRC = QLDB_NO_VALID_DATA;
  }
  else
  {
    usNodeCount--;                     // adjust actual number of nodes
    usNewLen = 1 + 2 + 1;              // reserve space for level identifier,
                                       // escape field number and
                                       // record end character
    while ( usNodeCount && *pusNode && ( usRC == QLDB_NO_ERROR ) )
    {
      usNodeIndex = INDEXOFCODE(pusNode);
      if ( ISDATANODE(pusNode) )
      {
        /**************************************************************/
        /* Check if number of fields on current level has been        */
        /* exceeded                                                   */
        /**************************************************************/
        if ( usFieldsExpected == 0  )  // no more fields to come ...
        {
          /************************************************************/
          /* more fields on current level as expected                 */
          /************************************************************/
          usRC = QLDB_NO_VALID_DATA;
        }
        else
        {
          if ( ausNoOfFields[usLevel] )  // if current level has fields ...
          {
            /************************************************************/
            /* address field data and add length of field data plus     */
            /* one (for field end delimiter) to length of converted     */
            /* LDB record.                                              */
            /************************************************************/
            pucField      = pucFieldData + pusNode[1];
            usFieldLen    = *((PUSHORT)pucField);
            usNewLen     += usFieldLen + 1;
            pucField     += sizeof(USHORT);

            /**********************************************************/
            /* Add space required for escape characters.              */
            /**********************************************************/
            while ( usFieldLen )
            {
              if ( *pucField <= QLDB_FOURTH_LEVEL )
              {
                usNewLen++;
              } /* endif */
              pucField++;
              usFieldLen--;
            } /* endwhile */
          } /* endif */
          usFieldsExpected--;
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /* check level completed status and level of new terminal node*/
        /**************************************************************/
        if ( usFieldsExpected )
        {
          /************************************************************/
          /* Not all fields of the previous level were found          */
          /************************************************************/
          usRC = QLDB_NO_VALID_DATA;
        }
        else if ( usNodeIndex < 1 )
        {
          /************************************************************/
          /* entry level of old LDB record: ignore the node           */
          /************************************************************/
        }
        else if ( usNodeIndex <= QLDB_MAX_LEVELS )
        {
          /************************************************************/
          /* headword level, pos level, sense level or target level   */
          /* of old LDB record: convert level to zero based level     */
          /* index                                                    */
          /************************************************************/
          usLevel = usNodeIndex - 1;

          /************************************************************/
          /* preset number of fields expected for this level.         */
          /* For empty levels we expect one field (the dummy field    */
          /* required in the old LDB approach for empty levels)       */
          /************************************************************/
          usFieldsExpected = ausNoOfFields[usLevel];
          if ( !usFieldsExpected )
          {
            usFieldsExpected = 1;
          } /* endif */

          /************************************************************/
          /* add space required for level identifier to length of     */
          /* converted LDB record                                     */
          /************************************************************/
          usNewLen++;
        }
        else
        {
          /************************************************************/
          /* Invalid level value. LDB record may be corrupted         */
          /************************************************************/
          usRC = QLDB_NO_VALID_DATA;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Continue with next node of old LDB record                    */
      /****************************************************************/
      pusNode += 2;
      usNodeCount--;
    } /* endwhile */

    /******************************************************************/
    /* Check if all fields of last level were found                   */
    /******************************************************************/
    if ( usRC == QLDB_NO_ERROR )
    {
      if ( usFieldsExpected )
      {
        /************************************************************/
        /* Not all fields of the previous level were found          */
        /************************************************************/
        usRC = QLDB_NO_VALID_DATA;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* allocate space for converted LDB record                          */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    UtlAlloc( (PVOID *)ppchrQLDBRecord, 0L,
              (LONG)max( MIN_ALLOC,  usNewLen), ERROR_STORAGE );
    if ( !*ppchrQLDBRecord )
    {
      usRC = QLDB_NO_MEMORY;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* create converted LDB record                                      */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    *pulQLDBRecLength = usNewLen;
    pucOutPos = (PUCHAR)(*ppchrQLDBRecord);      // set start position for output
    pusNode      = (PUSHORT)pchrLDBRecord;    // get start of nodes
    usNodeCount  = pusNode[1];                // get node count = first offset
    pucFieldData = (PUCHAR)(pchrLDBRecord +            // get start of field data
                 (usNodeCount * sizeof(USHORT) * 2));

    /******************************************************************/
    /* fill-in header data                                            */
    /******************************************************************/
    *pucOutPos++ = (UCHAR)QLDB_FIRST_LEVEL;
    pusEscFields = (PUSHORT)pucOutPos;
    *pucOutPos++ = 0;                    // number of escape character fields
    *pucOutPos++ = 0;

    usNodeCount--;                     // adjust actual number of nodes
    while ( usNodeCount && *pusNode && ( usRC == QLDB_NO_ERROR ) )
    {
      if ( ISDATANODE(pusNode) )
      {
        if ( usFieldsExpected )
        {
          /**************************************************************/
          /* get length of field and pointer to field start             */
          /**************************************************************/
          pucField  = pucFieldData + pusNode[1];
          usI       = *((PUSHORT)pucField);
          pucField += sizeof(USHORT);

          /**************************************************************/
          /* copy field data                                            */
          /**************************************************************/
          fFirstEscapeChar = TRUE;
          while ( usI )
          {
            if ( *pucField <= QLDB_FOURTH_LEVEL )
            {
              *pucOutPos++ = QLDB_ESC_CHAR;
              if ( fFirstEscapeChar )
              {
                *pusEscFields += 1;
                fFirstEscapeChar = FALSE;
              } /* endif */
            } /* endif */
            *pucOutPos++ = *pucField++;
            usI--;
          } /* endwhile */

          /**************************************************************/
          /* terminate the field                                        */
          /**************************************************************/
          *pucOutPos++ = (UCHAR)QLDB_FIELD_DELIMITER;
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /* add level identifier to output record for levels all       */
        /* levels but entry level                                     */
        /**************************************************************/
        usLevel = INDEXOFCODE(pusNode);
        if ( usLevel )
        {
          usFieldsExpected = ausNoOfFields[usLevel-1];

          switch ( usLevel )
          {
            case 1 :
              *pucOutPos++ = (UCHAR)QLDB_FIRST_LEVEL;
              break;
            case 2 :
              *pucOutPos++ = (UCHAR)QLDB_SECOND_LEVEL;
              break;
            case 3 :
              *pucOutPos++ = (UCHAR)QLDB_THIRD_LEVEL;
              break;
            case 4 :
            default :
              *pucOutPos++ = (UCHAR)QLDB_FOURTH_LEVEL;
              break;
          } /* endswitch */
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* Continue with next node of old LDB record                    */
      /****************************************************************/
      pusNode += 2;
      usNodeCount--;
    } /* endwhile */

    /******************************************************************/
    /* add record end character                                       */
    /******************************************************************/
    *pucOutPos++ = (UCHAR)QLDB_END_OF_REC;
  } /* endif */

  return( usRC );

} /* end of function QLDBConvertRecord */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBCopyNode                                                 |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBCopyNode( phTree, pSourceNode, ppTargetNode )                  |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function creates a full copy of a node. The target node is           |
//|  filled with the same information as the source node, i.e. the target      |
//|  node contains the same data in its field and references the same          |
//|  parent, right, left and child nodes.                                      |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // pointer to the tree     |
//|  PQLDB_NODE   pSourceNode;                      // pointer to source       |
//|                                                 // node, from where        |
//|                                                 // it is copied            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PQLDB_NODE   *ppTargetNode;                    // pointer to copy of      |
//|                                                 // the source node         |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle                                                     |
//|  - a valid node                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  creates a copy of the source node                                         |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBCopyNode( &hTree, hTree.pCurNode, &pTempNode );           |
//|                                                                            |
//|  This creates a copy of the current node in pTempNode.                     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Check if valid input parameters were entered                              |
//|  Create an array of pointers for the data                                  |
//|  Create the target node on the same level (QLDBCreateNode)                 |
//|  Copy the parent, right, left and child pointers to the target node        |
//+----------------------------------------------------------------------------+

USHORT QLDBCopyNode
(
  PQLDB_HTREE  phTree,                           // pointer to the tree
  PQLDB_NODE   pSourceNode,                      // pointer to source
                                                 // node, from where
                                                 // it is copied
  PQLDB_NODE   *ppTargetNode                     // pointer to copy of
                                                 // the source node
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  USHORT       usFields;                         // no. of fields in
                                                 // the node
  PSZ_W        *ppszData;                        // pointer array to the
                                                 // data fields



  /********************************************************************/
  /*                    set output pointer to NULL                    */
  /********************************************************************/
  *ppTargetNode = NULL;
  ppszData = NULL;

  /********************************************************************/
  /*               get the number of fields in the node               */
  /********************************************************************/
  usFields = phTree->ausNoOfFields[(pSourceNode->usLevel)-1];

  /********************************************************************/
  /*     allocate an array of pointers for the fields in the node     */
  /********************************************************************/
  if ( usFields != 0 )
  {
    if ( !( UtlAlloc( (PVOID *)&ppszData, 0L,
                      (LONG)max( MIN_ALLOC, usFields * sizeof( PSZ_W ) ),
                      NOMSG ) ) )
    {
      usRC = QLDB_NO_MEMORY;
    } /* endif */
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*    now allocate for each field the amount of storage needed    */
    /*  and copy the data from the source node to the pointer array   */
    /******************************************************************/
    for ( i = 0; ( i < usFields ) && ( usRC == QLDB_NO_ERROR ); i++ )
    {
      if ( !( UtlAlloc( (PVOID *)&(ppszData[i]), 0L,
                        (LONG)
                        max( MIN_ALLOC,
                             UTF16strlenBYTE( pSourceNode->aFields[i].pszData ) + sizeof(CHAR_W) ),
                        NOMSG ) ) )
      {
        usRC = QLDB_NO_MEMORY;
      }
      else
      {
        UTF16strcpy( ppszData[i], pSourceNode->aFields[i].pszData );
      } /* endif */
    } /* endfor */

    if ( usRC == QLDB_NO_ERROR )
    {
      /****************************************************************/
      /*                    create the target node                    */
      /****************************************************************/
      usRC = QLDBCreateNode( phTree, pSourceNode->usLevel,
                             ppszData, ppTargetNode );

      if ( usRC == QLDB_NO_ERROR )
      {
        /**************************************************************/
        /*         copy the parent, sister and child pointer          */
        /**************************************************************/
        (*ppTargetNode)->pParent = pSourceNode->pParent;
        (*ppTargetNode)->pLeft = pSourceNode->pLeft;
        (*ppTargetNode)->pRight = pSourceNode->pRight;
        (*ppTargetNode)->pChild = pSourceNode->pChild;
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBCopyNode */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBCopyNodeToRecord                                         |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBCopyNodeToRecord( phTree, pchrRecord, pusRecCount )            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function copies the fields from the current node to the record.      |
//|  If any QLDB control characters are found in one of the fields             |
//|  the field data is extended by an QLDB_ESC_CHAR character before           |
//|  each control character.                                                   |
//|                                                                            |
//|  If the function returns an error nothing is copied to the record and      |
//|  the record counter remains unchanged.                                     |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//|  PCHAR        pchrRecord;                       // pointer to the          |
//|                                                 // record to which         |
//|                                                 // to copy                 |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  USHORT       *pusRecCount;                     // record counter          |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the data from the fields is copied to the record                        |
//|  - the record counter is increased by the number of bytes copied           |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBCopyNodeToRecord( phTree, pchrRecord, &usRecCount );      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Check the validity of the tree handle                                     |
//|  Copy the level information to the record                                  |
//|  FOR 1 to usNOOfFields DO                                                  |
//|    IF a control character is in the field                                  |
//|      Copy the data from the field to the record and copy a                 |
//|        QLDB_ESC_CHAR before any control character                          |
//|    ELSE                                                                    |
//|      Copy the data field to the record                                     |
//+----------------------------------------------------------------------------+

USHORT QLDBCopyNodeToRecord
(
  PQLDB_HTREE  phTree,                           // handle to tree
  PSZ_W        pchrRecord,                       // pointer to the
                                                 // record to which
                                                 // to copy
  USHORT       *pusRecCount                      // record counter
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  USHORT       j;                                // another index
  BOOL         fCtrlCharsInField;                // are there control
                                                 // characters in the
                                                 // field ?



  /********************************************************************/
  /*               copy level information to the record               */
  /********************************************************************/
  pchrRecord[*pusRecCount] = (CHAR_W)( phTree->pCurNode->usLevel - 1 +
                             QLDB_FIRST_LEVEL - QLDB_FIELD_DELIMITER );

  (*pusRecCount)++;

  /********************************************************************/
  /*    now loop over the fields and copy the data from the fields    */
  /*                          to the record                           */
  /*     if a field contains one or more control characters than      */
  /*   process the field character by character otherwise copy the    */
  /*                       field to the record                        */
  /********************************************************************/
  for ( i = 0; i < phTree->ausNoOfFields[phTree->pCurNode->usLevel-1];
        i++ )
  {
    if ( UTF16strchr( phTree->pCurNode->aFields[i].pszData,
                 (CHAR_W)( QLDB_ESC_CHAR ) ) != NULL )
    {
      fCtrlCharsInField = TRUE;
    }
    else
    {
      if ( UTF16strchr( phTree->pCurNode->aFields[i].pszData,
                   (CHAR_W)( QLDB_END_OF_REC ) ) != NULL )
      {
        fCtrlCharsInField = TRUE;
      }
      else
      {
        if ( UTF16strchr( phTree->pCurNode->aFields[i].pszData,
                     (CHAR_W)( QLDB_FIRST_LEVEL ) ) != NULL )
        {
          fCtrlCharsInField = TRUE;
        }
        else
        {
          if ( UTF16strchr( phTree->pCurNode->aFields[i].pszData,
                       (CHAR_W)( QLDB_SECOND_LEVEL ) ) != NULL )
          {
            fCtrlCharsInField = TRUE;
          }
          else
          {
            if ( UTF16strchr( phTree->pCurNode->aFields[i].pszData,
                         (CHAR_W)( QLDB_THIRD_LEVEL ) ) != NULL )
            {
              fCtrlCharsInField = TRUE;
            }
            else
            {
              if ( UTF16strchr( phTree->pCurNode->aFields[i].pszData,
                           (CHAR_W)( QLDB_FOURTH_LEVEL ) ) != NULL )
              {
                fCtrlCharsInField = TRUE;
              }
              else
              {
                fCtrlCharsInField = FALSE;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    if ( fCtrlCharsInField == TRUE )
    {
      for ( j = 0; j < UTF16strlenCHAR( phTree->pCurNode->aFields[i].pszData );
            j++ )
      {
        switch ( phTree->pCurNode->aFields[i].pszData[j] )
        {
          case (CHAR_W)( QLDB_ESC_CHAR ) :
          case (CHAR_W)( QLDB_END_OF_REC ) :
          case (CHAR_W)( QLDB_FIRST_LEVEL ) :
          case (CHAR_W)( QLDB_SECOND_LEVEL ) :
          case (CHAR_W)( QLDB_THIRD_LEVEL ) :
          case (CHAR_W)( QLDB_FOURTH_LEVEL ) :
            {
              pchrRecord[*pusRecCount] = (CHAR_W)( QLDB_ESC_CHAR );
              (*pusRecCount)++;
              pchrRecord[*pusRecCount] =
                phTree->pCurNode->aFields[i].pszData[j];
              (*pusRecCount)++;
            }
          break;
          default :
            {
              pchrRecord[*pusRecCount] =
                phTree->pCurNode->aFields[i].pszData[j];
              (*pusRecCount)++;
            }
          break;
        } /* endswitch */
      } /* endfor */

      /**************************************************************/
      /*        copy the QLDB_FIELD_DELIMITER to the record         */
      /**************************************************************/
      pchrRecord[*pusRecCount] = (CHAR_W)( QLDB_FIELD_DELIMITER );
      (*pusRecCount)++;
    }
    else
    {
      UTF16strcpy( &pchrRecord[*pusRecCount],
              phTree->pCurNode->aFields[i].pszData );
      *pusRecCount = *pusRecCount +
        (USHORT)(UTF16strlenCHAR( phTree->pCurNode->aFields[i].pszData ) + 1);
    } /* endif */
  } /* endfor */

  return( usRC );

} /* end of function QLDBCopyNodeToRecord */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBCreateNode                                               |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBCreateNode( phTree, usLevel, ppszData, pNode )                 |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function creates a node and fills in the data for the node on the    |
//|  specified input level.                                                    |
//|                                                                            |
//|  The data is to be supplied in an array of pointers referencing            |
//|  zero-terminated strings (PSZ).                                            |
//|                                                                            |
//|  The function expects a valid level number (between 1 and QLDB_MAX_LEVELS).|
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // pointer to the tree     |
//|                                                 // handle structure        |
//|  USHORT       usLevel;                          // level number on         |
//|                                                 // which the node is       |
//|                                                 // created                 |
//|  PSZ          *ppszData;                        // pointer to an           |
//|                                                 // array of pointers       |
//|                                                 // to data to be filled    |
//|                                                 // in the node             |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PQLDB_NODE   *pNode;                           // pointer to created      |
//|                                                 // node                    |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle is required                                         |
//|  - a valid level number is required                                        |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  a new node is created                                                     |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBCreateNode( phTree, 3, ppszData, &pNode );                |
//|                                                                            |
//|  This creates a third-level node pNode and fills in the data in the        |
//|  from the pointer array ppszData.                                          |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|   Compute the amount of storage needed for variable part (QLDBNodeStorage) |
//|   Allocate computed storage                                                |
//|   IF storage allocation went ok                                            |
//|     Set usLevel to input level number                                      |
//|     FOR 1 TO usNoOfFields DO                                               |
//|       Set fDataInNode in field array to TRUE                               |
//|       IF (length of data >= FIELD_SIZE)                                    |
//|         Allocate storage for the data                                      |
//|         Change fDataInNode in field array to FALSE                         |
//|       Set pointer to data in field array                                   |
//|       Copy data to field processed (QLDBFillData)                          |
//|   RETURN pointer to created node                                           |
//+----------------------------------------------------------------------------+

USHORT QLDBCreateNode(

  PQLDB_HTREE  phTree,                           // pointer to the tree
                                                 // handle structure
  USHORT       usLevel,                          // level number on
                                                 // which the node is
                                                 // created
  PSZ_W        *ppszData,                        // pointer to an
                                                 // array of pointers
                                                 // to data to be filled
                                                 // in the node
  PQLDB_NODE   *pNode )                          // pointer to created
                                                 // node
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  ULONG        ulNodeStorage;                    // storage needed for
                                                 // allocating a node
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  USHORT       usOffset;                         // offset of the first
                                                 // preallocated data
                                                 // field



  /********************************************************************/
  /*                    set output pointer to NULL                    */
  /********************************************************************/
  *pNode = NULL;

  /********************************************************************/
  /*          compute the amount of storage needed for node           */
  /********************************************************************/
  usRC = QLDBNodeStorage( phTree, usLevel, &ulNodeStorage );
  if ( usRC == QLDB_NO_ERROR )
  {
    if ( UtlAlloc( (PVOID *)pNode, 0L, ulNodeStorage, NOMSG ) )
    {
      /****************************************************************/
      /*                   set level number in node                   */
      /****************************************************************/
      (*pNode)->usLevel = usLevel;

      /****************************************************************/
      /*         set the pointers to the other nodes to NULL          */
      /****************************************************************/
      (*pNode)->pParent = NULL;
      (*pNode)->pChild  = NULL;
      (*pNode)->pLeft   = NULL;
      (*pNode)->pRight  = NULL;

      /****************************************************************/
      /*   compute the offset of the first preallocated data field    */
      /*            but only if the no. of fields is not 0            */
      /*    if it 0 set the program control variable fOK to FALSE     */
      /*         in order finish the function and not to copy         */
      /*                     any data in the node                     */
      /****************************************************************/
      if ( phTree->ausNoOfFields[usLevel-1] != 0 )
      {
        QLDBComputeFirstFieldOffset( phTree, usLevel, &usOffset );

        /**************************************************************/
        /*        now copy the data into the allocated fields         */
        /*   if the data is larger than QLDB_FIELD_SIZE allocate a    */
        /*   seperate data area in the size of the data length and    */
        /*           copy the data to the created data area           */
        /**************************************************************/
        for ( i = 0; ( i < phTree->ausNoOfFields[usLevel-1] ) &&
                     ( usRC == QLDB_NO_ERROR ); i++ )
        {
          /************************************************************/
          /*   indicate that data for that field is in preallocated   */
          /*                data area and not external                */
          /************************************************************/
          (*pNode)->aFields[i].fDataInNode = TRUE;

          /************************************************************/
          /*   check if data length is larger than QLDB_FIELD_SIZE    */
          /************************************************************/
          if ( UTF16strlenBYTE( ppszData[i] ) >= QLDB_FIELD_SIZE )
          {
            /**********************************************************/
            /*    allocate the storage for the external data area     */
            /**********************************************************/
            if ( UtlAlloc( (PVOID *)&( (*pNode)->aFields[i].pszData ), 0L,
                           (LONG)( UTF16strlenBYTE( ppszData[i] ) + sizeof(CHAR_W) ),
                           NOMSG ) )
            {
              /********************************************************/
              /* indicate that field data is now in external data area*/
              /********************************************************/
              (*pNode)->aFields[i].fDataInNode = FALSE;
            }
            else
            {
              /********************************************************/
              /*        destroy the node and set returncode to        */
              /*                    QLDB_NO_MEMORY                    */
              /********************************************************/
              QLDBDestroyNode( phTree, pNode );
              *pNode = NULL;
              usRC = QLDB_NO_MEMORY;
            } /* endif */
          }
          else
          {
            /**********************************************************/
            /*         set pointer to preallocated data field         */
            /**********************************************************/
            if ( phTree->ausNoOfFields[usLevel-1] != 0 )
            {
              (*pNode)->aFields[i].pszData = (PSZ_W)((PSZ)( *pNode ) +
                                             usOffset +
                                             i * QLDB_FIELD_SIZE);
            } /* endif */
          } /* endif */

          /************************************************************/
          /*                  copy data to the field                  */
          /************************************************************/
          if ( usRC == QLDB_NO_ERROR )
          {
            /**********************************************************/
            /*     check if the pointer to the data fields exists     */
            /*     if not call QLDBFillData with an empty string      */
            /**********************************************************/
            if ( ppszData[i] != NULL )
            {
              usRC = QLDBFillData( phTree, *pNode, (USHORT)(i + 1), ppszData[i] );
            }
            else
            {
              usRC = QLDBFillData( phTree, *pNode, (USHORT)(i + 1),
                                   QLDB_EMPTY_STRINGW );
            } /* endif */

            if ( usRC != QLDB_NO_ERROR )
            {
              /********************************************************/
              /*        destroy the node and set returncode to        */
              /*             returncode from QLDBFillData             */
              /********************************************************/
              QLDBDestroyNode( phTree, pNode );
              *pNode = NULL;
            } /* endif */
          } /* endif */
        } /* endfor */
      }
      else
      {
        (*pNode)->aFields[0].pszData = NULL;
      } /* endif */
    }
    else
    {
      usRC = QLDB_NO_MEMORY;
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBCreateNode */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBCreateTreeHandle                                         |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBCreateTreeHandle( ausNoOfFields, pphTree )                     |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function allocates the tree handle structure and sets the initial    |
//|  initial values in the tree handle structure.                              |
//|                                                                            |
//|  If any problems occurr the pointer to the tree handle is set to NULL      |
//|  and the returncode is set according to the problem.                       |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  USHORT       ausNoOfFields[];                  // no. of fields on        |
//|                                                 // each level              |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PQLDB_HTREE  *pphTree;                         // pointer to a pointer    |
//|                                                 // to the tree handle      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_TOO_MANY_FIELDS        - too many fields for a level were requested  |
//+----------------------------------------------------------------------------+
//|Prerequesits: none                                                          |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  the tree handle structure is allocated and filled with initial values     |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|  PQLDB_HTREE  phTree;                                                      |
//|  USHORT       ausFields[QLDB_MAX_LEVELS] = { 2, 3, 1, 5 };                 |
//|                                                                            |
//|  usRetCode = QLDBCreateTreeHandle( ausFields, &phTree );                   |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Allocate storage for handle structure                                     |
//|  IF storage allocation went ok                                             |
//|    Set fWorkWithRecord in tree handle to FALSE                             |
//|    Set the current node and record pointers                                |
//|    Set the current level pointers                                          |
//|    Set no. of fields on each level                                         |
//|    Set the values in ausStorageOnLevel to NO_STORAGE_COMPUTED (-1)         |
//|  RETURN pointer to handle structure                                        |
//+----------------------------------------------------------------------------+

USHORT QLDBCreateTreeHandle
(
  USHORT       ausNoOfFields[],                  // no. of fields on
                                                 // each level
  PQLDB_HTREE  *pphTree                          // pointer to a pointer
                                                 // to the tree handle
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pphTree = NULL;

  /********************************************************************/
  /*          allocate storage for the tree handle structure          */
  /********************************************************************/
  if ( !( UtlAlloc( (PVOID *)pphTree, 0L, (LONG)( sizeof( QLDB_HTREE ) ),
                    NOMSG ) ) )
  {
    usRC = QLDB_NO_MEMORY;
  } /* endif */


  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*      set the indicator that the tree is a record to FALSE      */
    /******************************************************************/
    (*pphTree)->fWorkWithRecord = FALSE;

    /******************************************************************/
    /*        set the current node and record pointers to NULL        */
    /******************************************************************/
    (*pphTree)->pCurNode  = NULL;
    (*pphTree)->pszCurRec = NULL;

    /******************************************************************/
    /*     now set the level information to their initial values      */
    /******************************************************************/
    for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
          i++ )
    {
      /****************************************************************/
      /*        set the level node and record pointers to NULL        */
      /****************************************************************/
      (*pphTree)->apCurLevelNode[i] = NULL;
      (*pphTree)->apszCurLevelRec[i] = NULL;

      /****************************************************************/
      /*        set the storage for the nodes on each level to        */
      /*                       QLDB_NO_STORAGE                        */
      /****************************************************************/
      (*pphTree)->aulStorageOnLevel[i] = QLDB_NO_STORAGE;

      /****************************************************************/
      /*      copy the input number of fields to the tree handle      */
      /*  check if the maximum number of fields per level is overrun  */
      /*  if so set the function return code to QLDB_TOO_MANY_FIELDS  */
      /*                     and finish the loop                      */
      /****************************************************************/
      if ( ausNoOfFields[i] <= QLDB_MAX_LEVELFIELDS )
      {
        (*pphTree)->ausNoOfFields[i] = ausNoOfFields[i];
      }
      else
      {
        /**************************************************************/
        /*              deallocate the handle structure               */
        /**************************************************************/
        UtlAlloc( (PVOID *)pphTree, 0L, 0L, NOMSG );
        *pphTree = NULL;

        usRC = QLDB_TOO_MANY_FIELDS;
      } /* endif */
    } /* endfor */

    /******************************************************************/
    /*   set the no of fields containing QLDB_ESC_CHAR to 0 and the   */
    /*               pointer to the buffer area to NULL               */
    /******************************************************************/
    (*pphTree)->usEscFields = 0;
    (*pphTree)->usBufferStorage = 0;
    (*pphTree)->pchrEscBuffer = NULL;
  } /* endif */

  return( usRC );

} /* end of function QLDBCreateTreeHandle */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBDestroyIncompleteSubtree                                 |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBDestroyIncompleteSubtree( phTree, usLevel )                    |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function destroys a subtree which is not necessarily filled up       |
//|  to the last level node. The function stops destroying if no more          |
//|  child nodes can be found.                                                 |
//|                                                                            |
//|  The function takes no care about references to other node (apart from     |
//|  the child nodes), thus it is up to the calling function to set the        |
//|  right references.                                                         |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // pointer to the tree     |
//|                                                 // handle structure        |
//|  USHORT       usLevel;                          // level from which on     |
//|                                                 // the tree is to be       |
//|                                                 // destroyed               |
//+----------------------------------------------------------------------------+
//|Output parameter: none                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type:  USHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle and a valid node is required                          |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  all children of the current node of the input level and the node itself   |
//|  are destroyed                                                             |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|  usRetCode = QLDBDestroyIncompleteSubtree( phTree, 1 )                     |
//|                                                                            |
//|  This destroys the subtree starting from the first level node.             |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Set first node to destroy to the current node of the input level          |
//|  WHILE child of node to destroy exists DO                                  |
//|    Set next node to destroy to pChild of node to destroy                   |
//|    Destroy the node                                                        |
//+----------------------------------------------------------------------------+

USHORT QLDBDestroyIncompleteSubtree(

  PQLDB_HTREE  phTree,                           // pointer to the tree
                                                 // handle structure
  USHORT       usLevel )                         // level from which on
                                                 // the tree is to be
                                                 // destroyed
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  PQLDB_NODE   pDestroyNode;                     // pointer to node to
                                                 // destroy
  PQLDB_NODE   pNextDestroyNode;                 // pointer to next node
                                                 // to destroy



  /********************************************************************/
  /*     set the first node to destroy to the current node of the     */
  /*                           input level                            */
  /********************************************************************/
  pNextDestroyNode = phTree->apCurLevelNode[usLevel-1];

  /********************************************************************/
  /*   now loop until no more child nodes are available or an error   */
  /*                             occurred                             */
  /********************************************************************/
  while ( ( pNextDestroyNode != NULL ) && ( usRC == QLDB_NO_ERROR ) )
  {
    /******************************************************************/
    /*   set the node to destroy and set the next node to the child   */
    /*        node of the node which is going to be destroyed         */
    /******************************************************************/
    pDestroyNode = pNextDestroyNode;
    pNextDestroyNode = pNextDestroyNode->pChild;

    /******************************************************************/
    /*                        Destroy the node                        */
    /******************************************************************/
    usRC = QLDBDestroyNode( phTree, &pDestroyNode );
  } /* endwhile */

  return( usRC );

} /* end of function QLDBDestroyIncompleteSubtree */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBDestroyNode                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBDestroyNode( phTree, pNode )                                   |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function deallocates the storage that is used up by a node.          |
//|  First all external data fields will be deallocated and then the node      |
//|  structure is deallocated. The pointer to the node is then set to NULL.    |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // pointer to the tree     |
//|                                                 // handle structure        |
//|  PQLDB_NODE   *pNode;                           // pointer to node to      |
//|                                                 // destroy                 |
//+----------------------------------------------------------------------------+
//|Output parameter: none                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  the input tree handle must be a valid tree handle and the input node      |
//|  must be a valid one                                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  all external data fields in the node and the node itself are destroyed    |
//|  the pointer to the node is set to NULL                                    |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBDestroyNode( phTree, phTree->pCurNode );                  |
//|                                                                            |
//|  This would destroy the current node.                                      |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF pointer to node != NULL pointer                                        |
//|    FOR 1 TO usNoOfFields in the node DO                                    |
//|      IF fDataInNode == FALSE                                               |
//|        Deallocate external data area                                       |
//|    Deallocate node structure                                               |
//|    Set pointer to node to NULL pointer                                     |
//+----------------------------------------------------------------------------+

USHORT QLDBDestroyNode
(
  PQLDB_HTREE  phTree,                           // pointer to the tree
                                                 // handle structure
  PQLDB_NODE   *pNode                            // pointer to node to
                                                 // destroy
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index



  if ( *pNode != NULL )
  {
    /******************************************************************/
    /*     loop over the fields in the node and check if it is an     */
    /*  "external field, so that its data area has to be deallocated  */
    /******************************************************************/
    for ( i = 0; i < phTree->ausNoOfFields[(*pNode)->usLevel - 1]; i++ )
    {
      if ( (*pNode)->aFields[i].fDataInNode == FALSE )
      {
        /**************************************************************/
        /*             deallocate the external data area              */
        /**************************************************************/
        if ( (*pNode)->aFields[i].pszData != NULL )
        {
          UtlAlloc( (PVOID *)&( (*pNode)->aFields[i].pszData ), 0L, 0L, NOMSG );
        } /* endif */
      } /* endif */
    } /* endfor */

    /******************************************************************/
    /*                 deallocate the node structure                  */
    /******************************************************************/
    UtlAlloc( (PVOID *)pNode, 0L, 0L, NOMSG );
    *pNode = NULL;
  } /* endif */

  return( usRC );

} /* end of function QLDBDestroyNode */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBFillData                                                 |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBFillData( phTree, pNode, usFieldNo, pszData )                  |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function copies a zero-terminated string in the field specified by   |
//|  the input field number.                                                   |
//|                                                                            |
//|  If the field number cannot be found it returns QLDB_INVALID_FIELDNO and   |
//|  nothing is copied. The calling function has to assure that the field to   |
//|  which the data is to be copied has a storage large enough to fit the      |
//|  size of the data.                                                         |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // pointer to the tree     |
//|                                                 // handle structure        |
//|  PQLDB_NODE   pNode;                            // pointer to node to      |
//|                                                 // fill data in            |
//|  USHORT       usFieldNo;                        // No. of field in node    |
//|                                                 // to which data is to be  |
//|                                                 // copied                  |
//|  PSZ          pszData;                          // pointer to data         |
//|                                                 // (zero-terminated string)|
//+----------------------------------------------------------------------------+
//|Output parameter: none                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_VALID_DATA          - no valid data supplied                      |
//|  QLDB_INVALID_FIELDNO        - an invalid field number was entered         |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle                                                     |
//|  - a valid node                                                            |
//|  - an allocated data area in which data is to be copied                    |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  the data area is updated                                                  |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBFillData( phTree, pNode, 4, "TEST" );                     |
//|                                                                            |
//|  This copies "TEST" in field 4 of node pNode.                              |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Check if prerequesits for copying exist and are valid                     |
//|  Copy the data to data area referenced by pointer in characteristics array |
//+----------------------------------------------------------------------------+

USHORT QLDBFillData(

  PQLDB_HTREE  phTree,                           // pointer to the tree
                                                 // handle structure
  PQLDB_NODE   pNode,                            // pointer to node to
                                                 // fill data in
  USHORT       usFieldNo,                        // No. of field in node
                                                 // to which data is to be
                                                 // copied
  PSZ_W        pszData )                         // pointer to data
                                                 // (zero-terminated string)

{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code



  /********************************************************************/
  /*            check if a valid field number was entered             */
  /********************************************************************/
  if ( ( usFieldNo > phTree->ausNoOfFields[pNode->usLevel-1] ) ||
       ( usFieldNo < 1 ) )
  {
    usRC = QLDB_INVALID_FIELDNO;
  } /* endif */

  /********************************************************************/
  /*            check if a valid data pointer was entered             */
  /********************************************************************/
  if ( ( pszData == NULL ) && ( usRC == QLDB_NO_ERROR ) )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */

  /********************************************************************/
  /*              check if pointer to a data area exists              */
  /********************************************************************/
  if ( ( usRC == QLDB_NO_ERROR ) &&
       ( pNode->aFields[usFieldNo-1].pszData == NULL ) )
  {
    usRC = QLDB_INVALID_FIELDNO;
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*                 copy the data to the data area                 */
    /******************************************************************/
    UTF16strcpy( pNode->aFields[usFieldNo-1].pszData, pszData );
  } /* endif */

  return( usRC );

} /* end of function QLDBFillData */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBFilterEscChar                                            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBFilterEscChar( phTree, pusBufCount, ppszDataInRec, ppszData )  |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function eliminates all QLDB_ESC_CHARs from a data field.            |
//|                                                                            |
//|  The pointer to the data field changes as the function scans along the     |
//|  data field and will be set to the end of the data when the function       |
//|  has finished.                                                             |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // pointer to the tree     |
//|                                                 // handle structure        |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  USHORT       *pusBufCount;                     // counter for the         |
//|                                                 // esc field buffer        |
//|  PSZ          *ppszDataInRec;                   // pointer to the          |
//|                                                 // zero-terminated         |
//|                                                 // data field to be        |
//|                                                 // filtered                |
//|  PSZ          *ppszData;                        // pointer to the          |
//|                                                 // filtered data field     |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle                                                     |
//|  - a valid data field                                                      |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - changes ppszDataInRec so that it points to the end of the field         |
//|    when the function finishes                                              |
//|  - ppszData is set to the data field without QLDB_ESC_CHAR                 |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBFilterEscChar( hTree, &usBuffer, &pszRec, &pszData );     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF QLDB_ESC_CHAR is in data field                                         |
//|    IF buffer is full                                                       |
//|      allocate more storage for the buffer                                  |
//|    set the returned data pointer to the buffer                             |
//|    WHILE data field is not at its end                                      |
//|      IF current character is QLDB_ESC_CHAR                                 |
//|        move one character further                                          |
//|      copy the character to the buffer                                      |
//|  ELSE                                                                      |
//|    set the returned data pointer to the data field                         |
//+----------------------------------------------------------------------------+

USHORT QLDBFilterEscChar
(
  PQLDB_HTREE  phTree,                          // pointer to the tree
                                                 // handle structure
  USHORT       *pusBufCount,                     // counter for the
                                                 // esc field buffer
  PSZ_W        *ppszDataInRec,                   // pointer to the
                                                 // zero-terminated
                                                 // data field to be
                                                 // filtered
  PSZ_W        *ppszData                         // pointer to the
                                                 // filtered data field
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  PSZ_W        pchrTempEscBuffer;                // temporary esc field
                                                 // buffer pointer



  /********************************************************************/
  /*     check if a QLDB_ESC_CHAR can be found in the data field      */
  /********************************************************************/
  if ( UTF16strchr( *ppszDataInRec, (CHAR_W)( QLDB_ESC_CHAR ) ) != NULL )
  {
    /******************************************************************/
    /*        before moving the characters from the record to         */
    /*        the buffer check whether there is enough space          */
    /*                      left in the buffer                        */
    /*          if not allocate more storage for the buffer           */
    /*        and move the current data to the new allocated          */
    /*                          buffer area                           */
    /******************************************************************/
    if ( (*pusBufCount) + UTF16strlenCHAR(*ppszDataInRec) >
         phTree->usBufferStorage )
    {
      if ( (*pusBufCount) + UTF16strlenCHAR(*ppszDataInRec) >
           QLDB_MAX_REC_LENGTHW )
      {
        usRC = QLDB_NO_MEMORY;
      }
      else
      {
        LONG ulBytesToAlloc = 0L;
        /**************************************************************/
        /*        allocate the storage for the new buffer area        */
        /**************************************************************/
        pchrTempEscBuffer = phTree->pchrEscBuffer;
        ulBytesToAlloc = (LONG) ( sizeof(CHAR_W) *
                               ((*pusBufCount)+UTF16strlenCHAR(*ppszDataInRec))  );
        if ( !( UtlAlloc( (PVOID *)&(phTree->pchrEscBuffer), 0L,
                          ulBytesToAlloc, NOMSG ) ) )
        {
          usRC = QLDB_NO_MEMORY;
        }
        else
        {
          phTree->usBufferStorage = (*pusBufCount) +
                                    (USHORT)UTF16strlenCHAR( *ppszDataInRec );

          /************************************************************/
          /*         move the current data to the new buffer          */
          /************************************************************/
          memcpy( (PBYTE)phTree->pchrEscBuffer, (PBYTE)pchrTempEscBuffer,
                  ((*pusBufCount)+1) * sizeof(CHAR_W) );

          /************************************************************/
          /*              deallocate the old buffer area              */
          /************************************************************/
          if ( !( UtlAlloc( (PVOID *)&pchrTempEscBuffer, 0L, 0L, NOMSG ) ) )
          {
            usRC = QLDB_ERR_DEALLOC;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    if ( usRC == QLDB_NO_ERROR )
    {
      /****************************************************************/
      /*  set the data pointer to the appropriate area in the buffer  */
      /****************************************************************/
      *ppszData = &( phTree->pchrEscBuffer[(*pusBufCount)] );

      /****************************************************************/
      /*    now move the characters from the record to the buffer     */
      /****************************************************************/
      i = 0;
      while ( *(*ppszDataInRec) != (CHAR_W)( QLDB_FIELD_DELIMITER ) )
      {
        if ( *(*ppszDataInRec) == (CHAR_W)( QLDB_ESC_CHAR ) )
        {
          (*ppszDataInRec)++;
        } /* endif */

        (*ppszData)[i] = *(*ppszDataInRec);
        i++;
        (*pusBufCount)++;
        (*ppszDataInRec)++;
      } /* endwhile */

      (*ppszDataInRec)++;
      (*ppszData)[(*pusBufCount)] = QLDB_NULC;
      (*pusBufCount)++;
    } /* endif */
  }
  else
  {
    *ppszData = *ppszDataInRec;
    *ppszDataInRec += UTF16strlenCHAR( *ppszDataInRec ) + 1;
  } /* endif */

  return( usRC );

} /* end of function QLDBFilterEscChar */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBJoinSameSubtree                                          |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBJoinSameSubtree( phTree, pNode )                               |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function is a recursive function joining subtrees where root         |
//|  nodes have the same data in their nodes.                                  |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // pointer to the tree     |
//|                                                 // handle structure        |
//|  PQLDB_NODE   pNode;                            // pointer to node of      |
//|                                                 // which the subtree is    |
//|                                                 // joined                  |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_ERROR_IN_TREE          - an unrecoverable error is in the tree       |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle                                                     |
//|  - a valid pointer to a node having a subtree (a node is a subtree itself) |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  joins subtrees where root nodes are the same                              |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBJoinSameSubtrees( phTree, phTree->pCurNode );             |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Set check pointer to input pointer                                        |
//|  WHILE pRight of check pointer != NULL pointer                             |
//|    Set run pointer to pRight of check pointer                              |
//|    WHILE run pointer != NULL pointer                                       |
//|      Compare nodes of check pointer and run pointer                        |
//|      IF both nodes are the same                                            |
//|        Set temp pointer to run pointer                                     |
//|        Set run pointer to pLeft of run pointer                             |
//|        IF pRight of temp pointer != NULL pointer                           |
//|          Set pRight of run pointer to pRight of temp pointer               |
//|          Set pLeft of node right of run pointer to run pointer             |
//|        ELSE                                                                |
//|          Set pRight of run pointer to NULL pointer                         |
//|        IF pChild of check pointer != NULL pointer                          |
//|          Set join pointer to pChild of check pointer                       |
//|          WHILE pRight of join pointer != NULL pointer                      |
//|            Set join pointer to pRight of join pointer                      |
//|          Set pRight of join pointer to pChild of temp pointer              |
//|          Set pLeft of pChild of temp pointer to join pointer               |
//|          Deallocate node of temp pointer (QLDBDestroyNode)                 |
//|          Set temp pointer to pRight of join pointer                        |
//|          WHILE temp pointer != NULL pointer                                |
//|            Set pParent of temp pointer to pParent of join pointer          |
//|            Set temp pointer to pRight of temp pointer                      |
//|      Set run pointer to pRight of run pointer                              |
//|    Set check pointer to pRight of check pointer                            |
//|  Set check pointer to input pointer                                        |
//|  WHILE check pointer != NULL pointer                                       |
//|    IF pChild of check pointer != NULL pointer                              |
//|      CALL QLDBJoinSameSubtree with pChild of check pointer                 |
//|    Set check pointer to pRight of check pointer                            |
//+----------------------------------------------------------------------------+

USHORT QLDBJoinSameSubtree
(
  PQLDB_HTREE  phTree,                           // pointer to the tree
                                                 // handle structure
  PQLDB_NODE   pNode                             // pointer to node of
                                                 // which the subtree is
                                                 // joined
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  PQLDB_NODE   pCheckNode;                       // pointer used for
                                                 // comparing 2 nodes
  PQLDB_NODE   pRunNode;                         // pointer used for
                                                 // comparing 2 nodes
  PQLDB_NODE   pTempNode;                        // pointer for chaining
                                                 // the nodes
  PQLDB_NODE   pJoinNode;                        // pointer for chaining
                                                 // the nodes
  BOOL         fNodesAreSame;                    // are 2 nodes the same ?



  /********************************************************************/
  /*               check if there is a subtree to join                */
  /********************************************************************/
  if ( pNode != NULL )
  {
    pCheckNode = pNode;

    /******************************************************************/
    /*          look to the right if there are nodes to join          */
    /******************************************************************/
    while ( ( pCheckNode->pRight != NULL ) && ( usRC == QLDB_NO_ERROR ) )
    {
      pRunNode = pCheckNode->pRight;

      /****************************************************************/
      /*                     now run to the right                     */
      /****************************************************************/
      while ( ( pRunNode != NULL ) && ( usRC == QLDB_NO_ERROR ) )
      {
        /**************************************************************/
        /*          compare the run node and the check node           */
        /*           if both are the same join the subtrees           */
        /**************************************************************/
        usRC = QLDBCompare2Nodes( phTree, pCheckNode, pRunNode,
                                  &fNodesAreSame );

        if ( usRC == QLDB_NO_ERROR )
        {
          if ( fNodesAreSame == TRUE )
          {
            /**********************************************************/
            /*   now pointer the subtree of the run pointer to that   */
            /*                  of the check pointer                  */
            /**********************************************************/
            pTempNode = pRunNode;
            pRunNode = pRunNode->pLeft;

            if ( pTempNode->pRight != NULL )
            {
              pRunNode->pRight = pTempNode->pRight;
              pRunNode->pRight->pLeft = pRunNode;
            }
            else
            {
              pRunNode->pRight = NULL;
            } /* endif */

            if ( pCheckNode->pChild != NULL )
            {
              pJoinNode = pCheckNode->pChild;

              while ( pJoinNode->pRight != NULL )
              {
                pJoinNode = pJoinNode->pRight;
              } /* endwhile */

              pJoinNode->pRight = pTempNode->pChild;
              pTempNode->pChild->pLeft = pJoinNode;

              usRC = QLDBDestroyNode( phTree, &pTempNode );

              if ( usRC == QLDB_NO_ERROR )
              {
                pTempNode = pJoinNode->pRight;

                while ( pTempNode != NULL )
                {
                  pTempNode->pParent = pJoinNode->pParent;
                  pTempNode = pTempNode->pRight;
                } /* endwhile */
              } /* endif */
            }
            else
            {
              usRC = QLDBDestroyNode( phTree, &pTempNode );
            } /* endif */
          } /* endif */

          pRunNode = pRunNode->pRight;
        } /* endif */
      } /* endwhile */

      if ( pCheckNode->pRight != NULL )
      {
        pCheckNode = pCheckNode->pRight;
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /*               now go on the lower subtree levels               */
    /******************************************************************/
    pCheckNode = pNode;

    while ( ( pCheckNode != NULL ) && ( usRC == QLDB_NO_ERROR ) )
    {
      if ( pCheckNode->pChild != NULL )
      {
        usRC = QLDBJoinSameSubtree( phTree, pCheckNode->pChild );
      } /* endif */

      if ( usRC == QLDB_NO_ERROR )
      {
        pCheckNode = pCheckNode->pRight;
      } /* endif */
    } /* endwhile */
  } /* endif */

  return( usRC );

} /* end of function QLDBJoinSameSubtree */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBNextRecCtrlChar                                          |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBNextRecCtrlChar( ppchrRecord )                                 |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function scans a part of a record from left to right until it has    |
//|  found a level control character.                                          |
//|                                                                            |
//|  It returns a pointer to the place in the record where it has              |
//|  found the control character.                                              |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PCHAR        *ppchrRecord;                      // pointer to the         |
//|                                                  // record                 |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid record                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  moves the input record pointer to the next control character              |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBNextRecCtrlChar( &pchrRecord );                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF record pointer != END_OF_REC                                           |
//|    Move record pointer to next character                                   |
//|    WHILE record pointer != (FIRST_LEVEL or SECOND_LEVEL or                 |
//|                             THIRD_LEVEL or FOURTH_LEVEL or                 |
//|                             END_OF_REC)                                    |
//|      IF current record pointer == ESC_CHAR                                 |
//|        Move current record pointer to next character                       |
//|      Move current record pointer to next character                         |
//+----------------------------------------------------------------------------+

USHORT QLDBNextRecCtrlChar
(
  PSZ_W        *ppchrRecord                       // pointer to the
                                                  // record
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;            // function return code



  /********************************************************************/
  /*            check if the record is already at its end             */
  /********************************************************************/
  if ( *(*ppchrRecord) != (CHAR_W)(QLDB_END_OF_REC) )
  {
    /******************************************************************/
    /*         move the record pointer to the next character          */
    /******************************************************************/
    (*ppchrRecord)++;

    /******************************************************************/
    /*        now loop until a new control character is found         */
    /******************************************************************/
    while ( ( *(*ppchrRecord) != (CHAR_W)(QLDB_FIRST_LEVEL) ) &&
            ( *(*ppchrRecord) != (CHAR_W)(QLDB_SECOND_LEVEL) ) &&
            ( *(*ppchrRecord) != (CHAR_W)(QLDB_THIRD_LEVEL) ) &&
            ( *(*ppchrRecord) != (CHAR_W)(QLDB_FOURTH_LEVEL) ) &&
            ( *(*ppchrRecord) != (CHAR_W)(QLDB_END_OF_REC) ) )
    {
      /****************************************************************/
      /*           if a QLDB_ESC_CHAR is found jump over it           */
      /****************************************************************/
      if ( *(*ppchrRecord) == (CHAR_W)(QLDB_ESC_CHAR) )
      {
        (*ppchrRecord)++;
      } /* endif */

      /****************************************************************/
      /*                   goto the next character                    */
      /****************************************************************/
      (*ppchrRecord)++;
    } /* endwhile */
  } /* endif */

  return( usRC );

} /* end of function QLDBNextRecCtrlChar */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBNextTreeTemplate                                         |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBNextTreeTemplate( phTree, usCheckLevel, pusLevelInfo )         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function is a recursive function creating the next template in the   |
//|  tree handle.                                                              |
//|                                                                            |
//|  It will check if a node can be found to the right of the current          |
//|  template node of the specified level. If it can be found all children     |
//|  template nodes most left are created. If no node to the right can be      |
//|  found the function will call itself with a level number decreased by one  |
//|  (thus checking on a higher level in the tree).                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // pointer to the tree     |
//|                                                 // handle structure        |
//|  USHORT       usCheckLevel;                     // no of level on which    |
//|                                                 // to check for node on    |
//|                                                 // same level              |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  USHORT       *pusLevelInfo;                    // level information       |
//|                                                 // can be:                 |
//|                                                 // QLDB_MAX_LEVELS         |
//|                                                 // QLDB_END_OF_TREE        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_ERROR_IN_TREE          - an unrecoverable error is in the tree       |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  changes the current level node pointers in the tree handle to the         |
//|  new template                                                              |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBNextTreeTemplate( phTree, QLDB_MAX_LEVELS, pusLevel );    |
//|                                                                            |
//|  Gets the next template starting to search with level QLDB_MAX_LEVELS.     |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF level number < 2                                                       |
//|    RETURN END_OF_TREE                                                      |
//|  ELSE                                                                      |
//|    IF pRight of level node on specified level != NULL pointer              |
//|      Set pointer to level node on specified level to pRight                |
//|        of current level node                                               |
//|      FOR specified level number + 1 TO MAX_LEVELS DO                       |
//|        Set pointer level node on level number to pChild of                 |
//|          node on level number - 1                                          |
//|      RETURN MAX_LEVELS                                                     |
//|    ELSE                                                                    |
//|      CALL QLDBNextTreeTemplate with specified level number - 1             |
//|      RETURN returned level number                                          |
//+----------------------------------------------------------------------------+

USHORT QLDBNextTreeTemplate(

  PQLDB_HTREE  phTree,                           // pointer to the tree
                                                 // handle structure
  USHORT       usCheckLevel,                     // no of level on which
                                                 // to check for node on
                                                 // same level
  USHORT       *pusLevelInfo )                   // level information
                                                 // can be:
                                                 // QLDB_MAX_LEVELS
                                                 // QLDB_END_OF_TREE

{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index



  /********************************************************************/
  /*     check if the input level number is less than 2, i.e the      */
  /*     level is 1 and the template has reached QLDB_END_OF_TREE     */
  /********************************************************************/
  if ( usCheckLevel < 2 )
  {
    *pusLevelInfo = QLDB_END_OF_TREE;
  }
  else
  {
    /******************************************************************/
    /* check if a node to the right of the current level node exists  */
    /*    if so set the new current level node to the node on the     */
    /*      right and set the children node pointers accordingly      */
    /*      if not call QLDBNextTreeTemplate with a higher level      */
    /******************************************************************/
    if ( phTree->apCurLevelNode[usCheckLevel-1]->pRight != NULL )
    {
      phTree->apCurLevelNode[usCheckLevel-1] =
        phTree->apCurLevelNode[usCheckLevel-1]->pRight;

      for ( i = usCheckLevel; ( i < QLDB_MAX_LEVELS ) &&
                              ( usRC == QLDB_NO_ERROR ); i++ )
      {
        if ( phTree->apCurLevelNode[i-1]->pChild != NULL )
        {
          phTree->apCurLevelNode[i] = phTree->apCurLevelNode[i-1]->pChild;
        }
        else
        {
          usRC = QLDB_ERROR_IN_TREE;
        } /* endif */
      } /* endfor */

      if ( usRC == QLDB_NO_ERROR )
      {
        *pusLevelInfo = QLDB_MAX_LEVELS;
      }
      else
      {
        *pusLevelInfo = QLDB_END_OF_TREE;
      } /* endif */
    }
    else
    {
      usRC = QLDBNextTreeTemplate( phTree, (USHORT)(usCheckLevel-1), pusLevelInfo );
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBNextTreeTemplate */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBNodeStorage                                              |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBNodeStorage( phTree, usLevel, ulStorage )                      |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function computes the size of a node on a specified level.           |
//|                                                                            |
//|  It returns the computed amount of storage needed and sets the array       |
//|  aulStorageOnLevel in the tree handle accordingly. If a value in this      |
//|  array is already set the function doesn't recalculate the storage value   |
//|  but takes the value from the array.                                       |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // pointer to the tree     |
//|                                                 // handle structure        |
//|  USHORT       usLevel;                          // level from which        |
//|                                                 // the storage is computed |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  ULONG        *ulStorage;                       // storage computed        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects: none                                                          |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBNodeStorage( phTree, 2, &ulStorage );                     |
//|                                                                            |
//|  This computes the amount of storage needed for a level 2 node.            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//| IF ausStorageOnLevel[ level specified ] == NO_STORAGE_COMPUTED             |
//|   Set storage needed to sizeof( QLDB_NODE )                                |
//|   Add to storage needed (usNoOfFields - 1)*sizeof( QLDB_FIELDCHAR)         |
//|   FOR 1 TO usNoOfFields DO                                                 |
//|       Add to storage needed FIELD_SIZE                                     |
//|   Set storage needed in tree handle to calculated storage                  |
//| ELSE                                                                       |
//|   RETURN ausStorageOnLevel[ level specified ]                              |
//+----------------------------------------------------------------------------+

USHORT QLDBNodeStorage(

  PQLDB_HTREE  phTree,                           // pointer to the tree
                                                 // handle structure
  USHORT       usLevel,                          // level from which
                                                 // the storage is computed
  ULONG        *ulStorage )                      // storage computed

{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code



  /********************************************************************/
  /*    check if the amount of storage needed was already computed    */
  /*    if so just return this value otherwise copmute it and put     */
  /*              the computed value in the tree handle               */
  /********************************************************************/
  if ( phTree->aulStorageOnLevel[usLevel-1] == QLDB_NO_STORAGE )
  {
    phTree->aulStorageOnLevel[usLevel-1] = (LONG)( sizeof( QLDB_NODE ) );

    if ( phTree->ausNoOfFields[usLevel-1] > 0 )
    {
      phTree->aulStorageOnLevel[usLevel-1] +=
           (LONG)( ( phTree->ausNoOfFields[usLevel-1] - 1 ) *
                   sizeof( QLDB_FIELD ) );
      phTree->aulStorageOnLevel[usLevel-1] +=
           (LONG)( phTree->ausNoOfFields[usLevel-1] * QLDB_FIELD_SIZE );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*                   return the computed storage                    */
  /********************************************************************/
  *ulStorage = phTree->aulStorageOnLevel[usLevel-1];

  return( usRC );

} /* end of function QLDBNodeStorage */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBNodeToRecStorage                                         |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBNodeToRecStorage( phTree, pusStorage, pusEscFields )           |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function compute the number of bytes that are needed to store        |
//|  the current node in a record of QLDB format.                              |
//|                                                                            |
//|  It also returns the number of fields in the node that contain characters  |
//|  that are to be marked with and QLDB_ESC_CHAR.                             |
//|                                                                            |
//|  If an error occurrs the storage computed is set to 0.                     |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  USHORT       *pusStorage;                      // computed storage        |
//|  USHORT       *pusEscFields;                    // no of ESC fields        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBNodeToRecStorage( phTree, &usStorage );                   |
//|  usStorage is # of CHAR_W's needed!                                        |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Check the validity of the tree handle                                     |
//|  Add one CHAR_W for the level information                                    |
//|  Add the number of fields for the field delimeter characters               |
//|  FOR 1 to usNOOfFields DO                                                  |
//|    IF a control character is in the field                                  |
//|      Add the number of data bytes and the number of control characters     |
//|      Increase the number of esc fields by one                              |
//|    ELSE                                                                    |
//|      Add the number of data CHAR_W's                                          |
//+----------------------------------------------------------------------------+

USHORT QLDBNodeToRecStorage
(
  PQLDB_HTREE  phTree,                           // handle to tree
  USHORT       *pusStorage,                      // computed storage
  USHORT       *pusEscFields                     // no of ESC fields
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  USHORT       j;                                // another index
  BOOL         fCtrlCharsInField;                // are there control
                                                 // characters in the
                                                 // field ?



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pusStorage = 0;

  /********************************************************************/
  /*           add one CHAR_W for the level control character         */
  /********************************************************************/
  (*pusStorage)++;

  /********************************************************************/
  /*      now add same number of CHAR_W's as there are no of fields   */
  /*            for the field delimiter control characters            */
  /********************************************************************/
  (*pusStorage) = (*pusStorage) +
                   phTree->ausNoOfFields[phTree->pCurNode->usLevel-1];

  /********************************************************************/
  /*     now loop over the fields and compute the number of CHAR_W's  */
  /*                      that are in the fields                      */
  /*   if a field contains one or more control characters than add    */
  /*        another CHAR_W for each control character found           */
  /********************************************************************/
  for ( i = 0; i < phTree->ausNoOfFields[phTree->pCurNode->usLevel-1];
        i++ )
  {
    if ( UTF16strchr( phTree->pCurNode->aFields[i].pszData,
                 (CHAR_W)( QLDB_ESC_CHAR ) ) != NULL )
    {
      fCtrlCharsInField = TRUE;
    }
    else
    {
      if ( UTF16strchr( phTree->pCurNode->aFields[i].pszData,
                   (CHAR_W)( QLDB_END_OF_REC ) ) != NULL )
      {
        fCtrlCharsInField = TRUE;
      }
      else
      {
        if ( UTF16strchr( phTree->pCurNode->aFields[i].pszData,
                     (CHAR_W)( QLDB_FIRST_LEVEL ) ) != NULL )
        {
          fCtrlCharsInField = TRUE;
        }
        else
        {
          if ( UTF16strchr( phTree->pCurNode->aFields[i].pszData,
                       (CHAR_W)( QLDB_SECOND_LEVEL ) ) != NULL )
          {
            fCtrlCharsInField = TRUE;
          }
          else
          {
            if ( UTF16strchr( phTree->pCurNode->aFields[i].pszData,
                         (CHAR_W)( QLDB_THIRD_LEVEL ) ) != NULL )
            {
              fCtrlCharsInField = TRUE;
            }
            else
            {
              if ( UTF16strchr( phTree->pCurNode->aFields[i].pszData,
                           (CHAR_W)( QLDB_FOURTH_LEVEL ) ) != NULL )
              {
                fCtrlCharsInField = TRUE;
              }
              else
              {
                fCtrlCharsInField = FALSE;
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */

    if ( fCtrlCharsInField == TRUE )
    {
      (*pusEscFields)++;
      for ( j = 0; j < UTF16strlenCHAR( phTree->pCurNode->aFields[i].pszData );
            j++ )
      {
        switch ( phTree->pCurNode->aFields[i].pszData[j] )
        {
          case (CHAR_W)( QLDB_ESC_CHAR ) :
          case (CHAR_W)( QLDB_END_OF_REC ) :
          case (CHAR_W)( QLDB_FIRST_LEVEL ) :
          case (CHAR_W)( QLDB_SECOND_LEVEL ) :
          case (CHAR_W)( QLDB_THIRD_LEVEL ) :
          case (CHAR_W)( QLDB_FOURTH_LEVEL ) :
            {
              (*pusStorage) += 2;
            }
          break;
          default :
            {
              (*pusStorage) ++;
            }
          break;
        } /* endswitch */
      } /* endfor */
    }
    else
    {
      (*pusStorage) = (*pusStorage) +
                       (USHORT)UTF16strlenCHAR( phTree->pCurNode->aFields[i].pszData );
    } /* endif */
  } /* endfor */

  return( usRC );

} /* end of function QLDBNodeToRecStorage */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBPrevRecCtrlChar                                          |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBPrevRecCtrlChar( ppchrRecord )                                 |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function scans a part of a record from right to left until it has    |
//|  found a level control character.                                          |
//|                                                                            |
//|  It returns a pointer to the place in the record where it has              |
//|  found the control character.                                              |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PCHAR        *ppchrRecord;                      // pointer to the         |
//|                                                  // record                 |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid record                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  moves the input record pointer to the previous control character          |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBPrevRecCtrlChar( &pchrRecord );                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF current record pointer != FIRST_LEVEL                                  |
//|    Move current record pointer to previous character                       |
//|    WHILE current record pointer != (FIRST_LEVEL or SECOND_LEVEL or         |
//|                                     THIRD_LEVEL or FOURTH_LEVEL or         |
//|                                     END_OF_REC )                           |
//|      Move current record pointer to previous character                     |
//|      IF (current record pointer == (FIRST_LEVEL or SECOND_LEVEL or         |
//|                                     THIRD_LEVEL or FOURTH_LEVEL or         |
//|                                     END_OF_REC) ) and                      |
//|         (previous character of current record pointer == ESC_CHAR)         |
//|        Move current record pointer to previous character                   |
//+----------------------------------------------------------------------------+

USHORT QLDBPrevRecCtrlChar
(
  PSZ_W        *ppchrRecord                       // pointer to the
                                                  // record
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code



  /********************************************************************/
  /*         check if the record is already at its beginning          */
  /********************************************************************/
  if ( *(*ppchrRecord) != (CHAR_W)(QLDB_FIRST_LEVEL) )
  {
    /******************************************************************/
    /*       move the record pointer to the previous character        */
    /******************************************************************/
    (*ppchrRecord)--;

    /******************************************************************/
    /*        now loop until a new control character is found         */
    /******************************************************************/
    while ( ( *(*ppchrRecord) != (CHAR_W)(QLDB_FIRST_LEVEL) ) &&
            ( *(*ppchrRecord) != (CHAR_W)(QLDB_SECOND_LEVEL) ) &&
            ( *(*ppchrRecord) != (CHAR_W)(QLDB_THIRD_LEVEL) ) &&
            ( *(*ppchrRecord) != (CHAR_W)(QLDB_FOURTH_LEVEL) ) &&
            ( *(*ppchrRecord) != (CHAR_W)(QLDB_END_OF_REC) ) )
    {
      /****************************************************************/
      /*      move the record pointer to the previous character       */
      /****************************************************************/
      (*ppchrRecord)--;

      /****************************************************************/
      /*           if a QLDB_ESC_CHAR is found jump over it           */
      /****************************************************************/
      if ( ( ( *(*ppchrRecord) == (CHAR_W)(QLDB_FIRST_LEVEL) ) ||
             ( *(*ppchrRecord) == (CHAR_W)(QLDB_SECOND_LEVEL) ) ||
             ( *(*ppchrRecord) == (CHAR_W)(QLDB_THIRD_LEVEL) ) ||
             ( *(*ppchrRecord) == (CHAR_W)(QLDB_FOURTH_LEVEL) ) ||
             ( *(*ppchrRecord) == (CHAR_W)(QLDB_END_OF_REC) ) ) &&
           ( *((*ppchrRecord) - 1) == (CHAR_W)(QLDB_ESC_CHAR) ) )
      {
        (*ppchrRecord) -= 2;
      } /* endif */
    } /* endwhile */
  } /* endif */

  return( usRC );

} /* end of function QLDBPrevRecCtrlChar */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBPrevTreeTemplate                                         |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBPrevTreeTemplate( phTree, usCheckLevel, pusLevelInfo )         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function is a recursive function creating the previous template in   |
//|  the tree handle.                                                          |
//|                                                                            |
//|  It will check if a node can be found to the left of the current           |
//|  template node of the specified level. If it can be found all children     |
//|  template nodes most right are created. If no node to the left can be      |
//|  found the function will call itself with a level number decreased by one  |
//|  (thus checking on a higher level in the tree).                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // pointer to the tree     |
//|                                                 // handle structure        |
//|  USHORT       usCheckLevel;                     // no of level on which    |
//|                                                 // to check for node on    |
//|                                                 // same level              |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  USHORT       *pusLevelInfo;                    // level information       |
//|                                                 // can be:                 |
//|                                                 // QLDB_MAX_LEVELS         |
//|                                                 // QLDB_END_OF_TREE        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_ERROR_IN_TREE          - an unrecoverable error is in the tree       |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  changes the current level node pointers in the tree handle to the         |
//|  new template                                                              |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBPrevTreeTemplate( phTree, QLDB_MAX_LEVELS, pusLevel );    |
//|                                                                            |
//|  Gets the previous template starting to search with level QLDB_MAX_LEVELS. |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF level number < 2                                                       |
//|    RETURN END_OF_TREE                                                      |
//|  ELSE                                                                      |
//|    IF pLeft of level node on specified level != NULL pointer               |
//|      Set pointer to level node on specified level to pLeft                 |
//|        of current level node                                               |
//|      FOR specified level number + 1 TO MAX_LEVELS DO                       |
//|        Set pointer to level node on level number to pChild of              |
//|          level node on level number - 1                                    |
//|        WHILE pRight of level node on level number != NULL                  |
//|          Set pointer to level node on level number to pRight of            |
//|            level node on level number                                      |
//|      RETURN MAX_LEVELS                                                     |
//|    ELSE                                                                    |
//|      CALL QLDBPrevTreeTemplate with specified level number - 1             |
//|      RETURN returned level number                                          |
//+----------------------------------------------------------------------------+

USHORT QLDBPrevTreeTemplate(

  PQLDB_HTREE  phTree,                           // pointer to the tree
                                                 // handle structure
  USHORT       usCheckLevel,                     // no of level on which
                                                 // to check for node on
                                                 // same level
  USHORT       *pusLevelInfo )                   // level information
                                                 // can be:
                                                 // QLDB_MAX_LEVELS
                                                 // QLDB_END_OF_TREE

{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index



  /********************************************************************/
  /*     check if the input level number is less than 2, i.e the      */
  /*     level is 1 and the template has reached QLDB_END_OF_TREE     */
  /********************************************************************/
  if ( usCheckLevel < 2 )
  {
    *pusLevelInfo = QLDB_END_OF_TREE;
  }
  else
  {
    /******************************************************************/
    /*  check if a node to the left of the current level node exists  */
    /*    if so set the new current level node to the node on the     */
    /*      left and set the children node pointers accordingly       */
    /*      if not call QLDBPrevTreeTemplate with a higher level      */
    /******************************************************************/
    if ( phTree->apCurLevelNode[usCheckLevel-1]->pLeft != NULL )
    {
      phTree->apCurLevelNode[usCheckLevel-1] =
        phTree->apCurLevelNode[usCheckLevel-1]->pLeft;

      for ( i = usCheckLevel; ( i < QLDB_MAX_LEVELS ) &&
                              ( usRC == QLDB_NO_ERROR ); i++ )
      {
        /**************************************************************/
        /*    set the child node pointer and walk then to the node    */
        /*                  most right on the level                   */
        /*    if a child pointer of NULL is found is found in the     */
        /*             for loop return QLDB_ERROR_IN_TREE             */
        /**************************************************************/
        if ( phTree->apCurLevelNode[i-1]->pChild != NULL )
        {
          phTree->apCurLevelNode[i] =
            phTree->apCurLevelNode[i-1]->pChild;

          while ( phTree->apCurLevelNode[i]->pRight != NULL )
          {
            phTree->apCurLevelNode[i] = phTree->apCurLevelNode[i]->pRight;
          } /* endwhile */
        }
        else
        {
          usRC = QLDB_ERROR_IN_TREE;
        } /* endif */
      } /* endfor */

      if ( usRC == QLDB_NO_ERROR )
      {
        *pusLevelInfo = QLDB_MAX_LEVELS;
      }
      else
      {
        *pusLevelInfo = QLDB_END_OF_TREE;
      } /* endif */
    }
    else
    {
      usRC = QLDBPrevTreeTemplate( phTree, (USHORT)(usCheckLevel-1), pusLevelInfo );
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBPrevTreeTemplate */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBRemoveChildSubtree                                       |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBRemoveChildSubtree( phTree, pNode )                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This is a recursive function that removes the whole subtree of a          |
//|  specified node.                                                           |
//|                                                                            |
//|  The node itself is not removed. It is up to the calling                   |
//|  function to destroy the node itself and to set the pointers accordingly.  |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//|  PQLDB_NODE   pNode;                            // node of which subtree   |
//|                                                 // is to be destroyed      |
//+----------------------------------------------------------------------------+
//|Output parameter: none                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle                                                     |
//|  - a valid subtree under the specified node ( an empty subtree is also     |
//|    a subtree )                                                             |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  the subtree under the input node pointer is destroyed                     |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBRemoveChildSubtree( phTree, phTree->pCurNode );           |
//|                                                                            |
//|  This destroys the subtree under the current node.                         |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF pChild of input pointer != NULL pointer                                |
//|    Set temp pointer to pChild of input pointer                             |
//|    WHILE pRight of temp pointer != NULL pointer                            |
//|      Set temp pointer to pRight of temp pointer                            |
//|    REPEAT                                                                  |
//|      IF pLeft of temp pointer != NULL pointer                              |
//|        Set left pointer to pLeft of temp pointer                           |
//|      ELSE                                                                  |
//|        Set left pointer to NULL pointer                                    |
//|      CALL QLDBRemoveChildSubtree with temp pointer                         |
//|      CALL QLDBDestroyNode with temp pointer                                |
//|      Set temp pointer to NULL pointer                                      |
//|      IF left pointer != NULL pointer                                       |
//|        Set temp pointer to left pointer                                    |
//|    UNTIL left pointer == NULL pointer                                      |
//+----------------------------------------------------------------------------+

USHORT QLDBRemoveChildSubtree
(
  PQLDB_HTREE  phTree,                           // handle to tree
  PQLDB_NODE   pNode                             // node of which subtree
                                                 // is to be destroyed
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  PQLDB_NODE   pTempNode;                        // temp node
  PQLDB_NODE   pLeftNode;                        // node left of temp node



  /********************************************************************/
  /*                 check if a child pointer exists                  */
  /********************************************************************/
  if ( pNode->pChild != NULL )
  {
    pTempNode = pNode->pChild;

    /******************************************************************/
    /*          now move to the node most right on the level          */
    /******************************************************************/
    while ( pTempNode->pRight != NULL )
    {
      pTempNode = pTempNode->pRight;
    } /* endwhile */

    /******************************************************************/
    /*      now loop as long as there are left subtrees existing      */
    /******************************************************************/
    do
    {
      pLeftNode = pTempNode->pLeft;

      /****************************************************************/
      /*             destroy the subtree under pTempNode              */
      /****************************************************************/
      usRC = QLDBRemoveChildSubtree( phTree, pTempNode );

      /****************************************************************/
      /*                destroy the node of pTempNode                 */
      /****************************************************************/
      if ( usRC == QLDB_NO_ERROR )
      {
        usRC = QLDBDestroyNode( phTree, &pTempNode );
      } /* endif */

      if ( usRC == QLDB_NO_ERROR )
      {
        if ( pLeftNode != NULL )
        {
          pTempNode = pLeftNode;
        } /* endif */
      } /* endif */
    } while ( ( pLeftNode != NULL ) &&
              ( usRC == QLDB_NO_ERROR ) ); /* enddo */
  } /* endif */

  return( usRC );

} /* end of function QLDBRemoveChildSubtree */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBUpdateFieldData                                          |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBUpdateFieldData( phTree, pNode, usField, pszData )             |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function makes an update to a field in a node.                       |
//|  The data area for the field is used according to the size of the new      |
//|  data.                                                                     |
//|  In case of problems the old value is restored in the field and the        |
//|  returncode set accordingly.                                               |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//|  PQLDB_NODE   pNode;                            // node in which field     |
//|                                                 // is updated              |
//|  USHORT       usField;                          // No. of field to be      |
//|                                                 // updated                 |
//|  PSZ          pszData;                          // pointer to              |
//|                                                 // zero-terminated string  |
//|                                                 // with the data for the   |
//|                                                 // field to be updated     |
//+----------------------------------------------------------------------------+
//|Output parameter: none                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_INVALID_FIELDNO        - an invalid field number was entered         |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle                                                     |
//|  - a valid node                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  updates the data in field usField                                         |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBUpdateFieldData( phTree, pNode, 3, "NEW" );               |
//|                                                                            |
//|  This updates the field 2 of pNode with "NEW".                             |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF all input parameters are valid                                         |
//|    Save old data from field                                                |
//|    IF field data is in node                                                |
//|      Clear the field                                                       |
//|    ELSE                                                                    |
//|      Deallocate the external storage area                                  |
//|    IF new data >= QLDB_FIELD_SIZE                                          |
//|      Allocate external storage area                                        |
//|    ELSE                                                                    |
//|      Compute the offset of the data field in the node                      |
//|        (QLDBComputeFirstFieldOffset)                                       |
//|      Set the data pointer to the data field in the node                    |
//|    Copy the data to the field (QLDBFillData)                               |
//+----------------------------------------------------------------------------+

USHORT QLDBUpdateFieldData(

  PQLDB_HTREE  phTree,                           // handle to tree
  PQLDB_NODE   pNode,                            // node in which field
                                                 // is updated
  USHORT       usField,                          // No. of field to be
                                                 // updated
  PSZ_W        pszData )                         // pointer to
                                                 // zero-terminated string
                                                 // with the data for the
                                                 // field to be updated

{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       usOffset;                         // offset of the first
                                                 // preallocated data
                                                 // field

  /********************************************************************/
  /*                  check if field number is valid                  */
  /********************************************************************/
  if ( ( usField < 1 ) ||
       ( usField > phTree->ausNoOfFields[pNode->usLevel-1] ) )
  {
    usRC = QLDB_INVALID_FIELDNO;
  } /* endif */

  /********************************************************************/
  /*      check if the field into which should be copied exists       */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    if ( pNode->aFields[usField-1].pszData == NULL )
    {
      usRC = QLDB_INVALID_FIELDNO;
    } /* endif */
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*      if the old data is in the node then clear the field       */
    /*   if the old data is external than deallocate the data area    */
    /******************************************************************/
    if ( pNode->aFields[usField-1].fDataInNode )
    {
      memset( (PBYTE)(pNode->aFields[usField-1].pszData), QLDB_NULC,
              QLDB_FIELD_SIZE );
    }
    else
    {
      UtlAlloc( (PVOID *)&(pNode->aFields[usField-1].pszData), 0L, 0L, NOMSG );
    } /* endif */

    /******************************************************************/
    /*      check if the data length is >= QLDB_FIELD_SIZE then       */
    /*          allocate an external data area for the field          */
    /*   otherwise set the data pointer in the node to the in-node    */
    /*                          data fields                           */
    /******************************************************************/
    if ( pszData != NULL )
    {
      if ( UTF16strlenBYTE( pszData ) >= QLDB_FIELD_SIZE )
      {
         if ( UtlAlloc( (PVOID *)&( pNode->aFields[usField-1].pszData ), 0L,
                        (LONG)( UTF16strlenBYTE( pszData ) + sizeof(CHAR_W) ),
                        NOMSG ) )
         {
           /***********************************************************/
           /*  indicate that field data is now in external data area  */
           /***********************************************************/
           pNode->aFields[usField-1].fDataInNode = FALSE;
         }
         else
         {
           usRC = QLDB_NO_MEMORY;
         } /* endif */
      }
      else
      {
        /**************************************************************/
        /*  compute the offset of the first preallocated data field   */
        /**************************************************************/
        QLDBComputeFirstFieldOffset( phTree, pNode->usLevel, &usOffset );

        /**************************************************************/
        /*     set in-node data pointer to the preallocated field     */
        /**************************************************************/
        pNode->aFields[usField-1].pszData = (PSZ_W)((PSZ)( pNode ) + usOffset +
                                            (usField-1) * QLDB_FIELD_SIZE);

        /**************************************************************/
        /*    indicate that field data is now in in_node data area    */
        /**************************************************************/
        pNode->aFields[usField-1].fDataInNode = TRUE;
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /*   compute the offset of the first preallocated data field    */
      /****************************************************************/
      QLDBComputeFirstFieldOffset( phTree, pNode->usLevel, &usOffset );

      /****************************************************************/
      /*      set in-node data pointer to the preallocated field      */
      /****************************************************************/
      pNode->aFields[usField-1].pszData = (PSZ_W)((PSZ)( pNode ) + usOffset +
                                          (usField-1) * QLDB_FIELD_SIZE);

      /****************************************************************/
      /*     indicate that field data is now in in_node data area     */
      /****************************************************************/
      pNode->aFields[usField-1].fDataInNode = TRUE;
    } /* endif */

    if ( usRC == QLDB_NO_ERROR )
    {
      /****************************************************************/
      /*                  copy the data to the field                  */
      /****************************************************************/
      if ( pszData != NULL )
      {
        usRC = QLDBFillData( phTree, pNode, usField, pszData );
      }
      else
      {
        usRC = QLDBFillData( phTree, pNode, usField,
                                      QLDB_EMPTY_STRINGW );
      } /* endif */
    } /* endif */

  } /* endif */

  return( usRC );

} /* end of function QLDBUpdateFieldData */
