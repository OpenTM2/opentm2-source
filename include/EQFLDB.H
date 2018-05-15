//+----------------------------------------------------------------------------+
//|EQFLDB.H                                                                    |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This file contains the type definitions and the constants used            |
//|  in QLDB processing (source code is in EQFQLDB.C and EQFQLDBI.C)           |
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
// $Revision: 1.1 $ ----------- 31 Aug 2004
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
// $Revision: 1.3 $ ----------- 22 Oct 2001
// -- RJ: move QLDB_START_CTRL_INFO (from EQFLDBI.h), add QLDBTreeToRecordNonUnicode
//
//
// $Revision: 1.2 $ ----------- 19 Oct 2001
// --RJ: unicode enabling
//
//
// $Revision: 1.1 $ ----------- 17 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   J:\DATA\EQFLDB.HV_   1.0   09 Jan 1996 09:46:26   BUILD  $
 *
 * $Log:   J:\DATA\EQFLDB.HV_  $
 *
 *    Rev 1.0   09 Jan 1996 09:46:26   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define QLDB_MAX_LEVELS         4     // maximum levels of a dictionary entry
#define QLDB_END_OF_TREE  QLDB_MAX_LEVELS+1 // end-of-tree constant

/**********************************************************************/
/*                        returncodes for QLDB                        */
/**********************************************************************/
#define QLDB_NO_ERROR        NO_ERROR           // OK
#define QLDB_NO_MEMORY       QLDB_ERR_BASE+1    // no memory left
#define QLDB_ERR_DEALLOC     QLDB_ERR_BASE+2    // error deallocating
                                                // memory
#define QLDB_NO_TREEHANDLE   QLDB_ERR_BASE+3    // no valid tree handle
#define QLDB_TOO_MANY_FIELDS QLDB_ERR_BASE+4    // too many fields
                                                // in a level node
#define QLDB_NO_VALID_DATA   QLDB_ERR_BASE+5    // no valid data supplied
#define QLDB_INVALID_LEVEL   QLDB_ERR_BASE+6    // invalid level number
                                                // entered
#define QLDB_NO_NODE         QLDB_ERR_BASE+7    // not a valid node
#define QLDB_INVALID_FIELDNO QLDB_ERR_BASE+8    // not a valid field number
#define QLDB_ERROR_IN_TREE   QLDB_ERR_BASE+9    // an unrecoverable error
                                                // is in the tree
#define QLDB_REC_TOO_LONG    QLDB_ERR_BASE+10   // the record is too long
                                                // to process

#define QLDB_START_CTRL_INFO sizeof(CHAR)+sizeof(USHORT)
                                           // no of bytes for ctrl info
                                           // at beginning of the record
// for UNICODE: QLDB_START_CTRL_INFO = # of char_w's in Ctrl-Info:
// actually it contains one char_w
// and a following ULONG; ULONG needs double space as USHORT
// so the total is duplicated in # of bytes needed!


#define QLDB_CTRL_INFO       sizeof(CHAR)+QLDB_START_CTRL_INFO
                                           // no of bytes for ctrl info



/**********************************************************************/
/*               definitions of the node relationsships               */
/**********************************************************************/
typedef enum
{
  QLDB_NEXT_NODE,
  QLDB_PREV_NODE,
  QLDB_NEXT_NODE_ON_SAME_LEVEL,
  QLDB_PREV_NODE_ON_SAME_LEVEL,
  QLDB_CHILD_NODE,
  QLDB_PARENT_NODE
} QLDB_RELATIONSSHIPS;                          // node relationsships

/**********************************************************************/
/*            definitions of the record control characters            */
/**********************************************************************/
typedef enum
{
  QLDB_FIELD_DELIMITER,
  QLDB_ESC_CHAR,
  QLDB_END_OF_REC,
  QLDB_FIRST_LEVEL,
  QLDB_SECOND_LEVEL,
  QLDB_THIRD_LEVEL,
  QLDB_FOURTH_LEVEL
} QLDB_LEVEL_NO;                                // Level numbers

__declspec(dllexport) USHORT QLDBAddSubtree( PVOID, USHORT, PSZ_W * );
__declspec(dllexport) USHORT QLDBCreateTree( USHORT [], PSZ_W *, PVOID * );
__declspec(dllexport) USHORT QLDBCurrNode( PVOID, PSZ_W *, USHORT * );
__declspec(dllexport) USHORT QLDBCurrTemplate( PVOID, PSZ_W * );
USHORT QLDBDestroySubtree( PVOID * );
__declspec(dllexport) USHORT QLDBDestroyTree( PVOID * );
USHORT QLDBFlattenTree( PVOID * );
USHORT QLDBJoinSameNodes( PVOID );
USHORT QLDBMoveToNode( PVOID, USHORT, PSZ_W *, USHORT * );
__declspec(dllexport) USHORT QLDBNextNode( PVOID, PSZ_W *, USHORT * );
__declspec(dllexport) USHORT QLDBNextTemplate( PVOID, PSZ_W *, USHORT * );
USHORT QLDBPrevNode( PVOID, PSZ_W *, USHORT * );
USHORT QLDBPrevTemplate( PVOID, PSZ_W *, USHORT * );
__declspec(dllexport) USHORT QLDBRecordToTree( USHORT [], PSZ_W, ULONG, PVOID * );
__declspec(dllexport) USHORT QLDBResetTreePositions( PVOID );
__declspec(dllexport) USHORT QLDBTreeToRecord( PVOID, PSZ_W *, ULONG * );
__declspec(dllexport) USHORT QLDBTreeToRecordNonUnicode( PVOID, PSZ *, ULONG * );
USHORT QLDBUpdateCurNodeData( PVOID, USHORT, PSZ_W, PSZ_W * );
__declspec(dllexport) USHORT QLDBFillFieldTables( PPROPDICTIONARY, PUSHORT, PUSHORT );
USHORT QLDBUpdateCurrTemplate( PVOID, PSZ_W * );
