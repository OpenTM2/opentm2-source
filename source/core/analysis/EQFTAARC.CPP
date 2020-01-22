/* \brief EQFTAARC.C - Functions for "Build Archive TM"
	Copyright (c) 1990-2014, International Business Machines Corporation and others. All rights reserved.
*/

#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TAGTABLE         // tagtable defines and functions
#define INCL_EQF_TP               // public translation processor functions
#define INCL_EQF_FOLDER           // folder list and document list functions
#include <eqf.h>                  // General Translation Manager include file

#include "eqfiana1.id"            // Analysis IDs

#undef _WPTMIF                         // we don't care about WP I/F
#include "eqfhelp.id"                  // help resource IDs
#include "eqfhlp1.h"                   // first part of help tables
#include "eqfmsg.htb"                          // message help table

#include "eqftai.h"               // Private include file of analysis
  #include "OTMFUNC.H"            // function call interface public defines
  #include "eqffunci.h"           // function call interface private defines

#include <string>
#include <vector>
#include "core\pluginmanager\PluginManager.h"
#include "core\pluginmanager\OtmMemory.h"
#include "core\pluginmanager\OtmMemoryPlugin.h"
#include "core\memory\MemoryFactory.h"

#include "eqftaarc.h"             // Private include file of archvie TM function


extern HELPSUBTABLE hlpsubtblArchTMDlg[];

USHORT TAFuncPrepArchTM
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  CHAR        chDrive,                 // drive letter
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // output Translation Memory
  LONG        lOptions                 // options for archive TM
);
USHORT TAFuncArchTMProcess
(
PFCTDATA    pData                    // ptr to function interface data area
);

void ArchTMCleanUp( PARCHTM_IDA pIda );

MRESULT TABatchArchTMProcCallBack
(
PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
HWND             hwnd,               // handle of process window
WINMSG           message,            // message to be processed
WPARAM           mp1,                // first message parameter
LPARAM           mp2                 // second message parameter
);


MRESULT TAArchTMProcCallBack
(
PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
HWND             hwnd,               // handle of process window
WINMSG           message,            // message to be processed
WPARAM           mp1,                // first message parameter
LPARAM           mp2                 // second message parameter
)
{
  PARCHTM_IDA      pIda;                // pointer to instance area
  MRESULT          mResult = FALSE;     // return code of procedure

  switch ( message)
  {
    /******************************************************************/
    /* WM_CREATE:                                                     */
    /*                                                                */
    /* Fill fields in communication area                              */
    /* Initialize data of callback function                           */
    /******************************************************************/
    case WM_CREATE :
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /**************************************************************/
        /* Anchor IDA                                                 */
        /**************************************************************/
        pIda                = (PARCHTM_IDA) PVOIDFROMMP2(mp2);
        pCommArea->pUserIDA = pIda;
        pIda->hwndErrMsg = hwnd;

        /****************************************************************/
        /* supply all information required to create the process        */
        /* window                                                       */
        /****************************************************************/
        pCommArea->sProcessWindowID = ID_ARCHTM_PROC_WINDOW;
        pCommArea->sProcessObjClass = clsANALYSIS;
        pCommArea->Style            = PROCWIN_SLIDERONLY;
        pCommArea->sSliderID        = ID_TASLIDER;
        LOADSTRING( NULLHANDLE, hResMod, IDS_ARCHTM_TITLE, pCommArea->szTitle );
        strcat( pCommArea->szTitle, pIda->szArchTM );
        pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_ANAICON); //hiconANA;
        pCommArea->fNoClose         = FALSE;
        pCommArea->swpSizePos.x     = 100;
        pCommArea->swpSizePos.y     = 100;
        pCommArea->swpSizePos.cx    = (SHORT)UtlQueryULong( QL_AVECHARWIDTH ) * 60;
        pCommArea->swpSizePos.cy    = (SHORT)UtlQueryULong( QL_PELSPERLINE ) * 10;
        pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
        pCommArea->asMsgsWanted[1]  = 0;
        pCommArea->usComplete       = 0;
      }
      break;

      /******************************************************************/
      /* WM_EQF_INITIALIZE:                                             */
      /*                                                                */
      /* Start the process                                              */
      /******************************************************************/
    case WM_EQF_INITIALIZE:
      {
        pIda     = (PARCHTM_IDA)pCommArea->pUserIDA;

        WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(BUILDARCHTM_TASK), 0L );
      }
      break;

      /******************************************************************/
      /* WM_CLOSE:                                                      */
      /*                                                                */
      /* Prepare/initialize shutdown of process                         */
      /******************************************************************/
    case WM_CLOSE:
      pIda = (PARCHTM_IDA) pCommArea->pUserIDA;
      if ( pIda )
      {
        pIda->fKill = TRUE;
        mResult = MRFROMSHORT( TRUE );   // = do not close right now
      }
      else
      {
        mResult = MRFROMSHORT( FALSE );  // = continue with close
      } /* endif */
      break;

      /******************************************************************/
      /* WM_DESTROY:                                                    */
      /*                                                                */
      /* Cleanup all resources used by the process                      */
      /******************************************************************/
    case WM_DESTROY:
      pIda = (PARCHTM_IDA) pCommArea->pUserIDA;
      if ( pIda )
      {
        if ( pIda->hwndDocLB != NULLHANDLE ) WinDestroyWindow( pIda->hwndDocLB );
        UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
        pCommArea->pUserIDA = NULL;
      } /* endif */
      break;

      /******************************************************************/
      /* WM_EQF_TERMINATE:                                              */
      /*                                                                */
      /* Allow or disable termination of process                        */
      /******************************************************************/
    case WM_EQF_TERMINATE:
      mResult = MRFROMSHORT( FALSE );          // = continue with close
      break;

    case WM_INITMENU:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

      /******************************************************************/
      /* WM_EQF_PROCESSTASK:                                            */
      /*                                                                */
      /* Do the actual processing                                       */
      /******************************************************************/
    case WM_EQF_PROCESSTASK:
      if ( SHORT1FROMMP1(mp1) == BUILDARCHTM_TASK )
      {
        pIda =(PARCHTM_IDA)  pCommArea->pUserIDA;
        if ( pIda->fKill )
        {
          USHORT  usMBCode;        // return code of message box call

          usMBCode = UtlError( ERROR_CANCELARCHTM, MB_YESNO,
                               0, NULL, EQF_QUERY );
          if ( usMBCode == MBID_YES )
          {
            // cleanup here!
            EqfRemoveObject( TWBFORCE, hwnd );
          }
          else
          {
            pIda->fKill = FALSE;
            WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                        MP1FROMSHORT(BUILDARCHTM_TASK), 0l );
          } /* endif */
        }
        else
        {
          /************************************************************/
          /* Handle the current processing step                       */
          /************************************************************/
          switch ( pIda->CurStep )
          {
            case PROCESS_INIT :
              /********************************************************/
              /* Initialize the process                               */
              /*                                                      */
              /* Create temp TM if required                           */
              /* Prepare next step                                    */
              /********************************************************/
              {
                BOOL fOK = TRUE;       // O.K. flag

                // create temporary TM if required
                if ( fOK && pIda->fClearTM )
                {
                  // remember original TM object
                  pIda->pMemOrg = pIda->pMem;
                  pIda->pMem = NULL;

                  // setup name for a temporary TM
                  strcpy( pIda->szTempTM, "$" );
                  strcat( pIda->szTempTM, pIda->szArchTM );
                  strcat( pIda->szTempTM, "-Temp" );

                  // create the temporary TM
                  MemoryFactory *pFactory = MemoryFactory::getInstance();
                  int iRC;

                  CHAR cDrive = 0;                   /* 7-10-14 */
                  OtmMemoryPlugin::PMEMORYINFO pArchMemInfo = new( OtmMemoryPlugin::MEMORYINFO );
                  pArchMemInfo->szOwner[0] = EOS;
                  if ( pFactory->getMemoryInfo( NULL, pIda->szArchTM, pArchMemInfo ) == 0 )
                  {
                     cDrive = pArchMemInfo->szFullPath[0];
                  }

                  OtmPlugin *pMemPlugin = (OtmPlugin *)pIda->pMemOrg->getPlugin();
                  pIda->pMemOrg->getDescription( pIda->szMemDescription, sizeof(pIda->szMemDescription) );
                  pIda->pMemOrg->getSourceLanguage( pIda->szMemSourceLang, sizeof(pIda->szMemSourceLang) );
                  pIda->pMem = pFactory->createMemory( (PSZ)pMemPlugin->getName(), pIda->szTempTM, pIda->szMemDescription, pIda->szMemSourceLang, cDrive, pArchMemInfo->szOwner, true, &iRC );

                  delete( pArchMemInfo );

                  if ( pIda->pMem == NULL )
                  {
                    pFactory->showLastError( NULL, pIda->szArchTM, NULL, pIda->hwndErrMsg );
                    fOK = FALSE;
                  } /* end */                     
                } /* endif */

                // load QF tag table
                if ( fOK )
                {
                  USHORT usRC;

                  usRC = TALoadTagTableExHwnd( QFTAG_TABLE, &(pIda->pLoadedQFTable), TRUE,   // load internal table
                                               0, TRUE, // do message handling 
                                               pIda->hwndErrMsg );
                  fOK = (usRC == NO_ERROR);
                } /* endif */

                // prepare next processing step
                if ( fOK )
                {
                  pIda->sMaxDocs = QUERYITEMCOUNTHWND( pIda->hwndDocLB );
                  pIda->sCurDoc  = -1;
                  pIda->CurStep = NEXT_DOCUMENT;
                }
                else
                {
                  pIda->CurStep = PROCESS_COMPLETE;
                  pIda->fErrorStop = TRUE;
                } /* endif */
              }
              break;

            case NEXT_DOCUMENT :
              /********************************************************/
              /* Position to next document, check if document has     */
              /* analyzed                                             */
              /********************************************************/
              pIda->sCurDoc++;
              if ( pIda->sCurDoc < pIda->sMaxDocs )
              {
                // Check existence of segmented source document (may not be
                // there if document has not been analyzed yet)

                // get name of active document
                QUERYITEMTEXTHWND( pIda->hwndDocLB, pIda->sCurDoc, pIda->szCurDoc );

                // build path of segmented source document
                UtlMakeEQFPath( pIda->szSourceDocName, pIda->szFolObjName[0],
                                DIRSEGSOURCEDOC_PATH, pIda->szFolder );
                strcat( pIda->szSourceDocName, BACKSLASH_STR );
                strcat( pIda->szSourceDocName, pIda->szCurDoc );

                // check if file exists
                if ( UtlFileExist(pIda->szSourceDocName) )
                {
                  // continue with this document
                  pIda->CurStep = START_DOCUMENT;
                }
                else
                {
                  // try next document
                  pIda->CurStep = NEXT_DOCUMENT;

                  // update slider position (actual slider update is done at
                  // the end of the processing switch)
                  pIda->usComplete = (USHORT)((LONG)pIda->sCurDoc * 100L /
                                              (LONG)pIda->sMaxDocs);
                } /* endif */
              }
              else
              {
                pIda->CurStep = PROCESS_COMPLETE;
              } /* endif */
              break;

            case START_DOCUMENT :
              /********************************************************/
              /* Load document                                        */
              /********************************************************/
              {
                BOOL fOK = TRUE;       // O.K. flag

                // get name of active document
                QUERYITEMTEXTHWND( pIda->hwndDocLB, pIda->sCurDoc, pIda->szCurDoc );

                // Get document info
                strcpy( pIda->szDocObjName, pIda->szFolObjName );
                strcat( pIda->szDocObjName, BACKSLASH_STR );
                strcat( pIda->szDocObjName, pIda->szCurDoc );

                DocQueryInfoEx( pIda->szDocObjName,   // document object name
                                NULL,                 // memory of document
                                pIda->szDocFormat,    // format of document
                                pIda->szDocSourceLang,// document source language
                                pIda->szDocTargetLang,// --   target language
                                pIda->szLongName,     // long document name
                                pIda->szAlias,        // alias
                                NULL,                 // editor
                                NULL,                 // conversion
                                &(pIda->ulMemTimeStamp),// document completion/update time
                                NULL,                 // unused
                                TRUE,                 // handle errors in function
                                pIda->hwndErrMsg );   // error message window

                if ( UtlIsLongFileName( pIda->szAlias ) )
                {
                  UtlLongToShortName( pIda->szAlias, pIda->szShortAlias );
                }
                else
                {
                  strcpy( pIda->szShortAlias, pIda->szAlias );
                  pIda->szAlias[0] = EOS;
                } /* endif */

                // allocate structure for segmented source document
                fOK = UtlAllocHwnd( (PVOID *) &pIda->pSourceDoc, 0L,
                                    sizeof(TBDOCUMENT), ERROR_STORAGE,
                                    pIda->hwndErrMsg );

                // load document tag table
                if ( fOK )
                {
                  fOK = (TALoadTagTableExHwnd( pIda->szDocFormat,
                                               (PLOADEDTABLE *)&(pIda->pSourceDoc->pDocTagTable),
                                               FALSE,
                                               TALOADUSEREXIT | TALOADGETSEGCONTEXTFUNC,
                                               TRUE, pIda->hwndErrMsg  ) == NO_ERROR);
                } /* endif */

                // load segmented source document
                if ( fOK )
                {
                  pIda->pSourceDoc->pQFTagTable = pIda->pLoadedQFTable;
                  UtlMakeEQFPath( pIda->szSourceDocName, pIda->szFolObjName[0],
                                  DIRSEGSOURCEDOC_PATH, pIda->szFolder );
                  strcat( pIda->szSourceDocName, BACKSLASH_STR );
                  strcat( pIda->szSourceDocName, pIda->szCurDoc );

                  // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to SrcLang
                  pIda->pSourceDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pIda->szDocSourceLang);
                  pIda->pSourceDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pIda->szDocSourceLang);

                  fOK = EQFBFileReadExW( pIda->szSourceDocName,
                                        pIda->pSourceDoc,
                                        FILEREAD_METADATA  ) == NO_ERROR;

                  // allocate structure for segmented target document
                  if ( fOK )
                  {
                    fOK = UtlAllocHwnd( (PVOID *) &pIda->pTargetDoc, 0L,
                                        sizeof(TBDOCUMENT), ERROR_STORAGE,
                                        pIda->hwndErrMsg );
                  } /* endif */

                  // load segmented target document
                  if ( fOK )
                  {
                    pIda->pTargetDoc->pQFTagTable = pIda->pLoadedQFTable;
                    pIda->pTargetDoc->docType = STARGET_DOC;
                    UtlMakeEQFPath( pIda->szTargetDocName, pIda->szFolObjName[0],
                                    DIRSEGTARGETDOC_PATH, pIda->szFolder );
                    strcat( pIda->szTargetDocName, BACKSLASH_STR );
                    strcat( pIda->szTargetDocName, pIda->szCurDoc );
                    strcpy( pIda->pTargetDoc->szDocName, pIda->szTargetDocName );

                     // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to SrcLang
                    pIda->pTargetDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pIda->szDocTargetLang);
                    pIda->pTargetDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pIda->szDocTargetLang);

                    fOK = EQFBFileReadExW( pIda->szTargetDocName,
                                          pIda->pTargetDoc,
                                          FILEREAD_METADATA ) == NO_ERROR;
                  } /* endif */

                  // determine next processing step
                  if ( fOK )
                  {
                    pIda->ulSegNum = 1;
                    pIda->ulAddSegNum = 1;
                    pIda->ulActiveTable = STANDARDTABLE;
                    pIda->CurStep = PROCESS_DOCUMENT;
                  }
                  else
                  {
                    pIda->fErrorStop = TRUE;
                    pIda->CurStep = PROCESS_COMPLETE;
                  } /* endif */
                } /* endif */

              }
              break;

            case PROCESS_DOCUMENT :
              /********************************************************/
              /* Work on loaded document                              */
              /********************************************************/
              {
                BOOL fOK = TRUE;       // O.K. flag
                int  i = 20;           // segment counter
                PTBSEGMENT pSourceSeg; // pointer to source segment
                PTBSEGMENT pTargetSeg; // pointer to target segment
                USHORT usRC = 0;       // return code of TmReplace call

                // process next 20 segments
                while ( fOK &&
                        (i > 0) &&
                        (pIda->ulSegNum <= pIda->pSourceDoc->ulMaxSeg) )
                {
                  ULONG ulTempTable, ulTempSeg, ulTempAddSeg, ulCurrentSegNum;

                  ulTempTable  = pIda->ulActiveTable;
                  ulTempSeg    = pIda->ulSegNum;
                  ulTempAddSeg = pIda->ulAddSegNum;
                  ulCurrentSegNum = pIda->ulSegNum;

                  // get segment pointers
                  pSourceSeg = EQFBGetFromBothTables( pIda->pSourceDoc,
                                                      &(pIda->ulSegNum),
                                                      &(pIda->ulAddSegNum),
                                                      &(pIda->ulActiveTable));
                  pTargetSeg = EQFBGetFromBothTables( pIda->pTargetDoc,
                                                      &ulTempSeg,
                                                      &ulTempAddSeg,
                                                      &ulTempTable );

                  // write segment to TM if translated
                  if ( (pSourceSeg != NULL) && (pTargetSeg != NULL) )
                  {
                    // GQ: fix for P031940: ensure that source and target segments are in-sync
                    if ( pTargetSeg->ulSegNum != pSourceSeg->ulSegNum )
                    {
                      pSourceSeg = EQFBGetSeg( pIda->pSourceDoc, pTargetSeg->ulSegNum );
                    } /* endif */

                    if ( (pSourceSeg == NULL) || (pTargetSeg->ulSegNum != pSourceSeg->ulSegNum) )
                    {
                      // no matching segment found...
                      int iError = 1;             // dummy statement to have a breakpoint location
          					  iError++;                   // and another one to get rid of compiler warning
                    }
                    else if ( pTargetSeg->qStatus == QF_XLATED )
                    {
                      OtmProposal Proposal;

                      Proposal.setUpdateTime( pIda->ulMemTimeStamp );
                      Proposal.setSource( pSourceSeg->pDataW );
                      Proposal.setTarget( pTargetSeg->pDataW );
                      Proposal.setDocShortName( pIda->szCurDoc );
                      if ( pIda->szShortAlias[0] != EOS )
                      {
                        Proposal.setDocShortName( pIda->szShortAlias );
                        Proposal.setDocName( pIda->szAlias );
                      }
                      else
                      {
                        Proposal.setDocName( pIda->szLongName );
                      } /* endif */

                      Proposal.setSegmentNum( pSourceSeg->ulSegNum );
                      Proposal.setMarkup( pIda->szDocFormat );
                      Proposal.setSourceLanguage( pIda->szDocSourceLang );
                      Proposal.setTargetLanguage( pIda->szDocTargetLang );

                      // handle any segment context
                      {
                        PLOADEDTABLE pTable = (PLOADEDTABLE)pIda->pSourceDoc->pDocTagTable;
                        if ( pTable->pfnGetSegContext )
                        {
                          // ulSegnum is already positioned on next segment!!!
                          PTBSEGMENT pPrevSeg = EQFBGetSegW( pIda->pSourceDoc, pIda->ulSegNum - 2 );
                          PTBSEGMENT pNextSeg = EQFBGetSegW( pIda->pSourceDoc, pIda->ulSegNum );
                          PSZ_W pszPrevSegData = ( pPrevSeg ) ? pPrevSeg->pDataW : NULL;
                          PSZ_W pszNextSegData = ( pNextSeg ) ? pNextSeg->pDataW : NULL;

                          (pTable->pfnGetSegContext)( pSourceSeg->pDataW,
                                                      pszPrevSegData,
                                                      pszNextSegData,
                                                      pIda->szContext,
                                                      (LONG)pIda->pSourceDoc, pIda->ulSegNum - 1  );
                          Proposal.setContext( pIda->szContext );
                        } /* endif */
                      }

                      // process any segment metadata
                      if ( pTargetSeg->pvMetadata != NULL )
                      {
                        MDConvertToMemMetadata( pTargetSeg->pvMetadata, pIda->szMetaData );
                        Proposal.setAddInfo( pIda->szMetaData );
                      } /* endif */

                      pIda->pMem->putProposal( Proposal );

                      if ( usRC == NO_ERROR )
                      {
                        pIda->ulSegments++;
                      }
                      else
                      {
                        fOK = FALSE;
                      } /* endif */
                    } /* endif */
                  } /* endif */

                  // continue with next segment
                  i--;
                  // pIda->ulSegNum++; // not required anymore ...
                } /* endwhile */

                // update slider position (actual slider update is done at
                // the end of the processing switch)
                if ( fOK )
                {
                  LONG  lPosInDoc, lSlotPerDoc, lPosOfDoc;

                  lPosInDoc = (LONG)pIda->ulSegNum * 100L /
                              (LONG)pIda->pSourceDoc->ulMaxSeg;
                  lSlotPerDoc = 100L / (LONG)pIda->sMaxDocs;
                  lPosOfDoc   = (LONG)pIda->sCurDoc * 100L /
                                (LONG)pIda->sMaxDocs;
                  pIda->usComplete = (USHORT)(lPosOfDoc +
                                              (lPosInDoc * lSlotPerDoc / 100L));
                } /* endif */

                // set next processing step
                if ( !fOK )
                {
                  pIda->CurStep = PROCESS_COMPLETE;
                  pIda->fErrorStop = TRUE;
                }
                else if ( pIda->ulSegNum > pIda->pSourceDoc->ulMaxSeg )
                {
                  pIda->CurStep = END_DOCUMENT;
                }
                else
                {
                  pIda->CurStep = PROCESS_DOCUMENT;
                } /* endif */
              }
              break;

            case END_DOCUMENT :
              /********************************************************/
              /* Terminate current document                           */
              /********************************************************/

              // free source document
              if ( pIda->pSourceDoc )
              {
                if ( pIda->pSourceDoc->pDocTagTable ) TAFreeTagTable( (PLOADEDTABLE) pIda->pSourceDoc->pDocTagTable  );
                TAFreeDoc( (PVOID *) &(pIda->pSourceDoc) );
                pIda->pSourceDoc = NULL;
              } /* endif */

              // free target document
              if ( pIda->pTargetDoc )
              {
                TAFreeDoc( (PVOID *) &(pIda->pTargetDoc) );
                pIda->pTargetDoc = NULL;
              } /* endif */

              pIda->CurStep = NEXT_DOCUMENT;
              break;

            case PROCESS_COMPLETE :
              ArchTMCleanUp( pIda );
              pIda->CurStep = END_PROCESS;
              break;

            default:
              break;
          } /* endswitch */

          /************************************************************/
          /* Prepare the next step                                    */
          /************************************************************/
          if ( pIda->CurStep != END_PROCESS )
          {
            UtlDispatch();
            WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                        MP1FROMSHORT(BUILDARCHTM_TASK), 0l );

            if ( pIda->usComplete != pIda->usLastComplete )
            {
              WinSendMsg( hwnd, WM_EQF_UPDATESLIDER,
                          MP1FROMSHORT(pIda->usComplete), NULL );
              pIda->usLastComplete = pIda->usComplete;
            } /* endif */
          }
          else
          {
            // set slider to 100%
            if ( !pIda->fErrorStop && !pIda->fBatch )
            {
              WinSendMsg( hwnd, WM_EQF_UPDATESLIDER,
                          MP1FROMSHORT(100), NULL );
            } /* endif */

            // show completion message
            if ( pIda->fErrorStop )
            {
              PSZ pszErrParm = pIda->szArchTM;

              UtlErrorHwnd( ERROR_ARCHTM_TERMINATED, MB_CANCEL, 1,
                            &pszErrParm, EQF_ERROR, pIda->hwndErrMsg );
            }
            else if ( pIda->ulSegments == 0L )
            {
              PSZ pszErrParm = pIda->szArchTM;

              UtlErrorHwnd( ERROR_ARCHTM_NOSEGSFOUND, MB_CANCEL, 1,
                            &pszErrParm, EQF_ERROR, pIda->hwndErrMsg );
            }
            else
            {
              PSZ   apszErrParm[2];
              CHAR  szSegNum[20];

              apszErrParm[0] = pIda->szArchTM;
              ltoa( pIda->ulSegments, szSegNum, 10 );
              apszErrParm[1] = szSegNum;

              UtlErrorHwnd( INFO_ARCHTM_COMPLETED, MB_OK, 2,
                            apszErrParm, EQF_INFO, pIda->hwndErrMsg );
            } /* endif */

            // end process window
            EqfRemoveObject( TWBFORCE, hwnd);
          } /* endif */
        } /* endif */
      } /* endif */
      break;
  } /* endswitch */
  return( mResult );
} /* end of function TAArchTMProcCallBack */


INT_PTR CALLBACK ARCHTMDLGPROC
(
HWND hwndDlg,                       /* handle of dialog window             */
WINMSG msg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT  mResult = MRFROMSHORT( FALSE );      // result value of procedure

  switch ( msg )
  {
    case WM_EQF_QUERYID: HANDLEQUERYID( ID_ARCHTM_DLG, mp2 ); break;

    case WM_INITDLG:
      mResult = ArchTMInit( hwndDlg, mp1, mp2 );
      break;

    case WM_HELP:
      /*************************************************************/
      /* pass on a HELP_WM_HELP request                            */
      /*************************************************************/
      {
        LPHELPINFO pHelpInfo = (LPHELPINFO)mp2;
        EqfDisplayContextHelp( (HWND) pHelpInfo->hItemHandle, &hlpsubtblArchTMDlg[0] );
        mResult = TRUE;  // message processed
      }
      break;

    case WM_COMMAND:
      mResult = ArchTMCommand( hwndDlg, WMCOMMANDID( mp1, mp2 ),
                               WMCOMMANDCMD( mp1, mp2 ) );
      break;

    case WM_CLOSE:
      mResult = ArchTMClose( hwndDlg, mp1, mp2 );
      break;

    default:
      mResult = WinDefDlgProc( hwndDlg, msg, mp1, mp2 );
      break;
  } /* endswitch */

  return mResult;
} /* end of ARCHTMDLGPROC */

MRESULT ArchTMInit
(
HWND    hwndDlg,                    // handle of folder export dialog window
WPARAM  mp1,                        // first parameter of WM_INITDLG
LPARAM  mp2                         // second parameter of WM_INITDLG
)
{
  PARCHTM_IDA  pIda;                  // ptr to archive TM IDA

  mp1 = mp1;                          // suppress 'unreferenced parameter' msg

  //--- adress IDA and store pointer to it ---
  pIda = (PARCHTM_IDA) mp2;
  if ( pIda )
  {
    ANCHORDLGIDA( hwndDlg, pIda );
  } /* endif */

  // fill document listbox and check for XLATE documents
  if ( pIda )
  {
    SHORT i = 0;                      // listbox item index
    HWND     hwndLB = GETHANDLEFROMID( hwndDlg, ID_ARCHTM_DOC_LB );

    // check for XLATE docs and add documents to document listbox
    pIda->fXlateDocs = FALSE;         // no XLATE docs found yet
    pIda->fOtherDocs = FALSE;         // no non-XLATE docs found yet

    pIda->sMaxDocs = QUERYITEMCOUNTHWND( pIda->hwndDocLB );

    while ( i < pIda->sMaxDocs )
    {
      QUERYITEMTEXTHWND( pIda->hwndDocLB, i, pIda->szCurDoc );

      // Get document info
      strcpy( pIda->szDocObjName, pIda->szFolObjName );
      strcat( pIda->szDocObjName, BACKSLASH_STR );
      strcat( pIda->szDocObjName, pIda->szCurDoc );
      pIda->szLongName[0] = EOS;

      DocQueryInfo2( pIda->szDocObjName,   // document object name
                     NULL,                 // memory of document
                     pIda->szDocFormat,    // format of document
                     NULL,                 // document source language
                     NULL,                 // --   target language
                     pIda->szLongName,     // long document name
                     NULL,                 // alias
                     NULL,                 // editor
                     TRUE );               // handle errors in function

      // check for XLATEWIN documents has been removed, XLATEWIN
      // documents should be now processed by archive TM function
//       if ( (strcmp( pIda->szDocFormat, XLATEWIN_MARKUP ) == 0) )
//       {
//         pIda->fXlateDocs = TRUE;
//       }
//       else
      {
        pIda->fOtherDocs = TRUE;
      } /* endif */

      // insert document into our listbox
      if ( pIda->szLongName[0] == EOS )
      {
        INSERTITEMHWND( hwndLB, pIda->szCurDoc );
      }
      else
      {
        OEMTOANSI( pIda->szLongName );
        INSERTITEMHWND( hwndLB, pIda->szLongName );
      } /* endif */

      i++;                      // next document
    } /* endwhile */
  } /* endif */

  {
    HWND hwndLB1 = GetDlgItem(hwndDlg, ID_ARCHTM_DOC_LB);
    UtlSetHorzScrollingForLB(hwndLB1);
  }


  // fill TM combobox
  if ( pIda )
  {
    EqfSend2Handler( MEMORYHANDLER, WM_EQF_INSERTNAMES,
                     MP1FROMHWND( WinWindowFromID( hwndDlg, ID_ARCHTM_TOTM_COMBO) ),
                     MP2FROMP( MEMORY_ALL ) );
    CBSETTEXTLIMIT( hwndDlg, ID_ARCHTM_TOTM_COMBO, MAX_LONGFILESPEC );
  } /* endif */

  // use folder TM as default
  { 
    SHORT sItem = 0;

    FolQueryInfo( pIda->szFolObjName,    // folder object name
                  pIda->szFolMemory,     // memory of folder
                  NULL,                  // format of folder
                  NULL,                  // folder source language
                  NULL,                  // folder target language
                  TRUE );                // handle errors in function
    CBSEARCHSELECT( sItem, hwndDlg, ID_ARCHTM_TOTM_COMBO, pIda->szFolMemory );
    SendDlgItemMessage( hwndDlg, ID_ARCHTM_TOTM_COMBO, CB_SETEDITSEL, (WPARAM)-1, (WPARAM)-1 );
  }



  // disable build pushbutton
  ENABLECTRL( hwndDlg, ID_ARCHTM_BUILD_PB, FALSE );

  // ensure that dialog is not positioned outside the screen area
  UtlCheckDlgPos( hwndDlg, FALSE );

  return( DIALOGINITRETURN(FALSE) );
} /* end of ArchTMInit */

MRESULT ArchTMCommand
(
HWND hwndDlg,                       // dialog handle
SHORT sId,                          // id of button
SHORT sNotification                 // notification type
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);
  PARCHTM_IDA pIda;                   // ptr to archive TM IDA
  BOOL        fOK;                    // internal OK flag

  // --- get IDA pointer ---
  pIda = ACCESSDLGIDA( hwndDlg, PARCHTM_IDA );

  switch ( sId )
  {
    case ID_ARCHTM_HELP_PB:
      UtlInvokeHelp();
      break;

    case ID_ARCHTM_BUILD_PB:
      fOK = TRUE;                   // assume everything is o.k.

      if ( fOK )
      {
        // Get selected TM
        QUERYTEXT( hwndDlg, ID_ARCHTM_TOTM_COMBO, pIda->szArchTM );
        UtlStripBlanks( pIda->szArchTM );

        // get other flags
        pIda->fUseAsFolderTM = QUERYCHECK( hwndDlg, ID_ARCHTM_USEFORFOLDER_CB );

        // call new TM dialog for new TMs or get replace/add confirmation
        {
          SHORT sItem;

          SETTEXT( hwndDlg, ID_ARCHTM_TOTM_COMBO, pIda->szArchTM );
          sItem = CBSEARCHITEM( hwndDlg, ID_ARCHTM_TOTM_COMBO,
                                pIda->szArchTM );
          if ( sItem == LIT_NONE)
          {
            //--- set 'dialog is disabled' flag
            // Note: using WinEnableWindow to disable the dialog
            //       does not work correctly (probably the dialog
            //       owner/parentship is not set correctly within
            //       the memory handler)
            pIda->fDisabled = TRUE;

            //--- get folder properties (use document values as buffer) ---
            FolQueryInfo( pIda->szFolObjName,    // folder object name
                          NULL,                  // memory of folder
                          pIda->szDocFormat,     // format of folder
                          pIda->szDocSourceLang, // folder source language
                          pIda->szDocTargetLang, // folder target language
                          TRUE );                // handle errors in function

            //--- call the memory handler to create a new TM ---
            strcat( pIda->szBuffer, pIda->szArchTM );
            strcat( pIda->szBuffer, X15_STR );
            strcat( pIda->szBuffer, pIda->szDocFormat );
            strcat( pIda->szBuffer, X15_STR );
            strcat( pIda->szBuffer, pIda->szDocSourceLang );
            strcat( pIda->szBuffer, X15_STR );
            strcat( pIda->szBuffer, pIda->szDocTargetLang );
            strcat( pIda->szBuffer, X15_STR );
            sItem = (SHORT)(!(BOOL) EqfSend2Handler( MEMORYHANDLER,
                                             WM_EQF_CREATE,
                                             MP1FROMSHORT(0),
                                             MP2FROMP( pIda->szBuffer ) ));

            //--- reset 'dialog is disabled' flag, dialog is active
            pIda->fDisabled = FALSE;
            SETFOCUSHWND( hwndDlg );

            if ( !sItem )
            {
              strcpy( pIda->szArchTM, pIda->szBuffer );
//              UtlDispatch();
            }
            else
            {
              fOK = FALSE;
            } /* endif */
          }
          else
          {
            // TM exists: get overwrite/add/cancel confirmation
            USHORT usMBReturn;        // return code of UtlError call

            usMBReturn = UtlError( QUERY_ARCHTM_EXISTS,
                                   MB_YESNOCANCEL | MB_DEFBUTTON2, 0,
                                   NULL, EQF_QUERY );
            switch ( usMBReturn )
            {
              case MBID_YES :
                pIda->fClearTM = TRUE;
                break;
              case MBID_NO :
                pIda->fClearTM = FALSE;
                break;
              default :
                fOK = FALSE;
                break;
            } /* endswitch */
          } /* endif */
        }

      } /* endif */

      // open TM
      if ( fOK )
      {
        int iRC = 0;
        MemoryFactory *pFactory = MemoryFactory::getInstance();

        pIda->pMem = pFactory->openMemory( NULL, pIda->szArchTM, EXCLUSIVE, &iRC );

        if ( iRC != 0 )
        {
          pFactory->showLastError( NULL, pIda->szArchTM, NULL, hwndDlg );
          fOK = FALSE;
        } /* endif */
      } /* endif */

      // end dialog and start archive TM process if O.K.
      if ( fOK )
      {
        POSTCLOSE( hwndDlg, TRUE );
      } /* endif */
      break;

    case ID_ARCHTM_TOTM_COMBO:
      ArchTMControl( hwndDlg, sId, sNotification );
      break;
    case ID_ARCHTM_CANCEL_PB:
    case DID_CANCEL:
      POSTCLOSE( hwndDlg, 0 );
      break;
  } /* endswitch */

  return( mResult );
} /* end of ArchTMCommand */

MRESULT ArchTMControl
(
HWND hwndDlg,                       // dialog handle
SHORT sId,                          // id of button
SHORT sNotification                 // notification type
)
{
  MRESULT mResult = MRFROMSHORT(TRUE);
  PARCHTM_IDA pIda;                   // ptr to archive TM IDA

  // --- get IDA pointer ---
  pIda = ACCESSDLGIDA( hwndDlg, PARCHTM_IDA );

  switch ( sId )
  {
    case ID_ARCHTM_TOTM_COMBO:
      if ( sNotification == CBN_EFCHANGE )
      {
        QUERYTEXT( hwndDlg, ID_ARCHTM_TOTM_COMBO, pIda->szArchTM );
        UtlStripBlanks( pIda->szArchTM );
        ENABLECTRL( hwndDlg, ID_ARCHTM_BUILD_PB, (pIda->szArchTM[0] != EOS) );
      }
      else if ( sNotification == CBN_SELCHANGE )
      {
        SHORT sItem;

        CBQUERYSELECTEDITEMTEXT( sItem, hwndDlg, ID_ARCHTM_TOTM_COMBO,
                                 pIda->szArchTM );
        UtlStripBlanks( pIda->szArchTM );
        ENABLECTRL( hwndDlg, ID_ARCHTM_BUILD_PB, (pIda->szArchTM[0] != EOS) );
      } /* endif */
      break;
  } /* endswitch */

  return( mResult );
} /* end of ArchTMControl */

MRESULT ArchTMClose
(
HWND hwndDlg,
WPARAM mp1,
LPARAM mp2
)
{
  MRESULT mResult = FALSE;
  PARCHTM_IDA pIda;                   // ptr to folder export IDA

  mp2 = mp2;                          // supress compiler warning

  // get IDA pointer
  pIda = ACCESSDLGIDA( hwndDlg, PARCHTM_IDA );

  if ( pIda )
  {
  } /* endif */

  //--- get rid off archive TM dialog ---
  WinDismissDlg( hwndDlg, SHORT1FROMMP1(mp1) );

  return( mResult );
} /* end of ArchTMClose */


VOID TABuildArchTM( HWND hwnd, PSZ pSelFolderName )
{
  PARCHTM_IDA     pIda = NULL;        // archive TM IDA
  BOOL            fOK = TRUE;         // return value

  hwnd;

  // create archive TM IDA
  fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG) sizeof(ARCHTM_IDA), ERROR_STORAGE );

  // create document listbox
  if ( fOK )
  {
    strcpy( pIda->szParentObjName, pSelFolderName );
    strcpy( pIda->szFolObjName, pSelFolderName );
    if ( FolIsSubFolderObject( pIda->szFolObjName ) )
    {
      // get main folder object name to allow the remaining code to work
      // without changes
      UtlSplitFnameFromPath( pIda->szFolObjName ); // cut off subfolder name
      UtlSplitFnameFromPath( pIda->szFolObjName ); // cut off property directory
    } /* endif */
    strcpy( pIda->szFolder, UtlGetFnameFromPath( pIda->szFolObjName ) );
    Utlstrccpy( pIda->szFolName, pIda->szFolder, DOT );




    pIda->hwndDocLB = WinCreateWindow( hwnd, WC_LISTBOX, "",
                                       WS_CHILDWINDOW | LBS_STANDARD,
                                       0, 0, 10, 10, hwnd,
                                       HWND_TOP, ID_TATODOLB, NULL, NULL);
    fOK = pIda->hwndDocLB != NULLHANDLE;
  } /* endif */

  // fill document listbox with document names
  if ( fOK )
  {
    USHORT         usRc;        // return code
    HWND           hwndActive;  // handle of active window

    //get Active folder and fill listbox with selected documents
    hwndActive = EqfQueryActiveFolderHwnd();

    //get status of active folder
    usRc = EqfQueryObjectStatus( hwndActive );
    if ( usRc & OBJ_FOCUS )             //documents selected in folder
    {
      EqfSend2Handler( FOLDERHANDLER, WM_EQF_QUERYSELECTEDNAMES,
                       MP1FROMHWND(pIda->hwndDocLB),
                       MP2FROMP(pIda->szParentObjName) );
    }
    else
    {
      EqfSend2Handler( FOLDERHANDLER, WM_EQF_INSERTNAMES,
                       MP1FROMHWND(pIda->hwndDocLB),
                       MP2FROMP(pIda->szFolObjName) );
    } /* endif */
  } /* endif */

  // show archive TM window
  if ( fOK )
  {
    HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
    DIALOGBOX( EqfQueryTwbClient(), ARCHTMDLGPROC, hResMod, ID_ARCHTM_DLG,
               pIda, fOK );
  } /* endif */

  // start archive TM process
  if ( fOK )
  {
    strcpy( pIda->szObjName, "ARCHTM:" );
    strcat( pIda->szObjName, pIda->szArchTM );
    fOK = CreateProcessWindow( pIda->szObjName, TAArchTMProcCallBack, pIda );
  } /* endif */

  // cleanup
  if ( !fOK )
  {
    if ( pIda != NULL )
    {
      if ( pIda->hwndDocLB != NULLHANDLE ) WinDestroyWindow( pIda->hwndDocLB );
      UtlAlloc( (PVOID *)&(pIda), 0L, 0L, NOMSG) ;
    } /* endif */
  } /* endif */
}

USHORT TABatchArchTM
(
HWND             hwndParent,         // handle of analysis handler window
PDDEARCHTM       pArchTM             // archive TM data structure
)
{
  BOOL             fOK = TRUE;        // internal O.K. flag
  PFCTDATA         pData = NULL;      // function I/F session data
  PSZ              pszFiles = NULL;   // file name buffer

  hwndParent;
  // allocate function call data area
  fOK = UtlAlloc( (PVOID *)&pData, 0L, (LONG)sizeof(FCTDATA), NOMSG );
  if ( !fOK )
  {
    pArchTM->DDEReturn.usRc = ERROR_STORAGE;
    UtlErrorHwnd( pArchTM->DDEReturn.usRc, MB_CANCEL, 0,
                  NULL, EQF_ERROR, pArchTM->hwndErrMsg );
  }
  else
  {
    pData->hwndErrMsg = pArchTM->hwndErrMsg;
  } /* endif */

  // setup document name string
  if ( fOK )
  {
    int i;
    LONG lLen = 0L;

    // compute length of required buffer
    for ( i = 0; i < pArchTM->usFileNums; i++ )
    {
      lLen += strlen(pArchTM->ppFileArray[i]) + 1;
    } /* endfor */
    lLen++;

    // allocate buffer
    fOK = UtlAlloc( (PVOID *)&pszFiles, 0L, lLen, NOMSG );
    if ( !fOK )
    {
      pArchTM->DDEReturn.usRc = ERROR_STORAGE;
      UtlErrorHwnd( pArchTM->DDEReturn.usRc, MB_CANCEL, 0,
                    NULL, EQF_ERROR, pArchTM->hwndErrMsg );
    } /* endif */

    // copy document names to buffer
    if ( fOK )
    {
      for ( i = 0; i < pArchTM->usFileNums; i++ )
      {
        if ( pszFiles[0] != EOS )
        {
          strcat( pszFiles, "," );
        } /* endif */
        strcat( pszFiles, pArchTM->ppFileArray[i] );
      } /* endfor */
    } /* endif */
  } /* endif */

  // use function interface to initialize process
  if ( fOK )
  {
    pArchTM->DDEReturn.usRc =
    TAFuncPrepArchTM( pData, pArchTM->szFolder, NULC, pszFiles,
                      pArchTM->szMemName,
                      pArchTM->fOverWrite ? OVERWRITE_OPT : 0L );
    fOK = (pArchTM->DDEReturn.usRc == NO_ERROR);
  } /* endif */

  // Cleanup or start process
  if ( pszFiles ) UtlAlloc( (PVOID *)&pszFiles, 0L, 0L, NOMSG );
  if ( !fOK )
  {
    if ( pData ) UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );

    // report end of task to DDE handler
    WinPostMsg( pArchTM->hwndOwner, WM_EQF_DDE_ANSWER, NULL,
                MP2FROMP(&pArchTM->DDEReturn) );
  }
  else
  {
    PARCHTM_IDA pIda = (PARCHTM_IDA)pData->pvArchTMIda;
    pIda->pDDEArchTM  = pArchTM;
    strcpy( pIda->szObjName, "ARCHTM:" );
    strcat( pIda->szObjName, pIda->szArchTM );
    fOK = CreateProcessWindow( pIda->szObjName, TABatchArchTMProcCallBack, pData );
  } /* endif */

  if ( !fOK ) pArchTM->DDEReturn.usRc = UtlGetDDEErrorCode( pArchTM->hwndErrMsg );

  return( pArchTM->DDEReturn.usRc );

} /* end of function TABatchArchTM */

USHORT TAFuncArchTM
(
PFCTDATA    pData,                   // function I/F session data
PSZ         pszFolderName,           // name of folder
CHAR        chToDrive,               // Drive (folder)
PSZ         pszDocuments,            // list with document names or NULL
PSZ         pszMemName,              // Translation Memory for analysis
LONG        lOptions                 // options for analysis
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new analysis run or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new archive TM run
    usRC = TAFuncPrepArchTM( pData, pszFolderName,
                             chToDrive, pszDocuments, pszMemName,
                             lOptions );
  }
  else
  {
    // continue current archive TM process
    usRC = TAFuncArchTMProcess( pData );
  } /* endif */
  return( usRC );
} /* end of function TAFuncArchTM */

// prepare the function I/F archive TM
USHORT TAFuncPrepArchTM
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszFolderName,           // name of folder
  CHAR        chToDrive,               // Drive
  PSZ         pszDocuments,            // list with document names or NULL
  PSZ         pszMemName,              // output Translation Memory
  LONG        lOptions                 // options for archive TM
)
{
  PSZ         pszParm;                 // pointer for error parameters
  BOOL        fOK = TRUE;              // internal O.K. flag
  USHORT      usRC = NO_ERROR;         // function return code
  USHORT      usDocuments = 0;         // number of documents being analyzed
  PSZ         pDocNameBuffer = NULL;   // document name buffer
  LONG        lBufferSize = 0L;        // size of document buffer
  LONG        lBufferUsed = 0L;        // used bytes in document buffer
  OBJNAME     szFolObject;             // folder object name
  PARCHTM_IDA pIda = NULL;
  CHAR        szFolShortName[MAX_FILESPEC];// buffer for folder short name
  CHAR        szFolderName[MAX_LONGFILESPEC];
  CHAR        szMemName[MAX_LONGFILESPEC];
  CHAR        szDocuments[MAX_LONGFILESPEC];

  // Check if folder target drive is valid
  if ( fOK && (chToDrive != EOS) )
  {
    CHAR szEqfDrives[MAX_DRIVELIST];   // buffer for EQF drive letters

    /* Get valid EQF drives                                           */
    UtlGetCheckedEqfDrives( szEqfDrives );

    /* Check if specified target drive is in list of valid drives     */
    if ( strchr( szEqfDrives, toupper(chToDrive) ) == NULL )
    {
      CHAR szDrive[MAX_DRIVE];

      fOK = FALSE;
      szDrive[0] = chToDrive;
      szDrive[1] = COLON;
      szDrive[2] = EOS;
      pszParm = szDrive;
      usRC = ERROR_EQF_DRIVE_NOT_VALID;
      UtlErrorHwnd( usRC, MB_CANCEL, 1,
                    &pszParm, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // check if folder exists
  if ( fOK )
  {
    if ( (pszFolderName == NULL) || (*pszFolderName == EOS) )
    {
      fOK = FALSE;
      usRC = TA_MANDFOLDER;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, pData->hwndErrMsg );
    }
    else
    {
      BOOL fIsNew;
      strcpy( szFolderName, pszFolderName);

      fIsNew = !SubFolNameToObjectName( szFolderName,  pData->szObjName );

      if ( chToDrive == EOS ) chToDrive = pData->szObjName[0];
      if ( pData->szObjName[0] != toupper(chToDrive))
      {
        fOK = FALSE;
        pszParm = pszFolderName;
        usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
        UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
      }
      if ( fOK)
      {
        if ( !fIsNew )
        {
          PSZ pszDelim;
          pszDelim = strchr( szFolderName, BACKSLASH );
          if ( pszDelim ) *pszDelim = EOS;
          ObjLongToShortName( szFolderName, szFolShortName, FOLDER_OBJECT, &fIsNew );
          if ( pszDelim ) *pszDelim = BACKSLASH;
        } /* endif */

        if ( fIsNew )
        {
          fOK = FALSE;
          pszParm = pszFolderName;
          usRC = ERROR_XLATE_FOLDER_NOT_EXIST;
          UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, HWND_FUNCIF );
        } /* endif */
      }
    } /* endif */
  } /* endif */

  // check if a TM has been specified
  if ( fOK )
  {
    if ( (pszMemName == NULL) || (*pszMemName == EOS) )
    {
      fOK = FALSE;
      UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL,
                    EQF_ERROR, pData->hwndErrMsg );
    } /* endif */
  } /* endif */


  // check if documents exist
  if ( fOK && (pszDocuments != NULL) && (*pszDocuments != EOS))
  {
    PSZ    pszTemp = pszDocuments;    // ptr for document list processing
    PSZ    pszDocNameStart;           // ptr for document list processing
    CHAR   chTemp;                    // buffer for current character

    // build folder object name (access to folder properties is
    // required to correct folder drive letter)
    {
      PPROPFOLDER  ppropFolder;        // pointer to folder properties
      HPROP        hpropFolder;        // folder properties handle
      ULONG        ulErrorInfo;        // error indicator from property handler

      UtlMakeEQFPath( szFolObject, chToDrive, SYSTEM_PATH, NULL );
      strcat( szFolObject, BACKSLASH_STR );
      strcat( szFolObject, szFolShortName );
      strcat( szFolObject, EXT_FOLDER_MAIN );
      hpropFolder = OpenProperties( szFolObject, NULL,
                                    PROP_ACCESS_READ, &ulErrorInfo);
      if ( hpropFolder )
      {
        ppropFolder = (PPROPFOLDER) MakePropPtrFromHnd( hpropFolder );
        if ( (ppropFolder->chDrive != EOS) && (ppropFolder->chDrive != ' ') )
        {
          szFolObject[0] = ppropFolder->chDrive;
        } /* endif */
        CloseProperties( hpropFolder, PROP_QUIT, &ulErrorInfo );
      } /* endif */
    }

    // isolate current document name
    strcpy(szDocuments, pszDocuments);
    pszDocNameStart = szDocuments;
    while ( fOK && (*pszDocNameStart != EOS) )
    {
      BOOL fIsNew = FALSE;

      // isolate current document name
      {
        // skip leading whitespace and seperators
        while ( (*pszDocNameStart == ' ') || (*pszDocNameStart == COMMA) )
        {
          pszDocNameStart++;
        } /* endwhile */

        // find end of document name
        pszTemp = pszDocNameStart;
        while ( *pszTemp && (*pszTemp != ' ') && (*pszTemp != COMMA) )
        {
          pszTemp++;
        } /* endwhile */
        chTemp = *pszTemp;
        *pszTemp = EOS;
      }

      if ( *pszDocNameStart != EOS)
      {
        CHAR szDocShortName[MAX_FILESPEC];

        FolLongToShortDocName( szFolObject, pszDocNameStart,
                               szDocShortName, &fIsNew );
        if ( fIsNew )
        {
          PSZ pszParms[2];
          fOK = FALSE;
          pszParms[0] = pszDocNameStart;
          pszParms[1] = pszFolderName;
          usRC = DDE_DOC_NOT_IN_FOLDR;
          UtlErrorHwnd( usRC, MB_CANCEL, 2, pszParms, EQF_ERROR,
                        pData->hwndErrMsg );
        }
        else
        {
          // add document short name to document name buffer
          LONG lAddLen = strlen(szDocShortName) + 1;
          if ( lBufferSize < (lBufferUsed + lAddLen) )
          {
            UtlAllocHwnd( (PVOID *)&pDocNameBuffer, lBufferSize,
                          lBufferSize + 8096L, ERROR_STORAGE,
                          pData->hwndErrMsg );
            lBufferSize += 8096L;
          } /* endif */

          if ( pDocNameBuffer != NULL )
          {
            strcpy( pDocNameBuffer + lBufferUsed, szDocShortName );
            lBufferUsed += lAddLen;
          } /* endif */
          usDocuments++;
        } /* endif */
      } /* endif */

      // next document name
      *pszTemp = chTemp;
      pszDocNameStart = pszTemp;
    } /* endwhile */
  } /* endif */

  // check if TM exist
  if ( fOK && (pszMemName != NULL) && (pszMemName[0] != EOS ))
  {
    strcpy(szMemName, pszMemName);
    if ( !UtlCheckIfExist( szMemName, TM_OBJECT ) )
    {
      fOK = FALSE;
      pszParm = szMemName;
      usRC = ERROR_MEMORY_NOTFOUND;
      UtlErrorHwnd( usRC, MB_CANCEL, 1, &pszParm, EQF_ERROR, pData->hwndErrMsg );
    } /* endif */
  } /* endif */

  // get number of documents if none specific documents have been specified
  if ( fOK )
  {
    if ( usDocuments == 0 )
    {
      usDocuments = LoadDocumentNames( pData->szObjName, HWND_FUNCIF,
                                       LOADDOCNAMES_INCLSUBFOLDERS,
                                       (PSZ)&pDocNameBuffer );
    } /* endif */
  } /* endif */

  // allocate storage for archive TM IDA
  if ( fOK )
  {
    fOK = UtlAlloc( (PVOID *)&pIda, 0L, (LONG)sizeof(ARCHTM_IDA), NOMSG );
    if ( !fOK )
    {
      usRC = ERROR_STORAGE;
      UtlErrorHwnd( usRC, MB_CANCEL, 0, NULL, EQF_ERROR, HWND_FUNCIF );
    } /* endif */
  } /* endif */

  // prepare IDA fields
  if ( fOK )
  {
    strcpy( pIda->szArchTM, szMemName );
    pIda->fUseAsFolderTM = FALSE;
    pIda->fClearTM         = (lOptions & OVERWRITE_OPT);
    pIda->fUseAsFolderTM   = (lOptions & USEASFOLDERTM_OPT);
    pIda->fSourceSourceMem = (lOptions & SOURCESOURCEMEM_OPT);
    pIda->fSetMFlag        = (lOptions & SETMFLAG_OPT);
    pIda->hwndErrMsg       = pData->hwndErrMsg;
    pIda->fBatch = TRUE;
    pIda->pszDocNames = pDocNameBuffer;
    strcpy( pIda->szFolObjName, pData->szObjName );
    strcpy( pIda->szFolder,     UtlGetFnameFromPath(pData->szObjName) );
    strcpy( pIda->szFolLongName, szFolderName );


    // check if folder is locked and lock it if not locked yet
    if ( fOK )
    {
      if ( QUERYSYMBOL( pIda->szFolObjName ) != -1 )
      {
        fOK = FALSE;
        pszParm = szFolderName;
        usRC = ERROR_FOLDER_LOCKED;
        UtlErrorHwnd( ERROR_FOLDER_LOCKED, MB_CANCEL, 1, &pszParm,
                      EQF_ERROR, HWND_FUNCIF );
      }
      else
      {
        SETSYMBOL( pIda->szFolObjName );
        pIda->fFolderLocked = TRUE;
      } /* endif */

    } /* endif */

    // open TM
    if ( fOK )
    {
      int iRC = 0;
      MemoryFactory *pFactory = MemoryFactory::getInstance();

      pIda->pMem = pFactory->openMemory( NULL, pIda->szArchTM, EXCLUSIVE, &iRC );

      if ( !fOK )
      {
        PSZ pszParm = pIda->szArchTM;
        UtlErrorHwnd( ERROR_TM_FOR_DOC_NOT_EXIST, MB_CANCEL, 1,
                      &pszParm, EQF_ERROR, pData->hwndErrMsg );
        fOK = FALSE;
      } /* endif */
    } /* endif */
  } /* endif */

  // Cleanup
  if ( !fOK )
  {
    if ( pIda != NULL )
    {
      if ( pIda->fFolderLocked ) REMOVESYMBOL( pIda->szFolObjName );
      UtlAlloc( (PVOID *)&pIda->pszDocNames, 0L, 0L, NOMSG) ;
      UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG) ;
    } /* endif */
  } /* endif */

  if ( fOK )
  {
    pData->fComplete = FALSE;
    pIda->CurStep = PROCESS_INIT;
    pData->pvArchTMIda = (PVOID)pIda;
    pIda->sMaxDocs = usDocuments;
  }
  else
  {
    usRC = UtlQueryUShort( QS_LASTERRORMSGID );
  } /* endif */
  return( usRC );

} /* end of function TAFuncPrepArchTM */

//----------------------------------------------------------------------------
// TaFuncArchTMProcess: Archive TM Function Of The Function Call Interface
//----------------------------------------------------------------------------
USHORT TAFuncArchTMProcess
(
PFCTDATA    pData                    // ptr to function interface data area
)
{
  PARCHTM_IDA      pIda;                // pointer to instance area
  USHORT     usRC = NO_ERROR;         // function return code
  CHAR szFolPath[MAX_EQF_PATH];// buffer for folder short name

  pIda = (PARCHTM_IDA)pData->pvArchTMIda;

  UtlSetUShort( QS_LASTERRORMSGID, 0 );

  // handle the current processing step
  switch ( pIda->CurStep )
  {
    case PROCESS_INIT :
      /********************************************************/
      /* Initialize the process                               */
      /*                                                      */
      /* Create temp TM if required                           */
      /* Prepare next step                                    */
      /********************************************************/
      {
        BOOL fOK = TRUE;       // O.K. flag

        // create temporary TM if required
        if ( fOK && pIda->fClearTM )
        {
          // remember original TM object
          pIda->pMemOrg = pIda->pMem;
          pIda->pMem = NULL;

          // setup name for a temporary TM
          strcpy( pIda->szTempTM, "$" );
          strcat( pIda->szTempTM, pIda->szArchTM );
          strcat( pIda->szTempTM, "-Temp" );

          // create the temporary TM
          MemoryFactory *pFactory = MemoryFactory::getInstance();
          int iRC;

          CHAR cDrive = 0;                          /* 7-10-14 */
          OtmMemoryPlugin::PMEMORYINFO pArchMemInfo = new( OtmMemoryPlugin::MEMORYINFO );
          if ( pFactory->getMemoryInfo( NULL, pIda->szArchTM, pArchMemInfo ) == 0 )
          {
             cDrive = pArchMemInfo->szFullPath[0];
          }
          delete( pArchMemInfo );

          OtmPlugin *pMemPlugin = (OtmPlugin *)pIda->pMemOrg->getPlugin();
          pIda->pMemOrg->getDescription( pIda->szMemDescription, sizeof(pIda->szMemDescription) );
          pIda->pMemOrg->getSourceLanguage( pIda->szMemSourceLang, sizeof(pIda->szMemSourceLang) );
          pIda->pMem = pFactory->createMemory( (PSZ)pMemPlugin->getName(), pIda->szTempTM, pIda->szMemDescription, pIda->szMemSourceLang, cDrive, &iRC );
          if ( pIda->pMem == NULL )
          {
            pFactory->showLastError( NULL, pIda->szArchTM, NULL, pIda->hwndErrMsg );
            fOK = FALSE;
          } /* end */                     
        }

        // load QF tag table
        if ( fOK )
        {
          USHORT usRC;

          usRC = TALoadTagTableHwnd( QFTAG_TABLE,
                                     &(pIda->pLoadedQFTable),
                                     TRUE,   // load internal table
                                     TRUE, // do message handling
                                     pIda->hwndErrMsg );
          fOK = (usRC == NO_ERROR);
        } /* endif */

        // prepare next processing step
        if ( fOK )
        {
          pIda->sCurDoc  = -1;
          pIda->CurStep = NEXT_DOCUMENT;
        }
        else
        {
          pIda->CurStep = PROCESS_COMPLETE;
          pIda->fErrorStop = TRUE;
        } /* endif */
      }
      break;

    case NEXT_DOCUMENT :


      /********************************************************/
      /* Position to next document, check if document has     */
      /* analyzed                                             */
      /********************************************************/
      pIda->sCurDoc++;
      if ( pIda->sCurDoc < pIda->sMaxDocs )
      {
        // Check existence of segmented source document (may not be
        // there if document has not been analyzed yet)

        // get name of active document
        {
          int i = pIda->sCurDoc;
          PSZ pszCurDoc = pIda->pszDocNames;
          while ( i )
          {
            pszCurDoc += strlen(pszCurDoc) + 1;
            i--;
          } /* endwhile */
          strcpy( pIda->szCurDoc, pszCurDoc );
        }

        // ******************** WL ******************
        if ( FolIsSubFolderObject( pIda->szFolObjName ) )
        {
          // get main folder object name to allow the remaining code to work
          // without changes
          UtlSplitFnameFromPath( pIda->szFolObjName ); // cut off subfolder name
          UtlSplitFnameFromPath( pIda->szFolObjName ); // cut off property directory
        } /* endif */
        strcpy( pIda->szFolder, UtlGetFnameFromPath( pIda->szFolObjName ) );
        //Utlstrccpy( pIda->szFolName, pIda->szFolder, DOT );

        // *******************WL**********************


        //ObjLongToShortName( pIda->szFolLongName, szFolShortName, FOLDER_OBJECT, &fIsNew );
        // build path of segmented source document
        //strcat( szFolShortName, EXT_FOLDER_MAIN );
        UtlMakeEQFPath( pIda->szSourceDocName, pIda->szFolObjName[0],
                        DIRSEGSOURCEDOC_PATH, pIda->szFolder );
        strcat( pIda->szSourceDocName, BACKSLASH_STR );
        strcat( pIda->szSourceDocName, pIda->szCurDoc );

        // check if file exists
        if ( UtlFileExist(pIda->szSourceDocName) )
        {
          // continue with this document
          pIda->CurStep = START_DOCUMENT;
        }
        else
        {
          // try next document
          pIda->CurStep = NEXT_DOCUMENT;

          // update slider position (actual slider update is done at
          // the end of the processing switch)
          pIda->usComplete = (USHORT)((LONG)pIda->sCurDoc * 100L /
                                      (LONG)pIda->sMaxDocs);
        } /* endif */
      }
      else
      {
        pIda->CurStep = PROCESS_COMPLETE;
      } /* endif */
      break;

    case START_DOCUMENT :
      /********************************************************/
      /* Load document                                        */
      /********************************************************/
      {
        BOOL fOK = TRUE;       // O.K. flag

        // ObjLongToShortName( pIda->szFolLongName, szFolShortName, FOLDER_OBJECT, &fIsNew );

        // strcat( szFolShortName, EXT_FOLDER_MAIN );
        UtlMakeEQFPath( szFolPath, pIda->szFolObjName[0], SYSTEM_PATH, pIda->szFolder );
        // build path of segmented source document
        // Get document info
        strcpy( pIda->szDocObjName, szFolPath );
        strcat( pIda->szDocObjName, BACKSLASH_STR );
        strcat( pIda->szDocObjName, pIda->szCurDoc );

//      DocQueryInfo2Hwnd( pIda->szDocObjName,   // document object name
//                         NULL,                 // memory of document
//                         pIda->szDocFormat,    // format of document
//                         pIda->szDocSourceLang,// document source language
//                         pIda->szDocTargetLang,// --   target language
//                         pIda->szLongName,     // long document name
//                         pIda->szAlias,        // alias
//                         NULL,                 // editor
//                         TRUE,                 // handle errors in function
//                         pIda->hwndErrMsg );   // error message window


        DocQueryInfoEx( pIda->szDocObjName,   // document object name
                        NULL,                 // memory of document
                        pIda->szDocFormat,    // format of document
                        pIda->szDocSourceLang,// document source language
                        pIda->szDocTargetLang,// --   target language
                        pIda->szLongName,     // long document name
                        pIda->szAlias,        // alias
                        NULL,                 // editor
                        NULL,                 // conversion
                        &(pIda->ulMemTimeStamp),// document completion/update time  9-19-18
                        NULL,                 // unused
                        TRUE,                 // handle errors in function
                        pIda->hwndErrMsg );   // error message window

        if ( UtlIsLongFileName( pIda->szAlias ) )
        {
          UtlLongToShortName( pIda->szAlias, pIda->szShortAlias );
        }
        else
        {
          strcpy( pIda->szShortAlias, pIda->szAlias );
          pIda->szAlias[0] = EOS;
        } /* endif */

        {
          // allocate structure for segmented source document
          fOK = UtlAllocHwnd( (PVOID *) &pIda->pSourceDoc, 0L,
                              sizeof(TBDOCUMENT), ERROR_STORAGE,
                              pIda->hwndErrMsg );

          // load document tag table
          if ( fOK )
          {
            fOK = (TALoadTagTableExHwnd( pIda->szDocFormat,
                                         (PLOADEDTABLE *)&(pIda->pSourceDoc->pDocTagTable),
                                         FALSE,
                                         TALOADUSEREXIT | TALOADGETSEGCONTEXTFUNC,
                                         TRUE, pIda->hwndErrMsg  ) == NO_ERROR);
          } /* endif */

          // load segmented source document
          if ( fOK )
          {

            pIda->pSourceDoc->pQFTagTable = pIda->pLoadedQFTable;
            UtlMakeEQFPath( pIda->szSourceDocName, pIda->szFolObjName[0],
                            DIRSEGSOURCEDOC_PATH, pIda->szFolder );
            strcat( pIda->szSourceDocName, BACKSLASH_STR );
            strcat( pIda->szSourceDocName, pIda->szCurDoc );


            // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to SrcLang
            pIda->pSourceDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pIda->szDocSourceLang);
            pIda->pSourceDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pIda->szDocSourceLang);

            fOK = EQFBFileReadExW( pIda->szSourceDocName,
                                  pIda->pSourceDoc,
                                  FILEREAD_METADATA  ) == NO_ERROR;
          } /* endif */

          // allocate structure for segmented target document
          if ( fOK )
          {
            fOK = UtlAllocHwnd( (PVOID *) &pIda->pTargetDoc, 0L,
                                sizeof(TBDOCUMENT), ERROR_STORAGE,
                                pIda->hwndErrMsg );
          } /* endif */

          // load segmented target document
          if ( fOK )
          {
            pIda->pTargetDoc->pQFTagTable = pIda->pLoadedQFTable;
            pIda->pTargetDoc->docType = STARGET_DOC;
            UtlMakeEQFPath( pIda->szTargetDocName, pIda->szFolObjName[0],
                            DIRSEGTARGETDOC_PATH, pIda->szFolder );
            strcat( pIda->szTargetDocName, BACKSLASH_STR );
            strcat( pIda->szTargetDocName, pIda->szCurDoc );
            strcpy( pIda->pTargetDoc->szDocName, pIda->szTargetDocName );


            // set TBDOCUMENT ulOEMCodePage/ulAnsiCodePage acc. to SrcLang
            pIda->pTargetDoc->ulOemCodePage = GetLangCodePage(OEM_CP, pIda->szDocTargetLang);
            pIda->pTargetDoc->ulAnsiCodePage = GetLangCodePage(ANSI_CP, pIda->szDocTargetLang);

            fOK = EQFBFileReadExW( pIda->szTargetDocName,
                                  pIda->pTargetDoc,
                                  FILEREAD_METADATA  ) == NO_ERROR;
          } /* endif */

          // determine next processing step
          if ( fOK )
          {
            pIda->ulSegNum = 1;
            pIda->ulAddSegNum = 1;
            pIda->ulActiveTable = STANDARDTABLE;
            pIda->CurStep = PROCESS_DOCUMENT;
          }
          else
          {
            pIda->fErrorStop = TRUE;
            pIda->CurStep = PROCESS_COMPLETE;
          } /* endif */
        }

      }
      break;

    case PROCESS_DOCUMENT :
      /********************************************************/
      /* Work on loaded document                              */
      /********************************************************/
      {
        BOOL fOK = TRUE;       // O.K. flag
        int  i = 20;           // segment counter
        PTBSEGMENT pSourceSeg; // pointer to source segment
        PTBSEGMENT pTargetSeg; // pointer to target segment
        int iRC = 0;           // return code of memory putProposal method

        // process next 20 segments
        while ( fOK &&
                (i > 0) &&
                (pIda->ulSegNum <= pIda->pSourceDoc->ulMaxSeg) )
        {
          // get segment pointers
          ULONG ulTempTable, ulTempSeg, ulTempAddSeg, ulCurrentSegNum;

          ulTempTable  = pIda->ulActiveTable;
          ulTempSeg    = pIda->ulSegNum;
          ulTempAddSeg = pIda->ulAddSegNum;
          ulCurrentSegNum = pIda->ulSegNum;

          // get segment pointers
          pSourceSeg = EQFBGetFromBothTables( pIda->pSourceDoc,
                                              &(pIda->ulSegNum),
                                              &(pIda->ulAddSegNum),
                                              &(pIda->ulActiveTable));
          pTargetSeg = EQFBGetFromBothTables( pIda->pTargetDoc,
                                              &ulTempSeg,
                                              &ulTempAddSeg,
                                              &ulTempTable );

          // write segment to TM
          if ( (pSourceSeg != NULL) && (pTargetSeg != NULL) )
          {
            // for source/source memory write all translatable segments
            if ( pIda->fSourceSourceMem )
            {
              if ( pTargetSeg->qStatus != QF_NOP )
              {
                OtmProposal Proposal;

                Proposal.setSource( pSourceSeg->pDataW );
                //Proposal.setTarget( pTargetSeg->pDataW );
                Proposal.setTarget( pSourceSeg->pDataW );

                Proposal.setDocShortName( pIda->szCurDoc );
                if ( pIda->szShortAlias[0] != EOS )
                {
                  Proposal.setDocShortName( pIda->szShortAlias );
                  Proposal.setDocName( pIda->szAlias );
                }
                else
                {
                  Proposal.setDocName( pIda->szLongName );
                } /* endif */

                Proposal.setSegmentNum( pSourceSeg->ulSegNum );
                Proposal.setMarkup( pIda->szDocFormat );
                Proposal.setSourceLanguage( pIda->szDocSourceLang );
                Proposal.setTargetLanguage( pIda->szDocTargetLang );

                // handle any segment context
                {
                  PLOADEDTABLE pTable = (PLOADEDTABLE)pIda->pSourceDoc->pDocTagTable;
                  if ( pTable->pfnGetSegContext )
                  {
                    // ulSegnum is already positioned on next segment!!!
                    PTBSEGMENT pPrevSeg = EQFBGetSegW( pIda->pSourceDoc, pIda->ulSegNum - 2 );
                    PTBSEGMENT pNextSeg = EQFBGetSegW( pIda->pSourceDoc, pIda->ulSegNum );
                    PSZ_W pszPrevSegData = ( pPrevSeg ) ? pPrevSeg->pDataW : NULL;
                    PSZ_W pszNextSegData = ( pNextSeg ) ? pNextSeg->pDataW : NULL;

                    (pTable->pfnGetSegContext)( pSourceSeg->pDataW,
                                                pszPrevSegData,
                                                pszNextSegData,
                                                pIda->szContext,
                                                (LONG)pIda->pSourceDoc, pIda->ulSegNum - 1  );
                    Proposal.setContext( pIda->szContext );
                  } /* endif */
                }

                // process any segment metadata
                if ( pTargetSeg->pvMetadata != NULL )
                {
                  MDConvertToMemMetadata( pTargetSeg->pvMetadata, pIda->szMetaData );
                  Proposal.setAddInfo( pIda->szMetaData );
                } /* endif */

                pIda->pMem->putProposal( Proposal );

                if ( iRC == NO_ERROR )
                {
                  pIda->ulSegments++;
                }
                else
                {
                  fOK = FALSE;
                } /* endif */
              } /* endif */
            }
            else
            {
              // write translated segments only
              if ( pTargetSeg->qStatus == QF_XLATED )
              {
                OtmProposal Proposal;

                Proposal.setUpdateTime( pIda->ulMemTimeStamp );   /* 9-19-18 */
                Proposal.setSource( pSourceSeg->pDataW );
                Proposal.setTarget( pTargetSeg->pDataW );
                Proposal.setDocShortName( pIda->szCurDoc );
                if ( pIda->szShortAlias[0] != EOS )
                {
                  Proposal.setDocShortName( pIda->szShortAlias );
                  Proposal.setDocName( pIda->szAlias );
                }
                else
                {
                  Proposal.setDocName( pIda->szLongName );
                } /* endif */

                Proposal.setSegmentNum( pSourceSeg->ulSegNum );
                Proposal.setMarkup( pIda->szDocFormat );
                Proposal.setSourceLanguage( pIda->szDocSourceLang );
                Proposal.setTargetLanguage( pIda->szDocTargetLang );

                // handle any segment context
                {
                  PLOADEDTABLE pTable = (PLOADEDTABLE)pIda->pSourceDoc->pDocTagTable;
                  if ( pTable->pfnGetSegContext )
                  {
                    // ulSegnum is already positioned on next segment!!!
                    PTBSEGMENT pPrevSeg = EQFBGetSegW( pIda->pSourceDoc, pIda->ulSegNum - 2 );
                    PTBSEGMENT pNextSeg = EQFBGetSegW( pIda->pSourceDoc, pIda->ulSegNum );
                    PSZ_W pszPrevSegData = ( pPrevSeg ) ? pPrevSeg->pDataW : NULL;
                    PSZ_W pszNextSegData = ( pNextSeg ) ? pNextSeg->pDataW : NULL;

                    (pTable->pfnGetSegContext)( pSourceSeg->pDataW,
                                                pszPrevSegData,
                                                pszNextSegData,
                                                pIda->szContext,
                                                (LONG)pIda->pSourceDoc, pIda->ulSegNum - 1  );
                    Proposal.setContext( pIda->szContext );
                  } /* endif */
                }

                // process any segment metadata
                if ( pTargetSeg->pvMetadata != NULL )
                {
                  MDConvertToMemMetadata( pTargetSeg->pvMetadata, pIda->szMetaData );
                  Proposal.setAddInfo( pIda->szMetaData );
                } /* endif */

                pIda->pMem->putProposal( Proposal );

                if ( iRC == NO_ERROR )
                {
                  pIda->ulSegments++;
                }
                else
                {
                  fOK = FALSE;
                } /* endif */
              } /* endif */
            } /* endif */
          } /* endif */

          // continue with next segment
          i--;
          // pIda->ulSegNum++; not required anymore
        } /* endwhile */

        // update slider position (actual slider update is done at
        // the end of the processing switch)
        if ( fOK )
        {
          LONG  lPosInDoc, lSlotPerDoc, lPosOfDoc;

          lPosInDoc = (LONG)pIda->ulSegNum * 100L /
                      (LONG)pIda->pSourceDoc->ulMaxSeg;
          lSlotPerDoc = 100L / (LONG)pIda->sMaxDocs;
          lPosOfDoc   = (LONG)pIda->sCurDoc * 100L /
                        (LONG)pIda->sMaxDocs;
          pIda->usComplete = (USHORT)(lPosOfDoc +
                                      (lPosInDoc * lSlotPerDoc / 100L));
        } /* endif */

        // set next processing step
        if ( !fOK )
        {
          pIda->CurStep = PROCESS_COMPLETE;
          pIda->fErrorStop = TRUE;
        }
        else if ( pIda->ulSegNum > pIda->pSourceDoc->ulMaxSeg )
        {
          pIda->CurStep = END_DOCUMENT;
        }
        else
        {
          pIda->CurStep = PROCESS_DOCUMENT;
        } /* endif */
      }
      break;

    case END_DOCUMENT :
      /********************************************************/
      /* Terminate current document                           */
      /********************************************************/

      // free source document
      if ( pIda->pSourceDoc )
      {
        if ( pIda->pSourceDoc->pDocTagTable ) TAFreeTagTable((PLOADEDTABLE) pIda->pSourceDoc->pDocTagTable );
        TAFreeDoc((PVOID *)&(pIda->pSourceDoc) );
        pIda->pSourceDoc = NULL;
      } /* endif */

      // free target document
      if ( pIda->pTargetDoc )
      {
        TAFreeDoc( (PVOID *)&(pIda->pTargetDoc) );
        pIda->pTargetDoc = NULL;
      } /* endif */

      pIda->CurStep = NEXT_DOCUMENT;
      break;

    case PROCESS_COMPLETE :
      ArchTMCleanUp( pIda );

      if ( pIda->fFolderLocked ) REMOVESYMBOL( pIda->szFolObjName );
      pIda->fFolderLocked = FALSE;
      pIda->CurStep = END_PROCESS;
      break;

    default:
      break;
  } /* endswitch */

  if ( pIda->fErrorStop )
  {
    usRC = UtlQueryUShort( QS_LASTERRORMSGID );
  } /* endif */

  if ( pIda->CurStep == END_PROCESS )
  {
    if ( pData->hwndErrMsg == HWND_FUNCIF )    // if we are in normal function call mode ...
    {
      // ... free our internal memory areas
      UtlAlloc( (PVOID *)&pIda->pszDocNames, 0L, 0L, NOMSG );    // free instance data space
      UtlAlloc( (PVOID *)&pIda, 0L, 0L, NOMSG );    // free instance data space
    } /* endif */
    pData->fComplete = TRUE;
  } /* endif */

  return( usRC );
} /* end of function TAFuncArchTMProcess */


MRESULT TABatchArchTMProcCallBack
(
PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
HWND             hwnd,               // handle of process window
WINMSG           message,            // message to be processed
WPARAM           mp1,                // first message parameter
LPARAM           mp2                 // second message parameter
)
{
  PARCHTM_IDA      pIda;                // pointer to instance area
  PFCTDATA         pData;               // function I/F session data
  MRESULT          mResult = FALSE;     // return code of procedure

  switch ( message)
  {
    /******************************************************************/
    /* WM_CREATE:                                                     */
    /*                                                                */
    /* Fill fields in communication area                              */
    /* Initialize data of callback function                           */
    /******************************************************************/
    case WM_CREATE :
      {
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /**************************************************************/
        /* Anchor IDA                                                 */
        /**************************************************************/
        pData               = (PFCTDATA) PVOIDFROMMP2(mp2);
        pIda = (PARCHTM_IDA)pData->pvArchTMIda;
        pCommArea->pUserIDA = pData;

        /****************************************************************/
        /* supply all information required to create the process        */
        /* window                                                       */
        /****************************************************************/
        pCommArea->sProcessWindowID = ID_ARCHTM_PROC_WINDOW;
        pCommArea->sProcessObjClass = clsANALYSIS;
        pCommArea->Style            = PROCWIN_BATCH;
        pCommArea->sSliderID        = ID_TASLIDER;
        LOADSTRING( NULLHANDLE, hResMod, IDS_ARCHTM_TITLE, pCommArea->szTitle );
        strcat( pCommArea->szTitle, pIda->szArchTM );
        pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_ANAICON); //hiconANA;
        pCommArea->fNoClose         = FALSE;
        pCommArea->swpSizePos.x     = 100;
        pCommArea->swpSizePos.y     = 100;
        pCommArea->swpSizePos.cx    = (SHORT)UtlQueryULong( QL_AVECHARWIDTH ) * 60;
        pCommArea->swpSizePos.cy    = (SHORT)UtlQueryULong( QL_PELSPERLINE ) * 10;
        pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
        pCommArea->asMsgsWanted[1]  = 0;
        pCommArea->usComplete       = 0;
      }
      break;

      /******************************************************************/
      /* WM_EQF_INITIALIZE:                                             */
      /*                                                                */
      /* Start the process                                              */
      /******************************************************************/
    case WM_EQF_INITIALIZE:
      {
        pData =(PFCTDATA) pCommArea->pUserIDA;
        pIda  = (PARCHTM_IDA)pData->pvArchTMIda;

        WinPostMsg( hwnd, WM_EQF_PROCESSTASK, MP1FROMSHORT(BUILDARCHTM_TASK), 0L );
      }
      break;

      /******************************************************************/
      /* WM_CLOSE:                                                      */
      /*                                                                */
      /* Prepare/initialize shutdown of process                         */
      /******************************************************************/
    case WM_CLOSE:
      pData = (PFCTDATA)pCommArea->pUserIDA;
      pIda  = (PARCHTM_IDA)pData->pvArchTMIda;
      if ( pIda )
      {
        pIda->fKill = TRUE;
        mResult = MRFROMSHORT( TRUE );   // = do not close right now
      }
      else
      {
        mResult = MRFROMSHORT( FALSE );  // = continue with close
      } /* endif */
      break;

      /******************************************************************/
      /* WM_DESTROY:                                                    */
      /*                                                                */
      /* Cleanup all resources used by the process                      */
      /******************************************************************/
    case WM_DESTROY:
      pData =(PFCTDATA) pCommArea->pUserIDA;
      pIda  = (PARCHTM_IDA)pData->pvArchTMIda;
      if ( pIda )
      {
        // report end of task to DDE handler
        if ( pIda->fErrorStop )
        {
          pIda->pDDEArchTM->DDEReturn.usRc = UtlGetDDEErrorCode( pIda->pDDEArchTM->hwndErrMsg );
        } /* endif */
        WinPostMsg( pIda->pDDEArchTM->hwndOwner, WM_EQF_DDE_ANSWER, NULL,
                    MP2FROMP(&pIda->pDDEArchTM->DDEReturn) );

        // cleanup
        UtlAlloc( (PVOID *) &pIda, 0L, 0L, NOMSG );
        pCommArea->pUserIDA = NULL;
        if ( pData ) UtlAlloc( (PVOID *)&pData, 0L, 0L, NOMSG );
      } /* endif */
      break;

      /******************************************************************/
      /* WM_EQF_TERMINATE:                                              */
      /*                                                                */
      /* Allow or disable termination of process                        */
      /******************************************************************/
    case WM_EQF_TERMINATE:
      mResult = MRFROMSHORT( FALSE );          // = continue with close
      break;

    case WM_INITMENU:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

      /******************************************************************/
      /* WM_EQF_PROCESSTASK:                                            */
      /*                                                                */
      /* Do the actual processing                                       */
      /******************************************************************/
    case WM_EQF_PROCESSTASK:
      if ( SHORT1FROMMP1(mp1) == BUILDARCHTM_TASK )
      {
        pData =(PFCTDATA) pCommArea->pUserIDA;
        pIda  = (PARCHTM_IDA)pData->pvArchTMIda;
        if ( pIda->fKill )
        {
          // cleanup here!
          EqfRemoveObject( TWBFORCE, hwnd );
        }
        else
        {
          TAFuncArchTMProcess( pData );
          if ( pIda->CurStep != END_PROCESS )
          {
            UtlDispatch();
            WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                        MP1FROMSHORT(BUILDARCHTM_TASK), 0l );
          }
          else
          {
            // end process window
            EqfRemoveObject( TWBFORCE, hwnd);
          } /* endif */
        } /* endif */
      } /* endif */
      break;
  } /* endswitch */
  return( mResult );
} /* end of function TABatchArchTMProcCallBack */

void ArchTMCleanUp( PARCHTM_IDA pIda )
{
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  if ( pIda->pMem != NULL )
  {
    pFactory->closeMemory( pIda->pMem );
    pIda->pMem = NULL;
  } /* endif */

  if ( pIda->pMemOrg != NULL )
  {
    pFactory->closeMemory( pIda->pMemOrg );
    pIda->pMemOrg = NULL;
  } /* endif */

  if ( pIda->pSourceDoc )
  {
    if ( pIda->pSourceDoc->pDocTagTable ) TAFreeTagTable((PLOADEDTABLE) pIda->pSourceDoc->pDocTagTable  );
    TAFreeDoc((PVOID *) &(pIda->pSourceDoc) );
    pIda->pSourceDoc = NULL;
  } /* endif */

  if ( pIda->pTargetDoc )
  {
    TAFreeDoc((PVOID *) &(pIda->pTargetDoc) );
    pIda->pTargetDoc = NULL;
  } /* endif */

  if ( pIda->pLoadedQFTable )
  {
    TAFreeTagTable( pIda->pLoadedQFTable );
  } /* endif */

  /********************************************************/
  /* Rename tempory TM or delete it (in case of errors)   */
  /********************************************************/
  if ( pIda->fClearTM )
  {
    if ( pIda->fErrorStop )
    {
      pFactory->deleteMemory( NULL, pIda->szTempTM );
    }
    else
    {
      pFactory->replaceMemory( NULL, pIda->szArchTM, pIda->szTempTM ); 
    } /* endif */
  } /* endif */

  /********************************************************/
  /* Use archive TM as folder TM if requested             */
  /********************************************************/
  if ( pIda->fUseAsFolderTM && !pIda->fErrorStop )
  {
    HPROP           hFolProp;   // folder properties handler
    PPROPFOLDER     pFolProp;   // folder properties pointer
    EQFINFO         ErrorInfo;  // error returned by property handler

    hFolProp = OpenProperties( pIda->szFolObjName, NULL,
                                PROP_ACCESS_READ, &ErrorInfo);
    if ( hFolProp )
    {
      if ( SetPropAccess( hFolProp, PROP_ACCESS_WRITE) )
      {
        pFolProp = (PPROPFOLDER)MakePropPtrFromHnd( hFolProp );
        strcpy( pFolProp->szLongMemory, pIda->szArchTM );
        strcpy( pFolProp->szMemory, pIda->szShortArchTM );
        SaveProperties( hFolProp, &ErrorInfo );
        ResetPropAccess( hFolProp, PROP_ACCESS_WRITE);
      } /* endif */
      CloseProperties( hFolProp, PROP_FILE, &ErrorInfo);
    } /* endif */
  } /* endif */
}