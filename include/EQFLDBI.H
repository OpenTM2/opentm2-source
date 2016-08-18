//+----------------------------------------------------------------------------+
//|EQFLDBI.H                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|  This file contains the internal definitions for LDB processing            |
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
// -- RJ: move QLDB_START_CTRL_INFO to eqfldbi.h
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
 * $Header:   J:\DATA\EQFLDBI.HV_   1.0   09 Jan 1996 09:46:28   BUILD  $
 *
 * $Log:   J:\DATA\EQFLDBI.HV_  $
 *
 *    Rev 1.0   09 Jan 1996 09:46:28   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

/**********************************************************************/
/*                            definitions                             */
/**********************************************************************/
#define     MAX_LEVELS       4             // maximum no. of levels
#define QLDB_FIELD_SIZE      128           // no. of bytes per
                                           // preallocated field
#define QLDB_MAX_LEVELFIELDS 64000/(QLDB_MAX_LEVELS*QLDB_FIELD_SIZE)
                                           // maximum no. of fields per
                                           // level
#define QLDB_NO_STORAGE      0L            // no storage for the field
                                           // is computed
#define QLDB_EMPTY_STRING    ""            // an empty string
#define QLDB_EMPTY_STRINGW    L""            // an empty string
#define QLDB_NULC            '\0'          // the null character
#define QLDB_MAX_REC_LENGTH  32000         // maximum record length
#define QLDB_MAX_REC_LENGTHW 16000         // max.rec.len. in number of char_ws

/**********************************************************************/
/*                   field characteristic structure                   */
/**********************************************************************/
typedef struct _QLDB_FIELD
{
  BOOL          fDataInNode;                // Data in node or in
                                            // separate area
  PSZ_W         pszData;                    // Pointer to data area
} QLDB_FIELD, *PQLDB_FIELD;


/**********************************************************************/
/*                           node structure                           */
/**********************************************************************/
typedef struct _QLDB_NODE
{
  struct _QLDB_NODE *pParent;               // Pointer to parent node
  struct _QLDB_NODE *pLeft;                 // Pointer to left node
  struct _QLDB_NODE *pRight;                // Pointer to right node
  struct _QLDB_NODE *pChild;                // Pointer to child node
  USHORT            usLevel;                // Level number on which
                                            // node resides
  QLDB_FIELD        aFields[1];             // Array of field
                                            // characteristc for each
                                            // field
} QLDB_NODE, *PQLDB_NODE;


/**********************************************************************/
/*                       tree handle structure                        */
/**********************************************************************/
typedef struct _QLDB_HTREE
{
  BOOL          fWorkWithRecord;                // flag indicating
                                                // if data is in record
                                                // or in tree nodes
  PQLDB_NODE    pCurNode;                       // Pointer to current
                                                // active node
                                                // or record part
                                                // only valid if
                                                // fWorkWithRecord=FALSE
  PQLDB_NODE    apCurLevelNode[QLDB_MAX_LEVELS];// Array of pointers
                                                // to the current node
                                                // or record part
                                                // on each level
                                                // only valid if
                                                // fWorkWithRecord=FALSE
  PSZ_W           pszCurRec;                      // Pointer to current
                                                // active record part
                                                // only valid if
                                                // fWorkWithRecord=TRUE
  PSZ_W           apszCurLevelRec[QLDB_MAX_LEVELS]; // Array of pointers
                                                // to the record parts
                                                // on each level
                                                // only valid if
                                                // fWorkWithRecord=TRUE
  ULONG         aulStorageOnLevel[QLDB_MAX_LEVELS]; // No. of bytes to be
                                                // allocated for each
                                                // node on every level
  USHORT        ausNoOfFields[QLDB_MAX_LEVELS]; // No. of fields on
                                                // each level
  USHORT        usEscFields;                    // No. of fields containing
                                                // QLDB_ESC_CHAR
  USHORT        usBufferStorage;                // No. of bytes used for
                                                // the esc field buffer
  PSZ_W         pchrEscBuffer;                  // Buffer used to copy data
                                                // from fields containing
                                                // QLDB_ESC_CHAR
} QLDB_HTREE, *PQLDB_HTREE;


/**********************************************************************/
/*                        function prototypes                         */
/**********************************************************************/


USHORT QLDBCompare2Nodes( PQLDB_HTREE, PQLDB_NODE, PQLDB_NODE, BOOL * );
USHORT QLDBComputeFirstFieldOffset( PQLDB_HTREE, USHORT, USHORT * );
USHORT QLDBComputeSubtreeToDestroy( PQLDB_NODE, PQLDB_NODE * );
// QLDBConvertRecord is an old function...
USHORT QLDBConvertRecord( PCHAR, ULONG, USHORT [], PCHAR *, ULONG * );
USHORT QLDBCopyNode( PQLDB_HTREE, PQLDB_NODE, PQLDB_NODE * );
USHORT QLDBCopyNodeToRecord( PQLDB_HTREE, PSZ_W, USHORT * );
USHORT QLDBCreateNode( PQLDB_HTREE, USHORT, PSZ_W *, PQLDB_NODE * );
USHORT QLDBCreateTreeHandle( USHORT [], PQLDB_HTREE * );
USHORT QLDBDestroyIncompleteSubtree( PQLDB_HTREE, USHORT );
USHORT QLDBDestroyNode( PQLDB_HTREE, PQLDB_NODE * );
USHORT QLDBFillData( PQLDB_HTREE, PQLDB_NODE, USHORT, PSZ_W );
USHORT QLDBFilterEscChar( PQLDB_HTREE, USHORT *, PSZ_W *, PSZ_W * );
USHORT QLDBJoinSameSubtree( PQLDB_HTREE, PQLDB_NODE );
USHORT QLDBNextRecCtrlChar( PSZ_W * );
USHORT QLDBNextTreeTemplate( PQLDB_HTREE, USHORT, USHORT * );
USHORT QLDBNodeStorage( PQLDB_HTREE, USHORT, ULONG * );
USHORT QLDBNodeToRecStorage( PQLDB_HTREE, USHORT *, USHORT * );
USHORT QLDBPrevRecCtrlChar( PSZ_W * );
USHORT QLDBPrevTreeTemplate( PQLDB_HTREE, USHORT, USHORT * );
USHORT QLDBRemoveChildSubtree( PQLDB_HTREE, PQLDB_NODE );
USHORT QLDBUpdateFieldData( PQLDB_HTREE, PQLDB_NODE, USHORT, PSZ_W );


/**********************************************************************/
/*                               macros                               */
/**********************************************************************/

/**********************************************************************/
/*  this macro retrieves the level number from a pointer to a field   */
/*                 starting with a control character                  */
/**********************************************************************/
#define QLDB_LEVEL( pszRec ) ((USHORT)( *pszRec - QLDB_FIRST_LEVEL + 1 ))
/**********************************************************************/
/* this macro returns the number of fields of that node that contains */
/*                  the most fields in the template                   */
/**********************************************************************/
#define QLDB_MAX_NODE_FIELDS( phTree, pusFields )                      \
{                                                                      \
  *pusFields = 0;                                                      \
  for ( i = 0; i < QLDB_MAX_LEVELS; i++)                               \
  {                                                                    \
    if ( phTree->ausNoOfFields[i] > *pusFields )                       \
    {                                                                  \
      *pusFields = phTree->ausNoOfFields[i];                           \
    } /* endif */                                                      \
  } /* endfor */                                                       \
}

/**********************************************************************/
/*    this macro returns the total number of fields in a template     */
/**********************************************************************/
#define QLDB_TOTAL_FIELDS( phTree, pusFields )                         \
{                                                                      \
  *pusFields = 0;                                                      \
  for ( i = 0; i < QLDB_MAX_LEVELS; i++)                               \
  {                                                                    \
    *pusFields = *pusFields + phTree->ausNoOfFields[i];                            \
  } /* endfor */                                                       \
}

/**********************************************************************/
/* Defines for access to data in old LDB format                       */
/**********************************************************************/
#define FIRST_NODE_NUMBER       1
#define ABSOLUTE_MIN_NUM_NODES 10
#define NON_TERMINAL           FALSE
#define TERMINAL               TRUE
#define ENTRY_LEVEL             1
#define HW_LEVEL                2
#define POS_LEVEL               3
#define SENSE_LEVEL             4
#define TARGET_LEVEL            5
#define UNSPEC_TERM_NAME        "Terminal-Node"

#define SEG_SIZE                4

#define POINTER_BUFFER    0x0001
#define ON_DISPLAY        0x0002
#define MLE_DATA          0x0004
#define NO_DEALLOC        0x0008
#define NO_UPDATE         0x0010
#define READ_ONLY         0x0020

typedef struct    _OLDLDBNODE { struct _LDBNODE  *pstParent;
                          struct _LDBNODE  *pstSister;
                          struct _LDBNODE  *pstChild;
                                  BOOL   fTerminal;
                                  USHORT fsStatus;
                        } OLDLDBNODE;

typedef OLDLDBNODE *PSTOLDLDBNODE;

typedef PSTOLDLDBNODE HOLDLDBNODE;

typedef HOLDLDBNODE *PHOLDLDBNODE;

typedef struct _LDBSEG {           UCHAR ucbCode[2];
                                   USHORT usSister;
                          struct _LDBSEG *pstNextSeg;
                        } LDBSEG;

typedef LDBSEG *PSTLDBSEG;


typedef struct _LDBTEMP { PUCHAR pucLDBStruct;
                          PUCHAR pucLDBTrans;
                        } LDBTEMP;

typedef LDBTEMP *PSTLDBTEMP;


typedef struct  _LDBREC { PUCHAR pucLDBStruct;
                          PUCHAR pucLDBData;
                        } LDBREC;

typedef LDBREC *PSTLDBREC;


typedef struct     _LDB { PSTLDBTEMP pstLDBTemp;
                          PSTLDBREC  pstLDBRec;
                          USHORT     usNodesHW;
                          USHORT     usNodesPOS;
                          USHORT     usNodesSense;
                          USHORT     usNodesTarget;
                        } LDB;

typedef LDB *PSTLDB;

#define FIRST_NODE_NUMBER 1
#define NON_TERMINAL      FALSE
#define TERMINAL          TRUE


#define HW_FROM_ENTRY(hFirstNode) hFirstNode->pstChild
#define NEXT_POS(hPOSNode)        hPOSNode->pstSister
#define NEXT_SENSE(hSenseNode)    hSenseNode->pstSister
#define NEXT_TARGET(hTargetNode)  hTargetNode->pstSister
#define DATA_HANDLE(hNode)        (hNode->fTerminal ? NULL : (hNode->pstChild->fTerminal ? hNode->pstChild : NULL))
#define NEXT_DATA(hNode)          (hNode->fTerminal ? hNode->pstSister : NULL)
#define DATA(hNode)               ((hNode)->fTerminal ? (PUCHAR)((hNode)->pstChild) : NULL)
#define ISLASTNODE(hNode)         (hNode->pstSister == hNode->pstParent->pstSister ? TRUE : FALSE)
#define CODE_FROM_SEG(pucLDB, usNodeNr)  &pucLDB [4 * (usNodeNr - 1)]
#define INDEX_FROM_SEG(pucLDB, usNodeNr) (*((PUSHORT) &pucLDB [ (4 * (usNodeNr - 1)) + 2 ]))


//+----------------------------------------------------------------------------+
//|  Macro to get number of nodes of a signature record.                       |
//|  Input parameter is the pointer to the signature record                    |
//+----------------------------------------------------------------------------+
#define NUMOFNODES( pucSigRec ) ((PUSHORT)pucSigRec)[1]

//+----------------------------------------------------------------------------+
//|  Macro to get pointer to first code-name-to-field-name entry in a signature|
//|  record. Input parameter is the pointer to the signature record.           |
//+----------------------------------------------------------------------------+
#define FIRSTCODE2NAMEENTRY( pucSigRec ) \
   (PNAMEENTRY) (pucSigRec + (NUMOFNODES(pucSigRec) * SEG_SIZE))

//+----------------------------------------------------------------------------+
//|  Macro to get pointer to the next code-name-to-field-name entry in a       |
//|  signature record. Input parameter is the pointer to the current entry.    |
//+----------------------------------------------------------------------------+
#define NEXTCODE2NAMEENTRY( pCurEntry ) \
   (PNAMEENTRY) ( (PBYTE)pCurEntry + (sizeof(NAMEENTRY) + \
                  pCurEntry->ucNameLength - 1) )

//+----------------------------------------------------------------------------+
//|  Macro to convert a codename (e.g. 'AA') to an index value.                |
//|  Input parameter is the pointer to the codename value.                     |
//+----------------------------------------------------------------------------+
#define INDEXOFCODE( pCode ) \
   (USHORT)(((toupper(((PUCHAR)pCode)[0]) - 'A') * 26) + \
   (toupper(((PUCHAR)pCode)[1]) - 'A'))

//+----------------------------------------------------------------------------+
//|  Macro to check if a given codename is the codename of a data node         |
//|  Input parameter is the pointer to the codename value.                     |
//+----------------------------------------------------------------------------+
#define ISDATANODE( pCode ) \
   ((((PUCHAR)pCode)[0] >= 'A') && (((PUCHAR)pCode)[0] <= 'Z') &&   \
    (((PUCHAR)pCode)[1] >= 'A') && (((PUCHAR)pCode)[1] <= 'Z'))

//+----------------------------------------------------------------------------+
//|  Layout of codename<->systen name table in signature record                |
//+----------------------------------------------------------------------------+
typedef struct _NAMEENTRY
{
   USHORT  usCodeName;                 // code name
   UCHAR   ucNameLength;               // length of following name
   UCHAR   ucSysName[1];               // variable length system name field
} NAMEENTRY, *PNAMEENTRY;
