//------------------------------------------------------------------------------
//EQFMEMRP.C                                                                  
//------------------------------------------------------------------------------
//Copyright Notice:                                                           
//                                                                            
//      Copyright (C) 1990-2015, International Business Machines              
//      Corporation and others. All rights reserved                           
//------------------------------------------------------------------------------
//Description:                                                                
// The function EQFMemOrganizeStart initializes the translation               
// memory organize.                                                           
// The function EQFMemOrganizeProcess extraxts a segment from                 
// a translation memory and adds it to a temporary translation                
// memory.                                                                    
// The function EQFMemOrganizeEnd terminates the organize process             
//------------------------------------------------------------------------------

// ******************* Translation memory organize initialization and start **********

// define for memory import logging (remove to disable logging)
#ifdef _DEBUG
  #define ORGANIZE_LOGGING
#endif

#define INCL_EQF_ANALYSIS         // analysis functions
#define INCL_EQF_TM               // general Transl. Memory functions
#define INCL_EQF_TMREMOTE         // remote Transl. Memory functions (U-Code)

#include "core\PluginManager\PluginManager.h"
#include "core\PluginManager\OtmMemoryPlugin.h"
#include "core\PluginManager\OtmMemory.h"
#include "MemoryFactory.h"
#include "MemoryUtil.h"


#include <time.h>                 // C library for time functions
#include <EQFTMI.H>               // Private header file of Translation Memory
#include <EQFMEM.ID>              // Translation Memory IDs
  #include "OTMFUNC.H"            // public defines for function call interface
  #include "EQFFUNCI.H"           // private defines for function call interface
// #include <TIME.H>                 // C time functions

// import logging 
#ifdef ORGANIZE_LOGGING
  static LONG lOrganizeStartTime = 0;
#endif


#define MEM_START_ORGANIZE  USER_TASK + 1
#define MEM_ORGANIZE_TASK   USER_TASK + 2
#define MEM_END_ORGANIZE    USER_TASK + 3


USHORT CloseTmAndTempTm( PMEM_ORGANIZE_IDA );
USHORT MemFuncPrepOrganize( PFCTDATA pData, PSZ pszMemName );
USHORT MemFuncOrganizeProcess( PFCTDATA pData );

/************************************************/
/* Callback function for Organize ProcessWindow */
/************************************************/

MRESULT MemOrganizeCallBack
(
  PPROCESSCOMMAREA pCommArea,          // ptr to commmunication area
  HWND             hwnd,               // handle of process window
  WINMSG           message,            // message to be processed
  WPARAM           mp1,                // first message parameter
  LPARAM           mp2                 // second message parameter
)
{
  PMEM_ORGANIZE_IDA pRIDA;             // pointer to instance area
  MRESULT           mResult = FALSE;   // return code for handler proc

  switch( message)
  {
    /******************************************************************/
    /* WM_CREATE:                                                     */
    /*                                                                */
    /* Fill fields in communication area                              */
    /* Initialize data of callback function                           */
    /******************************************************************/
    case WM_CREATE :
      {
        /**************************************************************/
        /* Anchor IDA                                                 */
        /**************************************************************/
        pRIDA               = (PMEM_ORGANIZE_IDA)PVOIDFROMMP2(mp2);
        pCommArea->pUserIDA = pRIDA;
        HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);

        /****************************************************************/
        /* supply all information required to create the process        */
        /* window                                                       */
        /****************************************************************/
        pCommArea->sProcessWindowID = ID_MEMORG_WINDOW;
        pCommArea->sProcessObjClass = clsMEMORYORG;
        pCommArea->Style            = pRIDA->fBatch ? PROCWIN_BATCH :
                                                      PROCWIN_SLIDERONLY ;
        pCommArea->sSliderID        = ID_MEMORG_SLIDER;
        LOADSTRING( NULLHANDLE, hResMod, MEM_ORTITLE, pCommArea->szBuffer );
        OEMTOANSI( pRIDA->szMemName );
        sprintf( pCommArea->szTitle, "%s %s", pCommArea->szBuffer,
                 pRIDA->szMemName );
        ANSITOOEM( pRIDA->szMemName );
        pCommArea->hIcon            = (HPOINTER) UtlQueryULong(QL_TMEMORGICON); //hiconTMEMORG;
        pCommArea->fNoClose         = FALSE;
        pCommArea->swpSizePos.x     = 100;
        pCommArea->swpSizePos.y     = 100;
        pCommArea->swpSizePos.cx    = (SHORT) UtlQueryULong( QL_AVECHARWIDTH ) * 60;
        pCommArea->swpSizePos.cy    = (SHORT) UtlQueryULong( QL_PELSPERLINE ) * 8;
        pCommArea->asMsgsWanted[0]  = WM_EQF_PROCESSTASK;
        pCommArea->asMsgsWanted[1]  = 0;
        pCommArea->usComplete       = 0;
      }
      break;


      /****************************************************************/
      /* Start processing by posting WM_EQF_PROCESSTASK               */
      /****************************************************************/
    case WM_EQF_INITIALIZE:
      WinPostMsg( hwnd, WM_EQF_PROCESSTASK,
                  MP1FROMSHORT( MEM_START_ORGANIZE ), 0L );
      break;

    /******************************************************************/
    /* WM_CLOSE:                                                      */
    /*                                                                */
    /* Prepare/initialize shutdown of process                         */
    /******************************************************************/
    case WM_CLOSE:
      pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;
      if ( pRIDA->fBatch )
      {
        mResult = MRFROMSHORT( FALSE );  // = continue with close
      }
      else if ( pRIDA->pMem != NULL )
      {
        //cancel processing?
        if ( UtlError ( ERROR_TMORG_CANCEL, MB_YESNO, 0, NULL, EQF_QUERY ) == MBID_YES )
        {
          EQFMemOrganizeEnd( pCommArea, hwnd, MP2FROMSHORT(1), TRUE );
          mResult = MRFROMSHORT( FALSE );  // = continue with close
        }
        else
        {
          mResult = MRFROMSHORT( TRUE );  // = do not close right now
        } /* endif */
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
      pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;

      if ( pRIDA )
      {
        if ( pRIDA->pProposal != NULL ) delete( pRIDA->pProposal );
        pRIDA->pProposal = NULL;

        if ( pRIDA->pMem != NULL)
        {
          EQFMemOrganizeEnd( pCommArea, hwnd, MP2FROMSHORT(1), TRUE );
        } /* endif */
        if ( pRIDA->fBatch )
        {
          WinPostMsg( pRIDA->pDDEMemOrg->hwndOwner, WM_EQF_DDE_ANSWER, NULL,
                      MP2FROMP(&pRIDA->pDDEMemOrg->DDEReturn) );
        } /* endif */
        if ( pRIDA->pszNameList ) UtlAlloc( (PVOID *)&pRIDA->pszNameList, 0L, 0L, NOMSG );
      } /* endif */

      REMOVESYMBOL( pCommArea->szObjName );
      UtlAlloc( (PVOID *)&pRIDA, 0L, 0L, NOMSG );
      pCommArea->pUserIDA = NULL;
      break;


    /******************************************************************/
    /* WM_EQF_TERMINATE:                                              */
    /*                                                                */
    /* Allow or disable termination of process                        */
    /******************************************************************/
    case WM_EQF_TERMINATE:
      mResult = MRFROMSHORT( FALSE );          // = continue with close
      break;

    /******************************************************************/
    /* WM_INITMENU:                                                   */
    /*                                                                */
    /* Enable/Disable actionbar items                                 */
    /******************************************************************/
    case WM_INITMENU:
      UtlMenuDisableAll( hwnd, SHORT1FROMMP1( mp1) );
      break;

    case WM_EQF_PROCESSTASK:
      switch ( SHORT1FROMMP1(mp1) )
      {
        case MEM_START_ORGANIZE:
          EQFMemOrganizeStart( pCommArea, hwnd );
          break;

        case MEM_ORGANIZE_TASK:
          EQFMemOrganizeProcess( pCommArea, hwnd );
          break;

        case MEM_END_ORGANIZE:
          {
            PMEM_ORGANIZE_IDA pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;
            BOOL fRestart = FALSE;

            // check if more TMs are waiting to be organized
            // show end message if we are done
            if ( pRIDA->pszNameList )
            {
              pRIDA->pszActiveName += strlen(pRIDA->pszActiveName) + 1;
              fRestart = (*pRIDA->pszActiveName != EOS);

              if ( !fRestart )
              {
                // show completion message
                PSZ pReplAddr[3];
                PSZ pszTMNames = NULL;
                CHAR szNumber1[10];    // buffer for character string
                CHAR szNumber2[10];    // buffer for character string

                // setup list of TM names for the display in the message box
                {
                  // compute size of required buffer
                  PSZ pszNext = pRIDA->pszNameList;
                  int iSize = 5;
                  while ( *pszNext != EOS )
                  {
                    int iNameLen = strlen(pszNext);
                    pszNext += iNameLen + 1;
                    iSize += iNameLen + 5;
                  } /* endwhile */

                  // allocate buffer
                  UtlAlloc( (PVOID *)&pszTMNames, 0L, iSize, ERROR_STORAGE );

                  // fill buffer with TM names
                  if ( pszTMNames )
                  {
                    PSZ pszNext = pRIDA->pszNameList;
                    PSZ pszCurPos = pszTMNames;
                    while ( *pszNext != EOS )
                    {
                      int iNameLen = strlen(pszNext);
                      *pszCurPos++ = '\"';
                      strcpy( pszCurPos, pszNext );
                      pszNext += iNameLen + 1;
                      pszCurPos += iNameLen;
                      *pszCurPos++ = '\"';
                      if ( *pszNext != EOS )
                      {
                        // add delimiters for following TM name
                        *pszCurPos++ = ',';
                        *pszCurPos++ = ' ';
                      } /* endif */
                    } /* endwhile */
                  } /* endif */

                }

                pReplAddr[0] = pszTMNames;
                pReplAddr[1] = ltoa( pRIDA->ulSegmentCounter, szNumber1, 10 );
                pReplAddr[2] = ltoa( pRIDA->ulInvSegmentCounter, szNumber2, 10 );
                UtlError( MESSAGE_MULT_MEM_ORGANIZE_COMPLETED, MB_OK, 3,
                          &pReplAddr[0], EQF_INFO );
                if ( pszTMNames ) UtlAlloc( (PVOID *)&pszTMNames, 0L, 0L, NOMSG );
              } /* endif */
            } /* endif */

            // terminate current organize
            EQFMemOrganizeEnd( pCommArea, hwnd, mp2, !fRestart );

            // prepare organize of next TM
            if ( fRestart )
            {
              BOOL fOK = TRUE;
              USHORT usTmtRc = 0;
              MemoryFactory *pFactory = MemoryFactory::getInstance();

              pRIDA->pMem = NULLHANDLE;
              memset( &(pRIDA->stExtIn), 0, sizeof(pRIDA->stExtIn) );
              memset( &(pRIDA->stExtOut), 0, sizeof(pRIDA->stExtOut) );
              memset( &(pRIDA->stPutIn), 0, sizeof(pRIDA->stPutIn) );
              memset( &(pRIDA->stPutOut), 0, sizeof(pRIDA->stPutOut) );
              pFactory->splitObjName( pRIDA->pszActiveName, pRIDA->szPluginName, sizeof(pRIDA->szPluginName), pRIDA->szMemName, sizeof(pRIDA->szMemName) );

              if ( fOK )
              {
                int iRC = 0;  
                pRIDA->pMem = pFactory->openMemory( pRIDA->szPluginName, pRIDA->szMemName, FOR_ORGANIZE, &iRC );
                pRIDA->fFirstGet = TRUE;
                usTmtRc = (USHORT)iRC;
              } /* endif */

              if ( fOK )
              {
                switch ( usTmtRc )
                {
                  case OK:
                  case FILE_MIGHT_BE_CORRUPTED:
                  case VERSION_MISMATCH:
                  case CORRUPT_VERSION_MISMATCH:
                  case BTREE_CORRUPTED:
                   fOK = TRUE;
                   break;

                  default:
                   {
                     pFactory->showLastError( NULL, pRIDA->szMemName, NULL, hwnd );
                     fOK = FALSE;
                   }
                   break;
                } /* endswitch */
              } /* endif */

              if ( fOK )
              {
                // start organize of next TM
                EQFMemOrganizeStart( pCommArea, hwnd );
              }
              else
              {
                // start of next organize failed, so end process window
                EqfRemoveObject( TWBFORCE, hwnd );
              } /* endif */
            } /* endif */
          }
          break;
      } /* endswitch */
      break;
  } /* endswitch */
  return( mResult );
} /* end of function MemOrganizeCallBack */



// ================ Handle the message WM_EQF_MEMORGANIZE_START =======================

USHORT EQFMemOrganizeStart
(
  PPROCESSCOMMAREA  pCommArea,
  HWND              hWnd
)
  // The function initializes the TM organize. It returns TRUE if
  // the initialization was OK else it returns FASLE.
{
    PMEM_ORGANIZE_IDA pRIDA;          // Pointer to the dialog IDA
    USHORT            usRc = TRUE;    // Return code
    PSZ               pReplAddr[4];   // Pointer to an address list of replacement strings
    MemoryFactory *pFactory = MemoryFactory::getInstance();

    // Address the organize IDA
    pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;
    pRIDA->fFirstGet = TRUE;
    pRIDA->pProposal = new OtmProposal();

#ifdef ORGANIZE_LOGGING
  {
    FILE *hfLog = NULL;
    CHAR szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    strcat( szLogFile, "\\MEMORG.LOG" );
    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      time( &lOrganizeStartTime );
      fprintf( hfLog, "************ Memory Organize Log *********************\n"  );
      fprintf( hfLog, "Memory organize started at : %s", asctime( localtime( &lOrganizeStartTime ) ) );
      fprintf( hfLog, "Memory name                : %s\n", pRIDA->szMemName );
      fclose( hfLog );
    } /* endif */
  }
#endif


    // refresh titlebar text (may be a different TM right now)
    if ( !pRIDA->fBatch && (pRIDA->hwndErrMsg != HWND_FUNCIF) )
    {
      HMODULE hResMod = (HMODULE) UtlQueryULong(QL_HRESMOD);
      LOADSTRING( NULLHANDLE, hResMod, MEM_ORTITLE, pCommArea->szBuffer );
      OEMTOANSI( pRIDA->szMemName );
      sprintf( pCommArea->szTitle, "%s %s", pCommArea->szBuffer, pRIDA->szMemName );
      ANSITOOEM( pRIDA->szMemName );
      SetWindowText( hWnd, pCommArea->szTitle );
    } /* endif */

    // --------------------------------------------------------------------
    // Create the Output translation memory
    if ( usRc )
    {
      int iRC = 0;
      OtmMemoryPlugin::MEMORYINFO MemInfo;
      OtmPlugin *pAnyPlugin = (OtmPlugin *)pRIDA->pMem->getPlugin();
      if ( pAnyPlugin->getType() == OtmPlugin::eTranslationMemoryType )
      {
        OtmMemoryPlugin *pPlugin = (OtmMemoryPlugin *)pAnyPlugin;
        pRIDA->pMem->getSourceLanguage( pRIDA->szSourceLanguage, sizeof(pRIDA->szSourceLanguage) );
        pRIDA->pMem->getDescription( pRIDA->szBuffer, sizeof(pRIDA->szBuffer) );
        pPlugin->getMemoryInfo( pRIDA->szMemName, &MemInfo );
      }
      else if ( pAnyPlugin->getType() == OtmPlugin::eSharedTranslationMemoryType )
      {
        OtmSharedMemoryPlugin *pPlugin = (OtmSharedMemoryPlugin *)pAnyPlugin;
        pRIDA->pMem->getSourceLanguage( pRIDA->szSourceLanguage, sizeof(pRIDA->szSourceLanguage) );
        pRIDA->pMem->getDescription( pRIDA->szBuffer, sizeof(pRIDA->szBuffer) );
        pPlugin->getMemoryInfo( pRIDA->szMemName, &MemInfo );
      }
      strcpy( pRIDA->szPluginName, pAnyPlugin->getName() );
      strcpy( pRIDA->szTempMemName, "$Org-" );
      strcat( pRIDA->szTempMemName, pRIDA->szMemName );

      // delete any memory left over from a previous organize call
      pFactory->deleteMemory( pRIDA->szPluginName, pRIDA->szTempMemName );

      if ( MemInfo.szFullPath[0] != '\0' )
      {
        pRIDA->pMemTemp = pFactory->createMemory( pRIDA->szPluginName, pRIDA->szTempMemName, pRIDA->szBuffer, pRIDA->szSourceLanguage, MemInfo.szFullPath[0], NULL, true, &iRC );
      }
      else
      {
        pRIDA->pMemTemp = pFactory->createMemory( pRIDA->szPluginName, pRIDA->szTempMemName, pRIDA->szBuffer, pRIDA->szSourceLanguage, '\0', NULL, true, &iRC );
      }

      if ( (iRC != 0) || (pRIDA->pMemTemp == NULL ) )
      {
        usRc = FALSE;
        pRIDA->fMsg = FALSE;
        pFactory->showLastError( pRIDA->szPluginName, pRIDA->szTempMemName, NULL, pRIDA->hwndErrMsg );
      } /* endif */
    } /* endif */

    // --------------------------------------------------------------------
    // The initialization of translation memory organize failed
    // Close translation memories. Clean the storage allocations.
    if ( !usRc )
    {
      // Issue an error message.
      if ( pRIDA != NULL )
      {
        // Close the Translation memory which should be organized.
        if ( pRIDA->pMemTemp != NULL ) 
        {
          pFactory->closeMemory( pRIDA->pMemTemp );
          pRIDA->pMemTemp = NULL;
          pFactory->deleteMemory( pRIDA->szPluginName, pRIDA->szTempMemName );
        } /* endif */

        // Issue the error message :"Initialization of organize for
        // translation memory %1 failed". Issue the message only if
        // no other significant message has been issued already.
        if ( !pRIDA->fMsg )
        {
          OEMTOANSI( pRIDA->szMemName );
          pReplAddr[0] = pRIDA->szMemName;
          UtlErrorHwnd( ERROR_MEM_ORGANIZE_INITFAILED, MB_CANCEL, 1,
                        &pReplAddr[0], EQF_ERROR, pRIDA->hwndErrMsg );
          ANSITOOEM( pRIDA->szMemName );
        } /* endif */

        if ( pRIDA->fBatch )
        {
          pRIDA->usRC = UtlGetDDEErrorCode( pRIDA->hwndErrMsg );
          if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
          {
            pRIDA->pDDEMemOrg->DDEReturn.usRc = pRIDA->usRC;
          } /* endif */
        } /* endif */

        // Dismiss the slider window if it had been created
        if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
        {
          EqfRemoveObject( TWBFORCE, hWnd );
        } /* endif */
      } /* endif */
    }
    // --------------------------------------------------------------------
    // Move process-ID into mp1 and issue a message WM_EQF_MEMORGANIZE_PROCESS
    else if ( pRIDA->hwndErrMsg == HWND_FUNCIF )
    {
      pRIDA->NextTask = MEM_ORGANIZE_TASK;
    }
    else
    {
      WinPostMsg( hWnd, WM_EQF_PROCESSTASK,
                  MP1FROMSHORT( MEM_ORGANIZE_TASK ), NULL);
    } /* endif */
    return usRc;
  } /* end of function EQFMemOrganizeStart */


// ================ Handle the message WM_EQF_MEMORGANIZE_PROCESS =====================

VOID EQFMemOrganizeProcess
(
  PPROCESSCOMMAREA  pCommArea,
  HWND              hWnd
)
{
    USHORT            usRc = TRUE;    // Return code to control a process
    USHORT            usDosRc;        // Dos Returncode
    PMEM_ORGANIZE_IDA pRIDA;          // Pointer to the organize IDA
    PSZ               pReplAddr[3];   // Arrey of pointers to replacement strings

    // Get the address of the process IDA by means of the process handle
    pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;

    // If usRc. Call the memory database load function
    // and handle return codes
    if ( usRc )
    {
      int iProgress = 0;
      int iRC = 0;

      if ( pRIDA->fFirstGet )
      {
        iRC = pRIDA->pMem->getFirstProposal( *(pRIDA->pProposal), &iProgress );
        pRIDA->fFirstGet = FALSE;
      }
      else
      {
        iRC = pRIDA->pMem->getNextProposal( *(pRIDA->pProposal), &iProgress );
      } /* endif */         

      if ( iRC == NO_ERROR )
      {
        pCommArea->usComplete = (USHORT)iProgress;

        // do some consistency checking
        pRIDA->pProposal->getMarkup( pRIDA->szTagTable, sizeof(pRIDA->szTagTable) );
        pRIDA->pProposal->getTargetLanguage( pRIDA->szTargetLanguage, sizeof(pRIDA->szTargetLanguage) );
        if ( (pRIDA->pProposal->getSourceLen() == 0) || (pRIDA->pProposal->getTargetLen() == 0) ||
             (pRIDA->szTargetLanguage[0] == EOS) || (pRIDA->szTagTable[0] == EOS) )
        {
          // ignore invalid proposal
          pRIDA->ulInvSegmentCounter++;
        }
        else
        {
          // write proposal to output memory
          iRC = pRIDA->pMemTemp->putProposal( *(pRIDA->pProposal) );
          if ( iRC != 0 )
          {
            pRIDA->ulInvSegmentCounter++;
          }
          else
          {
            pRIDA->ulSegmentCounter++;
          } /* endif */             
        } /* endif */

        // Post the next WM_EQF_MEMORGANIZE_PROCESS message but check
        // first whether an other message is in the message queue
        // and if it is dispatch it.
        if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
        {
          UtlDispatch();
          WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_ORGANIZE_TASK ), NULL );
        } /* endif */
      }
      else if ( iRC == OtmMemory::ERROR_ENTRYISCORRUPTED )
      {
        pCommArea->usComplete = (USHORT)iProgress;
        pRIDA->ulInvSegmentCounter++;
        if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
        {
          UtlDispatch();
          WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_ORGANIZE_TASK ), NULL );
        } /* endif */
      }
      else if ( iRC == OtmMemory::INFO_ENDREACHED )
      {
        MemoryFactory *pFactory = MemoryFactory::getInstance();

        pRIDA->pMemTemp->rebuildIndex();

        pFactory->closeMemory( pRIDA->pMemTemp );

        pRIDA->pMemTemp = NULL;

        if (usRc)
        {
          usDosRc = NTMCloseOrganize( pRIDA, FALSE );

          // set the progress indicator to 100 percent
          if ( !pRIDA->fBatch )
          {
            pCommArea->usComplete = 100;
            WinSendMsg( hWnd, WM_EQF_UPDATESLIDER, MP1FROMSHORT( 100 ), NULL );
          } /* endif */

          if ( usDosRc == NO_ERROR )
          {
            // everything in the termination process was ok
            pRIDA->pMem = NULL;

            // end message only in GUI single organize mode
            if ( !pRIDA->fBatch && !pRIDA->pszNameList)
            {
              CHAR szNumber1[10];    // buffer for character string
              CHAR szNumber2[10];    // buffer for character string

              OEMTOANSI( pRIDA->szMemName );
              pReplAddr[0] = pRIDA->szMemName;
              pReplAddr[1] = ltoa( pRIDA->ulSegmentCounter, szNumber1, 10 );
              pReplAddr[2] = ltoa( pRIDA->ulInvSegmentCounter, szNumber2, 10 );
              UtlError( MESSAGE_MEM_ORGANIZE_COMPLETED, MB_OK, 3, &pReplAddr[0], EQF_INFO );
              ANSITOOEM( pRIDA->szMemName );
            } /* endif */
          }
          else
          {
            usRc = FALSE;
          } /* endif */
        } /* endif */

        if (!usRc)
        {
          // something in the termination process failed
          OEMTOANSI( pRIDA->szMemName );
          pReplAddr[0] = pRIDA->szMemName;
          UtlErrorHwnd( ERROR_MEM_ORGANIZE_TERMFAILED, MB_CANCEL, 1, &pReplAddr[0], EQF_ERROR, pRIDA->hwndErrMsg );
          ANSITOOEM( pRIDA->szMemName );
          if ( pRIDA->fBatch )
          {
            pRIDA->usRC = UtlGetDDEErrorCode( pRIDA->hwndErrMsg );
            if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
            {
              pRIDA->pDDEMemOrg->DDEReturn.usRc = pRIDA->usRC;
            } /* endif */
          } /* endif */
        } /* endif */

        // -----------------------------------------------------
        // Issue message WM_EQF_MEMORGANIZE_END
        if ( pRIDA->hwndErrMsg == HWND_FUNCIF )
        {
          pRIDA->NextTask = MEM_END_ORGANIZE;
        }
        else
        {
          WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_END_ORGANIZE ), NULL );
        } /* endif */
      }
      else
      {
        // Issue a message "Translation memory organize abnormally terminated."
        if ( !pRIDA->fBatch )
        {
          OEMTOANSI( pRIDA->szMemName );
          pReplAddr[0] = pRIDA->szMemName;
          UtlError( ERROR_MEM_ORGANIZE_TERMINATED, MB_CANCEL, 1,
                    &pReplAddr[0], EQF_ERROR );
          ANSITOOEM( pRIDA->szMemName );
        }
        else
        {
          pRIDA->usRC = UtlGetDDEErrorCode( pRIDA->hwndErrMsg );
          if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
          {
            pRIDA->pDDEMemOrg->DDEReturn.usRc = pRIDA->usRC;
          } /* endif */
        } /* endif */

        // Close the input translation memory and temporary translation memory
        CloseTmAndTempTm( pRIDA );

          // Delete the temporary translation memory
        MemoryFactory *pFactory = MemoryFactory::getInstance();
        pFactory->deleteMemory( pRIDA->szPluginName, pRIDA->szTempMemName );

        // Issue message WM_EQF_MEMORGANIZE_END
        if ( pRIDA->hwndErrMsg == HWND_FUNCIF )
        {
          pRIDA->NextTask = MEM_END_ORGANIZE;
        }
        else
        {
          WinPostMsg( hWnd, WM_EQF_PROCESSTASK, MP1FROMSHORT( MEM_END_ORGANIZE ), NULL );
        } /* endif */
      } /* endif */
    } /* endif */
} /* end of function EQFMemOrganizeProcess */


// ================ Handle the message WM_EQF_MEMORGANIZE_END =========================

VOID EQFMemOrganizeEnd
(
  PPROCESSCOMMAREA  pCommArea,
  HWND              hWnd,
  LPARAM            mp2,
  BOOL              fTerminate
)
{
    PMEM_ORGANIZE_IDA pRIDA;          // Pointer to the data for the organize process
    PSZ               pReplAddr[2];   // Arrey of pointers to replacement strings

    // Get the address of the process IDA by means of the process handle
    pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;

    // Check if the termination was due to a
    // CLOSE message. Mp2 is in that case not zero.
    if (SHORT1FROMMP2(mp2) != 0 )
    {
      // -----------------------------------------------------
      // Issue a message "Termination of memory database load"
      // "was forced and is not completed"
      if ( !pRIDA->fBatch )
      {
        OEMTOANSI( pRIDA->szMemName );
        pReplAddr[0] = pRIDA->szMemName;
        UtlError( ERROR_MEM_ORGANIZE_TERM_FORCED, MB_CANCEL, 1,
                &pReplAddr[0], EQF_WARNING );
        ANSITOOEM( pRIDA->szMemName );
      } /* endif */
      // -----------------------------------------------------
      // Close the input translation memory and temporary translation memory
      CloseTmAndTempTm( pRIDA );

      // Delete the temporary translation memory
      MemoryFactory *pFactory = MemoryFactory::getInstance();
      pFactory->deleteMemory( pRIDA->szPluginName, pRIDA->szTempMemName );
    }
    else
    {
      // Refresh the memory database list box
      sprintf( pCommArea->szBuffer, "%s:%s", pRIDA->szPluginName, pRIDA->szMemName );

      if ( pRIDA->hwndErrMsg == HWND_FUNCIF )
      {
        ObjBroadcast( WM_EQFN_PROPERTIESCHANGED, PROP_CLASS_MEMORY, pCommArea->szBuffer );
      }
      else
      {
        EqfSend2Handler( MEMORYHANDLER, WM_EQFN_PROPERTIESCHANGED, MP1FROMSHORT( PROP_CLASS_MEMORY ), MP2FROMP( pCommArea->szBuffer ));
      } /* endif */
    } /* endif */

    if ( fTerminate )
    {
      if ( pRIDA->hwndErrMsg != HWND_FUNCIF )
      {
        EqfRemoveObject( TWBFORCE, hWnd );
      } /* endif */
    } /* endif */

#ifdef ORGANIZE_LOGGING
  {
    FILE *hfLog = NULL;
    CHAR szLogFile[MAX_EQF_PATH];

    UtlMakeEQFPath( szLogFile, NULC, LOG_PATH, NULL );
    strcat( szLogFile, "\\MEMORG.LOG" );
    hfLog = fopen( szLogFile, "a" );
    if ( hfLog )
    {
      LONG lCurTime = 0;  

      time( &lCurTime );
      fprintf( hfLog, "Memory organize ended at   : %s", asctime( localtime( &lCurTime ) ) );
      if ( lOrganizeStartTime )
      {
        LONG lDiff = lCurTime - lOrganizeStartTime;
        fprintf( hfLog, "Overall organize time is %ld:%2.2ld:%2.2ld\n", lDiff / 3600, 
                (lDiff - (lDiff / 3600 * 3600)) / 60,
                (lDiff - (lDiff / 3600 * 3600)) % 60 );
      }
      fclose( hfLog );
    } /* endif */
  }
#endif


} /* end of function EQFMemOrganizeEnd */


// ================ Close the Tm to be organized and close the temporary Tm ============

 USHORT CloseTmAndTempTm
 (
    PMEM_ORGANIZE_IDA  pRIDA                  // Pointer to the organize instance area
 )
   // The function returns TRUE if everything OK else FALSE
 {
   USHORT    usTmRc1 = FALSE;        // Tm return code
   USHORT    usTmRc2 = FALSE;        // Tm return code
   USHORT    usRc = TRUE;            // Function return code
   MemoryFactory *pFactory = MemoryFactory::getInstance();

   if ( pRIDA->pMem != NULL )
   {
     // Close the translation memory to be organized
     usTmRc1 = (USHORT)pFactory->closeMemory( pRIDA->pMem );
     pRIDA->pMem = NULL;
   } /* endif */

   if ( pRIDA->pMemTemp != NULL )
   {
     // Close the temporary translation memory
     usTmRc2 = (USHORT)pFactory->closeMemory( pRIDA->pMemTemp );
     pRIDA->pMemTemp = NULL;
   } /* endif */

   if ( usTmRc1 || usTmRc2 )
   {
     usRc = FALSE;
   } /* endif */

 return usRc;
 } /* end of function CloseTmAndTempTm  */


// Organize a TM in function call mode
USHORT MemFuncOrganizeMem
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemName               // Translation Memory being deleted
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  // prepare a new organize run or continue current one
  if ( pData->fComplete )              // has last run been completed
  {
    // prepare a new analysis run
    usRC = MemFuncPrepOrganize( pData, pszMemName );
  }
  else
  {
    // continue current organize process
    usRC = MemFuncOrganizeProcess( pData );
  } /* endif */
  return( usRC );
} /* end of function MemFuncOrganizeMem */

// Prepare the organize of a TM in function call mode
USHORT MemFuncPrepOrganize
(
  PFCTDATA    pData,                   // function I/F session data
  PSZ         pszMemName               // Translation Memory being deleted
)
{
  USHORT      usRC = NO_ERROR;         // function return code
  PPROCESSCOMMAREA pCommArea = NULL;   // ptr to commmunication area
  PMEM_ORGANIZE_IDA pRIDA = NULL;      // pointer to instance area
  BOOL        fOK = TRUE;              // internal O.K. flag
  PSZ         pszParm;                 // error parameter pointer
  CHAR        szMemPath[MAX_EQF_PATH];
  OtmMemory *pMem = NULL;       // TM handle
  SHORT       sRC;                     // value used for symbol checking..
  MemoryFactory *pFactory = MemoryFactory::getInstance();

   // check if a TM has been specified
   if ( fOK )
   {
     if ( (pszMemName == NULL) || (*pszMemName == EOS) )
     {
       fOK = FALSE;
       UtlErrorHwnd( TMT_MANDCMDLINE, MB_CANCEL, 0, NULL,
                     EQF_ERROR, HWND_FUNCIF );
     } /* endif */
   } /* endif */

   // check if there is a TM with the given name
   if ( fOK )
   {
     int iRC = 0;

     pMem = pFactory->openMemory( NULL, pszMemName, FOR_ORGANIZE, &iRC );
     if ( iRC != 0)
     {
       pszParm = pszMemName;
       UtlErrorHwnd( ERROR_MEMORY_NOTFOUND, MB_CANCEL, 1,
                     &pszParm, EQF_ERROR, HWND_FUNCIF );
     } /* endif */
   } /* endif */

   if ( fOK )
   {
     // prepare TM organize process
     if ( fOK )
     {
       // allocate storage for the organize process communication area
       fOK = UtlAllocHwnd( (PVOID *)&pCommArea, 0L,
                           (LONG)sizeof(PROCESSCOMMAREA),
                           ERROR_STORAGE, HWND_FUNCIF );

       // allocate storage for the MEM_ORGANIZE_IDA.
       if ( fOK )
       {
         fOK = UtlAllocHwnd( (PVOID *)&pRIDA, 0L,
                             (LONG)sizeof(MEM_ORGANIZE_IDA),
                             ERROR_STORAGE, HWND_FUNCIF );
       } /* endif */

       if ( fOK )
       {
         // Fill  IDA with necessary values
         pCommArea->pUserIDA = pRIDA;
         pRIDA->pMem = pMem;
         pRIDA->usRC = NO_ERROR;
         strcpy( pRIDA->szMemName, pszMemName );
         pRIDA->fBatch = TRUE;
         pRIDA->hwndErrMsg = HWND_FUNCIF;
         pRIDA->NextTask = MEM_START_ORGANIZE;

         // build object name
         sprintf( szMemPath, "MEMORG: %s", pszMemName );
         strcpy( pCommArea->szObjName, szMemPath );
         sRC = QUERYSYMBOL( szMemPath );
         if ( sRC == -1 )
         {
            // TM is not in use ==> lock this TM
            SETSYMBOL( szMemPath );
         }
         else
         {
           PSZ pTemp = pszMemName;
           UtlErrorHwnd( ERROR_MEM_NOT_ACCESSIBLE, MB_CANCEL,
                         1, &pTemp, EQF_ERROR, HWND_FUNCIF );
           fOK = FALSE;
         } /* endif */

         if ( !fOK )
         {
           // free IDA, otherwise it will be freed in organize process
           UtlAlloc( (PVOID *)&pRIDA, 0L, 0L, NOMSG );
         } /* endif */
       } /* endif */
     } /* endif */
   } /* endif */

   // enable organize process if OK
   if ( fOK )
   {
     pData->fComplete = FALSE;
     pData->sLastFunction = FCT_EQFORGANIZEMEM;
     pData->pvMemOrganizeCommArea = pCommArea;
     pCommArea->pUserIDA = pRIDA;
    } /* endif */

   // cleanup in case of errors
   if ( !fOK )
   {
     usRC = UtlGetDDEErrorCode( HWND_FUNCIF );
     if ( pMem != NULL )
     {
       pFactory->closeMemory( pMem );
     } /* endif */
     pData->fComplete = TRUE;
   } /* endif */

  return( usRC );
} /* end of function MemFuncPrepOrganize */

// Prepare the organize of a TM in function call mode
USHORT MemFuncOrganizeProcess
(
  PFCTDATA    pData                    // function I/F session data
)
{
  USHORT      usRC = NO_ERROR;         // function return code

  PPROCESSCOMMAREA pCommArea;          // ptr to commmunication area
  PMEM_ORGANIZE_IDA pRIDA;             // pointer to instance area

  pCommArea = (PPROCESSCOMMAREA)pData->pvMemOrganizeCommArea;
  pRIDA = (PMEM_ORGANIZE_IDA)pCommArea->pUserIDA;

  switch ( pRIDA->NextTask )
  {
    case MEM_START_ORGANIZE:
      EQFMemOrganizeStart( pCommArea, HWND_FUNCIF );
      usRC = pRIDA->usRC;
      break;

    case MEM_ORGANIZE_TASK:
      EQFMemOrganizeProcess( pCommArea, HWND_FUNCIF );
      usRC = pRIDA->usRC;
      break;

    case MEM_END_ORGANIZE:
      pData->fComplete = TRUE;
      EQFMemOrganizeEnd( pCommArea, HWND_FUNCIF, 0L, TRUE );
      if ( pRIDA->pszNameList ) UtlAlloc( (PVOID *)&pRIDA->pszNameList, 0L, 0L, NOMSG );
      REMOVESYMBOL( pCommArea->szObjName );
      usRC = pRIDA->usRC;
      if ( pCommArea ) UtlAlloc( (PVOID *)&pCommArea, 0L, 0L, NOMSG );
      if ( pRIDA )     UtlAlloc( (PVOID *)&pRIDA, 0L, 0L, NOMSG );
      break;
  } /* endswitch */

  return( usRC );
} /* end of function MemFuncOrganizeProcess */

USHORT
NTMConvertProperties( PPROPTRANSLMEM       pPropOrgTM,
                      PMEM_ORGANIZE_IDA    pRIDA    )
{
  HPROP      hPropTempTM;
  PPROP_NTM  pPropTempTM;
  EQFINFO    ErrorInfo;
  PSZ        pszReplace;
  USHORT     usRc = NO_ERROR;

  /********************************************************************/
  /* create the property file for temp TM                             */
  /********************************************************************/
  hPropTempTM = CreateProperties( pRIDA->szTempPropertyName,
                                  pRIDA->szEqfPath,
                                  PROP_CLASS_MEMORYDB,
                                  &ErrorInfo);

  /****************************************************************/
  /* if the TM property file is not created, display the          */
  /* appropriate error message                                    */
  /****************************************************************/
  if ( !hPropTempTM )
  {
    usRc = TMERR_PROP_WRITE_ERROR;
    pszReplace = pRIDA->szMemName;
    UtlError( ERROR_CREATING_PROPS, MB_CANCEL, 1, &pszReplace, EQF_ERROR );
  }
  else
  {
    /******************************************************************/
    /* the temp properties are successfully created                   */
    /* fill properties structure for temp TM                          */
    /******************************************************************/
    pPropTempTM = (PPROP_NTM) MakePropPtrFromHnd( hPropTempTM );
    strcpy( pPropTempTM->szNTMMarker, NTM_MARKER );
    strcpy( pPropTempTM->stTMSignature.szName, pRIDA->szMemName );
    strcat( pPropTempTM->stTMSignature.szName, EXT_OF_TMDATA );
    strcpy( pPropTempTM->stTMSignature.szServer, pPropOrgTM->szServer );
    strcpy( pPropTempTM->stTMSignature.szUserid, pPropOrgTM->szUserid );
    UtlTime( &pPropTempTM->stTMSignature.lTime );
    strcpy( pPropTempTM->stTMSignature.szSourceLanguage,
            pPropOrgTM->szSourceLang);
    pPropTempTM->stTMSignature.bMajorVersion = TM_MAJ_VERSION;
    pPropTempTM->stTMSignature.bMinorVersion = TM_MIN_VERSION;
    strcpy( pPropTempTM->stTMSignature.szDescription, pPropOrgTM->szMemDesc );
    Utlstrccpy( pPropTempTM->szFullMemName, pPropOrgTM->szFullMemName, DOT );
    strcat( pPropTempTM->szFullMemName, EXT_OF_TMDATA );
    pPropTempTM->usThreshold = TM_DEFAULT_THRESHOLD;

    /******************************************************************/
    /* correct the property name in prophead                          */
    /* the tem properties are created with extension *.TPR and will   */
    /* later be renamed to *.MEM                                      */
    /* Openproperties checks if the filename and the name in the      */
    /* property header are identical, so it must be corrected here    */
    /******************************************************************/
    Utlstrccpy( pPropTempTM->stPropHead.szName, pRIDA->szTempPropertyName, DOT );
    strcat( pPropTempTM->stPropHead.szName, EXT_OF_TMPROP );

    /******************************************************************/
    /* save and close the properties of temp TM                       */
    /******************************************************************/
    if ( SaveProperties( hPropTempTM, &ErrorInfo) )
    {
      usRc = TMERR_PROP_WRITE_ERROR;
      pszReplace = pRIDA->szMemName;
      UtlError( ERROR_WRITE_PROPERTIES, MB_CANCEL, 1, &pszReplace, EQF_ERROR );
    } /* endif */
    CloseProperties( hPropTempTM, PROP_QUIT, &ErrorInfo);
  }/*endif*/
  return usRc;
} /* end of function NTMConvertProperties */

USHORT
NTMCloseOrganize ( PMEM_ORGANIZE_IDA pRIDA,           //pointer to organize IDA
                   USHORT            usMsgHandling )  //message handling flag
{
  USHORT  usURc = NO_ERROR ;   //function returncode
  MemoryFactory *pFactory = MemoryFactory::getInstance();

  usMsgHandling;

  /* close the original TM                                          */
  if ( pRIDA->pMem != NULL )
  {
    int iRC = 0;

    pFactory->closeMemory( pRIDA->pMem );
    pRIDA->pMem = NULL;

    // replace original memory with the organized one
    iRC = pFactory->replaceMemory( pRIDA->szPluginName, pRIDA->szMemName, pRIDA->szTempMemName );

 } /* endif */

 return usURc;
} /* end of function NTMCloseOrganize */
