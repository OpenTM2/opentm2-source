/*! \file
	Description: Translation memory handling

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_MORPH            // morph. functions
#include <eqf.h>                  // General Translation Manager include file
#include <eqftmi.h>

#include <eqftpi.h>               // private Translation Processor include file

#include <eqftai.h>               // private include file of Text Analysis
#include <eqfdoc00.h>
// #include <TIME.H>                 // C time functions

static  USHORT  TM_Error        (PDOCUMENT_IDA, USHORT);
static  USHORT  TlmOpen         (PDOCUMENT_IDA, PSZ);
static  USHORT  TlmReplace      (PDOCUMENT_IDA, PSTEQFSAB );
static  USHORT  TlmDelete       (PDOCUMENT_IDA, PSTEQFSAB );
static  USHORT  TlmGet          (PDOCUMENT_IDA, PSTEQFSAB );
static  USHORT  TlmExtract      (PDOCUMENT_IDA, PSTEQFSAB );
static  USHORT  TlmDeleteProp   (PDOCUMENT_IDA, PSTEQFSAB );
static  USHORT  TlmClose        (PDOCUMENT_IDA);



static BOOL  MemCalcFuzzyness(PTBDOCUMENT pTBDoc, PSTEQFSAB  pstEQFSab, PDOCUMENT_IDA  pDoc, PULONG pulMatches );

static int FuzzynessCompare(const void *p1, const void *p2 );
static VOID MemSortAccordingToFuzzyness(PSTEQFSAB, ULONG);

#define GOOD_FUZZY   70

/*------------------------------------------------------------------------------
* Globals
*-----------------------------------------------------------------------------*/
UCHAR           szIndustry[128] = "";           // temporary!!!         001mk
CHAR            chSourcePropSeg[ EQF_SEGLEN ];   // segment size

// static temporary buffer used for replacement matches in TlmGet (size is twice segment size)
static  CHAR_W szReplBuffer[EQF_SEGLEN+EQF_SEGLEN];


#include "OtmProposal.h"
#include "EditorMemLookupData.h"
#include "core\memory\MemoryFactory.h"
#include "vector"

static USHORT SetAvailFlags( std::vector<OtmProposal *>&Matches );
static  VOID  MemCheckMTProps(std::vector<OtmProposal *>&, BOOL, LONG);
static OtmProposal::eProposalType GetProposalTypeFromFlag( USHORT usMachineFlag );
static OtmProposal::eMatchType GetMatchType( USHORT usFuzziness );


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name: EQFTM
//------------------------------------------------------------------------------
// Function call:     EQFTM( PDOCUMENT_IDA, USHORT, PSTEQFSAB );
//------------------------------------------------------------------------------
// Description:       This function will distribute every call to the correct
//                    TM function
//
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA    pointer to document ida
//                    USHORT           request to be fulfilled
//                    PSTEQFSAB        send ahead buffer to be used
//------------------------------------------------------------------------------
// Returncode type:   VOID
//
//------------------------------------------------------------------------------
// Function flow:     switch on the following requests:
//                      case EQFCMD_INIT:
//                         close any already opened TM
//                         open TM and set return code
//                      case EQFCMD_CLEAR:
//                         set return code to NO_ERROR and do nothing else
//                      case EQFCMD_SAVESEG:
//                         call TlmReplace and set return code
//                      case EQFCMD_DELSEG:
//                         call TlmDelete and set return code if error
//                           happened (usRc  AND usRc != SEG_NOT_FOUND)
//                      case EQFCMD_EXTSEG:
//                         call TlmExtract to extract the segment
//                           and set return code
//                      case EQFCMD_TRANSSEG:
//                         call TlmGet and set return code
//                      case EQFCMD_CLOSE:
//                         call TlmClose; set return code and free memory
//                      default:
//                         invalid request; set return code
//                    endswitch
//------------------------------------------------------------------------------
VOID  EQFTM
(
    PDOCUMENT_IDA  pDoc,                  // pointer to document instance data
    USHORT usRequest,                     // request
    PSTEQFSAB pstQReq
)

{
  USHORT        usRc;
  PSTEQFGEN     pstEQFGen;                // pointer to generic struct

  pstEQFGen = pDoc->pstEQFGen;

  // request access to TM
  switch (usRequest)
  {
  /*--------------------------------------------------------------------------
  * Open a TM database
  *-------------------------------------------------------------------------*/
    case EQFCMD_INIT:
      {
         PTPMEMLOOKUPDATA pMemData = (PTPMEMLOOKUPDATA)pDoc->pvMemLookup;

         // allocate area when required
         if ( pMemData == NULL )
         {
           pMemData = new(TPMEMLOOKUPDATA);
           pDoc->pvMemLookup = (PVOID)pMemData;

           // prepare our proposal vectors
           for ( int i=0; i < EQF_NPROP_TGTS; i++ ) pMemData->FoundProposals.push_back( new(OtmProposal) );
           for ( int i=0; i < EQF_NPROP_TGTS; i++ ) pMemData->BestProposals.push_back( new(OtmProposal) );

         } /* endif */          

         // add dummy entry to list when list is empty
         if ( pMemData->MemoryList.size() == 0 )
         {
           pMemData->MemoryList.push_back( NULL );
         } /* endif */

         // close any open document TM first
         if ( pMemData->MemoryList[0] != NULL )
         {
           MemoryFactory *pFactory = MemoryFactory::getInstance();
           pFactory->closeMemory( pMemData->MemoryList[0] );            
           pMemData->MemoryList[0] = NULL;
         } /* endif */

         if ( pMemData->MemoryList[0] == NULL )
         {
           usRc = TlmOpen (pDoc, pDoc->szMemory[0]);
           if (usRc == NO_ERROR)
           {
             pDoc->fRunTM = TRUE;
           }
           else if (usRc == ERROR_MSG_HANDLED)     // error already checked for
           {
             pstEQFGen->usRC = ERROR_MSG_HANDLED;
           }
           else if (usRc < TMERR_BASE)        // system error
           {
             CHAR chRC[10];
             pstEQFGen->usRC = EQFRS_SYSTEM_ERR;
             itoa (usRc, chRC, 10);
             ASCII2Unicode( chRC, pstEQFGen->szMsgBuffer, 0L );
           }
           else
           {
             pstEQFGen->usRC = TM_Error( pDoc, usRc );
           } // end if
         }
         else
         {
           pstEQFGen->usRC    = EQFRS_ALREADY_INIT;
         } // end if (check initialization)
       }
       break;

  /*--------------------------------------------------------------------------
  * CLEAR is not used for TM.
  *-------------------------------------------------------------------------*/
     case EQFCMD_CLEAR:
       usRc = NO_ERROR;
       break;

  /*--------------------------------------------------------------------------
  * Store/Replace a segment in the translation memory.
  *-------------------------------------------------------------------------*/
     case EQFCMD_SAVESEGW:
     case EQFCMD_SAVESEG2W:

       usRc = TlmReplace ( pDoc, pstQReq );
       if (usRc)
       {
         pstEQFGen->usRC = TM_Error ( pDoc, usRc );
       }
       break;


  /*--------------------------------------------------------------------------
  * Delete a segment in the translation memory.
  *-------------------------------------------------------------------------*/
     case EQFCMD_DELSEGW:
       usRc = TlmDelete (pDoc, pstQReq );
       if (usRc && usRc != SEG_NOT_FOUND)
       {
         pstEQFGen->usRC = TM_Error ( pDoc, usRc );
       } /* endif */
       break;


  /*--------------------------------------------------------------------------
  * Delete a proposal in the translation memory.
  *-------------------------------------------------------------------------*/
     case EQFCMD_DELPROP:
       usRc = 0;
       usRc = TlmDeleteProp ( pDoc, pstQReq );
       if (usRc && usRc != SEG_NOT_FOUND)
       {
         pstEQFGen->usRC = TM_Error ( pDoc, usRc );
       } /* endif */
       break;

  /*--------------------------------------------------------------------------
  * Extract a segment from the translation memory.
  *-------------------------------------------------------------------------*/
     case EQFCMD_EXTSEGW:
       usRc = TlmExtract ( pDoc, pstQReq );
       if (usRc && usRc != SEG_NOT_FOUND )
       {
         pstEQFGen->usRC  = TM_Error ( pDoc, usRc );
       }
       break;

  /*--------------------------------------------------------------------------
  * Translate a segment.
  *-------------------------------------------------------------------------*/
     case EQFCMD_TRANSSEGW:
       usRc = TlmGet (pDoc, pstQReq);
       if ( usRc  )
       {
         pstEQFGen->usRC = TM_Error ( pDoc, usRc );
       } // endif (no error from TlmGet)
       break;


  /*--------------------------------------------------------------------------
  * Close TM database.
  *-------------------------------------------------------------------------*/
     case EQFCMD_CLOSE:
       usRc = TlmClose ( pDoc );
       if (usRc == NO_ERROR)
       {
         PTPMEMLOOKUPDATA pMemData = (PTPMEMLOOKUPDATA)pDoc->pvMemLookup;
         pDoc->fRunTM = FALSE;

         // free OtmProposals
         if ( pMemData != NULL )
         {
           for ( int i = 0; i < (int)pMemData->BestProposals.size(); i++ ) delete( pMemData->BestProposals[i] ); 
           for ( int i = 0; i < (int)pMemData->FoundProposals.size(); i++ ) delete( pMemData->FoundProposals[i] ); 
           delete( pMemData );
         }

         // free our area
         pDoc->pvMemLookup = NULL;

       }
       else
       {
         pstEQFGen->usRC = TM_Error (pDoc, usRc );
       } // end if
       break;

  /*--------------------------------------------------------------------------
  * everything else is invalid
  *-------------------------------------------------------------------------*/
     case EQFCMD_EXTSEG:
     case EQFCMD_DELSEG:
     case EQFCMD_TRANSSEG:
     case EQFCMD_SAVESEG:
     default:
       {
     CHAR chRC[10];
         pstEQFGen->usRC = EQFRS_INVALID_CMD;
         itoa (pstEQFGen->usRC, chRC, 10);
         ASCII2Unicode( chRC, pstEQFGen->szMsgBuffer, 0L );
     }
       break;
  } // end switch (req.code)

  return;
} // end 'EQFTM'

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TlmOpen
//------------------------------------------------------------------------------
// Function call:     TlmOpen ( PDOCUMENT_IDA, PSZ );
//------------------------------------------------------------------------------
// Description:       This function will open the specified translation
//                    memory and allocate the necessary structures.
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA        pointer to document ida
//                    PSZ                  file name
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       ERROR_MSG_HANDLED    error message already handled
//                    NO_ERROR             everything went okay
//------------------------------------------------------------------------------
// Side effects:      Specified Translation Memory will be opened
//                    In case of Organize necessary the active window for
//                    return will be reset.
//------------------------------------------------------------------------------
// Function flow:     Allocate the buffers necessary for Input and Output,
//                    if not okay then
//                      set usRc to ERROR_MSG_HANDLED
//                    endif
//                    if okay so far
//                       open TM in NONEXCLUSIVE mode and allow for error handl
//                       if error == TM_WAS_CORRUPTED_IS_ORGANIZED
//                         reset active window handle to have slider in front
//                       endif
//                       set usRc to ERROR_MSG_HANDLED
//                    endif
//                    return
//------------------------------------------------------------------------------
static
USHORT  TlmOpen
(
   PDOCUMENT_IDA  pDoc,
   PSZ pszTMFile
)
{
  USHORT        usRc = NO_ERROR;
  MemoryFactory *pFactory = MemoryFactory::getInstance();
  PTPMEMLOOKUPDATA pMemData = (PTPMEMLOOKUPDATA)pDoc->pvMemLookup;

  // allocate OtmProposals for the memory lookup
  for ( int i = 0; i < EQF_NPROP_TGTS; i++ )
  {
    pMemData->BestProposals.push_back( new OtmProposal );
    pMemData->FoundProposals.push_back( new OtmProposal );
  } /* endfor */     


  if (usRc == NO_ERROR )
  {
    int iRC = 0;
    OtmMemory *pMem = pFactory->openMemory( NULL, pszTMFile, NONEXCLUSIVE, &iRC );
    if ( iRC != 0 ) 
    {
      pFactory->showLastError( NULL, pszTMFile, NULL, NULLHANDLE );
    }
    else
    {
      pMem->setGlobalMemoryOptions( pDoc->pvGlobalMemOptFile );
      pMemData->MemoryList[0] = pMem;
    }
    usRc = (USHORT)iRC;
  }

  if ( !usRc )
  {
    /******************************************************************/
    /* try to open list of R/O memories                               */
    /******************************************************************/
    USHORT usI = 1;
    while ((usRc== NO_ERROR) && (usI < EQF_MAX_TM_BASES) && pDoc->szMemory[usI][0])
    {
      int iRC = 0;
      OtmMemory *pMem = pFactory->openMemory( NULL, pDoc->szMemory[usI], NONEXCLUSIVE, &iRC );
      if ( iRC != 0 )
      {
        pFactory->showLastError( NULL, pszTMFile, NULL, NULLHANDLE );
        usRc = (USHORT)iRC;
      }
      else
      {
        pMem->setGlobalMemoryOptions( pDoc->pvGlobalMemOptFile );
        pMemData->MemoryList.push_back( pMem );
      } /* endif */         
      usI++;
    } /* endwhile */
  } /* endif */

  return usRc;
} // end 'TlmOpen'
/* $PAGEIF20 */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TlmReplace
//------------------------------------------------------------------------------
// Function call:     TlmReplace( PDOCUMENT_IDA, PSTEQFSAB );
//------------------------------------------------------------------------------
// Description:       replace the segment in the translation memory
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA        pointer to document ida
//                    PSTEQFSAB            send ahead buffer with segment
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       ...                  error message from TM
//                    NO_ERROR             everything went okay
//------------------------------------------------------------------------------
// Function flow:     setup prefix for IN
//                    setup segment
//                    call TmReplace for doing the Replace call; set return cod
//                    return returncode
//------------------------------------------------------------------------------
static
USHORT  TlmReplace
(
   PDOCUMENT_IDA    pDoc,
   PSTEQFSAB pstQReq
)
{
  USHORT usRc;
  PTWBSDEVICE    pTBDevice;                     // pointer to device structure
  PTBDOCUMENT    pTBDoc;                        // pointer to document
  PTPMEMLOOKUPDATA pMemData = (PTPMEMLOOKUPDATA)pDoc->pvMemLookup;
  HWND           hwnd = NULL;
  PTBDOCUMENT    pSTARGETDoc = NULL;

  pMemData->SearchKey.clear();

  pTBDevice = &(pDoc->tbDevProposal);
  pTBDoc = &(pTBDevice->tbDoc);

  hwnd = ((PSTEQFGEN)(pTBDoc->pstEQFGen))->hwndEditorTgt;
  if (hwnd )
  {
    pSTARGETDoc = ACCESSWNDIDA(hwnd, PTBDOCUMENT);
    if (pSTARGETDoc &&
                pSTARGETDoc->fLineWrap && pSTARGETDoc->fAutoLineWrap )
    {
      USHORT  i;
      USHORT  usSegOffset = 0;
      EQFBBufRemoveSoftLF(pTBDoc->hwndRichEdit, pstQReq->pucTargetSegs, &i, &usSegOffset);
      usSegOffset = 0;
      EQFBBufRemoveSoftLF(pTBDoc->hwndRichEdit, pstQReq->pucSourceSeg, &i, &usSegOffset);
    } /* endif */

    // remove any softline feeds contained in the target segment data
    {
      PSZ_W pIn = pstQReq->pucTargetSegs;
      PSZ_W pOut = pstQReq->pucTargetSegs;
      while ( *pIn != 0 )
      {
        if ( *pIn == SOFTLF_CHAR )
        {
          pIn++;
        }
        else
        {
          *pOut++ = *pIn++;
        } /* endif */
      } /*endwhile */
      *pOut = 0;
    }
  } /* endif */


  if (pDoc->ulSrcOemCP == 0)
  {
    pDoc->ulSrcOemCP = GetLangOEMCP( pDoc->szDocSourceLang );
  }
  if (pDoc->ulTgtOemCP == 0)
  {
    pDoc->ulTgtOemCP = GetLangOEMCP( pDoc->szDocTargetLang );
  }

  EQFBBufRemoveTRNote(pstQReq->pucSourceSeg, pTBDoc->pDocTagTable,
                        pTBDoc->pfnUserExit, pTBDoc->pfnUserExitW,
                        pDoc->ulSrcOemCP);
  EQFBBufRemoveTRNote(pstQReq->pucTargetSegs, pTBDoc->pDocTagTable,
                        pTBDoc->pfnUserExit, pTBDoc->pfnUserExitW,
                        pDoc->ulTgtOemCP );

  if ( IsDBCS_CP(pDoc->ulTgtOemCP) || IsDBCS_CP(pDoc->ulSrcOemCP) )
  {
    ULONG i;
    i = UTF16strlenCHAR( pstQReq->pucSourceSeg ) + 1;
    EQFBUtlConvertSOSI( pMemData->szSource, &i, pstQReq->pucSourceSeg, DELETE_SOSI, pDoc->ulSrcOemCP );

    i = UTF16strlenCHAR( pstQReq->pucTargetSegs ) + 1;
    EQFBUtlConvertSOSI( pMemData->szTarget, &i, pstQReq->pucTargetSegs, DELETE_SOSI, pDoc->ulTgtOemCP );

  }
  else
  {
      UTF16strcpy( pMemData->szSource, pstQReq->pucSourceSeg );
      UTF16strcpy( pMemData->szTarget, pstQReq->pucTargetSegs);
  } /* endif */
  pMemData->SearchKey.setSource( pMemData->szSource );
  pMemData->SearchKey.setTarget( pMemData->szTarget );

  if ( pDoc->szShortAlias[0] != EOS )
  {
    pMemData->SearchKey.setDocShortName( pDoc->szShortAlias );
    pMemData->SearchKey.setDocName( pDoc->szAlias );
  }
  else
  {
    pMemData->SearchKey.setDocShortName( pDoc->szDocName );
    pMemData->SearchKey.setDocName( pDoc->szDocLongName );
  } /* endif */

  pMemData->SearchKey.setType( OtmProposal::eptManual );
//  TIME      lTime;                             //time stamp

  pMemData->SearchKey.setSegmentNum( pstQReq->ulParm1 );
  pMemData->SearchKey.setMarkup( pDoc->szDocFormat );
  pMemData->SearchKey.setSourceLanguage( pDoc->szDocSourceLang );
  pMemData->SearchKey.setTargetLanguage( pDoc->szDocTargetLang );

  // supply any segment context info
  if ( pstQReq->pszContext )
  {
    pMemData->SearchKey.setContext( pstQReq->pszContext );
  } /* endif */
  if ( pstQReq->pszAddData )
  {
    pMemData->SearchKey.setAddInfo( pstQReq->pszAddData );
  } /* endif */

  // let EQFCHECKSEGTYPE function of the user exit adjust the memory type
  if ( pSTARGETDoc && pSTARGETDoc->pfnCheckSegType )
  {
     PTBSEGMENT pTBPrevSourceSeg = EQFBGetSegW( pSTARGETDoc->twin, pstQReq->ulParm1 - 1 );
     EQF_BOOL  fChanged = FALSE;
     PSZ_W     pPrevSegData = NULL;
     SHORT     sMemType = CST_MANUAL_MATCH;
     SHORT     sCountType = CST_MANUAL_MATCH;
     
     pPrevSegData = ( pTBPrevSourceSeg ) ? pTBPrevSourceSeg->pDataW : NULL;
     fChanged = (pSTARGETDoc->pfnCheckSegType)( pDoc->szDocFormat, pPrevSegData, pMemData->szSource,
                                                pMemData->szTarget, (LONG)pSTARGETDoc, pstQReq->ulParm1, &sCountType, &sMemType );
     if ( fChanged )
     {
       switch( sMemType )
       {
         case CST_MACHINE_MATCH: pMemData->SearchKey.setType( OtmProposal::eptMachine ); break;
         case CST_MANUAL_MATCH: pMemData->SearchKey.setType( OtmProposal::eptManual ); break;
         case CST_GLOBMEM_MATCH: pMemData->SearchKey.setType( OtmProposal::eptGlobalMemory ); break;
         default: pMemData->SearchKey.setType( OtmProposal::eptManual ); break;
       }
     } /* endif */
  } /* endif */

  usRc = (USHORT)pMemData->MemoryList[0]->putProposal( pMemData->SearchKey );

  return usRc;
} // end 'TlmReplace'

/* $PAGEIF20 */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TlmDelete
//------------------------------------------------------------------------------
// Function call:     TlmDelete ( PDOCUMENT_IDA, PSTEQFSAB );
//------------------------------------------------------------------------------
// Description:       delete  the segment in the translation memory
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA        pointer to document ida
//                    PSTEQFSAB            send ahead buffer with segment
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       ...                  error message from TM
//                    NO_ERROR             everything went okay
//------------------------------------------------------------------------------
// Function flow:     setup prefix for IN
//                    setup segment
//                    call TmDelete for doing the delete call; set return code
//                    return returncode
//------------------------------------------------------------------------------
static
USHORT  TlmDelete
(
   PDOCUMENT_IDA    pDoc,
   PSTEQFSAB        pstQReq
)
{
  USHORT usRc;
  PTWBSDEVICE      pTBDevice = &(pDoc->tbDevProposal);
  PTBDOCUMENT      pTBDoc = &(pTBDevice->tbDoc);
  PTPMEMLOOKUPDATA pMemData = (PTPMEMLOOKUPDATA)pDoc->pvMemLookup;

  pMemData->SearchKey.clear();

  // Remove TRNotes and SoftLinefeeds
  if (pTBDoc && pTBDoc->fLineWrap && pTBDoc->fAutoLineWrap )
  {
    USHORT  i;
    USHORT  usSegOffset = 0;
    EQFBBufRemoveSoftLF(pTBDoc->hwndRichEdit, pstQReq->pucTargetSegs, &i, &usSegOffset);
    usSegOffset = 0;
    EQFBBufRemoveSoftLF(pTBDoc->hwndRichEdit, pstQReq->pucSourceSeg, &i, &usSegOffset);
  } /* endif */
  if (pDoc->ulSrcOemCP == 0)
  {
      pDoc->ulSrcOemCP = GetLangOEMCP( pDoc->szDocSourceLang );
  }
  if (pDoc->ulTgtOemCP == 0)
  {
      pDoc->ulTgtOemCP = GetLangOEMCP( pDoc->szDocTargetLang );
  }
  EQFBBufRemoveTRNote(pstQReq->pucSourceSeg, pTBDoc->pDocTagTable,
                      pTBDoc->pfnUserExit, pTBDoc->pfnUserExitW,
                      pDoc->ulSrcOemCP );
  EQFBBufRemoveTRNote(pstQReq->pucTargetSegs, pTBDoc->pDocTagTable,
                       pTBDoc->pfnUserExit, pTBDoc->pfnUserExitW,
                       pDoc->ulTgtOemCP );

  if ( IsDBCS_CP(pDoc->ulTgtOemCP) || IsDBCS_CP(pDoc->ulSrcOemCP) )
  {
    ULONG i;
    i = UTF16strlenCHAR( pstQReq->pucSourceSeg ) + 1;
    EQFBUtlConvertSOSI( pMemData->szSource, &i, pstQReq->pucSourceSeg, DELETE_SOSI, pDoc->ulSrcOemCP );

    i = UTF16strlenCHAR( pstQReq->pucTargetSegs ) + 1;
    EQFBUtlConvertSOSI( pMemData->szTarget, &i, pstQReq->pucTargetSegs, DELETE_SOSI, pDoc->ulTgtOemCP );

  }
  else
  {
      UTF16strcpy( pMemData->szSource, pstQReq->pucSourceSeg );
      UTF16strcpy( pMemData->szTarget, pstQReq->pucTargetSegs);
  } /* endif */
  pMemData->SearchKey.setSource( pMemData->szSource );
  pMemData->SearchKey.setTarget( pMemData->szTarget );

  if ( pDoc->szShortAlias[0] != EOS )
  {
    pMemData->SearchKey.setDocShortName( pDoc->szShortAlias );
    pMemData->SearchKey.setDocName( pDoc->szAlias );
  }
  else
  {
    pMemData->SearchKey.setDocShortName( pDoc->szDocName );
    pMemData->SearchKey.setDocName( pDoc->szDocLongName );
  } /* endif */

  pMemData->SearchKey.setType( OtmProposal::eptManual );

  pMemData->SearchKey.setSegmentNum( pstQReq->ulParm1 );
  pMemData->SearchKey.setMarkup( pDoc->szDocFormat );
  pMemData->SearchKey.setSourceLanguage( pDoc->szDocSourceLang );
  pMemData->SearchKey.setTargetLanguage( pDoc->szDocTargetLang );

  usRc = (USHORT)pMemData->MemoryList[0]->deleteProposal( pMemData->SearchKey );

  return usRc;
} // end 'TlmDelete'

/* $PAGEIF20 */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TlmGet
//------------------------------------------------------------------------------
// Function call:     TlmGet    ( PDOCUMENT_IDA, PSTEQFSAB );
//------------------------------------------------------------------------------
// Description:       get the specified segment from the translation memory
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA        pointer to document ida
//                    PSTEQFSAB            send ahead buffer with segment
//                    USHORT               usRequest (ASCII/UNICODE)
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       ...                  error message from TM
//                    NO_ERROR             everything went okay
//------------------------------------------------------------------------------
// Function flow:     setup prefix for IN
//                    setup segment
//                    call TmGet for doing the get; set return code
//                    if okay then
//                      copy all found matches with levels to SAB
//                       ( do not copy duplicates )
//                    endif
//                    return returncode
//------------------------------------------------------------------------------
static
USHORT  TlmGet
(
   PDOCUMENT_IDA    pDoc,
   PSTEQFSAB        pstReq
)
{
  USHORT usRc = 0;
  BOOL   fContinue;
  ULONG         i, k,
                ulNumMatches,
                ulNumMemMatches,      // matches form the memory
                ulLenProp, ulLen;
  ULONG         ulDisp = 0;
  ULONG         ulPropDisp = 0;       // position of source of proposal
  PSZ_W         pszProp, pszBuffer;
  BOOL          fMTPending = pDoc->fRunMT;            // set depend. on MT
  PSTEQFGEN     pstEQFGen;             // pointer to generic struct
  PTBDOCUMENT   pTBDoc;                // pointer to document structure
  PTWBSDEVICE   pDevice;               // pointer to device structure
  USHORT        usPrepFuzzyProp;       // return code of PrepareFuzzyProp funct.
  PTBDOCUMENT    pSTARGETDoc;
  HWND           hwnd = NULL;
  ULONG          ulCP = 0L;
  BOOL           fDone = FALSE;
  PTPMEMLOOKUPDATA pMemData = (PTPMEMLOOKUPDATA)pDoc->pvMemLookup;
  MemoryFactory *pFactory = MemoryFactory::getInstance();
  int iOptions = 0;

  pstEQFGen = pDoc->pstEQFGen;
  pDevice = &(pDoc->tbDevProposal);    // device contexts for proposal
  pTBDoc = &(pDevice->tbDoc);          // pointer to document structure

  pMemData->SearchKey.clear();

  if (pDoc->ulSrcOemCP == 0)
  {
      pDoc->ulSrcOemCP = GetLangOEMCP( pDoc->szDocSourceLang );
  }
  if (pDoc->ulTgtOemCP == 0)
  {
      pDoc->ulTgtOemCP = GetLangOEMCP( pDoc->szDocTargetLang );
  }

  if ( pstReq->pszContext ) pMemData->SearchKey.setContext( pstReq->pszContext );

  if ( pDoc->fsConfig & EQFF_EXACTANDFUZZY )
  {
    iOptions = GET_EXACT_AND_FUZZY;
  }
  else
  {
    iOptions = GET_EXACT;
  } /* endif */
  if ( pDoc->fsConfig & EQFF_EXACTCONTEXT )
  {
    iOptions |= GET_EXACT_AND_CONTEXT;
  }
  else if ( pDoc->fsConfig & EQFF_ALLEXACTONES )
  {
    iOptions |= GET_ALL_EXACT_MATCHES;
  } /* endif */

   // get no generic place flag from system properties
   {
     PPROPSYSTEM pSysProp;             // ptr to EQF system properties
     pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
     if ( pSysProp->fNoGenericMarkup )
     {
       iOptions |= GET_NO_GENERICREPLACE;
     } /* endif */
   }

  hwnd = ((PSTEQFGEN)(pTBDoc->pstEQFGen))->hwndEditorTgt;
  if (hwnd )
  {
    pSTARGETDoc = ACCESSWNDIDA(hwnd, PTBDOCUMENT);
    if (pSTARGETDoc && pSTARGETDoc->fLineWrap && pSTARGETDoc->fAutoLineWrap )
    {
      USHORT  i;
      USHORT  usSegOffset = 0;
      EQFBBufRemoveSoftLF(pTBDoc->hwndRichEdit, pstReq->pucSourceSeg, &i, &usSegOffset);
      ulCP = pSTARGETDoc->twin->ulOemCodePage;
    } /* endif */
  } /* endif */

   /*******************************************************************/
   /* remove TRNote before requesting proposal from memory            */
   /*******************************************************************/
   EQFBBufRemoveTRNote(pstReq->pucSourceSeg, pTBDoc->pDocTagTable,
                         pTBDoc->pfnUserExit, pTBDoc->pfnUserExitW, pDoc->ulSrcOemCP );

  // reduce CRLF combinations into LF before calling TM
  usConvertCRLFW(pstReq->pucSourceSeg,              // pointer to input area
                 (USHORT)(UTF16strlenCHAR(pstReq->pucSourceSeg) + 1),  // length of input area
                 pMemData->szSource,       // pointer to output area
                 TRUE );

  wcscpy( pMemData->szDocSource, pMemData->szSource ); // remember original source text, szSource is later overwritten with the target of following proposals

  pMemData->SearchKey.setSource( pMemData->szSource );

  if ( pDoc->szShortAlias[0] != EOS )
  {
    pMemData->SearchKey.setDocShortName( pDoc->szShortAlias );
    pMemData->SearchKey.setDocName( pDoc->szAlias );
  }
  else
  {
    pMemData->SearchKey.setDocShortName( pDoc->szDocName );
    pMemData->SearchKey.setDocName( pDoc->szDocLongName );
  } /* endif */


  // GQTODO pstGetIn->stTmGet.usConvert   = MEM_OUTPUT_LF;
  pDoc->ulLastSegNum = pstReq->ulParm1;
  pMemData->SearchKey.setSegmentNum( pDoc->ulLastSegNum );
  pMemData->SearchKey.setMarkup( pDoc->szDocFormat );
  pMemData->SearchKey.setSourceLanguage( pDoc->szDocSourceLang );
  pMemData->SearchKey.setTargetLanguage( pDoc->szDocTargetLang );
  // pstGetIn->stTmGet.ulSrcOemCP = pDoc->ulSrcOemCP;
  // pstGetIn->stTmGet.ulTgtOemCP = pDoc->ulTgtOemCP;

  i = 0;
  fDone = FALSE;
  OtmProposal::clearAllProposals( pMemData->BestProposals );
  while ( !usRc && (i < pMemData->MemoryList.size()) && (pMemData->MemoryList[i] != NULL) && !fDone )
  {
    OtmProposal::clearAllProposals( pMemData->FoundProposals );
    usRc = (USHORT)pMemData->MemoryList[i]->searchProposal( pMemData->SearchKey, pMemData->FoundProposals, iOptions );
    if ( !usRc )
    {
      int iNumOfProposals = OtmProposal::getNumOfProposals( pMemData->FoundProposals );
      for ( int iProposal = 0; iProposal < iNumOfProposals; iProposal++ ) pMemData->FoundProposals[iProposal]->setMemoryIndex( i ); 
    } /* endif */       

    if ( !usRc )
    {
      USHORT j = EQF_NPROP_TGTS;
      if ( !(pDoc->fsConfig & EQFF_ALLEXACTONES ))
      {
	    	 j = EQF_NPROP_NOTALL_TGTS;
  	  } /* endif */

      pFactory->copyBestMatches( pMemData->FoundProposals, pMemData->BestProposals, j, -1, pDoc->fsConfig & EQFF_EXACTANDFUZZY  );

      i++;
    } /* endif */

    if ( pDoc->fStopAtFirstExact )
    {
      if ( pMemData->BestProposals[0]->isExactMatch() )
      {
        fDone = TRUE;
      } /* endif */
    } /* endif */
  } /* endwhile */


  /*----------------------------------------------------------------------------
  * Since TM is only concerned by foreground requests, PropCount may be cleared
  * in any case.
  *---------------------------------------------------------------------------*/
  pstReq->usPropCount = 0;

  if (usRc == NO_ERROR)
  {
    ulDisp = ulNumMatches = ulNumMemMatches = 0;
    USHORT usNumMatchesFound = (USHORT)OtmProposal::getNumOfProposals( pMemData->BestProposals );

    /******************************************************************/
    /* check if we are concerned with automatic translation           */
    /* in such a case we do only want to get one exact one....        */
    /*  if there is more than one available, reset the number of      */
    /*  matches found to 0                                            */
    /******************************************************************/
    if ( (usNumMatchesFound >= 1) && (pDoc->fsConfig & EQFF_ONLYONEEXACT) )
    {
      if ( !pFactory->isAutoSubstCandidate( pMemData->BestProposals ) )
      {
        usNumMatchesFound = 0;
      } /* endif */
    } /* endif */

    // insert any XLIFF proposals first
    //if ( pstReq->pvMetaData )
    {
      static MD_PROPOSAL MDProposal;

      BOOL fProposalAvailable = MDGetFirstProposal( pstReq->pvMetaData, &MDProposal );
      while ( fProposalAvailable )
      {
        pstReq->usLevel[ulNumMatches]         = MDProposal.sQuality ;
        pstReq->fEqualSource[ulNumMatches]    = TRUE; // ????
        strcpy( pstReq->szFName[ulNumMatches], MDProposal.szDocName );
        pstReq->lDate[ulNumMatches]           = MDProposal.lDate;
        pstReq->usDBIndex[ulNumMatches]       = 0;
        pstReq->ulKey[ulNumMatches]           = 0;
        pstReq->usTargetNum[ulNumMatches]     = 0;
        pstReq->ulSegNum[ulNumMatches]        = MDProposal.ulSegmentId;
        pstReq->usFuzzyPercents[ulNumMatches] = MDProposal.sQuality;

        pstReq->pszSortTargetSeg[ulNumMatches] = pstReq->pucTargetSegs + ulDisp;
        pstReq->pszSortPropsSeg[ulNumMatches] = pstReq->pucPropsSegs + ulPropDisp;

        pstReq->usMachineTrans[ulNumMatches] = XLIFF_PROP;
        if ( MDProposal.sQuality < 100 )
        {
          pstReq->usMachineTrans[ulNumMatches] |= FUZZY_PROP;
        }
        else
        {
          pstReq->usMachineTrans[ulNumMatches] |= EXACT_PROP;
        } /* endif */
        UTF16strcpy (&pstReq->pucTargetSegs[ulDisp], MDProposal.szTarget );
        ulDisp += UTF16strlenCHAR( MDProposal.szTarget) + 1;;
        UTF16strcpy( &(pstReq->pucPropsSegs[ulPropDisp]), MDProposal.szSource );
        ulPropDisp += UTF16strlenCHAR( MDProposal.szSource) + 1;
        ulNumMatches++;

        fProposalAvailable = MDGetNextProposal( &MDProposal );
      } /*endwhile */
    }

    if ( usNumMatchesFound)
    {
      pstReq->fsAvailFlags = SetAvailFlags( pMemData->BestProposals );

      /****************************************************************/
      /* one match found  ...                                         */
      /****************************************************************/
      if ( pDoc->fRunMT )
      {
        fMTPending = ! pDoc->fNoMTProp;
      } /* endif */

      /**************************************************************/
      /* do not displ. MT prop if a good fuzzy is available         */
      /**************************************************************/
      MemCheckMTProps(pMemData->BestProposals, pTBDoc->pUserSettings->UserOptFlags.bDispMTAlways, pstEQFGen->lExactMatchLevel );

      /*------------------------------------------------------------------------
      * Copy all found matches (w/levels) to SAB
      *-----------------------------------------------------------------------*/
      USHORT usNumMatchesFound = (USHORT)OtmProposal::getNumOfProposals( pMemData->BestProposals );
      for (i = 0; i < usNumMatchesFound; i++)
      {
        fContinue = FALSE;

        // we use the szTarget buffer for the target string of the current proposal and
        // the szSource buffer for the target string of the propsals already in our list
        ulLenProp = pMemData->BestProposals[i]->getTarget( pMemData->szTarget, MAX_SEGMENT_SIZE );
        pszProp = pMemData->szTarget;


        // skip proposals below the current display threshhold value
        {
          USHORT usDisplayLevel = 0;       // display level value to be used for the compare

          ULONG ulTest = UtlQueryULong( QL_LARGEFUZZLEVEL );
          ulTest = UtlQueryULong( QL_MEDIUMFUZZLEVEL );
          ulTest = UtlQueryULong( QL_SMALLFUZZLEVEL );

          if ( (UtlQueryULong( QL_LARGEFUZZLEVEL ) == UtlQueryULong( QL_MEDIUMFUZZLEVEL ) ) &&
               (UtlQueryULong( QL_LARGEFUZZLEVEL ) == UtlQueryULong( QL_SMALLFUZZLEVEL ) ))
          {
            // all display level values are identical, so we do not need to count the words of the proposal
            usDisplayLevel = (USHORT)(UtlQueryULong( QL_LARGEFUZZLEVEL ) / 100);
          }
          else
          {
            // the get number of words in the segment
            ULONG ulSrcWords = 0;
            ULONG ulSrcMarkUp = 0;
            PLOADEDTABLE pTable = NULL;
            
            pMemData->BestProposals[i]->getMarkup( pMemData->szNameBuffer, sizeof(pMemData->szNameBuffer) );

            TALoadTagTable( pMemData->szNameBuffer, (PLOADEDTABLE *) &pTable, FALSE, FALSE );

            if ( pTable != NULL ) EQFBWordCntPerSeg( pTable, (PTOKENENTRY)pTBDoc->pTokBuf, pMemData->szDocSource, pDoc->sSrcLanguage, &ulSrcWords, &ulSrcMarkUp, pTBDoc->ulOemCodePage);

            if ( pTable != NULL ) TAFreeTagTable( pTable );
            
	          if ( ulSrcWords > 15 )
	          {
		          usDisplayLevel = (USHORT)(UtlQueryULong( QL_LARGEFUZZLEVEL ) / 100);
	          }
	          else if ( ulSrcWords > 4 )
	          {
		          usDisplayLevel = (USHORT)(UtlQueryULong( QL_MEDIUMFUZZLEVEL ) / 100);
	          }
	          else
	          {
		          usDisplayLevel = (USHORT)(UtlQueryULong( QL_SMALLFUZZLEVEL ) / 100);
	          } /* endif */
          } /* endif */             

          fContinue = (pMemData->BestProposals[i]->getFuzziness() < usDisplayLevel);
        }

        // avoid empty proposals                                      
        if ( !fContinue && (ulLenProp == 0) )
        {
          fContinue = TRUE;
        } /* endif */

          // check if same match already copied (here the ->szSource buffer is used to store the target text of the following proposals
        if ( !fContinue )
        {
          for (k = 0; k < ulNumMemMatches; k++)
          {
            ulLen = pMemData->BestProposals[k]->getTarget( pMemData->szSource, MAX_SEGMENT_SIZE );

            // skip proposal if it is identical to a previous one
            if ( (ulLenProp == ulLen) && (wcscmp( pMemData->szTarget, pMemData->szSource ) == 0) &&
                 (pMemData->BestProposals[k]->getType() == pMemData->BestProposals[i]->getType() ) )
            {
              fContinue = TRUE;
              break;
            } // endif
          }                              // endfor
        } /* endif */

        if (fContinue)
          continue;

        if ( fMTPending )
        {
          if ( (LONG)pMemData->BestProposals[i]->getFuzziness() < pstEQFGen->lExactMatchLevel )
          {
            fMTPending = FALSE;
            UTF16strcpy (&pstReq->pucTargetSegs[ulDisp], L" ");
            ulDisp += 2;

            pstReq->usLevel[ulNumMatches] = (USHORT)pstEQFGen->lExactMatchLevel;
            pstReq->fEqualSource[ulNumMatches] = FALSE;
            pstReq->usMachineTrans[ulNumMatches] = MACHINE_TRANS_PENDING;
            ulNumMatches++; ulNumMemMatches++;
          } /* endif */
        } /* endif */

        pstReq->usLevel[ulNumMatches] = (USHORT)pMemData->BestProposals[i]->getFuzziness();
        if ( pMemData->BestProposals[i]->getType() == OtmProposal::eptMachine )
        {
          pstReq->usMachineTrans[ulNumMatches] = MACHINE_TRANS_PROP;
        
        } 
        else if ( pMemData->BestProposals[i]->getType() == OtmProposal::eptGlobalMemory )
        {
          pstReq->usMachineTrans[ulNumMatches] = GLOBMEM_TRANS_PROP;
        }        
        else if (  pMemData->BestProposals[i]->getType() == OtmProposal::eptGlobalMemoryStar )
        {
          pstReq->usMachineTrans[ulNumMatches] = GLOBMEMSTAR_TRANS_PROP;
        }        
        else 
        {
          pstReq->usMachineTrans[ulNumMatches] = 0;
        } /* end */           
        pMemData->BestProposals[i]->getSource( pMemData->szSource, MAX_SEGMENT_SIZE );
        pstReq->fEqualSource[ulNumMatches] = (EQF_BOOL)UTF16strcmp( pMemData->szDocSource, pMemData->szSource ) == 0;

        /**************************************************************/
        /* add file name of source in our structure                   */
        /**************************************************************/
        pMemData->BestProposals[i]->getDocName( pMemData->szNameBuffer, sizeof(pMemData->szNameBuffer) );
        if ( pMemData->szNameBuffer[0] != EOS )
        {
          strcpy( pstReq->szFName[ulNumMatches], pMemData->szNameBuffer );
        }
        else
        {
          pMemData->BestProposals[i]->getDocShortName( pMemData->szNameBuffer, sizeof(pMemData->szNameBuffer) );
          strcpy( pstReq->szFName[ulNumMatches], pMemData->szNameBuffer );
        } /* endif */

        /**************************************************************/
        /* add date of match into our structure                       */
        /**************************************************************/
        pstReq->lDate[ulNumMatches] = pMemData->BestProposals[i]->getUpdateTime();
        /**************************************************************/
        /* add index of matching TM into our structure                */
        /**************************************************************/
        pstReq->usDBIndex[ulNumMatches] = (USHORT)pMemData->BestProposals[i]->getMemoryIndex();
        /**************************************************************/
        /* add record number and number of proposal                   */
        /**************************************************************/
        pMemData->BestProposals[i]->getInternalKey( pstReq->szUniqueKey[ulNumMatches], sizeof(pstReq->szUniqueKey[ulNumMatches]) );
        pstReq->ulSegNum[ulNumMatches]    = pMemData->BestProposals[i]->getSegmentNum();

        pstReq->pszSortTargetSeg[ulNumMatches] = pstReq->pucTargetSegs + ulDisp;
        pstReq->pszSortPropsSeg[ulNumMatches] = pstReq->pucPropsSegs + ulPropDisp;
        /**************************************************************/
        /* if requested substitute fuzzy match                        */
        /**************************************************************/
        if ( pTBDoc->pUserSettings->fAutoRepl &&
          ((LONG)pMemData->BestProposals[i]->getFuzziness() < pstEQFGen->lExactMatchLevel))
        {
          /**********************************************************/
          /* copy unmodified translation ....                       */
          /**********************************************************/
          pszBuffer = pstReq->pucTargetSegs+ulDisp;
          UTF16strcpy (pszBuffer, pszProp);
          /**********************************************************/
          /* modify translation                                     */
          /**********************************************************/
          if ( pDoc->sTgtLanguage == -1 )
          {
            /***********************************************************/
            /* Get languages and format for current document           */
            /***********************************************************/
            if ((MorphGetLanguageID( pDoc->szDocTargetLang,
                   &pDoc->sTgtLanguage ) != MORPH_OK) )
            {
               pDoc->sTgtLanguage = pDoc->sSrcLanguage ;
            } /* endif */
          } /* endif */
          if (pDoc->ulTgtOemCP == 0)
          {
              pDoc->ulTgtOemCP = GetLangOEMCP( pDoc->szDocTargetLang );
          }

          // this function may exceed the max segment size limit during replacement!
          // so we use a temporary buffer for the replaced proposal and ignore it if it
          // is too large after replacement
          UTF16strcpy( szReplBuffer, pszBuffer );
          
          // store source of segment being looked up in szTarget of pMemData
          pMemData->SearchKey.getSource( pMemData->szTarget, MAX_SEGMENT_SIZE );

          pMemData->BestProposals[i]->getSource( pMemData->szSource, MAX_SEGMENT_SIZE );
          usPrepFuzzyProp = EQFBPrepareFuzzyPropEx( pTBDoc,
                                                  pMemData->szTarget,
                                                  pMemData->szSource,
                                                  szReplBuffer,
                                                  pDoc->sSrcLanguage,
                                                  pDoc->sTgtLanguage,
                                                  NULL,
                                                  pDoc->ulSrcOemCP,
                                                  pDoc->ulTgtOemCP);

          if ( UTF16strlenCHAR( szReplBuffer) < EQF_SEGLEN )
          {
            UTF16strcpy( pszBuffer, szReplBuffer );
          }
          else
          {
            // replaced segment is too large, ignore it
            usPrepFuzzyProp = PROP_NOT_REPLACED;
          } /* endif */

          if ( (usPrepFuzzyProp == PROP_REPLACED) ||
               (usPrepFuzzyProp == PROP_REPLACED_FUZZY) )
          {
            if ( usPrepFuzzyProp == PROP_REPLACED )
            {
              pstReq->usMachineTrans[ulNumMatches] |= REPLACE_PROP;
            }
            else
            {
              pstReq->usMachineTrans[ulNumMatches] |= FUZZY_REPLACE_PROP;
            } /* endif */

            /********************************************************/
            /* check if same match already copied                   */
            /********************************************************/
            ulLen = UTF16strlenCHAR( pszBuffer );
            pszProp = pstReq->pucTargetSegs;
            for (k = 0; k < ulNumMemMatches; k++)
            {
              ulLenProp = UTF16strlenCHAR (pszProp);    // length of first prop.
              // skip proposal if it is identical to a previous one
              if ((ulLenProp == ulLen) && (memcmp ((PBYTE)pszProp, (PBYTE)pszBuffer, ulLen * sizeof(CHAR_W)) == 0) && 
                 (pMemData->BestProposals[k]->getType() == pMemData->BestProposals[i]->getType() ) )
              {
                *pszBuffer = EOS;    // contents not valid any more....
                fContinue = TRUE;
                break;
              }
              else
              {
                pszProp += ulLenProp + 1;      // get next prop.
              } /* endif */
            } /* endfor */
            if ( fContinue )
            {
              continue;
            } /* endif */
          }
          else
          {
            pstReq->usMachineTrans[ulNumMatches] |= FUZZY_PROP;
          } /* endif */
          ulDisp += UTF16strlenCHAR( pszBuffer ) + 1;
          /**********************************************************/
          /* add an EOS because string might have become shorter... */
          /**********************************************************/
          *(pstReq->pucTargetSegs + ulDisp) = EOS;
          /************************************************************/
          /* fill source buffer if necessary                          */
          /************************************************************/
          if (UTF16strlenCHAR(pszBuffer) )
          {
            pMemData->BestProposals[i]->getSource( pMemData->szSource, MAX_SEGMENT_SIZE );
            PSZ_W pTemp = pMemData->szSource;
            UTF16strcpy( &(pstReq->pucPropsSegs[ulPropDisp]), pTemp );
            ulPropDisp += UTF16strlenCHAR( pTemp ) + 1;
          } /* endif */
        }
        else
        {
          if ( pMemData->BestProposals[i]->getFuzziness() < pstEQFGen->lExactMatchLevel )
          {
            pstReq->usMachineTrans[ulNumMatches] |= FUZZY_PROP;
          }
          else if ( pMemData->BestProposals[i]->getType() == OtmProposal::eptManual )
          {
            pstReq->usMachineTrans[ulNumMatches] |= EXACT_PROP;
          } /* endif */
          /**************************************************************/
          /* if fConvSOSI set in UserSettings insert them               */
          /**************************************************************/
          if (IsDBCS_CP(pDoc->ulTgtOemCP) && pTBDoc->pUserSettings->UserOptFlags.bConvSOSI )
          {
            ULONG ulSize = MAX_SEGMENT_SIZE;
            PSZ_W  pOutData = &pstReq->pucTargetSegs[ulDisp];
            EQFBUtlConvertSOSI( pOutData, &ulSize, pszProp, INSERT_SOSI , pDoc->ulTgtOemCP);
            ulLenProp = UTF16strlenCHAR(pOutData) ;
          }
          else
          {
            UTF16strcpy (&pstReq->pucTargetSegs[ulDisp], pszProp);
          } /* endif */

          ulDisp += ulLenProp + 1;
          /************************************************************/
          /* fill source buffer if necessary                          */
          /************************************************************/
          {
            pMemData->BestProposals[i]->getSource( pMemData->szSource, MAX_SEGMENT_SIZE );
            PSZ_W pTemp = pMemData->szSource;
            UTF16strcpy( &(pstReq->pucPropsSegs[ulPropDisp]), pTemp );
            ulPropDisp += UTF16strlenCHAR( pTemp ) + 1;
          } /* endif */
        } /* endif */


        // copy any additional proposal data
        if ( pMemData->BestProposals[i]->getAddInfoLen() != 0  )
        {
          pstReq->pszSortPropsData[ulNumMatches] = pstReq->pucPropAddData + (ulNumMatches * EQF_SEGLEN);
          pMemData->BestProposals[i]->getAddInfo( pstReq->pszSortPropsData[ulNumMatches], EQF_SEGLEN );
        } /* endif */

        ulNumMatches++; ulNumMemMatches++;
      } // endfor (all matches)
    }                                  // endif (match found)
    pstReq->usPropCount = (USHORT)ulNumMatches;

    if (ulNumMatches)
    {
        // init sort array if proposal eliminated
        int i;
        BOOL fAllExact = FALSE;         // true if all props are exact

        for ( i = ulNumMatches; i < EQF_NPROP_TGTS; i++ )
        {
           pstReq->pszSortTargetSeg[i] = NULL;
           pstReq->pszSortPropsSeg[i] =  NULL;
        }
        fAllExact= MemCalcFuzzyness(pTBDoc, pstReq, pDoc, &ulNumMatches );
        pstReq->usPropCount = (USHORT)ulNumMatches; // may have been changed ...
        // sorting exact proposals destroys order according to date
        if (!fAllExact)
        {
          MemSortAccordingToFuzzyness(pstReq, ulNumMatches);
        }
    }

    /******************************************************************/
    /* still MT translation not inserted ...                          */
    /******************************************************************/
    if ( fMTPending )
    {
      fMTPending = FALSE;
      pstReq->pszSortTargetSeg[ulNumMatches] = pstReq->pucTargetSegs + ulDisp;
      pstReq->pszSortPropsSeg[ulNumMatches] = pstReq->pucPropsSegs + ulPropDisp;
      UTF16strcpy (&pstReq->pucTargetSegs[ulDisp], L" ");
      ulDisp += 2;
      UTF16strcpy( &pstReq->pucPropsSegs[ulPropDisp], pstReq->pucSourceSeg );
      ulPropDisp += UTF16strlenCHAR( pstReq->pucSourceSeg ) + 1;

      pstReq->usLevel[ulNumMatches] =(USHORT) pstEQFGen->lExactMatchLevel;
      pstReq->usMachineTrans[ulNumMatches] = MACHINE_TRANS_PENDING;
      pstReq->usNumExactBytes[ulNumMatches] = 0;
      pstReq->usPropCount++;
    } /* endif */
  }                                    // endif (no error)

  return usRc;
} // end 'TlmGet'


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TlmDeleteProp
//------------------------------------------------------------------------------
// Function call:     TlmDeleteProp( PDOCUMENT_IDA, PSTEQFSAB);
//------------------------------------------------------------------------------
// Description:       delete the specified segment from translation memory
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA        pointer to document ida
//                    PSTEQFSAB            control block to proposals
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       ...                  error message from TM
//                    NO_ERROR             everything went okay
//------------------------------------------------------------------------------
// Function flow:     allocate memory
//                    setup prefix for IN
//                    setup segment
//                    call TmExtract for doing the extract; set return code
//                    if okay then
//                      setup data for deletion of segment and issue call
//                    endif
//                    return returncode
//------------------------------------------------------------------------------
static
USHORT  TlmDeleteProp
(
   PDOCUMENT_IDA   pDoc,
   PSTEQFSAB       pstQReq
)
{
  USHORT         usRc = 0;
  PTPMEMLOOKUPDATA pMemData = (PTPMEMLOOKUPDATA)pDoc->pvMemLookup;

  if ( !usRc )
  {
    // GQ: the unique key for this proposal is located in one of our SABs but not necessarly in the first one...
    PSZ pszUniqueKey = NULL;
    if ( pDoc->stEQFSab[pDoc->usFI].szUniqueKey[pstQReq->ulParm1-1][0] != EOS )  /* 8-27-14 */
    {
      pszUniqueKey = pDoc->stEQFSab[pDoc->usFI].szUniqueKey[pstQReq->ulParm1-1];
    } else 
    if ( pDoc->stEQFSab[0].szUniqueKey[pstQReq->ulParm1-1][0] != EOS )
    {
      pszUniqueKey = pDoc->stEQFSab[0].szUniqueKey[pstQReq->ulParm1-1];
    } 
    else if ( pDoc->stEQFSab[1].szUniqueKey[pstQReq->ulParm1-1][0] != EOS )
    {
      pszUniqueKey = pDoc->stEQFSab[1].szUniqueKey[pstQReq->ulParm1-1];
    } 
    else if ( pDoc->stEQFSab[2].szUniqueKey[pstQReq->ulParm1-1][0] != EOS )
    {
      pszUniqueKey = pDoc->stEQFSab[2].szUniqueKey[pstQReq->ulParm1-1];
    } 
    else
    {
      pszUniqueKey = pstQReq->szUniqueKey[pstQReq->ulParm1-1];
    }
    usRc = (USHORT)pMemData->MemoryList[0]->getProposal( pszUniqueKey, pMemData->SearchKey );
    if (usRc == NO_ERROR)
    {
      usRc = (USHORT)pMemData->MemoryList[0]->deleteProposal( pMemData->SearchKey );
    }
  } /* endif */
  return usRc;
}
/* $PAGEIF20 */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TlmClose
//------------------------------------------------------------------------------
// Function call:     TlmClose( PDOCUMENT_IDA )
//------------------------------------------------------------------------------
// Description:       close the translation memory
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA        pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       ...                  error message from TM
//                    NO_ERROR             everything went okay
//------------------------------------------------------------------------------
// Function flow:     if TM open then
//                      call TmClose, set return code
//                      if okay then
//                        reset handle to TM
//                      endif
//                      issue a properties changed message to all TMs
//                    endif
//                    return returncode
//------------------------------------------------------------------------------
static
USHORT  TlmClose
(
   PDOCUMENT_IDA   pDoc                   // pointer to document structure
)

{
  USHORT        usRc = NO_ERROR;
  PTPMEMLOOKUPDATA pMemData = (PTPMEMLOOKUPDATA)pDoc->pvMemLookup;
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  if ( pMemData != NULL )
  {
    if ( pMemData->MemoryList[0] != NULL)
    {
      /******************************************************************/
      /* close list of open memories                                    */
      /******************************************************************/
      USHORT i;
      for (i=0 ; !usRc && (i < pMemData->MemoryList.size()); i++)
      {
        if ( pMemData->MemoryList[i] != NULL )
        {
          pFactory->closeMemory( pMemData->MemoryList[i] );
          pMemData->MemoryList[i] = NULL;
        } /* endif */
      } /* endfor */
      /******************************************************************/
      /* update size of R/W memory                                      */
      /******************************************************************/
      EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED, MP1FROMSHORT( PROP_CLASS_MEMORY ), MP2FROMP( pDoc->szMemory[0] ));
    } /* endif */     
  } /* endif */     

  return usRc;
} // end 'TlmClose'
/* $PAGEIF20 */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TM_Error
//------------------------------------------------------------------------------
// Function call:     TM_Error( PDOCUMENT_IDA, USHORT);
//------------------------------------------------------------------------------
// Description:       map TM error to EQF error msg number
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA        pointer to document ida
//                    USHORT               error number
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       ...                  mapped error number
//
//------------------------------------------------------------------------------
// Function flow:     map the TM error to EQF error
//                    fill message buffer with Transl.Memory name
//                    return mapped error
//------------------------------------------------------------------------------
static
USHORT  TM_Error
(
   PDOCUMENT_IDA  pDoc,                   // pointer to document ida
   USHORT         usErr                   // transl. memory error code
)
{
  USHORT        usErrID;                  // error id to be set
  BOOL          fMemName = TRUE;          // mem file name or server name


  switch (usErr)
  {
      case BTREE_DISK_FULL:
      case BTREE_WRITE_ERROR:
      case DISK_FULL:
           usErrID = ERROR_MEM_DISK_FULL;
           break;
      case DB_FULL:
           usErrID = ERROR_MEM_DB_FULL;
           break;
      case SEG_NOT_FOUND:
           usErrID = EQFRC_OK;
           break;
      case TM_FILE_NOT_FOUND:
           usErrID = EQFS_TM_NOT_FOUND;
           break;
      case FILE_ALREADY_OPEN:
           usErrID = ERROR_MEM_IN_EXCL_USE;
           break;
      case CORRUPTION_FLAG_ON:
      case FILE_MIGHT_BE_CORRUPTED:
      case BTREE_CORRUPTED:
           usErrID = ERROR_MEM_MIGHT_BE_CORRUPTED;
           break;
      case VERSION_MISMATCH:
      case CORRUPT_VERSION_MISMATCH:
           usErrID = ERROR_MEM_VERSION_MISMATCH;
           break;
      case NOT_REPLACED_OLD_SEGMENT:
           usErrID = ERROR_MEM_NOT_REPLACED;
           break;
      case BTREE_NETWORK_ACCESS_DENIED:
           usErrID = ERROR_NETWORK_ACCESS_DENIED_MSG;
           break;
      case BTREE_IN_USE:
           usErrID = ERROR_MEM_NOT_ACCESSIBLE;
           break;
      default:
            usErrID = ERROR_MEM_UNDEFINED;
            break;
  }
  /********************************************************************/
  /* display name                                                     */
  /********************************************************************/
  if ( fMemName )
  {
    //CHAR chMem[MAX_EQF_PATH];

    //pData = UtlGetFnameFromPath( pDoc->szMemory[0] ); // extract name only
    //Utlstrccpy (chMem, pData, DOT); // set tm file name
    //ASCII2Unicode( chMem, pDoc->pstEQFGen->szMsgBuffer, 0L );
    ASCII2Unicode( pDoc->szMemory[0], pDoc->pstEQFGen->szMsgBuffer, 0L );
  } /* endif */
  return usErrID;
}                                      // end 'TM_Error'

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     TlmExtract
//------------------------------------------------------------------------------
// Function call:     TlmExtract( PDOCUMENT_IDA, PTM_ADDRESS, PSZ, PUSHORT);
//------------------------------------------------------------------------------
// Description:       extract the specified segment from translation memory
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA        pointer to document ida
//                    PSTEQFSAB            sentence buffer
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       ...                  error message from TM
//                    NO_ERROR             everything went okay
//------------------------------------------------------------------------------
// Function flow:     allocate memory
//                    setup prefix for IN
//                    setup segment
//                    call TmExtract for doing the extract; set return code
//                    if okay then
//                      copy the extracted segment into the provided buffer
//                    endif
//                    return returncode
//------------------------------------------------------------------------------
static
USHORT  TlmExtract
(
   PDOCUMENT_IDA   pDoc,
   PSTEQFSAB       pstQReq
)
{
  // this function is not available anymore
  USHORT         usRc = (USHORT)-1;

  pDoc;

  pstQReq->usRC = usRc;
  return usRc;
}                                      // end 'TlmExtract'

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     MemCheckMTProps
//------------------------------------------------------------------------------
// Function call:     MemCheckMTProps(PTMX_GET_OUT)
//------------------------------------------------------------------------------
// Description:       if MT Props exist and a good fuzzy prop, delete MT props
//------------------------------------------------------------------------------
// Parameters:        PTMX_GET_OUT
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Returncodes:       -
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------
static
VOID    MemCheckMTProps
(
   std::vector<OtmProposal *>&Proposals,
   BOOL            fDispMTAlways,
   LONG            lExactMatchLevel
)
{
  BOOL      fMT         = FALSE;
  BOOL      fMTFound    = FALSE;
  USHORT    usBestMatch = 0;
  USHORT    usI, usJ;
  USHORT    usNumMatchesFound = (USHORT)OtmProposal::getNumOfProposals( Proposals );
  BOOL      fFuzzyMT     = FALSE;


  for (usI = 0 ; usI < usNumMatchesFound ; usI++ )
  {
    fMT = Proposals[usI]->getType() == OtmProposal::eptMachine;
    fMTFound |= fMT;
    if (!fMT )
    {
      usBestMatch = (USHORT)max (usBestMatch, Proposals[usI]->getFuzziness() ); 
    } /* endif */
  } /* endfor */
  if (fMTFound )
  {
    usJ = 0;
    for (usI = 0 ; usI < usNumMatchesFound ; usI++ )
    {
      fMT = Proposals[usI]->getType() == OtmProposal::eptMachine;
      if (!fMT )
      {
        *Proposals[usJ] = *Proposals[usI];
        usJ++;
      }
      else
      {
        if (Proposals[usI]->getFuzziness() >= (lExactMatchLevel-1))
        {
          if (fDispMTAlways || (usBestMatch < GOOD_FUZZY))
          {
             *Proposals[usJ] = *Proposals[usI];
             usJ++;
          } /* endif */
        }
        else
        {
          /************************************************************/
          /* Fuzzy MT or replace MT matches are ignored always        */
          /************************************************************/
          fFuzzyMT = TRUE;
        } /* endif */
      } /* endif */
    } /* endfor */
    while ( usJ < usNumMatchesFound )
    {
      Proposals[usJ]->clear();
      usJ++;
    } /* end */       
  } /* endif */

  return;
}

static BOOL
MemCalcFuzzyness
(
   PTBDOCUMENT    pTBDoc,
   PSTEQFSAB      pstEQFSab,
   PDOCUMENT_IDA  pDoc,
   PULONG         pulMatches
)
{
    BOOL          fAllExact = TRUE;   // suppose all proposals are exact

    {
       BOOL          fOK = TRUE;
       PSZ_W           pucSeg0;
       PSZ_W           pucSeg;
       USHORT        usProps = 0;
       USHORT        usTokens = 0;
       PFUZZYTOK     pFuzzyTok = NULL;
       USHORT        usModWords = 0;
       PSTEQFGEN     pstEQFGen;
       SHORT         sPropLevel;
       PSZ_W         pucSrcOfProp;
       BOOL          fRemoved = FALSE;     //current prop not removed


       pucSeg0 = pstEQFSab->pucSourceSeg;

       usProps = 0;
       pstEQFGen = pDoc->pstEQFGen;
       pucSeg = pstEQFSab->pszSortTargetSeg[usProps];
       while ( pucSeg && (pDoc->fRunMT || usProps < EQF_NPROP_TGTS))
       {
          sPropLevel = pstEQFSab->usLevel[usProps];
          if (sPropLevel < (SHORT) pstEQFGen->lExactMatchLevel)
          {
             fAllExact = FALSE;
            if ( pstEQFSab->usFuzzyPercents[usProps] == 0 )
            {
              usModWords = 0;
              usTokens = 0;

              pucSrcOfProp = pstEQFSab->pszSortPropsSeg[usProps];
              fOK = EQFBFindDiff( pTBDoc, pucSeg0, pucSrcOfProp,
                                  pDoc->sSrcLanguage, &pFuzzyTok,
                                  &usModWords, &usTokens, pDoc->ulSrcOemCP);
              /******************************************************************/
              /* free allocated resources                                      */
              /*****************************************************************/
              if ( pFuzzyTok )
              {
                 UtlAlloc( (PVOID *) &pFuzzyTok, 0L, 0L, NOMSG );
              } /* endif */
              if (fOK)
              {
                pstEQFSab->usFuzzyPercents[usProps] = CalcPercent(usModWords, usTokens );

                // remove proposal if its below the selected fuzziness level
                {
                  USHORT usFuzzyLevel = 0;
                  USHORT usDisplayFuzzyLevel = 0;

                  // get fuzziness level for this segment
                  if ( usTokens > 15 )
                  {
                    usFuzzyLevel = (USHORT)(UtlQueryULong( QL_LARGELOOKUPFUZZLEVEL ) / 100);
                    usDisplayFuzzyLevel = (USHORT)(UtlQueryULong( QL_LARGEFUZZLEVEL ) / 100);
                  }
                  else if ( usTokens > 4 )
                  {
                    usFuzzyLevel = (USHORT)(UtlQueryULong( QL_MEDIUMLOOKUPFUZZLEVEL ) / 100);
                    usDisplayFuzzyLevel = (USHORT)(UtlQueryULong( QL_MEDIUMFUZZLEVEL ) / 100);
                  }
                  else
                  {
                    usFuzzyLevel = (USHORT)(UtlQueryULong( QL_SMALLLOOKUPFUZZLEVEL ) / 100);
                    usDisplayFuzzyLevel = (USHORT)(UtlQueryULong( QL_SMALLFUZZLEVEL ) / 100);
                  } /* endif */
                  if (usDisplayFuzzyLevel < usFuzzyLevel)
                  {
					  usFuzzyLevel = usDisplayFuzzyLevel;
			      }
			      if (pstEQFSab->usFuzzyPercents[usProps] < usDisplayFuzzyLevel)
				  { // do not display this proposal, it is below DisplayFuzzylevel
				      pstEQFSab->fInvisible[usProps] = TRUE;
                  } /* endif */

                  // remove proposal if below fuzzy level
                  if ( pstEQFSab->usFuzzyPercents[usProps] < usFuzzyLevel )
                  {
                    // move remaining proposals one position to the beginning
                    USHORT u = usProps;
                    while ( u < EQF_NPROP_TGTS-1)
                    {
                      pstEQFSab->usFuzzyPercents[u]  = pstEQFSab->usFuzzyPercents[u+1];
                      pstEQFSab->fInvisible[u]       = pstEQFSab->fInvisible[u+1];
                      pstEQFSab->usLevel[u]          = pstEQFSab->usLevel[u+1];
                      pstEQFSab->usNumExactBytes[u]  = pstEQFSab->usNumExactBytes[u+1];
                      pstEQFSab->usMachineTrans[u]   = pstEQFSab->usMachineTrans[u+1];
                      pstEQFSab->fEqualSource[u]     = pstEQFSab->fEqualSource[u+1];
                      strcpy( pstEQFSab->szFName[u], pstEQFSab->szFName[u+1] );
                      pstEQFSab->lDate[u]            = pstEQFSab->lDate[u+1];
                      pstEQFSab->ulKey[u]            = pstEQFSab->ulKey[u+1];
                      pstEQFSab->usTargetNum[u]      = pstEQFSab->usTargetNum[u+1];
                      pstEQFSab->usDBIndex[u]        = pstEQFSab->usDBIndex[u+1];
                      pstEQFSab->ulSegNum[u]         = pstEQFSab->ulSegNum[u+1];
                      pstEQFSab->pszSortPropsSeg[u]  = pstEQFSab->pszSortPropsSeg[u+1];
                      pstEQFSab->pszSortTargetSeg[u] = pstEQFSab->pszSortTargetSeg[u+1];
                      u++;
                    } /* endwhile */

                    // correct number of matches
                    *pulMatches -= 1;
                    fRemoved = TRUE;
                    // blank out last element
                    pstEQFSab->usFuzzyPercents[EQF_NPROP_TGTS-1] = 0;
                    pstEQFSab->pszSortTargetSeg[EQF_NPROP_TGTS-1] = NULL;
                    pstEQFSab->pszSortPropsSeg[EQF_NPROP_TGTS-1] = NULL;
                    pstEQFSab->fInvisible[EQF_NPROP_TGTS-1] = 0;

                  } /* endif */
                }
              }
            } /* endif */
          }
          else
          {
            pstEQFSab->usFuzzyPercents[usProps] = 100;  //exact proposal!
          } /* endif */
          // increment usProps only if not a Proposal has been deleted
          if (!fRemoved )
            usProps++;
          pucSeg = pstEQFSab->pszSortTargetSeg[usProps];
          fRemoved = FALSE;
       } // endwhile
    } /* endif */

   return(fAllExact);
}

static OtmProposal::eProposalType GetProposalTypeFromFlag( USHORT usMachineFlag )
{
  if ( usMachineFlag & MACHINE_TRANS_PROP ) return ( OtmProposal::eptMachine );
  if ( usMachineFlag & GLOBMEM_TRANS_PROP ) return ( OtmProposal::eptGlobalMemory );
  if ( usMachineFlag & GLOBMEMSTAR_TRANS_PROP ) return ( OtmProposal::eptGlobalMemoryStar );
  return ( OtmProposal::eptManual );
}

static OtmProposal::eMatchType GetMatchType( USHORT usFuzziness )
{
  if ( usFuzziness == 100) return ( OtmProposal::emtExact );
  if ( usFuzziness == 101 ) return ( OtmProposal::emtExactSameDoc );
  if ( usFuzziness == 102 ) return ( OtmProposal::emtExactExact );
  return ( OtmProposal::emtFuzzy );
}


//+----------------------------------------------------------------------------+
//|Internal function                                                           |
//+----------------------------------------------------------------------------+
//|Function name:     FuzzinessCompare                                         |
//+----------------------------------------------------------------------------+
//|Description:       compare function used by QSORT                           |
//+----------------------------------------------------------------------------+
//|Returncode type:   int                                                      |
//+----------------------------------------------------------------------------+
//|Returncodes:       = 0 if equal                                             |
//|                   < > if unequal                                           |
//+----------------------------------------------------------------------------+
//|Function flow:     compare fuzziness of both elements                       |
//+----------------------------------------------------------------------------+
static int FuzzynessCompare
(
const void *p1,                      // first element
const void *p2                       // second element
)
{
  MemoryFactory *pFactory = MemoryFactory::getInstance();
  PSTFUZZYORDER  pFuzzy1 = (PSTFUZZYORDER) p1;
  PSTFUZZYORDER  pFuzzy2 = (PSTFUZZYORDER) p2;

  int iMTDisplayFactor = (int)UtlQueryULong( QL_MTDISPLAYFACTOR );

  int iSortKey1 = pFactory->getProposalSortKey( GetMatchType(pFuzzy1->usFuzzyPercents), GetProposalTypeFromFlag( pFuzzy1->usMachineTrans ), (int)pFuzzy1->usFuzzyPercents, iMTDisplayFactor, 0, FALSE );
  int iSortKey2 = pFactory->getProposalSortKey( GetMatchType(pFuzzy2->usFuzzyPercents), GetProposalTypeFromFlag( pFuzzy2->usMachineTrans ), (int)pFuzzy2->usFuzzyPercents, iMTDisplayFactor, 0, FALSE );

  if ( iSortKey1 == iSortKey2 )
  {
    // base sort result on proposal time stamp when sort keys are identical
    return( pFuzzy2->lDate - pFuzzy1->lDate );
  }
  else
  {
    return( iSortKey2 - iSortKey1 );
  }
} /* end of function FuzzinessCompare */


static VOID MemSortAccordingToFuzzyness
(
  PSTEQFSAB      pstEQFSab,
  ULONG          ulNumMatches
)
{
      STFUZZYORDER  stFuzzyPropOrder[EQF_NPROP_TGTS]; // order of props acc. to fuzziness
      USHORT        k;

      memset(&(stFuzzyPropOrder[0]), 0, sizeof(&stFuzzyPropOrder) );

      for (k = 0; k < ulNumMatches; k++)
      {
        stFuzzyPropOrder[k].usIndex = k ;
        stFuzzyPropOrder[k].usFuzzyPercents = pstEQFSab->usFuzzyPercents[k];
        stFuzzyPropOrder[k].usLevel         = pstEQFSab->usLevel[k];
        stFuzzyPropOrder[k].usNumExactBytes = pstEQFSab->usNumExactBytes[k];
        stFuzzyPropOrder[k].usMachineTrans  = pstEQFSab->usMachineTrans[k];
        stFuzzyPropOrder[k].fEqualSource    = pstEQFSab->fEqualSource[k];
        strcpy(stFuzzyPropOrder[k].szFName, pstEQFSab->szFName[k]);
        stFuzzyPropOrder[k].lDate           = pstEQFSab->lDate[k];
        stFuzzyPropOrder[k].ulKey           = pstEQFSab->ulKey[k];
        stFuzzyPropOrder[k].usTargetNum     = pstEQFSab->usTargetNum[k];
        stFuzzyPropOrder[k].usDBIndex       = pstEQFSab->usDBIndex[k];
        stFuzzyPropOrder[k].ulSegNum        = pstEQFSab->ulSegNum[k];
        stFuzzyPropOrder[k].pszPropsSeg     = pstEQFSab->pszSortPropsSeg[k];
        stFuzzyPropOrder[k].pszTargetSeg    = pstEQFSab->pszSortTargetSeg[k];
        stFuzzyPropOrder[k].fInvisible      = pstEQFSab->fInvisible[k];
        stFuzzyPropOrder[k].pszAddData      = pstEQFSab->pszSortPropsData[k];
      }

      qsort( &(stFuzzyPropOrder[0]), ulNumMatches, /*tEQF_NPROP_TGTS,*/
             sizeof(STFUZZYORDER), FuzzynessCompare );

      for ( k = 0; k < ulNumMatches; k++ )
      {
        pstEQFSab->usFuzzyPercents[k]   = stFuzzyPropOrder[k].usFuzzyPercents;
        pstEQFSab->usLevel[k]           = stFuzzyPropOrder[k].usLevel;
        pstEQFSab->usNumExactBytes[k]   = stFuzzyPropOrder[k].usNumExactBytes;
        pstEQFSab->usMachineTrans[k]    = stFuzzyPropOrder[k].usMachineTrans;
        pstEQFSab->fEqualSource[k]      = stFuzzyPropOrder[k].fEqualSource;
        pstEQFSab->lDate[k]             = stFuzzyPropOrder[k].lDate;
        pstEQFSab->ulKey[k]             = stFuzzyPropOrder[k].ulKey;
        pstEQFSab->usTargetNum[k]       = stFuzzyPropOrder[k].usTargetNum;
        pstEQFSab->usDBIndex[k]         = stFuzzyPropOrder[k].usDBIndex;
        pstEQFSab->ulSegNum[k]          = stFuzzyPropOrder[k].ulSegNum;
        strcpy(pstEQFSab->szFName[k], stFuzzyPropOrder[k].szFName );
        pstEQFSab->pszSortPropsSeg[k]   = stFuzzyPropOrder[k].pszPropsSeg;
        pstEQFSab->pszSortTargetSeg[k]  = stFuzzyPropOrder[k].pszTargetSeg;
        pstEQFSab->fInvisible[k]        = stFuzzyPropOrder[k].fInvisible;
        pstEQFSab->pszSortPropsData[k]  = stFuzzyPropOrder[k].pszAddData;
      }  // endfor

 return;
} /* end of function MemSortAccordingToFuzzyness */

static USHORT SetAvailFlags( std::vector<OtmProposal *>&Proposals )
{
  USHORT usAvailFlags = 0;
  USHORT usProposalsFound = (USHORT)OtmProposal::getNumOfProposals( Proposals );

  // store indication if more matches are available
  if ( usProposalsFound > 1 )
  {
     USHORT   i;                                     // index value

       // indicate if other exact matches are available
     usAvailFlags |= Proposals[1]->isExactMatch() ? GET_MORE_EXACTS_AVAIL : 0;

     // indicate if fuzzy matches are available
     for ( i=1; i < usProposalsFound ;i++ )
     {
       if ( Proposals[i]->getFuzziness() < 100 )
       {
         usAvailFlags |= GET_ADDITIONAL_FUZZY_AVAIL;
         break;
       } /* endif */
     } /* endfor */
  }
  return( usAvailFlags );
}


