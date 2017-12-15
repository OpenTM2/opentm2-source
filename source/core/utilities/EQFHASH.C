//+----------------------------------------------------------------------------+
//|EQFHASH.C                       Hash and String Pool Functions              |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:    G. Queck    QSoft Quality Software Development GmbH              |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|  HashCreate              - Create a hash table                             |
//|  HashAdd                 - Add entry to hash table                         |
//|  HashDelete              - Delete a hash entry                             |
//|  HashReset               - Reset hash table                                |
//|  HashDestroy             - Destroy hash table                              |
//|  HashSearch              - Search for a hash element                       |
//|  HashIterate             - Iterate through hash table                      |
//|  PoolCreate              - Create a string pool                            |
//|  PoolAddString           - Add string to string pool                       |
//|  PoolAddData             - Add data area to string pool                    |
//|  PoolDestroy             - Destroy string pool                             |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|To be done / known limitations / caveats:                                   |
//+----------------------------------------------------------------------------+

#include "eqf.h"                  // General .H for EQF

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     HashCreate          Create a hash table                  |
//+----------------------------------------------------------------------------+
//|Description:       Create a new hash table                                  |
//+----------------------------------------------------------------------------+
//|Function call:     HashCreate( USHORT usElementSize, USHORT usHashSize,     |
//|                               PFN_HASHVALUE pfnHashValue,                  |
//|                               PFN_COMPARE pfnCompare );                    |
//+----------------------------------------------------------------------------+
//|Input parameter:   usElementSize  The size of the objects that will be      |
//|                                  stored in the hash table  (unused!)       |
//|                   usHashSize     The number of buckets                     |
//|                   pfnHashValue   The function that maps between an object  |
//|                                  and 0 .. usHashSize - 1.                  |
//|                                  This is used to decide which bucket to    |
//|                                  insert the element in to                  |
//|                   pfnCompare     Compares two objects, returning <0 if     |
//|                                  first is less than the second, >0 if      |
//|                                  first is greater than the second and      |
//|                                  ==0 if they are equal                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   PHASH                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL          create of hash table failed                |
//|                   !NULL         pointer to hash table                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:      none                                                     |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     check input parameters                                   |
//|                   allocate hash control block                              |
//|                   allocate hash element table                              |
//|                   clear all elements in table                              |
//|                   return pointer to hash control block                     |
//+----------------------------------------------------------------------------+
PHASH HashCreate
(
  USHORT        usElementSize,
  ULONG         ulHashSize,                                 /* No of buckets  */
  PFN_HASHVALUE pfnHashValue,
  PFN_HASHCOMP  pfnCompare,
  PFN_HASHFREE  pfnFree,
  PVOID         pUserPtr               // User pointer, is passed to all user functions
)
{
  PHASH pNewHash;
  ULONG ulI;

  if (ulHashSize==0 || usElementSize==0)                     /* Illegal parms  */
    return NULL;

  if (pfnHashValue==NULL || pfnCompare==NULL)    /* Ditto          */
    return NULL;

  /***************************************************************************/
  /* We need to claim space for the hash table and for usHashSize buckets to  */
  /* hold pointers to the lists of hash elements                             */
  /***************************************************************************/
  UtlAlloc( (PVOID *)&pNewHash, 0L, (LONG)sizeof(HASH), NOMSG );
  if (pNewHash!=NULL)                                      /* Out of memory ?*/
  {
    UtlAlloc( (PVOID *)&pNewHash->apHashElements, 0L, sizeof(PHASHLINK )*ulHashSize, NOMSG );
    if (pNewHash->apHashElements==NULL)
    {
      UtlAlloc( (PVOID *)&pNewHash, 0L, 0L, NOMSG );
      pNewHash=NULL;
    }
    else
    {
      pNewHash->usElementSize=usElementSize;
      pNewHash->ulHashSize=ulHashSize;
      pNewHash->pfnHashValue=pfnHashValue;
      pNewHash->pfnCompare=pfnCompare;
      pNewHash->pfnFree=pfnFree;
      pNewHash->pUserPtr = pUserPtr;

      /***********************************************************************/
      /* Set all buckets to empty                                            */
      /***********************************************************************/
      for ( ulI=0; ulI < ulHashSize; ++ulI )
      {
        pNewHash->apHashElements[ulI] = NULL;
      } /* endfor */
    }
  }
  return pNewHash;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     HashAdd                                                  |
//+----------------------------------------------------------------------------+
//|Description:       Add a new element to the hash table                      |
//+----------------------------------------------------------------------------+
//|Function call:     HashAdd( PHASH pHash, VOID pAddElement );                |
//+----------------------------------------------------------------------------+
//|Input parameter:   pHash    The table to add to                             |
//|                   pAddElement The pointer to the object.                   |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE      The object was added to the collection.        |
//|                   FALSE     The object was not added.  This may be         |
//|                                   because                                  |
//|                             a) There was not enough memory in the system   |
//|                                to save the object in the collection        |
//|                             b) No duplicates are allowed in the list.      |
//|                                Thus, FALSE is returned if the object is    |
//|                                already in the collection.                  |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pHash must have been created using HashCreate            |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:    calculate bucket for new item                             |
//|                  loop through linked list of bucket                        |
//|                    if found                                                |
//|                      return FALSE                                          |
//|                    elseif key is greater                                   |
//|                      leave loop                                            |
//|                    endif                                                   |
//|                  endloop                                                   |
//|                  if ok TBD                                                 |
//+----------------------------------------------------------------------------+
BOOL HashAdd
(
  PHASH pHash,
  PVOID pAddElement
)
{
  ULONG ulHashValue;                  /* Which bucket   */
  PHASHLINK    pElement;               /* Start of list  */
  PHASHLINK    *ppInsElement;          /* Where to insert*/
  int sCompResult;                     /* Add < Item     */

  /***************************************************************************/
  /* Calculate which bucket to access                                        */
  /* Round to the hash size                                                  */
  /***************************************************************************/
  ulHashValue = (*pHash->pfnHashValue)( pHash->ulHashSize, pAddElement); 
  ulHashValue = ulHashValue % pHash->ulHashSize;
  pElement    = pHash->apHashElements[ulHashValue];

  /***************************************************************************/
  /* Search the table looking for the element that is being added.           */
  /* This guarantees that all elements are unique                            */
  /***************************************************************************/
  while ( pElement != NULL )
  {
    /*************************************************************************/
    /* If the item is already in the structure, then do not add it.          */
    /*************************************************************************/
    sCompResult = (*pHash->pfnCompare)( pElement, pAddElement, pHash->pUserPtr );
    if (sCompResult==0)
      return FALSE;

    /*************************************************************************/
    /* If the list is sorted and the element compared is > the object we are */
    /* adding, we've gone past it in the list, so it won't be found.         */
    /*************************************************************************/
    else if (sCompResult > 0)
      break;
    pElement = pElement->pNext;
  }

  /***************************************************************************/
  /* We couldn't find it, so lets add it to the start of the list.           */
  /***************************************************************************/
  pElement = (PHASHLINK)pAddElement;

  /***************************************************************************/
  /* Assume that we're going to insert at the beginning of the list.         */
  /***************************************************************************/
  ppInsElement= &pHash->apHashElements[ulHashValue];

  /***************************************************************************/
  /* Set up the links so that the element is part of the list                */
  /*  and inserted in its place in the list so that all previous elements    */
  /*  are less than it.                                                      */
  /***************************************************************************/
  while ( *ppInsElement != NULL )
  {
    if ((*pHash->pfnCompare)( *ppInsElement, pAddElement, pHash->pUserPtr) > 0)
      break;
    ppInsElement= &(*ppInsElement)->pNext;
  }

  /***************************************************************************/
  /* Set up links                                                            */
  /* element is pointing to the object we want to insert                     */
  /* *ppInsElement is pointing at the link we need to update to position  */
  /*   the element in the right place.                                       */
  /***************************************************************************/
  pElement->pNext= *ppInsElement;
  *ppInsElement=pElement;
  return TRUE;
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     HashDelete          Delete a hash entry                  |
//+----------------------------------------------------------------------------+
//|Description:       Removes the requested element from the hash table and    |
//|                   calls the user-supplied free function to erase the       |
//|                   element.                                                 |
//+----------------------------------------------------------------------------+
//|Function call:     HashDelete( PHASH pHash, PVOID pDelElement );            |
//+----------------------------------------------------------------------------+
//|Input parameter:   PHASH    pHash       The table to delete from            |
//|                   PVOID    pDelElement The pointer to the object.          |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE       The object was deleted from the collection    |
//|                   FALSE      The object was not deleted. This is because   |
//|                              it could not be found.                        |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pHash must have been created using HashCreate            |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:    calculate bucket for delete item                          |
//|                  search list of bucket for delete item                     |
//|                  if found remove item from list and call                   |
//|                     user defined free routine for item                     |
//|                  return return code to caller                              |
//+----------------------------------------------------------------------------+
BOOL HashDelete
(
  PHASH         pHash,                 // pointer to hash data structure
  PVOID         pDelElement            // pointer to element to be deleted
)
{
  PHASHLINK  *ppElement;               // adress of current element pointer
  PHASHLINK  pNextElement;             // pointer to next element
  ULONG      ulHashValue;              // hash value for element
  SHORT      sCompResult;              // result of compare function

  /***************************************************************************/
  /* Calculate which bucket to access                                        */
  /* Round to the hash size                                                  */
  /***************************************************************************/
  ulHashValue  = (*pHash->pfnHashValue)( pHash->ulHashSize, pDelElement );
  ulHashValue  %= pHash->ulHashSize;
  ppElement    = &pHash->apHashElements[ulHashValue];

  /***************************************************************************/
  /* Search the linear list looking for the element that is being deleted.   */
  /* Since the elements are unique, we can stop when we find it.             */
  /***************************************************************************/
  while ( *ppElement != NULL )
  {
    /*************************************************************************/
    /* If the item is found, then delete it, rearranging the pointers        */
    /*************************************************************************/
    sCompResult= (*pHash->pfnCompare)( *ppElement, pDelElement,
                                                pHash->pUserPtr);
    if ( sCompResult==0)                              /* Match          */
    {
      pNextElement=(*ppElement)->pNext;                      /* Remove item    */
      (*pHash->pfnFree)( *ppElement, pHash->pUserPtr );
      *ppElement=pNextElement;                              /* Connect link   */
      return TRUE;                                         /* Found match    */
    }
    ppElement= &(*ppElement)->pNext;
  }
  /***************************************************************************/
  /* We could not find the element.                                          */
  /***************************************************************************/
  return FALSE;                                            /* No match       */
}


//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     HashReset           Reset hash table                     |
//+----------------------------------------------------------------------------+
//|Description:       Resets the hash table by removing all entries            |
//+----------------------------------------------------------------------------+
//|Function call:     HashReset( PHASH pHash );                                |
//+----------------------------------------------------------------------------+
//|Input parameter:   PHASH  pHash  table to remove all of the elements from   |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE   in any case                                       |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pHash must have been created using HashCreate            |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     loop over all buckets of hash table                      |
//|                     loop over all elements of current bucket               |
//|                       call user-define delete function                     |
//|                     endloop                                                |
//|                   endloop                                                  |
//+----------------------------------------------------------------------------+
BOOL HashReset
(
  PHASH pHash
)
{
  PHASHLINK element;
  PHASHLINK next_item;
  ULONG hash_index;

  /***************************************************************************/
  /* Iterate over all of the hash table lists and free the associated linked */
  /* list                                                                    */
  /***************************************************************************/
  for (hash_index=0;hash_index<pHash->ulHashSize;++hash_index)
  {
    element=pHash->apHashElements[hash_index];
    pHash->apHashElements[hash_index]=NULL;
    /*************************************************************************/
    /* Search the linear list looking for elements                           */
    /* When we find them, free up the memory associated the element.         */
    /* We do not need to relink the pointers in the list elements as we are  */
    /* going to delete the whole list.                                       */
    /*************************************************************************/
    while (element!=NULL)
    {
      next_item=element->pNext;
      (*pHash->pfnFree)( element, pHash->pUserPtr );
      element=next_item;
    }
  }
  return TRUE;
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     HashDestroy                                              |
//+----------------------------------------------------------------------------+
//|Description:       Destroy a hash table.                                    |
//+----------------------------------------------------------------------------+
//|Function call:     HashDestroy( PHASH pHash );                              |
//+----------------------------------------------------------------------------+
//|Input parameter:   PHASH pHash     The hash table to dispose of             |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pHash must have been created using HashCreate            |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     reset hash by calling HashReset                          |
//|                   free hash array                                          |
//|                   free hash control block                                  |
//+----------------------------------------------------------------------------+
VOID HashDestroy
(
  PHASH pHash
)
{
  HashReset(pHash);                    // remove all entries of the hash table

  UtlAlloc( (PVOID *)&pHash->apHashElements, 0L, 0L, NOMSG); // free hash array

  pHash->apHashElements = NULL;

  UtlAlloc( (PVOID *)&pHash,0L,0L,NOMSG);       // free hash control block

  return;
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     HashSearch          Search for a hash element            |
//+----------------------------------------------------------------------------+
//|Description:       Looks for a specific element and returns a pointer to it.|
//+----------------------------------------------------------------------------+
//|Function call:     HashSearch( PHASH pHash, PVOID pSearchElement );         |
//+----------------------------------------------------------------------------+
//|Input parameter:   PHASH  pHash          The hash table to search           |
//|                   PVOID  pSearchElement The pointer to the object to match.|
//+----------------------------------------------------------------------------+
//|Returncode type:   PVOID                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       !=NULL          The object that matched pSearchElement   |
//|                   NULL            No object matched pSearchElement         |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pHash must have been created using HashCreate            |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     get bucket for search element                            |
//|                   while not through list of bucket                         |
//|                     if current element matchs                              |
//|                       return pointer to current element                    |
//|                     endif                                                  |
//|                     go to next element in list                             |
//|                   endwhile                                                 |
//|                   return NULL                                              |
//+----------------------------------------------------------------------------+
PVOID HashSearch
(
  PHASH pHash,
  PVOID pSearchElement
)
{
  ULONG     ulHashValue;
  int       sCompResult;
  PHASHLINK *ppElement;

  /***************************************************************************/
  /* Go to the correct bucket as determined by the hash function             */
  /***************************************************************************/
  ulHashValue = (*pHash->pfnHashValue)( pHash->ulHashSize, pSearchElement) %
                pHash->ulHashSize;
  ppElement= &pHash->apHashElements[ulHashValue];

  /***************************************************************************/
  /* Search the chain looking for the element                                */
  /***************************************************************************/
  while ((*ppElement)!=NULL)
  {
    sCompResult=
      (*pHash->pfnCompare)( *ppElement, pSearchElement, pHash->pUserPtr );
    /*************************************************************************/
    /* If the item is in the structure, return it.                           */
    /*************************************************************************/
    if (sCompResult==0)
    {
      return (*ppElement);
    }

    /*************************************************************************/
    /* If the lists are sorted and we find an element that is > the search   */
    /* then we have failed as elements are arranged in increasing order      */
    /*************************************************************************/
    if (sCompResult > 0)
      return NULL;

    ppElement= &(*ppElement)->pNext;                   /* Next element  */
  }
  /***************************************************************************/
  /* We couldn't find it , so return NULL to show failure                    */
  /***************************************************************************/
  return NULL;
}

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     HashIterate                                              |
//+----------------------------------------------------------------------------+
//|Description:       Calls the supplied iterate function for all elements     |
//|                   in the hash table. The pointet ptr is passed through to  |
//|                   the iterate function.                                    |
//+----------------------------------------------------------------------------+
//|Function call:     HashIterate( PHASH pHash, PFN_HASHITERATE IterateFunc,   |
//|                                PVOID ptr );                                |
//+----------------------------------------------------------------------------+
//|Input parameter:   PHASH       pHash       The table to iterate over        |
//|                   PFN_HASHITERATE iterate_fn The function to call for each |
//|                                           object in the collection.        |
//|                   PVOID       ptr         The pointer to pass through to   |
//|                                           the iterate function.            |
//|                                           This allows additional info to   |
//|                                           be passed from the caller of the |
//|                                           HashIterate to iterate_fn        |
//+----------------------------------------------------------------------------+
//|Returncode type:   PVOID                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       !=NULL     The iterate_fn returned a non-zero value      |
//|                              for this object in the collection.            |
//|                   NULL       The iterate_fn was called for all members of  |
//|                              the collection, and it returned zero for      |
//|                              all of them                                   |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pHash must have been created using HashCreate            |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     for all elements in hash array do                        |
//|                     loop over all elements in current list                 |
//|                       call iterate function for element                    |
//|                     endloop                                                |
//|                   endfor                                                   |
//+----------------------------------------------------------------------------+
PVOID HashIterate
(
  PHASH       pHash,                   // The table to iterate over
  PFN_HASHITERATE pfnIterate,          // The function to call for each object
                                       //   in the collection.
  PVOID       ptr                      // The pointer to pass through to
                                       //   the iterate function.
)
{
  PHASHLINK pElement;                                 /*Iterated element*/
  PHASHLINK pNext;
  ULONG     ulIndex;

  for ( ulIndex = 0; ulIndex < pHash->ulHashSize; ++ulIndex )
  {
    pElement = pHash->apHashElements[ulIndex];               /* First element  */

    /*************************************************************************/
    /* Iterate over the list calling the function for each element           */
    /*************************************************************************/
    while ( pElement != NULL )
    {
      /***********************************************************************/
      /* Save the next pointer just in case the iterate function performs a  */
      /* delete operation which would really mess up the links.              */
      /* If the iterate function returns a non-zero value, then we must quit */
      /***********************************************************************/
      pNext = pElement->pNext;
      if ((*pfnIterate)( pElement, ptr ) != 0 )
      {
        return pElement;
      } /* endif */
      pElement = pNext;
    }
    /*************************************************************************/
    /* There were no elements in the list such that caused the walk function */
    /* to complete with a non-zero return code.  Thus return NULL            */
    /*************************************************************************/
  }
  return NULL;
} /* end of function HashIterate */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     PoolCreate                                               |
//+----------------------------------------------------------------------------+
//|Description:       Creates a pool which can be used to store strings.       |
//+----------------------------------------------------------------------------+
//|Function call:     PoolCreate( USHORT usPoolSize )                          |
//+----------------------------------------------------------------------------+
//|Input parameter:   USHORT usPoolSize    pool slot size                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   PPOOL                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL      pool could not be created due to memory        |
//|                             shortage                                       |
//|                   other     pointer to string pool                         |
//+----------------------------------------------------------------------------+
//|Samples:           PPOOL    pStringPool;                                    |
//|                                                                            |
//|                   pStringPool = PoolCreate( 16000 );                       |
//|                                                                            |
//|                   this example creates a pool with a initial size of       |
//|                   16000 bytes.                                             |
//+----------------------------------------------------------------------------+
//|Function flow:     allocate first pool                                      |
//|                   if ok then                                               |
//|                     set pool data                                          |
//|                   endif                                                    |
//|                   return pointer to allocated area or NULL if allocation   |
//|                     failed                                                 |
//+----------------------------------------------------------------------------+
PPOOL PoolCreate
(
  LONG    lPoolSize                 // pool slot size
)
{
  PPOOL     pPool = NULL;              // ptr to new pool area

  UtlAlloc( (PVOID *)&pPool, 0L, lPoolSize, ERROR_STORAGE );

  if ( pPool )
  {
    pPool->pNextPool = NULL;           // no follow-on pools yet
    pPool->lSize    = lPoolSize;
    pPool->lUsed    = sizeof(POOL);
  } /* endif */

  return( pPool );
} /* end of function PoolCreate */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     PoolAddString                                            |
//+----------------------------------------------------------------------------+
//|Description:       Adds a string to a string pool.                          |
//+----------------------------------------------------------------------------+
//|Function call:     PoolAddString( PPOOL pPool, PSZ pszString );             |
//+----------------------------------------------------------------------------+
//|Input parameter:   PPOOL   pPool      pointer to a pool created using       |
//|                                      PoolCreate                            |
//|                   PSZ     pszString  string which should be stored in the  |
//|                                      pool                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL     add to pool failed due to memory shortage       |
//|                   other    location of string in string pool               |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pPool must have been created using PoolCreate            |
//+----------------------------------------------------------------------------+
//|Side effects:      More pool areas are allocated if needed.                 |
//+----------------------------------------------------------------------------+
//|Samples:           pszLoc = PoolAddString( pPool, "the string in the pool" )|
//+----------------------------------------------------------------------------+
//|Function flow:     get length of string                                     |
//|                   add string to pool using PoolAddData                     |
//|                   return pointer to added string                           |
//+----------------------------------------------------------------------------+
PSZ PoolAddString
(
  PPOOL   pPool,                       // pointer to a pool created using
                                       // PoolCreate
  PSZ     pszString                    // string which should be stored in the
                                       // pool
)
{
  LONG    lStringLen;                 // length of string
  PSZ     pszStringInPool;             // ptr to location of string in pool

  lStringLen = strlen( pszString) + 1;// get length of string + delimiter

  pszStringInPool = (PSZ) PoolAddData( pPool, lStringLen, pszString );

  return( pszStringInPool );

} /* end of function PoolAddString */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     PoolAddString                                            |
//+----------------------------------------------------------------------------+
//|Description:       Adds a string to a string pool.                          |
//+----------------------------------------------------------------------------+
//|Function call:     PoolAddString( PPOOL pPool, PSZ pszString );             |
//+----------------------------------------------------------------------------+
//|Input parameter:   PPOOL   pPool      pointer to a pool created using       |
//|                                      PoolCreate                            |
//|                   PSZ_W   pszString  string which should be stored in the  |
//|                                      pool                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ_W                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL     add to pool failed due to memory shortage       |
//|                   other    location of string in string pool               |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pPool must have been created using PoolCreate            |
//+----------------------------------------------------------------------------+
//|Side effects:      More pool areas are allocated if needed.                 |
//+----------------------------------------------------------------------------+
//|Samples:           pszLoc = PoolAddString( pPool, "the string in the pool" )|
//+----------------------------------------------------------------------------+
//|Function flow:     get length of string                                     |
//|                   add string to pool using PoolAddData                     |
//|                   return pointer to added string                           |
//+----------------------------------------------------------------------------+
PSZ_W PoolAddStringW
(
  PPOOL   pPool,                       // pointer to a pool created using
                                       // PoolCreate
  PSZ_W   pszString                    // string which should be stored in the
                                       // pool
)
{
  LONG    lStringLen;                 // length of string
  PSZ_W   pszStringInPool;             // ptr to location of string in pool

  lStringLen = UTF16strlenCHAR( pszString) + 1;// get length of string + delimiter

  pszStringInPool = (PSZ_W)PoolAddData( pPool,
                                       (lStringLen * sizeof(CHAR_W)),
                                       pszString );

  return( pszStringInPool );

} /* end of function PoolAddStringW */




//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     PoolAddData                                              |
//+----------------------------------------------------------------------------+
//|Description:       Adds data to the string pool.                            |
//+----------------------------------------------------------------------------+
//|Function call:     PoolAddData( PPOOL pPool, USHORT usDataLen, PVOID pData);|
//+----------------------------------------------------------------------------+
//|Input parameter:   PPOOL   pPool      pointer to a pool created using       |
//|                                      PoolCreate                            |
//|                   USHORT  usDataLen  length of data                        |
//|                   PVOID   pData      data which should be stored in the    |
//|                                      pool                                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   PVOID                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL     add to pool failed due to memory shortage       |
//|                   other    location of data in string pool                 |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pPool must have been created using PoolCreate            |
//+----------------------------------------------------------------------------+
//|Side effects:      More pool areas are allocated if needed.                 |
//+----------------------------------------------------------------------------+
//|Samples:           pData = PoolAddData( pPool, 10, &MyData );               |
//+----------------------------------------------------------------------------+
//|Function flow:     locate a pool area large enough to contain the data      |
//|                   if no pool area was found then                           |
//|                     allocate and prepend a new pool area                   |
//|                   endif                                                    |
//|                   if no errors occured then                                |
//|                     store data in pool                                     |
//|                     adjust values in pool header                           |
//|                   endif                                                    |
//|                   return pointer to added data or NULL in case of errors   |
//+----------------------------------------------------------------------------+
PVOID PoolAddData
(
  PPOOL   pPool,                       // pointer to a pool created using
                                       // PoolCreate
  LONG    lDataLen,                   // length of data
  PVOID   pData                        // data which should be stored in the pool                                  |
)
{
  PPOOL   pAddPool;                    // ptr to string pool where string can
                                       // be stored
  PVOID   pDataInPool = NULL;          // ptr to location of data in pool

  /********************************************************************/
  /* Locate a pool area large enough to contain the data              */
  /********************************************************************/
  pAddPool = pPool;                    // start with first pool area
  while ( pAddPool && ((pAddPool->lSize - pAddPool->lUsed) < lDataLen) )
  {
    pAddPool = pAddPool->pNextPool;
  } /* endwhile */

  /********************************************************************/
  /* If no pool area was found, allocate a new pool area and link it  */
  /* into pool list.                                                  */
  /********************************************************************/
  if ( !pAddPool )
  {
    LONG lSize = max((LONG)(lDataLen + sizeof( POOL )), pPool->lSize );
    UtlAlloc( (PVOID *)&pAddPool, 0L, lSize, NOMSG );

    if ( pAddPool )
    {
      pAddPool->pNextPool = pPool->pNextPool;
      pAddPool->lSize    = lSize;
      pAddPool->lUsed    = sizeof(POOL);
      pPool->pNextPool    = pAddPool;
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* If OK add data to string pool                                    */
  /********************************************************************/
  if ( pAddPool )
  {
    pDataInPool = (PBYTE)pAddPool + pAddPool->lUsed;
    memcpy( pDataInPool, pData, lDataLen );
    pAddPool->lUsed = pAddPool->lUsed + lDataLen;
  } /* endif */

  /********************************************************************/
  /* Return pointer to data in pool or NULL in case of errors         */
  /********************************************************************/
  return( ( pAddPool ) ? pDataInPool : NULL );
} /* end of function PoolAddData */

//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     PoolDestroy                                              |
//+----------------------------------------------------------------------------+
//|Description:       Destroys a string pool created using PoolCreate          |
//+----------------------------------------------------------------------------+
//|Function call:     PoolDestroy( PPOOL pPool );                              |
//+----------------------------------------------------------------------------+
//|Input parameter:   PPOOL    pPool    pointer to an existing string pool     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      pPool must have been created using PoolCreate            |
//+----------------------------------------------------------------------------+
//|Side effects:      All pool areas linked to the string pool are freed.      |
//+----------------------------------------------------------------------------+
//|Function flow:     while pPool is set                                       |
//|                     free pool area                                         |
//|                     set pPool to next pool                                 |
//|                   endwhile                                                 |
//+----------------------------------------------------------------------------+
VOID PoolDestroy
(
  PPOOL    pPool                       // pointer to an existing string pool
)
{
  PPOOL    pNextPool;                  // pointer to next string pool

  while ( pPool )
  {
    pNextPool = pPool->pNextPool;      // get pointer to next pool area
    UtlAlloc( (PVOID *)&pPool, 0L, 0L, NOMSG ); // free current pool area
    pPool = pNextPool;                 // make next pool area the active one
  } /* endwhile */
} /* end of function PoolDestroy */

/**********************************************************************/
/* Simulated OS/2 Atom functions for the Windows environment          */
/*                                                                    */
/**********************************************************************/
/**********************************************************************/
/*  Prototypes for the user called by the low level hash routines     */
/**********************************************************************/
LONG   AtomHashCompare( PATOMENTRY pEntry, PATOMENTRY pEntry2, PVOID pUserPtr );
SHORT  AtomHashFree( PATOMENTRY pEntry, PVOID pUserPtr  );
ULONG AtomHashKeyValue( ULONG, PVOID );

HATOMTBL APIENTRY WinCreateAtomTable
(
  USHORT cbInitial,
  USHORT cBuckets
)
{
  BOOL             fOK = TRUE;         // internal O.K. flag
  HATOMTBL         pAtomTable = NULL;  // ptr to new atom table

  cbInitial;
 /********************************************************************/
 /* Allocate atom table control area                                 */
 /********************************************************************/
 if ( fOK )
 {
   fOK = UtlAlloc( (PVOID *)&pAtomTable, 0L,
                   max( MIN_ALLOC, (LONG)sizeof(ATOMTBL)), ERROR_STORAGE );
 } /* endif */

 /********************************************************************/
 /* Create low level hash                                            */
 /********************************************************************/
 if ( fOK  )
 {
   pAtomTable->pHash = (PHASH)HashCreate( sizeof(ATOMENTRY),
                                   (( cBuckets ) ? cBuckets : 1024),
                                   (PFN_HASHVALUE)AtomHashKeyValue,
                                   (PFN_HASHCOMP)AtomHashCompare,
                                   (PFN_HASHFREE)AtomHashFree,
                                   (PVOID)pAtomTable );
   fOK = pAtomTable->pHash != NULL;
 } /* endif */

 /********************************************************************/
 /* Cleanup in case of errors                                        */
 /********************************************************************/
 if ( !fOK )
 {
   if ( pAtomTable )
   {
     if ( pAtomTable->pHash )
     {
       HashDestroy( pAtomTable->pHash );
     } /* endif */
     UtlAlloc( (PVOID *)&pAtomTable, 0L, 0L, NOMSG );
     pAtomTable = NULL;
   } /* endif */
 } /* endif */

  return( pAtomTable );
} /* end of function WinCreateAtomTable */

VOID     APIENTRY WinDestroyAtomTable
(
  HATOMTBL hAtomTbl
)
{
   if ( hAtomTbl )
   {
     if ( hAtomTbl->pHash )
     {
       HashDestroy( hAtomTbl->pHash );
     } /* endif */
     UtlAlloc( (PVOID *)&hAtomTbl, 0L, 0L, NOMSG );
  } /* endif */
  return;
} /* end of function WinDestroyAtomTable */

EQF_ATOM APIENTRY WinAddAtom
(
  HATOMTBL hAtomTbl,
  PSZ pszAtomName
)
{
  PATOMENTRY       pEntry;             // ptr to atom table elements
  LONG             lNameLen;           // lenght of new entry

  /********************************************************************/
  /* build temporary atom table element in our atom table buffer      */
  /********************************************************************/
  lNameLen = min( strlen(pszAtomName) + 1,
                  sizeof(hAtomTbl->szBuffer) );
  strncpy( hAtomTbl->NewEntry.szAtomName, pszAtomName, lNameLen );

  /********************************************************************/
  /* Lookup new element in hash table                                 */
  /********************************************************************/
  pEntry = (PATOMENTRY) HashSearch( hAtomTbl->pHash, &(hAtomTbl->NewEntry) );

  /********************************************************************/
  /* Allocate buffer for element and add it to table if not in        */
  /* table yet                                                        */
  /********************************************************************/
  if ( pEntry == NULL )
  {
    UtlAlloc( (PVOID *) &pEntry, 0L,
              max( MIN_ALLOC, lNameLen + sizeof(ATOMENTRY) ),
              ERROR_STORAGE );
    if ( pEntry )
    {
      memcpy( pEntry, &(hAtomTbl->NewEntry), lNameLen + sizeof(ATOMENTRY) );
      HashAdd( hAtomTbl->pHash, pEntry );
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* indicate error condition                                       */
    /******************************************************************/
    pEntry = NULL;
  } /* endif */

  return( pEntry );
} /* end of function WinAddAtom */

EQF_ATOM     APIENTRY WinFindAtom
(
  HATOMTBL hAtomTbl,
  PSZ  pszAtomName
)
{
  PATOMENTRY       pEntry;             // ptr to atom table elements
  LONG             lNameLen;           // lenght of new entry

  /********************************************************************/
  /* build temporary atom table element in our atom table buffer      */
  /********************************************************************/
  lNameLen = min( strlen(pszAtomName) + 1,
                  sizeof(hAtomTbl->szBuffer) );
  strncpy( hAtomTbl->NewEntry.szAtomName, pszAtomName, lNameLen );

  /********************************************************************/
  /* Lookup element in hash table                                     */
  /********************************************************************/
  pEntry = (PATOMENTRY) HashSearch( hAtomTbl->pHash, &(hAtomTbl->NewEntry) );

  return( pEntry );
} /* end of function WinFindAtom */

EQF_ATOM     APIENTRY WinDeleteAtom
(
  HATOMTBL hAtomTbl,
  EQF_ATOM atom
)
{
  PATOMENTRY       pEntry;             // ptr to atom table elements

  pEntry = (PATOMENTRY)atom;

  /********************************************************************/
  /* Delete element in hash table                                     */
  /********************************************************************/
  HashDelete( hAtomTbl->pHash, pEntry );
  return( NULL );
} /* end of function WinDeleteAtom */

USHORT   APIENTRY WinQueryAtomName
(
  HATOMTBL hAtomTbl,
  EQF_ATOM atom,
  PSZ pchBuffer,
  USHORT cchBufferMax
)
{
  PATOMENTRY       pEntry;             // ptr to atom table elements
  hAtomTbl;
  if ( atom != 0L )
  {
    pEntry = (PATOMENTRY)atom;
    strncpy( pchBuffer, pEntry->szAtomName, cchBufferMax - 1 );
    pchBuffer[cchBufferMax-1] = EOS;
  } /* endif */
  return( 0 );
} /* end of function WinQueryAtomName */

/* Two random permutations of the numbers 0..255                             */
static BYTE random1[256] = {
149, 184,  28, 173, 213,  98,  14,  87, 157,  78,  31,  62, 226, 165, 120, 143,
251, 233, 227, 122, 146,  20, 106,  92, 197, 148, 102, 241,  29, 218,  69, 228,
153,  70, 249, 204, 214, 219, 234, 195, 152, 217,  65, 254,   8,  73, 119, 230,
206, 244, 246, 248, 216,  39, 128,  55,  75, 123, 189, 155,  38, 232,  99, 116,
118, 104, 160, 190, 215,  58,  54, 242,  72,  57,   7, 202, 192,  12,  96,  43,
 95, 177, 193,  51, 127, 247,  19,  40,  34, 147, 212,  42, 203, 239, 112,  93,
208, 201, 109,  90, 176,  59, 223,  94, 133, 111, 130,   5,  67, 222,  22, 209,
131, 169,  15, 174,  79,   6,  49,  46,   3, 142,  63, 107, 154, 182, 224, 186,
168,  60, 181, 125, 207,  52, 166,  45,  41, 188, 164, 240, 185, 161, 200,  97,
255, 250,   2,  89, 158,  30,  53,  21,  44, 235,  23, 187, 198, 205,  88, 121,
162,  11,  25, 170,  50,   4, 211, 243,   0,  37,  24, 196, 144, 129,  91,  86,
 76,  26,  18, 220, 237, 210, 194, 136, 156, 100,  27, 229, 137, 101, 236, 191,
221, 159,  35, 126,  68,  71,  10, 183, 179, 110, 150,  36, 151,  85,  82, 114,
  9,  81, 124, 172,  66,   1,  83,  77, 252, 105, 145,  13,  64, 238, 163, 140,
180, 113, 178, 108,  80,  56, 225, 134, 245,  32, 103,  74,  33,  61, 199, 117,
253, 231,  48,  47,  16, 141, 115, 167, 138,  84, 135, 139,  17, 132, 175, 171
};

static BYTE random2[256] = {
247,  72, 239, 215,  34, 179,  66,  63, 229, 107, 127, 226,  96, 105, 110, 197,
 27, 241, 251,   7, 146,  44, 230, 238, 153,  86, 104, 155, 163, 130,  55, 195,
240, 149,  73,  85, 196,   8,  98, 253, 208, 212, 102,  53, 165, 124, 128,  70,
174, 111, 151,  10, 114, 125, 220, 211,  93, 106,  54,  30, 156, 250, 122,  41,
252, 170, 232, 100,  28,   6,  81, 200, 168,  91, 249,  90,  84, 219, 244,  19,
193,  38,  18,  32,  79,  71, 227,  83,  47, 213,  75,  15,  22,  21,  45, 236,
 43, 180,  76, 248,  24, 234, 205,  35,  77, 184, 235, 202,  57,  69,  29, 203,
109, 167,  89, 135, 190, 123, 141, 217, 164,  94, 255,  65,  92, 150,   3, 152,
192, 112, 218,  39,  61,  74, 214, 204, 148,  62, 103, 198, 187, 172, 166,   4,
 16,  88,  46, 224,  56,  87,  36,  99,  95, 126, 185, 201,  50,  20, 160,  67,
242, 245, 133, 101, 158, 145, 154,  78, 136, 132,  49, 225,  80, 188,  48, 237,
 40, 228,  11, 143, 231, 221,  26, 194,   0, 243, 129, 171, 108, 161,  12, 209,
 25, 182, 137,  33, 189,  97, 254,  68, 162, 176, 113,  58, 147, 246,  31,  51,
121,  23, 186,  13, 178,  64,  14,  37, 216, 142,   1, 223, 183,   5,  59,  42,
119,  17, 144, 199,  52, 131, 157, 134, 207, 117, 120,   9, 222, 233, 210, 191,
159, 116, 181, 169, 206, 175,  60, 140, 139, 173, 118, 177, 115, 138,   2,  82
};

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AtomHashKeyValue   Get hash value for an entry           |
//+----------------------------------------------------------------------------+
//|Description:       Computes the hash value of an entry by 'OR'ing the       |
//|                   bytes which make up the term of the entry. The bytes     |
//|                   are converted using two random tables.                   |
//|                   The returned value will be in the range from             |
//|                   0 to 65535.                                              |
//+----------------------------------------------------------------------------+
//|Function call:     AtomHashKeyValue( lSize, pEntry );                      |
//+----------------------------------------------------------------------------+
//|Input parameter:   ULONG     ulSize        size of hash table               |
//|                   PATOMENTRY pEntry       point to hash entry              |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       any          hash value for entry                        |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     while not end of term                                    |
//|                     exor table1 value of character to hash1 value          |
//|                     exor table2 value of character to hash2 value          |
//|                     go to next character                                   |
//|                   endwhile                                                 |
//|                   return ushort from hash1 and hash2 value                 |
//+----------------------------------------------------------------------------+
ULONG AtomHashKeyValue
(
  ULONG       ulSize,                   // size of hash table
  PVOID       pvEntry                    // point to hash entry
)
{
  PATOMENTRY pEntry = (PATOMENTRY)pvEntry;
  PBYTE pbTerm = (PBYTE)pEntry->szAtomName;
  ULONG hash1 = 0;
  ULONG hash2 = 0;

  ulSize;                              // avoid compiler warning

  while ( *pbTerm )
  {
    hash1 = random1[hash1 ^ *pbTerm ];
    hash2 = random2[hash2 ^ *pbTerm++];
  }
  return (( hash1 << 8) | hash2);
} /* end of function AtomHashKeyValue */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AtomHashCompare    Compare two hash entries              |
//+----------------------------------------------------------------------------+
//|Description:       Compare the terms of two hashentries and return the      |
//|                   result.                                                  |
//|                   This function is used by the low level hash functions to |
//|                   compare two has entries.                                 |
//+----------------------------------------------------------------------------+
//|Function call:     AtomHashCompare( PATOMENTRY pEntry1, PATOMENTRY pEntry2) |
//+----------------------------------------------------------------------------+
//|Input parameter:   PATOMENTRY  pEntry1     pointer to first hash entry      |
//|                   PATOMENTRY  pEntry2     pointer to second hash entry     |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       -1   if term in pEntry1 is smaller than term in pEntry2  |
//|                    0   if term in pEntry1 is equal to term in pEntry2      |
//|                    1   if term in pEntry1 is greater than term in pEntry2  |
//+----------------------------------------------------------------------------+
//|Prerequesits:      none                                                     |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     return result of strcmp function                         |
//+----------------------------------------------------------------------------+
LONG AtomHashCompare
(
  PATOMENTRY pEntry1,
  PATOMENTRY pEntry2,
  PVOID      pUserPtr
)
{
  pUserPtr;                            // avoid compiler warning

  return strcmp( pEntry1->szAtomName, pEntry2->szAtomName );
} /* end of function AtomHashCompare */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AtomHashFree       Free a hash entry                     |
//+----------------------------------------------------------------------------+
//|Description:       Is called by the low level hash functions to free        |
//|                   a hash element.                                          |
//+----------------------------------------------------------------------------+
//|Function call:     AtomHashFree( PATOMENTRY pEntry );                       |
//+----------------------------------------------------------------------------+
//|Input parameter:   PATOMENTRY pEntry     pointer to entry to be freed       |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE    in any case                                      |
//+----------------------------------------------------------------------------+
//|Prerequesits:      none                                                     |
//+----------------------------------------------------------------------------+
//|Side effects:      none                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     free memory of hash entry                                |
//|                   return TRUE                                              |
//+----------------------------------------------------------------------------+
SHORT AtomHashFree( PATOMENTRY pEntry, PVOID pUserPtr )
{
  pUserPtr;
  UtlAlloc( (PVOID *) &pEntry, 0L, 0L, NOMSG );

  return( TRUE );
} /* end of function AtomHashFree */


EQF_ATOM     APIENTRY WinFindAtomW
(
  HATOMTBL hAtomTbl,
  PSZ_W    pszAtomNameW,
  ULONG    ulCP
)
{
  LONG            lI;
  LONG            lLen;
  PSZ              pszAtomName;
  PATOMENTRY       pEntry;             // ptr to atom table elements

  lI = UTF16strlenCHAR(pszAtomNameW);
  lLen = max( (lI+1) * sizeof(CHAR_W), MIN_ALLOC ); // to be sure if all is DBCS!
  if ( UtlAlloc( (PVOID *) &pszAtomName, 0L,
                    lLen, ERROR_STORAGE ) )
  {
     Unicode2ASCII( pszAtomNameW, pszAtomName, ulCP );
  }

  pEntry = (PATOMENTRY) WinFindAtom(hAtomTbl, pszAtomName);
  UtlAlloc((PVOID *) &pszAtomName, 0L, 0L, NOMSG);
  return( pEntry );
} /* end of function WinFindAtom */


EQF_ATOM APIENTRY WinAddAtomW
(
  HATOMTBL hAtomTbl,
  PSZ_W    pszAtomNameW,
  ULONG    ulCP
)
{
   LONG            lI;
   LONG            lLen;
   PSZ              pszAtomName;
   PATOMENTRY       pEntry;             // ptr to atom table elements

   lI = UTF16strlenCHAR(pszAtomNameW);
   lLen = max( (lI+1) * sizeof(CHAR_W), MIN_ALLOC ); // to be sure if all is DBCS!
   if ( UtlAlloc( (PVOID *) &pszAtomName, 0L,
                      lLen, ERROR_STORAGE ) )
   {
      Unicode2ASCII( pszAtomNameW, pszAtomName, ulCP );
   }

   pEntry = (PATOMENTRY) WinAddAtom(hAtomTbl, pszAtomName);

   UtlAlloc((PVOID *) &pszAtomName, 0L, 0L, NOMSG);

  return( pEntry );
} /* end of function WinAddAtom */

//USHORT   APIENTRY WinQueryAtomNameW
//(
//  HATOMTBL hAtomTbl,
//  EQF_ATOM atom,
//  PSZ_W    pchBufferW,
//  USHORT   cchBufferMax
//)
//{
//  PSZ    pszBuffer;
//
//  if (UtlAlloc( (PVOID *) &pszBuffer, 0L, (LONG) cchBufferMax, ERROR_STORAGE ))
//  {
//    WinQueryAtomName(hAtomTbl, atom, pszBuffer, cchBufferMax);
//
//    ASCII2Unicode(pszBuffer, pchBufferW);
//  }
//
//  return( 0 );
//} /* end of function WinQueryAtomNameW */
