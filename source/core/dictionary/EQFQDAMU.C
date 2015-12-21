//+----------------------------------------------------------------------------+
//|EQFQDAMU.C                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author: R.Jornitz                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:  The underlying technique used is a modified B+ tree.          |
//|              Each node in the B+ Tree contains a variable number of keys.  |
//|              The key data are stored separately, but in the same           |
//|              file.                                                         |
//|              The elements in the non-leaf nodes contain                    |
//|              pointers to other nodes within the tree.                      |
//|              The leaf nodes contain pointers to the actual data,           |
//|              which are stored in the same file.                            |
//|                                                                            |
//|              The leaf nodes are chained so that the data can be            |
//|              searched both sequentially and via a key.                     |
//|                                                                            |
//|            Error Handling                                                  |
//|              When an error is encountered, a value is returned             |
//|              that indicates an error. For pointers, a NULL is              |
//|              returned.                                                     |
//|                                                                            |
//|            Comparison Function                                             |
//|              The comparison function currently used is a                   |
//|              user-defined compare function supporting different            |
//|              collating sequences.                                          |
//|              The user can specify a Collating sequence and a               |
//|              CaseMap table (for case insensitive search).                  |
//|                                                                            |
//|            Structure                                                       |
//|                                                                            |
//|              A B-tree consists of a series of blocks, which can            |
//|              contain several keys arranged in order. For example,          |
//|                                                                            |
//|                             +-б----б-б----б-б---б-+                        |
//|                             | | 10 | | 32 | |   | |                        |
//|                             +ба----аба----аба---а-+                        |
//|                    +---------+      |      +----------+                    |
//|            +б--б-б-аб-б--б-++-б--б-ба-б-б--б-++-б--б-ба-б-б--б-+           |
//|            || 5| | 7| | 9| || |10| |21| |  | || |32| |  | |  | |           |
//|            +а--а-а--а-а--а-++-а--а-а--а-а--а-++-а--а-а--а-а--а-+           |
//|                                                                            |
//|            Each block contains the same number of slots for key            |
//|            values, although some slots may not be occupied.                |
//|            By storing several keys within a block, I/O is reduced          |
//|            as we can quickly get to the correct leaf block.                |
//|            If the data was stored in a binary tree, i.e. only one          |
//|            key value per node, each access to a node would require         |
//|            1 block to be read.                                             |
//|            By storing more than one key in a node, the next block          |
//|            to read can be found for less I/O operations                    |
//|                                                                            |
//|            Locating the correct record uses a extension of the             |
//|            Binary tree algorithm, given that there are multiple            |
//|            keys per node.                                                  |
//|                                                                            |
//|            Thus, if we needed to locate 21 within the above tree,          |
//|            we would find the link in the root node that                    |
//|            corresponds to the key 21.  This would require the root         |
//|            node to be read and searched for a value that is greater        |
//|            than the key we are searching for.                              |
//|            In this case, the search would stop at 32.  The                 |
//|            link to the left is followed and the process repeated.          |
//|                                                                            |
//|            Insertion and deletion are more complicated because it          |
//|            is sometimes necessary to split or coalesce the blocks.         |
//|            for example, if we add a value 6 to the tree above, the         |
//|            left hand block cannot hold the 6 as well as the 5, 7 and 9.    |
//|            Thus, the tree needs to be re-arranged. The code here           |
//|            tries to move one of the nodes to the neighbouring block.       |
//|                                                                            |
//|                              +-б----б-б----б-б----б-+                      |
//|                              | |  9 | | 32 | |    | |                      |
//|                              +ба----аба----аба----а-+                      |
//|                    +----------+      |      +-------------+                |
//|            +б--б-б-аб-б--б-++-б--б-б-а--б-б---б-++-б--б-б-аб-б-б-+         |
//|            || 5| | 6| | 7| || | 9| | 10 | | 21| || |32| |  | | | |         |
//|            +а--а-а--а-а--а-++-а--а-а----а-а---а-++-а--а-а--а-а-а-+         |
//|                                                                            |
//|            See the BTreeSplitNode function for more details on the         |
//|             lgorithms used.                                                |
//|                                                                            |
//|            Optimisations                                                   |
//|                                                                            |
//|               Delaying the writing of a block to disk.                     |
//|               This allows the data to be cached in memory and only         |
//|               written when the buffer is needed for another record.        |
//|               Often, a record is updated many times in a short             |
//|               period, so this can reduce the number of writes to           |
//|               disk by as much as 10 times.                                 |
//|               Unfortunately, it also means that in the event of a          |
//|               power failure or the program crashing, the tree that         |
//|               is on the disk may not be consistent.                        |
//|               In order to reduce the chances of this, a function           |
//|               BTreeFlush is provided which forces any records not          |
//|               yet written to disk.                                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                QDAMDictOpen                                                |
//|                QDAMDictCreate                                              |
//|                QDAMDictClose                                               |
//|                QDAMDictSign                                                |
//|                QDAMDictUpdSign                                             |
//|                QDAMDictSubStr                                              |
//|                QDAMDictEquiv                                               |
//|                QDAMDictExact                                               |
//|                QDAMDictNext                                                |
//|                QDAMDictPrev                                                |
//|                QDAMDictCurrent                                             |
//|                QDAMDictInsert                                              |
//|                QDAMDictUpdate                                              |
//|                QDAMDictDelete                                              |
//|                QDAMDictFlush                                               |
//|                QDAMDictCopy                                                |
//|                QDAMDictNumEntries                                          |
//|                QDAMDictFirst                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                Utl - functions                                             |
//|                DosBufReset                                                 |
//|                DosSetFHandState                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|                QDAMDictOpenLocal            local subroutines              |
//|                QDAMDictCreateLocal            for API calls                |
//|                QDAMDictCloseLocal               which will be used in      |
//|                QDAMDictSignLocal                 the remote case, too      |
//|                QDAMDictUpdSignLocal                                        |
//|                QDAMDictSubStrLocal                                         |
//|                QDAMDictEquivLocal                                          |
//|                QDAMDictExactLocal                                          |
//|                QDAMDictNextLocal                                           |
//|                QDAMDictPrevLocal                                           |
//|                QDAMDictCurrentLocal                                        |
//|                QDAMDictInsertLocal                                         |
//|                QDAMDictUpdateLocal                                         |
//|                QDAMDictDeleteLocal                                         |
//|                QDAMDictFlushLocal                                          |
//|                QDAMDictCopyLocal                                           |
//|                QDAMDictNumEntriesLocal                                     |
//|                QDAMDictFirstLocal                                          |
//|                                                                            |
//|                QDAMDictFind                 internal subroutines           |
//|                QDAMInsertKey                   which will do the           |
//|                QDAMWRecordToDisk                 dirty job like            |
//|                QDAMFindParent                      inserting a term, etc.  |
//|                QDAMLocateKey                                               |
//|                QDAMFreeRecord                                              |
//|                QDAMWriteHeader                                             |
//|                QDAMWriteRecord                                             |
//|                QDAMChangeKey                                               |
//|                QDAMNewRecord                                               |
//|                QDAMReadRecord                                              |
//|                QDAMReadRecordFromDisk                                      |
//|                QDAMAddToBuffer                                             |
//|                QDAMDeleteDataFromBuffer                                    |
//|                QDAMSplitNode                                               |
//|                QDAMGetszKey                                                |
//|                QDAMGetszKeyParam                                           |
//|                QDAMGetszData                                               |
//|                QDAMDeleteKey                                               |
//|                QDAMDestroy                                                 |
//|                QDAMKeyCompare                                              |
//|                QDAMFirst                                                   |
//|                QDAMNext                                                    |
//|                QDAMPrev                                                    |
//|                QDAMHeaderFirst                                             |
//|                QDAMHeaderNext                                              |
//|                QDAMAllocTempAreas                                          |
//|                QDAMUpdateList                                              |
//|                QDAMFreeFromList                                            |
//|                QDAMTerseInit                                               |
//|                QDAMUnTerseData                                             |
//|                QDAMTerseData                                               |
//|                QDAMValidateIndex                                           |
//|                QDAMGetrecData                                              |
//|                QDAMSetrecData                                              |
//|                QDAMGetrecKey                                               |
//|                QDAMGetrecDataLen                                           |
//|                QDAMReArrangeKRec                                           |
//|                QDAMCopyKeyTo                                               |
//|                QDAMCopyDataTo                                              |
//|                QDAMLastEntry                                               |
//|                QDAMFirstEntry                                              |
//|                QDAMFetchFromIndexList                                      |
//|                QDAMAddToIndexList                                          |
//|                QDAMAllocKeyRecords                                         |
//|                QDAMLocSubstr                                               |
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
// $Revision: 1.2 $ ----------- 29 Oct 2007
// GQ: - enabled code for new record size
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
// -- RJ: fix problem in tersing position
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 4 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFQDAMU.CV_   1.1   29 Sep 1998 07:34:02   BUILD  $
 *
 * $Log:   K:\DATA\EQFQDAMU.CV_  $
 *
 *    Rev 1.1   29 Sep 1998 07:34:02   BUILD
 * - enabled QDAMTerseData and QDAMUntersDate for records larger than 32kB
 *
 *    Rev 1.0   09 Jan 1996 09:12:54   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define INCL_EQF_ASD              // dictionary access functions (Asd...)
#define INCL_EQF_DAM              // low level dict. access functions (Nlp/Dam)
#define INCL_EQF_TM               // general Transl. Memory functions
#include <eqf.h>                  // General Translation Manager include file

#include <eqfqdami.h>             // Private QDAM defines

/**********************************************************************/
/*  encoding sequence used                                            */
/*  The following should never be changed......                       */
/*  The general structure used is an algorithm based on HUFFMANN      */
/*  but working with fixed distributions.                             */
/*                                                                    */
/* Algorithm:                                                         */
/*   The algorithm is a variable length based algorithm, varying in   */
/* length from 3 bits to 11 bits.                                     */
/* The most frequent 15 characters will be encoded in 3 to 5 bits.    */
/* The bit sequence used are listed in the following table.           */
/* All others are encoded in 11 bits, i.e. '000' plus the character.  */
/*                                                                    */
/* The last three tables are used to allow fast access to determine   */
/* the encode values to allow decomposition.                          */
/**********************************************************************/
//                                       Leng  Value   Bits
static STENCODEBITS stEncodeBits[16] = { {3,0},     // 000
                                         {3,1},     // 001
                                         {3,2},     // 010
                                         {3,3},     // 011
                                         {4,8},     // 1000
                                         {4,9},     // 1001
                                         {4,12},    // 1100
                                         {4,13},    // 1101
                                         {5,20},    // 10100
                                         {5,21},    // 10101
                                         {5,22},    // 10110
                                         {5,23},    // 10111
                                         {5,28},    // 11100
                                         {5,29},    // 11101
                                         {5,30},    // 11110
                                         {5,31}};   // 11111

static  USHORT us1BitNibble[16] = { 0,       // 00000000 00000000
                                    1,       // 00000000 00000001
                                    2,       // 00000000 00000010
                                    4,       // 00000000 00000100
                                    8,       // 00000000 00001000
                                    16,      // 00000000 00010000
                                    32,      // 00000000 00100000
                                    64,      // 00000000 01000000
                                    128,     // 00000000 10000000
                                    256,     // 00000001 00000000
                                    512,     // 00000010 00000000
                                    1024,    // 00000100 00000000
                                    2048,    // 00001000 00000000
                                    4096,    // 00010000 00000000
                                    8192,    // 00100000 00000000
                                    16384 }; // 01000000 00000000

static  USHORT us2BitNibble[16] = { 0,       // 00000000 00000000
                                    1,       // 00000000 00000001
                                    3,       // 00000000 00000011
                                    6,       // 00000000 00000110
                                    12,      // 00000000 00001100
                                    24,      // 00000000 00011000
                                    48,      // 00000000 00110000
                                    96,      // 00000000 01100000
                                    192,     // 00000000 11000000
                                    384,     // 00000001 10000000
                                    768,     // 00000011 00000000
                                    1536,    // 00000110 00000000
                                    3072,    // 00001100 00000000
                                    6144,    // 00011000 00000000
                                    12288,   // 00110000 00000000
                                    24576 }; // 01100000 00000000

static  USHORT us3BitNibble[16] = { 0,       // 00000000 00000000
                                    1,       // 00000000 00000001
                                    3,       // 00000000 00000011
                                    7,       // 00000000 00000111
                                    14,      // 00000000 00001110
                                    28,      // 00000000 00011100
                                    56,      // 00000000 00111000
                                    112,     // 00000000 01110000
                                    224,     // 00000000 11100000
                                    448,     // 00000001 11000000
                                    896,     // 00000011 10000000
                                    1792,    // 00000111 00000000
                                    3584,    // 00001110 00000000
                                    7168,    // 00011100 00000000
                                    14336,   // 00111000 00000000
                                    28672 }; // 01110000 00000000

static  USHORT us8BitNibble[16] = { 0,       // 00000000 00000000
                                    1,       // 00000000 00000001
                                    3,       // 00000000 00000011
                                    7,       // 00000000 00000111
                                    15,      // 00000000 00001111
                                    31,      // 00000000 00011111
                                    63,      // 00000000 00111111
                                    127,     // 00000000 01111111
                                    255,     // 00000000 11111111
                                    510,     // 00000001 11111110
                                    1020,    // 00000011 11111100
                                    2040,    // 00000111 11111000
                                    4080,    // 00001111 11110000
                                    8160,    // 00011111 11100000
                                    16320,   // 00111111 11000000
                                    32640 }; // 01111111 10000000



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDAMTerseInit  Initialize Tersing                        |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMTerseInit( PBTREE, PUCHAR );                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       This call will build the compression table from          |
//|                   passed frequency table                                   |
//|                   A decode table is built as well.                         |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE           pointer to btree structure              |
//|                   PUCHAR           pointer to frequent character table     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Side effects:      The most frequent 15 characters will be stored as 3 to 5 |
//|                   bits. The rest is stored as 11 bits, i.e. '000' plus the |
//|                   character.                                               |
//+----------------------------------------------------------------------------+
//|Function flow:     init encoding scheme                                     |
//|                   use the first 15 characters of the passed byte aray to   |
//|                     build encoding scheme                                  |
//|                   use the first 15 characters to build decoding scheme     |
//|                   return                                                   |
//+----------------------------------------------------------------------------+

VOID QDAMTerseInit
(
   PBTREE  pBTIda,                               // pointer to btree
   PUCHAR  pComp                                 // pointer to comp table
)
{
   USHORT usI;                                  // index
   PBTREEGLOB    pBT = pBTIda->pBTree;

   /*******************************************************************/
   /* init values                                                     */
   /*******************************************************************/
   memset( pBT->chEncodeVal, (BYTE) stEncodeBits[0].usVal, COLLATE_SIZE);
   memset( pBT->bEncodeLen, (BYTE) stEncodeBits[0].usLen, COLLATE_SIZE);
   /*******************************************************************/
   /* use the 15 first characters to build the encoding sequence      */
   /*******************************************************************/
   for ( usI = 0; usI < 15; usI++ )
   {
     pBT->bEncodeLen[*(pComp+usI)] = (BYTE) stEncodeBits[usI+1].usLen;
     pBT->chEncodeVal[*(pComp+usI)] = (BYTE) stEncodeBits[usI+1].usVal;
     pBT->chDecode[ stEncodeBits[ usI+1 ].usVal ] = *(pComp+usI);
   } /* endfor */

   return;
}



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDAMTerseData   Terse Data                               |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMTerseData( PBTREE, PUCHAR, PUSHORT );                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       This function will run the passed string against the     |
//|                   compression table.                                       |
//|                   If the resulting string is shorter the compressed one is |
//|                   used, otherwise the original is used.                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to btree structure        |
//|                   PUCHAR                 pointer to data string            |
//|                   PUSHORT                length of the string              |
//|                   PUCHAR                 pointer to output data string     |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:      The most frequent 15 characters will be stored as 3 to 5 |
//|                   bits. The rest is stored as 11 bits, i.e. '000' plus the |
//|                   character.                                               |
//+----------------------------------------------------------------------------+
//|Function flow:      -- use the modified Huffman algorithm to terse data     |
//|                    get length of passed string                             |
//|                    set output pointer and length of output string          |
//|                    while remaining length > 0                              |
//|                      encode byte; increase encodelen appropriatly          |
//|                      if byte filled up move it into data record;           |
//|                      if byte to encode is not one of the majors then       |
//|                        get next byte and put it into encode sequence       |
//|                      endif                                                 |
//|                    endwhile                                                |
//|                    if still something left in encode table                 |
//|                      put it into output string                             |
//|                    endif                                                   |
//|                    if output string shorter than input string then         |
//|                      set return code to TRUE; store length                 |
//|                    endif                                                   |
//|                    return return code                                      |
//+----------------------------------------------------------------------------+

BOOL QDAMTerseData
(
   PBTREE  pBTIda,                  //
   PUCHAR  pData,                   // pointer to data
   PULONG  pulLen,                  // length of the string
   PUCHAR  pTempRecord              // area for getting data
)
{
   ULONG    ulLen ;                    // length of string
   BOOL     fShorter = FALSE;          // new string is not shorter
   USHORT   usCurByte = 0;             // currently processed byte
   PUCHAR   pTempData;                 // pointer to temp data area
   PUCHAR   pEndData;                  // pointer to end data area
   USHORT   usI;                       // index
   USHORT   usEncodeLen = 0;           // currently encoded bits
   USHORT   usTerseLen;                // length of tersed byte
   PBTREEGLOB    pBT = pBTIda->pBTree;

#if defined(MEASURE)
  ulBeg = pGlobInfoSeg->msecs;
#endif
   // get length of passed string
   ulLen = *pulLen;


   if ( pTempRecord )
   {
      pTempData = STARTOFDATA( pBT, pTempRecord );

      pEndData = pTempData + ulLen;             // position  Tersed > untersed
      while ( ulLen > 0)
      {
         usI = *pData ++;
         ulLen --;                               // point to next character

         usTerseLen = (USHORT) pBT->bEncodeLen[usI];
         usCurByte = (usCurByte << usTerseLen) + pBT->chEncodeVal[usI];

         usEncodeLen = usEncodeLen + usTerseLen;

         /*************************************************************/
         /* byte filled up so move it into data record                */
         /*************************************************************/
         if ( usEncodeLen >= 8  )

         {
           usEncodeLen -= 8;
           if ( pTempData < pEndData  )
           {
             *pTempData = (CHAR) (usCurByte >> usEncodeLen);
             usCurByte -= (*pTempData << usEncodeLen );
           }
           else
           {
             ulLen = 0;       // loop end condition
           } /* endif */
           pTempData ++;
         } /* endif */

         /*************************************************************/
         /* if it is not one of the majors put in the full character  */
         /*************************************************************/
         if ( !pBT->chEncodeVal[usI] )
         {
           usCurByte = (usCurByte << 8) + usI;
           if ( pTempData < pEndData  )
           {
             *pTempData = (CHAR) (usCurByte >> usEncodeLen);
             usCurByte -= (*pTempData << usEncodeLen);
           }
           else
           {
             ulLen = 0;       // loop end condition
           } /* endif */
           pTempData ++;
         } /* endif */
      } /* endwhile */

      /****************************************************************/
      /* save last chunk if some bits are left...                     */
      /* move them left so they will fill up a full byte              */
      /****************************************************************/
      if ( usEncodeLen )
      {
        if ( pTempData < pEndData  )
        {
          usCurByte = ( usCurByte << ( 8 - usEncodeLen ) );
          *pTempData++ = (CHAR) usCurByte;
        } /* endif */
      } /* endif */

      /****************************************************************/
      /* check if compression longer than org data                    */
      /* return TRUE/FALSE accordingly                                */
      /****************************************************************/
      ulLen = pTempData - pTempRecord;
      if ( ulLen < *pulLen )
      {
//        *(PUSHORT) pTempRecord = (USHORT)*pulLen;  // insert record length
        SETDATALENGTH( pBT, pTempRecord, *pulLen );
        *pulLen = ulLen;                           // set new length
        fShorter = TRUE;
      } /* endif */

   } /* endif */
#if defined(MEASURE)
  ulTerseEnd += (pGlobInfoSeg->msecs -ulBeg );
#endif
   return ( fShorter );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     QDAMUnTerseData      UnTerse Data                        |
//+----------------------------------------------------------------------------+
//|Function call:     QDAMUnTerseData( PBTREE, PUCHAR, USHORT, PUSHORT );      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:       This function will run the passed string against         |
//|                   the decompression table.                                 |
//|                   If the resulting string will not fit in the passed       |
//|                   data area a warning return code is issued.               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Parameters:        PBTREE                 pointer to btree structure        |
//|                   PUCHAR                 pointer to data string            |
//|                   USHORT                 data length on input              |
//|                   PUSHORT                length of the string on output    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   SHORT                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       0                     okay                               |
//|                   BTREE_BUFFER_SMALL    provided buffer too small          |
//|                   BTREE_NO_ROOM         not enough memory                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:      The most frequent 15 characters will be stored as 3 to 5 |
//|                   bits. The rest is stored as 11 bits, i.e. '000' plus the |
//|                   character.                                               |
//+----------------------------------------------------------------------------+
//|Function flow:      -- use the modified Huffman algorithm to unterse data   |
//|                   while remaining length > 0                               |
//|                     if usStartBit < 8 then                                 |
//|                       get next byte and add it to usCurByte                |
//|                       increase usStartBit                                  |
//|                     endif                                                  |
//|                     get the next three significant bits, adjust usStartBit |
//|                     decode them according to the decode table, i.e.        |
//|                      case 0:                                               |
//|                        get next character, decode it and put it into output|
//|                        string.                                             |
//|                      case 1, 2, 3:                                         |
//|                        just decode the character and put into output string|
//|                      case 4, 6:                                            |
//|                        get the next bit and decode it afterwards           |
//|                      case 5, 7:                                            |
//|                        get the next 2 bits and decode them afterwards      |
//|                      default:                                              |
//|                        we are out of synch, indicate that something went   |
//|                        wrong.                                              |
//|                                                                            |
//|                      endswitch                                             |
//|                   endwhile                                                 |
//|                   if enough space in passed record then                    |
//|                     copy data into passed record                           |
//|                   else                                                     |
//|                     set Rc to BTREE_BUFFER_SMALL                           |
//|                   endif                                                    |
//|                   return Rc                                                |
//+----------------------------------------------------------------------------+
SHORT QDAMUnTerseData
(
   PBTREE  pBTIda,                  //
   PUCHAR  pData,                   // pointer to data
   ULONG   ulDataLen,               // data length (uncompressed)
   PULONG  pulLen                   // length of the string
)
{
   ULONG    ulLen ;                    // length of string
   PUCHAR   pTempData;                 // pointer to temp data area
   USHORT   usCurByte = 0;             // current byte
   USHORT   usStartBit = 0;            // start bit looking at from high to low
   USHORT   usDecode;                  // value to be decoded
   SHORT    sRc = 0;                   // okay;
   PSZ      pInData = (PSZ)pData;           // pointer to input data
   PBTREEGLOB    pBT = pBTIda->pBTree;
#if defined(MEASURE)
  ulBeg =  pGlobInfoSeg->msecs;
#endif

   // get length of passed string
   ulLen = ulDataLen;

   // try to uncompress the passed string ( use pTempData as temporary space)

   if ( pBT->pTempRecord )
   {
     pTempData = pBT->pTempRecord;

     while ( ulLen )
     {
       /***************************************************************/
       /* get next byte for decoding                                  */
       /***************************************************************/
       if ( usStartBit < 8 )
       {
         usCurByte = (usCurByte << 8) + *pData++;
         usStartBit += 8;
       } /* endif */

       /***************************************************************/
       /* get three significant bits                                  */
       /***************************************************************/
       usDecode = (usCurByte & us3BitNibble[usStartBit]) >> (usStartBit-3);
       usStartBit -= 3;

       /***************************************************************/
       /* decode the next character                                   */
       /***************************************************************/
       switch ( usDecode )
       {
         case  0:    // get next character
           if ( usStartBit < 8 )
           {
             usCurByte = (usCurByte << 8) + *pData++;
             usStartBit += 8;
           } /* endif */
           *pTempData++ =
              (CHAR) ((usCurByte & us8BitNibble[usStartBit]) >> (usStartBit-8));
           ulLen --;
           usStartBit -= 8;
           if ( usStartBit < 8 )
           {
             usCurByte = (usCurByte << 8) + *pData++;
             usStartBit += 8;
           } /* endif */
           break;
         case  1:
         case  2:
         case  3:    // we are done ...
           *pTempData++ = pBT->chDecode[usDecode];
           ulLen--;                       // another character found
           break;
         case  4:
         case  6:    // get the next bit and decode it afterwards
           usDecode = (usDecode<<1) +
                     ((usCurByte & us1BitNibble[usStartBit])>> (usStartBit-1));
           usStartBit --;
           *pTempData++ = pBT->chDecode[usDecode];
           ulLen--;                       // another character found
           break;
         case  5:
         case  7:    // get the next 2 bits and decode it afterwards
           usDecode = (usDecode<<2) +
                    ((usCurByte & us2BitNibble[usStartBit]) >> (usStartBit-2));
           usStartBit -= 2;
           *pTempData++ = pBT->chDecode[usDecode];
           ulLen--;                       // another character found
           break;
         default :   // we are out of synch, should never happen.....
           sRc = BTREE_CORRUPTED;
           ulLen = 0;
           break;
       } /* endswitch */

     } /* endwhile */
   } /* endif */

   /*******************************************************************/
   /* copy untersed data back ....                                    */
   /*******************************************************************/
   if ( !sRc  )
   {
     if ( *pulLen >= ulDataLen )
     {
       memcpy( pInData, pBT->pTempRecord, *pulLen );
       *pulLen = ulDataLen;
     }
     else
     {
      sRc = BTREE_BUFFER_SMALL;
     } /* endif */
   } /* endif */
#if defined(MEASURE)
  ulUnTerseEnd += (pGlobInfoSeg->msecs - ulBeg);
#endif
   return sRc;
}

