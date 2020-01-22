/*! \file
	Description: This file contains all functions concerned with the TENV services

	Copyright Notice:

	Copyright (C) 1990-2016, International Business Machines
	Corporation and others. All rights reserved
*/
#define INCL_EQF_ASD              // dictionary functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_EDITORAPI        // editor API
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#include <eqf.h>                  // General Translation Manager include file
#include <eqftmi.h>               // to get defines for EQUAL_EQUAL..

#include "OtmDictionaryIF.H"
#include <eqftpi.h>               // private Translation Processor include file
#include <eqftai.h>               // private text analysis include file
#include <eqfdoc00.h>             // for document handler defines
#include <OTMGLOBMem.h>           // Global Memory defines
#include "EQFHELP.ID"             // help ids for tutorial
#include "EQFB.ID"                // ids for AAB
#include <PROCESS.H>                   // C library for multi-tasking

#define MAX_TGT_LINES   3
#define MAX_TGT_LEN     20
#define MAX_PROP_LEN    78
#define MSGBOXDATALEN   60        // data len of segment to be displ.
/**********************************************************************/
/* the following items will remain in the C-file because they cannot  */
/* be configured and should therefore remain unchanged during the     */
/* translation                                                        */
/**********************************************************************/
#define FIRST_MARK_CHAR 'a'
#define LAST_MARK_CHAR  'z'
#define FIRST_UPPER_MARK_CHAR 'A'
#define LAST_UPPER_MARK_CHAR  'Z'
#define DICT_PREFIX     L"a) "
#define DICT_PREFIX_LEN 3
#define PROP_PREFIX     L"0 - "
#define PROP_PREFIX_LEN 4
#define CONT_STRING     L"..."                  // continuation string
#define DASH_STRING     L" - "                  // the Dash after the proposal
//#define SRC_DELETED_STRING L"? "                // string to mark deleted chars

#define DA_TIMER          50L                   // wait for dictionary
#define DA_TIMEWAITS      200                   // 100 cycles
#define DA_TIMERID        10

#define HDW         1                           // headword is following
#define TARGET      2                           // target is following
#define ADDDICTINFO 3                           // additional dict info
#define ADDINFO_MARK_PRE  0x01                  // additional info marker (before term)
#define ADDINFO_MARK_POST 0x02                  // additional info marker (after term)

#define MORE_EXACT_PROPS_STRING L"++"           // show that more Exact Props are available

static CHAR_W szDictStyleNotAllowed[10] = L"-";
static CHAR_W szDictStylePreferred[10]  = L"+";
static CHAR_W szNoteIndicator[10] = L"[Note]";
static CHAR_W EMPTYSTRINGW[2] = L"";
CHAR_W SRC_DELETED_STRING[3] = L"? ";

static STEQFSAB stEQFExt;                        // sab used for extract
static HWND hwndPopupMenu = NULLHANDLE;
                                       // clear the prop/dict window
static VOID  EQFClearWindow ( PTBDOCUMENT pTBDoc, BOOL fRedraw );
static VOID  EQFClearSegTable( PTBDOCUMENT pTBDoc );
static VOID  EQFClearDictUpd ( PDOCUMENT_IDA pDoc, BOOL fClear );

                                       // display the window
static VOID  EQFDispWindow ( PTBDOCUMENT  pTBDoc, BOOL bNoReset );

static VOID  EQFTMDAInit ( PDOCUMENT_IDA pDoc );
static BOOL PropDictWndCreate ( PDOCUMENT_IDA  pDoc,
                                USHORT         usType,
                                USHORT         usTitle);

static VOID InsertProposal ( PTWBSDEVICE  pDevice, BOOL fDisplay ) ;

static VOID InsertDictionary ( PTWBSDEVICE );

static VOID InsertSource ( PTWBSDEVICE, BOOL, BOOL );

static VOID DeleteProposal ( PTWBSDEVICE );
static USHORT DeleteProposalAllowed( PDOCUMENT_IDA, PTBDOCUMENT  );

static SHORT  PropLevel ( PDOCUMENT_IDA  pDoc, ULONG ulNum );
static SHORT  EqualFlag ( PDOCUMENT_IDA  pDoc, ULONG ulNum );

static SHORT  MachineTrans ( PDOCUMENT_IDA  pDoc, USHORT usNum );
static PSZ_W  GetPropSZ ( PDOCUMENT_IDA, ULONG, PULONG );
static PSZ_W  GetSrcPropSZ ( PDOCUMENT_IDA, USHORT );

static VOID  StartDictLookUp ( PTWBSDEVICE, USHORT );

static VOID ClearSAB ( PSTEQFSAB   pstEQFSab);             // clear SAB element


static VOID  SetLineNum ( PTBDOCUMENT, PSZ_W );


MRESULT HandleWMCharr( HWND, USHORT, WPARAM, LPARAM ); // handle char msgs
static VOID HandleWMSize ( HWND, WPARAM, LPARAM );         //    size msg
static VOID HandleWMMove ( HWND );                         //    move msg


static ULONG  EQF_Unpack(PUCHAR, PSZ, USHORT);            // get data back

static VOID PrepAddSeg ( PTBSEGMENT, PTBSEGMENT );
static TBSEGMENT  tbSegment;
static TBSEGMENT  tbPrepSeg;

CHAR_W szNewLine[2] = L"\n";
TBSEGMENT  tbInitSegment = { NULL, 0, QF_PROP0PREFIX, 0, NULL,{0}, 0,{0},0,0,0, 0L, NULL, L"", NULL};
TBSEGMENT  tbMemNewLineSegment = { NULL , 1,QF_PROP0PREFIX ,0, NULL,{0}, 0,{0},0,0,0, 0L, NULL, szNewLine, NULL};
TBSEGMENT  tbDictNewLineSegment = { NULL , 1,QF_DICTHEAD ,0, NULL,{0}, 0,{0},0,0,0, 0L, NULL, szNewLine, NULL};
//TBSEGMENT  tbNewLineSegment = { NULL , 1,QF_PROP0PREFIX ,0, NULL,{0}, 0,{0},0,0,0, 0L, NULL, L"\n", NULL};

static BOOL  InitProof ( PDOCUMENT_IDA );           // init proof reading

USHORT ProofTokenize( PDOCUMENT_IDA, PUCHAR, PUSHORT, PTERMLENOFFS * );
static VOID PrepData ( PTWBSDEVICE, PSZ_W, ULONG, USHORT, PULONG, SHORT );
static VOID PrepDataWithDiff( PTWBSDEVICE pDevice, PSZ_W pszSeg, PSZ_W pucCompareWith, PULONG pulSegNum, SHORT sMTLevel );
static VOID PrepPrefix ( PTBDOCUMENT, PSZ_W  *, USHORT, PULONG, BOOL );
static BOOL  InitMorphSrcDict ( PDOCUMENT_IDA );
static BOOL EQFPrepTMSegs ( PTWBSDEVICE, USHORT, PSZ_W, PSZ_W, PULONG, PUSHORT, PUSHORT );

static BOOL UpdateLoadedTagTables ( PDOCUMENT_IDA pDoc, PTWBSDEVICE pDevice );
static VOID PrepDictTitle( PDOCUMENT_IDA pDoc, BOOL fDispDictName );
static VOID PrepMemTitle( PDOCUMENT_IDA pDoc, BOOL fDispMemIndicator, USHORT usTitle );
static VOID EQFInsertDictCopyWord (PDOCUMENT_IDA pDoc,
                                   PSZ_W pSrcWord, PSZ_W pTgtWord, PSZ pAsciiBuf,
                                   USHORT usLength, USHORT usDispLen, USHORT usMaxTgtlen );

static ULONG GetMaxDictFont( PTBDOCUMENT pTBDoc );
static VOID EQFAbbrevFileName (PSZ_W pszAbbrFile, LONG lMaxLen, PSZ_W pszInFile );

MRESULT HandleMessageTWBRTF( HWND hwnd, PTBDOCUMENT pDoc, WINMSG msg, WPARAM mp1, LPARAM mp2);
static BOOL EqfFillFuzzyness (PSTEQFSAB pstEQFSab, USHORT usPropNum, PDOCUMENT_IDA pDoc,
                               USHORT sProps, PTBDOCUMENT pTBDoc, PSZ_W pucSeg0, SHORT sMTLevel);
static BOOL ContainsSegmentNote( PVOID pvAddInfo );

static PSZ_W GetPropData( PDOCUMENT_IDA pDoc, ULONG ulNum, PULONG  pulLen );

/* $PAGE */
/**********************************************************************/
/* define pragma to split segment ...                                 */
/**********************************************************************/
//#pragma alloc_text(EQF_TWBS1, EQFR_Init, EQFR_Close, EQFR_DumpSeg,     \
//                              EQFR_DelSeg, EQFR_GetSegNum, EQFR_ExtSeg, HandleWMMove,  \
//                              HandleWMSize, PropDictWndCreate)
//#pragma alloc_text(EQF_TWBS2, EQFR_ProofSeg, EQFR_ProofAid, EQFR_ProofAdd, \
//                              InitProof, EQFR_AddAbbrev )
//#pragma alloc_text(EQF_TWBS3, EQFR_GetProp, EQFR_GetDict, EQFR_GetSource )
//#pragma alloc_text(EQF_TWBS4, InsertDictionary, InsertProposal, InsertSource,\
//                              EQFPrepTMSegs, EQFDispWindow )
//#pragma alloc_text(EQF_TWBS5, EQFR_XDocNext, EQFR_XDocAdd, EQFR_XDocRemove,\
//                              EQFR_XDocAct, EQFR_XDocNum, EQFR_XDocInList )
/*******************************************************************************
*
*       function:       EQFR_Init
*
*******************************************************************************/


VOID  EQFR_Init
(
  PDOCUMENT_IDA  pDoc                           // pointer to document struct
)

{
  PSTEQFPCMD    pstPCmd;
  PSTEQFGEN     pstEQFGen;                      // pointer to generic structure
  BOOL          fOK;                            // success indicator

  pstEQFGen = pDoc->pstEQFGen;                  // pointer to generic structure
  pstPCmd = pstEQFGen->pstEQFPCmd;               // pointer to command struct
  pstEQFGen->lExactMatchLevel = EQUAL_EQUAL;
  pstEQFGen->usRC = NO_ERROR;                   // init return value
  pDoc->tidDA = (TID) -1;                       // no thread started for DA yet
  pDoc->tidMT = (TID) -1;                       // no thread started for MT

  /*-----------------------------------------------------------------------
  * Unpack TMT/Dict.Names to appr. arrays
  *----------------------------------------------------------------------*/
  EQF_Unpack (&pstPCmd->ucbBuffer[pstPCmd->usLen1],
              pDoc->szMemory[0], min(EQF_MAX_TM_BASES,MAX_NUM_OF_READONLY_MDB));
  EQF_Unpack (&pstPCmd->ucbBuffer[pstPCmd->usLen1 + pstPCmd->ulParm1],
              pDoc->szDicts[0], (MAX_DICTS-2)/2);

  /********************************************************************/
  /* the following is now already done in EQFDOC00  ....              */
  /* fOK =  EQFBInit();                   // init colors and keys     */
  /********************************************************************/

  /********************************************************************/
  /* create the different service windows                             */
  /********************************************************************/
  fOK = PropDictWndCreate ( pDoc, SERVPROP_DOC, SID_PROP_TITLE );
  if ( fOK )
  {
     fOK =  PropDictWndCreate ( pDoc, SERVDICT_DOC, SID_DCT_TITLE );
     if ( fOK )
     {
        fOK = PropDictWndCreate ( pDoc, SERVSOURCE_DOC, SID_SRC_TITLE );
     } /* endif */
  } /* endif */


  if ( fOK )
  {
    // initialize return length to 0 (default for INIT)
    pstPCmd->usLen1 = pstPCmd->usLen2 = 0;

    EQFTMDAInit( pDoc );                         // init structures

    if ( !pstEQFGen->usRC && (pstEQFGen->fsConfiguration & EQFF_TM_CONF))
    {
       EQFTM (pDoc, EQFCMD_INIT, &(pDoc->stEQFQCmd));       // INIT to TM
    } /* endif */
    if ( !pstEQFGen->usRC && (pstEQFGen->fsConfiguration & EQFF_DA_CONF) )
    {
       EQFDAInit( pDoc );
    } /* endif */
    if ( !pstEQFGen->usRC && (pstEQFGen->fsConfiguration & EQFF_MT_CONF) )
    {
       EQFMTInit( pDoc );
    } /* endif */

    /******************************************************************/
    /* init morphological service for source language                 */
    /******************************************************************/
    if ( !pstEQFGen->usRC )
    {
       InitMorphSrcDict ( pDoc );
    } /* endif */
  }
  else
  {
    pstEQFGen->usRC = ERROR_MSG_HANDLED;       // message already processed
  } // endif (dict. / prop. dialgs loaded)

  return ;
} // end 'EQFR_Init'
/* $PAGE */
/*******************************************************************************
*
*       function:       EQFR_TransSeg
*
*******************************************************************************/

VOID  EQFR_TransSeg
(
   PDOCUMENT_IDA  pDoc                          // pointer to document inst ida
)
{
  ULONG         ulSegNum;                       // segment number
  USHORT        fsXltFlags;                     // translation flags
  USHORT        i;                              // general
  USHORT        j;                              // general wait
  PSTEQFSAB     pstEQFSab;                      // ptr to SAB element
  BOOL          fSegmentPresent = FALSE;        // indicator
  BOOL          fForeground;                    // foreground
  PSTEQFPCMD    pstPCmd;
  PSTEQFGEN     pstEQFGen;                      // pointer to generic structure
  PTBDOCUMENT   pTBDoc;                         // document
  BOOL          fShowProposal;


  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct

  pstEQFGen->usRC = pDoc->usDAError;
  UTF16strcpy (pstEQFGen->szMsgBuffer, pDoc->szMsgBuffer);

  // store segment number and priority of TRANSLATE request
  ulSegNum   = pstPCmd->ulParm1;
  fForeground= pstPCmd->usParm2;
  fsXltFlags = pstPCmd->usParm3;               // automatic translation flags
  pDoc->fsConfig = fsXltFlags;                 // store configuration flags

  /********************************************************************/
  /* get rid of Source of proposal window if not specifically wanted  */
  /* by user...                                                       */
  /********************************************************************/
  pTBDoc = &(pDoc->tbDevSource.tbDoc);
  if (!pTBDoc->pUserSettings )
  {
    EQFBGetUserSettings(pTBDoc);
  } /* endif */
  fShowProposal = (pTBDoc->pUserSettings->fSrcPropWnd && !( fsXltFlags & EQFF_NOPROPWND ));
  WinShowWindow( pTBDoc->hwndFrame, fShowProposal );

  // set default return match level
  pstPCmd->ulParm1 = (ULONG) -1;      // default: no exact match


  if ( !pDoc->fRunDA && (pstEQFGen->fsConfiguration & EQFF_DA_CONF) )
  {
    // determine num of passed dicts and wait accordingly
    i = 0;
    while ( *(pDoc->szDicts[i++]) );

    j = 0;
    /******************************************************************/
    /* let thread time to process                                     */
    /******************************************************************/

    while ( pDoc->fSemDAProc && (j < i * (SHORT) DA_TIMER))
    {
      j++;
    } /* endwhile */

    if (!(fsXltFlags & EQFF_NOAUTODICT) && pDoc->fRunDA
           && (pstEQFGen->fsConfiguration & EQFF_DA_CONF)    )
    {
         // display empty dictionary window
         pTBDoc = &(pDoc->tbDevDictionary.tbDoc);
         EQFClearWindow( pTBDoc , TRUE );
         WinPostMsg (pDoc->hwndDictionary, WM_EQF_PROCESSTASK,
                     MP1FROMSHORT( SHOW_PROPDICT ),
                     MP2FROMSHORT( TRUE ));
    }                               // endif

    if ( ! pDoc->fRunDA )
    {
       if ( pDoc->usDAError )
       {
          // use old error code
          pstEQFGen->usRC = pDoc->usDAError;
          UTF16strcpy (pstEQFGen->szMsgBuffer, pDoc->szMsgBuffer);
          pDoc->usDAError = 0;
       }
       else
       {
          pstEQFGen->usRC = EQFS_DICT_START;
       } /* endif */
       pstEQFGen->fsConfiguration &=  ~EQFF_DA_CONF;
    } /* endif */
  } // end if (segment not present)

  if ( !pDoc->fRunMT && (pstEQFGen->fsConfiguration & EQFF_MT_CONF) )
  {
    /******************************************************************/
    /* let thread time to process                                     */
    /******************************************************************/
    j = 0;

    while ( pDoc->fSemMTProc && j <  DA_TIMEWAITS)
    {
      j++;
    } /* endwhile */

    if ( !pDoc->fRunMT )
    {
      pstEQFGen->szMsgBuffer[0] = EOS;
      pstEQFGen->usRC = ERROR_START_MT;
      pstEQFGen->fsConfiguration &= ~EQFF_MT_CONF;
    } /* endif */
  } /* endif */
  /*---------------------------------------------------------------------------*
  * Search for same segment number in SAB.
  *---------------------------------------------------------------------------*/
  if ( !pstEQFGen->usRC )
  {
    pstEQFSab = pDoc->stEQFSab;                   // init buffer pointer

    for (i = 0; i < EQF_NSAB; i++)
    {
      if (pstEQFSab->fbInUse &&                   // segment in use
          pstEQFSab->ulParm1 == ulSegNum)         // segment no. already present
      {
        fSegmentPresent = TRUE;
        if ( fForeground )
        {
          pDoc->usEI = pDoc->usFI = i;      // set foreground index
          pDoc->fForeground = TRUE;            // remember fg.seg. present
        } // endif high priority
        break;
      }
      pstEQFSab++;                                // point to next one
    } // endfor (EQF_NSEGS)

    /*---------------------------------------------------------------------------*
    * It is a new segment and has to be entered into SAB.
    *---------------------------------------------------------------------------*/
    if (!fSegmentPresent)
    {
                                             // new entry index (modulo EQF_NSAB)
      pDoc->usEI = (pDoc->usEI >= EQF_NSAB-1) ? 0 : ++pDoc->usEI;
      if (pDoc->usEI == pDoc->usFI)          // if the next is a foreground
      {
        pDoc->usEI = (pDoc->usEI >= EQF_NSAB - 1) ? 0 : ++pDoc->usEI;
      } /* endif */
      // is it the current working segment within the dict. service
      pstEQFSab = pDoc->stEQFSab + pDoc->usEI;             // point to current element
      if ( pDoc->stTWBS.pstEQFSabDict == pstEQFSab )
      {
         pDoc->usEI = (pDoc->usEI >= EQF_NSAB-1) ? 0 : ++pDoc->usEI;
         pstEQFSab = pDoc->stEQFSab + pDoc->usEI;             // point to current element
      } /* endif */


      ClearSAB( pstEQFSab );                    // clear SAB
      memcpy ((PBYTE)(pstEQFSab->pucSourceSeg),          // copy source seg. from
              (PBYTE)(pstPCmd->ucbBuffer),               // pipe buffer to
              pstPCmd->usLen1 * sizeof(CHAR_W));// SAB

      // handle any context information
      if ( pstPCmd->usLen2 )
      {
        memcpy( pstEQFSab->pszContext,
                pstPCmd->ucbBuffer + (pstPCmd->usLen1 * sizeof(CHAR_W)),
                pstPCmd->usLen2 * sizeof(CHAR_W));
      } /* endif */
      pstEQFSab->ulParm1 = ulSegNum;            // new segment number
      pstEQFSab->fbInUse = TRUE;                // indicate 'in use'

      // pass metadata to SAB
      pstEQFSab->pvMetaData = pstPCmd->pvMetaData;

      if ( fForeground )
      {
         pDoc->fForeground = TRUE;           // remember fg.segment present
         pDoc->usFI = pDoc->usEI;            // new foreground index
      } /* endif */

    } // end if (segment not present)
  } /* endif */

  pstPCmd->usLen1  = pstPCmd->usLen2  = 0;

  /*-------------------------------------------------------------------------*
  * If it is a foreground request - process TM request and wait for
  * eventual DA completion.
  *-------------------------------------------------------------------------*/
  if ( fForeground && ! pstEQFGen->usRC )
  {
    pstEQFSab = pDoc->stEQFSab + pDoc->usFI; // point to current element
	pstEQFSab->usPropCount = 0;
	memset(&pstEQFSab->pszSortTargetSeg[0], '\0', EQF_NPROP_TGTS*sizeof(PSZ_W));
  memset(&pstEQFSab->pszSortPropsSeg[0], '\0', EQF_NPROP_TGTS*sizeof(PSZ_W));
	memset(&pstEQFSab->usFuzzyPercents[0], '\0', EQF_NPROP_TGTS*sizeof(USHORT));
	memset(&pstEQFSab->fInvisible[0], '\0', EQF_NPROP_TGTS*sizeof(EQF_BOOL));
  memset(&pstEQFSab->pszSortPropsData[0], '\0', EQF_NPROP_TGTS*sizeof(PSZ_W));
    UTF16memset (pstEQFSab->pucTargetSegs, '\0', EQF_TGTLEN);
    UTF16memset (pstEQFSab->pucPropsSegs, '\0', EQF_TGTLEN);
    UTF16memset (pstEQFSab->pucPropAddData, '\0', EQF_TGTLEN);
    EQFTM (pDoc, EQFCMD_TRANSSEGW, pstEQFSab);     // TRANSSEG to TM

    /*------------------------------------------------------------------------
    * If there was no automatic lookup and TM match level is not exact,
    * process dict.lookup.
    *-----------------------------------------------------------------------*/
    if (fsXltFlags & EQFF_NOAUTODICT && pDoc->fRunDA &&
        (!pstEQFSab->usPropCount ||
         PropLevel (pDoc, 1) < (SHORT)pstEQFGen->lExactMatchLevel))
    {
      fsXltFlags &= ~EQFF_NOAUTODICT;   // change flag to force display
    } // endif (no automatic dict.lookup)

    /*-----------------------------------------------------------------------*
    * Display results if there was no error.
    *-----------------------------------------------------------------------*/
    if ( (pstEQFGen->usRC == 0) || (pstEQFGen->usRC==EQFRS_SEG_NOT_FOUND))
    {
      if (pstEQFGen->usRC == 0)
      {
        // if there is an exact match, return the info with EQFTRANSSEG
        if (pstEQFSab->usPropCount &&
            PropLevel (pDoc, 1) >= (SHORT)pstEQFGen->lExactMatchLevel)
        {
          pstPCmd->ulParm1 = 1;
        } // endif

        // if more info is requested, provide it...
        if (pDoc->fsConfig & EQFF_MOREPROPINDIC )
        {
                        pstPCmd->ulParm1 |=  (pstEQFSab->fsAvailFlags & GET_MORE_EXACTS_AVAIL) ? EQFF_MORE_EXACTS_AVAIL : 0;
                        pstPCmd->ulParm1 |=  (pstEQFSab->fsAvailFlags & GET_ADDITIONAL_FUZZY_AVAIL) ? EQFF_ADDITIONAL_FUZZY_AVAIL : 0;
                }
      } // endif (exact match)

      /****************************************************************/
      /* update counting information                                  */
      /****************************************************************/
      if ( pDoc->fRunTM )
      {
        /**************************************************************/
        /* prepare our internal structures used for EQF_GETPREFIX and */
        /* EQF_GETLCS                                                 */
        /**************************************************************/
        InsertProposal ( &(pDoc->tbDevProposal), FALSE );
        /********************************************************/
        /* prepare the usModWords for the calculation report    */
        /********************************************************/
        InsertSource(&(pDoc->tbDevSource), FALSE, FALSE );
      } /* endif */

      /*---------------------------------------------------------------------*
      * Display proposal data if configured.
      *---------------------------------------------------------------------*/
      if ( pDoc->fRunTM  && !(fsXltFlags & EQFF_NOPROPWND))
      {
        WinPostMsg (pDoc->hwndProposals, WM_EQF_PROCESSTASK,
                    MP1FROMSHORT( INSERT_PROPOSAL ), NULL );
      }
      else
      {
        WinPostMsg (pDoc->hwndProposals, WM_EQF_PROCESSTASK,
                    MP1FROMSHORT( SHOW_PROPDICT ), NULL );
      } // end if (display proposal data)

      /*---------------------------------------------------------------------*
      * Display dictionary data if configured.
      *---------------------------------------------------------------------*/
      if ( pDoc->fRunDA  && !(fsXltFlags & EQFF_NODICTWND))
      {
                                                  // clear dictionary window
         pTBDoc = &(pDoc->tbDevDictionary.tbDoc);
         EQFClearWindow( pTBDoc , TRUE );
         /************************************************/
         /* force a dictionary lookup for the sentence   */
         /************************************************/
         if (!pstEQFSab->fDAReady )
         {
           pDoc->pstEQFQDA  = pstEQFSab;  // get dictentries for SAB
           pDoc->stTWBS.pstEQFSabDict  = pstEQFSab;
           pDoc->fSemDAProc = TRUE;
           EQFDAEntries( pDoc );
         } /* endif */
         pDoc->sDictPrefixPage = 1 ;
         WinPostMsg (pDoc->hwndDictionary, WM_EQF_PROCESSTASK,
                     MP1FROMSHORT( INSERT_DICTIONARY) , pstEQFSab );
      }
      else
      {
        WinPostMsg (pDoc->hwndDictionary, WM_EQF_PROCESSTASK,
                     MP1FROMSHORT( SHOW_PROPDICT) , NULL );
      } // end if (display dictionary data)

    } // end if (display foreground request)
  } /* endif */

  return;
} // end 'EQFR_TransSeg'
/* $PAGE */
/*******************************************************************************
*
*       function:       EQFR_Close
*
*******************************************************************************/

VOID  EQFR_Close
(
  PDOCUMENT_IDA pDoc                            // pointer to document ida
)
{
  PSTEQFPCMD    pstPCmd;
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  USHORT        j  ;                            // index

  if ( pDoc )
  {
     WinStopTimer( (HAB) UtlQueryULong( QL_HAB ), pDoc->hwndDictionary,
                   DA_TIMERID );
     pstEQFGen = pDoc->pstEQFGen;
     pstPCmd = pstEQFGen->pstEQFPCmd;                      // pointer to command struct
     /*****************************************************************/
     /* indicate closing for any running thread                       */
     /*****************************************************************/
     if ( pstEQFGen )
     {
       if ( pDoc->tidDA != -1 )
       {
         j = 0;
         pDoc->fRunDA = FALSE;                             // close dictionary


         while (  pDoc->fSemDAProc && j < DA_TIMEWAITS )
         {
           j++;
         } /* endwhile */
         pDoc->fRunDA = FALSE;                         // close dictionary
         pDoc->fSemDAProc = TRUE;

           /***********************************************************/
           /* close the dictionary by hand -- since we have no Thread */
           /* doing this for us under Windows...                      */
           /***********************************************************/
           EQFDAClose( pDoc );
       } /* endif */
       if ( pDoc->tidMT != -1 )
       {
         j = 0;
         pDoc->fRunMT = FALSE;                             // close MT-tt bridge

         while (  pDoc->fSemMTProc && j < DA_TIMEWAITS )
         {
           j++;
         } /* endwhile */

         pDoc->fRunMT = FALSE;
         pDoc->fSemMTProc = TRUE;
       } /* endif */
     } /* endif */

     /*****************************************************************/
     /* close translation memory                                      */
     /*****************************************************************/
     if (pstEQFGen && (pstEQFGen->fsConfiguration & EQFF_TM_CONF)  )
     {
       EQFTM (pDoc, EQFCMD_CLOSE, &(pDoc->stEQFQCmd));         // CLOSE to TM
     } // end if
     /*****************************************************************/
     /* destroy the windows                                           */
     /*****************************************************************/
     if ( pDoc->hwndProposals)
     {
        WinDestroyWindow (pDoc->hwndProposals);
        pDoc->hwndProposals = NULLHANDLE;
     } /* endif */
     if ( pDoc->hwndDictionary)
     {
        WinDestroyWindow (pDoc->hwndDictionary);
        pDoc->hwndDictionary = NULLHANDLE;
     } /* endif */
     if ( pDoc->hwndSource)
     {
        WinDestroyWindow (pDoc->hwndSource);
        pDoc->hwndSource = NULLHANDLE;
     } /* endif */

     /*****************************************************************/
     /* check again if threads are finished                           */
     /*****************************************************************/
     if ( pstEQFGen )
     {
       if ( pDoc->tidDA != -1 )
       {
         pstEQFGen->fsConfiguration &=  ~EQFF_DA_CONF; // no dict any more
          // wait til thread cleanup up
         j = 0;

         while ( pDoc->fSemDAProc  && j < DA_TIMEWAITS )
         {
           j++;
         } /* endwhile */

       } /* endif */

       if ( pDoc->tidMT != -1 )
       {
         pstEQFGen->fsConfiguration &=  ~EQFF_MT_CONF; // no MT any more
         j = 0;

         while ( pDoc->fSemMTProc  && j < DA_TIMEWAITS )
         {
           j++;
         } /* endwhile */

       } /* endif */
     } /* endif */

  } /* endif */
  return;
} // end 'EQFR_Close'

/* $PAGE */

/*******************************************************************************
*
*       function:       EQFR_Clear
*
*******************************************************************************/

VOID  EQFR_Clear
(
  PDOCUMENT_IDA  pDoc
)

{
  USHORT        i;
  PSTEQFPCMD    pstPCmd;
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  PSTEQFSAB     pstEQFSab;                      // pointer to send ahead buffer
  PTBDOCUMENT   pTBDoc;                         // pointer to document

  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;              // pointer to command struct
  pDoc->fsConfig = pstPCmd->ulParm1;            // store configuration

  pstPCmd->usLen1  =  pstPCmd->usLen2  = 0;     // no return data

  /*---------------------------------------------------------------------------*
  * Clear 'in use' flags to invalidate contents of SAB
  *---------------------------------------------------------------------------*/
  pstEQFSab = pDoc->stEQFSab;
  for (i = 0; i < EQF_NSAB; i++)
  {
     ClearSAB( pstEQFSab );                     // clear send a head buffer
     pstEQFSab++;                               // point to next one
  } // endfor
  pDoc->fForeground = FALSE;
  pDoc->usFI = pDoc->usEI = 0;            // no data available

  /*---------------------------------------------------------------------------*
  * CLEAR TM window
  *---------------------------------------------------------------------------*/
  if (pDoc->fRunTM)                          // TM running
  {
    pTBDoc = &(pDoc->tbDevProposal.tbDoc);
    EQFClearWindow( pTBDoc , TRUE );
    if (pstPCmd->ulParm1 & EQFF_NOPROPWND)      // hide proposal window
    {
      WinShowWindow (pTBDoc->hwndFrame, FALSE);
    } /* endif */
    pTBDoc = &(pDoc->tbDevSource.tbDoc);
    EQFClearWindow( pTBDoc , TRUE );
    if (pstPCmd->ulParm1 & EQFF_NOPROPWND)      // hide source of proposal wnd
    {
      WinShowWindow (pTBDoc->hwndFrame, FALSE);
    } /* endif */
  } // end if

  if (pDoc->fRunDA)                            // DA running
  {
    pTBDoc = &(pDoc->tbDevDictionary.tbDoc);
    EQFClearWindow( pTBDoc , TRUE );
    if (pstPCmd->ulParm1 & EQFF_NODICTWND)      // hide window
      WinShowWindow (pTBDoc->hwndFrame, FALSE);
  } /* endif */

  // hide any segment property window when source of proposal window is hidden
  if (pstPCmd->ulParm1 & EQFF_NOSEGPROPWND )      // hide source of proposal wnd
  {
    MDShowSegPropDialog( FALSE );
  } /* endif */

  return;
} // end 'EQFR_Clear'
/* $PAGEIF20 */
/*******************************************************************************
*
*       function:       EQFR_ClearDictUpd
*
*      This function is called via the API
*******************************************************************************/

VOID  EQFR_ClearDictUpd
(
  PDOCUMENT_IDA  pDoc
)
{
  EQFClearDictUpd( pDoc, TRUE );
} // end 'EQFR_ClearDictUpd'
//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFClearDictUpd
//------------------------------------------------------------------------------
// Function call:     EQFClearDictUpd( pDoc, fRedraw );
//------------------------------------------------------------------------------
// Description:       This function will clear all send-a-head buffers
//                    and force depending on the fReDraw flag a redisplay of
//                    the dictionary window
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc    document ida
//                    BOOL           fRedraw redraw indication
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     loop through the send-a-head buffers and clear them
//                    In the case of the foreground buffer, force a redisplay
//                    of the window
//------------------------------------------------------------------------------
static VOID  EQFClearDictUpd
(
  PDOCUMENT_IDA  pDoc,
  BOOL           fRedraw
)
{
  USHORT        i;
  PSTEQFSAB     pstEQFSab;                      // pointer to send ahead buffer

  /*---------------------------------------------------------------------------*
  * Clear 'in use' flags to invalidate contents of SAB, clear paste buffers.
  *---------------------------------------------------------------------------*/
  pstEQFSab = pDoc->stEQFSab;
  for (i = 0; i < EQF_NSAB; i++)
  {
     // clear buffer if not foreground segment else send it for lookup again
     if ( pstEQFSab != (pDoc->stEQFSab+pDoc->usFI) )
     {
        ClearSAB( pstEQFSab );                  // clear send a head buffer
     }
     else
     {// clear only if there is a dictionary attached!
		if (pDoc->hUCB)
		{
			// clear dict window and lookit up again - might be changed
			EQFClearWindow( &pDoc->tbDevDictionary.tbDoc, fRedraw );
			pstEQFSab->fDAReady = FALSE;
			UTF16memset (pstEQFSab->pucDictWords, '\0', EQF_DICTLEN);
			 /************************************************/
			 /* force a dictionary lookup for the sentence   */
			 /************************************************/
			 pDoc->pstEQFQDA  = pstEQFSab;  // get dictentries for SAB
			 pDoc->stTWBS.pstEQFSabDict  = pstEQFSab;
			 pDoc->fSemDAProc = TRUE;
			 EQFDAEntries( pDoc );
			 WinPostMsg (pDoc->hwndDictionary, WM_EQF_PROCESSTASK,
						 MP1FROMSHORT( INSERT_DICTIONARY) , pstEQFSab );
	     }
     } /* endif */
     pstEQFSab++;                               // point to next one
  } // endfor

  return;
} // end 'EQFClearDictUpd'
/* $PAGEIF20 */

/*******************************************************************************
*
*       function:       EQFR_GetProp
*
*******************************************************************************/

VOID  EQFR_GetProp
(
  PDOCUMENT_IDA  pDoc                              // pointer to document ida
)

{
  ULONG         ulParm;                         // parms
  ULONG         ulLen;
  USHORT        i;                              // index
  PSZ_W         pData;                          // poitner to data
  PSZ_W         pBuffer;                        // pointer to buffer
  PSZ_W         pszProp;                        // ptr to prop.buffer
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  PSTEQFPCMD    pstPCmd;                        // pointer to pipe cmd struct
  PSTEQFSAB     pstEQFSab;                      // ptr to SAB element

  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct

    pstEQFGen->usRC    = NO_ERROR;


  if (pDoc->fRunTM)                             //  if TM is running.
  {
    ulParm = pstPCmd->ulParm1;                  // GETPROP parameter
    pstPCmd->ulParm1 = (ULONG) -1;             // return: match level
    pstPCmd->usParm2 = FALSE;                   // return: equal flag

    /*-------------------------------------------------------------------------*
    * Check parameter (get proposal/scroll)
    *-------------------------------------------------------------------------*/
    switch (ulParm)
    {
      case EQF_SCROLL_UP:
           WinPostMsg (pDoc->hwndProposals, WM_VSCROLL,
                          SB_LINEUP, MP2FROM2SHORT (0, 0));
           break;

      case EQF_SCROLL_DOWN:
           WinPostMsg (pDoc->hwndProposals, WM_VSCROLL,
                         SB_LINEDOWN, MP2FROM2SHORT (0, 0));
           break;

      case EQF_ACTIVATE:
           WinPostMsg (pDoc->hwndProposals, WM_EQF_PROCESSTASK,
                       MP1FROMSHORT(ACT_PROPDICT), NULL );
           break;

      case EQF_IS_AVAIL:
           if ( ! WinIsWindowVisible(pDoc->hwndProposals))
           {
             pstEQFGen->usRC = EQFRS_NOT_AVAILABLE;
           } /* endif */
           break;

       /***************************************************************/
       /* return the prefix areas for every proposal                  */
       /***************************************************************/
     case EQF_GETLCS:
           pstPCmd->ulParm1 =  pDoc->stTWBS.usLCSModWords;
           break;
     case EQF_GETMARKEDBLOCK:
           {
            PTBDOCUMENT  pTBDoc;
            PEQFBBLOCK   pstBlock;
            USHORT       usBytes = 1;
            PTBSEGMENT   pSeg;

            pBuffer = (PSZ_W)&pstPCmd->ucbBuffer[0];

            pBuffer[0] = '\0';
            pTBDoc = &(pDoc->tbDevProposal.tbDoc);
            if (pTBDoc->pBlockMark)
            {
              pstBlock = (PEQFBBLOCK)(pTBDoc->pBlockMark);
              if (pstBlock->pDoc == pTBDoc )
              {
               if (pstBlock->ulSegNum == pstBlock->ulEndSegNum)
               {
                 usBytes = pstBlock->usEnd - pstBlock->usStart + 2;
                 pSeg = EQFBGetSegW(pstBlock->pDoc, pstBlock->ulSegNum);
                 memcpy((PBYTE)pBuffer,
                       (PBYTE)(pSeg->pDataW+pstBlock->usStart),
                       usBytes*sizeof(CHAR_W));
                 pBuffer[usBytes - 1] = '\0';
                 {
                   USHORT     usNewLen;
                   USHORT     usSegOffset;
                   if (pTBDoc->fLineWrap && pTBDoc->fAutoLineWrap)
                   {
                     EQFBBufRemoveSoftLF(pTBDoc->hwndRichEdit, pBuffer, &usNewLen,
                                             &usSegOffset );
                     usBytes = usNewLen;
                   } /* endif  */
                 }
               } /* endif */
              } /* endif */

            } /* endif */
            pstPCmd->usLen1  = usBytes ;           // return data len
           }
           break;

     case EQF_GETPREFIXES:
       {
         /*************************************************************/
         /* Assumption: InsertProposal done during first search for   */
         /*             TM Proposal                                   */
         /*************************************************************/
         pBuffer = (PSZ_W)&pstPCmd->ucbBuffer[0];
         i = 1;
         while ( pDoc->stTWBS.pszPrefix[i] )
         {
           /*******************************************************/
           /* proposal available,                                 */
           /* check if we are dealing with Exact or others        */
           /*******************************************************/
           pData = UTF16strchr(pDoc->stTWBS.pszPrefix[i], pDoc->chStartFlag[0]);
           if ( !pData )
           {
             *pBuffer++ = 'e';
             *pBuffer++ = EOS;
           }
           else
           {
             pData++;
             while ( (*pData != pDoc->chEndFlag[0]) && (*pData) )
             {
               *pBuffer++ = *pData++;
             } /* endwhile */
             *pBuffer++ = EOS;
           } /* endif */

           /*********************************************************/
           /* next proposal                                         */
           /*********************************************************/
           i++;
         } /* endwhile */

         /***********************************************************/
         /* set end indication and length                           */
         /***********************************************************/
         *pBuffer++ = EOS;
         *pBuffer++ = EOS;

         pstPCmd->usLen1  = (USHORT)(pBuffer - (PSZ_W)&pstPCmd->ucbBuffer[0]);
        }
        break;



      default:
           if (ulParm <= EQF_NPROP_TGTS)
           {
			   ULONG  ulPropsInEqfSAB = 0;
               pszProp = NULL;
               ulLen = 0;

			 ulPropsInEqfSAB = (ULONG)(pDoc->stTWBS.usIndexInEqfSAB[(USHORT)ulParm]);
             if (ulPropsInEqfSAB == 0 && ulParm != 0 )
             {
				pstEQFGen->usRC   = EQFRS_ENTRY_NOT_AVAIL;
		     }
		     else
		     {
                pszProp = GetPropSZ (pDoc, ulPropsInEqfSAB , &ulLen); // fetch the proposal
		     }
             if (!pstEQFGen->usRC && pszProp )
             {
               /*******************************************************/
               /* check if it is the MT subsystem proposal            */
               /*******************************************************/
               if ( MachineTrans( pDoc, (USHORT)ulPropsInEqfSAB  ) & MACHINE_TRANS_PENDING )
               {
                 ulLen ++;
                 pstEQFSab = pDoc->stEQFSab + pDoc->usFI; // pointer to foreground
                 if ( pstEQFSab->fMTReady )
                 {
                   UTF16strcpy ((PSZ_W)pstPCmd->ucbBuffer, pstEQFSab->pucMTSeg );
                   pstPCmd->usLen1  = (USHORT)(UTF16strlenCHAR((PSZ_W)pstPCmd->ucbBuffer) + 1);
                   pstPCmd->ulParm1 = PropLevel (pDoc, ulPropsInEqfSAB );
                   pstPCmd->usParm2 = EqualFlag (pDoc, ulPropsInEqfSAB );
                 }
                 else
                 {
                   pstEQFGen->usRC   = EQFRS_ENTRY_NOT_AVAIL;
                 } /* endif */
                                          // get dictionary on top if visible
               }
               else
               {
                 ulLen ++;
                 UTF16strcpy ((PSZ_W)pstPCmd->ucbBuffer, pszProp );
                 pstPCmd->usLen1  = (USHORT)ulLen ;           // return data len
                 pstPCmd->ulParm1 = PropLevel (pDoc, ulPropsInEqfSAB );
                 pstPCmd->usParm2 = EqualFlag (pDoc, ulPropsInEqfSAB );

                 // copy any additional data of proposal
                 {
                   ULONG ulDataLen = 0;
                   PSZ_W pszPropData  = GetPropData( pDoc, ulPropsInEqfSAB , &ulDataLen ); // fetch the proposal
                   if ( (pszPropData != NULL) && (ulDataLen != 0) )
                   {
                     UTF16strcpy ((PSZ_W)(pstPCmd->ucbBuffer + (pstPCmd->usLen1 * sizeof(CHAR_W))), pszPropData );
                     pstPCmd->usLen2  = (USHORT)(ulDataLen + 1);
                   }
                   else
                   {
                     pstPCmd->usLen2  = 0;
                   } /* endif */                      
                 }

                 // set last used segment number
                 {
                   pstEQFSab = pDoc->stEQFSab + pDoc->usFI;  // pointer to foreground segm
                   if ( pDoc->fForeground && pstEQFSab->fbInUse && (ulPropsInEqfSAB  <= EQF_NPROP_TGTS) )
                   {
                     if ( ulParm == 0 )               // request for source segment
                     {
                       pDoc->ulLastSegNum = 0; // no segment number for proposal 0
                     }
                     else
                     {
                       pDoc->ulLastSegNum = pstEQFSab->ulSegNum[ulPropsInEqfSAB -1];
                     } /* endif */
                  } /* endif */
                 }
               } /* endif */
               if ( WinIsWindowVisible(pDoc->tbDevDictionary.tbDoc.hwndFrame))
               {
                   WinSetWindowPos ( pDoc->tbDevDictionary.tbDoc.hwndFrame,
                                     HWND_TOP,
                                     0, 0, 0, 0, EQF_SWP_SHOW | EQF_SWP_ZORDER);
               } /* endif */
             }
             else
             {
               pstEQFGen->usRC   = EQFRS_ENTRY_NOT_AVAIL;
             } // end if (get proposal)
           }
           else
           {
             pstEQFGen->usRC   = EQFRS_ENTRY_NOT_AVAIL;
           } // end if (check parameter)
    } // end switch (GETPROP parameter)
  }
  else                                          // ERROR: TM not running
  {
    pstEQFGen->usRC =   EQFRS_ENTRY_NOT_AVAIL;
  } // end if (check TM running)

  return;
} // end 'EQFR_GetProp'
/* $PAGEIF20 */

/*******************************************************************************
*
*       function:       EQFR_GetDict
*
*******************************************************************************/

VOID EQFR_GetDict
(
  PDOCUMENT_IDA  pDoc                              // pointer to document ida
)
{
  ULONG         ulParm;                         // parms
  PSZ_W         pszTgt;                         // ptr to dict.target
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  PSTEQFPCMD    pstPCmd;                        // pointer to pipe cmd struct
  ULONG         ulLen;                          // length of term
  PSTDICTWORD   pstDictWord;                    // pointer to dict term struct
  PSTEQFSAB     pstEQFSab;                      // pointer to send a head buffer

  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct

  pstEQFGen->usRC    = NO_ERROR;

  if ( pDoc->fRunDA )                           // dict thread is running
  {
    ulParm = pstPCmd->ulParm1;                  // GETDICT parameter
    switch (ulParm)
    {
    case EQF_SCROLL_UP:
         WinPostMsg (pDoc->hwndDictionary, WM_VSCROLL,
                        SB_LINEUP, MP2FROM2SHORT (0, 0));
         break;

    case EQF_SCROLL_DOWN:
         WinPostMsg (pDoc->hwndDictionary, WM_VSCROLL,
                        SB_LINEDOWN, MP2FROM2SHORT (0, 0));
         break;

    case EQF_ACTIVATE:
         WinPostMsg (pDoc->hwndDictionary, WM_EQF_PROCESSTASK,
                       MP1FROMSHORT(ACT_PROPDICT), NULL );
         break;

    case EQF_IS_AVAIL:
         if ( ! WinIsWindowVisible(pDoc->hwndDictionary))
         {
           pstEQFGen->usRC = EQFRS_NOT_AVAILABLE;
         } /* endif */
         break;

    case EQF_SCROLL_PREFIX_UP:
         if ( abs(pDoc->sDictPrefixPage) > 1 ) {   // Negative indicates at end of list
            pDoc->sDictPrefixPage = (SHORT)abs(pDoc->sDictPrefixPage) - 1 ;
            pstEQFSab = pDoc->stEQFSab + pDoc->usFI;
            WinPostMsg (pDoc->hwndDictionary, WM_EQF_PROCESSTASK,
                        MP1FROMSHORT( INSERT_DICTIONARY) , pstEQFSab );
         }
         break;

    case EQF_SCROLL_PREFIX_DOWN:
         if ( pDoc->sDictPrefixPage >= 0 )        // Negative indicates at end of list
            ++pDoc->sDictPrefixPage ; 
         pstEQFSab = pDoc->stEQFSab + pDoc->usFI;
         WinPostMsg (pDoc->hwndDictionary, WM_EQF_PROCESSTASK,
                     MP1FROMSHORT( INSERT_DICTIONARY) , pstEQFSab );
         break;


    default:
           if (ulParm <= EQF_NDICT_TGTS)
           {
              pstEQFSab = pDoc->stEQFSab + pDoc->usFI;
              if ( !pstEQFSab->fDAReady )
              {
                 pstEQFGen->usRC   =  EQFRS_ENTRY_NOT_AVAIL;
              }
              else
              {
                 ULONG    ulCurParm = ulParm ;
                 USHORT   usPrefixPerPage = 0 ;
                 USHORT   usPrefixPage = (USHORT) abs(pDoc->sDictPrefixPage) ;
                 USHORT   usCurPrefixPage = 1 ;

                 if ( usPrefixPage > 1 ) {            // Number of letters in a page
                    pszTgt = wcschr( pDoc->chDictPrefixes, L'*' ) ;
                    if ( pszTgt ) 
                       usPrefixPerPage = (SHORT)(pszTgt - pDoc->chDictPrefixes) ; 
                    else
                       usPrefixPerPage = ALL_LETTERS ;
                 }
                                                      // get the dict.target
                  pstDictWord = pDoc->stTWBS.stDictWord;
                  while ( pstDictWord->usType != 0 )  // data available
                  {
                     if ( pstDictWord->usType == TARGET
                               &&  pstDictWord->usNum == (USHORT)ulCurParm )
                     {
                        if ( usCurPrefixPage == usPrefixPage ) {
                           break;                     // Found entry on this page
                        }
                        ++usCurPrefixPage ;           // Look for entry on next page
                        ulCurParm += usPrefixPerPage ; 
                     } /* endif */
                     pstDictWord++;                    // point to next word
                  } /* endwhile */
                  if ( pstDictWord->usType != 0 )
                  {
                    //first char is Dictindicator; donot copy it
                    ulLen = pstDictWord->usLength-1;

                    pszTgt = (pDoc->stEQFSab+pDoc->usFI)->pucDictWords + pstDictWord->usOffset + 1;

                    // skip any style prefix
                    if ( (*pszTgt == STYLEPREFIX_NOTALLOWED) ||
                         (*pszTgt == STYLEPREFIX_UNDEFINED) ||
                         (*pszTgt == STYLEPREFIX_PREFERRED) )
                    {
                      ulLen--;
                      pszTgt++;
                    } /* endif */

                    memcpy ((PBYTE)pstPCmd->ucbBuffer, (PBYTE)pszTgt, ulLen * sizeof(CHAR_W));

                    pstPCmd->usLen1 = (USHORT)(ulLen + 2);       // returndata len

                    *(PUSHORT)&pstPCmd->ucbBuffer[ulLen*sizeof(CHAR_W)] = 0;
                  }
                  else
                  {
                    pstEQFGen->usRC   =  EQFRS_ENTRY_NOT_AVAIL;
                  } /* endif */
              } /* endif */
           }
           else
           {
             pstEQFGen->usRC   =  EQFRS_ENTRY_NOT_AVAIL;
           } // end if (check parameter)
           break;
    } // end switch (GETDICT parameter)
  }
  else                                          // ERROR: DA not running
  {
    pstEQFGen->usRC = (pstPCmd->ulParm1 == EQF_IS_AVAIL) ?
                                    EQFRS_NOT_AVAILABLE : EQFRS_ENTRY_NOT_AVAIL;
  } // end if (check TM running)

  return ;
} // end 'EQFR_GetDict'
/* $PAGEIF20 */

/*******************************************************************************
*
*       function:       EQFR_DictLook
*
*******************************************************************************/

VOID  EQFR_DictLook
(
  PDOCUMENT_IDA  pDoc                              // pointer to document ida
)
{
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  PSTEQFPCMD    pstPCmd;                        // pointer to pipe cmd struct

  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct

  WinSendMsg (pDoc->pstEQFGen->hwndTWBS,
              WM_EQF_PROCESSTASK,
              MP1FROMSHORT(DICTIONARY_LOOKUP),
              MP2FROMP( &pstPCmd->ucbBuffer[pstPCmd->usLen1] ));

  pstPCmd->usLen1  = pstPCmd->usLen2  = 0;

  return ;
} // end 'EQFR_DictLook'
/* $PAGE */
/*******************************************************************************
*
*       function:       EQFR_DictEdit
*
*******************************************************************************/

VOID  EQFR_DictEdit
(
  PDOCUMENT_IDA  pDoc                              // pointer to document ida
)
{
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  PSTEQFPCMD    pstPCmd;                        // pointer to pipe cmd struct

  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct

  WinSendMsg (pDoc->pstEQFGen->hwndTWBS,
              WM_EQF_PROCESSTASK,
              MP1FROMSHORT(DICTIONARY_EDIT),
              MP2FROMP( &pstPCmd->ucbBuffer[0] ));

  pstPCmd->usLen1  = pstPCmd->usLen2  = 0;

  return ;
} // end 'EQFR_DictEdit'
/* $PAGE */
/*******************************************************************************
*
*       function:       EQFR_SaveSeg
*
*******************************************************************************/

VOID  EQFR_SaveSeg
(
  PDOCUMENT_IDA  pDoc                              // pointer to document ida
)

{
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  PSTEQFPCMD    pstPCmd;                        // pointer to pipe cmd struct

  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct
  pstEQFGen->usRC    = NO_ERROR;

  if (pDoc->fRunTM)                          // TM running
  {
    PSZ_W pTemp;
    pDoc->stEQFQCmd.ulParm1       = pstPCmd->ulParm1; // segment number
    pTemp = pDoc->stEQFQCmd.pucSourceSeg  = (PSZ_W)pstPCmd->ucbBuffer;
    pDoc->stEQFQCmd.pucTargetSegs = pTemp + pstPCmd->usLen1;
    pDoc->stEQFQCmd.pszContext = NULL;
    pDoc->stEQFQCmd.pvMetaData = NULL;
    if ( pstPCmd->usCmd == EQFCMD_SAVESEG2W )
    {
      pstPCmd->usCmd = EQFCMD_SAVESEGW;
      if ( pstPCmd->usLen3 )
      {
        pDoc->stEQFQCmd.pszContext = pTemp + pstPCmd->usLen1 + pstPCmd->usLen2;
      }
      else
      {
        pDoc->stEQFQCmd.pszContext = NULL;
      } /* endif */

      if ( pstPCmd->usLen4 )
      {
        pDoc->stEQFQCmd.pszAddData = pTemp + (pstPCmd->usLen1 + pstPCmd->usLen2 + pstPCmd->usLen3);
      }
      else
      {
        pDoc->stEQFQCmd.pszAddData = NULL;
      } /* endif */
    } /* endif */

    pstPCmd->usLen1  = pstPCmd->usLen2  = 0;

    EQFTM (pDoc, pstPCmd->usCmd, &(pDoc->stEQFQCmd));       // SAVESEG to TM
  }
  else                                          // TM NOT running
  {
    pstEQFGen->usRC    = EQFRS_TM_NOT_ACTIVE;
  } // end if (TM running)


  return ;
} // end 'EQFR_SaveSeg'
/* $PAGE */
/*******************************************************************************
*
*       function:       EQFR_DumpSeg
*
*******************************************************************************/

VOID  EQFR_DumpSeg
(
  PDOCUMENT_IDA  pDoc                              // pointer to document ida
)

{
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  PSTEQFPCMD    pstPCmd;                        // pointer to pipe cmd struct
  PSTEQFSAB     pstEQFSab;                      // pointer to send a head buffer
  PSTEQFDUMP    pstEQFDump;                     // pointer to dump area
  PCHAR_W       pData;                          // pointer to data
  PCHAR_W       pSrc;                           // pointer to proposals
  USHORT        usI;                            // index
  USHORT        j  ;                            // index
  ULONG         ulLen;                          // length of proposal
  USHORT        usActSeg;                       // active segment
  PSZ           pFileName;                      // pointer to file name
  CHAR          szFileName[EQF_SNAMELEN+1];     // filename of file

  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct
  pstEQFGen->usRC    = NO_ERROR;
  pDoc->fsConfig = pstPCmd->usParm2;           // store configuration flags

  // find send a head buffer slot
  usActSeg = pDoc->usFI;               // save current active segment

  pDoc->usEI = (pDoc->usEI >= EQF_NSAB-1) ? 0 : ++pDoc->usEI;
  pDoc->usFI = pDoc->usEI;
  pstEQFSab = pDoc->stEQFSab+pDoc->usEI;

  // clear it and fill initial data
  ClearSAB( pstEQFSab );                    // clear send a head buffer
  memcpy ((PBYTE)pstEQFSab->pucSourceSeg,          // copy source seg. from
          (PBYTE)pstPCmd->ucbBuffer,               // pipe buffer to
          pstPCmd->usLen1 * sizeof(CHAR_W));// SAB
  pFileName  = (PSZ)( &pstPCmd->ucbBuffer[pstPCmd->usLen1*sizeof(CHAR_W)]);

  UTF16memset (pstEQFSab->pucTargetSegs, '\0', EQF_TGTLEN);
  UTF16memset (pstEQFSab->pucPropsSegs, '\0', EQF_TGTLEN);
  UTF16memset (pstEQFSab->pucPropAddData, '\0', EQF_TGTLEN);

  pstEQFSab->ulParm1 = pstPCmd->ulParm1;    // get segment number
  pstEQFSab->fbInUse = TRUE;                // indicate 'in use'

  pDoc->fForeground = TRUE;           // remember fg.segment present

  if ( pDoc->fRunDA )
  {
     // let thread a looooooooong time to process
     j = 0;

     while ( pDoc->fSemDAProc && j < DA_TIMEWAITS)
     {
       j++;
     } /* endwhile */

     pDoc->pstEQFQDA  = pstEQFSab;  // get dictentries for SAB
     pDoc->stTWBS.pstEQFSabDict  = pstEQFSab;
     pDoc->fSemDAProc = TRUE;

  } // end if (segment not present)

  if (pDoc->fRunTM)                          // TM running
  {

    pstPCmd->usLen1  = pstPCmd->usLen2  = 0;
    // copy passed file name temporarily to generic structure
    strcpy (szFileName, (PSZ)(pDoc->pstEQFGen->szFileName));
    strncpy( (PSZ)(pDoc->pstEQFGen->szFileName), pFileName, EQF_SNAMELEN );
    pDoc->pstEQFGen->szFileName[EQF_SNAMELEN] = EOS;     // null terminate
    EQFTM (pDoc, EQFCMD_TRANSSEGW, pstEQFSab);     // TRANSSEG to TM
    strcpy ((PSZ)(pDoc->pstEQFGen->szFileName), szFileName);

  } // end if (TM running)

  if ( pDoc->fRunDA && !pstEQFSab->fDAReady )
  {
     // let thread a looooooooong time to process
     j = 0;

     while (  pDoc->fSemDAProc && j < DA_TIMEWAITS )
     {
       j++;
     } /* endwhile */

  } /* endif */

 // copy source and dictionary data
 pstEQFDump = (PSTEQFDUMP) (pstPCmd + 1 );

 pData = pstEQFDump->chProp;           // pointer to data
 UTF16memset( pData, '\0', EQF_TGTLEN);
 pSrc  = pstEQFSab->pucTargetSegs;
 usI = 0;
 while ( usI < pstEQFSab->usPropCount )
 {
   *((PUSHORT) pData) = pstEQFSab->usLevel[ usI ];
   pData += sizeof( USHORT );
   *((PUSHORT) pData) = pstEQFSab->usNumExactBytes[ usI ];
   pData += sizeof( USHORT );
   ulLen = UTF16strlenCHAR( pSrc ) + 1;
   memcpy( (PBYTE)pData, (PBYTE)pSrc, ulLen*sizeof(CHAR_W)) ;
   pSrc += ulLen;
   pData += ulLen;
   usI++;
 } /* endwhile */

 if ( pDoc->fRunDA && pstEQFSab->fDAReady )
 {
    memcpy( (PBYTE)pstEQFDump->chDict, (PBYTE)pstEQFSab->pucDictWords,
             EQF_DICTLEN * sizeof(CHAR_W) );
 }
 else
 {
    UTF16memset( pstEQFDump->chDict, '\0', EQF_DICTLEN );  // init dict area
    // no data available - probably any error occured
    if ( pDoc->fRunDA && ! pstEQFGen->usRC  )
    {
       pstEQFGen->usRC = EQFS_DICT_START;
    } /* endif */
 } /* endif */

  pDoc->usFI = usActSeg;               // restore active segment
  return ;
} // end 'EQFR_DumpSeg'
/* $PAGE */
/*******************************************************************************
*
*       function:       EQFR_DelSeg
*
*******************************************************************************/

VOID  EQFR_DelSeg
(
  PDOCUMENT_IDA  pDoc                              // pointer to document ida
)
{
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  PSTEQFPCMD    pstPCmd;                        // pointer to pipe cmd struct
  PSTEQFSAB     pstEQFSab;                      // ptr to SAB element
  USHORT        usI;                            // index

  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct
  pstEQFGen->usRC    = NO_ERROR;

  if (pDoc->fRunTM)                          // TM running
  {
  PSZ_W pTemp;
    pDoc->stEQFQCmd.ulParm1       = pstPCmd->ulParm1; // segment number
    pTemp = pDoc->stEQFQCmd.pucSourceSeg  = (PSZ_W)pstPCmd->ucbBuffer;
    pDoc->stEQFQCmd.pucTargetSegs = pTemp + pstPCmd->usLen1;

    pstPCmd->usLen1  = pstPCmd->usLen2  = 0;

    EQFTM (pDoc, EQFCMD_DELSEGW, &(pDoc->stEQFQCmd));        // DELSEG to TM

    /******************************************************************/
    /* clear the deleted translation memory from buffer if available  */
    /******************************************************************/
    pstEQFSab = pDoc->stEQFSab;
    for (usI = 0; usI < EQF_NSAB; usI++)
    {
      if ( pstEQFSab->ulParm1 == pDoc->stEQFQCmd.ulParm1 )
      {
         ClearSAB( pstEQFSab );                 // clear send a head buffer
      } /* endif */
      pstEQFSab ++;                             // point to next one
    } // endfor
  }
  else                                          // TM NOT running
  {
    pstEQFGen->usRC    = EQFRS_TM_NOT_ACTIVE;
  } // end if (TM running)

  return ;
} // end 'EQFR_DelSeg'
/* $PAGE */

/*******************************************************************************
*
*       function:       EQFR_GetSegNum
*
*******************************************************************************/

VOID  EQFR_GetSegNum
(
  PDOCUMENT_IDA  pDoc                              // pointer to document ida
)
{
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  PSTEQFPCMD    pstPCmd;                        // pointer to pipe cmd struct

  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct
  pstEQFGen->usRC    = NO_ERROR;

  if (pDoc->fRunTM)                          // TM running
  {
    pDoc->stEQFQCmd.ulParm1       = pstPCmd->ulParm1; // segment number

    pstPCmd->ulParm1 = HIWORD( pDoc->ulLastSegNum );
    pstPCmd->usParm2 = LOWORD( pDoc->ulLastSegNum );
  }
  else                                          // TM NOT running
  {
    pstEQFGen->usRC    = EQFRS_TM_NOT_ACTIVE;
  } // end if (TM running)

  return ;
} // end 'EQFR_GetSegNum'
/* $PAGE */


/*******************************************************************************
*
*       function:       EQFR_ExtSeg
*
*******************************************************************************/

VOID  EQFR_ExtSeg
(
  PDOCUMENT_IDA  pDoc                              // pointer to document ida
)

{
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  PSTEQFPCMD    pstPCmd;                        // pointer to pipe cmd struct

  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct

  if (pDoc->fRunTM)                          // TM  running
  {
	pDoc->stEQFQCmd.ulParm1       = pstPCmd->ulParm1; // segment number

    pDoc->stEQFQCmd.pucSourceSeg  = (PSZ_W)pstPCmd->ucbBuffer; // output buffer

    pstPCmd->usLen1  = pstPCmd->usLen2  = 0;

    if (pDoc->fForeground &&
        (pDoc->stEQFSab + pDoc->usFI)->fbInUse &&
        pstPCmd->ulParm1 <= (pDoc->stEQFSab + pDoc->usFI)->usPropCount)
    {
      EQFTM (pDoc, EQFCMD_EXTSEGW, &(pDoc->stEQFQCmd));        // EXTSEG to TM
      pstPCmd->usLen1 = (USHORT)pDoc->stEQFQCmd.ulParm1 + 1;  // return length
    }
    else
    {
      pstEQFGen->usRC    = EQFRS_ENTRY_NOT_AVAIL;
    } // endif valid parms

  }
  else                                          // TM  NOT running
  {
    pstEQFGen->usRC    = EQFRS_TM_NOT_ACTIVE;
  } // end if (TM running)

  return;
} // end 'EQFR_ExtSeg'





/*//////////////////////////////////////////////////////////////
:h3.PropDictWndCreate
*///////////////////////////////////////////////////////////////
// Description:
// Flow: - create standard window
//       if create is ok:
//       - Get screen device context
//       - Get a micro PS for clipping
//       - Get an AVIO PS for editor use
//       - Get cell size of AVIO char
//       - Get length of video buffer
//       else: warning message
//
// Parameters:
//
////////////////////////////////////////////////////////////////
static
BOOL PropDictWndCreate
(
    PDOCUMENT_IDA  pDoc,                    // pointer to document structure
    USHORT         usType,                   // type of window (Proposal/Dict)
    USHORT         usTitle                   // id of title string
)
{
    static CHAR szLongName[MAX_LONGFILESPEC];
    BOOL  fOK = TRUE;                        // success indicator
    HAB       hab;                               // anchor block handle
    PSTEQFGEN     pstEQFGen;                  // pointer to generic struct
    ULONG flFrameFlags;
    PHWND   phwndType = NULL;                // handle of window
    PTWBSDEVICE pDevice = NULL;              // pointer to device data
    PTBDOCUMENT pTBDoc;                      // pointer to document struct
    PRECTL  prcl = NULL;                     // rectangle
    PSZ     pData = NULL;                    // pointer to error data
    USHORT  usDocType = 0;                   // type of window/document
    PVIOFONTCELLSIZE pVioFont;               // pointer to font
    USHORT  usWinId;                         // window resource id
    CHAR    chTemp[3];
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);


    pstEQFGen = pDoc->pstEQFGen;
    hab = (HAB) UtlQueryULong( QL_HAB );

    // get pDevice depending on tyoe of window
    switch ( usType)
    {
      case SERVDICT_DOC: pDevice = &(pDoc->tbDevDictionary); break;
      case SERVPROP_DOC: pDevice = &(pDoc->tbDevProposal); break;
      case SERVSOURCE_DOC: pDevice = &(pDoc->tbDevSource);  break;
    } /* endswitch */

    pTBDoc = &(pDevice->tbDoc);              // pointer to document structure
    pTBDoc->pstEQFGen = pstEQFGen;           // store pointer to generic struct
    EQFBGetUserSettings( pTBDoc );           // get access to user options
    pTBDoc->bOperatingSystem = (BYTE) UtlGetOperatingSystemInfo();

    switch ( usType)
    {
      case SERVDICT_DOC:
         phwndType = &(pDoc->hwndDictionary);
         prcl = &(pstEQFGen->rclDictionary);    // coordinates of dict.window
         WinLoadString (hab, hResMod, SID_NO_ENTRY,
                        128, pDoc->stTWBS.szNoDictEntry );

         /*************************************************************/
         /* convert string to OEM, because we assume everything is    */
         /* in OEM and convert it only during display....             */
         /*************************************************************/
         AnsiToOem( pDoc->stTWBS.szNoDictEntry, pDoc->stTWBS.szNoDictEntry );

         usDocType  = SERVDICT_DOC;             // dictionary window
         usWinId = ID_TWBS_DICT_WINDOW;
         /*************************************************************/
         /* set style bits of window to be created                    */
         /*************************************************************/
         if (pstEQFGen->flDictStyle )
         {
           flFrameFlags = (pstEQFGen->flDictStyle & AVAILSTYLES) | FCF_SIZEBORDER;
         }
         else
         {
           flFrameFlags = FCF_TITLEBAR | FCF_SIZEBORDER  | FCF_SYSMENU |
                          FCF_VERTSCROLL;
         } /* endif */
         break;
      case SERVPROP_DOC:
         /*************************************************************/
         /* get prefix area for proposals                             */
         /*************************************************************/

         WinLoadString (hab, hResMod, SID_START_FLAG, 2, chTemp );
         ASCII2Unicode( chTemp, &pDoc->chStartFlag[0], 0L);
         WinLoadString (hab, hResMod, SID_END_FLAG  , 2, chTemp );
         ASCII2Unicode( chTemp, &pDoc->chEndFlag[0], 0L);
         WinLoadString (hab, hResMod, SID_F_FLAG    , 2, chTemp );
         ASCII2Unicode( chTemp, &pDoc->chFFlag[0], 0L);
         WinLoadString (hab, hResMod, SID_R_FLAG    , 2, chTemp );
         ASCII2Unicode( chTemp, &pDoc->chRFlag[0], 0L);
         WinLoadString (hab, hResMod, SID_M_FLAG    , 2, chTemp );
         ASCII2Unicode( chTemp, &pDoc->chMFlag[0], 0L);

         phwndType = &(pDoc->hwndProposals);
         prcl     = &(pstEQFGen->rclProposals); // coordinates of prop.window
         usDocType  = SERVPROP_DOC;             // proposal window
         usWinId = ID_TWBS_PROP_WINDOW;
         /*************************************************************/
         /* set style bits of window to be created                    */
         /*************************************************************/
         if ( pstEQFGen->flPropStyle )
         {
           flFrameFlags = (pstEQFGen->flPropStyle & AVAILSTYLES) | FCF_SIZEBORDER;
         }
         else
         {
           flFrameFlags = FCF_TITLEBAR | FCF_SIZEBORDER  | FCF_SYSMENU |
                          FCF_VERTSCROLL  | FCF_HORZSCROLL;
         } /* endif */


         break;
      case SERVSOURCE_DOC:
         phwndType = &(pDoc->hwndSource);
         prcl     = &(pstEQFGen->rclSource); // coordinates of prop.window
         usDocType  = SERVSOURCE_DOC;        // source window for proposals
         usWinId = ID_TWBS_PROP_WINDOW;
         /*************************************************************/
         /* set style bits of window to be created                    */
         /*************************************************************/
         if ( pstEQFGen->flSrcStyle )
         {
           flFrameFlags = (pstEQFGen->flSrcStyle & AVAILSTYLES) | FCF_SIZEBORDER;
         }
         else
         {
           flFrameFlags = FCF_TITLEBAR | FCF_SIZEBORDER  | FCF_SYSMENU |
                          FCF_VERTSCROLL  | FCF_HORZSCROLL;
         } /* endif */
         break;
    } /* endswitch */

    // get the visible whitespaces in Unicode -- remember, they are stored in ANSI
    UtlGetUTF16VisibleWhiteSpace(pTBDoc, pTBDoc->pUserSettings, 0L);

    if (usType == SERVDICT_DOC)
    {
      PrepDictTitle( pDoc, pTBDoc->pUserSettings->fDispDictName );
    }
    else if (usType == SERVPROP_DOC)
    {
      PrepMemTitle( pDoc, pTBDoc->pUserSettings->fDispMemIndicator, usTitle );
    }
    else
    {
       WinLoadString (hab, hResMod, usTitle, EQF_MSGBUF_SIZE, pDoc->szBuf );
    } /* endif */

   strncpy( pTBDoc->chTitle, pDoc->szBuf, sizeof( pTBDoc->chTitle ));
   pTBDoc->chTitle[ sizeof(pTBDoc->chTitle)-1] = EOS;
   if ( pDoc->usEditor == RTFEDIT_EDITOR )
   {
      CHAR_W chTitleW[ sizeof(pTBDoc->chTitle)];
      flFrameFlags &= ~(FCF_VERTSCROLL | FCF_HORZSCROLL);

      ASCII2Unicode( pDoc->szBuf, chTitleW, 0L );
       pTBDoc->hwndClient = pTBDoc->hwndFrame =
               CreateWindowExW( WS_EX_RTLREADING, // | WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT,
                                TWBSRTF_W,    // window class name
                                &chTitleW[0], // pDoc->szBuf,        // window caption
                                WS_CLIPSIBLINGS |
                                WS_CHILD  |
                                flFrameFlags,       // window style
                                CW_USEDEFAULT,  // initial x position
                                CW_USEDEFAULT,  // initial y position
                                CW_USEDEFAULT,  // initial x size
                                CW_USEDEFAULT,  // initial y size
                                pDoc->hwnd,     // parent  handle
                                NULL,
                                (HINSTANCE)UtlQueryULong( QL_HAB ),
                                NULL            );    // creation parameters


      *phwndType = pTBDoc->hwndFrame;

     switch ( usType )
     {
       case SERVDICT_DOC:
         SendMessage( pTBDoc->hwndFrame, WM_SETICON, ICON_SMALL,
						(LONG) UtlQueryULong(QL_DICTLISTICON)); //(LONG)hiconDICTLIST );
         break;
       case SERVPROP_DOC:
         SendMessage( pTBDoc->hwndFrame, WM_SETICON, ICON_SMALL,
						(LONG) UtlQueryULong(QL_MEMICON)); //(LONG)hiconMEM );
         break;
//       case SERVSOURCE_DOC:
//         SendMessage( pTBDoc->hwndFrame, WM_SETICON, ICON_SMALL,
//						 (LONG) UtlQueryULong(QL_DOCICON)); //(LONG)hiconDoc );
         break;
       default:
         break;
     } /* endswitch */
   }
   else
   {
	 CHAR_W chTitleW[256];
     PSZ_W  pTitleW = ASCII2Unicode( pDoc->szBuf, chTitleW, 0L );

     pTBDoc->docType = (DOCTYPE)usType;              // dicitonary/proposal

     EQFBDocSetCodePage( pTBDoc, pDoc );

     pTBDoc->hwndClient = pTBDoc->hwndFrame =
                 CreateWindowExW ( 0, //WS_EX_RTLREADING, // | WS_EX_LEFTSCROLLBAR | WS_EX_RIGHT,
                                  TWBS_W,    // window class name
                                  pTitleW, //pDoc->szBuf,        // window caption
                                  WS_CLIPSIBLINGS |
                                  WS_CHILD  |
                                  flFrameFlags,       // window style
                                  CW_USEDEFAULT,  // initial x position
                                  CW_USEDEFAULT,  // initial y position
                                  CW_USEDEFAULT,  // initial x size
                                  CW_USEDEFAULT,  // initial y size
                                  pDoc->hwnd,     // parent  handle
                                  (HMENU)NULL,
                                  (HINSTANCE)UtlQueryULong( QL_HAB ),
                                  NULL            );    // creation parameters
     *phwndType = pTBDoc->hwndFrame;
   } /* endif */
   if ( pTBDoc->hwndFrame )
   {
     pTBDoc->usLangTypeTgt = MorphGetLanguageType( pDoc->szDocTargetLang );
     pTBDoc->usLangTypeSrc = MorphGetLanguageType( pDoc->szDocSourceLang );


      pDevice->pDoc = pDoc;                  // cross pointering
      pDevice->usType = usType;              // dicitonary/proposal
      pTBDoc->docType = (DOCTYPE)usType;              // dicitonary/proposal
      fOK = ( ANCHORWNDIDA( (HWND)(*phwndType), pDevice) &&
              ANCHORWNDIDA( pTBDoc->hwndFrame, pDevice));

      if ( fOK )
      {
        if ( pDoc->usEditor == RTFEDIT_EDITOR )
        {
          /***************************************************************/
          /* Create a richedit control for the window                    */
          /***************************************************************/
          fOK = EQFBCreateRichEditCtrl( pTBDoc, TWBSERVICEWPRTFCTRL  );
        } /* endif */
      } /* endif */
      if ( fOK )
      {
        pVioFont = get_vioFontSize();

        EQFBSetNewCellSize( pTBDoc,
                            (pVioFont + usDocType)->cx,
                            (pVioFont + usDocType)->cy );

        fOK = UtlAlloc( (PVOID *) &pTBDoc->pTokBuf, 0L, (LONG) TOK_BUFFER_SIZE,
                        ERROR_STORAGE );

      } /* endif */

      if ( fOK )                    // allocate buffer for marking area
      {
         fOK = UtlAlloc( (PVOID *) &pTBDoc->pBlockMark, 0L,
                         (LONG) max( sizeof( EQFBBLOCK ), MIN_ALLOC) ,
                         ERROR_STORAGE );
      } /* endif */

      if ( fOK )
      {
         fOK  = !TALoadTagTable( (PSZ)(pstEQFGen->szTagTable),
                                 (PLOADEDTABLE *) &pTBDoc->pDocTagTable,
                                 FALSE, FALSE );

         if ( !fOK )                // file could not be accessed
         {
            pData = (PSZ)(pstEQFGen->szTagTable);
            UtlError(ERROR_FILE_ACCESS_ERROR, MB_CANCEL,
                     1, &pData, EQF_ERROR);
         }
         else
         {
            /***************************************************************/
            /* check if special user exit handling is necessary,           */
            /***************************************************************/
            TALoadEditUserExit( pTBDoc->pDocTagTable,
                                (PSZ)(pstEQFGen->szTagTable), 
                                &pTBDoc->hModule,
                                (PFN*)&pTBDoc->pfnUserExit,
                                (PFN*)&pTBDoc->pfnCheckSegExit,
                                (PFN*)&pTBDoc->pfnShowTrans,
                                (PFN*)&pTBDoc->pfnTocGoto,
                                (PFN*)&pTBDoc->pfnGetSegContext,
                                NULL, 
                                (PFN*)&pTBDoc->pfnFormatContext, 
                                NULL,
                                (PFN*)&pTBDoc->pfnUserExitW,
                                (PFN*)&pTBDoc->pfnCheckSegExitW,
                                (PFN*)&pTBDoc->pfnCheckSegExExitW,
                                (PFN*)&pTBDoc->pfnCheckSegType );
         } /* endif */
      } /* endif */

      if ( fOK )
      {
          // allocate input buffer large enough to contain the fuzzy token table which
          // may exceed 40 KB
          fOK = UtlAlloc( (PVOID *) &pTBDoc->pInBuf, 0L, (LONG)max( IO_BUFFER_SIZE+10,40000),
                          ERROR_STORAGE );
      } /* endif */


      EQFBValidatePositions( prcl, usType ); // check if passed size info okay

      if ( fOK )
      {
         WinSetWindowPos (pTBDoc->hwndFrame,
                          HWND_TOP,
                          (SHORT)PRECTL_XLEFT(prcl),
                          (SHORT)PRECTL_YBOTTOM(prcl),
                          (SHORT)(PRECTL_XRIGHT(prcl) - PRECTL_XLEFT(prcl)),
                          (SHORT)(PRECTL_YTOP(prcl) - PRECTL_YBOTTOM(prcl)),
                          EQF_SWP_MOVE | EQF_SWP_SIZE);
      } /* endif */
      /****************************************************************/
      /* add specific handling necessary for dictionary and source of */
      /* proposal window ...                                          */
      /****************************************************************/
      switch ( usType )
      {
        case  SERVDICT_DOC:
          WinStartTimer( (HAB) UtlQueryULong( QL_HAB ), *phwndType,
                         DA_TIMERID, DA_TIMER);
          break;
        case  SERVSOURCE_DOC:
          EQFClearWindow ( pTBDoc, TRUE );
          break;
        default :
          break;
      } /* endswitch */
   }
   else
   {
      fOK = FALSE;
   } /* endif */
   return ( fOK );
}



/*
     TWBS     Instance Window Proc
*/

MRESULT APIENTRY TWBSERVICEWPRTF ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
    MRESULT       mResult = FALSE;  //result value of window proc
    PTWBSDEVICE   pDevice;          // pointer to document ida
    PTBDOCUMENT   pTBDoc;              // pointer to device data

    switch ( msg )
    {
      case WM_ERASEBKGND:
        return (LRESULT) 1;
        break;
      case WM_EQF_FONTCHANGED:
        pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
        pTBDoc = &(pDevice->tbDoc);    // pointer to document struct
        if ( pTBDoc->pDispFileRTF )
        {
          pTBDoc->pDispFileRTF->pHeader = NULL;  // reset fonts used
          EQFBDisplayFileNewRTF( pTBDoc );
          InvalidateRect( pTBDoc->hwndRichEdit, NULL, FALSE );
        } /* endif */
        break;
      case WM_NOTIFY:
        switch ( ((NMHDR *) mp2)->code )
        {
          case EN_PROTECTED:
            {
              ENPROTECTED *penProtected = (ENPROTECTED *) mp2;
              pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
              pTBDoc = &(pDevice->tbDoc);    // pointer to document struct

              // allow change of protected attribute
              if ( pTBDoc->pDispFileRTF->bRTFFill & (RTF_FILL | RTF_INITFILL) )
              {
                return 0;
              }
              else
              if(penProtected->msg == EM_SETCHARFORMAT &&
                 ((CHARFORMAT2 *) penProtected->lParam)->dwMask & CFM_PROTECTED)
              {
                return 0;
              } /* endif */
            }
            break;
          case EN_SELCHANGE:
            pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
            pTBDoc = &(pDevice->tbDoc);    // pointer to document struct
            if ( pTBDoc->pDispFileRTF->bRTFFill & (RTF_FILL | RTF_INITFILL) )
            {
              return 0;
            }
            else
            {
              SELCHANGE *pSelChange = (SELCHANGE *) mp2;
              CHARRANGE chrg = pSelChange->chrg;
              if ( chrg.cpMin == chrg.cpMax )
              {
                EQFBFuncMarkClear( pTBDoc );
              }
              else
              {
                PEQFBBLOCK   pstBlock = (PEQFBBLOCK)pTBDoc->pBlockMark;
                TBROWOFFSET  tbCursor;

                memcpy( &tbCursor, &pTBDoc->TBCursor, sizeof( tbCursor ));
                if (pstBlock->pDoc == NULL)           // no mark yet
                {
                  EQFBGetSegFromCaretRTF( pTBDoc, &pTBDoc->TBCursor, chrg.cpMin );
                  EQFBFuncMarkBlock( pTBDoc );
                }

                EQFBGetSegFromCaretRTF( pTBDoc, &pTBDoc->TBCursor, chrg.cpMax-1 );
                EQFBFuncMarkBlock( pTBDoc );
                memcpy( &pTBDoc->TBCursor, &tbCursor, sizeof( tbCursor ));
              }
            } /* endif */
            break;

          case EN_MSGFILTER:
            {
              MSGFILTER *     pmsgFilter = (MSGFILTER *) mp2;
              pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
              pTBDoc = &(pDevice->tbDoc);    // pointer to document struct
              mResult = HandleMessageTWBRTF( hwnd, pTBDoc, pmsgFilter->msg,
                                       pmsgFilter->wParam, pmsgFilter->lParam);
            }
            break;

          default:
            break;

        } /* endswitch */
        break;
      case WM_SIZE:
        pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
        pTBDoc = &(pDevice->tbDoc);    // pointer to document struct
        mResult = TWBSERVICEWP( hwnd, msg, mp1, mp2 );
        if ( pTBDoc && pTBDoc->hwndRichEdit )
        {
          BringWindowToTop( pTBDoc->hwndRichEdit );
	    }
        break;
      case WM_PAINT:
        mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );
        break;
      case WM_SETFOCUS:
      case WM_EQF_SETFOCUS:
        pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
        pTBDoc = &(pDevice->tbDoc);    // pointer to document struct

        if ( pTBDoc && pTBDoc->hwndRichEdit )
        {
          BringWindowToTop( pTBDoc->hwndRichEdit );
          WinSetFocus( HWND_DESKTOP, pTBDoc->hwndRichEdit );
        }
        break;
      case WM_ACTIVATE:
        WinSetFocus( HWND_DESKTOP, hwnd );
        break;
//      case WM_MOUSEACTIVATE:
//        WinSetFocus( HWND_DESKTOP, hwnd );
//        break;

      case WM_MOUSEACTIVATE:
        /********************************************************/
        /* inform TENV that the active window has changed.....  */
        /********************************************************/
        pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
        pTBDoc = &(pDevice->tbDoc);    // pointer to document struct
        if ( (hwnd != GetFocus()) && (pTBDoc->pstEQFGen ))
        {
          WinSendMsg( ((PSTEQFGEN)pTBDoc->pstEQFGen)->hwndTWBS,
                      WM_EQF_SETFOCUS,
                      0, MP2FROMHWND( hwnd ));
          BringWindowToTop( hwnd );
        } /* endif */
        mResult = MA_ACTIVATE;  // ANDEAT;
        break;

      default:
        /**************************************************************/
        /* let our standard wndproc take care                         */
        /**************************************************************/
        mResult = TWBSERVICEWP( hwnd, msg, mp1, mp2 );
        break;
    } /* endswitch */
    return( mResult );
}



MRESULT HandleMessageTWBRTF( HWND hwnd, PTBDOCUMENT pDoc, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
  MRESULT mResult = FALSE;
  ULONG ulCaret;
  POINTL point;

  switch( msg )
  {
    case WM_BUTTON1DOWN:
      point.x = LOWORD( mp2 );  point.y = HIWORD( mp2 );
      ulCaret = SendMessage( pDoc->hwndRichEdit, EM_CHARFROMPOS, 0L, (LONG)&point );
      EQFBGetSegFromCaretRTF( pDoc, &pDoc->TBCursor, ulCaret );
      break;

    case WM_KEYDOWN:
      if ( mp1 == VK_F1 )
      {
        /**************************************************************/
        /* Trigger help by posting HM_HELPSUBITEM_NOT_FOUND to TWB    */
        /**************************************************************/
        USHORT usID;
        switch ( pDoc->docType )
        {
           case SSOURCE_DOC:
             usID = ID_TWBS_ORIG_WINDOW;
             break;
           case STARGET_DOC:
             usID = ID_TWBS_SOURCE_WINDOW;
             break;
           case SERVDICT_DOC:
             usID = ID_TWBS_DICT_WINDOW;
             break;
           case SERVPROP_DOC:
           case SERVSOURCE_DOC:
             usID = ID_TWBS_PROP_WINDOW;
             break;
           case TRNOTE_DOC:
             usID = ID_TWBS_TRNOTE_WINDOW;
             break;
           default:
           case OTHER_DOC:
             usID = ID_TWBS_OTHER_WINDOW;
             break;
        } /* endswitch */
        PostMessage( (HWND)UtlQueryULong( QL_TWBFRAME ),
                     HM_HELPSUBITEM_NOT_FOUND,
                     0,
                     MP2FROM2SHORT( usID, usID ));
        break;
      } /* endif */
    case WM_SYSCHAR:
    case WM_SYSKEYDOWN:
    case WM_CHAR:           // determine character and pass it to editor
      mResult = HandleWMCharr( hwnd, (USHORT)msg, mp1, mp2 );
      break;
    default:
      break;
  } /* end switch */
  return mResult;
}


MRESULT APIENTRY TWBSERVICEWPRTFCTRL ( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
    MRESULT       mResult = FALSE;  //result value of window proc
    PTWBSDEVICE   pDevice;          // pointer to document ida
    PTBDOCUMENT   pTBDoc;              // pointer to device data

    pDevice = ACCESSWNDIDA( GetParent(hwnd), PTWBSDEVICE );
    pTBDoc = &(pDevice->tbDoc);    // pointer to document struct

    if ( pTBDoc )
    {
      switch ( msg )
      {
        case WM_KILLFOCUS:
          mResult = CALLWINDOWPROC( pTBDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
          WinSendMsg( GetParent(hwnd), WM_NCACTIVATE, FALSE, NULL );
          break;
        case WM_EQF_SETFOCUS:
        case WM_SETFOCUS:
          mResult = CALLWINDOWPROC( pTBDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
          WinSendMsg( GetParent(hwnd), WM_NCACTIVATE, TRUE, NULL );
          break;

        case WM_ERASEBKGND:
          return (LRESULT) 1;
          break;
        case WM_RBUTTONDOWN:
        case WM_BUTTON1DBLCLK:          // display dict dialog or TM window
          pTBDoc->pDispFileRTF->fTBCursorReCalc = TRUE;
          EQFBUpdateTBCursor( pTBDoc );
          mResult = TWBSERVICEWP( GetParent(hwnd), msg, mp1, mp2 );
          break;
        case WM_PAINT:
          mResult = CALLWINDOWPROC( pTBDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
          DrawFormatChars( pTBDoc );
          break;

        default:
          if ( pTBDoc )
            mResult = CALLWINDOWPROC( pTBDoc->pDispFileRTF->orgRichEditProc, hwnd, msg, mp1, mp2 );
          break;
      } /* endswitch */
    } /* endif */
    return( mResult );
}


MRESULT APIENTRY TWBSERVICEWP
( HWND hwnd, WINMSG msg, WPARAM mp1, LPARAM mp2)
{
    MRESULT       mResult = FALSE;  //result value of window proc
    HWND          hwndOrder;        // used for z-order setting
    HWND          hwndBehind;       // used for z-order setting
    PTWBSDEVICE   pDevice;          // pointer to document ida
    PTBDOCUMENT   pTBDoc;           // pointer to device data
    PDOCUMENT_IDA pDoc;             // pointer to document ida
    PSTEQFSAB     pstEQFSab;        // pointer to send ahead buffer
    ULONG         ulOldRow;         // row
    ULONG         ulOldCol;            //  and column

    switch( msg )
    {
       //---------------------------------------------------------------------
       case WM_CLOSE:
            pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
            switch ( pDevice->usType )
            {
               case SERVSOURCE_DOC:
                  pDoc = (PDOCUMENT_IDA)pDevice->pDoc;                    // pointer to document ida
                    /*****************************************************/
                    /* change input focus to proposal window             */
                    /*****************************************************/
                    SendMessage(pDoc->pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                                0, MP2FROMHWND( pDoc->hwndProposals ) );
                  pTBDoc = &(pDevice->tbDoc);    // pointer to document struct
                  WinShowWindow( pTBDoc->hwndFrame , FALSE );
                  break;
               default :
                  break;
            } /* endswitch */
          break;
     //----------------------------------------------------------------------
       case WM_INITMENU:
          pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
          switch ( pDevice->usType )
          {
               case SERVDICT_DOC:
               case SERVPROP_DOC:
                {
                  HMENU hSysMenu;         // handle of system menu

                  hSysMenu = GetSystemMenu( hwnd, FALSE );
                  if ( hSysMenu != NULL )
                  {
                    CHAR chText[80];
                    ULONG ulNum;
                    HMENU hSysParent = GetSystemMenu( GETPARENT(hwnd), FALSE);

                    chText[0] = EOS;
                    GetMenuString( hSysParent, SC_CLOSE,
                                   chText, sizeof( chText ), MF_BYCOMMAND);
                    if ( chText[0] )
                    {
                      ModifyMenu( hSysMenu, SC_CLOSE, MF_BYCOMMAND, SC_CLOSE,
                                  chText );
                    } /* endif */
                    RemoveMenu( hSysMenu, SC_TASKLIST, MF_BYCOMMAND );
                    /*******************************************************/
                    /* remove separator line - separator id = 0            */
                    /*******************************************************/
                    ulNum = GetMenuItemCount( hSysMenu );
                    if ( (ulNum > 1) &&  !GetMenuItemID( hSysMenu, ulNum-1)  )
                    {
                      RemoveMenu( hSysMenu, ulNum-1, MF_BYPOSITION );
                    } /* endif */
                    EnableMenuItem( hSysMenu, SC_CLOSE, MF_GRAYED );
                    EnableMenuItem( hSysMenu, SC_MINIMIZE, MF_GRAYED );
                    EnableMenuItem( hSysMenu, SC_MAXIMIZE, MF_GRAYED );
                    EnableMenuItem( hSysMenu, SC_RESTORE, MF_GRAYED );

                  } /* endif */
                }
                  break;
                case SERVSOURCE_DOC:
                 {
                    HMENU hSysMenu;         // handle of system menu

                    hSysMenu = GetSystemMenu( hwnd, FALSE );
                    if ( hSysMenu != NULL )
                    {
                      CHAR chText[80];
                      ULONG ulNum;
                      HMENU hSysParent = GetSystemMenu( GETPARENT(hwnd), FALSE);

                      chText[0] = EOS;
                      GetMenuString( hSysParent, SC_CLOSE,
                                     chText, sizeof( chText ), MF_BYCOMMAND);
                      if ( chText[0] )
                      {
                        ModifyMenu( hSysMenu, SC_CLOSE, MF_BYCOMMAND, SC_CLOSE,
                                    chText );
                      } /* endif */
                      RemoveMenu( hSysMenu, SC_TASKLIST, MF_BYCOMMAND );
                      /*******************************************************/
                      /* remove separator line - separator id = 0            */
                      /*******************************************************/
                      ulNum = GetMenuItemCount( hSysMenu );
                      if ( (ulNum > 1) &&  !GetMenuItemID( hSysMenu, ulNum-1)  )
                      {
                        RemoveMenu( hSysMenu, ulNum-1, MF_BYPOSITION );
                      } /* endif */
                      EnableMenuItem( hSysMenu, SC_MINIMIZE, MF_GRAYED );
                      EnableMenuItem( hSysMenu, SC_MAXIMIZE, MF_GRAYED );
                      EnableMenuItem( hSysMenu, SC_RESTORE, MF_GRAYED );
                    } /* endif */
                  }
                  break;
               default :
                  break;
          } /* endswitch */
            mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );
            break;

       case WM_EQF_INITMENU:
          /************************************************************/
          /* it is only our popup...                                  */
          /************************************************************/
          break;
       case WM_INITMENUPOPUP:
          /************************************************************/
          /* pass on focus and request to editor window ...           */
          /************************************************************/
          pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
          pDoc = (PDOCUMENT_IDA)pDevice->pDoc;
          SendMessage(pDoc->pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                      0, MP2FROMHWND( pDoc->pstEQFGen->hwndEditorTgt ) );
          PostMessage( pDoc->pstEQFGen->hwndEditorTgt, msg, mp1, mp2 );
          break;
     //----------------------------------------------------------------------
       case WM_MOVE:
          HandleWMMove( hwnd );
          mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );
          break;

       case WM_COMMAND:
          pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
          switch ( SHORT1FROMMP1(mp1) )
          {
            case IDM_PRINT:
              EQFBDocPrint( &pDevice->tbDoc );   // Print current document
              break;
            case IDM_DICTLOOK:
              StartDictLookUp( pDevice, DICTIONARY_LOOKUP );
              break;
            case IDM_EDITTERM:
              StartDictLookUp( pDevice, DICTIONARY_EDIT );
              break;
            case IDM_SRCPROPWND:
              pDoc = (PDOCUMENT_IDA)pDevice->pDoc;   // get pointer to doc ida
              InsertSource( &(pDoc->tbDevSource), TRUE, TRUE );
              break;

            case IDM_SEGPROPWND:
              {
                PTBDOCUMENT pStartDoc = NULL;
                PTBDOCUMENT pTargetDoc = NULL;

                pStartDoc = pTargetDoc = &(pDevice->tbDoc);
                while ( pTargetDoc->docType != STARGET_DOC )
                {
                  if ( pTargetDoc->next )
                  {
                    pTargetDoc = pTargetDoc->next;
                    if ( pTargetDoc == pStartDoc )
                    {
                      break;   // avoid wrap-around
                    } /* endif */
                  }
                  else
                  {
                    break; // no next document
                  } /* endif */
                } /*endwhile */

                // show dialog
                MDStartDialog( pTargetDoc );

                // refresh with data from current segment
                {
                  PTBSEGMENT pSeg = EQFBGetSegW( pTargetDoc, pTargetDoc->tbActSeg.ulSegNum );
                  if ( pSeg )
                  {
                    PSZ_W pszContext = EQFBGetContext( pTargetDoc, pSeg, pTargetDoc->tbActSeg.ulSegNum );
                    pTargetDoc->szContextBuffer[0] = 0;

                    if ( (pszContext != NULL) && (*pszContext != 0) && (pTargetDoc->pfnFormatContext != NULL) )
                    {
                      (pTargetDoc->pfnFormatContext)( pszContext, pTargetDoc->szContextBuffer );
                    } /* endif */             
                    MDRefreshMetadata( pTargetDoc, pSeg, pTargetDoc->szContextBuffer );
                  } /* endif */
                }
              }
              break;

            case IDM_UNMARK:
              EQFBFuncMarkClear( &(pDevice->tbDoc) );      // unmark
              break;
            case IDM_COPYPROPBLOCK:
                pDoc = (PDOCUMENT_IDA)pDevice->pDoc;            // pointer to document ida
                  /**************************************************************/
                  /* change input focus to editor window                        */
                  /**************************************************************/
                  SendMessage(pDoc->pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                              0, MP2FROMHWND( pDoc->pstEQFGen->hwndEditorTgt ) );
               WinPostMsg( pDoc->pstEQFGen->hwndEditorTgt, msg, mp1, mp2 );
              break;

            case IDM_PROTECTED:                  // switch to protected mode
               EQFBChangeStyle( &(pDevice->tbDoc),  DISP_PROTECTED);
               pDevice->tbDoc.pUserSettings->DispTM  = DISP_PROTECTED;
               EQFBWriteProfile(&(pDevice->tbDoc));
               if ( pDevice->tbDoc.hwndRichEdit )
               {
                 SendMessage(pDevice->tbDoc.hwndClient,WM_EQF_FONTCHANGED,0L,0L);
               }
               else
               {
                 pDevice->tbDoc.Redraw = REDRAW_ALL;
                 EQFBRefreshScreen( &(pDevice->tbDoc) ); // refresh the screen
               } /* endif */
               break;
            case IDM_HIDE:                       // switch to hided mode
               EQFBChangeStyle( &(pDevice->tbDoc),  DISP_HIDE);
               pDevice->tbDoc.pUserSettings->DispTM  = DISP_HIDE;
               EQFBWriteProfile(&(pDevice->tbDoc));
               if ( pDevice->tbDoc.hwndRichEdit )
               {
                 SendMessage(pDevice->tbDoc.hwndClient,WM_EQF_FONTCHANGED,0L,0L);
               }
               else
               {
                 pDevice->tbDoc.Redraw = REDRAW_ALL;
                 EQFBRefreshScreen( &(pDevice->tbDoc) ); // refresh the screen
               } /* endif */
               break;
            case IDM_COMPACT:                    // switch to compact style
               EQFBChangeStyle( &(pDevice->tbDoc),  DISP_COMPACT);
               pDevice->tbDoc.pUserSettings->DispTM  = DISP_COMPACT;
               EQFBWriteProfile(&(pDevice->tbDoc));
               if ( pDevice->tbDoc.hwndRichEdit )
               {
                 SendMessage(pDevice->tbDoc.hwndClient,WM_EQF_FONTCHANGED,0L,0L);
               }
               else
               {
                 pDevice->tbDoc.Redraw = REDRAW_ALL;
                 EQFBRefreshScreen( &(pDevice->tbDoc) ); // refresh the screen
               } /* endif */
               break;
             case IDM_SHORTEN:                    // switch to compact style
               EQFBChangeStyle( &(pDevice->tbDoc),  DISP_SHORTEN);
               pDevice->tbDoc.pUserSettings->DispTM  = DISP_SHORTEN;
               EQFBWriteProfile(&(pDevice->tbDoc));
               if ( pDevice->tbDoc.hwndRichEdit )
               {
                 SendMessage(pDevice->tbDoc.hwndClient,WM_EQF_FONTCHANGED,0L,0L);
               }
               else
               {
                 pDevice->tbDoc.Redraw = REDRAW_ALL;
                 EQFBRefreshScreen( &(pDevice->tbDoc) ); // refresh the screen
               } /* endif */
               break;

            case IDM_PROPDELETE:
              switch (  pDevice->usType )
              {
                case SERVPROP_DOC :
                  DeleteProposal( pDevice );
                  break;
                default :
                  WinAlarm( HWND_DESKTOP, WA_WARNING );
                  break;
              } /* endswitch */
              break;

            case IDM_SC_HORZ:
              EQFBSetResetFrameCtrls( &(pDevice->tbDoc), hwnd, FCF_HORZSCROLL );
              break;
            case IDM_SC_VERT:
              EQFBSetResetFrameCtrls( &(pDevice->tbDoc), hwnd, FCF_VERTSCROLL );
              break;
            case IDM_SC_TITLE:
              EQFBSetResetFrameCtrls( &(pDevice->tbDoc), hwnd,
                                      (FCF_TITLEBAR | FCF_SYSMENU));
              break;

            case PID_SYS_CLOSE:
               WinSendMsg( hwnd, WM_SYSCOMMAND, SC_CLOSE, NULL );

              break;
            case PID_SYS_SIZE:
              WinSendMsg( hwnd, WM_SYSCOMMAND, SC_SIZE, NULL );

              break;
            case PID_SYS_MOVE:
              WinSendMsg( hwnd, WM_SYSCOMMAND, SC_MOVE, NULL );

              break;
            default:
              mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );
              break;
          } /* endswitch */
          break;

       case WM_SIZE:
          HandleWMSize( hwnd, mp1, mp2 );
          mResult = DefWindowProc( hwnd, msg, mp1, mp2 );
          break;


       case WM_TIMER:
          pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
          pDoc = (PDOCUMENT_IDA) pDevice->pDoc;

          if ( pDoc->fRunDA )
          {
             pstEQFSab = pDoc->stEQFSab+pDoc->usFI;
             if ( pstEQFSab->fDAReady && pDoc->fDAInsert )
             {
                WinPostMsg (pDoc->hwndDictionary, WM_EQF_PROCESSTASK,
                            MP1FROMSHORT( INSERT_DICTIONARY),
                            ( pDoc->stEQFSab + pDoc->usFI ) );
                pDoc->fDAInsert = FALSE;
             } /* endif */
             // is thread ready
             if ( !  pDoc->fSemDAProc )
             {
                /******************************************************/
                /* check if dictionary data already available         */
                /******************************************************/
                if ( pstEQFSab->fDAReady )
                {
                   /***************************************************/
                   /* check if background segment should be processed */
                   /***************************************************/
                   pstEQFSab = pDoc->stEQFSab+pDoc->usEI;
                } /* endif */
                if ( !pstEQFSab->fDAReady )
                {
                   pDoc->pstEQFQDA  = pstEQFSab;  // get dictentries for SAB
                   pDoc->stTWBS.pstEQFSabDict  = pstEQFSab;
                   pDoc->fSemDAProc = TRUE;

                } /* endif */
             } /* endif */
          } /* endif */

          if ( pDoc->fRunMT )
          {
            /**********************************************************/
            /* check if insertion of MT proposal is pending ...       */
            /**********************************************************/
             pstEQFSab = pDoc->stEQFSab+pDoc->usFI;
             if ( pstEQFSab->fMTReady && pDoc->fMTInsert )
             {
                WinPostMsg (pDoc->hwndProposals, WM_EQF_PROCESSTASK,
                            MP1FROMSHORT( INSERT_PROPOSAL),
                            ( pDoc->stEQFSab + pDoc->usFI ) );
                pDoc->fMTInsert = FALSE;
             } /* endif */
             // is thread ready
             if ( !  pDoc->fSemMTProc )
             {
                /******************************************************/
                /* check if MT data are     already available         */
                /******************************************************/
                if ( pstEQFSab->fMTReady )
                {
                   /***************************************************/
                   /* check if background segment should be processed */
                   /***************************************************/
                   pstEQFSab = pDoc->stEQFSab+pDoc->usEI;
                } /* endif */
                if ( !pstEQFSab->fMTReady )
                {
                   pDoc->pstEQFQMT  = pstEQFSab;  // get dictentries for SAB
                   pDoc->fSemMTProc = TRUE;
                } /* endif */
             } /* endif */
          } /* endif */
          break;


       case WM_BUTTON1DBLCLK:          // display dict dialog or TM window
          RELEASECAPTURE;              // release the mouse
          pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );

          mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );

          switch ( pDevice->usType )
          {
             case SERVDICT_DOC:         // start dict lookup for dict terms
                pTBDoc = &(pDevice->tbDoc);               // get pointer to data

                ulOldRow = pTBDoc->mouseRow;    // temporary storage
                ulOldCol = pTBDoc->mouseCol;
                pTBDoc->mouseRow = (HIWORD( mp2 ) / pTBDoc->cy );
                pTBDoc->mouseCol = (LOWORD( mp2 ) / pTBDoc->cx );

                pTBDoc->lCursorRow = min( pTBDoc->mouseRow, pTBDoc->lScrnRows-1 ) ;
                pTBDoc->lCursorCol = pTBDoc->mouseCol;  // get new column position
                EQFBCurSegFromCursor( pTBDoc );       // get TBCursor

                if ( pTBDoc->TBCursor.ulSegNum == 0 ) // stay at old position
                {
                   WinAlarm( HWND_DESKTOP, WA_WARNING );
                   pTBDoc->lCursorRow = ulOldRow;
                   pTBDoc->lCursorCol = ulOldCol;
                   EQFBCurSegFromCursor( pTBDoc );    // get TBCursor
                }
                else
                {
                   EQFBScreenCursor( pTBDoc );      // position cursor and slider
                   StartDictLookUp( pDevice, DICTIONARY_LOOKUP );
                } /* endif */
                break;
             case SERVPROP_DOC:         // display source of proposals
               {
                 PTBSEGMENT pSeg;

                  pDoc = (PDOCUMENT_IDA)pDevice->pDoc;   // get pointer to doc ida

                  pTBDoc = &(pDevice->tbDoc);               // get pointer to data

  //                usOldRow = pTBDoc->mouseRow;    // temporary storage
  //                usOldCol = pTBDoc->mouseCol;
                  pTBDoc->mouseRow = (HIWORD( mp2 ) / pTBDoc->cy );
                  pTBDoc->mouseCol = (LOWORD( mp2 ) / pTBDoc->cx );

                  pTBDoc->lCursorRow = min( pTBDoc->mouseRow, pTBDoc->lScrnRows-1 ) ;
                  pTBDoc->lCursorCol = pTBDoc->mouseCol;  // get new column position
                  EQFBCurSegFromCursor( pTBDoc );       // get TBCursor

                  pSeg = EQFBGetSegW( pTBDoc, pTBDoc->TBCursor.ulSegNum );
                  if ( (pSeg != NULL) && (pSeg->qStatus == QF_DICTSTYLENOT) )
                  {
                    if ( pSeg->pvMetadata != NULL )
                    {
                      EQFBShowSegmentComment( hwnd, pTBDoc, pSeg->pvMetadata, LOWORD( mp2 ), HIWORD( mp2 ) );
                    } /* endif */
                  }
                  else if ( pTBDoc->TBCursor.ulSegNum == 1 )
                  {
                    // request display of all proposals (if avail)
                    USHORT bAllExactProps = pTBDoc->pUserSettings->UserOptFlags.bAllExactProposals;
                    pTBDoc->pUserSettings->UserOptFlags.bAllExactProposals = TRUE;
                    EQFRepaint( pDoc );
                    pTBDoc->pUserSettings->UserOptFlags.bAllExactProposals = bAllExactProps;
                  }
                  else
                  {
                    InsertSource( &(pDoc->tbDevSource), TRUE, TRUE );
                  } /* endif */
                }
                break;
             case SERVSOURCE_DOC:
               {
                  PTBSEGMENT pSeg;

                  pTBDoc = &(pDevice->tbDoc);    // pointer to document struct
                  WinShowWindow( pTBDoc->hwndFrame , FALSE );
                  pDoc = (PDOCUMENT_IDA)pDevice->pDoc;            // pointer to document ida
                  pTBDoc = &(pDevice->tbDoc);               // get pointer to data
                  pTBDoc->mouseRow = (HIWORD( mp2 ) / pTBDoc->cy );
                  pTBDoc->mouseCol = (LOWORD( mp2 ) / pTBDoc->cx );

                  pTBDoc->lCursorRow = min( pTBDoc->mouseRow, pTBDoc->lScrnRows-1 ) ;
                  pTBDoc->lCursorCol = pTBDoc->mouseCol;  // get new column position
                  EQFBCurSegFromCursor( pTBDoc );       // get TBCursor

                  pSeg = EQFBGetSegW( pTBDoc, pTBDoc->TBCursor.ulSegNum );
                  if ( (pSeg != NULL) && (pSeg->qStatus == QF_DICTSTYLENOT) )
                  {
                    if ( pSeg->pvMetadata != NULL )
                    {
                      EQFBShowSegmentComment( hwnd, pTBDoc, pSeg->pvMetadata, LOWORD( mp2 ), HIWORD( mp2 ) );
                    } /* endif */
                  }
                  else
                  {
                    /*****************************************************/
                    /* change input focus to proposal window             */
                    /*****************************************************/
                    SendMessage(pDoc->pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                                0, MP2FROMHWND(pDoc->pstEQFGen->hwndEditorSrc));
                  }
               }
                break;
             default :
                break;
          } /* endswitch */
          break;

        case WM_RBUTTONDOWN:
          /* Draw the "floating" popup in the app's client area */
          {
            RECT rc;
            SHORT sPopUpId;
            POINT pt;

            pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
            GetClientRect( hwnd, (LPRECT)&rc);
            pt.x = LOWORD(mp2);
            pt.y = HIWORD(mp2);
            if (PtInRect ((LPRECT)&rc, pt))
            {
              /********************************************************/
              /* find correct popup ...                               */
              /********************************************************/
              switch ( pDevice->usType )
              {
                case SERVPROP_DOC:     // proposal window
                  sPopUpId = ID_SRV_POPUP_TM;
                  break;
                case SERVDICT_DOC:     // dictionary window
                  sPopUpId = ID_SRV_POPUP_DCT;
                  break;
                case SERVSOURCE_DOC:   // source window for proposals
                  sPopUpId = ID_SRV_POPUP_SRCP;
                  break;
                default:
                  sPopUpId = 0;
                  break;
              } /* endswitch */
              if (sPopUpId)
              {
                HMENU hMenu;
                HMENU hMenuTrackPopup;
                ULONG ulFrameFlags;
                HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);


                /* Get the menu for the popup from the resource file. */
                hMenu = LoadMenu( hResMod, MAKEINTRESOURCE( ID_POPUP_MENU ) );
                if ( hMenu )
                {
                  PTBDOCUMENT pDoc = &(pDevice->tbDoc); // get pointer to data
                  POINT  point;
                  point.x = LOWORD(mp2);
                  point.y = HIWORD(mp2);

                  hMenuTrackPopup = GetSubMenu( hMenu, sPopUpId );

                  /********************************************************************/
                  /* set checkmarks for FrameCtrls...                                 */
                  /********************************************************************/
                  ulFrameFlags = GetWindowLong( hwnd, GWL_STYLE );

                  SETAABITEMCHECK( hMenuTrackPopup, IDM_SC_HORZ, (ulFrameFlags & WS_HSCROLL) );
                  SETAABITEMCHECK( hMenuTrackPopup, IDM_SC_VERT, (ulFrameFlags & WS_VSCROLL) );
                  SETAABITEMCHECK( hMenuTrackPopup, IDM_SC_TITLE, (ulFrameFlags & WS_CAPTION) );
                  if ( pDoc->pBlockMark )
                  {
                    SETAABITEM( hMenuTrackPopup, IDM_UNMARK,
                                ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc );
                    SETAABITEM( hMenuTrackPopup, IDM_COPYPROPBLOCK,
                                ((PEQFBBLOCK)pDoc->pBlockMark)->pDoc );
                  } /* endif */

                  /***********************************************************/
                  /* set IDM_PROPDELETE enabled/disabled depending on status */
                  /***********************************************************/
                  if ( pDevice->usType == SERVPROP_DOC  )
                  {
                    SETAABITEM( hMenuTrackPopup, IDM_PROPDELETE,
                           DeleteProposalAllowed((PDOCUMENT_IDA) pDevice->pDoc,
                                                  pDoc ));

                    /**************************************************/
                    /* we don't need a vertical scroll bar option     */
                    /**************************************************/
                    if ( pDevice->tbDoc.hwndRichEdit  )
                    {
                      RemoveMenu( hMenuTrackPopup, IDM_SC_VERT, MF_BYCOMMAND );
                    } /* endif */
                  } /* endif */

                  SETAABITEMCHECK( hMenuTrackPopup, IDM_PROTECTED,
                                   (pDevice->tbDoc.pUserSettings->DispTM == DISP_PROTECTED));
                  SETAABITEMCHECK( hMenuTrackPopup, IDM_HIDE,
                                   (pDevice->tbDoc.pUserSettings->DispTM == DISP_HIDE));
                  SETAABITEMCHECK( hMenuTrackPopup, IDM_COMPACT,
                                   (pDevice->tbDoc.pUserSettings->DispTM == DISP_COMPACT));
                              SETAABITEMCHECK( hMenuTrackPopup, IDM_SHORTEN,
                                   (pDevice->tbDoc.pUserSettings->DispTM == DISP_SHORTEN));

                  /* Convert the mouse point to screen coordinates since that is what
                   * TrackPopup expects.
                   */
                  ClientToScreen( hwnd, (LPPOINT)&point );

                  /* Draw and track the "floating" popup */
                  ((PDOCUMENT_IDA) pDevice->pDoc)->fTWBSPopup = TRUE;
                  TrackPopupMenu( hMenuTrackPopup, 0, point.x, point.y, 0,
                                  (HWND)UtlQueryULong( QL_TWBFRAME ), NULL );

                  /* Destroy the menu since were are done with it. */
                  DestroyMenu( hMenu );
                  ((PDOCUMENT_IDA) pDevice->pDoc)->fTWBSPopup = FALSE;
                } /* endif */
              } /* endif */
              mResult = FALSE;
            } /* endif */
          }
          break;

       case WM_BUTTON2DBLCLK:
          RELEASECAPTURE;                     // release the mouse
          pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
          mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );

          switch ( pDevice->usType )
          {
             case SERVDICT_DOC:         // start dict edit for dict terms
                pTBDoc = &(pDevice->tbDoc);     // get pointer to data

                ulOldRow = pTBDoc->mouseRow;    // temporary storage
                ulOldCol = pTBDoc->mouseCol;
                pTBDoc->mouseRow = (HIWORD( mp2 ) / pTBDoc->cy );
                pTBDoc->mouseCol = (LOWORD( mp2 ) / pTBDoc->cx );

                pTBDoc->lCursorRow = min( pTBDoc->mouseRow, pTBDoc->lScrnRows-1 ) ;
                pTBDoc->lCursorCol = pTBDoc->mouseCol;  // get new column position
                EQFBCurSegFromCursor( pTBDoc );       // get TBCursor
                if ( pTBDoc->TBCursor.ulSegNum == 0 ) // stay at old position
                {
                   WinAlarm( HWND_DESKTOP, WA_WARNING );
                   pTBDoc->lCursorRow = ulOldRow;
                   pTBDoc->lCursorCol = ulOldCol;
                   EQFBCurSegFromCursor( pTBDoc );    // get TBCursor
                }
                else
                {
                   EQFBScreenCursor( pTBDoc );      // position cursor and slider
                   StartDictLookUp( pDevice, DICTIONARY_EDIT );
                } /* endif */
                break;
             default :
                break;
          } /* endswitch */
          break;

      case WM_EQF_PROCESSTASK:
          pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
          pDoc = (PDOCUMENT_IDA) pDevice->pDoc;
          hwndOrder = pDevice->tbDoc.hwndFrame;// get frame handle
          switch ( SHORT1FROMMP1(mp1) )
          {
            case INSERT_PROPOSAL:            // insert the proposal entries
              InsertProposal ( pDevice, TRUE );
              if ( !WinIsWindowVisible( hwndOrder ))
              {
                 WinShowWindow ( hwndOrder, TRUE );
              } /* endif */
              /********************************************************/
              /* prepare update for transl.memory source window       */
              /* if explicitly asked for by the user ...              */
              /* do it always to cals usModWords                      */
              /********************************************************/
              pTBDoc = &(pDoc->tbDevSource.tbDoc);
              if ( pTBDoc->pUserSettings->fSrcPropWnd && pDoc->stTWBS.pszPrefix[0] )
              {
                InsertSource(&(pDoc->tbDevSource), FALSE, TRUE );
              } /* endif */
              break;

            case SHOW_PROPDICT:
              hwndOrder = pDevice->tbDoc.hwndFrame;     // get frame handle
              WinShowWindow (hwndOrder, SHORT1FROMMP2(mp2));
              break;

            case ACT_PROPDICT:
                 if (WinIsWindowVisible (hwnd))
                     /*****************************************************/
                     /* change input focus to proposal window             */
                     /*****************************************************/
                     SendMessage(pDoc->pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                                 0, MP2FROMHWND(hwnd) );
                 else
                   WinAlarm( HWND_DESKTOP, WA_WARNING );
                 break;


          case INSERT_DICTIONARY:          // insert the dictionary
              if ( pDoc->fRunDA )
              {
                 pstEQFSab = (PSTEQFSAB) mp2;   // get pointer to active sab
                                                // still the active one ???
                 if ( pstEQFSab == (pDoc->stEQFSab + pDoc->usFI) )
                 {
                    pstEQFSab = ( pDoc->stEQFSab + pDoc->usFI );
                    if ( pstEQFSab->usPropCount )  // check what to be top
                    {
                      hwndBehind = pDoc->hwndProposals;
                    }
                    else
                    {
                       hwndBehind = HWND_TOP;
                    } /* endif */

                    if ( pstEQFSab->fDAReady )
                    {
                          if ( !(pDoc->fsConfig & EQFF_NODICTWND)  )
                          {
                             InsertDictionary ( pDevice );
                             WinSetWindowPos (hwndOrder, hwndBehind, 0, 0, 0, 0,
                                              EQF_SWP_SHOW | EQF_SWP_ZORDER);
                             /*****************************************/
                             /* if dictionary in use, force a retry   */
                             /*****************************************/
                             if ( pstEQFSab->fDAReady & DICT_IN_USE )
                             {
                               EQFClearDictUpd( pDoc, FALSE );
                             } /* endif */
                          } /* endif */
                    }
                    else
                    {
                       /***********************************************/
                       /* postpone it for later try                   */
                       /***********************************************/
                       pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
                       if ( pDevice )
                       {
                          pDoc = (PDOCUMENT_IDA) pDevice->pDoc;
                          pDoc->fDAInsert = TRUE;
                       }
                       else
                       {
                          pDoc = NULL;
                       } /* endif */
                    } /* endif */
                 } /* endif */
              } /* endif */
              break;

            case ACT_SOURCEPROP:        // display source of proposal
              InsertSource( pDevice, TRUE, TRUE );
              break;

            default:
              break;
          } /* endswitch */
          break;

        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_SYSCHAR:
        case WM_SYSDEADCHAR:
        case WM_DEADCHAR:
            mResult = HandleWMCharr( hwnd, (USHORT)msg, mp1, mp2 );
            break;

        case WM_HSCROLL:
        case WM_VSCROLL:
            pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
            mResult = EQFBDispClass( hwnd, (USHORT)msg, mp1, mp2, &(pDevice->tbDoc) );
            EQFBScreenData( &(pDevice->tbDoc) );     // display screen
            EQFBScreenCursor( &(pDevice->tbDoc) );   // update cursor and sliders

            break;

        case WM_DESTROY:                  // free memory and resources
            CleanUp( hwnd ) ;
            break;

       default:
          pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
          if (pDevice)
          {
            mResult = EQFBDispClass( hwnd, (USHORT)msg, mp1, mp2,
                                     &(pDevice->tbDoc) );
          }
          else
          {

            mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );

          } /* endif */
          break;
    }/*end switch*/
    return( mResult );
}/*end TWBS_WP*/



//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFTMDAInit
//------------------------------------------------------------------------------
// Function call:     EQFTMDAInit( PDOCUMENT_IDA );
//------------------------------------------------------------------------------
// Description:       initialise the structures needed by services
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     Initialize SAB pointers to allocated SRC/TGT buffers
//------------------------------------------------------------------------------
static
VOID EQFTMDAInit
(
   PDOCUMENT_IDA pDoc
)
{
   PSTEQFSAB     pstEQFSab;                     // pointer to send a head buf.
   PSTEQFSRCDCT  pstEQFSrcDct;                  // pointer to dictionary source
   USHORT        i;

   pDoc->fForeground = FALSE;
   pDoc->usEI = pDoc->usFI = 0;
   /*-----------------------------------------------------------------------*
   * Initialize SAB pointers to allocated SRC/TGT buffers
   *-----------------------------------------------------------------------*/
   pstEQFSab = pDoc->stEQFSab;
   pstEQFSrcDct = pDoc->pstEQFSrcDct;
   memset( pstEQFSab, 0, sizeof( pDoc->stEQFSab ));
   for (i = 0; i < EQF_NSAB; i++)
   {
     pstEQFSab->pucSourceSeg  = pstEQFSrcDct->ucbSourceSeg;
     pstEQFSab->pucMTSeg      = pstEQFSrcDct->ucbMTSeg;
     pstEQFSab->pucDictWords  = pstEQFSrcDct->ucbDictWords;
     pstEQFSab->pszContext    = pstEQFSrcDct->szContext;
     pstEQFSab->pucTargetSegs = pDoc->pucEQFTgt;
     pstEQFSab->pucPropsSegs  = pDoc->pucEQFTgt + EQF_TGTLEN;
     // GQ: we have allocated a data area at the end of pucEQFTgt for additional data of the memory proposal
     pstEQFSab->pucPropAddData = pstEQFSab->pucPropsSegs + EQF_TGTLEN;
     pstEQFSab++;
     pstEQFSrcDct++;
   }

   return ;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     InsertProposal
//------------------------------------------------------------------------------
// Function call:     InsertProposal( PTWBSDEVICE );
//------------------------------------------------------------------------------
// Description:       Inserts the source segment and proposals (if existing)
//                    into the proposal area
//------------------------------------------------------------------------------
// Parameters:        PTWBSDEVICE        pointer to document device struct
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if foreground segment then
//                      clear window and free allocations for TM window
//                      init prefixes
//                      get first proposal string
//                      while proposals available and number of props <= 9
//                        get pointer to prefix
//                        if MT Subsystem and not exact mathc then
//                          insert subsystem proposal
//                        else
//                          prepare prefix ( fuzzy match and/or MT )
//                        endif
//                        add '-' to prefix
//                        display prefix via EQFBAddSeg function
//                        if source proposal display only first line,
//                          i.e. cut it to first line
//                        display proposal data via EQFBAddSeg function
//                        add a LF segment if not at end available
//                        get next proposal
//                      endwhile
//                      display the window
//                    endif
//                    return
//------------------------------------------------------------------------------
static
VOID InsertProposal
(
   PTWBSDEVICE  pDevice,               // pointer to device contexts
   BOOL         fDisplay               // display proposal
)
{
  PCHAR_W       pucSeg;
  ULONG         ulPropLen;
  USHORT        usPropsInEqfSAB;
  PSTEQFGEN     pstEQFGen;                         // pointer to generic struct
  PDOCUMENT_IDA pDoc;                              // pointer to document inst
  PTBDOCUMENT   pTBDoc;
  PSZ_W         pPrefix;                           // pointer to prefix
  ULONG         ulSegNum = 1;                      // segment number
  USHORT        usPropNum;                         // proposal number displayed
  SHORT         sPropLevel;                        // level of match
  SHORT         sMTLevel = 0;                      // level of MT match
  BOOL          fMTPending;                        // check if MT is necessec.
  PSTEQFSAB     pstEQFSab;                         // ptr. to SendAHead struct
  PSZ_W         pucSeg0;
  LONG          lMaxPrefixLen = 0;
  LONG          lCurLen = 0;
  USHORT        usI = 0;
  static CHAR szLongName[MAX_LONGFILESPEC];
//CHAR szShortName[MAX_FNAME];
  // fields for displaying the difference between machine and best fuzzy match
  USHORT        usFuzzyIndex = 0;                // index of best fuzzy proposal
  USHORT        usMachineIndex = 0;              // index of machine proposal
  PSZ_W         pszFuzzyProp = NULL;             // ptr to fuzzy proposal
  PSZ_W         pszMachineProp = NULL;           // ptr to machine proposal

  pDoc = (PDOCUMENT_IDA) pDevice->pDoc;

  if (pDoc->fForeground)
  {
    if ( !pDoc->ulSrcOemCP)
    {
      pDoc->ulSrcOemCP = GetLangOEMCP( pDoc->szDocSourceLang);
    }

    // initialize some fields
    fMTPending = pDoc->fRunMT;                     // set fMTPending accordingly
    pstEQFGen = pDoc->pstEQFGen;                   // pointer to gen.struct
    pTBDoc = &(pDevice->tbDoc);                    // pointer to document
    pTBDoc->DispStyle = (DISPSTYLE)pTBDoc->pUserSettings->DispTM; // select correct style
    if ( fDisplay )
    {
      EQFClearWindow ( pTBDoc, FALSE );              // free previous allocations
      memset(  &(pDoc->stTWBS.StartStopProp[0]), 0, sizeof(pDoc->stTWBS.StartStopProp));
    } /* endif */

    usI = sizeof(pDoc->stTWBS.usIndexInEqfSAB);
    memset( &(pDoc->stTWBS.usIndexInEqfSAB[0]), 0, sizeof(pDoc->stTWBS.usIndexInEqfSAB) );
    memset( (PBYTE)pDoc->stTWBS.szPrefix, '\0' , sizeof(pDoc->stTWBS.szPrefix));
    memset(  pDoc->stTWBS.pszPrefix, 0 , sizeof(pDoc->stTWBS.pszPrefix));
    pPrefix = pDoc->stTWBS.szPrefix;               // init array

    // check if we need to check for differences between machine and the best fuzzy match
    if ( pTBDoc->pUserSettings->fMachFuzzyDiff  )
    {
      USHORT usI = 0;
      ULONG ulLen = 0;

      // scan available proposals
      pucSeg0 = pucSeg = GetPropSZ ( (PDOCUMENT_IDA)(pDevice->pDoc), usI, &ulLen);
      while ( pucSeg && (usI <= 9) )
      {
        if (usI > 0)
        {
          SHORT sLevel = PropLevel( pDoc, usI );
          SHORT sMT    = MachineTrans( pDoc, usI );

          if ( sMT & MACHINE_TRANS_PROP )
          {
            // remember machine proposal
            if ( usMachineIndex == 0 ) // no machine proposal yet?  
            {
              usMachineIndex = usI;
              pszMachineProp = pucSeg;
            } /* endif */               
          } 
          else if ( (sMT & GLOBMEM_TRANS_PROP) || (sMT & GLOBMEMSTAR_TRANS_PROP)  )
          {
            // nothing to do for global memory proposals
          }
          else if ( sLevel < 100 )
          {
            if ( usFuzzyIndex == 0 ) // no fuzzy proposal yet?
            {
              if ( sLevel >= pTBDoc->pUserSettings->usFuzzyForDiv )
              {
                usFuzzyIndex = usI;
                pszFuzzyProp = pucSeg;
              } /* endif */                 
            } /* endif */               
          } /* endif */                     

        } /* endif */           
        usI++;
        pucSeg = GetPropSZ( (PDOCUMENT_IDA)(pDevice->pDoc), usI, &ulLen);
      } /* endwhile */   

      // reset fuzzy and machine proposal index if not both have been set
      if ( (usFuzzyIndex == 0) || (usMachineIndex == 0) )
      {
        usFuzzyIndex = 0; 
        usMachineIndex = 0;
        pszFuzzyProp = NULL;
        pszMachineProp = NULL;
      } /* endif */         
    } /* endif */       

    pucSeg = GetPropSZ ((PDOCUMENT_IDA)pDevice->pDoc, 0, &ulPropLen);
    pucSeg0 = pucSeg;
    usPropNum = usPropsInEqfSAB = 0;
    while ( pucSeg && (fMTPending || usPropNum <= 9))
    {
       CHAR chTemp[10];
       static CHAR_W chTempPrefix[1000]; 
       PSZ_W  pTempPrefix;

       memset( chTempPrefix, '\0' , sizeof(chTempPrefix));
       pTempPrefix = &chTempPrefix[0];
       pstEQFSab = pDoc->stEQFSab + pDoc->usFI; // pointer to foreground

       pDoc->stTWBS.pszPrefix[usPropNum] = pPrefix; // remember start of prefix
       itoa( usPropNum, chTemp, 10);
       ASCII2Unicode( chTemp, pPrefix, pDoc->ulSrcOemCP );
       if (usPropNum > 0)
       {
          sPropLevel = PropLevel (pDoc, usPropsInEqfSAB);   // get level of match
          sMTLevel   = MachineTrans( pDoc, usPropsInEqfSAB );

          /************************************************************/
          /* check if MT match should be inserted                     */
          /************************************************************/
          if ( fMTPending && ( sMTLevel & MACHINE_TRANS_PENDING))
          {
            fMTPending = FALSE;
            UTF16strcat (pPrefix, pDoc->chStartFlag);
            UTF16strcat (pPrefix, pDoc->chMFlag);
            UTF16strcat (pPrefix, pDoc->chEndFlag);

            if ( pstEQFSab->fMTReady )
            {
              pucSeg = pstEQFSab->pucMTSeg;        // addr. of source
            }
            else
            {
              pucSeg = CONT_STRING;                // continuation string
              pDoc->fMTInsert = TRUE;              // indicate displ.pending
            } /* endif */
            ulPropLen = UTF16strlenCHAR (pucSeg);
          }
          else
          { 
            /* calc. Fuzzyness always to retrieve it for calc.reports! */
      			BOOL    fOK = TRUE;

            fOK = EqfFillFuzzyness(pstEQFSab, usPropNum, pDoc, usPropsInEqfSAB, pTBDoc, pucSeg0, sMTLevel);

            // insert proposal if fuzzyness > fDisplayFuzzyLevel
            if (!pstEQFSab->fInvisible[usPropsInEqfSAB-1])
            {
				      /**********************************************************/
				      /* check if prefix is necessary                           */
				      /**********************************************************/
				      if ( (sMTLevel & ~(EXACT_PROP| FUZZY_PROP) ) ||
					      (sPropLevel < (SHORT) pstEQFGen->lExactMatchLevel) )
				      {
				        /********************************************************/
				        /* prefix start                                         */
				        /********************************************************/
				        UTF16strcat (pPrefix, pDoc->chStartFlag);
				        /**********************************************************/
				        /* indicate machine transl. flag...                       */
				        /**********************************************************/
				        if ( sMTLevel & MACHINE_TRANS_PROP )
				        {
					      UTF16strcat (pPrefix, pDoc->chMFlag);
					      sPropLevel ++;        // increase to set to exact (if applic.)
				        } /* endif */
				        if ( (sMTLevel & GLOBMEM_TRANS_PROP ) && (pstEQFSab->usFuzzyPercents[usPropsInEqfSAB-1] >= 100) ) 
				        {
					        UTF16strcat (pPrefix, L"h");
					        sPropLevel ++;        // increase to set to exact (if applic.)
				        } /* endif */
				        if ( (sMTLevel & GLOBMEMSTAR_TRANS_PROP) && (pstEQFSab->usFuzzyPercents[usPropsInEqfSAB-1] >= 100) )
				        {
					      UTF16strcat (pPrefix, L"h*");
					      sPropLevel ++;        // increase to set to exact (if applic.)
				        } /* endif */


				        if ( sMTLevel & XLIFF_PROP )
				        {
					      UTF16strcat (pPrefix, L"x" );
				        }

				        if ( sMTLevel & FUZZY_REPLACE_PROP )
				        {
					      UTF16strcat (pPrefix, pDoc->chRFlag);
					      UTF16strcat (pPrefix, pDoc->chFFlag);
				        }
				        else if ( sMTLevel & REPLACE_PROP )
				        {
					      UTF16strcat (pPrefix, pDoc->chRFlag);
				        }
				        else if (sPropLevel < (SHORT)pstEQFGen->lExactMatchLevel )
				        {
					      UTF16strcat (pPrefix, pDoc->chFFlag);
				        } /* endif */


                // show memory indicator
                if ( pTBDoc->pUserSettings->fDispMemIndicator )
					      {
                  CHAR_W szMemIndicator[5];
                  int iDBIndex = (int)(pDoc->stEQFSab + pDoc->usFI)->usDBIndex[usPropsInEqfSAB-1];
                  swprintf( szMemIndicator, L" [%C]", 'a' + iDBIndex ); 
                  UTF16strcat( pPrefix, szMemIndicator );
                } /* endif */                   

                // show memory name 
                if ( pTBDoc->pUserSettings->fDispMemName )
					      {
                  int iDBIndex = (int)(pDoc->stEQFSab + pDoc->usFI)->usDBIndex[usPropsInEqfSAB-1];

                  strcpy( szLongName, pDoc->szMemory[iDBIndex] );
                  OEMTOANSI( szLongName );
                  swprintf( pPrefix+UTF16strlenCHAR(pPrefix), L"[%S]", szLongName); 
                } /* endif */                   


				        /********************************************************/
				        /* add the filename of proposal as reference if request */
				        /********************************************************/
				        if ( pTBDoc->pUserSettings->fOriginProp )
				        {
                   PSZ_W pszDocName;
					          UTF16strcat( pTempPrefix, L" " );

                    // use system ASCII CP for document names
                    pszDocName = pTempPrefix + UTF16strlenCHAR(pTempPrefix);

					          ASCII2Unicode( &((pDoc->stEQFSab + pDoc->usFI)->szFName[usPropsInEqfSAB-1][0]), pszDocName, CP_OEMCP );

					      EQFAbbrevFileName ( pPrefix, lMaxPrefixLen, pTempPrefix);
				        } /* endif */
				        /********************************************************/
				        /* add date of proposal as reference if requested       */
				        /********************************************************/
				        if ( pTBDoc->pUserSettings->fDateOfProp )
				        {
					      CHAR_W szDateW[50];      // date converted to a character string
					      UTF16strcat( pPrefix, L" " );
					      UtlLongToDateStringW(
						      (pDoc->stEQFSab + pDoc->usFI)->lDate[usPropsInEqfSAB-1],
						      szDateW, sizeof( szDateW )/ sizeof(CHAR_W) );
					      UTF16strcat( pPrefix, &szDateW[0] );
				        } /* endif */

				        /********************************************************/
				        /* add the quality of proposal as percentage if request */
				        /********************************************************/
				        if (fOK && pTBDoc->pUserSettings->UserOptFlags.bDispPropQuality )
				        {
                  if ( (sMTLevel & MACHINE_TRANS_PROP) ||
                       (((sMTLevel & GLOBMEM_TRANS_PROP) || (sMTLevel & GLOBMEMSTAR_TRANS_PROP)) && (pstEQFSab->usFuzzyPercents[usPropsInEqfSAB-1] >= 100)) )
                  {
                    // no display of fuzziness for machine proposals and 100% global memory proposals
                  }
                  else
                  {
						          CHAR    chNum[10];
					            CHAR_W  chNumW[10];
					            UTF16strcat( pPrefix, L"  " );
					            itoa( pstEQFSab->usFuzzyPercents[usPropsInEqfSAB-1], chNum, 10);
					            ASCII2Unicode( chNum, &chNumW[0], pDoc->ulSrcOemCP );
					            UTF16strcat (pPrefix, chNumW);
					            UTF16strcat( pPrefix, L"%" );
  				          } /* endif */
				          } /* endif */
				        /********************************************************/
				        /* prefix end                                           */
				        /********************************************************/
				        UTF16strcat (pPrefix, pDoc->chEndFlag);
				      }
				      else
				      {
				        /********************************************************/
				        /* add the filename of proposal as reference if request */
				        /********************************************************/
				        if ( pTBDoc->pUserSettings->fOriginProp ||
					        pTBDoc->pUserSettings->fDateOfProp )
				        {
					      UTF16strcat (pPrefix, pDoc->chStartFlag);

                // show memory indicator
                if ( pTBDoc->pUserSettings->fDispMemIndicator )
					      {
                  CHAR_W szMemIndicator[5];
                  int iDBIndex = (int)(pDoc->stEQFSab + pDoc->usFI)->usDBIndex[usPropsInEqfSAB-1];
                  swprintf( szMemIndicator, L" [%C]", 'a' + iDBIndex ); 
                  UTF16strcat( pPrefix, szMemIndicator );
                } /* endif */                   

                // show memory name 
                if ( pTBDoc->pUserSettings->fDispMemName )
					      {
                  int iDBIndex = (int)(pDoc->stEQFSab + pDoc->usFI)->usDBIndex[usPropsInEqfSAB-1];

                  strcpy( szLongName, pDoc->szMemory[iDBIndex] );
                  OEMTOANSI( szLongName );
                  swprintf( pPrefix+UTF16strlenCHAR(pPrefix), L"[%S]", szLongName); 
                } /* endif */  

					      if ( pTBDoc->pUserSettings->fOriginProp )
					      {
                  PSZ_W pszDocName;

					        UTF16strcat( pTempPrefix, L" " );

                  // use system ASCII CP for document names
                  pszDocName = pTempPrefix + UTF16strlenCHAR(pTempPrefix);

					        ASCII2Unicode( &((pDoc->stEQFSab + pDoc->usFI)->szFName[usPropsInEqfSAB-1][0]),
										       pszDocName, CP_OEMCP /* pDoc->ulSrcOemCP */);

					        EQFAbbrevFileName ( pPrefix, lMaxPrefixLen, pTempPrefix);
					      } /* endif */
					      /********************************************************/
					      /* add date of proposal as reference if requested       */
					      /********************************************************/
					      if ( pTBDoc->pUserSettings->fDateOfProp )
					      {
					        CHAR_W szDateW[50];
					        if ( pTBDoc->pUserSettings->fOriginProp )
					        {
						      UTF16strcat( pPrefix, L" " );
					        } /* endif */
					        UtlLongToDateStringW(
						        (pDoc->stEQFSab + pDoc->usFI)->lDate[usPropsInEqfSAB-1],
						        szDateW, sizeof( szDateW )/sizeof(CHAR_W) );
					        UTF16strcat( pPrefix, &szDateW[0] );
					      } /* endif */
					      UTF16strcat (pPrefix, pDoc->chEndFlag);
				        } /* endif */
				      } /* endif */
            } /* endif  if not invisible*/
          } /* endif */
       }
       else
       {
         /*************************************************************/
         /* check if we have to add the number of available proposals */
         /* into the prefix string                                    */
         /*************************************************************/
         lMaxPrefixLen = sizeof(pDoc->stTWBS.szPrefix)/ sizeof(CHAR_W);
         if ( pTBDoc->pUserSettings->fNumProp )
         {
           CHAR  chNum[10];
           CHAR_W chNumW[10];

           /********************************************************/
           /* generate prefix for source as follows:               */
           /*   0[2]  where the number in brackets indicates the   */
           /*         number of props available...                 */
           /********************************************************/
           UTF16strcat (pPrefix, pDoc->chStartFlag);
           itoa( (pDoc->stEQFSab + pDoc->usFI)->usPropCount, chNum, 10);
           ASCII2Unicode( chNum, &chNumW[0], pDoc->ulSrcOemCP );
           UTF16strcat (pPrefix, &chNumW[0]);
           if ( (pDoc->stEQFSab+pDoc->usFI)->fsAvailFlags & GET_MORE_EXACTS_AVAIL )
           {
              UTF16strcat( pPrefix, MORE_EXACT_PROPS_STRING );
           }
           UTF16strcat (pPrefix, pDoc->chEndFlag);

           lCurLen = UTF16strlenCHAR(pPrefix);
           lMaxPrefixLen -= lCurLen + 10;
           if ((pDoc->stEQFSab + pDoc->usFI)->usPropCount > 0)
		   {
			  lMaxPrefixLen = lMaxPrefixLen / (pDoc->stEQFSab + pDoc->usFI)->usPropCount;
		   }
         } /* endif */
         lMaxPrefixLen -= 36;    // some bytes info around filename!
       } /* endif */

       if ((usPropNum == 0 ) ||
           (usPropNum > 0) && (!pstEQFSab->fInvisible[usPropsInEqfSAB-1]))
       {
		    pDoc->stTWBS.StartStopProp[ usPropNum ].usStartSeg = (USHORT)ulSegNum;
		    PrepPrefix( pTBDoc, &pPrefix, usPropNum, &ulSegNum, fDisplay );

		    if ( fDisplay )
		    {
          if ( (usPropNum != 0 ) && ContainsSegmentNote( pstEQFSab->pszSortPropsData[usPropsInEqfSAB-1] )  )
          {
            tbSegment.pDataW = szNoteIndicator;
	          tbSegment.usLength = (USHORT)UTF16strlenCHAR(tbSegment.pDataW) + 1;
	          tbSegment.qStatus = (USHORT)QF_DICTSTYLENOT;
	          tbSegment.ulSegNum = ulSegNum ++;
	          tbSegment.pusBPET = NULL;
	          tbSegment.pusHLType = NULL;
            tbSegment.pvMetadata = pstEQFSab->pszSortPropsData[usPropsInEqfSAB-1];
	          SetLineNum( pTBDoc, tbSegment.pDataW );
	          PrepAddSeg ( &tbSegment, &tbPrepSeg );
	          tbPrepSeg.SegFlags.NoReorder = FALSE;
	          EQFBAddSegW( pTBDoc, &tbPrepSeg );

          } /* endif */

          if ( pucSeg == pszFuzzyProp )
          {
            PrepDataWithDiff( pDevice, pszFuzzyProp, pszMachineProp, &ulSegNum, sMTLevel );
          }
          else if ( pucSeg == pszMachineProp )
          {
            PrepDataWithDiff( pDevice, pszMachineProp, pszFuzzyProp, &ulSegNum, sMTLevel );
          }
          else
          {
			      PrepData( pDevice, pucSeg, ulPropLen, usPropNum, &ulSegNum, sMTLevel );
          } /* endif */

			    pDoc->stTWBS.StartStopProp[ usPropNum ].usLastSeg = (USHORT)(ulSegNum-1);
		    } /* endif */

		    pDoc->stTWBS.usIndexInEqfSAB[usPropNum] = usPropsInEqfSAB;
		    usPropNum++;
       } /* endif */

       usPropsInEqfSAB++;
       if (usPropNum == 0)
       {
		    pDoc->stTWBS.usIndexInEqfSAB[usPropNum] = usPropsInEqfSAB;
		    usPropNum++;
       }
       pucSeg = GetPropSZ ((PDOCUMENT_IDA)pDevice->pDoc, usPropsInEqfSAB, &ulPropLen);
    } // endwhile

    if ( fDisplay )
    {
      /****************************************************************/
      /* set wrapping according to document window user settings and  */
      /* add soft LF if necessary                                     */
      /****************************************************************/
      pTBDoc->fLineWrap = pTBDoc->fAutoLineWrap = pTBDoc->pUserSettings->fLineWrap;
      if ( pTBDoc->fAutoLineWrap )
      {
        pTBDoc->sRMargin = (USHORT)pTBDoc->lScrnCols;
        EQFBSoftLFInsert( pTBDoc );
      } /* endif */

      EQFDispWindow ( pTBDoc, FALSE );                // display the window
    } /* endif */
  } // endif (vaild fg.segment)

  return;
} // end 'InsertProposal'


static
VOID PrepData
(
   PTWBSDEVICE   pDevice,                    // pointer to device contexts
   PSZ_W         pucSeg,
   ULONG         ulPropLen,
   USHORT        usProps,
   PULONG        pulSegNum,
   SHORT         sMTLevel
)
{
  PTBDOCUMENT   pTBDoc;
  PDOCUMENT_IDA pDoc;                              // pointer to docu ida
  PCHAR_W       pucTgt;                            // pointer to display string
  PCHAR_W       pucSrc;                            // pointer to source string
  CHAR_W        c;                                 // current character
  USHORT        usDispNum = 0;                     // number of chars in line

  pDoc = (PDOCUMENT_IDA) pDevice->pDoc;
  pTBDoc = &(pDevice->tbDoc);                      // pointer to document
  /***************************************************************/
  /* copy only first line of source plus three '...' if more     */
  /* than one line                                               */
  /* if requested ( pUserSettings->fFullSeg)                     */
  /***************************************************************/
  if ( (usProps == 0) && !pTBDoc->pUserSettings->fFullSeg )
  {
    pucTgt = (PSZ_W)pDoc->szBuf;
    pucSrc = pucSeg;

    while ( usDispNum++ < MAX_PROP_LEN && ((c = *pucSrc ++)!= NULC) )
    {
      /***********************************************************/
      /* we found a linefeed and still something in segment      */
      /***********************************************************/
      if ( c == LF && *pucSrc )
      {
        UTF16memset( pucTgt, '.', 3);
        pucTgt += 3;
        *pucTgt++ = LF;
        usDispNum = MAX_PROP_LEN + 100;
      }
      else
      {
        *pucTgt++ = c;
      } /* endif */
    } /* endwhile */
    /******************************************************************/
    /* check if still something left over -- even if only one line    */
    /******************************************************************/
    if ( (usDispNum == MAX_PROP_LEN + 1) && *pucSrc  )
    {
      pucTgt -= 3;
      //*pucTgt++ = '.';
      //*pucTgt++ = '.';
      //*pucTgt++ = '.';
      UTF16memset( pucTgt, '.', 3);
      pucTgt += 3;
      *pucTgt++ = LF;
    } /* endif */

    *pucTgt = EOS;
    tbSegment.pDataW = (PSZ_W)pDoc->szBuf;
    ulPropLen = UTF16strlenCHAR(tbSegment.pDataW);
    tbSegment.usLength = (USHORT)(ulPropLen + 1);
  }
  else
  {
    tbSegment.pDataW = pucSeg;
    tbSegment.usLength = (USHORT)ulPropLen; // KBT0261: remove   +1;
  } /* endif */
                                        // display segment data
  if ( pTBDoc->pUserSettings->fMachFuzzyColor )
  {
    if ( (sMTLevel == FUZZY_PROP) || (sMTLevel == FUZZY_REPLACE_PROP) )
    {
      tbSegment.qStatus = (USHORT)QF_FUZZYPROPOSAL;
    }
    else if ( sMTLevel == MACHINE_TRANS_PROP )
    {
      tbSegment.qStatus = (USHORT)QF_MACHPROPOSAL;
    }
    else
    {
      tbSegment.qStatus = (USHORT)((usProps == 0) ? QF_PROP0TEXT : QF_PROPNTEXT);
    } /* endif */       
  }
  else
  {
    tbSegment.qStatus = (USHORT)((usProps == 0) ? QF_PROP0TEXT : QF_PROPNTEXT);
  } /* endif */     
  tbSegment.ulSegNum = (*pulSegNum)++;
  tbSegment.pusBPET = NULL;
  SetLineNum( pTBDoc, tbSegment.pDataW );

  PrepAddSeg( &tbSegment, &tbPrepSeg );

  tbPrepSeg.SegFlags.NoReorder = (usProps == 0);
  EQFBAddSegW( pTBDoc, &tbPrepSeg );
                                          // add linefeed if not at eos
  if ( *(tbSegment.pDataW+ulPropLen-1) != LF   )
  {
     if ( pTBDoc->docType == SERVDICT_DOC ) {
        tbDictNewLineSegment.ulSegNum = (*pulSegNum)++;
        SetLineNum( pTBDoc, tbDictNewLineSegment.pDataW );
        PrepAddSeg ( &tbDictNewLineSegment, &tbPrepSeg );
     } else {
        tbMemNewLineSegment.ulSegNum = (*pulSegNum)++;
        SetLineNum( pTBDoc, tbMemNewLineSegment.pDataW );
        PrepAddSeg ( &tbMemNewLineSegment, &tbPrepSeg );
     }
     tbPrepSeg.SegFlags.NoReorder = (usProps == 0);
     EQFBAddSegW( pTBDoc, &tbPrepSeg );
  } /* endif */

}

// show segment data with differences
static VOID PrepDataWithDiff
(
   PTWBSDEVICE   pDevice,                    // pointer to device contexts
   PSZ_W         pszSeg,
   PSZ_W         pszCompareWith,
   PULONG        pulSegNum,
   SHORT         sMTLevel
)
{
  BOOL  fOK = TRUE;                    // success indicator
  ULONG   ulSegNum = *pulSegNum;       // active segment number
  PFUZZYTOK  pFuzzyTok = NULL;         // pointer to fuzzy tokens
  PFUZZYTOK  pTok;                     // pointer to fuzzy token list
  PSZ_W      pData;                    // pointer to data
  PDOCUMENT_IDA  pDoc;                 // pointer to document ida
  PTBDOCUMENT pTBDoc;                  // pointer to devices struct
  CHAR_W      chTemp;                   // temp. character ...
	USHORT        usTokens = 0;
	PSTEQFSAB     pstEQFSab = NULL;
  USHORT        usModWords = 0;

  pDoc = (PDOCUMENT_IDA)pDevice->pDoc; // get pointer to document ida
  pTBDoc = &(pDevice->tbDoc);          // pointer to document structure

  // find out the differences                                       
	pstEQFSab = pDoc->stEQFSab + pDoc->usFI; // pointer to foreground
	fOK = EQFBFindDiff( pTBDoc, pszCompareWith, pszSeg, pDoc->sTgtLanguage, &pFuzzyTok, &usModWords,
						  &usTokens, pDoc->ulTgtOemCP);

  tbSegment.pDataW = pszSeg;

  pTok = pFuzzyTok;
  while ( pTok->ulHash )
  {
    if ( pTok->sType != MARK_DELETED )
    {
      tbSegment.usLength = pTok->usStop - pTok->usStart + 1;
      tbSegment.pDataW = pszSeg + pTok->usStart;
      tbSegment.ulSegNum = ulSegNum ++;
      tbSegment.pusBPET = NULL;
      tbSegment.pusHLType = NULL;

      switch ( pTok->sType )
      {
        case  MARK_INSERTED:
          tbSegment.qStatus =  QF_PROPSRCINS;
          break;
        case  MARK_MODIFIED:
          tbSegment.qStatus =  QF_PROPSRCUNEQU;
          break;
        case  MARK_EQUAL:
        default :
          tbSegment.qStatus =  QF_PROPSRCEQU;
          if ( pTBDoc->pUserSettings->fMachFuzzyColor )
          {
            if ( (sMTLevel == FUZZY_PROP) || (sMTLevel == FUZZY_REPLACE_PROP) )
            {
              tbSegment.qStatus = (USHORT)QF_FUZZYPROPOSAL;
            }
            else if ( sMTLevel == MACHINE_TRANS_PROP )
            {
              tbSegment.qStatus = (USHORT)QF_MACHPROPOSAL;
            }
            else
            {
              tbSegment.qStatus = (USHORT)QF_PROPNTEXT;
            } /* endif */       
          }
          else
          {
            tbSegment.qStatus = (USHORT)QF_PROPNTEXT;
          } /* endif */     
          break;
      } /* endswitch */



      /**************************************************************/
      /* check if last char is a lf - stop coloring                 */
      /**************************************************************/
      if ( tbSegment.pDataW[tbSegment.usLength-1] == LF )
      {
        /**************************************************************/
        /* add segment data to rest                                   */
        /**************************************************************/
        tbSegment.usLength --;
        chTemp = tbSegment.pDataW[ tbSegment.usLength];
        tbSegment.pDataW[ tbSegment.usLength] = EOS;
        SetLineNum( pTBDoc, tbSegment.pDataW );
        PrepAddSeg ( &tbSegment, &tbPrepSeg );
        tbSegment.pDataW[ tbSegment.usLength] = chTemp;
        tbPrepSeg.SegFlags.NoReorder = TRUE;
        EQFBAddSegW( pTBDoc, &tbPrepSeg );

        /************************************************************/
        /* add the linefeed character                               */
        /************************************************************/
        if ( pTBDoc->docType == SERVDICT_DOC ) {
           tbDictNewLineSegment.ulSegNum = ulSegNum ++;
           SetLineNum( pTBDoc, tbDictNewLineSegment.pDataW );
           PrepAddSeg ( &tbDictNewLineSegment, &tbPrepSeg );
        } else {
           tbMemNewLineSegment.ulSegNum = ulSegNum ++;
           SetLineNum( pTBDoc, tbMemNewLineSegment.pDataW );
           PrepAddSeg ( &tbMemNewLineSegment, &tbPrepSeg );
        }
        EQFBAddSegW( pTBDoc, &tbPrepSeg );

      }
      else
      {
        /**************************************************************/
        /* add segment data to rest                                   */
        /**************************************************************/
        chTemp = tbSegment.pDataW[ tbSegment.usLength];
        tbSegment.pDataW[ tbSegment.usLength] = EOS;
        SetLineNum( pTBDoc, tbSegment.pDataW );
        PrepAddSeg ( &tbSegment, &tbPrepSeg );
        tbSegment.pDataW[ tbSegment.usLength] = chTemp;
        tbPrepSeg.SegFlags.NoReorder = TRUE;
        EQFBAddSegW( pTBDoc, &tbPrepSeg );
      } /* endif */
    } /* endif */       

    /**************************************************************/
    /* point to next token ...                                    */
    /**************************************************************/
    pTok++;
  } /* endwhile */

  // free allocated resources                                      
  if ( pFuzzyTok ) UtlAlloc( (PVOID *) &pFuzzyTok, 0L, 0L, NOMSG );

  // if last character is not linefeed, add one....                   
  pData = tbSegment.pDataW + tbSegment.usLength - 1;
  if ((*pData != LF) && fOK )
  {
    if ( pTBDoc->docType == SERVDICT_DOC ) {
       tbDictNewLineSegment.ulSegNum = ulSegNum ++;
       SetLineNum( pTBDoc, tbDictNewLineSegment.pDataW );
       PrepAddSeg ( &tbDictNewLineSegment, &tbPrepSeg );
    } else {
       tbMemNewLineSegment.ulSegNum = ulSegNum ++;
       SetLineNum( pTBDoc, tbMemNewLineSegment.pDataW );
       PrepAddSeg ( &tbMemNewLineSegment, &tbPrepSeg );
    }
    tbPrepSeg.SegFlags.NoReorder = TRUE;
    EQFBAddSegW( pTBDoc, &tbPrepSeg );
  } /* endif */

  *pulSegNum = ulSegNum;               // pass back active segment number

  return;
} /* end of function PrepDataWithDiff */


static
VOID PrepPrefix
(
   PTBDOCUMENT   pTBDoc,
   PSZ_W         * ppPrefix,
   USHORT        usProps,
   PULONG        pulSegNum,
   BOOL          fDisplay
)
{
  PSZ_W  pPrefix = *ppPrefix;

  UTF16strcat (pPrefix, DASH_STRING);

                                        // display prefix
  tbSegment.pDataW = pPrefix;
  tbSegment.usLength = (USHORT)(UTF16strlenCHAR(pPrefix) + 1);
  tbSegment.qStatus = (USHORT)(
       (usProps == 0) ? QF_PROP0PREFIX : QF_PROPNPREFIX);
  tbSegment.ulSegNum = *pulSegNum;
  (*pulSegNum)++;
  tbSegment.pusBPET = NULL;
  tbSegment.pusHLType = NULL;

  if ( fDisplay )
  {
    SetLineNum( pTBDoc, tbSegment.pDataW );
    PrepAddSeg ( &tbSegment, &tbPrepSeg );
    tbPrepSeg.SegFlags.NoReorder = (usProps == 0);
    EQFBAddSegW( pTBDoc, &tbPrepSeg );
  } /* endif */

  *ppPrefix += tbSegment.usLength ;       // adjust prefix
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     InsertSource
//------------------------------------------------------------------------------
// Function call:     InsertSource( PTWBSDEVICE, fDisplay, fSrcPropWnd );
//------------------------------------------------------------------------------
// Description:       display source of proposals
//------------------------------------------------------------------------------
// Parameters:        PTWBSDEVICE       pointer to device description
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Prerequesits:      AVIO window has to be created first
//------------------------------------------------------------------------------
// Function flow:     if in foreground mode then
//                      clear the window and free the space
//                      prepare the display of the original
//                      while no error and proposals available
//                        get segments for current proposal using the EXTSEG
//                        call, be careful if it is a MT proposal....
//                      endwhile
//                      Show and activate window
//                      display the window
//                    endif
//                    return
//------------------------------------------------------------------------------
static
VOID InsertSource
(
   PTWBSDEVICE  pDevice,               // pointer to device contexts
   BOOL         fActivate,              // activate the window
   BOOL         fSrcProp               // user setting : TRUE displ.src
)
{
  USHORT    usPropNum = 0;            // index of prop in stTWBS struct
  PSTEQFSAB pstEQFSab;                // pointer to free seg in sab
  PSTEQFSAB pstEQFActSab;             // pointer to active segment insab
  PSZ_W     pSrcStarts[10];           // start of sources
  PDOCUMENT_IDA  pDoc;                // pointer to document ida
  PSTEQFGEN pstEQFGen;                // pointer to generic structure
  ULONG     ulSegNum = 1;             // segment number
  PTBDOCUMENT pTBDoc;                 // pointer to devices struct
  SHORT     sMTLevel;                 // MT Level
  USHORT    usModWords = 0;
  USHORT    usPropsInEqfSab = 0;      // index of prop in stEqfSAB struc
  BOOL      fAdded = FALSE;

  pDoc = (PDOCUMENT_IDA)pDevice->pDoc;               // get pointer to document ida
  pTBDoc = &(pDevice->tbDoc);         // pointer to document structure
  memset( pSrcStarts, 0, sizeof(pSrcStarts));
  pDoc->stTWBS.usLCSModWords = 0;


  if (pDoc->fForeground && fSrcProp )
  {
    EQFClearWindow( pTBDoc , FALSE );
  }
  else
  {
    /******************************************************************/
    /* clear allocated resources                                      */
    /******************************************************************/
    EQFClearSegTable( pTBDoc );
  } /* endif*/

    pstEQFGen = pDoc->pstEQFGen;        // pointer to generic structure
                                        // use space of next element
    pstEQFSab = pDoc->stEQFSab + pDoc->usFI;   // point to current element

    pstEQFActSab = (pDoc->stEQFSab + pDoc->usFI);
    pSrcStarts[usPropNum ] = pstEQFActSab->pucSourceSeg;

    pDoc->stTWBS.stEQFExt.pucSourceSeg = pDoc->stTWBS.chInsertSource;
    /************************************************************/
    /* prepare the display of the original ...                  */
    /************************************************************/

    fAdded = EQFPrepTMSegs( pDevice, 0, pSrcStarts[0], NULL, &ulSegNum, &usModWords, &usPropNum );
    usPropsInEqfSab++;
                            // get segments for all available props
    while ( (pstEQFGen->usRC == EQFRC_OK || pstEQFGen->usRC == EQFRS_NOT_AVAILABLE) &&
            (usPropsInEqfSab <= (pDoc->stEQFSab + pDoc->usFI)->usPropCount))
    {

      /**************************************************************/
      /* check if it is a machine translation proposal ...          */
      /**************************************************************/
      sMTLevel = MachineTrans(pDoc, usPropsInEqfSab);
      if ( !(sMTLevel & MACHINE_TRANS_PENDING) )
      {
        UTF16strcpy( pDoc->stTWBS.stEQFExt.pucSourceSeg, GetSrcPropSZ( pDoc, usPropsInEqfSab ));
      }
      else
      {
        UTF16strcpy( pDoc->stTWBS.stEQFExt.pucSourceSeg, pstEQFActSab->pucSourceSeg );
      } /* endif */
      /************************************************************/
      /* find the differences and prepare the display ...         */
      /************************************************************/
      fAdded = EQFPrepTMSegs( pDevice, usPropsInEqfSab,
                     pSrcStarts[0], pDoc->stTWBS.stEQFExt.pucSourceSeg,
                     &ulSegNum, &usModWords, &usPropNum );

      /**************************************************************/
      /* store numb of different words if later requested by editor */
      /* from best proposal which is first one                      */
      /**************************************************************/
      if (usPropsInEqfSab == 1 )
      {
        pDoc->stTWBS.usLCSModWords = usModWords;
      } /* endif */

      /**************************************************************/
      /* prepare pointers for next time ...                         */
      /**************************************************************/
      if (fAdded)
      { // PrepTMSegs added a src-of-proposal!
        pSrcStarts[usPropNum-1] = pDoc->stTWBS.stEQFExt.pucSourceSeg; // store pointer

        pDoc->stTWBS.stEQFExt.pucSourceSeg +=
                       UTF16strlenCHAR( pDoc->stTWBS.stEQFExt.pucSourceSeg ) + 3;
      }
      usPropsInEqfSab ++;
    } /* endwhile */

  if (pDoc->fForeground && fSrcProp )
  {
    /****************************************************************/
    /* set wrapping according to document window user settings and  */
    /* add soft LF if necessary                                     */
    /****************************************************************/
    pTBDoc->fLineWrap = pTBDoc->fAutoLineWrap = pTBDoc->pUserSettings->fLineWrap;
    if ( pTBDoc->fAutoLineWrap )
    {
      pTBDoc->sRMargin = (SHORT)pTBDoc->lScrnCols;
      EQFBSoftLFInsert( pTBDoc );
    } /* endif */
    EQFDispWindow ( pTBDoc, FALSE );             // display the window
    WinSetWindowPos(pTBDoc->hwndFrame, HWND_TOP, 0, 0, 0, 0,
                 (USHORT)((fActivate) ? EQF_SWP_SHOW | EQF_SWP_ACTIVATE : EQF_SWP_SHOW));

    if (fActivate)
    {
      /*****************************************************/
      /* change input focus                                */
      /*****************************************************/
      SendMessage(pDoc->pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                  0, MP2FROMHWND(pTBDoc->hwndFrame) );
    } /* endif */
  } /* endif*/
//  }                                    // endif (valid fg.segment)

}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFPrepTMSegs
//------------------------------------------------------------------------------
// Description:       prepares the segments and the differences within the
//                    current source and the previous sources
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     everything okay
//                    FALSE    could not allocate the necessary memory
//                    or       nothing added to stTWBSstruct
//------------------------------------------------------------------------------
// Function flow:     init all nec. variables
//                    prepare and display the prefix...
//                    prepare and display the segment
//                    (taking into account the differences )
//                      - call EQFBFindDiff
//                      - prepare the display according to the found difference
//                      - free the allocated memory
//                    if last character is not LF add one
//                    return success indicator
//------------------------------------------------------------------------------

static
BOOL EQFPrepTMSegs
(
  PTWBSDEVICE    pDevice,              // pointer to device contexts
  USHORT         usPropsInEqfSab,               // active proposal
  PSZ_W          pSource,              // first source text
  PSZ_W          pTarget,              // second source text
  PULONG         pulSegNum,            // active segment
  PUSHORT        pusModWords,
  PUSHORT        pusPropNum            // number of proposal in stTWBS struct
)
{
  BOOL  fOK = TRUE;                    // success indicator
  ULONG   ulSegNum = *pulSegNum;       // active segment number
  PFUZZYTOK  pFuzzyTok = NULL;         // pointer to fuzzy tokens
  PFUZZYTOK  pTok;                     // pointer to fuzzy token list
  PSZ_W      pData;                    // pointer to data
  PDOCUMENT_IDA  pDoc;                 // pointer to document ida
  PTBDOCUMENT pTBDoc;                  // pointer to devices struct
  CHAR_W      chTemp;                   // temp. character ...
  USHORT      usProp = *pusPropNum;

  pDoc = (PDOCUMENT_IDA)pDevice->pDoc;                // get pointer to document ida
  pTBDoc = &(pDevice->tbDoc);          // pointer to document structure

  if ( !pDoc->ulSrcOemCP)
  {
      pDoc->ulSrcOemCP = GetLangOEMCP( pDoc->szDocSourceLang);
  }

  if ( usPropsInEqfSab )
  {                            // color num of bytest equal ...
    /******************************************************************/
    /* find out the differences                                       */
    /******************************************************************/
	USHORT        usTokens = 0;
	USHORT        usDisplayFuzzyLevel = 0;
	PSTEQFSAB     pstEQFSab = NULL;
	pstEQFSab = pDoc->stEQFSab + pDoc->usFI; // pointer to foreground
	fOK = EQFBFindDiff( pTBDoc, pSource, pTarget,
						  pDoc->sSrcLanguage, &pFuzzyTok, pusModWords,
						  &usTokens, pDoc->ulSrcOemCP);
	if ((pstEQFSab->usFuzzyPercents[usPropsInEqfSab-1] == 0 ) && fOK )
	{
	  pstEQFSab->usFuzzyPercents[usPropsInEqfSab-1] = CalcPercent(*pusModWords, usTokens );
	} /* endif */
	if (fOK)
	{
	  if ( usTokens > 15 )
	  {
		usDisplayFuzzyLevel = (USHORT)(UtlQueryULong( QL_LARGEFUZZLEVEL ) / 100);
	  }
	  else if ( usTokens > 4 )
	  {
		usDisplayFuzzyLevel = (USHORT)(UtlQueryULong( QL_MEDIUMFUZZLEVEL ) / 100);
	  }
	  else
	  {
		usDisplayFuzzyLevel = (USHORT)(UtlQueryULong( QL_SMALLFUZZLEVEL ) / 100);
	  } /* endif */

	  if (pstEQFSab->usFuzzyPercents[usPropsInEqfSab-1] < usDisplayFuzzyLevel)
	  { // do not display this proposal, it is below DisplayFuzzylevel
		 pstEQFSab->fInvisible[usPropsInEqfSab-1] = TRUE;
	  } /* endif */
	} /* endif */

    if ( fOK && !pstEQFSab->fInvisible[usPropsInEqfSab-1] )
    {
	  /********************************************************************/
	  /* prepare and display the prefix ...                               */
	  /********************************************************************/
	  tbSegment.pDataW = pDoc->stTWBS.pszPrefix[usProp] ;
	  tbSegment.usLength = (USHORT)UTF16strlenCHAR(tbSegment.pDataW) + 1;
	  tbSegment.qStatus = (USHORT)
		   ((usProp == 0) ? QF_PROP0PREFIX : QF_PROPNPREFIX);
	  tbSegment.ulSegNum = ulSegNum ++;
	  tbSegment.pusBPET = NULL;
	  tbSegment.pusHLType = NULL;
	  SetLineNum( pTBDoc, tbSegment.pDataW );
	  PrepAddSeg ( &tbSegment, &tbPrepSeg );
	  tbPrepSeg.SegFlags.NoReorder = (usProp == 0);
	  EQFBAddSegW( pTBDoc, &tbPrepSeg );


    // show note indicator when there is a proposal note available
    if ( ContainsSegmentNote( pstEQFSab->pszSortPropsData[usPropsInEqfSab-1] ) )
    {
      tbSegment.pDataW = szNoteIndicator;
	    tbSegment.usLength = (USHORT)UTF16strlenCHAR(tbSegment.pDataW) + 1;
	    tbSegment.qStatus = (USHORT)QF_DICTSTYLENOT;
	    tbSegment.ulSegNum = ulSegNum ++;
	    tbSegment.pusBPET = NULL;
	    tbSegment.pusHLType = NULL;
	    SetLineNum( pTBDoc, tbSegment.pDataW );
	    PrepAddSeg ( &tbSegment, &tbPrepSeg );
	    tbPrepSeg.SegFlags.NoReorder = (usProp == 0);
	    EQFBAddSegW( pTBDoc, &tbPrepSeg );

    } /* endif */

	  /********************************************************************/
      /* prepare and display the segment                                  */
      /********************************************************************/
      tbSegment.pDataW = pTarget;

      pTok = pFuzzyTok;
      while ( pTok->ulHash )
      {
        tbSegment.usLength = pTok->usStop - pTok->usStart + 1;
        tbSegment.pDataW = pTarget + pTok->usStart;
        tbSegment.ulSegNum = ulSegNum ++;
        tbSegment.pusBPET = NULL;
        tbSegment.pusHLType = NULL;

        switch ( pTok->sType )
        {
          case  MARK_EQUAL:
            tbSegment.qStatus =  QF_PROPSRCEQU;
            break;
          case  MARK_INSERTED:
            tbSegment.qStatus =  QF_PROPSRCINS;
            break;
          case  MARK_MODIFIED:
            tbSegment.qStatus =  QF_PROPSRCUNEQU;
            break;
          case  MARK_DELETED:
            tbSegment.qStatus =  QF_PROPSRCDEL;
            tbSegment.pDataW = SRC_DELETED_STRING;
            tbSegment.usLength = (USHORT)UTF16strlenCHAR(tbSegment.pDataW);
            break;
          default :
            tbSegment.qStatus =  QF_PROPSRCEQU;
            break;
        } /* endswitch */

        /**************************************************************/
        /* check if last char is a lf - stop coloring                 */
        /**************************************************************/
        if ( tbSegment.pDataW[tbSegment.usLength-1] == LF )
        {
          /**************************************************************/
          /* add segment data to rest                                   */
          /**************************************************************/
          tbSegment.usLength --;
          chTemp = tbSegment.pDataW[ tbSegment.usLength];
          tbSegment.pDataW[ tbSegment.usLength] = EOS;
          SetLineNum( pTBDoc, tbSegment.pDataW );
          PrepAddSeg ( &tbSegment, &tbPrepSeg );
          tbSegment.pDataW[ tbSegment.usLength] = chTemp;
          tbPrepSeg.SegFlags.NoReorder = TRUE;
          EQFBAddSegW( pTBDoc, &tbPrepSeg );

          /************************************************************/
          /* add the linefeed character                               */
          /************************************************************/
          if ( pTBDoc->docType == SERVDICT_DOC ) {
             tbDictNewLineSegment.ulSegNum = ulSegNum ++;
             SetLineNum( pTBDoc, tbDictNewLineSegment.pDataW );
             PrepAddSeg ( &tbDictNewLineSegment, &tbPrepSeg );
          } else {
             tbMemNewLineSegment.ulSegNum = ulSegNum ++;
             SetLineNum( pTBDoc, tbMemNewLineSegment.pDataW );
             PrepAddSeg ( &tbMemNewLineSegment, &tbPrepSeg );
          }
          EQFBAddSegW( pTBDoc, &tbPrepSeg );

        }
        else
        {
          /**************************************************************/
          /* add segment data to rest                                   */
          /**************************************************************/
          chTemp = tbSegment.pDataW[ tbSegment.usLength];
          tbSegment.pDataW[ tbSegment.usLength] = EOS;
          SetLineNum( pTBDoc, tbSegment.pDataW );
          PrepAddSeg ( &tbSegment, &tbPrepSeg );
          tbSegment.pDataW[ tbSegment.usLength] = chTemp;
          tbPrepSeg.SegFlags.NoReorder = TRUE;
          EQFBAddSegW( pTBDoc, &tbPrepSeg );
        } /* endif */

        /**************************************************************/
        /* point to next token ...                                    */
        /**************************************************************/
        pTok++;
      } /* endwhile */
    }
    else
    { // make sure if proposal is invisible, to return fAdded = FALSE!
		fOK = FALSE;        //rc= FALSE indicating nothing has been added!
    } /* endif */


     /*****************************************************************/
     /* free allocated resources                                      */
     /*****************************************************************/
     if ( pFuzzyTok )
     {
       UtlAlloc( (PVOID *) &pFuzzyTok, 0L, 0L, NOMSG );
     } /* endif */
  }
  else
  {
    /******************************************************************/
    /* prepare the display for the source -- take into account if we  */
    /* want to have the source abbreviated...                         */
    /******************************************************************/
    if ( !pTBDoc->pUserSettings->fFullSeg )
    {
      PCHAR_W      pucTgt;                            // pointer to display string
      PCHAR_W      pucSrc;                            // pointer to source string
      CHAR_W       c;                                 // current character
      USHORT       usDispNum = 0;                     // number of chars in line

      pucTgt = (PSZ_W)pDoc->szBuf;
      pucSrc = pSource;

      while ( usDispNum++ < MAX_PROP_LEN && ((c = *pucSrc ++) != NULC) )
      {
        /***********************************************************/
        /* we found a linefeed and still something in segment      */
        /***********************************************************/
        if ( c == LF && *pucSrc )
        {
          UTF16memset( pucTgt, '.', 3);
          pucTgt += 3;
          *pucTgt++ = LF;
          usDispNum = MAX_PROP_LEN + 100;
        }
        else
        {
          *pucTgt++ = c;
        } /* endif */
      } /* endwhile */
      /******************************************************************/
      /* check if still something left over -- even if only one line    */
      /******************************************************************/
      if ( (usDispNum == MAX_PROP_LEN + 1) && *pucSrc  )
      {
        pucTgt -= 3;
        UTF16memset( pucTgt, '.', 3);
        pucTgt += 3;
       // *pucTgt++ = '.';
       // *pucTgt++ = '.';
       // *pucTgt++ = '.';
        *pucTgt++ = LF;
      } /* endif */

      *pucTgt = EOS;
      tbSegment.pDataW = (PSZ_W)pDoc->szBuf;
      tbSegment.usLength = (USHORT)UTF16strlenCHAR(tbSegment.pDataW);
    }
    else
    {
      tbSegment.pDataW = pSource;
      tbSegment.usLength = (USHORT)UTF16strlenCHAR(pSource);
    } /* endif */

    tbSegment.qStatus  = QF_PROPSRCEQU;
    tbSegment.ulSegNum = ulSegNum ++;
    tbSegment.pusBPET  = NULL;
    tbSegment.pusHLType = NULL;
    SetLineNum( pTBDoc, tbSegment.pDataW );
    PrepAddSeg ( &tbSegment, &tbPrepSeg );
    tbPrepSeg.SegFlags.NoReorder = TRUE;
    EQFBAddSegW( pTBDoc, &tbPrepSeg );
  } /* endif */

  /********************************************************************/
  /* if last character is not linefeed, add one....                   */
  /********************************************************************/
  pData = tbSegment.pDataW + tbSegment.usLength - 1;
  if ((*pData != LF) && fOK )
  {
    if ( pTBDoc->docType == SERVDICT_DOC ) {
       tbDictNewLineSegment.ulSegNum = ulSegNum ++;
       SetLineNum( pTBDoc, tbDictNewLineSegment.pDataW );
       PrepAddSeg ( &tbDictNewLineSegment, &tbPrepSeg );
    } else {
       tbMemNewLineSegment.ulSegNum = ulSegNum ++;
       SetLineNum( pTBDoc, tbMemNewLineSegment.pDataW );
       PrepAddSeg ( &tbMemNewLineSegment, &tbPrepSeg );
    }
    tbPrepSeg.SegFlags.NoReorder = TRUE;
    EQFBAddSegW( pTBDoc, &tbPrepSeg );
  } /* endif */

  // GQ: fix for P020272: moved increase of usProp out of if group above
  if ( fOK )
  {
    // P019994: increase usProp for next call!
  	usProp++;
	  *pusPropNum = usProp;
  } /* endif */


  *pulSegNum = ulSegNum;               // pass back active segment number

  return fOK;
} /* end of function EQFPrepTMSegs */

/*******************************************************************************
*
*       function      InsertDictionary()
*
* -----------------------------------------------------------------------------
*       Inserts the dictionary hits into the dictionary area
*******************************************************************************/
static
VOID InsertDictionary
(
   PTWBSDEVICE  pDevice                      // pointer to device contexts
)
{
   PSTDICTWORD   pstDictWord;                // pointer to dict word
   PSTDICTWORD   pstDictHDWWord;             // pointer to headword
   PSZ_W         pData;                      // pointer to data
   PSTEQFSAB     pstEQFSab;                  // pointer to send a head
   PDOCUMENT_IDA pDoc;                       // pointer to document ida
   ULONG         ulNum = 0;                  // number of term
   USHORT        usSlotlen;                  // length of a slot
   ULONG         ulLinelen;                  // length of the line
   USHORT        usMaxTgtlen;                // maximum target length
// ULONG         ulNVSlots;                  // number of vertical slots
   ULONG         ulNHSlots;                  // number of horiz. slots
   USHORT        usMaxSlots;                 // numb.of maximal supported slots
   USHORT        usMaxSlotLines;             // maximum number of lines           
   USHORT        i;
   USHORT        usSlotLine;
   USHORT        usSlot;
   PTBDOCUMENT   pTBDoc;                     // pointer to document ida
   ULONG         ulSegNum = 1;               // start segment number
   PSZ_W         pucBuffer;                  // pointer to buffer
   USHORT        usTargetLine;               // target line
   USHORT        usActualTgtLines = 0;       // target line
   USHORT        j;                          // index
   USHORT        usActSlotLine;              // actual slot line
   PSTHDW        pstHDW;                         // pointer to headword struct
   USHORT        usMaxTgtLines = MAX_TGT_LINES;            // maximum target lines..
   PSZ           pAsciiBuf = NULL;
   USHORT        usMaxDispDictlen = 0;
   USHORT        usMaxCharWTgtlen = 0;
   ULONG         ulAsciiLen = 0;
   USHORT        usDictPrefixOffset = 0 ;
   USHORT        usPrefixPage ;
   USHORT        usCurPrefixPage = 1 ;
   ULONG         ulSaveSegNum = 0 ;
   ULONG         ulSaveMaxSeg = 0 ;

   pDoc = (PDOCUMENT_IDA) pDevice->pDoc;     // get pointer to doc ida
   pstDictWord = pDoc->stTWBS.stDictWord;    // point to first entry
   memset( (PBYTE)pstDictWord, 0, sizeof(pDoc->stTWBS.stDictWord));
   pTBDoc = &(pDevice->tbDoc);               // pointer to document structure
   if (pDoc->ulSrcOemCP == 0)
   {
      pDoc->ulSrcOemCP = GetLangOEMCP( pDoc->szDocSourceLang );
   }
   if (pDoc->ulTgtOemCP == 0)
  {
       pDoc->ulTgtOemCP = GetLangOEMCP( pDoc->szDocTargetLang );
  }

   if  (IsDBCS_CP(pDoc->ulTgtOemCP))
   {
       UtlAlloc((PVOID *) &(pAsciiBuf),
                0L, (LONG)(MAX_SEGMENT_SIZE + 1) * sizeof(CHAR),
                ERROR_STORAGE);
   } /* endif */

   usPrefixPage = (USHORT) abs(pDoc->sDictPrefixPage) ;
   if ( usPrefixPage > 1 ) {
      ulSaveSegNum = pTBDoc->TBCursor.ulSegNum ;
      ulSaveMaxSeg = pTBDoc->ulMaxSeg ;
   }
   EQFClearWindow ( pTBDoc, FALSE );         // free previous allocations
   if ( usPrefixPage > 1 ) {
      pTBDoc->TBCursor.ulSegNum = ulSaveSegNum ;
   }

   pucBuffer =  pDoc->stTWBS.szDictBuffer;   // get pointer to dict buffer
   pstEQFSab   = (pDoc->stEQFSab+pDoc->usFI);
   if (pDoc->chDictPrefixes[0] == EOS )
   {
     EQFBBuildPrefixes((PSZ_W)&(pDoc->chDictPrefixes));
   } /* endif */
   if ( pstEQFSab->fDAReady )
   {
      pData = pstEQFSab->pucDictWords;
      pstHDW      = pDoc->stHDW ;               // get pointer to hdw struct
      memset( pstHDW, 0, sizeof( pDoc->stHDW ) );

      /****************************************************************/
      /* allow for display of 6 lines per entry                       */
      /****************************************************************/
      if (pTBDoc->pUserSettings->fAddInfoDic)
      {
        usMaxTgtLines = MAX_TGT_LINES * 2;
      } /* endif */

      if ( *pData )
      {
         i = 1;
         while ( *pData != NULC )                  // build up reference structure
         {
            CHAR_W   c;
            pData++;                          // skip handle where term found

            // get any style prefix from data
            if ( (*pData == STYLEPREFIX_NOTALLOWED) || (*pData == STYLEPREFIX_PREFERRED) || (*pData == STYLEPREFIX_UNDEFINED))
            {
              pstDictWord->chStylePrefix = *pData;
            }
            else
            {
              pstDictWord->chStylePrefix = 0;
            } /* endif */

            if (*pData == STYLEPREFIX_UNDEFINED) pData++;

            pstDictHDWWord = pstDictWord;     // store pointer to headword
            pstDictWord->usOffset = (USHORT)(pData - pstEQFSab->pucDictWords);
            pstDictWord->usType   = HDW;      // headword
            pstDictWord->usLength = (USHORT)UTF16strlenCHAR( pData );
            if ( IsDBCS_CP(pDoc->ulTgtOemCP)&& pAsciiBuf)
            {  // pData is in Doc Target language!
               // use OEMCP from DOCUMENT_IDA!
//                memset( pAsciiBuf, 0, MAX_SEGMENT_SIZE );

                pstDictWord->usDispLen = (USHORT)Unicode2ASCIIBuf(pData, pAsciiBuf,
                                                    pstDictWord->usLength,
                                                    MAX_SEGMENT_SIZE, pDoc->ulTgtOemCP);

                usMaxDispDictlen = max( usMaxDispDictlen, pstDictWord->usDispLen);
            }
            else
            {
                usMaxDispDictlen = max( usMaxDispDictlen, pstDictWord->usLength );
            }

            pstEQFSab->usMaxDictlen = max( pstEQFSab->usMaxDictlen, pstDictWord->usLength );
            pData += pstDictWord->usLength + 1;
            pstDictWord++;
            while ( (c = *pData) != NULC )          // get target data
            {
              c = *(pData+1);
              if ( ((c == ADDINFO_MARK_PRE)||(c == ADDINFO_MARK_POST)) && !pTBDoc->pUserSettings->fAddInfoDic)
              {
                /******************************************************/
                /* ignore additional info ...                         */
                /******************************************************/
                pData += UTF16strlenCHAR( pData ) + 1;
              }
              else
              {
                if ( (c == ADDINFO_MARK_PRE) || (c == ADDINFO_MARK_POST) )
                {
                  pstDictWord->usType  = ADDDICTINFO;// type of target
                  pstDictWord->chStylePrefix = 0;
                  pData++;               // skip the info mark
                }
                else
                {
                  // get any style prefix from data
                  if ( (*pData == STYLEPREFIX_NOTALLOWED) || (*pData == STYLEPREFIX_PREFERRED)  || (*pData == STYLEPREFIX_UNDEFINED))
                  {
                    pstDictWord->chStylePrefix = *pData;
                  }
                  else
                  {
                    pstDictWord->chStylePrefix = 0;
                  } /* endif */
                  pstDictWord->usType  = TARGET;       // type of target
                } /* endif */

                if (*pData == STYLEPREFIX_UNDEFINED) pData++;

                pstDictWord->usOffset = (USHORT)(pData - pstEQFSab->pucDictWords);
                pstDictWord->usLength = (USHORT)UTF16strlenCHAR( pData );

                if ( IsDBCS_CP(pDoc->ulTgtOemCP)&& pAsciiBuf)
                {
                    pstDictWord->usDispLen = (USHORT)Unicode2ASCIIBuf(pData, pAsciiBuf,
                                                        (ULONG)pstDictWord->usLength,
                                                        MAX_SEGMENT_SIZE, pDoc->ulTgtOemCP);

                    usMaxDispDictlen = max( usMaxDispDictlen, pstDictWord->usDispLen);
                }
                else
                {
                    usMaxDispDictlen = max( usMaxDispDictlen, pstDictWord->usLength );
                }

                pstEQFSab->usMaxDictlen = max( pstEQFSab->usMaxDictlen, pstDictWord->usLength );

                pstDictWord->usNum    = (USHORT)(((c != ADDINFO_MARK_PRE)&&(c != ADDINFO_MARK_POST)) ? ulNum++ : 0);  // number of term
                pData                += pstDictWord->usLength + 1;
                pstDictWord++;                       // point to new term
                if ( i++ >=  MAX_DICT_TERMS )
                {
                   break;
                } /* endif */
              } /* endif */
            } /* endwhile */
            pData++;                         // skip the end of this term
            if ( i++ >=  MAX_DICT_TERMS )    // end of allowed terms reached
            {
               break;
            } /* endif */
         } /* endwhile */

         // fill slots of real display
         // calculate number of horizontal slots

         usMaxCharWTgtlen = min (pstEQFSab->usMaxDictlen, MAX_TGT_LEN);
         usMaxTgtlen = min (usMaxDispDictlen, MAX_TGT_LEN);      // in display!!

         if (pTBDoc->pUserSettings->fDispDictName)
         {
           usMaxTgtlen += 3;
         } /* endif */
         usSlotlen   = usMaxTgtlen + 5;
         if ( pTBDoc->hwndRichEdit )
         {
           USHORT usTabs[2];
           RECT rc;
           USHORT usBaseUnit;
           ULONG  ulFontSize;
           GetClientRect( pTBDoc->hwndRichEdit, &rc );
           ulFontSize = GetMaxDictFont( pTBDoc );
           ulLinelen   = max((USHORT)(rc.right/ulFontSize)+1 , usSlotlen ) ;

           /****************************************************************/
           /* adjust tab settings in RichEdit control                      */
           /****************************************************************/
           usBaseUnit = (USHORT)GetDialogBaseUnits();
           usTabs[0] =  (USHORT)((usSlotlen * ulFontSize * 4) / usBaseUnit);
           usTabs[1] = 0;
           SendMessage( pTBDoc->hwndRichEdit, EM_SETTABSTOPS, 1, (LONG)&usTabs[0] );

           SendMessage( pTBDoc->hwndRichEdit, EM_GETRECT, 0, (LONG) &rc );
           rc.right += 100;
           SendMessage( pTBDoc->hwndRichEdit, EM_SETRECT, 0, (LONG) &rc);
         }
         else
         {
           ulLinelen   = max((USHORT)pTBDoc->lScrnCols , usSlotlen ) ;
         } /* endif */

         ulNHSlots   = ulLinelen / usSlotlen;   // determine number of horiz. slots
         ulLinelen   = ulNHSlots * usSlotlen + 1;
         usMaxSlotLines = (USHORT)(MAX_DICTBUFF_SIZE/(ulNHSlots*(usSlotlen+1))) ;
         if ( ulSaveSegNum > ulSaveMaxSeg / usMaxSlotLines * 4 ) {  // handle end of list
            pTBDoc->TBCursor.ulSegNum = min( ulSaveSegNum, ulSaveMaxSeg - 2*(ulSaveMaxSeg/(usMaxSlotLines-2)) ) ;
         }

         // calculate max. number of vertical slots
//       ulNVSlots = MAX_DICTBUFF_SIZE / (ulLinelen * (usMaxTgtLines + 2));

         // prepare slot entries (buffer with blanks)
         UTF16memset (pucBuffer, ' ', MAX_DICTBUFF_SIZE);

         usMaxSlots = MAX_DICTBUFF_SIZE / (usSlotlen + 1);
         pData = pucBuffer + usSlotlen;         // pointer to buffer
         for (i = 1; i < usMaxSlots; i++)      // init slot entries
         {
           *pData = '\0';
           pData += usSlotlen + 1;
         } /* endfor */

         usSlotLine = usActSlotLine = 0;
         usSlot     = 0;
         pstDictWord = pDoc->stTWBS.stDictWord;    // point to first entry

         while ( pstDictWord->usType == HDW  )  // process headwords
         {
            pstDictHDWWord = pstDictWord;                     // set positions

            if ( usSlot >= ulNHSlots )
            {
               usSlot = 0;
               usSlotLine += usActualTgtLines + 1;
               usActSlotLine = usSlotLine;   // set actual slot line
               usActualTgtLines = 0;
            } /* endif */

            if ( (usSlotLine+2) >= usMaxSlotLines ) {
               pstDictWord++ ;
            } else {
               pDoc->stTWBS.fStatusLine[ usSlotLine ] = TRUE;

               pData = pucBuffer + (usSlotLine*ulNHSlots+usSlot) * (usSlotlen + 1);
               /**********************************************************/
               /* setup initial info for dictionary lookup via double    */
               /* click                                                  */
               /* the segment number will be prepared in the display     */
               /* routine prior to the real display                      */
               /**********************************************************/
//             pstHDW->ulSegNum = usSlotLine*(ulNHSlots+1) + usSlot+ 1;
               pstHDW->pData = pstEQFSab->pucDictWords + pstDictWord->usOffset;
               pstHDW++;

               EQFInsertDictCopyWord( pDoc,
                                    (pstEQFSab->pucDictWords + pstDictWord->usOffset),
                                       pData, pAsciiBuf, pstDictWord->usLength,
                                       pstDictWord->usDispLen, usMaxTgtlen );

               usSlotLine ++;                          // point to next slot line
               pstDictWord++;                          // point to next target
               usTargetLine = 0;                       // new target starts

               while ( (pstDictWord->usType == TARGET) ||
                       (pstDictWord->usType == ADDDICTINFO) )
               {
                  if ( (usSlotLine+1) >= usMaxSlotLines ) {
                     pstDictWord++ ;
                  } else {
                     pDoc->stTWBS.fStatusLine[ usSlotLine ] = FALSE;
                     pData = pucBuffer + (usSlotLine*ulNHSlots+usSlot) * (usSlotlen + 1);

                     if ( usTargetLine < usMaxTgtLines )   // not max. reached
                     {
                       if ( pstDictWord->usType == TARGET )
                       {
                         if (pDoc->chDictPrefixes[0] != EOS )
                         {
                            USHORT usOffset = pstDictWord->usNum - usDictPrefixOffset ; 
                            if ( ( usOffset > ALL_LETTERS ) || 
                                 ( pDoc->chDictPrefixes[usOffset] == '*' ) ) {
                               ++usCurPrefixPage ;
                               usDictPrefixOffset = pstDictWord->usNum ;
                               usOffset = 0 ;
                            } 
                            if ( usCurPrefixPage == usPrefixPage ) {
                               *pData = pDoc->chDictPrefixes[usOffset];
                            } else {
                               *pData = '*';
                            }
                         }
                         else
                         {
                           *pData = (UCHAR) (FIRST_MARK_CHAR + pstDictWord->usNum) ;

                           if ((UCHAR) *pData > LAST_MARK_CHAR )
                           {
                              *pData = (UCHAR) (FIRST_UPPER_MARK_CHAR +
                                                pstDictWord->usNum - 26 );
                              if ((UCHAR) *pData > LAST_UPPER_MARK_CHAR )
                              {
                                *pData = '*';
                              } /* endif */
                           }  /* endif */
                         } /* endif */
                         *(pData+1) = ')';

                         // add dictindicator(s) if requested by user
                         if (pTBDoc->pUserSettings->fDispDictName)
                         {
                           CHAR_W c;
                           int iIndicatorChars = 0;
                           int iIndex = 1;                 // index of dictionary
                           c = (*(pstEQFSab->pucDictWords + pstDictWord->usOffset));
                           *(pData + 3) = '[';
                           while ( iIndex < 16 )
                           {
                             if ( c & 0x0001 )
                             {
                               if ( iIndicatorChars != 0 )
                               {
                                 *(pData+4 + iIndicatorChars) = ',';
                                 iIndicatorChars++;
                               } /* endif */
                               if ( iIndex > 9)
                               {
                                 *(pData+4 + iIndicatorChars) = (CHAR_W)(L'A'+ iIndex - 10);
                               }
                               else
                               {
                                 *(pData + 4 + iIndicatorChars) = (CHAR_W)(L'0'+ iIndex);
                               } /* endif */
                               iIndicatorChars++;
                             } /* endif */
                             iIndex++;
                             c = c >> 1;
                           } /* endwhile */
                           *(pData + 4 + iIndicatorChars) = ']';
                           i = (USHORT)(5 + iIndicatorChars);
                         }
                         else
                         {
                            i = 3;             //donot add dictindicator
                         } /*  endif */
                       }
                       else
                       {
                         *pData = ADDINFO_MARK_PRE;    // we deal with additional info...
                         i = 3;
                       } /* endif */
                       EQFInsertDictCopyWord( pDoc, (pstEQFSab->pucDictWords + pstDictWord->usOffset+1),
                                               pData + i,
                                               pAsciiBuf, (USHORT)(pstDictWord->usLength -1),
                                               (USHORT)(pstDictWord->usDispLen - 1),
                                               (USHORT)(usMaxTgtlen -i+3) );

                        pstDictWord++;                    // point to next target
                        usTargetLine++;                   // next target line
                        usActualTgtLines = max( usTargetLine, usActualTgtLines );

                     }
                     else                                // start new slot for this one
                     {
                        usTargetLine = 0;
                        usSlotLine = usActSlotLine;      // set actual slot line
                        usSlot ++ ;                      // go ahead one slot
                        if ( usSlot < ulNHSlots )
                        {
                           pData = pucBuffer +
                                     (usSlotLine*ulNHSlots+usSlot) * (usSlotlen + 1);
                           UTF16memset( pData, '.', 3);
                        }
                        else
                        {
                           usSlot = 0;
                           usSlotLine += usActualTgtLines + 1;
                           usActSlotLine = usSlotLine;   // set actual slot line
                           usActualTgtLines = 0;
                           pDoc->stTWBS.fStatusLine[ usSlotLine ] = TRUE; // it's a hdw again
                                                         // copy old hdw as heading
                           pData = pucBuffer +
                                     (usSlotLine*ulNHSlots+usSlot) * (usSlotlen + 1);
                           EQFInsertDictCopyWord(pDoc, (pstEQFSab->pucDictWords+pstDictHDWWord->usOffset),
                                                  pData, pAsciiBuf, pstDictHDWWord->usLength,
                                                  pstDictHDWWord->usDispLen, usMaxTgtlen );

                        } /* endif */
                                                      // insert new headword
                        pstHDW->ulSegNum = usSlotLine*(ulNHSlots+1) + usSlot+ 1;
                        pstHDW->pData = pstEQFSab->pucDictWords
                                         + pstDictHDWWord->usOffset;
                        pstHDW++;

                     } /* endif */
                  }
                  usSlotLine ++;                       // point to next slot line
               } /* endwhile */
            }
            usSlotLine  = usActSlotLine;            // restore actual slot line
            usSlot++;                               // increase slot
         } /* endwhile */

         usSlotLine += usActualTgtLines + 1;        // maximal number of lines
         pData = pucBuffer;                         // point to start of data
         if ( usCurPrefixPage == usPrefixPage ) 
            pDoc->sDictPrefixPage = -usPrefixPage ; // Negative indicates at end of list

         // fill in segment structures and prepare display
         /*************************************************************/
         /* set up pointer to pstHDW                                  */
         /*************************************************************/
         pstHDW = pDoc->stHDW;
         if ( (usSlotLine+1)*ulNHSlots*(usSlotlen+1) >= MAX_DICTBUFF_SIZE-10 ) {
            for( --usSlotLine ; (usSlotLine+1)*ulNHSlots*(usSlotlen+1) >= MAX_DICTBUFF_SIZE-10 ; --usSlotLine ) {
            }
            ++usSlotLine;
         }

         for ( i = 0; i< usSlotLine ; i++ )
         {
            /**********************************************************/
            /* check if we are dealing with headwords or translations */
            /**********************************************************/
            if ( pDoc->stTWBS.fStatusLine[i] )
            {
              tbSegment.SegFlags.NoReorder = TRUE;
              tbSegment.qStatus = QF_DICTHEAD;
              for ( j = 0; j < ulNHSlots ; j++ )
              {
                int iStyleIndicatorLen = 0;          // length of used style indicator

                // add dict style indicator segment
                if ( *pData == STYLEPREFIX_NOTALLOWED )
                {
                  tbSegment.qStatus = QF_DICTSTYLENOT;
                  tbSegment.pDataW = szDictStyleNotAllowed;
                  tbSegment.usLength = 1;
                  tbSegment.ulSegNum = ulSegNum ++;
                  tbSegment.pusBPET = NULL;
                  tbSegment.pusHLType = NULL;
                  EQFBAddSegW( pTBDoc, &tbSegment );
                  iStyleIndicatorLen = 1;
                }
                else if ( *pData == STYLEPREFIX_PREFERRED )
                {
                  tbSegment.qStatus = QF_DICTSTYLEPREF;
                  tbSegment.pDataW = szDictStylePreferred;
                  tbSegment.usLength = 1;
                  tbSegment.ulSegNum = ulSegNum ++;
                  tbSegment.pusBPET = NULL;
                  tbSegment.pusHLType = NULL;
                  EQFBAddSegW( pTBDoc, &tbSegment );
                  iStyleIndicatorLen = 1;
                }
                else if ( *pData == STYLEPREFIX_UNDEFINED )
                {
                  iStyleIndicatorLen = 1; // ignore dummy style indicator
                } /* endif */

                /******************************************************/
                /* setup correct segment number                       */
                /******************************************************/
                pstHDW->ulSegNum = ulSegNum;
                pstHDW++;

                /******************************************************/
                /* setup segment data                                 */
                /******************************************************/
                tbSegment.qStatus = QF_DICTHEAD;
                tbSegment.pDataW = pData + iStyleIndicatorLen;
                tbSegment.usLength = (USHORT)(usSlotlen + 1 - iStyleIndicatorLen);
                tbSegment.ulSegNum = ulSegNum ++;
                tbSegment.pusBPET = NULL;
                tbSegment.pusHLType = NULL;
                EQFBAddSegW( pTBDoc, &tbSegment );

                if ( pTBDoc->hwndRichEdit )
                {
                  pData[ usSlotlen-iStyleIndicatorLen ] = '\t';
                } /* endif */

                pData += usSlotlen + 1;                // point to next data
              } /* endfor */
            }
            else
            {
              /********************************************************/
              /* we are dealing with translations, i.e. we should     */
              /* have different attributes for the selection character*/
              /* and the real value                                   */
              /********************************************************/
              for ( j = 0; j < ulNHSlots ; j++ )
              {
                /******************************************************/
                /* we are dealing with the prefix (if there is any)   */
                /* (the length of the prefix is 3 character           */
                /******************************************************/
                switch ( *pData )
                {
                  case BLANK:
                    tbSegment.qStatus = QF_DICTTRANS;
                    break;
                  case ADDINFO_MARK_PRE:
                  case ADDINFO_MARK_POST:
                    *pData = BLANK;
                    tbSegment.qStatus = QF_DICTADDINFO;
                    break;
                  default:
                    tbSegment.qStatus = QF_DICTPREFIX;
                    break;
                } /* endswitch */
                tbSegment.pDataW = pData;
                tbSegment.usLength = 3;
                tbSegment.ulSegNum = ulSegNum ++;
                tbSegment.pusBPET = NULL;
                tbSegment.pusHLType = NULL;
                EQFBAddSegW( pTBDoc, &tbSegment );

                /******************************************************/
                /* we are dealing with the dictionary data ...        */
                /******************************************************/
                if ( tbSegment.qStatus != QF_DICTADDINFO )
                {
                  int iDictIndicatorLen = 0;           // length of used dictionary indicator
                  int iStyleIndicatorLen = 0;          // length of used style indicator
                  BOOL fInsertDummyStyle = FALSE;      // TRUE = we have to insert a dummy style value to keep display inline

                  if (pTBDoc->pUserSettings->fDispDictName)
                  {
                    if (*pData != BLANK)
                    {
                      // find end of indicator
                      PSZ_W pszIndicator = pData+3;
                      if ( *pszIndicator == L'['  )
                      {
                        while ( (*(pszIndicator+iDictIndicatorLen) != 0) && (*(pszIndicator+iDictIndicatorLen) != L']') ) iDictIndicatorLen++;
                        if ( *(pszIndicator+iDictIndicatorLen) == L']' )
                        {
                          iDictIndicatorLen += 1;  // include closing bracket
                        }
                        else
                        {
                          // unknown indicator type, assume default length
                          iDictIndicatorLen = 3;
                        } /* endif */

                        {

                        } /* endwhile */
                      }
                      else
                      {
                        // unknown indicator type, assume default length
                        iDictIndicatorLen = 3;
                      } /* endif */


                      // add indicator segment
                      tbSegment.qStatus = QF_DICTINDIC;
                      tbSegment.pDataW = pData+3;
                      tbSegment.usLength = (USHORT)iDictIndicatorLen;
                      tbSegment.ulSegNum = ulSegNum ++;
                      tbSegment.pusBPET = NULL;
                      tbSegment.pusHLType = NULL;
                      EQFBAddSegW( pTBDoc, &tbSegment );
                    }
                    else
                    {
                      iDictIndicatorLen = 3;
                      tbSegment.qStatus = QF_DICTTRANS;
                      tbSegment.pDataW = pData+3;
                      tbSegment.usLength = 3;
                      tbSegment.ulSegNum = ulSegNum ++;
                      tbSegment.pusBPET = NULL;
                      tbSegment.pusHLType = NULL;
                      EQFBAddSegW( pTBDoc, &tbSegment );
                    }

                    // add style indicator segment if any
                    if ( pData[3+iDictIndicatorLen] == STYLEPREFIX_NOTALLOWED )
                    {
                      tbSegment.qStatus = QF_DICTSTYLENOT;
                      tbSegment.pDataW = szDictStyleNotAllowed;
                      tbSegment.usLength = 1;
                      tbSegment.ulSegNum = ulSegNum ++;
                      tbSegment.pusBPET = NULL;
                      tbSegment.pusHLType = NULL;
                      EQFBAddSegW( pTBDoc, &tbSegment );
                      iStyleIndicatorLen = 1;
                    }
                    else if ( pData[3+iDictIndicatorLen] == STYLEPREFIX_PREFERRED )
                    {
                      tbSegment.qStatus = QF_DICTSTYLEPREF;
                      tbSegment.pDataW = szDictStylePreferred;
                      tbSegment.usLength = 1;
                      tbSegment.ulSegNum = ulSegNum ++;
                      tbSegment.pusBPET = NULL;
                      tbSegment.pusHLType = NULL;
                      EQFBAddSegW( pTBDoc, &tbSegment );
                      iStyleIndicatorLen = 1;
                    }
                    else if ( pData[3+iDictIndicatorLen] == STYLEPREFIX_UNDEFINED )
                    {
                      // ignore flag
                      iStyleIndicatorLen = 1;
                      fInsertDummyStyle = TRUE;
                    } /* endif */

                    tbSegment.qStatus = QF_DICTTRANS;
                    tbSegment.pDataW = pData+3+iDictIndicatorLen+iStyleIndicatorLen;
                    if (IsDBCS_CP(pDoc->ulTgtOemCP)&& pAsciiBuf)
                    {
//                       memset( pAsciiBuf, 0, MAX_SEGMENT_SIZE );
                         ulAsciiLen=Unicode2ASCIIBuf( pData+3+iDictIndicatorLen+iStyleIndicatorLen, pAsciiBuf,
                                                      (usSlotlen- 2-iStyleIndicatorLen-iDictIndicatorLen),
                                                      MAX_SEGMENT_SIZE, pDoc->ulTgtOemCP);
                         //  5 must be checked and changed here in next line!!
                         // + 1 - 6- 5=-10 warum
                         //   tbSegment.usLength = usSlotlen+usSlotlen-5-ulAsciiLen;
                          tbSegment.usLength = (USHORT)(usSlotlen+usSlotlen-8-iStyleIndicatorLen-iDictIndicatorLen-ulAsciiLen);
                    }
                   else
                    {
                        tbSegment.usLength = (USHORT)(usSlotlen - 2-iDictIndicatorLen-iStyleIndicatorLen);   // + 1 - 6
                    }

                    tbSegment.ulSegNum = ulSegNum ++;
                    tbSegment.pusBPET = NULL;
                    tbSegment.pusHLType = NULL;
                    // in case of Arabic change the ordering of dict hits
                    tbSegment.SegFlags.NoReorder = FALSE;
                    EQFBAddSegW( pTBDoc, &tbSegment );
                    tbSegment.SegFlags.NoReorder = TRUE;

                  }
                  else
                  {
                    // add style indicator segment if any
                    if ( pData[3] == STYLEPREFIX_NOTALLOWED )
                    {
                      tbSegment.qStatus = QF_DICTSTYLENOT;
                      tbSegment.pDataW = szDictStyleNotAllowed;
                      tbSegment.usLength = 1;
                      tbSegment.ulSegNum = ulSegNum ++;
                      tbSegment.pusBPET = NULL;
                      tbSegment.pusHLType = NULL;
                      EQFBAddSegW( pTBDoc, &tbSegment );
                      iStyleIndicatorLen = 1;
                    }
                    else if ( pData[3] == STYLEPREFIX_PREFERRED )
                    {
                      tbSegment.qStatus = QF_DICTSTYLEPREF;
                      tbSegment.pDataW = szDictStylePreferred;
                      tbSegment.usLength = 1;
                      tbSegment.ulSegNum = ulSegNum ++;
                      tbSegment.pusBPET = NULL;
                      tbSegment.pusHLType = NULL;
                      EQFBAddSegW( pTBDoc, &tbSegment );
                      iStyleIndicatorLen = 1;
                    }
                    else if ( pData[3] == STYLEPREFIX_UNDEFINED )
                    {
                      // ignore style
                      iStyleIndicatorLen = 1;
                      fInsertDummyStyle = TRUE;
                    } /* endif */

                    tbSegment.qStatus = QF_DICTTRANS;
                    tbSegment.pDataW = pData+3+iStyleIndicatorLen;
                    if ( IsDBCS_CP(pDoc->ulTgtOemCP)&& pAsciiBuf)
                    {
//                       memset( pAsciiBuf, 0, MAX_SEGMENT_SIZE );
                       ulAsciiLen=Unicode2ASCIIBuf( pData+3+iStyleIndicatorLen, pAsciiBuf,
                                                 ( usSlotlen- 2 - iStyleIndicatorLen),
                                                 MAX_SEGMENT_SIZE, pDoc->ulTgtOemCP);
                         // + 1 - 3  - 3 =-5
                        tbSegment.usLength = (USHORT)(usSlotlen+usSlotlen-5-ulAsciiLen-iStyleIndicatorLen);
                    }
                   else
                    {
                        tbSegment.usLength = (USHORT)(usSlotlen - 2 - iStyleIndicatorLen);   // + 1 - 3
                    }
                    tbSegment.ulSegNum = ulSegNum ++;
                    tbSegment.pusBPET = NULL;
                    tbSegment.pusHLType = NULL;
                    EQFBAddSegW( pTBDoc, &tbSegment );
                  }

                  // add empty dummy style field if necessary to keep display aligned
                  if ( fInsertDummyStyle )
                  {
                      tbSegment.qStatus = QF_DICTSTYLEPREF;
                      tbSegment.pDataW = L" ";
                      tbSegment.usLength = 1;
                      tbSegment.ulSegNum = ulSegNum ++;
                      tbSegment.pusBPET = NULL;
                      tbSegment.pusHLType = NULL;
                      EQFBAddSegW( pTBDoc, &tbSegment );
                  } /* endif */
                }
                else
                {
                  // stay with tbSegment.qStatus as QF_DICTADDINFO
                  tbSegment.pDataW = pData+3;
                  if (IsDBCS_CP(pDoc->ulTgtOemCP) && pAsciiBuf)
                  {
  //          memset( pAsciiBuf, 0, MAX_SEGMENT_SIZE );
                     ulAsciiLen=Unicode2ASCIIBuf( pData+3, pAsciiBuf,
                                               (usSlotlen- 2),
                                                MAX_SEGMENT_SIZE, pDoc->ulTgtOemCP);
                     tbSegment.usLength = (USHORT)(usSlotlen+usSlotlen-5-ulAsciiLen);
                 }
                else
                 {
                   tbSegment.usLength = usSlotlen - 2;   // + 1 - 3
                  }
                  tbSegment.ulSegNum = ulSegNum ++;
                  tbSegment.pusBPET = NULL;
                  tbSegment.pusHLType = NULL;

                  EQFBAddSegW( pTBDoc, &tbSegment );
                } /* endif */

                /******************************************************/
                /* add tab character to adjust display                */
                /******************************************************/
                if ( pTBDoc->hwndRichEdit )
                {
                  pData[ usSlotlen ] = '\t';
                } /* endif */
                pData += usSlotlen + 1;                // point to next data
              } /* endfor */
            } /* endif */

            if ( pTBDoc->docType == SERVDICT_DOC ) {
               tbDictNewLineSegment.ulSegNum = ulSegNum ++;
               EQFBAddSegW( pTBDoc, &tbDictNewLineSegment );
            } else {
               tbMemNewLineSegment.ulSegNum = ulSegNum ++;
               EQFBAddSegW( pTBDoc, &tbMemNewLineSegment );
            }
            pTBDoc->ulMaxLine ++;                       // increment line number
          } /* endfor */
      }
      else                                   // no dictionary entry
      {
          ulLinelen = pTBDoc->xClient;
          if ( pstEQFSab->fDAReady & DICT_IN_USE )
          {
            HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
            CHAR chText[40];

            WinLoadString ((HAB) UtlQueryULong( QL_HAB ), hResMod,
                           SID_ENTRY_IN_USE,
                           40, chText );
            if ( !chText[0] )
            {
              UTF16strcpy( pucBuffer, L"*** dictionary in use ***" );
            }
            else
            {
              ASCII2Unicode(chText, pucBuffer, pDoc->ulTgtOemCP);
            } /* endif */
          }
          else
          {
            ASCII2Unicode( pDoc->stTWBS.szNoDictEntry, pucBuffer, pDoc->ulTgtOemCP );
          } /* endif */
          tbSegment.pDataW = pucBuffer;
          tbSegment.usLength = (USHORT)(UTF16strlenCHAR( pucBuffer ) + 1);
          pData = pucBuffer + tbSegment.usLength;
          tbSegment.qStatus = QF_DICTTRANS ;
          tbSegment.ulSegNum = ulSegNum ++;
          tbSegment.pusBPET = NULL;
          tbSegment.pusHLType = NULL;

          EQFBAddSegW( pTBDoc, &tbSegment );

          tbDictNewLineSegment.ulSegNum = ulSegNum ++;
          EQFBAddSegW( pTBDoc, &tbDictNewLineSegment );
          pTBDoc->ulMaxLine ++;                       // increment line number

      } /* endif */

     EQFDispWindow( pTBDoc, TRUE );         // display the window
   } /* endif */
   if ( IsDBCS_CP(pDoc->ulTgtOemCP) && pAsciiBuf)
   {
      if ( pAsciiBuf) UtlAlloc((PVOID *) &(pAsciiBuf), 0L, 0L, NOMSG);
   } /* endif */

  return ;
} // end 'InsertDictEntries'
/* $PAGEIF20 */


static
void
EQFInsertDictCopyWord
(
  PDOCUMENT_IDA pDoc,
  PSZ_W        pSrcWord,
  PSZ_W        pTgtWord,
  PSZ          pAsciiBuf,
  USHORT       usLength,
  USHORT       usDispLen,               //nec for Non-Unicode DBCS systems only
  USHORT       usMaxTgtlen
)
{
    if ( IsDBCS_CP(pDoc->ulTgtOemCP) && pAsciiBuf)
    {
      memset( pAsciiBuf, 0, MAX_SEGMENT_SIZE );
      Unicode2ASCIIBuf( pSrcWord, pAsciiBuf, usLength, MAX_SEGMENT_SIZE, pDoc->ulTgtOemCP);

      if ( usDispLen > usMaxTgtlen)
      {
        BOOL  fEnd = FALSE;
        PSZ_W  pTempW = pSrcWord;
        ULONG  ulASCIIBytes = 0;
        ULONG  ulBytes = 0;

        ulASCIIBytes = EQFIsDBCSChar(*(pTempW), pDoc->ulTgtOemCP)? 2:1;

        while (!fEnd && *(pTempW+1) )
        {
          ulBytes = EQFIsDBCSChar(*(pTempW+1), pDoc->ulTgtOemCP) ?  2 : 1;

          if (ulASCIIBytes+ ulBytes  > usMaxTgtlen )
          {
            memset( (pAsciiBuf + ulASCIIBytes),  '.', 3);
            *(pAsciiBuf + ulASCIIBytes + 3 ) = EOS;
            fEnd = TRUE;
          }
          else
          {
            ulASCIIBytes += ulBytes;
            pTempW ++;
          }
        }
      } /* endif */

      ASCII2UnicodeBuf( pAsciiBuf, pTgtWord,
                            (min(usDispLen, usMaxTgtlen + 3)),
                            pDoc->ulTgtOemCP);
    }
    else
    {
        memcpy((PBYTE) pTgtWord, (PBYTE)(pSrcWord),
                           min(usLength, usMaxTgtlen) * sizeof(CHAR_W));
        if ( usLength > usMaxTgtlen)
        {
           UTF16memset( (pTgtWord + usMaxTgtlen - 2 ), '.', 3);
        } /* endif */
    }
  return;
}

/**********************************************************************/
/* return the maximum size of the font to be used                     */
/**********************************************************************/

static ULONG GetMaxDictFont( PTBDOCUMENT pTBDoc )
{
  TBSEGMENT tbSeg;
  CHARFORMAT2 CharFormat2;
  ULONG ulSize = 10;

  memset(&tbSeg, 0, sizeof( tbSeg ));

  tbSeg.qStatus = QF_DICTTRANS;
  memset(&CharFormat2, 0, sizeof( CharFormat2 ));
  CharFormat2.cbSize = sizeof( CharFormat2 );
  EQFBSegColRTF( &CharFormat2, &tbSeg, PROTECTED_CHAR, 0, DISP_PROTECTED,
                 FALSE, pTBDoc->pDispFileRTF );
  ulSize = max( CharFormat2.yHeight/20, (LONG)ulSize );

  tbSeg.qStatus = QF_DICTADDINFO;
  memset(&CharFormat2, 0, sizeof( CharFormat2 ));
  CharFormat2.cbSize = sizeof( CharFormat2 );
  EQFBSegColRTF( &CharFormat2, &tbSeg, PROTECTED_CHAR, 0, DISP_PROTECTED,
                 FALSE, pTBDoc->pDispFileRTF );
  ulSize = max( CharFormat2.yHeight/20, (LONG)ulSize );

  tbSeg.qStatus = QF_DICTPREFIX;
  memset(&CharFormat2, 0, sizeof( CharFormat2 ));
  CharFormat2.cbSize = sizeof( CharFormat2 );
  EQFBSegColRTF( &CharFormat2, &tbSeg, PROTECTED_CHAR, 0, DISP_PROTECTED,
                 FALSE, pTBDoc->pDispFileRTF );
  ulSize = max( CharFormat2.yHeight/20, (LONG)ulSize );
  return ulSize;
}

/*/////////////////////////////////////////////////////////////////////
:h3.EQFBDispWindow - display the proposal or dictionary window data
*//////////////////////////////////////////////////////////////////////
// Description:
//   display the contents of the dictionary/proposal window
// Flow:
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
///////////////////////////////////////////////////////////////////////


static
VOID  EQFDispWindow
(
   PTBDOCUMENT  pTBDoc,
   BOOL         bNoReset 
)
{
//  pTBDoc->usMaxSeg  = pTBDoc->usSegments; // max segments stored
  if ( ! bNoReset ) {
     pTBDoc->lCursorRow = 0;
     pTBDoc->lCursorCol = 0;
     pTBDoc->lSideScroll = 0;

     pTBDoc->TBCursor.ulSegNum = 1;
     pTBDoc->TBCursor.usSegOffset = 0;
  }

  if ( pTBDoc->hwndRichEdit )
  {
    EQFBDisplayFileNewRTF( pTBDoc );
    SendMessage( pTBDoc->hwndRichEdit, EM_SETMODIFY, 1L, 0L );
    EQFBUpdateDispTable( pTBDoc );
  }
  else
  {
    EQFBScrnLinesFromSeg                   // build line table
           ( pTBDoc,
             pTBDoc->lCursorRow,           // starting row
             (pTBDoc->lScrnRows-pTBDoc->lCursorRow), // number of rows
             &(pTBDoc->TBCursor) );        // starting segment

    /**********************************************************************/
    /* no possibility found to set the thumbsize in windows, so only      */
    /* the setscrollbar is done in windows                                */
    /**********************************************************************/
       if ( GetWindowLong( pTBDoc->hwndFrame, GWL_STYLE ) & WS_VSCROLL )
       {
         SetScrollRange (pTBDoc->hwndFrame, SB_VERT,     //scroll slider
                         0,                              // min
                         max(pTBDoc->ulMaxSeg - 2, 2),   //   max
                         FALSE);
         SetScrollPos(pTBDoc->hwndFrame, SB_VERT,
                      pTBDoc->TBCursor.ulSegNum - 1, TRUE );    // Position
       } /* endif */

       if ( GetWindowLong( pTBDoc->hwndFrame, GWL_STYLE ) & WS_HSCROLL )
       {
         SetScrollRange (pTBDoc->hwndFrame, SB_HORZ, 0, 255, FALSE);
         if (pTBDoc->fTARight)
		 {
			if (pTBDoc-> lSideScroll < 255)
			{
				SetScrollPos(pTBDoc->hwndFrame, SB_HORZ,255 - pTBDoc->lSideScroll, TRUE );
			}
			else
			{
				SetScrollPos(pTBDoc->hwndFrame, SB_HORZ,0, TRUE );
			}
		 }
		 else
		 {
			 SetScrollPos(pTBDoc->hwndFrame, SB_HORZ,pTBDoc->lSideScroll, TRUE );
         }
       } /* endif */

    pTBDoc->Redraw |= REDRAW_ALL;          // redraw the screen
    EQFBScreenData( pTBDoc );
    EQFBScreenCursor( pTBDoc );            // update cursor and sliders

  } /* endif */
}




/*/////////////////////////////////////////////////////////////////////
:h3.EQFBClearWindow - clear the proposal or dictionary window data
*//////////////////////////////////////////////////////////////////////
// Description:
//   clear the contents of the dictionary/proposal window
//   and free the allocated memory
// Flow:
//   - loop through all the segments and free the BPET tables
//   - init data structures
//
// Parameters:
//  PTBDOCUMENT   - pointer to document ida
///////////////////////////////////////////////////////////////////////
static VOID EQFClearSegTable( PTBDOCUMENT pTBDoc )
{
    ULONG  ulSegNum = 1;                           // segment number
    PTBSEGMENT  pSeg;                              // pointer to segment
    PEQFBBLOCK  pBlockMark;

    while ( ulSegNum < pTBDoc->ulMaxSeg )          // free previous allocations
    {
       pSeg = EQFBGetSegW(pTBDoc, ulSegNum );
       UtlAlloc( (PVOID *) &(pSeg->pusBPET), 0L, 0L, NOMSG );

       if (pSeg->pusHLType) UtlAlloc((PVOID *)&(pSeg->pusHLType),0L,0L,NOMSG);

       if ( (pTBDoc->docType == SERVPROP_DOC) || ( pTBDoc->docType == SERVSOURCE_DOC ))
       {
         UtlAlloc( (PVOID *) &(pSeg->pDataW), 0L, 0L, NOMSG );
       } /* endif */

       if ( pSeg->pData )
       {
          UtlAlloc((PVOID *) &pSeg->pData, 0L, 0L, NOMSG );
       } /* endif */

       ulSegNum++;
    } /* endwhile */
    pTBDoc->ulMaxSeg  = 0;
    pTBDoc->ulMaxLine = 0;                         // increment line number
    pBlockMark = (PEQFBBLOCK)pTBDoc->pBlockMark;               // reset marked area
    pBlockMark->pDoc = NULL;

    if ( pTBDoc->pSegTables )
    {
      // remove all segment tables but first one

      // remove segment arrays
      PTBSEGMENTTABLE pSegTable = pTBDoc->pSegTables;
      ULONG ulTables = pTBDoc->ulSegTables;
      while ( ulTables > 1 )
      {
        pSegTable++;
        UtlAlloc((PVOID *) &pSegTable->pSegments, 0L, 0L, NOMSG );
        ulTables--;
      } /*endwhile */

      // resize segment table array to one single entry
      if ( pTBDoc->ulSegTables > 1 )
      {
         ULONG ulOldSize = sizeof( TBSEGMENTTABLE ) * pTBDoc->ulSegTables;
         ULONG ulNewSize = sizeof( TBSEGMENTTABLE ) * 1;
         UtlAlloc((PVOID *) &(pTBDoc->pSegTables), ulOldSize, ulNewSize, ERROR_STORAGE );
         pTBDoc->ulSegTables = 1;
      } /* endif */

      pTBDoc->pSegTables->ulSegments = 0;     // init number of stored segs
    } /* endif */

    EQFBAddSegW( pTBDoc, &tbInitSegment );          // add dummy segment
}

static
void EQFClearWindow
(
   PTBDOCUMENT  pTBDoc,                            // pointer to document ida
   BOOL         fRedraw                            // issue a redraw
)
{
   EQFClearSegTable( pTBDoc );

   pTBDoc->lCursorRow = 0;
   pTBDoc->lSideScroll = 0;
   pTBDoc->mouseRow =  pTBDoc->mouseCol = 0;
   pTBDoc->TBCursor.ulSegNum = 1;
   pTBDoc->TBCursor.usSegOffset = 0;
   EQFBScrnLinesFromSeg                   // build line table
          ( pTBDoc,
            pTBDoc->lCursorRow,           // starting row
            (pTBDoc->lScrnRows-pTBDoc->lCursorRow),// number of rows
            &(pTBDoc->TBCursor) );          // starting segment

   pTBDoc->Redraw |= REDRAW_ALL;            // redraw the screen
   if ( fRedraw )
   {
       EQFBScreenData( pTBDoc );
   } /* endif */
}


/*******************************************************************************
*
*       function:       GetPropSZ
*
* -----------------------------------------------------------------------------
*       Returns a pointer to the indicated proposal.
*******************************************************************************/
static
PSZ_W  GetPropSZ
(
  PDOCUMENT_IDA  pDoc,                       // pointer to document ida
  ULONG   ulNum,                             // requested number of prop in stEQFSab
  PULONG  pulLen                             // length of text
)
{
  PCHAR_W       pucProp;                        // ptr to prop.buffer
  PSZ_W         pszProp = NULL;                 // return pointer
  ULONG         ulLen;                          // len of props
  PSTEQFSAB     pstEQFSab;                      // pointer to send a head buff.

  *pulLen = 0;

  pstEQFSab = pDoc->stEQFSab + pDoc->usFI;  // pointer to foreground segm
  if (pDoc->fForeground &&
       pstEQFSab->fbInUse && ulNum <= EQF_NPROP_TGTS)
  {
    if (ulNum == 0)                             // request for source segment
    {
      pszProp = pstEQFSab->pucSourceSeg;        // addr. of source
      *pulLen = UTF16strlenCHAR(pszProp);
    }
    else
    {
      ulNum -= 1;
      pucProp = pstEQFSab->pszSortTargetSeg[ulNum];
      if (pucProp)
      {
          pszProp = pucProp;
          ulLen = UTF16strlenCHAR(pucProp);
          *pulLen = ulLen;
      }
    } // end if (source/target selection)
  } // end if (valid foreground segment)

  return pszProp;
} // end 'PropSZ'
/* $PAGEIF20 */


/*******************************************************************************
*
*       function:       GetPropData
*
* -----------------------------------------------------------------------------
*       Returns a pointer to the indicated proposal.
*******************************************************************************/
static
PSZ_W  GetPropData
(
  PDOCUMENT_IDA  pDoc,                       // pointer to document ida
  ULONG   ulNum,                             // requested number of prop in stEQFSab
  PULONG  pulLen                             // length of text
)
{
  PCHAR_W       pucProp;                        // ptr to prop.buffer
  PSZ_W         pszPropData = NULL;                 // return pointer
  ULONG         ulLen;                          // len of props
  PSTEQFSAB     pstEQFSab;                      // pointer to send a head buff.

  *pulLen = 0;

  pstEQFSab = pDoc->stEQFSab + pDoc->usFI;  // pointer to foreground segm
  if (pDoc->fForeground && (pstEQFSab->fbInUse && ulNum <= EQF_NPROP_TGTS) )
  {
    if (ulNum == 0)                             // request for source segment
    {
      pszPropData = EMPTYSTRINGW;    
      *pulLen = 0;
    }
    else
    {
      ulNum -= 1;
      pucProp = pstEQFSab->pszSortPropsData[ulNum];
      if (pucProp)
      {
          pszPropData = pucProp;
          ulLen = UTF16strlenCHAR(pucProp);
          *pulLen = ulLen;
      }
    } // end if (source/target selection)
  } // end if (valid foreground segment)

  return pszPropData;
} // end 'GetPropData'


/*******************************************************************************
*
*       function:       GetSrcPropSZ
*
* -----------------------------------------------------------------------------
*       Returns a pointer to the indicated source of proposal.
*******************************************************************************/
static
PSZ_W  GetSrcPropSZ
(
  PDOCUMENT_IDA  pDoc,                       // pointer to document ida
  USHORT usNum                               // requested number
)
{
  PCHAR_W       pucProp;                        // ptr to prop.buffer
  PSZ_W         pszProp = NULL;                 // return pointer
  PSTEQFSAB     pstEQFSab;                      // pointer to send a head buff.

  pstEQFSab = pDoc->stEQFSab + pDoc->usFI;  // pointer to foreground segm
  if (pDoc->fForeground &&
       pstEQFSab->fbInUse && usNum <= EQF_NPROP_TGTS)
  {
    if (usNum == 0)                             // request for source segment
    {
      pszProp = pstEQFSab->pucSourceSeg;        // addr. of source
    }
    else
    {
      usNum -= 1;
      pucProp = pstEQFSab->pszSortPropsSeg[usNum];
      if (pucProp)
      {
        pszProp = pucProp;
      }
    } // end if (source/target selection)
  } // end if (valid foreground segment)
  if (!pszProp )
  {
    pszProp = EMPTY_STRINGW;
  } /* endif */
  return pszProp;
} // end 'PropSZ'
/* $PAGEIF20 */

/*******************************************************************************
*
*       function:       EQF_Allocate
*
* -----------------------------------------------------------------------------
*       Accesses/releases generic structure and paste buffer (in NAMED shared
*       memory segments).
*******************************************************************************/

USHORT  EQF_Allocate (BOOL fAccess)

{
  USHORT        usRc = 0;

  if (fAccess)
  {
    EQFGETSTRUCT();
  } /* endif */
  return usRc;
} // end 'EQF_Allocate'
/* $PAGEIF20 */


/*******************************************************************************
*
*       function:       PropLevel
*
* -----------------------------------------------------------------------------
*       Returns the match level for the indicated proposal.
*******************************************************************************/
static
SHORT  PropLevel
(
  PDOCUMENT_IDA  pDoc,                       // pointer to document ida
  ULONG          ulNum
)
{
  if (pDoc->fForeground && (pDoc->stEQFSab+pDoc->usFI)->fbInUse && ulNum)
    return (pDoc->stEQFSab+pDoc->usFI)->usLevel[ulNum - 1];    // match level array
  else
    return -1;
} // end 'PropLevel'
/* $PAGEIF20 */


/*******************************************************************************
*
*       function:       EqualFlag
*
* -----------------------------------------------------------------------------
*       Returns the equal level for the indicated proposal.
*******************************************************************************/
static
SHORT  EqualFlag
(
  PDOCUMENT_IDA  pDoc,                       // pointer to document ida
  ULONG          ulNum
)
{
  if (pDoc->fForeground && (pDoc->stEQFSab+pDoc->usFI)->fbInUse && ulNum)
    return (pDoc->stEQFSab+pDoc->usFI)->fEqualSource[ulNum - 1]; // equal flag array
  else
    return -1;
} // end 'EqualFlag'
/* $PAGEIF20 */

/*******************************************************************************
*
*       function:       MachineTrans
*
* -----------------------------------------------------------------------------
*       Returns the machine translation indictation
*******************************************************************************/
static
SHORT  MachineTrans
(
  PDOCUMENT_IDA  pDoc,                       // pointer to document ida
  USHORT usNum
)
{
   SHORT sMachine = 0;

   // get machine translation flag
   if (pDoc->fForeground && (pDoc->stEQFSab+pDoc->usFI)->fbInUse && usNum)
   {
    sMachine = (SHORT) (pDoc->stEQFSab+pDoc->usFI)->usMachineTrans[usNum-1];
   } /* endif */      

   // clear fuzzy/repl-fuzzy flag for exact matches and machine matches
   if ( (sMachine & MACHINE_TRANS_PROP) || (sMachine & EXACT_PROP) )
   {
     sMachine &= ~(FUZZY_REPLACE_PROP | FUZZY_PROP);
   } /* endif */

   // handle fuzzy global memory proposals 
   if ( (sMachine & GLOBMEM_TRANS_PROP) || (sMachine & GLOBMEMSTAR_TRANS_PROP) )
   {
     USHORT usFuzzy = (pDoc->stEQFSab+pDoc->usFI)->usFuzzyPercents[usNum-1];
     if ( usFuzzy >= 100 )
     {
       // clear fuzzy flag
       sMachine &= ~(FUZZY_REPLACE_PROP | FUZZY_PROP);
     }
     else
     {
       // set global memory fuzzy flag
       sMachine = GLOBMEMFUZZY_TRANS_PROP;
     } /* endif */        
   } /* endif */

   return sMachine;
} // end 'Machine Trans'




/*******************************************************************************
*
*       function:       EQF_Unpack
*
* -----------------------------------------------------------------------------
*       Unpacks dictionary/memory filenames from a serial buffer.
*******************************************************************************/
static
ULONG  EQF_Unpack (PUCHAR pucBuffer, PSZ pszTgt, USHORT usMax)

{
  ULONG      ulLen, ulNum = 0;

  while (*pucBuffer && usMax)
  {
    ulLen = strlen ((PSZ)pucBuffer);
    strncpy (pszTgt, (PSZ)pucBuffer, MAX_LONGFILESPEC-1);
    ulNum++;
    pszTgt += MAX_LONGFILESPEC;
    pucBuffer += ulLen + 2;
    usMax--;
  } // endwhile

  return ulNum;
} // end 'EQF_Unpack'
/* $PAGEIF20 */

/*******************************************************************************
*
*       function        SetLineNum
*
* -----------------------------------------------------------------------------
*       Set maximum line number
*******************************************************************************/

static
VOID  SetLineNum
(
   PTBDOCUMENT pTBDoc,              // pointer to document
   PSZ_W       pData                // pointer to data
)
{
   CHAR_W  ch;
   if (pData)                // avoid trap if pData is NULL- should not happen
   {
     while ( (ch = *pData) != NULC )
     {
        if ( ch == '\n')
        {
           pTBDoc->ulMaxLine++;
        } /* endif */
        pData ++;
     } /* endwhile */
   }
   else
   {
	   // for debug only P018339
	   USHORT usI = 0;
	   usI = usI + 1;
   }
}



/*******************************************************************************
*
*       function        ClearSAB
*
* -----------------------------------------------------------------------------
*       clear the passed SendAHead buffer
*******************************************************************************/
static
VOID ClearSAB
(
   PSTEQFSAB   pstEQFSab                        // pointer to SAB
)
{
      pstEQFSab->usDictCount = 0;               // dictinary count = 0
      pstEQFSab->usPropCount = 0;               // proposals count = 0
      pstEQFSab->fDAReady    = FALSE;           // no dictionary data available
      pstEQFSab->fMTReady    = FALSE;            // no MT data available
      if ( pstEQFSab->pucSourceSeg )
      {
        UTF16memset (pstEQFSab->pucSourceSeg,  '\0', EQF_SRCLEN);
      } /* endif */
      if ( pstEQFSab->pucDictWords )
      {
        UTF16memset (pstEQFSab->pucDictWords,  '\0', EQF_DICTLEN);
      } /* endif */

      memset(&pstEQFSab->usMachineTrans[0], 0,
               sizeof(pstEQFSab->usMachineTrans));
      memset(&pstEQFSab->usFuzzyPercents[0], 0,
               sizeof(pstEQFSab->usFuzzyPercents));

      memset( &pstEQFSab->pszSortTargetSeg[0], 0, sizeof(pstEQFSab->pszSortTargetSeg) );
      memset( &pstEQFSab->pszSortPropsSeg[0], 0, sizeof(pstEQFSab->pszSortPropsSeg) );
      memset( &pstEQFSab->fInvisible[0], 0, sizeof(pstEQFSab->fInvisible) );
      memset(&pstEQFSab->pszSortPropsData[0], '\0', sizeof(pstEQFSab->pszSortPropsData) );

      pstEQFSab->fbInUse = FALSE;                // indicate 'not in use'
}






/*******************************************************************************
*
*       function        CleanUp
*
* -----------------------------------------------------------------------------
*       free all resources for a special window
*******************************************************************************/
VOID CleanUp
(
   HWND        hwnd                              // window handle
)
{
   PTWBSDEVICE   pDevice;              // pointer to document ida
   PTBDOCUMENT   pTBDoc;               // pointer to device data

   PTBSEGMENTTABLE pSegTable;          // ptr for segment table deleting
   ULONG           ulI, ulJ;           // loop counter
   PTBSEGMENT      pSegment;           // ptr for segment deleting

   pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
   if ( pDevice )
   {
     pTBDoc = &(pDevice->tbDoc);

   if ( pTBDoc->pDocTagTable )
   {
     TAFreeTagTable( (PLOADEDTABLE)pTBDoc->pDocTagTable );
   } /* endif */

   if ( pTBDoc->pvGlobalMemOptFile ) GlobMemFreeOptionFile( &(pTBDoc->pvGlobalMemOptFile) );

                                  // free previous segm. allocations
   pSegTable = pTBDoc->pSegTables;
   for ( ulI = 0; ulI < pTBDoc->ulSegTables; ulI++ )
   {
      pSegment = pSegTable->pSegments;
      for ( ulJ = 0; ulJ < pSegTable->ulSegments; ulJ++ )
      {
         if ( pSegment->pusBPET )
         {
            UtlAlloc( (PVOID *) &pSegment->pusBPET, 0L, 0L, NOMSG );
         } /* endif */
         if(pSegment->pusHLType) UtlAlloc((PVOID *)&pSegment->pusHLType,0L,0L,NOMSG);
         if (pSegment->pContext) UtlAlloc((PVOID *)&(pSegment->pContext),0L,0L,NOMSG);
         if ( pSegment->pData )
         {
            UtlAlloc((PVOID *) &pSegment->pData, 0L, 0L, NOMSG );
         } /* endif */
         pSegment++;
      } /* endfor */
      UtlAlloc( (PVOID *) &pSegTable->pSegments, 0L, 0L, NOMSG );
      pSegTable++;
   } /* endfor */
   UtlAlloc( (PVOID *) &pTBDoc->pSegTables, 0L, 0L, NOMSG );

   UtlAlloc( (PVOID *) &pTBDoc->pTokBuf, 0L, 0L, NOMSG );
   UtlAlloc( (PVOID *) &pTBDoc->pInBuf, 0L, 0L, NOMSG );
   UtlAlloc( (PVOID *) &pTBDoc->pBlockMark, 0L, 0L, NOMSG );
   if (pTBDoc->pContext) UtlAlloc((PVOID *)&(pTBDoc->pContext),0L,0L,NOMSG);
   if (pTBDoc->pWSList) UtlAlloc((PVOID *)&(pTBDoc->pWSList), 0L, 0, NOMSG);
   /*******************************************************************/
   /* free the protecttable entries                                   */
   /* don't care about errors ....                                    */
   /*******************************************************************/
   TAEndProtectTable( &pTBDoc->hModule,
                      (PFN*)&pTBDoc->pfnUserExit,
                      (PFN*)&pTBDoc->pfnCheckSegExit,
                      (PFN*)&pTBDoc->pfnShowTrans,
                      (PFN*)&pTBDoc->pfnTocGoto,
                      (PFN*)&pTBDoc->pfnUserExitW,
                      (PFN*)&pTBDoc->pfnCheckSegExitW);

   ANCHORWNDIDA( hwnd, NULL );
   } /* endif */
}

/*******************************************************************************
*
*       function        HandleWMCharr
*
* -----------------------------------------------------------------------------
*       handle all keystrokes in service windows
*******************************************************************************/
MRESULT HandleWMCharr
(
   HWND        hwnd,               // window handle
   USHORT      msg,
   WPARAM      mp1,
   LPARAM      mp2
)
{
   PTWBSDEVICE   pDevice;          // pointer to document ida
   PTBDOCUMENT   pTBDoc;           // pointer to device data
   PDOCUMENT_IDA pDoc;             // pointer to document ida
   BOOL          fDefault = FALSE;     // default processing required
   BOOL          fShift;
   BOOL          fCtrl;
   BOOL          fAlt;              // alt key pressed
   UCHAR         ucCode;
   MRESULT       mResult = MRFROMSHORT(TRUE);// indicate message is processed
   PFUNCTIONTABLE pFuncTab = get_FuncTab();

   /*******************************************************************/
   /* determine control key states ....                               */
   /*******************************************************************/
     fCtrl  = GetKeyState(VK_CTRL ) >> 15 ;
     fShift = GetKeyState(VK_SHIFT) >> 15 ;
     fAlt   = GetKeyState(VK_ALT) >> 15 ;
     ucCode = (UCHAR) mp1;

   pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
   pTBDoc = &(pDevice->tbDoc);
   if ( fCtrl )  // ctrl key pressed
   {
     if ( fShift )
     {
       switch ( ucCode )
       {
         case  VK_LEFT:
            if ( pTBDoc->docType != SERVPROP_DOC )
	    	{
			  EQFBFuncMarkPrevWord( pTBDoc );
			}
			else
			{
			//activate BidiLRSwap action in FuncTab
			  void (*function)( PTBDOCUMENT );
			  function = (pFuncTab + MARKPREV_FUNC)->function;
			  (*function)(pTBDoc);
            }
           break;
         case  VK_RIGHT:
            if ( pTBDoc->docType != SERVPROP_DOC )
	    	{
			  EQFBFuncMarkNextWord( pTBDoc );
			}
			else
			{
			//activate BidiLRSwap action in FuncTab
			  void (*function)( PTBDOCUMENT );
			  function = (pFuncTab + MARKNEXT_FUNC)->function;
			  (*function)(pTBDoc);
            }
           break;
         default :
           break;
       } /* endswitch */
     }
     else
     {
       switch ( ucCode )
       {
          case 'C':
          case VK_INSERT:
             EQFBFuncCopyToClip( pTBDoc );
             break;
          case VK_HOME:
             EQFBFuncTopDoc( pTBDoc );
             break;
          case VK_END:
             EQFBFuncBottomDoc( pTBDoc );
             break;
          case VK_LEFT:
            if ( pTBDoc->docType != SERVPROP_DOC )
			{
			  EQFBFuncPrevWord( pTBDoc );
			}
			else
			{
			//activate BidiLRSwap action in FuncTab
			  void (*function)( PTBDOCUMENT );
			  function = (pFuncTab + PREVWORD_FUNC)->function;
			  (*function)(pTBDoc);
             }
             break;
          case VK_RIGHT:
              if ( pTBDoc->docType != SERVPROP_DOC )
			  {
				   EQFBFuncNextWord( pTBDoc );
			  }
			  else
			  {
				   void (*function)( PTBDOCUMENT );
				   function = (pFuncTab + NEXTWORD_FUNC)->function;
				   (*function)(pTBDoc);
              }
             break;
          case VK_F4 :
          case VK_F5 :
          case VK_F6 :
          case VK_F7 :
          case VK_F8 :
          case VK_F9 :
          case VK_F10:
          case VK_CTRL:
             break;
          default:
             fDefault = TRUE;
             break;
       } /* endswitch */
     } /* endif */
   }
   else if ( fAlt && fShift )                    // alt key pressed
   {
     switch ( ucCode )
     {
        case VK_HOME:
          EQFBFuncMarkSegStartCUA( pTBDoc );
          break;
        case VK_END:
          EQFBFuncMarkSegEndCUA( pTBDoc );
          break;
        default:
          break;
     } /* endswitch */
   }
   else if ( fShift )  // shift key pressed
   {
      switch ( ucCode )
      {
         case VK_INSERT:
            WinAlarm( HWND_DESKTOP, WA_WARNING );
            break;
         case VK_LEFT:
              if ( pTBDoc->docType != SERVPROP_DOC )
			  {
				  EQFBFuncMarkLeftCUA( pTBDoc );
			  }
			  else
			  {
				  void (*function)( PTBDOCUMENT );
				  function = (pFuncTab + MARKLEFT_FUNC)->function;
				  (*function)(pTBDoc);
              }
            break;
         case VK_RIGHT:
              if ( pTBDoc->docType != SERVPROP_DOC )
			  {
				  EQFBFuncMarkRightCUA( pTBDoc );
			  }
			  else
			  {
				  void (*function)( PTBDOCUMENT );
				  function = (pFuncTab + MARKRIGHT_FUNC)->function;
				  (*function)(pTBDoc);
              }
            break;
         case VK_UP:
            EQFBFuncMarkUpCUA( pTBDoc );
            break;
         case VK_DOWN:
            EQFBFuncMarkDownCUA( pTBDoc );
            break;
         case VK_TAB:     // shift tab == backtab...
            EQFBFuncBacktab( pTBDoc );
            break;
         case VK_SHIFT:                // do nothing
         case VK_ESC:                  // do nothing
            break;
          case VK_HOME:
            EQFBFuncMarkStartCUA( pTBDoc );
            break;
          case VK_END:
            EQFBFuncMarkEndCUA( pTBDoc );
            break;
         default:
            fDefault = TRUE;
            break;
      } /* endswitch */
   }
   else                                   // no control key
   {
      switch ( ucCode )
      {
         case VK_ESC:
           switch ( pDevice->usType )
           {
              case SERVSOURCE_DOC:
                pTBDoc = &(pDevice->tbDoc);    // pointer to document struct
                WinShowWindow( pTBDoc->hwndFrame , FALSE );
                break;
              case SERVDICT_DOC:
              case SERVPROP_DOC:
              default :
                break;
           } /* endswitch */
           /**************************************************************/
           /* change input focus to editor window                        */
           /**************************************************************/
           pDoc = (PDOCUMENT_IDA)pDevice->pDoc;   // get pointer to doc ida
           SendMessage(pDoc->pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                       0, MP2FROMHWND( pDoc->pstEQFGen->hwndEditorTgt ) );
           break;

         case VK_UP:
            EQFBFuncMarkClear( pTBDoc );
            EQFBFuncUp( pTBDoc );
            break;
         case VK_DOWN:
            EQFBFuncMarkClear( pTBDoc );
            EQFBFuncDown( pTBDoc );
            break;
         case VK_LEFT:
            EQFBFuncMarkClear( pTBDoc );
            if ( pTBDoc->docType != SERVPROP_DOC )
			{
			  EQFBFuncLeft( pTBDoc );
			}
			else
			{
			//activate BidiLRSwap action in FuncTab
			  void (*function)( PTBDOCUMENT );
			  function = (pFuncTab + LEFT_FUNC)->function;
			  (*function)(pTBDoc);
            }
            break;
         case VK_RIGHT:
            EQFBFuncMarkClear( pTBDoc );
            if ( pTBDoc->docType != SERVPROP_DOC )
			{
			  EQFBFuncRight( pTBDoc );
			}
			else
			{
			//activate BidiLRSwap action in FuncTab
			  void (*function)( PTBDOCUMENT );
			  function = (pFuncTab + RIGHT_FUNC)->function;
			  (*function)(pTBDoc);
            }
            break;
         case VK_HOME:
            EQFBFuncStartLine( pTBDoc );
            break;
         case VK_END:
            EQFBFuncEndLine( pTBDoc );
            break;
         case VK_PAGEUP:
            EQFBFuncPageUp( pTBDoc );
            break;
         case VK_PAGEDOWN:
            EQFBFuncPageDown( pTBDoc );
            break;
         case VK_TAB:
            EQFBFuncTab( pTBDoc );
            break;
          case VK_ENTER:
            switch ( pDevice->usType )
            {
               case SERVDICT_DOC:         // start dict lookup for dict terms
                  StartDictLookUp( pDevice, DICTIONARY_LOOKUP );
                  break;
               case SERVPROP_DOC:         // display source of proposals
                  pDoc = (PDOCUMENT_IDA)pDevice->pDoc;   // get pointer to doc ida
                  InsertSource( &(pDoc->tbDevSource), TRUE, TRUE );
                  break;
               case SERVSOURCE_DOC:
                  WinAlarm( HWND_DESKTOP, WA_WARNING );
                  break;
               default :
                  fDefault = TRUE;
                  break;
            } /* endswitch */
            break;

         case '-':           // for system icon...
         case VK_MENU:
            break;
          case '+':
            {
              POINT  lppt;
              GetCaretPos( &lppt );
              return( EQFBDispClass( hwnd, WM_RBUTTONDOWN,
                                     0, MP2FROMLONG(*((PLONG)&lppt)),
                                     &(pDevice->tbDoc)) );
            }
            break;
          case VK_F1:
            /**************************************************************/
            /* Trigger help by posting HM_HELPSUBITEM_NOT_FOUND to TWB    */
            /**************************************************************/
            {
              USHORT usId;
              switch ( pDevice->usType )
              {
                 case SERVDICT_DOC:
                   usId = ID_TWBS_DICT_WINDOW;
                   break;
                 default :
                 case SERVPROP_DOC:
                 case SERVSOURCE_DOC:
                   usId = ID_TWBS_PROP_WINDOW;
                   break;
              } /* endswitch */
              PostMessage( (HWND)UtlQueryULong( QL_TWBFRAME ),
                           HM_HELPSUBITEM_NOT_FOUND,
                           0,
                           MP2FROM2SHORT( usId, usId )) ;
            }
//            return mResult;
            break;

         /*************************************************************/
         /* allow for deletion of a displayed proposal                */
         /*************************************************************/
         case VK_DELETE:
            switch (  pDevice->usType )
            {
              case SERVPROP_DOC :
                DeleteProposal( pDevice );
                if ( pDevice->tbDoc.hwndRichEdit )
                {
                  /****************************************************/
                  /* skip any further processing...                   */
                  /****************************************************/
                  return TRUE;
                } /* endif */
                break;
              default :
                WinAlarm( HWND_DESKTOP, WA_WARNING );
                break;
            } /* endswitch */
            break;
         default :
            fDefault = TRUE;
            break;
      } /* endswitch */
    } /* endif */

    /******************************************************************/
    /* pass on any default keystrokes to editor window....            */
    /******************************************************************/
    if ( fDefault )
    {
      pDoc = (PDOCUMENT_IDA)pDevice->pDoc;            // pointer to document ida
        /**************************************************************/
        /* change input focus to editor window                        */
        /**************************************************************/
        SendMessage(pDoc->pstEQFGen->hwndTWBS, WM_EQF_SETFOCUS,
                    0, MP2FROMHWND( pDoc->pstEQFGen->hwndEditorTgt ) );
      WinPostMsg( pDoc->pstEQFGen->hwndEditorTgt, msg, mp1, mp2 );
    }
    else
    {
      if ( !pTBDoc->hwndRichEdit )
      {
           mResult = DefWindowProcW( hwnd, msg, mp1, mp2 );
      }
    } /* endif */
    EQFBScreenData( pTBDoc );          // display screen
    EQFBScreenCursor( pTBDoc );        // update cursor and sliders
    return mResult;
}




/*******************************************************************************
*
*       function        HandleWMMove
*
* -----------------------------------------------------------------------------
*       save the position and size of service windows
*******************************************************************************/
static
VOID HandleWMMove
(
   HWND  hwnd
)
{
   PTWBSDEVICE   pDevice;          // pointer to document ida
   PDOCUMENT_IDA pDoc;             // pointer to document ida
   PRECTL        prcl;             // pointer to Rectl struct
   RECTL         rclDummy;         // dummy Rectl structure
   PSTEQFGEN     pstEQFGen;        // pointer to generic structure
   SWP           swp;              // size position array
   HWND          hwndOrder;        // window handle

   pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );
   if (pDevice &&
        (( pDoc = (PDOCUMENT_IDA) pDevice->pDoc) != NULL))
   {
     pstEQFGen = pDoc->pstEQFGen;       // pointer to generic structure
     switch ( pDevice->usType )
     {
        case SERVPROP_DOC:
           prcl = &(pstEQFGen->rclProposals);  //  prop.wnd
           break;
        case SERVDICT_DOC:
           prcl = &(pstEQFGen->rclDictionary); //  dict.wnd
           break;
        case SERVSOURCE_DOC:
           prcl = &(pstEQFGen->rclSource);     //  src.wnd
           break;
        default:
           prcl = &rclDummy;
           break;
     } /* endswitch */

     hwndOrder = hwnd;

     if (prcl && hwndOrder && WinQueryWindowPos ( hwndOrder, &swp))
     {
       PRECTL_XLEFT(prcl)      = (LONG)swp.x;
       PRECTL_XRIGHT(prcl)     = (LONG)(swp.x + swp.cx);
       PRECTL_YBOTTOM(prcl)    = (LONG)swp.y;
       PRECTL_YTOP(prcl)       = (LONG)(swp.y + swp.cy);
     } /* endif */
   } /* endif */
}

/*******************************************************************************
*
*       function        HandleWMSize
*
* -----------------------------------------------------------------------------
*       get the new size and set the document property values
*******************************************************************************/

static
VOID HandleWMSize
(
  HWND   hwnd,
  WPARAM mp1,
  LPARAM mp2
)
{
   PTWBSDEVICE   pDevice;          // pointer to document ida
   PTBDOCUMENT   pTBDoc;           // pointer to device data
   PDOCUMENT_IDA pDoc;             // pointer to document ida
   PRECTL        prcl;             // pointer to Rectl struct
   RECTL         rclDummy;         // dummy Rectl structure
   PSTEQFGEN     pstEQFGen;        // pointer to generic structure
   SWP           swp;              // size window position
   HWND          hwndOrder;        // window handle

   mp1;
   pDevice = ACCESSWNDIDA( hwnd, PTWBSDEVICE );

   if (pDevice &&
       ( (pTBDoc = &(pDevice->tbDoc)) != NULL))            // get pointer to data
   {
     pTBDoc->xClient = SHORT1FROMMP2(mp2) ;
     pTBDoc->yClient = SHORT2FROMMP2(mp2) ;
              // set row and columns of currently available size
     pTBDoc->lScrnCols = pTBDoc->xClient / pTBDoc->cx -1 ; //@@
     pTBDoc->lScrnRows = pTBDoc->yClient / pTBDoc->cy ;

     // GQ: avoid traps with 0 screen rows (is used as index with (screen rows - 1) ...)
     if ( pTBDoc->lScrnRows == 0 ) pTBDoc->lScrnRows = 1;

     // readjust allocations for specifics bidi array
     ReAllocArabicStruct( pTBDoc );
     //  save new size of service windows in generic struct
     pDoc = (PDOCUMENT_IDA) pDevice->pDoc;
     pstEQFGen = pDoc->pstEQFGen;       // pointer to generic structure
     if ( pTBDoc->ulMaxSeg )     // document already loaded?
     {
        switch ( pDevice->usType )
        {
           case SERVPROP_DOC:
              prcl = &(pstEQFGen->rclProposals);  //  prop.wnd
              InsertProposal( pDevice, TRUE );
              break;
           case SERVDICT_DOC:
              prcl = &(pstEQFGen->rclDictionary); //  dict.wnd
              InsertDictionary( pDevice );
              break;
           case SERVSOURCE_DOC:
              prcl = &(pstEQFGen->rclSource);     //  src.wnd
              InsertSource( pDevice, TRUE, TRUE );
              break;
           default:
              prcl = &rclDummy;
              break;
        } /* endswitch */
        hwndOrder = hwnd;

        if (hwndOrder && WinQueryWindowPos ( hwndOrder, &swp))
        {
           PRECTL_XLEFT(prcl)      = (LONG)swp.x;
           PRECTL_XRIGHT(prcl)     = (LONG)(swp.x + swp.cx);
           PRECTL_YBOTTOM(prcl)    = (LONG)swp.y;
           PRECTL_YTOP(prcl)       = (LONG)(swp.y + swp.cy);
        } /* endif */
     } /* endif */

	 if ( pTBDoc->hwndRichEdit )
	 {
	   RECT rc;
	   GetClientRect( hwnd, &rc );
	   SetWindowPos( pTBDoc->hwndRichEdit, HWND_TOP,
					 0, 0, rc.right, pTBDoc->yClient,
					 SWP_SHOWWINDOW );
	 } /* endif */

   } /* endif */
}

/*******************************************************************************
*
*       function        Start Dictionary Lookup
*
* -----------------------------------------------------------------------------
*       get the double clicked item and start the dictionary lookup/edit for it
*       if it is a Headword of a term otherwise ignore it
*******************************************************************************/

static
VOID StartDictLookUp
(
   PTWBSDEVICE pDevice,                // pointer to device data
   USHORT      usAction                // lookup/edit indicator
)
{
   PDOCUMENT_IDA  pDoc;                // pointer to document ida
   PTBDOCUMENT pTBDoc;                 // pointer to devices struct
   PSTHDW      pstHDW;                 // pointer to headwords

   pTBDoc = &(pDevice->tbDoc);         // get pointer to data

   if ( pTBDoc->TBCursor.ulSegNum == 0 ) // stay at old position
   {
      WinAlarm( HWND_DESKTOP, WA_WARNING );
   }
   else
   {
      pDoc =  (PDOCUMENT_IDA)pDevice->pDoc;              // get pointer to data
      pstHDW = pDoc->stHDW;               // pointer to dictionary area

      while ( pstHDW->pData )
      {
         if ( pstHDW->ulSegNum == pTBDoc->TBCursor.ulSegNum )
         {
            break;                        // match found
         }
         else
         {
            pstHDW++;                     // next entry
         } /* endif */
      } /* endwhile */

      if ( pstHDW->pData )                 // headword found...
      {                                    // so lookit up
        PSZ_W pszHeadword = pstHDW->pData;
        if ( (*pszHeadword == STYLEPREFIX_NOTALLOWED) ||
             (*pszHeadword == STYLEPREFIX_PREFERRED) ||
             (*pszHeadword == STYLEPREFIX_UNDEFINED) )
        {
          pszHeadword += 1;             // skip style prefix
        } /* endif */
        WinSendMsg (pDoc->pstEQFGen->hwndTWBS,
                    WM_EQF_PROCESSTASK,
                    MP1FROMSHORT(usAction),
                    MP2FROMP( pszHeadword ));
      }
      else
      {
        WinAlarm( HWND_DESKTOP, WA_WARNING );
      } /* endif */

   } /* endif */

}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFR_ProofSeg
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       performs proofreading against the passed string.
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc      pointer to document inst ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     check that the morphological dictionary is activated
//                    if not try to activate it
//
//                    if okay remove any tags and attributes from the segment
//                    put the resulting segment into the same buffer
//                    and call the MorphVerify function for verifying the seg
//
//------------------------------------------------------------------------------


VOID  EQFR_ProofSeg
(
   PDOCUMENT_IDA  pDoc                          // pointer to document inst ida
)
{
   BOOL   fOK = TRUE;                           // success indicator
   PTBDOCUMENT   pTBDoc;                        // pointer to document
   PSTEQFPCMD    pstPCmd;                       // request queue
   PSTEQFGEN     pstEQFGen;                     // pointer to generic structure
   PTERMLENOFFS  pTermList = NULL;              // ptr to created term list
   USHORT        usLen = 0;                     // length of buffer
   USHORT        usRC;                          // return code from NLP services
   PSZ_W           pData;                         // pointer to data
   pstEQFGen = pDoc->pstEQFGen;
   pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct

   pstEQFGen->usRC = 0;                          // reset return code
   pstEQFGen->szMsgBuffer[0] = EOS;

   /*******************************************************************/
   /*  check that the morphological dictionary is activated           */
   /*  if not try to activate it                                      */
   /*******************************************************************/
   if ( pDoc->sTgtLanguage == -1)
   {
      fOK = InitProof( pDoc );
   } /* endif */


   /*******************************************************************/
   /*  if okay remove any tags and attributes from the segment        */
   /*  put the resulting segment into the same buffer                 */
   /*  and call the MorphVerify function for verifying the segment    */
   /*******************************************************************/
   if ( fOK )
   {
      pTBDoc = &(pDoc->tbDevProposal.tbDoc);
      EQFBNormSeg( pTBDoc, (PSZ_W)pstPCmd->ucbBuffer, (PSZ_W)pstPCmd->ucbBuffer );
      /****************************************************************/
      /* get rid of newline characters because they make trouble      */
      /* in the morphological analysis.                               */
      /****************************************************************/
      pData =(PSZ_W)( pstPCmd->ucbBuffer);
      while ( *pData )
      {
        if ( *pData == L'\n'/*LF*/ )
        {
          *pData = L' ';//BLANK;
        } /* endif */
        pData++;
      } /* endwhile */

      usRC = MorphVerify( pDoc->sTgtLanguage,
                          (PSZ_W)pstPCmd->ucbBuffer,
                          &usLen,                 // size in BYTEs of list
                          (PVOID *)&pTermList,
                          MORPH_ZTERMLIST, pDoc->ulTgtOemCP);
      if (  usRC != MORPH_OK )
      {
         fOK = FALSE;
         pstPCmd->usLen1 = 0;
         pstEQFGen->usRC = EQFRS_NOMORPH_DICT ;            // no morphol.dictionary
      }
      else
      {
        usLen = min( usLen, (sizeof( pstPCmd->ucbBuffer) / sizeof(CHAR_W)) - 1 );
        memcpy( pstPCmd->ucbBuffer, pTermList, usLen * sizeof(CHAR_W) );
        pstPCmd->usLen1 = usLen;
        UtlAlloc( (PVOID *) &pTermList, 0L, 0L, NOMSG );
      } /* endif */
   } /* endif */

  return;
} /* end of function EQFR_ProofSeg */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFR_ProofAdd
//------------------------------------------------------------------------------
// Function call:     EQFR_ProofAdd ( pDoc )
//------------------------------------------------------------------------------
// Description:       add the word to the addenda
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc    pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if language not already activated, activate it.
//                    if okay then
//                      add word to addenda
//                    endif
//------------------------------------------------------------------------------

VOID  EQFR_ProofAdd
(
   PDOCUMENT_IDA  pDoc                          // pointer to document inst ida
)
{
   PSTEQFPCMD    pstPCmd;              // request queue
   PSTEQFGEN     pstEQFGen;            // pointer to generic structure
   USHORT        usRC;                 // return of Morph services
   BOOL          fOK = TRUE;           // success indicator

   pstEQFGen = pDoc->pstEQFGen;
   pstPCmd = pstEQFGen->pstEQFPCmd;    // pointer to command struct

   pstEQFGen->usRC = 0;                // reset return code
   pstEQFGen->szMsgBuffer[0] = EOS;

   /*******************************************************************/
   /*  check that the addenda is activated                            */
   /*  if not try to activate it                                      */
   /*******************************************************************/
   if ( pDoc->sTgtLanguage == -1 )
   {
     fOK = InitProof( pDoc );
   } /* endif */


   //  if okay add the word to the addenda
   if ( fOK )
   {
      usRC = MorphAddToAddenda( pDoc->sTgtLanguage, (PSZ_W)pstPCmd->ucbBuffer,
                                pDoc->ulTgtOemCP);
      if ( usRC != MORPH_OK )
      {
         pstEQFGen->usRC = EQFRS_NOADDENDA_DICT;
      } /* endif */
   } /* endif */

  return;
} /* end of function EQFR_ProofAdd */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFR_ProofAid
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       get a word help for the misspelled word
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc       pointer to document inst ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if morph dictionary not activated, try to activate it
//                    if okay then
//                      call the spellaid for the specified word
//                    endif
//------------------------------------------------------------------------------

VOID  EQFR_ProofAid
(
   PDOCUMENT_IDA  pDoc                 // pointer to document inst ida
)
{

   BOOL  fOK = TRUE;                   // success indicator
   PSTEQFPCMD    pstPCmd;              // request queue
   PSTEQFGEN     pstEQFGen;            // pointer to generic structure
   USHORT        usListSize = 0;       // size of list
   PSZ           pszList = NULL;       // list of terms for aid
   USHORT        usRC;                 // return code

   pstEQFGen = pDoc->pstEQFGen;
   pstPCmd = pstEQFGen->pstEQFPCmd;    // pointer to command struct

   pstEQFGen->usRC = 0;                // reset return code
   pstEQFGen->szMsgBuffer[0] = EOS;


   /*******************************************************************/
   /*  check that the morphological dictionary is activated           */
   /*  if not try to activate it                                      */
   /*******************************************************************/
   if ( pDoc->sTgtLanguage  == -1 )
   {
      fOK = InitProof( pDoc );
   } /* endif */

   /*******************************************************************/
   /*  if okay try to get a word help for the misspelled word         */
   /*******************************************************************/
   if ( fOK )
   {
     usRC = MorphSpellAid( pDoc->sTgtLanguage, (PSZ_W)pstPCmd->ucbBuffer,
                           &usListSize, (PVOID *)&pszList, pDoc->ulTgtOemCP );

     /*****************************************************************/
     /* retrieve the returned information and put it into the buffer  */
     /*                                                               */
     /* Rational: No error condition will be handled, either we can   */
     /*           get a spellaid or not....                           */
     /*****************************************************************/
     if ( usRC == MORPH_OK )
     {
       memcpy( pstPCmd->ucbBuffer, pszList, usListSize );
       UtlAlloc( (PVOID *) &pszList, 0L, 0L, FALSE );
     } /* endif */
     pstPCmd->usLen1  = usListSize;              // return data len
   } /* endif */

   return;
} /* end of function EQFR_ProofAid */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     InitProof
//------------------------------------------------------------------------------
// Function call:     InitProof( PDOCUMENT_IDA )
//------------------------------------------------------------------------------
// Description:       do the initialisation requested for activating
//                    proofread
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA     pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     spellchecking services could be activated
//                    FALSE    else
//------------------------------------------------------------------------------
// Function flow:     get target language
//                    get language id
//                    return success indicator
//------------------------------------------------------------------------------
static
BOOL  InitProof
(
   PDOCUMENT_IDA  pDoc
)
{
   BOOL   fOK = TRUE;                           // success indicator
   PSTEQFGEN     pstEQFGen;                     // pointer to generic structure

   pstEQFGen = pDoc->pstEQFGen;                 // pointer generic struct

   // activate the morphological and the addenda dictionary
   if ( MorphGetLanguageID( pDoc->szDocTargetLang,
                            &pDoc->sTgtLanguage ) != MORPH_OK )
   {
     fOK = FALSE;
   } /* endif */
   else
   {
     pDoc->ulTgtOemCP = GetLangOEMCP( pDoc->szDocTargetLang);
   }

   if ( !fOK )
   {
     pstEQFGen->usRC = EQFRS_NOMORPH_DICT;
     ASCII2Unicode( pDoc->szDocTargetLang, pstEQFGen->szMsgBuffer, 0L );
     ASCII2Unicode( pDoc->szDocTargetLang, pDoc->szMsgBuffer, 0L );
   } /* endif */

   return fOK;
}


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     InitMorphSrcDict
//------------------------------------------------------------------------------
// Function call:     InitMorphSrcDict( PDOCUMENT_IDA );
//------------------------------------------------------------------------------
// Description:       do the initialisation requested for activating
//                    the morphological dict for the source
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA     pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE     spellchecking services could be activated
//                    FALSE    else
//------------------------------------------------------------------------------
// Function flow:     get source language
//                    get language id
//                    return success indicator
//------------------------------------------------------------------------------
static
BOOL  InitMorphSrcDict
(
   PDOCUMENT_IDA  pDoc
)
{
   BOOL   fOK = TRUE;                           // success indicator
   PSTEQFGEN     pstEQFGen;                     // pointer to generic structure

   pstEQFGen = pDoc->pstEQFGen;                 // pointer generic struct

   if ( MorphGetLanguageID( pDoc->szDocSourceLang,
                            &pDoc->sSrcLanguage ) != MORPH_OK )
   {
     fOK = FALSE;
   } /* endif */
   else
   {
     pDoc->ulSrcOemCP = GetLangOEMCP(pDoc->szDocSourceLang);
   }

  fOK = TRUE;

   if ( !fOK )
   {
     pstEQFGen->usRC = EQFRS_NOMORPH_DICT;
     ASCII2Unicode( pDoc->szDocSourceLang, pstEQFGen->szMsgBuffer,0L );
     ASCII2Unicode( pDoc->szDocSourceLang, pDoc->szMsgBuffer, 0L );
   } /* endif */

   return fOK;
}

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EQFR_GetSource
//------------------------------------------------------------------------------
// Function call:     EQFR_GetSource( pDoc );
//------------------------------------------------------------------------------
// Description:       display the source of proposal window
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc   pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     get pointers to structure
//                    if parameter is EQF_ACTIVATE then activate the windows
//                    else return error code.
//------------------------------------------------------------------------------
VOID
EQFR_GetSource
(
  PDOCUMENT_IDA  pDoc                              // pointer to document ida
)
{
  PSTEQFGEN     pstEQFGen;                      // pointer to generic struct
  PSTEQFPCMD    pstPCmd;                        // pointer to pipe cmd struct

  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct

  pstEQFGen->usRC    = NO_ERROR;

  switch ( pstPCmd->ulParm1 )
  {
    case EQF_ACTIVATE:
      WinPostMsg (pDoc->hwndSource, WM_EQF_PROCESSTASK,
                       MP1FROMSHORT(ACT_SOURCEPROP), NULL );
      break;

    default:
      pstEQFGen->usRC   =  EQFRS_ENTRY_NOT_AVAIL;
      break;
  } /* endswitch */

  return ;
} /* end of function EQFR_GetSource */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFRepaint
//------------------------------------------------------------------------------
// Function call:     EQFRepaint( pDoc );
//------------------------------------------------------------------------------
// Description:       force a repaint of all visible service windows
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc    -- pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     update all service windows if they are visible..
//------------------------------------------------------------------------------

VOID
EQFRepaint
(
  PDOCUMENT_IDA  pDoc                  // pointer to document ida
)
{

  /********************************************************************/
  /* update the translation memory proposal                           */
  /* might happen if user changes from 'automatic replace' to normal  */
  /* and vice-versa...                                                */
  /********************************************************************/
  PSTEQFSAB pstEQFSab;                       // ptr to SAB element

  pstEQFSab = pDoc->stEQFSab + pDoc->usFI; // point to current element
  UTF16memset (pstEQFSab->pucTargetSegs, '\0', EQF_TGTLEN);
  UTF16memset (pstEQFSab->pucPropsSegs, '\0', EQF_TGTLEN);
  UTF16memset (pstEQFSab->pucPropAddData, '\0', EQF_TGTLEN);

  // set configuration of what to be displayed in proposal window
  if ( pDoc->tbDevProposal.tbDoc.pUserSettings->UserOptFlags.bAllExactProposals )
  {
    pDoc->fsConfig |= EQFF_ALLEXACTONES;
  }
  else
  {
    pDoc->fsConfig &= ~EQFF_ALLEXACTONES;
  }

  EQFTM (pDoc, EQFCMD_TRANSSEGW, pstEQFSab);     // TRANSSEG to TM
  /********************************************************************/
  /* display proposal window                                          */
  /********************************************************************/
  if ( WinIsWindowVisible( pDoc->tbDevProposal.tbDoc.hwndFrame ) )
  {
    PrepMemTitle( pDoc, pDoc->tbDevProposal.tbDoc.pUserSettings->fDispMemIndicator, SID_PROP_TITLE );
    WinSetWindowText( pDoc->hwndProposals, pDoc->szBuf );
    InsertProposal ( &(pDoc->tbDevProposal), TRUE );
  } /* endif */

  /********************************************************************/
  /* display dictionary window and update title                       */
  /********************************************************************/
  if ( WinIsWindowVisible( pDoc->tbDevDictionary.tbDoc.hwndFrame ) )
  {
    BOOL fDispDictName = FALSE;
    fDispDictName = pDoc->tbDevDictionary.tbDoc.pUserSettings->fDispDictName;
    PrepDictTitle( pDoc, fDispDictName );

    WinSetWindowText( pDoc->hwndDictionary, pDoc->szBuf);
    InsertDictionary ( &(pDoc->tbDevDictionary) );
  } /* endif */

  /********************************************************************/
  /* display source of proposal window                                */
  /********************************************************************/
  if ( WinIsWindowVisible( pDoc->tbDevSource.tbDoc.hwndFrame ) )
  {
    InsertSource( &(pDoc->tbDevSource), FALSE , TRUE);
  } /* endif */

} /* end of function EQFRepaint */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFR_AddAbbrev
//------------------------------------------------------------------------------
// Function call:     EQFR_AddAbbrev( pDoc )
//------------------------------------------------------------------------------
// Description:       add the word to the abbreviation dictionary
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc    pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     if language not already activated, activate it.
//                    if okay then
//                      add word to addenda
//                    endif
//------------------------------------------------------------------------------

VOID  EQFR_AddAbbrev
(
   PDOCUMENT_IDA  pDoc                          // pointer to document inst ida
)
{
   PSTEQFPCMD    pstPCmd;              // request queue
   PSTEQFGEN     pstEQFGen;            // pointer to generic structure
   USHORT        usRC;                 // return of Morph services
   BOOL          fOK = TRUE;           // success indicator

   pstEQFGen = pDoc->pstEQFGen;
   pstPCmd = pstEQFGen->pstEQFPCmd;    // pointer to command struct

   pstEQFGen->usRC = 0;                // reset return code
   pstEQFGen->szMsgBuffer[0] = EOS;

   //  if okay add the word to the abbreviation dictionary
   if ( fOK )
   {
      usRC = MorphAddToAbbrev( pDoc->sSrcLanguage,
                              (PSZ_W) pstPCmd->ucbBuffer, pDoc->ulSrcOemCP );
      if ( usRC != MORPH_OK )
      {
         pstEQFGen->usRC = EQFRS_NOADDENDA_DICT;
      } /* endif */
   } /* endif */

  return;
} /* end of function EQFR_AddAbbrev */

/**********************************************************************/
/* open folder properties in read access                              */
/**********************************************************************/
HPROP OpenFolderProps( PSZ pFolderObjName )
{
  HPROP    hpropFolder;
  ULONG    ulErrorInfo;            //error indicator from PRHA
  PPROPSYSTEM   pSysProp;                 // ptr to EQF system properties
  CHAR   chOrgDrive;                      // original drive
  pSysProp = (PPROPSYSTEM)MakePropPtrFromHnd( EqfQuerySystemPropHnd());
  chOrgDrive = pFolderObjName[0];          // original drive
  pFolderObjName[0] = pSysProp->szPrimaryDrive[0];
  if( (hpropFolder = OpenProperties( pFolderObjName, NULL,
                                      PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
  {
     // display error message if not already displayed
     if ( ulErrorInfo != Err_NoStorage )
     {
        PSZ    pData;                           // pointer to data
        pData = pFolderObjName;
        UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pData, EQF_ERROR);
     } /* endif */
  }/*endif*/
  pFolderObjName[0] = chOrgDrive;                // reset orgdrive
  return hpropFolder;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFR_XDocAct
//------------------------------------------------------------------------------
// Function call:     EQFR_XDocAct( pDoc )
//------------------------------------------------------------------------------
// Description:       activate the translation memory and dictionary of the
//                    specified document
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc    pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------

VOID  EQFR_XDocAct
(
   PDOCUMENT_IDA  pDoc                          // pointer to document inst ida
)
{
  PSTEQFPCMD    pstPCmd;              // request queue
  PSTEQFGEN     pstEQFGen;            // pointer to generic structure

  pstEQFGen = pDoc->pstEQFGen;
  if ( pstEQFGen )
  {
    pstPCmd = pstEQFGen->pstEQFPCmd;    // pointer to command struct

    pstEQFGen->usRC = 0;                // reset return code
    pstEQFGen->szMsgBuffer[0] = EOS;

    //  if okay get object name of document
    if ( pstPCmd )
    {
      /****************************************************************/
      /* check if we are dealing with a new document or if someone    */
      /* wants to play a joke....                                     */
      /****************************************************************/
      if ( strcmp( pDoc->IdaHead.szObjName, (PSZ)(pstPCmd->ucbBuffer) ) == 0 )
      {
        /**************************************************************/
        /* ready -- nothing to change                                 */
        /**************************************************************/
      }
      else
      {
        BOOL fOK = TRUE;
        PADDDOC_IDA pAddDoc = NULL;
        PSZ  pszNewDoc = (PSZ)(pstPCmd->ucbBuffer);
        PSZ  pTemp;
        CHAR c;
        /**************************************************************/
        /* allocate ida..                                             */
        /**************************************************************/
        fOK = UtlAlloc( (PVOID *)&pAddDoc,
                        0L, (LONG) sizeof( ADDDOC_IDA ), ERROR_STORAGE );

        /*******************************************************************/
        /* Get memory, tag table and languages for this document           */
        /*******************************************************************/
        if ( fOK )
        {
          if ( DocQueryInfo2( pszNewDoc,                // document object name
                              pAddDoc->szDocMemory,     // document translation memory
                              pAddDoc->szDocFormat,     // format of document
                              pAddDoc->szDocSourceLang, // document source language
                              pAddDoc->szDocTargetLang, // document target language
                              pAddDoc->szDocLongName,   // long name of document
                              pAddDoc->szAlias,         // document alias name
                              pAddDoc->szEditor,        // document editor
                              TRUE ) != NO_ERROR )      // handle errors in function
          {
            fOK = FALSE;
          } /* endif */
        }
        if ( fOK )
        {
          PPROPFOLDER  ppropFolder;                        //pointer to folder properties
          ULONG        ulErrorInfo;
          BOOL         fNewFolder = FALSE;
          BOOL         fNewDicts  = FALSE;
          BOOL         fNewTMs    = FALSE;

          /************************************************************/
          /* get folder name ...                                      */
          /************************************************************/
          pTemp = UtlGetFnameFromPath( pszNewDoc );
          pTemp --;
          c = *pTemp;
          *pTemp = EOS;
          strcpy(pAddDoc->szFolderObjName, pszNewDoc );
          *pTemp = c;
          /**************************************************************/
          /* check if we deal with new folder...                        */
          /**************************************************************/
          if ( strcmp( pAddDoc->szFolderObjName, pDoc->szFolderObjName ) )
          {
            /************************************************************/
            /* open folder properties and get folder TM and dicts       */
            /************************************************************/
            pAddDoc->hpropFolder =  OpenFolderProps( pAddDoc->szFolderObjName );
            fNewFolder = TRUE;
            if( pAddDoc->hpropFolder )
            {
              ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( pAddDoc->hpropFolder );
              if ( ppropFolder->aLongDicTbl[0][0] != EOS )
              {
                memcpy( pAddDoc->aDicts, ppropFolder->aLongDicTbl, sizeof(pAddDoc->aDicts));
                memcpy( pAddDoc->aMemDBs, ppropFolder->aLongMemTbl, sizeof(pAddDoc->aMemDBs));
              }
              else
              {
                // copy names from old style dictionary table
                PSZ pszPos = ppropFolder->DicTbl;
                int i = 0;
                while ( (*pszPos != X15) && (*pszPos != EOS) )
                {
                  Utlstrccpy( pAddDoc->aDicts[i], pszPos, X15 );
                  pszPos += strlen(pAddDoc->aDicts[i]) + 1;
                  i++;
                } /* endwhile */

                // copy names from old style mem table
                pszPos = ppropFolder->MemTbl;
                i = 0;
                while ( (*pszPos != X15) && (*pszPos != EOS) )
                {
                  Utlstrccpy( pAddDoc->aMemDBs[i], pszPos, X15 );
                  pszPos += strlen(pAddDoc->aMemDBs[i]) + 1;
                  i++;
                } /* endwhile */
              } /* endif */
              strcpy( pAddDoc->szGlobalMemOptFile, ppropFolder->szGlobalMemOptFile );
              CloseProperties( pAddDoc->hpropFolder, PROP_QUIT, &ulErrorInfo);
            }/*endif*/
          }
          else
          {
            //get access(pointer) to folder properties
            ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( pDoc->hpropFolder );

            if ( ppropFolder->aLongDicTbl[0][0] != EOS )
            {
              memcpy( pAddDoc->aDicts, ppropFolder->aLongDicTbl, sizeof(pAddDoc->aDicts));
              memcpy( pAddDoc->aMemDBs, ppropFolder->aLongMemTbl, sizeof(pAddDoc->aMemDBs));
            }
            else
            {
              // copy names from old style dictionary table
              PSZ pszPos = ppropFolder->DicTbl;
              int i = 0;
              while ( (*pszPos != X15) && (*pszPos != EOS) )
              {
                Utlstrccpy( pAddDoc->aDicts[i], pszPos, X15 );
                pszPos += strlen(pAddDoc->aDicts[i]) + 1;
                i++;
              } /* endwhile */

              // copy names from old style mem table
              pszPos = ppropFolder->MemTbl;
              i = 0;
              while ( (*pszPos != X15) && (*pszPos != EOS) )
              {
                Utlstrccpy( pAddDoc->aMemDBs[i], pszPos, X15 );
                pszPos += strlen(pAddDoc->aMemDBs[i]) + 1;
                i++;
              } /* endwhile */
            } /* endif */
            strcpy( pAddDoc->szGlobalMemOptFile, ppropFolder->szGlobalMemOptFile );
          } /* endif */
          /************************************************************/
          /* call Doc initialize function ...                         */
          /************************************************************/
          if ( fOK )
          {
            ppropFolder = (PPROPFOLDER)MakePropPtrFromHnd( pDoc->hpropFolder );
            /**********************************************************/
            /* adjust dictionary                                      */
            /**********************************************************/
            if ( fOK )
            {
              fNewDicts = memcmp( &pAddDoc->aDicts, &ppropFolder->aLongDicTbl,
                                  sizeof( pAddDoc->aDicts ) );
                          // if no new dictionary is set ...
                          fNewDicts |= !pAddDoc->aDicts[0][0];
            } /* endif */
            /**********************************************************/
            /* adjust TM                                              */
            /**********************************************************/
            if ( fOK )
            {
              fNewTMs = memcmp( &pAddDoc->aMemDBs, &ppropFolder->aLongMemTbl,
                                sizeof( pAddDoc->aMemDBs ) );
                          // if no new memory is set ...
                          fNewTMs |= !pAddDoc->aMemDBs[0][0];
              fNewTMs |= strcmp( pAddDoc->szDocMemory, pDoc->szDocMemory );
              strcpy( pDoc->szDocMemory, pAddDoc->szDocMemory );
            } /* endif */

            /**********************************************************/
            /* adjust document specific settings                      */
            /**********************************************************/
            if ( fOK )
            {
              PSZ   apszTlm[MAX_NUM_OF_READONLY_MDB+1]; //translation memory
              PSZ   apszLDct[NUM_OF_FOLDER_DICS+1]; //folder dictionaries
                                                    //add 1 for NULL termination

              // handle any global memory filter file
              if ( strcmp( pAddDoc->szGlobalMemOptFile, pDoc->szGlobalMemOptFile ) != 0 )
              {
                if ( pDoc->pvGlobalMemOptFile ) GlobMemFreeOptionFile( &(pDoc->pvGlobalMemOptFile) );
                if ( pAddDoc->szGlobalMemOptFile[0] != EOS )
                {
                  CHAR szGlobMemFilter[MAX_EQF_PATH];
                  UtlMakeEQFPath( szGlobMemFilter, pAddDoc->szFolderObjName[0], SYSTEM_PATH, UtlGetFnameFromPath( pAddDoc->szFolderObjName ));
                  strcat( szGlobMemFilter, BACKSLASH_STR  );
                  strcat( szGlobMemFilter, pAddDoc->szGlobalMemOptFile );
                  GlobMemLoadOptionFile( szGlobMemFilter, &(pDoc->pvGlobalMemOptFile) );
                } /* endif */                   
              } /* endif */                 

              /**********************************************************/
              /* adjust document specific settings                      */
              /**********************************************************/
              if ( strcmp( pAddDoc->szDocFormat, pDoc->szDocFormat )  )
              {
                /******************************************************/
                /* load correct tagtables for service windows         */
                /******************************************************/
                strcpy( (PSZ)(pstEQFGen->szTagTable), pAddDoc->szDocFormat );
                fOK = UpdateLoadedTagTables( pDoc, &(pDoc->tbDevProposal) );
                if ( fOK )
                {
                  fOK = UpdateLoadedTagTables( pDoc, &(pDoc->tbDevSource) );
                } /* endif */
              } /* endif */
              if ( fOK )
              {
                strcpy( pDoc->szDocFormat, pAddDoc->szDocFormat );
                strcpy( pDoc->szDocName, UtlGetFnameFromPath( pszNewDoc ));
                strcpy( pDoc->szDocLongName, pAddDoc->szDocLongName );
                strcpy( pDoc->IdaHead.szObjName, (PSZ)(pstPCmd->ucbBuffer) );
                strcpy( pDoc->szFolderObjName, pAddDoc->szFolderObjName );
                {
                  strcpy( pAddDoc->szDocMemory, pDoc->szMemory[0] );
                }

                if( !fOK )
                {
                  PSZ apszReplace[2];
                  apszReplace[0] = pDoc->szDocMemory;
                  Utlstrccpy( pDoc->szBuf, pDoc->szFolderName, '.' );
                  apszReplace[1] = pDoc->szBuf;
                  UtlError( ERROR_TM_NOT_EXIST, MB_CANCEL, 2,
                            apszReplace, EQF_ERROR );
                  pDoc->pstEQFGen->usRC = ERROR_TM_NOT_EXIST;
                }
                else
                {
                  /******************************************************/
                  /* set correct document properties, drop old ones     */
                  /******************************************************/
                  CloseProperties( pDoc->IdaHead.hProp, PROP_QUIT, &ulErrorInfo);
                  if( (pDoc->IdaHead.hProp = OpenProperties( pszNewDoc, NULL,
                                                   PROP_ACCESS_READ, &ulErrorInfo)) == NULL)
                  {
                     fOK = FALSE;
                     // display error message if not already displayed
                     if ( ulErrorInfo != Err_NoStorage )
                     {
                        UtlError( ERROR_OPEN_PROPERTIES, MB_CANCEL, 1, &pszNewDoc, EQF_ERROR);
                        pDoc->pstEQFGen->usRC = ERROR_OPEN_PROPERTIES;
                     } /* endif */
                  }
                  else
                  {
                    pDoc->ppropDoc = (PPROPDOCUMENT)MakePropPtrFromHnd( pDoc->IdaHead.hProp );
                  }/*endif*/
                  /******************************************************/
                  /* check for folder                                   */
                  /******************************************************/
                  if ( fOK && fNewFolder )
                  {
                    CloseProperties( pDoc->hpropFolder, PROP_QUIT, &ulErrorInfo);
                    pDoc->hpropFolder =  OpenFolderProps( pAddDoc->szFolderObjName );
                    fOK = (pDoc->hpropFolder != NULLHANDLE);
                  } /* endif */
                  if ( fOK )
                  {
                    if ( fNewDicts )
                    {
                      ReleaseDictAccess( pDoc->szDicts[0] );
                    } /* endif */
                    fOK = BuildGenericStructure( pDoc, pDoc->pstEQFGen, apszTlm, apszLDct, FALSE );
                    if ( fOK )
                    {
                      // fill szSegSource and szSegTarget path, check
                      // analysis and source language availability
                      fOK = CheckResources( pDoc );
                      if ( !fOK )
                      {
                        pstEQFGen->usRC = ERROR_NOT_ANALYZED;
                      } /* endif */
                    } /* endif */
                  } /* endif */
                } /* endif */
              } /* endif */
            } /* endif */

            /**********************************************************/
            /* generic initialisation for new file ...                */
            /**********************************************************/
            if ( fOK )
            {
              EQFTMDAInit( pDoc );
              if ( !*pDoc->szDicts[0] )
              {
                pDoc->pstEQFGen->fsConfiguration &= ~EQFF_DA_CONF;
              } /* endif */
            } /* endif */
            /**********************************************************/
            /* handling for new dictionary                            */
            /**********************************************************/
            if ( fOK && fNewDicts )
            {
              PTBDOCUMENT pTBDoc = &(pDoc->tbDevDictionary.tbDoc);
              if ( !*pDoc->szDicts[0] )
              {
                if ( pTBDoc->hwndFrame )
                {
                  WinShowWindow( pTBDoc->hwndFrame, FALSE );
                } /* endif */
              }
              else
              {
                if ( pTBDoc->hwndFrame )
                {
                  WinShowWindow( pTBDoc->hwndFrame, TRUE );
                } /* endif */
              } /* endif */

              /***********************************************************/
              /* close the dictionary by hand -- since we have no Thread */
              /* doing this for us under Windows...                      */
              /***********************************************************/
              pDoc->fNewDictAccess = TRUE;
              if ( pDoc->hDCB )
              {
                AsdClose( pDoc->hUCB, pDoc->hDCB);  // close dictionaries
                pDoc->hDCB = NULL;
              } /* endif */
              ActivateASD( pDoc );
              pDoc->fNewDictAccess = FALSE;
              fOK = !pstEQFGen->usRC;
              if ( fOK )
              {
                BOOL fDispDictName = FALSE;
                fDispDictName = pTBDoc->pUserSettings->fDispDictName;
                PrepDictTitle( pDoc, fDispDictName );
                WinSetWindowText( pDoc->hwndDictionary, pDoc->szBuf);
                fOK = SetDictAccess( pDoc->szDicts[0] );
              } /* endif */
            } /* endif */

            if ( fOK && fNewTMs )
            {
              /******************************************************/
              /* close memories and open new ones....               */
              /******************************************************/
              EQFTM (pDoc, EQFCMD_INIT, NULL );
              fOK = !pstEQFGen->usRC;
              /*************************************************************/
              /* get filename of active Translation memory                 */
              /*************************************************************/
              if ( fOK )
              {
                HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
                USHORT usI;
                PSZ pData;
                pData = pDoc->szBuf + 2* MAX_PATH144;
                Utlstrccpy(pData, UtlGetFnameFromPath(pDoc->szMemory[0]),  DOT);

                if ( pDoc->szMemory[1][0] )
                {
                  strcat( pData, "  (" );
                  usI = 1;
                  while ( pDoc->szMemory[usI][0] )
                  {
                    Utlstrccpy(pDoc->szBuf,UtlGetFnameFromPath(pDoc->szMemory[usI]),DOT);
                    strcat (pData, pDoc->szBuf);
                    if ( pDoc->szMemory[usI+1][0] )
                    {
                      strcat(pData,"  ");
                    }
                    else
                    {
                      strcat(pData,")");
                    } /* endif */
                    usI ++;
                  } /* endwhile */
                } /* endif */
                if ( pData )
                {

                  ULONG  Length;
                  WinLoadString ((HAB) UtlQueryULong( QL_HAB ), hResMod, SID_PROP_TITLE,
                                 EQF_MSGBUF_SIZE, (pDoc->szBuf+MAX_PATH144));
                  DosInsMessage( &pData, 1, (pDoc->szBuf+MAX_PATH144), MAX_PATH144,
                                 pDoc->szBuf, MAX_PATH144, &Length );
                }
                else
                {
                  WinLoadString ((HAB) UtlQueryULong( QL_HAB ), hResMod, SID_PROP_TITLE,
                                 EQF_MSGBUF_SIZE, pDoc->szBuf);
                } /* endif */
                WinSetWindowText( (pDoc->hwndProposals), pDoc->szBuf );
              } /* endif */
            } /* endif */
            if ( !pstEQFGen->usRC && (pstEQFGen->fsConfiguration & EQFF_MT_CONF) )
            {
               EQFMTInit( pDoc );
            } /* endif */
          } /* endif */
          /************************************************************/
          /* set title of changed translation environment             */
          /************************************************************/
          if ( fOK )
          {
            SetDocWindowText( pDoc, pDoc->hwnd, pszNewDoc );
            EqfChangeObjectName( pDoc->hwnd, pszNewDoc );
          } /* endif */
        } /* endif */
        /**************************************************************/
        /* free resource                                              */
        /**************************************************************/
        UtlAlloc( (PVOID *)&pAddDoc, 0L, 0L, NOMSG );
      } /* endif */
    }
    else
    {
      pstEQFGen->usRC = EQFRS_INVALID_CMD;
    } /* endif */
  } /* endif */
  return;
} /* end of function EQFR_XDocAct */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFR_XDocAdd
//------------------------------------------------------------------------------
// Function call:     EQFR_XDocAdd( pDoc )
//------------------------------------------------------------------------------
// Description:       add the specified document to the list of docuemnts
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc    pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------

VOID  EQFR_XDocAdd
(
   PDOCUMENT_IDA  pDoc                          // pointer to document inst ida
)
{
  PSTEQFPCMD    pstPCmd;              // request queue
  PSTEQFGEN     pstEQFGen;            // pointer to generic structure

  pstEQFGen = pDoc->pstEQFGen;
  if ( pstEQFGen )
  {
    pstPCmd = pstEQFGen->pstEQFPCmd;    // pointer to command struct

    pstEQFGen->usRC = 0;                // reset return code
    pstEQFGen->szMsgBuffer[0] = EOS;

    //  if okay get object name of document
    if ( pstPCmd )
    {
      if ( ! AddDocToList( pDoc, (PSZ)(pstPCmd->ucbBuffer) , BOTTOM_OF_LIST ) )
      {
        pstEQFGen->usRC = ERROR_STORAGE;
      } /* endif */
    }
    else
    {
      pstEQFGen->usRC = EQFRS_INVALID_CMD;
    } /* endif */
  } /* endif */
  return;
} /* end of function EQFR_XDocAdd */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFR_XDocRemove
//------------------------------------------------------------------------------
// Function call:     EQFR_XDocRemove( pDoc )
//------------------------------------------------------------------------------
// Description:       remove the specified document from list of docuemnts
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc    pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------

VOID  EQFR_XDocRemove
(
   PDOCUMENT_IDA  pDoc                          // pointer to document inst ida
)
{
  PSTEQFPCMD    pstPCmd;              // request queue
  PSTEQFGEN     pstEQFGen;            // pointer to generic structure

  pstEQFGen = pDoc->pstEQFGen;
  if ( pstEQFGen )
  {
    pstPCmd = pstEQFGen->pstEQFPCmd;    // pointer to command struct

    pstEQFGen->usRC = 0;                // reset return code
    pstEQFGen->szMsgBuffer[0] = EOS;

    //  if okay get object name of document
    if ( pstPCmd )
    {
      RemoveDocFromList( pDoc, (PSZ)(pstPCmd->ucbBuffer) );
    }
    else
    {
      pstEQFGen->usRC = EQFRS_INVALID_CMD;
    } /* endif */
  } /* endif */
  return;
} /* end of function EQFR_XDocRemove */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFR_XDocNext
//------------------------------------------------------------------------------
// Function call:     EQFR_XDocNext( pDoc )
//------------------------------------------------------------------------------
// Description:       move to the next document in the list
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc    pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------

VOID  EQFR_XDocNext
(
   PDOCUMENT_IDA  pDoc                          // pointer to document inst ida
)
{
  PSTEQFPCMD    pstPCmd;              // request queue
  PSTEQFGEN     pstEQFGen;            // pointer to generic structure

  pstEQFGen = pDoc->pstEQFGen;
  if ( pstEQFGen )
  {
    pstPCmd = pstEQFGen->pstEQFPCmd;    // pointer to command struct

    pstEQFGen->usRC = 0;                // reset return code
    pstEQFGen->szMsgBuffer[0] = EOS;

    //  if okay get next (or first doccument from list )
    if ( pstPCmd )
    {
      if ( FindNextDocInList( pDoc, (PSZ)(pstPCmd->ucbBuffer), (PSZ)(pstPCmd->ucbBuffer) ) )
      {
        pstPCmd->usLen1 = (USHORT)(strlen( (PSZ)(pstPCmd->ucbBuffer) ) + 1);
      }
      else
      {
        pstPCmd->usLen1 = 0;
        pstPCmd->ucbBuffer[0] = EOS;
        pstEQFGen->usRC = EQFRS_INVALID_CMD;
      } /* endif */
    }
    else
    {
      pstEQFGen->usRC = EQFRS_INVALID_CMD;
    } /* endif */
  } /* endif */
  return;
} /* end of function EQFR_XDocNext */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFR_XDocNum
//------------------------------------------------------------------------------
// Function call:     EQFR_XDocNum ( pDoc )
//------------------------------------------------------------------------------
// Description:       move to the next document in the list
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc    pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------

VOID  EQFR_XDocNum
(
   PDOCUMENT_IDA  pDoc                          // pointer to document inst ida
)
{
  PSTEQFPCMD    pstPCmd;              // request queue
  PSTEQFGEN     pstEQFGen;            // pointer to generic structure

  pstEQFGen = pDoc->pstEQFGen;
  if ( pstEQFGen )
  {
    pstPCmd = pstEQFGen->pstEQFPCmd;    // pointer to command struct

    pstEQFGen->usRC = 0;                // reset return code
    pstEQFGen->szMsgBuffer[0] = EOS;

    //  if okay get next (or first doccument from list )
    if ( pstPCmd )
    {
      FindDocInList( pDoc, pstPCmd->usLen1, (PSZ)(pstPCmd->ucbBuffer) );
      if ( pstPCmd->ucbBuffer[0] )
      {
        pstPCmd->usLen1 = (USHORT)(strlen( (PSZ)(pstPCmd->ucbBuffer) ) + 1);
      } /* endif */
    }
    else
    {
      pstEQFGen->usRC = EQFRS_INVALID_CMD;
    } /* endif */
  } /* endif */
  return;
} /* end of function EQFR_XDocNum */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     EQFR_XDocInList
//------------------------------------------------------------------------------
// Function call:     EQFR_XDocInList (pDoc)
//------------------------------------------------------------------------------
// Description:       find if the specified document is part of the list
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  pDoc    pointer to document ida
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:
//------------------------------------------------------------------------------

VOID  EQFR_XDocInList
(
   PDOCUMENT_IDA  pDoc                          // pointer to document inst ida
)
{
  PSTEQFPCMD    pstPCmd;              // request queue
  PSTEQFGEN     pstEQFGen;            // pointer to generic structure

  pstEQFGen = pDoc->pstEQFGen;
  if ( pstEQFGen )
  {
    pstPCmd = pstEQFGen->pstEQFPCmd;    // pointer to command struct

    pstEQFGen->usRC = 0;                // reset return code
    pstEQFGen->szMsgBuffer[0] = EOS;

    //  if okay find doc in list
    if ( pstPCmd )
    {
      BOOL fOK = QueryDocInList( pDoc, (PSZ)(pstPCmd->ucbBuffer), &pstPCmd->usLen1 );
      if (!fOK)
      {
        pstEQFGen->usRC = FILE_NOT_EXISTS;
      }
    }
    else
    {
      pstEQFGen->usRC = EQFRS_INVALID_CMD;
    } /* endif */
  } /* endif */
  return;
} /* end of function EQFR_XDocInList */

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     DeleteProposalAllowed
//------------------------------------------------------------------------------
// Function call:     fOK = DeleteProposalAllowed( pDoc );
//------------------------------------------------------------------------------
// Description:       Check if cursor positioned in valid area
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA  document ida
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       n       number of proposal
//                    0       no delete proposal allowed
//------------------------------------------------------------------------------
// Function flow:     loop thru our sentence buffer and check if cursor
//                    segment contains a valid proposal
//------------------------------------------------------------------------------
static USHORT
DeleteProposalAllowed
(
  PDOCUMENT_IDA pDoc,
  PTBDOCUMENT pTBDoc
)
{
  USHORT usProp = 0;
  USHORT usPropInEQFSab = 0;
  /********************************************************************/
  /* get the segment we are dealing with                              */
  /********************************************************************/
  while ( usProp <= (pDoc->stEQFSab + pDoc->usFI)->usPropCount )
  {
    if ( (pDoc->stTWBS.StartStopProp[ usProp ].usStartSeg <= pTBDoc->TBCursor.ulSegNum) &&
         (pTBDoc->TBCursor.ulSegNum <= pDoc->stTWBS.StartStopProp[ usProp ].usLastSeg ) )
    {
      break;
    }
    else
    {
      usProp++;
    } /* endif */
  } /* endwhile */

  usPropInEQFSab = pDoc->stTWBS.usIndexInEqfSAB[usProp];

  if ( (usPropInEQFSab > (pDoc->stEQFSab + pDoc->usFI)->usPropCount) || (usPropInEQFSab==0) )
  {
    /****************************************************************/
    /* nothing found - touched in not supported area...             */
    /****************************************************************/
    usPropInEQFSab = 0;
  } /* endif */

  return usPropInEQFSab;
} /* end of function DeleteProposalAllowed */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     DeleteProposal
//------------------------------------------------------------------------------
// Function call:     DeleteProposal( pTBDoc );
//------------------------------------------------------------------------------
// Description:       This function enables the user to delete a proposal.
//                    A confirmation message is displayed prior to deletion
//                    to warn the user about what he is doing
//------------------------------------------------------------------------------
// Parameters:        PTBDOCUMENT  pDoc;     // pointer to document structure
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     get the segment we are dealing with;
//                    ignore request if user is outside the valid area or if
//                    user wants to delete the 0-proposal
//                    if okay
//                      get source for the selected proposal and display msg
//                      with source and target (portions)...
//                      if user confirms issue a delete request to the TM-
//                    return
//------------------------------------------------------------------------------
static VOID
DeleteProposal
(
  PTWBSDEVICE  pDevice
)
{
  BOOL fOK = TRUE;                     // success indicator
  USHORT   usProp = 0;                 // number of proposal
  PSZ_W    pSrcProp = NULL;            // pointer to source proposal
  PDOCUMENT_IDA  pDoc = (PDOCUMENT_IDA)pDevice->pDoc;
  PTBDOCUMENT    pTBDoc = &(pDevice->tbDoc);

  /********************************************************************/
  /* allocate buffers                                                 */
  /********************************************************************/
  fOK = UtlAlloc( (PVOID *)&pSrcProp, 0L, (LONG) 2 * MAX_SEGMENT_SIZE * sizeof(CHAR_W),
                   ERROR_STORAGE );
  /********************************************************************/
  /* get the segment we are dealing with                              */
  /********************************************************************/
  if ( fOK )
  { // returns index of proposal in stEQFSab, which may be different from
    // index in proposal window due to invisible proposals!
    usProp = DeleteProposalAllowed( pDoc, pTBDoc );
    if ( !usProp )
    {
      /****************************************************************/
      /* nothing found - touched in not supported area...             */
      /****************************************************************/
      fOK = FALSE;
      WinAlarm( HWND_DESKTOP, WA_WARNING );
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* extract the source for it...                                     */
  /********************************************************************/
  if ( fOK )
  {
    stEQFExt.pucSourceSeg = pSrcProp;
    if ( !(MachineTrans( pDoc, usProp )  & MACHINE_TRANS_PENDING) )
    {
      /****************************************************************/
      /* only the first memory is R/W, all others are R/O             */
      /* Have in mind, that usProp indicates n-th proposal where      */
      /* usDBIndex starts at 0.                                       */
      /****************************************************************/
      if ( (pDoc->stEQFSab+pDoc->usFI)->usDBIndex[usProp-1] != 0 )
      {
        UtlError(ERROR_PROP_FROM_RO_MEMORY, MB_CANCEL,
                 0, NULL, EQF_ERROR);

        fOK = FALSE;
      }
      //else
      //{
      //  stEQFExt.ulParm1 = usProp;                 // number segment
      //  memcpy( &(stEQFExt.ulKey[0]), &((pDoc->stEQFSab+pDoc->usFI)->ulKey[0]),
      //          sizeof(stEQFExt.ulKey) );
      //  memcpy( &(stEQFExt.usTargetNum[0]),
      //          &((pDoc->stEQFSab+pDoc->usFI)->usTargetNum[0]),
      //          sizeof(stEQFExt.usTargetNum) );
      //  EQFTM( pDoc, EQFCMD_EXTSEGW, &stEQFExt );   // extract segment from T M
      //  fOK = (stEQFExt.usRC == 0);
      //} /* endif */
    }
    else
    {
      fOK = FALSE;
    } /* endif */

      /****************************************************************/
      /* display message                                              */
      /****************************************************************/
    if ( fOK )
    {
      USHORT usResp;                   // response from message box
      ULONG  ulLen;

      CHAR_W chSrc[ MSGBOXDATALEN + 1];
      CHAR_W chTgt[ MSGBOXDATALEN + 1];
      PSZ_W  pError[2];

      UTF16strncpy( chTgt, GetPropSZ (pDoc, usProp, &ulLen), MSGBOXDATALEN);
      chSrc[ MSGBOXDATALEN ] = EOS;   // set end of string
      if ( UTF16strlenCHAR(chTgt) >= MSGBOXDATALEN )
      {                                // fill last three chars with ï...ï
         UTF16strcpy(chTgt+MSGBOXDATALEN-4,L"...");
      } /* endif */

      UTF16strncpy( chSrc, GetSrcPropSZ(pDoc, usProp ), MSGBOXDATALEN);
      chSrc[ MSGBOXDATALEN ] = EOS;   // set end of string
      if ( UTF16strlenCHAR(chSrc) >= MSGBOXDATALEN )
      {                                // fill last three chars with ï...ï
         UTF16strcpy(chSrc+MSGBOXDATALEN-4,L"...");
      } /* endif */

      pError[0] = chSrc;
      pError[1] = chTgt;
      usResp = UtlErrorW(TMPROP_DEL_MSG, MB_YESNO | MB_DEFBUTTON2, 2, &pError[0], EQF_QUERY, TRUE );

      if ( usResp == MBID_YES )
      {
        PSTEQFSAB   pstEQFSab = pDoc->stEQFSab + pDoc->usFI;
        /**************************************************************/
        /* delete the proposal from the T M ...                       */
        /**************************************************************/
        stEQFExt.ulParm1 = usProp;                 // number segment
        EQFTM( pDoc, EQFCMD_DELPROP, &stEQFExt );
        /**************************************************************/
        /* retrieve proposals and display them again                  */
        /**************************************************************/
        UTF16memset(pstEQFSab->pucTargetSegs, '\0', EQF_TGTLEN);
        memset(&pstEQFSab->usMachineTrans[0], 0, sizeof(pstEQFSab->usMachineTrans));
        memset(&pstEQFSab->usFuzzyPercents[0], 0, sizeof(pstEQFSab->usFuzzyPercents));
        memset( &pstEQFSab->pszSortTargetSeg[0], 0, sizeof(pstEQFSab->pszSortTargetSeg) );
        memset( &pstEQFSab->pszSortPropsSeg[0], 0, sizeof(pstEQFSab->pszSortPropsSeg) );
        memset( &pstEQFSab->fInvisible[0], 0, sizeof(pstEQFSab->fInvisible) );
        memset(&pstEQFSab->pszSortPropsData[0], '\0', sizeof(pstEQFSab->pszSortPropsData) );

        EQFTM( pDoc, EQFCMD_TRANSSEGW, pstEQFSab );
        InsertProposal ( pDevice, TRUE );
        /**************************************************************/
        /* update Source of proposal window if visible                */
        /**************************************************************/
        if ( WinIsWindowVisible( pDoc->tbDevSource.tbDoc.hwndFrame ) )
        {
          InsertSource( &(pDoc->tbDevSource), FALSE , TRUE);
        } /* endif */

      } /* endif */
    } /* endif */
  } /* endif */

  /********************************************************************/
  /* free allocated resources                                         */
  /********************************************************************/
  UtlAlloc( (PVOID *)&pSrcProp, 0L, 0L, NOMSG );
  return;
} /* end of function DeleteProposal */


/**********************************************************************/
/* helper function for preparing segments...                         */
/**********************************************************************/
static VOID PrepAddSeg
(
  PTBSEGMENT ptbFromSegment,
  PTBSEGMENT ptbToSegment
)
{
  memcpy( ptbToSegment, ptbFromSegment, sizeof( TBSEGMENT) );
  {
    PSZ_W pData;
    /********************************************************************/
    /* allocate segment data and leave some extra space for automatic   */
    /* linewrap                                                         */
    /********************************************************************/
    UtlAlloc( (PVOID *)&pData, 0L, (LONG)max((ptbFromSegment->usLength+ 1)*sizeof(CHAR_W), MIN_ALLOC),
              NOMSG );
    if ( pData )
    {
      UTF16strncpy( pData, ptbFromSegment->pDataW, (USHORT)(ptbFromSegment->usLength+1) );
      ptbToSegment->pDataW = pData;
    } /* endif */
  }
}

/**********************************************************************/
/* update the loaded tagtables for the service windows if other       */
/* document is loaded                                                 */
/**********************************************************************/
static
BOOL UpdateLoadedTagTables
(
  PDOCUMENT_IDA pDoc,
  PTWBSDEVICE   pDevice
)
{
  PTBDOCUMENT pTBDoc;
  BOOL        fOK = TRUE;
  PSTEQFGEN     pstEQFGen;            // pointer to generic structure

  pstEQFGen = pDoc->pstEQFGen;
  pTBDoc    = &(pDevice->tbDoc);      // pointer to document structure

  if ( pTBDoc->pDocTagTable )
  {
    if ( pTBDoc->hModule )
    {
      TAEndProtectTable( &pTBDoc->hModule,
                         (PFN*)&pTBDoc->pfnUserExit,
                         (PFN*)&pTBDoc->pfnCheckSegExit,
                         (PFN*)&pTBDoc->pfnShowTrans,
                         (PFN*)&pTBDoc->pfnTocGoto,
                         (PFN*)&pTBDoc->pfnUserExitW,
                         (PFN*)&pTBDoc->pfnCheckSegExitW);
    } /* endif */
    TAFreeTagTable( (PLOADEDTABLE)pTBDoc->pDocTagTable );

    fOK  = !TALoadTagTable( (PSZ)(pstEQFGen->szTagTable),
                            (PLOADEDTABLE *) &pTBDoc->pDocTagTable,
                             FALSE, FALSE );

    if ( !fOK )                // file could not be accessed
    {
       PSZ pData = (PSZ)(pstEQFGen->szTagTable);
       UtlError(ERROR_FILE_ACCESS_ERROR, MB_CANCEL,
                1, &pData, EQF_ERROR);
    }
    else
    {
       /***************************************************************/
       /* check if special user exit handling is necessary,           */
       /***************************************************************/
       TALoadEditUserExit( pTBDoc->pDocTagTable, 
                           (PSZ)(pstEQFGen->szTagTable),
                           &pTBDoc->hModule,
                           (PFN*)&pTBDoc->pfnUserExit,
                           (PFN*)&pTBDoc->pfnCheckSegExit,
                           (PFN*)&pTBDoc->pfnShowTrans,
                           (PFN*)&pTBDoc->pfnTocGoto,
                           (PFN*)&pTBDoc->pfnGetSegContext,
                           NULL, 
                           (PFN*)&pTBDoc->pfnFormatContext, 
                           NULL,
                           &pTBDoc->pfnUserExitW,
                           (PFN*)&pTBDoc->pfnCheckSegExitW,
                           (PFN*)&pTBDoc->pfnCheckSegExExitW,
                           (PFN*)&pTBDoc->pfnCheckSegType );
       } /* endif */
  } /* endif */
  return fOK;
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     PrepDictTitle
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       _
//------------------------------------------------------------------------------
// Parameters:        _
//------------------------------------------------------------------------------
// Returncode type:   _
//------------------------------------------------------------------------------
// Returncodes:       _
//------------------------------------------------------------------------------
// Prerequesits:      _
//------------------------------------------------------------------------------
// Side effects:      _
//------------------------------------------------------------------------------
// Samples:           _
//------------------------------------------------------------------------------
// Function flow:     _
//------------------------------------------------------------------------------
static
VOID PrepDictTitle( PDOCUMENT_IDA pDoc, BOOL fDispDictName )
{
  USHORT usI = 0;
  static CHAR szLongName[MAX_LONGFILESPEC];
  CHAR szShortName[MAX_FNAME];
  PSZ  pData;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);


  WinLoadString ((HAB) UtlQueryULong( QL_HAB ), hResMod, SID_DCT_TITLE, EQF_MSGBUF_SIZE, pDoc->szBuf );

  // find parameter position
  pData = strchr( pDoc->szBuf, '%' );

  if ( pData )
  {
    CHAR   chNum[10];               // add dictname indicator

    *pData = EOS;
    pData = pDoc->szBuf;

    while ( *(pDoc->szDicts[usI]))
    {
      usI ++;
      if ( fDispDictName )
      {
        strcat(pData, "[");         //pDoc->chStartFlag check!!
        if (usI > 9)
        {
          chNum[0] = (UCHAR)('A'+ usI - 10);
        }
        else
        {
          itoa(usI, chNum, 10);
        }
        strcat(pData, chNum);
        strcat(pData, "]");         //pDoc->chEndFlag
      } /* endif */
      Utlstrccpy( szShortName, UtlGetFnameFromPath(pDoc->szDicts[usI-1]), DOT );
      ObjShortToLongName( szShortName, szLongName, DICT_OBJECT );
      if ( (strlen(pDoc->szBuf) + strlen(szLongName) + 2) < sizeof(pDoc->szBuf) )
      {
        OEMTOANSI( szLongName );
        strcat( pData, szLongName );
        strcat( pData,"  " );
      }
      else
      {
        strcat( pData, "  ..." );
      } /* endif */
    } /* endwhile */
  }
} /* end of function PrepDictTitle */


static VOID PrepMemTitle
( 
  PDOCUMENT_IDA pDoc, 
  BOOL fDispMemIndicator, 
  USHORT usTitle                       // string ID of titlebar text
)
{
  USHORT usI = 0;
  static CHAR szLongName[MAX_LONGFILESPEC];
  ULONG  Length;
  PSZ  pData;
  HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

  pData = pDoc->szBuf + 2* MAX_PATH144;
  strcpy( szLongName, pDoc->szMemory[0] );
  OEMTOANSI( szLongName );
  *pData = EOS;
  if ( fDispMemIndicator )
  {
    strcpy( pData, "[a]" );
  } /* endif */            
  strcat( pData, szLongName );

  if ( pDoc->szMemory[1][0] )
  {
    strcat( pData, "  (" );
    usI = 1;
    while ( pDoc->szMemory[usI][0] && (usI < EQF_MAX_TM_BASES) )
    {
      int iCurLen = 0;
      int iAddLen = 0;
      strcpy( szLongName, pDoc->szMemory[usI] );
      iCurLen = strlen(pData);
      iAddLen = strlen(szLongName) + 2;
      if ( fDispMemIndicator )
      {
        iAddLen += 3;
      } /* endif */            

      if ( (iCurLen + iAddLen) < MAX_PATH144 )
      {
        OEMTOANSI( szLongName );
        if ( fDispMemIndicator )
        {
          sprintf( pData+iCurLen, "[%c]", 'a' + usI ); 
        } /* endif */            
        strcat( pData, szLongName );
      }
      else
      {
        strcat( pData, ".." );
      } /* endif */

      if ( pDoc->szMemory[usI+1][0] )
      {
        strcat(pData,"  ");
      }
      else
      {
        strcat(pData,")");
      } /* endif */
      usI ++;
    } /* endwhile */
  } /* endif */

  WinLoadString (hab, hResMod, usTitle, EQF_MSGBUF_SIZE, (pDoc->szBuf+MAX_PATH144));
  DosInsMessage( &pData, 1, (pDoc->szBuf+MAX_PATH144), MAX_PATH144, pDoc->szBuf, MAX_PATH144, &Length );
} /* end of function PrepMemTitle */


//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     EqfGetPropState
//------------------------------------------------------------------------------
// Function call:     _
//------------------------------------------------------------------------------
// Description:       return flagging of type of proposal(m,f,r,e...)
//------------------------------------------------------------------------------
// Parameters:        PSTEQFGEN     pstEQFGen
//                    USHORT        usProp ( 0 - return flagging of all props
//                                           n - return flagging of nth prop
//------------------------------------------------------------------------------
// Returncode type:   USHORT
//------------------------------------------------------------------------------
// Returncodes:       MACHINE_TRANS_PROP
//                    FUZZY_PROP
//                    EXACT_PROP
//                    REPLACE_PROP
//                    FUZZY_REPLACE_PROP
//------------------------------------------------------------------------------
// Prerequesits:      ensure that usMachineTrans is set to 0
//------------------------------------------------------------------------------
// Side effects:      This function is nec since exact proposals do not have
//                    'e' indicator in front of the text and thus the first
//                    character of the filename in front of the proposal may
//                    be taken as indicator for the type of proposal
//------------------------------------------------------------------------------
// Function flow:     if flagging of one proposal required
//                      get flagging of this proposal
//                    else
//                      get flagging of all proposals
//                    endif
//------------------------------------------------------------------------------

USHORT EqfGetPropState
(
  PSTEQFGEN pstEqfGen,
  USHORT    usProp )
{

  PDOCUMENT_IDA  pDoc;                           // ptr to document inst ida
  USHORT         usMTLevel = 0;
  USHORT         usPropInEQFSab;

   pDoc = (PDOCUMENT_IDA) pstEqfGen->pDoc;

   // usProp is index in Proposal window, so get index in EQFSAB
   usPropInEQFSab = pDoc->stTWBS.usIndexInEqfSAB[usProp];

   if ( usPropInEQFSab )                // get type of one specified proposal
   {
     usMTLevel = MachineTrans( pDoc, usPropInEQFSab);
   }
   else
   {
     USHORT i=1;
     while ( i <= EQF_NPROP_TGTS )
     {
       usMTLevel |= MachineTrans(pDoc, i );
       i++;
     } /* endwhile */
   } /* endif */
   return (usMTLevel);
 } /* end of function EqfGetPropState*/


/*******************************************************************************
*
*       function:       EQFR_WordCntPerSeg
*
*******************************************************************************/
VOID  EQFR_WordCntPerSeg
(
  PDOCUMENT_IDA  pDoc                              // pointer to document ida
)
{
  PSTEQFGEN   pstEQFGen;               // pointer to generic struct
  PSTEQFPCMD  pstPCmd;                 // pointer to pipe cmd struct
  PSZ_W       pszSeg = NULL;           // ptr to Segment
  PSZ         pszLang = NULL;                 // ptr to Language of Segment
  PSZ         pszFormat = NULL;               // ptr to Format
  ULONG       ulResult = 0L;           // result to be counted
  ULONG       ulMarkUp = 0L;           // result for markup
  PVOID       pVoidTable = NULL;       // ptr to loaded tag table
  PTOKENENTRY pTokBuf = NULL;
  SHORT       sLangID = -1;
  ULONG       ulOemCP = 0L;

  pstEQFGen = pDoc->pstEQFGen;
  pstPCmd = pstEQFGen->pstEQFPCmd;            // pointer to command struct
  pstEQFGen->usRC    = NO_ERROR;

  // extract parameters from generic structure
  if ( pstEQFGen->usRC == NO_ERROR )
  {
    pszLang   = (PSZ)(pstPCmd->ucbBuffer);
    pszFormat = (PSZ)(pstPCmd->ucbBuffer + pstPCmd->usLen1);
    pszSeg    = (PSZ_W)(pstPCmd->ucbBuffer + pstPCmd->usLen1 + pstPCmd->usLen2);
  } /* endif */

   // allocate required buffers
  if ( pstEQFGen->usRC == NO_ERROR )
  {
    BOOL fOK = UtlAlloc((PVOID *)&pTokBuf, 0L, (LONG)TOK_BUFFER_SIZE, NOMSG );
    if (!fOK )
    {
      pstEQFGen->usRC = ERROR_INVALID_DATA;
    } /* endif */
  } /* endif */

  // Load Tag Table
  if ( pstEQFGen->usRC == NO_ERROR )
  {
    pstEQFGen->usRC =  TALoadTagTable( pszFormat,
                            (PLOADEDTABLE *) &pVoidTable,
                             FALSE, FALSE );
  } /* endif */

  // Get Language ID
  if ( pstEQFGen->usRC == NO_ERROR )
  {
    pstEQFGen->usRC = MorphGetLanguageID( pszLang, &sLangID );
    ulOemCP = GetLangOEMCP( pszLang);
  } /* end if */

  // do the actual counting
  if ( pstEQFGen->usRC == NO_ERROR )
  {
    pstEQFGen->usRC = EQFBWordCntPerSeg(
                     (PLOADEDTABLE)pVoidTable,
                     pTokBuf,
                     pszSeg,
                     sLangID,
                     &ulResult, &ulMarkUp, ulOemCP);
  } /* endif */

  //  return count value using generic structure
  if ( pstEQFGen->usRC == NO_ERROR )
  {
    pstPCmd->ulParm1 = ulResult;
    pstPCmd->usParm2 = (USHORT)ulMarkUp;
  } /* endif */

  // cleanup
  if ( pTokBuf )   UtlAlloc((PVOID *)&pTokBuf, 0L, 0L, NOMSG );
  if ( pVoidTable) TAFreeTagTable( (PLOADEDTABLE)pVoidTable );

  return ;
} // end 'EQFR_WordCntPerSeg'

USHORT
CalcPercent
(
  USHORT usDifferent,
  USHORT usTotal
)
{
   if (usDifferent > usTotal )
   {
     usDifferent = usTotal;
   }
   return(  (usTotal != 0) ? ((usTotal - usDifferent) * 100 / usTotal) : 100 );
}


// abbreviate filenames to the max.length specified
// logic:
// search until the 1st backslash or up to 30 chars:
// abbreviate here the filename with ...
// then skip enough chars so that the rest of the filename will fit
// pszAbbrFile already contains some data, so do only strcat the abbreviation!!

static VOID
EQFAbbrevFileName
 (
    PSZ_W      pszAbbrFile, 			// out
    LONG       lMaxLen,					// in:max length allowed
    PSZ_W      pszInFile                // in: filename to be abbreviated to maxlen
 )
{
    CHAR_W           chStop;
    PSZ_W            pszEnd;
    LONG             lLen;
    USHORT           usI = 0;
    USHORT           usBack = 0;
    LONG             lInLen = 0;

    lInLen = UTF16strlenCHAR(pszInFile);
    if (lInLen > lMaxLen )
    {
      while ((usI<30) && (usBack < 1))
      {
        if (*(pszInFile+usI ) == BACKSLASH )
        {
          usBack ++;
        } /* endif */
        usI++;
      } /* endwhile */
      chStop = pszInFile[usI];
      pszInFile[usI] = EOS;

      lLen = usI + lInLen + 3 - lMaxLen;

      pszEnd = pszInFile + lLen;
	  UTF16strcat(pszAbbrFile, pszInFile);
	  UTF16strcat(pszAbbrFile, L"...");
	  UTF16strcat(pszAbbrFile, pszEnd);

      pszInFile[usI] = chStop;
    }
    else
    {
		UTF16strcat(pszAbbrFile, pszInFile);
    } /* endif */
   return ;
} /* end of AbbrevFileName */

// get the visible whitespaces in Unicode -- remember, they are stored in ANSI
VOID
UtlGetUTF16VisibleWhiteSpace
(
	PTBDOCUMENT pNewDoc,
	PUSEROPT    pEQFBUserOpt,
	ULONG       ulAnsiCP
)
{
     CHAR_W      chW[2];
     CHAR        b[2];

     b[0] = pEQFBUserOpt->bVisibleBlank;
     b[1] = EOS;
     UtlDirectAnsi2Unicode( b, chW, ulAnsiCP );
     pNewDoc->chVisibleBlank = chW[0];
     b[0] = pEQFBUserOpt->bVisibleLineFeed;
     UtlDirectAnsi2Unicode( b, chW, ulAnsiCP );
     pNewDoc->chVisibleLF = chW[0];

     b[0] = pEQFBUserOpt->bSegmentBoundary;
     b[1] = EOS;
     UtlDirectAnsi2Unicode( b, chW, ulAnsiCP );
     pNewDoc->chSegBound = chW[0];
    return;
 }

// get fuzzyness of first fuzzy proposal
USHORT EqfGetPropFuzzyness
(
  PSTEQFGEN  pstEqfGen,
  PULONG    pulFuzzyness )
{

  PDOCUMENT_IDA  pDoc;                           // ptr to document inst ida
  PSTEQFSAB      pstEQFSab;
  USHORT         usRC = 0;

  pDoc = (PDOCUMENT_IDA)pstEqfGen->pDoc;
  if (pDoc && (pDoc->stEQFSab + pDoc->usFI)->fbInUse )
  {
	  pstEQFSab = pDoc->stEQFSab+ pDoc->usFI;
	  if (pstEQFSab)
	  {
      USHORT i = 0;
      USHORT usFuzzy = 0;
      while ( (usFuzzy == 0) && (i < EQF_NPROP_TGTS) )
      {
        USHORT usCurFuzzy = pstEQFSab->usFuzzyPercents[i];
        if ( usCurFuzzy <= 100 ) usFuzzy = usCurFuzzy;
        i++;
      } /* endwhile */
	   *pulFuzzyness = usFuzzy;
    }
    else
    {
  		usRC = EQFRS_NOT_AVAILABLE;
    }
  }
  else
  {
	  usRC = EQFRS_NOT_AVAILABLE;
  }
  return(usRC);
} /* end of function EqfGetPropFuzzyness*/


static
BOOL EqfFillFuzzyness
(
   PSTEQFSAB        pstEQFSab,
   USHORT           usPropNum,
   PDOCUMENT_IDA    pDoc,
   USHORT           usPropsInEqfSAB,
   PTBDOCUMENT      pTBDoc,
   PSZ_W            pucSeg0,
   SHORT            sMTLevel
)
{
   BOOL fOK = TRUE;
   usPropNum;                   // avoid compiler warning - unuser parameter

  if ( pstEQFSab->usFuzzyPercents[usPropsInEqfSAB-1] == 0 )
  {
	 USHORT     usTokens = 0;
	 PFUZZYTOK  pFuzzyTok = NULL;
	 USHORT     usModWords = 0;
	 USHORT     usDisplayFuzzyLevel = 0;
	 PSZ_W         pucSrcOfProp;

	 if ( !(sMTLevel & MACHINE_TRANS_PENDING) )
	 {
	   pucSrcOfProp = GetSrcPropSZ( pDoc, usPropsInEqfSAB );
	 }
	 else
	 {
	   pucSrcOfProp = pstEQFSab->pucSourceSeg;
	 } /* endif */


	fOK = EQFBFindDiff( pTBDoc, pucSeg0, pucSrcOfProp,
					  pDoc->sSrcLanguage, &pFuzzyTok, &usModWords,
					  &usTokens, pDoc->ulSrcOemCP );
	/******************************************************************/
	/* free allocated resources                                      */
	/*****************************************************************/
	if ( pFuzzyTok )
	{
	   UtlAlloc( (PVOID *) &pFuzzyTok, 0L, 0L, NOMSG );
	} /* endif */
	if (fOK)
	{
	  pstEQFSab->usFuzzyPercents[usPropsInEqfSAB-1] =
		   CalcPercent(usModWords, usTokens);
      // get fuzziness level for this segment
	  if ( usTokens > 15 )
	  {
		usDisplayFuzzyLevel = (USHORT)(UtlQueryULong( QL_LARGEFUZZLEVEL ) / 100);
	  }
	  else if ( usTokens > 4 )
	  {
		usDisplayFuzzyLevel = (USHORT)(UtlQueryULong( QL_MEDIUMFUZZLEVEL ) / 100);
	  }
	  else
	  {
		usDisplayFuzzyLevel = (USHORT)(UtlQueryULong( QL_SMALLFUZZLEVEL ) / 100);
	  } /* endif */
	  if (pstEQFSab->usFuzzyPercents[usPropsInEqfSAB-1] < usDisplayFuzzyLevel)
	  { // do not display this proposal, it is below DisplayFuzzylevel
          pstEQFSab->fInvisible[usPropsInEqfSAB-1] = TRUE;
      } /* endif */
	} /* endif */

  } /* endif */
  return (fOK);
 }  /* end of function EqfFillFuzzyness*/

static BOOL ContainsSegmentNote( PVOID pvAddInfo )
{
  BOOL fContainsNote = FALSE;
  static CHAR_W szBuffer[100];

  HADDDATAKEY hKey = MADSearchKey( (PSZ_W)pvAddInfo, L"Note" );
  if ( hKey != NULL )
  {
    MADGetAttr( hKey, L"style", szBuffer, sizeof(szBuffer) / sizeof(CHAR_W), L"" );
    if ( szBuffer[0] != EOS ) fContainsNote = TRUE;
    MDAGetValueForKey( hKey, szBuffer, sizeof(szBuffer) / sizeof(CHAR_W), L"" );
    if ( szBuffer[0] != EOS ) fContainsNote = TRUE;
  } /* endif */

  return( fContainsNote );
}

//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     AddDocToList
//------------------------------------------------------------------------------
// Function call:     fOK = AddDocToList( pIda, pDocName, usType );
//------------------------------------------------------------------------------
// Description:       add the provided document to a list of selected docs
//                    and mark the document to be in use.
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA pointer to document ida
//                    PSZ           document name
//                    USHORT        Insertion point - top or bottom of list
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE  - doc is added to list
//                    FALSE - not enough memory to add document to list
//------------------------------------------------------------------------------
// Function flow:     create a document pool (if not done yet)
//                    check if document is already in list
//                      if so move it to the required position
//                      else add it to list (at required position) and mark
//                           document to be in use
//                    return success
//------------------------------------------------------------------------------
BOOL AddDocToList
(
  PDOCUMENT_IDA pDocIda,
  PSZ           pDocName,
  USHORT        usType
)
{
  BOOL   fOK = TRUE;
  SHORT  sIndex = 0;
  PSZ    pszPool = NULL;

  /********************************************************************/
  /* do initial create if not done yet                                */
  /********************************************************************/
  if ( !pDocIda->pDocNamePool )
  {
    pDocIda->pDocNamePool = PoolCreate( MAX_EQF_PATH );
    fOK = (pDocIda->pDocNamePool != NULL);
  } /* endif */

  if ( fOK && pDocIda->usDocNamesAllocated <= (pDocIda->usDocNamesUsed+2) )
  {
    UtlAlloc( (PVOID *)&pDocIda->apszDocNames,
              (LONG) sizeof(PSZ) * pDocIda->usDocNamesAllocated,
              (LONG) sizeof(PSZ) * (pDocIda->usDocNamesAllocated+50),
              ERROR_STORAGE );
    if ( pDocIda->apszDocNames )
    {
      pDocIda->usDocNamesAllocated += 50;
    }
    else
    {
      fOK = FALSE;
    } /* endif */
  } /* endif */
  /********************************************************************/
  /* check if document is part of list already                        */
  /********************************************************************/
  if ( fOK )
  {
    while ( pDocIda->apszDocNames[sIndex] &&
            strcmp( pDocIda->apszDocNames[sIndex], pDocName ) != 0  )
    {
      sIndex++;
    } /* endwhile */

    if ( pDocIda->apszDocNames[sIndex]  )
    {
      short i;
      pszPool = pDocIda->apszDocNames[ sIndex ];
      if ( usType == TOP_OF_LIST )
      {
        for ( i=sIndex;i>0;i-- )
        {
          pDocIda->apszDocNames[i] = pDocIda->apszDocNames[ i-1 ];
        } /* endfor */
        pDocIda->apszDocNames[0] = pszPool;
      }
      else
      {
        for ( i=sIndex;i<(SHORT) pDocIda->usDocNamesUsed;i++ )
        {
          pDocIda->apszDocNames[i] = pDocIda->apszDocNames[ i+1 ];
        } /* endfor */
        pDocIda->apszDocNames[pDocIda->usDocNamesUsed-1] = pszPool;
      } /* endif */
    }
    else
    {
      pszPool = PoolAddString( pDocIda->pDocNamePool, pDocName );
      fOK = ( pszPool != NULL );

      if ( fOK )
      {
        if ( usType == TOP_OF_LIST )
        {
          memmove( &pDocIda->apszDocNames[1], & pDocIda->apszDocNames[0],
                   sizeof(PSZ) * (pDocIda->usDocNamesUsed+1) );
          pDocIda->usDocNamesUsed ++;
          pDocIda->apszDocNames[ 0 ] = pszPool;
        }
        else
        {
          pDocIda->apszDocNames[ pDocIda->usDocNamesUsed ] = pszPool;
          pDocIda->usDocNamesUsed ++;
        } /* endif */
        if ( QUERYSYMBOL(pDocName) != -1 )
        {
           PSZ pData = UtlGetFnameFromPath( pDocName );
           UtlError( ERROR_DOC_LOCKED, MB_CANCEL, 1, &pData, EQF_ERROR );
           fOK = FALSE;
        }
        else
        {
          SETSYMBOL( pDocName );
        } /* endif */
      } /* endif */
    } /* endif */
  } /* endif */
  return fOK;
} /* end of function AddDocToList */


//------------------------------------------------------------------------------
// Internal function
//------------------------------------------------------------------------------
// Function name:     QueryDocInList
//------------------------------------------------------------------------------
// Function call:     fOK = QueryDocInList( pIda, pDocName, &i );
//------------------------------------------------------------------------------
// Description:       check if the provided document is already in the list
//                    Return success indicator and set the index of the
//                    document in the list
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA pointer to document ida
//                    PSZ           document name
//                    PUSHORT       index
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE  - doc is in list
//                    FALSE - not in list
//------------------------------------------------------------------------------
// Function flow:     consistency check
//                    check if document is already in list
//                      if so set index and return
//                      else set failure
//                    return success
//------------------------------------------------------------------------------
BOOL QueryDocInList
(
  PDOCUMENT_IDA pDocIda,
  PSZ           pDocName,
  PUSHORT       pusI
)
{
  BOOL   fOK = TRUE;
  SHORT  sIndex = 0;
  /********************************************************************/
  /* check if document is part of list already                        */
  /********************************************************************/
  while ( pDocIda->apszDocNames[sIndex] &&
          (sIndex < (SHORT)pDocIda->usDocNamesUsed) &&
          strcmp( pDocIda->apszDocNames[sIndex], pDocName ) != 0  )
  {
    sIndex++;
  } /* endwhile */

  if ( pDocIda->apszDocNames[sIndex]  )
  {
    *pusI = sIndex;
  }
  else
  {
    fOK = FALSE;
    *pusI = 0;
  } /* endif */
  return fOK;
} /* end of function QueryDocInList */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     RemoveDocFromList
//------------------------------------------------------------------------------
// Function call:     RemoveDocFromList( pIda, pDocName );
//------------------------------------------------------------------------------
// Description:       remove the document from the list of worked on documents
//                    and mark it unused.
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA    -- document ida
//                    PSZ              -- document to be removed
//------------------------------------------------------------------------------
// Returncode type:   VOID
//------------------------------------------------------------------------------
// Function flow:     consistency check
//                    find document position in list
//                      -- if found - move everything following one up
//                                    and remove symbol
//                         else  ignore request
//------------------------------------------------------------------------------

VOID RemoveDocFromList
(
  PDOCUMENT_IDA   pDocIda,
  PSZ             pDocName
)
{
  BOOL   fOK = TRUE;
  SHORT  sIndex = 0;

  /********************************************************************/
  /* consistency checking                                             */
  /********************************************************************/
  fOK = ( pDocIda->pDocNamePool && pDocIda->apszDocNames );

  /********************************************************************/
  /* check if document is part of list already                        */
  /********************************************************************/
  if ( fOK )
  {
    while ( pDocIda->apszDocNames[sIndex] &&
            strcmp( pDocIda->apszDocNames[sIndex], pDocName ) != 0  )
    {
      sIndex++;
    } /* endwhile */

    if ( pDocIda->apszDocNames[sIndex]  )
    {
      short i;

      REMOVESYMBOL( pDocName );      // remove document from used list
      for ( i=sIndex;i<(SHORT)pDocIda->usDocNamesUsed;i++ )
      {
        pDocIda->apszDocNames[i] = pDocIda->apszDocNames[ i+1 ];
      } /* endfor */
      pDocIda->usDocNamesUsed--;
    }
    else
    {
      /****************************************************************/
      /* document not part of list -- ignore request ...              */
      /****************************************************************/
    } /* endif */
  } /* endif */

} /* end of function RemoveDocFromList */

//------------------------------------------------------------------------------
// External function
//------------------------------------------------------------------------------
// Function name:     FindNextDocInList
//------------------------------------------------------------------------------
// Function call:     fOK = FindNextDocInList(pDocIda,pInDocName, pOutDocName);
//------------------------------------------------------------------------------
// Description:       Find next document in list.
//                    Prereq is that the pNextDoc is pointing to space large
//                    enough
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA    -- pointer to ida
//                    PSZ              -- pointer to prov. start document
//                    PSZ              -- pointer to returned next document
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE             -- success
//                    FALSE            -- no further document found
//------------------------------------------------------------------------------
// Function flow:     consistency check
//                    try to find start document
//                      if found -- copy next document into pOutDocName
//                      else set error return and init pOutDocName
//                    return success
//------------------------------------------------------------------------------
BOOL FindNextDocInList
(
  PDOCUMENT_IDA   pDocIda,
  PSZ             pInDocName,
  PSZ             pOutDocName
)
{
  BOOL   fOK = TRUE;
  SHORT  sIndex = 0;

  /********************************************************************/
  /* consistency checking                                             */
  /********************************************************************/
  fOK = ( pDocIda->pDocNamePool && pDocIda->apszDocNames &&
          pInDocName && pOutDocName );

  /********************************************************************/
  /* check if document is part of list already                        */
  /********************************************************************/
  if ( fOK )
  {
    if ( *pInDocName )
    {
      while ( pDocIda->apszDocNames[sIndex] &&
              strcmp( pDocIda->apszDocNames[sIndex], pInDocName ) != 0  )
      {
        sIndex++;
      } /* endwhile */

      if ( pDocIda->apszDocNames[sIndex]  )
      {
        sIndex++;
        if ( sIndex < (SHORT)pDocIda->usDocNamesUsed)
        {
          strcpy( pOutDocName, pDocIda->apszDocNames[ sIndex ]);
        }
        else
        {
          *pOutDocName = EOS;
        } /* endif */
      }
      else
      {
        /****************************************************************/
        /* document not part of list -- ignore request ...              */
        /****************************************************************/
        *pOutDocName = EOS;
      } /* endif */
    }
    else if (pDocIda->apszDocNames[0])
    {
      strcpy( pOutDocName, pDocIda->apszDocNames[ 0 ]);
    }
    else
    {
      *pOutDocName = EOS;
    } /* endif */
  }
  else
  {
    /******************************************************************/
    /* init for error return                                          */
    /******************************************************************/
    fOK = FALSE;
  } /* endif */
  return fOK;
} /* end of function FindNextDocInList */

//------------------------------------------------------------------------------
// Function name:     FindDocInList
//------------------------------------------------------------------------------
// Function call:     fOK = FindDocInList(pDocIda, usI, pOutDocName);
//------------------------------------------------------------------------------
// Description:       Find document i in list
//------------------------------------------------------------------------------
// Parameters:        PDOCUMENT_IDA    -- pointer to ida
//                    USHORT           -- number of document
//                    PSZ              -- pointer to returned next document
//------------------------------------------------------------------------------
// Returncode type:   BOOL
//------------------------------------------------------------------------------
// Returncodes:       TRUE             -- success
//                    FALSE            -- failure
//------------------------------------------------------------------------------
// Function flow:     consistency check
//                    if ok copy i-th document to output buffer, else error
//                    return success indicator
BOOL FindDocInList
(
  PDOCUMENT_IDA   pDocIda,
  USHORT          usI,
  PSZ             pOutDocName
)
{
  BOOL   fOK;
  /********************************************************************/
  /* consistency checking                                             */
  /********************************************************************/
  fOK = ( pDocIda->pDocNamePool && pDocIda->apszDocNames && pOutDocName
          && (usI < pDocIda->usDocNamesUsed) && pDocIda->apszDocNames[ usI ]);

  /********************************************************************/
  /* return i-th document                                             */
  /********************************************************************/
  if ( fOK )
  {
    strcpy( pOutDocName, pDocIda->apszDocNames[ usI ]);
  }
  else
  {
    /******************************************************************/
    /* init for error return                                          */
    /******************************************************************/
    if ( pOutDocName )
    {
      *pOutDocName = EOS;
    } /* endif */
  } /* endif */
  return fOK;
} /* end of function FindDocInList */

