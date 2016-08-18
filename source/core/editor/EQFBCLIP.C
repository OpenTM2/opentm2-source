/*! \file
	Copyright Notice:

	Copyright (C) 1990-2013, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // public EDITOR API functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#include "eqf.h"                  // General Translation Manager include file

#include "EQFTPI.H"               // Translation Processor priv. include file


//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     EQFBFuncPasteFromClip                                     
//------------------------------------------------------------------------------
// Function call:     EQFBFuncPasteFromClip(PTBDOCUMENT)                        
//------------------------------------------------------------------------------
// Description:       paste from clipboard                                      
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc pointer to document instance             
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     - open clipboard                                          
//                    - if text in clipboard:                                   
//                         - if new segment length would be too long:           
//                                  issue error message                         
//                           else if input not allowed: beep                    
//                                else if right of text:                        
//                                        pad out with blanks                   
//                                     copy text from clipboard                 
//                                     call EQFBUpdateChangedSeg                
//                      else issue error message ( clipboard empty)             
//                    - close clipboard                                         
//------------------------------------------------------------------------------


VOID  EQFBFuncPasteFromClip
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
    LONG    lLength;                  // length of area to copy

    PSZ_W   pchClipText;               // pointer to clip board text
    PSZ_W   pSrc;                      // tmp ptr to data             /* @7AA */
    PSZ_W   pTgt;                      // tmp ptr to data             /* @7AA */
    PSZ_W   pStart;

    HANDLE  hClipMemory;

    OpenClipboard(pDoc->hwndClient);
    hClipMemory = GetClipboardData(CF_UNICODETEXT);
    pchClipText = (PSZ_W)GlobalLock (hClipMemory);

    // if text in clipboard
    if ( pchClipText )
    {
      /****************************************************************/
      /* copy data in own memory                                      */
      /****************************************************************/
      lLength = UTF16strlenBYTE(pchClipText) + sizeof(CHAR_W); // get length of text
      UtlAlloc( (PVOID *)&pStart, 0L, lLength, ERROR_STORAGE );
      memcpy(  pStart, pchClipText, lLength);

      pTgt = pStart;
      pSrc = pStart;

      /****************************************************************/
      /* normalize CRLF sequences to LF only                          */
      /****************************************************************/
      while ( *pSrc )
      {
        /************************************************************//* @7AA */
        /* reduce CR LF sequence to one                             *//* @7AA */
        /************************************************************//* @7AA */
        if ( (*pSrc == CR) && (*(pSrc+1) == LF) )                     /* @7AA */
        {                                                             /* @7AA */
          pSrc++;                                                     /* @7AA */
        }                                                             /* @7AA */
        else                                                          /* @7AA */
        {                                                             /* @7AA */
          *pTgt++ = *pSrc++;                                          /* @7AA */
        } /* endif */                                                 /* @7AA */
      } /* endwhile */                                                /* @7AA */
      *pTgt = EOS;                                                    /* @7AA */

     EQFBDoCopyData(pDoc, pStart);

      UtlAlloc( (PVOID *)&pStart, 0L, 0L, NOMSG );
    }
    else
    {
      // try with text format only
      hClipMemory = GetClipboardData(CF_TEXT);
      pchClipText = (PSZ_W)GlobalLock (hClipMemory);
      // if text in clipboard
      if ( pchClipText )
      {
        /****************************************************************/
        /* copy data in own memory                                      */
        /****************************************************************/
        lLength = strlen((PSZ)pchClipText) + 1; // get length of text
        UtlAlloc( (PVOID *)&pStart, 0L, lLength * sizeof(CHAR_W), ERROR_STORAGE );
        UtlDirectAnsi2Unicode((PSZ)pchClipText, pStart, pDoc->ulAnsiCodePage );

        pTgt = pStart;
        pSrc = pStart;

        /****************************************************************/
        /* normalize CRLF sequences to LF only                          */
        /****************************************************************/
        while ( *pSrc )
        {
		  /************************************************************/
          /* reduce CR LF sequence to one                             */
          /************************************************************/
          if ( (*pSrc == CR) && (*(pSrc+1) == LF) )
          {
            pSrc++;
          }
          else
          {
            *pTgt++ = *pSrc++;
          } /* endif */
        } /* endwhile */
        *pTgt = EOS;

         EQFBDoCopyData(pDoc, pStart);

         UtlAlloc( (PVOID *)&pStart, 0L, 0L, NOMSG );
       }
       else
       {
         UtlError( TB_NOCLIPBOARDDATA, MB_CANCEL, 0, NULL, EQF_WARNING);
       }
    } /* endif */

    GlobalUnlock(hClipMemory);
    CloseClipboard();
   return;
}
//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     EQFBDoCopyData                                            
//------------------------------------------------------------------------------
// Function call:     EQFBDoCopyData    (PTBDOCUMENT)                           
//------------------------------------------------------------------------------
// Description:       copy data into active segment at cursor position          
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc   pointer to document instance           
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     if block is marked                                        
//------------------------------------------------------------------------------

BOOL  EQFBDoCopyData
(
    PTBDOCUMENT  pDoc,                                     // pointer to document ida
    PSZ_W        pData
)
{
    LONG         lLength;
    LONG         lPos;
    SHORT        sLen;
    USHORT       usSegOffOld = 0;
    ULONG        ulAbsLocat = 0;
    USHORT       usLocat = 0;
    USHORT       usType = 0;
    BOOL         fEndOfSeg = FALSE;
    BOOL         fLineFeed = FALSE;
    BOOL         fOK = TRUE;

    lLength = UTF16strlenCHAR(pData); // get length of text
    lPos = EQFBCurSegFromCursor( pDoc );
    sLen = (SHORT)UTF16strlenCHAR( pDoc->pEQFBWorkSegmentW );   // length of work segment
    if (!pDoc->EQFBFlags.Reflow)
    {
		PSZ_W pSrc = pData;

		while ( *pSrc )
		{
		  if (*pSrc == LF)
		  {
			  fLineFeed = TRUE;
		  }
		  pSrc++;
		} /* endwhile */
		if (fLineFeed)
		{
			UtlError( TB_LFCHANGE_NOT_ALLOWED, MB_CANCEL, 0, NULL, EQF_WARNING);
			fOK = FALSE;
		}
    } /* endif */
    if (fOK)
    {
      if ( lLength - lPos + sLen >= MAX_SEGMENT_SIZE )
      {
         UtlError( TB_TOOLONG, MB_CANCEL, 0, NULL, EQF_ERROR);
      }
      else                // copy data and check if blanks to be padded
      {
       if ( pDoc->TBCursor.ulSegNum != pDoc->ulWorkSeg )
       {
          UtlError( TB_NOTINACTSEG, MB_CANCEL, 0, NULL, EQF_ERROR);
       }
       else if ( ! EQFBFuncCharIn( pDoc ))
       {
          EQFBFuncNothing( pDoc );               // beep
       }
       else
       {
          if ( pDoc->pUndoSegW )
          {                                      //update undo buffer
            UTF16strcpy(pDoc->pUndoSegW,pDoc->pEQFBWorkSegmentW);
            pDoc->usUndoSegOff = pDoc->TBCursor.usSegOffset;
            pDoc->fUndoState = FALSE;            // not a typing func
          } /* endif */
          /**********************************************************/
          /* delete the marked area in case of fNoCUASel...         */
          /**********************************************************/
          {
            PEQFBBLOCK  pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;        // pointer to block struct

            // redo blockmark in current segment at character input
            if ( pstBlock->pDoc == pDoc &&
                 (pstBlock->ulSegNum == pDoc->ulWorkSeg) )
            {
              EQFBFuncMarkDelete( pstBlock->pDoc );
              pstBlock->pDoc = NULL;                           // reset block mark
            } /* endif */
	      }
          if ( lPos < 0)
          {
             EQFBFuncPad ( pDoc, lPos);             // pad with blanks
          } /* endif */

          usSegOffOld = pDoc->TBCursor.usSegOffset;
          fEndOfSeg = pDoc->EQFBFlags.EndOfSeg;

          if (pDoc->fLineWrap & pDoc->fAutoLineWrap )
          {
             USHORT      usNewLen;
             /********************************************************/
             /* position temporarily cursor at begin of segment      */
             /********************************************************/
             EQFBGotoSeg(pDoc, pDoc->TBCursor.ulSegNum, 0);

             EQFBBufRemoveSoftLF( pDoc->hwndRichEdit, pDoc->pTBSeg->pDataW,
                                  &usNewLen, &usSegOffOld);
          } /* endif */

          lPos = usSegOffOld;
          sLen = (SHORT)UTF16strlenCHAR( pDoc->pEQFBWorkSegmentW );

          ulAbsLocat = lPos;
          if (pDoc->EQFBFlags.inserting || fEndOfSeg)
          {
              DISPSTYLE  DispStyle;
              USHORT     usSegOffset = 0;

              if ( fEndOfSeg)   //if triple mode,
              {                                // do not move cur. char
                 DispStyle = (pDoc->pTBSeg->SegFlags.Expanded) ?
                              DISP_PROTECTED : (pDoc->DispStyle);
                 usSegOffset = pDoc->TBCursor.usSegOffset;
                 if ( (DispStyle == DISP_COMPACT) &&
                  ((EQFBCharType(pDoc,pDoc->pTBSeg,usSegOffset))
                                    == COMPACT_CHAR) )
                 {
                    usType = HIDDEN_CHAR;
                    while (usType == HIDDEN_CHAR  )
                    {
                      usSegOffset++;
                      usType = EQFBCharType(pDoc,pDoc->pTBSeg, usSegOffset);
                    } /* endwhile */
                    usLocat = usSegOffset - pDoc->TBCursor.usSegOffset;
                 }
                 else
                 {
                   usLocat = 1;
                 } /* endif */
              } /* endif */

            ulAbsLocat = usSegOffOld + usLocat;
          }

          memmove( &pDoc->pEQFBWorkSegmentW[ulAbsLocat + lLength],
                   &pDoc->pEQFBWorkSegmentW[ulAbsLocat] ,
                   (sLen - ulAbsLocat + 1)*sizeof(CHAR_W));

          memcpy(  &pDoc->pEQFBWorkSegmentW[ulAbsLocat], pData, lLength*sizeof(CHAR_W));

          usSegOffOld = (USHORT)(usSegOffOld + lLength);

          EQFBUpdateChangedSeg(pDoc);                      // force update of seg info

          if (pDoc->fLineWrap & pDoc->fAutoLineWrap )
          {
             LONG      lEndCol;
             /********************************************************/
             /* no softlf's are in Data which is  inserted           */
             /********************************************************/
             EQFBBufAddSoftLF(pDoc, pDoc->pTBSeg,
                               pDoc->pTBSeg->pDataW,
                               pDoc->lCursorCol + pDoc->lSideScroll,
                               &lEndCol,
                               &usSegOffOld);
          } /* endif */
          EQFBUpdateChangedSeg(pDoc);            // force update of seg info

          pDoc->Redraw |= REDRAW_ALL;
          // reset any old block mark in same segment
          EQFBFuncResetMarkInSeg( pDoc );

          /********************************************************/
          /* reset cursor position                                */
          /********************************************************/
          EQFBGotoSeg(pDoc, pDoc->TBCursor.ulSegNum, usSegOffOld);
          pDoc->EQFBFlags.EndOfSeg = (USHORT)fEndOfSeg;
       } /* endif */
      } /* endif */
   } /* endif */

   return(fOK);
}
//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     EQFBFuncCopyToClip                                        
//------------------------------------------------------------------------------
// Function call:     EQFBFuncCopyToClip(PTBDOCUMENT)                           
//------------------------------------------------------------------------------
// Description:       copy to clipboard                                         
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc   pointer to document instance           
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     if block is marked                                        
//                       if enough space                                        
//                          - get marked area                                   
//                          - open clipboard                                    
//                          - empty clipboard                                   
//                          - move text to clipboard                            
//                          - close clipboard                                   
//                        else issue warning                                    
//                     else issue warning                                       
//                                                                              
//------------------------------------------------------------------------------

VOID  EQFBFuncCopyToClip
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
   ULONG ulBytes = 0;                        // number of bytes to copy
   PTBSEGMENT pSeg;                          // pointer to segment struct
   PEQFBBLOCK  pstBlock;                     // pointer to block struct
   USHORT usRc = 0;                          // return code of dos funct.
   PSZ_W  pchClipText = NULL;                // pointer to clipboard

   GLOBALHANDLE hGlobalMemory;

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

   if ( pDoc == pstBlock->pDoc )
   {
      if (pstBlock->ulSegNum == pstBlock->ulEndSegNum )
      {
         ulBytes = pstBlock->usEnd - pstBlock->usStart + 2;
      }
      else
      {
        ULONG    ulCurSeg;
        ulCurSeg = pstBlock->ulSegNum;
        pSeg = EQFBGetSegW( pstBlock->pDoc, ulCurSeg );
        if (pSeg )
        {
          ulBytes = pSeg->usLength - pstBlock->usStart + 1;
        } /* endif */
        ulCurSeg++;
        while (ulCurSeg < pstBlock->ulEndSegNum )
        {
          pSeg = EQFBGetSegW( pstBlock->pDoc, ulCurSeg );
          if (pSeg )
          {
            ulBytes += pSeg->usLength;
          } /* endif */
          ulCurSeg++;
        } /* endwhile */
        ulBytes += pstBlock->usEnd + 2;
      } /* endif */

      hGlobalMemory = GlobalAlloc (GHND, (DWORD) ulBytes*sizeof(CHAR_W));
      if ( hGlobalMemory )
      {
        pchClipText = (PSZ_W)GlobalLock (hGlobalMemory);
      }
      else
      {
        usRc = ERROR_STORAGE;
      } /* endif */

      if (!usRc )
      {
         pSeg = EQFBGetSegW( pstBlock->pDoc, pstBlock->ulSegNum );
         if (pstBlock->ulSegNum == pstBlock->ulEndSegNum )
         {
           memcpy( &pchClipText[0], &pSeg->pDataW[pstBlock->usStart], ulBytes * sizeof(CHAR_W) );
         }
         else
         {
           ULONG   ulCurSeg;
           PSZ_W   pCurClip;
           memcpy( &pchClipText[0], &pSeg->pDataW[pstBlock->usStart],
                   (pSeg->usLength - pstBlock->usStart + 1)*sizeof(CHAR_W));
           pCurClip = pchClipText + pSeg->usLength - pstBlock->usStart;
           ulCurSeg = pstBlock->ulSegNum + 1;
           while (ulCurSeg < pstBlock->ulEndSegNum )
           {
             pSeg = EQFBGetSegW(pstBlock->pDoc, ulCurSeg);
             if (pSeg )
             {
               memcpy(pCurClip, pSeg->pDataW, pSeg->usLength * sizeof(CHAR_W));
               pCurClip += pSeg->usLength;
             } /* endif */
             ulCurSeg++;
           } /* endwhile */
           pSeg = EQFBGetSegW(pstBlock->pDoc, ulCurSeg);
           if (pSeg )
           {
             memcpy(pCurClip, pSeg->pDataW, pstBlock->usEnd * sizeof(CHAR_W));
           } /* endif */
         } /* endif */

         pchClipText[ ulBytes - 1] = '\0';

         if (pDoc->fLineWrap && pDoc->fAutoLineWrap )
         {
           /**************************************************************/
           /* remove soft lf's in copy of data                           */
           /**************************************************************/
           USHORT   usBufSize = (USHORT) ulBytes;
           USHORT   usSegOffset = 0;
           EQFBBufRemoveSoftLF( pDoc->hwndRichEdit, pchClipText,
                                &usBufSize, &usSegOffset);
         } /* endif */

        GlobalUnlock(hGlobalMemory);

        OpenClipboard(pDoc->hwndClient);
        EmptyClipboard();

        SetClipboardData (CF_UNICODETEXT, hGlobalMemory);

        CloseClipboard();

      }
      else
      {
         UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR);
      } /* endif */
   }
   else
   {
      UtlError( TB_NOMARKACTIVE, MB_CANCEL, 0, NULL, EQF_WARNING);
   } /* endif */

   return;
}


//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     EQFBFuncCutToClip                                         
//------------------------------------------------------------------------------
// Function call:     EQFBFuncCutToClip(PTBDOCUMENT)                            
//------------------------------------------------------------------------------
// Description:       cut to clipboard                                          
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT                                               
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     if mark exists in this doc                                
//                      call EQFBFuncCopyToClip                                 
//                      if mark is in target:                                   
//                        call EQFBFuncMarkDelete                               
//                    else issue error message                                  
//------------------------------------------------------------------------------

VOID  EQFBFuncCutToClip
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{

   PEQFBBLOCK  pstBlock;                     // pointer to block struct

   pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

   if ( pDoc == pstBlock->pDoc )
   {
       EQFBFuncCopyToClip( pDoc );

       if ( pDoc->docType == STARGET_DOC )         // delete only in target doc
       {
          EQFBFuncMarkDelete( pDoc );              // delete area
          EQFBScrnLinesFromSeg ( pDoc,              // pointer to doc ida
                                 0,                 // starting row
                                 pDoc->lScrnRows,  // number of rows
                                                    // starting segment
                                 (pDoc->TBRowOffset+1));
       }
       else
       {
          EQFBFuncNothing ( pDoc );
       } /* endif */
   }
   else
   {
      UtlError( TB_NOMARKACTIVE, MB_CANCEL, 0, NULL, EQF_WARNING);
   } /* endif */

   return;
}

//------------------------------------------------------------------------------
// External function                                                            
//------------------------------------------------------------------------------
// Function name:     EQFBFuncPropMarkCopy                                      
//------------------------------------------------------------------------------
// Function call:     EQFBFuncPropMarkCopy (PTBDOCUMENT)                        
//------------------------------------------------------------------------------
// Description:       copy marked block from proposal window                    
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT pDoc pointer to document instance             
//------------------------------------------------------------------------------
// Returncode type:   VOID                                                      
//------------------------------------------------------------------------------
// Function flow:     -                                                         
//------------------------------------------------------------------------------
VOID  EQFBFuncPropMarkCopy
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
    PSZ_W   pStart;
    BOOL    fOK = TRUE;
    USHORT  usRc;

    fOK = UtlAlloc( (PVOID *)&pStart, 0L, MAX_PROPLENGTH * sizeof(CHAR_W) * 2, ERROR_STORAGE );
    if ( fOK )
    {
       usRc = EQFGETPROPW( EQF_GETMARKEDBLOCK, pStart, NULL );   // get marked data
    } /* endif */

    EQFBDoCopyData(pDoc, pStart);

    UtlAlloc( (PVOID *)&pStart, 0L, 0L, NOMSG );
    return;
}

GLOBALHANDLE  EQFBFuncCopyToHGlob
(
    PTBDOCUMENT  pDoc                              // pointer to document ida
)
{
   ULONG  ulBytes = 0L;                       // number of bytes to copy
   PTBSEGMENT pSeg;                          // pointer to segment struct
   PEQFBBLOCK  pstBlock;                     // pointer to block struct
   USHORT usRc = 0;                          // return code of dos funct.
   PSZ_W  pchClipText = NULL;                // pointer to clipboard

   GLOBALHANDLE hGlobalMemory = NULL;

   if ( pDoc )
   {
     pstBlock = (PEQFBBLOCK)pDoc->pBlockMark;

     if ( pDoc == pstBlock->pDoc )
     {
        if (pstBlock->ulSegNum == pstBlock->ulEndSegNum )
        {
           ulBytes = pstBlock->usEnd - pstBlock->usStart + 2;
        }
        else
        {
          ULONG    ulCurSeg;
          ulCurSeg = pstBlock->ulSegNum;
          pSeg = EQFBGetSegW( pstBlock->pDoc, ulCurSeg );
          if (pSeg )
          {
            ulBytes = pSeg->usLength - pstBlock->usStart + 1;
          } /* endif */
          ulCurSeg++;
          while (ulCurSeg < pstBlock->ulEndSegNum )
          {
            pSeg = EQFBGetSegW( pstBlock->pDoc, ulCurSeg );
            if (pSeg )
            {
              ulBytes += pSeg->usLength;
            } /* endif */
            ulCurSeg++;
          } /* endwhile */
          ulBytes += pstBlock->usEnd + 2;
        } /* endif */

        hGlobalMemory = GlobalAlloc (GHND, (DWORD) ulBytes * sizeof(CHAR_W));
        if ( hGlobalMemory )
        {
          pchClipText = (PSZ_W)GlobalLock (hGlobalMemory);
        }
        else
        {
          usRc = ERROR_STORAGE;
        } /* endif */

        if (!usRc )
        {
           pSeg = EQFBGetSegW( pstBlock->pDoc, pstBlock->ulSegNum );
           if (pstBlock->ulSegNum == pstBlock->ulEndSegNum )
           {
             memcpy( pchClipText, pSeg->pDataW + pstBlock->usStart, ulBytes * sizeof(CHAR_W) );
           }
           else
           {
             ULONG   ulCurSeg;
             PSZ_W   pCurClip;
             memcpy( pchClipText, pSeg->pDataW + pstBlock->usStart,
                     (pSeg->usLength - pstBlock->usStart + 1)*sizeof(CHAR_W));
             pCurClip = pchClipText + pSeg->usLength - pstBlock->usStart;
             ulCurSeg = pstBlock->ulSegNum + 1;
             while (ulCurSeg < pstBlock->ulEndSegNum )
             {
               pSeg = EQFBGetSegW(pstBlock->pDoc, ulCurSeg);
               if (pSeg )
               {
                 memcpy(pCurClip, pSeg->pDataW, pSeg->usLength*sizeof(CHAR_W));
                 pCurClip += pSeg->usLength;
               } /* endif */
               ulCurSeg++;
             } /* endwhile */
             pSeg = EQFBGetSegW(pstBlock->pDoc, ulCurSeg);
             if (pSeg )
             {
               memcpy(pCurClip, pSeg->pDataW, pstBlock->usEnd*sizeof(CHAR_W));
             } /* endif */
           } /* endif */

           pchClipText[ ulBytes - 1] = '\0';
           if (pDoc->fLineWrap && pDoc->fAutoLineWrap )
           {
             /**************************************************************/
             /* remove soft lf's in copy of data                           */
             /**************************************************************/
             USHORT   usBufSize = (USHORT)ulBytes;
             USHORT   usSegOffset = 0;
             EQFBBufRemoveSoftLF( pDoc->hwndRichEdit, pchClipText,
                                  &usBufSize, &usSegOffset);
           } /* endif */
           GlobalUnlock(hGlobalMemory);
        }
        else
        {
           UtlError( ERROR_STORAGE, MB_CANCEL, 0, NULL, EQF_ERROR);
        } /* endif */
        /**************************************************************/
        /* reset blockmark                                            */
        /**************************************************************/
        if ( pDoc->pBlockMark && (((PEQFBBLOCK)pDoc->pBlockMark)->pDoc != pDoc ))
        {
          memset( pDoc->pBlockMark, 0, sizeof( EQFBBLOCK ));
          pDoc->Redraw |= REDRAW_ALL;        // force redraw the screen
        } /* endif */
     }
     else
     {
        UtlError( TB_NOMARKACTIVE, MB_CANCEL, 0, NULL, EQF_WARNING);
     } /* endif */
   }
   else
   {
   } /* endif */

   return hGlobalMemory;
}
