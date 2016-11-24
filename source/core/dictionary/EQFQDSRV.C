/*! \file
	Description:  Main functions for the QDAM code

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)
#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#include <eqf.h>                  // General Translation Manager include file

//#include <EQFTM.H>              // Translation Memory public include file
#include <EQFQDAMI.h>             // internal include file for QDAM
#include <EQFSETUP.H>             // for path constants

static VOID DeleteTempFiles( PSZ, CHAR );
/**********************************************************************/
/* global struct containing struct. for open dicts ...                */
/**********************************************************************/
QDAMDICT QDAMDict[MAX_NUM_DICTS];

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictOpen   Open Dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDictOpen( PSZ, PSZ, SHORT, BOOL, PBTREE * );
//
//------------------------------------------------------------------------------
// Description:       Open a file for processing
//
//------------------------------------------------------------------------------
// Parameters:        PSZ              name of the index file
//                    PSZ              name of the server
//                    SHORT            number of bytes per record
//                    BOOL             TRUE  read/write FALSE  read/only
//                    PPBTREE          pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_NO_ROOM     memory shortage
//                    BTREE_OPEN_ERROR  dictionary already exists
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_ILLEGAL_FILE not a valid dictionary
//                    BTREE_CORRUPTED   dictionary is corrupted
//------------------------------------------------------------------------------
// Function flow:     Allocate memory for basic structure
//                    if ok then
//                      determine type of operation (Local/Remote) and call
//                      appropriate functions
//                    endif
//                    if open error then
//                      free allocated memory
//                    endif
//                    set base pointer to tree structure
//                    return return code
//------------------------------------------------------------------------------
SHORT  QDAMDictOpen
(
  PSZ   pName,                         // name of the file
  PSZ   pServer,                       // name of the server
  SHORT sNumberOfBuffers,              // number of buffers
  USHORT usOpenFlags,                  // Read Only or Read/Write
  PBTREE * ppBTIda                     // pointer to BTREE structure
)
{
   PBTREE    pBTIda;                   // pointer to BTRee structure
   SHORT     sRc = 0;                  // return code

   if ( ! UtlAlloc( (PVOID *)&pBTIda, 0L , (LONG) sizeof( BTREE ), NOMSG )  )
   {
      sRc = BTREE_NO_ROOM;
   }
   else
   {
     if ( !pServer )
     {
       pServer = EMPTY_STRING;
     } /* endif */
     /*****************************************************************/
     /* check if same dictionary is already open else return index    */
     /* of next free slot...                                          */
     /*****************************************************************/
     sRc = QDAMCheckDict( pName, pBTIda );
     if ( !sRc )
     {
       /***************************************************************/
       /* check if dictionary is locked                               */
       /***************************************************************/
       if ( ! pBTIda->usDictNum )
       {
         if ( *pServer  )
         {
           //sRc = QDAMDictOpenRemote ( &pBTIda, pName, pServer,
           //                           usOpenFlags, sNumberOfBuffers );
         }
         else
         {
           sRc = QDAMDictOpenLocal( pName, sNumberOfBuffers,
                                    usOpenFlags, &pBTIda );
         } /* endif */
       } /* endif */
      /****************************************************************/
      /* fill in relevant data for this open dictionary               */
      /****************************************************************/
     } /* endif */
   } /* endif */

   *ppBTIda = pBTIda;                               // set base pointer

   return ( sRc );
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictCreate      Create Dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDictCreate( PSZ, PSZ, SHORT, PCHAR, USHORT, PCHAR,
//                                    PCHAR, PCHAR, PPBTREE);
//------------------------------------------------------------------------------
// Description:       Establishes the basic parameters for searching a
//                    user dictionary.
//                    These parameters are stored in the first record of the
//                    index file so that subsequent accesses
//                    know what the index is like.
//
//                    If no server name is given (NULL pointer or EOS) than
//                    it is tried to open a local dictionary.
//                    If no collating sequence is given (NULL pointer) the
//                    default collating sequence is assumed
//------------------------------------------------------------------------------
// Parameters:        PSZ              name of the index file
//                    PSZ              name of the server
//                    SHORT            number of buffers used
//                    PCHAR            pointer to user data
//                    USHORT           length of user data
//                    PCHAR            pointer to term encoding sequence
//                    PCHAR            pointer to collating sequence
//                    PCHAR            pointer to case map file
//                    PBTREE *         pointer to btree structure
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_NO_ROOM     memory shortage
//                    BTREE_USERDATA    user data too long
//                    BTREE_OPEN_ERROR  dictionary already exists
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//------------------------------------------------------------------------------
// Function flow:     allocate space for BTREE
//                    if ok then
//                      determine type of operation (Local/Remote) and call
//                      appropriate functions
//                    endif
//                    return return code
//------------------------------------------------------------------------------
SHORT QDAMDictCreate
(
   PSZ    pName,                       // name of file
   PSZ    pServer,                     // name of the server
   SHORT  sNumberOfKey,                // number of key buffers
   PCHAR  pUserData,                   // user data
   USHORT usLen,                       // length of user data
   PCHAR  pTermTable,                  // term encoding sequence
   PCHAR  pCollating,                  // collating sequence
   PCHAR  pCaseMap,                    // case map string
   PBTREE * ppBTIda                    // pointer to btree structure
)
{
  PBTREE  pBTIda;
  SHORT sRc = 0;                       // return code

  *ppBTIda = NULL;                        // init return value

  UtlAlloc( (PVOID *)&pBTIda, 0L, (LONG) sizeof(BTREE), NOMSG );
  if ( !pBTIda )
  {
    sRc = BTREE_NO_ROOM;
  }
  else
  {
    *ppBTIda = pBTIda;                     // set pointer to base structure

     if ( !pServer )
     {
       pServer = EMPTY_STRING;
     } /* endif */

    /****************************************************************/
    /* depending on pServer either try to create it locally or remote */
    /****************************************************************/
    if ( *pServer  )
    {
      // not active in WIndows!!
    }
    else
    {
      sRc = QDAMDictCreateLocal( pName, sNumberOfKey, pUserData, usLen,
                                pTermTable, pCollating, pCaseMap, ppBTIda,NULL);
      if ( sRc == BTREE_OPEN_ERROR )
      {
         UtlAlloc( (PVOID *)&(pBTIda->pBTree), 0L, 0L, NOMSG );
         UtlAlloc( (PVOID *)ppBTIda, 0L, 0L, NOMSG );
      } /* endif */
    } /* endif */


  } /* endif */

  return( sRc );
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictClose    close the dictionary
//------------------------------------------------------------------------------
// Function call:     QDAMDictClose( PPBTREE );
//
//------------------------------------------------------------------------------
// Description:       Close the file
//
//------------------------------------------------------------------------------
// Parameters:        PPBTREE                pointer to btree structure
//
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     incorrect pointer
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_CLOSE_ERROR error closing dictionary
//------------------------------------------------------------------------------
// Function flow:     if BTree does not exist
//                      set Rc = BTREE_INVALID
//                    else
//                      depending on type (fRemote or local) call the
//                      appropriate routine
//                      if okay so far then
//                        free space of BTree
//                      endif
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMDictClose
(
   PPBTREE ppBTIda
)
{
   SHORT sRc = 0;                      // error return

   if ( ! *ppBTIda )
   {
     sRc = BTREE_INVALID;
   }
   else
   {
      sRc = QDAMDictCloseLocal( *ppBTIda );
      if ( !sRc )
      {
        UtlAlloc( (PVOID *)ppBTIda, 0L, 0L, NOMSG );
      } /* endif */
   } /* endif */

   return sRc;
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictSign    Read User Data
//------------------------------------------------------------------------------
// Function call:     QDAMDictSign( PBTREE, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Gets the second part of the first record ( user data )
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to user data
//                    PUSHORT                length of user data area (input)
//                                           filled length (output)
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     pointer invalid
//                    BTREE_USERDATA    user data too long
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//
//------------------------------------------------------------------------------
// Side effects:      return signature record even if dictionary is corrupted
//
//------------------------------------------------------------------------------
// Function flow:     if BTree does not exist
//                      set Rc = BTREE_INVALID
//                    else
//                      depending on type (fRemote or local) call the
//                      appropriate routine
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMDictSign
(
   PBTREE pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   PUSHORT pusLen                      // length of user data
)
{
  SHORT  sRc=0;                        // return code
  if ( pBTIda )
  {
    /******************************************************************/
    /* no pointer to user data passed                                 */
    /******************************************************************/
    if ( !pUserData || !*pusLen )
    {
      *pusLen = 0;
      pUserData = NULL;
    } /* endif */
    sRc = QDAMDictSignLocal( pBTIda, pUserData, pusLen );
  }
  else
  {
     sRc = BTREE_INVALID;
  } /* endif */
  return sRc;
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictSubStr   Find Key starting with stubstring
//------------------------------------------------------------------------------
// Function call:     QDAMDictSubStr(PBTREE,PCHAR,PCHAR,PUSHORT,PCHAR,PUSHORT);
//
//------------------------------------------------------------------------------
// Description:       Find the first key starting with the passed key and
//                    pass it back.
//                    If no error happened set this location as new
//                    current position
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key to be looked for
//                    PCHAR                  buffer for the key
//                    PUSHORT                on input length of buffer
//                                           on output length of filled data
//                    PCHAR                  buffer for the user data
//                    PUSHORT                on input length of buffer
//                                           on output length of filled data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//
//------------------------------------------------------------------------------
// Function flow:     if BTree does not exist
//                      set Rc = BTREE_INVALID
//                    else
//                      depending on type (fRemote or local) call the
//                      appropriate routine
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMDictSubStr
(
   PBTREE   pBTIda,                      // pointer to btree struct
   PCHAR_W  pKey,                        // key to be searched for
   PBYTE    pchBuffer,                   // space for key data
   PULONG   pulLength,                   // in/out length of returned key data
   PBYTE    pchUserData,                 // space for user data
   PULONG   pulUserLen                   // in/out length of returned user data
)
{
  SHORT    sRc  = 0;                   // return code

  if ( pBTIda )
  {
     /******************************************************************/
     /* no pointer to user data passed                                 */
     /******************************************************************/
     if ( !pchUserData || ! *pulUserLen )
     {
       *pulUserLen = 0;
       pchUserData = NULL;
     } /* endif */
     if ( !pchBuffer || ! *pulLength )
     {
       *pulLength = 0;
       pchBuffer = NULL;
     } /* endif */

     sRc = QDAMDictSubStrLocal( pBTIda, pKey, pchBuffer, pulLength,
                                  pchUserData, pulUserLen );
  }
  else
  {
     sRc = BTREE_INVALID;
  } /* endif */
  return ( sRc );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictEquiv      Find equivalent match
//------------------------------------------------------------------------------
// Function call:     QDAMDictEquiv(PBTREE,PCHAR,PCHAR,PUSHORT,PCHAR,PUSHORT);
//
//------------------------------------------------------------------------------
// Description:       Find the first key which is equivalent to
//                    the passed key
//                    If no error happened set this location as
//                    new current position
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE               pointer to btree structure
//                    PCHAR                key to be looked for
//                    PCHAR                buffer for the key
//                    PUSHORT              on input length of buffer
//                                         on output length of filled data
//                    PCHAR                buffer for the user data
//                    PUSHORT              on input length of buffer
//                                         on output length of filled data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//
//------------------------------------------------------------------------------
// Function flow:     if BTree does not exist
//                      set Rc = BTREE_INVALID
//                    else
//                      depending on type ( remote or local) call the
//                      appropriate routine
//                    endif
//
//------------------------------------------------------------------------------
SHORT QDAMDictEquiv
(
   PBTREE   pBTIda,                      // pointer to btree struct
   PCHAR_W  pKey,                        // key to be searched for
   PBYTE    pchBuffer,                   // space for key data
   PULONG   pulLength,                  // in/out length of returned key data
   PBYTE    pchUserData,                 // space for user data
   PULONG   pulUserLen                  // in/out length of returned user data
)
{
  SHORT    sRc  = 0;                   // return code

  if ( pBTIda )
  {
     /******************************************************************/
     /* no pointer to user data passed                                 */
     /******************************************************************/
     if ( !pchUserData || !*pulUserLen )
     {
       *pulUserLen = 0;
       pchUserData = NULL;
     } /* endif */
     if ( !pchBuffer || !*pulLength )
     {
       *pulLength = 0;
       pchBuffer = NULL;
     } /* endif */
     sRc = QDAMDictEquivLocal( pBTIda, pKey, pchBuffer, pulLength,
                                 pchUserData, pulUserLen );
  }
  else
  {
     sRc = BTREE_INVALID;
  } /* endif */
  return ( sRc );
}
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictExact      Find Exact match
//------------------------------------------------------------------------------
// Function call:     QDAMDictExact( PBTREE, PCHAR, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:        Find an exact match for the passed key
//
//------------------------------------------------------------------------------
// Parameters:         PBTREE               pointer to btree structure
//                     PCHAR                key to be inserted
//                     PCHAR                buffer for user data
//                     PUSHORT              on input length of buffer
//                                          on output length of filled data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//------------------------------------------------------------------------------
// Function flow:     if BTree does not exist
//                      set Rc = BTREE_INVALID
//                    else
//                      depending on type ( remote or local) call the
//                      appropriate routine
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMDictExact
(
   PBTREE  pBTIda,                      // pointer to btree struct
   PCHAR_W pKey,                        // key to be searched for
   PBYTE   pchBuffer,                   // space for user data
   PULONG  pulLength,                   // in/out length of returned user data
   USHORT  usSearchSubType              // special hyphenation lookup flag
)
{
  SHORT      sRc  = 0;                  // return code

  if ( pBTIda )
  {
     /******************************************************************/
     /* no pointer to user data passed                                 */
     /******************************************************************/
     if ( !pchBuffer || !*pulLength )
     {
       *pulLength = 0;
       pchBuffer = NULL;
     } /* endif */
     sRc = QDAMDictExactLocal( pBTIda, pKey, pchBuffer, pulLength, usSearchSubType ) ;
  }
  else
  {
    sRc = BTREE_INVALID;
  } /* endif */
  return ( sRc );
}


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictNext   Get the next entry back -
//------------------------------------------------------------------------------
// Function call:     QDAMDictNext( PBTREE, PCHAR, PUSHORT, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Locate the next entry (by collating sequence) and
//                    pass back the associated information into
//                    the user provided buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//------------------------------------------------------------------------------
// Function flow:     if BTree does not exist
//                      set Rc = BTREE_INVALID
//                    else
//                      depending on type ( remote or local) call the
//                      appropriate routine
//                    endif
//
//------------------------------------------------------------------------------
SHORT QDAMDictNext
(
   PBTREE     pBTIda,
   PCHAR_W    pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
)
{
  SHORT      sRc = 0;                 // return code

  if ( pBTIda )
  {
    /******************************************************************/
    /* no pointer to user data passed                                 */
    /******************************************************************/
    if ( !pUserData|| !*pulUserLen )
    {
      *pulUserLen = 0;
      pUserData = NULL;
    } /* endif */
    if ( !pKeyData || !*pulKeyLen )
    {
      *pulKeyLen = 0;
      pKeyData = NULL;
    } /* endif */

    sRc = QDAMDictNextLocal( pBTIda, pKeyData,pulKeyLen,
                               pUserData,pulUserLen );
  }
  else
  {
     sRc = BTREE_INVALID;
  } /* endif */
  return ( sRc );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictPrev     Get the prev entry back
//------------------------------------------------------------------------------
// Function call:     QDAMDictPrev( PBTREE, PCHAR, PUSHORT, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Locate the previous entry (by collating sequence)
//                    and pass back the associated
//                    information into the user provided buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//------------------------------------------------------------------------------
// Function flow:     if BTree does not exist
//                      set Rc = BTREE_INVALID
//                    else
//                      depending on type ( remote or local) call the
//                      appropriate routine
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMDictPrev
(
   PBTREE     pBTIda,
   PCHAR_W    pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
)
{
  SHORT      sRc = 0;                 // return code

  if ( pBTIda )
  {
    /******************************************************************/
    /* no pointer to user data passed                                 */
    /******************************************************************/
    if ( !pUserData || !*pulUserLen )
    {
      *pulUserLen = 0;
      pUserData = NULL;
    } /* endif */
    if ( !pKeyData || !*pulKeyLen )
    {
      *pulKeyLen = 0;
      pKeyData = NULL;
    } /* endif */
    sRc = QDAMDictPrevLocal( pBTIda, pKeyData,pulKeyLen, pUserData,pulUserLen );
  }
  else
  {
     sRc = BTREE_INVALID;
  } /* endif */
  return ( sRc );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictCurrent   Get current entry back
//------------------------------------------------------------------------------
// Function call:     QDAMDictCurrent( PBTREE, PCHAR, PUSHORT, PCHAR, PUSHORT )
//
//------------------------------------------------------------------------------
// Description:       Locate the current entry  and pass back the
//                    associatedinformation into the user provided buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_INVALID     invalid pointer passed
//                    BTREE_EOF_REACHED eof or start reached
//                    BTREE_CORRUPTED   dictionary corrupted
//------------------------------------------------------------------------------
// Function flow:     if BTree does not exist
//                      set Rc = BTREE_INVALID
//                    else
//                      depending on type ( remote or local) call the
//                      appropriate routine
//                    endif
//
//------------------------------------------------------------------------------
SHORT QDAMDictCurrent
(
   PBTREE     pBTIda,
   PBYTE      pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
)
{
  SHORT      sRc = 0;                 // return code

  if ( pBTIda )
  {
    /******************************************************************/
    /* no pointer to user data passed                                 */
    /******************************************************************/
    if ( !pUserData || !*pulUserLen )
    {
      *pulUserLen = 0;
      pUserData = NULL;
    } /* endif */
    if ( !pKeyData || !*pulKeyLen )
    {
      *pulKeyLen = 0;
      pKeyData = NULL;
    } /* endif */
    sRc = QDAMDictCurrentLocal( pBTIda, pKeyData, pulKeyLen,
                                  pUserData, pulUserLen );
  }
  else
  {
     sRc = BTREE_INVALID;
  } /* endif */
  return ( sRc );
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictCopy         Copy Entries
//------------------------------------------------------------------------------
// Function call:     QDAMDictCopy     ( PBTREE, PBTREE );
//
//------------------------------------------------------------------------------
// Description:       Insert the current entry from the source dictionary
//                    into the target dictionary and point to the next
//                    entry in the source dictionary
//
//                    This is the only function which will have NO local
//                    pendant. This is necessary to allow to copy/organize
//                    on different levels
//------------------------------------------------------------------------------
// Parameters:        PBTREE       pointer to btree struct of source dict
//                    PBTREE       pointer to btree struct of target dict
//
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary corrupted
//                    BTREE_NOT_FOUND   invalid data
//                    BTREE_INVALID     invalid data pointer
//
//------------------------------------------------------------------------------
// Function flow:     if both tree pointers are given then
//                      store corruption flag temporarily
//                      if target is corrupted then
//                        set Rc to BTREE_CORRUPTED
//                      else
//                        reset corruption flag of source dictionary
//                        get current entry from source; set Rc appropriate
//                      endif
//                      if okay then
//                        insert dictionary entry into target dict; set Rc appr
//                      endif
//                    else
//                      set Rc to BTREE_INVALID
//                    endif
//                    return Rc;
//
//------------------------------------------------------------------------------
#ifdef UNUSED_CODE
SHORT QDAMDictCopy
(
   PBTREE pBTSourceIda,               // pointer of source tree structure
   PBTREE pBTTargetIda                // pointer of target tree structure
)
{
   SHORT    sRc = 0;                  // return code
   ULONG    ulKeyLen = HEADTERM_SIZE; // key length
   ULONG    ulDataLen = MAXDATASIZE;  // data length
   PBTREEBUFFER pRecord;              // pointer to record buffer
   BOOL     fCorrupted;               // current corruption flag
   PBTREEGLOB pBTSource;              // pointer to global struct
   PBTREEGLOB pBTTarget;              // pointer to global struct

   pBTSource = pBTSourceIda->pBTree;  // get pointer to global struct
   pBTTarget = pBTTargetIda->pBTree;  // get pointer to global struct
   if ( pBTSource && pBTTarget )
   {
      fCorrupted = pBTSource->fCorrupted;
      if ( pBTTarget->fCorrupted )
      {
         sRc = BTREE_CORRUPTED;
      }
      else
      {
         // reset corrupted flag for pBTSource for the moment
         //   otherwise we will have no chance in doing anything
         pBTSource->fCorrupted = FALSE;

         if ( !sRc )
         {
            // get entry from source
            sRc = QDAMDictCurrent( pBTSourceIda, (PBYTE)pBTSource->pTempKey, &ulKeyLen,
                                   pBTSource->pTempRecord, &ulDataLen);
         } /* endif */
      } /* endif */

      // if okay add it to target
      if ( !sRc )
      {
         sRc = QDAMDictInsertLocal( pBTTargetIda, pBTSource->pTempKey,
                               pBTSource->pTempRecord, ulDataLen );
      } /* endif */

      // increment position indicator
      if ( !sRc )
      {
         pBTSourceIda->sCurrentIndex++;               // point to next word
                                                      // and validate index
         sRc = QDAMReadRecord( pBTSourceIda,
                               pBTSourceIda->usCurrentRecord,&pRecord,
                               FALSE);
         if ( !sRc )
         {
            sRc = QDAMValidateIndex( pBTSourceIda, &pRecord );
         } /* endif */
         pBTSource->fCorrupted = fCorrupted;          // restore corrupt flag
      } /* endif */
   }
   else
   {
      sRc = BTREE_INVALID;
   } /* endif */
   return( sRc );
}
#endif
//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictFirst   Get the first entry back
//------------------------------------------------------------------------------
// Function call:     QDAMDictFirst( PBTREE, PCHAR, PUSHORT, PCHAR, PUSHORT );
//
//------------------------------------------------------------------------------
// Description:       Locate the first entry and pass back the
//                    associated information into the user provided
//                    buffers
//
//------------------------------------------------------------------------------
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//------------------------------------------------------------------------------
// Function flow:     if BTree does not exist
//                      set Rc = BTREE_INVALID
//                    else
//                      depending on type ( remote or local) call the
//                      appropriate routine
//                    endif
//------------------------------------------------------------------------------
SHORT QDAMDictFirst
(
   PBTREE     pBTIda,
   PCHAR_W    pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
)
{
  SHORT      sRc = 0;                 // return code

   if ( pBTIda )
   {
     /******************************************************************/
     /* no pointer to user data passed                                 */
     /******************************************************************/
     if ( !pUserData || !*pulUserLen )
     {
       *pulUserLen = 0;
       pUserData = NULL;
     } /* endif */
     if ( !pKeyData || !*pulKeyLen )
     {
       *pulKeyLen = 0;
       pKeyData = NULL;
     } /* endif */

     sRc = QDAMDictFirstLocal( pBTIda, pKeyData, pulKeyLen,
                                 pUserData, pulUserLen );
   }
   else
   {
      sRc = BTREE_INVALID;
   } /* endif */
   return ( sRc );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMCheckDict
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       check if the specified dictionary is already open
//------------------------------------------------------------------------------
// Parameters:        PSZ    pName,           name of dictionary
//                    PBTREE pBTIda           pointer to ida
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0  success
//                    BTREE_DICT_LOCKED   dictionary is locked
//                    BTREE_MAX_DICTS     maximum of dictionaries exceeded
//------------------------------------------------------------------------------
// Function flow:     check if dict is already open
//                    if so use this handle and check for lock;
//                    if locked then
//                      set return code to locked
//                    else
//                      copy significant data into our global structure
//                    endif
//                    if too many open dicts
//                      set return code to BTREE_MAX_DICTS
//                    endif
//                    return success
//
//------------------------------------------------------------------------------
SHORT
QDAMCheckDict
(
  PSZ    pName,                        // name of dictionary
  PBTREE pBTIda                        // pointer to ida
)
{
  USHORT  usI = 1;
  SHORT   sRc = 0;
  PQDAMDICT pQDict;

  /*******************************************************************/
  /* check if dict is already open - if so use this handle           */
  /*******************************************************************/
  while ( QDAMDict[usI].usOpenCount )
  {
    if ( stricmp(QDAMDict[usI].chDictName, pName ) == 0)
    {
      /***************************************************************/
      /* check if dictionary is locked                               */
      /***************************************************************/
      if ( QDAMDict[usI].fDictLock )
      {
        sRc = BTREE_DICT_LOCKED;
      }
      else
      {
        /***************************************************************/
        /* copy relevant data from global struct                       */
        /***************************************************************/
        pQDict = &(QDAMDict[usI]);
        pQDict->usOpenCount++;
        pQDict->pIdaList[ pQDict->usOpenCount ] = pBTIda;
        pBTIda->pBTree = pQDict->pBTree;
        pBTIda->usDictNum = usI;
      } /* endif */
      break;
    }
    else
    {
      usI++;
    } /* endif */
  } /* endwhile */

  if ( usI >= MAX_NUM_DICTS - 1 )
  {
    sRc = BTREE_MAX_DICTS;
  } /* endif */

  return ( sRc );
} /* end of function QDAMCheckDict */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMAddDict
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       add dictionary to our global list of open dicts
//------------------------------------------------------------------------------
// Parameters:        PSZ    pName,           name of dictionary
//                    PBTREE pBTIda           pointer to ida
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0  success
//                    BTREE_MAX_DICTS     maximum of dictionaries exceeded
//------------------------------------------------------------------------------
// Function flow:     loop thru global struct to find open slot.
//                    if too many open dicts
//                      set return code to BTREE_MAX_DICTS
//                    else
//                      copy significant data into free slot
//                    endif
//                    return success
//
//------------------------------------------------------------------------------
SHORT
QDAMAddDict
(
  PSZ  pName,
  PBTREE  pBTIda
)
{
  USHORT usI = 1;
  SHORT  sRc = 0;
  PQDAMDICT  pQDict;

  while ( QDAMDict[usI].usOpenCount )
  {
    usI++;
  } /* endwhile */
  /********************************************************************/
  /* if still space is available fill it up                           */
  /********************************************************************/
  if ( usI < MAX_NUM_DICTS )
  {
    pQDict = &(QDAMDict[usI]);
    pQDict->pBTree = pBTIda->pBTree;
    pQDict->usOpenCount = 1;
    pQDict->pIdaList[pQDict->usOpenCount] = pBTIda;
    strcpy(pQDict->chDictName, pName);
    pBTIda->usDictNum = usI;
  }
  else
  {
    sRc = BTREE_MAX_DICTS;
  } /* endif */

  return sRc;
} /* end of function QDAMAddDict */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMRemoveDict
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       _
//------------------------------------------------------------------------------
// Parameters:        PBTREE pBTIda           pointer to ida
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       open count of current dict
//------------------------------------------------------------------------------
// Function flow:     find dictionary in list
//                    remove our entry from the list
//                    we closed one dictionary..
//                    if open count=0
//                      clear structure
//                    endif
//                    return number of open count
//------------------------------------------------------------------------------
USHORT
QDAMRemoveDict
(
  PBTREE pBTIda                        // pointer to ida
)
{
  USHORT usI;
  PPBTREE  ppBTSrc;
  PPBTREE  ppBTTgt;
  PQDAMDICT  pQDict;
  /********************************************************************/
  /* find dictionary in list                                          */
  /********************************************************************/
  usI = pBTIda->usDictNum;

  pQDict = &(QDAMDict[usI]);
  ppBTSrc = &(pQDict->pIdaList[1]);
  ppBTTgt = ppBTSrc;

  /********************************************************************/
  /* remove our entry from the list                                   */
  /********************************************************************/
  while ( (*ppBTTgt = *ppBTSrc) != NULL )
  {
    if ( *ppBTSrc != pBTIda )
    {
      ppBTTgt++;           // copy entry
    } /* endif */
    ppBTSrc++;
  } /* endwhile */
  *ppBTTgt = NULL;

  /********************************************************************/
  /* we closed one dictionary..                                       */
  /********************************************************************/
  if ( pQDict->usOpenCount )
  {
    pQDict->usOpenCount --;
  } /* endif */

  if ( ! pQDict->usOpenCount  )
  {
    memset( pQDict, 0, sizeof( QDAMDICT ));
  } /* endif */

  return( pQDict->usOpenCount );
} /* end of function QDAMRemoveDict */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictLockDictLocal
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       try to lock the dictionary for exclusive use
//------------------------------------------------------------------------------
// Parameters:        PBTREE  pBTIda,               pointer to ida
//                    BOOL    fLock                 lock or unlock
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0   success
//                    BTREE_DICT_LOCKED   dictionary could not be locked
//------------------------------------------------------------------------------
// Function flow:      if locking
//                        if open count > 1  then
//                          set return code to BTREE_DICT_LOCKED;
//                        else
//                          set lock flag
//                        endif
//                     else
//                        set lock flag as passed as input parameter
//                     endif
//                     return success
//------------------------------------------------------------------------------
SHORT
QDAMDictLockDictLocal
(
  PBTREE  pBTIda,                      // pointer to ida
  BOOL    fLock                        // lock or unlock
)
{
  SHORT  sRc = 0;

  /********************************************************************/
  /* check if open count > 1, then no exclusive use possible any more */
  /********************************************************************/
  if ( fLock )
  {
    if ( QDAMDict[ pBTIda->usDictNum ].usOpenCount > 1 )
    {
      sRc = BTREE_DICT_LOCKED;
    }
    else
    {
      QDAMDict[ pBTIda->usDictNum ].fDictLock = fLock;
    } /* endif */
  }
  else
  {
    QDAMDict[ pBTIda->usDictNum ].fDictLock = fLock;
  } /* endif */

  return sRc;
} /* end of function QDAMDictLockDictLocal */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictLockEntryLocal
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       try to lock specific entry
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0  success
//                    BTREE_ENTRY_LOCKED
//------------------------------------------------------------------------------
// Function flow:     lock or unlock the entry
//                    if locking then
//                      check the subject entry is not locked somewhere else...
//                      if not locked then
//                        lock it
//                      else
//                        set return code to BTREE_ENTRY_LOCKED;
//                      endif
//                    else
//                      unlock entry
//                    endif
//                    return success
//------------------------------------------------------------------------------
SHORT
QDAMDictLockEntryLocal
(
  PBTREE  pBTIda,                      // success indicator
  PSZ_W   pKey,                        // key
  BOOL    fLock                        // lock or unlock
)
{
  SHORT sRc = 0;
  PPBTREE  ppBTemp;
  PQDAMDICT pQDict;

  /********************************************************************/
  /* lock or unlock the entry                                         */
  /********************************************************************/
  if ( fLock )
  {
    /******************************************************************/
    /* check the subject entry is not locked somewhere else...        */
    /******************************************************************/
    pQDict = &(QDAMDict[pBTIda->usDictNum]);
    ppBTemp = &(pQDict->pIdaList[1]);

    while ( *ppBTemp && !sRc)
    {
      if ( (*ppBTemp)->fLock && ! UTF16stricmp( pKey, (*ppBTemp)->chLockedTerm) )
      {
        sRc = BTREE_ENTRY_LOCKED;
      }
      else
      {
        ppBTemp++;
      } /* endif */
    } /* endwhile */

    /******************************************************************/
    /* lock it if not locked somewhere else                           */
    /******************************************************************/
    if ( !sRc )
    {
      pBTIda->fLock = fLock;
      UTF16strcpy( pBTIda->chLockedTerm, pKey );
    } /* endif */

    /******************************************************************/
    /* For shared databases only: lock entry by adding entry to       */
    /* record containing the list of locked terms                     */
    /******************************************************************/
    if ( !sRc && (pBTIda->pBTree->usOpenFlags & ASD_SHARED) )
    {
      sRc = QDAMUpdateLockRec( pBTIda, pKey, TRUE );
      if ( sRc )                       // if lock fails ...
      {
        pBTIda->fLock = FALSE;         // ... remove local lock
        pBTIda->chLockedTerm[0] = EOS;
      } /* endif */
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* unlock the entry ..                                            */
    /******************************************************************/
    pBTIda->fLock = fLock;
    pBTIda->chLockedTerm[0] = EOS;

    /******************************************************************/
    /* For shared databases only: unlock entry by removing entry from */
    /* record containing the list of locked terms                     */
    /******************************************************************/
    if ( !sRc && (pBTIda->pBTree->usOpenFlags & ASD_SHARED) )
    {
      sRc = QDAMUpdateLockRec( pBTIda, pKey, FALSE );
    } /* endif */
  } /* endif */
  return sRc;
} /* end of function QDAMDictLockEntryLocal */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictLockStatus
//------------------------------------------------------------------------------
// Function call:     fLocked = QDAMDictLockStatus( pBTIda, pHeadTerm );
//------------------------------------------------------------------------------
// Description:       This call returns the status of the current entry
//------------------------------------------------------------------------------
// Parameters:        PBTREE  pBTIDa   pointer to instance data
//                    PSZ     pHeadTerm  pointer to head term
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE   entry is locked
//                    FALSE  entry is free for update
//------------------------------------------------------------------------------
// Function flow:     get pointer to list of all open instances of our dict..
//                    run thru list and check if this headterm is locked ..
//------------------------------------------------------------------------------
BOOL
QDAMDictLockStatus
(
  PBTREE  pBTIda,
  PSZ_W   pKey
)
{
  PPBTREE  ppBTemp;
  PQDAMDICT pQDict;
  BOOL     fLock = FALSE;
  /******************************************************************/
  /* check the subject entry is not locked somewhere else...        */
  /******************************************************************/
  pQDict = &(QDAMDict[pBTIda->usDictNum]);
  ppBTemp = &(pQDict->pIdaList[1]);

  while ( *ppBTemp && !fLock)
  {
    if ( (*ppBTemp)->fLock && ! UTF16stricmp( pKey, (*ppBTemp)->chLockedTerm) )
    {
      /****************************************************************/
      /* set fLock only if it's not me who want to update ....        */
      /****************************************************************/
      if ( pBTIda != *ppBTemp )
      {
        fLock = TRUE;
      }
      else
      {
        ppBTemp++;
      } /* endif */
    }
    else
    {
      ppBTemp++;
    } /* endif */
  } /* endwhile */

  return( fLock );
} /* end of function QDAMDictLockStatus */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMDictUpdStatus
//------------------------------------------------------------------------------
// Function call:     QDAMDictUpdStatus( pBTIda );
//------------------------------------------------------------------------------
// Description:       invalidates the status of the current record
//------------------------------------------------------------------------------
// Parameters:        PBTREE  pBTIda   pointer to instance data
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     get pointer to list of instances
//                    run through list and invalidate the current entry for
//                      all instances ...
//                    Our instance data will be updated after this call, so
//                      it's okay to get rid of it, too.
//------------------------------------------------------------------------------
VOID
QDAMDictUpdStatus
(
  PBTREE  pBTIda
)
{
  PPBTREE  ppBTemp;
  PQDAMDICT pQDict;
  /********************************************************************/
  /* update, i.e. invalidate the sCurrent record and offset           */
  /********************************************************************/
  pQDict = &(QDAMDict[pBTIda->usDictNum]);
  ppBTemp = &(pQDict->pIdaList[1]);

  while ( *ppBTemp )
  {
    (*ppBTemp)->sCurrentIndex = RESET_VALUE;
    (*ppBTemp)->usCurrentRecord = 0;
    ppBTemp++;
  } /* endwhile */

} /* end of function QDAMDictUpdStatus */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDeleteFile
//------------------------------------------------------------------------------
// Function call:     QDAMDeleteFile( szServer, pFileName );
//------------------------------------------------------------------------------
// Description:       delete the specified file on the server
//------------------------------------------------------------------------------
// Parameters:        SERVERNAME   szServer   name of the server
//                    PSZ          pFileName  pointer to file name
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Function flow:     prepare the DELFILE struct to delete the file
//------------------------------------------------------------------------------
USHORT
QDAMDeleteFile
(
   SERVERNAME szServer,             // Servername
   PSZ        pszFilePath           // Full file path
)
{

   BOOL                fOK;                          // Process returncode
   USHORT              usURc = BTREE_NO_ROOM;        // Function returncode
   PDELFILE_IN         pDelFileIn = NULL;            // Pointer to DELFILE_IN struct
   PDELFILE_OUT        pDelFileOut = NULL;           // Pointer to DELFILE_OUT struct

   // Allocate storage for all structures and pointers needed
   fOK = UtlAlloc( (PVOID *)&pDelFileOut, 0L,
                   (LONG)( sizeof( DELFILE_IN ) + sizeof( DELFILE_OUT ) ),
                   NOMSG );

   if ( fOK )
   {
     // Security check
     if ( ( szServer != NULL ) && ( pszFilePath != NULL ) )
     {
       // Set pointers
       pDelFileIn = (PDELFILE_IN)( pDelFileOut + 1 );

       // Fill input structure
       pDelFileIn->prefin.usLenIn   = sizeof( DELFILE_IN );
       pDelFileIn->prefin.idCommand = QDAM_DELETE_FILE;  // !!! CHM removed comment before ! assignment
       strcpy( pDelFileIn->szServer, szServer );
       strcpy( pDelFileIn->szFileName, pszFilePath );

       if ( !usURc )
       {
         usURc = pDelFileOut->prefout.rcTmt;
       } /* endif */
     } /* endif */

     // Free allocated storage
     UtlAlloc( (PVOID *)&pDelFileOut, 0L, 0L, NOMSG );
   } /* endif */

   return usURc;

} /* end of function QDAMDeleteFile */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QDAMFindInstance
//------------------------------------------------------------------------------
// Function call:     QDAMFindInstance( pBTIda, pUserId )
//------------------------------------------------------------------------------
// Description:       This function will find the instance for the user.
//                    This function is necessary, because in the remote case
//                    one will not get back the instancepointer through the
//                    open or create call and the pipe handle is the same
//                    for all instances
//------------------------------------------------------------------------------
// Parameters:        PBTIDA  pBTIda    pointer to ida
//                    PSZ     pUserId   pointer to user id
//------------------------------------------------------------------------------
// Returncode type:   PBTIDA
//------------------------------------------------------------------------------
// Returncodes:       pointer to active instance data
//------------------------------------------------------------------------------
// Function flow:     loop thru all instances for this dictionary and find
//                    the correct one.
//------------------------------------------------------------------------------
PBTREE
QDAMFindInstance
(
  PBTREE  pBTIda,
  PSZ     pUserId
)
{
  PPBTREE  ppBTemp;
  PQDAMDICT pQDict;
  BOOL     fFound = FALSE;
  /******************************************************************/
  /* check the subject entry is not locked somewhere else...        */
  /******************************************************************/
  pQDict = &(QDAMDict[pBTIda->usDictNum]);
  ppBTemp = &(pQDict->pIdaList[1]);

  while ( *ppBTemp && !fFound )
  {
    if ( !stricmp( pUserId, (*ppBTemp)->szUserId) )
    {
      fFound = TRUE;
      pBTIda = *ppBTemp;
    }
    else
    {
      ppBTemp++;
    } /* endif */
  } /* endwhile */

  return pBTIda;
} /* end of function QDAMFindInstance */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictCloseOrganize
//------------------------------------------------------------------------------
// Function call:     QDAMDictCloseOrganize( pBTree, pszAsdDictPath,
//                                           chPrimaryDrive, usCloseRc )
//------------------------------------------------------------------------------
// Description:       calls communication code for the original basic
//                    dictionary which unlocks, closes and renames in one go.
//                    The object is to avoid other processes from interrupting
//                    any of the individual processes and causing havoc.
//------------------------------------------------------------------------------
// Input parameter:   PPBTREE   ppBTreeofAsd
// Parameters:        PSZ       pszOriginalAsdPath
//                    CHAR      chRemotePrimaryDrive
//                    USHORT    usOrganizeCloseRc
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       u_code call return code
//------------------------------------------------------------------------------
// Function flow:     fill dictionary organize close structure for u_code
//                    call u_code
//                    free allocated structures and set back counters to
//                    show that dict has been closed
//                    return return code from u_code
//------------------------------------------------------------------------------

SHORT QDAMDictCloseOrganize
(
   PPBTREE ppBTIda,
   PSZ     pszDictPath,
   CHAR    chPrimDrive,
   USHORT  usRc
)
{
   SHORT sRc = 0;                      // error return
   PQDAMLAN  pQDAMLan;                 // pointer to flat LAN structure
   PDICTORG  pDictOrg;                 // organize structure

   if ( ! *ppBTIda )
   {
     sRc = BTREE_INVALID;
   }
   else
   {
     /**************************************************************/
     /* prepare structure for closing remote dictionary            */
     /**************************************************************/
     pQDAMLan = (*ppBTIda)->pQDAMLanIn;
     pQDAMLan->DictCmd  = QDAMDICTCLOSEORGANIZE;
     pQDAMLan->pBTreeRemote = (*ppBTIda)->pBTreeRemote;

     pDictOrg = (PDICTORG) (pQDAMLan + 1);
     strcpy( pDictOrg->szDictName, pszDictPath );
     pDictOrg->chPrimDrive = chPrimDrive;
     pDictOrg->usCloseRc = usRc;

     /************************************************************/
     /* call the U code here ....                                */
     /************************************************************/
     pQDAMLan->PrefInOut.prefin.idCommand = QDAM_CLOSEORGANIZE;
     pQDAMLan->usOutLen = sizeof(QDAMLAN);
     ((PIN)pQDAMLan)->usLenIn = sizeof(QDAMLAN) + sizeof(DICTORG);
     /************************************************************/
     /* free the LAN structure if no other instance is open ...  */
     /************************************************************/
     if ( ! QDAMRemoveDict( *ppBTIda ))
     {
       UtlAlloc( (PVOID *)&(*ppBTIda)->pQDAMLanOut, 0L, 0L, NOMSG );
       UtlAlloc( (PVOID *)&(*ppBTIda)->pQDAMLanIn, 0L, 0L, NOMSG );
       UtlAlloc( (PVOID *)ppBTIda, 0L, 0L, NOMSG );
     } /* endif */
   } /* endif */

   return sRc;
} /* end QDAMDictCloseorganize */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictCloseOrganizeLocal
//------------------------------------------------------------------------------
// Function call:     QDAMDictCloseOrganizeLocal( pQDAMLan )
//------------------------------------------------------------------------------
// Description:       Unlocks the basic dictionary, then closes it
//                    and consequently renames the new temp files
//                    and gives then the original file names
//------------------------------------------------------------------------------
// Parameters:        PQDAMLAN   pQDAMLan
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       sRc = LX_RC_OK_ASD - unlock, close, rename successful
//------------------------------------------------------------------------------
// Function flow:     unlock the original basic(asd) dictionary
//                    close the asd dictionary
//                    rename the newly created temp files to the original
//                    files
//------------------------------------------------------------------------------

SHORT QDAMDictCloseOrganizeLocal
(
   PQDAMLAN pQDAMLan
)
{
  SHORT          sRc;
  PDICTORG       pDictOrg;             // pointer to dictorg structure

  pDictOrg = (PDICTORG) (pQDAMLan+1);


  //close the original asd file
  sRc = QDAMDictCloseLocal( pQDAMLan->pBTreeRemote );
  /********************************************************************/
  /* lock the original asd dictionary again -- it is unlocked in the  */
  /* closing operation....                                            */
  /* -- do not take care about return code...                         */
  /********************************************************************/
  QDAMDictLockDictLocal( pQDAMLan->pBTreeRemote, TRUE  );

  //rename files
  //if asdclose of the temp files failed or organize failed
  //then pass this on to renameorganize
  if ( pDictOrg->usCloseRc != LX_RC_OK_ASD )
  {
    sRc = pDictOrg->usCloseRc;
  } /* endif */

  /********************************************************************/
  /* return code of above function will be used in RenameOrganize...  */
  /********************************************************************/
  sRc = RenameOrganize( pDictOrg->szDictName, pDictOrg->chPrimDrive, sRc );

  /********************************************************************/
  /* unlock the original asd dictionary                               */
  /* -- do not take care about return code...                         */
  /********************************************************************/
  QDAMDictLockDictLocal( pQDAMLan->pBTreeRemote, FALSE );

  return( sRc);
} /* endQDAMDictCloseOrganizeLocal */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     RenameOrganize
//------------------------------------------------------------------------------
// Function call:     RenameOrganize( PSZ pszDictPath, CHAR chPrimDrive,
//                                    USHORT usCloseRc )
//------------------------------------------------------------------------------
// Description:       rename all the newly created temp files to the
//                    original file names
//------------------------------------------------------------------------------
// Input parameter:   PSZ pszOriginalAsdPath
// Parameters:        CHAR chRemotePrimaryDrive
//                    USHORT usOrganizeCloseRc
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       0 - successful
//------------------------------------------------------------------------------
// Function flow:     build temporary property file path with ext .TPR
//                    build temporary asd copy file with ext .TSC
//                    move passed szDictPath file to asd copy as a savety
//                      measure
//                    build original asi file path
//                    build temporty asi copy file path with ext .TIC
//                    move original asi to asi copy again as savety measure
//                      disregarding the return code as original asi need not
//                      necessarily be available
//                    if all goes well move temporary asd to original asd
//                      and temprary asi to original asi
//                      delete copy asd, asi and property copy
//                    if things go wrong delete the temp files and keep the
//                      originals
//------------------------------------------------------------------------------

USHORT RenameOrganize( PSZ pszDictPath, CHAR chPrimDrive, USHORT usCloseRc )
{
  USHORT  usRc = 0;                        //return code
  CHAR    szOriginalAsd[MAX_EQF_PATH];     //copy asd file name
  CHAR    szOriginalAsi[MAX_EQF_PATH];     //copy asi file name
  CHAR    szCopyAsd[MAX_EQF_PATH];     //copy asd file name
  CHAR    szCopyAsi[MAX_EQF_PATH];     //copy asi file name
  CHAR    szTempProps[MAX_EQF_PATH];   //temp props file name
  CHAR    szTempFile[MAX_EQF_PATH];    //temp file name

  //build property path
  sprintf( szTempProps, "%c:\\%s\\%s\\",
           chPrimDrive, PATH, COMPROPDIR );
  Utlstrccpy( szTempProps + strlen(szTempProps),
              UtlGetFnameFromPath( pszDictPath ), DOT );
  strcat( szTempProps, EXT_TMP_DICTPROP );

  if ( usCloseRc == LX_RC_OK_ASD)  //all previous qdam close/unlock went well
  {
    //build temporary copy of original dictionary asd
    Utlstrccpy( szCopyAsd, pszDictPath, DOT );
    strcat( szCopyAsd, EXT_TMP_DIC_COPY );

    //rename original and give temp copy
    //first asd
    strcpy( szOriginalAsd, pszDictPath );
    usRc = UtlMove( szOriginalAsd, szCopyAsd, 0L, FALSE );
    if ( !usRc )
    {
      //then asi
      Utlstrccpy( szCopyAsi, pszDictPath, DOT );
      strcat( szCopyAsi, EXT_TMP_DICTINDEX_COPY );
      Utlstrccpy( szOriginalAsi, pszDictPath, DOT );
      strcat( szOriginalAsi, EXT_OF_DICTINDEX );

      UtlMove( szOriginalAsi, szCopyAsi, 0L, FALSE );
    } /* endif */

    if ( !usRc )
    {
      //rename newly organized and give original name
      Utlstrccpy( szTempFile, szOriginalAsd, DOT );
      strcat( szTempFile, EXT_TMP_DIC );

      //rename original and give temp copy
      //first asd
      usRc = UtlMove( szTempFile, szOriginalAsd, 0L, FALSE );
      if ( !usRc )
      {
        //then asi
        Utlstrccpy( szTempFile, szOriginalAsi, DOT );
        strcat( szTempFile, EXT_TMP_DICTINDEX );

        usRc = UtlMove( szTempFile, szOriginalAsi, 0L, FALSE );
      } /* endif */

      if ( !usRc )
      {
        //delete temp asd and asi copy, and temp prop file as all went well
        UtlDelete( szCopyAsd, 0L, FALSE );
        UtlDelete( szCopyAsi, 0L, FALSE );
        UtlDelete( szTempProps, 0L, FALSE );
      }
      else
      {
        //renames failed so rename original to temp copy
        //first asd
        usRc = UtlMove( szCopyAsd, szOriginalAsd, 0L, FALSE );
        if ( !usRc )
        {
          //then asi
          usRc = UtlMove( szCopyAsi, szOriginalAsi, 0L, FALSE );
          if ( !usRc )
            //delete newly organized file - szOrganizedProps is prop path
            DeleteTempFiles( szTempProps, *pszDictPath );
        } /* endif */
      } /* endif */
    }
    else
    {
      //rename of original to temp copy failed
      DeleteTempFiles( szTempProps, *pszDictPath );
      usCloseRc = LX_UNEXPECTED_ASD;  //indication that organize failed
    } /* endif */
  }
  else
  {
    //asdclose of the newly organized dict failed or something went wrong
    //during organize which lead to a interruption and incomplete new
    //dictionary copy. The original is left as is.
    //The usCloseRc passed into function is passed out again as was
    DeleteTempFiles( szTempProps, *pszDictPath );
  } /* endif */

  if ( !usRc )
    usCloseRc = LX_RC_OK_ASD;
  else
    usCloseRc = LX_UNEXPECTED_ASD;  //indication that something failed

  return( usCloseRc );
} /* endRenameOrganize */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     DeleteTempFiles
//------------------------------------------------------------------------------
// Function call:     DeleteTempFiles( pszTempDict, chDictdrive )
//------------------------------------------------------------------------------
// Description:       deletes the temp asd, asi and pro files similar to
//                    AsdDelete which doesn't exist in server code
//------------------------------------------------------------------------------
// Input parameter:   PSZ  pszTemppropFilePath
// Parameters:        CHAR chTempAsdDriveLetter
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     delete the temp property file
//                    build the temp asd dictionary path
//                    delete the temp asd dictionary
//                    build the temp asi dictionary path
//                    delete the temp asi dictionary
//------------------------------------------------------------------------------
static
VOID DeleteTempFiles
(
   PSZ   pszTempDict,
   CHAR  chDictDrive
)
{
  CHAR    szTempFile[MAX_EQF_PATH];

  //delete temp property file - string passed to function
  UtlDelete( pszTempDict, 0L, FALSE );

  //build temp asd path and delete file
  sprintf( szTempFile, "%c:\\%s\\%s\\",
           chDictDrive, PATH, COMDICTDIR );
  Utlstrccpy( szTempFile + strlen(szTempFile),
              UtlGetFnameFromPath( pszTempDict ), DOT );
  strcat( szTempFile, EXT_TMP_DIC );
  UtlDelete( szTempFile, 0L, FALSE );

  //build temp asi path and delete file
  sprintf( szTempFile, "%c:\\%s\\%s\\",
           chDictDrive, PATH, COMDICTDIR );
  Utlstrccpy( szTempFile + strlen(szTempFile),
              UtlGetFnameFromPath( pszTempDict ), DOT );
  strcat( szTempFile, EXT_TMP_DICTINDEX );
  UtlDelete( szTempFile, 0L, FALSE );
} /* endDeleteTempFiles */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     QDAMDictUpdTimeLocal
//------------------------------------------------------------------------------
// Function call:     QDAMDictUpdTimeLocal( pBTree, &lTime );
//------------------------------------------------------------------------------
// Description:       gets the update time
//------------------------------------------------------------------------------
// Parameters:        PBTREE    pBTIda   pointer to control block
//                    PLONG     plTime   time of last open
//------------------------------------------------------------------------------
// Returncode type:   SHORT
//------------------------------------------------------------------------------
// Returncodes:       sRc = LX_RC_OK_ASD -
//------------------------------------------------------------------------------
// Function flow:     retrieve the update/open time
//                    return
//------------------------------------------------------------------------------
SHORT QDAMDictUpdTimeLocal
(
  PBTREE  pBTIda,
  PLONG   plTime
)
{
  SHORT          sRc = LX_RC_OK_ASD;
  PBTREEGLOB  pBT = pBTIda->pBTree;

  if ( pBT->usOpenFlags & ASD_SHARED )
  {
    sRc = QDAMGetUpdCounter( pBTIda, plTime, 0, 1 );
  }
  else
  {
    *plTime = pBT->lTime;;
  } /*endif */

  return( sRc);
} /* endQDAMDictUpdTimeLocal  */


