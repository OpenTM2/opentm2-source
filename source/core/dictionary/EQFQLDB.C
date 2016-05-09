//+----------------------------------------------------------------------------+
//|EQFQLDB.C                                                                   |
//+----------------------------------------------------------------------------+
//| Changes by Gerhard:                                                        |
//|  - Added QLDBFillFieldTables function                                      |
//|  - Added QLDBUpdateCurrTemplate function                                   |
//|  - fixed bug in QLDBDestroyTree (memory corrupted message if working with  |
//|    records --> record pointer was not corrected at UtlAlloc call)          |
//|  - fixed bug in QLDBTreeToRecord: if work with records is active, no copy  |
//|    was made of the internal record buffer, but the pointer to the          |
//|    internal record buffer was returned. This caused problems when the      |
//|    tree was destroyed before the record was processed.                     |
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
//|  This file contains the functions that are to be called when working       |
//|  with QLDB records and trees.                                              |
//|  The file EQFQLDBI.C contains the internal functions that QLDB             |
//|  uses in maintaining the records and trees.                                |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//|    QLDBAddSubtree                                                          |
//|    QLDBCreateTree                                                          |
//|    QLDBCurrNode                                                            |
//|    QLDBCurrTemplate                                                        |
//|    QLDBDestroySubtree                                                      |
//|    QLDBDestroyTree                                                         |
//|    QLDBFlattenTree                                                         |
//|    QLDBJoinSameNodes                                                       |
//|    QLDBMoveToNode                                                          |
//|    QLDBNextNode                                                            |
//|    QLDBNextTemplate                                                        |
//|    QLDBPrevNode                                                            |
//|    QLDBPrevTemplate                                                        |
//|    QLDBRecordToTree                                                        |
//|    QLDBResetTreePositions                                                  |
//|    QLDBTreeToRecord                                                        |
//|    QLDBUpdateCurNodeData                                                   |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
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
// $Revision: 1.2 $ ----------- 3 Mar 2004
// GQ: - check level information of record data before using it (QLDBCurNode/QLDBNextNode)
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
// $Revision: 1.2 $ ----------- 29 Jul 2002
// -- RJ: R7197: add system pref. lang for conversion
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.5 $ ----------- 17 Dec 2001
// GQ: - Unicode2AsciiBuf function: add 1 to input length to include the end-of-string delimiter
//
//
// $Revision: 1.4 $ ----------- 7 Dec 2001
// GQ: - pass buffer length to Unicode2ASCIIBuf and Unicode2AnsiBuf functions
//
//
// $Revision: 1.3 $ ----------- 22 Oct 2001
// -- RJ: function TreeToRecordNonUnicode
//
//
// $Revision: 1.2 $ ----------- 19 Oct 2001
// -- RJ: unicode enabling
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
 * $Header:   J:\DATA\EQFQLDB.CV_   1.0   09 Jan 1996 09:13:50   BUILD  $
 *
 * $Log:   J:\DATA\EQFQLDB.CV_  $
 *
 *    Rev 1.0   09 Jan 1996 09:13:50   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define INCL_EQF_LDB              // dictionary data encoding functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqfldbi.h"              // private LDB include file

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBAddSubtree adds a subtree to a specific level            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBAddSubtree( phTree, usLevel, ppszData )                        |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  Adds a subtree on the specified level to an existing node tree            |
//|  and fills-in the supplied data.                                           |
//|  The insert level may have a range from 2 to QLDB_MAX_LEVEL                |
//|  (because the first level node always stays the same).                     |
//|  The function will create a subtree right to the rightmost node on         |
//|  the specified level.                                                      |
//|  The current pointers will be set to the following after the function      |
//|  has finished:                                                             |
//|  - the current node pointer is set to the node created on the specified    |
//|    level                                                                   |
//|  - the level pointers will reference the created template                  |
//|                                                                            |
//|  The data is to be supplied for all the fields from insert level up to     |
//|  QLDB_MAX_LEVEL.                                                           |
//|                                                                            |
//|  If any problems occur (e.g.storage, incorrect data) the returncode of     |
//|  the function is set accordingly. Everything already allocated will be     |
//|  destroyed.                                                                |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree to       |
//|                                                 // which subtree is        |
//|                                                 // added                   |
//|  USHORT       usLevel;                          // level on which          |
//|                                                 // subtree is added        |
//|  PSZ          *ppszData;                        // pointer to an           |
//|                                                 // array of pointers       |
//|                                                 // pointing to the data    |
//|                                                 // that is to be supplied  |
//|                                                 // for each field          |
//+----------------------------------------------------------------------------+
//|Output parameter: none                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_TOO_MANY_FIELDS        - too many fields for a level were requested  |
//|  QLDB_NO_VALID_DATA          - the data fields did not exist               |
//|  QLDB_INVALID_LEVEL          - invalid level number entered                |
//|  QLDB_ERROR_IN_TREE          - an unrecoverable error is in the tree       |
//|  QLDB_ERR_DEALLOC            - error deallocating memory                   |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle with a valid tree (at least one template from       |
//|    level 1 to QLDB_MAX_LEVEL is needed)                                    |
//|  - valid input data, i.e. an array of pointers that reference              |
//|    the fields that are needed for the nodes from the specified level       |
//|    up to QLDB_MAX_LEVELS                                                   |
//|  - the fields must be zero-terminated strings                              |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - a new template is created                                               |
//|  - the current node pointer is set to the node created on the input level  |
//|  - the template pointers reference the template most right                 |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBAddSubtree( phTree, 2, ppszData )                         |
//|                                                                            |
//|  This creates a subtree from level 2 to level QLDB_MAX_LEVELS              |
//|  and fills in the data from ppszData                                       |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|    Convert the record to a tree (QLDBRecordToTree)                         |
//|  IF (insert level >= 2) and (insert level <= MAX_LEVELS)                   |
//|    FOR insert level TO MAX_LEVELS DO                                       |
//|      Create node and fill in data for that node (QLDBCreateNode)           |
//|      IF node is created at insert level                                    |
//|        Pointer node to node most right at insert level                     |
//|        Set the current node pointer to the node                            |
//|      Pointer node to parent node                                           |
//|      Set current level pointer to created node                             |
//+----------------------------------------------------------------------------+

USHORT QLDBAddSubtree(

  PVOID        pvTree,                           // handle to tree to
                                                 // which subtree is
                                                 // added
  USHORT       usLevel,                          // level on which
                                                 // subtree is added
  PSZ_W        *ppszData )                       // pointer to an
                                                 // array of pointers
                                                 // pointing to the data
                                                 // that is to be supplied
                                                 // for each field

{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  USHORT       usTotalFields = 0;                // total fields in a
                                                 // complete subtree
  PQLDB_NODE   apTempNodes[QLDB_MAX_LEVELS];     // temp array of pointers
                                                 // to allow restoring
                                                 // the old current node
                                                 // pointers in case of
                                                 // problems
  PQLDB_NODE   pTempCurNode;                     // former current node
  PQLDB_NODE   pNode = NULL;                     // pointer to created
                                                 // node
  PQLDB_NODE   pTempNode;                        // a temporary pointer
  PSZ_W        *ppszTempData;                    // pointer to the
                                                 // array of pointers
                                                 // for node data



  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*        compute the number of total fields in the subtree         */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    QLDB_TOTAL_FIELDS( phTree, &usTotalFields );
  } /* endif */

  /********************************************************************/
  /*   check if the pointer to the pointer array is a valid pointer   */
  /********************************************************************/
  if ( ( usRC == QLDB_NO_ERROR ) && ( ppszData == NULL ) &&
       ( usTotalFields != 0 ) )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */

  /********************************************************************/
  /*          check if an invalid level number was supplied           */
  /********************************************************************/
  if ( ( ( usLevel < 2 ) || ( usLevel > QLDB_MAX_LEVELS ) ) &&
       ( usRC == QLDB_NO_ERROR ) )
  {
    usRC = QLDB_INVALID_LEVEL;
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          check if the tree handle references a record          */
    /*   if so convert the record to a tree before starting to work   */
    /******************************************************************/
    if ( phTree->fWorkWithRecord == TRUE )
    {
      /****************************************************************/
      /*                 convert the record to a tree                 */
      /****************************************************************/
      usRC = QLDBRecordToTree( phTree->ausNoOfFields,
                               phTree->apszCurLevelRec[0],
                               0, (PVOID *)&phTree );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /* as the loop already walks along the pointers to the level nodes  */
  /*     save them in an array to easily restore them in case of      */
  /*                          later problems                          */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( phTree->apCurLevelNode[i] == NULL )
    {
      usRC = QLDB_NO_TREEHANDLE;
    }
    else
    {
      apTempNodes[i] = phTree->apCurLevelNode[i];
    } /* endif */
  } /* endfor */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*  as the old current nodes were alread saved before just save   */
    /*  the actual current node pointer to have it for restoring in   */
    /*                     case of later problems                     */
    /******************************************************************/
    pTempCurNode = phTree->pCurNode;

    /******************************************************************/
    /*   set the pointer to the level data fields to the beginning    */
    /*                       of the data array                        */
    /******************************************************************/
    if ( ppszData != NULL )
    {
      ppszTempData = ppszData;
    }
    else
    {
      ppszTempData = NULL;
    } /* endif */

    /******************************************************************/
    /*        start at the insert level and build up a subtree        */
    /*        right to the node most right on the insert level        */
    /******************************************************************/
    for ( i = usLevel - 1; ( i < QLDB_MAX_LEVELS ) &&
                           ( usRC == QLDB_NO_ERROR ); i++ )
    {
      /****************************************************************/
      /*      create the node and fill in the data for that node      */
      /****************************************************************/
      usRC = QLDBCreateNode( phTree, (USHORT)( i + 1), ppszTempData,
                             &pNode );
      if ( usRC != QLDB_NO_ERROR )
      {
        /**************************************************************/
        /*      destroy the already built up tree and restore the     */
        /*            old current and level node pointers             */
        /*           but only in case of a node being build           */
        /**************************************************************/
        if ( i > usLevel - 1 )
        {
          QLDBDestroyIncompleteSubtree( phTree, 1 );
          for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
          {
            phTree->apCurLevelNode[i] = apTempNodes[i];
          } /* endfor */

          phTree->pCurNode = pTempCurNode;
        } /* endif */
      }
      else
      {
        /**************************************************************/
        /*  if the node is created on the insert level then pointer   */
        /*  the node the node most right on the insert level and set  */
        /*      the new current node pointer to the created node      */
        /**************************************************************/
        if ( i == usLevel - 1 )
        {
          /************************************************************/
          /*      walk to the rightmost node on the insert level      */
          /************************************************************/
          pTempNode = phTree->apCurLevelNode[i];
          while ( pTempNode->pRight != NULL )
          {
            pTempNode = pTempNode->pRight;
          } /* endwhile */

          /************************************************************/
          /*   now pointer the rightmost node with the created node   */
          /************************************************************/
          pTempNode->pRight = pNode;
          pNode->pLeft = pTempNode;
          phTree->pCurNode = pNode;
        } /* endif */

        /**************************************************************/
        /*   set the new level node pointer and pointer the created   */
        /*                    node with its parent                    */
        /**************************************************************/
        phTree->apCurLevelNode[i] = pNode;
        pNode->pParent = phTree->apCurLevelNode[i-1];
        if ( i != usLevel - 1 )
        {
          phTree->apCurLevelNode[i-1]->pChild = pNode;
        } /* endif */

        /**************************************************************/
        /*      set the temp data pointer to the first field of       */
        /*                the data for the next level                 */
        /**************************************************************/
        if ( ppszTempData != NULL )
        {
          ppszTempData += phTree->ausNoOfFields[i];
        } /* endif */
      } /* endif */
    } /* endfor */
  } /* endif */

  return( usRC );

} /* end of function QLDBAddSubtree */



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBCreateTree creates a node tree and fills in the data     |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBCreateTree( ausNoOfFields, ppszData, phTree )                  |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  The function creates a node tree (one node for each level) and fills      |
//|  in the data for each field in every node.                                 |
//|  It returns a pointer to the allocated tree handle structure.              |
//|  The calling function has to take care of this handle structure as it      |
//|  is the only way for the calling function to work with the tree.           |
//|                                                                            |
//|  The current node pointer is set to the first level node and the           |
//|  current template is set to the appropriate level nodes.                   |
//|                                                                            |
//|  If any problems occurr the pointer to the tree handle structure is NULL   |
//|  and the returncode will specify the type of problems arised.              |
//|  The tree will not be created and everything already allocated is          |
//|  destroyed.                                                                |
//|                                                                            |
//|  This function cannot work with tree handles already filled (e.g. pointing |
//|  to records).                                                              |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  USHORT       ausNoOfFields[];                  // no. of fields on        |
//|                                                 // each level              |
//|  PSZ          *ppszData;                        // pointer to an           |
//|                                                 // array of pointers       |
//|                                                 // pointing to the data    |
//|                                                 // that is to be supplied  |
//|                                                 // for each field          |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PQLDB_HTREE  *pphTree;                         // pointer to a            |
//|                                                 // pointer to the tree     |
//|                                                 // handle structure        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_TOO_MANY_FIELDS        - too many fields for a level were requested  |
//|  QLDB_NO_VALID_DATA          - the data fields did not exist               |
//+----------------------------------------------------------------------------+
//|Prerequesits: none                                                          |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the tree handle structure is allocated                                  |
//|  - the current node pointer is set to the first level node                 |
//|  - the current template is set to the template most left                   |
//|  - storage for QLDB_MAX_LEVELS nodes is allocated                          |
//|  - data is copied to the nodes                                             |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|  PQLDB_HTREE  phTree;                                                      |
//|  USHORT       ausFields[QLDB_MAX_LEVELS] = { 2, 3, 1, 5 };                 |
//|  PSZ          apszData[11] = { "Level1Field1", "Level1Field2",             |
//|                                "Level2Field2", "Level2Field2",             |
//|                                "Level2Field3", "Level3Field1",             |
//|                                "Level4Field1", "Level4Field2",             |
//|                                "Level4Field3", "Level4Field4",             |
//|                                "Level4Field5" }                            |
//|                                                                            |
//|                                                                            |
//|  usRetCode = QLDBCreateTree( ausFields, apszData, &phTree );               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Create Tree handle (QLDBCreateTreeHandle)                                 |
//|  IF everything went ok                                                     |
//|    FOR 1 TO QLDB_MAX_LEVELS DO                                             |
//|      Create node and fill in data (QLDBCreateNode)                         |
//|      IF level on which node is created != 1                                |
//|        Pointer node to parent node                                         |
//|      ELSE                                                                  |
//|        set current node pointer to first level node                        |
//|      Set current node level pointer in tree handle to created node         |
//|  RETURN pointer to tree handle and returncode                              |
//+----------------------------------------------------------------------------+

USHORT QLDBCreateTree
(
  USHORT       ausNoOfFields[],                  // no. of fields on
                                                 // each level
  PSZ_W        *ppszData,                        // pointer to an
                                                 // array of pointers
                                                 // pointing to the data
                                                 // that is to be supplied
                                                 // for each field
  PVOID        *ppvTree                          // pointer to the tree
                                                 // handle structure
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  *pphTree = (PQLDB_HTREE *)ppvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  USHORT       usTotalFields = 0;                // total fields in a
                                                 // complete subtree
  PQLDB_NODE   pNode = NULL;                     // pointer to created
                                                 // node
  PSZ_W        *ppszTempData;                    // pointer to the
                                                 // array of pointers
                                                 // for node data



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *ppvTree = NULL;

  /********************************************************************/
  /*        compute the number of total fields in the subtree         */
  /********************************************************************/
  for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
  {
    usTotalFields = usTotalFields + ausNoOfFields[i];
  } /* endfor */

  /********************************************************************/
  /*   check if the pointer to the pointer array is a valid pointer   */
  /********************************************************************/
  if ( ( usRC == QLDB_NO_ERROR ) && ( ppszData == NULL ) &&
       ( usTotalFields != 0 ) )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */


  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*           create and fill in the tree handle structre          */
    /******************************************************************/
    usRC = QLDBCreateTreeHandle( ausNoOfFields, pphTree );

    /******************************************************************/
    /*   set the pointer to the level data fields to the beginning    */
    /*                       of the data array                        */
    /******************************************************************/
    if ( ppszData != NULL )
    {
      ppszTempData = ppszData;
    }
    else
    {
      ppszTempData = NULL;
    } /* endif */

    /******************************************************************/
    /*               Now create one node for each level               */
    /******************************************************************/
    for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
          i++ )
    {
      /****************************************************************/
      /*      create the node and fill in the data for that node      */
      /****************************************************************/
      usRC = QLDBCreateNode( *pphTree, (USHORT)(i + 1), ppszTempData, &pNode );
      if ( usRC != QLDB_NO_ERROR )
      {
        /**************************************************************/
        /*     destroy the already built up tree and destroy the      */
        /*                   tree handle structure                    */
        /**************************************************************/
        QLDBDestroyIncompleteSubtree( *pphTree, 1 );
        UtlAlloc( (PVOID *)pphTree, 0L, 0L, NOMSG );
        *pphTree = NULL;
      }
      else
      {
        /**************************************************************/
        /*     if the node created is not on the first level then     */
        /*        pointer the created node to its parent node         */
        /**************************************************************/
        if ( i != 0 )
        {
          (*pphTree)->apCurLevelNode[i-1]->pChild = pNode;
          pNode->pParent = (*pphTree)->apCurLevelNode[i-1];
        }
        else
        {
          (*pphTree)->pCurNode = pNode;
        } /* endif */

        /**************************************************************/
        /*   set node level pointer in tree handle to created node    */
        /**************************************************************/
        (*pphTree)->apCurLevelNode[i] = pNode;

        /**************************************************************/
        /*      set the temp data pointer to the first field of       */
        /*                the data for the next level                 */
        /**************************************************************/
        if ( ppszTempData != NULL )
        {
          ppszTempData += ausNoOfFields[i];
        } /* endif */
      } /* endif */
    } /* endfor */
  } /* endif */

  return( usRC );

} /* end of function QLDBCreateTree */



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBCurrNode gets the data from current node                 |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBCurrNode( phTree, ppszData, pusLevel )                         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function returns the data fields of the current node and the         |
//|  level number of the current node. The data is passed via an array of      |
//|  pointers referencing the data.                                            |
//|                                                                            |
//|  In case of any problems the level number will be QLDB_END_OF_TREE and     |
//|  the pointers in the pointer array are set to NULL.                        |
//|                                                                            |
//|  If the node contains no fields at all the pointers in the pointer array   |
//|  will not change.                                                          |
//|                                                                            |
//|  The function expects an array of pointers to zero-terminated strings.     |
//|  The number of pointers should be the same as the number of fields in      |
//|  the template, because the calling application doesn't know which          |
//|  level (and therefore how many fields) will be returned.                   |
//|  This processing also allows multiple calls to QLDBCurrNode with the       |
//|  same storage allocated for the array of pointers and therefore a better   |
//|  performance than an allocation every time when QLDBCurrNode is called.    |
//|                                                                            |
//|  The function is able to work with records. The results will be the same   |
//|  as described above.                                                       |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PSZ          *ppszData;                        // pointer to an           |
//|                                                 // array of pointers       |
//|                                                 // pointing to the data    |
//|                                                 // that is read from the   |
//|                                                 // next node               |
//|  USHORT       *pusLevel;                        // level of node           |
//|                                                 // can be:                 |
//|                                                 // 2 to QLDB_MAX_LEVELS    |
//|                                                 // QLDB_END_OF_TREE        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_NO_VALID_DATA          - the pointer to data fields did not exist    |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle                                                     |
//|  - a pointer array that should contain at least the same number of         |
//|    pointers as the greatest amount of fields in a node                     |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the current node pointer is changed                                     |
//|  - the current level pointers are changed so that they form a new          |
//|    valid template                                                          |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBCurrNode( phTree, ppszData, &usLevel );                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|  ELSE                                                                      |
//|    Set returned level number to level number in current node               |
//|    Set pointer array to data fields                                        |
//+----------------------------------------------------------------------------+

USHORT QLDBCurrNode
(
  PVOID        pvTree,                           // handle to tree
  PSZ_W        *ppszData,                        // pointer to an
                                                 // array of pointers
                                                 // pointing to the data
                                                 // that is read from the
                                                 // next node
  USHORT       *pusLevel                         // level of node
                                                 // can be:
                                                 // 2 to QLDB_MAX_LEVELS
                                                 // QLDB_END_OF_TREE
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  USHORT       usTotalFields = 0;                // number of fields in
                                                 // node
  USHORT       usBufCount;                       // counter for the
                                                 // esc field buffer in
                                                 // the tree handle
  PSZ_W        pszDataInRec;                     // pointer to the
                                                 // current field in the
                                                 // record



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pusLevel = QLDB_END_OF_TREE;

  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( ( phTree->apCurLevelNode[i] == NULL ) &&
         ( phTree->apszCurLevelRec[i] == NULL ) )
    {
      usRC = QLDB_NO_TREEHANDLE;
    } /* endif */
  } /* endfor */

  /********************************************************************/
  /*        compute the maximum number of fields in the nodes         */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    QLDB_MAX_NODE_FIELDS( phTree, &usTotalFields );
  } /* endif */

  /********************************************************************/
  /*      check if the pointer to the pointer array is not NULL       */
  /********************************************************************/
  if ( ( usTotalFields != 0 ) && ( ppszData == NULL ) &&
       ( usRC == QLDB_NO_ERROR ) )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          check if the tree handle references a record          */
    /******************************************************************/
    if ( phTree->fWorkWithRecord == TRUE )
    {
      /****************************************************************/
      /*          get the current node data from the record           */
      /****************************************************************/
      /****************************************************************/
      /*                     get the level number                     */
      /****************************************************************/
      *pusLevel = QLDB_LEVEL( phTree->pszCurRec );
      if ( *pusLevel > QLDB_MAX_LEVELS )
      {
        usRC = QLDB_NO_VALID_DATA;
      }
      else
      {
        /****************************************************************/
        /*       now we set the data pointer array to the current       */
        /*                 record pointer's data fields                 */
        /****************************************************************/
        if ( phTree->ausNoOfFields[(*pusLevel)-1] != 0 )
        {
          usBufCount = 0;

          pszDataInRec = phTree->pszCurRec + 1;

          for ( i = 0; ( i < phTree->ausNoOfFields[(*pusLevel)-1] ) &&
                      ( usRC == QLDB_NO_ERROR ); i++ )
          {
            /************************************************************/
            /*       set the pointers in the pointer array to the       */
            /*                       data fields                        */
            /*       but check whether they contain QLDB_ESC_CHAR       */
            /************************************************************/
            if ( phTree->usEscFields != 0 )
            {
              usRC = QLDBFilterEscChar( phTree, &usBufCount,
                                        &pszDataInRec, &( ppszData[i] ) );
            }
            else
            {
              ppszData[i] = pszDataInRec;
              pszDataInRec += UTF16strlenCHAR( pszDataInRec ) + 1;
            } /* endif */
          } /* endfor */
        } /* endif */
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /*           get the current node data from the tree            */
      /****************************************************************/
      *pusLevel = phTree->pCurNode->usLevel;

      if ( phTree->ausNoOfFields[(*pusLevel)-1] != 0 )
      {
        /**************************************************************/
        /*   ok, then set the pointers in the pointer array to the    */
        /*                  data fields in the node                   */
        /**************************************************************/
        for ( i = 0; i < phTree->ausNoOfFields[(*pusLevel)-1]; i++ )
        {
          ppszData[i] = phTree->pCurNode->aFields[i].pszData;
        } /* endfor */
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*         if the output level is QLDB_END_OF_TREE set the          */
  /*               data pointer array to NULL pointers                */
  /********************************************************************/
  if ( ( *pusLevel == QLDB_END_OF_TREE ) && ( ppszData != NULL ) )
  {
    if ( usTotalFields != 0 )
    {
      for ( i = 0; i < usTotalFields; i++ )
      {
        ppszData[i] = NULL;
      } /* endfor */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBCurrNode */



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBCurrTemplate gets the data from the current template     |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBCurrTemplate( phTree, ppszData )                               |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  Returns the current template of the node tree.                            |
//|  A template is a target level node plus all parent nodes.                  |
//|                                                                            |
//|  The function returns an array of pointers to the data fields.             |
//|  It expects a pointer to an array of pointers as input. This array of      |
//|  pointers will then be filled with the pointers to the data fields of      |
//|  each node on each level. The calling application therefore                |
//|  has to keep the information about how many fields are on each level.      |
//|  Nodes containing no fields will not be referenced in the pointer          |
//|  array thus the pointer array contains only pointers to fields             |
//|  that may contain data.                                                    |
//|                                                                            |
//|  If each node in each level contains no fields the pointer to the pointer  |
//|  array is expected to be a NULL pointer.                                   |
//|                                                                            |
//|  This function is able to work with records. The calling application       |
//|  will get the same results as described above.                             |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//|  PSZ          *ppszData;                        // pointer to an           |
//|                                                 // array of pointers       |
//|                                                 // pointing to the data    |
//|                                                 // that is read from the   |
//|                                                 // nodes in the current    |
//|                                                 // template                |
//+----------------------------------------------------------------------------+
//|Output parameter: none                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_NO_VALID_DATA          - the pointer to data fields did not exist    |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects: none                                                          |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBCurrTemplate( phTree, ppszData );                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|    IF total number of fields != 0                                          |
//|      Set pointer array to data fields ( watch the ESC_CHAR )               |
//|  ELSE                                                                      |
//|    IF total number of fields != 0                                          |
//|      Set pointer array to data fields                                      |
//+----------------------------------------------------------------------------+

USHORT QLDBCurrTemplate
(
  PVOID        pvTree,                           // handle to tree
  PSZ_W        *ppszData                         // pointer to an
                                                 // array of pointers
                                                 // pointing to the data
                                                 // that is read from the
                                                 // nodes in the current
                                                 // template
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       usTotalFields = 0;                // number of fields in
                                                 // template
  USHORT       i;                                // an index
  USHORT       j;                                // another index
  USHORT       usPrevFields = 0;                 // no. of fields on all
                                                 // former levels
  USHORT       usBufCount;                       // counter for the
                                                 // esc field buffer in
                                                 // the tree handle
  PSZ_W        pszDataInRec;                     // pointer to the
                                                 // current field in the
                                                 // record



  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( ( phTree->apCurLevelNode[i] == NULL ) &&
         ( phTree->apszCurLevelRec[i] == NULL ) )
    {
      usRC = QLDB_NO_TREEHANDLE;
    } /* endif */
  } /* endfor */

  /********************************************************************/
  /*        compute the number of total fields in the template        */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    QLDB_TOTAL_FIELDS( phTree, &usTotalFields );
  } /* endif */

  /********************************************************************/
  /*   check if the pointer to the pointer array is a valid pointer   */
  /********************************************************************/
  if ( ( usRC == QLDB_NO_ERROR ) && ( ppszData == NULL ) &&
       ( usTotalFields != 0 ) )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */


  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          check if the tree handle references a record          */
    /******************************************************************/
    if ( phTree->fWorkWithRecord == TRUE )
    {
      /****************************************************************/
      /*   set the pointers in the pointer array to the data fields   */
      /****************************************************************/
      if ( usTotalFields != 0 )
      {
        usBufCount = 0;

        for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
              i++ )
        {
          pszDataInRec = phTree->apszCurLevelRec[i] + 1;

          /************************************************************/
          /*       get the number of fields of the former levels      */
          /************************************************************/
          if ( i == 0 )
          {
            usPrevFields = 0;
          }
          else
          {
            usPrevFields = usPrevFields + phTree->ausNoOfFields[i-1];
          } /* endif */

          /************************************************************/
          /*            loop over the fields in the nodes             */
          /************************************************************/
          for ( j = 0; ( j < phTree->ausNoOfFields[i]) &&
                       ( usRC == QLDB_NO_ERROR ); j++ )
          {
            /**********************************************************/
            /*      set the pointers in the pointer array to the      */
            /*                      data fields                       */
            /*      but check whether they contain QLDB_ESC_CHAR      */
            /**********************************************************/
            if ( phTree->usEscFields != 0 )
            {
              usRC = QLDBFilterEscChar( phTree, &usBufCount,
                                        &pszDataInRec,
                                        &( ppszData[usPrevFields+j] ) );
            }
            else
            {
              ppszData[usPrevFields+j] = pszDataInRec;
              pszDataInRec += UTF16strlenCHAR( pszDataInRec ) + 1;
            } /* endif */
          } /* endfor */
        } /* endfor */
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /*   set the pointers in the pointer array to the data fields   */
      /****************************************************************/
      if ( usTotalFields != 0 )
      {
        /**************************************************************/
        /*                    loop over the levels                    */
        /**************************************************************/
        for ( i = 0; ( i < QLDB_MAX_LEVELS ) &&
                     ( usRC == QLDB_NO_ERROR ); i++ )
        {
          /************************************************************/
          /*       get the number of fields of the former levels      */
          /************************************************************/
          if ( i == 0 )
          {
            usPrevFields = 0;
          }
          else
          {
            usPrevFields = usPrevFields + phTree->ausNoOfFields[i-1];
          } /* endif */

          /************************************************************/
          /*            loop over the fields in the nodes             */
          /************************************************************/
          for ( j = 0; ( j < phTree->ausNoOfFields[i]) &&
                       ( usRC == QLDB_NO_ERROR ); j++ )
          {
            /**********************************************************/
            /*      set the pointers in the pointer array to the      */
            /*                      data fields                       */
            /**********************************************************/
            ppszData[usPrevFields+j] =
              phTree->apCurLevelNode[i]->aFields[j].pszData;
          } /* endfor */
        } /* endfor */
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*                  if  an error occurred set the                   */
  /*               data pointer array to NULL pointers                */
  /********************************************************************/
  if ( ( usRC != QLDB_NO_ERROR ) && ( ppszData != NULL ) )
  {
    if ( usTotalFields != 0 )
    {
      for ( i = 0; i < usTotalFields; i++ )
      {
        ppszData[i] = NULL;
      } /* endfor */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBCurrTemplate */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBDestroySubtree                                           |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBDestroySubtree( pphTree )                                      |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function destroys the subtree of the current node pointer.           |
//|  If the current node has no sister nodes (neither to the left node nor to  |
//|  the right) the next parent node of the current node with sister nodes is  |
//|  computed. If there are no parent nodes of the current node with sister    |
//|  nodes the whole tree will be destroyed. This processing is necessary in   |
//|  order to maintain always consistent and full templates in the tree.       |
//|                                                                            |
//|  If single templates are to be destroyed the tree should be flattened in   |
//|  order to assure that two nodes with the same (higher) level number        |
//|  but NOT belonging to the same template are destroyed by calling this      |
//|  function.                                                                 |
//|                                                                            |
//|  The current node will become the node left of the node of which the       |
//|  subtree and itself is to be destroyed. If there is no node to the left    |
//|  the node to the right will become the current node. If the current node   |
//|  has no sister nodes at all the current node will become the parent node   |
//|  of the current node.                                                      |
//|                                                                            |
//|  This function is able to work with records. If a record is to be          |
//|  processed the function converts the record in a tree before it starts     |
//|  working.                                                                  |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  *pphTree;                         // pointer to tree handle  |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_TOO_MANY_FIELDS        - too many fields for a level were requested  |
//|  QLDB_NO_VALID_DATA          - the pointer to data fields did not exist    |
//|  QLDB_ERROR_IN_TREE          - an unrecoverable error is in the tree       |
//|  QLDB_ERR_DEALLOC            - error deallocating memory                   |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  destroys the subtree of the current node and if necessary a subtree       |
//|  above the current node (if the tree consists of only one template         |
//|  this is the whole tree)                                                   |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBDestroySubtree( &phTree );                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|    Convert the record to a tree (QLDBRecordToTree)                         |
//|  Set temp pointer to pointer of current node                               |
//|  CALL QLDBComputeSubtreeToDestroy with temp pointer                        |
//|  IF pRight of returned node pointer != NULL pointer                        |
//|    IF pLeft of returned node pointer != NULL pointer                       |
//|      Set pRight of pLeft of returned node pointer to                       |
//|        pRight of returned node pointer                                     |
//|      Set pLeft of pRight of returned node pointer to                       |
//|        pLeft of returned node pointer                                      |
//|      Set pointer of current node to pLeft of returned node pointer         |
//|    ELSE                                                                    |
//|      Set pLeft of pRight of returned node pointer to NULL pointer          |
//|      Set pChild of pParent of returned node pointer to                     |
//|        pRight of returned node pointer                                     |
//|      Set pointer of current node to pRight of returned node pointer        |
//|  ELSE                                                                      |
//|    IF pLeft of returned node pointer != NULL pointer                       |
//|      Set pRight of pLeft of returned node pointer to NULL pointer          |
//|      Set pointer of current node to pLeft of returned node pointer         |
//|    ELSE                                                                    |
//|      Deallocate tree handle structure (because this case means that        |
//|        the whole tree will be destroyed)                                   |
//|      Set pointer to tree handle to NULL pointer                            |
//|  IF pointer to tree handle != NULL pointer                                 |
//|    Set level node pointer of level of current node to current node         |
//|    FOR level number of current node  + 1 TO MAX_LEVELS DO                  |
//|      Set level node pointer of level number to                             |
//|        node of level node with level number - 1                            |
//|  CALL QLDBRemoveChildSubtree with returned node pointer                    |
//|  CALL QLDBDestroyNode with returned node pointer                           |
//+----------------------------------------------------------------------------+

USHORT QLDBDestroySubtree
(
  PVOID        *ppvTree                          // pointer to tree handle
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  *pphTree = (PQLDB_HTREE *)ppvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  PQLDB_NODE   pTempNode = NULL;                 // pointer to node of
                                                 // which subtree is to
                                                 // destroy
  PQLDB_NODE   pTempCurNode = NULL;              // former current node
                                                 // pointer
  PQLDB_NODE   apTempNodes[QLDB_MAX_LEVELS];     // temp array of pointers
                                                 // to allow restoring
                                                 // the old current node
                                                 // pointers in case of
                                                 // problems



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
  {
    apTempNodes[i] = NULL;
  } /* endfor */

  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( *pphTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          check if the tree handle references a record          */
    /******************************************************************/
    if ( (*pphTree)->fWorkWithRecord == TRUE )
    {
      /****************************************************************/
      /*                 convert the record to a tree                 */
      /****************************************************************/
      usRC = QLDBRecordToTree( (*pphTree)->ausNoOfFields,
                               (*pphTree)->apszCurLevelRec[0],
                               0, (PVOID *)pphTree );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( (*pphTree)->apCurLevelNode[i] == NULL )
    {
      usRC = QLDB_NO_TREEHANDLE;
    } /* endif */
  } /* endfor */


  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*               now compute the subtree to destroy               */
    /******************************************************************/
    pTempCurNode = (*pphTree)->pCurNode;
    usRC = QLDBComputeSubtreeToDestroy( (*pphTree)->pCurNode,
                                        &pTempNode );
  } /* endif */

  /********************************************************************/
  /*    now chain the tree so that it forms a new valid tree after    */
  /*                  the subtree has been destroyed                  */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    if ( pTempNode->pRight != NULL )
    {
      if ( pTempNode->pLeft != NULL )
      {
        pTempNode->pLeft->pRight = pTempNode->pRight;
        pTempNode->pRight->pLeft = pTempNode->pLeft;
        (*pphTree)->pCurNode = pTempNode->pLeft;
      }
      else
      {
        pTempNode->pRight->pLeft = NULL;
        pTempNode->pParent->pChild = pTempNode->pRight;
        (*pphTree)->pCurNode = pTempNode->pRight;
      } /* endif */
    }
    else
    {
      if ( pTempNode->pLeft != NULL )
      {
        pTempNode->pLeft->pRight = NULL;
        (*pphTree)->pCurNode = pTempNode->pLeft;
      }
      else
      {
        /**************************************************************/
        /*        deallocate the esc field buffer if it exists        */
        /**************************************************************/
        if ( (*pphTree)->pchrEscBuffer != NULL )
        {
          if ( !( UtlAlloc( (PVOID *)&( (*pphTree)->pchrEscBuffer ), 0L, 0L,
                            NOMSG ) ) )
          {
            usRC = QLDB_ERR_DEALLOC;
          } /* endif */
        } /* endif */

        /**************************************************************/
        /*              deallocate the handle structure               */
        /**************************************************************/
        if ( usRC == QLDB_NO_ERROR )
        {
          if ( !( UtlAlloc( (PVOID *)pphTree, 0L, 0L, NOMSG ) ) )
          {
            usRC = QLDB_ERR_DEALLOC;
          } /* endif */
        } /* endif */

        *pphTree = NULL;
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*                       set the new template                       */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    if ( *pphTree != NULL )
    {
      (*pphTree)->apCurLevelNode[((*pphTree)->pCurNode->usLevel)-1] =
         (*pphTree)->pCurNode;

      /****************************************************************/
      /*               build the right subtree template               */
      /****************************************************************/
      for ( i = (*pphTree)->pCurNode->usLevel; i < QLDB_MAX_LEVELS; i++ )
      {
        (*pphTree)->apCurLevelNode[i] =
                    (*pphTree)->apCurLevelNode[i-1]->pChild;
      } /* endfor */

      /****************************************************************/
      /*               build the right parent template                */
      /****************************************************************/
      for ( i = ((*pphTree)->pCurNode->usLevel) - 1; i >= 1 ; i-- )
      {
        (*pphTree)->apCurLevelNode[i-1] =
                    (*pphTree)->apCurLevelNode[i]->pParent;
      } /* endfor */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*                now eventually destroy the subtree                */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    usRC = QLDBRemoveChildSubtree( *pphTree, pTempNode );

    if ( usRC == QLDB_NO_ERROR )
    {
      usRC = QLDBDestroyNode( *pphTree, &pTempNode );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*         if an error occurred restore the saved pointers          */
  /********************************************************************/
  if ( usRC != QLDB_NO_ERROR )
  {
    (*pphTree)->pCurNode = pTempCurNode;
    for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
    {
      (*pphTree)->apCurLevelNode[i] = apTempNodes[i];
    } /* endfor */
  } /* endif */


  return( usRC );

} /* end of function QLDBDestroySubtree */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBDestroyTree removes a node tree                          |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBDestroyTree( pphTree )                                         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  Removes a tree from memory. The tree to be removed is described by its    |
//|  tree handle.                                                              |
//|                                                                            |
//|  The function will remove all nodes, external data areas and the tree      |
//|  handle structure.                                                         |
//|  The handle structure is then removed as well.                             |
//|                                                                            |
//|  This function is able to work with records. If a record is to be          |
//|  processed the function deallocates the record and the tree handle         |
//|  structure. The pointer to the tree handle is set to NULL.                 |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  *pphTree;                         // pointer to tree handle  |
//+----------------------------------------------------------------------------+
//|Output parameter: none                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_ERR_DEALLOC            - error deallocating memory                   |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the complete tree is destroyed                                          |
//|  - the handle structure is deallocated                                     |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBDestroyTree( &phTree );                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|    Deallocate record                                                       |
//|  ELSE                                                                      |
//|    CALL QLDBRemoveChildSubtree with pointer to first level node            |
//|    CALL QLDBDestroyNode with pointer to first level node                   |
//|  Deallocate the tree handle structure                                      |
//+----------------------------------------------------------------------------+

USHORT QLDBDestroyTree
(
  PVOID        *ppvTree                         // pointer to tree handle
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  *pphTree = (PQLDB_HTREE *)ppvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index



  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( *pphTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( ( (*pphTree)->apCurLevelNode[i] == NULL ) &&
         ( (*pphTree)->apszCurLevelRec[i] == NULL ) )
    {
      usRC = QLDB_NO_TREEHANDLE;
    } /* endif */
  } /* endfor */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          check if the tree handle references a record          */
    /******************************************************************/
    if ( (*pphTree)->fWorkWithRecord == TRUE )
    {
      /****************************************************************/
      /*                    deallocate the record                     */
      /****************************************************************/
      (*pphTree)->apszCurLevelRec[0] -= QLDB_START_CTRL_INFO;
      if ( !( UtlAlloc( (PVOID *)&((*pphTree)->apszCurLevelRec[0]), 0L, 0L,
                        NOMSG ) ) )
      {
        usRC = QLDB_ERR_DEALLOC;
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /*    call QLDBRemoveChildSubtree to get rid of the subtree     */
      /*                   of the first level node                    */
      /****************************************************************/
      usRC = QLDBRemoveChildSubtree( *pphTree,
                                     (*pphTree)->apCurLevelNode[0] );

      if ( usRC == QLDB_NO_ERROR )
      {
        /**************************************************************/
        /*    call QLDBDestroyNode to destroy the first level node    */
        /**************************************************************/
        usRC = QLDBDestroyNode( *pphTree,
                                &((*pphTree)->apCurLevelNode[0]) );
      } /* endif */
    } /* endif */

    if ( usRC == QLDB_NO_ERROR )
    {
      /****************************************************************/
      /*         deallocate the esc field buffer if it exists         */
      /****************************************************************/
      if ( (*pphTree)->pchrEscBuffer != NULL )
      {
        if ( !( UtlAlloc( (PVOID *)&( (*pphTree)->pchrEscBuffer ), 0L, 0L,
                          NOMSG ) ) )
        {
          usRC = QLDB_ERR_DEALLOC;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /*               deallocate the handle structure                */
      /****************************************************************/
      if ( usRC == QLDB_NO_ERROR )
      {
        if ( !( UtlAlloc( (PVOID *)pphTree, 0L, 0L, NOMSG ) ) )
        {
          usRC = QLDB_ERR_DEALLOC;
        } /* endif */
      } /* endif */

      *pphTree = NULL;
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBDestroyTree */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBFlattenTree creates fully widespread tree                |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBFlattenTree( pphTree )                                         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function converts a node tree to a tree where all nodes except the   |
//|  first level node have only one child node.                                |
//|                                                                            |
//|  The positions of level pointers and current pointer are reset after the   |
//|  function has finished.                                                    |
//|                                                                            |
//|  This function is able to work with records. If a record is found it is    |
//|  converted to a tree before the processing of flattening the tree starts.  |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  *pphTree;                         // pointer to tree handle  |
//+----------------------------------------------------------------------------+
//|Output parameter: none                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_TOO_MANY_FIELDS        - too many fields for a level were requested  |
//|  QLDB_NO_VALID_DATA          - the data fields did not exist               |
//|  QLDB_INVALID_LEVEL          - invalid level number entered                |
//|  QLDB_ERROR_IN_TREE          - an unrecoverable error is in the tree       |
//|  QLDB_ERR_DEALLOC            - error deallocating memory                   |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the input tree is copied into a flat tree and then destroyed            |
//|  - the flat tree handle will be returned in the input parameter            |
//|    pphTree                                                                 |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBFlattenTree( &phTree );                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Reset tree postions (QLDBResetTreePositions)                              |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|    Convert the record to a tree (QLDBRecordToTree)                         |
//|  Create temp tree with data from the first template (QLDBCreateTree)       |
//|  Get next template (QLDBNextTemplate)                                      |
//|  WHILE returned level number != END_OF_TREE                                |
//|    Add subtree on level number 2 with data from current template           |
//|      (QLDBAddSubtree)                                                      |
//|    Get next template (QLDBNextTemplate)                                    |
//|  Reset tree positions (QLDBResetTreePositions)                             |
//+----------------------------------------------------------------------------+

USHORT QLDBFlattenTree
(
  PVOID        *ppvTree                          // pointer to tree handle
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  *pphTree = (PQLDB_HTREE *)ppvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  PQLDB_HTREE  phTempTree = NULL;                // temporaray tree
  USHORT       i;                                // an index
  USHORT       usTotalFields = 0;                // no of total fields
                                                 // in a template
  USHORT       usLevel = 0;                      // level number
  PSZ_W        *ppszData = NULL;                 // pointer array for
                                                 // passing data



  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( *pphTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*                    reset the tree positions                    */
    /******************************************************************/
    usRC = QLDBResetTreePositions( *pphTree );
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*    check if the tree is a record and so has to be converted    */
    /******************************************************************/
    if ( (*pphTree)->fWorkWithRecord == TRUE )
    {
      usRC = QLDBRecordToTree( (*pphTree)->ausNoOfFields,
                               (*pphTree)->apszCurLevelRec[0],
                               0, (PVOID *)pphTree );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( (*pphTree)->apCurLevelNode[i] == NULL )
    {
      usRC = QLDB_NO_TREEHANDLE;
    }
    else
    {
      usTotalFields = usTotalFields + (*pphTree)->ausNoOfFields[i];
    } /* endif */
  } /* endfor */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          allocate storage for the data pointer array           */
    /******************************************************************/
    if ( usTotalFields != 0 )
    {
      if ( !( UtlAlloc( (PVOID *)&ppszData, 0L,
                        (LONG)max( MIN_ALLOC, usTotalFields * sizeof(PSZ_W) ),
                        NOMSG ) ) )
      {
        usRC = QLDB_NO_MEMORY;
      } /* endif */
    } /* endif */
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*              get the data from the first template              */
    /******************************************************************/
    usRC = QLDBCurrTemplate( *pphTree, ppszData );
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*                   create the temporary tree                    */
    /******************************************************************/
    usRC = QLDBCreateTree( (*pphTree)->ausNoOfFields, ppszData,
                           (PVOID *)&phTempTree );
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*                     get the next template                      */
    /******************************************************************/
    usRC = QLDBNextTemplate( *pphTree, ppszData, &usLevel );
  } /* endif */

  /********************************************************************/
  /*       now build up the whole temporaray tree by adding on        */
  /*               subtrees on level 2 of all templates               */
  /********************************************************************/
  while ( ( usLevel != QLDB_END_OF_TREE ) && ( usRC == QLDB_NO_ERROR ) )
  {
    /******************************************************************/
    /*      add subtree on level number 2 with data from current      */
    /*                            template                            */
    /******************************************************************/
    usRC = QLDBAddSubtree( phTempTree, 2,
                           &(ppszData[(*pphTree)->ausNoOfFields[0]]) );

    if ( usRC == QLDB_NO_ERROR )
    {
      /****************************************************************/
      /*                    get the next template                     */
      /****************************************************************/
      usRC = QLDBNextTemplate( *pphTree, ppszData, &usLevel );
    } /* endif */
  } /* endwhile */

  /********************************************************************/
  /*                     now destroy the old tree                     */
  /*       if an error occurred destroy the newly created tree        */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    usRC = QLDBDestroyTree( (PVOID *)pphTree );
  }
  else
  {
    if ( phTempTree != NULL )
    {
      QLDBDestroyTree( (PVOID *)&phTempTree );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*                     reset the tree positions                     */
  /* and set the returned tree handle to the newly created flat tree  */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    *pphTree = phTempTree;
    usRC = QLDBResetTreePositions( *pphTree );
  } /* endif */

  /********************************************************************/
  /*                     deallocate pointer array                     */
  /********************************************************************/
  if ( ppszData != NULL )
  {
    UtlAlloc( (PVOID *)&ppszData, 0L, 0L, NOMSG );
  } /* endif */

  return( usRC );

} /* end of function QLDBFlattenTree */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBJoinSameNodes joins same nodes on same levels            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBJoinSameNodes( phTree )                                        |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function scans the tree and joins nodes on the same level which      |
//|  have the same data in their data fields.                                  |
//|                                                                            |
//|  The positions of level pointers and current pointer are reset after the   |
//|  function has finished.                                                    |
//|                                                                            |
//|  This function is able to work with records. If a record is found it is    |
//|  converted to a tree before the processing starts.                         |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // pointer to tree handle  |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  none                                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_TOO_MANY_FIELDS        - too many fields for a level were requested  |
//|  QLDB_NO_VALID_DATA          - the pointer to data fields did not exist    |
//|  QLDB_ERROR_IN_TREE          - an unrecoverable error is in the tree       |
//|  QLDB_ERR_DEALLOC            - error deallocating memory                   |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - nodes on same levels and same subtrees which are the same are           |
//|    joined together                                                         |
//|  - current node and level pointers are reset                               |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBJoinSameNodes( phTree );                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Reset tree postions (QLDBResetTreePositions)                              |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|    Convert the record to a tree (QLDBRecordToTree)                         |
//|  CALL QLDBJoinSameSubtree with pChild of first level node pointer          |
//|  Reset tree postions (QLDBResetTreePositions)                              |
//+----------------------------------------------------------------------------+

USHORT QLDBJoinSameNodes
(
  PVOID        pvTree                            // pointer to tree handle
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index

  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*                    reset the tree positions                    */
    /******************************************************************/
    usRC = QLDBResetTreePositions( phTree );
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*    check if the tree is a record and so has to be converted    */
    /******************************************************************/
    if ( phTree->fWorkWithRecord == TRUE )
    {
      usRC = QLDBRecordToTree( phTree->ausNoOfFields,
                               phTree->apszCurLevelRec[0],
                               0, (PVOID *)&phTree );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( phTree->apCurLevelNode[i] == NULL )
    {
      usRC = QLDB_NO_TREEHANDLE;
    } /* endif */
  } /* endfor */

  /********************************************************************/
  /*     now join all subtrees starting from the child subtree of     */
  /*                     the first level pointer                      */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    if ( phTree->apCurLevelNode[0]->pChild != NULL )
    {
      usRC = QLDBJoinSameSubtree( phTree,
                                  phTree->apCurLevelNode[0]->pChild );
    }
    else
    {
      usRC = QLDB_ERROR_IN_TREE;
    } /* endif */
  } /* endif */


  /********************************************************************/
  /*                     reset the tree positions                     */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    usRC = QLDBResetTreePositions( phTree );
  } /* endif */

  return( usRC );

} /* end of function QLDBJoinSameNodes */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBMoveToNode moves current node to specific location       |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBMoveToNode( phTree, usRelationship, ppszData, pusLevel )       |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function returns a node which has a specified relationship to the    |
//|  current node in the node tree. The returned node gets the new current     |
//|  node.                                                                     |
//|                                                                            |
//|  The relationships may be:                                                 |
//|    - QLDB_NEXT_NODE:   next node (same as QLDBNextNode)                    |
//|    - QLDB_PREV_NODE:   previous node (same as QLDBPrevNode)                |
//|    - QLDB_NEXT_NODE_ON_SAME_LEVEL: next node on same level                 |
//|    - QLDB_PREV_NODE_ON_SAME_LEVEL: previous node on same level             |
//|    - QLDB_CHILD_NODE:  child node of current node                          |
//|    - QLDB_PARENT_NODE: parent node of current node                         |
//|                                                                            |
//|  If no node with the specified relationship is available QLDB_END_OF_TREE  |
//|  is returned.                                                              |
//|  Otherwise the level of the node and its data pointer are returned.        |
//|  The sequence of node processing is top-to-bottom and left-to-right.       |
//|                                                                            |
//|  As the current pointer walks along the tree the level pointers are        |
//|  updated in order to form a valid template.                                |
//|  If the function returns QLDB_END_OF_TREE the level pointers and the       |
//|  current pointer will not change.                                          |
//|  The pointer array for the data is set to all pointers being NULL pointers.|
//|                                                                            |
//|  The calling function has to assure that the array is large enough to      |
//|  hold the pointers to all fields of the returned node.                     |
//|  If the node contains no fields at all the pointer array is set to         |
//|  NULL, the level returned is set accordingly and the returncode will be    |
//|  QLDB_NO_ERROR.                                                            |
//|                                                                            |
//|  This function is able to work with records. The processing is as          |
//|  described above.                                                          |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//|  USHORT       usRelationship;                   // the relationsship       |
//|                                                 // between the current     |
//|                                                 // node and the node to    |
//|                                                 // where to move           |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PSZ          *ppszData;                        // pointer to an           |
//|                                                 // array of pointers       |
//|                                                 // pointing to the data    |
//|                                                 // that is read from the   |
//|                                                 // next node               |
//|  USHORT       *pusLevel;                        // level of node           |
//|                                                 // can be:                 |
//|                                                 // 2 to QLDB_MAX_LEVELS    |
//|                                                 // QLDB_END_OF_TREE        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_NO_VALID_DATA          - the pointer to data fields did not exist    |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle                                                     |
//|  - a pointer array that should contain at least the same number of         |
//|    pointers as the greatest amount of fields in a node                     |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the current node pointer is changed                                     |
//|  - the current level pointers are changed so that they form a new          |
//|    valid template                                                          |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBMoveToNode( phTree, QLDB_CHILD_NODE, ppszData, &usLevel );|
//|                                                                            |
//|  gets the child node of the current node                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Save level pointers and current pointer                                   |
//|  SWITCH on the relationship                                                |
//|    CASE next node                                                          |
//|      CALL QLDBNextNode                                                     |
//|                                                                            |
//|    CASE previous node                                                      |
//|      CALL QLDBPrevNode                                                     |
//|                                                                            |
//|    CASE next node on same level                                            |
//|      REPEAT                                                                |
//|        CALL QLDBNextNode                                                   |
//|      UNTIL returned level number == (level number of current node or       |
//|                                      QLDB_END_OF_TREE)                     |
//|    CASE previous node on same level                                        |
//|      REPEAT                                                                |
//|        CALL QLDBPrevNode                                                   |
//|      UNTIL returned level number == (level number of current node or       |
//|                                      QLDB_END_OF_TREE)                     |
//|    CASE child node                                                         |
//|      IF level number of current node < QLDB_MAX_LEVELS                     |
//|        REPEAT                                                              |
//|          CALL QLDBNextNode                                                 |
//|        UNTIL returned level number == (level number of current node  + 1   |
//|                                        or QLDB_END_OF_TREE)                |
//|      ELSE                                                                  |
//|        RETURN END_OF_TREE                                                  |
//|                                                                            |
//|    CASE parent node                                                        |
//|      IF level number of current node > 1                                   |
//|        REPEAT                                                              |
//|          CALL QLDBPrevNode                                                 |
//|        UNTIL returned level number == (level number of current node - 1    |
//|                                        or QLDB_END_OF_TREE)                |
//|      ELSE                                                                  |
//|        RETURN END_OF_TREE                                                  |
//|                                                                            |
//|    IF returned level number == END_OF_TREE                                 |
//|      Reset level pointers and current pointer in tree handle to            |
//|        saved pointers and saved current pointer                            |
//|      Set pointer array for data to NULL pointers                           |
//+----------------------------------------------------------------------------+

USHORT QLDBMoveToNode(

  PVOID        pvTree,                           // handle to tree
  USHORT       usRelationship,                   // the relationsship
                                                 // between the current
                                                 // node and the node to
                                                 // where to move
  PSZ_W        *ppszData,                        // pointer to an
                                                 // array of pointers
                                                 // pointing to the data
                                                 // that is read from the
                                                 // next node
  USHORT       *pusLevel )                       // level of node
                                                 // can be:
                                                 // 2 to QLDB_MAX_LEVELS
                                                 // QLDB_END_OF_TREE

{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       usTotalFields = 0;                // number of fields in
                                                 // node
  USHORT       i;                                // an index
  USHORT       usLevel;                          // level number
  PQLDB_NODE   pTempCurNode = NULL;              // former current node
                                                 // pointer
  PQLDB_NODE   apTempNodes[QLDB_MAX_LEVELS];     // temp array of pointers
                                                 // to allow restoring
                                                 // the old current node
                                                 // pointers in case of
                                                 // problems
  PSZ_W          pszTempCurRec = NULL;           // former current record
                                                 // pointer
  PSZ_W          apszTempRec[QLDB_MAX_LEVELS];     // temp array of pointers
                                                 // to allow restoring the
                                                 // old current record
                                                 // pointers in case of
                                                 // problems



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pusLevel = QLDB_END_OF_TREE;

  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /* as the loop already walks along the pointers to the level nodes  */
  /*     save them in an array to easily restore them in case of      */
  /*                          later problems                          */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( ( phTree->apCurLevelNode[i] == NULL ) &&
         ( phTree->apszCurLevelRec[i] == NULL ) )
    {
      usRC = QLDB_NO_TREEHANDLE;
    }
    else
    {
      if ( phTree->fWorkWithRecord == TRUE )
      {
        apszTempRec[i] = phTree->apszCurLevelRec[i];
      }
      else
      {
        apTempNodes[i] = phTree->apCurLevelNode[i];
      } /* endif */
    } /* endif */
  } /* endfor */

  /********************************************************************/
  /*        compute the maximum number of fields in the nodes         */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    QLDB_MAX_NODE_FIELDS( phTree, &usTotalFields );
  } /* endif */

  /********************************************************************/
  /*      check if the pointer to the pointer array is not NULL       */
  /********************************************************************/
  if ( ( usTotalFields != 0 ) && ( ppszData == NULL ) &&
       ( usRC == QLDB_NO_ERROR ) )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /****************************************************************/
    /*                  save the current pointers                   */
    /****************************************************************/
    pszTempCurRec = phTree->pszCurRec;
    pTempCurNode = phTree->pCurNode;

    /******************************************************************/
    /*      now let's see what relationship is to be processed        */
    /******************************************************************/
    switch ( usRelationship )
    {
      case QLDB_NEXT_NODE :
        {
          usRC = QLDBNextNode( phTree, ppszData, pusLevel );
        }
      break;

      case QLDB_PREV_NODE :
        {
          usRC = QLDBPrevNode( phTree, ppszData, pusLevel );
        }
      break;

      case QLDB_NEXT_NODE_ON_SAME_LEVEL :
        {
          /************************************************************/
          /*     get the level number of the former current level     */
          /************************************************************/
          if ( phTree->fWorkWithRecord == TRUE )
          {
            usLevel = QLDB_LEVEL( pszTempCurRec );
          }
          else
          {
            usLevel = pTempCurNode->usLevel;
          } /* endif */

          /************************************************************/
          /*    now loop until a node with the same level is found    */
          /*           or until and end of tree is reached            */
          /************************************************************/
          do
          {
            usRC = QLDBNextNode( phTree, ppszData, pusLevel );
          } while ( ( *pusLevel != usLevel ) &&
                    ( *pusLevel != QLDB_END_OF_TREE ) ); /* enddo */
        }
      break;

      case QLDB_PREV_NODE_ON_SAME_LEVEL :
        {
          /************************************************************/
          /*     get the level number of the former current level     */
          /************************************************************/
          if ( phTree->fWorkWithRecord == TRUE )
          {
            usLevel = QLDB_LEVEL( pszTempCurRec );
          }
          else
          {
            usLevel = pTempCurNode->usLevel;
          } /* endif */

          /************************************************************/
          /*    now loop until a node with the same level is found    */
          /*           or until and end of tree is reached            */
          /************************************************************/
          do
          {
            usRC = QLDBPrevNode( phTree, ppszData, pusLevel );
          } while ( ( *pusLevel != usLevel ) &&
                    ( *pusLevel != QLDB_END_OF_TREE ) ); /* enddo */
        }
      break;

      case QLDB_CHILD_NODE :
        {
          /************************************************************/
          /*     get the level number of the former current level     */
          /************************************************************/
          if ( phTree->fWorkWithRecord == TRUE )
          {
            usLevel = QLDB_LEVEL( pszTempCurRec );
          }
          else
          {
            usLevel = pTempCurNode->usLevel;
          } /* endif */

          /************************************************************/
          /*     now loop until a node with a level below (higher     */
          /*     level number) is found or end of tree is reached     */
          /*     but only if the current level number was not at      */
          /*                     the lowest level                     */
          /************************************************************/
          if ( usLevel < QLDB_MAX_LEVELS )
          {
            do
            {
              usRC = QLDBNextNode( phTree, ppszData, pusLevel );
            } while ( ( *pusLevel != ( usLevel + 1 ) ) &&
                      ( *pusLevel != QLDB_END_OF_TREE ) ); /* enddo */
          }
          else
          {
            *pusLevel = QLDB_END_OF_TREE;
          } /* endif */
        }
      break;

      case QLDB_PARENT_NODE :
        {
          /************************************************************/
          /*     get the level number of the former current level     */
          /************************************************************/
          if ( phTree->fWorkWithRecord == TRUE )
          {
            usLevel = QLDB_LEVEL( pszTempCurRec );
          }
          else
          {
            usLevel = pTempCurNode->usLevel;
          } /* endif */

          /************************************************************/
          /*     now loop until a node with a level higher (lower     */
          /*     level number) is found or end of tree is reached     */
          /*     but only if the current level number was not at      */
          /*                     the highest level                    */
          /************************************************************/
          if ( usLevel > 1 )
          {
            do
            {
              usRC = QLDBPrevNode( phTree, ppszData, pusLevel );
            } while ( ( *pusLevel != ( usLevel - 1 ) ) &&
                      ( *pusLevel != QLDB_END_OF_TREE ) ); /* enddo */
          }
          else
          {
            *pusLevel = QLDB_END_OF_TREE;
          } /* endif */
        }
      break;

      default :
        {
          *pusLevel = QLDB_END_OF_TREE;
        }
      break;
    } /* endswitch */
  } /* endif */

  /********************************************************************/
  /*             if the output level is QLDB_END_OF_TREE              */
  /*             restore saved current and level pointers             */
  /********************************************************************/
  if ( *pusLevel == QLDB_END_OF_TREE )
  {
    phTree->pszCurRec = pszTempCurRec;
    phTree->pCurNode = pTempCurNode;
    for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
    {
      phTree->apszCurLevelRec[i] = apszTempRec[i];
      phTree->apCurLevelNode[i] = apTempNodes[i];
    } /* endfor */
  } /* endif */

  /********************************************************************/
  /*         if the output level is QLDB_END_OF_TREE set the          */
  /*               data pointer array to NULL pointers                */
  /*           and restore saved current and level pointers           */
  /********************************************************************/
  if ( ( *pusLevel == QLDB_END_OF_TREE ) && ( ppszData != NULL ) )
  {
    if ( usTotalFields != 0 )
    {
      for ( i = 0; i < usTotalFields; i++ )
      {
        ppszData[i] = NULL;
      } /* endfor */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBMoveToNode */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBNextNode gets data from next node                        |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBNextNode( phTree, ppszData, pusLevel )                         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function returns the next node of the current node (precisely it     |
//|  returns an array of pointers to the data fields in the next node          |
//|  plus the level number of the next node).                                  |
//|                                                                            |
//|  The sequence of node processing is top-to-bottom and left-to-right.       |
//|                                                                            |
//|  The function changes the current node and the current template so that    |
//|  always a valid template is formed depending on the new current node.      |
//|                                                                            |
//|  If no more nodes are available the function returns QLDB_END_OF_TREE      |
//|  as level number and the pointers in the array are set to NULL.            |
//|  If the function returns QLDB_END_OF_TREE the current node and template    |
//|  pointers will not change.                                                 |
//|                                                                            |
//|  If the node contains no fields at all the pointers in the pointer array   |
//|  are left unchanged.                                                       |
//|                                                                            |
//|  The function expects an array of pointers to zero-terminated strings.     |
//|  The number of pointers should be the same as the number of fields in      |
//|  the template, because the calling application doesn't know which          |
//|  level (and therefore how many fields) will be returned.                   |
//|  This processing also allows multiple calls to QLDBNextNode with the       |
//|  same storage allocated for the array of pointers and therefore a better   |
//|  performance than an allocation every time when QLDBNextNode is called.    |
//|                                                                            |
//|  The function is able to work with records. The results will be the same   |
//|  as described above.                                                       |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PSZ          *ppszData;                        // pointer to an           |
//|                                                 // array of pointers       |
//|                                                 // pointing to the data    |
//|                                                 // that is read from the   |
//|                                                 // next node               |
//|  USHORT       *pusLevel;                        // level of node           |
//|                                                 // can be:                 |
//|                                                 // 2 to QLDB_MAX_LEVELS    |
//|                                                 // QLDB_END_OF_TREE        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_NO_VALID_DATA          - the pointer to data fields did not exist    |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle                                                     |
//|  - a pointer array that should contain at least the same number of         |
//|    pointers as the greatest amount of fields in a node                     |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the current node pointer is changed                                     |
//|  - the current level pointers are changed so that they form a new          |
//|    valid template                                                          |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBNextNode( phTree, ppszData, &usLevel );                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|    Find  next control character (QLDBNextRecCtrlChar)                      |
//|    IF current record pointer != END_OF_REC                                 |
//|      Set returned level number to current level number                     |
//|      Set level record pointer of level number to current record            |
//|        pointer                                                             |
//|      Set temp pointer to current record pointer                            |
//|      FOR level number + 1 TO MAX_LEVELS DO                                 |
//|        Find next control char (QLDBNextRecCtrlChar)                        |
//|        Set level record pointer of level number to temp pointer            |
//|      Set pointer array to data fields (watch the ESC_CHAR)                 |
//|    ELSE                                                                    |
//|      Set returned level number to END_OF_TREE                              |
//|      Set pointer array to NULL pointers                                    |
//|  ELSE                                                                      |
//|    IF pChild of current node != NULL pointer                               |
//|      Set current node pointer to child node                                |
//|    ELSE                                                                    |
//|      IF pRight of current node != NULL pointer                             |
//|        Set current node pointer to right node                              |
//|      ELSE                                                                  |
//|        Set loop-end-variable to FALSE                                      |
//|        WHILE loop-end-variable == FALSE                                    |
//|          IF pParent of current node != NULL pointer                        |
//|            Set current node pointer to parent node                         |
//|          ELSE                                                              |
//|            Set loop-end-variable to TRUE (meaning no more nodes            |
//|              were available and current pointer has reached the            |
//|              first level node)                                             |
//|          IF loop-end-variable == FALSE                                     |
//|            IF pRight of current node != NULL pointer                       |
//|              Set current node pointer to right node                        |
//|              Set loop-end-variable to TRUE                                 |
//|    IF current node pointer == first level pointer                          |
//|      Set returned level number to END_OF_TREE                              |
//|      Restore current node and level pointers                               |
//|      Set pointer in the pointer array to NULL pointers                     |
//|    ELSE                                                                    |
//|      Set returned level number to level number in current node             |
//|      Set level node pointer of level number to current node                |
//|      Set pointer array to data fields                                      |
//|      FOR level number + 1 TO MAX_LEVELS DO                                 |
//|        Set the level node pointer to pChild of node on level - 1           |
//+----------------------------------------------------------------------------+

USHORT QLDBNextNode
(
  PVOID        pvTree,                           // handle to tree
  PSZ_W        *ppszData,                        // pointer to an
                                                 // array of pointers
                                                 // pointing to the data
                                                 // that is read from the
                                                 // next node
  USHORT       *pusLevel                         // level of node
                                                 // can be:
                                                 // 2 to QLDB_MAX_LEVELS
                                                 // QLDB_END_OF_TREE
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       usTotalFields = 0;                // number of fields in
                                                 // node
  USHORT       i;                                // an index
  USHORT       usBufCount;                       // counter for the
                                                 // esc field buffer in
                                                 // the tree handle
  PQLDB_NODE   pTempCurNode;                     // former current node
                                                 // pointer
  PQLDB_NODE   apTempNodes[QLDB_MAX_LEVELS];     // temp array of pointers
                                                 // to allow restoring
                                                 // the old current node
                                                 // pointers in case of
                                                 // problems
  PSZ_W        pszTempCurRec;                    // former current record
                                                 // pointer
  PSZ_W        apszTempRec[QLDB_MAX_LEVELS];     // temp array of pointers
                                                 // to allow restoring the
                                                 // old current record
                                                 // pointers in case of
                                                 // problems
  PSZ_W        pszTempRec;                       // temp pointer to find
                                                 // the next ctrl
                                                 // character in the record
  PSZ_W        pszDataInRec;                     // pointer to the
                                                 // current field in the
                                                 // record
  BOOL         fStopLoop;                        // loop control variable



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pusLevel = QLDB_END_OF_TREE;

  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /* as the loop already walks along the pointers to the level nodes  */
  /*     save them in an array to easily restore them in case of      */
  /*                          later problems                          */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( ( phTree->apCurLevelNode[i] == NULL ) &&
         ( phTree->apszCurLevelRec[i] == NULL ) )
    {
      usRC = QLDB_NO_TREEHANDLE;
    }
    else
    {
      if ( phTree->fWorkWithRecord == TRUE )
      {
        apszTempRec[i] = phTree->apszCurLevelRec[i];
      }
      else
      {
        apTempNodes[i] = phTree->apCurLevelNode[i];
      } /* endif */
    } /* endif */
  } /* endfor */

  /********************************************************************/
  /*        compute the maximum number of fields in the nodes         */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    QLDB_MAX_NODE_FIELDS( phTree, &usTotalFields );
  } /* endif */

  /********************************************************************/
  /*      check if the pointer to the pointer array is not NULL       */
  /********************************************************************/
  if ( ( usTotalFields != 0 ) && ( ppszData == NULL ) &&
       ( usRC == QLDB_NO_ERROR ) )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          check if the tree handle references a record          */
    /******************************************************************/
    if ( phTree->fWorkWithRecord == TRUE )
    {
      /****************************************************************/
      /*               save the current record pointer                */
      /****************************************************************/
      pszTempCurRec = phTree->pszCurRec;

      /****************************************************************/
      /*               get the next node in the record                */
      /****************************************************************/
      usRC = QLDBNextRecCtrlChar( &( phTree->pszCurRec ) );

      if ( usRC == QLDB_NO_ERROR )
      {
        /**************************************************************/
        /*       check where the current record pointer now is        */
        /*   if an QLDB_END_OF_REC is reached set the output level    */
        /*  to QLDB_END_OF_TREE and the current record pointer back   */
        /*                   to its original value                    */
        /*            otherwise set also the new template             */
        /**************************************************************/
        if ( *( phTree->pszCurRec ) != (CHAR)( QLDB_END_OF_REC ) )
        {
          /************************************************************/
          /*                   get the level number                   */
          /************************************************************/
          *pusLevel = QLDB_LEVEL( phTree->pszCurRec );
          if ( *pusLevel > QLDB_MAX_LEVELS )
          {
            usRC = QLDB_NO_VALID_DATA;
          }
          else
          {
            /************************************************************/
            /*     set the level record pointer of the level to the     */
            /*                  current record pointer                  */
            /************************************************************/
            phTree->apszCurLevelRec[(*pusLevel)-1] = phTree->pszCurRec;

            /************************************************************/
            /*    now build up the whole new template so that other     */
            /*           "template" calls get the right stuff           */
            /************************************************************/
            pszTempRec = phTree->pszCurRec;
            for ( i = *pusLevel; ( i < QLDB_MAX_LEVELS ) &&
                                ( usRC == QLDB_NO_ERROR ); i++ )
            {
              usRC = QLDBNextRecCtrlChar( &pszTempRec );
              if ( usRC == QLDB_NO_ERROR )
              {
                phTree->apszCurLevelRec[i] = pszTempRec;
              } /* endif */
            } /* endfor */
          } /* endif */

          /************************************************************/
          /*     now we set the data pointer array to the current     */
          /*               record pointer's data fields               */
          /************************************************************/
          if ( usRC == QLDB_NO_ERROR )
          {
            if ( phTree->ausNoOfFields[(*pusLevel)-1] != 0 )
            {
              usBufCount = 0;

              pszDataInRec = phTree->pszCurRec + 1;

              for ( i = 0; ( i < phTree->ausNoOfFields[(*pusLevel)-1] ) &&
                           ( usRC == QLDB_NO_ERROR ); i++ )
              {
                /******************************************************/
                /*    set the pointers in the pointer array to the    */
                /*                    data fields                     */
                /*    but check whether they contain QLDB_ESC_CHAR    */
                /******************************************************/
                if ( phTree->usEscFields != 0 )
                {
                  usRC = QLDBFilterEscChar( phTree, &usBufCount,
                                            &pszDataInRec,
                                            &( ppszData[i] ) );
                }
                else
                {
                  ppszData[i] = pszDataInRec;
                  pszDataInRec += UTF16strlenCHAR( pszDataInRec ) + 1;
                } /* endif */
              } /* endfor */
            } /* endif */
          } /* endif */
        }
        else
        {
          *pusLevel = QLDB_END_OF_TREE;
          phTree->pszCurRec = pszTempCurRec;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /*                  check if an error occurred                  */
      /*     if so restore the old current level record pointers      */
      /****************************************************************/
      if ( usRC != QLDB_NO_ERROR )
      {
        for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
        {
          phTree->apszCurLevelRec[i] = apszTempRec[i];
        } /* endfor */
        phTree->pszCurRec = pszTempCurRec;
        *pusLevel = QLDB_END_OF_TREE;
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* as the old current nodes were alread saved before just save  */
      /* the actual current node pointer to have it for restoring in  */
      /*                    case of later problems                    */
      /****************************************************************/
      pTempCurNode = phTree->pCurNode;

      /****************************************************************/
      /*                get the next node in the tree                 */
      /****************************************************************/
      if ( phTree->pCurNode->pChild != NULL )
      {
        /**************************************************************/
        /* set the current node to the child node of the current node */
        /**************************************************************/
        phTree->pCurNode = phTree->pCurNode->pChild;
      }
      else
      {
        if ( phTree->pCurNode->pRight != NULL )
        {
          /************************************************************/
          /*set the current node to the right node of the current node*/
          /************************************************************/
          phTree->pCurNode = phTree->pCurNode->pRight;
        }
        else
        {
          /************************************************************/
          /*  now there is more to do, as the next node is somewhere  */
          /*                 else in another subtree                  */
          /************************************************************/
          fStopLoop = FALSE;
          while ( fStopLoop == FALSE )
          {
            /**********************************************************/
            /*      if a parent node exists set the current node      */
            /*                   to its parent node                   */
            /*  if it doesn't exist...too bad...the first level node  */
            /*  is reached and therefore no more nodes are available  */
            /**********************************************************/
            if ( phTree->pCurNode->pParent != NULL )
            {
              phTree->pCurNode = phTree->pCurNode->pParent;
            }
            else
            {
              fStopLoop = TRUE;
            } /* endif */

            /**********************************************************/
            /*   now we have to check if a node to the right exists   */
            /*               then this is the next node               */
            /*   if it doesn't exist...well then let's make another   */
            /*   turn in the while loop and move on a higher level    */
            /**********************************************************/
            if ( fStopLoop == FALSE )
            {
              if ( phTree->pCurNode->pRight != NULL )
              {
                phTree->pCurNode = phTree->pCurNode->pRight;
                fStopLoop = TRUE;
              } /* endif */
            } /* endif */
          } /* endwhile */
        } /* endif */
      } /* endif */

      /****************************************************************/
      /*               where is the current node now ?                */
      /*   if it is the first level node then restore the old node    */
      /*    pointers and the old current node pointer and set the     */
      /*      pointers in the pointer array to all NULL pointers      */
      /****************************************************************/
      if ( phTree->pCurNode->usLevel == 1 )
      {
        *pusLevel = QLDB_END_OF_TREE;
        phTree->pCurNode = pTempCurNode;

        for ( i = 1; i < QLDB_MAX_LEVELS; i++ )
        {
          phTree->apCurLevelNode[i] = apTempNodes[i];
        } /* endfor */
      }
      else
      {
        /**************************************************************/
        /*    now we have a valid next node and the returned level    */
        /*           number is the level number of the node           */
        /*set the pointers in the array to the data fields in the node*/
        /*           then set the pointers to the templates           */
        /**************************************************************/
        *pusLevel = phTree->pCurNode->usLevel;

        if ( phTree->ausNoOfFields[(*pusLevel)-1] != 0 )
        {
          /************************************************************/
          /*  ok, then set the pointers in the pointer array to the   */
          /*                 data fields in the node                  */
          /************************************************************/
          for ( i = 0; i < phTree->ausNoOfFields[(*pusLevel)-1];
                i++ )
          {
            ppszData[i] = phTree->pCurNode->aFields[i].pszData;
          } /* endfor */
        } /* endif */

        /************************************************************/
        /*     set the template to the subtree most left of the     */
        /*                     new current node                     */
        /************************************************************/
        if ( usRC == QLDB_NO_ERROR )
        {
          phTree->apCurLevelNode[(*pusLevel)-1] = phTree->pCurNode;

          /************************************************************/
          /*             build the right subtree template             */
          /************************************************************/
          for ( i = (*pusLevel); i < QLDB_MAX_LEVELS; i++ )
          {
            phTree->apCurLevelNode[i] =
              phTree->apCurLevelNode[i-1]->pChild;
          } /* endfor */

          /************************************************************/
          /*             build the right parent template              */
          /************************************************************/
          for ( i = (*pusLevel) - 1; i >= 1 ; i-- )
          {
            phTree->apCurLevelNode[i-1] =
              phTree->apCurLevelNode[i]->pParent;
          } /* endfor */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*         if the output level is QLDB_END_OF_TREE set the          */
  /*               data pointer array to NULL pointers                */
  /********************************************************************/
  if ( ( *pusLevel == QLDB_END_OF_TREE ) && ( ppszData != NULL ) )
  {
    if ( usTotalFields != 0 )
    {
      for ( i = 0; i < usTotalFields; i++ )
      {
        ppszData[i] = NULL;
      } /* endfor */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBNextNode */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBNextTemplate gets data from next template                |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBNextTemplate( phTree, ppszData, pusLevel )                     |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  Returns the next template of the node tree or end-of-tree if no more      |
//|  templates are available. A template is a target level node plus all       |
//|  parent nodes.                                                             |
//|                                                                            |
//|  The function returns an array of pointers to the data fields.             |
//|  It expects a pointer to an array of pointers as input. This array of      |
//|  pointers will then be filled with the pointers to the data fields of      |
//|  each node on each level. The calling application therefore                |
//|  has to keep the information about how many fields are on each level.      |
//|  Nodes containing no fields will not be referenced in the pointer          |
//|  array thus the pointer array contains only pointers to fields             |
//|  that may contain data.                                                    |
//|                                                                            |
//|  With the function the pointers of the current level nodes are updated.    |
//|                                                                            |
//|  If each node in each level contains no fields the pointer to the pointer  |
//|  array is expected to be a NULL pointer and the level returned is either   |
//|  QLDB_END_OF_TREE or QLDB_MAX_LEVELS.                                      |
//|                                                                            |
//|  If the level returned is QLDB_MAX_LEVELS the function has found a next    |
//|  template and the data referenced by the pointer array is valid data.      |
//|  A level returned of QLDB_END_OF_TREE indicates that no more templates     |
//|  are available. The function leaves the current node pointers unchanged    |
//|  so that a next call to QLDBNextTemplate also returns QLDB_END_OF_TREE.    |
//|  If the level returned is QLDB_END_OF_TREE the pointers in the pointer     |
//|  array are set to NULL.                                                    |
//|                                                                            |
//|  This function is able to work with records. The calling application       |
//|  will get the same results as described above.                             |
//|                                                                            |
//|  In case of the returncode not being QLDB_NO_ERROR the current             |
//|  template will not change.                                                 |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//|  PSZ          *ppszData;                        // pointer to an           |
//|                                                 // array of pointers       |
//|                                                 // pointing to the data    |
//|                                                 // that is read from the   |
//|                                                 // nodes in the next       |
//|                                                 // template                |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  USHORT       *pusLevel;                        // level of template       |
//|                                                 // can be:                 |
//|                                                 // QLDB_MAX_LEVELS         |
//|                                                 // QLDB_END_OF_TREE        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_NO_VALID_DATA          - the pointer to data fields did not exist    |
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
//|  usRetCode = QLDBNextTemplate( phTree, ppszData, &usLevel );               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|    Set temp pointer to level record pointer on level QLDB_MAX_LEVELS       |
//|    Find next control character (QLDBNextRecCtrlChar)                       |
//|    IF temp pointer != QLDB_END_OF_REC                                      |
//|      Set level record pointer of level that temp pointer represents        |
//|        to temp pointer                                                     |
//|      FOR level of returned control character + 1 TO QLDB_MAX_LEVEL         |
//|        Find next control character (QLDBNextRecCtrlChar)                   |
//|        Set level record pointer of level that temp pointer represents      |
//|          to temp pointer                                                   |
//|      Set pointer array to data fields (watch the ESC_CHAR)                 |
//|      RETURN QLDB_MAX_LEVELS                                                |
//|    ELSE                                                                    |
//|      Set pointer array to NULL pointers                                    |
//|      RETURN QLDB_END_OF_TREE                                               |
//|  ELSE                                                                      |
//|    CALL QLDBNextTreeTemplate with QLDB_MAX_LEVELS                          |
//|    IF returned level number != QLDB_END_OF_TREE                            |
//|      Set pointer array to data fields                                      |
//|      RETURN QLDB_MAX_LEVELS                                                |
//|    ELSE                                                                    |
//|      Set pointer array to NULL pointers                                    |
//|      RETURN QLDB_END_OF_TREE                                               |
//+----------------------------------------------------------------------------+

USHORT QLDBNextTemplate
(
  PVOID        pvTree,                           // handle to tree
  PSZ_W        *ppszData,                        // pointer to an
                                                 // array of pointers
                                                 // pointing to the data
                                                 // that is read from the
                                                 // nodes in the next
                                                 // template
  USHORT       *pusLevel                         // level of template
                                                 // can be:
                                                 // QLDB_MAX_LEVELS
                                                 // QLDB_END_OF_TREE
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;    // function return code
  USHORT       usTotalFields = 0;                // number of fields in
                                                 // template
  USHORT       i;                                // an index
  USHORT       j;                                // another index
  USHORT       usPrevFields = 0;                 // no. of fields on all
                                                 // former levels
  USHORT       usLevel;                          // level number from
                                                 // record ctrl char
  USHORT       usBufCount;                       // counter for the
                                                 // esc field buffer in
                                                 // the tree handle
  PQLDB_NODE   apTempNodes[QLDB_MAX_LEVELS];     // temp array of pointers
                                                 // to allow restoring
                                                 // the old current node
                                                 // pointers in case of
                                                 // problems
  PSZ_W        apszTempRec[QLDB_MAX_LEVELS];     // temp array of pointers
                                                 // to allow restoring the
                                                 // old current record
                                                 // pointers in case of
                                                 // problems
  PSZ_W        pszTempRec;                       // temp pointer to find
                                                 // the next ctrl
                                                 // character in the record
  PSZ_W        pszDataInRec;                     // pointer to the
                                                 // current field in the
                                                 // record



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pusLevel = QLDB_END_OF_TREE;

  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /* as the loop already walks along the pointers to the level nodes  */
  /*     save them in an array to easily restore them in case of      */
  /*                          later problems                          */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( ( phTree->apCurLevelNode[i] == NULL ) &&
         ( phTree->apszCurLevelRec[i] == NULL ) )
    {
      usRC = QLDB_NO_TREEHANDLE;
    }
    else
    {
      if ( phTree->fWorkWithRecord == TRUE )
      {
        apszTempRec[i] = phTree->apszCurLevelRec[i];
      }
      else
      {
        apTempNodes[i] = phTree->apCurLevelNode[i];
      } /* endif */
    } /* endif */
  } /* endfor */

  /********************************************************************/
  /*        compute the number of total fields in the template        */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    QLDB_TOTAL_FIELDS( phTree, &usTotalFields );
  } /* endif */

  /********************************************************************/
  /*   check if the pointer to the pointer array is a valid pointer   */
  /********************************************************************/
  if ( ( usRC == QLDB_NO_ERROR ) && ( ppszData == NULL ) &&
       ( usTotalFields != 0 ) )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */


  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          check if the tree handle references a record          */
    /******************************************************************/
    if ( phTree->fWorkWithRecord == TRUE )
    {
      /****************************************************************/
      /*             get the next template in the record              */
      /*   but first set a temp pointer to the level record pointer   */
      /* of level QLBD_MAX_LEVELS, so that the next control character */
      /* that can be found is either the starting of the new template */
      /*                      or QLDB_END_OF_REC                      */
      /****************************************************************/
      pszTempRec = phTree->apszCurLevelRec[QLDB_MAX_LEVELS-1];
      usRC = QLDBNextRecCtrlChar( &pszTempRec );

      /****************************************************************/
      /*             check if an QLDB_END_OF_REC is found             */
      /****************************************************************/
      if ( usRC == QLDB_NO_ERROR )
      {
        if ( *pszTempRec != QLDB_END_OF_REC )
        {
          /************************************************************/
          /*          set the current level record pointers           */
          /************************************************************/
          usLevel = QLDB_LEVEL( pszTempRec );

          /************************************************************/
          /*            check if the level info is correct            */
          /************************************************************/
          if ( ( usLevel < 1 ) || ( usLevel > QLDB_MAX_LEVELS ) )
          {
            usRC = QLDB_ERROR_IN_TREE;
          } /* endif */

          /************************************************************/
          /*  set the level record pointer to the temp data pointer   */
          /************************************************************/
          if ( usRC == QLDB_NO_ERROR )
          {
            phTree->apszCurLevelRec[usLevel-1] = pszTempRec;
          } /* endif */

          /************************************************************/
          /*     now set the level pointers of all lower levels       */
          /************************************************************/
          if ( usRC == QLDB_NO_ERROR )
          {
            for ( i = usLevel + 1; ( i <= QLDB_MAX_LEVELS ) &&
                                   ( usRC == QLDB_NO_ERROR );
                  i++ )
            {
              usRC = QLDBNextRecCtrlChar( &pszTempRec );

              if ( usRC == QLDB_NO_ERROR )
              {
                usLevel = QLDB_LEVEL( pszTempRec );
                if ( ( usLevel < 1 ) || ( usLevel > QLDB_MAX_LEVELS ) )
                {
                  usRC = QLDB_ERROR_IN_TREE;
                } /* endif */
              } /* endif */

              /********************************************************/
              /*set the level record pointer to the temp data pointer */
              /********************************************************/
              if ( usRC == QLDB_NO_ERROR )
              {
                phTree->apszCurLevelRec[usLevel-1] = pszTempRec;
              } /* endif */
            } /* endfor */
          } /* endif */

          /************************************************************/
          /*       now set the pointer array to the data fields       */
          /************************************************************/
          if ( usRC == QLDB_NO_ERROR )
          {
            if ( usTotalFields != 0 )
            {
              usBufCount = 0;

              for ( i = 0; ( i < QLDB_MAX_LEVELS ) &&
                           ( usRC == QLDB_NO_ERROR ); i++ )
              {
                pszDataInRec = phTree->apszCurLevelRec[i] + 1;

                /******************************************************/
                /*    get the number of fields of the former levels   */
                /******************************************************/
                if ( i == 0 )
                {
                  usPrevFields = 0;
                }
                else
                {
                  usPrevFields = usPrevFields + phTree->ausNoOfFields[i-1];
                } /* endif */

                /******************************************************/
                /*         loop over the fields in the nodes          */
                /******************************************************/
                for ( j = 0; ( j < phTree->ausNoOfFields[i]) &&
                             ( usRC == QLDB_NO_ERROR );
                      j++ )
                {
                  /****************************************************/
                  /*   set the pointers in the pointer array to the   */
                  /*                   data fields                    */
                  /*   but check whether they contain QLDB_ESC_CHAR   */
                  /****************************************************/
                  if ( phTree->usEscFields != 0 )
                  {
                    usRC = QLDBFilterEscChar( phTree, &usBufCount,
                                          &pszDataInRec,
                                          &( ppszData[usPrevFields+j] ) );
                  }
                  else
                  {
                    ppszData[usPrevFields+j] = pszDataInRec;
                    pszDataInRec += UTF16strlenCHAR( pszDataInRec ) + 1;
                  } /* endif */
                } /* endfor */
              } /* endfor */

              if ( usRC == QLDB_NO_ERROR )
              {
                *pusLevel = QLDB_MAX_LEVELS;
              } /* endif */
            } /* endif */
          } /* endif */
        }
        else
        {
          *pusLevel = QLDB_END_OF_TREE;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /*                  check if an error occurred                  */
      /*     if so restore the old current level record pointers      */
      /****************************************************************/
      if ( usRC != QLDB_NO_ERROR )
      {
        for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
        {
          phTree->apszCurLevelRec[i] = apszTempRec[i];
        } /* endfor */
        *pusLevel = QLDB_END_OF_TREE;
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /*              get the next template in the tree               */
      /****************************************************************/
      usRC = QLDBNextTreeTemplate( phTree, QLDB_MAX_LEVELS, pusLevel );
      if ( usRC != QLDB_NO_ERROR )
      {
        /**************************************************************/
        /*       restore the saved current level node pointers        */
        /**************************************************************/
        for ( i = 0 ; i < QLDB_MAX_LEVELS; i++ )
        {
          phTree->apCurLevelNode[i] = apTempNodes[i];
        } /* endfor */
        *pusLevel = QLDB_END_OF_TREE;
      }
      else
      {
        /**************************************************************/
        /*  set the pointers in the pointer array to the data fields  */
        /*     if the level returned from QLDBNextTreeTemplate is     */
        /*   QLDB_END_OF_TREE set all pointers in the pointer array   */
        /*                      to NULL pointers                      */
        /**************************************************************/
        if ( usTotalFields != 0 )
        {
          if ( *pusLevel != QLDB_END_OF_TREE )
          {
            /**********************************************************/
            /*                  loop over the levels                  */
            /**********************************************************/
            for ( i = 0; ( i < QLDB_MAX_LEVELS ) &&
                         ( usRC == QLDB_NO_ERROR ); i++ )
            {
              /********************************************************/
              /*     get the number of fields of the former levels    */
              /********************************************************/
              if ( i == 0 )
              {
                usPrevFields = 0;
              }
              else
              {
                usPrevFields = usPrevFields + phTree->ausNoOfFields[i-1];
              } /* endif */

              /********************************************************/
              /*          loop over the fields in the nodes           */
              /********************************************************/
              for ( j = 0; ( j < phTree->ausNoOfFields[i]) &&
                           ( usRC == QLDB_NO_ERROR ); j++ )
              {
                /******************************************************/
                /*    set the pointers in the pointer array to the    */
                /*                    data fields                     */
                /******************************************************/
                ppszData[usPrevFields+j] =
                          phTree->apCurLevelNode[i]->aFields[j].pszData;
              } /* endfor */
            } /* endfor */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*         if the output level is QLDB_END_OF_TREE set the          */
  /*               data pointer array to NULL pointers                */
  /********************************************************************/
  if ( ( *pusLevel == QLDB_END_OF_TREE ) && ( ppszData != NULL ) )
  {
    if ( usTotalFields != 0 )
    {
      for ( i = 0; i < usTotalFields; i++ )
      {
        ppszData[i] = NULL;
      } /* endfor */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBNextTemplate */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBPrevNode gets data from previous node                    |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBPrevNode( phTree, ppszData, pusLevel )                         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function returns the previous node of the current node (precisely it |
//|  returns an array of pointers to the data fields in the previous node      |
//|  plus the level number of the previous node).                              |
//|                                                                            |
//|  The sequence of node processing is top-to-bottom and left-to-right.       |
//|                                                                            |
//|  The function changes the current node and the current template so that    |
//|  always a valid template is formed depending on the new current node.      |
//|                                                                            |
//|  If no more nodes are available the function returns QLDB_END_OF_TREE      |
//|  as level number and the pointers in the array are set to NULL.            |
//|  If the function returns QLDB_END_OF_TREE the current node and template    |
//|  pointers will not change.                                                 |
//|                                                                            |
//|  If the node contains no fields at all the pointers in the pointer array   |
//|  are left unchanged.                                                       |
//|                                                                            |
//|  The function expects an array of pointers to zero-terminated strings.     |
//|  The number of pointers should be the same as the number of fields in      |
//|  the template, because the calling application doesn't know which          |
//|  level (and therefore how many fields) will be returned.                   |
//|  This processing also allows multiple calls to QLDBPrevNode with the       |
//|  same storage allocated for the array of pointers and therefore a better   |
//|  performance than an allocation every time when QLDBPrevNode is called.    |
//|                                                                            |
//|  The function is able to work with records. The results will be the same   |
//|  as described above.                                                       |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PSZ          *ppszData;                        // pointer to an           |
//|                                                 // array of pointers       |
//|                                                 // pointing to the data    |
//|                                                 // that is read from the   |
//|                                                 // next node               |
//|  USHORT       *pusLevel;                        // level of node           |
//|                                                 // can be:                 |
//|                                                 // 2 to QLDB_MAX_LEVELS    |
//|                                                 // QLDB_END_OF_TREE        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_NO_VALID_DATA          - the pointer to data fields did not exist    |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle                                                     |
//|  - a pointer array that should contain at least the same number of         |
//|    pointers as the greatest amount of fields in a node                     |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the current node pointer is changed                                     |
//|  - the current level pointers are changed so that they form a new          |
//|    valid template                                                          |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBPrevNode( phTree, ppszData, &usLevel );                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|    IF current record pointer != FIRST_LEVEL                                |
//|      Find previous control character (QLDBPrevRecCtrlChar)                 |
//|      Set returned level number to current level number                     |
//|      Set level record pointer of level number to current record            |
//|        pointer                                                             |
//|      Set temp pointer to current record pointer                            |
//|      FOR level number TO 2 STEP -1 DO                                      |
//|        WHILE temp pointer >= level number                                  |
//|          Find previous control character (QLDBPrevRecCtrlChar)             |
//|        Set level record pointer of level number - 1 to temp pointer        |
//|      Set pointer array to data fields (watch the ESC_CHAR)                 |
//|    ELSE                                                                    |
//|      Set returned level number to END_OF_TREE                              |
//|      Set pointer array to NULL pointers                                    |
//|  ELSE                                                                      |
//|    IF pLeft of current node != NULL pointer                                |
//|      Set current node pointer to left node                                 |
//|      Set level pointer of level to current node                            |
//|      REPEAT                                                                |
//|        IF pChild of current node != NULL pointer                           |
//|          Set current node pointer to child node                            |
//|          Set level pointer of level to current node                        |
//|          WHILE pRight of current node != NULL pointer                      |
//|            Set current node pointer to right node                          |
//|            Set level pointer of level to current node                      |
//|      UNTIL pChild of current node == NULL                                  |
//|    ELSE                                                                    |
//|      IF pParent of current node != NULL pointer                            |
//|        Set current node pointer to parent node                             |
//|        Set level pointer of level to current node                          |
//|      ELSE                                                                  |
//|        Set the returned level number to END_OF_TREE                        |
//|        Set pointer array to NULL pointers                                  |
//|    IF returned level number != END_OF_TREE                                 |
//|      Set returned level number to level number in tree handle              |
//|      Set pointer array to data fields                                      |
//+----------------------------------------------------------------------------+

USHORT QLDBPrevNode
(
  PVOID        pvTree,                           // handle to tree
  PSZ_W        *ppszData,                        // pointer to an
                                                 // array of pointers
                                                 // pointing to the data
                                                 // that is read from the
                                                 // next node
  USHORT       *pusLevel                         // level of node
                                                 // can be:
                                                 // 2 to QLDB_MAX_LEVELS
                                                 // QLDB_END_OF_TREE
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       usTotalFields = 0;                // number of fields in
                                                 // node
  USHORT       i;                                // an index
  USHORT       usBufCount;                       // counter for the
                                                 // esc field buffer in
                                                 // the tree handle
  PQLDB_NODE   pTempCurNode;                     // former current node
                                                 // pointer
  PQLDB_NODE   apTempNodes[QLDB_MAX_LEVELS];     // temp array of pointers
                                                 // to allow restoring
                                                 // the old current node
                                                 // pointers in case of
                                                 // problems
  PSZ_W        pszTempCurRec;                    // former current record
                                                 // pointer
  PSZ_W        apszTempRec[QLDB_MAX_LEVELS];     // temp array of pointers
                                                 // to allow restoring the
                                                 // old current record
                                                 // pointers in case of
                                                 // problems
  PSZ_W        pszTempRec;                       // temp pointer to find
                                                 // the next ctrl
                                                 // character in the record
  PSZ_W        pszDataInRec;                     // pointer to the
                                                 // current field in the
                                                 // record



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pusLevel = QLDB_END_OF_TREE;

  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /* as the loop already walks along the pointers to the level nodes  */
  /*     save them in an array to easily restore them in case of      */
  /*                          later problems                          */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( ( phTree->apCurLevelNode[i] == NULL ) &&
         ( phTree->apszCurLevelRec[i] == NULL ) )
    {
      usRC = QLDB_NO_TREEHANDLE;
    }
    else
    {
      if ( phTree->fWorkWithRecord == TRUE )
      {
        apszTempRec[i] = phTree->apszCurLevelRec[i];
      }
      else
      {
        apTempNodes[i] = phTree->apCurLevelNode[i];
      } /* endif */
    } /* endif */
  } /* endfor */

  /********************************************************************/
  /*        compute the maximum number of fields in the nodes         */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    QLDB_MAX_NODE_FIELDS( phTree, &usTotalFields );
  } /* endif */

  /********************************************************************/
  /*      check if the pointer to the pointer array is not NULL       */
  /********************************************************************/
  if ( ( usTotalFields != 0 ) && ( ppszData == NULL ) &&
       ( usRC == QLDB_NO_ERROR ) )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          check if the tree handle references a record          */
    /******************************************************************/
    if ( phTree->fWorkWithRecord == TRUE )
    {
      /****************************************************************/
      /*               save the current record pointer                */
      /****************************************************************/
      pszTempCurRec = phTree->pszCurRec;
      /****************************************************************/
      /*             get the previous node in the record              */
      /****************************************************************/
      if ( *( phTree->pszCurRec ) != (CHAR)( QLDB_FIRST_LEVEL ) )
      {
        /**************************************************************/
        /*         get the previous record control character          */
        /**************************************************************/
        usRC = QLDBPrevRecCtrlChar( &( phTree->pszCurRec ) );

        /**************************************************************/
        /*       check where the current record pointer now is        */
        /*   if an QLDB_FIRST_LEVEL is reached set the output level   */
        /*  to QLDB_END_OF_TREE and the current record pointer back   */
        /*                   to its original value                    */
        /*            otherwise set also the new template             */
        /**************************************************************/
        if ( usRC == QLDB_NO_ERROR )
        {
          /************************************************************/
          /*                   get the level number                   */
          /************************************************************/
          *pusLevel = QLDB_LEVEL( phTree->pszCurRec );

          /************************************************************/
          /*     set the level record pointer of the level to the     */
          /*                  current record pointer                  */
          /************************************************************/
          phTree->apszCurLevelRec[(*pusLevel)-1] = phTree->pszCurRec;

          /************************************************************/
          /*    now build up the whole new template so that other     */
          /*           "template" calls get the right stuff           */
          /************************************************************/
          pszTempRec = phTree->pszCurRec;
          for ( i = *pusLevel; ( i >= 2 ) && ( usRC == QLDB_NO_ERROR );
                i-- )
          {
            while ( ( QLDB_LEVEL( pszTempRec ) >= i ) &&
                    ( usRC == QLDB_NO_ERROR ) )
            {
               usRC = QLDBPrevRecCtrlChar( &pszTempRec );
            } /* endwhile */

            if ( usRC == QLDB_NO_ERROR )
            {
              phTree->apszCurLevelRec[i-2] = pszTempRec;
            } /* endif */
          } /* endfor */

          /************************************************************/
          /*     now we set the data pointer array to the current     */
          /*               record pointer's data fields               */
          /************************************************************/
          if ( usRC == QLDB_NO_ERROR )
          {
            if ( phTree->ausNoOfFields[(*pusLevel)-1] != 0 )
            {
              usBufCount = 0;

              pszDataInRec = phTree->pszCurRec + 1;

              for ( i = 0; ( i < phTree->ausNoOfFields[(*pusLevel)-1] ) &&
                           ( usRC == QLDB_NO_ERROR ); i++ )
              {
                /******************************************************/
                /*    set the pointers in the pointer array to the    */
                /*                    data fields                     */
                /*    but check whether they contain QLDB_ESC_CHAR    */
                /******************************************************/
                if ( phTree->usEscFields != 0 )
                {
                  usRC = QLDBFilterEscChar( phTree, &usBufCount,
                                            &pszDataInRec,
                                            &( ppszData[i] ) );
                }
                else
                {
                  ppszData[i] = pszDataInRec;
                  pszDataInRec += UTF16strlenCHAR( pszDataInRec ) + 1;
                } /* endif */
              } /* endfor */
            } /* endif */
          } /* endif */
        } /* endif */
      }
      else
      {
        *pusLevel = QLDB_END_OF_TREE;
        phTree->pszCurRec = pszTempCurRec;
      } /* endif */

      /****************************************************************/
      /*                  check if an error occurred                  */
      /*     if so restore the old current level record pointers      */
      /****************************************************************/
      if ( usRC != QLDB_NO_ERROR )
      {
        for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
        {
          phTree->apszCurLevelRec[i] = apszTempRec[i];
        } /* endfor */
        phTree->pszCurRec = pszTempCurRec;
        *pusLevel = QLDB_END_OF_TREE;
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /* as the old current nodes were alread saved before just save  */
      /* the actual current node pointer to have it for restoring in  */
      /*                    case of later problems                    */
      /****************************************************************/
      pTempCurNode = phTree->pCurNode;

      /****************************************************************/
      /*              get the previous node in the tree               */
      /****************************************************************/
      if ( phTree->pCurNode->pLeft != NULL )
      {
        /**************************************************************/
        /* set the current node to the left node of the current node  */
        /**************************************************************/
        phTree->pCurNode = phTree->pCurNode->pLeft;
        (*pusLevel) = phTree->pCurNode->usLevel;
        phTree->apCurLevelNode[(*pusLevel)-1] = phTree->pCurNode;

        /**************************************************************/
        /* now check if there is a child subtree of which the deepest */
        /*    node on the right would then be the new current node    */
        /**************************************************************/
        do
        {
          if ( phTree->pCurNode->pChild != NULL )
          {
            /**********************************************************/
            /*        set the new current node one level down         */
            /**********************************************************/
            phTree->pCurNode = phTree->pCurNode->pChild;
            (*pusLevel) = phTree->pCurNode->usLevel;
            phTree->apCurLevelNode[(*pusLevel)-1] = phTree->pCurNode;

            /**********************************************************/
            /*      now loop to the node most right on the level      */
            /**********************************************************/
            while ( phTree->pCurNode->pRight != NULL )
            {
              phTree->pCurNode = phTree->pCurNode->pRight;
              (*pusLevel) = phTree->pCurNode->usLevel;
              phTree->apCurLevelNode[(*pusLevel)-1] = phTree->pCurNode;
            } /* endwhile */
          } /* endif */
        } while ( phTree->pCurNode->pChild != NULL ); /* enddo */
      }
      else
      {
        if ( phTree->pCurNode->pParent != NULL )
        {
          /************************************************************/
          /*      set the current node to the parent node of the      */
          /*                       current node                       */
          /************************************************************/
          phTree->pCurNode = phTree->pCurNode->pParent;
          (*pusLevel) = phTree->pCurNode->usLevel;
          phTree->apCurLevelNode[(*pusLevel)-1] = phTree->pCurNode;
        }
        else
        {
          /************************************************************/
          /*    set the returned level number to QLDB_END_OF_TREE     */
          /*     an set the pointer to the pointer array to NULL      */
          /*                and restore the saved data                */
          /************************************************************/
          phTree->pCurNode = pTempCurNode;
          for ( i = 0; i < QLDB_MAX_LEVELS; i++)
          {
            phTree->apCurLevelNode[i] = apTempNodes[i];
          } /* endfor */

          *pusLevel = QLDB_END_OF_TREE;
        } /* endif */
      } /* endif */

      if ( *pusLevel != QLDB_END_OF_TREE )
      {
        /**************************************************************/
        /*    now we have a valid next node and the returned level    */
        /*           number is the level number of the node           */
        /*set the pointers in the array to the data fields in the node*/
        /**************************************************************/
        if ( phTree->ausNoOfFields[(*pusLevel)-1] != 0 )
        {
          /************************************************************/
          /*  ok, then set the pointers in the pointer array to the   */
          /*                 data fields in the node                  */
          /************************************************************/
          for ( i = 0; i < phTree->ausNoOfFields[(*pusLevel)-1]; i++ )
          {
            ppszData[i] = phTree->pCurNode->aFields[i].pszData;
          } /* endfor */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*         if the output level is QLDB_END_OF_TREE set the          */
  /*               data pointer array to NULL pointers                */
  /********************************************************************/
  if ( ( *pusLevel == QLDB_END_OF_TREE ) && ( ppszData != NULL ) )
  {
    if ( usTotalFields != 0 )
    {
      for ( i = 0; i < usTotalFields; i++ )
      {
        ppszData[i] = NULL;
      } /* endfor */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBPrevNode */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBPrevTemplate gets data from previous template            |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBPrevTemplate( phTree, ppszData, pusLevel )                     |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  Returns the previous template of the node tree or end-of-tree if no more  |
//|  templates are available. A template is a target level node plus all       |
//|  parent nodes.                                                             |
//|                                                                            |
//|  The function returns an array of pointers to the data fields.             |
//|  It expects a pointer to an array of pointers as input. This array of      |
//|  pointers will then be filled with the pointers to the data fields of      |
//|  each node on each level. The calling application therefore                |
//|  has to keep the information about how many fields are on each level.      |
//|  Nodes containing no fields will not be referenced in the pointer          |
//|  array thus the pointer array contains only pointers to fields             |
//|  that may contain data.                                                    |
//|                                                                            |
//|  With the function the pointers of the current level nodes are updated.    |
//|                                                                            |
//|  If each node in each level contains no fields the pointer to the pointer  |
//|  array is expected to be a NULL pointer and the level returned is either   |
//|  QLDB_END_OF_TREE or QLDB_MAX_LEVELS.                                      |
//|                                                                            |
//|  If the level returned is QLDB_MAX_LEVELS the function has found a previous|
//|  template and the data referenced by the pointer array is valid data.      |
//|  A level returned of QLDB_END_OF_TREE indicates that no more templates     |
//|  are available. The function leaves the current node pointers unchanged    |
//|  so that a next call to QLDBPrevTemplate also returns QLDB_END_OF_TREE.    |
//|  If the level returned is QLDB_END_OF_TREE the pointers in the pointer     |
//|  array are set to NULL.                                                    |
//|                                                                            |
//|  This function is able to work with records. The calling application       |
//|  will get the same results as described above.                             |
//|                                                                            |
//|  In case of the returncode not being QLDB_NO_ERROR the current             |
//|  template will not change.                                                 |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//|  PSZ          *ppszData;                        // pointer to an           |
//|                                                 // array of pointers       |
//|                                                 // pointing to the data    |
//|                                                 // that is read from the   |
//|                                                 // nodes in the previous   |
//|                                                 // template                |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  USHORT       *pusLevel;                        // level of template       |
//|                                                 // can be:                 |
//|                                                 // QLDB_MAX_LEVELS         |
//|                                                 // QLDB_END_OF_TREE        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_NO_VALID_DATA          - the pointer to data fields did not exist    |
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
//|  usRetCode = QLDBPrevTemplate( phTree, ppszData, &usLevel );               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|    Set temp pointer to level record pointer of level MAX_LEVELS            |
//|    Find previous control character (QLDBPrevRecCtrlChar)                   |
//|    IF temp pointer != MAX_LEVELS                                           |
//|      FOR MAX_LEVELS TO 2 STEP - 1 DO                                       |
//|        IF level number != MAX_LEVELS                                       |
//|          Set temp pointer to level record pointer of level number + 1      |
//|        ELSE                                                                |
//|          Set temp pointer to level record pointer of level MAX_LEVELS      |
//|        Find previous control character (QLDBPrevRecCtrlChar)               |
//|        WHILE (temp pointer != FIRST_LEVEL) and                             |
//|              (temp pointer != level number )                               |
//|          Find previous control character (QLDBPrevRecCtrlChar)             |
//|        IF temp pointer != FIRST_LEVEL                                      |
//|          Set level record pointer of level number to temp pointer          |
//|        ELSE                                                                |
//|          IF level number == MAX_LEVELS                                     |
//|            Set fEndOfTree to TRUE                                          |
//|    ELSE                                                                    |
//|      Set level record pointer of level MAX_LEVELS to temp pointer          |
//|    IF fEndOfTree == TRUE                                                   |
//|      Set pointer array to NULL pointers                                    |
//|      RETURN END_OF_TREE                                                    |
//|    ELSE                                                                    |
//|      Set pointer array to data fields                                      |
//|      RETURN MAX_LEVELS                                                     |
//|  ELSE                                                                      |
//|    CALL QLDBPrevTreeTemplate with MAX_LEVELS                               |
//|    IF returned level number != END_OF_TREE                                 |
//|      Set pointer array to data fields                                      |
//|      RETURN MAX_LEVELS                                                     |
//|    ELSE                                                                    |
//|      Set pointer array to NULL pointers                                    |
//|      RETURN END_OF_TREE                                                    |
//+----------------------------------------------------------------------------+

USHORT QLDBPrevTemplate
(
  PVOID        pvTree,                           // handle to tree
  PSZ_W        *ppszData,                        // pointer to an
                                                 // array of pointers
                                                 // pointing to the data
                                                 // that is read from the
                                                 // nodes in the previous
                                                 // template
  USHORT       *pusLevel                         // level of template
                                                 // can be:
                                                 // QLDB_MAX_LEVELS
                                                 // QLDB_END_OF_TREE
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       usTotalFields = 0;                // number of fields in
                                                 // template
  USHORT       i;                                // an index
  USHORT       j;                                // another index
  USHORT       usPrevFields = 0;                 // no. of fields on all
                                                 // former levels
  USHORT       usBufCount;                       // counter for the
                                                 // esc field buffer in
                                                 // the tree handle
  PQLDB_NODE   apTempNodes[QLDB_MAX_LEVELS];     // temp array of pointers
                                                 // to allow restoring
                                                 // the old current node
                                                 // pointers in case of
                                                 // problems
  PSZ_W        apszTempRec[QLDB_MAX_LEVELS];     // temp array of pointers
                                                 // to allow restoring the
                                                 // old current record
                                                 // pointers in case of
                                                 // problems
  PSZ_W        pszTempRec;                       // temp pointer to find
                                                 // the next ctrl
                                                 // character in the record
  PSZ_W        pszDataInRec;                     // pointer to the
                                                 // current field in the
                                                 // record
  BOOL         fEndOfTree = FALSE;               // flag to indicate that
                                                 // end of tree is reached



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pusLevel = QLDB_END_OF_TREE;

  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /* as the loop already walks along the pointers to the level nodes  */
  /*     save them in an array to easily restore them in case of      */
  /*                          later problems                          */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( ( phTree->apCurLevelNode[i] == NULL ) &&
         ( phTree->apszCurLevelRec[i] == NULL ) )
    {
      usRC = QLDB_NO_TREEHANDLE;
    }
    else
    {
      if ( phTree->fWorkWithRecord == TRUE )
      {
        apszTempRec[i] = phTree->apszCurLevelRec[i];
      }
      else
      {
        apTempNodes[i] = phTree->apCurLevelNode[i];
      } /* endif */
    } /* endif */
  } /* endfor */

  /********************************************************************/
  /*        compute the number of total fields in the template        */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    QLDB_TOTAL_FIELDS( phTree, &usTotalFields );
  } /* endif */

  /********************************************************************/
  /*   check if the pointer to the pointer array is a valid pointer   */
  /********************************************************************/
  if ( ( usRC == QLDB_NO_ERROR ) && ( ppszData == NULL ) &&
       ( usTotalFields != 0 ) )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */


  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          check if the tree handle references a record          */
    /******************************************************************/
    if ( phTree->fWorkWithRecord == TRUE )
    {
      /****************************************************************/
      /*           get the previous template in the record            */
      /****************************************************************/
      /****************************************************************/
      /*    set the temp pointer to the last level record pointer     */
      /****************************************************************/
      pszTempRec = phTree->apszCurLevelRec[QLDB_MAX_LEVELS-1];

      /****************************************************************/
      /*      get the previous control character from the record      */
      /****************************************************************/
      usRC = QLDBPrevRecCtrlChar( &pszTempRec );

      if ( usRC == QLDB_NO_ERROR )
      {
        /**************************************************************/
        /*     now check if the temp pointer points to a control      */
        /*     character indicating that level QLDB_MAX_LEVELS is     */
        /*                    the new level number                    */
        /*    if so just set the level record pointer to the found    */
        /*                        temp pointer                        */
        /*      otherwise we have to search for the new template      */
        /**************************************************************/
        if ( QLDB_LEVEL( pszTempRec ) != QLDB_MAX_LEVELS )
        {
          /************************************************************/
          /*   now we have to find the new template by looping over   */
          /*                        the levels                        */
          /************************************************************/
          for ( i = QLDB_MAX_LEVELS; ( i >= 2 ) && ( !fEndOfTree );
                i-- )
          {
            /**********************************************************/
            /*  if we process the level QLDB_MAX_LEVEL set the temp   */
            /*     pointer to the current level record pointer of     */
            /*    QLDB_MAX_LEVEL and start the search for the next    */
            /*  control character with QLDB_MAX_LEVEL from there on   */
            /*     for all other levels start the search from the     */
            /*  current control character of the level below (with a  */
            /*      higher level number) of the level processed       */
            /**********************************************************/
            if ( i != QLDB_MAX_LEVELS )
            {
              pszTempRec = phTree->apszCurLevelRec[i];
            }
            else
            {
              pszTempRec = phTree->apszCurLevelRec[QLDB_MAX_LEVELS-1];
            } /* endif */

            /**********************************************************/
            /*    now search for a control character with the same    */
            /*          level number as the level processed           */
            /**********************************************************/
            usRC = QLDBPrevRecCtrlChar( &pszTempRec );

            while ( ( *pszTempRec != (CHAR)(QLDB_FIRST_LEVEL) ) &&
                    ( QLDB_LEVEL( pszTempRec ) != i ) &&
                    ( usRC == QLDB_NO_ERROR ) )
            {
              usRC = QLDBPrevRecCtrlChar( &pszTempRec );
            } /* endwhile */

            /**********************************************************/
            /*     now let's see what we have found...it could be     */
            /*          that we are at the end of the record          */
            /**********************************************************/
            if ( usRC == QLDB_NO_ERROR )
            {
              if ( *pszTempRec != (CHAR)(QLDB_FIRST_LEVEL) )
              {
                phTree->apszCurLevelRec[i-1] = pszTempRec;
                *pusLevel = QLDB_MAX_LEVELS;
              }
              else
              {
                if ( i == QLDB_MAX_LEVELS )
                {
                  *pusLevel = QLDB_END_OF_TREE;
                  fEndOfTree = TRUE;
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endfor */
        }
        else
        {
          phTree->apszCurLevelRec[QLDB_MAX_LEVELS-1] = pszTempRec;
          *pusLevel = QLDB_MAX_LEVELS;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /*   now we have to set the pointer array to the data fields    */
      /****************************************************************/
      if ( ( *pusLevel != QLDB_END_OF_TREE ) &&
           ( usRC == QLDB_NO_ERROR ) )
      {
        if ( usTotalFields != 0 )
        {
          usBufCount = 0;

          for ( i = 0; ( i < QLDB_MAX_LEVELS ) &&
                       ( usRC == QLDB_NO_ERROR ); i++ )
          {
            pszDataInRec = phTree->apszCurLevelRec[i] + 1;

            /******************************************************/
            /*    get the number of fields of the former levels   */
            /******************************************************/
            if ( i == 0 )
            {
              usPrevFields = 0;
            }
            else
            {
              usPrevFields = usPrevFields + phTree->ausNoOfFields[i-1];
            } /* endif */

            /******************************************************/
            /*         loop over the fields in the nodes          */
            /******************************************************/
            for ( j = 0; ( j < phTree->ausNoOfFields[i]) &&
                         ( usRC == QLDB_NO_ERROR );
                  j++ )
            {
              /****************************************************/
              /*   set the pointers in the pointer array to the   */
              /*                   data fields                    */
              /*   but check whether they contain QLDB_ESC_CHAR   */
              /****************************************************/
              if ( phTree->usEscFields != 0 )
              {
                usRC = QLDBFilterEscChar( phTree, &usBufCount,
                                      &pszDataInRec,
                                      &( ppszData[usPrevFields+j] ) );
              }
              else
              {
                ppszData[usPrevFields+j] = pszDataInRec;
                pszDataInRec += UTF16strlenCHAR( pszDataInRec ) + 1;
              } /* endif */
            } /* endfor */
          } /* endfor */
        } /* endif */
      } /* endif */

      /****************************************************************/
      /*                  check if an error occurred                  */
      /*                if so restore the old pointers                */
      /*         and set the output level to QLDB_END_OF_TREE         */
      /****************************************************************/
      if ( usRC != QLDB_NO_ERROR )
      {
        for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
        {
          phTree->apszCurLevelRec[i] = apszTempRec[i];
        } /* endfor */
        *pusLevel = QLDB_END_OF_TREE;
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /*             get the previous template in the tree             */
      /****************************************************************/
      usRC = QLDBPrevTreeTemplate( phTree, QLDB_MAX_LEVELS, pusLevel );
      if ( usRC != QLDB_NO_ERROR )
      {
        /**************************************************************/
        /*       restore the saved current level node pointers        */
        /**************************************************************/
        for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
        {
          phTree->apCurLevelNode[i] = apTempNodes[i];
        } /* endfor */
        *pusLevel = QLDB_END_OF_TREE;
      }
      else
      {
        /**************************************************************/
        /*  set the pointers in the pointer array to the data fields  */
        /*     if the level returned from QLDBPrevTreeTemplate is     */
        /*   QLDB_END_OF_TREE set all pointers in the pointer array   */
        /*                      to NULL pointers                      */
        /**************************************************************/
        if ( usTotalFields != 0 )
        {
          if ( *pusLevel != QLDB_END_OF_TREE )
          {
            /**********************************************************/
            /*                  loop over the levels                  */
            /**********************************************************/
            for ( i = 0; ( i < QLDB_MAX_LEVELS ) &&
                         ( usRC == QLDB_NO_ERROR ); i++ )
            {
              /********************************************************/
              /*     get the number of fields of the former levels    */
              /********************************************************/
              if ( i == 0 )
              {
                usPrevFields = 0;
              }
              else
              {
                usPrevFields = usPrevFields + phTree->ausNoOfFields[i-1];
              } /* endif */

              /********************************************************/
              /*          loop over the fields in the nodes           */
              /********************************************************/
              for ( j = 0; ( j < phTree->ausNoOfFields[i]) &&
                           ( usRC == QLDB_NO_ERROR ); j++ )
              {
                /******************************************************/
                /*    set the pointers in the pointer array to the    */
                /*                    data fields                     */
                /******************************************************/
                ppszData[usPrevFields+j] =
                          phTree->apCurLevelNode[i]->aFields[j].pszData;
              } /* endfor */
            } /* endfor */
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /*         if the output level is QLDB_END_OF_TREE set the          */
  /*               data pointer array to NULL pointers                */
  /********************************************************************/
  if ( ( *pusLevel == QLDB_END_OF_TREE ) && ( ppszData != NULL ) )
  {
    if ( usTotalFields != 0 )
    {
      for ( i = 0; i < usTotalFields; i++ )
      {
        ppszData[i] = NULL;
      } /* endfor */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBPrevTemplate */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBRecordToTree converts a flat record to a node tree       |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBRecordToTree( ausNoOfFields, pchrRecord,                       |
//|                           usRecLength, pphTree )                           |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function converts a flat record to an in-memory node tree.           |
//|  If the functions finds an entry that is saved in the old LDB              |
//|  processing format it will accept it, convert it to the QLDB format        |
//|  and then build up the in-memory node tree.                                |
//|                                                                            |
//|  The function will return the tree handle of the tree just created.        |
//|                                                                            |
//|  If the input pointer to the tree handle is a NULL pointer it indicates    |
//|  that the record was just read from disk. Then the function will create    |
//|  the tree handle but leaves the record unchanged and chaines it with the   |
//|  tree handle.                                                              |
//|                                                                            |
//|  If the input pointer to the tree handle references an already allocated   |
//|  tree handle and the record is not yet converted the function will build   |
//|  up a full node tree. The current node pointer and the level node          |
//|  pointers will be set to the same nodes as the current record and the      |
//|  level record pointers were set.                                           |
//|  The input parameter usRecLength is not needed in this case and can        |
//|  be set to 0.                                                              |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  USHORT       ausNoOfFields[];                  // no. of fields on        |
//|                                                 // each level              |
//|  PCHAR        pchrRecord;                       // pointer to the          |
//|                                                 // record to convert       |
//|  USHORT       usRecLength;                      // length of record        |
//|                                                 // to convert              |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PQLDB_HTREE  *pphTree;                         // pointer to the tree     |
//|                                                 // handle structure        |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_TOO_MANY_FIELDS        - too many fields for a level were requested  |
//|  QLDB_NO_VALID_DATA          - the pointer to data fields did not exist    |
//|  QLDB_ERROR_IN_TREE          - an unrecoverable error is in the tree       |
//|  QLDB_ERR_DEALLOC            - error deallocating memory                   |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid record (either old LDB format or new QLDB format)               |
//|  - if the tree handle was already created a valid tree handle              |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  the record is converted to a tree                                         |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBRecordToTree( ausNoOfFields, pchrRecord, usRecLength,     |
//|                                &phTree );                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF pointer to tree handle == NULL pointer                                 |
//|    IF record is an old LDB record                                          |
//|      Convert the record to new QLDB record format (QLDBConvertRecord)      |
//|    Create tree handle (QLDBCreateTreeHandle)                               |
//|    Set current record pointer to first control character in record         |
//|    Set level record pointer of first level to current record pointer       |
//|    Set temp pointer to current record pointer                              |
//|    FOR 2 TO MAX_LEVELS DO                                                  |
//|      Find next control character (QLDBNextRecCtrlChar)                     |
//|      Set level record pointer of level number to temp pointer              |
//|    Set fWorkWithRecord in tree handle to TRUE                              |
//|  ELSE                                                                      |
//|    Set temp pointer to first control character in record                   |
//|    Set array of temp level node pointers to NULL pointers                  |
//|    WHILE temp pointer != END_OF_REC                                        |
//|      Get the level information of temp pointer                             |
//|      Set array of data pointers to fields (watch the ESC_CHAR)             |
//|      Create node on level number (QLDBCreateNode)                          |
//|      Set temp level node pointer of level number to created node           |
//|      IF level number != 1                                                  |
//|        Chain created node to node of level number - 1 (watch for           |
//|          already created sister nodes)                                     |
//|      IF temp pointer == level record pointer of level number               |
//|        Set level node pointer to created node                              |
//|      IF temp pointer == current record pointer                             |
//|        Set current node pointer to created node                            |
//|      Find next control character (QLDBNextRecCtrlChar)                     |
//|    Deallocate the record from the tree handle                              |
//+----------------------------------------------------------------------------+

USHORT QLDBRecordToTree(
  USHORT       ausNoOfFields[],                  // no. of fields on
                                                 // each level
  PSZ_W        pchrRecord,                       // pointer to the
                                                 // record to convert
  ULONG        ulRecLength,                      // length of rec( # of charw's)
                                                 // to convert
  PVOID        *ppvTree )                        // pointer to the tree
                                                 // handle structure

{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  *pphTree = (PQLDB_HTREE *)ppvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  ULONG        ulTempRecLength;                  // length of temporary
                                                 // record
  USHORT       i;                                // an index
  USHORT       usMaxFields;                      // max no of fields in
                                                 // a node in the tree
  USHORT       usLevel;                          // level information
                                                 // from the record
  USHORT       usBufCount;                       // counter for the
                                                 // esc field buffer in
                                                 // the tree handle
  PSZ_W        *ppszData;                        // pointer array for
                                                 // passing data to
                                                 // QLDBCreateNode
  PSZ_W        pszDataInRec;                     // pointer to the
                                                 // current field in the
                                                 // record
  PSZ_W        pchrTempRecord = NULL;            // temporaray record
  PSZ_W        pchrTempCtrlChar;                 // a temp pointer
                                                 // pointing to ctrl
                                                 // chars in the record
  PQLDB_NODE   pNode = NULL;                     // node pointer
  PQLDB_NODE   pTempNode;                        // temp node pointer
  PQLDB_NODE   apTempLevelNode[QLDB_MAX_LEVELS]; // temp array for the
                                                 // level node pointers
                                                 // to build up the tree


   /*******************************************************************/
   /*    check if the pointer to the tree handle is a NULL pointer    */
   /*******************************************************************/
   if ( *pphTree == NULL )
   {
     /*****************************************************************/
     /*    now create a new tree handle and join the record to the    */
     /*                          tree handle                          */
     /*   but first check if the input record is an old LDB record    */
     /*        if it is one convert it to the new QLDB format         */
     /*****************************************************************/
     if ( pchrRecord != NULL )
     {
       switch ( pchrRecord[0] )
       {
         case (CHAR_W)( QLDB_FIRST_LEVEL ) :
           {
             /*********************************************************/
             /*      allocate storage for the copy of the record      */
             /*********************************************************/
             if ( !( UtlAlloc( (PVOID *)&pchrTempRecord, 0L,
                         (LONG)( sizeof(CHAR_W) * max( MIN_ALLOC, ulRecLength )),
                               NOMSG ) ) )
             {
               usRC = QLDB_NO_MEMORY;
             } /* endif */

             /*********************************************************/
             /*        copy the record to the temporary record        */
             /*********************************************************/
             if ( usRC == QLDB_NO_ERROR )
             {
               memcpy( (PBYTE)pchrTempRecord, (PBYTE)pchrRecord,
                         ulRecLength * sizeof(CHAR_W));
               ulTempRecLength = ulRecLength;
             } /* endif */
           }
         break;

         default :
           {
             /*********************************************************/
             /*       convert the record to the new QLDB format       */
             /*********************************************************/
             usRC = QLDBConvertRecord( (PCHAR)pchrRecord, ulRecLength,
                                       ausNoOfFields,
                                       (PCHAR *)(&pchrTempRecord),
                                       &ulTempRecLength );
           }
         break;
       } /* endswitch */
     }
     else
     {
       usRC = QLDB_NO_VALID_DATA;
     } /* endif */

     if ( usRC == QLDB_NO_ERROR )
     {
       /***************************************************************/
       /*                   create the tree handle                    */
       /***************************************************************/
       usRC = QLDBCreateTreeHandle( ausNoOfFields, pphTree );
     } /* endif */

     if ( usRC == QLDB_NO_ERROR )
     {
       /***************************************************************/
       /*   Get the control information from the record and save it   */
       /*                     in the tree handle                      */
       /*   then check if a buffer area has to be allocated for the   */
       /*                         esc fields                          */
       /***************************************************************/

       (*pphTree)->usEscFields = (USHORT) *((PULONG)(&pchrTempRecord[1]));

       if ( (*pphTree)->usEscFields != 0 )
       {
         (*pphTree)->usBufferStorage = (*pphTree)->usEscFields *
                                       QLDB_FIELD_SIZE;

         if ( !( UtlAlloc( (PVOID *)&((*pphTree)->pchrEscBuffer), 0L,
                           (LONG)( sizeof(CHAR_W) * (*pphTree)->usBufferStorage),
                            NOMSG ) ) )
         {
           usRC = QLDB_NO_MEMORY;
         } /* endif */
       } /* endif */

       /***************************************************************/
       /*    set the temporaray record pointer to the "real" data     */
       /***************************************************************/
       pchrTempRecord += QLDB_START_CTRL_INFO;

       if ( usRC == QLDB_NO_ERROR )
       {
         /*************************************************************/
         /*set the current record pointer and the first level pointer */
         /*************************************************************/
         (*pphTree)->pszCurRec = (PSZ_W)pchrTempRecord;
         (*pphTree)->apszCurLevelRec[0] = (PSZ_W)pchrTempRecord;

         /*************************************************************/
         /*       set the temporaray control character pointer        */
         /*************************************************************/
         pchrTempCtrlChar = pchrTempRecord;
       } /* endif */

       for ( i = 1; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
             i++ )
       {
         /*************************************************************/
         /*                get next control character                 */
         /*************************************************************/
         usRC = QLDBNextRecCtrlChar( &pchrTempCtrlChar );

         /*************************************************************/
         /* set the level record pointer to the returned temp pointer */
         /*************************************************************/
         if ( usRC == QLDB_NO_ERROR )
         {
           (*pphTree)->apszCurLevelRec[i] = (PSZ_W)pchrTempCtrlChar;
         } /* endif */
       } /* endfor */
     } /* endif */

     if ( usRC == QLDB_NO_ERROR )
     {
       /***************************************************************/
       /*     set the fWorkWithRecord in the tree handle to TRUE      */
       /***************************************************************/
       (*pphTree)->fWorkWithRecord = TRUE;
     } /* endif */

     /*****************************************************************/
     /*    if an error occurred deallocate the tree handle and the    */
     /*                       temporary record                        */
     /*****************************************************************/
     if ( usRC != QLDB_NO_ERROR )
     {
       if ( pchrTempRecord != NULL )
       {
         pchrTempRecord -= QLDB_START_CTRL_INFO;
         UtlAlloc( (PVOID *)&pchrTempRecord, 0L, 0L, NOMSG );
       } /* endif */
       UtlAlloc( (PVOID *)pphTree, 0L, 0L, NOMSG );
       *pphTree = NULL;
     } /* endif */
   }
   else
   {
     /*****************************************************************/
     /*        a record already exists within the tree handle         */
     /*               now convert this record to a tree               */
     /*      but before check whether this record really exists       */
     /*****************************************************************/
     if ( (*pphTree)->apszCurLevelRec[0] == NULL )
     {
       usRC = QLDB_NO_TREEHANDLE;
     } /* endif */

     if ( usRC == QLDB_NO_ERROR )
     {
       /***************************************************************/
       /*   allocate storage for a pointer array to the data fields   */
       /*                   of a node in the record                   */
       /***************************************************************/
       usMaxFields = 0;
       for ( i = 0; i < QLDB_MAX_LEVELS; i++)
       {
         if ( (*pphTree)->ausNoOfFields[i] > usMaxFields )
         {
           usMaxFields = (*pphTree)->ausNoOfFields[i];
         } /* endif */
       } /* endfor */

       ppszData = NULL;
       if ( usMaxFields != 0 )
       {
         if ( !( UtlAlloc( (PVOID *)&ppszData, 0L,
                           (LONG)max( usMaxFields * sizeof( PSZ_W ), MIN_ALLOC ),
                           NOMSG ) ) )
         {
           usRC = QLDB_NO_MEMORY;
         } /* endif */
       } /* endif */

       if ( usRC == QLDB_NO_ERROR )
       {
         /*************************************************************/
         /*   now set the pointer to the first (control) character    */
         /*                       in the record                       */
         /*************************************************************/
         pchrTempCtrlChar = (PSZ_W)(*pphTree)->apszCurLevelRec[0];

         /*************************************************************/
         /*  now walk down the lonely street of the record till the   */
         /*                  ultimate end is reached                  */
         /*************************************************************/
         while ( ( *pchrTempCtrlChar != QLDB_END_OF_REC ) &&
                 ( usRC == QLDB_NO_ERROR ) )
         {
           usBufCount = 0;

           /***********************************************************/
           /*        get the level information from the record        */
           /***********************************************************/
           usLevel = QLDB_LEVEL( pchrTempCtrlChar );

           /***********************************************************/
           /*            check if the level info is valid             */
           /*         if it is not return QLDB_ERROR_IN_TREE          */
           /***********************************************************/
           if ( ( usLevel < 1 ) || ( usLevel > QLDB_MAX_LEVELS ) )
           {
             usRC = QLDB_ERROR_IN_TREE;
           } /* endif */

           if ( usRC == QLDB_NO_ERROR )
           {
             /*********************************************************/
             /*     now set the pointer array to the data fields      */
             /*     check if the record contains any fields with      */
             /*                     QLDB_ESC_CHAR                     */
             /*********************************************************/
             pszDataInRec = (PSZ_W)( pchrTempCtrlChar + 1 );
             for ( i = 0; ( i < (*pphTree)->ausNoOfFields[usLevel-1] ) &&
                          ( usRC == QLDB_NO_ERROR ); i++ )
             {
               if ( (*pphTree)->usEscFields != 0 )
               {
                 usRC = QLDBFilterEscChar( *pphTree, &usBufCount,
                                           &pszDataInRec,
                                           &( ppszData[i] ) );
               }
               else
               {
                 ppszData[i] = pszDataInRec;
                 pszDataInRec += UTF16strlenCHAR( pszDataInRec ) + 1;
               } /* endif */
             } /* endfor */
           } /* endif */

           /***********************************************************/
           /*                then let's create a node                 */
           /***********************************************************/
           if ( usRC == QLDB_NO_ERROR )
           {
             usRC = QLDBCreateNode( *pphTree, usLevel, ppszData, &pNode );
           } /* endif */

           /***********************************************************/
           /*         build up the tree by chaining the nodes         */
           /***********************************************************/
           if ( usRC == QLDB_NO_ERROR )
           {
             apTempLevelNode[usLevel-1] = pNode;

             if ( usLevel != 1 )
             {
               if ( apTempLevelNode[usLevel-2]->pChild != NULL )
               {
                 /*****************************************************/
                 /* walk to the node most right on the created level  */
                 /*      and chain the created node to that node      */
                 /*****************************************************/
                 pTempNode = apTempLevelNode[usLevel-2]->pChild;
                 while ( pTempNode->pRight != NULL )
                 {
                   pTempNode = pTempNode->pRight;
                 } /* endwhile */

                 pNode->pParent = pTempNode->pParent;
                 pNode->pLeft = pTempNode;
                 pTempNode->pRight = pNode;
               }
               else
               {
                 /*****************************************************/
                 /*  ok no child exists then let's make the created   */
                 /*                node the child node                */
                 /*****************************************************/
                 apTempLevelNode[usLevel-2]->pChild = pNode;
                 pNode->pParent = apTempLevelNode[usLevel-2];
               } /* endif */
             } /* endif */

             /*********************************************************/
             /*   now check if the current node pointers have to be   */
             /*                          set                          */
             /*********************************************************/
             if ( pchrTempCtrlChar ==
                  (PSZ_W)( (*pphTree)->apszCurLevelRec[usLevel-1] ) )
             {
               (*pphTree)->apCurLevelNode[usLevel-1] = pNode;
             } /* endif */

             if ( pchrTempCtrlChar == (PSZ_W)( (*pphTree)->pszCurRec ) )
             {
               (*pphTree)->pCurNode = pNode;
             } /* endif */
           } /* endif */

           /***********************************************************/
           /*      and now get the next node from the record by       */
           /*       searching the next record control character       */
           /***********************************************************/
           usRC = QLDBNextRecCtrlChar( &pchrTempCtrlChar );
         } /* endwhile */

         /*************************************************************/
         /*               deallocate the pointer array                */
         /*************************************************************/
         UtlAlloc( (PVOID *)&ppszData, 0L, 0L, NOMSG );

         /*************************************************************/
         /*  if everything went ok we can now deallocate the record   */
         /*            and the esc field buffer if exists             */
         /*      and set the pointer in the tree handle to NULL       */
         /*  if an error ocurred deallocate the tree just build and   */
         /*    set the current level and node pointers in the tree    */
         /*                      handle to NULL                       */
         /*************************************************************/
         if ( usRC == QLDB_NO_ERROR )
         {
           if ( (*pphTree)->pchrEscBuffer != NULL )
           {
             if ( !( UtlAlloc( (PVOID *)&( (*pphTree)->pchrEscBuffer ), 0L, 0L,
                               NOMSG ) ) )
             {
               usRC = QLDB_ERR_DEALLOC;
             } /* endif */
           } /* endif */
         } /* endif */

         if ( usRC == QLDB_NO_ERROR )
         {
           (*pphTree)->apszCurLevelRec[0] -= QLDB_START_CTRL_INFO;
           if ( !( UtlAlloc( (PVOID *)&( (*pphTree)->apszCurLevelRec[0] ), 0L,
                             0L, NOMSG ) ) )
           {
             usRC = QLDB_ERR_DEALLOC;
           }
           else
           {
             (*pphTree)->pszCurRec = NULL;
             for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
             {
               (*pphTree)->apszCurLevelRec[i] = NULL;
             } /* endfor */

             (*pphTree)->fWorkWithRecord = FALSE;
           } /* endif */
         } /* endif */

         if ( usRC != QLDB_NO_ERROR )
         {
           QLDBDestroyIncompleteSubtree( *pphTree, 1 );
           (*pphTree)->pCurNode = NULL;
           for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
           {
             (*pphTree)->apCurLevelNode[i] = NULL;
           } /* endfor */
         } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */

   return( usRC );

} /* end of function QLDBRecordToTree */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBResetTreePositions resets curr. node and level positions |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBResetTreePositions( phTree )                                   |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function sets the current node pointer to the first level pointer    |
//|  and the pointers to the level nodes to template most left.                |
//|                                                                            |
//|  This function is able to work with records. The pointers to the record    |
//|  are set accordingly                                                       |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//+----------------------------------------------------------------------------+
//|Output parameter: none                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the current node pointer is set to the node on the first level          |
//|  - the level node pointers are set to the template most left               |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBResetTreePositions( &hTree );                             |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|    Set pointer to current record part to beginning of record               |
//|    FOR 2 to MAX_LEVELS DO                                                  |
//|      Set temp pointer to pointer to level record of level number - 1       |
//|      Find next control character (QLDBNextRecCtrlChar)                     |
//|      Set pointer to level record of level number to temp pointer           |
//|  ELSE                                                                      |
//|    Set pointer to current node to the pointer to the first level node      |
//|    FOR 2 to MAX_LEVELS DO                                                  |
//|      Set pointer to level node on level number to the child node of        |
//|        level node of level number - 1                                      |
//+----------------------------------------------------------------------------+

USHORT QLDBResetTreePositions
(
  PVOID        pvTree                            // handle to tree
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  PSZ_W        pszTempCurRec;                    // temporaray pointer
                                                 // to save the current
                                                 // record pointer
                                                 // to restore in case
                                                 // of problems
  PSZ_W        apszTemp[QLDB_MAX_LEVELS];        // temporaray array to
                                                 // save old current
                                                 // pointers in case of
                                                 // problems



  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( ( phTree->apCurLevelNode[i] == NULL ) &&
         ( phTree->apszCurLevelRec[i] == NULL ) )
    {
      usRC = QLDB_NO_TREEHANDLE;
    }
    else
    {
      if ( phTree->fWorkWithRecord == TRUE )
      {
        apszTemp[i] = phTree->apszCurLevelRec[i];
      } /* endif */
    } /* endif */
  } /* endfor */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          check if the tree handle references a record          */
    /******************************************************************/
    if ( phTree->fWorkWithRecord == TRUE )
    {
      pszTempCurRec = phTree->pszCurRec;
      /****************************************************************/
      /*                  reset the record pointers                   */
      /****************************************************************/
      phTree->pszCurRec = phTree->apszCurLevelRec[0];
      for ( i = 1; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
            i++ )
      {
        phTree->apszCurLevelRec[i] = phTree->apszCurLevelRec[i-1];
        usRC = QLDBNextRecCtrlChar( &( phTree->apszCurLevelRec[i] ) );
      } /* endfor */

      /****************************************************************/
      /*       if an error occurred restore the saved pointers        */
      /****************************************************************/
      if ( usRC != QLDB_NO_ERROR )
      {
        phTree->pszCurRec = pszTempCurRec;

        for ( i = 1; i < QLDB_MAX_LEVELS; i++ )
        {
          phTree->apszCurLevelRec[i] = apszTemp[i];
        } /* endfor */
      } /* endif */
    }
    else
    {
      /****************************************************************/
      /*    reset the current node pointer to the first level node    */
      /*       and reset the current level node pointers to the       */
      /*                      template most left                      */
      /****************************************************************/
      phTree->pCurNode = phTree->apCurLevelNode[0];

      for ( i = 1; i < QLDB_MAX_LEVELS; i++ )
      {
        phTree->apCurLevelNode[i] =
          phTree->apCurLevelNode[i-1]->pChild;
      } /* endfor */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBResetTreePositions */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBTreeToRecord converts a node tree to a flat record       |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBTreeToRecord( phTree, ppchrRecord, pusRecLength )              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function converts an in-memory node tree to a flat data record.      |
//|  The record is returned as well as the length of the record.               |
//|                                                                            |
//|  After the function finished the tree is reset to its initial positions.   |
//|                                                                            |
//|  If a problem occurrs the pointer to the record is set to NULL and the     |
//|  returned length is 0. The returncode will give the reason for the         |
//|  error.                                                                    |
//|                                                                            |
//|  If the record would exceed the maximum allowed size for a record the      |
//|  function gives a returncode of QLDB_REC_TOO_LONG.                         |
//|                                                                            |
//|  The function is able to work with records. If a record is already found   |
//|  only its length is computed and then the record is returned.              |
//|  The record is also reset to its initial positions.                        |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|  PCHAR        *ppchrRecord;                     // pointer to a            |
//|                                                 // pointer to the          |
//|                                                 // record                  |
//|  USHORT       *pusRecLength;                    // record length           |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_NO_VALID_DATA          - the pointer to data fields did not exist    |
//|  QLDB_ERROR_IN_TREE          - an unrecoverable error is in the tree       |
//|  QLDB_REC_TOO_LONG           - record is too long to process               |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the data of the fields from the tree is copied to the record            |
//|  - the current node pointer is set to the first level node                 |
//|  - the current level pointers are set to the template most left            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBTreeToRecord( phTree, &pchrRecord, &usRecLength );        |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  Reset tree position (QLDBResetTreePositions)                              |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|   Compute length of record                                                 |
//|   Set pointer to record to first level record pointer                      |
//|  ELSE                                                                      |
//|    Convert the record to the most compact form (QLDBJoinSameNodes)         |
//|    Set record length to 0                                                  |
//|    REPEAT                                                                  |
//|      Compute the bytes for the current node (QLDBNodeToRecStorage)         |
//|      Get next node (QLDBNextNode)                                          |
//|    UNTIL returned level == QLDB_END_OF_TREE                                |
//|    Add QLDB_CTRL_INFO to record length                                     |
//|    Allocate storage for record                                             |
//|    IF storage allocation went ok                                           |
//|      Reset tree positions (QLDBResetTreePositions)                         |
//|      REPEAT                                                                |
//|        Copy data fields in node to record (QLDBCopyNodeToRecord)           |
//|        Get next node (QLDBNextNode)                                        |
//|      UNTIL returned level == QLDB_END_OF_TREE                              |
//|      Copy QLDB_END_OF_REC to record                                        |
//|      Reset tree positions (QLDBResetTreePositions)                         |
//+----------------------------------------------------------------------------+

USHORT QLDBTreeToRecord
(
  PVOID        pvTree,                           // handle to tree
  PSZ_W        *ppchrRecord,                     // pointer to a
                                                 // pointer to the
                                                 // record
  ULONG       *pulRecLength                     // record length
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  USHORT       usTotalFields = 0;                // no. of fields in
                                                 // template
  USHORT       usLevel = 0;                          // level number from
                                                 // QLDBNextNode
  USHORT       usRecCount;                       // record counter
  USHORT       usStorage;                        // storage needed for
                                                 // a node
  USHORT       usEscFields;                      // no. of fields with
                                                 // at least one
                                                 // QLDB_ESC_CHAR
  PSZ_W        *ppszData;                        // pointer array for
                                                 // QLDBNextNode
  PSZ_W        pszTempRec;                       // temp pointer to walk
                                                 // along the record



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pulRecLength = 0;
  *ppchrRecord = NULL;
  usEscFields = 0;

  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( ( phTree->apCurLevelNode[i] == NULL ) &&
         ( phTree->apszCurLevelRec[i] == NULL ) )
    {
      usRC = QLDB_NO_TREEHANDLE;
    } /* endif */
  } /* endfor */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*                    reset the tree positions                    */
    /******************************************************************/
    usRC = QLDBResetTreePositions( phTree );

    if ( usRC == QLDB_NO_ERROR )
    {
      /****************************************************************/
      /*           check if the "tree" is already a record            */
      /****************************************************************/
      if ( phTree->fWorkWithRecord == TRUE )
      {
        /**************************************************************/
        /*                 compute the record length                  */
        /* in number of char_w's                                      */
        /**************************************************************/
        pszTempRec = phTree->apszCurLevelRec[0];
        *pulRecLength = QLDB_START_CTRL_INFO ;
        while ( *pszTempRec != (CHAR_W)( QLDB_END_OF_REC ) )
        {
          if ( *pszTempRec == (CHAR_W)( QLDB_ESC_CHAR ) )
          {
            pszTempRec += 2;
            (*pulRecLength) += 2;
          } /* endif */

          pszTempRec++;
          (*pulRecLength)++;
        } /* endwhile */

        /**************************************************************/
        /*        add 1 byte for the QLDB_END_OF_REC character        */
        /**************************************************************/
        (*pulRecLength)++;

        /**********************************************************/
        /*          allocate the storage for the record           */
        /**********************************************************/
        if ( !( UtlAlloc( (PVOID *)ppchrRecord, 0L,
                      (LONG)max(MIN_ALLOC,(*pulRecLength)*sizeof(CHAR_W)),
                          NOMSG ) ) )
        {
          usRC = QLDB_NO_MEMORY;
        } /* endif */

        /**************************************************************/
        /* copy the record to the allocated area                      */
        /**************************************************************/
        if ( usRC == QLDB_NO_ERROR )
        {
          pszTempRec = phTree->apszCurLevelRec[0];
          pszTempRec -= QLDB_START_CTRL_INFO;
          memcpy( (PBYTE)*ppchrRecord, (PBYTE)pszTempRec,
                  (*pulRecLength) * sizeof(CHAR_W) );
        } /* endif */

      }
      else
      {
        /**************************************************************/
        /*  convert the tree to the compact form, so that less data   */
        /*                      is used on disk                       */
        /**************************************************************/
        usRC = QLDBJoinSameNodes( phTree );

        if ( usRC == QLDB_NO_ERROR )
        {
          /************************************************************/
          /*  now let's calculate the storage needed for the record   */
          /************************************************************/

          /************************************************************/
          /*    allocate storage for the pointer array to the data    */
          /*             fields returned by  QLDBNextNode             */
          /************************************************************/
          ppszData = NULL;
          for ( i = 0; i < QLDB_MAX_LEVELS; i++ )
          {
            usTotalFields = usTotalFields + phTree->ausNoOfFields[i];
          } /* endfor */

          if ( usTotalFields != 0 )
          {
            if ( !( UtlAlloc( (PVOID *)&ppszData, 0L,
                              (LONG)max( MIN_ALLOC, usTotalFields * sizeof( PSZ_W ) ),
                              NOMSG ) ) )
            {
              usRC = QLDB_NO_MEMORY;
            } /* endif */
          } /* endif */

          if ( usRC == QLDB_NO_ERROR )
          {
            /**********************************************************/
            /*   now loop over the tree and get the number of CHAR_W's */
            /*                  needed for each node                  */
            /**********************************************************/
            do
            {
              usRC = QLDBNodeToRecStorage( phTree, &usStorage,
                                           &usEscFields );

              /********************************************************/
              /*   check if the record length would exceed maximum    */
              /*             size of QLDB_MAX_REC_LENGTH              */
              /*  QLDB_CTRL_INFOW, *pulRecLength is in no of char_w's */
              /********************************************************/
              if ( usRC == QLDB_NO_ERROR )
              {
                if ( *pulRecLength + usStorage + QLDB_CTRL_INFO >
                     QLDB_MAX_REC_LENGTHW )
                {
                  usRC = QLDB_REC_TOO_LONG;
                } /* endif */
              } /* endif */

              if ( usRC == QLDB_NO_ERROR )
              {
                /******************************************************/
                /*     increase the record length by the computed     */
                /*                       value                        */
                /******************************************************/
                *pulRecLength += usStorage;

                /******************************************************/
                /*                   get next node                    */
                /******************************************************/
                usRC = QLDBNextNode( phTree, ppszData, &usLevel );
              } /* endif */
            } while ( ( usLevel != QLDB_END_OF_TREE ) &&
                      ( usRC == QLDB_NO_ERROR ) ); /* enddo */

            /**********************************************************/
            /*       add the number of bytes needed for the control   */
            /* information at the beginning and the end of the record */
            /**********************************************************/
            if ( usRC == QLDB_NO_ERROR )
            {
              (*pulRecLength) += QLDB_CTRL_INFO;
            } /* endif */
          } /* endif */

          if ( usRC == QLDB_NO_ERROR )
          {
            /**********************************************************/
            /*          allocate the storage for the record           */
            /**********************************************************/
            if ( !( UtlAlloc( (PVOID *)ppchrRecord, 0L,
                           (LONG)max( MIN_ALLOC, (*pulRecLength)*sizeof(CHAR_W)),
                            NOMSG ) ) )
            {
              usRC = QLDB_NO_MEMORY;
            } /* endif */
          } /* endif */

          if ( usRC == QLDB_NO_ERROR )
          {
            /**********************************************************/
            /*                reset the tree positions                */
            /**********************************************************/
            usRC = QLDBResetTreePositions( phTree );
          } /* endif */

          if ( usRC == QLDB_NO_ERROR )
          {
            ULONG  ulEscFields = 0L;
            usRecCount = 0;

            /**********************************************************/
            /*   copy the ctrl info at the beginning of the record    */
            /*                     to the record                      */
            /**********************************************************/
            (*ppchrRecord)[0] = (CHAR_W)QLDB_FIRST_LEVEL;
            ulEscFields = usEscFields;
            memcpy( &((*ppchrRecord)[1]), &ulEscFields, sizeof(ULONG) );
            usRecCount = QLDB_START_CTRL_INFO;  // no char_w's filled already

            /**********************************************************/
            /*              copy the data to the record               */
            /**********************************************************/
            do
            {
              usRC = QLDBCopyNodeToRecord( phTree, *ppchrRecord,
                                           &usRecCount );
              if ( usRC == QLDB_NO_ERROR )
              {
                usRC = QLDBNextNode( phTree, ppszData, &usLevel );
              } /* endif */
            } while ( ( usLevel != QLDB_END_OF_TREE ) &&
                      ( usRC == QLDB_NO_ERROR ) ); /* enddo */

            /**********************************************************/
            /*    add the QLDB_END_OF_REC character to the record     */
            /**********************************************************/
            if ( usRC == QLDB_NO_ERROR )
            {
              (*ppchrRecord)[usRecCount] = (CHAR_W)( QLDB_END_OF_REC );
            } /* endif */

            /**********************************************************/
            /*                reset the tree positions                */
            /**********************************************************/
            if ( usRC == QLDB_NO_ERROR )
            {
              usRC = QLDBResetTreePositions( phTree );
            } /* endif */
          } /* endif */

          /************************************************************/
          /*    check if an error occurred and deallocate then the    */
          /*          record and set the record length to 0           */
          /************************************************************/
          if ( usRC != QLDB_NO_ERROR )
          {
            if ( *ppchrRecord != NULL )
            {
              UtlAlloc( (PVOID *)ppchrRecord, 0L, 0L, NOMSG );
            } /* endif */

            *ppchrRecord = NULL;
            *pulRecLength = 0;
          } /* endif */

          /************************************************************/
          /*               deallocate the pointer array               */
          /************************************************************/
          if ( ppszData != NULL )
          {
            UtlAlloc( (PVOID *)&ppszData, 0L, 0L, NOMSG );
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBTreeToRecord */



USHORT QLDBTreeToRecordNonUnicode
(
  PVOID        pvTree,                           // handle to tree
  PCHAR        *ppchrRecord,                     // pointer to a
                                                 // pointer to the
                                                 // record
  ULONG       *pulRecLength                     // record length
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  PSZ_W        pszTempRec;                       // temp pointer to walk
                                                 // along the record



  /********************************************************************/
  /*                       initialize variables                       */
  /********************************************************************/
  *pulRecLength = 0;
  *ppchrRecord = NULL;

  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( ( phTree->apCurLevelNode[i] == NULL ) &&
         ( phTree->apszCurLevelRec[i] == NULL ) )
    {
      usRC = QLDB_NO_TREEHANDLE;
    } /* endif */
  } /* endfor */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*                    reset the tree positions                    */
    /******************************************************************/
    usRC = QLDBResetTreePositions( phTree );

    if ( usRC == QLDB_NO_ERROR )
    {
      /**************************************************************/
      /*                 compute the record length                  */
      /**************************************************************/
      pszTempRec = phTree->apszCurLevelRec[0];
      *pulRecLength = QLDB_START_CTRL_INFO;
      while ( *pszTempRec != (CHAR)( QLDB_END_OF_REC ) )
      {
        if ( *pszTempRec == (CHAR)( QLDB_ESC_CHAR ) )
        {
          pszTempRec += 2;
          (*pulRecLength) += 2;
        } /* endif */
        pszTempRec++;
        (*pulRecLength)++;
      } /* endwhile */

      /**************************************************************/
      /*        add 1 byte for the QLDB_END_OF_REC character        */
      /**************************************************************/
      (*pulRecLength)++;

      /**********************************************************/
      /*          allocate the storage for the record           */
      /**********************************************************/
      if ( !( UtlAlloc( (PVOID *)ppchrRecord, 0L,
                        (LONG)max( MIN_ALLOC, *pulRecLength * sizeof(CHAR_W)),
                        NOMSG ) ) )
      {
        usRC = QLDB_NO_MEMORY;
      } /* endif */
      /**************************************************************/
      /* copy the record to the allocated area                      */
      /**************************************************************/
      if ( usRC == QLDB_NO_ERROR )
      {
        ULONG ulRecLen = (*pulRecLength - QLDB_START_CTRL_INFO);
        PBYTE  pchrRecord = (PBYTE)(*ppchrRecord + QLDB_START_CTRL_INFO);

        ulRecLen = Unicode2ASCIIBuf( phTree->apszCurLevelRec[0], (PSZ) pchrRecord,
                                     UTF16strlenCHAR(phTree->apszCurLevelRec[0])+1,
                                     ulRecLen, 0L );
        *((PBYTE)pchrRecord-QLDB_START_CTRL_INFO) = (BYTE) *(phTree->apszCurLevelRec[0]-QLDB_START_CTRL_INFO);
        *((PUSHORT)pchrRecord-QLDB_START_CTRL_INFO+1) = (USHORT)*((PULONG)(phTree->apszCurLevelRec[0]-QLDB_START_CTRL_INFO+1));
        *pulRecLength = ulRecLen + QLDB_START_CTRL_INFO;
//        pszTempRec -= QLDB_START_CTRL_INFO;
//        memcpy( *ppchrRecord, pszTempRec, *pulRecLength );
      } /* endif */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBTreeToRecordNonUnicode */






//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBUpdateCurNodeData updates the data in the current node   |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBUpdateCurNodeData( phTree, usField, pszData, ppszData )        |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This function makes an update to the data in the current node.            |
//|  It can either be called to fill a specified field in the current          |
//|  node with the data for just that field or to fill up all fields in the    |
//|  node.                                                                     |
//|                                                                            |
//|  If only one field should be updated the function gets the number of the   |
//|  field to be filled and a pointer to a zero-terminated string (PSZ) which  |
//|  references the data to be put in the field.                               |
//|                                                                            |
//|  In order to update all fields the function requires an array of pointers  |
//|  that reference zero-terminated strings (PSZ) and the calling function     |
//|  has to assure that the number of pointers in the array correspond with    |
//|  the number of fields in that node.                                        |
//|                                                                            |
//|  If the function is called on a node that contains no fields at all        |
//|  the function does nothing and ends with QLDB_NO_ERROR.                    |
//|                                                                            |
//|  This function is able to work with records. The record will be converted  |
//|  into a full tree and then the function makes an update on the current node|
//|  (the tree will then not be reconverted to a record).                      |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;                           // handle to tree          |
//|  USHORT       usField;                          // No. of field to be      |
//|                                                 // updated, a value of     |
//|                                                 // 0 indicates that all    |
//|                                                 // fields in the node      |
//|                                                 // are to be updated and   |
//|                                                 // the data can be found   |
//|                                                 // in ppszData             |
//|                                                 // otherwise only the      |
//|                                                 // field is updated with   |
//|                                                 // pszData                 |
//|  PSZ          pszData;                          // pointer to              |
//|                                                 // zero-terminated string  |
//|                                                 // with the data for the   |
//|                                                 // field to be updated     |
//|  PSZ          *ppszData;                        // pointer to an           |
//|                                                 // array of pointers       |
//|                                                 // pointing to the data    |
//|                                                 // that is to be supplied  |
//|                                                 // for each field in the   |
//|                                                 // node                    |
//+----------------------------------------------------------------------------+
//|Output parameter: none                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_MEMORY              - not enough memory left                      |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_TOO_MANY_FIELDS        - too many fields for a level were requested  |
//|  QLDB_NO_VALID_DATA          - the data fields did not exist               |
//|  QLDB_INVALID_FIELDNO        - an invalid field number was entered         |
//|  QLDB_ERROR_IN_TREE          - an unrecoverable error is in the tree       |
//|  QLDB_ERR_DEALLOC            - error deallocating memory                   |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  - a valid tree handle                                                     |
//|  - if ppszData is used the calling function has to assure that             |
//|    the pointer array contains the same number of pointers as there         |
//|    are fields in the node                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|  - the field(s) in the node is/are updated                                 |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBUpdateCurNodeData( phTree, pNode, 3, "NEW" );             |
//|                                                                            |
//|  The field 3 of pNode is updated with "NEW".                               |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|    Convert the record to a tree (QLDBRecordToTree)                         |
//|  IF one field is to be updated                                             |
//|    Update the field (QLDBUpdateFieldData)                                  |
//|  ELSE                                                                      |
//|    Make a copy of the current node in order to allow a possible restore    |
//|    FOR 1 TO usNoOfFields DO                                                |
//|      Update the field (QLDBUpdateFieldData)                                |
//+----------------------------------------------------------------------------+

USHORT QLDBUpdateCurNodeData(

  PVOID        pvTree,                           // handle to tree
  USHORT       usField,                          // No. of field to be
                                                 // updated, a value of
                                                 // 0 indicates that all
                                                 // fields in the node
                                                 // are to be updated and
                                                 // the data can be found
                                                 // in ppszData
                                                 // otherwise only the
                                                 // field is updated with
                                                 // pszData
  PSZ_W        pszData,                          // pointer to
                                                 // zero-terminated string
                                                 // with the data for the
                                                 // field to be updated
  PSZ_W        *ppszData )                       // pointer to an
                                                 // array of pointers
                                                 // pointing to the data
                                                 // that is to be supplied
                                                 // for each field in the
                                                 // node

{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       i;                                // an index
  PQLDB_NODE   pTempNode;                        // temporary pointer to
                                                 // copy of current node



  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /********************************************************************/
  for ( i = 0; ( i < QLDB_MAX_LEVELS ) && ( usRC == QLDB_NO_ERROR );
        i++ )
  {
    if ( ( phTree->apCurLevelNode[i] == NULL ) &&
         ( phTree->apszCurLevelRec[i] == NULL ) )
    {
      usRC = QLDB_NO_TREEHANDLE;
    } /* endif */
  } /* endfor */

  /******************************************************************/
  /*  check if the pointer to the data area is not a NULL pointer   */
  /*            if it is NULL return QLDB_NO_VALID_DATA             */
  /******************************************************************/
  if ( ( usField == 0 ) && ( ppszData == NULL ) &&
       ( usRC == QLDB_NO_ERROR ) )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          check if the tree handle references a record          */
    /*   if so convert the record to a tree before starting to work   */
    /******************************************************************/
    if ( phTree->fWorkWithRecord == TRUE )
    {
      /****************************************************************/
      /*                 convert the record to a tree                 */
      /****************************************************************/
      usRC = QLDBRecordToTree( phTree->ausNoOfFields,
                               phTree->apszCurLevelRec[0],
                               0, (PVOID *)&phTree );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* check if the update should be done on a node with no data fields */
  /*                              at all                              */
  /********************************************************************/
  if ( ( phTree != NULL ) &&
       ( phTree->ausNoOfFields[(phTree->pCurNode->usLevel)-1] != 0 ) &&
       ( usRC == QLDB_NO_ERROR ) )
  {
    /******************************************************************/
    /*       check if one field is to be updated or all fields        */
    /******************************************************************/
    if ( usField > 0 )
    {
      /****************************************************************/
      /*                       update the field                       */
      /****************************************************************/
      usRC = QLDBUpdateFieldData( phTree, phTree->pCurNode, usField,
                                  pszData );
    }
    else
    {
      /****************************************************************/
      /*    make a copy of the current node in order to restore it    */
      /*                  easily in case of problems                  */
      /****************************************************************/
      usRC = QLDBCopyNode( phTree, phTree->pCurNode, &pTempNode );

      /****************************************************************/
      /*           now loop over all fields and update them           */
      /****************************************************************/
      for ( i = 0;
            ( i < phTree->ausNoOfFields[(phTree->pCurNode->usLevel)-1] )
            && ( usRC == QLDB_NO_ERROR ); i++ )
      {
        usRC = QLDBUpdateFieldData( phTree, phTree->pCurNode, (USHORT)(i + 1),
                                    ppszData[i] );
        if ( usRC != QLDB_NO_ERROR )
        {
          /************************************************************/
          /*                   restore the old node                   */
          /************************************************************/
          QLDBDestroyNode( phTree, &(phTree->pCurNode) );
          QLDBCopyNode( phTree, pTempNode, &(phTree->pCurNode) );
        } /* endif */
      } /* endfor */

      /****************************************************************/
      /*                  destroy the temporary node                  */
      /****************************************************************/
      QLDBDestroyNode( phTree, &pTempNode );
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBUpdateCurNodeData */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QLDBFillFieldTables  Create field tables from profile    |
//+----------------------------------------------------------------------------+
//|Function call:     QLDBFillFieldTables( PPROPDICTIONARY pProfile,           |
//|                                          PUSHORT         ausNoOfFields,    |
//|                                          PUSHORT         ausFirstField );  |
//+----------------------------------------------------------------------------+
//|Description:       Creates the no-of-fields-per-dictionary-level table and  |
//|                   the first-field-of-level table. If a table area pointer  |
//|                   is NULL the table is not filled by the function.         |
//|                   The no-of-fields-per-dictionary-level table contains     |
//|                   the number of fields per level of the dictionary. The    |
//|                   index into the table is the dictionary level - 1.        |
//|                   The first-field-of-level table contains the number of    |
//|                   the first dictionary field of the dictionary levels.     |
//|                   The index into the table is the dictionary level - 1.    |
//+----------------------------------------------------------------------------+
//|Input parameter:   PPROPDICTIONARY   pProfile  pointer to dictionary profile|
//|                   PUSHORT           ausNoOfFields address of table area    |
//|                                                   or NULL if table is not  |
//|                                                   needed                   |
//|                   PUSHORT           ausFirstField address of table area    |
//|                                                   or NULL if table is not  |
//|                                                   needed                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       QLDB_NO_ERROR        if function completed OK            |
//|                   QLDB_NO_VALID_DATA   if profile pointer is not set       |
//+----------------------------------------------------------------------------+
//|Prerequesits:      The memory for the tables must have been allocated by    |
//|                   the calling function. The size of the tables must be     |
//|                   QLDB_MAX_LEVELS.                                         |
//+----------------------------------------------------------------------------+
//|Function flow:     check profile pointer                                    |
//|                   erase tables if table pointers are given                 |
//|                   loop through profile field table                         |
//|                     if last level is smaller than level of current field   |
//|                       while last level is smaller than field level         |
//|                         store field number in first field table            |
//|                         increment last level                               |
//|                       endwhile                                             |
//|                     endif                                                  |
//|                     increment field number of current level                |
//|                   endloop                                                  |
//+----------------------------------------------------------------------------+
USHORT QLDBFillFieldTables
(
  PPROPDICTIONARY   pProfile,          // pointer to dictionary profile
  PUSHORT           ausNoOfFields,     // address of table area or NULL if
                                       // table is not needed
  PUSHORT           ausFirstField      // address of table area or NULL if
                                       // table is not needed
)
{
  USHORT           usRC = QLDB_NO_ERROR; // function return code
  USHORT           usI;                // loop index
  PPROFENTRY       pProfEntry;         // pointer to dictionary profile entry
  USHORT           usLastLevel;        // number of last level processed

  /********************************************************************/
  /* Check dictionary profile pointer                                 */
  /********************************************************************/
  if ( !pProfile )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */


  /********************************************************************/
  /* Erase field tables                                               */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    if ( ausNoOfFields )
    {
      memset( ausNoOfFields, 0, sizeof(USHORT) * QLDB_MAX_LEVELS );
    } /* endif */
    if ( ausFirstField )
    {
      memset( ausFirstField, 0, sizeof(USHORT) * QLDB_MAX_LEVELS );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* Fill tables                                                      */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    usLastLevel = 0;
    pProfEntry = pProfile->ProfEntry;
    for ( usI = 0; usI < pProfile->usLength; usI++, pProfEntry++ )
    {
      if ( pProfEntry->usLevel > usLastLevel )
      {
        do
        {
          if ( ausFirstField )
          {
            ausFirstField[ usLastLevel ] = usI;
          } /* endif */
          usLastLevel++;
        } while ( pProfEntry->usLevel > usLastLevel ); /* enddo */
      } /* endif */
      if ( ausNoOfFields )
      {
        ausNoOfFields[ usLastLevel - 1 ]++;
      } /* endif */
    } /* endfor */
  } /* endif */

  return( usRC );
} /* end of function QLDBFillFieldTables */


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name: QLDBUpdateCurrTemplate sets the data for the current template|
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|  USHORT QLDBUpdateCurrTemplate( phTree, ppszData )                         |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  Updates the fields of the current template with the given data.           |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|  PQLDB_HTREE  phTree;       handle to tree                                 |
//|  PSZ          *ppszData;    pointer to an array of pointers pointing to    |
//|                             the data that is used to update the fields of  |
//|                             the current template                           |
//+----------------------------------------------------------------------------+
//|Output parameter: none                                                      |
//+----------------------------------------------------------------------------+
//|Returncode type: USHORT                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|  QLDB_NO_ERROR               - everything went ok                          |
//|  QLDB_NO_TREEHANDLE          - no valid pointer to a handle structure      |
//|                                was given as input                          |
//|  QLDB_NO_VALID_DATA          - the pointer to data fields did not exist    |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|  a valid tree handle                                                       |
//+----------------------------------------------------------------------------+
//|Side effects: none                                                          |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//|  usRetCode = QLDBUpdateCurrTemplate( phTree, ppszData );                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|  check input paramters                                                     |
//|  IF fWorkWithRecord in tree handle == TRUE                                 |
//|     convert record to tree using QLDBRecordToTree                          |
//|  LOOP over all nodes of the template                                       |
//|    LOOP over all fields of the current node                                |
//|      Update data fields                                                    |
//+----------------------------------------------------------------------------+
USHORT QLDBUpdateCurrTemplate
(
  PVOID        pvTree,                           // handle to tree
  PSZ_W        *ppszData                         // pointer to an
                                                 // array of pointers
                                                 // pointing to the data
                                                 // that is read from the
                                                 // nodes in the current
                                                 // template
)
{
  /********************************************************************/
  /*                   local variables definitions                    */
  /********************************************************************/
  PQLDB_HTREE  phTree = (PQLDB_HTREE)pvTree;     // handle to tree
  USHORT       usRC = QLDB_NO_ERROR;             // function return code
  USHORT       usTotalFields = 0;                    // number of fields in
                                                 // template
  USHORT       i;                                // an index
  USHORT       j;                                // another index
  USHORT       usPrevFields = 0;                     // no. of fields on all
                                                 // former levels

  /********************************************************************/
  /*      check if pointer to tree handle structure is not NULL       */
  /*             if it is NULL return QLDB_NO_TREEHANDLE              */
  /********************************************************************/
  if ( phTree == NULL )
  {
    usRC = QLDB_NO_TREEHANDLE;
  } /* endif */

  /********************************************************************/
  /*    check if the level pointers to the nodes are all not NULL     */
  /*  if one of them is NULL return QLDB_NO_TREEHANDLE because this   */
  /*         means not a valid tree handle was given as input         */
  /********************************************************************/
  for ( i = 1; ( i <= QLDB_MAX_LEVELS ) && (usRC == QLDB_NO_ERROR); i++ )
  {
    if ( phTree->apCurLevelNode[i-1] == NULL )
    {
      usRC = QLDB_NO_TREEHANDLE;
    } /* endif */
  } /* endfor */

  /********************************************************************/
  /*        compute the number of total fields in the template        */
  /********************************************************************/
  if ( usRC == QLDB_NO_ERROR )
  {
    usTotalFields = 0;
    for ( i = 1; i <= QLDB_MAX_LEVELS; i++ )
    {
      usTotalFields = usTotalFields + phTree->ausNoOfFields[i-1];
    } /* endfor */
  } /* endif */

  /********************************************************************/
  /*   check if the pointer to the pointer array is a valid pointer   */
  /********************************************************************/
  if ( (usRC == QLDB_NO_ERROR)  &&
       (ppszData == NULL)       &&
       (usTotalFields != 0) )
  {
    usRC = QLDB_NO_VALID_DATA;
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /******************************************************************/
    /*          check if the tree handle references a record          */
    /*   if so convert the record to a tree before starting to work   */
    /******************************************************************/
    if ( phTree->fWorkWithRecord == TRUE )
    {
      /****************************************************************/
      /*                 convert the record to a tree                 */
      /****************************************************************/
      usRC = QLDBRecordToTree( phTree->ausNoOfFields,
                               phTree->apszCurLevelRec[0],
                               0, (PVOID *)&phTree );
    } /* endif */
  } /* endif */

  if ( usRC == QLDB_NO_ERROR )
  {
    /****************************************************************/
    /*   set the pointers in the pointer array to the data fields   */
    /****************************************************************/
    if ( usTotalFields != 0 )
    {
      /**************************************************************/
      /*                    loop over the levels                    */
      /**************************************************************/
      for ( i = 1; (i <= QLDB_MAX_LEVELS) && (usRC == QLDB_NO_ERROR); i++)
      {
        /************************************************************/
        /*       get the number of fields of the former levels      */
        /************************************************************/
        if ( i == 1 )
        {
          usPrevFields = 0;
        }
        else
        {
          usPrevFields = usPrevFields + phTree->ausNoOfFields[i-2];
        } /* endif */

        /****************************************************************/
        /*           now loop over all fields and update them           */
        /****************************************************************/
        for ( j = 1;
              (j <= phTree->ausNoOfFields[(phTree->apCurLevelNode[i-1]->usLevel)-1] )
              && (usRC == QLDB_NO_ERROR);
              j++ )
        {
          if ( UTF16strcmp( phTree->apCurLevelNode[i-1]->aFields[j-1].pszData,
                       ppszData[usPrevFields+j-1] ) != 0 )
          {
            usRC = QLDBUpdateFieldData( phTree,
                                        phTree->apCurLevelNode[i-1],
                                        j,
                                        ppszData[usPrevFields+j-1] );
          } /* endif */
        } /* endfor */
      } /* endfor */
    } /* endif */
  } /* endif */

  return( usRC );

} /* end of function QLDBUpdateCurrTemplate */
