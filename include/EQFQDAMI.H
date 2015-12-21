/*! EQFQDAMI.H
	Description: internal header file for QDAM
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

#define BTREE_REC_SIZE_V2  (4096)          // record size V2
#define BTREE_REC_SIZE_V3 (16384)          // record size V3

#define BTREE_BUFFER_V2   (BTREE_REC_SIZE_V2 + 10*sizeof(USHORT)) // buffer size
#define BTREE_BUFFER_V3   (BTREE_REC_SIZE_V3 + 10*sizeof(USHORT)) // buffer size

#define MAX_LIST           20          // number of recently used records
#define MIN_SIZE          128          // minimum free space requested in entry
#define MAXWASTESIZE      512          // max size  to be wasted due to updates
#define MAXDATASIZE    0x8000          // maximum data size allowed
#define MAX_LOCKREC_SIZE 4000          // max size for locked terms record
#define MAX_UPD_CTR        10          // max number of update counters
#define HEADTERM_SIZE     256          // size of the head term
#define COLLATE_SIZE      256          // size of the collating sequence
#define ENTRYENCODE_LEN    15          // number of significant characters
#define ENTRYDECODE_LEN    32          // length of decoding array
#define USERDATA_START   2046          // start of user data
#define INDEX_BUFFERS      20          // index buffers
#define NUMBER_OF_BUFFERS  20          // 20 // number of buffers to be used
#define MAX_NUM_DICTS      99          // max. number of dict concurrently open

#define RESET_VALUE       -3

#define RETRY_COUNT        5           // number of retries for BTREE_IN_USE condition

// max time [ms] to wait for resources currently in use
#define MAX_WAIT_TIME     100

// max number of retries for resources currently in use
#define MAX_RETRY_COUNT    30

/**********************************************************************/
/* defines for the tersing to be used....                             */
/**********************************************************************/
#define BTREE_TERSE_HUFFMAN  1
#define BTREE_TERSE_LZSSHMAN 2
#define  QDAM_TERSE_FLAG   0x8000
#define  QDAM_TERSE_FLAGL   0x80000000L
/**********************************************************************/
/* To check that we are opening a valid B-tree file, there is a       */
/* magic cookie stored at the beginning.  This is as follows          */
/* This value might be changed to include version identifications...  */
/**********************************************************************/
//#define BTREE_VERSION       0   // moved to EQFQDAM.h
//#define BTREE_VERSION2      2   // moved to EQFQDAM.h
//#define BTREE_VERSION3      3   // moved to EQFQDAM.h
#define BTREE_HEADER_VALUE_V0 "EQF "
#define BTREE_HEADER_VALUE_V1 "EQF"
#define BTREE_HEADER_VALUE_V2 "EQF"
#define BTREE_HEADER_VALUE_V3 "EQF"

#define NTM_VERSION         1
#define BTREE_HEADER_VALUE_TM1 "NTM"
// BTREE databases of version NTM_VERSION2 support data records with
// a size of more than 32k, the length field at the begin of the
// data record is of type ULONG instead of USHORT
#define NTM_VERSION2       2
#define BTREE_HEADER_VALUE_TM2 "NTM"

#define NTM_VERSION3       3
#define BTREE_HEADER_VALUE_TM3 "NTM"

typedef enum _DICTCMD
{
  QDAMDICTOPEN,               // dictionary open
  QDAMDICTCREATE,             // dictionary create
  QDAMDICTCLOSE,              // dictionary close
  QDAMDICTSIGN,               // return signature record
  QDAMDICTUPDSIGN,            // update signature record
  QDAMDICTSUBSTR,             // find term containing substring
  QDAMDICTEQUIV,              // find equivalent term
  QDAMDICTEXACT,              // find exact term
  QDAMDICTNEXT,               // return next term
  QDAMDICTPREV,               // return prev term
  QDAMDICTCURR,               // return current term
  QDAMDICTINSERT,             // insert term
  QDAMDICTUPDATE,             // update term
  QDAMDICTDELETE,             // delete term
  QDAMDICTFLUSH,              // resynchronize dict.
  QDAMDICTCOPY,               // copy dict
  QDAMDICTNUMENTRIES,         // find number of terms in dict
  QDAMDICTFIRST,              // position at first entry
  QDAMDICTLOCKDICT,           // lock the dictionary
  QDAMDICTLOCKENTRY,          // lock the entry
  QDAMDICTCLOSEORGANIZE,      // end the organize
  QDAMDICTUPDTIME             // get update status
} DICTCMD;

typedef enum _RECTYPE
{
  DATAREC,                 // record contains data
  KEYREC,                  // record contains keys
  ROOTREC                  // record contains root key data
} RECTYPE;

typedef enum _SEARCHTYPE
{
  FEXACT,                  // exact match requested
  FSUBSTR,                 // only substring match
  FEQUIV                   // equivalent match
} SEARCHTYPE;

  typedef struct _RECPARAM
  {
    USHORT  usNum;           // record number
    USHORT  usOffset;        // record offset
    ULONG   ulLen;           // record length
  }  RECPARAM, * PRECPARAM;
  typedef struct _RECPARAMOLD
  {
    USHORT  usNum;           // record number
    USHORT  usOffset;        // record offset
    SHORT  sLen;             // record length
  }  RECPARAMOLD, * PRECPARAMOLD;

typedef struct _STENCODEBITS
{
  USHORT usLen;
  USHORT usVal;
} STENCODEBITS;
/**********************************************************************/
/*  structure for Client/Server access                                */
/**********************************************************************/
typedef struct  _QDAMLAN
{
   union PREFINOUT
   {
     PREFIX_IN  prefin;                           // prefix in of each command
     PREFIX_OUT prefout;                          // prefix out of each command
   }          PrefInOut;
   DICTCMD    DictCmd;                            // dictionary command
   struct       _BTREEIDA  *  pBTreeRemote;       // pointer to remote BTREE
   USHORT     usOutLen;                           // length of the output struct
   CHAR       szUserId [MAX_USERID];              // logged on userid
} QDAMLAN , * PQDAMLAN;




/*****************************************************************************/
/* Used to determine amount of padding required for the Btree record so      */
/* that the header + keys fix in to a single disk record                     */
/*****************************************************************************/
typedef struct _BTREEHEADER
{
  CHAR    chType;                                 // record type
  USHORT  usNum;                                  // record number
  USHORT  usPrevious;                             // previous leaf node
  USHORT  usNext;                                 // next leaf node
  USHORT  usParent;                               // parent node
  USHORT  usOccupied;                             // # of keys in record
  USHORT  usFilled;                               // number of bytes filled
  USHORT  usLastFilled;                           // ptr. to next free byte
  USHORT  usWasteSize;                            // waste size?
} BTREEHEADER , * PBTREEHEADER;


/*****************************************************************************/
/* BTREERECORD  is the format of each of the blocks on the disk.             */
/*  This structure should be BTREE_REC_SIZE long                             */
/*****************************************************************************/

#define FREE_SIZE_V3  (BTREE_REC_SIZE_V3 - sizeof(BTREEHEADER))
typedef struct _BTREERECORD_V3
{
  BTREEHEADER  header;                             // 16 bytes header
  UCHAR        uchData[ FREE_SIZE_V3 ] ;              // free size to be used
} BTREERECORD_V3, *PBTREERECORD_V3, **PPBTREERECORD_V3;

#define FREE_SIZE_V2  (BTREE_REC_SIZE_V2 - sizeof(BTREEHEADER))
typedef struct _BTREERECORD_V2
{
  BTREEHEADER  header;                             // 16 bytes header
  UCHAR        uchData[ FREE_SIZE_V2 ] ;              // free size to be used
} BTREERECORD_V2, *PBTREERECORD_V2, **PPBTREERECORD_V2;



/*****************************************************************************/
/* BTREEBUFFER  is the format of the buffers when read in to the buffer      */
/* cache.  It maps a record number and its properties to its contents        */
/*****************************************************************************/
typedef struct _BTREEBUFFER_V2
{
  USHORT usRecordNumber;                           // index of rec in buffer
  BOOL   fLocked;                                  // Is the record locked ?
  BOOL   fNeedToWrite;                             // Commit before reuse
  SHORT  sUsed;                                    // buffer used count
  ULONG  ulCheckSum;                               // CheckSum of contents data
  BTREERECORD_V2 contents;                         // data from disk
} BTREEBUFFER_V2, * PBTREEBUFFER_V2;

typedef struct _BTREEINDEX_V2
{
  struct _BTREEINDEX_V2  * pNext;                    // point to next index buffer
  BTREEBUFFER_V2  btreeBuffer;                       // data from disk
} BTREEINDEX_V2, * PBTREEINDEX_V2;

typedef struct _BTREEBUFFER_V3
{
  USHORT usRecordNumber;                           // index of rec in buffer
  BOOL   fLocked;                                  // Is the record locked ?
  BOOL   fNeedToWrite;                             // Commit before reuse
  SHORT  sUsed;                                    // buffer used count
  ULONG  ulCheckSum;                               // CheckSum of contents data
  BTREERECORD_V3 contents;                            // data from disk
} BTREEBUFFER_V3, *PBTREEBUFFER_V3;

typedef struct _BTREEINDEX_V3
{
  struct _BTREEINDEX_V3 * pNext;                    // point to next index buffer
  BTREEBUFFER_V3  btreeBuffer;                       // data from disk
} BTREEINDEX_V3, * PBTREEINDEX_V3;

/*****************************************************************************/
/* BTree is the memory resident structure describing the current state of    */
/* the B tree.  It contains the house keeping information for all of the     */
/* functions                                                                 */
/*****************************************************************************/

typedef SHORT _PFN_QDAMCOMPARE( PVOID, PVOID, PVOID );
typedef _PFN_QDAMCOMPARE *PFN_QDAMCOMPARE;

///* Maximum size (entries) of the lookup table (it is a USHORT).
#define MAX_NUMBER_OF_LOOKUP_ENTRIES 0x0FFF0
/* Initial size (entries) of the lookup table */
#define MIN_NUMBER_OF_LOOKUP_ENTRIES 32
/* Every MAX_READREC_CALLS calls to QDAMReadRecord unlocked records with
   access counter < MAX_READREC_CALLS will be written to disk and memory
   will be freed.
   Every read access of a record will increase the access counter of that
   record by ACCESSBONUSPOINTS */
//#ifndef _WINDOWS
  #define MAX_READREC_CALLS 1000000L
  #define ACCESSBONUSPOINTS 5000L
//#else
//  /* To avoid swapping under Windows the values are very low */
//  #define MAX_READREC_CALLS 2000L
//  #define ACCESSBONUSPOINTS 100L
//#endif

typedef struct _LOOKUPENTRY_V2
{
  PBTREEBUFFER_V2 pBuffer; // Pointer to BTREEBUFFER
} LOOKUPENTRY_V2, * PLOOKUPENTRY_V2;

typedef struct _LOOKUPENTRY_V3
{
  PBTREEBUFFER_V3 pBuffer; // Pointer to BTREEBUFFER
} LOOKUPENTRY_V3, *PLOOKUPENTRY_V3;

typedef struct _ACCESSCOUNTERTABLEENTRY
{
  ULONG ulAccessCounter;
} ACCESSCTRTABLEENTRY, * PACCESSCTRTABLEENTRY;


typedef struct _BTREEGLOB
{
   HFILE        fp;                               // index file handle
   HTM          htm;                              // handle in remote case

   USHORT       usFirstNode;                      // file pointer of record
   USHORT       usFirstLeaf;                      // file pointer of record

   PBTREEINDEX_V2  pIndexBuffer_V2;               // Pointer to index records
   PBTREEINDEX_V3  pIndexBuffer_V3;               // Pointer to index records
   USHORT       usIndexBuffer;                    // number of index buffers
   PFN_QDAMCOMPARE compare;                       // Comparison function
   USHORT       usNextFreeRecord;                 // Next record to expand to
   CHAR         chFileName[144];                  // Name of B-tree file
   RECPARAM     DataRecList[ MAX_LIST ];          // last used data records
   BOOL         fGuard;                           // write every record
   BOOL         fOpen;                            // open flag
   BOOL         fCorrupted;                       // mark as corrupted
   PCHAR_W      pTempKey;                         // pointer to temp. key
   PBYTE        pTempRecord;                      // pointer to temp record
   ULONG        ulTempRecSize;                    // size of temp record area
   BOOL         fTerse;                           // tersing requested
   BYTE         chEntryEncode[ ENTRYENCODE_LEN];  // significant characters
   BYTE         bEncodeLen[COLLATE_SIZE];         // encoding table length
   CHAR         chEncodeVal[COLLATE_SIZE];        // encoding table
   BYTE         chDecode[ENTRYDECODE_LEN];        // decoding table
   BYTE         chCollate[COLLATE_SIZE];          // collating sequence to use
   BYTE         chCaseMap[COLLATE_SIZE];          // case mapping to be used
   USHORT       usFreeKeyBuffer;                  // index of buffer to use
   USHORT       usFreeDataBuffer;                 // first data buffer chain
   USHORT       usFirstDataBuffer;                // first data buffer
   BTREEBUFFER_V2  BTreeTempBuffer_V2;            // temporary 4k buffer
   BTREEBUFFER_V3  BTreeTempBuffer_V3;            // temporary V3 buffer
   BOOL         fWriteHeaderPending;              // write of header pending
   LONG         lTime;                            // time of last update/open
   USHORT       usVersion;                        // version identification...
   CHAR         chEQF[7];                         // The type of file
   BYTE         bVersion;                         // version flag
   BOOL         fTransMem;                        // translation memory???
   USHORT       usOpenFlags;                      // settings used for open
   LONG         alUpdCtr[MAX_UPD_CTR];            // list of update counters
   HFILE        fpDummy;                          // dummy/lock semaphore file handle
   BOOL         fUpdated;                         // database-has-been-modified flag
   USHORT       usNumberOfLookupEntries;    // Number of allocated lookup-table-entries
   USHORT       usNumberOfAllocatedBuffers; // Number of allocated buffers
   ULONG        ulReadRecCalls;             // Number of calls to QDAMReadRecord
   PLOOKUPENTRY_V2 LookupTable_V2;                // Pointer to lookup-table
   PLOOKUPENTRY_V3 LookupTable_V3;                // Pointer to lookup-table
   PACCESSCTRTABLEENTRY AccessCtrTable;     // Pointer to access-counter-table
   USHORT       usBtreeRecSize;                   // size of BTREE records
   BYTE         bRecSizeVersion;                  // record size version flag
} BTREEGLOB, * PBTREEGLOB, ** PPBTREEGLOB ;

typedef struct _BTREEIDA
{
   HTM          htm;                              // handle in remote case
   BOOL         fRemote;                          // we are dealing with remote
   SHORT        sCurrentIndex;                    // current sequence array
   USHORT       usCurrentRecord;                  // current sequence record
   PBTREEGLOB   pBTree;                           // pointer to global struct
   USHORT       usDictNum;                        // index in global structure
   CHAR_W       chHeadTerm[HEADTERM_SIZE];        // last active head term
   BOOL         fLock;                            // head term is locked
   CHAR_W       chLockedTerm[HEADTERM_SIZE];      // locked term if any
   CHAR         chServer[ MAX_SERVER_NAME + 1];   // server name
   PQDAMLAN     pQDAMLanIn;                       // pointer to buffer for LAN
   PQDAMLAN     pQDAMLanOut;                      // pointer to buffer for LAN
   struct       _BTREEIDA  *  pBTreeRemote;       // pointer to remote BTREE
   CHAR         szUserId [MAX_USERID];            // logged on userid
   BOOL         fPhysLock;                        // db has been physically locked
   CHAR         szFileName[MAX_LONGPATH];         // fully qualified name of file
 } BTREE, * PBTREE, ** PPBTREE;

typedef struct _QDAMDICT
{
   PBTREEGLOB   pBTree;                           // pointer to global struct
   ULONG        ulNum;
   USHORT       usNextHandle;
   USHORT       usOpenCount;                      // number of accesses...
   USHORT       usOpenRC;
   CHAR         chDictName[ MAX_EQF_PATH ];
   BOOL         fDictLock;                        // is dictionary locked
   PBTREE       pIdaList[ MAX_NUM_DICTS ];        // number of instances ...
} QDAMDICT, *PQDAMDICT;

/*****************************************************************************/
/* BTREEHEADRECORD is the record at the start of a B-tree file.  It is used  */
/* to confirm that the file is a valid one; it holds  database wide          */
/* persistent information                                                    */
/*****************************************************************************/
typedef struct _BTREEHEADRECORD
{
  CHAR      chEQF[7];                             // The type of file
//  BYTE      bVersion;                             // version flag
  struct
  {
   unsigned char bVersion     : 4;                // does the attribute contain text to be translated?
   unsigned char free1        : 1;                // unused
   unsigned char free2        : 1;                // unused
   unsigned char free3        : 1;                // unused
   unsigned char f16kRec      : 1;                // TRUE = BTREE uses 16k Recs (V3 record size)
  } Flags;

  EQF_BOOL  fOpen;                                // open flag/corruption flag
  USHORT    usFirstNode;                          // first node record
  USHORT    usFirstLeaf;                          // first leaf record
   RECPARAMOLD  DataRecList[ MAX_LIST ];          // last used data records
  EQF_BOOL  fTerse;                               // tersing requested
  BYTE      chCollate[COLLATE_SIZE];              // collating sequence to use
  BYTE      chCaseMap[COLLATE_SIZE];              // case mapping to be used
  BYTE      chEntryEncode[ ENTRYENCODE_LEN ];     // significant characters
  USHORT    usFreeKeyBuffer;                      // index of buffer to use
  USHORT    usFreeDataBuffer;                     // first data buffer chain
  USHORT    usFirstDataBuffer;                    // first data buffer
  ULONG     ulUpdCount;                           // last update counter
  USHORT    usNextFreeRecord;                     // Next record to expand to
} BTREEHEADRECORD, PBTREEHEADRECORD ;

// Value for chVersion in older databses (intitial version)
#define BTREE_V0 0x00

// Value for chVersion in second BTREE version
// Note: In this version the usNextFreeRecord field was introduced. As the
//       part of the header record containing this field was not initialized
//       in the older version the contents of the field has to be ignored if
//       chVersion is BTREE_V0.
#define BTREE_V1 0x01

// Value for chVersion in BTREE version 2
// Note: In this version the 32k length limit of data records was eliminated
#define BTREE_V2 0x02

// Value for chVersion in BTREE version 3
// Note: In this version the record size has been changed to 16k
#define BTREE_V3 0x03


/**********************************************************************/
/* typedef used for all vital information on our new approach...      */
/* NOTE: this structure is limited to COLLATE_SIZE, which is 256      */
/**********************************************************************/
typedef struct _TMVITALINFO
{
  ULONG   ulStartKey;                  // key to start with
  ULONG   ulNextKey;                   // currently active key
} NTMVITALINFO, * PNTMVITALINFO;

#define NTMNEXTKEY( pBT )  ((PNTMVITALINFO)(&pBT->chCollate[0]))->ulNextKey
#define NTMSTARTKEY( pBT ) ((PNTMVITALINFO)(&pBT->chCollate[0]))->ulStartKey



/* Access Macros */
#define TYPE(x)          ((x)->contents.header.chType)
#define LEAF_NODE        0x00
#define INNER_NODE       0x01
#define ROOT_NODE        0x02
#define DATA_NODE        0x04
#define DATA_NEXTNODE    0x08
#define DATA_KEYNODE     0x10

#define IS_LEAF(x)          ((TYPE(x) & INNER_NODE) == 0)
#define IS_ROOT(x)          ((TYPE(x) & ROOT_NODE) != 0)

#define PREV(x)             ((x)->contents.header.usPrevious)
#define NEXT(x)             ((x)->contents.header.usNext)
#define PARENT(x)           ((x)->contents.header.usParent)
#define RECORDNUM(x)        ((x)->usRecordNumber)
#define OCCUPIED(x)         ((x)->contents.header.usOccupied)
#define FILLEDUP(x)         ((x)->contents.header.usFilled)


#define  MINFREEKEYS        3
// minimum keys to be in a record - otherwise try to reduce the node
#define  MIN_KEY            MINFREEKEYS
// lock a record for fixing it in the memory
#define  BTREELOCKRECORD(x)  ((x)->fLocked=TRUE)
// test if a record is locked
#define  BTREETESTLOCKRECORD(x)  ((x)->fLocked)
// unlock a (previously locked) record
#define  BTREEUNLOCKRECORD(x)  ((x)->fLocked=FALSE)

/**********************************************************************/
/* Structure used in DAM to QDAM functions                            */
/**********************************************************************/
typedef struct _DAM2QDAM
{
   PBTREE pDamBTree;
   ULONG  ulNum;
   USHORT usNextHandle;
   USHORT usOpenCount;
   USHORT usOpenRC;
   CHAR   chDictName[ MAX_EQF_PATH ];
} DAM2QDAM, *PDAM2QDAM;

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictOpen   Open Dictionary
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictOpen( PSZ, PSZ, SHORT, BOOL, PBTREE * );
//
//+----------------------------------------------------------------------------+
// Description:       Open a file for processing
//
//+----------------------------------------------------------------------------+
// Parameters:        PSZ              name of the index file
//                    PSZ              name of the server
//                    SHORT            number of bytes per record
//                    BOOL             TRUE  read/write FALSE  read/only
//                    PPBTREE          pointer to btree structure
//
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
SHORT  QDAMDictOpen
(
  PSZ   pName,                        // name of the file
  PSZ   pServer,                      // name of the server
  SHORT sNumberOfBuffers,             // number of buffers
  USHORT usOpenFlags,                 // Read Only or Read/Write
  PBTREE * ppBT                       // pointer to BTREE structure
);

typedef struct _DICTOPEN
{
  HTM    htm;                         // pointer to BTREE structure
  SERVERNAME szServer;                // servername
  USHORT usName;                      // offset of name of file in chData
  SHORT  sNumberOfBuffers;            // number of buffers
  USHORT usOpenFlags;                 // Read Only or Read/Write
  BYTE   chVarData;                   // data pointer
} DICTOPEN, * PDICTOPEN;

///////////////////////////////////////////////////////////////////////////////
//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictCreate      Create Dictionary
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictCreate( PSZ, PSZ, SHORT, PCHAR, USHORT, PCHAR,
//                                    PCHAR, PCHAR, PPBTREE);
//+----------------------------------------------------------------------------+
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
//+----------------------------------------------------------------------------+
// Parameters:        PSZ              name of the index file
//                    PSZ              name of the server
//                    SHORT            number of buffers used
//                    PCHAR            pointer to user data
//                    USHORT           length of user data
//                    PCHAR            pointer to term encoding sequence
//                    PCHAR            pointer to collating sequence
//                    PCHAR            pointer to case map structure
//                    PBTREE *         pointer to btree structure
//
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
SHORT QDAMDictCreate
(
   PSZ    pName,                       // name of file
   PSZ    pServer,                     // name of the server
   SHORT  sNumberOfBuffers,            // number of buffers
   PCHAR  pUserData,                   // user data
   USHORT usLen,                       // length of user data
   PCHAR  pTermTable,                  // term encoding table
   PCHAR  pCollate,                    // pointer to collating sequence
   PCHAR  pCaseMap,                    // pointer to casemap sequence
   PBTREE * ppBT                       // pointer to btree structure
);
typedef struct _DICTCREATE
{
   HTM    htm;                         // pointer to BTREE structure
   SERVERNAME szServer;                // servername
   USHORT usName;                      // name of the file
   SHORT  sNumberOfBuffers;            // number of buffers
   USHORT usUserData;                  // user data
   USHORT usLen;                       // length of user data
   USHORT usTermTable;                 // term encoding table
   USHORT usCollate;                   // offset to collating sequence
   USHORT usCaseMap;                   // offset to casemap sequence
   BYTE   chVarData;                   // data pointer
} DICTCREATE, *PDICTCREATE;

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictClose    close the dictionary
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictClose( PPBTREE );
//
//+----------------------------------------------------------------------------+
// Description:       Close the file
//
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
SHORT QDAMDictClose
(
   PPBTREE ppBT
);

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictSign    Read User Data
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictSign( PBTREE, PCHAR, PUSHORT );
//
//+----------------------------------------------------------------------------+
// Description:       Gets the second part of the first record ( user data )
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to user data
//                    PUSHORT                length of user data area (input)
//                                           filled length (output)
//
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
SHORT QDAMDictSign
(
   PBTREE pBT,                         // pointer to btree structure
   PCHAR  pUserData,                   // pointer to user data
   PUSHORT pusLen                      // length of user data
);

typedef struct _DICTSIGN
{
   USHORT usUserData;                  // pointer to user data
   USHORT usLen;                       // length of user data
   BYTE   chVarData;                   // data pointer
} DICTSIGN, * PDICTSIGN;

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictInsert     Insert entry
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictInsert( PBTREE, PCHAR, PCHAR, USHORT );
//
//+----------------------------------------------------------------------------+
// Description:       Add a key and all associated data.
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE          pointer to btree structure
//                    PCHAR           key to be inserted
//                    PCHAR           user data to be associated with the key
//                    USHORT          length of the user data
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
//
//+----------------------------------------------------------------------------+
// Function flow:     if BTree does not exist
//                      set Rc = BTREE_INVALID
//                    else
//                      depending on type ( remote or local) call the
//                      appropriate routine
//                    endif
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictInsert
(
  PBTREE  pBT,               // pointer to binary tree struct
  PCHAR_W   pKey,              // pointer to key data
  PBYTE   pData,             // pointer to user data
  ULONG   ulLen              // length of user data
);

typedef struct _DICTINSERT
{
  USHORT  usKey;             // pointer to key data
  USHORT  usData;            // pointer to user data
  ULONG   ulLen;             // length of user data
  BYTE    chVarData;         // data pointer
} DICTINS, * PDICTINS;

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictExact      Find Exact match
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictExact( PBTREE, PCHAR, PCHAR, PUSHORT );
//
//+----------------------------------------------------------------------------+
// Description:        Find an exact match for the passed key
//
//+----------------------------------------------------------------------------+
// Parameters:         PBTREE               pointer to btree structure
//                     PCHAR                key to be inserted
//                     PCHAR                buffer for user data
//                     PUSHORT              on input length of buffer
//                                          on output length of filled data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//+----------------------------------------------------------------------------+
SHORT QDAMDictExact
(
   PBTREE   pBT,                         // pointer to btree struct
   PCHAR_W  pKey,                        // key to be searched for
   PBYTE    pchBuffer,                   // space for user data
   PULONG   pulLength                    // in/out length of returned user data
);


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictSubStr   Find Key starting with stubstring
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictSubStr(PBTREE,PCHAR,PCHAR,PUSHORT,PCHAR,PUSHORT);
//
//+----------------------------------------------------------------------------+
// Description:       Find the first key starting with the passed key and
//                    pass it back.
//                    If no error happened set this location as new
//                    current position
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  key to be looked for
//                    PCHAR                  buffer for the key
//                    PUSHORT                on input length of buffer
//                                           on output length of filled data
//                    PCHAR                  buffer for the user data
//                    PUSHORT                on input length of buffer
//                                           on output length of filled data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictSubStr
(
   PBTREE pBT,                         // pointer to btree struct
   PCHAR_W  pKey,                        // key to be searched for
   PBYTE  pchBuffer,                   // space for key data
   PULONG pulLength,                  // in/out length of returned key data
   PBYTE  pchUserData,                 // space for user data
   PULONG pulUserLen                   // in/out length of returned user data
);

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictEquiv      Find equivalent match
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictEquiv(PBTREE,PCHAR,PCHAR,PUSHORT,PCHAR,PUSHORT);
//
//+----------------------------------------------------------------------------+
// Description:       Find the first key which is equivalent to
//                    the passed key
//                    If no error happened set this location as
//                    new current position
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE               pointer to btree structure
//                    PCHAR                key to be looked for
//                    PCHAR                buffer for the key
//                    PUSHORT              on input length of buffer
//                                         on output length of filled data
//                    PCHAR                buffer for the user data
//                    PUSHORT              on input length of buffer
//                                         on output length of filled data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary is corrupted
//                    BTREE_NOT_FOUND   key not found
//                    BTREE_INVALID     tree pointer invalid
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictEquiv
(
   PBTREE pBT,                         // pointer to btree struct
   PCHAR_W  pKey,                        // key to be searched for
   PBYTE  pchBuffer,                   // space for key data
   PULONG pulLength,                   // in/out length of returned key data
   PBYTE  pchUserData,                 // space for user data
   PULONG pulUserLen                   // in/out length of returned user data
);

typedef struct _DICTFIND
{
   USHORT usKey;                       // offset to key
   USHORT usBuffer;                    // space for key data
   ULONG  ulLength;                    // in/out length of returned user data
   USHORT usUserBuffer;                // space for user data
   ULONG  ulUserLen;                   // in/out length of returned user data
   BYTE   chVarData;                   // data pointer
} DICTFIND, * PDICTFIND;


typedef struct _DICTNUMENTRIES
{
   ULONG ulNum;                     // in/out length of returned user data
} DICTNUM, * PDICTNUM;


typedef struct _DICTLOCK
{
   BOOL      fLock;                    // lock/unlock dictionary
   USHORT    usKey;                    // pointer to key
   BYTE      chVarData;                // data pointer
} DICTLOCK, * PDICTLOCK;


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictLockEntry  lock/unlock the specified entry
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictLockEntry(PBTREE, PSZ, BOOL );
//+----------------------------------------------------------------------------+
// Description:       Locks/Unlocks the specified dictionary entry
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE pBTIda          pointer to btree structure
//                    PSZ    pEntry          dictionary entry
//                    BOOL   fLock           lock/unlock dictionary entry
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 no error happened
//                    BTREE_CORRUPTED   dictionary is corrupted
//+----------------------------------------------------------------------------+
SHORT QDAMDictLockEntry
(
   PBTREE pBTIda,
   PSZ_W    pEntry,
   BOOL   fLock
);
//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictFirst   Get the first entry back
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictFirst( PBTREE, PCHAR, PUSHORT, PCHAR, PUSHORT );
//
//+----------------------------------------------------------------------------+
// Description:       Locate the first entry and pass back the
//                    associated information into the user provided
//                    buffers
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictFirst
(
   PBTREE     pBT,
   PCHAR_W      pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
);

//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictNext   Get the next entry back -
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictNext( PBTREE, PCHAR, PUSHORT, PCHAR, PUSHORT );
//
//+----------------------------------------------------------------------------+
// Description:       Locate the next entry (by collating sequence) and
//                    pass back the associated information into
//                    the user provided buffers
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictNext
(
   PBTREE     pBT,
   PCHAR_W    pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
);


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictPrev     Get the prev entry back
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictPrev( PBTREE, PCHAR, PUSHORT, PCHAR, PUSHORT );
//
//+----------------------------------------------------------------------------+
// Description:       Locate the previous entry (by collating sequence)
//                    and pass back the associated
//                    information into the user provided buffers
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_EMPTY       dictionary contains no data
//                    BTREE_INVALID     invalid pointer passed
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictPrev
(
   PBTREE     pBT,
   PCHAR_W      pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
);


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictCurrent   Get current entry back
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictCurrent( PBTREE, PCHAR, PUSHORT, PCHAR, PUSHORT )
//
//+----------------------------------------------------------------------------+
// Description:       Locate the current entry  and pass back the
//                    associatedinformation into the user provided buffers
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    PCHAR                  pointer to space for key data
//                    PUSHORT                length of space for key data
//                    PCHAR                  pointer to space for user data
//                    PUSHORT                length of space for user data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_INVALID     invalid pointer passed
//                    BTREE_EOF_REACHED eof or start reached
//                    BTREE_CORRUPTED   dictionary corrupted
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictCurrent
(
   PBTREE     pBT,
   PBYTE      pKeyData,            //   pointer to space for key data
   PULONG     pulKeyLen,           //   length of space for key data
   PBYTE      pUserData,           //   pointer to space for user data
   PULONG     pulUserLen           //   length of space for user data
);


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictCopy    Copy Entries
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictCopy( PBTREE, PBTREE );
//
//+----------------------------------------------------------------------------+
// Description:       Insert the current entry from the source dictionary
//                    into the target dictionary and point to the next
//                    entry in the source dictionary
//                    Source and Target have to be both either local or remote
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE       pointer to btree struct of source dict
//                    PBTREE       pointer to btree struct of target dict
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary corrupted
//                    BTREE_NOT_FOUND   invalid data
//                    BTREE_INVALID     invalid data pointer
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictCopy
(
   PBTREE pBTSource,                // pointer of source tree structure
   PBTREE pBTTarget                 // pointer of target tree structure
);

typedef struct _DICTDELETE
{
   USHORT    usKey;                    // pointer to key
   BYTE      chVarData;                // data pointer
} DICTDEL, * PDICTDEL;



//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictNumber  Position at passed entry number
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictNumber( PBTREE, ULONG, PUCHAR, PUSHORT, PUCHAR,
//                                    PUSHORT );
//+----------------------------------------------------------------------------+
// Description:       Try to position at the passed entry number
//                    and fill the provided area with the data.
//
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to btree structure
//                    ULONG                  term number
//                    PUCHAR                 pointer to space for term
//                    PUSHORT                length of key
//                    PUCHAR                 pointer to data
//                    PUSHORT                length of data
//
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0                 if no error happened
//                    BTREE_NO_BUFFER   no buffer free
//                    BTREE_READ_ERROR  read error from disk
//                    BTREE_DISK_FULL   disk full condition encountered
//                    BTREE_WRITE_ERROR write error to disk
//                    BTREE_CORRUPTED   dictionary corrupted
//                    BTREE_NOT_FOUND   invalid data
//                    BTREE_INVALID     invalid data pointer
//
//+----------------------------------------------------------------------------+
SHORT QDAMDictNumber
(
    PBTREE   pBT,                    // pointer to generic structure
    ULONG    ulTermNum,              // term number
    PCHAR_W   pKey,                   // pointer to space for term
    PULONG   pulKeyLen,              // length of key
    PBYTE    pData,                  // pointer to data
    PULONG   pulLen                  // length of data
);

/**********************************************************************/
/* structure used for end of remote organize                          */
/**********************************************************************/
typedef struct _DICTORG
{
  CHAR   szDictName[MAX_EQF_PATH];     // dictionary asd name
  CHAR   chPrimDrive;                  // remote primary drive
  USHORT usCloseRc;                 // organize/Close successful??
} DICTORG, * PDICTORG;


//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictUpdTimeLocal
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictUpdTimeLocal( pBTree, &lTime );
//+----------------------------------------------------------------------------+
// Description:       gets the update time
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE    pBTIda   pointer to control block
//                    PLONG     plTime   time of last open
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       sRc = LX_RC_OK_ASD -
//+----------------------------------------------------------------------------+
// Function flow:     retrieve the update/open time
//                    return
//+----------------------------------------------------------------------------+
 SHORT QDAMDictUpdTimeLocal ( PBTREE, PLONG );
//+----------------------------------------------------------------------------+
// External function
//+----------------------------------------------------------------------------+
// Function name:     QDAMDictUpdTime
//+----------------------------------------------------------------------------+
// Function call:     QDAMDictUpdTime     ( pBTree, &lTime );
//+----------------------------------------------------------------------------+
// Description:       gets the update time
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE    pBTIda   pointer to control block
//                    PLONG     plTime   time of last open
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       sRc = LX_RC_OK_ASD -
//+----------------------------------------------------------------------------+
// Function flow:     retrieve the update/open time
//                    return
//+----------------------------------------------------------------------------+
 SHORT QDAMDictUpdTime( PBTREE, PLONG );
/**********************************************************************/
/* structure used for DictUpdTime                                     */
/**********************************************************************/
typedef struct _DICTTIME
{
  LONG   lTime;                     // time of last update or open
} DICTTIME, *PDICTTIME;


/**********************************************************************/
/*  define some items for measuring performance                       */
/**********************************************************************/
#if defined(MEASURE)
    USHORT usASI;
    USHORT usASD;
    ULONG ulStart, ulEnd;
    ULONG ulBegin, ulTerseEnd;
    ULONG ulBeg;
    ULONG ulBuffer;
    ULONG ulSeek;
    ULONG ulUnTerseEnd;
    ULONG ulUpdate;
    ULONG ulFind;
    ULONG ulComp;
    ULONG ulBegComp;
    ULONG ulBegRead;
    ULONG ulBegRealRead;
    ULONG ulBegString;
    ULONG ulString;
    ULONG ulRealReadASD;
    ULONG ulRealReadASI;
    ULONG ulRead;
    ULONG ulGet;
    ULONG ulAdd;
    SEL selGlobalSeg, selLocalSeg;
    GINFOSEG far * pGlobInfoSeg;

    USHORT  usEntry;
    USHORT  usEntryLength;
    USHORT  usNumIndexBuffer[2];
  #if defined(DISTRIB)
    ULONG  ulChar[256];
  #endif
#endif

__declspec(dllexport)
 SHORT QDAMDictOpenLocal   ( PSZ, SHORT, USHORT, PPBTREE );
__declspec(dllexport)
 SHORT QDAMDictCreateLocal ( PSZ, SHORT, PCHAR, USHORT, PCHAR,
                             PCHAR, PCHAR, PBTREE * , PNTMVITALINFO );
__declspec(dllexport)
 SHORT QDAMDictCloseLocal  ( PBTREE );
__declspec(dllexport)
 SHORT QDAMDictSignLocal   ( PBTREE, PCHAR, PUSHORT  );
__declspec(dllexport)
 SHORT QDAMDictUpdSignLocal( PBTREE, PCHAR, ULONG  );
__declspec(dllexport)
 SHORT QDAMDictSubStrLocal ( PBTREE, PCHAR_W, PBYTE, PULONG,  PBYTE,PULONG );
__declspec(dllexport)
 SHORT QDAMDictEquivLocal  ( PBTREE, PCHAR_W, PBYTE, PULONG,  PBYTE,PULONG );
__declspec(dllexport)
 SHORT QDAMDictExactLocal  ( PBTREE, PCHAR_W, PBYTE, PULONG  );
__declspec(dllexport)
 SHORT QDAMDictNextLocal   ( PBTREE, PCHAR_W, PULONG,  PBYTE, PULONG  );
__declspec(dllexport)
 SHORT QDAMDictPrevLocal   ( PBTREE, PCHAR_W, PULONG,  PBYTE, PULONG  );
__declspec(dllexport)
 SHORT QDAMDictCurrentLocal( PBTREE, PBYTE, PULONG,  PBYTE, PULONG  );
__declspec(dllexport)
 SHORT QDAMDictInsertLocal ( PBTREE, PCHAR_W, PBYTE, ULONG );
__declspec(dllexport)
 SHORT QDAMDictUpdateLocal ( PBTREE, PCHAR_W, PBYTE, ULONG );
 SHORT QDAMDictDeleteLocal ( PBTREE, PCHAR_W );
 SHORT QDAMDictFlushLocal  ( PBTREE );
 SHORT QDAMDictCopyLocal   ( PBTREE, PBTREE );
 __declspec(dllexport)
 SHORT QDAMDictNumEntriesLocal ( PBTREE, PULONG );
__declspec(dllexport)
 SHORT QDAMDictFirstLocal ( PBTREE, PCHAR_W, PULONG, PBYTE, PULONG );


 SHORT QDAMInsertKey_V2( PBTREE, PBTREEBUFFER_V2, PCHAR_W, RECPARAM, RECPARAM );
 SHORT QDAMInsertKey_V3( PBTREE, PBTREEBUFFER_V3, PCHAR_W, RECPARAM, RECPARAM );
 SHORT QDAMWRecordToDisk_V2( PBTREE, PBTREEBUFFER_V2 );
 SHORT QDAMWRecordToDisk_V3( PBTREE, PBTREEBUFFER_V3 );
 SHORT QDAMFindParent_V2(PBTREE, PBTREEBUFFER_V2, PUSHORT );
 SHORT QDAMFindParent_V3(PBTREE, PBTREEBUFFER_V3, PUSHORT );

 SHORT QDAMFindChild_V2 ( PBTREE, PCHAR_W, USHORT, PBTREEBUFFER_V2 * );
 SHORT QDAMFindChild_V3 ( PBTREE, PCHAR_W, USHORT, PBTREEBUFFER_V3 * );
 SHORT QDAMLocateKey_V2(PBTREE, PBTREEBUFFER_V2, PCHAR_W, PSHORT, SEARCHTYPE, PSHORT);
 SHORT QDAMLocateKey_V3(PBTREE, PBTREEBUFFER_V3, PCHAR_W, PSHORT, SEARCHTYPE, PSHORT);
 SHORT QDAMFreeRecord_V2 ( PBTREE, PBTREEBUFFER_V2, RECTYPE );
 SHORT QDAMFreeRecord_V3 ( PBTREE, PBTREEBUFFER_V3, RECTYPE );
__declspec(dllexport)
 SHORT QDAMWriteHeader ( PBTREE );
 SHORT QDAMWriteRecord_V2(PBTREE, PBTREEBUFFER_V2);
 SHORT QDAMWriteRecord_V3(PBTREE, PBTREEBUFFER_V3);
 SHORT QDAMChangeKey_V2(PBTREE, USHORT, PCHAR_W, PCHAR_W );
 SHORT QDAMChangeKey_V3(PBTREE, USHORT, PCHAR_W, PCHAR_W );
 SHORT QDAMNewRecord_V2( PBTREE, PBTREEBUFFER_V2 *, RECTYPE );
 SHORT QDAMNewRecord_V3( PBTREE, PBTREEBUFFER_V3 *, RECTYPE );
 SHORT QDAMReadRecord_V2( PBTREE, USHORT, PBTREEBUFFER_V2 *, BOOL );
 SHORT QDAMReadRecord_V3 ( PBTREE, USHORT, PBTREEBUFFER_V3 *, BOOL );
 SHORT QDAMReadRecordFromDisk_V2( PBTREE, USHORT, PBTREEBUFFER_V2 *, BOOL );
 SHORT QDAMReadRecordFromDisk_V3( PBTREE, USHORT, PBTREEBUFFER_V3 *, BOOL );
 SHORT QDAMAddToBuffer_V2(PBTREE, PBYTE, ULONG, PRECPARAM);
 SHORT QDAMAddToBuffer_V3(PBTREE, PBYTE, ULONG, PRECPARAM);
 SHORT QDAMDeleteDataFromBuffer ( PBTREE, RECPARAM );
 SHORT QDAMSplitNode_V2( PBTREE, PBTREEBUFFER_V2 *, PCHAR_W );
 SHORT QDAMSplitNode_V3( PBTREE, PBTREEBUFFER_V3 *, PCHAR_W );

 SHORT QDAMGetszKeyParam_V2( PBTREE, RECPARAM, PCHAR_W, PULONG );
 SHORT QDAMGetszKeyParam_V3( PBTREE, RECPARAM, PCHAR_W, PULONG );
 SHORT QDAMGetszData_V2 ( PBTREE, RECPARAM,  PBYTE, PULONG, CHAR );
 SHORT QDAMGetszData_V3 ( PBTREE, RECPARAM,  PBYTE, PULONG, CHAR );

 BOOL  QDAMDeleteKey_V2( PBTREE, PBTREEBUFFER_V2, PCHAR);
 BOOL  QDAMDeleteKey_V3( PBTREE, PBTREEBUFFER_V3, PCHAR);
 SHORT QDAMDestroy ( PBTREE );
 SHORT QDAMKeyCompare ( PVOID, PVOID, PVOID );
 __declspec(dllexport)
 SHORT QDAMKeyCompareNonUnicode ( PVOID, PVOID, PVOID );
 SHORT QDAMFirst_V2( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
 SHORT QDAMFirst_V3( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
 SHORT QDAMNext_V2( PBTREE, PRECPARAM,  PRECPARAM,PRECPARAM );
 SHORT QDAMNext_V3( PBTREE, PRECPARAM,  PRECPARAM,PRECPARAM );
 SHORT QDAMPrev_V2( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
 SHORT QDAMPrev_V3( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
 SHORT QDAMHeaderFirst ( PBTREE, PRECPARAM, PRECPARAM, PRECPARAM );
 SHORT QDAMHeaderNext ( PBTREE, PRECPARAM,  PRECPARAM,PRECPARAM );
 SHORT QDAMAllocTempAreas ( PBTREE );
 VOID  QDAMUpdateList_V2( PBTREE, PBTREEBUFFER_V2 );
 VOID  QDAMUpdateList_V3( PBTREE, PBTREEBUFFER_V3 );
 VOID  QDAMFreeFromList_V2(PRECPARAM ,PBTREEBUFFER_V2 );
 VOID  QDAMFreeFromList_V3(PRECPARAM ,PBTREEBUFFER_V3 );
 VOID  QDAMTerseInit ( PBTREE, PUCHAR );
 SHORT QDAMUnTerseData ( PBTREE, PUCHAR, ULONG, PULONG );
 BOOL  QDAMTerseData ( PBTREE, PUCHAR, PULONG, PUCHAR );
 SHORT QDAMValidateIndex_V2( PBTREE,  PBTREEBUFFER_V2 * );
 SHORT QDAMValidateIndex_V3( PBTREE,  PBTREEBUFFER_V3 * );

 PSZ_W     QDAMGetszKey_V2( PBTREEBUFFER_V2,  USHORT, USHORT );
 PSZ_W     QDAMGetszKey_V3 ( PBTREEBUFFER_V3,  USHORT, USHORT );
 RECPARAM  QDAMGetrecData_V2 ( PBTREEBUFFER_V2, SHORT, USHORT );
 RECPARAM  QDAMGetrecData_V3 ( PBTREEBUFFER_V3, SHORT, USHORT );
 VOID      QDAMSetrecData_V2 ( PBTREEBUFFER_V2, SHORT, RECPARAM, USHORT );
 VOID      QDAMSetrecData_V3 ( PBTREEBUFFER_V3, SHORT, RECPARAM, USHORT );
 RECPARAM  QDAMGetrecKey_V2( PBTREEBUFFER_V2, SHORT );
 RECPARAM  QDAMGetrecKey_V3 ( PBTREEBUFFER_V3, SHORT );
 ULONG QDAMGetrecDataLen_V2( PBTREE, PBTREEBUFFER_V2, SHORT );
 ULONG QDAMGetrecDataLen_V3 ( PBTREE, PBTREEBUFFER_V3, SHORT );
 VOID  QDAMReArrangeKRec_V2( PBTREE, PBTREEBUFFER_V2 );
 VOID  QDAMReArrangeKRec_V3 ( PBTREE, PBTREEBUFFER_V3 );
 VOID  QDAMCopyKeyTo_V2( PBTREEBUFFER_V2, SHORT, PBTREEBUFFER_V2, SHORT, USHORT );
 VOID  QDAMCopyKeyTo_V3( PBTREEBUFFER_V3, SHORT, PBTREEBUFFER_V3, SHORT, USHORT );
 VOID  QDAMCopyDataTo_V2 ( PBTREEBUFFER_V2, SHORT, PBTREEBUFFER_V2, SHORT, USHORT );
 VOID  QDAMCopyDataTo_V3( PBTREEBUFFER_V3, SHORT, PBTREEBUFFER_V3, SHORT, USHORT );
 SHORT QDAMLastEntry_V2( PBTREE, PBTREEBUFFER_V2 * );
 SHORT QDAMLastEntry_V3( PBTREE, PBTREEBUFFER_V3 * );
 SHORT QDAMFirstEntry_V2( PBTREE, PBTREEBUFFER_V2 * );
 SHORT QDAMFirstEntry_V3( PBTREE, PBTREEBUFFER_V3 * );
 BOOL  QDAMFetchFromIndexList_V2( PBTREE, PBTREEBUFFER_V2, USHORT );
 BOOL  QDAMFetchFromIndexList_V3( PBTREE, PBTREEBUFFER_V3, USHORT );
 VOID  QDAMAddToIndexList_V2( PBTREE, PBTREEBUFFER_V2 );
 VOID  QDAMAddToIndexList_V3( PBTREE, PBTREEBUFFER_V3 );


 SHORT  QDAMAllocKeyRecords ( PBTREE, USHORT );
 SHORT  QDAMLocSubstr_V2( PBTREE, PBTREEBUFFER_V2, PCHAR_W, PBYTE, PULONG, PBYTE, PULONG );
 SHORT  QDAMLocSubstr_V3( PBTREE, PBTREEBUFFER_V3, PCHAR_W, PBYTE, PULONG, PBYTE, PULONG );

__declspec(dllexport)
 SHORT  QDAMCheckDict( PSZ, PBTREE );
 SHORT  QDAMAddDict( PSZ, PBTREE );
 USHORT QDAMRemoveDict( PBTREE );
 SHORT  QDAMDictLockDictLocal( PBTREE, BOOL );
 SHORT  QDAMDictLockEntryLocal( PBTREE, PSZ_W, BOOL );
 BOOL   QDAMDictLockStatus ( PBTREE, PCHAR_W );
 VOID   QDAMDictUpdStatus ( PBTREE);
 USHORT QDAMMoveProperties ( PPROPDICTIONARY , PSZ );
 PBTREE QDAMFindInstance ( PBTREE, PSZ );
 SHORT QDAMFindRecord_V2( PBTREE, PCHAR_W, PBTREEBUFFER_V2 * );
 SHORT QDAMFindRecord_V3( PBTREE, PCHAR_W, PBTREEBUFFER_V3 * );

 SHORT QDAMDictCloseOrganizeLocal ( PQDAMLAN pQDAMLan );
 SHORT QDAMDictCloseOrganize ( PPBTREE, PSZ, CHAR, USHORT );

 USHORT RenameOrganize( PSZ pszDictPath, CHAR chPrimDrive, USHORT usCloseRc );

 __declspec(dllexport)
 USHORT DamBTreeRc ( SHORT sRc );

 /*********************************************************************/
 /* our own Nlp layer...                                              */
 /*********************************************************************/
#define DAMLINK _loadds far pascal

__declspec(dllexport)
void DAMLINK NlpLockEntryW ( USHORT, PSZ_W, BOOL, PUSHORT );
__declspec(dllexport)
void DAMLINK NlpLockDict ( USHORT, BOOL, PUSHORT );
__declspec(dllexport)
void DAMLINK NlpRetBTree ( USHORT, PVOID * , PLONG );
__declspec(dllexport)
void DAMLINK NlpCloseOrganize( USHORT, PSZ, CHAR, PUSHORT, USHORT );
__declspec(dllexport)
void DAMLINK NlpDictUpdTime( USHORT, PLONG, PUSHORT );
__declspec(dllexport)
void DAMLINK NlpTermListW ( USHORT, USHORT, PSZ_W, USHORT, USHORT,
                                      PSZ_W, USHORT, PUSHORT );

__declspec(dllexport)
void DAMLINK
  NlpBegAsd( USHORT,                 /* in  - max. files allowed  */
             USHORT,                 /* in  - max. indexes / base */
             USHORT,                 /* in  - max. associations   */
             USHORT,                 /* in  - max. files / assoc. */
             PUSHORT,                /* out - user handle         */
             PUSHORT);               /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpEndAsd( USHORT,                 /* in  - user handle         */
             PUSHORT);               /* out - return code         */

void DAMLINK
  NlpOpenAsd(PUCHAR,                /* in  - name of dictionary  */
             USHORT,                /* in  - number of pages     */
             USHORT,                /* in  - immediate writing?  */
             USHORT,                /* in  - user handle         */
             PUSHORT,               /* out - dictionary handles  */
             PUSHORT,               /* out - length of user area */
             PUSHORT);              /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpBuildAsd(PUCHAR,               /* in  - dictionary name     */
              USHORT,               /* in  - dictionary type     */
              USHORT,               /* in  - number of pages     */
              USHORT,               /* in  - immediate writing?  */
              USHORT,               /* in  - language            */
              USHORT,               /* in  - codepage            */
              USHORT,               /* in  - feature mask        */
              PUCHAR,               /* in  - primary collate seq.*/
              PUCHAR,               /* in  - secondary coll. seq.*/
              PUCHAR,               /* in  - edit command string */
              PUCHAR,               /* in  - term encoding table */
              PUCHAR,               /* in  - entry encoding table*/
              PUSHORT,              /* in  - length of user data */
              PUCHAR,               /* in  - user defined data   */
              PUCHAR,               /* in  - index name list     */
              USHORT,               /* in  - user handle         */
              PUSHORT,              /* out - dictionary handle   */
              PUSHORT);             /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpCloseAsd(USHORT,               /* in  - dictionary handle   */
              USHORT,               /* in  - user handle         */
              PUSHORT);             /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpAssocAsd(PUSHORT,              /* in  - dictionary handles  */
              USHORT,               /* in  - user handle         */
              PUSHORT,              /* out - association handles */
              PUSHORT);             /* out - return code         */

void DAMLINK
  NlpRetSignature(USHORT,           /* in  - dictionary handle   */
                  USHORT,           /* in  - user handle         */
                  PUSHORT,          /* out - dictionary type     */
                  PUSHORT,          /* out - feature mask        */
                  PUSHORT,          /* out - codepage            */
                  PUCHAR,           /* out - primary collate seq.*/
                  PUCHAR,           /* out - secondary coll. seq.*/
                  PUCHAR,           /* out - edit command string */
                  PUCHAR,           /* out - term encoding table */
                  PUCHAR,           /* out - entry encoding tbl. */
                  PUCHAR,           /* out - dictionary names    */
                  PUSHORT,          /* out - length of user-data */
                  PUCHAR,           /* out - user-defined data   */
                  PUSHORT);         /* out - return code         */

void DAMLINK
  NlpUpdSignature(USHORT,           /* in  - dictionary handle   */
                  USHORT,           /* in  - user handle         */
                  USHORT,           /* in  - protection mode     */
                  PUCHAR,           /* in  - dictionary names    */
                  USHORT,           /* in  - length of user data */
                  PUCHAR,           /* in  - user-defined data   */
                  PUSHORT);         /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpRenumberAsd(USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user area handle    */
                 PUSHORT);          /* out - return code         */

void DAMLINK
  NlpCompTermsAsd(PUCHAR,           /* in  - 1st compare term    */
                  PUCHAR,           /* in  - 2nd compare term    */
                  USHORT,           /* in  - type of compare     */
                  USHORT,           /* in  - dictionary handle   */
                  USHORT,           /* in  - user area handle    */
                  PUSHORT,          /* out - result of compare   */
                  PUSHORT);         /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpFndBeginAsdW(PSZ_W,            /* in  - desired substring   */
                 USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PSZ_W ,            /* out - matching term found */
                 PULONG,            /* out - term number         */
                 PULONG,            /* out - entry data length   */
                 PUSHORT,           /* out - dictionary of match */
                 PUSHORT);          /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpFndEquivAsdW(PSZ_W,            /* in  - desired term        */
                 USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PSZ_W,            /* out - matching term found */
                 PULONG,            /* out - term number         */
                 PULONG,            /* out - entry data length   */
                 PUSHORT,           /* out - dictionary of match */
                 PUSHORT);          /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpFndMatchAsdW(PSZ_W,            /* in  - desired term        */
                 USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PBYTE,             /* out - matching term found */
                 PULONG,            /* out - term number         */
                 PULONG,            /* out - entry data length in bytes*/
                 PUSHORT,           /* out - dictionary of match */
                 PUSHORT);          /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpFndNumberAsdW(ULONG,            /* in  - desired term number */
                  USHORT,           /* in  - dictionary handle   */
                  USHORT,           /* in  - relocation flag     */
                  USHORT,           /* in  - user handle         */
                  PSZ_W ,           /* out - matching term found */
                  PULONG,           /* out - entry data length   */
                  PUSHORT,          /* out - dictionary of match */
                  PUSHORT);         /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpNxtTermAsdW(USHORT,             /* in  - dictionary handle   */
                USHORT,             /* in  - user handle         */
                PSZ_W ,             /* out - matching term found */
                PULONG,             /* out - term number         */
                PULONG,             /* out - entry data length   */
                PUSHORT,            /* out - dictionary of match */
                PUSHORT);           /* out - return code         */
__declspec(dllexport)
void DAMLINK
NlpNxtTermAsd
(
   USHORT usHandle,
   USHORT usUser,
   PUCHAR pMatch,
   PULONG pulNum,
   PULONG pulLen,
   PUSHORT pusDict,
   PUSHORT pusRc
);             // needed for organize of old dicts

__declspec(dllexport)
void DAMLINK
  NlpPrvTermAsdW(USHORT,             /* in  - dictionary handle   */
                USHORT,             /* in  - user handle         */
                PSZ_W ,             /* out - matching term found */
                PULONG,             /* out - term number         */
                PULONG,             /* out - entry data length   */
                PUSHORT,            /* out - dictionary of match */
                PUSHORT);           /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpInsEntryAsdW(PSZ_W,            /* in  - term for new entry  */
                 PBYTE,            /* in  - data for term entry */
                 ULONG,             /* in  - entry data length   */
                 USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PULONG,            /* out - inserted term's #   */
                 PUSHORT);          /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpUpdEntryAsdW(PSZ_W ,            /* in  - term to update      */
                 PBYTE ,            /* in  - data for term entry */
                 ULONG,             /* in  - entry data length in bytes   */
                 USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PULONG,            /* out - updated term number */
                 PUSHORT);          /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpDelEntryAsdW(PSZ_W,            /* in  - term to be deleted  */
                 USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PUSHORT);          /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpRetEntryAsdW(USHORT,            /* in  - dictionary handle   */
                 USHORT,            /* in  - user handle         */
                 PSZ_W ,            /* out - term for this entry */
                 PULONG,            /* out - term number         */
                 PBYTE ,            /* out - data for term entry */
                 PULONG,            /* out - entry data length in bytes*/
                 PUSHORT,           /* out - dictionary of term  */
                 PUSHORT);          /* out - return code         */

__declspec(dllexport)
void DAMLINK
NlpRetEntryAsd
(
   USHORT usHandle,
   USHORT usUser,
   PUCHAR pTerm,
   PULONG pulNum,
   PUCHAR pData,
   PULONG pulDataLen,
   PUSHORT pusDict,
   PUSHORT pusRc
);
void DAMLINK
  NlpRetSpellAsd(PUCHAR,            /* in  - input term          */
                 USHORT,            /* in  - dictionary handle   */
                 PUCHAR,            /* in  - term editing rules  */
                 USHORT,            /* in  - verify words?       */
                 USHORT,            /* in  - user handle         */
                 PUSHORT,           /* out - # of output terms   */
                 PUCHAR,            /* out - output term list    */
                 PULONG,            /* out - length of list      */
                 PUSHORT);          /* out - return code         */

void DAMLINK
  NlpRetListAsd(USHORT,             /* in  - dictionary handle   */
                USHORT,             /* in  - requested # terms   */
                USHORT,             /* in  - user handle         */
                PUSHORT,            /* out - output # terms      */
                PUCHAR,             /* out - output term list    */
                PULONG,             /* out - length of list      */
                PUSHORT);           /* out - return code         */

__declspec(dllexport)
void DAMLINK
  NlpResynchAsd(USHORT,             /* in  - dictionary handle   */
                USHORT,             /* in  - user handle         */
                PUSHORT);           /* out - return code         */

//+----------------------------------------------------------------------------+
// Internal function
//+----------------------------------------------------------------------------+
// Function name:     NTMKeyCompare      Generic compare function
//+----------------------------------------------------------------------------+
// Function call:     NTMKeyCompare( PBTREE,PULONG,PULONG);
//+----------------------------------------------------------------------------+
// Description:       This is the generic compare function used
//                    for comparision
//+----------------------------------------------------------------------------+
// Parameters:        PBTREE                 pointer to tree structure
//                    PULONG                 first key
//                    PULONG                 second key
//+----------------------------------------------------------------------------+
// Returncode type:   SHORT
//+----------------------------------------------------------------------------+
// Returncodes:       0   keys are equal
//                    <> keys are unequal
//+----------------------------------------------------------------------------+
__declspec(dllexport)
SHORT NTMKeyCompare
(
    PVOID  pBTIda,                     // pointer to tree structure
    PVOID  pulKey1,                    // pointer to first key
    PVOID  pulKey2                     // pointer to second key
);

/**********************************************************************/
/* validate the passed BTree pointer                                  */
/**********************************************************************/
#define CHECKPBTREE(pBTIda, sRc)                  \
   if ( !pBTIda )                                 \
   {                                              \
     sRc = BTREE_INVALID;                         \
   }                                              \
   else                                           \
   {                                              \
     sRc = pBTIda->pBTree ? 0 : BTREE_INVALID;    \
   } /* endif */

  #define LENGTHOFDATA( pBT, pData ) \
    ((pBT->usVersion >= NTM_VERSION2) ? *((PULONG)(pData)) : (ULONG)*((PUSHORT)(pData)) )

  #define STARTOFDATA( pBT, pData ) \
    ((pBT->usVersion >= NTM_VERSION2) ? (PBYTE)pData + sizeof(ULONG) : (PBYTE)pData + sizeof(USHORT) )

  #define SETDATALENGTH( pBT, pData, ulLen ) \
    { if (pBT->usVersion >= NTM_VERSION2) \
        *((PULONG)(pData)) = ulLen; \
      else \
        *((PUSHORT)(pData)) = (USHORT)ulLen; }

__declspec(dllexport)
SHORT QDAMPhysLock
(
   PBTREE         pBTIda,
   BOOL           fLock,
   PBOOL          pfLocked
);

SHORT QDAMCheckForUpdates
(
   PBTREE         pBTIda
);

__declspec(dllexport)
SHORT QDAMIncrUpdCounter
(
   PBTREE     pBTIda,                  // pointer to btree structure
   SHORT      sIndex,                  // index of requested update counter
   PLONG      plNewValue               // ptr to buffer for new counte value
);

__declspec(dllexport)
SHORT QDAMGetUpdCounter
(
   PBTREE     pBTIda,                  // pointer to btree structure
   PLONG      plUpdCounter,            // buffer for update counter value
   SHORT      sIndex,                  // index of requested update counter
   SHORT      sNumCounters             // number of requested update counters
);

__declspec(dllexport)
SHORT QDAMUpdateLockRec
(
   PBTREE     pBTIda,                  // pointer to btree structure
   PSZ_W      pszTerm,                 // term being added or removed
   BOOL       fAdd                     // TRUE = Add term to lock record
                                       // FALSE = remove term from lock record
);

SHORT QDAMDosRC2BtreeRC
(
  SHORT sDosRC,                        // Dos return code
  SHORT sDefaultRC,                    // RC for default case
  USHORT usOpenFlags                   // open flags of database
);

__declspec(dllexport)
USHORT QDamWildCardList
(
  USHORT,             // in  - user handle
  USHORT,             // in  - dictionary handle
  PSZ_W,                // in - term to start with
  PSZ_W,                // in - search pattern/compound(s)
  USHORT,             // in - number of terms to be looked up
  BOOL,               // in - compound search flag
  PSZ_W,             // in - buffer for term list
  ULONG               // in - size of buffer
);

SHORT QDAMDictNextWildLocal
(
   PBTREE     pBTIda,
   PSZ_W      pPattern,                // search pattern or compound
   BOOL       fCompound,               // compound search flag
   PBYTE      pKeyData,                // pointer to space for key data
   PULONG     pulKeyLen                // length of space for key data
);

BOOL QDAMMatchCompound
(
  PSZ_W  pKey,
  PSZ_W  pCompound
);

BOOL QDAMMatchWild
(
  PUCHAR pKey,
  PUCHAR pPattern
);

ULONG QDAMComputeCheckSum_V2(PBTREEBUFFER_V2 pRecord );
ULONG QDAMComputeCheckSum_V3 (PBTREEBUFFER_V3 pRecord );
BOOL QDAMCheckCheckSum_V2( PBTREEBUFFER_V2 pRecord, SHORT sLocation );
BOOL QDAMCheckCheckSum_V3( PBTREEBUFFER_V3 pRecord, SHORT sLocation );

// get nect dictionary of an association list
__declspec(dllexport)
USHORT DAMGetNextDict
(
                 USHORT             /* in  - dictionary handle   */
);

__declspec(dllexport)
PBTREE DamGetBTreeFromDamRec(USHORT usIndex);
