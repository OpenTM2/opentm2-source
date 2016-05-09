//+----------------------------------------------------------------------------+
//|EQFCMPR.C                                                                   |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|      Copyright (C) 1990-2012, International Business Machines              |
//|      Corporation and others. All rights reserved                           |
//+----------------------------------------------------------------------------+
//|Author: Christian Michel (CHM at SDFVM1)                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description: Compression routines to be used from QDAM                      |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|  fUtlCompress                                                              |
//|  fUtlExpand                                                                |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|  fUtlHuffmanCompress                                                       |
//|  fUtlHuffmanExpand                                                         |
//|  fUtlLZSSCompress                                                          |
//|  fUtlLZSSExpand                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
//|  psStartInputBitMem                                                        |
//|  psStartOutputBitMem                                                       |
//|  ulGetOutBufferSize                                                        |
//|  OutputBits                                                                |
//|  ulInputBits                                                               |
//|  CountBytes                                                                |
//|  ScaleCounts                                                               |
//|  SaveScaledCounts                                                          |
//|  LoadScaledCounts                                                          |
//|  usBuildTree                                                               |
//|  ConvertTreeToCode                                                         |
//|  fCompressDataHuffman                                                      |
//|  fExpandDataHuffman                                                        |
//|  InitLZSSBinTree                                                           |
//|  ContractNode                                                              |
//|  ReplaceNode                                                               |
//|  FindNextNode                                                              |
//|  DeleteString                                                              |
//|  usAddString                                                               |
//|                                                                            |
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
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
//
//
// $Revision: 1.2 $ ----------- 4 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFCMPR.CV_   1.2   29 Sep 1998 07:31:02   BUILD  $
 *
 * $Log:   K:\DATA\EQFCMPR.CV_  $
 *
 *    Rev 1.2   29 Sep 1998 07:31:02   BUILD
 * - use ULONG type length fields for function parameters
 *
 *    Rev 1.1   09 Feb 1998 17:29:02   BUILD
 * - Win32: avoid compiler warning
 *
 *    Rev 1.0   09 Jan 1996 09:03:14   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+
#ifdef DEBUG
  #define INCL_DOSINFOSEG
#endif

#include "eqf.h"

#include "eqfcmpr.h"
#include "eqfcmpri.h"


/*****************************************************************************/
/* function implementation                                                   */
/*****************************************************************************/

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     psStartInputBitMem                                       |
//+----------------------------------------------------------------------------+
//|Function call:     psStartInputBitMem (PUCHAR)                              |
//+----------------------------------------------------------------------------+
//|Description:       allocate structure needed for handling of bit oriented   |
//|                   memory and initialize structure variables                |
//+----------------------------------------------------------------------------+
//|Input parameter:   PUCHAR - Address of input data area                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   PBIT_MEM                                                 |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL - memory allocation failed                          |
//|                   other - pointer to allocated structure                   |
//+----------------------------------------------------------------------------+
//|Samples:           psInputBitMem = psStartInputBitMem (pucInputData);       |
//+----------------------------------------------------------------------------+
//|Function flow:     If input data area is a valid pointer Then               |
//|                     Allocate memory for structure                          |
//|                     If memory allocation was successful Then               |
//|                       Set bit memory pointer to start of input data area   |
//|                       Initialize bit mask to 0x80 (MSB)                    |
//|                   Return pointer to bit memory structure                   |
//+----------------------------------------------------------------------------+

PBIT_MEM psStartInputBitMem (PUCHAR pucInBuffer)
{
  PBIT_MEM  psBitMem = NULL;

  if ( pucInBuffer )
  {
//    psBitMem = malloc (sizeof (BIT_MEM));
//    if ( psBitMem )

    if ( UtlAlloc ((PVOID *)&psBitMem, 0L, (LONG) sizeof (BIT_MEM), NOMSG) )
    {
      /* memory successfully allocated */
      /* start with MSB of first byte in input buffer */
      psBitMem->pucValue = pucInBuffer;
      psBitMem->ucMask = 0x80;
    } /* endif */
  } /* endif */

  return (psBitMem);
} /* end of function psStartInputBitMem */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     psStartOutputBitMem                                      |
//+----------------------------------------------------------------------------+
//|Function call:     psStartOutputBitMem (PUCHAR)                             |
//+----------------------------------------------------------------------------+
//|Description:       allocate structure needed for handling of bit oriented   |
//|                   memory and initialize structure variables                |
//+----------------------------------------------------------------------------+
//|Input parameter:   PUCHAR - Address of output data area                     |
//+----------------------------------------------------------------------------+
//|Returncode type:   PBIT_MEM                                                 |
//+----------------------------------------------------------------------------+
//|Returncodes:       NULL - memory allocation failed                          |
//|                   other - pointer to allocated structure                   |
//+----------------------------------------------------------------------------+
//|Samples:           psOutputBitMem = psStartOutputBitMem (pucOutputData);    |
//+----------------------------------------------------------------------------+
//|Function flow:     Call psStartInputBitMem                                  |
//|                   If pointer to bit memory structure is valid Then         |
//|                     Initiliaze value of first output byte to 0             |
//|                     Set length of output data area to 1 byte               |
//|                   Return pointer to bit memory structure                   |
//+----------------------------------------------------------------------------+

PBIT_MEM psStartOutputBitMem (PUCHAR pucOutBuffer)
{
  PBIT_MEM  psBitMem = NULL;

  /* memory allocation and structure initialization is the */
  /* same for input and output routines                    */
  psBitMem = psStartInputBitMem (pucOutBuffer);
  if ( psBitMem )
  {
    /* initialize output buffer value to 0, size of buffer to 1 */
    *(psBitMem->pucValue) = 0;
    psBitMem->ulLength = 1;
  } /* endif */

  return (psBitMem);
} /* end of function psStartOutputBitMem */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ulGetOutBufferSize                                       |
//+----------------------------------------------------------------------------+
//|Function call:     ulGetOutBufferSize (PBIT_MEM)                            |
//+----------------------------------------------------------------------------+
//|Description:       This function returns the size of the bit oriented output|
//|                   data area. The size is adjusted if the current byte is   |
//|                   not used.                                                |
//+----------------------------------------------------------------------------+
//|Input parameter:   PBIT_MEM - pointer to output bit memory structure        |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       number of bytes in output data area                      |
//+----------------------------------------------------------------------------+
//|Samples:           usSize = ulGetOutBufferSize (psOutputBitMem);            |
//+----------------------------------------------------------------------------+
//|Function flow:     If current byte in output bit memory is not used Then    |
//|                     Return length of bit memory reduced by 1               |
//|                   Else                                                     |
//|                     Return length of bit memory                            |
//+----------------------------------------------------------------------------+

ULONG ulGetOutBufferSize (PBIT_MEM psBitMem)
{
  if ( psBitMem->ucMask == 0x80 )
  {
    /* New byte wasn't touched, so real byte count is one less */
    return (psBitMem->ulLength - 1);
  }
  else
  {
    /* new byte was already touched, byte count is correct */
    return (psBitMem->ulLength);
  } /* endif */
} /* end of function ulGetOutBufferSize */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     OutputBits                                               |
//+----------------------------------------------------------------------------+
//|Function call:     OutputBits (PBIT_MEM, ULONG, USHORT)                     |
//+----------------------------------------------------------------------------+
//|Description:       This function writes the specified number of bits to the |
//|                   output bit memory data area.                             |
//+----------------------------------------------------------------------------+
//|Input parameter:   PBIT_MEM - pointer to output bit memory structure        |
//|                   ULONG - value to be written to output bit memory         |
//|                   USHORT - number of bits to write                         |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Samples:           OutputBits (psOutputBitMem, 234L, 8);                    |
//+----------------------------------------------------------------------------+
//|Function flow:     Set output mask to MSB according to bit count            |
//|                   While output mask is not 0 Do                            |
//|                     Output the bit masked by output mask                   |
//|                     Shift output mask to right                             |
//+----------------------------------------------------------------------------+

VOID OutputBits (PBIT_MEM psBitMem, ULONG ulBits, USHORT usBitCount)
{
  ULONG  ulMask;

  /* get bit mask for MSB of data to output */
  ulMask = 1L << (usBitCount - 1);

  /* output all bits starting with MSB going to LSB */
  while ( ulMask != 0L )
  {
    OUTPUT_BIT(psBitMem, (ulBits & ulMask));

    /* shift bit mask one right to next lower bit */
    ulMask >>= 1;
  } /* endwhile */
} /* end of function OutputBits */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ulInputBits                                              |
//+----------------------------------------------------------------------------+
//|Function call:     ulInputBits (PBIT_MEM, USHORT)                           |
//+----------------------------------------------------------------------------+
//|Description:       This function reads the specified number of bits from the|
//|                   input bit memory and returns the read value.             |
//+----------------------------------------------------------------------------+
//|Input parameter:   PBIT_MEM - pointer to input bit memory structure         |
//|                   USHORT - number of bits to read                          |
//+----------------------------------------------------------------------------+
//|Returncode type:   ULONG                                                    |
//+----------------------------------------------------------------------------+
//|Returncodes:       value of read bits                                       |
//+----------------------------------------------------------------------------+
//|Samples:           ulValue = ulInputBits (psInputBitMem, 12);               |
//+----------------------------------------------------------------------------+
//|Function flow:     Initialize output value to 0                             |
//|                   Set input bit mask according to MSB of bits to read      |
//|                   While input bit mask is not 0 Do                         |
//|                     Input one bit from input bit memory                    |
//|                     If read bit is 1 Then                                  |
//|                       Add input bit mask to output value                   |
//|                     Shift input bit mask to right                          |
//|                   Return output value                                      |
//+----------------------------------------------------------------------------+

ULONG ulInputBits (PBIT_MEM psBitMem, USHORT usBitCount)
{
  ULONG  ulMask;
  ULONG  ulReturnValue = 0L;
  USHORT usBitValue;

  /* get bit mask for MSB of data to output */
  ulMask = 1L << (usBitCount - 1);

  /* input all bits starting with MSB going to LSB */
  while ( ulMask != 0L )
  {
    INPUT_BIT(psBitMem, usBitValue);

    if ( usBitValue )
    {
      /* add bit mask value to return value */
      ulReturnValue |= ulMask;
    } /* endif */

    /* shift bit mask one right to next lower bit */
    ulMask >>= 1;
  } /* endwhile */

  return (ulReturnValue);
} /* end of function ulInputBits */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     CountBytes                                               |
//+----------------------------------------------------------------------------+
//|Function call:     CountBytes (PUCHAR, ULONG, PULONG)                       |
//+----------------------------------------------------------------------------+
//|Description:       This function reads all bytes in the supplied data area  |
//|                   and counts the occurence of each byte value. Result is   |
//|                   returned in a counts array.                              |
//+----------------------------------------------------------------------------+
//|Input parameter:   PUCHAR - Addres of data area                             |
//|                   ULONG  - Length of data area                             |
//+----------------------------------------------------------------------------+
//|Output parameter:  PULONG  - Pointer to Array [256] containing count result |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      Count array must be initialized to 0 (all entries!)      |
//+----------------------------------------------------------------------------+
//|Samples:           CountBytes (pucInputData, ulInputDataLength, aulCounts); |
//+----------------------------------------------------------------------------+
//|Function flow:     For all bytes in input data area Do                      |
//|                     Read byte                                              |
//|                     Increment counter belonging to read byte value         |
//+----------------------------------------------------------------------------+

VOID CountBytes (PUCHAR pucData, ULONG ulDataLength, PULONG aulCounts)
{
  ULONG ulIndex;

  for ( ulIndex = 0; ulIndex < ulDataLength; ulIndex++)
  {
    aulCounts [*(pucData++)]++;
  } /* endfor */
} /* end of function CountBytes */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ScaleCounts                                              |
//+----------------------------------------------------------------------------+
//|Function call:     ScaleCounts (PUSHORT, PHUFF_NODE)                        |
//+----------------------------------------------------------------------------+
//|Description:       This function is used to scale down the counts of each   |
//|                   byte in the input data area. Since the data area has a   |
//|                   maximum size of 64 KB, each count can also have up to    |
//|                   this value. For building the huffman tree, it is         |
//|                   sufficient to have 8 bit values, so this function scales |
//|                   down all counts so that only 8 bits are used.            |
//+----------------------------------------------------------------------------+
//|Input parameter:   PUSHORT - address of counts array                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  PHUFF_NODE - address of Huffman nodes array              |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Prerequesits:      nodes array must be initialized to 0 (all entries!)      |
//+----------------------------------------------------------------------------+
//|Samples:           ScaleCounts (ausCounts, asNodes);                        |
//+----------------------------------------------------------------------------+
//|Function flow:     Search maximum value in counts array                     |
//|                   If no maximum was found Then                             |
//|                     Set a dummy value                                      |
//|                   Calculate divisor                                        |
//|                   For all entries in counts and nodes arrays Do            |
//|                     Node count = Count / Divisor                           |
//|                   Node count for end of stream indicator is set to 1       |
//+----------------------------------------------------------------------------+

VOID ScaleCounts (PULONG  aulCounts, PHUFF_NODE asNodes)
{
  ULONG  ulMaxCount;
  USHORT usIndex;
  ULONG  ulDivisor;

  /* find maximum count value */
  ulMaxCount = 0;
  for ( usIndex = 0; usIndex < 256; usIndex++)
  {
    if ( aulCounts [usIndex] > ulMaxCount )
    {
      ulMaxCount = aulCounts [usIndex];
    } /* endif */
  } /* endfor */

  /* if no byte value was used, set a dummy count to the '0' byte */
  if ( ulMaxCount == 0 )
  {
    aulCounts [0] = 1;
    ulMaxCount = 1;
  } /* endif */

  /* calculate divisor (ulMaxCount/256 + 1) */
  ulDivisor = (ulMaxCount >> 4) + 1;
//  usDivisor = usMaxCount / 255 + 1;

  /* for performance reasons the 'if' is outside the 'for' loop */
  if ( ulDivisor > 1 )
  {
    for ( usIndex = 0; usIndex < 256; usIndex++)
    {
      asNodes [usIndex].usCount =(USHORT)(aulCounts [usIndex] / ulDivisor);
      if ( (asNodes [usIndex].usCount == 0) && (aulCounts [usIndex] != 0) )
      {
        asNodes [usIndex].usCount = 1;
      } /* endif */
    } /* endfor */
  }
  else
  {
    for ( usIndex = 0; usIndex < 256; usIndex++)
    {
      asNodes [usIndex].usCount = (USHORT)aulCounts [usIndex];
    } /* endfor */
  } /* endif */

  /* the end of stream value will be used once, so set its count accordingly */
  asNodes [END_OF_STREAM].usCount = 1;
} /* end of function ScaleCounts */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     SaveScaledCounts                                         |
//+----------------------------------------------------------------------------+
//|Function call:     SaveScaledCounts (PBIT_MEM, PHUFF_NODE)                  |
//+----------------------------------------------------------------------------+
//|Description:       This function is used to save the resulting count values |
//|                   in the output bit memory area. For best memory usage it  |
//|                   saves only the used count blocks, preceeded by a start   |
//|                   and stop index.                                          |
//+----------------------------------------------------------------------------+
//|Input parameter:   PHUFF_NODE - address of nodes array                      |
//+----------------------------------------------------------------------------+
//|Output parameter:  PBIT_MEM - address of output bit memory data area        |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Samples:           SaveScaledCounts (psOutputBitMem, asNodes);              |
//+----------------------------------------------------------------------------+
//|Function flow:     Find start index of first used count                     |
//|                   While end of nodes array is not reached Do               |
//|                     Find gap in used counts of at least 3 entries          |
//|                     Store counts block in output bit memory                |
//+----------------------------------------------------------------------------+

VOID SaveScaledCounts (PBIT_MEM psBitMem, PHUFF_NODE asNodes)
{
  USHORT usStart;
  USHORT usStop;
  USHORT usNext;
  USHORT usIndex;
  BOOL   fExit;

  /* search first active node */
  usStart = 0;
  while ( (usStart < 255) && (asNodes [usStart].usCount == 0) )
  {
    usStart++;
  } /* endwhile */

  /* now search for a continous area of nodes, if only one or two inactive */
  /* nodes are found, include them in the area (otherwise it would be      */
  /* ineffective); after area is found, copy it to output memory area      */
  while ( usStart < 256 )
  {
    /* first node to check is node after start node */
    usStop = usStart + 1;
    fExit = FALSE;

    do
    {
      /* find first inactive node */
      while ( (usStop < 256) && (asNodes [usStop].usCount != 0) )
      {
        usStop++;
      } /* endwhile */

      /* position usStop back to last active node */
      usStop--;

      /* now find first active node */
      usNext = usStop + 1;
      while ( (usNext < 256) && (asNodes [usNext].usCount == 0) )
      {
        usNext++;
      } /* endwhile */

      /* find loop end criteria: consecutive block is found if either more  */
      /* than 3 inactive nodes are found or if end of node array is reached */
      if ( (usNext > 255) || (usNext - usStop > 3) )
      {
        fExit = TRUE;
      }
      else
      {
        /* continue search for continous block */
        usStop = usNext;
      } /* endif */
    } while ( !fExit ); /* enddo */

    /* now write the continous count block out to memory with start and stop */
    /* index followed by the count bytes                                     */
    *(psBitMem->pucValue++) = (UCHAR) usStart;
    psBitMem->ulLength++;
    *(psBitMem->pucValue++) = (UCHAR) usStop;
    psBitMem->ulLength++;
    for ( usIndex = usStart; usIndex <= usStop; usIndex++ )
    {
      *(psBitMem->pucValue++) = (UCHAR) asNodes [usIndex].usCount;
      psBitMem->ulLength++;
    } /* endfor */

    usStart = usNext;
  } /* endwhile */

  /* mark the end of the count table with the 0 value */
  *(psBitMem->pucValue++) = 0;
  psBitMem->ulLength++;
} /* end of function SaveScaledCounts */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     LoadScaledCounts                                         |
//+----------------------------------------------------------------------------+
//|Function call:     LoadScaledCounts (PBIT_MEM, PHUFF_NODE)                  |
//+----------------------------------------------------------------------------+
//|Description:       This function reads in the counts array from the input   |
//|                   bit memory. It reads a start and stop index and then all |
//|                   values for the entries between these two indices. End of |
//|                   the counts array is marked with a start index of 0.      |
//+----------------------------------------------------------------------------+
//|Input parameter:   PBIT_MEM - address of output bit memory data area        |
//+----------------------------------------------------------------------------+
//|Output parameter:  PHUFF_NODE - address of nodes array                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Samples:           LoadScaledCounts (asNodes, psInputBitMem);               |
//+----------------------------------------------------------------------------+
//|Function flow:     Initialize counts in nodes array to 0                    |
//|                   Read start and stop index from input bit memory          |
//|                   While end of counts not reached                          |
//|                     Read in counts and store them to nodes array           |
//|                     Read new start and stop index                          |
//|                   Set count of END_OF_STREAM value to 1                    |
//+----------------------------------------------------------------------------+

VOID LoadScaledCounts (PBIT_MEM psBitMem, PHUFF_NODE asNodes)
{
  USHORT    usStart;
  USHORT    usStop;
  USHORT    usIndex;

  /* at first clear the count values */
  for ( usIndex = 0; usIndex < 256; usIndex++)
  {
    asNodes [usIndex].usCount = 0;
  } /* endfor */

  /* then read start and stop index from input memory area */
  usStart = (USHORT) *(psBitMem->pucValue++);
  usStop = (USHORT) *(psBitMem->pucValue++);
  do
  {
    for ( usIndex = usStart; usIndex <= usStop; usIndex++ )
    {
      asNodes [usIndex].usCount = (USHORT) *(psBitMem->pucValue++);
    } /* endfor */

    usStart = (USHORT) *(psBitMem->pucValue++);
    if ( usStart != 0 )
    {
      /* end of count table not reached, read also the stop index */
      usStop = (USHORT) *(psBitMem->pucValue++);
    } /* endif */
  } while ( usStart != 0 ); /* enddo */

  /* set the count for the END_OF_STREAM code to 1 (always the same) */
  asNodes [END_OF_STREAM].usCount = 1;
} /* end of function LoadScaledCounts  */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     usBuildTree                                              |
//+----------------------------------------------------------------------------+
//|Function call:     usBuildTree (PHUFF_NODE)                                 |
//+----------------------------------------------------------------------------+
//|Description:       This function creates the binary tree structure needed to|
//|                   build the huffman codes.                                 |
//+----------------------------------------------------------------------------+
//|Input parameter:   PHUFF_NODE - address of nodes array                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       array index of root node                                 |
//+----------------------------------------------------------------------------+
//|Samples:           usRootNode = usBuildTree (asNodes);                      |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

USHORT usBuildTree (PHUFF_NODE asNodes)
{
  USHORT    usNextFree;
  USHORT    usIndex;
  USHORT    usMin1;
  USHORT    usMin2;

  /* set the last node count to maximum available value so that it will */
  /* serve as a end of array indicator                                  */
  asNodes [513].usCount = 0xFFFF;

  /* the next free node will be the node following the END_OF_STREAM node */
  usNextFree = END_OF_STREAM + 1;

  do
  {
    usMin1 = 513;
    usMin2 = 513;

    for ( usIndex = 0; usIndex < usNextFree; usIndex++)
    {
      if ( asNodes [usIndex].usCount != 0 )
      {
        /* active node found */
        if ( asNodes [usIndex].usCount < asNodes [usMin1].usCount )
        {
          usMin2 = usMin1;
          usMin1 = usIndex;
        }
        else
        {
          if ( asNodes [usIndex].usCount < asNodes [usMin2].usCount )
          {
            usMin2 = usIndex;
          } /* endif */
        } /* endif */
      } /* endif */
    } /* endfor */

    if ( usMin2 != 513 )
    {
      /* two minimum values were found, combine these nodes to one parent */
      /* node and set the two child nodes inactive                        */
      asNodes [usNextFree].usCount = asNodes [usMin1].usCount +
                                     asNodes [usMin2].usCount;
      asNodes [usMin1].usCount = 0;
      asNodes [usMin2].usCount = 0;
      asNodes [usNextFree].usChild0 = usMin1;
      asNodes [usNextFree].usChild1 = usMin2;
      usNextFree++;
    } /* endif */
  } while ( usMin2 != 513 ); /* enddo */

  /* set usNextFree to last active node, it is the root of the tree */
  usNextFree--;

  return (usNextFree);
} /* end of function usBuildTree */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ConvertTreeToCode                                        |
//+----------------------------------------------------------------------------+
//|Function call:     ConvertTreeToCode (PHUFF_NODE, PHUFF_CODE, USHORT,       |
//|                                      USHORT, USHORT)                       |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PHUFF_NODE - address of nodes array                      |
//|                   USHORT - huffman code to start with                      |
//|                   USHORT - number of bits in start huffman code            |
//|                   USHORT - index of current node in nodes array            |
//|Parameters:        PHUFF_CODE - address of codes array                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Samples:           ConvertTreeToCode (asNodes, asCodes, 0, 0, usNode);      |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

VOID ConvertTreeToCode (PHUFF_NODE asNodes, PHUFF_CODE asCodes,
                        USHORT usCodeSoFar, USHORT usNoOfBits, USHORT usNode)
{
  /* walk recursively through the Huffman tree and add the child bits to */
  /* the code generated so far                                           */
  if ( usNode <= END_OF_STREAM )
  {
    /* leaf of the tree was encountered, store the aggregated bits in the */
    /* code array together with code size (no of bits)                    */
    asCodes [usNode].usCode = usCodeSoFar;
    asCodes [usNode].usNoOfBits = usNoOfBits;
  }
  else
  {
    /* shift code to left and call function recursively */
    usCodeSoFar <<= 1;
    usNoOfBits++;
    ConvertTreeToCode (asNodes, asCodes, usCodeSoFar, usNoOfBits,
                       asNodes [usNode].usChild0);
    ConvertTreeToCode (asNodes, asCodes, (USHORT)(usCodeSoFar | 1), usNoOfBits,
                       asNodes [usNode].usChild1);
  } /* endif */
} /* end of function ConvertTreeToCode */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     fCompressDataHuffman                                     |
//+----------------------------------------------------------------------------+
//|Function call:     fCompressDataHuffman (PUCHAR, ULONG, PBIT_MEM,           |
//|                                         PHUFF_CODE)                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PUCHAR - address of input data area                      |
//|                   USHORT - size of input data area                         |
//|                   PBIT_MEM - address of output bit memory structure        |
//|                   PHUFF_CODE - address of huffman codes array              |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE - compression successful                            |
//|                   FALSE - compression unsuccessful (possible reasons:      |
//|                           memory exhausted, algorithm ineffective)         |
//+----------------------------------------------------------------------------+
//|Samples:           fRc = fCompressDataHuffman (pucInputData, usInputLength, |
//|                                               psOutputBitMem, asNodes);    |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

BOOL fCompressDataHuffman (PUCHAR pucInputData, ULONG ulInputLength,
                           PBIT_MEM psBitOutput, PHUFF_CODE asCodes)
{
  ULONG ulIndex;
  BOOL   fRc = TRUE;

  for ( ulIndex = 0; ulIndex < ulInputLength; ulIndex++ )
  {
    OutputBits (psBitOutput, (ULONG) asCodes [*pucInputData].usCode,
                asCodes [*pucInputData].usNoOfBits);
    pucInputData++;
    if ( psBitOutput->ulLength > ulInputLength )
    {
      fRc = FALSE;
      break;
    } /* endif */
  } /* endfor */

  /* output the end of stream bits */
  OutputBits (psBitOutput, (ULONG) asCodes [END_OF_STREAM].usCode,
              asCodes [END_OF_STREAM].usNoOfBits);

  return (fRc);
} /* end of function CompressDataHuffman */



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     fUtlHuffmanCompress                                      |
//+----------------------------------------------------------------------------+
//|Function call:     fUtlHuffmanCompress (PUCHAR, USHORT, PUSHORT)            |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   PUCHAR - address of input and output data area           |
//|                   USHORT - size of input data area in bytes                |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT - pointer to size of output data area            |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE - compression successful                            |
//|                   FALSE - compression unsuccessful (possible reasons:      |
//|                           memory exhausted, algorithm ineffective)         |
//+----------------------------------------------------------------------------+
//|Samples:           fRc = fUtlHuffmanCompress (pucData, usDataSize,          |
//|                                              &usCompressedSize);           |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

BOOL fUtlHuffmanCompress (PUCHAR pucDataArea, ULONG ulInputLength,
                          PULONG pulOutputLength)
{
  BOOL         fRc = TRUE;
  ULONG        ulCompressSize;
  USHORT       usRootNode;
  PUCHAR       pucCompressedData = NULL;
  PHUFF_NODE   asNodes = NULL;
  PHUFF_CODE   asCodes = NULL;
  PULONG       aulCounts = NULL;
  PBIT_MEM     psOutputBitMem = NULL;

  /* this routine compresses an input memory block and then returns it in */
  /* the same memory area together with its length;                       */
  /* pure compression is done, returned memory will contain the raw       */
  /* compressed data, no length byte preceeded or encoding method ID;     */
  /* if output length is the same as input length, it is most likely that */
  /* no compression has been performed (i.e. compressed data was larger   */
  /* than uncompressed data)                                              */

  /* in case any of the memory allocations fails, output length is set to */
  /* input length so the input buffer can be returned in any case         */
  *pulOutputLength = ulInputLength;

  /* at first allocate all necessary memory needed for the compression */

  /* add some buffer bytes to allow safe recognition of overflow */
  ulCompressSize = ulInputLength + 5;

  if ( fRc )
  {
    /* allocate memory and initialize each byte to 0 */
    fRc = UtlAlloc ((PVOID *)&pucCompressedData, 0L, ulCompressSize, NOMSG);
  } /* endif */

  if ( fRc )
  {
    /* allocate memory and initialize each byte to 0 */
    fRc = UtlAlloc ((PVOID *)&aulCounts, 0L, (LONG) (256 * sizeof (ULONG)), NOMSG);
  } /* endif */

  if ( fRc )
  {
    /* allocate memory and initialize each byte to 0 */
    fRc = UtlAlloc ((PVOID *)&asNodes, 0L, (LONG) (514 * sizeof (HUFF_NODE)), NOMSG);
  } /* endif */

  if ( fRc )
  {
    /* allocate memory and initialize each byte to 0 */
    fRc = UtlAlloc ((PVOID *)&asCodes, 0L, (LONG) (257 * sizeof (HUFF_CODE)), NOMSG);
  } /* endif */

  /* allocate output bit memory structure */
  if ( fRc )
  {
    psOutputBitMem = psStartOutputBitMem (pucCompressedData);
    if ( !psOutputBitMem )
    {
      fRc = FALSE;
    } /* endif */
  } /* endif */

  if ( fRc )
  {
    /* count byte occurences in input data area */
    CountBytes (pucDataArea, ulInputLength, aulCounts);

    /* Scale counts to 8 bit values */
    ScaleCounts (aulCounts, asNodes);

    /* output the counts array to the output memory area */
    SaveScaledCounts (psOutputBitMem, asNodes);

    /* create the Huffman tree */
    usRootNode = usBuildTree (asNodes);

    /* convert the Huffman tree to the appropriate codes */
    ConvertTreeToCode (asNodes, asCodes, 0, 0, usRootNode);

    /* now after all things have been prepared, compress the data */
    fRc = fCompressDataHuffman (pucDataArea, ulInputLength, psOutputBitMem,
                                asCodes);
  } /* endif */

  /* if compression was successful, copy the output memory area to the */
  /* input buffer and set the output size accordingly                  */
  ulCompressSize = ulGetOutBufferSize(psOutputBitMem);
  fRc &= ulCompressSize < ulInputLength;
  if ( fRc )
  {
    *pulOutputLength = ulCompressSize;
    memcpy (pucDataArea, pucCompressedData, ulCompressSize);
  } /* endif */

  /* free all allocated memory */
  if ( pucCompressedData ) UtlAlloc ((PVOID *)&pucCompressedData, 0L, 0L, NOMSG);
  if ( aulCounts )         UtlAlloc ((PVOID *)&aulCounts, 0L, 0L, NOMSG);
  if ( asNodes )           UtlAlloc ((PVOID *)&asNodes, 0L, 0L, NOMSG);
  if ( asCodes )           UtlAlloc ((PVOID *)&asCodes, 0L, 0L, NOMSG);
  if ( psOutputBitMem )    UtlAlloc ((PVOID *)&psOutputBitMem, 0L, 0L, NOMSG);

  return (fRc);
} /* end of function fUtlHuffmanCompress */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     fExpandDataHuffman                                       |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

BOOL fExpandDataHuffman (PBIT_MEM psBitInput, PHUFF_NODE asNodes,
                         USHORT usRootNode, PUCHAR pucOutputData,
                         ULONG  ulMaxOutputLength, PULONG  pulOutputLength)
{
  /* This routine expands the data provided in the input structure       */
  /* psBitInput. As each symbol is read from the input bits, the huffman */
  /* tree is traversed. If a leaf node is found, the found code is put   */
  /* into the output memory area. This process is repeated until the     */
  /* END_OF_STREAM code is encountered.                                  */

  USHORT    usNode;
  USHORT    usInputBit;
  BOOL      fRc = TRUE;

  do
  {
    /* set starting node to root node */
    usNode = usRootNode;
    do
    {
      INPUT_BIT(psBitInput, usInputBit);
      if ( usInputBit )
      {
        usNode = asNodes [usNode].usChild1;
      }
      else
      {
        usNode = asNodes [usNode].usChild0;
      } /* endif */
    } while ( usNode > END_OF_STREAM ); /* enddo */

    if ( (usNode < END_OF_STREAM) && (*pulOutputLength < ulMaxOutputLength) )
    {
      /* this is a valid code, store it in output memory area */
      *(pucOutputData++) = (UCHAR) usNode;
      (*pulOutputLength)++;
    } /* endif */
  } while ( (usNode != END_OF_STREAM) &&
            (*pulOutputLength <= ulMaxOutputLength) ); /* enddo */

  /* check for error condition at end of expansion */
  if ( (usNode != END_OF_STREAM) &&
       (*pulOutputLength == ulMaxOutputLength) )
  {
    /* expansion was stopped because of output buffer overflow */
    fRc = FALSE;
  } /* endif */

  return (fRc);
} /* end of function fExpandDataHuffman */



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     fUtlHuffmanExpand                                        |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

BOOL fUtlHuffmanExpand (PUCHAR pucDataArea, ULONG ulMaxOutputLength,
                        PULONG pulOutputLength)
{
  BOOL         fRc = TRUE;
  USHORT       usRootNode;
  PUCHAR       pucExpandedData = NULL;
  PHUFF_NODE   asNodes = NULL;
  PUSHORT      ausCounts = NULL;
  PBIT_MEM     psInputBitMem = NULL;

  /* This routine expands the data provided in pucDataArea and returns the   */
  /* uncompressed data in the same area. At first it reads the scaled counts */
  /* stored in the compressed data area, then builds the huffman tree and    */
  /* then decodes the compressed data. If the output buffer overflows, the   */
  /* return value is set to FALSE and the data area is returned unchanged.   */

  /* initialize output length to 0 */
  *pulOutputLength = 0;

  /* then allocate needed buffers */
  fRc = UtlAlloc ((PVOID *)&pucExpandedData, 0L, ulMaxOutputLength + 2, NOMSG);

  if ( fRc )
  {
    fRc = UtlAlloc( (PVOID *)&ausCounts, 0L, (LONG)(256 * sizeof(USHORT)), NOMSG);
  } /* endif */

  if ( fRc )
  {
    fRc = UtlAlloc ((PVOID *)&asNodes, 0L, (LONG) (514 * sizeof (HUFF_NODE)), NOMSG);
  } /* endif */

  if ( fRc )
  {
    /* allocate input bit memory structure */
    psInputBitMem = psStartInputBitMem (pucDataArea);
    if ( !psInputBitMem )
    {
      fRc = FALSE;
    } /* endif */
  } /* endif */

  if ( fRc )
  {
    /* now read scaled counts */
    LoadScaledCounts (psInputBitMem, asNodes);

    /* create the Huffman tree */
    usRootNode = usBuildTree (asNodes);

    /* now expand the data */
    fRc = fExpandDataHuffman (psInputBitMem, asNodes, usRootNode,
                              pucExpandedData, ulMaxOutputLength,
                              pulOutputLength);
  } /* endif */

  if ( fRc )
  {
    /* if expansion was ok, copy the data to the data area */
    memcpy (pucDataArea, pucExpandedData, *pulOutputLength);
  }
  else
  {
    /* expansion was unsuccessful, set output length to 0 */
    *pulOutputLength = 0;
  } /* endif */

  /* free all allocated memory */
  if ( pucExpandedData )
  {
    UtlAlloc ((PVOID *)&pucExpandedData, 0L, 0L, NOMSG);
  } /* endif */

  if ( ausCounts )
  {
    UtlAlloc ((PVOID *)&ausCounts, 0L, 0L, NOMSG);
  } /* endif */

  if ( asNodes )
  {
    UtlAlloc ((PVOID *)&asNodes, 0L, 0L, NOMSG);
  } /* endif */

  if ( psInputBitMem )
  {
    UtlAlloc ((PVOID *)&psInputBitMem, 0L, 0L, NOMSG);
  } /* endif */

  return (fRc);
} /* end of function fUtlHuffmanExpand */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     InitLZSSBinTree                                          |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

VOID InitLZSSBinTree (PLZSS_NODE psLZSSTree, USHORT usNode)
{
  psLZSSTree [LZSS_TREE_ROOT].usLargerChild = usNode;
  psLZSSTree [usNode].usParent = LZSS_TREE_ROOT;
  psLZSSTree [usNode].usLargerChild = LZSS_UNUSED;
  psLZSSTree [usNode].usSmallerChild = LZSS_UNUSED;
} /* end of function InitLZSSBinTree */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ContractNode                                             |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

VOID ContractNode (PLZSS_NODE psLZSSTree, USHORT usOldNode, USHORT usNewNode)
{
  /* This function is used when a node has to be deleted from the  */
  /* binary tree. The link to its descendants is broken by pulling */
  /* it in to overlay the existing link.                           */

  USHORT    usOldParent;

  usOldParent = psLZSSTree [usOldNode].usParent;

  /* set parent of new node to old parent */
  psLZSSTree [usNewNode].usParent = usOldParent;

  /* insert new node into child index of parent */
  if ( psLZSSTree [usOldParent].usLargerChild == usOldNode )
  {
    psLZSSTree [usOldParent].usLargerChild = usNewNode;
  }
  else
  {
    psLZSSTree [usOldParent].usSmallerChild = usNewNode;
  } /* endif */

  /* set old node to unused */
  psLZSSTree [usOldNode].usParent = LZSS_UNUSED;
} /* end of function ContractNode */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ReplaceNode                                              |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

VOID ReplaceNode (PLZSS_NODE psLZSSTree, USHORT usOldNode, USHORT usNewNode)
{
  /* This function is used to replace a node that is to be deleted with */
  /* a node that wasn't in the tree previously.                         */
  USHORT    usOldParent;

  usOldParent = psLZSSTree [usOldNode].usParent;

  /* insert new node into child index of parent */
  if ( psLZSSTree [usOldParent].usLargerChild == usOldNode )
  {
    psLZSSTree [usOldParent].usLargerChild = usNewNode;
  }
  else
  {
    psLZSSTree [usOldParent].usSmallerChild = usNewNode;
  } /* endif */

  /* copy all values from old to new node (node indices) */
  psLZSSTree [usNewNode] = psLZSSTree [usOldNode];

  /* set parent of childs to new node */
  psLZSSTree [psLZSSTree [usNewNode].usSmallerChild].usParent = usNewNode;
  psLZSSTree [psLZSSTree [usNewNode].usLargerChild].usParent = usNewNode;

  /* set old node to unused */
  psLZSSTree [usOldNode].usParent = LZSS_UNUSED;
} /* end of function ReplaceNode */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FindNextNode                                             |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

USHORT usFindNextNode (PLZSS_NODE psLZSSTree, USHORT usNode)
{
  /* This function returns the index for the next smaller node in the */
  /* binary tree. This node is found by moving to the smaller child   */
  /* of the passed node and then looking for the end of the larger    */
  /* child chain.                                                     */

  USHORT    usNextNode;

  /* go to smaller child */
  usNextNode = psLZSSTree [usNode].usSmallerChild;

  /* look for largest child in chain */
  while ( psLZSSTree [usNextNode].usLargerChild != LZSS_UNUSED )
  {
    usNextNode = psLZSSTree [usNextNode].usLargerChild;
  } /* endwhile */

  return (usNextNode);
} /* end of function usFindNextNode */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     DeleteString                                             |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

VOID DeleteString (PLZSS_NODE psLZSSTree, USHORT usNode)
{
  /* This function performs the classic binary tree deletion. If the node */
  /* to be deleted has a null link, the non-null link replaces the        */
  /* existing node, otherwise the next link in order is deleted.          */

  USHORT    usReplacementNode;

  if ( psLZSSTree [usNode].usParent != LZSS_UNUSED )
  {
    if ( psLZSSTree [usNode].usLargerChild == LZSS_UNUSED )
    {
      ContractNode (psLZSSTree, usNode, psLZSSTree [usNode].usSmallerChild);
    }
    else
    {
      if ( psLZSSTree [usNode].usSmallerChild == LZSS_UNUSED )
      {
        ContractNode (psLZSSTree, usNode, psLZSSTree [usNode].usLargerChild);
      }
      else
      {
        usReplacementNode = usFindNextNode (psLZSSTree, usNode);
        DeleteString (psLZSSTree, usReplacementNode);
        ReplaceNode (psLZSSTree, usNode, usReplacementNode);
      } /* endif */
    } /* endif */
  } /* endif */
} /* end of function DeleteString */



//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     usAddString                                              |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

USHORT usAddString (PLZSS_NODE psLZSSTree, PUCHAR pucDictWindow,
                    USHORT usNewNode, PUSHORT pusMatchPosition)
{
  /* This function carries out most of the work needed for the LZSS encoding. */
  /* It has to add new nodes to the binary tree and also has to find the best */
  /* match in the already existing nodes. If a duplicate of the new node is   */
  /* already in the tree, it has to be removed for efficiency reasons.        */

  USHORT    usIndex;
  USHORT    usTestNode;
  SHORT     sDelta = 0;
  USHORT    usMatchLength = 0;
  PUSHORT   pusChildNode;
  BOOL      fMatchFound = FALSE;

  if ( usNewNode != LZSS_END_OF_STREAM )
  {
    /* start search with larger child of root */
    usTestNode = psLZSSTree [LZSS_TREE_ROOT].usLargerChild;

    do
    {
      for ( usIndex = 0; usIndex < LZSS_LOOKAHEAD_SIZE; usIndex++)
      {
        sDelta = pucDictWindow [LZSS_MOD_WINDOW(usNewNode + usIndex)] -
                 pucDictWindow [LZSS_MOD_WINDOW(usTestNode + usIndex)];
        if ( sDelta != 0 )
        {
          /* difference found, leave for loop */
          break;
        } /* endif */
      } /* endfor */

      /* check match length */
      if ( usIndex >= usMatchLength )
      {
        usMatchLength = usIndex;
        *pusMatchPosition = usTestNode;
      } /* endif */

      if ( usMatchLength >= LZSS_LOOKAHEAD_SIZE )
      {
        /* this is the largest match possible, search can be ended */
        fMatchFound = TRUE;
        ReplaceNode (psLZSSTree, usTestNode, usNewNode);
      }
      else
      {
        if ( sDelta >= 0 )
        {
          /* new node is greater than tested node, move to next larger node */
          pusChildNode = &(psLZSSTree [usTestNode].usLargerChild);
        }
        else
        {
          /* new node is less than tested node, move to next smaller node */
          pusChildNode = &(psLZSSTree [usTestNode].usSmallerChild);
        } /* endif */

        if ( *pusChildNode == LZSS_UNUSED )
        {
          /*  child is unused, previous found match (if any) must be used */
          fMatchFound = TRUE; /* abandon search for next match */

          /* copy new node to child node */
          *pusChildNode = usNewNode;
          psLZSSTree [usNewNode].usParent = usTestNode;
          psLZSSTree [usNewNode].usLargerChild = LZSS_UNUSED;
          psLZSSTree [usNewNode].usSmallerChild = LZSS_UNUSED;
        }
        else
        {
          /* child is available, test it for match */
          usTestNode = *pusChildNode;
        } /* endif */
      } /* endif */
    } while ( !fMatchFound ); /* enddo */
  } /* endif */

  return (usMatchLength);
} /* end of function usAddString */



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     fUtlLZSSCompress                                         |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

BOOL fUtlLZSSCompress (PUCHAR pucDataArea, ULONG ulInputLength,
                       PULONG pulOutputLength)
{
  BOOL         fRc = TRUE;
  ULONG        ulCompressSize = 0;
  PUCHAR       pucCompressedData = NULL;
  PUCHAR       pucInputData = pucDataArea;
  PBIT_MEM     psOutputBitMem = NULL;
  USHORT       usIndex;
  USHORT       usInputCount;
  USHORT       usLookAheadBytes;
  USHORT       usCurrentWindowPos;
  USHORT       usReplaceCount;
  USHORT       usMatchLength;
  USHORT       usMatchPosition;
  PUCHAR       pucDictWindow = NULL;
  PLZSS_NODE   psLZSSTree = NULL;
  USHORT       usWorkIndex;
  USHORT       usBitValue = 0;

  /* this routine compresses an input memory block and then returns it in */
  /* the same memory area together with its length;                       */
  /* pure compression is done, returned memory will contain the raw       */
  /* compressed data, no length byte preceeded or encoding method ID;     */
  /* if output length is the same as input length, it is most likely that */
  /* no compression has been performed (i.e. compressed data was larger   */
  /* than uncompressed data)                                              */

  /* in case any of the memory allocations fails, output length is set to */
  /* input length so the input buffer can be returned in any case         */
  *pulOutputLength = ulInputLength;

  /* at first allocate all necessary memory needed for the compression */
  if ( ulInputLength < 65500 )
  {
    /* add some buffer bytes to allow safe recognition of overflow */
    ulCompressSize = ulInputLength + 35;
  }
  else
  {
    fRc = FALSE;
  } /* endif */

  if ( fRc )
  {
    /* allocate memory and initialize each byte to 0 */
//    pucCompressedData = calloc (usCompressSize, 1);
//    if ( !pucCompressedData )
//    {
//      fRc = FALSE;
//    } /* endif */
    fRc = UtlAlloc ((PVOID *)&pucCompressedData, 0L, ulCompressSize, NOMSG);
  } /* endif */

  if ( fRc )
  {
    /* allocate memory and initialize each byte to 0 */
//    pucDictWindow = calloc (LZSS_WINDOW_SIZE, sizeof (UCHAR));
//    if ( !pucDictWindow )
//    {
//      fRc = FALSE;
//    } /* endif */
    fRc = UtlAlloc ((PVOID *)&pucDictWindow, 0L,
                    (LONG) (LZSS_WINDOW_SIZE * sizeof (UCHAR)), NOMSG);
  } /* endif */

  if ( fRc )
  {
    /* allocate memory and initialize each byte to 0 */
//    psLZSSTree = calloc (LZSS_WINDOW_SIZE + 1, sizeof (LZSS_NODE));
//    if ( !psLZSSTree )
//    {
//      fRc = FALSE;
//    } /* endif */
    fRc = UtlAlloc ((PVOID *)&psLZSSTree, 0L,
                    (LONG) ((LZSS_WINDOW_SIZE+1) * sizeof (LZSS_NODE)), NOMSG);
  } /* endif */

  /* allocate output bit memory structure */
  if ( fRc )
  {
    psOutputBitMem = psStartOutputBitMem (pucCompressedData);
    if ( !psOutputBitMem )
    {
      fRc = FALSE;
    } /* endif */
  } /* endif */

  if ( fRc )
  {
    /* now start compression        */
    /* set count of bytes read to 0 */
    usInputCount = 0;
    usCurrentWindowPos = 1;

    /* fill look ahead window with bytes read from input data area */
    for ( usIndex = 0; usIndex < LZSS_LOOKAHEAD_SIZE; usIndex++ )
    {
      pucDictWindow [usCurrentWindowPos + usIndex] = *(pucInputData++);
      usInputCount++;
      if ( usInputCount == ulInputLength )
      {
        /* all input bytes have been read, leave for loop */
        break;
      } /* endif */
    } /* endfor */

    /* set size of look ahead window */
    usLookAheadBytes = usIndex;

    /* initialize binary tree */
    InitLZSSBinTree (psLZSSTree, usCurrentWindowPos);

    /* initialize search variables */
    usMatchLength = 0;
    usMatchPosition = 0;

    while ( (usLookAheadBytes > 0) && fRc )
    {
      if ( usMatchLength > usLookAheadBytes )
      {
        /* match longer than bytes in look ahead buffer, reset it to max len */
        usMatchLength = usLookAheadBytes;
      } /* endif */

      if ( usMatchLength <= LZSS_BREAK_EVEN )
      {
        /* match is not long enough, so a single character is output */
        usReplaceCount = 1;

        /* single character is preceeded by a '1' bit */
        if ( psOutputBitMem->ulLength < ulInputLength )
        {
		  usBitValue = 1;
          OUTPUT_BIT(psOutputBitMem, usBitValue);
          OutputBits (psOutputBitMem,
                      (ULONG) pucDictWindow [usCurrentWindowPos], 8);
        }
        else
        {
          fRc = FALSE;
        } /* endif */
      }
      else
      {
        /* match is long enough to save space */
        usReplaceCount = usMatchLength;

        /* dictionary index is preceeded by a '0' bit */
        if ( psOutputBitMem->ulLength < ulInputLength )
        {
		  usBitValue = 0;
          OUTPUT_BIT(psOutputBitMem,usBitValue);
          OutputBits (psOutputBitMem, (ULONG) usMatchPosition, LZSS_INDEX_BITS);
          OutputBits (psOutputBitMem,
                      (ULONG) (usMatchLength - (LZSS_BREAK_EVEN + 1)),
                      LZSS_LENGTH_BITS);
        }
        else
        {
          fRc = FALSE;
        } /* endif */
      } /* endif */

      for ( usIndex = 0; usIndex < usReplaceCount; usIndex++ )
      {
        /* Delete string from binary tree */
        usWorkIndex = LZSS_MOD_WINDOW(usCurrentWindowPos + LZSS_LOOKAHEAD_SIZE);
        DeleteString (psLZSSTree, usWorkIndex);

        /* read new byte, if available */
        if ( usInputCount < ulInputLength )
        {
          pucDictWindow [usWorkIndex] = *(pucInputData++);
          usInputCount++;
        }
        else
        {
          usLookAheadBytes--;
        } /* endif */

        /* increase current position */
        usCurrentWindowPos = LZSS_MOD_WINDOW(usCurrentWindowPos + 1);

        /* if bytes in look ahead buffer, search for match and add string */
        if ( usLookAheadBytes )
        {
          usMatchLength = usAddString (psLZSSTree, pucDictWindow,
                                       usCurrentWindowPos, &usMatchPosition);
        } /* endif */
      } /* endfor */
    } /* endwhile */

    /* after compression is done, output end of stream indicator */
    if ( psOutputBitMem->ulLength < ulInputLength )
    {
	  usBitValue = 0;
      OUTPUT_BIT(psOutputBitMem, usBitValue);
      OutputBits (psOutputBitMem, (ULONG) LZSS_END_OF_STREAM, LZSS_INDEX_BITS);
    }
    else
    {
      fRc = FALSE;
    } /* endif */
  } /* endif */

  /* if compression was successful, copy the output memory area to the */
  /* input buffer and set the output size accordingly                  */
  ulCompressSize = ulGetOutBufferSize (psOutputBitMem);
  fRc &= ulCompressSize < ulInputLength;
  if ( fRc )
  {
    *pulOutputLength = ulCompressSize;
    memcpy (pucDataArea, pucCompressedData, ulCompressSize);
  } /* endif */

  /* free all allocated memory */
  if ( pucCompressedData )
  {
//    free (pucCompressedData);
    UtlAlloc ((PVOID *)&pucCompressedData, 0L, 0L, NOMSG);
  } /* endif */

  if ( pucDictWindow )
  {
//    free (pucDictWindow);
    UtlAlloc ((PVOID *)&pucDictWindow, 0L, 0L, NOMSG);
  } /* endif */

  if ( psLZSSTree )
  {
//    free (psLZSSTree);
    UtlAlloc ((PVOID *)&psLZSSTree, 0L, 0L, NOMSG);
  } /* endif */

  if ( psOutputBitMem )
  {
//    free (psOutputBitMem);
    UtlAlloc ((PVOID *)&psOutputBitMem, 0L, 0L, NOMSG);
  } /* endif */

  return (fRc);
} /* end of function fUtlLZSSCompress */



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     fUtlLZSSExpand                                           |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       _                                                        |
//+----------------------------------------------------------------------------+
//|Input parameter:   _                                                        |
//|Parameters:        _                                                        |
//+----------------------------------------------------------------------------+
//|Output parameter:  _                                                        |
//+----------------------------------------------------------------------------+
//|Returncode type:   _                                                        |
//+----------------------------------------------------------------------------+
//|Returncodes:       _                                                        |
//+----------------------------------------------------------------------------+
//|Samples:           _                                                        |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

BOOL fUtlLZSSExpand (PUCHAR pucDataArea, ULONG ulMaxOutputLength,
                     PULONG pulOutputLength)
{
  BOOL         fRc = TRUE;
  PUCHAR       pucExpandedData = NULL;
  PUCHAR       pucDictWindow = NULL;
  PUCHAR       pucOutputData;
  PBIT_MEM     psInputBitMem = NULL;
  UCHAR        ucByte;
  USHORT       usBit;
  USHORT       usIndex;
  USHORT       usCurrentWindowPos;
  USHORT       usMatchLength;
  USHORT       usMatchPosition = 1; /* initialize to 1 (different to End of */
                                    /* stream indicator)                    */

  /* This routine expands the data provided in pucDataArea and returns the   */
  /* uncompressed data in the same area. At first it reads the scaled counts */
  /* stored in the compressed data area, then builds the huffman tree and    */
  /* then decodes the compressed data. If the output buffer overflows, the   */
  /* return value is set to FALSE and the data area is returned unchanged.   */

  /* initialize output length to 0 */
  *pulOutputLength = 0;

  /* then allocate needed buffers */
//  pucExpandedData = calloc (usMaxOutputLength + 2, 1);
//  if ( !pucExpandedData )
//  {
//    fRc = FALSE;
//  } /* endif */
  fRc = UtlAlloc ((PVOID *)&pucExpandedData, 0L, ulMaxOutputLength + 2, NOMSG);
  pucOutputData = pucExpandedData;

  if ( fRc )
  {
    /* allocate memory and initialize each byte to 0 */
//    pucDictWindow = calloc (LZSS_WINDOW_SIZE, sizeof (UCHAR));
//    if ( !pucDictWindow )
//    {
//      fRc = FALSE;
//    } /* endif */
    fRc = UtlAlloc ((PVOID *)&pucDictWindow, 0L,
                    (LONG) (LZSS_WINDOW_SIZE * sizeof (UCHAR)), NOMSG);
  } /* endif */

  if ( fRc )
  {
    /* allocate input bit memory structure */
    psInputBitMem = psStartInputBitMem (pucDataArea);
    if ( !psInputBitMem )
    {
      fRc = FALSE;
    } /* endif */
  } /* endif */

  if ( fRc )
  {
    /* now expand the data */
    usCurrentWindowPos = 1;

    do
    {
      /* read preceeding bit */
      INPUT_BIT(psInputBitMem, usBit);

      if ( usBit == 1 )
      {
        /* this is a raw byte in the input bit stream */
        ucByte = (UCHAR) ulInputBits (psInputBitMem, 8);
        if ( *pulOutputLength < ulMaxOutputLength )
        {
          *(pucOutputData++) = ucByte;
          (*pulOutputLength)++;
        } /* endif */
        pucDictWindow [usCurrentWindowPos] = ucByte;
        usCurrentWindowPos = LZSS_MOD_WINDOW(usCurrentWindowPos + 1);
      }
      else
      {
        /* this is a previous match with position and length */
        usMatchPosition = (USHORT) ulInputBits (psInputBitMem, LZSS_INDEX_BITS);
        if ( usMatchPosition != LZSS_END_OF_STREAM )
        {
          usMatchLength = (USHORT) ulInputBits (psInputBitMem,
                                                LZSS_LENGTH_BITS);
          /* add the value that was subtracted before writing to bit stream */
          usMatchLength += LZSS_BREAK_EVEN + 1;

          for ( usIndex = 0; usIndex < usMatchLength; usIndex++ )
          {
            ucByte = pucDictWindow [LZSS_MOD_WINDOW(usMatchPosition + usIndex)];
            if ( *pulOutputLength < ulMaxOutputLength )
            {
              *(pucOutputData++) = ucByte;
              (*pulOutputLength)++;
            } /* endif */
            pucDictWindow [usCurrentWindowPos] = ucByte;
            usCurrentWindowPos = LZSS_MOD_WINDOW(usCurrentWindowPos + 1);
          } /* endfor */
        } /* endif */
      } /* endif */
    } while ( (usMatchPosition != LZSS_END_OF_STREAM) &&
              (*pulOutputLength <= ulMaxOutputLength) ); /* enddo */

    /* check if expansion was canceled because of output buffer overflow */
    if ( (*pulOutputLength >= ulMaxOutputLength) &&
         (usMatchPosition != LZSS_END_OF_STREAM) )
    {
      fRc = FALSE;
    } /* endif */
  } /* endif */

  if ( fRc )
  {
    /* if expansion was ok, copy the data to the data area */
    memcpy (pucDataArea, pucExpandedData, *pulOutputLength);
  }
  else
  {
    /* expansion was unsuccessful, set output length to 0 */
    *pulOutputLength = 0;
  } /* endif */

  /* free all allocated memory */
  if ( pucExpandedData )
  {
//    free (pucExpandedData);
    UtlAlloc ((PVOID *)&pucExpandedData, 0L, 0L, NOMSG);
  } /* endif */

  if ( pucDictWindow )
  {
//    free (pucDictWindow);
    UtlAlloc ((PVOID *)&pucDictWindow, 0L, 0L, NOMSG);
  } /* endif */

  if ( psInputBitMem )
  {
//    free (psInputBitMem);
    UtlAlloc ((PVOID *)&psInputBitMem, 0L, 0L, NOMSG);
  } /* endif */

  return (fRc);
} /* end of function fUtlLZSSExpand */



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     fUtlCompress                                             |
//+----------------------------------------------------------------------------+
//|Function call:     fUtlCompress (PUCHAR, USHORT, USHORT, PUSHORT)           |
//+----------------------------------------------------------------------------+
//|Description:       This function compresses the supplied data area          |
//|                   (usInputLength bytes starting at address pucDataArea).   |
//|                   The compressed data overwrites the supplied data!        |
//|                   After the data has been compressed, the size of the      |
//|                   compressed data is returned in pusOutputLength. The      |
//|                   compressed data area contains one leading info byte that |
//|                   is used to determine the algorithm used for compression  |
//|                   and immediately following the raw compressed data.       |
//|                   The function returns TRUE if compression was successful  |
//|                   and FALSE if it wasn't. However even if compression was  |
//|                   unsuccessful (i.e. algorithm wasn't effective, memory    |
//|                   allocation error, etc.), the output data area contains   |
//|                   the leading byte indicating the compression algorithm.   |
//|                   For that reason the data area must be allocated at least |
//|                   one byte larger than is really needed for the input data.|
//|                   The last parameter usCompressionType is used to select   |
//|                   the algorithm that will be used for compression. Normally|
//|                   it will be set to COMPRESS_AUTO, i.e. compression type is|
//|                   selected by this routine. For available compression types|
//|                   see the external header file.                            |
//+----------------------------------------------------------------------------+
//|Input parameter:   PUCHAR - pointer to input and output data area           |
//|                   USHORT - size of input data area in bytes                |
//|                   USHORT - compression algorithm to be used                |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT - pointer to size of output data area            |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE - compression was successful                        |
//|                   FALSE - compression wasn't successful (possible reasons: |
//|                           algorithm ineffective, errors allocating memory) |
//+----------------------------------------------------------------------------+
//|Prerequesits:      Input data area must be allocated at least one byte      |
//|                   larger than would be needed by uncompressed data since   |
//|                   returned data may increase by one byte in case the       |
//|                   compression wasn't successful.                           |
//+----------------------------------------------------------------------------+
//|Samples:           fRc = fUtlCompress (pucData, usDataLen, COMPRESS_AUTO,   |
//|                                       &usCompressedSize);                  |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

BOOL fUtlCompress (PUCHAR pucDataArea, ULONG ulInputLength,
                   USHORT usCompressionType, PULONG pulOutputLength)
{
  BOOL         fRc;

  if ( usCompressionType == COMPRESS_AUTO )
  {
    /* select the compression type */
    if ( ulInputLength <= MAX_LZSS_SIZE )
    {
      /* LZSS is good for this size */
      usCompressionType = COMPRESS_LZSS;
    }
    else
    {
      /* LZSS gets to slow for this size, use Huffman */
      usCompressionType = COMPRESS_HUFFMAN;
    } /* endif */
  } /* endif */

  /* move input data area one byte up to get room for leading */
  /* compression type indicator                               */
  memmove (pucDataArea + 1, pucDataArea, ulInputLength);

  /* store compression type indicator */
  *pucDataArea = (UCHAR) usCompressionType;

  /* now compress data */
  switch ( usCompressionType )
  {
    case COMPRESS_LZSS:
      fRc = fUtlLZSSCompress (pucDataArea + 1, ulInputLength, pulOutputLength);
      break;
    case COMPRESS_HUFFMAN:
      fRc = fUtlHuffmanCompress (pucDataArea + 1, ulInputLength,
                                 pulOutputLength);
      break;
    default :
      fRc = FALSE;
      *pulOutputLength = ulInputLength;
  } /* endswitch */

  /* if compression wasn't successful, compression type indicator must be */
  /* set to COMPRESS_NONE                                                 */
  if ( !fRc )
  {
    *pucDataArea = COMPRESS_NONE;
  } /* endif */

  /* add 1 to output length (byte used by compression type indicator) */
  (*pulOutputLength)++;

  return (fRc);
} /* end of function fUtlCompress */



//+----------------------------------------------------------------------------+
//|External function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     fUtlExpand                                               |
//+----------------------------------------------------------------------------+
//|Function call:     fUtlExpand (PUCHAR, USHORT, USHORT, PUSHORT)             |
//+----------------------------------------------------------------------------+
//|Description:       This function expands the data area that was compressed  |
//|                   by fUtlCompress. It reads the leading byte and then      |
//|                   selects the according expansion routine to expand the    |
//|                   data. The expanded data is returned in the same memory   |
//|                   area (pointed to by pucDataArea) whose length must be    |
//|                   passed in usMaxOutputLength (this output length should   |
//|                   be at least the size of the previously uncompressed data,|
//|                   so calling function is responsible to remember this size.|
//|                   The function returns the real size of uncompressed data  |
//|                   in pusOutputLength. The return code is used to determine |
//|                   whether expansion was successful or not.                 |
//+----------------------------------------------------------------------------+
//|Input parameter:   PUCHAR - pointer to input and output data area           |
//|                   USHORT - size of compressed data area                    |
//|                   USHORT - maximum size of output data area                |
//+----------------------------------------------------------------------------+
//|Output parameter:  PUSHORT - pointer to size of output data area            |
//+----------------------------------------------------------------------------+
//|Returncode type:   BOOL                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       TRUE - expansion was successful                          |
//|                   FALSE - expansion wasn't successful (possible reasons:   |
//|                           output size to small, errors allocating memory)  |
//+----------------------------------------------------------------------------+
//|Prerequesits:      The passed compressed data must be compressed with the   |
//|                   function fUtlCompress, otherwise the underlying functions|
//|                   may cause program errors trying to decode data which is  |
//|                   not correct.                                             |
//+----------------------------------------------------------------------------+
//|Samples:           fRc = fUtlExpand (pucData, usMaxSize, &usRealSize);      |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//|                   _                                                        |
//+----------------------------------------------------------------------------+

BOOL fUtlExpand (PUCHAR pucDataArea, ULONG ulInputLength,
                 ULONG ulMaxOutputLength, PULONG pulOutputLength)
{
  BOOL         fRc;
  USHORT       usCompressionType;

  /* read compression type */
  usCompressionType = (USHORT) (*pucDataArea);

  /* move remaining data one byte down (overwrite compression type) */
  memmove (pucDataArea, pucDataArea + 1, ulInputLength - 1);

  /* expand data */
  switch ( usCompressionType )
  {
    case COMPRESS_NONE:
      fRc = TRUE;
      *pulOutputLength = ulInputLength - 1;
      break;
    case COMPRESS_LZSS:
      fRc = fUtlLZSSExpand (pucDataArea, ulMaxOutputLength, pulOutputLength);
      break;
    case COMPRESS_HUFFMAN:
      fRc = fUtlHuffmanExpand (pucDataArea, ulMaxOutputLength, pulOutputLength);
      break;
    default :
      fRc = FALSE;
      *pulOutputLength = 0;
  } /* endswitch */

  return (fRc);
} /* end of function fUtlExpand */




#ifdef DEBUG

void main( argc, argv )
  int argc;
  char *argv[];
{
  FILE * input;
//  FILE * output;
  PUCHAR pucData;
  PUCHAR pucInputData;
  PUCHAR pucOutputData;
  USHORT usInputLength;
  USHORT usCompOutputLength;
  USHORT usExpOutputLength;
  BOOL   fRc;
  SEL            selGlobSeg, selLocalSeg;  // selectors returned by DosGetInfoSeg
  GINFOSEG FAR * pInfoSeg;
  ULONG          ulLastTime;
  ULONG          ulCompressTime;
  ULONG          ulExpandTime;

  DosGetInfoSeg (&selGlobSeg, &selLocalSeg);
  pInfoSeg = MAKEPGINFOSEG(selGlobSeg);

  pucData = malloc (65500);
  pucInputData = malloc (65500);
  pucOutputData = malloc (65500);
  if ( (pucData == NULL) || (pucInputData == NULL) || (pucOutputData == NULL) )
  {
    printf ("Error allocating memory.\n");
    exit (1);
  } /* endif */

  input = fopen( argv[ 1 ], "rb" );

  if ( input == NULL )
  {
    printf ("Error opening input file.\n");
    exit (1);
  } /* endif */

  usInputLength = fread (pucData, 1, 65500, input);
  fclose( input );

  printf ("Bytes read from input file: %u\nStarting compression.\n",
          usInputLength);

  printf ("\n\nHuffman - algorithm !\n");
  memcpy (pucInputData, pucData, usInputLength);
  ulLastTime = pInfoSeg->msecs;
  fRc = fUtlHuffmanCompress (pucInputData, usInputLength, &usCompOutputLength);
  ulCompressTime = pInfoSeg->msecs - ulLastTime;
  printf ("Output size : %u\n", usCompOutputLength);
  printf ("Compression time : %5.5lu ms\n", ulCompressTime);
  printf ("Compression result : %s\n", fRc ? "TRUE" : "FALSE");

//  output = fopen( argv[ 2 ], "wb" );
//
//  if ( output == NULL )
//  {
//    printf ("Error opening output file.\n");
//    exit (1);
//  } /* endif */
//
//  fwrite (pucInputData, 1, usCompOutputLength, output);
//  fclose (output);

  /* now prepare for expansion */
  if ( fRc )
  {
    memcpy (pucOutputData, pucInputData, usCompOutputLength);
    ulLastTime = pInfoSeg->msecs;
    fRc = fUtlHuffmanExpand (pucOutputData, usInputLength, &usExpOutputLength);
    ulExpandTime = pInfoSeg->msecs - ulLastTime;

    printf ("\nOutput size : %u\n", usExpOutputLength);
    printf ("Expansion time : %5.5lu ms\n", ulExpandTime);
    printf ("Expansion result : %s\n", fRc ? "TRUE" : "FALSE");
    printf ("Equality of input/output : %d\n",
            memcmp (pucData, pucOutputData, usExpOutputLength));
  } /* endif */

  printf ("\n\nLZSS - algorithm !\n");
  memcpy (pucInputData, pucData, usInputLength);
  ulLastTime = pInfoSeg->msecs;
  fRc = fUtlLZSSCompress (pucInputData, usInputLength, &usCompOutputLength);
  ulCompressTime = pInfoSeg->msecs - ulLastTime;
  printf ("Output size : %u\n", usCompOutputLength);
  printf ("Compression time : %5.5lu ms\n", ulCompressTime);
  printf ("Compression result : %s\n", fRc ? "TRUE" : "FALSE");

//  output = fopen( argv[ 2 ], "wb" );
//
//  if ( output == NULL )
//  {
//    printf ("Error opening output file.\n");
//    exit (1);
//  } /* endif */
//
//  fwrite (pucInputData, 1, usCompOutputLength, output);
//  fclose (output);

  /* now prepare for expansion */
  if ( fRc )
  {
    memcpy (pucOutputData, pucInputData, usCompOutputLength);
    ulLastTime = pInfoSeg->msecs;
    fRc = fUtlLZSSExpand (pucOutputData, usInputLength, &usExpOutputLength);
    ulExpandTime = pInfoSeg->msecs - ulLastTime;

    printf ("\nOutput size : %u\n", usExpOutputLength);
    printf ("Expansion time : %5.5lu ms\n", ulExpandTime);
    printf ("Expansion result : %s\n", fRc ? "TRUE" : "FALSE");
    printf ("Equality of input/output : %d\n",
            memcmp (pucData, pucOutputData, usExpOutputLength));
  } /* endif */
} /* end of main */

#endif
