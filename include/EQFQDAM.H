/*! EQFQDAM.H
	Description: This is the include file used for the QDAM package
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

USHORT QDAMDeleteFile ( SERVERNAME, PSZ );
/**********************************************************************/
/* NTM prototypes                                                     */
/**********************************************************************/
#define NTMREQUESTNEWKEY 0xFFFFFFFF
//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMCreate
//+----------------------------------------------------------------------------+
// Function call:     EQFNTMCreate( pTMName, pUserData, usLen, ulStart, &pNTM);
//+----------------------------------------------------------------------------+
// Description:       This function will create the appropriate Transl.Memory
//                    file.
//+----------------------------------------------------------------------------+
// Parameters:        PSZ  pTMName,         name of file to be created
//                    PCHAR  pUserData,     user data
//                    USHORT usLen,         length of user data
//                    ULONG  ulStartKey,    first key to start automatic insert
//                    PBTREE * ppBTIda      pointer to structure
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_NO_ROOM     memory shortage
//                    BTREE_USERDATA    user data too long
//                    BTREE_OPEN_ERROR  dictionary already exists
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//+----------------------------------------------------------------------------+
SHORT
EQFNTMCreate
(
   PSZ  pTMName,                       // name of file to be created
   PCHAR  pUserData,                   // user data
   USHORT usLen,                       // length of user data
   ULONG  ulStartKey,                  // first key to start automatic insert...
   PVOID  * ppBTIda                    // pointer to structure
);

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMOpen     Open a Translation Memory file
//+----------------------------------------------------------------------------+
// Function call:     EQFNTMOpen( PSZ, USHORT, PPBTREE );
//+----------------------------------------------------------------------------+
// Description:       Open a file locally for processing
//+----------------------------------------------------------------------------+
// Parameters:        PSZ              name of the index file
//                    USHORT           open flags
//                    PPBTREE          pointer to btree structure
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_NO_ROOM     memory shortage
//                    BTREE_OPEN_ERROR  dictionary already exists
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_ILLEGAL_FILE not a valid dictionary
//                    BTREE_CORRUPTED   dictionary is corrupted
//+----------------------------------------------------------------------------+

SHORT  EQFNTMOpen
(
  PSZ   pName,                        // name of the file
  USHORT usOpenFlags,                 // Read Only or Read/Write
  PVOID  * ppBTIda                    // pointer to BTREE structure
);

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMClose  close the TM file
//+----------------------------------------------------------------------------+
// Function call:     EQFNTMClose( PPBTREE );
//+----------------------------------------------------------------------------+
// Description:       Close the file
//+----------------------------------------------------------------------------+
// Parameters:        PPBTREE                pointer to btree structure
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     incorrect pointer
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_CLOSE_ERROR error closing dictionary
//+----------------------------------------------------------------------------+

SHORT EQFNTMClose
(
  PVOID * ppBTIda
);

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMUpdSign  Write User Data
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictUpdSignLocal( PBTREE, PCHAR, USHORT );
//
//+----------------------------------------------------------------------------+
// Description:       Writes the second part of the first record (user data)
//                    This is done using the original QDAMDictUpdSignLocal
//                    function
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to user data
//                    USHORT                 length of user data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_INVALID     pointer invalid
//                    BTREE_USERDATA    user data too long
//                    BTREE_CORRUPTED   dictionary is corrupted
//+----------------------------------------------------------------------------+
// NOTE:              This function could be implemented as MACRO too, but
//                    for consistency reasons, the little overhead was used...
//+----------------------------------------------------------------------------+

SHORT EQFNTMUpdSign
(
   PVOID  pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   USHORT usLen                        // length of user data
);

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMSign      Read signature record
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictSignLocal( PBTREE, PCHAR, PUSHORT );
//+----------------------------------------------------------------------------+
// Description:       Gets the second part of the first record ( user data )
//                    This is done using the original QDAMDictSignLocal func.
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to user data
//                    PUSHORT                length of user data area (input)
//                                           filled length (output)
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_INVALID     pointer invalid
//                    BTREE_USERDATA    user data too long
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//
//+----------------------------------------------------------------------------+
// Side effects:      return signature record even if dictionary is corrupted
//+----------------------------------------------------------------------------+
// NOTE:              This function could be implemented as MACRO too, but
//                    for consistency reasons, the little overhead was used...
//+----------------------------------------------------------------------------+

__declspec(dllexport)
SHORT EQFNTMSign
(
   PVOID  pBTIda,                      // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   PUSHORT pusLen                      // length of user data
);

SHORT
EQFNTMInsert
(
  PVOID   pBTIda,           // pointer to binary tree struct
  PULONG  pulKey,           // pointer to key
  PBYTE   pData,            // pointer to user data
  ULONG   ulLen             // length of user data
);


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMInsert
//+----------------------------------------------------------------------------+
// Function call:     sRc = EQFNTMInsert( pBTIda, &ulKey, pData, usLen );
//+----------------------------------------------------------------------------+
// Description:       insert a new key (ULONG) with data
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE  pBTIda,      pointer to binary tree struct
//                    PULONG  pulKey,      pointer to key
//                    PBYTE   pData,       pointer to user data
//                    ULONG   ulLen        length of user data
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       BTREE_NUMBER_RANGE   requested key not in allowed range
//                    BTREE_READONLY       file is opened read only - no write
//                    BTREE_CORRUPTED      file is corrupted
//                    errors returned by QDAMDictInsertLocal
//                    0                    success indicator
//+----------------------------------------------------------------------------+
SHORT
EQFNTMInsert
(
  PVOID   pBTIda,           // pointer to binary tree struct
  PULONG  pulKey,           // pointer to key
  PBYTE   pData,            // pointer to user data
  ULONG   ulLen             // length of user data
);

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMUpdate
//+----------------------------------------------------------------------------+
// Function call:     sRc = EQFNTMUpdate( pBTIda,  ulKey, pData, usLen );
//+----------------------------------------------------------------------------+
// Description:       update the data of an already inserted key
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE  pBTIda,      pointer to binary tree struct
//                    ULONG   ulKey,      key value
//                    PBYTE   pData,       pointer to user data
//                    ULONG   ulLen        length of user data
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       BTREE_NUMBER_RANGE   requested key not in allowed range
//                    BTREE_READONLY       file is opened read only - no write
//                    BTREE_CORRUPTED      file is corrupted
//                    errors returned by QDAMDictInsertLocal
//                    0                    success indicator
//+----------------------------------------------------------------------------+
SHORT
EQFNTMUpdate
(
  PVOID   pBTIda,           // pointer to binary tree struct
  ULONG   ulKey,            // key value
  PBYTE   pData,            // pointer to user data
  ULONG   ulLen             // length of user data
);

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMGet
//+----------------------------------------------------------------------------+
// Function call:     sRc = EQFNTMGet( pBTIda, ulKey, chData, &usLen );
//+----------------------------------------------------------------------------+
// Description:       get the data string for the passed key
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE pBTIda,       pointer to btree struct
//                    ULONG  ulKey,        key to be searched for
//                    PCHAR  pchBuffer,    space for user data
//                    PULONG  pulLength    in/out length of returned user data
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       same as for QDAMDictExactLocal...
//+----------------------------------------------------------------------------+
SHORT
EQFNTMGet
(
   PVOID  pBTIda,                      // pointer to btree struct
   ULONG  ulKey,                       // key to be searched for
   PCHAR  pchBuffer,                   // space for user data
   PULONG  pulLength                   // in/out length of returned user data
);


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMGetMaxNumber
//+----------------------------------------------------------------------------+
// Function call:     sRc = EQFNTMGetNextNumber( pBTIda, &ulKey, &ulNextFree );
//+----------------------------------------------------------------------------+
// Description:       get the start key and the next free key ...
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE pBTIda,       pointer to btree struct
//                    PULONG pulStartKey   first key
//                    PULONG pulNextKey    next key to be assigned
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0     always
//+----------------------------------------------------------------------------+
// Function flow:     access data from internal structure
//+----------------------------------------------------------------------------+
SHORT
EQFNTMGetNextNumber
(
   PVOID  pBTIda,                      // pointer to btree struct
   PULONG pulStartKey,                 // return start key number
   PULONG pulNextKey                   // return next key data
);



//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMInit
//+----------------------------------------------------------------------------+
// Function call:     QDAMInit();
//+----------------------------------------------------------------------------+
// Description:       init data areas for QDAM functions
//+----------------------------------------------------------------------------+
// Parameters:        VOID
//+----------------------------------------------------------------------------+
// Returncode type:   VOID
//+----------------------------------------------------------------------------+
__declspec(dllexport)
VOID  QDAMInit( VOID );

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMPhysLock
//+----------------------------------------------------------------------------+
// Function call:     sRc = EQFNTMPhysLock( pBTIda );
//+----------------------------------------------------------------------------+
// Description:       Physicall lock or unlock database.
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE             The database to be locked
//                    BOOL               TRUE = LOCK, FALSE = Unlock
//                    PBOOL              ptr to locked flag (set to TRUE if
//                                       locking was successful
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
SHORT EQFNTMPhysLock
(
   PVOID          pBTIda,
   BOOL           fLock,
   PBOOL          pfLocked
);

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     QDAMIncrUpdCounter      Inrement database update counter
//+----------------------------------------------------------------------------+
// Function call:     QDAMIncrUpdCounter( PBTREE, SHORT sIndex )
//
//+----------------------------------------------------------------------------+
// Description:       Update one of the update counter field in the dummy
//                    /locked terms file
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    SHORT                  index of counter field
//                                                                       PLONG                                                                  ptr to buffer for new counte value
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//
//+----------------------------------------------------------------------------+
// Function flow:     read update counter from dummy file
//                    increment update counter
//                    position ptr to begin of file
//                    write update counter to disk
//+----------------------------------------------------------------------------+
SHORT EQFNTMIncrUpdCounter
(
   PVOID      pBTIda,                  // pointer to btree structure
   SHORT      sIndex,                  // index of update counter
   PLONG                         plNewValue                                               // ptr to buffer for new counte value
);

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     EQFNTMGetUpdCounter       Get database update counter
//+----------------------------------------------------------------------------+
// Function call:     EQFNTMGetUpdCounter( PBTREE, PLONG, SHORT, SHORT );
//+----------------------------------------------------------------------------+
// Description:       Get one or more of the the database update counters
//                    from the dummy/locked terms file
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PLONG                  ptr to buffer for update counter
//                    SHORT                  index of requested update counter
//                    SHORT                  number of counters requested
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
SHORT EQFNTMGetUpdCounter
(
   PVOID      pBTIda,                   // pointer to btree structure
   PLONG      plUpdCount,               // ptr to buffer for update counter
   SHORT      sIndex,                   // index of requested update counter
   SHORT      sNumCounters              // number of counters requested
);

// the following define moved from eqfqdami.h since it must be known in
// eqfdorg.c
/**********************************************************************/
/* To check that we are opening a valid B-tree file, there is a       */
/* magic cookie stored at the beginning.  This is as follows          */
/* This value might be changed to include version identifications...  */
/**********************************************************************/
#define BTREE_VERSION       0
#define BTREE_VERSION2      2
#define BTREE_VERSION3      3

// function to organize index file
USHORT EQFNTMOrganizeIndex
(
   PVOID          *ppBTIda,            // ptr to BTREE being organized
   USHORT         usOpenFlags,         // open flags to be used for index file
   ULONG          ulStartKey           // first key to start automatic insert...
);
