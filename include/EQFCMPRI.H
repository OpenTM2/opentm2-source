//+----------------------------------------------------------------------------+
//|EQFCMPRI.H                                                                  |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author: Christian Michel (CHM at SDFVM1)                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Internal header file for compression routines                  |
//|                                                                            |
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
// $Revision: 1.1 $ ----------- 20 Feb 2003
//  -- New Release TM6.0.1!!
// 
// 
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
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
 * $Header:   K:\DATA\EQFCMPRI.HV_   1.1   29 Sep 1998 07:09:14   BUILD  $
 *
 * $Log:   K:\DATA\EQFCMPRI.HV_  $
 * 
 *    Rev 1.1   29 Sep 1998 07:09:14   BUILD
 * - enabled for compression or data areas with a size more than 64k bytes
 *
 *    Rev 1.0   09 Jan 1996 09:45:14   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

/*****************************************************************************/
/* type definitions                                                          */
/*****************************************************************************/

typedef struct huffman_node
{
  USHORT    usCount;
  USHORT    usChild0;
  USHORT    usChild1;
} HUFF_NODE, FAR * PHUFF_NODE;

typedef struct huffman_code
{
  USHORT    usCode;
  USHORT    usNoOfBits;
} HUFF_CODE, FAR * PHUFF_CODE;

typedef struct lzss_node
{
  USHORT    usParent;
  USHORT    usSmallerChild;
  USHORT    usLargerChild;
} LZSS_NODE, FAR * PLZSS_NODE, FAR * FAR * PPLZSS_NODE;


/*****************************************************************************/
/* constant definitions                                                      */
/*****************************************************************************/

/* constants for Huffman encoding: */
#define END_OF_STREAM         256

/* These constants are used for LZSS encoding: */
/*  LZSS_INDEX_BITS         - number of index bits for dictionary window    */
/*  LZSS_LENGTH_BITS        - number of bits for lookahead window           */
/*  LZSS_WINDOW_SIZE        - the resulting window size (from index bits)   */
/*  LZSS_RAW_LOOKAHEAD_SIZE - raw size of lookahead window                  */
/*  LZSS_BREAK_EVEN         - number of bytes when encoding match is more   */
/*                            effective (depends on the size used to encode */
/*                            a match vs. size of raw data in bit stream    */
/*  LZSS_LOOKAHEAD_SIZE     - real lookahead size                           */
/*  LZSS_TREE_ROOT          - array index for root node of binary tree      */
/*  LZSS_UNUSED             - array index for unused node                   */
/*  LZSS_END_OF_STREAM      - code indicating end of encoded data           */
/*  LZSS_MOD_WINDOW         - macro to calculate the MODULO vs. window size */
/*  MAX_LZSS_SIZE           - maximum size of input data to be compressed   */
/*                            with LZSS compression                         */
#define LZSS_INDEX_BITS          10
#define LZSS_LENGTH_BITS         4
#define LZSS_WINDOW_SIZE         (1 << LZSS_INDEX_BITS)
#define LZSS_RAW_LOOKAHEAD_SIZE  (1 << LZSS_LENGTH_BITS)
#define LZSS_BREAK_EVEN          ((1 + LZSS_INDEX_BITS + LZSS_LENGTH_BITS) / 9)
#define LZSS_LOOKAHEAD_SIZE      (LZSS_RAW_LOOKAHEAD_SIZE + LZSS_BREAK_EVEN)
#define LZSS_TREE_ROOT           LZSS_WINDOW_SIZE
#define LZSS_UNUSED              0
#define LZSS_END_OF_STREAM       0
#define LZSS_MOD_WINDOW(usPos)   ((usPos) & (LZSS_WINDOW_SIZE - 1))
#define MAX_LZSS_SIZE            1024


/*****************************************************************************/
/* macro definitions                                                         */
/*****************************************************************************/

#define OUTPUT_BIT(psBitMem, usBitValue)                                      \
  if ( usBitValue )                                                           \
  {                                                                           \
    /* set the appropriate bit in memory */                                   \
    *(psBitMem->pucValue) |= psBitMem->ucMask;                                \
  } /* endif */                                                               \
                                                                              \
  /* rotate bit mask to right, process overflow */                            \
  psBitMem->ucMask >>= 1;                                                     \
  if ( psBitMem->ucMask == 0 )                                                \
  {                                                                           \
    psBitMem->pucValue++;             /* move to next byte in memory */       \
    *(psBitMem->pucValue) = 0;        /* initialize it to 0 */                \
    psBitMem->ucMask = 0x80;          /* set mask back to MSB */              \
    psBitMem->ulLength++;             /* increase BitMem size (in bytes) */   \
  } /* endif */


#define INPUT_BIT(psBitMem, usBitValue)                                       \
  usBitValue = ( *(psBitMem->pucValue) & psBitMem->ucMask ) ? 1 : 0;          \
                                                                              \
  /* rotate bit mask to right, process overflow */                            \
  psBitMem->ucMask >>= 1;                                                     \
  if ( psBitMem->ucMask == 0 )                                                \
  {                                                                           \
    psBitMem->pucValue++;             /* move to next byte in memory */       \
    psBitMem->ucMask = 0x80;                                                  \
  } /* endif */


/*****************************************************************************/
/* function prototypes                                                       */
/*****************************************************************************/

/* general bit oriented memory functions */
PBIT_MEM    psStartInputBitMem (PUCHAR pucInBuffer);
PBIT_MEM    psStartOutputBitMem (PUCHAR pucOutBuffer);
USHORT      usGetOutBufferSize (PBIT_MEM psBitMem);
VOID        OutputBits (PBIT_MEM psBitMem, ULONG ulBits, USHORT usBitCount);
ULONG       ulInputBits (PBIT_MEM psBitMem, USHORT usBitCount);

/* functions specific to Huffman compression */
VOID        CountBytes (PUCHAR pucData, ULONG ulDataLength, PULONG aulCounts);
VOID        ScaleCounts (PULONG aulCounts, PHUFF_NODE asNodes);
VOID        SaveScaledCounts (PBIT_MEM psBitMem, PHUFF_NODE asNodes);
VOID        LoadScaledCounts (PBIT_MEM psBitMem, PHUFF_NODE asNodes);
USHORT      usBuildTree (PHUFF_NODE asNodes);
VOID        ConvertTreeToCode (PHUFF_NODE asNodes, PHUFF_CODE asCodes,
                               USHORT usCodeSoFar, USHORT usNoOfBits,
                               USHORT usNode);
BOOL        fCompressDataHuffman (PUCHAR pucInputData, ULONG ulInputLength,
                                  PBIT_MEM psBitOutput, PHUFF_CODE asCodes);
BOOL        fExpandDataHuffman (PBIT_MEM psBitInput, PHUFF_NODE asNodes,
                                USHORT usRootNode, PUCHAR pucOutputData,
                                ULONG ulMaxOutputLength,
                                PULONG pulOutputLength);

/* functions specific to LZSS compression */
VOID        InitLZSSBinTree (PLZSS_NODE psLZSSTree, USHORT usNode);
VOID        ContractNode (PLZSS_NODE psLZSSTree, USHORT usOldNode,
                          USHORT usNewNode);
VOID        ReplaceNode (PLZSS_NODE psLZSSTree, USHORT usOldNode,
                         USHORT usNewNode);
USHORT      usFindNextNode (PLZSS_NODE psLZSSTree, USHORT usNode);
VOID        DeleteString (PLZSS_NODE psLZSSTree, USHORT usNode);
USHORT      usAddString (PLZSS_NODE psLZSSTree, PUCHAR pucDictWindow,
                         USHORT usNewNode, PUSHORT pusMatchPosition);
