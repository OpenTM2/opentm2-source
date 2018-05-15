//+----------------------------------------------------------------------------+
//|EQFPARSE.C                                                                    |
//+----------------------------------------------------------------------------+
//|Copyright Notice:                                                           |
//|                                                                            |
//|          Copyright (C) 1990-2012, International Business Machines          |
//|          Corporation and others. All rights reserved                       |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Author:                                                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Entry Points:                                                               |
//|                                                                            |
//|                                                                           |
//|+-- status ("H"=Header,"D"=Design,"C"=Code,"T"=Test, " "=complete,          |
//|            "Q"=Quick-and-dirty )                                           |
//+----------------------------------------------------------------------------+
//|Externals:                                                                  |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Internals:                                                                  |
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
// $Revision: 1.2 $ ----------- 29 Mar 2010
// GQ: - fixed P400522: Cannot create memory for IBMIDDOC package which was
//       caused by a single byte character at the end of the ITF-16 encoded
//       source file
// 
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
// $Revision: 1.3 $ ----------- 30 Nov 2006
// GQ: - fix for S6011018: use TAFreeDoc to free TB documents
// 
// 
// $Revision: 1.2 $ ----------- 14 Aug 2006
// GQ: - added free of segment metadata
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
// $Revision: 1.2 $ ----------- 16 Dec 2004
// RJ: P020990: add new func Parse_IsDBCSChar, Parse_Unicode2ASCIIBufEx
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
// $Revision: 1.3 $ ----------- 29 Oct 2003
// --RJ: delete codeinspection info
//
//
// $Revision: 1.2 $ ----------- 27 Oct 2003
// --RJ: P018448: add new W-functions to parse UTF16 files with parser utility functions
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
// $Revision: 1.3 $ ----------- 24 Mar 2003
// --RJ: add function AddCharsToFileL
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
// $Revision: 1.4 $ ----------- 5 Dec 2002
// --RJ: P016332: ParseNextChar: init bNextChar= EOS to assure EOS is returned of End of file
//            usRC = EOF_REACHED but no EOF in file
//
//
// $Revision: 1.3 $ ----------- 25 Oct 2002
// --RJ: P016123: ParseGetCP/ParseFillCP: return usRC = EQFRS_INVALID_PARM if it is so
//
//
// $Revision: 1.2 $ ----------- 29 Jul 2002
// --RJ: R07197: add function ParseFillCP, ParseGetCP
//
//
// $Revision: 1.1 $ ----------- 26 Jul 2002
//  -- New Release TM6.0!!
//
//
// $Revision: 1.3 $ ----------- 31 Oct 2001
// GQ: - Added code to free context areas in TBDOCUMENT, free pEQFBWOrkSegmentW
//
//
// $Revision: 1.2 $ ----------- 3 Sep 2001
// -- RJ: Unicode enabling
//
//
// $Revision: 1.1 $ ----------- 16 Aug 2001
//  -- New Release TM2.7.2!!
//
// $Revision: 1.5 $ ----------- 25 Sep 2000
// -- add support for more than 64k segments
//
//
// $Revision: 1.4 $ ----------- 4 May 2000
// - added function AddStringToFile
//
//
// $Revision: 1.3 $ ----------- 12 Apr 2000
// - added function AddStringToFile
//
//
//
// $Revision: 1.2 $ ----------- 6 Dec 1999
//  -- Initial Revision!!
//
/*
 * $Header:   K:\DATA\EQFPARSE.CV_   1.6   15 Feb 1999 08:06:14   BUILD  $
 *
 * $Log:   K:\DATA\EQFPARSE.CV_  $
 *
 *    Rev 1.6   15 Feb 1999 08:06:14   BUILD
 * - fixed PTM KBT0423: Trap when analyze with IBMOSM00
 *   the trap was caused by a logic error in the segment split function
 *
 *    Rev 1.5   08 Feb 1999 09:14:52   BUILD
 * -- free pSegmentBuffer in document structure
 *
 *    Rev 1.4   12 Oct 1998 10:22:52   BUILD
 * -- R004350_HLUNMATCHTAG: add support for pSeg->pusHLType
 *
 *    Rev 1.3   22 Jun 1998 16:23:12   BUILD
 * - added function ParseNextLine for line based parsers
 *
 *    Rev 1.2   09 Feb 1998 17:33:34   BUILD
 * - Win32: avoid compiler warnings
 *
 *    Rev 1.1   25 Mar 1997 16:57:28   BUILD
 * - added new function ParseGetDocName
 *
 *    Rev 1.0   09 Jan 1996 09:12:40   BUILD
 * Initial revision.
*/
//+----------------------------------------------------------------------------+

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_FOLDER           // folder and document related functions
#define INCL_EQF_ANALYSIS
#define INCL_EQF_MORPH
#include <eqf.h>                  // General Translation Manager include file
#include "eqfparse.h"             // headers used

#define GETNUMBER( pszTarget, ulValue ) \
{                                   \
   ulValue = 0L;                    \
   if ( *pszTarget )                \
   {                                \
     while ( *pszTarget == ' ' )    \
     {                              \
       pszTarget++;                 \
     } /* endwhile */               \
     while ( isdigit(*pszTarget) )  \
     {                              \
        ulValue = (ulValue * 10) + (*pszTarget++ - '0'); \
     } /* endwhile */               \
   } /* endif */                    \
}

static BOOL Parse_IsDBCSChar( CHAR_W c, ULONG ulCP);
static ULONG Parse_Unicode2ASCIIBufEx( PSZ_W pszUni, PSZ pszASCII, ULONG usLen, LONG lBufLen,
                              ULONG ulCP,BOOL fMsg, PLONG plRc, DWORD dwFlags );
//+----------------------------------------------------------------------------+
//| StartSegment     - start a new segment                                     |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Write start tag of segment to output file and initialize segment buffer.|
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - write starttag for segment to output file and increment          |
//|           segment number                                                   |
//|         - reset output  buffer pointer and output buffer used count        |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PPARSEDATA pParsData            - ptr to parser global data structure     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - Output file must be open.                                               |
//|  - Mode field in global data structure must be set.                        |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  - segment number is incremented                                           |
//|  - segment type field in global data structure is set                      |
//|  - output buffer use count and output buffer pointer are reset             |
//+----------------------------------------------------------------------------+
USHORT StartSegment
(
   PPARSEDATA pParsData                // ptr to parser global data structure
)
{
   PSZ   pszStartTag;                  // ptr to string of semgent start tag
   USHORT usRC;                        // internal return code
   USHORT usBytesWritten;              // number of bytes written to output file

   switch ( pParsData->Mode )
   {
      case STRING_MODE:
         pszStartTag = pParsData->pQFFTag;
         pParsData->usSegType = QFF_SEGMENT;
         break;
      default:
         pszStartTag = pParsData->pQFNTag;
         pParsData->usSegType = QFN_SEGMENT;
         break;
   } /* endswitch */

   pParsData->ulSegNum++;

   sprintf( pParsData->abOutBuf, pszStartTag, pParsData->ulSegNum );

   usRC = UtlWrite( pParsData->hOutFile,
                    pParsData->abOutBuf,
                    (USHORT)strlen(pParsData->abOutBuf),
                    &usBytesWritten,
                    TRUE );

   pParsData->pOutBuf = pParsData->abOutBuf;
   pParsData->usOutBufUsed = 0;

   return( usRC );
} /* end of StartSegent */

//+----------------------------------------------------------------------------+
//| AddToSegment     - add data to current segment                             |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Add one byte to the current segment and, if the segment buffer is full, |
//|    write the segment to the output file and start a new one.               |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - if segment buffer is full end the current segment and start      |
//|           a new one.                                                       |
//|         - append given byte to current segment                             |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PPARSEDATA pParsData            - ptr to parser global data structure     |
//|  BYTE       bAddByte             - byte being added to segment             |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - Output file must be open.                                               |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
USHORT AddToSegment
(
   PPARSEDATA pParsData,               // ptr to parser global data structure
   BYTE       bAddByte                 // byte being added to segment
)
{
   USHORT usRC = 0;                    // internal return code
   USHORT usRest;                      // rest of segment
   PCHAR  pbTemp;                      // ptr for segment data processing

   //
   // force a new segment if segment buffer is full
   //
   if ( pParsData->usOutBufUsed == MAX_SEG_SIZE )
   {
      // look for the right place (= end of a line) to split data in
      // segment buffer
      pbTemp = pParsData->abOutBuf + (pParsData->usOutBufUsed - 1);
      while ( (pbTemp > pParsData->abOutBuf) &&
               (*pbTemp != LF) && (*pbTemp != CR) )
      {
         pbTemp--;
      } /* endwhile */

      // if no line end character is in segment write segment as-is
      // otherwise split segment at line end character
      if ( pbTemp <= pParsData->abOutBuf )
      {
         // no line end characters in segment
         usRC = EndSegment( pParsData );
         if ( !usRC )
         {
            usRC = StartSegment( pParsData );
         } /* endif */
      }
      else
      {
         // split segment after line end character
         pbTemp++;

         usRest = (USHORT)(pParsData->usOutBufUsed - (pbTemp - pParsData->abOutBuf));

         if ( usRest )
         {
            memcpy( pParsData->abTempBuf, pbTemp, usRest );
            pParsData->usOutBufUsed = pParsData->usOutBufUsed - usRest;
         } /* endif */

         usRC = EndSegment( pParsData );
         if ( !usRC )
         {
            usRC = StartSegment( pParsData );
         } /* endif */

         if ( !usRC && usRest )
         {
            memcpy( pParsData->abOutBuf, pParsData->abTempBuf, usRest );
            pParsData->usOutBufUsed = usRest;
            pParsData->pOutBuf = pParsData->abOutBuf + usRest;
         } /* endif */

      } /* endif */
   } /* endif */

   //
   // append new byte to segment buffer
   //
   if ( !usRC )
   {
      *(pParsData->pOutBuf)++ = bAddByte;
      pParsData->usOutBufUsed++;
   } /* endif */

   return( usRC );
} /* end of AddToSegment */


//+----------------------------------------------------------------------------+
//| ParseNextChar    - retrieve the next input character                       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Returns the next character from the input file and checks end-of-file   |
//|    condition.                                                              |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - if input buffer is empty read next data from input file and      |
//|           check end-of-file condition                                      |
//|         - if input buffer contains data and data is not end-of-file        |
//|           character return next byte from buffer else set return code to   |
//|           EOF_REACHED.                                                     |
//+----------------------------------------------------------------------------+
//|                                                                            |
//| Arguments:                                                                 |
//|  PPARSEDATA pParsData,           - ptr to parser global data structure     |
//|  PUSHORT    pusRC                - return code (set in case of errors)     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - Input file must be open.                                                |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  - input buffer use count and bytes to read count are changed              |
//+----------------------------------------------------------------------------+
BYTE  ParseNextChar
(
   PPARSEDATA pParsData,               // ptr to parser global data structure
   PUSHORT    pusRC                    // return code (set in case of errors)
)
{
   BYTE   bNextByte = EOS;                   // byte being returned
   USHORT  usBytesToRead;               // number of bytes to read from file
   USHORT usRC = 0;                    // return code of called functions
   USHORT usComplete;                  // completion ratio

   usComplete = pParsData->usPercentComplete;
   *pusRC = 0;                         // init return code...

   /*******************************************************************/
   /* check if next character is to be taken from undo buffer or from */
   /* input buffer                                                    */
   /*******************************************************************/
   if ( pParsData->usUndoBufUsed )
   {
     /*****************************************************************/
     /* UNDO buffer is not empty: get character from this buffer      */
     /*****************************************************************/
     bNextByte = pParsData->abUndoBuf[ --pParsData->usUndoBufUsed ];
     if ( bNextByte == LF )
     {
       pParsData->usLinePos = 0;                   // reset position in line
     }
     else
     {
       pParsData->usLinePos++;                     // next position in line
     } /* endif */

   }
   else
   {
     if ( !pParsData->usBytesInBuffer )
     {
        if ( !pParsData->lBytesToRead )
        {
           // error: file has been processed completely
           *pusRC = EOF_REACHED;
        }
        else
        {
           usBytesToRead =  (USHORT)min( (LONG)INBUF_SIZE,
                                         pParsData->lBytesToRead );
           usRC = UtlRead( pParsData->hInFile, pParsData->abInBuf,
                           usBytesToRead, &pParsData->usBytesInBuffer, TRUE );
           if ( usRC )
           {
              *pusRC = usRC;
           }
           else
           {
              pParsData->pInBuf = pParsData->abInBuf;
              pParsData->lBytesToRead -= pParsData->usBytesInBuffer;
           } /* endif */
        } /* endif */
     } /* endif */

     if ( pParsData->usBytesInBuffer )
     {
        bNextByte = *(pParsData->pInBuf)++;
        pParsData->usBytesInBuffer--;
        pParsData->usLinePos++;             // next position in line
        switch ( bNextByte )
        {
          case  END_OF_FILE:
            *pusRC = EOF_REACHED;
             usComplete = 100;           // we are through ...
            break;
          case  LF:
            pParsData->usLinePos = 0;       // start at line
            usComplete = (USHORT) ((pParsData->lTotalBytes -
                                    pParsData->lBytesToRead -
                                    (LONG)pParsData->usBytesInBuffer) *
                                    100 /
                                    pParsData->lTotalBytes);
            break;
          default :
            break;
        } /* endswitch */
     }
     else
     {
       usComplete = 100;               // we are through ...
     } /* endif */
   } /* endif */


   if ( usComplete != pParsData->usPercentComplete )
   {
     if ( pParsData->hwndSlider )
     {
       WinSendMsg( pParsData->hwndSlider, WM_EQF_UPDATESLIDER,
                   MP1FROMSHORT(usComplete), NULL );
     } /* endif */
     UtlDispatch();
     if ( *(pParsData->pfKill) )
     {
       // error: we have to leave asap -- simulate read error. ...
       *pusRC = (USHORT) ERR_READFILE;
     } /* endif */
   } /* endif */

   return( bNextByte );
} /* end of ParseNextChar */


//+----------------------------------------------------------------------------+
//| UndoNextChar     - put getted character back in input queue                |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Resets the pointers to indicate that last next character function is    |
//|    undone                                                                  |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - change input buffer pointer and number of bytes in input buffer  |
//+----------------------------------------------------------------------------+
//|                                                                            |
//| Arguments:                                                                 |
//|  PPARSEDATA pParsData            - ptr to parser global data structure     |
//|  BYTE       bChar                - character to be passed to undo buffer.. |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  VOID                                                                      |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - character must be get with NextChar                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  - input buffer use count and bytes to read count are changed              |
//+----------------------------------------------------------------------------+
VOID  UndoNextChar
(
   PPARSEDATA pParsData,               // ptr to parser global data structure
   BYTE       bChar                    // character to be undo ...
)
{

   if ( pParsData->usUndoBufUsed < MAX_SEG_SIZE )
   {
     pParsData->abUndoBuf[ pParsData->usUndoBufUsed++ ] = bChar;
     pParsData->usLinePos--;           // prev position in line
   } /* endif */
// pParsData->usBytesInBuffer++;       // put byte back into buffer
// pParsData->pInBuf --;

} /* end of UndoNextChar */


//+----------------------------------------------------------------------------+
//| EndSegment     - terminate current segment                                 |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Write data of segment and segment end tag to output file.               |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - write data in output buffer to output file                       |
//|         - reset output  buffer pointer and output buffer used count        |
//|         - write end tag for this segment type to output file               |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PPARSEDATA pParsData            - ptr to parser global data structure     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - Output file must be open.                                               |
//|  - Segment type field in global data structure must be set.                |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  - output buffer use count and output buffer pointer are reset             |
//+----------------------------------------------------------------------------+
USHORT EndSegment
(
   PPARSEDATA pParsData                // ptr to parser global data structure
)
{
   PSZ   pszEndTag = NULL;                    // ptr to string of segment end tag
   USHORT usRC = 0;                    // internal return code
   USHORT usBytesWritten;              // number of bytes written to output file

   // write data of segment to output file
   if ( pParsData->usOutBufUsed )
   {
      usRC = UtlWrite( pParsData->hOutFile,
                       pParsData->abOutBuf,
                       pParsData->usOutBufUsed,
                       &usBytesWritten,
                       TRUE );
   } /* endif */

   // reset outpuffer used count and output buffer pointer
   if ( !usRC )
   {
      pParsData->pOutBuf = pParsData->abOutBuf;
      pParsData->usOutBufUsed = 0;
   } /* endif */

   // write segment end tag to output file
   if ( !usRC )
   {
      switch ( pParsData->usSegType )
      {
         case QFF_SEGMENT:
            pszEndTag = pParsData->pEQFFTag;
            break;
         case QFN_SEGMENT:
            pszEndTag = pParsData->pEQFNTag;
            break;
      } /* endswitch */

      usRC = UtlWrite( pParsData->hOutFile,
                       pszEndTag,
                       (USHORT)strlen(pszEndTag),
                       &usBytesWritten,
                       TRUE );
   } /* endif */

   return( usRC );
} /* end of EndSegment */

//+----------------------------------------------------------------------------+
//| AddToFile        - add data to current segment                             |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Add one byte to the current segment                                     |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - if segment buffer is full end the current segment and start      |
//|           a new one.                                                       |
//|         - append given byte to current segment                             |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PPARSEDATA pParsData            - ptr to parser global data structure     |
//|  BYTE       bAddByte             - byte being added to segment             |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - Output file must be open.                                               |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
USHORT AddToFile
(
   PPARSEDATA pParsData,               // ptr to parser global data structure
   BYTE       bAddByte                 // byte being added to segment
)
{
   USHORT usRC = 0;                    // internal return code

   /*******************************************************************/
   /* add position in output line and reset it if necessary           */
   /*******************************************************************/
   pParsData->usOutPos++;              // increment position in out file
   if ( bAddByte == LF )
   {
     pParsData->usOutPos = 0;
   } /* endif */
   /*******************************************************************/
   /* if buffer filled up write it...                                 */
   /*******************************************************************/
   if ( pParsData->usOutBufUsed >= MAX_OUTBUF_SIZE )
   {
     // just for testing ...
     pParsData->usOutBufUsed = pParsData->usOutBufUsed;
   }

   if ( pParsData->usOutBufUsed == MAX_OUTBUF_SIZE )
   {
     usRC = WriteBuffer( pParsData );
   } /* endif */
   //
   // append new byte to segment buffer
   //
   if ( !usRC )
   {
      *(pParsData->pOutBuf)++ = bAddByte;
      pParsData->usOutBufUsed++;
   } /* endif */

   return( usRC );
} /* end of AddToFile */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     WriteBuffer                                              |
//+----------------------------------------------------------------------------+
//|Function call:     WriteBuffer ( PPARSEDATA );                              |
//+----------------------------------------------------------------------------+
//|Description:       this function will write out a buffer and reset          |
//|                   the appropriate pointers                                 |
//+----------------------------------------------------------------------------+
//|Parameters:        PPARSEDATA       pointer to buffer structure             |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0    okay                                                |
//|                   usRC DOS error                                           |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
USHORT WriteBuffer
(
  PPARSEDATA pParsData
)
{
   USHORT usRC;                        // internal return code
   USHORT usBytesWritten;

   // write data of segment to output file
   usRC = UtlWrite( pParsData->hOutFile,
                    pParsData->abOutBuf,
                    pParsData->usOutBufUsed,
                    &usBytesWritten,
                    TRUE );

   // reset outpuffer used count and output buffer pointer
   if ( !usRC )
   {
      pParsData->pOutBuf = pParsData->abOutBuf;
      pParsData->usOutBufUsed = 0;
   } /* endif */
   return( usRC );
} /* end of function WriteBuffer */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ParseInit                                                |
//+----------------------------------------------------------------------------+
//|Function call:     ParseInit( pParsData, hwndSlider, pfKill );              |
//+----------------------------------------------------------------------------+
//|Description:       do the initial settings                                  |
//+----------------------------------------------------------------------------+
//|Parameters:        PPARSEDATA pParsData,     parse data control block       |
//|                   HWND       hwndSlider     slider handle                  |
//|                   PBOOL      pfKill         kill flag                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     set the initial settings                                 |
//+----------------------------------------------------------------------------+
VOID
ParseInit
(
  PPARSEDATA pParsData,                // parse data control block
  HWND       hwndSlider,               // slider handle
  PEQF_BOOL  pfKill                    // pointer to kill flag
)
{
  memset( pParsData, 0, sizeof(PARSEDATA) );
  pParsData->Mode = OUTSIDE_STRING_MODE;
  pParsData->hwndSlider = hwndSlider;
  pParsData->pfKill     = pfKill;

  // Setup tag names
  // Note: this should be done dynamically based on the QFTAG table!
  //       Look at EQFBFileWrite for an example
  pParsData->pQFNTag   = ":QFN N=%d.";
  pParsData->pEQFNTag  = ":EQFN.";
  pParsData->pQFFTag   = ":QFF N=%d.";
  pParsData->pEQFFTag  = ":EQFF.";

  /********************************************************************/
  /* init the output buffer                                           */
  /********************************************************************/
  pParsData->pOutBuf = pParsData->abOutBuf;
  pParsData->usOutBufUsed = 0;
  return;
} /* end of function ParseInit */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ParseClose                                               |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = ParseClose( pParsData );                          |
//+----------------------------------------------------------------------------+
//|Description:       do a close of the Parseing block                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PPARSEDATA pParsData        control data block           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       DOS RCs                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     call the WriteBuffer function                            |
//+----------------------------------------------------------------------------+
USHORT
ParseClose
(
  PPARSEDATA pParsData
)
{
  return( WriteBuffer( pParsData ) );
} /* end of function ParseClose */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ParseFreeDoc                                             |
//+----------------------------------------------------------------------------+
//|Function call:     ParseFreeDoc(&pTBDoc );                                  |
//+----------------------------------------------------------------------------+
//|Description:       Free the allocated space for the document structure      |
//+----------------------------------------------------------------------------+
//|Parameters:        PVOID        * pTBDoc  pointer to docum. instance data   |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     free buffers for input and tokens                        |
//|                   loop thru seg table and free all these buffers           |
//|                   free segtable and undo segment                           |
//|                   return                                                   |
//+----------------------------------------------------------------------------+
VOID ParseFreeDoc
(
  PVOID * ppDoc
)
// function is obsolete, use SefFileFreeDoc instead
{
  SegFileFreeDoc( ppDoc );
}

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ConvertAscBin                                            |
//+----------------------------------------------------------------------------+
//|Function call:     bHex = ConvertAscBin( bByte1, bByte2 );                  |
//+----------------------------------------------------------------------------+
//|Description:       this function will convert two hex digits into a meaning |
//|                   ful byte.                                                |
//+----------------------------------------------------------------------------+
//|Parameters:        BYTE   bByte1      first byte                            |
//|                   BYTE   bByte2      second byte                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   BYTE                                                     |
//+----------------------------------------------------------------------------+
//|Returncodes:       value of the hex digits                                  |
//+----------------------------------------------------------------------------+
//|Prerequesits:      bByte1 and bByte2 are valid hex digits...                |
//+----------------------------------------------------------------------------+
//|Function flow:     use first character byte and convert it into a value     |
//|                   shift the value 4 bits and add the converted second value|
//+----------------------------------------------------------------------------+
BYTE
ConvertAscBin
(
  BYTE  bByte1,                        // first byte
  BYTE  bByte2                         // second byte
)
{
  BYTE  bValue;                        // return value

  /********************************************************************/
  /* get value of first byte and convert it into number               */
  /********************************************************************/
  if ( (bByte1 >= '0') && (bByte1 <= '9') )
  {
    bValue = bByte1 -'0';
  }
  else
  {
    bValue = bByte1 -'a' + 10;
  } /* endif */

  bValue <<= 4;                        // shift value for bytes


  /********************************************************************/
  /* get value of second byte and convert it into number              */
  /********************************************************************/
  if ( (bByte2 >= '0') && (bByte2 <= '9') )
  {
    bValue += bByte2 -'0';
  }
  else
  {
    bValue += bByte2 -'a' + 10;
  } /* endif */

  return bValue;
} /* end of function ConvertAscBin */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ConvertBinAsc                                            |
//+----------------------------------------------------------------------------+
//|Function call:     ConvertBinAsc( bByte, chTempBuf );                       |
//+----------------------------------------------------------------------------+
//|Description:       convert byte into two byte ascii value                   |
//+----------------------------------------------------------------------------+
//|Parameters:        BYTE   bByte   byte to be converted                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   PSZ                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       pointer to filled buffer                                 |
//+----------------------------------------------------------------------------+
//|Function flow:     get first half byte and convert it into a ascii hex numb.|
//|                   get 2nd half byte and convert it into a ascii hex numb   |
//|                   return pointer to filled buffer                          |
//+----------------------------------------------------------------------------+
PSZ
ConvertBinAsc
(
  BYTE bByte,                          // value to be converted
  PSZ  pAscii                          // pointer to ascii value
)
{
  BYTE bTemp;                          // temporary storage

  /********************************************************************/
  /* get first half ( shift rest out )                                */
  /********************************************************************/
  bTemp = bByte >> 4;

  if ( bTemp <= 9 )
  {
    bTemp += '0';
  }
  else
  {
    bTemp += 'a' - 10;
  } /* endif */

  *pAscii = bTemp;

  /********************************************************************/
  /* get second half ..                                               */
  /********************************************************************/
  bTemp = ( bByte & 0x0F );

  if ( bTemp <= 9 )
  {
    bTemp += '0';
  }
  else
  {
    bTemp += 'a' - 10;
  } /* endif */

  *(pAscii+1) = bTemp;

  return pAscii;
} /* end of function ConvertBinAsc */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AddCharsToFile                                           |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = AddCharsToFile( pParseData, pString, usLength );  |
//+----------------------------------------------------------------------------+
//|Description:       copy the specified number of bytes to the file           |
//+----------------------------------------------------------------------------+
//|Parameters:        PPARSEDATA pParseData      pointer to control block      |
//|                   PSZ        pString,        string to be copied           |
//|                   USHORT     usLen           number of bytes               |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       DOS RCs                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     loop as long as characters are available and no RC       |
//|                     Add them to the file                                   |
//|                   endfor                                                   |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+
USHORT AddCharsToFile
(
  PPARSEDATA pParseData,               // pointer to control block
  PSZ        pString,                  // string to be copied
  USHORT     usLen                     // number of bytes
)
{
	ULONG   ulLen = usLen;
	return (AddCharsToFileL( pParseData, pString, ulLen ));
}


USHORT AddCharsToFileL
(
  PPARSEDATA pParseData,               // pointer to control block
  PSZ        pString,                  // string to be copied
  ULONG      ulLen                     // number of bytes
)
{
  USHORT usRC = 0;                     // success indicator
  ULONG  i;                            // index


  for ( i=0; (i < ulLen) && !usRC ;i++ )
  {
    usRC = AddToFile( pParseData, *pString++);
  } /* endfor */
  return usRC;
} /* end of function AddCharsToFile */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AddStringToFile                                          |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = AddStringToFile( pParseData, pString );           |
//+----------------------------------------------------------------------------+
//|Description:       copy the specified string to the file                    |
//+----------------------------------------------------------------------------+
//|Parameters:        PPARSEDATA pParseData      pointer to control block      |
//|                   PSZ        pString         string to be copied           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       DOS RCs                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     loop as long as characters are available and no RC       |
//|                     Add them to the file                                   |
//|                   endfor                                                   |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+

USHORT AddStringToFile
(
  PPARSEDATA pParseData,               // pointer to control block
  PSZ        pString                   // string to be copied
)
{
  USHORT usRC = 0;                     // success indicator

  while ( (*pString != EOS) && !usRC )
  {
    usRC = AddToFile( pParseData, *pString++ );
  } /* endwhile */
  return usRC;
} /* end of function AddStringToFile */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AddIgnoredOnes                                           |
//+----------------------------------------------------------------------------+
//|Function call:     _                                                        |
//+----------------------------------------------------------------------------+
//|Description:       get prev. ignored data from source file into output file |
//+----------------------------------------------------------------------------+
//|Parameters:        PPARSEDATA  pParsData,  points to parser data structure  |
//|                   PSZ         pPosString  position string                  |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       DOS RCs                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     extract the positions                                    |
//|                   position at start position in source file                |
//|                   read in data from source file and pass it into output    |
//|                     file..                                                 |
//+----------------------------------------------------------------------------+
USHORT
AddIgnoredOnes
(
  PPARSEDATA  pParsData,               // points to parser data structure
  PSZ         pPosString               // position string
)
{
  USHORT  usRC = 0;
  ULONG   ulStartPos = 0L;
  ULONG   ulEndPos = 0L;
  PSZ     pTemp;

  /********************************************************************/
  /* get start and end position                                       */
  /********************************************************************/
  pTemp = strchr( pPosString, ':' );
  if ( pTemp )
  {
    pTemp++;
    GETNUMBER( pTemp, ulStartPos );
    if ( *pTemp == ',' )
    {
      pTemp++;
      GETNUMBER( pTemp, ulEndPos );
    }
    else
    {
      usRC = AddCharsToFile( pParsData, pPosString, (USHORT)strlen(pPosString) );
    } /* endif */
  }
  else
  {
    usRC = AddCharsToFile( pParsData, pPosString, (USHORT)strlen(pPosString) );
  } /* endif */

  /********************************************************************/
  /* write it out (if no error yet)                                   */
  /* Error will be detected if ulEndPos still 0 or less ulStartPos    */
  /********************************************************************/
  if ( ulStartPos < ulEndPos )
  {
    PSZ pSrcData = NULL;
    usRC = (USHORT)!UtlAlloc( (PVOID *) &pSrcData, 0L, 2048L, ERROR_STORAGE);
    if ( !usRC )
    {
      LONG lBytesToRead = ulEndPos - ulStartPos + 1;
      USHORT usBytesToRead, usBytesRead;
      ULONG  ulOffset;
      /****************************************************************/
      /* position at the correct file offset                          */
      /****************************************************************/
      usRC = UtlChgFilePtr( pParsData->hSrcFile, ulStartPos, FILE_BEGIN,
                            &ulOffset, FALSE );

      /****************************************************************/
      /* read in data ....                                            */
      /****************************************************************/
      while ( lBytesToRead && !usRC )
      {
        usBytesToRead =  (USHORT)min( 2048L, lBytesToRead );
        usRC = UtlRead( pParsData->hSrcFile, pSrcData,
                        usBytesToRead, &usBytesRead, TRUE );
        if ( !usRC )
        {
          lBytesToRead -= usBytesRead;
          usRC = AddCharsToFile( pParsData, pSrcData, usBytesRead );
        } /* endif */
      } /* endwhile */
      /****************************************************************/
      /* free storage                                                 */
      /****************************************************************/
      UtlAlloc( (PVOID *) &pSrcData, 0L, 0L, NOMSG);
    } /* endif */
  } /* endif */

  return usRC;
} /* end of function AddIgnoredOnes */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ParseDeleteSegment                                       |
//+----------------------------------------------------------------------------+
//|Function call:     ParseFreeDoc(&pTBDoc );                                  |
//+----------------------------------------------------------------------------+
//|Description:       Free the allocated space for the document structure      |
//+----------------------------------------------------------------------------+
//|Parameters:        PVOID        * pTBDoc  pointer to docum. instance data   |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     free buffers for input and tokens                        |
//|                   loop thru seg table and free all these buffers           |
//|                   free segtable and undo segment                           |
//|                   return                                                   |
//+----------------------------------------------------------------------------+
VOID ParseDeleteSegment
(
  PVOID       pDocument,
  ULONG       ulSegNum
)
{
  PTBSEGMENTTABLE pSegTable;          // ptr for segment table deleting
  PTBSEGMENT      pSegment;           // ptr for segment deleting
  PTBDOCUMENT     pDoc = (PTBDOCUMENT) pDocument;

  /*******************************************************************/
  /* get the segment to be deleted and free any allocated areas      */
  /*******************************************************************/
  if ( ulSegNum < pDoc->ulMaxSeg && ulSegNum > 0 )
  {
    ULONG  ulSegTables;
    pSegTable = pDoc->pSegTables;
    ulSegTables =  pDoc->ulSegTables;
    while ( ulSegTables && ( ulSegNum >= pSegTable->ulSegments) )
    {
       ulSegNum -= pSegTable->ulSegments;
       ulSegTables--;
       pSegTable++;
    } /* endwhile */

    if ( ulSegTables && ( ulSegNum <  pSegTable->ulSegments) )
    {
       pSegment = pSegTable->pSegments + ulSegNum;
       if ( pSegment->pData )
       {
          UtlAlloc( (PVOID *) &pSegment->pData, 0L, 0L, NOMSG );
       } /* endif */
       if ( pSegment->pusBPET )
       {
          UtlAlloc( (PVOID *) &pSegment->pusBPET, 0L, 0L, NOMSG );
       } /* endif */
       if (pSegment->pusHLType )
       {
         UtlAlloc((PVOID *)&(pSegment->pusHLType),0L ,0L, NOMSG);
       } /* endif */

       /*************************************************************/
       /* readjust areas within this specific segment table ...     */
       /*************************************************************/
       ulSegNum++;
       while ( ulSegTables )
       {
         /*************************************************************/
         /* get segment data                                          */
         /*************************************************************/
         if (ulSegNum >= pSegTable->ulSegments)
         {
           if ( ulSegTables > 1 )
           {
             pSegTable++;
             ulSegTables --;
             ulSegNum = 0;
           }
           else
           {
             /*********************************************************/
             /* we reached the last table ..                          */
             /*********************************************************/
             ulSegTables --;
           } /* endif */
         } /* endif */
         if ( ulSegTables )
         {
           PTBSEGMENT pSegNew = pSegTable->pSegments + ulSegNum;
           memmove(pSegment, pSegNew, sizeof( TBSEGMENT ));
           (pSegment->ulSegNum)--;
           pSegment = pSegNew;
           ulSegNum++;
         }
         else
         {
           if ( pSegTable->ulSegments )
           {
             /*********************************************************/
             /* indicate segment deleted                              */
             /*********************************************************/
             pSegTable->ulSegments--;
             memset(pSegment, 0, sizeof( TBSEGMENT ));
           }
           else
           {
             /*********************************************************/
             /* free this specific segment table ....                 */
             /*********************************************************/
             USHORT usJ;
             pSegment = pSegTable->pSegments;
             for ( usJ = 0; usJ < pSegTable->ulSegments; usJ++ )
             {
                if ( pSegment->pData )
                {
                   UtlAlloc( (PVOID *) &pSegment->pData, 0L, 0L, NOMSG );
                } /* endif */
                if ( pSegment->pusBPET )
                {
                   UtlAlloc( (PVOID *) &pSegment->pusBPET, 0L, 0L, NOMSG );
                } /* endif */
                if (pSegment->pusHLType )
                {
                  UtlAlloc((PVOID *)&(pSegment->pusHLType),0L ,0L, NOMSG);
                } /* endif */
                pSegment++;
             } /* endfor */
             UtlAlloc( (PVOID *) &pSegTable->pSegments, 0L, 0L, NOMSG );
             pDoc->ulSegTables --;
           } /* endif */
         } /* endif */
       } /* endwhile */
       /*************************************************************/
       /* now adjust max number of segments in document             */
       /*************************************************************/
       pDoc->ulMaxSeg--;
    } /* endif */
  } /* endif */
} /* end of function ParseDeleteSegment */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ParseGetDocName                                          |
//+----------------------------------------------------------------------------+
//|Function call:     ParseGetDocName( pInFile, pszDocName );                  |
//+----------------------------------------------------------------------------+
//|Description:       Get the long name or use the short name of a document.   |
//+----------------------------------------------------------------------------+
//|Parameters:        PSZ pszInFile   fully qualified input or target file name|
//|                   PSZ pszDocName  ptr to buffer for document name          |
//|                                   Note: the buffer must have a size of at  |                                                                            |
//|                                         least MAX_LONGFILESPEC bztes       |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
VOID ParseGetDocName
(
  PSZ         pszInFile,               // fullz qualified input or target file name
  PSZ         pszDocName               // ptr to buffer for document name
)
{
  CHAR        szDocObjName[MAX_EQF_PATH]; // buffer for document object name

  // build document object
  strcpy( szDocObjName, pszInFile );     // get fully qualified document file name
  UtlSplitFnameFromPath( szDocObjName ); // remove document name
  UtlSplitFnameFromPath( szDocObjName ); // remove SOURCE or TARGET sub-directory
  strcat( szDocObjName, BACKSLASH_STR ); // append a backslash character
  strcat( szDocObjName, UtlGetFnameFromPath( pszInFile ) ); // append doc name

  // get document long name (if any)
  *pszDocName = EOS;
  DocQueryInfo2( szDocObjName,         // document object name
                 NULL,                 // no memory needed
                 NULL,                 // no markup needed
                 NULL,                 // no source language needed
                 NULL,                 // no target language needed
                 pszDocName,           // document long name
                 NULL,                 // no alias name needed
                 NULL,                 // no editor needed
                 FALSE );              // do not handle errors in function

  // use document short name if no long name is available
  if ( *pszDocName == EOS )
  {
    strcpy( pszDocName, UtlGetFnameFromPath( pszInFile ) );
  }
  else
  {
    OEMTOANSI( pszDocName );
  } /* endif */

} /* end of function ParseGetDocName */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ParseNextLine                                            |
//+----------------------------------------------------------------------------+
//|Function call:     ParseNextLine( pParsData, pszBuffer, usBufSize )         |
//+----------------------------------------------------------------------------+
//|Description:       Get the next line from the input file and store it in    |
//|                   pszBuffer.                                               |
//+----------------------------------------------------------------------------+
//|Parameters:        PPARSEDATA pParsData   general parser data area          |
//|                   PSZ        pszBuffer   buffer for line                   |
//|                   USHORT     usBufSize   size of line buffer               |                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT     function return code                          |
//+----------------------------------------------------------------------------+
USHORT  ParseNextLine
(
  PPARSEDATA  pParsData,
  PSZ         pszBuffer,
  USHORT      usBufSize
)
{
  USHORT      usPos = 0;               // current position in line buffer
  USHORT      usRC = NO_ERROR;         // function return code
  BOOL        fEOL = FALSE;            // end-of-line flag

  // add characters to line buffer until line end detected
  // (line end is CR, CRLF, or LF)
  usPos = 0;
  do
  {
     BYTE bTemp;
     bTemp = ParseNextChar( pParsData, &usRC );
     if ( !usRC )
     {
       // add current character to line buffer
       if ( usPos < usBufSize )
       {
         pszBuffer[usPos++] = bTemp;
       } /* endif */

       // check for line end
       if ( bTemp == CR )
       {
         bTemp = ParseNextChar( pParsData, &usRC );
         if ( bTemp == LF )
         {
           // found CRLF

           // add LF to line buffer
           if ( usPos < usBufSize )
           {
             pszBuffer[usPos++] = bTemp;
           } /* endif */
         }
         else
         {
           // single CR so push current character back
           UndoNextChar( pParsData, bTemp );
         } /* endif */
         fEOL = TRUE;
       }
       else if ( bTemp == LF )
       {
         fEOL = TRUE;
       } /* endif */
     } /* endif */
  } while ( !usRC && !fEOL); /* enddo */

  // add end of string delimiter
  if ( usPos >= usBufSize )
  {
    usPos = usBufSize - 1;
  } /* endif */
  pszBuffer[usPos] = EOS;

  // check for end of file w/o end of line
  if ( !fEOL && (usRC == EOF_REACHED) && (usPos != 0) )
  {
    // reset return code to allow normal processing of data
    usRC = NO_ERROR;
  } /* endif */

  return( usRC );
} /* end of function ParseNextLine */

//+----------------------------------------------------------------------------+
//| ParseInitMorphSupport   - Initialize morphologic support functions         |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Gets the source language of the given document and activates the        |
//|    language support for this language                                      |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PSZ         pszDocSourcePath         ptr to document source path          |
//|  PSHORT      psLangID                 ptr to buffer for language support ID|
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos or Morph error code                 |
//+----------------------------------------------------------------------------+
USHORT ParseInitMorphSupport
(
  PSZ         pszDocPath,              // ptr to document source path
  PSHORT      psLangID                 // ptr to buffer for language support ID
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  CHAR        szDocObjName[MAX_EQF_PATH];// buffer for document object name
  CHAR        szLanguage[MAX_LANG_LENGTH];// buffer for document source language
  PSZ         pszSource, pszTarget;    // ptr for document path processing

  // setup document object name which is the source path without
  // the SOURCE sub-directory
  // (source path is d:\EQF\fff.F00\SOURCE\ddddddd.eee)

  // position pszSource to last backslash and pszTarget to 2nd last backslash
  strcpy( szDocObjName, pszDocPath );
  pszSource = strrchr( szDocObjName, BACKSLASH );
  *pszSource++ = EOS;
  pszTarget = strrchr( szDocObjName, BACKSLASH );
  pszTarget++;

  // copy document name from pszSource to pszTarget
  while ( *pszSource != EOS )
  {
    *pszTarget++ = *pszSource++;
  } /* endwhile */
  *pszTarget = EOS;

  // get document source language
  usRC = DocQueryInfo( szDocObjName, NULL, NULL, szLanguage, NULL, TRUE );

  // activate morphologic support for source language
  if ( usRC == NO_ERROR )
  {
    usRC = MorphGetLanguageID( szLanguage, psLangID );
  } /* endif */

  return( usRC );

} /* end of function ParseInitMorphSupport */

//+----------------------------------------------------------------------------+
//| ParsFillCP                                                                 |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Fill CP in ParsData structure for input file                            |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PPARSEDATA pParsData            - ptr to parser global data structure     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//| ParsData must be allocated                                                 |
//+----------------------------------------------------------------------------+
//| SideEffects:  -                                                             |
//+----------------------------------------------------------------------------+

USHORT ParseFillCP
(
   PPARSEDATA pParsData,                // ptr to parser global data structure
   PSZ        pszInFile
)
{
  PSZ    pszTemp = NULL;
  CHAR   szTgtLang[MAX_LANG_LENGTH];
  USHORT usRC = NO_ERROR;

  strcpy( pParsData->abOutBuf, pszInFile );
  pszTemp  = UtlGetFnameFromPath( pParsData->abOutBuf );
  pszTemp--;
  *pszTemp = EOS;
  pszTemp  = UtlGetFnameFromPath( pParsData->abOutBuf );
  pszTemp--;
  *pszTemp = EOS;
  pszTemp  = UtlGetFnameFromPath( pParsData->abOutBuf );
  if (pszTemp)
  {
    Utlstrccpy( pParsData->szFolder, pszTemp, DOT );
     //         UtlGetFnameFromPath( pParsData->abOutBuf ), DOT );

      UtlMakeEQFPath( pParsData->abTempBuf, *pszInFile, SYSTEM_PATH, NULL );
      strcat( pParsData->abTempBuf, BACKSLASH_STR );
      strcat( pParsData->abTempBuf, pParsData->szFolder );
      strcat( pParsData->abTempBuf, EXT_FOLDER_MAIN );
      strcat( pParsData->abTempBuf, BACKSLASH_STR );
      strcat( pParsData->abTempBuf, UtlGetFnameFromPath( pszInFile ) );

      usRC = DocQueryInfo( pParsData->abTempBuf,    // object name of document
                           NULL,                   // translation memory or NULL
                           NULL,                   // folder format or NULL
                           pParsData->szLanguage,  // source language or NULL
                           szTgtLang,                   // target language or NULL
                           TRUE );                 // do-message-handling flag
  }
  else
  {
      usRC = EQFRS_INVALID_PARM;
  }

  if ( usRC != NO_ERROR )
  {
    // set to system preferences default
    pParsData->ulSrcOemCP = GetLangOEMCP(NULL);
    pParsData->ulTgtOemCP = GetLangOEMCP(NULL);
    pParsData->ulSrcAnsiCP = GetLangAnsiCP(NULL);
    pParsData->ulTgtAnsiCP = GetLangAnsiCP(NULL);
  }
  else
  {
    pParsData->ulSrcOemCP = GetLangOEMCP(pParsData->szLanguage);
    pParsData->ulTgtOemCP = GetLangOEMCP(szTgtLang);
    pParsData->ulSrcAnsiCP = GetLangAnsiCP(pParsData->szLanguage);
    pParsData->ulTgtAnsiCP = GetLangAnsiCP(szTgtLang);
  }
  memset(&pParsData->abOutBuf[0], 0, sizeof(pParsData->abOutBuf));
  memset(&pParsData->abTempBuf[0], 0, sizeof(pParsData->abTempBuf));
  return(usRC);
}


USHORT ParseGetCP
(
  PSZ  pszInFile,
  PULONG pulSrcOemCP,
  PULONG pulTgtOemCP,
  PULONG pulSrcAnsiCP,
  PULONG pulTgtAnsiCP
)
{
  PSZ    pszTemp = NULL;
  CHAR   szTgtLang[MAX_LANG_LENGTH];
  CHAR   szSrcLang[MAX_LANG_LENGTH];
  CHAR   abTempBuf[MAX_LONGFILESPEC];
  CHAR   abOutBuf[MAX_LONGFILESPEC];
  CHAR   szFolder[MAX_LONGFILESPEC];

  USHORT usRC = NO_ERROR;

  strcpy( abOutBuf, pszInFile );
  pszTemp  = UtlGetFnameFromPath( abOutBuf );
  pszTemp--;
  *pszTemp = EOS;
  pszTemp  = UtlGetFnameFromPath( abOutBuf );
  pszTemp--;
  *pszTemp = EOS;
  pszTemp = UtlGetFnameFromPath( abOutBuf );
  if (pszTemp)
  {
    Utlstrccpy( szFolder, pszTemp, DOT );

    UtlMakeEQFPath( abTempBuf, *pszInFile, SYSTEM_PATH, NULL );
    strcat( abTempBuf, BACKSLASH_STR );
    strcat( abTempBuf, szFolder );
    strcat( abTempBuf, EXT_FOLDER_MAIN );
    strcat( abTempBuf, BACKSLASH_STR );
    strcat( abTempBuf, UtlGetFnameFromPath( pszInFile ) );

    usRC = DocQueryInfo( abTempBuf,    // object name of document
                       NULL,                   // translation memory or NULL
                       NULL,                   // folder format or NULL
                       szSrcLang,               // source language or NULL
                       szTgtLang,                   // target language or NULL
                       TRUE );                 // do-message-handling flag
  }
  else
  {
      usRC = EQFRS_INVALID_PARM;
  }

  if ( usRC != NO_ERROR )
  {
    // set to system preferences default
    if (pulSrcOemCP) *pulSrcOemCP = GetLangOEMCP(NULL);
    if (pulTgtOemCP) *pulTgtOemCP = GetLangOEMCP(NULL);
    if (pulSrcAnsiCP) *pulSrcAnsiCP = GetLangAnsiCP(NULL);
    if (pulTgtAnsiCP) *pulTgtAnsiCP = GetLangAnsiCP(NULL);
  }
  else
  {
    if (pulSrcOemCP) *pulSrcOemCP = GetLangOEMCP(szSrcLang);
    if (pulTgtOemCP) *pulTgtOemCP = GetLangOEMCP(szTgtLang);
    if (pulSrcAnsiCP) *pulSrcAnsiCP = GetLangAnsiCP(szSrcLang);
    if ( pulTgtAnsiCP) *pulTgtAnsiCP = GetLangAnsiCP(szTgtLang);
  }

  return(usRC);
}

//+----------------------------------------------------------------------------+
//| StartSegmentW     - start a new segment                                     |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Write start tag of segment to output file and initialize segment buffer.|
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - write starttag for segment to output file and increment          |
//|           segment number                                                   |
//|         - reset output  buffer pointer and output buffer used count        |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PPARSEDATAW pParsDataW            - ptr to parser global data structure     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - Output file must be open.                                               |
//|  - Mode field in global data structure must be set.                        |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  - segment number is incremented                                           |
//|  - segment type field in global data structure is set                      |
//|  - output buffer use count and output buffer pointer are reset             |
//+----------------------------------------------------------------------------+
USHORT StartSegmentW
(
   PPARSEDATAW pParsDataW                // ptr to parser global data structure
)
{
   PSZ_W   pszStartTag;                  // ptr to string of semgent start tag
   USHORT usRC;                        // internal return code
   ULONG   ulBytesWritten;              // number of bytes written to output file

   switch ( pParsDataW->Mode )
   {
      case STRING_MODE:
         pszStartTag = pParsDataW->pQFFTag;
         pParsDataW->usSegType = QFF_SEGMENT;
         break;
      default:
         pszStartTag = pParsDataW->pQFNTag;
         pParsDataW->usSegType = QFN_SEGMENT;
         break;
   } /* endswitch */

   pParsDataW->ulSegNum++;

   swprintf( pParsDataW->abOutBuf, pszStartTag, pParsDataW->ulSegNum );

   usRC = UtlWriteL( pParsDataW->hOutFile,
                    pParsDataW->abOutBuf,
                    UTF16strlenBYTE(pParsDataW->abOutBuf),
                    &ulBytesWritten,
                    TRUE );

   pParsDataW->pOutBuf = pParsDataW->abOutBuf;
   pParsDataW->ulOutBufUsed = 0;

   return( usRC );
} /* end of StartSegmentW */

//+----------------------------------------------------------------------------+
//| AddToSegmentW     - add data to current segment                             |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Add one byte to the current segment and, if the segment buffer is full, |
//|    write the segment to the output file and start a new one.               |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - if segment buffer is full end the current segment and start      |
//|           a new one.                                                       |
//|         - append given byte to current segment                             |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PPARSEDATA pParsData            - ptr to parser global data structure     |
//|  BYTE       bAddByte             - byte being added to segment             |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - Output file must be open.                                               |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
USHORT AddToSegmentW
(
   PPARSEDATAW pParsDataW,               // ptr to parser global data structure
   CHAR_W       chAddCharW                 // byte being added to segment
)
{
   USHORT usRC = 0;                    // internal return code
   ULONG  ulRest;                      // rest of segment
   PCHAR_W  pchTemp;                      // ptr for segment data processing

   //
   // force a new segment if segment buffer is full
   //
   if ( pParsDataW->ulOutBufUsed == MAX_SEG_SIZE )
   {
      // look for the right place (= end of a line) to split data in
      // segment buffer
      pchTemp = pParsDataW->abOutBuf + (pParsDataW->ulOutBufUsed - 1);
      while ( (pchTemp > pParsDataW->abOutBuf) &&
               (*pchTemp != LF) && (*pchTemp != CR) )
      {
         pchTemp--;
      } /* endwhile */

      // if no line end character is in segment write segment as-is
      // otherwise split segment at line end character
      if ( pchTemp <= pParsDataW->abOutBuf )
      {
         // no line end characters in segment
         usRC = EndSegmentW( pParsDataW );
         if ( !usRC )
         {
            usRC = StartSegmentW( pParsDataW );
         } /* endif */
      }
      else
      {
         // split segment after line end character
         pchTemp++;
         ulRest = (pParsDataW->ulOutBufUsed - (pchTemp - pParsDataW->abOutBuf));

         if ( ulRest )
         {
            memcpy( pParsDataW->abTempBuf, pchTemp, ulRest * sizeof(CHAR_W) );
            pParsDataW->ulOutBufUsed = pParsDataW->ulOutBufUsed - ulRest;
         } /* endif */

         usRC = EndSegmentW( pParsDataW );
         if ( !usRC )
         {
            usRC = StartSegmentW( pParsDataW );
         } /* endif */

         if ( !usRC && ulRest )
         {
            memcpy( pParsDataW->abOutBuf, pParsDataW->abTempBuf, ulRest * sizeof(CHAR_W));
            pParsDataW->ulOutBufUsed = ulRest;
            pParsDataW->pOutBuf = pParsDataW->abOutBuf + ulRest;
         } /* endif */

      } /* endif */
   } /* endif */

   //
   // append new byte to segment buffer
   //
   if ( !usRC )
   {
      *(pParsDataW->pOutBuf)++ = chAddCharW;
      pParsDataW->ulOutBufUsed++;
   } /* endif */

   return( usRC );
} /* end of AddToSegmentW */

//+----------------------------------------------------------------------------+
//| ParseNextCharW    - retrieve the next input character                       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Returns the next character from the input file and checks end-of-file   |
//|    condition.                                                              |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - if input buffer is empty read next data from input file and      |
//|           check end-of-file condition                                      |
//|         - if input buffer contains data and data is not end-of-file        |
//|           character return next byte from buffer else set return code to   |
//|           EOF_REACHED.                                                     |
//+----------------------------------------------------------------------------+
//|                                                                            |
//| Arguments:                                                                 |
//|  PPARSEDATA pParsData,           - ptr to parser global data structure     |
//|  PUSHORT    pusRC                - return code (set in case of errors)     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - Input file must be open.                                                |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  - input buffer use count and bytes to read count are changed              |
//+----------------------------------------------------------------------------+
CHAR_W  ParseNextCharW
(
   PPARSEDATAW pParsDataW,               // ptr to parser global data structure
   PUSHORT    pusRC                    // return code (set in case of errors)
)
{
   CHAR_W   chNextCharW = EOS;            // byte being returned
   ULONG    ulBytesToRead;               // number of bytes to read from file
   USHORT   usRC = 0;                    // return code of called functions
   USHORT   usComplete;                  // completion ratio

   usComplete = pParsDataW->usPercentComplete;
   *pusRC = 0;                         // init return code...

   /*******************************************************************/
   /* check if next character is to be taken from undo buffer or from */
   /* input buffer                                                    */
   /*******************************************************************/
   if ( pParsDataW->ulUndoBufUsed )
   {
     /*****************************************************************/
     /* UNDO buffer is not empty: get character from this buffer      */
     /*****************************************************************/
     chNextCharW = pParsDataW->abUndoBuf[ --pParsDataW->ulUndoBufUsed ];
     if ( chNextCharW == LF_W )
     {
       pParsDataW->ulLinePos = 0;                   // reset position in line
     }
     else
     {
       pParsDataW->ulLinePos++;                     // next position in line
     } /* endif */

   }
   else
   {
     if ( !pParsDataW->ulBytesInBuffer )
     {
        if ( !pParsDataW->lBytesToRead )
        {
           // error: file has been processed completely
           *pusRC = EOF_REACHED;
        }
        else
        {
           ulBytesToRead =  min( (LONG)(INBUF_SIZE * sizeof(CHAR_W)),
                                         pParsDataW->lBytesToRead );
           usRC = UtlReadL( pParsDataW->hInFile, pParsDataW->abInBuf,
                           ulBytesToRead, &pParsDataW->ulBytesInBuffer, TRUE );
           if ( usRC )
           {
              *pusRC = usRC;
           }
           else
           {
              pParsDataW->pInBuf = pParsDataW->abInBuf;
              pParsDataW->lBytesToRead -= pParsDataW->ulBytesInBuffer;
           } /* endif */
        } /* endif */
     } /* endif */

     if ( pParsDataW->ulBytesInBuffer )
     {
        if ( pParsDataW->ulBytesInBuffer == 1 )
        {
          // incomplete character in buffer, probably at the end of the file...
          PBYTE pbData = (PBYTE)pParsDataW->pInBuf;
          chNextCharW = *pbData;
          pParsDataW->ulBytesInBuffer -= 1;
        }
        else
        {
          chNextCharW = *(pParsDataW->pInBuf)++;
          pParsDataW->ulBytesInBuffer -= 2;
        } /* endif */
        pParsDataW->ulLinePos++;             // next position in line
        switch ( chNextCharW )
        {
          case  END_OF_FILE:
            *pusRC = EOF_REACHED;
             usComplete = 100;           // we are through ...
            break;
          case  LF:
            pParsDataW->ulLinePos = 0;       // start at line
            usComplete = (USHORT) ((pParsDataW->lTotalBytes -
                                    pParsDataW->lBytesToRead -
                                    (LONG)pParsDataW->ulBytesInBuffer) *
                                    100 /
                                    pParsDataW->lTotalBytes);
            break;
          default :
            break;
        } /* endswitch */
     }
     else
     {
       usComplete = 100;               // we are through ...
     } /* endif */
   } /* endif */


   if ( usComplete != pParsDataW->usPercentComplete )
   {
     if ( pParsDataW->hwndSlider )
     {
       WinSendMsg( pParsDataW->hwndSlider, WM_EQF_UPDATESLIDER,
                   MP1FROMSHORT(usComplete), NULL );
     } /* endif */
     UtlDispatch();
     if ( *(pParsDataW->pfKill) )
     {
       // error: we have to leave asap -- simulate read error. ...
       *pusRC = (USHORT) ERR_READFILE;
     } /* endif */
   } /* endif */

   return( chNextCharW );
} /* end of ParseNextCharW */


//+----------------------------------------------------------------------------+
//| UndoNextChar     - put getted character back in input queue                |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Resets the pointers to indicate that last next character function is    |
//|    undone                                                                  |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - change input buffer pointer and number of bytes in input buffer  |
//+----------------------------------------------------------------------------+
//|                                                                            |
//| Arguments:                                                                 |
//|  PPARSEDATA pParsData            - ptr to parser global data structure     |
//|  BYTE       bChar                - character to be passed to undo buffer.. |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  VOID                                                                      |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - character must be get with NextChar                                     |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  - input buffer use count and bytes to read count are changed              |
//+----------------------------------------------------------------------------+
VOID  UndoNextCharW
(
   PPARSEDATAW pParsDataW,               // ptr to parser global data structure
   CHAR_W       chCharW                    // character to be undo ...
)
{

   if ( pParsDataW->ulUndoBufUsed < MAX_SEG_SIZE )
   {
     pParsDataW->abUndoBuf[ pParsDataW->ulUndoBufUsed++ ] = chCharW;
     pParsDataW->ulLinePos--;           // prev position in line
   } /* endif */

} /* end of UndoNextCharW */

VOID  UndoNextCharWEx
(
   PPARSEDATAW pParsDataW,               // ptr to parser global data structure
   CHAR_W       chCharW,                 // character to be undo ...
   PBOOL        pfIsDBCS
)
{

   if ( pParsDataW->ulUndoBufUsed < MAX_SEG_SIZE )
   {
     pParsDataW->abUndoBuf[ pParsDataW->ulUndoBufUsed++ ] = chCharW;
     pParsDataW->ulLinePos--;           // prev position in line
   } /* endif */

   *pfIsDBCS = Parse_IsDBCSChar( chCharW, pParsDataW->ulTgtAnsiCP);

    if (*pfIsDBCS)
    pParsDataW->ulLinePos--;           // prev position in line


} /* end of UndoNextCharWEx */

//+----------------------------------------------------------------------------+
//| EndSegment     - terminate current segment                                 |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Write data of segment and segment end tag to output file.               |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - write data in output buffer to output file                       |
//|         - reset output  buffer pointer and output buffer used count        |
//|         - write end tag for this segment type to output file               |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PPARSEDATA pParsData            - ptr to parser global data structure     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - Output file must be open.                                               |
//|  - Segment type field in global data structure must be set.                |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  - output buffer use count and output buffer pointer are reset             |
//+----------------------------------------------------------------------------+
USHORT EndSegmentW
(
   PPARSEDATAW pParsDataW                // ptr to parser global data structure
)
{
   PSZ_W   pszEndTagW = NULL;                    // ptr to string of segment end tag
   USHORT  usRC = 0;                    // internal return code
   ULONG   ulBytesWritten;              // number of bytes written to output file

   // write data of segment to output file
   if ( pParsDataW->ulOutBufUsed )
   {
      usRC = UtlWriteL( pParsDataW->hOutFile,
                       pParsDataW->abOutBuf,
                       (pParsDataW->ulOutBufUsed) * sizeof(CHAR_W),
                       &ulBytesWritten,
                       TRUE );
   } /* endif */

   // reset outpuffer used count and output buffer pointer
   if ( !usRC )
   {
      pParsDataW->pOutBuf = pParsDataW->abOutBuf;
      pParsDataW->ulOutBufUsed = 0;
   } /* endif */

   // write segment end tag to output file
   if ( !usRC )
   {
      switch ( pParsDataW->usSegType )
      {
         case QFF_SEGMENT:
            pszEndTagW = pParsDataW->pEQFFTag;
            break;
         case QFN_SEGMENT:
            pszEndTagW = pParsDataW->pEQFNTag;
            break;
      } /* endswitch */

      usRC = UtlWriteL( pParsDataW->hOutFile,
                       pszEndTagW,
                       UTF16strlenBYTE(pszEndTagW),
                       &ulBytesWritten,
                       TRUE );
   } /* endif */

   return( usRC );
} /* end of EndSegmentW */

//+----------------------------------------------------------------------------+
//| AddToFileW        - add data to current segment                             |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Add one byte to the current segment                                     |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - if segment buffer is full end the current segment and start      |
//|           a new one.                                                       |
//|         - append given byte to current segment                             |
//+----------------------------------------------------------------------------+
//| Arguments:                                                                 |
//|  PPARSEDATAW pParsDataW            - ptr to parser global data structure     |
//|  BYTE       bAddByte             - byte being added to segment             |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - Output file must be open.                                               |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  None.                                                                     |
//+----------------------------------------------------------------------------+
USHORT AddToFileW
(
   PPARSEDATAW pParsDataW,               // ptr to parser global data structure
   CHAR_W       chAddCharW                 // byte being added to segment
)
{
   USHORT usRC = 0;                    // internal return code

   /*******************************************************************/
   /* add position in output line and reset it if necessary           */
   /*******************************************************************/
   pParsDataW->ulOutPos++;              // increment position in out file
   if ( chAddCharW == LF_W )
   {
     pParsDataW->ulOutPos = 0;
   } /* endif */
   /*******************************************************************/
   /* if buffer filled up write it...                                 */
   /*******************************************************************/
   if ( pParsDataW->ulOutBufUsed >= MAX_OUTBUF_SIZE )
   {
     // just for testing ...
     pParsDataW->ulOutBufUsed = pParsDataW->ulOutBufUsed;
   }

   if ( pParsDataW->ulOutBufUsed == MAX_OUTBUF_SIZE )
   {
     usRC = WriteBufferW( pParsDataW );
   } /* endif */
   //
   // append new byte to segment buffer
   //
   if ( !usRC )
   {
      *(pParsDataW->pOutBuf)++ = chAddCharW;
      pParsDataW->ulOutBufUsed++;
   } /* endif */

   return( usRC );
} /* end of AddToFileW */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     WriteBufferW                                              |
//+----------------------------------------------------------------------------+
//|Function call:     WriteBufferW ( PPARSEDATAW );                              |
//+----------------------------------------------------------------------------+
//|Description:       this function will write out a buffer and reset          |
//|                   the appropriate pointers                                 |
//+----------------------------------------------------------------------------+
//|Parameters:        PPARSEDATAW       pointer to buffer structure             |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       0    okay                                                |
//|                   usRC DOS error                                           |
//+----------------------------------------------------------------------------+
//|Function flow:     _                                                        |
//+----------------------------------------------------------------------------+
USHORT WriteBufferW
(
  PPARSEDATAW pParsDataW
)
{
   USHORT usRC;                        // internal return code
   ULONG  ulBytesWritten;

   // write data of segment to output file
   usRC = UtlWriteL( pParsDataW->hOutFile,
                    pParsDataW->abOutBuf,
                    pParsDataW->ulOutBufUsed * sizeof (CHAR_W),
                    &ulBytesWritten,
                    TRUE );

   // reset outpuffer used count and output buffer pointer
   if ( !usRC )
   {
      pParsDataW->pOutBuf = pParsDataW->abOutBuf;
      pParsDataW->ulOutBufUsed = 0;
   } /* endif */
   return( usRC );
} /* end of function WriteBufferW */


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ParseInitW                                                |
//+----------------------------------------------------------------------------+
//|Function call:     ParseInitW( pParsDataW, hwndSlider, pfKill );              |
//+----------------------------------------------------------------------------+
//|Description:       do the initial settings                                  |
//+----------------------------------------------------------------------------+
//|Parameters:        PPARSEDATAW pParsDataW,     parse data control block       |
//|                   HWND       hwndSlider     slider handle                  |
//|                   PBOOL      pfKill         kill flag                      |
//+----------------------------------------------------------------------------+
//|Returncode type:   VOID                                                     |
//+----------------------------------------------------------------------------+
//|Function flow:     set the initial settings                                 |
//+----------------------------------------------------------------------------+
VOID
ParseInitW
(
  PPARSEDATAW pParsDataW,                // parse data control block
  HWND       hwndSlider,               // slider handle
  PEQF_BOOL  pfKill                    // pointer to kill flag
)
{
  memset( pParsDataW, 0, sizeof(PARSEDATAW) );
  pParsDataW->Mode = OUTSIDE_STRING_MODE;
  pParsDataW->hwndSlider = hwndSlider;
  pParsDataW->pfKill     = pfKill;

  // Setup tag names
  // Note: this should be done dynamically based on the QFTAG table!
  //       Look at EQFBFileWrite for an example
  pParsDataW->pQFNTag   = L":QFN N=%d.";
  pParsDataW->pEQFNTag  = L":EQFN.";
  pParsDataW->pQFFTag   = L":QFF N=%d.";
  pParsDataW->pEQFFTag  = L":EQFF.";

  /********************************************************************/
  /* init the output buffer                                           */
  /********************************************************************/
  pParsDataW->pOutBuf = pParsDataW->abOutBuf;
  pParsDataW->ulOutBufUsed = 0;
  return;
} /* end of function ParseInitW */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     ParseCloseW                                               |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = ParseCloseW( pParsDataW );                          |
//+----------------------------------------------------------------------------+
//|Description:       do a close of the Parseing block                         |
//+----------------------------------------------------------------------------+
//|Parameters:        PPARSEDATAW pParsDataW        control data block           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       DOS RCs                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     call the WriteBuffer function                            |
//+----------------------------------------------------------------------------+
USHORT
ParseCloseW
(
  PPARSEDATAW pParsDataW
)
{
  return( WriteBufferW( pParsDataW ) );
} /* end of function ParseCloseW */

USHORT AddCharsToFileW
(
  PPARSEDATAW  pParseDataW,               // pointer to control block
  PSZ_W        pStringW,                  // string to be copied
  ULONG        ulLen                     // number of bytes
)
{
  USHORT usRC = 0;                     // success indicator
  ULONG  i;                            // index


  for ( i=0; (i < ulLen) && !usRC ;i++ )
  {
    usRC = AddToFileW( pParseDataW, *pStringW++);
  } /* endfor */
  return usRC;
} /* end of function AddCharsToFile */

//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     AddStringToFile                                          |
//+----------------------------------------------------------------------------+
//|Function call:     usRC = AddStringToFile( pParseData, pString );           |
//+----------------------------------------------------------------------------+
//|Description:       copy the specified string to the file                    |
//+----------------------------------------------------------------------------+
//|Parameters:        PPARSEDATA pParseData      pointer to control block      |
//|                   PSZ        pString         string to be copied           |
//+----------------------------------------------------------------------------+
//|Returncode type:   USHORT                                                   |
//+----------------------------------------------------------------------------+
//|Returncodes:       DOS RCs                                                  |
//+----------------------------------------------------------------------------+
//|Function flow:     loop as long as characters are available and no RC       |
//|                     Add them to the file                                   |
//|                   endfor                                                   |
//|                   return success indicator                                 |
//+----------------------------------------------------------------------------+

USHORT AddStringToFileW
(
  PPARSEDATAW pParseDataW,               // pointer to control block
  PSZ_W        pStringW                   // string to be copied
)
{
  USHORT usRC = 0;                     // success indicator

  while ( (*pStringW != EOS) && !usRC )
  {
    usRC = AddToFileW( pParseDataW, *pStringW++ );
  } /* endwhile */
  return usRC;
} /* end of function AddStringToFileW */

USHORT ParseFillCPW
(
   PPARSEDATAW pParsDataW,                // ptr to parser global data structure
   PSZ        pszInFile
)
{
  PSZ    pszTemp = NULL;
  CHAR   szTgtLang[MAX_LANG_LENGTH];
  USHORT usRC = NO_ERROR;
  CHAR   szObjName[MAX_EQF_PATH];        // buffer for document object name
  CHAR   szTemp[MAX_OUTBUF_SIZE];         // buffer for document name

  strcpy( szTemp, pszInFile );
  pszTemp  = UtlGetFnameFromPath( szTemp );
  pszTemp--;
  *pszTemp = EOS;
  pszTemp  = UtlGetFnameFromPath( szTemp );
  pszTemp--;
  *pszTemp = EOS;
  pszTemp  = UtlGetFnameFromPath( szTemp );
  if (pszTemp)
  {
    Utlstrccpy( pParsDataW->szFolder, pszTemp, DOT );
     //         UtlGetFnameFromPath( pParsData->abOutBuf ), DOT );

      UtlMakeEQFPath( szObjName, *pszInFile, SYSTEM_PATH, NULL );
      strcat( szObjName, BACKSLASH_STR );
      strcat( szObjName, pParsDataW->szFolder );
      strcat( szObjName, EXT_FOLDER_MAIN );
      strcat( szObjName, BACKSLASH_STR );
      strcat( szObjName, UtlGetFnameFromPath( pszInFile ) );

      usRC = DocQueryInfo( szObjName,    // object name of document
                           NULL,                   // translation memory or NULL
                           NULL,                   // folder format or NULL
                           pParsDataW->szLanguage,  // source language or NULL
                           szTgtLang,                   // target language or NULL
                           TRUE );                 // do-message-handling flag
  }
  else
  {
      usRC = EQFRS_INVALID_PARM;
  }

  if ( usRC != NO_ERROR )
  {
    // set to system preferences default
    pParsDataW->ulSrcOemCP = GetLangOEMCP(NULL);
    pParsDataW->ulTgtOemCP = GetLangOEMCP(NULL);
    pParsDataW->ulSrcAnsiCP = GetLangAnsiCP(NULL);
    pParsDataW->ulTgtAnsiCP = GetLangAnsiCP(NULL);
  }
  else
  {
    pParsDataW->ulSrcOemCP = GetLangOEMCP(pParsDataW->szLanguage);
    pParsDataW->ulTgtOemCP = GetLangOEMCP(szTgtLang);
    pParsDataW->ulSrcAnsiCP = GetLangAnsiCP(pParsDataW->szLanguage);
    pParsDataW->ulTgtAnsiCP = GetLangAnsiCP(szTgtLang);
  }
  memset(&pParsDataW->abOutBuf[0], 0, sizeof(pParsDataW->abOutBuf));
  memset(&pParsDataW->abTempBuf[0], 0, sizeof(pParsDataW->abTempBuf));
  return(usRC);
}

// Function to check if specified character is a DBCS character or not
static
BOOL Parse_IsDBCSChar( CHAR_W c, ULONG ulCP)
{
    CHAR_W chW[2];
    CHAR        ch[5];

    chW[0] = c; chW[1] = EOS;
    memset (ch, 0, sizeof(ch));

    return (Parse_Unicode2ASCIIBufEx(chW, ch, 1, sizeof(ch), ulCP, FALSE, NULL,
                               WC_NO_BEST_FIT_CHARS ) == 2 );
}
// copy of function Unicode2ASCIIBufEx from eqfutils.c
// since downward compat. is needed - and as hotfix for P020990 only exit dll is distributed
// no other possibility has been found

// ulLen = # of char-W's in pszUni!
// USE PARSE_Unicode2ASCIIBufEx only within Parse_IsDBCSChar - BIDI area is not handled
// correctly - BidiConvertFE functionality had been deleted since it is not available!!

static
ULONG Parse_Unicode2ASCIIBufEx( PSZ_W pszUni, PSZ pszASCII, ULONG ulLen, LONG lBufLen,
                          ULONG ulCP, BOOL fMsg, PLONG plRc, DWORD dwFlags )
{
  static CHAR_W szUniTemp[ MAX_SEGMENT_SIZE ];
  ULONG ulOutPut = 0;
  USHORT usCP = (USHORT) ulCP;
  LONG    lRc = 0;
  if (!usCP)
  {
     ulCP = (USHORT)GetLangOEMCP(NULL);
     usCP = (USHORT)ulCP;
  }

  if (dwFlags != 0)
  {
	  if ( (ulCP == 50220) || (ulCP == 50221) || (ulCP == 50222) || (ulCP== 50225)
	      || (ulCP == 50227) || (ulCP == 50229) || (ulCP== 52936) || (ulCP == 54936)
	      || (ulCP == 65000) || (ulCP == 65001) || ((57002 <= ulCP) && (ulCP <= 57011)) )
	  { // dwflags must be 0 for these cp, otherwise the functions fails with ERROR_INVALID_FLAGS
		  dwFlags = 0;
      }
  } /* endif */

  if ( pszUni && pszASCII )
  {
	PSZ_W pTemp = NULL;

	USHORT usCP = (USHORT) ulCP;

	// do special handling for Arabic to get rid of shaping to allow for conversion
	// back to 864
	*pszASCII = 0;
	if (ulLen)
	{
	   switch ( usCP )
	   {
		  case 864:
			 // NOP - use only to check for DBCS chars!
			 ulOutPut = 1;  // character length is 1 !
			break;
		  case 867:
		  case 862:
			// convert the LRM&RLM markers to codepoints in the standard 862 page,
			// so that our WideCharToMultiByte can handle it...
			UtlAlloc( (PVOID *) &pTemp, 0L, (ulLen+1) * sizeof(CHAR_W), ERROR_STORAGE );

			if (pTemp)
			{
			  CHAR_W c;
			  PSZ_W  p = pTemp;
			  UTF16strncpy(p, pszUni, ulLen);

			  while ((c = *p++)!= NULC)
			  {
				switch (c)
				{
				  case 0x200E:      // Left-to-Right marker
					*(p-1) = 0x00E1;// Unicode character converted from CP862 (0xA0)
					break;
				  case 0x200F:      // Right-to-Left marker
					*(p-1) = 0x00ED;// Unicode character converted from CP862 (0xA1)
					break;
				}
			  }
			  usCP = 862;     // we have to use CP862 since Windows only supports this

			  ulOutPut = WideCharToMultiByte( usCP, 0, (LPCWSTR)pTemp, ulLen,
											  pszASCII, lBufLen, NULL, NULL );
			  if (!ulOutPut )
			  {
				  lRc = GetLastError();
			  }

			  UtlAlloc( (PVOID *) &pTemp, 0L, 0L, NOMSG );
			}
			break;
		  default:
			  ulOutPut = WideCharToMultiByte( usCP, 0, (LPCWSTR)pszUni, ulLen,
											pszASCII, lBufLen, NULL, NULL );

			  if (!ulOutPut )
			  {
				  lRc = GetLastError();
			  }
			break;
	   } /* endswitch */
    } /* endif ulLen */
  }
  else if (pszASCII)
  {
  *pszASCII = EOS;
  }
  if (plRc)
  {
       *(plRc) = lRc;
  }
  if (fMsg && lRc)
  {        CHAR szTemp[10];
		   PSZ pszTemp = szTemp;
		   sprintf(szTemp, "%d", lRc);

		   UtlError( ERROR_DATA_CONVERSION, MB_CANCEL, 1, &pszTemp, EQF_ERROR );
  }
  return( ulOutPut );
}

//+----------------------------------------------------------------------------+
//| ParseNextCharW    - retrieve the next input character                       |
//+----------------------------------------------------------------------------+
//| Description:                                                               |
//|    Returns the next character from the input file and checks end-of-file   |
//|    condition.                                                              |
//+----------------------------------------------------------------------------+
//|   Flow:                                                                    |
//|         - if input buffer is empty read next data from input file and      |
//|           check end-of-file condition                                      |
//|         - if input buffer contains data and data is not end-of-file        |
//|           character return next byte from buffer else set return code to   |
//|           EOF_REACHED.                                                     |
//+----------------------------------------------------------------------------+
//|                                                                            |
//| Arguments:                                                                 |
//|  PPARSEDATA pParsData,           - ptr to parser global data structure     |
//|  PUSHORT    pusRC                - return code (set in case of errors)     |
//+----------------------------------------------------------------------------+
//| Returns:                                                                   |
//|  USHORT usRC      0              - success                                 |
//|                   other          - Dos error code                          |
//+----------------------------------------------------------------------------+
//| Prereqs:                                                                   |
//|  - Input file must be open.                                                |
//+----------------------------------------------------------------------------+
//| SideEffects:                                                               |
//|  - input buffer use count and bytes to read count are changed              |
//+----------------------------------------------------------------------------+
CHAR_W  ParseNextCharWEx
(
   PPARSEDATAW pParsDataW,               // ptr to parser global data structure
   PUSHORT    pusRC,                    // return code (set in case of errors)
   PBOOL      pfIsDBCS
)
{
   CHAR_W   chNextCharW = EOS;            // byte being returned
   ULONG    ulBytesToRead;               // number of bytes to read from file
   USHORT   usRC = 0;                    // return code of called functions
   USHORT   usComplete;                  // completion ratio

   usComplete = pParsDataW->usPercentComplete;
   *pusRC = 0;                         // init return code...

   /*******************************************************************/
   /* check if next character is to be taken from undo buffer or from */
   /* input buffer                                                    */
   /*******************************************************************/
   if ( pParsDataW->ulUndoBufUsed )
   {
     /*****************************************************************/
     /* UNDO buffer is not empty: get character from this buffer      */
     /*****************************************************************/
     chNextCharW = pParsDataW->abUndoBuf[ --pParsDataW->ulUndoBufUsed ];
     if ( chNextCharW == LF_W )
     {
       pParsDataW->ulLinePos = 0;                   // reset position in line
     }
     else
     {
       pParsDataW->ulLinePos++;                     // next position in line
     } /* endif */

   }
   else
   {
     if ( !pParsDataW->ulBytesInBuffer )
     {
        if ( !pParsDataW->lBytesToRead )
        {
           // error: file has been processed completely
           *pusRC = EOF_REACHED;
        }
        else
        {
           ulBytesToRead =  min( (LONG)(INBUF_SIZE * sizeof(CHAR_W)),
                                         pParsDataW->lBytesToRead );
           usRC = UtlReadL( pParsDataW->hInFile, pParsDataW->abInBuf,
                           ulBytesToRead, &pParsDataW->ulBytesInBuffer, TRUE );
           if ( usRC )
           {
              *pusRC = usRC;
           }
           else
           {
              pParsDataW->pInBuf = pParsDataW->abInBuf;
              pParsDataW->lBytesToRead -= pParsDataW->ulBytesInBuffer;
           } /* endif */
        } /* endif */
     } /* endif */

     if ( pParsDataW->ulBytesInBuffer )
     {
        chNextCharW = *(pParsDataW->pInBuf)++;
        pParsDataW->ulBytesInBuffer -= 2;
        pParsDataW->ulLinePos++;             // next position in line
        switch ( chNextCharW )
        {
          case  END_OF_FILE:
            *pusRC = EOF_REACHED;
             usComplete = 100;           // we are through ...
            break;
          case  LF:
            pParsDataW->ulLinePos = 0;       // start at line
            usComplete = (USHORT) ((pParsDataW->lTotalBytes -
                                    pParsDataW->lBytesToRead -
                                    (LONG)pParsDataW->ulBytesInBuffer) *
                                    100 /
                                    pParsDataW->lTotalBytes);
            break;
          default :
            break;
        } /* endswitch */
     }
     else
     {
       usComplete = 100;               // we are through ...
     } /* endif */
   } /* endif */


   if ( usComplete != pParsDataW->usPercentComplete )
   {
     if ( pParsDataW->hwndSlider )
     {
       WinSendMsg( pParsDataW->hwndSlider, WM_EQF_UPDATESLIDER,
                   MP1FROMSHORT(usComplete), NULL );
     } /* endif */
     UtlDispatch();
     if ( *(pParsDataW->pfKill) )
     {
       // error: we have to leave asap -- simulate read error. ...
       *pusRC = (USHORT) ERR_READFILE;
     } /* endif */
   } /* endif */

   *pfIsDBCS = Parse_IsDBCSChar( chNextCharW, pParsDataW->ulTgtAnsiCP);
   if (*pfIsDBCS)
    pParsDataW->ulLinePos++;           // next position in line

   return( chNextCharW );
} /* end of ParseNextCharWEx */

