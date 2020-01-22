/*----------------------------------------------------------------------------\*
|                                                                              |
|                Copyright (C) 1990-2016, International Business Machines      |
|                Corporation and others. All rights reserved                   |
|                                                                              |
|   EQFTAPH2.C                                                                 |
|                                                                              |
|   TextSegmentation Module  phase 2                                           |
|                                                                              |
|   Author:      G. Queck                                                      |
|                                                                              |
|   Status:      05/16/90                                                      |
|                                                                              |
|   Description: Perform Translation memory processing and List Proc.          |
|                on already segmented files                                    |
|                                                                              |
|                                                                              |
|   Entry points:  EQFTASeg                                                    |
|                                                                              |
|   Keywords (for searching specific code parts)                               |
|       MTINTERFACE    code dealing with output of MT NOMATCH file             |
\*----------------------------------------------------------------------------*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_SLIDER           // slider utility functions
#define INCL_EQF_MORPH            // morphologic functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_EDITORAPI        // editor API (required for EQFTPI.H)
#include <eqf.h>                  // General Translation Manager include file
#include <eqftmi.h>
#include "eqftag00.h"
#include <eqftpi.h>               // required for EQFBPrepareFuzzyProp function

#include "OtmProposal.h"
#include "AnalysisMemLookupData.h"
#include "core\memory\MemoryFactory.h"
#include "vector"

#include "EQFTAI.H"               // Analysis private include file
#include <EQFTMTAG.H>             // Translation Memory SGML tags
#include <TIME.H>                 // C time functions
#include "EQFHLOG.H"                 // defines for history log processing
#include "EQFRPT00.H"             // fuzzyness threshold values
#include <eqfaprof.h>             // analysis profile functions
#include "eqfserno.h"             // OpenTM2 driver info

#define EQFTAML_MATCHLISTFUNCTIONS
#include "EQFTAML.H"              // TA match list functions

#include <EQFTADIT.H>             // IBMIDDOC->IBMDITA special processing functions

// include list of markup tables which are supported by MT server
#define MTLIST_DEFINE_NOINLINETAGMASKING_ARRAY
#include "eqfmtlst.h"
#include <eqfentity.h>



// if the following define is active, segments containing not supported inline tags will
// be added to the MT segment file for later inline tag masking
//#define MT_INLINETAG_MASKING

//  #define LOGMATCHES


#ifdef _DEBUG
  // activate this define to log TM match counting results to file c:\TAMATCH.LOG
  //#define LOGMATCHES
  //#define MEASUREANALYSISTIME
  //#define TA_SEGMENTLOGGING
  //#define SESSIONLOG
#endif

#ifdef LOGMATCHES
FILE *hMatchLogFile = NULLHANDLE;
#endif


#define DUPTEMPMEM "$MTDUP"

// time and performance measurements
#ifdef MEASUREANALYSISTIME
  // global variables for time measurement
  clock_t clockCurrent, clockNew;
#endif

#ifdef MEASUREANALYSISTIME
  #define TIMERINIT() clockCurrent = clock()
#else
  #define TIMERINIT()
#endif

#ifdef MEASUREANALYSISTIME
  #define ADDTIME( timer ) \
  { clockNew = clock(); timer = timer + (ULONG)(clockNew - clockCurrent); clockCurrent = clockNew; }
#else
  #define ADDTIME( timer )
#endif

/*-------------- private functions -------------------------------------------*/
static int GetBestTMMatch
(
  PTAINPUT     pTAIn,                  // ta instance data
  PSZ_W        pszSegment,             // pointer to segmentboundary
  std::vector<OtmMemory *> &SearchMemories,     // pointer to tm's
  std::vector<OtmProposal *> &BestMatch,     // pointer to found proposals
  BOOL         * pfExactMatch,         // exact match found?
  USHORT       usNumMatches            // number of matches to be
);


static USHORT 
  GetExactTMMatch(PTAINPUT pTAIn,              // ta instance data
                  PSZ_W          pszSegment,     // pointer to segmentboundary
                  std::vector<OtmMemory *> &SearchMemories,     // pointer to tm's
                  BOOL         * pfExactMatch, // exact match found?
                  BOOL         * pfAnyMatch,   // any (=fuzzy) match found?
                  USHORT       usNumMatches,   // number of matches to be
                  PMATCHTYPE   pMatchType,     // type of matches found
                  PUSHORT      pusFuzzy,       // fuzzyness of proposal  
                  PVOID        pvMetaData);    // segment metadata

static USHORT TASegTMProc ( PTAINPUT pTAIn, PTBSEGMENT pSeg, ULONG ulSegNum, BOOL fAutoJoineMode, BOOL fOnlyLookup );

USHORT TACountMatch
(
  PTAINSTDATA pInD,                    // pointer to instance data for TA
  MATCHTYPE   MatchType,               // type of match
  USHORT      usFuzzyness,             // fuzzyness of match
  LONG        lSrcWords,               // number of source words
  SHORT       sSegments,               // number of segments
  BOOL        fTotal                   // TRUE = add to total word numner
);

VOID TAAdjustMatchType
(
  MATCHTYPE   *pMatchType,             // type of match
  USHORT      usFuzzyness,             // fuzzyness of match
  LONG        lSrcWords                // number of source words
);

BOOL TAMachTransAddSeg
(
  PTAINPUT    pTAIn,
  ULONG       ulSrcWords,
  BOOL        fContainsTagging,
  MATCHTYPE   MatchType,
  USHORT      usFuzzyness,
  ULONG       ulSegNum,
  PSZ_W       pszSegSource,
  PTBSEGMENT  pSeg
);

// subfunctions for TASegTMProc
static BOOL
TAHandleCheckSegExit
(
	PTAINSTDATA  pInD,
	PTBSEGMENT   pSeg,
	PSZ_W        pszMatch,
	ULONG        ulTgtOemCodePage,
    PBOOL        pfOK,
    HWND         hwndErrMsg
);

static BOOL
TAInsertToTM
(
	PTAINPUT     pTAIn,
	PTAINSTDATA  pInD,
	PTBSEGMENT   pSeg,
	PBOOL        pfExactMatch
);

static BOOL
TASegTMRedundCount
(
	PTAINPUT     pTAIn,
	PTAINSTDATA  pInD,
	PTBSEGMENT   pSeg,
	MATCHTYPE    MatchType
);
static BOOL
TAHandleAllAdjustOptions
(
	PTAINPUT      pTAIn,
	PTBSEGMENT    pSeg,
	PSZ_W         pszMatch,
	PSZ_W        *ppszNewTargetW,
	PBOOL         pfChanged
);


static BOOL
TACheckForWS
(
	CHAR_W c,
	PSZ_W  pWSList
);


static const CHAR szCloseSeq[] = "\x0D\x0A\x1A";  // file close sequence

#ifdef LOGMATCHES

  void TAMarkCRLF( PSZ_W pszSource, PSZ_W pszTarget )
  {
    while ( *pszSource )
    {
      if ( *pszSource == L'\n' )
      {
        wcscpy( pszTarget, L"<lf>" );
        pszTarget += 4;
      }
      else if ( *pszSource == L'\r' )
      {
        wcscpy( pszTarget, L"<cr>" );
        pszTarget += 4;
      }
      else 
      {
        *pszTarget++ = *pszSource;
      } /* endif */
      pszSource++;
    } /*endwhile */
    *pszTarget = 0;
  }

  void TALogSegData( FILE *hfLog, PSZ_W pszForm, PSZ_W pszSegData )
  {
    static CHAR_W szSegBuf[4096];
    static CHAR_W szLineBuf[4096];
    int iLen;

    TAMarkCRLF( pszSegData, szSegBuf );
    iLen = swprintf( szLineBuf, pszForm, szSegBuf );
    fwrite( szLineBuf, 2, iLen, hfLog ); 
  }
#endif


// prototypes for nFluent format XML output functions
  BOOL TAOpenMTFile( PTAINPUT pTAInput, PSZ pszFileName );
  BOOL TAWriteMTSegment( PTAINPUT pTAInput, ULONG ulLength, PSZ_W pszData, ULONG ulSegNum, USHORT usFuzzy, MATCHTYPE MatchType, PTBSEGMENT pSeg, ULONG ulSourceWords );
  BOOL TACloseMTFile( PTAINPUT pTAInput );

/*----------------------------------------------------------------------------*\
| TASegProc : process segmented file                                           |
| purpose   : if requested perform Translation Memory Processing               |
|             if requested perform List processing                             |
|                                                                              |
|  if there is another file to be processed in the list of files, the file     |
| Input     : pointer to the instance data area; the variable                  |
|             pszCurSourceFile must point to the name of the file              |
|             to analyze                                                       |
| Output    : returns TACOMPL  if everything is OK                             |
|               else  TAERROR                                                  |
|             the segmented files are read and the segmented target is         |
|             created, if needed also the List of terms are created            |
\*----------------------------------------------------------------------------*/
USHORT TASegProc       (HWND hwnd,    // text analysis window handle
             PTAINPUT  pTAInput,      // input structure for text analysis
             PUSHORT   pusPhase )     // current processing phase
{
   PTAINSTDATA   pInD;                 // pointer to instance data for TA
   USHORT        usReturn;             // return values from functions
   TATASK        usPhase;              // activity sequencer
   BOOL          fOK = TRUE;           // Processing status flag
   CHAR          szFileName[MAX_EQF_PATH];  // full name of file to be opended
   CHAR          szSegTarget[MAX_EQF_PATH]; // full name of file to be opended
   SHORT         sItem;                     // selected item
   PUSHORT       pusTemp;              // pointer to file status
   ULONG         ulTemp;               // scratch long variable
   BOOL          fWindowExists = TRUE;;

   pInD = pTAInput->pInD;                   // pointer to local variables

   usPhase =(TATASK) *pusPhase;

      if ( pTAInput->fKill )           // problems or
      {                             // request for stop processing
        pInD->fTerminate = TRUE;
        pTAInput->fKill = FALSE;      // close flag already processed
      }

      if ( pInD->fTerminate && (usPhase != EQFTA_FREE))
      {
         usPhase = EQFTA_ABORT;
      }

      switch ( usPhase )
      {
         case EQFTA_INIT:
#ifdef LOGMATCHES
              {
                CHAR szLogFile[MAX_EQF_PATH];
                UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
                UtlMkDir( szLogFile, 0, FALSE );
                strcat( szLogFile, "\\TAMATCH.LOG" );
                hMatchLogFile = fopen( szLogFile, "ab" );
                fwprintf( hMatchLogFile, L"EQFTA_INIT\r\n" );
              }
#endif
              pInD->fTMInserted = FALSE;         // no TM matches inserted right now
              pInD->fCrlfFlag = FALSE;           // no CRLF checking based on tags yet
			  pTAInput->fSetSegDateDone = FALSE; // no document properties updated yet
              if ( pTAInput->fInsertTMMatches )
              {
                pInD->ulSegsReplaced = 0L;
                pInD->ulReplacedWords = 0L;
                pInD->ulSegsNotReplaced = 0L;
                pInD->ulTotalSegs = 0L;
              } /* endif */

              pInD->ulNoProps    = 0L;
              pInD->ulFuzzy      = 0L;
              pInD->ulExactExact = 0L;
              pInD->ulExactOne   = 0L;
              pInD->ulExactMore  = 0L;
              pInD->ulTotal      = 0L;
              pInD->ulRepl       = 0L;
              pInD->ulFuzzyRepl  = 0L;
              pInD->ulSegNoProps    = 0L;
              pInD->ulSegFuzzy      = 0L;
              pInD->ulSegExactExact = 0L;
              pInD->ulSegExactOne   = 0L;
              pInD->ulSegExactMore  = 0L;
              pInD->ulSegTotal      = 0L;
              pInD->ulSegRepl       = 0L;
              pInD->ulSegFuzzyRepl  = 0L;
              pInD->ulSegMachineMatch  = 0L;
              pInD->ulMachineMatch  = 0L;

              TIMERINIT();

              memset( &pInD->Total, 0, sizeof(COUNTSUMS) );
              memset( &pInD->ExactExact, 0, sizeof(COUNTSUMS) );
              memset( &pInD->Exact, 0, sizeof(COUNTSUMS) );
              memset( &pInD->Fuzzy1, 0, sizeof(COUNTSUMS) );
              memset( &pInD->Fuzzy2, 0, sizeof(COUNTSUMS) );
              memset( &pInD->Fuzzy3, 0, sizeof(COUNTSUMS) );
              memset( &pInD->NoProps, 0, sizeof(COUNTSUMS) );
              memset( &pInD->MTProps, 0, sizeof(COUNTSUMS) );
              memset( &pInD->Repl, 0, sizeof(COUNTSUMS) );
              memset( &pInD->ExactMore, 0, sizeof(COUNTSUMS) );

              sprintf( szFileName,      // create full file name
                       PATHCATFILE,
                       pTAInput->szSOURCE_Path,
                       pInD->pszCurSourceFile);

              /*---- select file in list --------*/
              if ( hwnd != HWND_FUNCIF )
              {
                SEARCHSELECTHWND( sItem, pTAInput->hwndToDo,
                   pTAInput->apszLongNames[pInD->usCurNumSourceFile-1] );
              } /* endif */

              pusTemp = pInD->pusFileStatus +      // set file status to false
                         (pInD->usCurNumSourceFile - 1);
              *pusTemp = FALSE;   // set file status to failed

              pTAInput->sSliderPos = 0;

              /*---- reset slider -----------*/
              if ( hwnd != HWND_FUNCIF )
              {
                WinSendMsg( pTAInput->hwndProcWin, WM_EQF_UPDATESLIDER,
                            MP1FROMSHORT(0), NULL );
              } /* endif */
              usReturn = TACOMPL;
              usPhase = EQFTA_LIST;
              break;

         case EQFTA_LIST:     // segmentation: open file and select
              // if an error occured during opening, the file is skipped
              // and text analysis is cancelled, if user requested so.
              fOK = TASegOpenFiles(pTAInput);
              usPhase = ( fOK ) ? EQFTA_LIST1 : EQFTA_ABORT;
              pInD->szContext[0] = 0;     // clear context
              break;

         case EQFTA_LIST1:    //  process segments
              /*---- check end of file -------*/
              if (pInD->fAll)
              {
                if ( pTAInput->fAutoJoin )
                {
                  pInD->ulSegNum = 1;
                  pInD->fAll = FALSE;
                  pInD->szContext[0] = 0;     // clear context
                  pInD->fCrlfFlag = FALSE;    // no CRLF checking based on tags yet

                  usPhase =  EQFTA_AUTOJOIN;  // continue with autojoin test
                }
                else
                {
                  usPhase =  EQFTA_LIST2;  // continue with close files
                } /* endif */
              }
              else
                 {
                 if (TASegBlockProc( hwnd, pTAInput ))
                    usPhase =  EQFTA_LIST1;  // continue
                 else
                    usPhase = EQFTA_ABORT; // free resources and exit
                 }
              break;

         case EQFTA_AUTOJOIN:    //  process segments
              /*---- check end of file -------*/
              if (pInD->fAll) // EOF: end process
                 usPhase =  EQFTA_LIST2;  // continue with close files
              else
                 {
                 if (TASegAutoJoin( hwnd, pTAInput ))
                    usPhase =  EQFTA_AUTOJOIN;  // continue
                 else
                    usPhase = EQFTA_ABORT; // free resources and exit
                 }
              break;

        case EQFTA_LIST2:
            if (TASegCloseFiles(pTAInput))
            {
               usPhase = EQFTA_POSTTM;
            }
            else
            {
               PSZ pszDoc;
               pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
               if ( pTAInput->fBatch )
               {
                 usReturn = UtlErrorHwnd ( ERROR_TA_SAVE_SEGFILE, MB_CANCEL, 1,
                                           &pszDoc, EQF_QUERY,
                                           pTAInput->hwndErrMsg );
                 pTAInput->fKill = TRUE;
               }
               else
               {
                 usReturn = UtlError ( ERROR_TA_SAVE_SEGFILE, MB_YESNO,
                                       1, &pszDoc, EQF_QUERY );
                 pTAInput->fKill = (usReturn == MBID_YES) ? FALSE : TRUE;
               } /* endif */
               usPhase = EQFTA_ABORT;
            } /* endif */
            break;

         case EQFTA_COMPLETE:
            // insert processed file in done listbox
            INSERTITEMHWND( pTAInput->hwndDone,
                            pTAInput->apszLongNames[pInD->usCurNumSourceFile-1] );
            pusTemp = pInD->pusFileStatus +      // set file status to false
                      (pInD->usCurNumSourceFile - 1);
            *pusTemp = TRUE;  // set file status to OK
            UtlTime( (PLONG)&ulTemp );
            if ( !pTAInput->fSetSegDateDone ) SetSegDate (pTAInput, ulTemp);
            usPhase =  EQFTA_FREE; // go ahead with free resources
            break;


         case EQFTA_POSTTM:  //--------- do post TM user exit
            fOK = TRUE;
            fOK = TAProcessPostTM ( pTAInput, pInD,
                                    szFileName, szSegTarget,
                                    pInD->fTMInserted );
            if (fOK)
            {
               usPhase = EQFTA_COMPLETE;
            }
            else
            {
               PSZ pszDoc;
               pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];

               if ( pTAInput->fBatch )
               {
                 usReturn = UtlErrorHwnd ( ERROR_TA_SAVE_SEGFILE, MB_CANCEL, 1,
                                           &pszDoc, EQF_QUERY,
                                           pTAInput->hwndErrMsg);
                 pTAInput->fKill = TRUE;
               }
               else
               {
                 usReturn = UtlError ( ERROR_TA_SAVE_SEGFILE, MB_YESNO, 1,
                                       &pszDoc, EQF_QUERY );
                 pTAInput->fKill = (usReturn == MBID_YES) ? FALSE : TRUE;
               } /* endif */
               usPhase = EQFTA_ABORT;
            } /* endif */
            break;

         case EQFTA_ABORT:   // abort process: clean up
            pInD->usError = TAERROR;
            TASegCloseFiles(pTAInput);
            // delete segmented target
            if ( pInD->pszCurSourceFile && (pInD->pszCurSourceFile[0] != EOS) )
            {
              TADELETE( szFileName,
                        pTAInput->szSEGTARGET_Path,
                        pInD->pszCurSourceFile);
            } /* endif */
            usPhase =  EQFTA_FREE;       // free resources
            break;

         case EQFTA_FREE: // free allocated resources
            // delete file from process box
#ifdef LOGMATCHES
             fwprintf( hMatchLogFile, L"EQFTA_FREE\r\n" );
             fclose( hMatchLogFile );
#endif
             ADDTIME( pTAInput->ulTimeOther );

            if ( fWindowExists )
            {
               sItem = SEARCHITEMHWND( pTAInput->hwndToDo,
                        pTAInput->apszLongNames[pInD->usCurNumSourceFile-1] );
               DELETEITEMHWND( pTAInput->hwndToDo, sItem );
            } /* endif */
            usPhase =  EQFTA_DONE;          // done: exit
            break;

         } /* end task selection */

      // if user requested to stop the global variable fTerminate is set
      // so, return always TACOMPL

      *pusPhase = (USHORT) usPhase;

      return (TACOMPL);

   } /*-- end of TASegmProc ---*/

/*----------------------------------------------------------------------------*\
|  TASegCreate : perform  initialization for Segment processing                |
|  Input    : input parameters structure pTAInput                              |
|  Output   : dictionaries are opened, tag tables are read,                    |
|                                     memory areas are allocated....           |
|                                                                              |
\*----------------------------------------------------------------------------*/
USHORT TASegCreate ( HWND hwnd, PTAINPUT pTAInput )
   {
   PTAINSTDATA      pInD;                    // pointer to instance data
   BOOL             fOK = TRUE;              // Proessing status flag
   USHORT           usRc;                    // return from called functions

   hwnd = hwnd; //get rid of compiler warning

   pInD = pTAInput->pInD;              // store pointer to pInd temporarily

   usRc = TALoadTagTable( QFTAG_TABLE,
                          &(pInD->pLoadedQFTable),
                          TRUE,                  // load internal table
                          TRUE );                // do message handling
   if ( usRc != NO_ERROR )
   {
     fOK = FALSE;
     pInD->fTerminate = TRUE;
   } /* endif */

   if (fOK)   // allocate buffer for segment text
      {
      fOK = UtlAlloc( (PVOID *) &(pInD->pszSegment),
                      0L,
                      (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W),
                      ERROR_STORAGE);
      }

   if (fOK)   // allocate buffer for segmented text of prev. segment
   {
      fOK = UtlAlloc( (PVOID *) &(pInD->pszPrevSegment),
                      0L, (LONG) MAX_SEGMENT_SIZE * sizeof(CHAR_W), ERROR_STORAGE);
   } /* endif */

   if (fOK)   // allocate buffer for tag text
      {
      fOK = UtlAlloc( (PVOID *) &(pInD->pszTag),
                      0L,
                      (LONG) MAXTAGLEN,
                      ERROR_STORAGE);
      }


   // create temporary TM for redundancy counting
   if ( fOK && pTAInput->fRedundCount )
   {
     // initialize some fields
     pInD->sTMLangID = -1;             // no language activated yet

     // create the TM
     {
        PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;
        MemoryFactory *pFactory = MemoryFactory::getInstance();

        // get folder source language
        FolQueryInfo( pTAInput->szFolder, NULL, NULL, pInD->szTMLang, NULL, TRUE );

        pLookupData->pRedundCountMem = pFactory->createTempMemory( "$RA", pInD->szRedundCountTM, pInD->szTMLang );
        // create the Translation Memory
        if ( pLookupData->pRedundCountMem  == NULL )
        {
          // error during creation of temp TM
          // TODO: Error handling
#ifdef SESSIONLOG
          UtlLogWriteString( "Analysis: Error creating redundancy memory %s", pInD->szRedundCountTM );
#endif
          fOK = FALSE;
        }
        else
        {
#ifdef SESSIONLOG
          UtlLogWriteString( "Analysis: Created redundancy memory %s", pInD->szRedundCountTM );
#endif
        } /* endif */
     }

     // allocate tokenize buffer (required for EQFWrdCntPerSeg)
     if ( fOK )
     {
       fOK = UtlAllocHwnd( (PVOID *)&(pInD->pTokBuf), 0L, TOK_BUFFER_SIZE,
                             ERROR_STORAGE, pTAInput->hwndErrMsg );
     } /* endif */

     // prepare most used segment table
     // Note: If the threshhold value and the number of segments is
     //       to be set dynamically by the user this is the right
     //       place to do this. Read the values from any user profile
     //       instead of using the predefined constants. All other code
     //       dealing with this numbers will use the values in the
     //       pInD structure rather than accessing the constants directly
     if ( fOK )
     {
       // set the threshhold and number of segments values
       // note: the values are set to their max/min values, filtering is done in the report code
       pInD->sMaxMostUsedThreshhold = MAX_MOSTUSEDSEGSTHRESHHOLD;
       pInD->sMaxMostUsedSegments = MAX_MOSTUSEDSEGS;

       // allocate the most used segments table
       fOK = UtlAlloc( (PVOID *) &(pInD->pMostUsedSegs), 0L,
            (LONG)(pInD->sMaxMostUsedSegments * sizeof(TAMOSTUSEDSEG)),
            ERROR_STORAGE);
     } /* endif */
   } /* endif */

   return (fOK ? TACOMPL : TAERROR);
   } /* end of TASegCreate */

/*----------------------------------------------------------------------------*\
| TASegCloseFiles: save segmented target file                                  |
| input     : pointer to the data area                                         |
| output    : closes the files; return TRUE if everything OK                   |
\*----------------------------------------------------------------------------*/
USHORT TASegCloseFiles (PTAINPUT pTAInput)
{
   PTAINSTDATA    pInD;
   CHAR          szFileName[MAX_EQF_PATH];  // full name of file to be opended
   USHORT         fOK = TRUE;                      // flag for operation

   pInD = pTAInput->pInD;

   if ( pInD->pDoc )
   {
     SHORT sRc;

     /*****************************************************************/
     /* prepare write of segmented target file                        */
     /*****************************************************************/
      sprintf( szFileName,    // create full file name
               PATHCATFILE,
               pTAInput->szSEGTARGET_Path,
               pInD->pszCurSourceFile);
     strcpy( pInD->pDoc->szDocName, szFileName );
     pInD->pDoc->docType = STARGET_DOC;
     strcpy( pInD->pDoc->szDocLongName,
             pTAInput->apszLongNames[pInD->usCurNumSourceFile-1] );

     // do any entity processing
     if ( UtlQueryUShort( QS_ENTITYPROCESSING ) )
     {
       if ( isEntityMarkup( pInD->szCurFormat ) )
       {
         ScanForEntities( pInD->pDoc );
       } /* endif */
     } /* endif */


     /*****************************************************************/
     /* Write segmented target file                                   */
     /*****************************************************************/
     sRc = EQFBFileWriteEx( szFileName, pInD->pDoc, AUTOMATICSUBST_LOGTASK, 0 );
     fOK = (sRc == NO_ERROR);
#ifdef SESSIONLOG
     if ( !fOK )
     {
       UtlLogWriteString( "TASegCloseFiles : EQFBFileWriteEx for document %s failed", szFileName );
     } /* endif */        
#endif

     // write metadata if any metadata has been added during automatic substitution
     if ( fOK && pInD->fMetaDataAdded )
     {
       MDWriteMetaData( pInD->pDoc );
     } /* endif */        

     /*****************************************************************/
     /* Write segmented source file if autojoin has modified the      */
     /* file                                                          */
     /*****************************************************************/
     if ( fOK && pTAInput->fAutoJoin && pTAInput->fAutoJoinDone )
     {
       sprintf( szFileName,    // create full file name
                PATHCATFILE,
                pTAInput->szSEGSOURCE_Path,
                pInD->pszCurSourceFile);
       sRc = EQFBFileWrite( szFileName, pInD->pSourceDoc );
       fOK = (sRc == NO_ERROR);
     } /* endif */

     /*****************************************************************/
     /* Free document structure(s)                                    */
     /*****************************************************************/
#ifdef SESSIONLOG
     UtlLogWriteString( "Analysis: Document %s processed", pInD->pDoc->szDocLongName );
#endif
     TAFreeDoc((PVOID *) &(pInD->pDoc) );
     if ( pTAInput->fAutoJoin )
     {
       TAFreeDoc((PVOID *) &(pInD->pSourceDoc) );
     } /* endif */
   } /* endif */

   if ( fOK && pInD->pCBNewMatchFile )
   {
     fOK = ( UtlBufWrite( pInD->pCBNewMatchFile, NTM_END_TAG,
                          (USHORT)strlen(NTM_END_TAG), TRUE ) == NO_ERROR );
     if ( fOK )
     {
       fOK = ( UtlBufWrite( pInD->pCBNewMatchFile,
                            (PSZ)szCloseSeq,
                            (USHORT)strlen(szCloseSeq),
                            TRUE) == NO_ERROR );
     } /* endif */
#ifdef SESSIONLOG
     if ( !fOK )
     {
       UtlLogWriteString( "TASegCloseFiles : UtlBufWrite for document %s failed", szFileName );
     } /* endif */        
#endif
   } /* endif */

   if ( pInD->pCBNewMatchFile )
   {
     fOK = (UtlBufClose( pInD->pCBNewMatchFile, TRUE ) == NO_ERROR ) && fOK;
     pInD->pCBNewMatchFile = NULL;
     if ( !pInD->fNewMatchFileUsed ) // no segments have been written to file
     {
       sprintf( szFileName,    // create full file name
                PATHCATFILE,
                pTAInput->szSEGNEWMATCH_Path,
                pInD->pszCurSourceFile);
       UtlDelete( szFileName, 0L, NOMSG );

       /***************************************************************/
       /* Suppress info message if more than one document is analyzed */
       /***************************************************************/
       if ( !pTAInput->fBatch && (pTAInput->stSourcefiles.usNumber <= 1) )
       {
         UtlError ( INFO_NO_NOMATCH_SEGMENTS, MB_OK, 0, NULL, EQF_INFO );
       } /* endif */
     } /* endif */
     /*****************************************************************/
     /* force update of MT list ...                                   */
     /*****************************************************************/
     EqfSend2Handler( MTLISTHANDLER, WM_EQFN_CREATED,
                      MP1FROMSHORT( clsMTLIST ), MP2FROMP(NULL) );
   } /* endif */

   return (fOK);
}

/*----------------------------------------------------------------------------*\
| TASegOpenFiles: load input file                                              |
| input     : pointer to the data area                                         |
| output    : file is loaded;  return TRUE if everything OK                    |
\*----------------------------------------------------------------------------*/
USHORT TASegOpenFiles (PTAINPUT pTAInput)
   {
   PTAINSTDATA    pInD;
   CHAR           szFileName[MAX_EQF_PATH];  // full name of file to be opended
   USHORT         fOK = TRUE;                // flag for operation
   USHORT         usReturn;                  //
   PSZ             pszErrParm;               // ptr to parms for calls to UtlError
   BOOL           fMsgShown = FALSE;         // TRUE = error message already has been shown

   pInD = pTAInput->pInD;

   /********************************************************/
   /* Get source language and format of new document       */
   /********************************************************/
   strcpy( pInD->szDocObjName, pTAInput->szFolder );
   strcat( pInD->szDocObjName, BACKSLASH_STR );
   strcat( pInD->szDocObjName, pInD->pszCurSourceFile );
   DocQueryInfo( pInD->szDocObjName,    // document object name
                 pInD->szDocMemory,     // memory of document
                 pInD->szDocFormat,     // format of document
                 pInD->szDocSourceLang, // document source language
                 pInD->szDocTargetLang, // --   target language
                 TRUE );                // handle errors in function


   // get markup specific settings if a profile is active
   if ( pTAInput->fProfile && (pTAInput->hProfile != 0) )
   {
     AnaProfApplyProfile( pTAInput->hProfile, pTAInput, pInD->szDocFormat, FALSE );
   } /* endif */

   // get CRLF info structure for this markup
   pInD->pCrlfInfoForMarkup = TAGetCRLFInfoForMarkup( pInD->pCrlfLoadedInfo, pInD->szDocFormat );

   /********************************************************/
   /* Activate document language if not active yet         */
   /********************************************************/
   if ( strcmp( pInD->szCurSourceLang, pInD->szDocSourceLang ) != 0 )
   {
     /******************************************************/
     /* Deactivate language currently active               */
     /******************************************************/
     if ( pInD->szCurSourceLang[0] != EOS )
     {
       MorphFreeLanguageID( pInD->TolstControl.sLangID );
     } /* endif */

     /******************************************************/
     /* Activate new language                              */
     /******************************************************/
     usReturn = MorphGetLanguageID( pInD->szDocSourceLang,
                                &(pInD->TolstControl.sLangID) );

     if ( usReturn == NO_ERROR )
     {
       pInD->TolstControl.ulOemCP = GetLangOEMCP( pInD->szDocSourceLang);
       pInD->TolstControl.ulAnsiCP = GetLangAnsiCP( pInD->szDocSourceLang);
       strcpy( pInD->szCurSourceLang, pInD->szDocSourceLang );
     }
     else
     {
       pszErrParm = pInD->szDocSourceLang;
       usReturn = UtlErrorHwnd ( ERROR_TA_DICTACT,
                             MB_CANCEL ,
                             1,
                             &pszErrParm,
                             EQF_WARNING,
                             pTAInput->hwndErrMsg );
       fOK = FALSE;
#ifdef SESSIONLOG
      if ( !fOK )
      {
        UtlLogWriteString( "TASegOpenFiles: MorphGetLanguageID for language %s failed", pInD->szDocSourceLang );
      } /* endif */        
#endif
     } /* endif */
   } /* endif */

   /********************************************************/
   /* Activate document target language                    */
   /********************************************************/
   if ( fOK )
   {
     usReturn = MorphGetLanguageID( pInD->szDocTargetLang,
                                    &(pInD->sTargetLangID) );
     if ( usReturn != NO_ERROR )
     {
       // use ID of source language instead
       pInD->sTargetLangID = pInD->TolstControl.sLangID;
#ifdef SESSIONLOG
        UtlLogWriteString( "TASegOpenFiles: MorphGetLanguageID for language %s failed", pInD->szDocTargetLang );
#endif
     } /* endif */
   } /* endif */

   /********************************************************/
   /* Load tagtable for document format if not active yet  */
   /********************************************************/
   if ( fOK )
   {

     if ( strcmp( pInD->szCurFormat, pInD->szDocFormat ) != 0 )
     {
       /******************************************************/
       /* Free tag table currently active                    */
       /******************************************************/
       if ( pInD->szCurFormat[0] != EOS )
       {
         if ( pInD->pLoadedTable->pTagTable->szSegmentExit[0] != EOS )
         {
           TAFreeUserExit( pInD );
         } /* endif */
         TAFreeTagTable( pInD->pLoadedTable );
       } /* endif */

       /******************************************************/
       /* Activate new tag table                             */
       /******************************************************/
       usReturn = TALoadTagTable( pInD->szDocFormat,
                                  &pInD->pLoadedTable,
                                  FALSE,           // load external table
                                  TRUE );          // do message handling
       if ( usReturn == NO_ERROR )
       {
         CHAR szExitName[MAX_LONGFILESPEC];
         strcpy( pInD->szCurFormat, pInD->szDocFormat );
         if ( MUGetUserExitFileName(  pInD->szDocFormat, NULL, szExitName, sizeof(szExitName) ) )
         {
           usReturn = TALoadUserExit( szExitName, pTAInput );
           if ( usReturn != NO_ERROR )
           {
             fOK = FALSE;
#ifdef SESSIONLOG
            UtlLogWriteString( "TASegOpenFiles: TaLoadTagTable for markup %s failed", pInD->szDocFormat );
#endif
           } /* endif */
         }
         else
         {
          // reset all user exit function pointers
          pInD->pfnEQFPreSeg    = NULL;
          pInD->pfnEQFPreSeg2   = NULL;
          pInD->pfnEQFPreSegEx  = NULL;
          pInD->pfnEQFPostSeg   = NULL;
          pInD->pfnEQFPostSeg2  = NULL;
          pInD->pfnEQFPostSegEx = NULL;
          pInD->pfnEQFPreUnSeg  = NULL;
          pInD->pfnEQFPreUnSeg2 = NULL;
          pInD->pfnEQFPostTM    = NULL;
          pInD->pfnEQFPostTM2   = NULL;
          pInD->pfnEQFCheckSeg  = NULL;     
          pInD->pfnEQFPostSegW  = NULL;
          pInD->pfnEQFPostSegWEx = NULL;
          pInD->pfnEQFPostTmW   = NULL;
          pInD->pfnEQFPreUnSegW = NULL;
          pInD->pfnEQFCheckSegW = NULL;
          pInD->pfnEQFCheckSegExW = NULL;
          pInD->pfnUpdateContext = NULL;
          pInD->pfnGetSegContext = NULL;
          pInD->pfnEQFCheckSegType = NULL;
         } /* endif */
       }
       else
       {
         fOK = FALSE;
       } /* endif */
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Open document memory if it is not the current one               */
   /* and TM related processing is requested                          */
   /*******************************************************************/
   if ( fOK )
   {
     PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;
     MemoryFactory *pFactory = MemoryFactory::getInstance();

     if ( (strcmp( pInD->szDocMemory, pInD->szCurMemory ) != 0 ) &&
          ( pTAInput->fInsertToTM || pTAInput->fInsertTMMatches ||
            pTAInput->fTMMatch ||
            pTAInput->fRedundCount ||
            pTAInput->fMachTrans ||
            pTAInput->fInsertNewMatch ) )
     {
       /***************************************************************/
       /* Close any open document memory database                     */
       /* and post memory changed message if memory may have been     */
       /* changed                                                     */
       /***************************************************************/
       if ( pLookupData->pDocMem != NULL)
       {
         pFactory->closeMemory( pLookupData->pDocMem );
         pLookupData->pDocMem = NULL;

         if ( pTAInput->fInsertToTM )
         {
           if ( pTAInput->hwndErrMsg != HWND_FUNCIF )
           {
              EqfSend2AllHandlers( WM_EQFN_PROPERTIESCHANGED,
                                 MP1FROMSHORT( PROP_CLASS_MEMORY ),
                                 MP2FROMP( pInD->szDocumentMemDb ) );
           } /* endif */
         } /* endif */
       } /* endif */

       /***************************************************************/
       /* open document memory database                               */
       /***************************************************************/
      if ( fOK )
      {
        int iRC = 0;

        pLookupData->pDocMem = pFactory->openMemory( NULL, pInD->szDocMemory, NONEXCLUSIVE, &iRC );
        if ( pLookupData->pDocMem == NULL )
        {
          fOK = FALSE;
          pFactory->showLastError( NULL, pInD->szDocMemory, NULL, pTAInput->hwndErrMsg );
        } /* endif */           
      } /* endif */
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* allocate TBDOC structure(s)                                     */
   /*******************************************************************/
   if ( fOK )
   {
     fOK = (USHORT)UtlAlloc( (PVOID *)&(pInD->pDoc), 0L,
                     (LONG)sizeof(TBDOCUMENT), ERROR_STORAGE );
     if ( fOK && pTAInput->fAutoJoin )
     {
       fOK = (USHORT)UtlAlloc( (PVOID *)&(pInD->pSourceDoc), 0L,
                       (LONG)sizeof(TBDOCUMENT), ERROR_STORAGE );
     } /* endif */
   } /* endif */

  ADDTIME( pTAInput->ulTimeOther );

   /*******************************************************************/
   /* Load the segmented source file into the document structure      */
   /*******************************************************************/
   if ( fOK )
   {
     SHORT sRc;

     pInD->pDoc->pQFTagTable = pInD->pLoadedQFTable;
     pInD->pDoc->pDocTagTable = pInD->pLoadedTable;

     sprintf( szFileName,
              PATHCATFILE,
              pTAInput->szSEGSOURCE_Path,
              pInD->pszCurSourceFile);
     strcpy( pInD->pDoc->szDocName, szFileName );
     strcpy( pInD->pDoc->szDocLongName,
             pTAInput->apszLongNames[pInD->usCurNumSourceFile-1] );

     // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. pInD->szDocSourceLang
     pInD->pDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pInD->szDocSourceLang);
     pInD->pDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pInD->szDocSourceLang);

#ifdef SESSIONLOG
     UtlLogWriteString( "Analysis: Loading document %s", pInD->pDoc->szDocLongName );
#endif
     sRc = EQFBFileReadExW( szFileName, pInD->pDoc, FILEREAD_FPOS | FILEREAD_SINGLETABLE | FILEREAD_METADATA );
     pInD->fMetaDataAdded = FALSE;                        // no additional metadata has been added so far

     ADDTIME( pTAInput->ulTimeFileRead );

     if ( sRc == NO_ERROR )
     {
       sRc = EQFBCntAllSrcWords( pInD->pDoc, pInD->pDoc, pInD->TolstControl.sLangID,
                                  pInD->pDoc->ulOemCodePage);
     } /* endif */

     ADDTIME( pTAInput->ulTimeWordCount );

     if ( (sRc == NO_ERROR) && pTAInput->fAutoJoin )
     {
       pInD->pSourceDoc->pQFTagTable = pInD->pLoadedQFTable;
       pInD->pSourceDoc->pDocTagTable = pInD->pLoadedTable;
       strcpy( pInD->pSourceDoc->szDocName, szFileName );
       strcpy( pInD->pSourceDoc->szDocLongName,
               pTAInput->apszLongNames[pInD->usCurNumSourceFile-1] );

       // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. pInD->szDocSourceLang
       pInD->pSourceDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pInD->szDocSourceLang);
       pInD->pSourceDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pInD->szDocSourceLang);
       sRc = EQFBFileReadExW( szFileName, pInD->pSourceDoc,
                             FILEREAD_SINGLETABLE );
#ifdef SESSIONLOG
        if ( sRc != NO_ERROR )
        {
          UtlLogWriteString( "Analysis:TASegOpenFiles: EQFBFileReadExW of document %s failed", szFileName );
        } /* endif */             
#endif
     } /* endif */

     ADDTIME( pTAInput->ulTimeFileRead );

     fOK = ( sRc == NO_ERROR );
   } /* endif */

   /*******************************************************************/
   /* if fInsertNewMatch requested than create file for new segments  */
   /*******************************************************************/
   if (fOK && pTAInput->fInsertNewMatch)
   {
     fOK = ( UtlMkDir( pTAInput->szSEGNEWMATCH_Path, 0L, TRUE ) == 0 );

     if ( fOK )
     {
       sprintf( szFileName,    // create full file name
                PATHCATFILE,
                pTAInput->szSEGNEWMATCH_Path,
                pInD->pszCurSourceFile);

       fOK = ( UtlBufOpen ( &(pInD->pCBNewMatchFile), szFileName,
                            BUFFERSIZE, FILE_CREATE, TRUE ) == NO_ERROR );

       /***************************************************************/
       /* Write start tag to file                                     */
       /***************************************************************/
       if ( fOK )
       {
         pInD->fNewMatchFileUsed = FALSE;        // no segments written yet
         fOK = (UtlBufWrite( pInD->pCBNewMatchFile, NTM_BEGIN_TAG,
                            (USHORT)strlen(NTM_BEGIN_TAG), TRUE ) == NO_ERROR);
       } /* endif */
     } /* endif */
   } /* endif */

   // open file for machine translation
   /* MTINTERFACE */
   if ( fOK && pTAInput->fMachTrans && !pInD->fMachTransFileStarted )
   {
     pInD->fMachTransFileUsed = FALSE;           // no segments written yet
     pInD->fMachTransFileStarted = TRUE;         // file has been started

     fOK = (USHORT)TAOpenMTFile( pTAInput, pInD->pszCurSourceFile );
     if ( !fOK ) fMsgShown = TRUE; // TAOpenMTFile does error handling
   } /* endif pTAInput->fMatchTrans */

   if (!fOK)
   {
      if ( fMsgShown )
      {
        pTAInput->fKill = TRUE; // go to stop processing
      }
      else if ( pInD->pDoc )
      {
        // allocation okay - so definitely something wrong with the file

         // opening of source or target file failed
         //pop up message box and ask user whether to continue
         PSZ pszDoc;

         pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
#ifdef SESSIONLOG
         UtlLogWriteString( "Analysis: Error opening segmented source or target document%s", pszDoc );
#endif
         usReturn = UtlErrorHwnd ( ERROR_TA_SOURCEFILE, MB_YESNO, 1, &pszDoc,
                                   EQF_QUERY, pTAInput->hwndErrMsg );

         pTAInput->fKill = (usReturn != MBID_YES);  // go to stop processing
      }
      else
      {
         pInD->fTerminate = TRUE;  // terminate
      } /* endif */
    }
    else
    {
    /*----------- Init memory areas and control data --------------------*/
       pInD->SegmentControl.ulSegmentcount = 1;
       pInD->ulSegNum = 1;
       pInD->SegmentControl.Seginfo = SEG;
       pInD->SegmentControl.sSegState = 1;
       pInD->SegmentControl.usColPos = 0;
       pInD->fSeglength = FALSE;
       pInD->uCurTextPos = 0;
       pInD->fAll = FALSE;
       pInD->fNotComplete = FALSE;           //  completely translated
       pInD->pRest = NULL;
   } /* endif */

   return (fOK);
} /* end  open files  */


/*----------------------------------------------------------------------------*\
| TASegBlockProc : process next segment of file                                |
\*----------------------------------------------------------------------------*/
USHORT TASegBlockProc ( HWND hwnd, PTAINPUT pTAInput )
   {
   BOOL       fOK = TRUE;              // processing flag
   PTBSEGMENT pSeg = NULL;             // pointer to segments
   LPSEG      LPSeg;                   // segment definition structure
   PTAINSTDATA pInD;                   // pointer to instance data
   ULONG       ulSegLength;             // Segment length

   /*******************************************************************/
   /* Initialization                                                  */
   /*******************************************************************/
   pInD = pTAInput->pInD ;  // store pointer to instance data temp.
   LPSeg.pszFileName = NULL;
   LPSeg.ulPosition  = 0L;
   LPSeg.usLength    = 0;

    pSeg = EQFBGetSegW( pInD->pDoc, pInD->ulSegNum );

    // update segment context if user exit supplies the EQFUPDATECONTEXT function
    if ( pSeg && pSeg->pDataW && pInD->pfnUpdateContext )
    {
      (pInD->pfnUpdateContext)( pSeg->pDataW, pInD->szContext );
    } /* endif */

    // update CRLF flag of segment
    if ( fOK && pSeg && (pSeg->pDataW != NULL) )
    {
      TACheckForCRLFTags( pInD->pCrlfInfoForMarkup, pSeg->pDataW, &(pInD->fCrlfFlag) );
    } /* end */       

   /*******************************************************************/
   /* Do list and TM handling for current segment if the segment      */
   /* contains translatable data                                      */
   /*******************************************************************/
   if ( fOK && pSeg && (pSeg->pDataW != NULL) && (pSeg->qStatus != QF_NOP) )
   {
     ulSegLength = UTF16strlenCHAR(pSeg->pDataW);

     /*****************************************************************/
     /* List processing                                               */
     /*****************************************************************/
     if ( pTAInput->fNTLProcessing )
     {
       ADDTIME( pTAInput->ulTimeOther );

       LPSeg.usLength    = pSeg->usOrgLength;
       LPSeg.pszFileName = pInD->pszCurSourceFile;
       LPSeg.ulPosition  = *((PULONG)&(pSeg->pusBPET));

       fOK = LPSegProc( hwnd,          // window handle
                       (PLPDATA) pInD->pLPDATA, // ptr List data
                        pTAInput,      // ptr input struct.
                        pSeg->pDataW,   // segment of text
                        &LPSeg );      // segment def. struc

       ADDTIME( pTAInput->ulTimeListProcessing );

   } /* endif */

     /*****************************************************************/
     /* Translation Memory Processing                                 */
     /*****************************************************************/
     if ( fOK   &&
         ( pTAInput->fInsertToTM || pTAInput->fInsertTMMatches || pTAInput->fTMMatch ||
           pTAInput->fRedundCount || pTAInput->fMachTrans || pTAInput->fInsertNewMatch) )
     {
        fOK = TASegTMProc( pTAInput, pSeg, pInD->ulSegNum, FALSE, FALSE );
     } /* endif */
   }
   else if ( fOK && pSeg && (pSeg->pDataW != NULL) )
   {
     if ( !pSeg->SegFlags.NoCount )    // ignore nocount segments
     {
       pInD->ulTotalSegs++;
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Update slider                                                   */
   /* For autojoin mode allow only values from 0% to 49%              */
   /*******************************************************************/
   if ( fOK )
   {
     LONG   lNumber;                             // tick number

     lNumber = (LONG)pInD->ulSegNum * (LONG)(SLIDER_INCREMENT - 1)
                                      / (LONG)pInD->pDoc->ulMaxSeg;

     if ( pTAInput->fAutoJoin )
     {
       lNumber = lNumber / 2L;
     } /* endif */

     pTAInput->sSliderPos = (SHORT)lNumber;

     if ( pTAInput->hwndErrMsg != HWND_FUNCIF )
     {
       WinSendMsg( pTAInput->hwndProcWin,
                   WM_EQF_UPDATESLIDER,
                   MP1FROMSHORT((USHORT)lNumber),
                   MP2FROMP(NULL) );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Prepare for next call                                           */
   /*******************************************************************/
   if ( pSeg != NULL )
   {
     pInD->ulSegNum++;
     if ( pInD->ulSegNum > pInD->pDoc->ulMaxSeg )
     {
       pInD->fAll = TRUE;
     } /* endif */
   }
   else
   {
     pInD->fAll = TRUE;
   } /* endif */

   return ((USHORT)fOK);
}  /* end of TASegBlockProc */

// max segments checked for autojoin (large values decrease aut.subst. speed!)
#define MAX_AUTOJOIN_SEGS 10

// segment information required by autojoin
typedef struct _AUTOJOINSEGINFO
{
  PTBSEGMENT   pNextSeg;
  USHORT       usNextFuzzyness;               // fuzzyness of joined segments
  MATCHTYPE    NextMatchType;                 // match type of joined segments
  USHORT       usNextSegWords;                // words of joined segments
  CHAR_W       szJoinedData[MAX_SEGMENT_SIZE+1]; // buffer for joinded segment data
  USHORT       qStatus;                       // status of segment (QF_...)
} AUTOJOINSEGINFO, *PAUTOJOINSEGINFO;

/*----------------------------------------------------------------------------*\
| TASegAutoJoin  : try to autojoin segment                                     |
\*----------------------------------------------------------------------------*/
USHORT TASegAutoJoin( HWND hwnd, PTAINPUT pTAInput )
{
   BOOL       fOK = TRUE;              // processing flag
   PTBSEGMENT pSeg = NULL;             // pointer to segments
   PTBSEGMENT pSSeg = NULL;            // pointer to current source segment
   PTAINSTDATA pInD;                   // pointer to instance data
   MATCHTYPE CurMatchType;             // match type of joined segments
   USHORT usCurFuzzyness;              // fuzzyness of joined segments
   USHORT usCurSegWords;               // words of joined segments
   BOOL   fContainsTagging;            // TRUE = segment contains inline tags
   ULONG  ulNextSegs;                  // number of segments in table
   PAUTOJOINSEGINFO pSegTable = NULL;  // segment table for join tests

   hwnd;
   /*******************************************************************/
   /* Initialization                                                  */
   /*******************************************************************/
   pInD = pTAInput->pInD ;  // store pointer to instance data temp.

   /*******************************************************************/
   /* Get current segment                                             */
   /*******************************************************************/
   pSeg = EQFBGetSegW( pInD->pDoc, pInD->ulSegNum );

   // save copy of segment source (required later on in MT processing)
   if ( pSeg && pSeg->pDataW )
   {
     wcscpy( pTAInput->szOrgSegSource, pSeg->pDataW );
   }  /* endif */

   // update segment context if user exit supplies the EQFUPDATECONTEXT function
   if ( pSeg && pSeg->pDataW && pInD->pfnUpdateContext )
   {
     (pInD->pfnUpdateContext)( pSeg->pDataW, pInD->szContext );
   }  /* endif */

   // allocate segment info table for joined segments
   fOK = UtlAlloc( (PVOID *)&pSegTable, 0L, (sizeof(AUTOJOINSEGINFO)*MAX_AUTOJOIN_SEGS), ERROR_STORAGE );

   /*******************************************************************/
   /* Try to autojoin segment if it has been marked by TASegTMProc    */
   /*******************************************************************/
   if ( fOK && pSeg && pSeg->SegFlags.Marked )
   {
     BOOL fAutoJoined = FALSE;         // TRUE is segment has been auto-joined
     BOOL fNextSegsComplete = FALSE;   // no more next segments to follow flag
     ULONG ulCurSegLength = pSeg->usLength;

     // save match type and fuzzyness of involved segment
     CurMatchType    = (MATCHTYPE)pSeg->ulShrinkLen;
     usCurFuzzyness  = pSeg->usOrgLength;
     usCurSegWords   = pSeg->usSrcWords;
     fContainsTagging = pSeg->SegFlags.Changed;

     // get up to MAX_AUTOJOIN_SEGS next segments into our table
     ulNextSegs = 0;
     do
     {
       PTBSEGMENT pNextSeg = EQFBGetSegW( pInD->pDoc, pInD->ulSegNum + ulNextSegs + 1);
       if ( pNextSeg == NULL )
       {
         fNextSegsComplete = TRUE;     // not able to retrieve next segments
       }
       else if ( (pNextSeg->qStatus != QF_XLATED) &&
                 ((ulCurSegLength + pNextSeg->usLength) < MAX_SEGMENT_SIZE) )
       {
         // add next segment to our table and prepare for next segment
         pSegTable[ulNextSegs].pNextSeg = pNextSeg;
         pSegTable[ulNextSegs].NextMatchType = (MATCHTYPE)pNextSeg->ulShrinkLen;
         pSegTable[ulNextSegs].usNextFuzzyness = pNextSeg->usOrgLength;
         pSegTable[ulNextSegs].usNextSegWords  = pNextSeg->usSrcWords;
         pSegTable[ulNextSegs].qStatus         = pNextSeg->qStatus;
         if ( ulNextSegs == 0 )
         {
           // add text of start segment
           UTF16strcpy( pSegTable[ulNextSegs].szJoinedData, pSeg->pDataW );
         }
         else
         {
           // add text of previous next segment
           UTF16strcpy( pSegTable[ulNextSegs].szJoinedData, pSegTable[ulNextSegs-1].szJoinedData );
         } /* endif */
         UTF16strcat( pSegTable[ulNextSegs].szJoinedData, pNextSeg->pDataW );
         ulCurSegLength = UTF16strlenCHAR( pSegTable[ulNextSegs].szJoinedData ) + 1;
         ulNextSegs++;
       }
       else
       {
         fNextSegsComplete = TRUE;     // no more join candidats to follow
       } /* endif */
     } while ( !fNextSegsComplete && (ulNextSegs < MAX_AUTOJOIN_SEGS ) &&
               ((pInD->ulSegNum + ulNextSegs) < pInD->pDoc->ulMaxSeg) );

     // test for joined segment match
     if ( ulNextSegs != 0 )
     {
       // GQ: changed logic to start with shortest possible string to avoid problem P400814

       PSZ_W         pTempSegData = NULL;
       PSZ_W         pOldDataPtr;
//       ULONG       ulTestSeg = ulNextSegs;          // start with longest entry
       ULONG       ulTestSeg = 0;                     // start with shortest entry
       ULONG      ulNewLen;

       // try to get exact matchs for joined segments in segtable, start with
       // longest entry
       pOldDataPtr = pSeg->pDataW;
       do
       {
         // join segment temporarly
         // allocate always with max_segment_size to avoid problems in functions
         // assuming that always a larger buffer is available....
         ulNewLen = UTF16strlenCHAR( pSegTable[ulTestSeg].szJoinedData ) + 2;
         fOK = UtlAlloc( (PVOID *)&pTempSegData, 0L,
//                         (LONG) max( ulNewLen * sizeof(CHAR_W), MIN_ALLOC ), ERROR_STORAGE );
                        (LONG)( (MAX_SEGMENT_SIZE + 4) * sizeof(CHAR_W)), ERROR_STORAGE );
         if ( fOK )
         {
           pSeg->pDataW = pTempSegData;
           UTF16strcpy( pTempSegData, pSegTable[ulTestSeg].szJoinedData );
           pSeg->usLength = (USHORT)ulNewLen;
         } /* endif */

         // Call 'normal' TM processing for segment
         if ( fOK )
         {
           fOK = TASegTMProc( pTAInput, pSeg, pInD->ulSegNum, TRUE, FALSE );
           pTempSegData = pSeg->pDataW; // pointer may have been changed by autosubstitution
         } /* endif */

         // clear temp data if no match
         if ( (pSeg->qStatus != QF_XLATED) && (pTempSegData != NULL) )
         {
           UtlAlloc( (PVOID *)&pTempSegData, 0L, 0L, NOMSG );
           pTempSegData = NULL;
         } /* endif */

         // go to next table entry
         ulTestSeg++;


       } while ( fOK && (pSeg->qStatus != QF_XLATED) && (ulTestSeg < ulNextSegs) );

       /*************************************************************/
       /* If segment has been replaced by an exact match, join the  */
       /* segment permanently and skip the next segment(s)          */
       /*************************************************************/
       if ( fOK )
       {
         if ( pSeg->qStatus == QF_XLATED )
         {
           ULONG ulNum;                // index for segment table processing

           fAutoJoined = TRUE;
           ulNextSegs = ulTestSeg;   // remember how much next segments have been joinded

           UtlAlloc( (PVOID *)&pOldDataPtr, 0L, 0L, NOMSG );

           // adjust the memory match counts for the current and next
           // segment(s) as the match types of these segments have
           // changed
           if ( pTAInput->fTMMatch || pTAInput->fInsertTMMatches )
           {
#ifdef LOGMATCHES
             fwprintf( hMatchLogFile, L"Correcting joined segments (seg %ld to seg %ld)\r\n",
                      pInD->ulSegNum, pInD->ulSegNum + ulNextSegs );
#endif
             TACountMatch( pInD, CurMatchType, usCurFuzzyness,
                           usCurSegWords * (-1), -1, TRUE );
             for ( ulNum = 0; ulNum < ulNextSegs; ulNum++ )
             {
               // Attention: segments containing non-translatable data have not
               // been counted yet, so do not try to correct their TM match count
               pInD->ulSegNum += ulNum + 1;
               if ( pSegTable[ulNum].qStatus != QF_NOP  )
               {
                 TACountMatch( pInD, pSegTable[ulNum].NextMatchType,
                               pSegTable[ulNum].usNextFuzzyness,
                               pSegTable[ulNum].usNextSegWords * (-1), -1, TRUE );
               }
               else
               {
#ifdef LOGMATCHES
                 fwprintf( hMatchLogFile, L"Segment %lu is a QF_NOP and is ignored\r\n",
                          pInD->ulSegNum );
#endif
               } /* endif */
               pInD->ulSegNum -= ulNum + 1;
             } /* endfor */
#ifdef LOGMATCHES
             fwprintf( hMatchLogFile, L"End of correcting joined segments\r\n" );
#endif
           } /* endif */

           pSeg->usLength = (USHORT)(UTF16strlenCHAR( pTempSegData ) + 1);
           pSeg->SegFlags.JoinStart  = TRUE;
           pSeg->SegFlags.Typed      = FALSE;
           pSeg->SegFlags.Copied     = TRUE;

           for ( ulNum = 0; ulNum < ulNextSegs; ulNum++ )
           {
             pSegTable[ulNum].pNextSeg->SegFlags.Joined = TRUE;
             pSegTable[ulNum].pNextSeg->SegFlags.Marked = FALSE;
           } /* endfor */

           pTempSegData = NULL; // avoid free of data area

           // update segmented source
           {
             PTBSEGMENT pSNextSeg;
             pSSeg = EQFBGetSegW( pInD->pSourceDoc, pInD->ulSegNum );

             fOK = UtlAlloc( (PVOID *)&pTempSegData, 0L,
                             max( ulNewLen* sizeof(CHAR_W), MIN_ALLOC ), ERROR_STORAGE );
             if ( fOK )
             {
               UTF16strcpy( pTempSegData, pSegTable[ulNextSegs-1].szJoinedData );
               // save copy of segment source (required later on in MT processing)
               wcscpy( pTAInput->szOrgSegSource, pTempSegData );
			   
               UtlAlloc( (PVOID *)&(pSSeg->pDataW), 0L, 0L, NOMSG );
               pSSeg->pDataW = pTempSegData;
               pSSeg->usLength = (USHORT)ulNewLen;
               pSSeg->SegFlags.JoinStart = TRUE;
               pTempSegData = NULL;
               pTAInput->fAutoJoinDone = TRUE;

               for ( ulNum = 0; ulNum < ulNextSegs; ulNum++ )
               {
                 pSNextSeg = EQFBGetSegW( pInD->pSourceDoc, pInD->ulSegNum + ulNum + 1 );
                 pSNextSeg->SegFlags.Joined = TRUE;
               } /* endfor */

               // count source words of joined segment
               {
                 ULONG ulSrcWords  = 0L;
                 ULONG ulSrcMarkUp = 0L;
                 USHORT usRc;

                 usRc = EQFBWordCntPerSeg(
                          (PLOADEDTABLE)pInD->pDoc->pDocTagTable,
                          (PTOKENENTRY) pInD->pDoc->pTokBuf,
                          pSSeg->pDataW,
                          pInD->TolstControl.sLangID,
                          &ulSrcWords, &ulSrcMarkUp,
                          pInD->TolstControl.ulOemCP);
                 if (!usRc)
                 {
                   // some way to fool the compiler
                   usCurSegWords = pSeg->usSrcWords = (USHORT) ulSrcWords;
                   if ( pTAInput->fTMMatch || pTAInput->fInsertTMMatches )
                   {
                     TACountMatch( pInD, (MATCHTYPE) pSeg->ulShrinkLen,
                                   100, ulSrcWords,
                                   1, TRUE );
                   } /* endif */
                 } /* endif */
               }
             } /* endif */
           }

           pInD->ulSegNum += ulNextSegs;     // skip next (=joined) segments

           /*********************************************************/
           /* Correct segment counter (the segment counters have    */
           /* been already updated during normal mode of            */
           /* TASegTMProc)                                          */
           /* The current segment has been counted as notreplace.   */
           /* The next segment has been counted as notreplaced      */
           /* if it is an translatable one.                         */
           /*********************************************************/
           if ( !pSeg->SegFlags.NoCount )    // ignore nocount segments
           {
             pInD->ulSegsReplaced++;
             pInD->ulSegsNotReplaced--;
           } /* endif */

           for ( ulNum = 0; ulNum < ulNextSegs; ulNum++ )
           {
             USHORT usStatus = pSegTable[ulNum].pNextSeg->qStatus;
             if ( (usStatus == QF_TOBE) || (usStatus == QF_ATTR) )
             {
               if ( (pSegTable[ulNum].pNextSeg->pDataW == NULL) || (pSegTable[ulNum].pNextSeg->pDataW[0] == EOS) )
               {
                 // ignore empty segments
               }
               else if ( !(pSegTable[ulNum].pNextSeg->SegFlags.NoCount) )    // ignore nocount segments
               {
                 pInD->ulSegsReplaced++;
                 pInD->ulSegsNotReplaced--;
                 pInD->ulReplacedWords += (ULONG)(pSegTable[ulNum].pNextSeg->usSrcWords);
               } /* endif */
             } /* endif */
           } /* endfor */
         }
         else
         {
           // restore old data pointer
           pSeg->pDataW = pOldDataPtr;
           if ( pTempSegData ) UtlAlloc( (PVOID *)&pTempSegData, 0L, 0L, NOMSG );
           pTempSegData = NULL;
         } /* endif */
       } /* endif */

       /*************************************************************/
       /* Cleanup                                                   */
       /*************************************************************/
       if ( pTempSegData != NULL )
       {
         UtlAlloc( (PVOID *)&pTempSegData, 0L, 0L, NOMSG );
       } /* endif */
     } /* endif */

     /*****************************************************************/
     /* Reset marked flag in any case                                 */
     /*****************************************************************/
     pSeg->SegFlags.Marked = FALSE;

     // do redundancy processing when segment has not been processed yet
     if ( fOK && pTAInput->fRedundCount )
     {
       // do redundancy processing for the segment
       fOK = TASegTMRedundCount( pTAInput, pInD, pSeg, fAutoJoined ? EXACTONE_MATCH : NO_MATCH );
     } /* endif */        

     // write to SNOMATCH file if segment has not been joined
     if ( fOK && pTAInput->fInsertNewMatch && !fAutoJoined )
     {
        if ( (CurMatchType == MACHINE_MATCH) && (usCurFuzzyness >= 95) )
        {
          // do not write segment to SNOMATCH file, machine match exists
        }
        else if ( !pSeg->SegFlags.NoCount )
        {
          ULONG  ulLength = UTF16strlenCHAR(pSeg->pDataW);
          fOK =  WriteTMSegment( pTAInput, ulLength, pSeg->pDataW,
                                 (pInD->pCBNewMatchFile), pInD->ulSegNum,
                                 WRITETMSEGMENT_UTF8 );
          pInD->fNewMatchFileUsed = TRUE;
        } /* endif */


        if ( !fOK )
        {
           PSZ pszDoc;
           USHORT usMBReturn;
           pszDoc = pTAInput->apszLongNames[pInD->usCurNumSourceFile-1];
           usMBReturn = UtlErrorHwnd ( ERROR_TA_SAVE_SEGFILE, MB_YESNO, 1,
                                 &pszDoc, EQF_QUERY, pTAInput->hwndErrMsg );
           pTAInput->fKill = (usMBReturn != MBID_YES);
        } /* endif */
     } /* endif */


     // write to SNOMATCH file if segment has not been joined
     if ( fOK && pTAInput->fMachTrans )
     {
       // adjust match type when segment has beeb substituded 
       if (pSeg->qStatus == QF_XLATED )
       {
         CurMatchType = EXACTONE_MATCH;
         usCurFuzzyness = 100;
       } /* endif */       

       // GQ 2015/06/10: when the segment has not been auto-joined, the contents of the BestProposal vector is empty even if there may be
       //                proposals for the non-joined segment, to correct this we have to call TMSegTMProc for the segment again
       if ( pSeg->qStatus != QF_XLATED )
       {
         TASegTMProc( pTAInput, pSeg, pInD->ulSegNum, TRUE, TRUE );
       } /* endif */

       fOK = TAMachTransAddSeg( pTAInput, usCurSegWords,
                                fContainsTagging, CurMatchType,
                                usCurFuzzyness, pSeg->ulSegNum, (pSSeg != NULL) ? pSSeg->pDataW : pSeg->pDataW, pSeg );

     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Update slider (use range 50% to 99%)                            */
   /*******************************************************************/
   if ( fOK )
   {
     LONG   lNumber;                             // tick number

     lNumber = (LONG)(pInD->ulSegNum * ( SLIDER_INCREMENT - 1 )
                                      / (LONG)pInD->pDoc->ulMaxSeg );
     lNumber = (lNumber / 2L) + 50L;

     pTAInput->sSliderPos = (SHORT)lNumber;

     if ( pTAInput->hwndErrMsg != HWND_FUNCIF )
     {
       WinSendMsg( pTAInput->hwndProcWin,
                   WM_EQF_UPDATESLIDER,
                   MP1FROMSHORT((USHORT)lNumber),
                   MP2FROMP(NULL) );
     } /* endif */
   } /* endif */

   /*******************************************************************/
   /* Prepare for next call                                           */
   /*******************************************************************/
   if ( pSeg != NULL )
   {
     pInD->ulSegNum++;
     if ( pInD->ulSegNum > pInD->pDoc->ulMaxSeg )
     {
       pInD->fAll = TRUE;
     } /* endif */
   }
   else
   {
     pInD->fAll = TRUE;
   } /* endif */

   // cleanup
   if ( pSegTable ) UtlAlloc( (PVOID *)&pSegTable, 0L, 0L, NOMSG );

   return ((USHORT)fOK);
}  /* end of TASegBlockProc */


/*----------------------------------------------------------------------------*\
| TASegTMProc                                                                  |
| Purpose : perform Translation memory processing                              |
|           the  source segment received is searched on the input memories     |
|           and all similar segments are copied to the output memory           |
|           if an exact match is found, the translation is copied to the       |
|           output instead of the source.                                      |
| input   : structure pTAIn for work areas, handles to TM                      |
|           pszSegment source segment to process                               |
|           usSegNum   current segment number                                  |
|           SegXlat  type of segment (determine the type of tag)               |
|                                                                              |
| output  : the output memory is updated; the source segment or his            |
|           translation is written on output                                   |
|           if the operation is succesful, return TRUE, else FALSE             |
|                                                                              |
| called by: TASegSegmentation                                                 |
|                                                                              |
| calls    : GetBestTMMatch, MemReadWriteSegment, GetBestTMMatch,              |
|            TmExtract, TASegWrite                                             |
|                                                                              |
\*----------------------------------------------------------------------------*/
static USHORT TASegTMProc
( 
  PTAINPUT pTAIn,      // local data memory area
  PTBSEGMENT pSeg,     // segment data
  ULONG ulSegNum,     // segment number
  BOOL   fAutoJoinMode, // are we in auto-join mode???
  BOOL fOnlyLookup      // do only the memory lookup but skip counting and other processing  
) 
{
   BOOL fOK= TRUE;                     // processing flag
   ULONG  ulLength;                    // length of current segment

   PTAINSTDATA pInD;                    // pointer to instance data
   USHORT   usReturn;                     // return from TM calls
   PSZ_W    pszMatch = NULL;            // pointer to tm exact match
   BOOL     fTM;                        // insert matches to TM
   BOOL     fDoc;                       // insert match to document
   BOOL     fFile;                      // insert new match to file
   BOOL     fExactMatch = FALSE;        // exact tm match found?
   BOOL     fAnyMatch = FALSE;          // any matches found?
   TIME_L   tStamp;
   BOOL     fWriteSeg;
   BOOL     fTMMatch;                   // count TM matches
   BOOL     fMachTrans;                 // send no-matches to MT
   BOOL     fMatchList;                 // create match list flag
   BOOL     fRedundCount;               // redundancy counting
   MATCHTYPE MatchType;                 // type of match found
   ULONG     ulSrcWords  = 0L;          // number of words in source segment
   ULONG     ulSrcMarkup = 0L;          // number of tags in source segment
   USHORT    usFuzzyness = 0;             // fuzzyness of proposal
   ULONG     ulTgtOemCodePage = 0L;

   PSZ_W     pszNewTargetW = NULL;    //buffer for target

   // save copy of segment source (required later on in MT processing)
   if ( pSeg && pSeg->pDataW )
   {
     wcscpy( pTAIn->szOrgSegSource, pSeg->pDataW );
   }  /* endif */



#ifdef TA_SEGMENTLOGGING
   // segment logging for debugging only
   {
     static char szSegBuffer[4096];
     int iLen = UTF16strlenCHAR( pSeg->pDataW );
     FILE *hLog = NULL;
     
     UtlMakeEQFPath( szSegBuffer, NULC, LOG_PATH, NULL );
     UtlMkDir( szSegBuffer, 0, FALSE );
     strcat( szSegBuffer, "\\EQFSEG.LOG" );
     hLog = fopen( szSegBuffer, "a" );
     if ( hLog )
     {
       //Unicode2AnsiBuf( pSeg->pDataW, szSegBuffer, iLen, sizeof(szSegBuffer), 0L );
       fprintf( hLog, "Seg %4.4ld: L=%4.4ld, Data=\"%S\"\n", pSeg->ulSegNum, iLen, pSeg->pDataW/*szSegBuffer*/ );
       fclose( hLog );
     } /* endif */
   }
#endif

   memset(&MatchType, 0, sizeof(MatchType));

   pInD = (PTAINSTDATA) pTAIn->pInD;
   PANALYSISMEMLOOKUPDATA pLookup = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;
   pInD->ulSegNum = ulSegNum;          // set segment number
   UtlTime( &tStamp );

   fTM   = pTAIn->fInsertToTM;
   fDoc  = pTAIn->fInsertTMMatches;
   fFile = pTAIn->fInsertNewMatch;
   fMachTrans = pTAIn->fMachTrans;
   fMatchList = pTAIn->fMatchList;
   fTMMatch = pTAIn->fTMMatch;

   if ( pTAIn->fRedundCount )
   {
     fTMMatch = TRUE;                  // always perform match counting with redundancy counting
     fRedundCount = TRUE;
   }
   else
   {
     fRedundCount = FALSE;
   } /* endif */

   if ( !fOnlyLookup && fTM)  // insert found matches in the selected TM's
   {
     ADDTIME( pTAIn->ulTimeOther );

	   fOK = TAInsertToTM(pTAIn, pInD, pSeg, &fExactMatch );

     ADDTIME( pTAIn->ulTimeInsertIntoTM );

   } /* endif */

   fExactMatch = FALSE;                // no exact match yet
   fWriteSeg = FALSE;                  // no Segment already written
                                       // to starget

   //-- if insert segments to document, get match from TM
   //   or insert not found matches to file
   if ((fDoc || fFile || fTMMatch || fRedundCount || fMachTrans ) && fOK)
   {  // count number of source words (is required for word counting)
     if ( fDoc || fTMMatch || fRedundCount || fMachTrans )
     {
       if (!pSeg->SegFlags.NoCount )
       {
         USHORT usRc;

         ADDTIME( pTAIn->ulTimeOther );

         usRc = EQFBWordCntPerSeg( (PLOADEDTABLE)pInD->pDoc->pDocTagTable,
                                 (PTOKENENTRY) pInD->pDoc->pTokBuf,
                                 pSeg->pDataW,
                                 pInD->TolstControl.sLangID,
                                 &ulSrcWords, &ulSrcMarkup,
                                 pInD->TolstControl.ulOemCP);

         ADDTIME( pTAIn->ulTimeWordCount );


         // remember if segment contains inline tagging (use Changed flag for
         // this purpose)
         pSeg->SegFlags.Changed = (USHORT)(ulSrcMarkup != 0);

         if (!usRc && !fAutoJoinMode )
         {
           pSeg->usSrcWords = (USHORT) ulSrcWords;
         } /* endif */
       }
       else
       {
          pSeg->usSrcWords = 0;
       }
     } /* endif */

     // setup list of memories for lookup
     pLookup->SearchMemoryDBs.clear();
     pLookup->SearchMemoryDBs.push_back( pLookup->pDocMem );
     for ( int i = 0; i < (int)pLookup->ROMemoryDBs.size(); i++ )
     {
       pLookup->SearchMemoryDBs.push_back( pLookup->ROMemoryDBs[i] );
     } /* end */        
     ulTgtOemCodePage = GetLangCodePage(OEM_CP, pInD->szDocTargetLang);

     // get exact match from translation memory if available
     ADDTIME( pTAIn->ulTimeOther );


     // get segment context if not provided yet
     if ( (pInD->pfnUpdateContext == NULL) && (pInD->pfnGetSegContext != NULL) )
     {
      PSZ_W pszPrevSeg;              // ptr to text of previous segment (UTF16)
      PSZ_W pszNextSeg;              // ptr to text of next segment (UTF16)
      PTBSEGMENT pSeg;

      pSeg = EQFBGetSegW( pInD->pDoc, (ULONG)(pInD->ulSegNum-1) );
      pszPrevSeg = (pSeg) ? pSeg->pDataW : NULL;
      pSeg = EQFBGetSegW( pInD->pDoc, (ULONG)(pInD->ulSegNum+1) );
      pszNextSeg = (pSeg) ? pSeg->pDataW : NULL;
      memset( pInD->szContext, 0, sizeof(pInD->szContext) );
      (pInD->pfnGetSegContext)( pSeg->pDataW, pszPrevSeg, pszNextSeg, pInD->szContext, (LONG)(pInD->pDoc), pInD->ulSegNum );

     } /* endif */        

     usReturn = GetExactTMMatch(pTAIn,
                          pSeg->pDataW,
                          pLookup->SearchMemoryDBs,
                          &fExactMatch,
                          &fAnyMatch,
                          MAX_NUM_MATCHES, // get best three matches ...
                          &MatchType,
                          &usFuzzyness,
                          pSeg->pvMetadata );

     ADDTIME( pTAIn->ulTimeGetMatch );

     fOK = (usReturn == NO_ERROR);

     // remember match info for usage in autojoin mode
     pTAIn->usLastFuzzyness = usFuzzyness;
     pTAIn->iLastMatchType  = MatchType;
     pTAIn->ulLastSrcWords  = ulSrcWords;

     if (!fOK)   // TM error: terminate analysis process
     {
        pInD->fTerminate = TRUE;
        // no 'organize required' message for in-use conditions
        if ( usReturn != BTREE_IN_USE )
        {
          UtlErrorHwnd( ERROR_TA_TMERROR, MB_CANCEL, 0, NULL,
                        EQF_ERROR, pTAIn->hwndErrMsg );
        } /* endif */
     } /* endif */

     // get match state
     if ( fOK && (fDoc || fTMMatch || fMachTrans || fFile || fMatchList) )
     {
       // check for MT matches
       if ( (MatchType == EXACTEXACT_MATCH) ||
            (MatchType == EXACTONE_MATCH)   ||
            (MatchType == EXACTMORE_MATCH) )
       {
         if ( pLookup->BestProposals[0]->getType() == OtmProposal::eptMachine )
         {
           MatchType = MACHINE_MATCH;

           // GQ 2004/08/04
           // always reset fExactMatch for machine matches
           if ( fExactMatch )
           fExactMatch = FALSE;
         } 
         else if ( (pLookup->BestProposals[0]->getType() == OtmProposal::eptGlobalMemory) ||
		           (pLookup->BestProposals[0]->getType() == OtmProposal::eptGlobalMemoryStar) )
         {
           MatchType = GLOBMEM_MATCH;
           fExactMatch = FALSE;
         } /* endif */
       } /* endif */

       TAAdjustMatchType( &MatchType, usFuzzyness, ulSrcWords );

       if ( fDoc || fTMMatch )
       {
         // add word count values

         // GQ 2015/04/27: match counting is now done after auto-substitution

         // remember type of match and fuzzyness in segment structure (we use
         // the fields usShrinkLen and usOrgLength for this purpose) so we are
         // able to correct the match count information when the segment is
         // autojoined later on
         pSeg->ulShrinkLen = MatchType;
         pSeg->usOrgLength = usFuzzyness;
       } /* endif */
     } /* endif */

     // check if exact match is available, if so copy it
     if ( !fOnlyLookup && fOK)
     {
        if ( fDoc )
        {
          BOOL           fAllowReplace = fExactMatch;
		      BOOL           fReplaceSeg   = TRUE;
		      BOOL           fChanged      = FALSE;
		      BOOL           fSourceIsEqual = FALSE;

#ifdef LOGMATCHES
          {
            USHORT usI = 0;
            fwprintf( hMatchLogFile, L"Looking up segment %lu\r\n", pSeg->ulSegNum );
            TALogSegData( hMatchLogFile, L"Source\r\n>>>%s<<<\r\n", pSeg->pDataW );
            int iMatches = OtmProposal::getNumOfProposals( pLookup->BestProposals );
            fwprintf( hMatchLogFile, L"found %ld matches\r\n", iMatches );
            for( int i = 0; i < iMatches; i++ )
            {
              pLookup->BestProposals[i]->getDocName( pLookup->szNameBuffer, sizeof(pLookup->szNameBuffer) );
              pLookup->BestProposals[i]->getSource( pLookup->szSegmentBuffer, MAX_SEGMENT_SIZE );
              fwprintf( hMatchLogFile, L"Match %u, MatchLevel=%ld Doc=%S Seg=%ld\r\n", usI, pLookup->BestProposals[i]->getFuzziness(),
                pLookup->szNameBuffer, pLookup->BestProposals[i]->getSegmentNum() );
              TALogSegData( hMatchLogFile, L"MatchSource\r\n>>>%s<<<\r\n", pLookup->szSegmentBuffer );
            } /* endfor */
          }
#endif

          // check for equal source/target when flag is set
          if ( pTAIn->fIgnoreIdentical )
          {
            if ( pLookup->BestProposals[0]->isSourceAndTargetEqual() )
            {
              fAllowReplace = FALSE;
            } /* endif */
          } /* endif */

          if ( fAllowReplace )
          {
            pLookup->BestProposals[0]->getSource( pLookup->szSegmentBuffer, MAX_SEGMENT_SIZE );
            fSourceIsEqual = UTF16strcmp( pLookup->szSegmentBuffer, pSeg->pDataW ) == 0;
            pLookup->BestProposals[0]->getTarget( pLookup->szSegmentBuffer, MAX_SEGMENT_SIZE );
            pszMatch = pLookup->szSegmentBuffer;

             /***********************************************************/
             /* check if we have to do an additional checking for tag   */
             /* changes, if so call it but do not allow for message     */
             /* pop up.                                                 */
             /***********************************************************/

             fReplaceSeg = TAHandleCheckSegExit(pInD, pSeg, pszMatch, ulTgtOemCodePage, &fOK, pTAIn->hwndErrMsg);
             if ( fOK && fReplaceSeg )
             {
               fOK = TAHandleAllAdjustOptions(pTAIn, pSeg, pszMatch, &pszNewTargetW, &fChanged );
		         }
		         if ( fOK && fReplaceSeg )
		         {  // new for P018345
		            // if LF different and #ofLF != 0, do not allow replace
		            // if markup table with reflow=NO
		            if (pInD->pLoadedTable && (pInD->pLoadedTable->fReflow == TAGREFLOW_NO))
		            {
		              fAllowReplace = TACheckAndAdaptLFChanges(pSeg->pDataW, ( fChanged ) ? pszNewTargetW : pszMatch, FALSE);
			          }
		         }

             // GQ 2017/02/06: change match type if an exact match could not be used for automatic substitution
             if ( !fAllowReplace || !fReplaceSeg )
             {
               if ( (MatchType == EXACTONE_MATCH) || (MatchType == EXACTEXACT_MATCH) )
               {
                 MatchType = EXACTMORE_MATCH;
                 pSeg->ulShrinkLen = EXACTMORE_MATCH;
               }
             }
  	      }

          if (fOK && fAllowReplace )
          {
             // indicate that TM match was included
             if ( fOK && fReplaceSeg )
             {
               pInD->fTMInserted = TRUE;
               if ( !pSeg->SegFlags.NoCount )    // ignore nocount segments
               {
                 pInD->ulSegsReplaced++;
                 pInD->ulReplacedWords += (ULONG)(pSeg->usSrcWords);
               } /* endif */

               // GQ 2015/05/27: change match type of extact(2) matches when match has been used for autosubstitution
               if ( MatchType == EXACTMORE_MATCH )
               {
                 MatchType = EXACTONE_MATCH;
                 pSeg->ulShrinkLen = EXACTONE_MATCH;
               }
             }
             else
             {
                if ( !fAutoJoinMode )
                {
                  pInD->fNotComplete = TRUE;
                  if ( !pSeg->SegFlags.NoCount )    // ignore nocount segments
                  {
                    pInD->ulSegsNotReplaced++;
                  } /* endif */
                }
                else
                {
                  if ( !fReplaceSeg )
                  {
                    pSeg->SegFlags.Marked = TRUE;
                  } /* endif */
                } /* endif */
              } /* endif */
              if ( !pSeg->SegFlags.NoCount )    // ignore nocount segments
              {
                pInD->ulTotalSegs++;
              } /* endif */

             // replace source segment with matched segment
             if ( fOK && fReplaceSeg )
             {             // count source words (already counted ....)
               if ( pSeg->usSrcWords == 0 )
               {            // some way to fool the compiler
                 pSeg->usSrcWords = (USHORT) ulSrcWords;
               } /* endif */

               // use changed segment or original match
               if ( fChanged )
               {
                 ulLength = UTF16strlenCHAR(pszNewTargetW) + 1;
               }
               else
               {
                 ulLength = UTF16strlenCHAR(pszMatch) + 1;
               } /* endif */

               // make segment large enough to contain the :NONE. tag ...
               ulLength = max( ulLength, (sizeof(EMPTY_TAG)+4) );


               if ( ulLength > pSeg->usLength )
               {
                 // get new segment data area
                 UtlAlloc( (PVOID *)&(pSeg->pDataW), 0L, 0L, NOMSG );
                 fOK = UtlAlloc( (PVOID *)&(pSeg->pDataW), 0L, ulLength * sizeof(CHAR_W),
                                 ERROR_STORAGE );
                 pSeg->usLength = (USHORT)ulLength;
               } /* endif */


               if ( fOK )
               {
                 UTF16strcpy( pSeg->pDataW, ( fChanged ) ? pszNewTargetW : pszMatch );

                 // use :NONE. tag when segment is empty (the pDataW area is always large enough to contain the :NONE. tag)
                 if ( pSeg->pDataW[0] == 0 )
                 {
                   UTF16strcpy( pSeg->pDataW, EMPTY_TAG );
                 } /* endif */

                 pSeg->qStatus = QF_XLATED;
                 pSeg->SegFlags.Typed      = FALSE;
                 pSeg->SegFlags.Copied     = TRUE;
                 pSeg->CountFlag.AnalAutoSubst = TRUE;
                 pSeg->usModWords = 0;

                 // count target words
                 if ( !pSeg->SegFlags.NoCount)
                 {
                   ULONG ulTgtWords  = 0L;
                   ULONG ulTgtMarkUp = 0L;
                   USHORT usRc;

                   ADDTIME( pTAIn->ulTimeOther );


                   usRc = EQFBWordCntPerSeg(
                                (PLOADEDTABLE)pInD->pDoc->pDocTagTable,
                                (PTOKENENTRY) pInD->pDoc->pTokBuf,
                                pSeg->pDataW,
                                pInD->sTargetLangID,
                                &ulTgtWords, &ulTgtMarkUp, ulTgtOemCodePage);

                   ADDTIME( pTAIn->ulTimeWordCount );

                   if (!usRc)
                   {
                     pSeg->usTgtWords = (USHORT) ulTgtWords;
                   } /* endif */
				   
                   // handle any annotation data of the match (only when segment has no metadata so far)
                   {
                     int iAddInfoLen = pLookup->BestProposals[0]->getAddInfoLen();
                     if ( !usRc && (pSeg->pvMetadata == NULL) && (iAddInfoLen != 0) )
                     {
                       OtmProposal::eMatchType MatchType = pLookup->BestProposals[0]->getMatchType();
                       // compare match type with requrested mode
                       BOOL fCopyAnnotationData = FALSE;
                       if ( pInD->usAnnotationDataCopyMode == 0 ) // copy annotation data for exact matches
                       {
                         fCopyAnnotationData = pLookup->BestProposals[0]->isExactMatch();
                       } 
                       else if ( pInD->usAnnotationDataCopyMode == 1 ) // copy annotation data for exact context matches
                       {
                         fCopyAnnotationData = ( (MatchType == OtmProposal::emtExactSameDoc) || (MatchType == OtmProposal::emtExactExact) );
                       } 
                       else if ( pInD->usAnnotationDataCopyMode == 2 ) // copy annotation data for exact-exact matches
                       {
                         fCopyAnnotationData = (MatchType == OtmProposal::emtExactExact);
                       } /* endif */                        

                       // get any note information from proposal additional data area
                       if ( fCopyAnnotationData )
                       {
                         HADDDATAKEY hKey;
                         pLookup->BestProposals[0]->getAddInfo( pLookup->szAddInfoBuffer, sizeof(pLookup->szAddInfoBuffer) / sizeof(wchar_t) );
                         hKey = MADSearchKey( pLookup->szAddInfoBuffer, L"Note" );
                         if ( hKey != NULL )
                         {
                           MADGetAttr( hKey, L"style", pInD->szStyle, sizeof(pInD->szStyle) / sizeof(CHAR_W), L"" );
                           MDAGetValueForKey( hKey, pInD->szComment, sizeof(pInD->szComment) / sizeof(CHAR_W), L"" );
                           MDAddCommentData( &(pSeg->pvMetadata), pInD->szComment, pInD->szStyle );
                           pInD->fMetaDataAdded = TRUE;
                        } /* endif */
                       } /* endif */                        
                     } /* endif */                      
                   }
                 }
                 else
                 {
                    pSeg->usTgtWords = 0;
                 }

                 // if SGML-DITA special procesing is active, write replaced segment to folder/document memory
                 if ( UtlQueryUShort( QS_SGMLDITAPROCESSING ) )
                 {
                   char szMarkup[40];
                   pLookup->BestProposals[0]->getMarkup( szMarkup, sizeof(szMarkup) );
                   if ( DITASpecialProcessingRequired( pInD->szDocFormat, szMarkup, DITAWRITETOMEM ) )
                   {
                     OtmProposal NewProposal;
                     NewProposal = *(pLookup->BestProposals[0]);

                     NewProposal.setSegmentNum( pInD->ulSegNum );
                     NewProposal.setMarkup( pInD->szDocFormat );
                     NewProposal.setType( OtmProposal::eptManual );

                     LONG lTime = 0;
		                 UtlTime( &lTime );
                     NewProposal.setUpdateTime( lTime );
                     NewProposal.setContext( pInD->szContext );

                     pLookup->SearchMemoryDBs[0]->putProposal( NewProposal );
                   } /* endif */
                 } /* endif */
               } /* endif */
             } /* endif */

             /********************************************************* */
             /* Get rid of any changed target segment                   */
             /***********************************************************/
             if ( pszNewTargetW ) UtlAlloc( (PVOID *)&pszNewTargetW, 0L, 0L, NOMSG );
          }
          else
          {
            if ( fAnyMatch && pTAIn->fAutoJoin )
            {
              if ( (pSeg->pDataW == NULL) || (pSeg->pDataW[0] == EOS) )
              {
                // do not try empty segments in autojoin mode
              }
              else
              {
              pSeg->SegFlags.Marked = TRUE;
            } /* endif */
            } /* endif */

            if ( !fAutoJoinMode )
            {
              pInD->fNotComplete = TRUE;
              if ( !pSeg->SegFlags.NoCount )    // ignore nocount segments
              {
                // ignore emtpy segments
                if ( (pSeg->pDataW == NULL) || (pSeg->pDataW[0] == EOS) )
                {
                  // do not count empty segments
                }
                else
                {
                  pInD->ulSegsNotReplaced++;
                  pInD->ulTotalSegs++;
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */
        } /* endif */
     } /* endif */

     if ( fDoc || fTMMatch )
     {
       // add word count values
       if ( !fOnlyLookup && !fAutoJoinMode )
       {
         TACountMatch( pInD, MatchType, usFuzzyness, (LONG)ulSrcWords, 1L, TRUE );
       } /* endif */
     } /* endif */

     if ( fOK && !fOnlyLookup && !fExactMatch && !fAutoJoinMode && pTAIn->fMatchList )
     {
       pLookup->BestProposals[0]->getSource( pLookup->szSegmentBuffer, MAX_SEGMENT_SIZE );

       TAMatchListProcess( pTAIn, pSeg->pDataW, pLookup->szSegmentBuffer, MatchType, usFuzzyness );
     } /* endif */

     // handle fFile and !fExactMatch
     if ( !fOnlyLookup && fOK && fFile && !fExactMatch && !fAutoJoinMode && !(pTAIn->fAutoJoin && (pSeg->SegFlags.Marked == TRUE)) )
     {
        if ( (MatchType == MACHINE_MATCH) && (usFuzzyness >= 95) )
        {
          // do not write segment to SNOMATCH file, machine match exists
        }
        else if ( (MatchType == GLOBMEM_MATCH) && (usFuzzyness >= 95) )
        {
          // do not write segment to SNOMATCH file, global memory match exists
        }
        else if ( !pSeg->SegFlags.NoCount )
        {
          ULONG ulLength = UTF16strlenCHAR(pSeg->pDataW);

          ADDTIME( pTAIn->ulTimeOther );

          fOK =  WriteTMSegment( pTAIn, ulLength, pSeg->pDataW,
                                 (pInD->pCBNewMatchFile), ulSegNum,
                                 WRITETMSEGMENT_UTF8 );

          ADDTIME( pTAIn->ulTimeSNOMATCH );

          pInD->fNewMatchFileUsed = TRUE;
        } /* endif */

        if (!fOK)
        {
           PSZ pszDoc;

           pszDoc = pTAIn->apszLongNames[pInD->usCurNumSourceFile-1];

           usReturn = UtlErrorHwnd ( ERROR_TA_SAVE_SEGFILE, MB_YESNO, 1,
                                     &pszDoc, EQF_QUERY, pTAIn->hwndErrMsg );
           pTAIn->fKill = (usReturn != MBID_YES);
        } /* endif */

     } /* endif */

     // handle fMachTrans and !fExactMatch
     if ( fOK && !fOnlyLookup && fMachTrans && !fAutoJoinMode && !(pTAIn->fAutoJoin && (pSeg->SegFlags.Marked == TRUE)) )
     {
        ADDTIME( pTAIn->ulTimeOther );

        fOK = TAMachTransAddSeg( pTAIn, ulSrcWords, ulSrcMarkup != 0, MatchType, usFuzzyness, ulSegNum, pTAIn->szOrgSegSource, pSeg );
        ADDTIME( pTAIn->ulTimeMachMatch );
     } /* endif */

  } /* endif */

  // add segment to redundancy count TM and add segment words to
  // total words of document
  if (fOK && !fOnlyLookup && fRedundCount && !fAutoJoinMode )
  {
    ADDTIME( pTAIn->ulTimeOther );

    //exclude candidates for auto-joining (these segment will be processed
    // during auto-joining)
    if ( !(pTAIn->fAutoJoin && (pSeg->SegFlags.Marked == TRUE)))
    {
	  fOK = TASegTMRedundCount(pTAIn, pInD, pSeg, MatchType );
    } /* endif */       

    ADDTIME( pTAIn->ulTimeRedundCount );

  } /* endif */

#ifdef TA_SEGMENTLOGGING
   // segment logging for debugging only
   if ( !fOnlyLookup )
   {
     char szLogFile[MAX_EQF_PATH];
     FILE *hLog = NULL;
     
     UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
     UtlMkDir( szLogFile, 0, FALSE );
     strcat( szLogFile, "\\EQFSEG.LOG" );
     hLog = fopen( szLogFile, "a" );
     if ( hLog )
     {
       fprintf( hLog, "TaSegTMProc done\n" );
       fclose( hLog );
     } /* endif */
   }
#endif

  return ((USHORT)fOK);
} /* end of TASegTMProc */

 /***********************************************************/
 /* check if we have to do an additional checking for tag   */
 /* changes, if so call it but do not allow for message     */
 /* pop up.                                                 */
 /***********************************************************/
BOOL
TAHandleCheckSegExit
(
	PTAINSTDATA  pInD,
	PTBSEGMENT   pSeg,
	PSZ_W        pszMatch,
	ULONG        ulTgtOemCodePage,
    PBOOL        pfOK,
    HWND         hwndErrMsg
)
{
	BOOL            fReplaceSeg = TRUE;
	int             iReturn;

  PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;

	if ( pInD->pfnEQFCheckSegW || pInD->pfnEQFCheckSeg || pInD->pfnEQFCheckSegExW )
	{
	   EQF_BOOL fChanged = FALSE;

	   if (pInD->pfnEQFCheckSegExW )
	   {
		   fReplaceSeg =  pInD->pfnEQFCheckSegExW( pInD->pszPrevSegment,
											  pSeg->pDataW,
											  pszMatch,
											  &fChanged, (LONG)pInD->pDoc, pInD->ulSegNum, FALSE );
	   }
	   else if (pInD->pfnEQFCheckSegW )
	   {
		 fReplaceSeg =  pInD->pfnEQFCheckSegW( pInD->pszPrevSegment,
											pSeg->pDataW,
											pszMatch,
											&fChanged, FALSE );
	   }
	   else
	   {
		  PSZ pTemp = &pInD->szASCIIBuffer[0];
		  PSZ pTempMatch = &pInD->szASCIIBuffer[ MAX_SEGMENT_SIZE ];

		  PTBSEGMENT pTempSeg = EQFBGetSeg( pInD->pDoc, pInD->ulSegNum );

		  Unicode2ASCII(pInD->pszPrevSegment, pTemp, pInD->pDoc->ulOemCodePage );
		  Unicode2ASCII(pszMatch, pTempMatch, ulTgtOemCodePage);
		  // pTemp, PTempSeg->pData is source-seg, eg. in  SOURCE -lang
		  // pTempMatch is current transl. eg in TARGET-lang
		  fReplaceSeg =  pInD->pfnEQFCheckSeg( pTemp,
											pTempSeg->pData,
											   pTempMatch,
											&fChanged, FALSE );
      // convert segment data back to Unicode only if user exit changed the data
      if ( fChanged )
      {
		    ASCII2Unicode(pTempMatch, pszMatch, ulTgtOemCodePage);
      } /* endif */
	   }

	   if ( fReplaceSeg && fChanged )
	   {
       OtmProposal *newProposal = new(OtmProposal);
       *newProposal = *pLookupData->BestProposals[0];
       newProposal->setSource( pSeg->pDataW );
       newProposal->setTarget( pszMatch );

       iReturn = pLookupData->pDocMem->putProposal( *newProposal );

  		 if ( iReturn )
		   {
			   pInD->fTerminate = TRUE;
			   *pfOK = FALSE;                                       /* 1@KIT0905A */
			    // no 'organize required' message for in-use conditions
			   if ( iReturn != BTREE_IN_USE )
			   {
			     UtlErrorHwnd( ERROR_TA_TMERROR, MB_CANCEL, 0, NULL, EQF_ERROR, hwndErrMsg );
			   } /* endif */
		   } /* endif */
       delete ( newProposal );
	   } /* endif */
	 }
	 else
	 {
	   // no segment check exit, use the match for substitution
	   fReplaceSeg = TRUE;
	 } /* endif */

	return(fReplaceSeg);
}

// handle all extra work for fInsertToTM flag:
// insert found matches in the selected TM's
// and output Translation Memory
BOOL
TAInsertToTM
(
	PTAINPUT     pTAIn,
	PTAINSTDATA  pInD,
	PTBSEGMENT   pSeg,
	PBOOL        pfExactMatch
)
{
  BOOL     fOK = TRUE;
  int      iReturn = NO_ERROR;
  SHORT    sIndex = 0;
  BOOL     fExactMatch = FALSE;
  PANALYSISMEMLOOKUPDATA pLookup = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;

  // get best matches from translation memories
  OtmProposal::clearAllProposals( pLookup->BestProposals );
  iReturn = GetBestTMMatch(pTAIn,
					   pSeg->pDataW,
             pLookup->InputMemoryDBs,
					   pLookup->BestProposals,
					   &fExactMatch,
					   MAX_NUM_MATCHES);
  fOK = (iReturn == NO_ERROR);

  if (!fOK)   // TM error: terminate analysis process
  {
	 pInD->fTerminate = TRUE;
	 // no 'organize required' message for in-use conditions
	 if ( iReturn != BTREE_IN_USE )
	 {
	   UtlErrorHwnd ( ERROR_TA_TMERROR,
				  MB_CANCEL,
				  0,
				  NULL,
				  EQF_ERROR, pTAIn->hwndErrMsg);
	 } /* endif */
  } /* endif */

  if (fOK)       // copy best matches to output tm
  {
	/****************************************************************/
	/* put all segments..                                           */
	/****************************************************************/
  

  for (sIndex = 0; !pLookup->BestProposals[sIndex]->isEmpty() && (sIndex <  MAX_NUM_MATCHES) && fOK; sIndex++)
	{
    iReturn = (USHORT)pLookup->pDocMem->putProposal( *pLookup->BestProposals[sIndex] );

	  if ( iReturn )
	  {
		 pInD->fTerminate = TRUE;
		 fOK = FALSE;                                       /* 1@KIT0905A */
		 // no 'organize required' message for in-use conditions
		 if ( iReturn != BTREE_IN_USE )
		 {
		   UtlErrorHwnd( ERROR_TA_TMERROR, MB_CANCEL, 0, NULL,
						 EQF_ERROR, pTAIn->hwndErrMsg );
		 } /* endif */
	  } /* endif */
	} /* endfor */
   }  // end of processing for output Translation Memory
   *pfExactMatch = fExactMatch;

  return (fOK);
}

// do all redundancy counting

static BOOL
TASegTMRedundCount
(
	PTAINPUT     pTAIn,
	PTAINSTDATA  pInD,
	PTBSEGMENT   pSeg,
	MATCHTYPE    MatchType
)
{
  SHORT    sDocIndex  = 0;
  BOOL     fOK        = TRUE;
  PSZ      pszDocName = NULL;
  USHORT   usReturn   = NO_ERROR;

  // update total number of document segments
  // get index of document in document array
  pszDocName = pTAIn->apszLongNames[pInD->usCurNumSourceFile-1];
  sDocIndex = TADocNameToIndex( pTAIn, pszDocName );
  if ( sDocIndex >= 0 )
  {
	strcpy( pInD->pDocCounts[sDocIndex].szShortName, pInD->pszCurSourceFile );
	TAAddCounts( &(pInD->pDocCounts[sDocIndex].CountTotal), 1, pSeg->usSrcWords );
  }
  else
  {
	// memory allocation failed
	pInD->fTerminate = TRUE;
	fOK = FALSE;                                       /* 1@KIT0905A */
  } /* endif */

	if ( fOK )
	{
	  if ( (MatchType == EXACTEXACT_MATCH) ||
		   (MatchType == EXACTMORE_MATCH) ||
		   (MatchType == GLOBMEM_MATCH) ||
		   (MatchType == EXACTONE_MATCH) )
	  {
		// update exact match counts
		TAAddCounts( &(pInD->pDocCounts[sDocIndex].CountExact), 1, pSeg->usSrcWords );
	  }
	  else
	  {
		memset( &(pInD->stPutIn.stTmPut), 0, sizeof(pInD->stPutIn.stTmPut) );


    // normalize segment (remove LFs and reduce multiple blanks), remove blanks preceeding punctuation,
    // cut off trailing blanks
    {
      // we use the target segment as temporary buffer area 
  		PSZ_W pszTarget = pInD->stPutIn.stTmPut.szSource;
      PSZ_W pszSource = pInD->stPutIn.stTmPut.szTarget;
      CHAR_W chLast = 'X';

  		UTF16strcpy( pInD->stPutIn.stTmPut.szTarget, pSeg->pDataW );
      while ( *pszSource )
      {
        if ( *pszSource == L'\n' ) *pszSource = L' ';
        if ( (*pszSource == L' ') && (chLast == L' ') )
        {
          // ignore multiple blank
          pszSource++;
        }
        else if ( (*pszSource == L' ') && 
                  ((pszSource[1] == L'.') || (pszSource[1] == L',') || 
                   (pszSource[1] == L';') || (pszSource[1] == L'!') || (pszSource[1] == L'?')))
        {
          // ignore blank preceeding punctuation
          pszSource++;
        }
        else
        {
          // copy character to target
          *pszTarget++ = chLast = *pszSource++;
        } /* endif */
      } /*endwhile */
      *pszTarget = 0;

      // remove trailing blanks
      while ( (pszTarget > pInD->stPutIn.stTmPut.szSource) && (pszTarget[-1] == L' ') )
      {
        pszTarget--;
        *pszTarget = 0;
      } /*endwhile */
    }

    PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;
    OtmProposal prop;
    prop.setSource( pInD->stPutIn.stTmPut.szSource );
    prop.setTarget( pInD->stPutIn.stTmPut.szSource );
    prop.setDocShortName( pInD->pszCurSourceFile);
    prop.setDocName( pTAIn->apszLongNames[pInD->usCurNumSourceFile-1] );
    prop.setType( OtmProposal::eptManual );
    prop.setSegmentNum( pInD->ulSegNum );
    prop.setMarkup( pInD->szDocFormat );
    prop.setSourceLanguage( pInD->szDocSourceLang );
    prop.setTargetLanguage( pInD->szDocTargetLang );
    usReturn = (USHORT)pLookupData->pRedundCountMem->putProposal( prop );
		if ( usReturn )
		{
		   pInD->fTerminate = TRUE;
		   fOK = FALSE;                                       /* 1@KIT0905A */
		   // no 'organize required' message for in-use conditions
		   if ( usReturn != BTREE_IN_USE )
		   {
  			 UtlErrorHwnd( ERROR_TA_TMERROR, MB_CANCEL, 0, NULL, EQF_ERROR, pTAIn->hwndErrMsg );
		   } /* endif */
		} /* endif */
	  } /* endif */
	} /* endif */

#ifdef SESSIONLOG
  if ( !fOK )
  {
    UtlLogWriteString( "TASegTMRedundCount: failed for document %s", pInD->pszCurSourceFile );
  } /* endif */        
#endif

  return (fOK);
}

// handle the flags which may adjust the match:
//  -- adjust leading whitespaces
//  -- adjust trailing whitespaces
//  -- no additional blank at segment end

static BOOL
TAHandleAllAdjustOptions
(
	PTAINPUT      pTAIn,
	PTBSEGMENT    pSeg,
	PSZ_W         pszMatch,
	PSZ_W        *ppszNewTargetW,
	PBOOL         pfChanged
)
{
	BOOL   fOK      = TRUE;
	PSZ_W  pszNew   = NULL;
	BOOL   fChanged = *pfChanged;


	// Add CRLF or LF at end or begin of TM match if source segment has different whitespace
	if (pTAIn->fLeadingWS || pTAIn->fTrailingWS)
	{
		 // fill list of tags with CLS_WHITESPACE if not yet done
        if (pTAIn->pWSList == NULL)
        {
			TAFillWSList(pTAIn->szTagTableName, &(pTAIn->pWSList));
	    }
		fOK = TAAdjustWhiteSpace( pSeg->pDataW, pszMatch, &pszNew, pTAIn->fLeadingWS,
								 pTAIn->fTrailingWS, &fChanged, pTAIn->pWSList );
  } /* endif */

	// handle no-additional-blank-at-segment-end flag
	if ( fOK && pTAIn->fNoAddBlank )
	{
     // work on existing or changed segment
	   PSZ_W pszSeg = ( fChanged ) ? pszNew : pszMatch;
	   ULONG ulSegLen = UTF16strlenCHAR(pszSeg);

	   // cut off blank if the segment end is a period followed by blank
	   if ( (ulSegLen > 1) &&
			(pszSeg[ulSegLen-1] == BLANK) &&
			(pszSeg[ulSegLen-2] == '.') )
	   {
		 // truncate segment
		 pszSeg[ulSegLen-1] = EOS;
	   } /* endif */
  } /* endif */

  *ppszNewTargetW = pszNew;
  *pfChanged = fChanged;

  // avoid empty segments, if the segment content has been changed
  if ( *pfChanged )
  {
    if ( *pszNew == EOS )
    {
      // replace empty segment with :NONE., TAAdjustWhiteSpace ensures that the
      // newly allocated segment is large enough to contain this tag
      UTF16strcpy( pszNew, EMPTY_TAG );
    } /* endif */
  } /* endif */

  return(fOK);
}

// new for P018345
// if LF different and #ofLF != 0, do not allow replace
// if markup table with reflow=NO
// If reflow is not allowed by markup table,
// the number of LF's need to be checked

BOOL
TACheckAndAdaptLFChanges
(
	PSZ_W          pSegData,
	PSZ_W          pszMatch,
	BOOL           fMsg
)
{
  BOOL  fAllowReplace = TRUE;

  SHORT sNumLFDifferent = 0;
  SHORT sNumLFData      = 0;
  SHORT sNumLFMatch     = 0;
  PSZ_W pszTemp         = pszMatch;

  sNumLFDifferent = EQFBIsNumLFEqual(pSegData, pszMatch,
									 &sNumLFData, &sNumLFMatch);
  if (sNumLFDifferent )
  {
	  if (sNumLFData == 0)
	  {		  // change match: delete all LF's in MAtch!!
		  ULONG ulSegLen = UTF16strlenCHAR(pszMatch);
		  while ( ulSegLen )
		  {  // delete LF's in pszMatch
			 if (*pszMatch == LF)
			 {
				 *pszMatch++;
			 }
			 else
			 {
				 *pszTemp++ = *pszMatch++;
			 }
			 ulSegLen --;
		  } /* endif */
		  *pszTemp = EOS;
	  }
	  else
	  {
		 fAllowReplace = FALSE;
		 if (fMsg )
		 {
		   UtlError( TB_LFCHANGE_NOT_ALLOWED, MB_CANCEL, 0, NULL, EQF_WARNING);
		 }
		 else
		 {
		   WinAlarm( HWND_DESKTOP, WA_WARNING );  // therefore beep
	     }
	  }
  } /* endif */

  return(fAllowReplace);
}

/*
.cp
********************** GetExactTMMatch ****************************************
*                                                                              *
*  fill best matches from Translation Memory Get into buffer                   *
*******************************************************************************/

static USHORT GetExactTMMatch(PTAINPUT pTAIn,             // ta instance data
                           PSZ_W          pszSegment,     // pointer to segmentboundary
                           std::vector<OtmMemory *> &SearchMemories,     // pointer to tm's
                           BOOL         * pfExactMatch, // exact match found?
                           BOOL         * pfAnyMatch,   // any match found?
                           USHORT       usNumMatches,   // number of matches to be
                           PMATCHTYPE   pMatchType,     // type of matches found
                           PUSHORT      pusFuzzy,       // fuzzyness of proposal
                           PVOID        pvMetaData)     // segment metadata
{
   PTAINSTDATA pInD;                    // pointer to instance data
   USHORT    usRc = 0;         // return from tm functions
   BOOL      fOK = TRUE;       // success indicator
   USHORT    i;
   ULONG     ulTgtOemCP = 0L;
   ULONG     ulSrcOemCP = 0L;
   MATCHTYPE Bestmatch = NO_MATCH;
   USHORT    usBestFuzzy = 0;
   BOOL      fDone = FALSE;
   ULONG ulOptions = 0;
   PSZ pszLongName = NULL; // long document name

   *pMatchType = NO_MATCH;             // initialize match type


   pInD = (PTAINSTDATA) pTAIn->pInD;
   PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;
   MemoryFactory *pFactory = MemoryFactory::getInstance();

   *pfExactMatch = FALSE;
   *pfAnyMatch   = FALSE;

   ulTgtOemCP = GetLangOEMCP( pInD->szDocTargetLang);
   ulSrcOemCP = GetLangOEMCP( pInD->szDocSourceLang);

   LONG lProposalOrderOptions = 0;
   
   // ensure that proposals are sorted using the timestamps when use latest exact match is active and use context match is not active
   if ( !pInD->fExactContextTMMatch && pInD->fUseLatestTMMatch )
   {
     lProposalOrderOptions = MemoryFactory::LATESTPROPOSAL_FIRST;
   } /* endif */

   pLookupData->SearchKey.clear();
   for ( int i = 0; i < (int)pLookupData->BestProposals.size(); i++ )
   {
     pLookupData->BestProposals[i]->clear();
   } /* endfor */         


   {
     pszLongName = pTAIn->apszAlias[pInD->usCurNumSourceFile-1];

     if ( (pszLongName != NULL) && (pszLongName[0] != EOS) )
     {
       if ( UtlIsLongFileName( pszLongName ) )
       {
         UtlLongToShortName( pszLongName, pTAIn->szShortAlias );
         pLookupData->SearchKey.setDocShortName( pTAIn->szShortAlias );
       }
       else
       {
         pLookupData->SearchKey.setDocShortName( pszLongName );
       } /* endif */
     }
     else
     {
       pszLongName = pTAIn->apszLongNames[pInD->usCurNumSourceFile-1];
       pLookupData->SearchKey.setDocShortName( pInD->pszCurSourceFile );
     } /* endif */

     // remove path in document path if requested
     if ( pTAIn->fIgnorePath )
     {
       PSZ pszStartOfName = strrchr( pszLongName, BACKSLASH );
       if ( pszStartOfName != NULL )
       {
         pszLongName = pszStartOfName + 1;
       } /* endif */
     } /* endif */
     pLookupData->SearchKey.setDocName( pszLongName );


   }
   pLookupData->SearchKey.setSegmentNum( pInD->ulSegNum );

   // GQTODO: What with number of requested matches?: pGetIn->stTmGet.usRequestedMatches = usNumMatches;

   wcscpy( pLookupData->szSegmentBuffer, pszSegment );
   EQFBBufRemoveTRNote( pLookupData->szSegmentBuffer, pInD->pLoadedTable, NULL, NULL, ulSrcOemCP );
   pLookupData->SearchKey.setSource( pLookupData->szSegmentBuffer );

   ulOptions = GET_EXACT | GET_ALL_EXACT_MATCHES;
   if ( pTAIn->fNoGenericReplace ) ulOptions |= GET_NO_GENERICREPLACE;
   if ( pTAIn->fIgnoreComment ) ulOptions  |= GET_IGNORE_COMMENT;
   if ( pTAIn->fIgnorePath ) ulOptions |= GET_IGNORE_PATH;
   if ( pTAIn->fRespectCRLF || pInD->fCrlfFlag || (pInD->pLoadedTable && (pInD->pLoadedTable->fReflow == TAGREFLOW_NO)))
   {
     ulOptions |= GET_RESPECTCRLF;
   } /* endif */

   pLookupData->SearchKey.setMarkup( pInD->szDocFormat );
   pLookupData->SearchKey.setSourceLanguage( pInD->szDocSourceLang );
   pLookupData->SearchKey.setTargetLanguage( pInD->szDocTargetLang );
   pLookupData->SearchKey.setContext( pInD->szContext );
   // GQTODO: Handling for dita replacement list (move to utility function?): pGetIn->stTmGet.pvReplacementList = pTAIn->pvReplacementList;
   pLookupData->SearchKey.setReplacementList( (long)pTAIn->pvReplacementList ) ;


   // get matches from segment metadata
   if ( pvMetaData )
   {
      USHORT usNumMatchesFound = 0;
      static MD_PROPOSAL MDProposal;
      BOOL fProposalAvailable = MDGetFirstProposal( pvMetaData, &MDProposal );

      for ( int i = 0; i < (int)pLookupData->FoundProposals.size(); i++ )
      {
        pLookupData->FoundProposals[i]->clear();
      } /* endfor */         

      while ( fProposalAvailable && (usNumMatchesFound < usNumMatches) )
      {
        pLookupData->FoundProposals[usNumMatchesFound]->setUpdateTime( MDProposal.lDate );
        pLookupData->FoundProposals[usNumMatchesFound]->setDocShortName( "XLIFF" );
        pLookupData->FoundProposals[usNumMatchesFound]->setDocName( MDProposal.szDocName );
        pLookupData->FoundProposals[usNumMatchesFound]->setSource( MDProposal.szSource ); 
        pLookupData->FoundProposals[usNumMatchesFound]->setMarkup( "EQFXLIFF" );
        pLookupData->FoundProposals[usNumMatchesFound]->setTarget( MDProposal.szTarget );
        pLookupData->FoundProposals[usNumMatchesFound]->setAuthor( MDProposal.szAuthorName );
        pLookupData->FoundProposals[usNumMatchesFound]->setTargetLanguage( pInD->szDocTargetLang );
        pLookupData->FoundProposals[usNumMatchesFound]->setSegmentNum( MDProposal.ulSegmentId );
        pLookupData->FoundProposals[usNumMatchesFound]->setFuzziness( MDProposal.sQuality );
        usNumMatchesFound++;

        fProposalAvailable = MDGetNextProposal( &MDProposal );
      } /*endwhile */

      pFactory->copyBestMatches( pLookupData->FoundProposals, pLookupData->BestProposals, usNumMatches, -1, FALSE, lProposalOrderOptions );
   } /* endif */

   // get  best matches from translation memory list
   i = 0;
   ADDTIME( pTAIn->ulGetExactTMMatchstart );

   while ( !usRc && (i < SearchMemories.size()) && !fDone )
   {
     SearchMemories[i]->searchProposal( pLookupData->SearchKey, pLookupData->FoundProposals, ulOptions );
     ADDTIME( pTAIn->ulTMGetW);
     if ( !usRc )
     {
       int iNumOfProposals = OtmProposal::getNumOfProposals( pLookupData->FoundProposals );
       if ( iNumOfProposals  != 0 )
       {
         *pMatchType = FUZZY_MATCH;                // at least a fuzzy match found ...
         if ( Bestmatch == NO_MATCH )
         {
           Bestmatch = FUZZY_MATCH;
         } /* endif */

         // if TM match counting is active ...
         if ( pTAIn->fTMMatch || pTAIn->fInsertTMMatches || pTAIn->fMachTrans || pTAIn->fInsertNewMatch )
         {
           int i = 0;

           // set machine match type depending on machine flag of best match
           if ( pLookupData->FoundProposals[0]->getType() == OtmProposal::eptMachine )
           {
             *pMatchType = MACHINE_MATCH;
           } 
           else if ( (pLookupData->FoundProposals[0]->getType() == OtmProposal::eptGlobalMemory) || (pLookupData->FoundProposals[0]->getType()== OtmProposal::eptGlobalMemoryStar) )
           { 
             if ( pLookupData->FoundProposals[i]->getFuzziness() >= 100 )
             {
               *pMatchType = GLOBMEM_MATCH;
             }
             else
             {
               // GQ 2019/04/02: global memory matches which are no exactr matches are treated as normal fuzzy matches
               *pMatchType = FUZZY_MATCH;
             } /* endif */
           } /* endif */

           // check if one of our matches is a replace match
           i = 0;
           while ( (i < iNumOfProposals) && (*pMatchType != REPL_MATCH) )
           {
             if ( (pLookupData->FoundProposals[i]->getFuzziness() < 100) && (pLookupData->FoundProposals[i]->getType() != OtmProposal::eptMachine) )
             {
              USHORT usReplMatch;         // buffer for replace flag

              pLookupData->FoundProposals[i]->getSource( pTAIn->szReplSegmentSourceW, sizeof(pTAIn->szReplSegmentSourceW)/sizeof(CHAR_W) );
              pLookupData->FoundProposals[i]->getTarget( pTAIn->szReplSegmentW, sizeof(pTAIn->szReplSegmentW)/sizeof(CHAR_W) );
              usReplMatch = EQFBPrepareFuzzyPropEx(
                              pInD->pDoc,                 // document
                              pszSegment,                 // source
                              pTAIn->szReplSegmentSourceW, // source of proposal
                              pTAIn->szReplSegmentW,         // pointer to Translation
                              pInD->TolstControl.sLangID, // source language Id..
                              pInD->sTargetLangID,        // target language id..
                              pusFuzzy,                   // fuzzyness
                              pInD->TolstControl.ulOemCP,  // cp of source lang.
                              ulTgtOemCP  );               // cp of target lang ( sTargetLangID)

              if ( usReplMatch == PROP_REPLACED )
              {
                *pMatchType = REPL_MATCH;           // at least a replace match
              }
              else if ( usReplMatch == PROP_REPLACED_FUZZY )
              {
                *pMatchType = FUZZYREPL_MATCH;      // at least a fuzzy replace match
              } /* endif */

              if ( (*pMatchType == REPL_MATCH) || (*pMatchType == FUZZYREPL_MATCH) || (*pMatchType == FUZZY_MATCH) )
              {
                // use match level for fuzziness if larger than fuzziness of replace match
                *pusFuzzy = (USHORT)max( pLookupData->FoundProposals[i]->getFuzziness(), *pusFuzzy );
                if ( usBestFuzzy < *pusFuzzy )
                {
                  usBestFuzzy = *pusFuzzy;
                  Bestmatch = *pMatchType;
                } /* endif */
              } /* endif */
             } /* endif */

             i++;                      // next proposal
           } /* endwhile */
         } /* endif */
       } /* endif */


       pFactory->copyBestMatches( pLookupData->FoundProposals, pLookupData->BestProposals, usNumMatches, -1, FALSE, lProposalOrderOptions );
       ADDTIME(pTAIn->ulCopyBest);
       i++;
     } /* endif */

     // check if we can leave the lookup loop
     if ( pTAIn->fStopAtFirstExact )
     {
       OtmProposal::eMatchType MatchType = pLookupData->BestProposals[0]->getMatchType();
       OtmProposal::eProposalType PropType = pLookupData->BestProposals[0]->getType();
       if (  ( (MatchType == OtmProposal::emtExact) || 
               (MatchType == OtmProposal::emtExactSameDoc) || 
               (MatchType == OtmProposal::emtExactExact) ) && (PropType != OtmProposal::eptGlobalMemory) && (PropType != OtmProposal::eptGlobalMemoryStar) )
       {
         // we have an exact match, ignore matches from other memories
         fDone = TRUE;
       } /* endif */          
     } /* endif */        
   } /* endwhile */


   fOK = (usRc == NO_ERROR);

   if (fOK)
   {
      *pfAnyMatch = TRUE;

      if ( pLookupData->BestProposals[0]->isExactMatch() )
      {
        int iProposals = OtmProposal::getNumOfProposals( pLookupData->BestProposals );

        if ( iProposals > 1 )
        {
          OtmProposal::eProposalType eptProp0Type = pLookupData->BestProposals[0]->getType();
          OtmProposal::eProposalType eptProp1Type = pLookupData->BestProposals[1]->getType();
          OtmProposal::eMatchType emtProp0Match = pLookupData->BestProposals[0]->getMatchType();
          OtmProposal::eMatchType emtProp1Match = pLookupData->BestProposals[1]->getMatchType();

          // WLP: for R012645
          // if there are two or more exact-exact matches, same markup name one have higher priority
          if( emtProp0Match==OtmProposal::emtExactExact &&
              emtProp1Match==OtmProposal::emtExactExact )
          {
              char szMarkupName[MAX_FILESPEC+1] = {0};
              std::vector<OtmProposal*> sameVec;
              std::vector<OtmProposal*> diffVec;
              for( auto iter=pLookupData->BestProposals.begin(); iter!=pLookupData->BestProposals.end(); iter++)
              {
                  OtmProposal *pTemp = *iter;
                  if(pTemp->getMatchType()!=OtmProposal::emtExactExact)
                      break;
                  else
                  {
                      memset(szMarkupName,0,sizeof(szMarkupName));
                      pTemp->getMarkup(szMarkupName,MAX_FILESPEC);
                      if( strncmp(szMarkupName,pInD->szDocFormat,MAX_FILESPEC) == 0 )
                          sameVec.push_back(pTemp);
                      else
                          // with different markup name exact-exact matches are from same family name
                          diffVec.push_back(pTemp);
                  }
              }
              // if diffVec is empty, then all the exact-exact are from same markup name
              // if sameVec is empty, then all the exact-exact are from same family name
              // so only when above mixed, need to re-order the matches
              if( !diffVec.empty() && !sameVec.empty() )
              {
                  auto begIter = pLookupData->BestProposals.begin();
                  // the sameVec are exact-exact match, so have higher priority
                  for(auto sIter=sameVec.begin(); sIter!=sameVec.end(); sIter++)
                      *begIter++ = *sIter;

                  // the diffVec are exact match
                  for(auto dIter= diffVec.begin(); dIter!=diffVec.end(); dIter++)
                  {
                      (*dIter)->setMatchType(OtmProposal::emtExact);
                      *begIter++ = *dIter;
                  }    
              }
          }//end 


          // when second match is a global memory match treat normal exact match as EXACTONE_MATCH to allow autosubstitution
          if ( (eptProp0Type == OtmProposal::eptManual)  && 
		           ( (eptProp1Type == OtmProposal::eptGlobalMemory) || (eptProp1Type == OtmProposal::eptGlobalMemoryStar) ) )
          {
            if ( emtProp0Match == OtmProposal::emtExactExact ) 
            {
              *pMatchType = EXACTEXACT_MATCH;
            }
            else
            {
              *pMatchType = EXACTONE_MATCH;
            } /* endif */               
          }
          // new approach: 100% context matches are treated as exact-exact matches (but only if next match in list is not a 100% context match as well
          else if ( (eptProp0Type == OtmProposal::eptManual) && pLookupData->BestProposals[0]->isExactMatch() && (pLookupData->BestProposals[0]->getContextRanking() == 100) )
          {
            // if next match is a 100% context match as well, compare the document names
            if ( (eptProp1Type == OtmProposal::eptManual) && pLookupData->BestProposals[1]->isExactMatch() && (pLookupData->BestProposals[1]->getContextRanking() == 100))
            {
              BOOL fProp0DocNameMatch = FALSE;
              BOOL fProp1DocNameMatch = FALSE;
              
              pLookupData->BestProposals[0]->getDocName( pLookupData->szNameBuffer, sizeof(pLookupData->szNameBuffer) );
              fProp0DocNameMatch = stricmp( pLookupData->szNameBuffer, pszLongName ) == 0;
              pLookupData->BestProposals[1]->getDocName( pLookupData->szNameBuffer, sizeof(pLookupData->szNameBuffer) );
              fProp1DocNameMatch = stricmp( pLookupData->szNameBuffer, pszLongName ) == 0;

              if ( fProp0DocNameMatch && !fProp1DocNameMatch )
              {
                // only document name of first proposal matches so treat this proposal as exact-exact match
                *pMatchType = EXACTEXACT_MATCH;
              }
              else
              {
                // document names of first two proposal matches so treat these proposals as multiple exact match
                *pMatchType = EXACTMORE_MATCH;
              } /* endif */
            }
            else
            {
              *pMatchType = EXACTEXACT_MATCH;
            } /* endif */
          }
          // treat multiple exact-exact matches as EXACTMORE_MATCH
          else if ( (emtProp0Match == OtmProposal::emtExactExact) && (emtProp1Match != OtmProposal::emtExactExact) )
          {
            *pMatchType = EXACTEXACT_MATCH;
          }
          else if ( pLookupData->BestProposals[0]->getContextRanking() > pLookupData->BestProposals[1]->getContextRanking() )
          {
            // treat exact match with better context ranking as single exact match
            *pMatchType = EXACTONE_MATCH;
          }
          else
          {
            *pMatchType = EXACTMORE_MATCH;
          }
        }
        else
        {
          if ( (pLookupData->BestProposals[0]->getType() == OtmProposal::eptGlobalMemory) ||
               (pLookupData->BestProposals[0]->getType() == OtmProposal::eptGlobalMemoryStar)	)
          {
            *pMatchType = GLOBMEM_MATCH;
          }
          else if ( pLookupData->BestProposals[0]->isExactMatch() && (pLookupData->BestProposals[0]->getContextRanking() == 100) )
          {
            *pMatchType = EXACTEXACT_MATCH;
          }
          else if ( pLookupData->BestProposals[0]->getMatchType() == OtmProposal::emtExactExact )
          {
            *pMatchType = EXACTEXACT_MATCH;
          }
          else 
          {
            *pMatchType = EXACTONE_MATCH;
          }
        } /* endif iProposals > 1 */

        *pusFuzzy   = 100;

        // fix for P401665: do not check for autoreplace when there is an exact-exact or a single exact match
        if ( (*pMatchType == EXACTEXACT_MATCH) || (*pMatchType == EXACTONE_MATCH) )
        {
          *pfExactMatch = TRUE;
        }
        else
        {
          *pfExactMatch = pFactory->isAutoSubstCandidate( pLookupData->BestProposals );
        } /* endif */ 

        // check for context match flag (match from same document)
        if ( (!*pfExactMatch) &&  pInD->fExactContextTMMatch  )
        {
          // only use for substitution when this proposal (but not the next one) is from the same document
          if ( (pLookupData->BestProposals[0]->getMatchType() == OtmProposal::emtExactSameDoc) &&  
                (pLookupData->BestProposals[1]->getMatchType() != OtmProposal::emtExactSameDoc) )
          {
            *pfExactMatch = TRUE;
          }
        } /* endif */
		

        if ( (!*pfExactMatch) && pLookupData->BestProposals[1]->isExactMatch() )
        {
          /************************************************************/
          /* Match should not be used for automatic replace:          */
          /* reason 1: there are more than one exact match            */
          /* reason 2: the match is no exact match                    */
          /*                                                          */
          /* For reason 1: if user set the 'use-latest-match' flag use*/
          /*               the first match of exact matchs            */
          /************************************************************/
          *pfExactMatch = pInD->fUseLatestTMMatch;
          *pMatchType = EXACTMORE_MATCH;
        } /* endif */
      }
      else if ( Bestmatch != NO_MATCH )
      {
        // check if currently best match in match table is a machine translation
        // match
        if ( (pLookupData->BestProposals[0]->getType() == OtmProposal::eptMachine) &&
             (pLookupData->BestProposals[0]->getFuzziness() > usBestFuzzy) )
        {
          // use machine match
          *pusFuzzy   = (USHORT)pLookupData->BestProposals[0]->getFuzziness();
          *pMatchType = MACHINE_MATCH;
        }
        else if ( ( (pLookupData->BestProposals[0]->getType() == OtmProposal::eptGlobalMemory) || 
                    (pLookupData->BestProposals[0]->getType() == OtmProposal::eptGlobalMemoryStar) ) &&
                    (pLookupData->BestProposals[0]->getFuzziness() >= 100) )
        {
          // use Hamster match
          *pusFuzzy   = (USHORT)pLookupData->BestProposals[0]->getFuzziness();
          *pMatchType = GLOBMEM_MATCH;
        }
        else
        {
          // no exact match is available, use best match from our best match buffer
          *pusFuzzy   = usBestFuzzy;
          *pMatchType = Bestmatch;
        } /* endif */
      } /* endif */
   } /* endif */

   ADDTIME( pTAIn->ulTimeGetMatch );
   return (usRc);
} /* end of GetExactTMMatches */


/*
.cp
********************** GetBestTMMatch ******************************************
*                                                                              *
*  fill best matches from Translation Memory Get into buffer                   *
*******************************************************************************/

static int GetBestTMMatch
(
  PTAINPUT     pTAIn,                  // ta instance data
  PSZ_W        pszSegment,             // pointer to segmentboundary
  std::vector<OtmMemory *> &SearchMemories,     // pointer to tm's
  std::vector<OtmProposal *> &BestMatch,     // pointer to found proposals
  BOOL         * pfExactMatch,         // exact match found?
  USHORT       usNumMatches            // number of matches to be
)
{                                      // filled in matchtable
   SHORT     sIndex;       // index to tm's
   BOOL      fOK = TRUE;   // success indicator
   int       iReturn = NO_ERROR;     // return from tm functions
   PTAINSTDATA  pInD = pTAIn->pInD;     // ta instance data
   BOOL      fDone = FALSE;
   MemoryFactory *pFactory = MemoryFactory::getInstance();
   OtmProposal *pSearchKey = new(OtmProposal);
   PANALYSISMEMLOOKUPDATA pLookup = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;
   int iOptions = 0;

   *pfExactMatch = FALSE;
   sIndex = 0;

   // init tm data
   OtmProposal::clearAllProposals( BestMatch );
   pSearchKey->clear();

   pSearchKey->setDocShortName( pInD->pszCurSourceFile );

   if ( pTAIn->fIgnorePath )
   {
     PSZ pszStartOfName = strrchr( pTAIn->apszLongNames[pInD->usCurNumSourceFile-1],
                                   BACKSLASH );
     if ( pszStartOfName != NULL )
     {
       pSearchKey->setDocName( pszStartOfName + 1 );
     }
     else
     {
       pSearchKey->setDocName( pTAIn->apszLongNames[pInD->usCurNumSourceFile-1] );
     } /* endif */
   }
   else
   {
     pSearchKey->setDocName( pTAIn->apszLongNames[pInD->usCurNumSourceFile-1] );
   } /* endif */

   pSearchKey->setSegmentNum( pInD->ulSegNum );
   pSearchKey->setSource( pszSegment );

   // always run in non-generic replace mode and ensure that tags are
   // not removed as this functions is called
   // only to retrieve proposals from additional search TMs and the
   // segment will be written to the folder/document TM and
   // should not be converted in any way
   iOptions |= GET_NO_GENERICREPLACE;
   iOptions |= GET_ALWAYS_WITH_TAGS;

   if ( pTAIn->fRespectCRLF ||
        (pInD->pLoadedTable && (pInD->pLoadedTable->fReflow == TAGREFLOW_NO)))
   {
     iOptions  |= GET_RESPECTCRLF;
   } /* endif */

   if ( pTAIn->fIgnoreComment )
   {
     iOptions  |= GET_IGNORE_COMMENT;
   } /* endif */

   pSearchKey->setMarkup( pInD->szDocFormat );
   pSearchKey->setSourceLanguage( pInD->szDocSourceLang );
   pSearchKey->setTargetLanguage( pInD->szDocTargetLang );

   pSearchKey->setContext( pInD->szContext );

   while (fOK && (sIndex < (SHORT)SearchMemories.size()) && !fDone )
   {
      // get  best matches from translation memory
      iReturn = SearchMemories[sIndex]->searchProposal( *pSearchKey, pLookup->FoundProposals, iOptions );

      fOK = (iReturn == NO_ERROR);

      if (fOK)
      {
        pFactory->copyBestMatches( pLookup->FoundProposals, BestMatch, usNumMatches );

        if ( pLookup->FoundProposals[0]->isExactMatch() )
         {
            *pfExactMatch = TRUE;
         } /* endif */

      } /* endif */
      sIndex ++;

     // check if we can leave the lookup loop
     if ( pTAIn->fStopAtFirstExactAddMem )
     {
       if ( pLookup->FoundProposals[0]->isExactMatch() )
       {
         // we have an exact match, ignore matches from other memories
         fDone = TRUE;
       } /* endif */          
     } /* endif */        

   } /* endwhile */

   delete( pSearchKey );

   return (iReturn);
} /* end of GetBestTMMatches */


/**********************************************************************/
/* Free document structure                                            */
/**********************************************************************/
VOID TAFreeDoc( PVOID *ppvDoc )
{
  SegFileFreeDoc( ppvDoc );
} /* end of function TAFreeDoc */






/*----------------------------------------------------------------------------*\
| TARedundCount: Count redundant segments in temporary TM                     |
\*----------------------------------------------------------------------------*/
USHORT TARedundCount
(
  HWND        hwnd,                   // text analysis window handle
  PTAINPUT    pTAInput,               // TA data structure
  PUSHORT     pusPhase                // current processing phase
)
{
   PTAINSTDATA   pInD;                 // pointer to instance data for TA
   USHORT        usReturn;             // return values from functions
   TATASK        usPhase;              // activity sequencer
   BOOL          fOK = TRUE;           // Processing status flag

   hwnd;
   pInD = pTAInput->pInD;              // pointer to local variables

   usPhase = (TATASK)*pusPhase;

    switch ( usPhase )
    {
       case EQFTA_INIT:
            fOK = TRUE;

            pTAInput->sSliderPos = 0;

            if ( !pTAInput->fBatch )
            {
              // reset slider
              WinSendMsg( pTAInput->hwndProcWin, WM_EQF_UPDATESLIDER,
                          MP1FROMSHORT(0), NULL );
            } /* endif */

            // reset segment adress
            memset( &(pInD->stExtIn), 0, sizeof(pInD->stExtIn) );

            // activate TM source language
            usReturn = MorphGetLanguageID( pInD->szTMLang, &(pInD->sTMLangID) );

            if ( usReturn != NO_ERROR )
            {
              PSZ pszErrParm = pInD->szTMLang;
              usReturn = UtlErrorHwnd( ERROR_TA_DICTACT, MB_CANCEL, 1,
                                       &pszErrParm, EQF_WARNING,
                                       pTAInput->hwndErrMsg );
              fOK = FALSE;
            } /* endif */


            usPhase = ( fOK ) ? EQFTA_SEG1 : EQFTA_ABORT;
            break;


       case EQFTA_SEG1:
         {
           // retrieve current segment and update counters
           PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;
           OtmProposal prop;

           // get the next proposal from the memory
           if (  pInD->stExtIn.ulTmKey == 0L )
           {
             usReturn = (USHORT)pLookupData->pRedundCountMem->getFirstProposal( prop );
             pInD->stExtIn.ulTmKey = 1L;
           }
           else
           {
             usReturn = (USHORT)pLookupData->pRedundCountMem->getNextProposal( prop );
           } /* endif */

           // count segment data
           if ( usReturn == NO_ERROR )
           {
             SHORT sDocIndex;          // index of document into count table
             ULONG ulMarkUp;           // number of markup words in segment

             // copy some values from the proposal to our old data areas
             prop.getDocName( pInD->stExtOut.stTmExt.szLongName, sizeof( pInD->stExtOut.stTmExt.szLongName ) );
             prop.getMarkup( pInD->stExtOut.stTmExt.szTagTable, sizeof(pInD->stExtOut.stTmExt.szTagTable) );
             prop.getSource( pInD->stExtOut.stTmExt.szSource, sizeof(pInD->stExtOut.stTmExt.szSource)/sizeof(CHAR_W) );

             // get document index into count result table
             sDocIndex = TADocNameToIndex( pTAInput, pInD->stExtOut.stTmExt.szLongName );
             if ( sDocIndex < 0 ) usReturn = ERROR_NOT_ENOUGH_MEMORY;

             // handle new or same segment
             if ( usReturn == NO_ERROR )
             {
               // segment is a new one if markup or text differs
               if ( (strcmp( pInD->szCurMarkup,
                             pInD->stExtOut.stTmExt.szTagTable ) != 0) ||
                    (UTF16strcmp( pInD->szLastSeg,
                             pInD->stExtOut.stTmExt.szSource ) != 0) )
               {
                 // a new segment

                 // process segment used table
                 if ( pInD->szLastSeg[0] != EOS )
                 {
                    TAProcessSegmentUseCount( pTAInput );
                    memset( pInD->psUsed, 0, sizeof(SHORT) * pInD->sNumOfUseEntries );
                    pInD->lFrequency = 0;
                    memset( pInD->aCurDocs, 0, sizeof(pInD->aCurDocs) );
                 } /* endif */

                 // increment segment use count for document
                 pInD->psUsed[sDocIndex] += 1;

                 // remember new segment
                 UTF16strcpy( pInD->szLastSeg, pInD->stExtOut.stTmExt.szSource );

                 // load tag table if not active yet
                 if ( strcmp( pInD->szCurMarkup,
                              pInD->stExtOut.stTmExt.szTagTable ) != 0 )
                 {
                   if ( pInD->pCurTable != NULL)
                   {
                      TAFreeTagTable( pInD->pCurTable );
                     pInD->pCurTable = NULL;
                   } /* endif */
                    strcpy( pInD->szCurMarkup,
                            pInD->stExtOut.stTmExt.szTagTable );
                   usReturn = TALoadTagTable( pInD->szCurMarkup,
                                              &(pInD->pCurTable),
                                              FALSE, TRUE );
                 } /* endif */

                 // count words in segment
                 if ( usReturn == NO_ERROR )
                 {
                   ULONG  ulMTLangOEMCP = 0L;
                    ulMTLangOEMCP = GetLangOEMCP(pInD->szTMLang);

                    pInD->ulWords = 0L;
                    ulMarkUp = 0L;
                    usReturn = EQFBWordCntPerSeg( pInD->pCurTable,
                                 pInD->pTokBuf,
                                 pInD->stExtOut.stTmExt.szSource,
                                 pInD->sTMLangID,
                                 &(pInD->ulWords), &ulMarkUp, ulMTLangOEMCP);
                 } /* endif */
               }
               else
               {
                 // same segment continues

                 // increment segment use count for document
                 pInD->psUsed[sDocIndex] += 1;
               } /* endif */

               // increment segment frequency
               pInD->lFrequency++;

               // add document to document list if this is a new
               // document and there is still room in document table
               {
                 SHORT sDoc = 0;
                 while ( sDoc < MAX_MOSTUSEDDOCSPERSEG )
                 {
                   // check for empty slots (sSegNo == 0)
                   if ( pInD->aCurDocs[sDoc].ulSegNo == 0) // empty slot?
                   {
                     // store document in slot
                     pInD->aCurDocs[sDoc].ulSegNo = prop.getSegmentNum();
                     pInD->aCurDocs[sDoc].sDocIndex = sDocIndex;
                     break;            // leave loop, document is stored
                   }
                   else if ( pInD->aCurDocs[sDoc].sDocIndex == sDocIndex )
                   {
                     break;            // leave loop, document already in table
                   } /* endif */
                   sDoc++;             // next slot
                 } /* endwhile */
               }
             } /* endif */
           } /* endif */

           // update slider position
           if ( !pTAInput->fBatch )
           {
             ULONG ulNewPos;
             if ( pInD->stExtOut.ulMaxEntries )
             {
               ulNewPos = pInD->stExtOut.ulTmKey * 100 /
                          pInD->stExtOut.ulMaxEntries;
             }
             else
             {
               ulNewPos = 100;
             } /* endif */

             if ( ulNewPos != pInD->ulLastSliderPos )
             {
                WinSendMsg( pTAInput->hwndProcWin,
                            WM_EQF_UPDATESLIDER,
                            MP1FROMSHORT((USHORT)ulNewPos),
                            MP2FROMP(NULL) );
                pInD->ulLastSliderPos = ulNewPos;
             } /* endif */
           } /* endif */

           if ( usReturn == NO_ERROR )
           {
             usPhase =  EQFTA_SEG1;
           }
           else if ( usReturn == OtmMemory::INFO_ENDREACHED )
           {
             usPhase =  EQFTA_COMPLETE;
           }
           else
           {
             usPhase =  EQFTA_ABORT;
           } /* endif */
         }
         break;

       case EQFTA_COMPLETE:
           // process current use counts
           usReturn = TAProcessSegmentUseCount( pTAInput );

           // write redundance counting file
           if ( usReturn == NO_ERROR )
           {
             usReturn = TAWriteRedundCountFile( pTAInput );
           } /* endif */
           // continue with free of resources
           usPhase =  EQFTA_FREE;
           break;

       case EQFTA_FREE:
          // close and delete 
          {
            PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;
            MemoryFactory *pFactory = MemoryFactory::getInstance();

            // close TM
            if ( pLookupData->pRedundCountMem != NULL  )
            {
#ifdef SESSIONLOG
                UtlLogWriteString( "Analysis: Closing redundancy memory %s", pInD->szRedundCountTM );
#endif
                pFactory->closeTempMemory( pLookupData->pRedundCountMem );
                pLookupData->pRedundCountMem = NULL;
            } /* endif */

            // free redundancy counting specific memory areas
            if ( pInD->pNames )
            {
              UtlAlloc( (PVOID *)&pInD->pNames, 0L, 0L, NOMSG );
            } /* endif */

            if ( pInD->pDocCounts )
            {
              int i;
              for ( i = 0; i < pInD->sNumOfDocs; i++ )
              {
                if ( pInD->pDocCounts[i].pCrossRefs )
                {
                  UtlAlloc( (PVOID *)&(pInD->pDocCounts[i].pCrossRefs), 0L, 0L, NOMSG );
                } /* endif */
              } /* endfor */
              UtlAlloc( (PVOID *)&(pInD->pDocCounts), 0L, 0L, NOMSG );
            } /* endif */

            if ( pInD->pTokBuf )
            {
              UtlAlloc( (PVOID *)&pInD->pTokBuf, 0L, 0L, NOMSG );
            } /* endif */

            if ( pInD->psUsed )
            {
              UtlAlloc( (PVOID *)&pInD->psUsed, 0L, 0L, NOMSG );
            } /* endif */

            if ( pInD->pCurTable )
            {
              TAFreeTagTable( pInD->pCurTable );
              pInD->pCurTable = NULL;
            } /* endif */

            if ( pInD->sTMLangID >= 0 )
            {
              MorphFreeLanguageID( pInD->sTMLangID );
            } /* endif */

            if ( pInD->pMostUsedSegs )
            {
              UtlAlloc( (PVOID *)&pInD->pMostUsedSegs, 0L, 0L, NOMSG );
            } /* endif */

          }
          usPhase =  EQFTA_DONE;
          break;


       default:
       case EQFTA_ABORT:
          pInD->usError = TAERROR;
          usPhase =  EQFTA_FREE;       // free resources
          break;

       } /* end task selection */

      // if user requested to stop the global variable kill is set
      // so return always TACOMPL
      *pusPhase = (USHORT)usPhase;
      return (TACOMPL);

} /* end of function TARedundCount */

//+----------------------------------------------------------------------------+
//|Function name: TADocNameToIndex                                             |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


// convert document name to table index and enlarge document arrays
SHORT TADocNameToIndex
(
  PTAINPUT    pTAInput,                // TA data structure
  PSZ         pszDocName               // document name
)
{
  PTAINSTDATA   pInD;                 // pointer to instance data for TA
  PTANAMETABLEENTRY pEntry;           // pointer to found or new entry
  SHORT       sIndex = -1;            // index of document
  BOOL        fOK = TRUE;             // internal O.K. flag

  pInD = pTAInput->pInD;

  // lookup name in table
  if ( pInD->sNumOfNames )
  {
    static TANAMETABLEENTRY SearchEntry;
    strcpy( SearchEntry.szName, pszDocName );
    pEntry = (PTANAMETABLEENTRY)bsearch( &SearchEntry, pInD->pNames,
                                         pInD->sNumOfNames,
                                         sizeof(TANAMETABLEENTRY),
                                         TACompNames );
  }
  else
  {
    pEntry = NULL;
  } /* endif */

  // if not found add name as new entry to table else retrieve index
  if ( pEntry != NULL )
  {
    sIndex = pEntry->sID;
  }
  else
  {
    // enlarge table
    LONG lOldSize = sizeof(TANAMETABLEENTRY) * pInD->sNumOfNames;
    LONG lNewSize = lOldSize + sizeof(TANAMETABLEENTRY);
    fOK = UtlAlloc( (PVOID *)&(pInD->pNames), lOldSize, lNewSize, ERROR_STORAGE );

    // add new entry
    if ( fOK )
    {
      pEntry = pInD->pNames + pInD->sNumOfNames;
      strcpy( pEntry->szName, pszDocName );
      pEntry->sID = pInD->sNumOfNames;
      sIndex = pInD->sNumOfNames;
      pInD->sNumOfNames += 1;
    } /* endif */

    // sort name table
    if ( fOK && (pInD->sNumOfNames > 1) )
    {
      qsort( pInD->pNames, pInD->sNumOfNames, sizeof(TANAMETABLEENTRY),
             TACompNames );
    } /* endif */

    // enlarge psUsed table if necessary
    if ( fOK && (pInD->sNumOfUseEntries < pInD->sNumOfNames) )
    {
      lOldSize = sizeof(SHORT) * pInD->sNumOfUseEntries;
      lNewSize = lOldSize + (20 * sizeof(SHORT) );
      fOK = UtlAlloc( (PVOID *)&(pInD->psUsed), lOldSize, lNewSize, ERROR_STORAGE );
      if ( fOK )
      {
        pInD->sNumOfUseEntries += 20;
      } /* endif */
    } /* endif */

    // enlarge document array if necessary
    if ( fOK )
    {
      LONG lOldLen = pInD->sNumOfDocs * sizeof(TADOCCOUNTS);
      LONG lNewLen = lOldLen + sizeof(TADOCCOUNTS);
      if ( UtlAlloc( (PVOID *)&(pInD->pDocCounts), lOldLen,
                     lNewLen, ERROR_STORAGE ) )
      {
        strcpy( pInD->pDocCounts[sIndex].szDocName, pszDocName );
        pInD->sNumOfDocs += 1;
      }
      else
      {
        fOK = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */

  // return index or error condition
  if ( !fOK  )
  {
    sIndex = -1;
  } /* endif */
  return( sIndex );
} /* end of function TADocNameToIndex */

//+----------------------------------------------------------------------------+
//|Function name: TACompNames                                                  |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+


// compare two names in a name table
int TACompNames
(
  const void * pEntry1,
  const void * pEntry2
)
{
  return( strcmp( ((PTANAMETABLEENTRY)pEntry1)->szName,
                  ((PTANAMETABLEENTRY)pEntry2)->szName ));
} /* end of function TACompNames */


//+----------------------------------------------------------------------------+
//|Function name: TACompMostUsedSeg      Compare two entries of most used array|
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
int TACompMostUsedSeg
(
  const void * pEntry1,
  const void * pEntry2
)
{
  return( ((PTAMOSTUSEDSEG)pEntry2)->lFrequency -
          ((PTAMOSTUSEDSEG)pEntry1)->lFrequency );
} /* end of function TACompMostUsedSeg */


//+----------------------------------------------------------------------------+
//|Function name: TAProcessSegmentUseCount                                     |
//+----------------------------------------------------------------------------+
//|Function call:  called once per segment                                     |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
// process segment used in document table and update document
// redundancy counts
USHORT TAProcessSegmentUseCount
(
  PTAINPUT    pTAInput                // TA data structure
)
{
  PTAINSTDATA   pInD;                 // pointer to instance data for TA
  USHORT  usRC = NO_ERROR;
  int i;

  pInD = pTAInput->pInD;

  // store segment data in UniqueCount structure (if it is used more than once)
  //--------------------------------------------
  if ( pInD->lFrequency > 1 )
  {
    TAAddCounts( &(pInD->UniqueCount),1, pInD->ulWords);
  } /* endif */

  //
  // GQ 2004/07/30
  //
  // Compute to-be-translated count and redundand-count
  //
  // segment has to be translated one time, so add it to the to-be-translated counts
  TAAddCounts( &(pInD->ToBeTranslCount),1, pInD->ulWords);

  // all additional occurences of the segment are redundand
  if ( pInD->lFrequency > 1 )
  {
    int i = 0;
    for( i = 1; i < pInD->lFrequency;i++ )
    {
      TAAddCounts( &(pInD->RedundandCount),1, pInD->ulWords);
    } /* endfor */
  } /* endif */

  // Add segment to most used segment table if its frequency is
  // above the threshhold value
  if ( pInD->lFrequency >= pInD->sMaxMostUsedThreshhold )
  {
    SHORT sSlotNo = 0;                 // number of current slot
    SHORT sReplaceSlot = -1;           // slot to be replaced if none is free

    // look for an empty slot or for a slot with a smaller frequency
    while ( (pInD->pMostUsedSegs[sSlotNo].lFrequency != 0) &&
            (sSlotNo < pInD->sMaxMostUsedSegments) )
    {
      // check frequency of current slot
      if ( pInD->pMostUsedSegs[sSlotNo].lFrequency < pInD->lFrequency )
      {
        if ( sReplaceSlot == -1 )        // no replace slot yet
        {
          // use current as replace slot
          sReplaceSlot = sSlotNo;
        }
        else
        {
          // use current slot as replace slot if its frequency is
          // smaller than the frequency of the current replace slot
          if ( pInD->pMostUsedSegs[sSlotNo].lFrequency <
               pInD->pMostUsedSegs[sReplaceSlot].lFrequency )
          {
            sReplaceSlot = sSlotNo;
          } /* endif */
        } /* endif */
      } /* endif */

      sSlotNo++;                       // continue with next slot
    } /* endwhile */

    // use replace slot if no empty slot is available
    if ( (sSlotNo >= pInD->sMaxMostUsedSegments) && (sReplaceSlot != -1) )
    {
      sSlotNo = sReplaceSlot;
    } /* endif */

    // add segment and documents/segnum pairs to found slot (if any)
    if ( sSlotNo < pInD->sMaxMostUsedSegments )
    {
      UTF16strcpy( pInD->pMostUsedSegs[sSlotNo].szSegData, pInD->szLastSeg );
      pInD->pMostUsedSegs[sSlotNo].lFrequency = pInD->lFrequency;
      memcpy( pInD->pMostUsedSegs[sSlotNo].aDocs, pInD->aCurDocs,
              sizeof(pInD->pMostUsedSegs[sSlotNo].aDocs) );
    } /* endif */
  } /* endif */

  // GQ: new way of updating the document cross reference table
  for ( i = 0; ((i < pInD->sNumOfNames) && (usRC == NO_ERROR)); i++ )
  {
    // if segment is used in document i ...
    if ( pInD->psUsed[i] )
    {
      int j = 0;
      int iCrossRefIndex = 0;
      PTADOCCOUNTS pDocCounts = NULL;
      PTADOCCROSSREF pCrossRef = NULL;

      // loop over all document using the same segment...
      for ( j = 0; ((j < pInD->sNumOfNames) && (usRC == NO_ERROR)); j++ )
      {
        if ( pInD->psUsed[j] )
        {
          ULONG ulSegs = 0;
          ULONG ulWords = 0;

          // get number of redundant segments and words
          if ( i == j )                // same document ???
          {
             // for internal redundancy: reduce segment count by one
             ulSegs  = pInD->psUsed[j] - 1;
             ulWords = ulSegs * pInD->ulWords;
          }
          else
          {
             ulSegs  = pInD->psUsed[j];
             ulWords = ulSegs * pInD->ulWords;
          } /* endif */

          // find cross reference entry for document j
          pDocCounts = &(pInD->pDocCounts[i]);
          pCrossRef = NULL;
          iCrossRefIndex = 0;
          for ( iCrossRefIndex = 0; iCrossRefIndex < pDocCounts->sNumOfDocs; iCrossRefIndex++ )
          {
            if ( pDocCounts->pCrossRefs[iCrossRefIndex].sDocNumber == j  )
            {
              // found the correct entry
              pCrossRef = &(pDocCounts->pCrossRefs[iCrossRefIndex]);
              break;
            } /* endif */               
          } /* endfor */             

          // add entry for document if no entry exists 
          if ( pCrossRef == NULL )
          {
            // enlarge table when necessary
            if ( pDocCounts->sNumOfDocs >= pDocCounts->sNumOfEntries )
            {
              LONG lOldSize = sizeof(TADOCCROSSREF) * pDocCounts->sNumOfEntries;
              LONG lNewSize = sizeof(TADOCCROSSREF) * (pDocCounts->sNumOfEntries + 100);
              if ( UtlAlloc( (PVOID *)&(pDocCounts->pCrossRefs), lOldSize, lNewSize, ERROR_STORAGE ) )
              {
                pDocCounts->sNumOfEntries += 100;
              }
              else
              {
                usRC = ERROR_NOT_ENOUGH_MEMORY;
              } /* endif */
            } /* endif */    

            if ( usRC == 0 )
            {
              pCrossRef = &(pDocCounts->pCrossRefs[pDocCounts->sNumOfDocs]);
              pCrossRef->sDocNumber = (SHORT)j;
              pDocCounts->sNumOfDocs += 1;
            } /* endif */               
          } /* endif */             

          // update counts for cross reference entry j
          if ( pCrossRef != NULL  )
          {
            if ( pInD->ulWords < (ULONG)SIMPLE_SENT_BOUND )
            {
              pCrossRef->CountSums.ulSimpleSegs  += ulSegs;
              pCrossRef->CountSums.ulSimpleWords += ulWords;
            }
            else if ( pInD->ulWords < (ULONG)MEDIUM_SENT_BOUND )
            {
              pCrossRef->CountSums.ulMediumSegs  += ulSegs;
              pCrossRef->CountSums.ulMediumWords += ulWords;
            }
            else
            {
              pCrossRef->CountSums.ulComplexSegs  += ulSegs;
              pCrossRef->CountSums.ulComplexWords += ulWords;
            } /* endif */
          } /* endif */             
        } /* endif */
      } /* endfor */
    } /* endif */       
  } /* endfor */

#ifdef XXXXXXXXXXX
  // loop through segment used table and update document counts
  for ( i = 0; ((i < pInD->sNumOfNames) && (usRC == NO_ERROR)); i++ )
  {
    // if segment is used in document i ...
    if ( pInD->psUsed[i] )
    {
      int iMaxDocInd;

      // ... update all other documents for this segment

      // get max document index
      iMaxDocInd = pInD->sNumOfNames - 1;
      while ( (iMaxDocInd > 0) && (pInD->psUsed[iMaxDocInd] == 0) )
      {
        iMaxDocInd--;
      } /* endwhile */

      // realloc counts array of document if necessary
      if ( (iMaxDocInd + 1) > pInD->pDocCounts[i].sNumOfDocs )
      {
       LONG lOldSize = sizeof(COUNTSUMS) * pInD->pDocCounts[i].sNumOfDocs;
       LONG lNewSize = sizeof(COUNTSUMS) * (iMaxDocInd + 1);
       if ( UtlAlloc( (PVOID *)&(pInD->pDocCounts[i].pCountArray),
                       lOldSize, lNewSize, ERROR_STORAGE ) )
       {
         pInD->pDocCounts[i].sNumOfDocs = (SHORT)(iMaxDocInd + 1);
       }
       else
       {
         usRC = ERROR_NOT_ENOUGH_MEMORY;
       } /* endif */
      } /* endif */

      // update document counts array
      if ( usRC == NO_ERROR )
      {
        int j;

        for ( j = 0; j <= iMaxDocInd; j++ )
        {
          ULONG ulWords, ulSegs;

          // get number of redundant segments and words
          if ( i == j )                // same document ???
          {
             // for internal redundancy: reduce segment count by one
             ulSegs  = pInD->psUsed[j] - 1;
             ulWords = ulSegs * pInD->ulWords;
          }
          else
          {
             ulSegs  = pInD->psUsed[j];
             ulWords = ulSegs * pInD->ulWords;
          } /* endif */

          // update count values
          if ( pInD->ulWords < (ULONG)SIMPLE_SENT_BOUND )
          {
            pInD->pDocCounts[i].pCountArray[j].ulSimpleSegs  += ulSegs;
            pInD->pDocCounts[i].pCountArray[j].ulSimpleWords += ulWords;
          }
          else if ( pInD->ulWords < (ULONG)MEDIUM_SENT_BOUND )
          {
            pInD->pDocCounts[i].pCountArray[j].ulMediumSegs  += ulSegs;
            pInD->pDocCounts[i].pCountArray[j].ulMediumWords += ulWords;
          }
          else
          {
            pInD->pDocCounts[i].pCountArray[j].ulComplexSegs  += ulSegs;
            pInD->pDocCounts[i].pCountArray[j].ulComplexWords += ulWords;
          } /* endif */
        } /* endfor */
      } /* endif */
    } /* endif */
  } /* endfor */
#endif

  return( usRC );
} /* end of function TAProcessSegmentUseCount */

//+----------------------------------------------------------------------------+
//|Function name: TAWriteRedundCountFile  write redundancy counts to file      |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
USHORT TAWriteRedundCountFile
(
  PTAINPUT    pTAInput                // TA data structure
)
{
  PTAINSTDATA pInD;                    // pointer to instance data for TA
  USHORT      usRC = NO_ERROR;
  CHAR        szOutFile[MAX_EQF_PATH]; // name of output file
  HFILE       hOutFile = NULLHANDLE;   // handle of output file
  REDCOUNTHEADER Header;

  pInD = pTAInput->pInD;

  //
  // GQ 2004/07/30
  //
  // Write redundand log file if requested
  {
    CHAR szFileName[MAX_LONGPATH];

    // build trigger file name
    UtlMakeEQFPath( szFileName, NULC, SYSTEM_PATH, NULL );
    strcat( szFileName, "\\REDUND.TRG" );

    // write log data if trigger file exists
    if ( UtlFileExist( szFileName ) )
    {
      FILE *hFile = NULL;

      UtlMakeEQFPath( szFileName, NULC, LOG_PATH, NULL );
      strcat( szFileName, BACKSLASH_STR );
      SubFolObjectNameToName( pTAInput->szFolder, szFileName + strlen(szFileName) );
      strcat( szFileName, ".REDUNDANCE.LOG" );

      hFile = fopen( szFileName, "w" );
      if ( hFile )
      {
        int i = 0;                                                                                                  // document index

        // header
        fprintf( hFile, "                           Simple                      Medium                      Complex                     Total    \n" );
        fprintf( hFile, "Category          Segments        Words       Segments        Words       Segments        Words       Segments        Words\n" );
        fprintf( hFile, "===========   ============ ============   ============ ============   ============ ============   ============ ============\n" );

        // document counts
        while ( (i < pInD->sNumOfDocs) && (usRC == NO_ERROR) )
        {
          PTADOCCROSSREF pCrossRef = pInD->pDocCounts[i].pCrossRefs;
          int j = 0;

          fprintf( hFile, "\nDocument %s\n", pInD->pDocCounts[i].szDocName );
          fprintf( hFile, "Exact         %12lu %12lu   %12lu %12lu   %12lu %12lu   %12lu %12lu\n",
            pInD->pDocCounts[i].CountExact.ulSimpleSegs, pInD->pDocCounts[i].CountExact.ulSimpleWords,
            pInD->pDocCounts[i].CountExact.ulMediumSegs, pInD->pDocCounts[i].CountExact.ulMediumWords,
            pInD->pDocCounts[i].CountExact.ulComplexSegs, pInD->pDocCounts[i].CountExact.ulComplexWords,
            pInD->pDocCounts[i].CountExact.ulSimpleSegs + pInD->pDocCounts[i].CountExact.ulMediumSegs + pInD->pDocCounts[i].CountExact.ulComplexSegs,
            pInD->pDocCounts[i].CountExact.ulSimpleWords + pInD->pDocCounts[i].CountExact.ulMediumWords + pInD->pDocCounts[i].CountExact.ulComplexWords);
          fprintf( hFile, "Total         %12lu %12lu   %12lu %12lu   %12lu %12lu   %12lu %12lu\n",
            pInD->pDocCounts[i].CountTotal.ulSimpleSegs, pInD->pDocCounts[i].CountTotal.ulSimpleWords,
            pInD->pDocCounts[i].CountTotal.ulMediumSegs, pInD->pDocCounts[i].CountTotal.ulMediumWords,
            pInD->pDocCounts[i].CountTotal.ulComplexSegs, pInD->pDocCounts[i].CountTotal.ulComplexWords,
            pInD->pDocCounts[i].CountTotal.ulSimpleSegs + pInD->pDocCounts[i].CountTotal.ulMediumSegs + pInD->pDocCounts[i].CountTotal.ulComplexSegs,
            pInD->pDocCounts[i].CountTotal.ulSimpleWords + pInD->pDocCounts[i].CountTotal.ulMediumWords + pInD->pDocCounts[i].CountTotal.ulComplexWords);

          // write countsums
          while ( (j < pInD->pDocCounts[i].sNumOfDocs) && (usRC == NO_ERROR) )
          {
            PCOUNTSUMS pCounts = &(pCrossRef[j].CountSums);
            fprintf( hFile, "Cross[%4.4d]   %12lu %12lu   %12lu %12lu   %12lu %12lu   %12lu %12lu\n",
              pCrossRef[j].sDocNumber,
                pCounts->ulSimpleSegs, pCounts->ulSimpleWords,
                pCounts->ulMediumSegs, pCounts->ulMediumWords,
                pCounts->ulComplexSegs, pCounts->ulComplexWords,
                pCounts->ulSimpleSegs + pCounts->ulMediumSegs + pCounts->ulComplexSegs,
                pCounts->ulSimpleWords + pCounts->ulMediumWords + pCounts->ulComplexWords);
            j++;
          } /* endwhile */

          i++;
        } /* endwhile */

        fprintf( hFile, "ToBeTransl    %12lu %12lu   %12lu %12lu   %12lu %12lu   %12lu %12lu\n",
          pInD->ToBeTranslCount.ulSimpleSegs, pInD->ToBeTranslCount.ulSimpleWords,
          pInD->ToBeTranslCount.ulMediumSegs, pInD->ToBeTranslCount.ulMediumWords,
          pInD->ToBeTranslCount.ulComplexSegs, pInD->ToBeTranslCount.ulComplexWords,
          pInD->ToBeTranslCount.ulSimpleSegs + pInD->ToBeTranslCount.ulMediumSegs + pInD->ToBeTranslCount.ulComplexSegs,
          pInD->ToBeTranslCount.ulSimpleWords + pInD->ToBeTranslCount.ulMediumWords + pInD->ToBeTranslCount.ulComplexWords);
        fprintf( hFile, "Redundand     %12lu %12lu   %12lu %12lu   %12lu %12lu   %12lu %12lu\n",
          pInD->RedundandCount.ulSimpleSegs, pInD->RedundandCount.ulSimpleWords,
          pInD->RedundandCount.ulMediumSegs, pInD->RedundandCount.ulMediumWords,
          pInD->RedundandCount.ulComplexSegs, pInD->RedundandCount.ulComplexWords,
          pInD->RedundandCount.ulSimpleSegs + pInD->RedundandCount.ulMediumSegs + pInD->RedundandCount.ulComplexSegs,
          pInD->RedundandCount.ulSimpleWords + pInD->RedundandCount.ulMediumWords + pInD->RedundandCount.ulComplexWords);
        fprintf( hFile, "-----------   ------------ ------------   ------------ ------------   ------------ ------------\n" );
        fprintf( hFile, "Total         %12lu %12lu   %12lu %12lu   %12lu %12lu\n",
          pInD->RedundandCount.ulSimpleSegs   + pInD->ToBeTranslCount.ulSimpleSegs,
          pInD->RedundandCount.ulSimpleWords  + pInD->ToBeTranslCount.ulSimpleWords,
          pInD->RedundandCount.ulMediumSegs   + pInD->ToBeTranslCount.ulMediumSegs,
          pInD->RedundandCount.ulMediumWords  + pInD->ToBeTranslCount.ulMediumWords,
          pInD->RedundandCount.ulComplexSegs  + pInD->ToBeTranslCount.ulComplexSegs,
          pInD->RedundandCount.ulComplexWords + pInD->ToBeTranslCount.ulComplexWords);

        fclose( hFile );
      } /* endif */
    } /* endif */
  }

  memset(&Header, 0, sizeof(Header));
  // build fully qualified name of output file
  UtlMakeEQFPath( szOutFile, pTAInput->szFolder[0], PROPERTY_PATH,
                  UtlGetFnameFromPath( pTAInput->szFolder ) );
  strcat( szOutFile, BACKSLASH_STR );
  strcat( szOutFile, REDUNDCOUNTFILE );

  // open output file for write
  if ( usRC == NO_ERROR )
  {
    USHORT usOpenAction;               // action performed by DosOpen

    usRC = UtlOpen( szOutFile, &hOutFile, &usOpenAction, 0L,
                    FILE_NORMAL, FILE_OPEN | FILE_CREATE,
                    OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYWRITE,
                    0L, TRUE );
  } /* endif */

  // write output file header
  // ------------------------
  if ( usRC == NO_ERROR )
  {
    ULONG  ulWritten;                  // number of bytes written

    memset( &Header, 0, sizeof(Header) );
    Header.lDocuments = pInD->sNumOfDocs;
    UtlTime( &(Header.lTime) );
    Utlstrccpy( Header.szFolder, UtlGetFnameFromPath(pTAInput->szFolder), DOT );
//    Header.UniqueCount = pInD->RedundandCount;
    Header.UniqueCount = pInD->RedundandCount;
    usRC = UtlWriteL( hOutFile, &Header, sizeof(Header),
                         &ulWritten, TRUE );
  } /* endif */

  // write counts per document
  if ( usRC == NO_ERROR )
  {
    REDCOUNTDOC DocHead;                                                           // fixed part per document
    ULONG  ulWritten;                  // number of bytes written
    int i = 0;                                                                                                  // document index

    // allocate buffer for cross reference count sums of all documents in sorted order
    PCOUNTSUMS pDocCountSums = NULL;
    if ( !UtlAlloc( (PVOID *) &pDocCountSums, 0L, sizeof(COUNTSUMS) * pInD->sNumOfDocs, ERROR_STORAGE ) )
    {
      usRC = ERROR_NOT_ENOUGH_MEMORY;  
    } /* end */     

    while ( (i < pInD->sNumOfDocs) && (usRC == NO_ERROR) )
    {
      // fill fixed size part
      strcpy( DocHead.szDocName, pInD->pDocCounts[i].szDocName );
      strcpy( DocHead.szDocShortName, pInD->pDocCounts[i].szShortName );
      memcpy( &DocHead.TotalCount, &(pInD->pDocCounts[i].CountTotal),
                     sizeof(COUNTSUMS) );
      memcpy( &DocHead.ExactCount, &(pInD->pDocCounts[i].CountExact),
                     sizeof(COUNTSUMS) );
      DocHead.lSize = sizeof(DocHead) + (pInD->sNumOfDocs + sizeof(COUNTSUMS));

      // write fixed part
      usRC = UtlWriteL( hOutFile, &DocHead, sizeof(DocHead),
                       &ulWritten, TRUE );

      // write countsums
      if ( usRC == NO_ERROR )
      {
        PTADOCCROSSREF pCrossRef = NULL;
        int j = 0;

        memset( pDocCountSums, 0, sizeof(COUNTSUMS) * pInD->sNumOfDocs );

        // fill in document cross reference counts
        pCrossRef = pInD->pDocCounts[i].pCrossRefs;
        if ( pCrossRef != NULL )
        {
          for ( j = 0; j < pInD->pDocCounts[i].sNumOfDocs; j++ )
        {
            memcpy( &(pDocCountSums[pCrossRef[j].sDocNumber]), &(pCrossRef[j].CountSums), sizeof(COUNTSUMS) );
          } /* endfor */             
        } /* endif */           

        // write all doc counts
        j = 0;
        while ( (j < pInD->sNumOfDocs) && (usRC == NO_ERROR) )
        {
          usRC = UtlWriteL( hOutFile, &(pDocCountSums[j]), sizeof(COUNTSUMS), &ulWritten, TRUE );
          j++;
        } /* endwhile */
      } /* endif */

      i++;
    } /* endwhile */

    if ( pDocCountSums != NULL ) UtlAlloc( (PVOID *) &pDocCountSums, 0L, 0, NOMSG );
  } /* endif */

  // get file offset of most used segments area
  if ( usRC == NO_ERROR )
  {
    usRC = UtlChgFilePtr( hOutFile, 0L, FILE_CURRENT, (PULONG)(&Header.lMostUsedOffset), TRUE );
  } /* endif */

  // sort most used segments
  if ( usRC == NO_ERROR )
  {
    qsort( pInD->pMostUsedSegs, pInD->sMaxMostUsedSegments,
           sizeof(TAMOSTUSEDSEG), TACompMostUsedSeg );
  } /* endif */

  // write most used segments
  if ( usRC == NO_ERROR )
  {
    MOSTUSEDSEG MostUsedSeg;           // buffer for most used segments
    SHORT sSegNo = 0;
    PTAMOSTUSEDSEG pSeg = pInD->pMostUsedSegs;

    while ( (sSegNo < pInD->sMaxMostUsedSegments) &&
            (pSeg->lFrequency != 0L) && (usRC == NO_ERROR))
    {
      int i;
      ULONG ulWritten;                  // number of bytes written

      // fill most used segment entry
      memset( &MostUsedSeg, 0, sizeof(MostUsedSeg) );
      MostUsedSeg.lFrequency = pSeg->lFrequency;
      for ( i = 0; i < MAX_MOSTUSEDDOCSPERSEG; i++ )
      {
        MostUsedSeg.aDocs[i].sDocIndex = pSeg->aDocs[i].sDocIndex;
        MostUsedSeg.aDocs[i].ulSegNo    = pSeg->aDocs[i].ulSegNo;
      } /* endfor */
      UTF16strcpy( MostUsedSeg.szSegment, pSeg->szSegData );

      // write segment entry to output file
      usRC = UtlWriteL( hOutFile, &MostUsedSeg, sizeof(MostUsedSeg),
                       &ulWritten, TRUE );

      // next entry
      Header.lMostUsedSegments++;
      sSegNo++;
      pSeg++;
    } /* endwhile */
  } /* endif */

  // re-write header
  if ( usRC == NO_ERROR )
  {
    ULONG ulNewPos;
    usRC = UtlChgFilePtr( hOutFile, 0L, FILE_BEGIN, &ulNewPos, TRUE );

    if ( usRC == NO_ERROR )
    {
      ULONG  ulWritten;                  // number of bytes written
      usRC = UtlWriteL( hOutFile, &Header, sizeof(Header),
                       &ulWritten, TRUE );
    } /* endif */
  } /* endif */

  // close output file
  if ( hOutFile != NULLHANDLE )
  {
    UtlClose( hOutFile, FALSE );
  } /* endif */

  return( usRC );
} /* end of function TAWriteRedundCountFile */



//+----------------------------------------------------------------------------+
//|Function name: TAAddCounts                                                  |
//+----------------------------------------------------------------------------+
//|Function call:                                                              |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Description:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Input parameter:                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Output parameter:                                                           |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Returncode type:                                                            |
//+----------------------------------------------------------------------------+
//|Returncodes:                                                                |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Prerequesits:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Side effects:                                                               |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Samples:                                                                    |
//|                                                                            |
//+----------------------------------------------------------------------------+
//|Function flow:                                                              |
//|                                                                            |
//|                                                                            |
//|                                                                            |
//+----------------------------------------------------------------------------+

// add segs and words to correct category
USHORT TAAddCounts
(
  PCOUNTSUMS  pSum,
  LONG        lSegs,
  LONG        lWords
)
{
  LONG lAbsWords = abs( lWords );
  if ( lAbsWords < SIMPLE_SENT_BOUND )
  {
    pSum->ulSimpleSegs  += lSegs;
    pSum->ulSimpleWords += lWords;
  }
  else if ( lAbsWords < MEDIUM_SENT_BOUND )
  {
    pSum->ulMediumSegs  += lSegs;
    pSum->ulMediumWords += lWords;
  }
  else
  {
    pSum->ulComplexSegs  += lSegs;
    pSum->ulComplexWords += lWords;
  } /* endif */
  return( NO_ERROR );
} /* end of function TAAddCounts */


USHORT TACountMatch
(
  PTAINSTDATA pInD,                    // pointer to instance data for TA
  MATCHTYPE   MatchType,               // type of match
  USHORT      usFuzzyness,             // fuzzyness of match
  LONG        lSrcWords,               // number of source words
  SHORT       sSegments,               // number of segments
  BOOL        fTotal                   // TRUE = add to total word numner
)
{
#ifdef LOGMATCHES
   fwprintf( hMatchLogFile, L"TACountMatch, Seg=%-5lu MatchType=%-4d, Fuzzyness=%-4u, Words=%-4ld, Segs=%-2d\r\n",
            pInD->ulSegNum, (SHORT)MatchType, usFuzzyness, lSrcWords, sSegments );
#endif

   switch ( MatchType )
   {
     case NO_MATCH:
       pInD->ulNoProps += lSrcWords;
       pInD->ulSegNoProps += sSegments;
       break;
     case FUZZY_MATCH:
       pInD->ulFuzzy += lSrcWords;
       pInD->ulSegFuzzy += sSegments;
       break;
     case EXACTEXACT_MATCH:
       pInD->ulExactExact += lSrcWords;
       pInD->ulSegExactExact += sSegments;
       break;
     case EXACTONE_MATCH:
       pInD->ulExactOne += lSrcWords;
       pInD->ulSegExactOne += sSegments;
       break;
     case GLOBMEM_MATCH:
     case EXACTMORE_MATCH:
       pInD->ulExactMore += lSrcWords;
       pInD->ulSegExactMore += sSegments;
       break;
     case REPL_MATCH:
       pInD->ulRepl += lSrcWords;
       pInD->ulSegRepl += sSegments;
       break;
     case FUZZYREPL_MATCH:
       pInD->ulFuzzyRepl += lSrcWords;
       pInD->ulSegFuzzyRepl += sSegments;
       break;
     case MACHINE_MATCH:
       pInD->ulMachineMatch += lSrcWords;
       pInD->ulSegMachineMatch += sSegments;
       break;
   } /* endswitch */
   if ( fTotal )
   {
     pInD->ulTotal += lSrcWords;
   } /* endif */

   // count new fields
   switch ( MatchType )
   {
     case GLOBMEM_MATCH:
     case NO_MATCH:
       TAAddCounts( &pInD->NoProps,  sSegments, lSrcWords );
       break;
     case FUZZYREPL_MATCH:
     case FUZZY_MATCH:
       if( usFuzzyness >= (USHORT)(FUZZY_THRESHOLD_0 * 100))
       {
         if ( usFuzzyness <= (USHORT)(FUZZY_THRESHOLD_1 * 100))
         {
           TAAddCounts( &pInD->Fuzzy1, sSegments, lSrcWords );
         }
         else if ( usFuzzyness <= (USHORT)(FUZZY_THRESHOLD_2 * 100))
         {
           TAAddCounts( &pInD->Fuzzy2, sSegments, lSrcWords );
         }
         else
         {
           TAAddCounts( &pInD->Fuzzy3, sSegments, lSrcWords );
         }  /* end if fuzzyness */
       }
       else
       {
         // treat as no match
         TAAddCounts( &pInD->NoProps,  sSegments, lSrcWords );
       } /* endif */
       break;
     case EXACTONE_MATCH:
     case EXACTEXACT_MATCH:
       TAAddCounts( &pInD->ExactExact, sSegments, lSrcWords );
       break;
     case EXACTMORE_MATCH:
       TAAddCounts( &pInD->ExactMore, sSegments, lSrcWords );
       break;
     case REPL_MATCH:
       TAAddCounts( &pInD->Repl, sSegments, lSrcWords );
       break;
     case MACHINE_MATCH :
       TAAddCounts( &pInD->MTProps, sSegments, lSrcWords );
       break;
   } /* endswitch */
   if ( fTotal )
   {
     TAAddCounts( &pInD->Total, sSegments, lSrcWords );
   } /* endif */

   return( 0 );
}

// different adjustments of MatchType
// change match type if fuzziness is below current threshhold value
VOID TAAdjustMatchType
(
  MATCHTYPE   *pMatchType,             // type of match
  USHORT      usFuzzyness,             // fuzzyness of match
  LONG        lSrcWords                // number of source words
)
{ // Calculation of fuzzyness does is not yet implemented for Count Memory Match
  if (*pMatchType == FUZZY_MATCH && usFuzzyness == 0)
  {
     *pMatchType = NO_MATCH;
  }
  // change fuzzy to nomatch if the fuzzyness is below the current
  // threshhold
  if ( (*pMatchType == FUZZY_MATCH) || (*pMatchType == FUZZYREPL_MATCH) )
  {
    USHORT usFuzzyLevel = 0;          // buffer for fuzzyness level setting
    LONG   lWords = abs(lSrcWords);
    if ( lWords > 15 )
    {
      usFuzzyLevel = (USHORT)(UtlQueryULong( QL_LARGELOOKUPFUZZLEVEL ) / 100);
    }
    else if ( lWords > 4 )
    {
      usFuzzyLevel = (USHORT)(UtlQueryULong( QL_MEDIUMLOOKUPFUZZLEVEL ) / 100);
    }
    else
    {
      usFuzzyLevel = (USHORT)(UtlQueryULong( QL_SMALLLOOKUPFUZZLEVEL ) / 100);
    } /* endif */

    if ( usFuzzyness < usFuzzyLevel )
    {
      *pMatchType = NO_MATCH;
    } /* endif */
  } /* endif */
} /* end of function TAAdjustMatchType */

BOOL TAMachTransAddSeg
(
  PTAINPUT    pTAIn,
  ULONG       ulSrcWords,
  BOOL        fContainsTagging,
  MATCHTYPE   MatchType,
  USHORT      usFuzzyness,
  ULONG       ulSegNum,
  PSZ_W       pszSegSource,
  PTBSEGMENT  pSeg
)
{
 USHORT usFuzzyLimit;
 BOOL fSkipSeg = FALSE;
 BOOL fOK = TRUE;
 int  iCategory = 0;                   // category of segment

 fContainsTagging;

 // get fuzzy limit and category of segment
 if ( ulSrcWords < SIMPLE_SENT_BOUND )
 {
   usFuzzyLimit = pTAIn->stMTData.usSmallFuzzy;
   iCategory = 0;
 }
 else if ( ulSrcWords < MEDIUM_SENT_BOUND )
 {
   usFuzzyLimit = pTAIn->stMTData.usMediumFuzzy;
   iCategory = 1;
 }
 else
 {
   usFuzzyLimit = pTAIn->stMTData.usLargeFuzzy;
   iCategory = 2;
 } /* endif */

 // add to total segments/words processed
 pTAIn->fMTValuesAvailable = TRUE;  
 pTAIn->ulMTTotalWords[iCategory] += ulSrcWords;  
 pTAIn->ulMTTotalSegs[iCategory]  += 1;

 // apply fuzziness filter
 if ( usFuzzyLimit && (usFuzzyness > usFuzzyLimit) )
 {
   fSkipSeg = TRUE;
 } /* endif */

 // apply word limit filter
 if ( pTAIn->stMTData.usWordLimit && (ulSrcWords >= pTAIn->stMTData.usWordLimit) )
 {
   fSkipSeg = TRUE;
 } /* endif */

 // apply machine match filter
 if ( (MatchType == MACHINE_MATCH) && (usFuzzyness >= 95) )
 {
   fSkipSeg = TRUE;
 } /* endif */

 // currently no filtering for nFluent segments except for exact matches
 fSkipSeg = FALSE;

 if ( !fSkipSeg )
 {
   // ignore nocount segments
   fSkipSeg = pSeg->SegFlags.NoCount;
 } /* endif */

 if ( !fSkipSeg )
 {
    ULONG ulLength = UTF16strlenCHAR(pszSegSource);

    fOK =  TAWriteMTSegment( pTAIn, ulLength, pszSegSource, ulSegNum, usFuzzyness, MatchType, pSeg, ulSrcWords );

  } /* endif */
  return ( fOK );
}


void
TAFillWSList
(
	PSZ pszTagTableName,
	PSZ_W * ppWSList
)
{
	CHAR_W      szName_W[MAX_TAGSIZE];     // buffer for tag name
	PTAG        pTag;
	PTAGADDINFO pAddInfo;
	PTAGTABLE   pTagTable;
	BOOL        fOK = TRUE;
	PSZ_W       pszCurWS = NULL;
	int         i = 0;
	USHORT      usCount = 0;
	SHORT       sRc = NO_ERROR;
	PLOADEDTABLE pLoadedTable;
	USHORT       usTagCount = 0;
	PBYTE        pByte;
	PSZ          pTagNames = NULL;
	PSZ          pCurName = NULL;


    sRc = (SHORT) TALoadTagTable( pszTagTableName,
                                     (PLOADEDTABLE *)&pLoadedTable,
                                     FALSE, FALSE );
    if ( !sRc )
    {
	  pTagTable  = pLoadedTable->pTagTable;

	  pByte = (PBYTE) pTagTable;
	  pTag = (PTAG) (pByte + pTagTable->stFixTag.uOffset);
	  pTagNames = (PSZ) (pByte + pTagTable->uTagNames);
      usTagCount = pTagTable->uNumTags;

	  pAddInfo = (pTagTable->usVersion >= ADDINFO_VERSION) ?
						   ((PTAGADDINFO)((PBYTE)pTagTable + pTagTable->uAddInfos))  :
						   NULL;

	  for( i = 0; i < usTagCount; i++, pTag++)
	  {
		  if ( pAddInfo && (pAddInfo[i].ClassId== CLS_WHITESPACE)
			   && (pTag->usLength == 1))
		  {
			usCount++;
		  } /* endif */
	  } /* endfor */

      if (usCount)
      {
			// alloc array
		  fOK = UtlAlloc( (PVOID *) &(pszCurWS), 0L,
						  (LONG) (usCount+2) * sizeof(CHAR_W), ERROR_STORAGE);
          pTag = (PTAG) (pByte + pTagTable->stFixTag.uOffset);
		  pAddInfo = (pTagTable->usVersion >= ADDINFO_VERSION) ?
							   ((PTAGADDINFO)((PBYTE)pTagTable + pTagTable->uAddInfos))  :
							   NULL;

          * ppWSList = pszCurWS;

		  for( i = 0; i < usTagCount; i++ )
		  {
			  memset( &szName_W, 0, sizeof(szName_W) );

              pCurName = pTag[i].uTagnameOffs + pTagNames;
              if (pCurName)
              {
                ASCII2Unicode( pCurName, szName_W, 0L );
              }
			  if ( pAddInfo && (pAddInfo[i].ClassId== CLS_WHITESPACE)
				   && (pTag->usLength == 1))
			  {
				usCount--;
				*pszCurWS = szName_W[0];
				pszCurWS ++;
			  } /* endif */
		  } /* endfor */
      }
    } // endif
    // free pWSList - Analysis: if TAINput is freed at end of analysis
    // Editor auto-subst: if TBDOcument is freed at end of editing
    TAFreeTagTable( pLoadedTable );
    return;
 }

// nFluent format XML output functions

// hard-coded TM language to ISO language list
typedef struct _TM2ISOLANG
{
  CHAR szTMLang[40];
  CHAR szISOLang[10];
} TM2ISOLANG, *PTM2ISOLANG;

TM2ISOLANG TmToIsoLang[] = {
{ "English(U.S.)", "en-US" },
{ "English(U.K.)", "en-UK" },
{ "German(national)", "de-DE" },
{ "German(newnat)", "de-DE" },
{ "German(DPAnat)", "de-DE" },
{ "German(reform)", "de-DE" },
{ "German(Swiss)", "de-CH" },
{ "French(national)", "fr-FR" },
{ "French(Canadian)", "fr-CA" },
{ "Italian", "it" },
{ "Spanish", "es" },
{ "Dutch(restrictive)", "nl" },
{ "Dutch(permissive)", "nl" },
{ "Portuguese(Brasil)", "pt-BR" },
{ "Portuguese(nat.)", "pt-PT" },
{ "Catalan", "ca" },
{ "Danish", "da" },
{ "Icelandic", "is" },
{ "Norwegian(bokmal)", "no" },
{ "Norwegian(nynorsk)", "no" },
{ "Swedish", "sv" },
{ "Greek", "el" },
{ "Afrikaans", "af" },
{ "Finnish", "fi" },
{ "Polish", "pl" },
{ "Hungarian", "hu" },
{ "Czech", "cs" },
{ "Turkish", "tr" },
{ "Russian", "ru" },
{ "Japanese", "ja" },
{ "Chinese(simpl.)", "zh-CN" },
{ "Chinese(trad.)", "zh-TW" },
{ "Korean", "ko" },
{ "Australian", "en-AU" },
{ "Arabic", "ar" },
{ "Hebrew", "he" },
{ "Romanian", "ro" },
{ "Slovene", "sl" },
{ "Croatian", "hr" },
{ "Slovakian", "sk" },
{ "Lithuanian", "lt" },
{ "Latvian", "lv" },
{ "Estonian", "et" },
{ "Thai", "th" },
{ "Bulgarian", "bg" },
{ "Macedonian", "mk" },
{ "Serbian(Cyrillic)", "sr" },
{ "Belarusian", "br" },
{ "Ukrainian", "uk" },
{ "", "en-US" } };

// available formats for MT output files
typedef enum _MTOUTPUTFORMAT { EXP_FORMAT, XML_FORMAT, TMX_FORMAT, XLIFF_FORMAT } MTOUTPUTFORMAT; 


// options for a single MT output file
typedef struct _MTOUTFILE
{
  // fields filled with processing options
  char szName[MAX_LONGFILESPEC];                // name of the output file w/o extension
  MTOUTPUTFORMAT eFormat;                       // format of the output file (is used for the file extension as well)
  BOOL fNoDuplicates;                           // TRUE = suppress duplicate segments in output file
  BOOL fNoExact;                                // TRUE = suppress segments having exact memory proposals
  BOOL fNoMachine;                              // TRUE = suppress segments having machine translation proposals
  BOOL fNoHamster;                              // TRUE = suppress segments having Hamster proposals
  USHORT usNoFuzzyAbove;                        // suppress segments having fuzzy proposals with a fuzziness above the given value
  BOOL fIncludeWordCountInfo;                   // TRUE = add word count info to output file
  BOOL fWithMatch;                              // TRUE = include available memory proposals in output file (only supperted for XML)
  BOOL fWithMatchAndSource;                     // TRUE = include available memory proposals and the source of this proposals in output file (only supperted for XML)

  // fields used during processing
  CHAR szOutFile[MAX_LONGPATH];                 // buffer for output file name 
  CHAR szOutFuzzy[MAX_LONGPATH];                // buffer for output file name for fuzzy matches
  CHAR szCurDoc[MAX_LONGPATH];                  // name of current document
  int  iCurPar;                                 // current paragraph ID
  int  iCurSeg;                                 // current sentence/segment ID
  FILE *hfOut;                                  // output file handle
  FILE *hfOutFuzzy;                             // output file handle fuzzy matches
  BOOL  fProposalsWritten;                      //TRUE if proposals have been written to the hfOutFuzzy file
  ULONG ulTotal;                                // total number of words written to a specific output file
  ULONG ulCountPos;                             // position of the word count area in the output file (only used for EXP files)
} MTOUTFILE, *PMTOUTFILE;

// the data area for the MT output files
typedef struct _MTOUTDATA
{
  int iNumOfOutFiles;                            // number of used output files in OutFile array
  MTOUTFILE OutFile[10];                         // array with output file specifications and the variables used for the output processing

  //BOOL fFormat1;                                 // output in format 1 (switch "NOMATCH")        : all segs without exact matches in memory
  //BOOL fFormat2;                                 // output in format 2 (switch "ALLSEGS")        : all segs
  //BOOL fFormat3;                                 // output in format 3 (switch "ALLWMATCH")      : all segs with exact and fuzzy matches
  //BOOL fFormat4;                                 // output in format 4 (switch "NOMATCHEXP")     : all segs without exact matches in memory in EXP format
  //BOOL fFormat5;                                 // output in format 5 (switch "ALLSEGSEXP")     : all segs in EXP format
  //BOOL fFormat6;                                 // output in format 6 (switch "ALLWMATCHSOURCE"): all segs in XML format with matches and match source in XML
  //BOOL fFormat7;                                 // output in format 7 (switch "NOPROPOSAL")     : all segs in XML format which have no match at all
  //BOOL fFormat8;                                 // output in format 8 (switch "NOPROPOSALEXP")  : all segs in EXP format which have no match at all
  //BOOL fFormat9;                                 // output in format 9 (switch "XLIFF")          : everything in XLIFF format

  //BOOL fNoDups1;                                 // no duplicates switch in format 1 (switch "NOMATCH")        : all segs without exact matches in memory
  //BOOL fNoDups2;                                 // no duplicates switch in format 2 (switch "ALLSEGS")        : all segs
  //BOOL fNoDups3;                                 // no duplicates switch in format 3 (switch "ALLWMATCH")      : all segs with exact and fuzzy matches
  //BOOL fNoDups4;                                 // no duplicates switch in format 4 (switch "NOMATCHEXP")     : all segs without exact matches in memory in EXP format
  //BOOL fNoDups5;                                 // no duplicates switch in format 5 (switch "ALLSEGSEXP")     : all segs in EXP format
  //BOOL fNoDups6;                                 // no duplicates switch in format 6 (switch "ALLWMATCHSOURCE"): all segs in XML format with matches and match source in XML
  //BOOL fNoDups7;                                 // no duplicates switch in format 7 (switch "NOPROPOSAL")     : all segs in XML format which have no match at all
  //BOOL fNoDups8;                                 // no duplicates switch in format 9 (switch "NOPROPOSALEXP")  : all segs in EXP format which have no match at all

  //BOOL fNoDupsProcessing;                        // general "no duplicates" processing switch
  CHAR szDupTM[MAX_EQF_PATH];                    // name of temporary memory for duplicates recognition
  LHANDLE hDupTM;                                // handle of temporary memory for duplicates recognition
  TMX_PUT_IN_W       stPutIn;                    // put in structure
  TMX_PUT_OUT_W      stPutOut;                   // put out structure
  TMX_GET_IN_W       stGetIn;
  TMX_GET_OUT_W      stGetOut;

  CHAR szFileName[4096];                         // buffer for file names
  //CHAR szOutFile1[MAX_LONGPATH];                 // buffer for output file name type 1
  //CHAR szOutFile2[MAX_LONGPATH];                 // buffer for output file name type 2
  //CHAR szOutFile3[MAX_LONGPATH];                 // buffer for output file name type 3
  //CHAR szOutFile4[MAX_LONGPATH];                 // buffer for output file name type 4
  //CHAR szOutFile5[MAX_LONGPATH];                 // buffer for output file name type 5
  //CHAR szOutFile6[MAX_LONGPATH];                 // buffer for output file name type 6
  //CHAR szOutFile7[MAX_LONGPATH];                 // buffer for output file name type 7
  //CHAR szOutFile8[MAX_LONGPATH];                 // buffer for output file name type 8
  //CHAR szOutFile9[MAX_LONGPATH];                 // buffer for output file name type 9
  //CHAR szOutFuzzy[MAX_LONGPATH];                 // buffer for output file name for fuzzy matches
  //CHAR szOutFuzzy6[MAX_LONGPATH];                // buffer for output file name for fuzzy matches of report 6
  //CHAR szOutExact[MAX_LONGPATH];                 // buffer for output file name for exact matches
  CHAR szCurDoc[MAX_LONGPATH];                     // name of current document
  //CHAR szCurDoc1[MAX_LONGPATH];                  // name of current document
  //CHAR szCurDoc2[MAX_LONGPATH];                  // name of current document
  //CHAR szCurDoc3[MAX_LONGPATH];                  // name of current document
  //CHAR szCurDoc6[MAX_LONGPATH];                  // name of current document
  //CHAR szCurDoc7[MAX_LONGPATH];                  // name of current document
  //CHAR szCurDoc9[MAX_LONGPATH];                  // name of current document
  //int  iCurPar1;                                 // current paragraph ID
  //int  iCurSeg1;                                 // current sentence/segment ID
  //int  iCurPar2;                                 // current paragraph ID
  //int  iCurSeg2;                                 // current sentence/segment ID
  //int  iCurPar3;                                 // current paragraph ID
  //int  iCurSeg3;                                 // current sentence/segment ID
  //int  iCurSeg4;                                 // current sentence/segment ID
  //int  iCurSeg5;                                 // current sentence/segment ID
  //int  iCurPar6;                                 // current paragraph ID
  //int  iCurSeg6;                                 // current sentence/segment ID
  //int  iCurPar7;                                 // current paragraph ID
  //int  iCurSeg7;                                 // current sentence/segment ID
  //int  iCurPar8;                                 // current paragraph ID
  //int  iCurSeg8;                                 // current sentence/segment ID
  //int  iCurPar9;                                 // current paragraph ID
  //int  iCurSeg9;                                 // current sentence/segment ID
  //FILE *hfOut1;                                  // output file handle format 1
  //FILE *hfOut2;                                  // output file handle format 2
  //FILE *hfOut3;                                  // output file handle format 3
  //FILE *hfOut4;                                  // output file handle format 4
  //FILE *hfOut5;                                  // output file handle format 5
  //FILE *hfOut6;                                  // output file handle format 6
  //FILE *hfOut7;                                  // output file handle format 7
  //FILE *hfOut8;                                  // output file handle format 8
  //FILE *hfOut9;                                  // output file handle format 9
  //FILE *hfOutFuzzy;                              // output file handle fuzzy matches
  //FILE *hfOutFuzzy6;                             // output file handle mattches and match source*
  //FILE *hfOutExact;                              // output file handle exact matches
  BYTE bUTF8Buffer[16000];                       // buffer for strings converted to UTF-8 encoding
  CHAR_W szUTF16Buffer[16000];                   // buffer for UTF16 strings
  CHAR szSourceLang[80];                         // buffer for document source language
  CHAR szTargetLang[80];                         // buffer for document target language
  CHAR szMarkup[80];                             // buffer for document markup
  //BOOL fIncludeWordCountInfo;                    // TRUE = add word count info to output files
  ULONG ulSegSourceWords;                        // number of source words in current segment
  //ULONG ulTotal1;                                // total number of words written to a specific output file
  //ULONG ulTotal2;                                // total number of words written to a specific output file
  //ULONG ulTotal3;                                // total number of words written to a specific output file
  //ULONG ulTotal4;                                // total number of words written to a specific output file
  //ULONG ulTotal5;                                // total number of words written to a specific output file
  //ULONG ulTotal6;                                // total number of words written to a specific output file
  //ULONG ulTotal7;                                // total number of words written to a specific output file
  //ULONG ulTotal8;                                // total number of words written to a specific output file
  //ULONG ulTotal9;                                // total number of words written to a specific output file
  //ULONG ulCountPos1;                             // position of the word count area in the output file (only used for EXP files)
  //ULONG ulCountPos2;                             // position of the word count area in the output file (only used for EXP files)
  //ULONG ulCountPos3;                             // position of the word count area in the output file (only used for EXP files)
  //ULONG ulCountPos4;                             // position of the word count area in the output file (only used for EXP files)
  //ULONG ulCountPos5;                             // position of the word count area in the output file (only used for EXP files)
  //ULONG ulCountPos6;                             // position of the word count area in the output file (only used for EXP files)
  //ULONG ulCountPos7;                             // position of the word count area in the output file (only used for EXP files)
  //ULONG ulCountPos8;                             // position of the word count area in the output file (only used for EXP files)
  CHAR    szError[4096];                           // bufer for error messages
} MTOUTDATA, *PMTOUTDATA;

// names and defaults for the various MT output files and flags if a specific option can be changed by the user
typedef struct _MTOUTPUTFILEDEFAULT
{
  char szName[MAX_LONGFILESPEC];                // name of the MT output file identifier
  BOOL fUserFormat;                             // TRUE format can be specified by the user
  MTOUTPUTFORMAT eFormat;                       // format of the output file (is used for the file extension as well)
  BOOL fUserNoDuplicates;                       // TRUE no duplicates can be specified by the user
  BOOL fNoDuplicates;                           // TRUE = suppress duplicate segments in output file
  BOOL fUserExact;                              // TRUE no exact can be specified by the user
  BOOL fNoExact;                                // TRUE = suppress segments having exact memory proposals
  BOOL fUserNoMachine;                          // TRUE no machine can be specified by the user
  BOOL fNoMachine;                              // TRUE = suppress segments having machine translation proposals
  BOOL fUserNoHamster;                          // TRUE no Hamster can be specified by the user
  BOOL fNoHamster;                              // TRUE = suppress segments having Hamster proposals
  BOOL fUserNoFuzzyAbove;                       // TRUE no fuzzy above can be specified by the user
  USHORT usNoFuzzyAbove;                        // suppress segments having fuzzy proposals with a fuzziness above the given value
  BOOL fUserWordCount;                          // TRUE = word count flags can be specified by the user
  BOOL fIncludeWordCountInfo;                   // TRUE = include word count information into the output file
  BOOL fWithMatch;                              // TRUE = include available memory proposals in output file (only supperted for XML)
  BOOL fWithMatchAndSource;                     // TRUE = include available memory proposals and the source of this proposals in output file (only supperted for XML)
} MTOUTPUTFILEDEFAULT, *PMTOUTPUTFILEDEFAULT;

MTOUTPUTFILEDEFAULT MTOutputFilesDefaults[] =
{ // NAME              UserFormat?  Format         UserDups? NoDups    UserExact? NoExact UserMT?  NoMT    UserHamster? NoHamster UserFuzzy? NoFuzzyAbove UserWordCount? WordCountInfo  WithMatch WithMatchAndSource  
  { "NOMATCH",         TRUE,        XML_FORMAT,    TRUE,     FALSE,    FALSE,     TRUE,   TRUE,    TRUE,   TRUE,        TRUE,     TRUE,      99,          TRUE,          FALSE,         FALSE,    FALSE },
  { "ALLSEGS",         TRUE,        XML_FORMAT,    TRUE,     FALSE,    FALSE,     FALSE,  FALSE,   FALSE,  FALSE,       FALSE,    FALSE,     99,          TRUE,          FALSE,         FALSE,    FALSE },
  { "ALLWMATCH",       FALSE,       XML_FORMAT,    TRUE,     FALSE,    FALSE,     FALSE,  FALSE,   FALSE,  FALSE,       FALSE,    FALSE,     99,          TRUE,          FALSE,         TRUE,     FALSE },
  { "ALLWMATCHSOURCE", FALSE,       XML_FORMAT,    TRUE,     FALSE,    FALSE,     FALSE,  FALSE,   FALSE,  FALSE,       FALSE,    FALSE,     99,          TRUE,          FALSE,         FALSE,    TRUE },
  { "NOPROPOSAL",      TRUE,        XML_FORMAT,    TRUE,     FALSE,    FALSE,     TRUE,   TRUE,    TRUE,   TRUE,        TRUE,     TRUE,      49,          TRUE,          FALSE,         FALSE,    FALSE },
  { "XLIFF",           FALSE,       XLIFF_FORMAT,  TRUE,     FALSE,    FALSE,     TRUE,   TRUE,    TRUE,   TRUE,        TRUE,     TRUE,      49,          FALSE,         FALSE,         FALSE,    FALSE },
  { "" },  // end-of-list indicator
};

// add an output file to the MT output file array using the specified name the options
BOOL TAAddMTOutFile( PMTOUTDATA pData, PSZ pszOutFile )
{
  MTOUTFILE NewOutputFile;

  memset( &NewOutputFile, 0, sizeof(NewOutputFile) );

  // test parameters
  if ( pszOutFile == NULL )
  {
    strcpy( pData->szError, "Missing MT output file specification" );
    return( FALSE );
  } /* endif */

  // prepare MT output file specification
  strcpy( pData->szFileName, pszOutFile );
  UtlStripBlanks( pData->szFileName );
  UtlUpper( pData->szFileName );

  // find start of options 
  PSZ pszOptions = strchr( pData->szFileName, '(' );
  if ( pszOptions != NULL )
  {
    *pszOptions++ = EOS;
    UtlStripBlanks( pData->szFileName );
  }

  // check file name against our valid file name list
  int i = 0;
  while ( MTOutputFilesDefaults[i].szName[0] != EOS )
  {
    if ( strcmp( MTOutputFilesDefaults[i].szName, pData->szFileName ) == 0 )
    {
      break;
    } /* endif */
    i++;
  } /* endwhile */
  if ( MTOutputFilesDefaults[i].szName[0] == EOS )
  {
    sprintf( pData->szError, "\"%s\" is no valid MT output file identifier", pData->szFileName );
    return( FALSE );
  } /* endif */

  // get default settings for selected output file
  PMTOUTPUTFILEDEFAULT pDefaults = MTOutputFilesDefaults + i;
  NewOutputFile.eFormat = pDefaults->eFormat;
  NewOutputFile.fNoDuplicates = pDefaults->fNoDuplicates;
  NewOutputFile.fNoExact = pDefaults->fNoExact;
  NewOutputFile.fNoHamster = pDefaults->fNoHamster;
  NewOutputFile.fNoMachine = pDefaults->fNoMachine;
  NewOutputFile.usNoFuzzyAbove = pDefaults->usNoFuzzyAbove;
  strcpy( NewOutputFile.szName, pDefaults->szName );
  NewOutputFile.fIncludeWordCountInfo = pDefaults->fIncludeWordCountInfo;
  NewOutputFile.fWithMatch = pDefaults->fWithMatch;
  NewOutputFile.fWithMatchAndSource = pDefaults->fWithMatchAndSource;

  // process any specified options
  if ( pszOptions != NULL )
  {
    // prepare options
    PSZ pszEnd = strchr( pszOptions, ')' );
    if ( pszEnd != NULL ) *pszEnd = EOS;
    UtlStripBlanks( pszOptions );
    while ( (pszOptions != NULL) && (*pszOptions != EOS) )
    {
      PSZ pszEnd = strchr( pszOptions, ',' );
      if ( pszEnd != NULL ) *pszEnd++ = EOS;
      UtlStripBlanks( pszOptions );

      if ( strcmp( pszOptions, "EXP" ) == 0 )
      {
        if ( pDefaults->fUserFormat )
        {
          NewOutputFile.eFormat = EXP_FORMAT;
        }
        else
        {
          sprintf( pData->szError, "Format \"%s\" can not be specified for output file %s", pszOptions, pDefaults->szName );
          return( FALSE );
        } /* endif */
      }
      else if ( strcmp( pszOptions, "XML" ) == 0 )
      {
        if ( pDefaults->fUserFormat )
        {
          NewOutputFile.eFormat = XML_FORMAT;
        }
        else
        {
          sprintf( pData->szError, "Format \"%s\" can not be specified for output file %s", pszOptions, pDefaults->szName );
          return( FALSE );
        } /* endif */
      }
      else if ( strcmp( pszOptions, "TMX" ) == 0 )
      {
        if ( pDefaults->fUserFormat )
        {
          NewOutputFile.eFormat = TMX_FORMAT;
        }
        else
        {
          sprintf( pData->szError, "Format \"%s\" can not be specified for output file %s", pszOptions, pDefaults->szName );
          return( FALSE );
        } /* endif */
      }
      else if ( strcmp( pszOptions, "DUPLICATES" ) == 0 )
      {
        if ( pDefaults->fUserNoDuplicates )
        {
          NewOutputFile.fNoDuplicates = FALSE;
        }
        else
        {
          sprintf( pData->szError, "\"%s\" can not be specified for output file %s", pszOptions, pDefaults->szName );
          return( FALSE );
        } /* endif */
      }
      else if ( strcmp( pszOptions, "NODUPLICATES" ) == 0 )
      {
        if ( pDefaults->fUserNoDuplicates )
        {
          NewOutputFile.fNoDuplicates = TRUE;
        }
        else
        {
          sprintf( pData->szError, "\"%s\" can not be specified for output file %s", pszOptions, pDefaults->szName );
          return( FALSE );
        } /* endif */
      }
      else if ( strcmp( pszOptions, "HAMSTER" ) == 0 )
      {
        if ( pDefaults->fUserNoHamster )
        {
          NewOutputFile.fNoHamster = FALSE;
        }
        else
        {
          sprintf( pData->szError, "\"%s\" can not be specified for output file %s", pszOptions, pDefaults->szName );
          return( FALSE );
        } /* endif */
      }
      else if ( strcmp( pszOptions, "NOHAMSTER" ) == 0 )
      {
        if ( pDefaults->fUserNoHamster )
        {
          NewOutputFile.fNoHamster = TRUE;
        }
        else
        {
          sprintf( pData->szError, "\"%s\" can not be specified for output file %s", pszOptions, pDefaults->szName );
          return( FALSE );
        } /* endif */
      }
      else if ( strcmp( pszOptions, "MACHINEMATCH" ) == 0 )
      {
        if ( pDefaults->fUserNoMachine )
        {
          NewOutputFile.fNoMachine = FALSE;
        }
        else
        {
          sprintf( pData->szError, "\"%s\" can not be specified for output file %s", pszOptions, pDefaults->szName );
          return( FALSE );
        } /* endif */
      }
      else if ( strcmp( pszOptions, "NOMACHINEMATCH" ) == 0 )
      {
        if ( pDefaults->fUserNoMachine )
        {
          NewOutputFile.fNoMachine = TRUE;
        }
        else
        {
          sprintf( pData->szError, "\"%s\" can not be specified for output file %s", pszOptions, pDefaults->szName );
          return( FALSE );
        } /* endif */
      }
      else if ( strncmp( pszOptions, "NOFUZZYABOVE=", 13 ) == 0 )
      {
        if ( pDefaults->fUserNoFuzzyAbove )
        {
          PSZ pszNumber = pszOptions + 13;
          while ( *pszNumber != EOS )
          {
            if ( !isdigit(*pszNumber) )
            {
              sprintf( pData->szError, "\"%s\" is no valid number for the fuzziness, valid are numbers in the range from 0 up to 99", pszOptions + 13 );
              return( FALSE );
            } /* endif */
            pszNumber++;
          } /* endwhile */
          NewOutputFile.usNoFuzzyAbove = (USHORT)atoi( pszOptions + 13 );
          if ( NewOutputFile.usNoFuzzyAbove > 99 )
          {
            sprintf( pData->szError, "\"%s\" is no valid number for the fuzziness, valid are numbers in the range from 0 up to 99", pszOptions + 13 );
            return( FALSE );
          } /* endif */
        }
        else
        {
          sprintf( pData->szError, "\"%s\" can not be specified for output file %s", pszOptions, pDefaults->szName );
          return( FALSE );
        } /* endif */
      }
      else if ( strcmp( pszOptions, "WORDCOUNT" ) == 0 )
      {
        if ( pDefaults->fUserWordCount )
        {
          NewOutputFile.fIncludeWordCountInfo = TRUE;
        }
        else
        {
          sprintf( pData->szError, "\"%s\" can not be specified for output file %s", pszOptions, pDefaults->szName );
          return( FALSE );
        } /* endif */
      }
      else if ( strcmp( pszOptions, "NOWORDCOUNT" ) == 0 )
      {
        if ( pDefaults->fUserWordCount )
        {
          NewOutputFile.fIncludeWordCountInfo = FALSE;
        }
        else
        {
          sprintf( pData->szError, "\"%s\" can not be specified for output file %s", pszOptions, pDefaults->szName );
          return( FALSE );
        } /* endif */
      }
      else
      {
        sprintf( pData->szError, "\"%s\" is no valid option for output file %s", pszOptions, pDefaults->szName );
        return( FALSE );
      } /* endif */
      pszOptions = pszEnd;
    } /* endwhile */
  } /* endif */

  // check if maximum of output files has been reached
  if ( (pData->iNumOfOutFiles + 1) > sizeof(pData->OutFile)/sizeof(pData->OutFile[0]) )
  {
    sprintf( pData->szError, "\"%s\" is no valid option for output file %s", pszOptions, pDefaults->szName );
    return( FALSE );
  } /* endif */

  // add new output file to output file table
  memcpy( &(pData->OutFile[pData->iNumOfOutFiles]), &NewOutputFile, sizeof(pData->OutFile[pData->iNumOfOutFiles]) ); 
  pData->iNumOfOutFiles += 1;

  return( TRUE );
} /* end of TAAddMTOutFile */

// evaluate the specified MT output options
BOOL TAAddEvaluateMTOptions( PMTOUTDATA pData, PSZ pszOptions )
{
  BOOL fOK = TRUE;

  // copy specified options to a buffer area (here we use the UTF8 buffer so we need some type conversions...)
  strcpy( (PSZ)pData->bUTF8Buffer, pszOptions );
  PSZ pszEnd = (PSZ)pData->bUTF8Buffer;

  // remove any leading and trailing parenthesis
  UtlStripBlanks( pszEnd );
  if ( *pszEnd == '(' ) 
  {
    pszEnd += 1;
    if ( (*pszEnd != EOS) && (pszEnd[strlen(pszEnd)-1] == ')') ) pszEnd[strlen(pszEnd)-1] = EOS;
  } /* endif */
  UtlStripBlanks( pszEnd );

  do
  {
    PSZ pszCurFile = pszEnd;

    // prepare next MT output file name
    while ( (*pszEnd != EOS) && (*pszEnd != ',') )
    {
      if ( *pszEnd == '(' )
      {
        // skip options for the current file
        while ( (*pszEnd != EOS) && (*pszEnd != ')') ) pszEnd++;
      }
      else
      {
        pszEnd++;
      } /* endif */
    } /* endwhile */
    if ( *pszEnd == EOS ) pszEnd = NULL;
    if ( pszEnd != NULL ) *pszEnd++ = EOS;

    fOK = TAAddMTOutFile( pData, pszCurFile );
  } while ( fOK && (pszEnd != NULL) );

  return( fOK );
} /* end of function TAAddEvaluateMTOptions */

// get ISO language ID for TMlanguage name
BOOL TAGetIsoLang( PSZ pszTmLang, PSZ pszIsoLang )
{
  BOOL fOK = TRUE;
  PTM2ISOLANG pLang = TmToIsoLang;


  while ( (pLang->szTMLang[0] != EOS) && (stricmp( pszTmLang, pLang->szTMLang ) != 0) ) 
  {
    pLang++;
  } /*endwhile */

  strcpy( pszIsoLang, pLang->szISOLang );
  
  return( fOK );
}

// write UTF16 string in UTF8
BOOL TAWriteInUTF8( PMTOUTDATA pData, FILE *hfOut, PSZ_W pszUTF16 )
{
  BOOL fOK = TRUE;

  // convert to UTF-8
  int iOutLen =  WideCharToMultiByte( CP_UTF8, 0, (LPCWSTR)pszUTF16, -1, (LPSTR)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer), NULL, NULL );

  // write UTF-8 string to output file
  fwrite( pData->bUTF8Buffer, iOutLen - 1, 1, hfOut );

  return( fOK );
}

// prepare segment text for writing in EXP format
//   - LF is changed to CRLF
void TAPrepareSegmentText( PSZ_W pszSegmentIn, PSZ_W pszSegmentOut )
{
#ifdef FORCE_CRLF_IN_MEMORY_OUTPUT
  PSZ_W pszSource = pszSegmentIn;
  PSZ_W pszTarget = pszSegmentOut;

  while( *pszSource )
  {
    if ( *pszSource == L'\r' )
    {
      // ignore carriage return
      pszSource++;
    }
    else if ( *pszSource == L'\n' )
    {
      // convert to CRLF
      pszSource++;
      *pszTarget++ = L'\r';
      *pszTarget++ = L'\n';
    }
    else
    {
      // copy as-is
      *pszTarget++ = *pszSource++;
    } /* endif */
  } /* endwhile */
  *pszTarget = 0;
#else
  wcscpy( pszSegmentOut, pszSegmentIn );
#endif
}


// escape characters in segment data to form valid XML 
void TAEscapeChars( PSZ_W pszText, PSZ_W pszBuffer )
{
  while ( *pszText )
  {
    if ( *pszText == L'\n' )
    {
      BOOL fEndOfSegment = FALSE;

      // check if we are at the end of the text (=segment) data
      {
        PSZ_W pszTest = pszText + 1;
        while ( (*pszTest == L'\n') || (*pszTest == L'\r') ) pszTest++;
        fEndOfSegment = (*pszTest == 0);
      }

      // convert linebreak to space character when not at the end of the segment
      if ( !fEndOfSegment )
      {
        wcscpy( pszBuffer, L" " );
        pszBuffer += wcslen( pszBuffer );
      }
    }
    else if ( *pszText == L'\r' )
    {
    }
    else if ( *pszText == L'&' )
    {
      wcscpy( pszBuffer, L"&amp;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'<' )
    {
      wcscpy( pszBuffer, L"&lt;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'>' )
    {
      wcscpy( pszBuffer, L"&gt;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'\"' )
    {
      wcscpy( pszBuffer, L"&quot;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( (*pszText == L'\x1F') || (*pszText == L'\t') )
    {
      // replace whitspace characters with a blank
      *pszBuffer++ = L' ';
    }
    else if ( (*pszText >=1 ) && ( *pszText < 32 ) )
    {
      // replace remaining invalid characters with a question mark
      *pszBuffer++ = L'?';
    }
    else
    {
      *pszBuffer++ = *pszText;
    } /* endif */
    pszText++;
  } /*endwhile */
  *pszBuffer = 0;
}

// escape characters in segment data to form valid XLIFF
void TAEscapeXliffChars( PSZ_W pszText, PSZ_W pszBuffer )
{
  while ( *pszText )
  {
    if ( *pszText == L'\n' )
    {
      wcscpy( pszBuffer, L"\r\n" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'\r' )
    {
    }
    else if ( *pszText == L'&' )
    {
      wcscpy( pszBuffer, L"&amp;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'<' )
    {
      wcscpy( pszBuffer, L"&lt;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'>' )
    {
      wcscpy( pszBuffer, L"&gt;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( *pszText == L'\"' )
    {
      wcscpy( pszBuffer, L"&quot;" );
      pszBuffer += wcslen( pszBuffer );
    }
    else if ( (*pszText == L'\x1F') || (*pszText == L'\t') )
    {
      // suppress some special characters
      *pszBuffer++ = L' ';
    }
    else if ( (*pszText >=1 ) && ( *pszText < 32 ) )
    {
      // replace remaining invalid characters with a question mark
      *pszBuffer++ = L'?';
    }
    else
    {
      *pszBuffer++ = *pszText;
    } /* endif */
    pszText++;
  } /*endwhile */
  *pszBuffer = 0;
}
// get settings from trigger file
BOOL TAGetTriggerFileSettings( PMTOUTDATA pData )
{
  FILE *hfTrigger = NULL;
  BOOL fIncludeWordCountInfo = FALSE;

  // setup trigger file name
  UtlMakeEQFPath( pData->szFileName, NULC, PROPERTY_PATH, NULL );
  strcat( pData->szFileName, "\\EQFNFLUENT.TRG" );

  // loop over trigger file lines
  hfTrigger = fopen( pData->szFileName, "r" );
  if ( hfTrigger )
  {
    fgets( (PSZ)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer), hfTrigger );
    while( !feof( hfTrigger ) )
    {
      PSZ pszLine = (PSZ)pData->bUTF8Buffer;
      if ( *pszLine != '*' )
      {
        if ( _strnicmp( pszLine, "INCLUDEWORDCOUNT", 16 ) == 0 )
        {
          fIncludeWordCountInfo = TRUE;
        }
        else if ( _strnicmp( pszLine, "NOMATCHEXP_NODUPLICATE", 22 ) == 0 )
        {
          TAAddMTOutFile( pData, "NOMATCH(EXP,NODUPLICATES)" );
        }
        else if ( _strnicmp( pszLine, "NOMATCHEXP", 10 ) == 0 )
        {
          TAAddMTOutFile( pData, "NOMATCH(EXP)" );
        }
        else if ( _strnicmp( pszLine, "NOMATCH_NODUPLICATE", 19 ) == 0 )
        {
          TAAddMTOutFile( pData, "NOMATCH(XML,NODUPLICATES)" );
        }
        else if ( _strnicmp( pszLine, "NOMATCH", 7 ) == 0 )
        {
          TAAddMTOutFile( pData, "NOMATCH(XML)" );
        }
        else if ( _strnicmp( pszLine, "ALLSEGSEXP_NODUPLICATE", 22 ) == 0 )
        {
          TAAddMTOutFile( pData, "ALLSEGS(EXP,NODUPLICATES)" );
        }
        else if ( _strnicmp( pszLine, "ALLSEGSEXP", 10 ) == 0 )
        {
          TAAddMTOutFile( pData, "ALLSEGS(EXP)" );
        }
        else if ( _strnicmp( pszLine, "ALLSEGS_NODUPLICATE", 19 ) == 0 )
        {
          TAAddMTOutFile( pData, "ALLSEGS(XML,NODUPLICATES)" );
        }
        else if ( _strnicmp( pszLine, "ALLSEGS", 7 ) == 0 )
        {
          TAAddMTOutFile( pData, "ALLSEGS(XML)" );
        }
        else if ( _strnicmp( pszLine, "ALLWMATCHSOURCE_NODUPLICATE", 27 ) == 0 )
        {
          TAAddMTOutFile( pData, "ALLWMATCHSOURCE(NODUPLICATES)" );
        }
        else if ( _strnicmp( pszLine, "ALLWMATCHSOURCE", 15 ) == 0 )
        {
          TAAddMTOutFile( pData, "ALLWMATCHSOURCE" );
        }
        else if ( _strnicmp( pszLine, "ALLWMATCH_NODUPLICATE", 21 ) == 0 )
        {
          TAAddMTOutFile( pData, "ALLWMATCH(NODUPLICATES)" );
        }
        else if ( _strnicmp( pszLine, "ALLWMATCH", 9 ) == 0 )
        {
          TAAddMTOutFile( pData, "ALLWMATCH" );
        }
        else if ( _strnicmp( pszLine, "NOPROPOSALEXP_NODUPLICATE", 25 ) == 0 )
        {
          TAAddMTOutFile( pData, "NOPROPOSAL(EXP,NODUPLICATES)" );
        }
        else if ( _strnicmp( pszLine, "NOPROPOSALEXP", 13 ) == 0 )
        {
          TAAddMTOutFile( pData, "NOPROPOSAL(EXP)" );
        }
        else if ( _strnicmp( pszLine, "NOPROPOSAL_NODUPLICATE", 22 ) == 0 )
        {
          TAAddMTOutFile( pData, "NOPROPOSAL(XML,NODUPLICATES)" );
        }
        else if ( _strnicmp( pszLine, "NOPROPOSAL", 9 ) == 0 )
        {
          TAAddMTOutFile( pData, "NOPROPOSAL(XML)" );
        }
        else if ( _strnicmp( pszLine, "XLIFF", 5 ) == 0 )
        {
          TAAddMTOutFile( pData, "XLIFF" );
        }
        else
        {
          // ignore unknown switch
        } /* endif */

      } /* endif */
      fgets( (PSZ)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer), hfTrigger );
    } /*endwhile */
    fclose( hfTrigger );
  } /* endif */

  // apply global flag INCLUDEWORDCOUNT to all output files
  if ( fIncludeWordCountInfo && (pData->iNumOfOutFiles != 0) )
  {
    for( int i = 0; i < pData->iNumOfOutFiles; i++ )
    {
      pData->OutFile[i].fIncludeWordCountInfo = fIncludeWordCountInfo;
    } /* endfor */
  } /* endif */

  return( TRUE );
} /* end of function TAGetTriggerFileSettings */


// write XML prefix
BOOL TAWriteXMLPrefix( PMTOUTDATA pData, FILE *hfOut, PSZ pszTargetLang, BOOL fWriteBom )
{
  BOOL fOK = TRUE;
  CHAR szIso[20];

  fWriteBom;

  fwrite( UTF8FILEPREFIX, strlen(UTF8FILEPREFIX), 1, hfOut );
  fOK = TAWriteInUTF8( pData, hfOut, L"<?xml version=\'1.0\'?>\r\n" );
  TAGetIsoLang( pszTargetLang, szIso );
  swprintf( pData->szUTF16Buffer, L"<rqt action=\"trqt\" tl=\"%S\" uid=\"xyz@us.ibm.com\">\r\n", szIso );
  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, pData->szUTF16Buffer );
  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"   <docs>\r\n" );
  return( fOK );
} /* end of function TAWriteXMLPrefix */

// write TMX prefix
BOOL TAWriteTMXPrefix( PMTOUTDATA pData, FILE *hfOut, PSZ pszSourceLang, BOOL fWriteBom, BOOL fWordCountInfo, PULONG pulWordCountPos  )
{
  BOOL fOK = TRUE;
  CHAR szIso[20];

  fWriteBom;

  fwrite( UTF8FILEPREFIX, strlen(UTF8FILEPREFIX), 1, hfOut );
  fOK = TAWriteInUTF8( pData, hfOut, L"<?xml version=\'1.0\' encoding=\"UTF-8\"?>\r\n" );
  fOK = TAWriteInUTF8( pData, hfOut, L"<tmx version=\"1.4\">\r\n" );

  TAGetIsoLang( pszSourceLang, szIso );

  char szVersion[20];
  sprintf( szVersion, "%d.%d.%d.%d", EQF_DRIVER_VERSION, EQF_DRIVER_RELEASE, EQF_DRIVER_SUBRELEASE, EQF_DRIVER_BUILD );
  if ( fWordCountInfo )
  {
    swprintf( pData->szUTF16Buffer, L"<header creationtoolversion=\"%S\" segtype=\"sentence\" adminlang=\"en-us\" srclang=\"%S\" o-tmf=\"OpenTM2\" creationtool=\"OpenTM2\" datatype=\"rtf\" >\r\n",
              szVersion, szIso );
    if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, pData->szUTF16Buffer );
    if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"    <prop type=\"tmgr:description\">Total word count = &quot;" );
    *pulWordCountPos = ftell( hfOut );
    if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"              &quot;</prop>\r\n" );
    if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"</header>\r\n" );
  }
  else
  {
    swprintf( pData->szUTF16Buffer, L"<header creationtoolversion=\"%S\" segtype=\"sentence\" adminlang=\"en-us\" srclang=\"%S\" o-tmf=\"OpenTM2\" creationtool=\"OpenTM2\" datatype=\"rtf\" />\r\n",
              szVersion, szIso );
    if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, pData->szUTF16Buffer );
  } /* endif */
  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"   <body>\r\n" );
  return( fOK );
} /* end of function TAWriteXMLPrefix */


// write EXP prefix
BOOL TAWriteEXPPrefix( PMTOUTDATA pData, FILE *hfOut, BOOL fWordCountInfo, PULONG pulWordCountPos )
{
  BOOL fOK = TRUE;

  pData;

  fwrite( UNICODEFILEPREFIX, strlen(UNICODEFILEPREFIX), 1, hfOut );
  fputws( NTM_BEGIN_TAGW, hfOut );
  if ( fWordCountInfo )
  {
    fputws( NTM_DESCRIPTION_BEGIN_TAGW, hfOut );
    fputws( L"Total word count = ", hfOut );
    *pulWordCountPos = ftell( hfOut );
    fputws( L"              ", hfOut );  // do not remove these blanks, they are overwritten with the actual word count info at the end of the processing
    fputws( NTM_DESCRIPTION_END_TAGW, hfOut );
  } /* endif */
  fputws( NTM_CODEPAGE_BEGIN_TAGW, hfOut );
  fputws( NTM_CODEPAGE_UTF16_VALUE, hfOut );
  fputws( NTM_CODEPAGE_END_TAGW, hfOut );

  return( fOK );
} /* end of function TAWriteEXPPrefix */

// write XLIFF prefix
BOOL TAWriteXLIFFPrefix( PMTOUTDATA pData, FILE *hfOut, PSZ pszFolder, PSZ pszSourceLang, PSZ pszTargetLang, BOOL fWriteBom )
{
  BOOL fOK = TRUE;
  CHAR szSrcIso[20];
  CHAR szTgtIso[20];
  CHAR szFolShortName[MAX_FILESPEC];

  fWriteBom;

  fwrite( UTF8FILEPREFIX, strlen(UTF8FILEPREFIX), 1, hfOut );
  fOK = TAWriteInUTF8( pData, hfOut, L"<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n" );
  TAGetIsoLang( pszSourceLang, szSrcIso );
  TAGetIsoLang( pszTargetLang, szTgtIso );
  swprintf( pData->szUTF16Buffer, L"<xliff version=\"2.0\" srcLang=\"%S\" tgtLang=\"%S\"\r\n", szSrcIso, szTgtIso );
  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, pData->szUTF16Buffer );
  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"  xmlns:mda=\"urn:oasis:names:tc:xliff:metadata:2.0\"\r\n" );
  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"  xmlns:mtc=\"urn:oasis:names:tc:xliff:matches:2.0\">\r\n" );
  {
     // write meta data for this folder
     wchar_t *szMDA_META = L" <mda:meta type=\"%s\">%S</mda:meta>\r\n" ;
     if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"<mda:metadata><mda:metagroup category=\"otm\">\r\n" );
     swprintf ( pData->szUTF16Buffer, szMDA_META, L"OpenTM2", STR_DRIVER_LEVEL_NUMBER );
     if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, pData->szUTF16Buffer );
     Utlstrccpy( szFolShortName, UtlGetFnameFromPath( pszFolder ), DOT );
     ObjShortToLongName( szFolShortName, pData->szFileName, FOLDER_OBJECT );
     swprintf ( pData->szUTF16Buffer, szMDA_META, L"folder", pData->szFileName );
     if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, pData->szUTF16Buffer );
//   swprintf ( pData->szUTF16Buffer, szMDA_META, L"shortname", szFolShortName );
//   if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, pData->szUTF16Buffer );
     swprintf ( pData->szUTF16Buffer, szMDA_META, L"srclang", pszSourceLang );
     if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, pData->szUTF16Buffer );
     swprintf ( pData->szUTF16Buffer, szMDA_META, L"tgtlang", pszTargetLang );
     if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, pData->szUTF16Buffer );
     if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"</mda:metagroup></mda:metadata>\r\n" );
  } /* endif */
  return( fOK );
} /* end of function TAWriteXLIFFPrefix */

// open the MT file
BOOL TAOpenMTFile( PTAINPUT pTAInput, PSZ pszFileName )
{
  BOOL fOK = TRUE;
  PMTOUTDATA pData = NULL;

  pszFileName;

  // allocate our data area
  fOK = UtlAlloc( (PVOID *)&(pTAInput->pInD->pCBMachTransFile), 0L, sizeof(MTOUTDATA), ERROR_STORAGE );
  pData = (PMTOUTDATA)pTAInput->pInD->pCBMachTransFile;

  // use provided MT options or get settings from trigger file if no options are provided
  if ( fOK )
  {
    if ( pTAInput->szMtOptions[0] != EOS )
    {
      fOK = TAAddEvaluateMTOptions( pData, pTAInput->szMtOptions );
      if ( !fOK )
      {
        PSZ pszParm = pData->szError;
        UtlErrorHwnd( ERROR_TA_MTOPTIONS, MB_CANCEL, 1, &pszParm, EQF_ERROR,  pTAInput->hwndErrMsg );
      } /* endif */
    }
    else
    {
      fOK = TAGetTriggerFileSettings( pData );
    } /* endif */
  } /* endif */

  // initialize some output file fields
  if ( fOK && (pData->iNumOfOutFiles != 0) )
  {
    for( int i = 0; i < pData->iNumOfOutFiles; i++ )
    {
      pData->OutFile[i].iCurPar = 1;
      pData->OutFile[i].iCurSeg = 1;
    } /* endfor */
  } /* endif */

  // setup output file name(s)
  if ( fOK && (pData->iNumOfOutFiles != 0) )
  {
    CHAR szFolShortName[MAX_FILESPEC];

    UtlMakeEQFPath( pData->szFileName, NULC, DIRSEGMT_PATH, NULL );
    UtlMkDir( pData->szFileName, 0L, FALSE );
    strcat( pData->szFileName, BACKSLASH_STR );
    Utlstrccpy( szFolShortName, UtlGetFnameFromPath( pTAInput->szFolder ), DOT );
    ObjShortToLongName( szFolShortName, pData->szFileName + strlen(pData->szFileName), FOLDER_OBJECT );

    for( int i = 0; i < pData->iNumOfOutFiles; i++ )
    {
      strcpy( pData->OutFile[i].szOutFile, pData->szFileName );

      if ( strcmp( pData->OutFile[i].szName, "NOMATCH" ) == 0 )
      {
        strcat( pData->OutFile[i].szOutFile, ".nomatch" );
      }
      else if ( strcmp( pData->OutFile[i].szName, "ALLSEGS" ) == 0 )
      {
        strcat( pData->OutFile[i].szOutFile, ".allsegs" );
      }
      else if ( strcmp( pData->OutFile[i].szName, "ALLWMATCH" ) == 0 )
      {
        strcat( pData->OutFile[i].szOutFile, ".allwmatch" );
        strcpy( pData->OutFile[i].szOutFuzzy, pData->szFileName );
        strcat( pData->OutFile[i].szOutFuzzy, ".fuzzy.xml" );
      }
      else if ( strcmp( pData->OutFile[i].szName, "ALLWMATCHSOURCE" ) == 0 )
      {
        strcat( pData->OutFile[i].szOutFile, ".allwmatchsource" );
        strcpy( pData->OutFile[i].szOutFuzzy, pData->szFileName );
        strcat( pData->OutFile[i].szOutFuzzy, ".fuzzy-source.xml" );
      }
      else if ( strcmp( pData->OutFile[i].szName, "NOMATCH" ) == 0 )
      {
        strcat( pData->OutFile[i].szOutFile, ".nomatch" );
      }
      else if ( strcmp( pData->OutFile[i].szName, "NOPROPOSAL" ) == 0 )
      {
        strcat( pData->OutFile[i].szOutFile, ".noproposal" );
      }
      else if ( strcmp( pData->OutFile[i].szName, "XLIFF" ) == 0 )
      {
        strcat( pData->OutFile[i].szOutFile, ".all" );
      } /* endif */

      switch( pData->OutFile[i].eFormat )
      {
        case XML_FORMAT: strcat( pData->OutFile[i].szOutFile, ".xml" ); break;
        case EXP_FORMAT: strcat( pData->OutFile[i].szOutFile, ".exp" ); break;
        case TMX_FORMAT: strcat( pData->OutFile[i].szOutFile, ".tmx" ); break;
        case XLIFF_FORMAT: strcat( pData->OutFile[i].szOutFile, ".xlf" ); break;
      } /* endswitch */
    } /* endfor */
  } /* endif */

  // open output file(s)
  if ( fOK && (pData->iNumOfOutFiles != 0) )
  {
    CHAR szFolShortName[MAX_FILESPEC];

    UtlMakeEQFPath( pData->szFileName, NULC, DIRSEGMT_PATH, NULL );
    UtlMkDir( pData->szFileName, 0L, FALSE );
    strcat( pData->szFileName, BACKSLASH_STR );
    Utlstrccpy( szFolShortName, UtlGetFnameFromPath( pTAInput->szFolder ), DOT );
    ObjShortToLongName( szFolShortName, pData->szFileName + strlen(pData->szFileName), FOLDER_OBJECT );

    for( int i = 0; fOK && (i < pData->iNumOfOutFiles); i++ )
    {
      fOK = ((pData->OutFile[i].hfOut = fopen( pData->OutFile[i].szOutFile, "wb" )) != NULL);
    } /* endfor */
  } /* endif */

  // write output file prefix
  if ( fOK && (pData->iNumOfOutFiles != 0) )
  {
    for( int i = 0; fOK && (i < pData->iNumOfOutFiles); i++ )
    {
      switch( pData->OutFile[i].eFormat )
      {
        case XML_FORMAT: TAWriteXMLPrefix( pData, pData->OutFile[i].hfOut, pTAInput->pInD->szDocTargetLang, FALSE ); break;
        case EXP_FORMAT: TAWriteEXPPrefix( pData, pData->OutFile[i].hfOut, pData->OutFile[i].fIncludeWordCountInfo, &(pData->OutFile[i].ulCountPos) ); break;
        case TMX_FORMAT: TAWriteTMXPrefix( pData, pData->OutFile[i].hfOut, pTAInput->pInD->szDocSourceLang, FALSE,  pData->OutFile[i].fIncludeWordCountInfo, &(pData->OutFile[i].ulCountPos) ); break;
        case XLIFF_FORMAT: TAWriteXLIFFPrefix( pData, pData->OutFile[i].hfOut, pTAInput->szFolder, pTAInput->pInD->szDocSourceLang, pTAInput->pInD->szDocTargetLang, FALSE ); break;
      } /* endswitch */
    } /* endfor */
  } /* endif */

   // create temporary memory for "no duplicates" processing
  if ( fOK && (pData->iNumOfOutFiles != 0) )
  {
    BOOL fNoDuplicates = FALSE;
    PTAINSTDATA  pInD = pTAInput->pInD;   
    PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;

    // check if no duplicates processing is required for any of the output files
    for( int i = 0; i < pData->iNumOfOutFiles; i++ )
    {
      fNoDuplicates |= pData->OutFile[i].fNoDuplicates;
    } /* endfor */

    if ( fNoDuplicates )
    {

      // get folder source language
      FolQueryInfo( pTAInput->szFolder, NULL, NULL, pData->szSourceLang, NULL, TRUE );

      // create the temporary translation Memory
      {
        MemoryFactory *pFactory = MemoryFactory::getInstance();    
        char szTempMemory[MAX_LONGFILESPEC]; 
        pLookupData->pDupMem = pFactory->createTempMemory( "$DM", szTempMemory, pData->szSourceLang );
        if ( pLookupData->pDupMem == NULL )
        {
          fOK = FALSE;
        } /* endif */
      }
      // prepare vector for memory lokup results
      for ( int i = 0; i < 3; i++ )
      {
        pLookupData->DupSearchProposals.push_back( new OtmProposal );
      } /* endfor */     
    }
    else
    {
      pLookupData->pDupMem = NULL;
    } /* endif */
  } /* endif */     

  return( fOK );
}

BOOL TAAppendFile( PMTOUTDATA pData, FILE *hfOut, PSZ pszAppendFile )
{
  BOOL fOK = TRUE;
  FILE *hfAppend = fopen( pszAppendFile, "rb" );

  if ( hfAppend)
  {
    int iRead = fread( pData->bUTF8Buffer, 1, sizeof(pData->bUTF8Buffer), hfAppend );
    while ( iRead )
    {
      fwrite( pData->bUTF8Buffer, 1, iRead, hfOut );
      iRead = fread( pData->bUTF8Buffer, 1, sizeof(pData->bUTF8Buffer), hfAppend );
    } /*endwhile */
    fclose( hfAppend );
  } /* endif */

  return( fOK );
} /*end of function TAAppendFile */

BOOL TAWriteDocXmlEnd( PMTOUTDATA pData, PMTOUTFILE pOutFile )
{
  BOOL fOK = TRUE;
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"      </p>\r\n" );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"    </d>\r\n" );

  if ( pOutFile->fWithMatch || pOutFile->fWithMatchAndSource )
  {
    // append matches to file
    if ( pOutFile->hfOutFuzzy )
    {
      TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, L"    </d>\r\n" );
      fclose( pOutFile->hfOutFuzzy );
      if ( pOutFile->fProposalsWritten )
      {
        TAAppendFile( pData, pOutFile->hfOut, pOutFile->szOutFuzzy );
      } /* endif */
      UtlDelete( pOutFile->szOutFuzzy, 0, FALSE  );
      pOutFile->hfOutFuzzy = NULL;
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of function TAWriteDocXmlEnd */

BOOL TAWriteDocXliffEnd( PMTOUTDATA pData, FILE *hfOut, int iReport)
{
  BOOL fOK = TRUE;

  iReport;

  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"</file>\r\n" );

  return( fOK );
} /* end of function TAWriteDocXliffEnd */

BOOL TAWriteMTSegmentToXmlFile( PMTOUTDATA pData, PMTOUTFILE pOutFile, ULONG ulLength, PSZ_W pszData, ULONG ulSegNum )
{
  BOOL fOK = TRUE;

  ulLength;

  // write doc start if this is a new document
  if ( strcmp( pData->szCurDoc, pOutFile->szCurDoc ) != 0 )
  {
    // end current doc (if any) 
    if ( pOutFile->szCurDoc[0] )
    {
      fOK = TAWriteDocXmlEnd( pData, pOutFile );
    } /* endif */

    strcpy( pOutFile->szCurDoc, pData->szCurDoc );

    // write start of new document
    if ( fOK )
    {
      CHAR szIso[20];
      TAGetIsoLang( pData->szSourceLang, szIso );
 
      // write d element up to document name
      swprintf ( pData->szUTF16Buffer, L"    <d lang=\"%S\" dt=\"ndoc\" url=\"", szIso );
      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
      
      // escape and write document name (use UTF8 buffer as temporary storage)
      MultiByteToWideChar( CP_ACP, 0, pData->szCurDoc, -1, (PSZ_W)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer)/sizeof(CHAR_W) );
      TAEscapeChars( (PSZ_W)pData->bUTF8Buffer, pData->szUTF16Buffer );
      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );

     // write rest of d element data
      swprintf ( pData->szUTF16Buffer, L"\" markup=\"%S\">\r\n", pData->szMarkup );
      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
      swprintf ( pData->szUTF16Buffer, L"      <p pid=\"%ld\">\r\n", pOutFile->iCurPar );
      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
    } /* endif */
  } /* endif */

  // write segment data
  if ( fOK )
  {
    if ( pOutFile->fIncludeWordCountInfo )
    {
      swprintf ( pData->szUTF16Buffer, L"        <s sid=\"%ld\" seg=\"%lu\" words=\"%lu\"><t tid=\"1\" tt=\"orig\">", pOutFile->iCurSeg, ulSegNum, pData->ulSegSourceWords );
    }
    else
    {
      swprintf ( pData->szUTF16Buffer, L"        <s sid=\"%ld\" seg=\"%lu\"><t tid=\"1\" tt=\"orig\">", pOutFile->iCurSeg, ulSegNum );
    } /* endif */
    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );

    TAEscapeChars( pszData, pData->szUTF16Buffer );
    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );

    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"</t></s>\r\n" );
  } /* endif */

  return( fOK );
} /* end of function TAWriteMTSegmentToXmlFile */

BOOL TAWriteMTSegmentToTmxFile( PTAINPUT pTAInput, PMTOUTDATA pData, PMTOUTFILE pOutFile, PSZ_W pszData, ULONG ulSegNum, ULONG ulSegSourceWords )
{
  BOOL fOK = TRUE;

  SYSTEMTIME systime;
  GetSystemTime(&systime);
  swprintf ( pData->szUTF16Buffer, L"      <tu tuid=\"%ld\" datatype=\"rtf\" creationdate=\"%4.4d%2.2d%2.2dT%2.2d%2.2d%2.2dZ\">\r\n",
    pOutFile->iCurSeg, systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );

  swprintf ( pData->szUTF16Buffer, L"      <prop type=\"tmgr:segNum\">%lu</prop>\r\n", ulSegNum );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
  swprintf ( pData->szUTF16Buffer, L"      <prop type=\"tmgr:markup\">%S</prop>\r\n", pTAInput->pInD->szDocFormat );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
  swprintf ( pData->szUTF16Buffer, L"      <prop type=\"tmgr:docname\">%S</prop>\r\n", pTAInput->pInD->pszCurSourceFile );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
  swprintf ( pData->szUTF16Buffer, L"      <prop type=\"tmgr:translFlag\">1</prop>\r\n" );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
  if ( pOutFile->fIncludeWordCountInfo )
  {
    swprintf ( pData->szUTF16Buffer, L"      <prop type=\"tmgr:words\">%lu</prop>\r\n", ulSegSourceWords );
    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
  } /* endif */

  CHAR szIso[20];
  TAGetIsoLang( pTAInput->pInD->szDocSourceLang, szIso );
  swprintf ( pData->szUTF16Buffer, L"      <tuv xml:lang=\"%S\">\r\n", szIso );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
  swprintf ( pData->szUTF16Buffer, L"        <prop type=\"tmgr:language\">%S</prop>\r\n", pTAInput->pInD->szDocSourceLang );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );

  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"        <seg>" );
  TAEscapeChars( pszData, pData->szUTF16Buffer );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"</seg>\r\n" );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"      </tuv>\r\n" );

  TAGetIsoLang( pTAInput->pInD->szDocTargetLang, szIso );
  swprintf ( pData->szUTF16Buffer, L"      <tuv xml:lang=\"%S\">\r\n", szIso );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
  swprintf ( pData->szUTF16Buffer, L"        <prop type=\"tmgr:language\">%S</prop>\r\n", pTAInput->pInD->szDocTargetLang );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"        <seg></seg>\r\n" );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"      </tuv>\r\n" );
  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"    </tu>\r\n" );


  return( fOK );
} /* end of function TAWriteMTSegmentToTmxFile */


BOOL TAWriteMTSegmentToExpFile( PTAINPUT pTAInput, PMTOUTFILE pOutFile, PSZ_W pszData, ULONG ulSegNum, ULONG ulSegSourceWords )
{
  BOOL fOK = TRUE;
  PSZ pTemp = (stricmp( pTAInput->pInD->pszCurSourceFile, pTAInput->apszLongNames[pTAInput->pInD->usCurNumSourceFile-1]) == 0) ?
            "": pTAInput->apszLongNames[pTAInput->pInD->usCurNumSourceFile-1];


  fputws( MEM_SEGMENT_BEGIN_TAGW, pOutFile->hfOut );
  fwprintf( pOutFile->hfOut, L"%ld\r\n", pOutFile->iCurSeg );
  fputws( MEM_CONTROL_BEGIN_TAGW, pOutFile->hfOut );

  fwprintf( pOutFile->hfOut, L"%06hu%s%c%s%016lu%s%S%s%S%s%S%s%S%s%S%s%S", ulSegNum, X15_STRW, L'1', X15_STRW,
                        0L, X15_STRW, pTAInput->pInD->szDocSourceLang, X15_STRW,
                        pTAInput->pInD->szDocTargetLang, X15_STRW, "", X15_STRW,
                        pTAInput->pInD->szDocFormat, X15_STRW, pTAInput->pInD->pszCurSourceFile, X15_STRW, pTemp );
  fputws( MEM_CONTROL_END_TAGW, pOutFile->hfOut );
  if ( pOutFile->fIncludeWordCountInfo )
  {
    fputws( MEM_ADDINFO_BEGIN_TAGW, pOutFile->hfOut );
    fwprintf( pOutFile->hfOut, L"<wcnt words=\"%lu\"></wcnt>", ulSegSourceWords );
    fputws( MEM_ADDINFO_END_TAGW, pOutFile->hfOut );
  } /* endif */

  fputws( MEM_SOURCE_BEGIN_TAGW, pOutFile->hfOut );
  PMTOUTDATA pData = (PMTOUTDATA)pTAInput->pInD->pCBMachTransFile;
  TAPrepareSegmentText( pszData, pData->szUTF16Buffer );
  fputws( pData->szUTF16Buffer , pOutFile->hfOut );
  fputws( MEM_SOURCE_END_TAGW, pOutFile->hfOut );
  fputws( MEM_TARGET_BEGIN_TAGW, pOutFile->hfOut );
  fputws( MEM_TARGET_END_TAGW, pOutFile->hfOut );
  fputws( MEM_SEGMENT_END_TAGW, pOutFile->hfOut );
  return( fOK );

} /* end of function TAWriteMTSegmentExpToFile */

BOOL TAWriteMTSegmentToXliffFile( PTAINPUT pTAInput, PMTOUTDATA pData, PMTOUTFILE pOutFile, PSZ pszCurDoc, 
                                  ULONG ulLength, PSZ_W pszData, ULONG ulSegNum, int iReport, int iCurSeg  )
{
  BOOL fOK = TRUE;

  ulLength; pszData; pTAInput;

  // write doc start if this is a new document
  if ( strcmp( pData->szCurDoc, pszCurDoc ) != 0 )
  {
    // end current doc (if any) 
    if ( pszCurDoc[0] )
    {
      fOK = TAWriteDocXliffEnd( pData, pOutFile->hfOut, iReport );
    } /* endif */

    strcpy( pszCurDoc, pData->szCurDoc );

    // write start of new document
    if ( fOK )
    {
  //  CHAR szSrcIso[20];
  //  CHAR szTgtIso[20];
  //  TAGetIsoLang( pData->szSourceLang, szSrcIso );
  //  TAGetIsoLang( pData->szTargetLang, szTgtIso );
 
      // write d element up to document name
      swprintf ( pData->szUTF16Buffer, L"\r\n<file original=\"" );
      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
      // GQ 2017/06/14 fix for P403726, do not reset iCurSeg (old code: pOutFile->iCurSeg = 1;)
      // iCurSeg = 1;
      
      // escape and write document name (use UTF8 buffer as temporary storage)
      MultiByteToWideChar( CP_ACP, 0, pData->szCurDoc, -1, (PSZ_W)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer)/sizeof(CHAR_W) );
      TAEscapeChars( (PSZ_W)pData->bUTF8Buffer, pData->szUTF16Buffer );
      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"\">\r\n" );

     // write meta data for this file
      wchar_t *szMDA_META = L" <mda:meta type=\"%s\">%S</mda:meta>\r\n" ;
      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"<mda:metadata><mda:metagroup category=\"otm\">\r\n" );
//    swprintf ( pData->szUTF16Buffer, szMDA_META, L"shortname", pTAInput->pInD->pszCurSourceFile );
//    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
      swprintf ( pData->szUTF16Buffer, szMDA_META, L"markup", pData->szMarkup );
      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
      swprintf ( pData->szUTF16Buffer, szMDA_META, L"srclang", pData->szSourceLang );
      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
      swprintf ( pData->szUTF16Buffer, szMDA_META, L"tgtlang", pData->szTargetLang );
      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"</mda:metagroup></mda:metadata>\r\n" );
    } /* endif */
  } /* endif */

  // write segment data
  if ( fOK )
  {
    swprintf ( pData->szUTF16Buffer, L"<unit id=\"%ld\" name=\"%ld\">\r\n", iCurSeg, ulSegNum );
    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
  } /* endif */

  return( fOK );
} /* end of function TAWriteMTSegmentToXliffFile */

BOOL TAWriteProposalsToXML( PTAINPUT pTAInput, PANALYSISMEMLOOKUPDATA pLookupData, PMTOUTDATA pData, PMTOUTFILE pOutFile, int iMatchesFound )
{
	int i = 0;
	SHORT sTId = 1;
  BOOL fOK = TRUE;

	// create proposal file if not done yet
  if ( pOutFile->fWithMatch || pOutFile->fWithMatchAndSource )
  {
	  if ( pOutFile->hfOutFuzzy == NULL  )
	  {
	    pOutFile->hfOutFuzzy = fopen( pOutFile->szOutFuzzy, "wb" ); 
      pOutFile->fProposalsWritten = FALSE;
	    fOK = (pOutFile->hfOutFuzzy != NULL); 
	    if ( fOK )
	    {
		    CHAR szIso[20];
		    TAGetIsoLang( pTAInput->pInD->szDocTargetLang , szIso );

        swprintf ( pData->szUTF16Buffer, L"    <d lang=\"%S\" dt=\"sdoc\" url=\"", szIso );
        if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );

        // escape and write document name (use UTF8 buffer as temporary storage)
        MultiByteToWideChar( CP_ACP, 0, pData->szCurDoc, -1, (PSZ_W)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer)/sizeof(CHAR_W) );
        TAEscapeChars( (PSZ_W)pData->bUTF8Buffer, pData->szUTF16Buffer );
        if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );

        // write rest of d element data
        swprintf ( pData->szUTF16Buffer, L"\" markup=\"%S\">\r\n", pData->szMarkup );
        if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );
	    } /* endif */
	  } /* endif */
  } /* endif */

  if ( pOutFile->fWithMatch )
  {
    if ( iMatchesFound != 0 )
    {
		  swprintf ( pData->szUTF16Buffer, L"        <s sid=\"%ld\" >", pOutFile->iCurSeg );
		  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );
      pOutFile->fProposalsWritten = TRUE;

		  while ( i < iMatchesFound )
		  {
			  pLookupData->BestProposals[i]->getDocName( pLookupData->szNameBuffer, sizeof(pLookupData->szNameBuffer) );
			  swprintf ( pData->szUTF16Buffer, L"<t tid=\"%d\" tt=\"fm\" sc=\"%ld\" seg=\"%ld\" url=\"", sTId++, 
				  pLookupData->BestProposals[i]->getFuzziness(), pLookupData->BestProposals[i]->getSegmentNum() );
			  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );

        // escape and write document name (use UTF8 buffer as temporary storage)
        MultiByteToWideChar( CP_ACP, 0, pLookupData->szNameBuffer, -1, (PSZ_W)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer)/sizeof(CHAR_W) );
        TAEscapeChars( (PSZ_W)pData->bUTF8Buffer, pData->szUTF16Buffer );
        if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );

        // write rest of element 
        wcscpy( pData->szUTF16Buffer, L"\">" );
        if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );

			  if ( fOK )
			  {
				  pLookupData->BestProposals[i]->getTarget( pLookupData->szSegmentBuffer, MAX_SEGMENT_SIZE );
				  TAEscapeChars( pLookupData->szSegmentBuffer, pData->szUTF16Buffer );
				  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );
				  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, L"</t>" );
			  } /* endif */
			  i++;
		  } /*endwhile */
		  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, L"</s>\r\n" );
	  } /* endif */
  }
  else if ( pOutFile->fWithMatchAndSource )
  {
    if ( iMatchesFound != 0 )
    {
      pOutFile->fProposalsWritten = TRUE;
      swprintf ( pData->szUTF16Buffer, L"        <s sid=\"%ld\">\r\n           <tgt>\r\n", pOutFile->iCurSeg );
	    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );

	    while ( i < iMatchesFound )
	    {
		    pLookupData->BestProposals[i]->getDocName( pLookupData->szNameBuffer, sizeof(pLookupData->szNameBuffer) );
		    swprintf ( pData->szUTF16Buffer, L"              <t tid=\"%d\" tt=\"fm\" sc=\"%ld\" seg=\"%ld\" url=\"", sTId++, 
			    pLookupData->BestProposals[i]->getFuzziness(), pLookupData->BestProposals[i]->getSegmentNum() );
		    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );

        // escape and write document name (use UTF8 buffer as temporary storage)
        MultiByteToWideChar( CP_ACP, 0, pLookupData->szNameBuffer, -1, (PSZ_W)pData->bUTF8Buffer, sizeof(pData->bUTF8Buffer)/sizeof(CHAR_W) );
        TAEscapeChars( (PSZ_W)pData->bUTF8Buffer, pData->szUTF16Buffer );
        if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );

        // write rest of element 
        wcscpy( pData->szUTF16Buffer, L"\">" );
        if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );

	      if ( fOK )
	      {
		      pLookupData->BestProposals[i]->getTarget( pLookupData->szSegmentBuffer, sizeof(pLookupData->szSegmentBuffer)/sizeof(CHAR_W)  );
		      TAEscapeChars( pLookupData->szSegmentBuffer, pData->szUTF16Buffer );
		      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );
		      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, L"</t>\r\n" );
	      } /* endif */
	      i++;
	    } /*endwhile */

	    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, L"           </tgt>\r\n           <src>\r\n" );

	    i = 0;
	    sTId = 1;
	    while ( i < iMatchesFound )
	    {
	      pLookupData->BestProposals[i]->getDocName( pLookupData->szNameBuffer, sizeof(pLookupData->szNameBuffer) );
	      swprintf ( pData->szUTF16Buffer, L"              <t tid=\"%d\" tt=\"fm\" sc=\"%u\" seg=\"%lu\" url=\"%S\">", sTId++, 
		      pLookupData->BestProposals[i]->getFuzziness(), pLookupData->BestProposals[i]->getSegmentNum(), 
		      pLookupData->szNameBuffer );
	      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );

	      if ( fOK )
	      {
		      pLookupData->BestProposals[i]->getSource( pLookupData->szSegmentBuffer, sizeof(pLookupData->szSegmentBuffer)/sizeof(CHAR_W)  );
		      TAEscapeChars( pLookupData->szSegmentBuffer, pData->szUTF16Buffer );
		      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, pData->szUTF16Buffer );
		      if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, L"</t>\r\n" );
	      } /* endif */
	      i++;
	    } /*endwhile */

	    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOutFuzzy, L"           </src>\r\n        </s>\r\n" );
    } /* endif */
  } /* endif */

  return( fOK );
} /* end of function TAWriteProposalsToXML */


BOOL TAWriteMTSegment( PTAINPUT pTAInput, ULONG ulLength, PSZ_W pszData, ULONG ulSegNum, USHORT usFuzzy, MATCHTYPE MatchType, PTBSEGMENT pSeg, ULONG ulSourceWords )
{
  BOOL fOK = TRUE;
  PMTOUTDATA pData = (PMTOUTDATA)pTAInput->pInD->pCBMachTransFile;
  PTAINSTDATA pInD = (PTAINSTDATA) pTAInput->pInD;
  PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;
  BOOL fIsDuplicate = FALSE;
  
  MatchType;

  // check if segment is a duplicate one
  if ( pLookupData->pDupMem != NULL )
  {
    OtmProposal SearchKey;
    SearchKey.setSource( pszData );
    SearchKey.setSegmentNum(ulSegNum );
    SearchKey.setMarkup( pTAInput->pInD->szDocFormat );
    SearchKey.setSourceLanguage( pTAInput->pInD->szDocSourceLang );
  	SearchKey.setDocName( "NODUPS" );
    // use source language for target as well - for duplicate check we use a source-source memory
    SearchKey.setTargetLanguage( pTAInput->pInD->szDocSourceLang );

    for ( int i = 0; i < (int)pLookupData->DupSearchProposals.size(); i++ )
    {
      pLookupData->DupSearchProposals[i]->clear();
    } /* endfor */         

    pLookupData->pDupMem->searchProposal( SearchKey, pLookupData->DupSearchProposals, 0 );

    fIsDuplicate = pLookupData->DupSearchProposals[0]->isExactMatch();
  } /* endif */     

  pData->ulSegSourceWords = ulSourceWords;
  int iMatchesFound = OtmProposal::getNumOfProposals( pLookupData->BestProposals );

  // check if this is a new document
  if ( strcmp( pTAInput->apszLongNames[pTAInput->pInD->usCurNumSourceFile-1], pData->szCurDoc ) != 0 )
  {
    strcpy( pData->szSourceLang, pTAInput->pInD->szDocSourceLang );
    strcpy( pData->szTargetLang, pTAInput->pInD->szDocTargetLang );
    strcpy( pData->szMarkup, pTAInput->pInD->szCurFormat );
    strcpy( pData->szCurDoc, pTAInput->apszLongNames[pTAInput->pInD->usCurNumSourceFile-1] );
  } /* endif */

  // write segment to all output files
  for( int i = 0; i < pData->iNumOfOutFiles; i++ )
  {
    PMTOUTFILE pOutFile = pData->OutFile + i;
    BOOL fAddToOutput = TRUE;

    // check against filter criteria
    if ( pOutFile->fNoDuplicates && fIsDuplicate )
    {
      // skip duplicate ones
      fAddToOutput = FALSE;
    }
    else if ( iMatchesFound == 0 )
    {
      // no match for this proposal, so add it to output file
      fAddToOutput = TRUE;
    }
    else if ( usFuzzy < 100) 
    {
      // check a segment having a fuzzy proposal

        // add segment with fuzzy proposal when its fuzziness is below the specified threshhold
      fAddToOutput = (usFuzzy <= pOutFile->usNoFuzzyAbove);
    }
    else
    {
      // check segments having an exact match

      OtmProposal::eProposalType eType = pLookupData->BestProposals[0]->getType();
      if ( pOutFile->fNoHamster && ((eType == OtmProposal::eptGlobalMemory) || (eType == OtmProposal::eptGlobalMemoryStar)) )
      {
        // do no add Hamster match
        fAddToOutput = FALSE;
      }
      else if ( pOutFile->fNoMachine && (eType == OtmProposal::eptMachine) )
      {
        // do no add MT match
        fAddToOutput = FALSE;
      }
      else if ( pOutFile->fNoExact && (eType == OtmProposal::eptManual) )
      {
        // do no add normal (=manual) exact match
        fAddToOutput = FALSE;
      }
      else
      {
        // add segment as no filter criteria matches this segment
        fAddToOutput = TRUE;
      } /* endif */
    } /* endif */

    // write segment to output file
    if ( fAddToOutput )
    {
      switch( pOutFile->eFormat )
      {
        case XML_FORMAT: 
          {
            fOK = TAWriteMTSegmentToXmlFile( pData, pOutFile, ulLength, pszData, ulSegNum );
            pOutFile->ulTotal += ulSourceWords;

            if ( pOutFile->fWithMatch || pOutFile->fWithMatchAndSource )
            {
              fOK = TAWriteProposalsToXML( pTAInput, pLookupData, pData, pOutFile, iMatchesFound );
            } /* endif */
            pOutFile->iCurSeg += 1;
            break;
          }

        case EXP_FORMAT: 
          {
            fOK = TAWriteMTSegmentToExpFile( pTAInput, pOutFile, pszData, ulSegNum, ulSourceWords );
            pOutFile->ulTotal += ulSourceWords;
            pOutFile->iCurSeg += 1;
            break;
          }

        case TMX_FORMAT: 
          {
            fOK = TAWriteMTSegmentToTmxFile( pTAInput, pData, pOutFile, pszData, ulSegNum, ulSourceWords );
            pOutFile->ulTotal += ulSourceWords;
            pOutFile->iCurSeg += 1;
            break;
          }

        case XLIFF_FORMAT: 
          {
            BOOL fIsTranslated = FALSE;

            fOK = TAWriteMTSegmentToXliffFile( pTAInput, pData, pOutFile, pOutFile->szCurDoc, ulLength, pszData, ulSegNum, 1, pOutFile->iCurSeg );
            pOutFile->ulTotal += ulSourceWords;
            pOutFile->iCurSeg += 1;

            if ( pSeg && pSeg->pDataW && *(pSeg->pDataW) && (pSeg->qStatus == QF_XLATED) ) 
              fIsTranslated = TRUE ;


            // write meta data for this segment
            {
               wchar_t *szMDA_META = L"  <mda:meta type=\"%s\">%s</mda:meta>\r\n" ;
               if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L" <mda:metadata><mda:metagroup category=\"otm\">\r\n" );
               swprintf( (wchar_t*)pData->bUTF8Buffer, L"%ld", ulSegNum ) ;
               swprintf ( pData->szUTF16Buffer, szMDA_META, L"segnum", (wchar_t*)pData->bUTF8Buffer );
               if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
               if ( pTAInput->pInD->szContext[0] ) {
                  TAEscapeXliffChars( pTAInput->pInD->szContext, (wchar_t*)pData->bUTF8Buffer );
                  swprintf ( pData->szUTF16Buffer, szMDA_META, L"context", (wchar_t*)pData->bUTF8Buffer );
                  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
               }
               if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L" </mda:metagroup></mda:metadata>\r\n" );
            }

            // write any fuzzy and exact matches
            {
              if ( iMatchesFound != 0 )
              {
                int i = 0;
                int iFuzziness ;
                SHORT sMatchId = 1;
                wchar_t  szMatchType[8];
                wchar_t  szOrigin[15];
                wchar_t *szINDENT2 = L"  ";
                wchar_t *szINDENT3 = L"   ";
                wchar_t *szINDENT4 = L"    ";
                wchar_t *szMDA_META  = L"%s<mda:meta type=\"%s\">%S</mda:meta>\r\n" ;
                wchar_t *szMDA_METAs = L"%s<mda:meta type=\"%s\">%s</mda:meta>\r\n" ;
                int iAddInfoLen;
                HADDDATAKEY hKey;

                swprintf ( pData->szUTF16Buffer, L" <mtc:matches>\r\n" );
                if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );

                while ( i < iMatchesFound )
                {
                  iFuzziness = pLookupData->BestProposals[i]->getFuzziness();
                  wcscpy( szMatchType, L"tm" ) ;
                  wcscpy( szOrigin, L"OpenTM2" ) ;
                  if ( iFuzziness == 100 ) 
                  {
                     if ( pLookupData->BestProposals[i]->getMatchType() == OtmProposal::emtExactExact) {
                        wcscpy( szMatchType, L"idm" ) ; 
                     } else
                     if ( pLookupData->BestProposals[i]->getMatchType() == OtmProposal::emtExactSameDoc) {
                        wcscpy( szMatchType, L"icm" ) ; 
                     } else
                     if ( pLookupData->BestProposals[i]->getType() == OtmProposal::eptMachine ) {
                        wcscpy( szMatchType, L"mt" ) ; 
                     }
                  }

                  iAddInfoLen = pLookupData->BestProposals[i]->getAddInfoLen();
                  if ( fOK && (iAddInfoLen != 0) ) {
                    pLookupData->BestProposals[i]->getAddInfo( pLookupData->szAddInfoBuffer, sizeof(pLookupData->szAddInfoBuffer) / sizeof(wchar_t) );
                    hKey = MADSearchKey( pLookupData->szAddInfoBuffer, L"HAMSTER" );
                    if ( hKey ) {
                       wcscpy( szOrigin, L"Hamster" ) ;
                    }
                  }

                  swprintf ( pData->szUTF16Buffer, L"%s<mtc:match id=\"%d\" origin=\"%s\" matchQuality=\"%d\" type=\"%s\">\r\n",
                             szINDENT2, sMatchId++, szOrigin, iFuzziness, szMatchType );
                  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );


                  // write meta data for this match   
                  {
                    swprintf ( pData->szUTF16Buffer, L"%s<mda:metadata><mda:metagroup category=\"otm\">\r\n", szINDENT3 );
                    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
              //    pLookupData->BestProposals[i]->getID( (CHAR*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE );
              //    swprintf ( pData->szUTF16Buffer, L"            <mda:meta type=\"otm:memnum\">%S</mda:meta>\r\n", (CHAR*)pData->bUTF8Buffer ); 
              //    if ( fOK ) fOK = TAWriteInUTF8( pData, pData->hfOut9, pData->szUTF16Buffer );
              //    swprintf ( pData->szUTF16Buffer, L"            <mda:meta type=\"otm:memnum\">%ld</mda:meta>\r\n", pLookupData->BestProposals[i]->getMemoryIndex() ); 
              //    if ( fOK ) fOK = TAWriteInUTF8( pData, pData->hfOut9, pData->szUTF16Buffer );
                    swprintf ( pData->szUTF16Buffer, L"%s<mda:meta type=\"docsegnum\">%ld</mda:meta>\r\n", szINDENT4, pLookupData->BestProposals[i]->getSegmentNum() ); 
                    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                    pLookupData->BestProposals[i]->getDocName( (CHAR*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE );
                    if ( ! pData->bUTF8Buffer[0] ) 
                       pLookupData->BestProposals[i]->getDocShortName( (CHAR*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE );
                    swprintf ( pData->szUTF16Buffer, szMDA_META, szINDENT4, L"docname", (CHAR*)pData->bUTF8Buffer ); 
                    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                    pLookupData->BestProposals[i]->getDocShortName( (CHAR*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE );
        //          swprintf ( pData->szUTF16Buffer, szMDA_META, szINDENT4, L"docshortname", (CHAR*)pData->bUTF8Buffer ); 
        //          if ( fOK ) fOK = TAWriteInUTF8( pData, pData->hfOut9, pData->szUTF16Buffer );
                    pLookupData->BestProposals[i]->getMarkup( (CHAR*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE );
                    swprintf ( pData->szUTF16Buffer, szMDA_META, szINDENT4, L"markup", (CHAR*)pData->bUTF8Buffer );
                    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                    swprintf ( pData->szUTF16Buffer, L"%s<mda:meta type=\"datetime\">%016lu</mda:meta>\r\n", szINDENT4, pLookupData->BestProposals[i]->getUpdateTime() );
                    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                    pLookupData->BestProposals[i]->getContext( pData->szUTF16Buffer, MAX_SEGMENT_SIZE );
                    if ( pData->szUTF16Buffer[0] ) {
                       TAEscapeXliffChars( pData->szUTF16Buffer, (wchar_t*)pData->bUTF8Buffer );
                       swprintf ( pData->szUTF16Buffer, L"%s<mda:meta type=\"context\">%s</mda:meta>\r\n", szINDENT4, (wchar_t*)pData->bUTF8Buffer );
                       if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                    }
           
                    // handle any annotation data of the match
                    {
                      if ( fOK && (iAddInfoLen != 0) )
                      {
                          pLookupData->BestProposals[i]->getAddInfo( pLookupData->szAddInfoBuffer, sizeof(pLookupData->szAddInfoBuffer) / sizeof(wchar_t) );
                          hKey = MADSearchKey( pLookupData->szAddInfoBuffer, L"Note" );
                          if ( hKey != NULL )
                          {
                            MADGetAttr( hKey, L"style", (wchar_t*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE, L"" );
                            if ( (wchar_t*)pData->bUTF8Buffer[0] ) {
                               swprintf ( pData->szUTF16Buffer, szMDA_METAs, szINDENT4, L"notestyle", (wchar_t*)pData->bUTF8Buffer ) ;
                               if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                            }
                            MDAGetValueForKey( hKey, (wchar_t*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE, L"" );
                            if ( (wchar_t*)pData->bUTF8Buffer[0] ) {
                               swprintf ( pData->szUTF16Buffer, szMDA_METAs, szINDENT4, L"note", (wchar_t*)pData->bUTF8Buffer ) ;
                               if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                            }
                          } /* endif */
           
                          hKey = MADSearchKey( pLookupData->szAddInfoBuffer, L"MT" );
                          if ( hKey != NULL )
                          {
                            MADGetAttr( hKey, L"TM:MatchType", (wchar_t*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE, L"" );
                            if ( (wchar_t*)pData->bUTF8Buffer[0] ) {
                               swprintf ( pData->szUTF16Buffer, szMDA_METAs, szINDENT4, L"MatchType", (wchar_t*)pData->bUTF8Buffer ) ;
                               if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                            }
                            MADGetAttr( hKey, L"MT:ServiceID", (wchar_t*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE, L"" );
                            if ( (wchar_t*)pData->bUTF8Buffer[0] ) {
                               swprintf ( pData->szUTF16Buffer, szMDA_METAs, szINDENT4, L"ServiceID", (wchar_t*)pData->bUTF8Buffer ) ;
                               if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                            }
                            MADGetAttr( hKey, L"MT:MetricName", (wchar_t*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE, L"" );
                            if ( (wchar_t*)pData->bUTF8Buffer[0] ) {
                               swprintf ( pData->szUTF16Buffer, szMDA_METAs, szINDENT4, L"MetricName", (wchar_t*)pData->bUTF8Buffer ) ;
                               if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                            }
                            MADGetAttr( hKey, L"MT:MetricValue", (wchar_t*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE, L"" );
                            if ( (wchar_t*)pData->bUTF8Buffer[0] ) {
                               swprintf ( pData->szUTF16Buffer, szMDA_METAs, szINDENT4, L"MetricValue", (wchar_t*)pData->bUTF8Buffer ) ;
                               if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                            }
                            MADGetAttr( hKey, L"PE:EditDistanceChars", (wchar_t*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE, L"" );
                            if ( (wchar_t*)pData->bUTF8Buffer[0] ) {
                               swprintf ( pData->szUTF16Buffer, szMDA_METAs, szINDENT4, L"EditDistance", (wchar_t*)pData->bUTF8Buffer ) ;
                               if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                            }
                        //  MADGetAttr( hKey, L"PE:EditDistanceWords", (wchar_t*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE, L"" );
                        //  if ( (wchar_t*)pData->bUTF8Buffer[0] ) {
                        //     swprintf ( pData->szUTF16Buffer, szMDA_METAs, szINDENT4, L"EditDistanceWords", (wchar_t*)pData->bUTF8Buffer ) ;
                        //     if ( fOK ) fOK = TAWriteInUTF8( pData, pData->hfOut9, pData->szUTF16Buffer );
                        //  }
                          } /* endif */
           
                          hKey = MADSearchKey( pLookupData->szAddInfoBuffer, L"HAMSTER" );
                          if ( hKey != NULL )
                          {
                        //  swprintf ( pData->szUTF16Buffer, szMDA_METAs, szINDENT4, L"ServiceID", L"Hamster" ) ;
                        //  if ( fOK ) fOK = TAWriteInUTF8( pData, pData->hfOut9, pData->szUTF16Buffer );
                            hKey = MADSearchKey( pLookupData->szAddInfoBuffer, L"CTID" );
                            if ( hKey != NULL ) {
                               MDAGetValueForKey( hKey, (wchar_t*)pData->bUTF8Buffer, MAX_SEGMENT_SIZE, L"" );
                               if ( (wchar_t*)pData->bUTF8Buffer[0] ) {
                                  swprintf ( pData->szUTF16Buffer, szMDA_METAs, szINDENT4, L"ctid", (wchar_t*)pData->bUTF8Buffer ) ;
                                  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                               }
                            }
                          } /* endif */
           
                      } /* endif */                      
                    }
                    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"   </mda:metagroup></mda:metadata>\r\n" );
                  }

                  if ( fOK )
                  {
                    swprintf ( pData->szUTF16Buffer, L"   <source>" );
                    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                    pLookupData->BestProposals[i]->getSource( pLookupData->szSegmentBuffer, MAX_SEGMENT_SIZE );
                    TAEscapeXliffChars( pLookupData->szSegmentBuffer, pData->szUTF16Buffer );
                    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                    swprintf ( pData->szUTF16Buffer, L"</source>\r\n" );
                    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );

                    swprintf ( pData->szUTF16Buffer, L"   <target>" );
                    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                    pLookupData->BestProposals[i]->getTarget( pLookupData->szSegmentBuffer, MAX_SEGMENT_SIZE );
                    TAEscapeXliffChars( pLookupData->szSegmentBuffer, pData->szUTF16Buffer );
                    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                    swprintf ( pData->szUTF16Buffer, L"</target>\r\n" );
                    if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                  } /* endif */

                  if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"  </mtc:match>\r\n" );

                  i++;
                } /*endwhile */
                if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L" </mtc:matches>\r\n" );
              } /* endif */
            } /* endif */

            // write source text
            {
              if ( fIsTranslated ) 
                swprintf ( pData->szUTF16Buffer, L" <segment state=\"translated\">\r\n" );
              else
                swprintf ( pData->szUTF16Buffer, L" <segment state=\"initial\">\r\n" );
              if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
              if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"  <source>" );
              TAEscapeXliffChars( pszData, pData->szUTF16Buffer );
              if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
              if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"</source>\r\n" );
            }

            // write target text
            {
              if ( fIsTranslated ) {
                 if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"  <target>" );
                 TAEscapeXliffChars( pSeg->pDataW, pData->szUTF16Buffer );
                 if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, pData->szUTF16Buffer );
                 if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"</target>\r\n" );
              }
            }

            if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L" </segment>\r\n" );

            if ( fOK ) fOK = TAWriteInUTF8( pData, pOutFile->hfOut, L"</unit>\r\n" );
          }
          break;
      } /* endswitch */
    } /* endif */
  } /* endfor */

  // add segment to temporary memory
  if ( pLookupData->pDupMem != NULL )
  {
    OtmProposal PropData;

	  PropData.setSource( pszData );
    PropData.setTarget( pszData );
	  PropData.setDocName( "NODUPS" );
	  PropData.setType( OtmProposal::eptManual );
	  PropData.setSegmentNum( ulSegNum );
	  PropData.setMarkup( pTAInput->pInD->szDocFormat );
	  PropData.setSourceLanguage( pTAInput->pInD->szDocSourceLang );
	  PropData.setTargetLanguage( pTAInput->pInD->szDocSourceLang );
	  pLookupData->pDupMem->putProposal( PropData );
  } /* endif */     

  return( fOK );
}


BOOL TAWriteXmlEnd( PMTOUTDATA pData, FILE *hfOut, BOOL fIncludeWordCountInfo, ULONG ulTotalWords )
{
  BOOL fOK = TRUE;
  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"   </docs>\r\n" );
  if ( fIncludeWordCountInfo )
  {
    if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"   <summary>\r\n" );
    swprintf( pData->szUTF16Buffer, L"      <totalWords>%lu</totalWords>\r\n", ulTotalWords );
    if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, pData->szUTF16Buffer );
    if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"   </summary>\r\n" );
  } /* endif */
  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"</rqt>\r\n" );
  return( fOK );
} /* end of function TAWriteXmlEnd */

BOOL TAWriteExpEnd( FILE *hfOut, BOOL fIncludeWordCountInfo, ULONG ulTotalWords, ULONG ulCountPos )
{
  BOOL fOK = TRUE;
  fputws( NTM_END_TAGW, hfOut );
  if ( fIncludeWordCountInfo )
  {
    // position to place for word count and overwrite the blanks with the actual word count data
    fseek( hfOut, ulCountPos, SEEK_SET );
    fwprintf( hfOut, L"\"%lu\"", ulTotalWords );
  } /* endif */
  return( fOK );
} /* end of function TAWriteExpEnd */

BOOL TAWriteTmxEnd( PMTOUTDATA pData, FILE *hfOut, BOOL fIncludeWordCountInfo, ULONG ulTotalWords, ULONG ulCountPos )
{
  BOOL fOK = TRUE;

  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"    </body>\r\n" );
  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"</tmx>\r\n" );
  if ( fIncludeWordCountInfo )
  {
    // position to place for word count and overwrite the blanks with the actual word count data
    fseek( hfOut, ulCountPos, SEEK_SET );
    swprintf( pData->szUTF16Buffer, L"%lu&quot;                                   ", ulTotalWords );
    pData->szUTF16Buffer[20] = EOS; // truncate at 20th character (to overwrite the initial value completeley
    if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, pData->szUTF16Buffer );
  } /* endif */



  return( fOK );
} /* end of function TAWriteTmxEnd */

BOOL TAWriteXliffEnd( PMTOUTDATA pData, FILE *hfOut )
{
  BOOL fOK = TRUE;
  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L" </file>\r\n" );
  if ( fOK ) fOK = TAWriteInUTF8( pData, hfOut, L"</xliff>\r\n" );
  return( fOK );
} /* end of function TAWriteXliffEnd */

BOOL TACloseMTFile( PTAINPUT pTAInput )
{
  BOOL fOK = TRUE;
  PMTOUTDATA pData = (PMTOUTDATA)pTAInput->pInD->pCBMachTransFile;

  if ( pData == NULL ) return( TRUE );

  // end current doc for XML type output files
  if ( pData->szCurDoc[0] != 0  )
  {
    for( int i = 0; fOK && (i < pData->iNumOfOutFiles); i++ )
    {
      if ( (pData->OutFile[i].eFormat == XML_FORMAT) && (pData->OutFile[i].hfOut != NULL) )
      {
        fOK = TAWriteDocXmlEnd( pData, pData->OutFile + i );
      } /* endif */
    } /* endfor */
  } /* endif */

  // write file end data
  for( int i = 0; fOK && (i < pData->iNumOfOutFiles); i++ )
  {
    PMTOUTFILE pOutFile = pData->OutFile + i;
    if (  pData->OutFile[i].hfOut != NULL ) 
    {
      switch ( pOutFile->eFormat )
      {
        case XML_FORMAT: fOK = TAWriteXmlEnd( pData, pOutFile->hfOut, pOutFile->fIncludeWordCountInfo, pOutFile->ulTotal ); break;
        case EXP_FORMAT: fOK = TAWriteExpEnd( pOutFile->hfOut, pOutFile->fIncludeWordCountInfo, pOutFile->ulTotal, pOutFile->ulCountPos ); break;
        case TMX_FORMAT: fOK = TAWriteTmxEnd( pData, pOutFile->hfOut, pOutFile->fIncludeWordCountInfo, pOutFile->ulTotal, pOutFile->ulCountPos  ); break;
        case XLIFF_FORMAT: fOK = TAWriteXliffEnd( pData, pOutFile->hfOut ); break;
      } /* endswitch */
    } /* endif */
  } /* endfor */

  // close output files
  for( int i = 0; i < pData->iNumOfOutFiles; i++ )
  {
    if ( pData->OutFile[i].hfOut ) fclose( pData->OutFile[i].hfOut );
  } /* endfor */
  
  // close and delete temporary memory
  if ( pData->hDupTM )
  {
      MemoryFactory *pFactory = MemoryFactory::getInstance();
      PTAINSTDATA pInD = (PTAINSTDATA) pTAInput->pInD;
      PANALYSISMEMLOOKUPDATA pLookupData = (PANALYSISMEMLOOKUPDATA)pInD->pvMemoryLookupData;

      if ( pLookupData != NULL )
      {
        if ( pLookupData->pDupMem != NULL )
        {
          pFactory->closeTempMemory( pLookupData->pDupMem );
          pLookupData->pDupMem = NULL;
        } /* endif */
      } /* endif */

      for ( int i = 0; i < (int)pLookupData->DupSearchProposals.size(); i++ )
      {
        delete pLookupData->DupSearchProposals[i];
      } /* endfor */         

  } /* endif */     
  
  // free data area
  if ( pData ) UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );
  pTAInput->pInD->pCBMachTransFile = NULL;

  return( fOK );
}


// functions for CRLF tag processing

// free the CRLF tag information data
void TAFreeCRLFInfo( PLOADEDCRLFINFO pInfo )
{
  if ( pInfo != NULL )
  {
    int i;
    for ( i = 0; i < pInfo->iUsed; i ++ )
    {
      UtlAlloc( (PVOID *) &(pInfo->apCrlfInfos[i]), 0, 0, NOMSG );
    } /* end */       
    UtlAlloc( (PVOID *) &pInfo, 0, 0, NOMSG );
  } /* end */     
} /* end of function TAFreeCRLFInfo */

// get the CRLF tags for a specific markup table
PCRLFINFO TAGetCRLFInfoForMarkup
(
  PLOADEDCRLFINFO pInfo,
  PSZ             pszMarkup
)
{
  if ( pInfo != NULL )
  {
    int i;
    for ( i = 0; i < pInfo->iUsed; i ++ )
    {
      if ( _stricmp( pszMarkup, pInfo->apCrlfInfos[i]->szMarkup ) == 0 )
      {
        return( pInfo->apCrlfInfos[i] );
      } /* end */         
    } /* end */       
  } /* end */     

  return( NULL );
} /* end of function TAGetCRLFInfoForMarkup */

//
// set CRLF repect flag depening on tags withing given segment
//
void TACheckForCRLFTags
(
  PCRLFINFO   pCrlfInfo,
  PSZ_W       pszSegData,
  PBOOL       pbCrlfFlag
)
{
  if ( pCrlfInfo != NULL )
  {
    while ( *pszSegData )
    {
      CHAR_W chStartChar = towupper(*pszSegData);
      if ( pCrlfInfo->fLookForEndTag )
      {
        // check for matching end tag
        if ( chStartChar == pCrlfInfo->Tags[pCrlfInfo->iActiveTags].szEndTag[0]  )
        {
          if ( _wcsnicmp( pszSegData, pCrlfInfo->Tags[pCrlfInfo->iActiveTags].szEndTag, pCrlfInfo->Tags[pCrlfInfo->iActiveTags].iEndTagLen ) == 0 )
          {
            pCrlfInfo->fLookForEndTag = FALSE;
            pCrlfInfo->iActiveTags = 0;
            *pbCrlfFlag = FALSE;
            pszSegData += pCrlfInfo->Tags[pCrlfInfo->iActiveTags].iEndTagLen - 1;
          } /* end */               
        } /* end */           
      }
      else 
      {
        // look for for start tags
        BOOL fFound = FALSE;
        int i = 0;
        while ( !fFound && (i < pCrlfInfo->iUsed) )
        {
          if ( chStartChar == pCrlfInfo->Tags[i].szStartTag[0]  )
          {
            if ( _wcsnicmp( pszSegData, pCrlfInfo->Tags[i].szStartTag, pCrlfInfo->Tags[i].iStartTagLen ) == 0 )
            {
              fFound = TRUE;
              pCrlfInfo->fLookForEndTag = TRUE;
              pCrlfInfo->iActiveTags = i;
              *pbCrlfFlag = TRUE;
              pszSegData += pCrlfInfo->Tags[i].iStartTagLen - 1;
            } /* end */               
          } /* end */           
          i++;
        } /* end */         
      } /* end */       
      pszSegData++;
    } /* end */       
  } /* end */     
} /* end of function TACheckForCRLFTags */

//
// load CRLF tag info from file
//
USHORT TALoadCRLFInfo
( 
  PLOADEDCRLFINFO *ppLoadedInfo
)
{
  FILE *hf = NULL;
  char szLine[512];
  PLOADEDCRLFINFO pLoadedInfo = NULL;
  PCRLFINFO pActiveCrlfInfo = NULL;
  USHORT usRC = 0;
  CHAR        szFileName[MAX_EQF_PATH];

  // preset caller's ppointer
  *ppLoadedInfo = NULL;

  // allocate inital area for loaded CRLF info
  if ( UtlAlloc( (PVOID *) &pLoadedInfo, 0L, sizeof(LOADEDCRLFINFO) + (sizeof(CRLFINFO) * 10), ERROR_STORAGE ) )
  {
    pLoadedInfo->iEntries = 10; 
    pLoadedInfo->iUsed = 0;
  }
  else
  {
    return( ERROR_NOT_ENOUGH_MEMORY );  
  } /* end */     

  // load and process name file
  if ( !usRC )
  {
    UtlMakeEQFPath( szFileName, NULC, TABLE_PATH, NULL );
    strcat( szFileName, "\\CRLFTAGS.LST" );
    hf = fopen( szFileName, "r" );
  } /* end */     

  if ( hf )
  {
    // read first line
    fgets( szLine, sizeof(szLine), hf );

    do
    {
      // strip-off LF at end of line
      int iLen = strlen(szLine);
      if ( iLen && (szLine[iLen-1] == '\n') ) szLine[iLen-1] = '\0';

      // handle line
      if ( szLine[0] == '*' )
      {
        // ignore comment line
      }
      else
      {
        // strip leading and trailing blanks
        {
          PSZ pszSource = szLine;
          PSZ pszTarget = szLine;

          while ( *pszSource == ' ') pszSource++;
          while ( *pszSource ) *pszTarget++ = *pszSource++; 
          *pszTarget = '\0';
          
          if ( szLine[0] )
          {
            pszSource = szLine + strlen(szLine) - 1;
            while ( (pszSource > szLine) && (*pszSource == ' ') )
            {
              *pszSource = '\0';
              pszSource--;
            } /*endwhile */
          } /* endif */
        }

        // handle data line
        if ( szLine[0] == '\0' )
        {
          // ignore empty line
        }
        else if ( szLine[0] == '[' )
        {
          char *pszEndBracket;

          // process new markup header

          // enlarge area if necessary
          if ( pLoadedInfo->iEntries <= pLoadedInfo->iUsed )
          {
            if ( UtlAlloc( (PVOID *) &pLoadedInfo, 0L, sizeof(LOADEDCRLFINFO) + (sizeof(CRLFINFO) * (pLoadedInfo->iEntries + 10)), ERROR_STORAGE ) )
            {
              pLoadedInfo->iEntries += 10; 
            }
            else
            {
              fclose( hf );
              return( ERROR_NOT_ENOUGH_MEMORY );  
            } /* end */     
          } /* end */             

          // find end of markup name and cut-off rest of line
          pszEndBracket = strchr( szLine, ']' );
          if ( pszEndBracket ) *pszEndBracket = '\0';

          // allocate new entry for this markup
          pActiveCrlfInfo = NULL;
          if ( UtlAlloc( (PVOID *) &pActiveCrlfInfo , 0L, sizeof(CRLFINFO) + (sizeof(STARTTAGENDTAG) * 10), ERROR_STORAGE ) )
          {
            pActiveCrlfInfo ->iEntries = 10; 
            pActiveCrlfInfo->iUsed = 0;
            pLoadedInfo->apCrlfInfos[pLoadedInfo->iUsed++] = pActiveCrlfInfo;
            strcpy( pActiveCrlfInfo->szMarkup, szLine + 1 );
          }
          else
          {
            fclose( hf );
            UtlAlloc( (PVOID *) &pLoadedInfo, 0L, 0L, NOMSG ) ;
            return( ERROR_NOT_ENOUGH_MEMORY );  
          } /* end */     
        }
        else if ( pActiveCrlfInfo != NULL  )
        {
          // handle text line
          char *pszEndTag = strchr( szLine, ',' );
          if ( *pszEndTag )
          {
            strupr( szLine );
            *pszEndTag = '\0';          // split line
            pszEndTag++;                // position to start of end tag
            if ( (strlen(szLine) < MAX_CRLFINFO_TAG_LEN) && (strlen(pszEndTag) < MAX_CRLFINFO_TAG_LEN) )
            {
              // enlarge start tag /end tag array when necessary
              if ( pActiveCrlfInfo->iEntries <= pActiveCrlfInfo->iUsed )
              {
                if ( UtlAlloc( (PVOID *) &pActiveCrlfInfo, 0L, sizeof(CRLFINFO) + (sizeof(STARTTAGENDTAG) * (pActiveCrlfInfo->iEntries + 10)), ERROR_STORAGE ) )
                {
                  pActiveCrlfInfo->iEntries += 10; 
                  pLoadedInfo->apCrlfInfos[pLoadedInfo->iUsed - 1] = pActiveCrlfInfo;
                }
                else
                {
                  fclose( hf );
                  UtlAlloc( (PVOID *) &pLoadedInfo, 0L, 0L, NOMSG ) ;
                  return( ERROR_NOT_ENOUGH_MEMORY );  
                } /* end */     
              } /* end */             

              // add new tags
              {
                PSTARTTAGENDTAG pTags = pActiveCrlfInfo->Tags + pActiveCrlfInfo->iUsed;
                pTags->iStartTagLen = MultiByteToWideChar( OEM_CP, 0L, szLine, -1, pTags->szStartTag, MAX_CRLFINFO_TAG_LEN - 1 ) - 1;
                pTags->iEndTagLen = MultiByteToWideChar( OEM_CP, 0L, pszEndTag, -1, pTags->szEndTag, MAX_CRLFINFO_TAG_LEN - 1 ) - 1;
              }

              pActiveCrlfInfo->iUsed++;
            } /* end */               
          } /* endif */
        } 
        else 
        {
          // ignore text line outside of a markup group
        } /* endif */
      } /* endif */

      // read next line
      fgets( szLine, sizeof(szLine), hf );

    } while ( !feof(hf) );
    fclose( hf );

    *ppLoadedInfo = pLoadedInfo;
  }
  else
  {
    usRC = 123; // ERROR_FILE_OPEN_FAILED;
  } /* endif */

  return( usRC );
} /* end of function TALoadCRLFInfo */
