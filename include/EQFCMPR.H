/*! EQFCMPR.H
	Description: External header file for compression routines
	Copyright (c) 1990-2012, International Business Machines Corporation and others. All rights reserved.
*/

/*****************************************************************************/
/* type definitions                                                          */
/*****************************************************************************/

typedef struct bit_mem
{
  PUCHAR    pucValue;
  UCHAR     ucMask;
  ULONG     ulLength;
} BIT_MEM, FAR * PBIT_MEM;


/*****************************************************************************/
/* constant definitions                                                      */
/*****************************************************************************/

/* compression type constants */
#define     COMPRESS_AUTO     0
#define     COMPRESS_NONE     0
#define     COMPRESS_LZSS     1
#define     COMPRESS_HUFFMAN  2


/*****************************************************************************/
/* function prototypes                                                       */
/*****************************************************************************/

BOOL        fUtlHuffmanCompress (PUCHAR pucDataArea, ULONG  ulInputLength,
                                 PULONG pulOutputLength);
__declspec(dllexport)
BOOL        fUtlHuffmanExpand (PUCHAR pucDataArea, ULONG ulMaxOutputLength,
                               PULONG pulOutputLength);
BOOL        fUtlLZSSCompress (PUCHAR pucDataArea, ULONG ulInputLength,
                              PULONG pulOutputLength);
BOOL        fUtlLZSSExpand (PUCHAR pucDataArea, ULONG ulMaxOutputLength,
                            PULONG pulOutputLength);
BOOL        fUtlCompress (PUCHAR pucDataArea, ULONG ulInputLength,
                          USHORT usCompressionType, PULONG pulOutputLength);
BOOL        fUtlExpand (PUCHAR pucDataArea, ULONG ulInputLength,
                        ULONG ulMaxOutputLength, PULONG pulOutputLength);
