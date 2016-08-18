//+----------------------------------------------------------------------------+
//|  EQFTALP1.C -List processing unbalanced binary tree support                |
//+----------------------------------------------------------------------------+
//|  Copyright Notice:                                                         |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|  Author       : G. Queck                                                   |
//+----------------------------------------------------------------------------+
//|  Description  :                                                            |
//|                                                                            |
//|      These functions are coded in such a way that application details      |
//|      reside in the application specific code: structure of the node,       |
//|      allocation of it, comparing two nodes, freeing a node, process a      |
//|      node during the scan of the list; while the tree specific function    |
//|      reside in a general purpose module that contains function to search,  |
//|      insert a node ans initialize, terminate ans scan the tree.            |
//+----------------------------------------------------------------------------+
//|  Entry Points : ListAlloc                                                  |
//|                 ListFree                                                   |
//|                 ListInsert                                                 |
//|                 ListSearch                                                 |
//|                 ListScan                                                   |
//+----------------------------------------------------------------------------+
//|  Externals    : UtlAlloc                                                   |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|  Internals    : UNBTreeInit                                                |
//|                 UNBTreeFree                                                |
//|                 UNBNodeInsert                                              |
//|                 UNBNodeFind                                                |
//|                 UNBTreeIterate                                             |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|  Macros       : NODE_TO_LINK                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|  Include files: eqf.h                                                      |
//|                 eqflpunb.h                                                 |
//|                                                                            |
//+----------------------------------------------------------------------------+

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TP               // public translation processor functions
#include <eqf.h>                  // General Translation Manager include file
#include "EQFTAI.H"               // Analysis private include file


/*---------------------------------------------------------------------------*/
/* UNB internal functions                                                    */
/*---------------------------------------------------------------------------*/
static   VOID       UNBTreeInit(PUNB_TREE , USHORT, PFN_CMP, PFN_FREE);
static   BOOL       UNBTreeFree(PUNB_TREE , PTANODE);
static   BOOL       UNBNodeInsert(PUNB_TREE , PTANODE *, PTANODE);
static   PTANODE      UNBNodeFind(PUNB_TREE , PTANODE);
static   PTANODE      UNBTreeIterate(PUNB_TREE , PTANODE, PFN_ITERATE, VOID *);

/*---------------------------------------------------------------------------*/
/* Allocate the control structure for the tree                               */
/*                                                                           */
/* return: PTANODE = pointer to the control structure                          */
/*         NULL  = errors have occured                                       */
/*---------------------------------------------------------------------------*/
PUNB_TREE   ListAlloc(
                     USHORT    usOffset,   // offset of link in node
                     PFN_CMP   cmp_fcn,    // compare two nodes
                     PFN_FREE  free_fcn    // call when node removed
                     )
   {
         PUNB_TREE   pTree;                          // descriptor of tree


   UtlAlloc( (PVOID *) &pTree, 0L, (ULONG)sizeof(UNB_TREE), ERROR_STORAGE);

   if (pTree != NULL)
       UNBTreeInit(pTree, usOffset, cmp_fcn, free_fcn);

   return(pTree);
   }

/*---------------------------------------------------------------------------*/
/* Internal function to initialize the tree.                                 */
/*                                                                           */
/* Set the address of the comparison and free node function.                 */
/* Set the offset of link pointer from the beginning of node.                */
/*                                                                           */
/* return: NONE                                                              */
/*---------------------------------------------------------------------------*/
static   VOID       UNBTreeInit(
                               PUNB_TREE  pTree,    // descriptor of tree
                               USHORT     usOffset, // offset of link in node
                               PFN_CMP    cmp_fcn,  // compare two nodes
                               PFN_FREE   free_fcn  // call when node removed
                               )
    {
    pTree->root = NULL;
    pTree->link_offset = usOffset;         // offset of the beginning node
    pTree->cmp_fcn = cmp_fcn;            // address of comparison function
    pTree->free_fcn = free_fcn;          // address of free nod function
    pTree->ulCount = 0;

    return;
    }

/*---------------------------------------------------------------------------*/
/* Scan a complete tree in memory, for every element call the low-level      */
/* function->UNBTreeIterate().                                               */
/*                                                                           */
/* return: TRUE  - if successful                                             */
/*         FALSE - if interrupted                                            */
/*---------------------------------------------------------------------------*/
BOOL       ListScan(
                   PUNB_TREE   pTree,    /* descriptor of tree      */
                   PTANODE       pNode,    /* current node            */
                   PFN_ITERATE visit_fcn,/* called at each node     */
                   PVOID       ptr       /* 2nd arg of visit_fcn    */
                   )
   {
   PTANODE      pReturnNode;

   pReturnNode =  UNBTreeIterate(pTree, pNode, visit_fcn, ptr);

   return (pReturnNode == NULL);
   }

/*---------------------------------------------------------------------------*/
/* Internal routine to iterate tree.  If the visit function return a         */
/* non-zero value, the iteration stops and returns the node.                 */
/* Passed as a parameter giving top it the address of current node and the   */
/* pointer to the passed user structure(that will contains the application   */
/* data).                                                                    */
/*                                                                           */
/* return: PTANODE - pointer to node                                           */
/*         NULL  - if no node found                                          */
/*---------------------------------------------------------------------------*/
static   PTANODE     UNBTreeIterate(
                                 PUNB_TREE   pTree,     // descriptor of tree
                                 PTANODE       pNode,     // current node
                                 PFN_ITERATE visit_fcn, // called at each node
                                 VOID       *ptr        // 2nd arg of visit_fcn
                                 )

{
   PTANODE      pCurrent = pNode;            // Should be the root initially
   PUNB_LINK  pLink = NODE_TO_LINK(pTree, pCurrent);
   PTANODE      pLastVisited = pNode;
   BOOL       fOk = TRUE;


   while (pCurrent != NULL)         // No nodes in tree or gone to parent
      {                             //  from the root node.
      if ((pLink->p[LEFT] != NULL)          &&
          (pLink->p[LEFT] != pLastVisited)  &&
          (pLink->p[RIGHT] != pLastVisited))
         {
         while (pLink->p[LEFT] != NULL)        // Follow left as far as possible
            {
            pCurrent = (PTANODE) pLink->p[LEFT];
            pLink = NODE_TO_LINK(pTree, pCurrent); // Must update link every
            }                                    //  time pCurrent is assigned
         }

      if (pLastVisited != pLink->p[RIGHT])
         {                         // Process this node
         fOk = (*visit_fcn)(pCurrent, ptr);
         if (!fOk)                 // Abort loop if error in visit function
           break;
         }

      if ((pLink->p[RIGHT] != NULL)  &&
          (pLink->p[RIGHT] != pLastVisited))
         {                         // Go right one node
         pCurrent = (PTANODE) pLink->p[RIGHT];
         pLink = NODE_TO_LINK(pTree, pCurrent);
         }
      else
         {                         // Go to parent (& update pLastVisited)
         pLastVisited = pCurrent;
         pCurrent = (PTANODE) pLink->p[PARENT];
         pLink = NODE_TO_LINK(pTree, pCurrent);
         }
      }

   return(pCurrent);
}


/*---------------------------------------------------------------------------*/
/*  Terminate operation on a list and free all the allocated memory.         */
/*                                                                           */
/* return: TRUE  - if successful                                             */
/*         NULL  - if interrupted                                            */
/*---------------------------------------------------------------------------*/
BOOL       ListFree(
                   PUNB_TREE  pTree       /* descriptor of tree      */
                   )
    {
    BOOL       fOk = TRUE;

    if (pTree->free_fcn != NULL)
        fOk = UNBTreeFree(pTree, pTree->root);

    UtlAlloc( (PVOID *) &pTree, 0L, 0L, NOMSG);

    return(fOk);
    }

/*---------------------------------------------------------------------------*/
/* Internal routine to free the tree                                         */
/*                                                                           */
/* return: TRUE  - if successful                                             */
/*         NULL  - if interrupted                                            */
/*---------------------------------------------------------------------------*/
static   BOOL       UNBTreeFree(
                                PUNB_TREE  pTree, /* descriptor of tree      */
                                PTANODE      pNode  /* current subtree         */
                               )
{
   PTANODE      pCurrent = pNode;         // Should be the root initially
   PUNB_LINK  pLink = NODE_TO_LINK(pTree, pCurrent);
   PTANODE      pParent;
   BOOL       fOk = TRUE;

   while (pCurrent != NULL)
      {
      while ((pLink->p[LEFT] != NULL) || (pLink->p[RIGHT] != NULL))
         {                     // Current node is not a leaf
         if (pLink->p[LEFT] != NULL)
            {
            pCurrent = (PTANODE) pLink->p[LEFT];
            pLink = NODE_TO_LINK(pTree, pCurrent);
            }

         if (pLink->p[RIGHT] != NULL)
            {
            pCurrent = (PTANODE) pLink->p[RIGHT];
            pLink = NODE_TO_LINK(pTree, pCurrent);
            }
         }

      //
      // Current node is a leaf now
      //

      pParent = (PTANODE) pLink->p[PARENT];       // Save a pointer to the parent

      if (pParent != NULL)             // Detach the current node from tree
         {
         pLink = NODE_TO_LINK(pTree, pParent);   // Get the Parents link record

         if (pLink->p[LEFT] == pCurrent)
           pLink->p[LEFT] = NULL;       // Unlink left
         else
           pLink->p[RIGHT] = NULL;      // Unlink right
         }

      fOk = (*pTree->free_fcn)(pCurrent) && fOk;  // Free the current node

      pCurrent = pParent;             // Go back to the parent
      pLink = NODE_TO_LINK(pTree, pCurrent);
      }

   return(fOk);
}

/*---------------------------------------------------------------------------*/
/* Insert a node in the list. The node must be already allocated and filled  */
/* with values.                                                              */
/*                                                                           */
/* return: TRUE  - if successful                                             */
/*         FALSE - if error occured                                          */
/*---------------------------------------------------------------------------*/
BOOL       ListInsert(
                     PUNB_TREE pTree,         /* descriptor of tree      */
                     PTANODE    *ppSubtreeRoot, /* root of tree            */
                     PTANODE     pNewNode       /* node to be inserted     */
                     )
   {
   BOOL       fOk;

   fOk = UNBNodeInsert(pTree, ppSubtreeRoot, pNewNode);

   if (fOk)
      {
      pTree->ulCount++;
      }

   return(fOk);
   }

/*---------------------------------------------------------------------------*/
/* Internal routine to insert a node in a tree                               */
/*                                                                           */
/* return: TRUE  = OK                                                        */
/*         FALSE = node already in tree                                      */
/*---------------------------------------------------------------------------*/
static   BOOL       UNBNodeInsert(
                                 PUNB_TREE  pTree,        // descriptor of tree
                                 PTANODE     *ppSubtreeRoot,// root of tree
                                 PTANODE      pNewNode      // node to be inserted
                                 )
   {
   PTANODE     *pChild;
   PTANODE      pParent = NULL;
   SHORT      usCompareRC;                   // must be signed
   PUNB_LINK  pNewLink;
   BOOL       fOk = TRUE;
   BOOL       fLoopOK = TRUE;

   pNewLink = NODE_TO_LINK(pTree, pNewNode);       /* link field of a node    */
   pChild = ppSubtreeRoot;

   while(fLoopOK)
      {
      if (*pChild == NULL)           // Insert new node here
         {
         pNewLink->p[LEFT] = pNewLink->p[RIGHT] = NULL;
         pNewLink->p[PARENT] = pParent; // Ptr to parent (NULL for first node)
         *pChild = pNewNode;
         break;
         }
      else
         {
         usCompareRC = (*pTree->cmp_fcn)(*pChild, pNewNode);

         pParent = *pChild;         // Set parent before going down tree

         if (usCompareRC > 0)           // add node to left subtree
            {
            pChild = (PTANODE *) &(NODE_TO_LINK(pTree, *pChild)->p[LEFT]);
            }
         else if (usCompareRC < 0)      // add node to right subtree
            {
            pChild = (PTANODE *) &(NODE_TO_LINK(pTree, *pChild)->p[RIGHT]);
            }
         else                          // node already in tree
            {
            fOk = FALSE;
            break;
            }
         }
      }

   return(fOk);
   }

/*---------------------------------------------------------------------------*/
/* Retrieve a node with the same key in the tree as the passed node          */
/*                                                                           */
/* return: PTANODE = pointer to found node                                     */
/*         NULL  = if node not found                                         */
/*---------------------------------------------------------------------------*/
PTANODE      ListSearch(
                     PUNB_TREE pTree,      /* descriptor of tree    */
                     PTANODE     pSearchNode /* value to find         */
                     )
   {
   PTANODE      pCurrentNode;

   pCurrentNode = UNBNodeFind(pTree, pSearchNode);

   return(pCurrentNode);
   }

/*---------------------------------------------------------------------------*/
/* Internal routine to find a node in the tree                               */
/*                                                                           */
/* return: PTANODE = pointer to found node                                     */
/*         NULL  = if node not found                                         */
/*---------------------------------------------------------------------------*/
static   PTANODE      UNBNodeFind(
                               PUNB_TREE  pTree,       // descriptor of tree
                               PTANODE      pSearchNode  // value to find
                               )
   {
   PTANODE      pCurrentNode;
   SHORT      usCompareRC;

   pCurrentNode = pTree->root;

   while (pCurrentNode != NULL)
      {
      usCompareRC = (*pTree->cmp_fcn)(pCurrentNode, pSearchNode);

      if (usCompareRC > 0)
         {
         pCurrentNode = (PTANODE) NODE_TO_LINK(pTree, pCurrentNode)->p[LEFT];
         }
      else if (usCompareRC < 0)
         {
         pCurrentNode = (PTANODE) NODE_TO_LINK(pTree, pCurrentNode)->p[RIGHT];
         }
      else
         break;
      }

   return(pCurrentNode);
   }


